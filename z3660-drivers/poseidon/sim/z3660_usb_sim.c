/*
 * Z3660 USB Simulator Device Driver for Amiga
 * 
 * This is a simplified USB root hub simulator that implements the minimum
 * required functionality to test Poseidon USB stack integration.
 * 
 * Based on the working Ethernet driver structure.
 */

/* Manual type definitions to avoid include problems */
typedef unsigned char   UBYTE;
typedef unsigned short  UWORD;
typedef unsigned long   ULONG;
typedef signed char     BYTE;
typedef signed short    WORD;
typedef signed long     LONG;

/* Structures are defined in z3660_usb_sim.h */

// Standard USB Request Codes (UR_*) - inferred from usage
#define UR_GET_STATUS                           0x00
#define UR_CLEAR_FEATURE                        0x01
#define UR_SET_FEATURE                          0x03
#define UR_SET_ADDRESS                          0x05
#define UR_GET_DESCRIPTOR                       0x06
#define UR_SET_DESCRIPTOR                       0x07
#define UR_GET_CONFIGURATION                    0x08
#define UR_SET_CONFIG                           0x09
#define UR_GET_INTERFACE                        0x0A
#define UR_SET_INTERFACE                        0x0B
#define UR_SYNCH_FRAME                          0x0C

/* USB Hardware commands - missing from includes */
#ifndef UHCMD_USBOFFLINE
#define UHCMD_USBOFFLINE 8
#endif

/* USB Direction constants */
#ifndef UHDIR_IN
#define UHDIR_IN 2
#endif

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <exec/errors.h>
#include <exec/alerts.h>
#include <exec/devices.h>
#include <exec/libraries.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/intuition.h>

#include <libraries/configregs.h>
#include <stdarg.h>
#include <stdio.h>
#include <exec/tasks.h>
#include <exec/semaphores.h>
#include <exec/lists.h>

/* Cache coherency functions */
#include <proto/exec.h>
#include <clib/exec_protos.h>

#include "z3660_usb_sim.h"

/* Global flag to silence logs after fatal stop */
static volatile UBYTE g_log_silenced = 0;
static volatile ULONG *g_arm_debug_regs = NULL;  /* Pointer to Z3660 registers for ARM debug */
static volatile ULONG s_log_seq = 0;             /* Monotonic sequence counter */

/* HID protocol modes */
#define HID_PROTOCOL_BOOT   0
#define HID_PROTOCOL_REPORT 1

/* HID state for the simulated mouse */
struct HIDMouseState {
    UBYTE protocol;     /* Current protocol: 0=boot, 1=report */
    UBYTE idle_rate;    /* Current idle rate (units of 4ms) */
    LONG mouse_x;       /* Current mouse X position */
    LONG mouse_y;       /* Current mouse Y position */
    UBYTE buttons;      /* Current button state */
    LONG wheel;         /* Current wheel position */
    ULONG sim_counter;  /* Simulation counter */
    
    /* Timer-based simulation */
    struct timerequest *timer_req;
    struct MsgPort *timer_port;
    BOOL timer_active;
    int movement_count;
} hid_mouse_state = { HID_PROTOCOL_REPORT, 0, 0, 0, 0, 0, 0, NULL, NULL, FALSE, 0 };
// Global array for simulated units, assuming maximum 16 units as per devOpen logic
#define MAX_USB_SIM_UNITS 16
static struct USBSimUnit g_sim_usb_units[MAX_USB_SIM_UNITS];

/* Initialize ARM debug system */
void z3660_usb_debug_init_regs(volatile ULONG *z3660_regs)
{
    g_arm_debug_regs = z3660_regs;
    if (g_arm_debug_regs) {
        /* Send initialization marker to ARM */
        g_arm_debug_regs[REG_ZZ_DEBUG >> 2] = 0xDB600001;  /* Magic marker for ARM */
        kprintf("[ARM Debug] System initialized with crash-safe logging\n");
    }
}

/* Enhanced kprintf_good that sends debug to ARM for crash-safe logging */
void kprintf_good(const char *fmt, ...)
{
    if (g_log_silenced) {
        return;
    }
    
    static char message_buffer[256];  /* Static to avoid stack issues during crashes */
    va_list args;

    va_start(args, fmt);
    vsprintf(message_buffer, fmt, args);
    va_end(args);

    /* Add sequence counter for timing correlation */
    s_log_seq++;
    
    /* CRITICAL: Send debug message to ARM first (crash-safe) */
    if (g_arm_debug_regs) {
        /* Create a formatted message with sequence number */
        static char arm_message[280];  /* Static to avoid stack issues */
        sprintf(arm_message, "[AMG:%06lu] %s", s_log_seq, message_buffer);
        
        /* CACHE COHERENCY: Ensure data is visible to ARM before sending pointer */
        /* This cleans the CPU cache line containing our message to main memory */
        /* so the ARM can read the correct data from its cache-coherent view */
        {
            ULONG msg_len = strlen(arm_message) + 1;
            CachePreDMA((APTR)arm_message, &msg_len, 0);
        }
        
        /* Send string pointer to ARM via debug register */
        /* ARM will read the string from this address and output it */
        g_arm_debug_regs[REG_ZZ_DEBUG >> 2] = (ULONG)arm_message;
        
        /* Small delay to ensure ARM has time to read the string before we continue */
        volatile int arm_delay = 2000;  /* Adjusted for crash scenarios */
        while (arm_delay--) {
            __asm__ __volatile__("" ::: "memory");  /* Memory barrier */
        }
        
        /* CACHE COHERENCY: Restore cache state after ARM has read the data */
        /* This ensures the CPU cache is synchronized after the DMA-like operation */
        {
            ULONG msg_len2 = strlen(arm_message) + 1;
            CachePostDMA((APTR)arm_message, &msg_len2, 0);
        }
    }
    
    /* SECONDARY: Also send to Amiga console (may be lost in crash) */
    if(0)
    {
        static char amiga_message[320];  /* Static to avoid stack issues */
        sprintf(amiga_message, "[M:%06lu] %s", s_log_seq, message_buffer);
        kprintf(amiga_message);  /* Traditional Amiga output */
    }
}

void Z3660USB_SetLogSilence(BOOL on)
{
    g_log_silenced = on ? 1 : 0;
}

#define DEVNAME             "z3660_usb.device"
#define VERSION             1
#define REVISION            0
#define VSTRING             DEVNAME " 1.0 (" __DATE__ ")"

#ifdef DEBUG
void exit(int hmm) {
	while(1);
}
#endif

/* Required by libc.a even in non-debug builds */
void _exit(int code) {
	while(1);
}

/* Global variables */
struct ExecBase *SysBase = NULL;
struct DosLibrary *DOSBase = NULL;
struct Library *UtilityBase = NULL;
struct ExpansionBase *ExpansionBase = NULL;

/* Static global device base pointer to avoid RTF_AUTOINIT corruption */
static struct Z3660USBBase *g_Z3660USBBase = NULL;

/* Function prototypes for device vectors */
struct Library *devInit(BPTR segList asm("a0"), struct Library *base asm("d0"));
void devOpen(struct Library *base asm("a6"), struct IOUsbHWReq *ioreq asm("a1"), ULONG unit asm("d0"), ULONG flags asm("d1"));
BPTR devClose(struct Library *base asm("a6"), struct IOUsbHWReq *ioreq asm("a1"));
BPTR devExpunge(struct Library *base asm("a6"));
ULONG devReserved(void);
void devBeginIO(struct Library *base asm("a6"), struct IOUsbHWReq *ioreq asm("a1"));
ULONG devAbortIO(struct Library *base asm("a6"), struct IOUsbHWReq *ioreq asm("a1"));

/* Command handler prototypes */
LONG cmdQueryDevice(struct IOUsbHWReq *ioreq);
LONG cmdUsbReset(struct IOUsbHWReq *ioreq);
LONG cmdControlXfer(struct IOUsbHWReq *ioreq);
LONG cmdUsbOper(struct IOUsbHWReq *ioreq);
LONG cmdUsbResume(struct IOUsbHWReq *ioreq);
LONG cmdUsbSuspend(struct IOUsbHWReq *ioreq);
LONG cmdUsbOffline(struct IOUsbHWReq *ioreq);
LONG cmdIntXfer(struct IOUsbHWReq *ioreq);

/* Forward declarations for interrupt processing */
static void periodic_interrupt_processor(void);
static void clear_all_int_requests(void);
static BOOL abort_specific_int_request(struct IOUsbHWReq *target_ioreq);
static void stop_mouse_timer(void);

/* Device function table */
static APTR devVectors[] = {
    (APTR)devOpen,
    (APTR)devClose,
    (APTR)devExpunge,
    (APTR)devReserved,
    (APTR)devBeginIO,
    (APTR)devAbortIO,
    (APTR)-1
};

/* Init data for device */
struct InitData {
    ULONG dataSize;
    APTR *funcTable;
    APTR dataTable;
    APTR initFunc;
};

static struct InitData initData = {
    sizeof(struct Z3660USBBase),
    devVectors,
    NULL,
    (APTR)devInit
};

/* Resident structure */
struct Resident romTag = {
    RTC_MATCHWORD,              /* rt_MatchWord */
    &romTag,                    /* rt_MatchTag */
    &romTag + 1,                /* rt_EndSkip */
    RTF_AUTOINIT,               /* rt_Flags */
    VERSION,                    /* rt_Version */
    NT_DEVICE,                  /* rt_Type */
    0,                          /* rt_Pri */
    DEVNAME,                    /* rt_Name */
    VSTRING,                    /* rt_IdString */
    &initData                   /* rt_Init */
};

/*
 * Device initialization
 */
struct Library *devInit(BPTR segList asm("a0"), struct Library *base asm("d0"))
{
    /* Cast to our device base structure */
    struct Z3660USBBase *dev_base = (struct Z3660USBBase *)base;
    
    SysBase = *(struct ExecBase **)4L;

    /* Multiple debug methods to test what works */
    kprintf_good("***** Z3660 USB DRIVER: devInit STARTED *****\n");
	
    SysBase = *(struct ExecBase **)4L;
    dev_base->SysBase = (struct Library *)SysBase;
    
    /* Initialize device structure */
    dev_base->device.dd_Library.lib_Node.ln_Type = NT_DEVICE;
    dev_base->device.dd_Library.lib_Node.ln_Name = DEVNAME;
    dev_base->device.dd_Library.lib_Version = VERSION;
    dev_base->device.dd_Library.lib_Revision = REVISION;
    dev_base->device.dd_Library.lib_IdString = VSTRING;
        
    /* Open required libraries following ethernet driver pattern */
    if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 36))) {
        if ((UtilityBase = OpenLibrary("utility.library", 37))) {
            if ((dev_base->ExpansionBase = OpenLibrary("expansion.library", 0))) {
                /* Set global ExpansionBase so legacy funcs (FindConfigDev) see it */
                ExpansionBase = (struct ExpansionBase *)dev_base->ExpansionBase;
                kprintf_good("***** devInit: Libraries opened successfully *****\n");
            } else {
                kprintf_good("***** devInit: Failed to open expansion.library *****\n");
                CloseLibrary(UtilityBase);
                CloseLibrary((struct Library *)DOSBase);
                return NULL;
            }
        } else {
            kprintf_good("***** devInit: Failed to open utility.library *****\n");
            CloseLibrary((struct Library *)DOSBase);
            return NULL;
        }
    } else {
        kprintf_good("***** devInit: Failed to open dos.library *****\n");
        return NULL;
    }
        
    dev_base->MemPool = CreatePool(MEMF_PUBLIC | MEMF_CLEAR, 2048, 2048);
    if (!dev_base->MemPool) {
        CloseLibrary(dev_base->ExpansionBase);
        return NULL;
    }
    
    InitSemaphore(&dev_base->unit_lock);
    NEWLIST(&dev_base->Units);
        
    if (!Z3660USB_FindHardware(dev_base)) {
        DeletePool(dev_base->MemPool);
        CloseLibrary(dev_base->ExpansionBase);
        return NULL;
    }
    
    if (!Z3660USB_InitHardware(dev_base)) {
        DeletePool(dev_base->MemPool);
        CloseLibrary(dev_base->ExpansionBase);
        return NULL;
    }
    
    g_Z3660USBBase = dev_base;
    kprintf_good("*** devInit: Stored global base=0x%08lx ***\n", (uint32_t)g_Z3660USBBase);

    kprintf_good("***** Z3660 USB DRIVER - SUCCESS! Returning from devInit *****\n");
    
    return base;
}

/*
 * Device open
 */
void devOpen(struct Library *base asm("a6"), struct IOUsbHWReq *ioreq asm("a1"), ULONG unit asm("d0"), ULONG flags asm("d1"))
{
    struct Z3660USBBase *real_base = (struct Z3660USBBase *)base;
        
    kprintf_good("Z3660USB devOpen: *** DEVICE OPEN CALLED ***\n");
    kprintf_good("devOpen: ioreq=0x%08lx unit=%ld (0x%08lx) flags=0x%08lx base=0x%08lx\n", 
               (uint32_t)ioreq, unit, unit, flags, (uint32_t)base);

    /* Check if base pointer looks corrupted and use global if needed */
    if (g_Z3660USBBase && (!real_base || !real_base->MemPool || real_base != g_Z3660USBBase)) {
        kprintf_good("*** devOpen: Using global base (0x%08lx) instead of passed base (0x%08lx) ***\n", 
                   (uint32_t)g_Z3660USBBase, (uint32_t)real_base);
        real_base = g_Z3660USBBase;
    }

    /* Accept IOStdReq for OpenDevice; I/O commands later will use IOUsbHWReq */
    if (ioreq->iouh_Req.io_Message.mn_Length < sizeof(struct IOStdReq)) {
        kprintf_good("***** devOpen ERROR - Invalid IORequest length *****\n");
        kprintf_good("devOpen: Invalid IORequest length (got %ld, need >= %ld)\n",
                   (uint32_t)ioreq->iouh_Req.io_Message.mn_Length, (uint32_t)sizeof(struct IOStdReq));
        ioreq->iouh_Req.io_Error = IOERR_BADLENGTH;
        return;
    }

    /* Validate unit number - detect corrupted/invalid values */
    if ((LONG)unit == -1) {
        mybug(-1, ("devOpen: unit -1 (probe) mapped to 0\n"));
        unit = 0;
    } else if (unit > 1000000) {
        /* This looks like a corrupted pointer being passed as unit number */
        kprintf_good("*** devOpen: INVALID UNIT NUMBER 0x%08lx (%ld) - looks like pointer! ***\n", unit, unit);
        kprintf_good("*** Mapping invalid unit to 0 ***\n");
        unit = 0;
    } else if (unit > 16) {
        /* More than 16 units seems unreasonable */
        kprintf_good("*** devOpen: Unit number %ld too high, mapping to 0 ***\n", unit);
        unit = 0;
    }

    /* Default to open failure */
    ioreq->iouh_Req.io_Error = IOERR_OPENFAIL;

    /* Try to open the unit */
    ioreq->iouh_Req.io_Unit = (struct Unit *)Z3660USB_OpenUnit(ioreq, unit, real_base);
    if (!ioreq->iouh_Req.io_Unit) {
        kprintf_good("***** devOpen ERROR - Could not open unit *****\n");
        kprintf_good("devOpen: Could not open unit %ld\n", unit);
        return;
    }

    /* Success */
    ioreq->iouh_Req.io_Device = (struct Device *)&real_base->device; /* required for clients */
    ioreq->iouh_Req.io_Error = 0;
    
    real_base->device.dd_Library.lib_OpenCnt++;
    real_base->device.dd_Library.lib_Flags &= ~LIBF_DELEXP;
#if 0
    /* Create USB handler task now that libraries are properly initialized */
    if (real_base->usbunit200 && !real_base->usbunit200->handler_task) {
        kprintf_good("***** devOpen STEP 7 - Creating USB handler task *****\n");
        
        if (z3660_usb_start_handler(real_base->usbunit200)) {
            kprintf_good("***** devOpen STEP 7.1 - USB handler task created successfully *****\n");
        } else {
            kprintf_good("***** devOpen STEP 7.2 - WARNING: Failed to create USB handler task *****\n");            mybug(-1, ("AMIGA: WARNING: Failed to create USB handler task\n"));
            /* Continue anyway - might still work for some operations */
        }
    } else {
        kprintf_good("***** devOpen STEP 7.3 - Handler task already exists or no unit *****\n");
    }
    /* Clear any latched interrupt bits to avoid storm when installing server */
    if (real_base->usbunit200 && real_base->usbunit200->z3660_regs) {
        kprintf_good("***** devOpen STEP 8 - About to clear latched interrupt bits *****\n");
        ULONG st = real_base->usbunit200->z3660_regs[REG_ZZ_INT_STATUS >> 2];
        if (st & 1UL) real_base->usbunit200->z3660_regs[REG_ZZ_CONFIG >> 2] = (ULONG)(8 | 16); /* eth ack */
        if (st & 2UL) real_base->usbunit200->z3660_regs[REG_ZZ_CONFIG >> 2] = (ULONG)(8 | 32); /* audio ack */
        if (st & 4UL) real_base->usbunit200->z3660_regs[REG_ZZ_CONFIG >> 2] = (ULONG)(8 | 64); /* usb ack */
        kprintf_good("***** devOpen STEP 8.1 - Interrupt bits cleared *****\n");
    }

    /* Register INT6 interrupt server for USB wakeups (deferred from init) */
    if (real_base->usbunit200 && !real_base->usbunit200->usb_int_added) {
        kprintf_good("***** devOpen STEP 9 - About to register INT6 interrupt server *****\n");
        struct Z3660USBUnit *u = real_base->usbunit200;
        u->usb_int.is_Node.ln_Type = NT_INTERRUPT;
        u->usb_int.is_Node.ln_Pri = -70;
        u->usb_int.is_Node.ln_Name = "z3660.usb.int6";
        u->usb_int.is_Data = u;
        u->usb_int.is_Code = (APTR)z3660_usb_isr;
        kprintf_good("***** devOpen STEP 9.1 - About to call Forbid *****\n");
        Forbid();
        kprintf_good("***** devOpen STEP 9.2 - About to call AddIntServer *****\n");
        AddIntServer(INTB_EXTER, &u->usb_int);
        kprintf_good("***** devOpen STEP 9.3 - About to call Permit *****\n");
        Permit();
        u->usb_int_added = TRUE;
        kprintf_good("***** devOpen STEP 9.4 - INT6 interrupt server registered successfully *****\n");
    } else {
        kprintf_good("***** devOpen STEP 9.5 - INT6 server already added or no unit *****\n");
    }
#endif
    kprintf_good("***** devOpen - SUCCESS! Device opened successfully *****\n");
}

/*
 * Device close - exactly like Ethernet driver
 */
BPTR devClose(struct Library *base asm("a6"), struct IOUsbHWReq *ioreq asm("a1"))
{
	kprintf_good("[USB-SIM] Closing unit\n");
    struct Z3660USBBase *dev_base = (struct Z3660USBBase *)base;

	dev_base->device.dd_Library.lib_OpenCnt--;

	if (dev_base->device.dd_Library.lib_Flags & LIBF_DELEXP && dev_base->device.dd_Library.lib_OpenCnt == 0) {
		return devExpunge(base);
	}

	return (BPTR)NULL;
}

/*
 * Device expunge - exactly like Ethernet driver
 */
BPTR devExpunge(struct Library *base asm("a6"))
{
    struct Z3660USBBase *dev_base = (struct Z3660USBBase *)base;

	kprintf_good("[USB-SIM] Device expunge\n");

	if (dev_base->device.dd_Library.lib_OpenCnt > 0) {
		dev_base->device.dd_Library.lib_Flags |= LIBF_DELEXP;
		return (BPTR)NULL;
	}

    Remove(&dev_base->device.dd_Library.lib_Node);
    
    /* Stop mouse timer and clean up simulation */
    stop_mouse_timer();
    
    /* Close libraries */
    if (dev_base->ExpansionBase) CloseLibrary(dev_base->ExpansionBase);
    if (UtilityBase) CloseLibrary((struct Library*)UtilityBase);
    if (DOSBase) CloseLibrary((struct Library*)DOSBase);
	
	FreeMem(dev_base, sizeof(struct Z3660USBBase));

	return 0;
}

ULONG devReserved(void)
{
    return 0;
}

/*
 * Begin IO operations
 */
void devBeginIO(struct Library *base asm("a6"), struct IOUsbHWReq *ioreq asm("a1"))
{
	LONG result = RC_OK;

	/* Reduce BeginIO debug spam - only log non-INT commands or every 100th command */
	static ULONG beginIO_counter = 0;
	beginIO_counter++;
	if (ioreq->iouh_Req.io_Command != UHCMD_INTXFER || (beginIO_counter % 100) == 0) {
		kprintf_good("[USB-SIM] BeginIO: Command %u\n", ioreq->iouh_Req.io_Command);
	}
	
    /* Periodic interrupt processing */
    /* Call more frequently when timer is active to catch timer expiration */
    if (hid_mouse_state.timer_active) {
        /* Timer active - check frequently for expiration */
        periodic_interrupt_processor();
    } else if ((beginIO_counter % 10) == 0) {
        /* No timer - call occasionally for other processing */
        periodic_interrupt_processor();
    }

	/* Clear error */
	ioreq->iouh_Req.io_Error = 0;

	/* Handle commands */
	switch (ioreq->iouh_Req.io_Command) {
		case UHCMD_QUERYDEVICE:
			kprintf_good("[USB-SIM] QUERYDEVICE command\n");
			result = cmdQueryDevice(ioreq);
			break;

		case UHCMD_USBRESET:
			kprintf_good("[USB-SIM] USBRESET command\n");
			result = cmdUsbReset(ioreq);
			break;

		case UHCMD_CONTROLXFER:
			kprintf_good("[USB-SIM] CONTROLXFER command\n");
			result = cmdControlXfer(ioreq);
			break;

		case UHCMD_USBOPER:
			kprintf_good("[USB-SIM] USBOPER command - putting controller operational\n");
			result = cmdUsbOper(ioreq);
			break;

		case UHCMD_USBRESUME:
			kprintf_good("[USB-SIM] USBRESUME command\n");
			result = cmdUsbResume(ioreq);
			break;

		case UHCMD_USBSUSPEND:
			kprintf_good("[USB-SIM] USBSUSPEND command\n");
			result = cmdUsbSuspend(ioreq);
			break;

		case UHCMD_USBOFFLINE:
			kprintf_good("[USB-SIM] USBOFFLINE command\n");
			result = cmdUsbOffline(ioreq);
			break;

		case UHCMD_INTXFER:
			kprintf_good("[USB-SIM] INTXFER command\n");
			result = cmdIntXfer(ioreq);
			break;

		default:
			kprintf_good("[USB-SIM] Unknown command %u\n", ioreq->iouh_Req.io_Command);
			ioreq->iouh_Req.io_Error = IOERR_NOCMD;
			result = RC_OK;
			break;
	}

	/* Reply logic - silenced for normal operation */
	if (result != RC_DONTREPLY) {
		if (!(ioreq->iouh_Req.io_Flags & IOF_QUICK)) {
			/* Async operation - reply to message */
			ReplyMsg(&ioreq->iouh_Req.io_Message);
		} else {
			/* Sync operation (IOF_QUICK) - mark as replied but don't send ReplyMsg */
			ioreq->iouh_Req.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
		}
	}
}

/*
 * Abort IO operations
 */
ULONG devAbortIO(struct Library *base asm("a6"), struct IOUsbHWReq *ioreq asm("a1"))
{
	kprintf_good("[USB-SIM] AbortIO: Command %u\n", ioreq->iouh_Req.io_Command);
	
	/* Handle INTXFER aborts - remove from queue if pending */
	if (ioreq->iouh_Req.io_Command == UHCMD_INTXFER) {
		kprintf_good("[USB-SIM] Aborting INT transfer - searching queue...\n");
		
		if (abort_specific_int_request(ioreq)) {
			kprintf_good("[USB-SIM] INT request found and aborted successfully\n");
		} else {
			kprintf_good("[USB-SIM] INT request not found in queue (already processed?)\n");
		}
		return 0;
	}
	
	/* For other commands, we don't have async operations to abort */
	return 0;
}

/*
 * USB Reset command handler
 */
LONG cmdUsbReset(struct IOUsbHWReq *ioreq)
{
    kprintf_good("[USB-SIM] USB Reset - simulating port reset\n");
    struct Z3660USBUnit *unit = (struct Z3660USBUnit *)ioreq->iouh_Req.io_Unit;

    /* Simulate a device being connected after reset */
    unit->simunit.port_connected = TRUE;
    unit->simunit.device_address = 0;  /* Device starts at address 0 */
    unit->simunit.device_speed   = USB_SPEED_HIGH;  /* Simulate high-speed device */

    kprintf_good("[USB-SIM] Simulated device connected at high speed\n");
    return RC_OK;
}

/*
 * Query Device command handler
 */

// Stub for Z3660USB_InitHardware
BOOL Z3660USB_InitHardware(struct Z3660USBBase *dev_base)
{
    kprintf_good("Z3660USB_InitHardware: Simulating hardware initialization.\n");
    return TRUE;
}

// Stub for Z3660USB_OpenUnit - matches header declaration
struct Z3660USBUnit *Z3660USB_OpenUnit(struct IOUsbHWReq *ioreq, ULONG unit_num, struct Z3660USBBase *dev_base)
{
    kprintf_good("Z3660USB_OpenUnit: Simulating opening unit %lu.\n", unit_num);
    if (unit_num < MAX_USB_SIM_UNITS) {
        ioreq->iouh_Req.io_Unit = (struct Unit *)&g_sim_usb_units[unit_num];
        // Return a dummy Z3660USBUnit pointer (cast the USBSimUnit for now)
        return (struct Z3660USBUnit *)&g_sim_usb_units[unit_num];
    }
    kprintf_good("Z3660USB_OpenUnit: Unit %lu out of bounds.\n", unit_num);
    return NULL;
}

// Stub for z3660_usb_start_handler
BOOL z3660_usb_start_handler(struct Z3660USBUnit *unit)
{
    kprintf_good("z3660_usb_start_handler: Simulating starting USB handler.\n");
    if (unit) {
        unit->handler_task = (struct Task *)0x12345678; // Dummy task pointer
        return TRUE;
    }
    return FALSE;
}

LONG cmdQueryDevice(struct IOUsbHWReq *ioreq)
{
    struct TagItem *taglist = (struct TagItem *)ioreq->iouh_Data;

    kprintf_good("[USB-SIM] Processing QUERYDEVICE - START\n");
    kprintf_good("[USB-SIM] ioreq=0x%08lx, iouh_Data=0x%08lx\n", (ULONG)ioreq, (ULONG)ioreq->iouh_Data);

    /* Validate input parameters first */
    if (!ioreq) {
        kprintf_good("[USB-SIM] ERROR: NULL ioreq!\n");
        return IOERR_BADADDRESS;
    }
    
    if (!taglist) {
        kprintf_good("[USB-SIM] ERROR: NULL taglist in iouh_Data!\n");
        ioreq->iouh_Req.io_Error = IOERR_BADADDRESS;
        return RC_OK;
    }
    
    kprintf_good("[USB-SIM] taglist valid at 0x%08lx\n", (ULONG)taglist);

    /* Process tag list with safety checks */
    struct TagItem *tag = taglist;
    int tag_count = 0;
    
    while (tag && tag_count < 20) {  /* Safety limit to prevent infinite loops */
        kprintf_good("[USB-SIM] Processing tag[%d]: ti_Tag=0x%08lx ti_Data=0x%08lx\n", 
                    tag_count, tag->ti_Tag, tag->ti_Data);
        
        if (tag->ti_Tag == TAG_END) {
            kprintf_good("[USB-SIM] Found TAG_END, stopping\n");
            break;
        }
        
        /* Validate ti_Data pointer before using it */
        if (!tag->ti_Data) {
            kprintf_good("[USB-SIM] WARNING: NULL ti_Data for tag 0x%08lx\n", tag->ti_Tag);
            tag++;
            tag_count++;
            continue;
        }
        
        switch (tag->ti_Tag) {
            case UHA_ProductName:
                {
                    STRPTR *strptr = (STRPTR *)tag->ti_Data;
                    if (strptr) {
                        *strptr = "Z3660USB";
                        kprintf_good("[USB-SIM] ProductName set to %s\n", *strptr);
                    }
                }
                break;

            case UHA_Manufacturer:
                {
                    STRPTR *strptr = (STRPTR *)tag->ti_Data;
                    if (strptr) {
                        *strptr = "Z3660";
                        kprintf_good("[USB-SIM] Manufacturer set to %s\n", *strptr);
                    }
                }
                break;

            case UHA_Description:
                {
                    STRPTR *strptr = (STRPTR *)tag->ti_Data;
                    if (strptr) {
                        *strptr = "USB Hub";
                        kprintf_good("[USB-SIM] Description set to %s\n", *strptr);
                    }
                }
                break;

            case UHA_Version:
                kprintf_good("[USB-SIM] Setting Version to %d at 0x%08lx\n", VERSION, tag->ti_Data);
                *((ULONG *)tag->ti_Data) = VERSION;
                kprintf_good("[USB-SIM] Version set successfully\n");
                break;

            case UHA_Revision:
                kprintf_good("[USB-SIM] Setting Revision to %d at 0x%08lx\n", REVISION, tag->ti_Data);
                *((ULONG *)tag->ti_Data) = REVISION;
                kprintf_good("[USB-SIM] Revision set successfully\n");
                break;

            case UHA_Copyright:
                {
                    STRPTR *strptr = (STRPTR *)tag->ti_Data;
                    if (strptr) {
                        *strptr = "(C) 2025 Z3660 Team";
                        kprintf_good("[USB-SIM] Copyright set to %s\n", *strptr);
                    }
                }
                break;

            case UHA_RootHubAddr:
                kprintf_good("[USB-SIM] Setting RootHubAddr to 127 at 0x%08lx\n", tag->ti_Data);
                *((ULONG *)tag->ti_Data) = 127;  /* Use ARM_ROOT_HUB_ADDR equivalent */
                kprintf_good("[USB-SIM] RootHubAddr set to 127 successfully\n");
                break;

            case UHA_DriverVersion:
                {
                    ULONG drvver = (VERSION << 16) | REVISION;
                    kprintf_good("[USB-SIM] Setting DriverVersion to 0x%08lx at 0x%08lx\n", drvver, tag->ti_Data);
                    *((ULONG *)tag->ti_Data) = drvver;
                    kprintf_good("[USB-SIM] DriverVersion set successfully\n");
                }
                break;

            case UHA_Capabilities:
                kprintf_good("[USB-SIM] Setting Capabilities to 0 (basic USB) at 0x%08lx\n", tag->ti_Data);
                *((ULONG *)tag->ti_Data) = 0; /* No special capabilities - basic USB 1.1 */
                kprintf_good("[USB-SIM] Capabilities set successfully\n");
                break;

            default:
                kprintf_good("[USB-SIM] Unknown/unsupported tag 0x%08lx, ignoring\n", tag->ti_Tag);
                break;
        }
        
        tag++;
        tag_count++;
    }
    
    if (tag_count >= 20) {
        kprintf_good("[USB-SIM] WARNING: Hit safety limit, possible infinite loop in tag list\n");
    }
    
    kprintf_good("[USB-SIM] Processing QUERYDEVICE finished successfully (processed %d tags)\n", tag_count);
    return RC_OK;
}

/*
 * Find Z3660 hardware
 */
BOOL Z3660USB_FindHardware(struct Z3660USBBase *base)
{
    struct ConfigDev *cd = NULL;
    cd = FindConfigDev(NULL, Z3660_VENDOR_ID, Z3660_PRODUCT_ID);
    if (!cd) {
        return FALSE;
    }

    /* Store hardware info */
    base->z3660_cd = cd;
    base->z3660_regs = (volatile ULONG *)cd->cd_BoardAddr;
    z3660_usb_debug_init_regs(base->z3660_regs);
    return TRUE;
}

/*
 * USB Descriptor data - simulate a simple HID mouse device
 */
static UBYTE device_descriptor[] = {
    18,     /* bLength */
    0x01,   /* bDescriptorType (Device) */
    0x00, 0x02, /* bcdUSB (USB 2.0) */
    0x00,   /* bDeviceClass */
    0x00,   /* bDeviceSubClass */
    0x00,   /* bDeviceProtocol */
    64,     /* bMaxPacketSize0 */
    0x5E, 0x04, /* idVendor (Microsoft) */
    0x0D, 0x00, /* idProduct (Simulated Mouse) */
    0x00, 0x01, /* bcdDevice */
    1,      /* iManufacturer */
    2,      /* iProduct */
    0,      /* iSerialNumber */
    1       /* bNumConfigurations */
};

static UBYTE config_descriptor[] = {
    9,      /* bLength */
    0x02,   /* bDescriptorType (Configuration) */
    34, 0,  /* wTotalLength */
    1,      /* bNumInterfaces */
    1,      /* bConfigurationValue */
    0,      /* iConfiguration */
    0xA0,   /* bmAttributes (Remote Wakeup) */
    50,     /* bMaxPower (100mA) */
    
    /* Interface Descriptor */
    9,      /* bLength */
    0x04,   /* bDescriptorType (Interface) */
    0,      /* bInterfaceNumber */
    0,      /* bAlternateSetting */
    1,      /* bNumEndpoints */
    0x03,   /* bInterfaceClass (HID) */
    0x01,   /* bInterfaceSubClass (Boot Interface) */
    0x02,   /* bInterfaceProtocol (Mouse) */
    0,      /* iInterface */
    
    /* HID Descriptor */
    9,      /* bLength */
    0x21,   /* bDescriptorType (HID) */
    0x11, 0x01, /* bcdHID */
    0,      /* bCountryCode */
    1,      /* bNumDescriptors */
    0x22,   /* bDescriptorType (Report) */
    62, 0,  /* wDescriptorLength - FIXED: was 52, now correct 62 bytes */
    
    /* Endpoint Descriptor */
    7,      /* bLength */
    0x05,   /* bDescriptorType (Endpoint) */
    0x81,   /* bEndpointAddress (IN Endpoint 1) */
    0x03,   /* bmAttributes (Interrupt) */
    4, 0,   /* wMaxPacketSize */
    10      /* bInterval */
};

static char string_manufacturer[] = "Z3660 Team";
static char string_product[] = "Z3660 USB Simulator Mouse";

/*
 * HID Report Descriptor for a standard 3-button mouse with scroll wheel
 * This describes the format of HID reports the mouse will send
 */
static UBYTE hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x03,        //     Usage Maximum (0x03)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x03,        //     Report Count (3)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5)
    0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};


/* HID Mouse Report format (matches our report descriptor) */
struct MouseReport {
    UBYTE buttons;      /* Buttons: bits 0-2 for buttons 1-3 */
    BYTE  delta_x;      /* X movement delta (-127 to +127) */
    BYTE  delta_y;      /* Y movement delta (-127 to +127) */
    BYTE  wheel;        /* Wheel delta (-127 to +127) */
};

/* Queue for pending interrupt requests */
static struct IOUsbHWReq *pending_int_requests[8];
static int pending_int_count = 0;
static int pending_int_head = 0;
static int pending_int_tail = 0;


/* Add an interrupt request to the queue */
static void queue_int_request(struct IOUsbHWReq *ioreq) {
    if (pending_int_count < 8) {
        pending_int_requests[pending_int_tail] = ioreq;
        pending_int_tail = (pending_int_tail + 1) % 8;
        pending_int_count++;
        /* Silenced queue logging to reduce spam */
    } else {
        kprintf_good("[USB-SIM] INT request queue full, rejecting request\n");
        ioreq->iouh_Req.io_Error = IOERR_ABORTED;
        ReplyMsg(&ioreq->iouh_Req.io_Message);
    }
}

/* Get next interrupt request from queue */
static struct IOUsbHWReq *dequeue_int_request(void) {
    if (pending_int_count > 0) {
        struct IOUsbHWReq *ioreq = pending_int_requests[pending_int_head];
        pending_int_head = (pending_int_head + 1) % 8;
        pending_int_count--;
        return ioreq;
    }
    return NULL;
}

/* Clear all pending interrupt requests - for offline/abort operations */
static void clear_all_int_requests(void) {
    while (pending_int_count > 0) {
        struct IOUsbHWReq *ioreq = dequeue_int_request();
        if (ioreq) {
            ioreq->iouh_Req.io_Error = IOERR_ABORTED;
            ReplyMsg(&ioreq->iouh_Req.io_Message);
        }
    }
}

/* Find and abort a specific interrupt request */
static BOOL abort_specific_int_request(struct IOUsbHWReq *target_ioreq) {
    /* Search for the specific request in the queue */
    BOOL found = FALSE;
    for (int i = 0; i < pending_int_count; i++) {
        int idx = (pending_int_head + i) % 8;
        if (pending_int_requests[idx] == target_ioreq) {
            found = TRUE;
            
            /* Remove this specific request from queue */
            /* Shift remaining elements down */
            for (int j = i; j < pending_int_count - 1; j++) {
                int curr_idx = (pending_int_head + j) % 8;
                int next_idx = (pending_int_head + j + 1) % 8;
                pending_int_requests[curr_idx] = pending_int_requests[next_idx];
            }
            pending_int_count--;
            if (pending_int_count == 0) {
                pending_int_head = 0;
                pending_int_tail = 0;
            } else {
                pending_int_tail = (pending_int_tail - 1 + 8) % 8;
            }
            
            /* Reply to the specific request ONLY ONCE */
            target_ioreq->iouh_Req.io_Error = IOERR_ABORTED;
            ReplyMsg(&target_ioreq->iouh_Req.io_Message);
            break;
        }
    }
    
    return found;
}


/* Timer-based mouse simulation */
static void start_mouse_timer(void)
{
    if (hid_mouse_state.timer_active) return; /* Already running */
    
    /* Create timer port */
    hid_mouse_state.timer_port = CreateMsgPort();
    if (!hid_mouse_state.timer_port) {
        kprintf_good("[USB-SIM] ❌ Failed to create timer port\n");
        return;
    }
    
    /* Create timer request */
    hid_mouse_state.timer_req = (struct timerequest *)CreateIORequest(
        hid_mouse_state.timer_port, sizeof(struct timerequest));
    if (!hid_mouse_state.timer_req) {
        kprintf_good("[USB-SIM] ❌ Failed to create timer request\n");
        DeleteMsgPort(hid_mouse_state.timer_port);
        hid_mouse_state.timer_port = NULL;
        return;
    }
    
    /* Open timer.device */
    if (OpenDevice("timer.device", UNIT_MICROHZ, 
                   (struct IORequest *)hid_mouse_state.timer_req, 0) != 0) {
        kprintf_good("[USB-SIM] ❌ Failed to open timer.device\n");
        DeleteIORequest((struct IORequest *)hid_mouse_state.timer_req);
        DeleteMsgPort(hid_mouse_state.timer_port);
        hid_mouse_state.timer_req = NULL;
        hid_mouse_state.timer_port = NULL;
        return;
    }
    
    hid_mouse_state.timer_active = TRUE;
    hid_mouse_state.movement_count = 0;
    
    /* Start first timer */
    hid_mouse_state.timer_req->tr_node.io_Command = TR_ADDREQUEST;
    hid_mouse_state.timer_req->tr_time.tv_secs = 0;
    hid_mouse_state.timer_req->tr_time.tv_micro = 500000; /* 500ms */
    SendIO((struct IORequest *)hid_mouse_state.timer_req);
    
    kprintf_good("[USB-SIM] ⏰ Mouse timer started - will generate movements every 500ms\n");
}

static void stop_mouse_timer(void)
{
    if (!hid_mouse_state.timer_active) return;
    
    if (hid_mouse_state.timer_req) {
        AbortIO((struct IORequest *)hid_mouse_state.timer_req);
        WaitIO((struct IORequest *)hid_mouse_state.timer_req);
        CloseDevice((struct IORequest *)hid_mouse_state.timer_req);
        DeleteIORequest((struct IORequest *)hid_mouse_state.timer_req);
        hid_mouse_state.timer_req = NULL;
    }
    
    if (hid_mouse_state.timer_port) {
        DeleteMsgPort(hid_mouse_state.timer_port);
        hid_mouse_state.timer_port = NULL;
    }
    
    hid_mouse_state.timer_active = FALSE;
    kprintf_good("[USB-SIM] ⏰ Mouse timer stopped\n");
}

/* Check if timer expired and generate movement */
static BOOL check_mouse_timer(void)
{
    if (!hid_mouse_state.timer_active || !hid_mouse_state.timer_req) return FALSE;
    
    /* Check if timer completed */
    if (CheckIO((struct IORequest *)hid_mouse_state.timer_req)) {
        WaitIO((struct IORequest *)hid_mouse_state.timer_req);
        
        hid_mouse_state.movement_count++;
        
        /* Only generate 20 movements, then stop */
        if (hid_mouse_state.movement_count <= 20) {
            kprintf_good("[USB-SIM] ⏰ Timer expired - movement #%d\n", hid_mouse_state.movement_count);
            
            /* Restart timer for next movement */
            hid_mouse_state.timer_req->tr_node.io_Command = TR_ADDREQUEST;
            hid_mouse_state.timer_req->tr_time.tv_secs = 0;
            hid_mouse_state.timer_req->tr_time.tv_micro = 500000; /* 500ms */
            SendIO((struct IORequest *)hid_mouse_state.timer_req);
            
            return TRUE; /* We have movement! */
        } else {
            kprintf_good("[USB-SIM] 🎆 Mouse simulation COMPLETED after 20 movements\n");
            stop_mouse_timer();
            return FALSE;
        }
    }
    
    return FALSE; /* No movement yet */
}

/* Generate simulated mouse data - TIMER-BASED VERSION */
static void generate_mouse_data(struct MouseReport *report) {
    /* Always initialize to no movement */
    report->delta_x = 0;
    report->delta_y = 0;
    report->buttons = 0;
    report->wheel = 0;
    
    /* Only generate movement when timer expires */
    if (hid_mouse_state.movement_count <= 20 && hid_mouse_state.movement_count > 0) {
        int phase = (hid_mouse_state.movement_count - 1) % 4;
        
        switch (phase) {
            case 0: /* Move right */
                report->delta_x = 15;
                kprintf_good("[USB-SIM] 🐭 MOUSE MOVE #%d: Right (dx=15)\n", hid_mouse_state.movement_count);
                break;
            case 1: /* Move down */
                report->delta_y = 15;
                kprintf_good("[USB-SIM] 🐭 MOUSE MOVE #%d: Down (dy=15)\n", hid_mouse_state.movement_count);
                break;
            case 2: /* Move left */
                report->delta_x = -15;
                kprintf_good("[USB-SIM] 🐭 MOUSE MOVE #%d: Left (dx=-15)\n", hid_mouse_state.movement_count);
                break;
            case 3: /* Move up */
                report->delta_y = -15;
                kprintf_good("[USB-SIM] 🐭 MOUSE MOVE #%d: Up (dy=-15)\n", hid_mouse_state.movement_count);
                break;
        }
    }
    
    /* Update internal state */
    hid_mouse_state.mouse_x += report->delta_x;
    hid_mouse_state.mouse_y += report->delta_y;
    hid_mouse_state.buttons = report->buttons;
    hid_mouse_state.wheel += report->wheel;
}

/* Process pending interrupt requests with simulated data */
static void process_pending_interrupts(void) {
    struct IOUsbHWReq *ioreq;
    struct MouseReport report;
    static ULONG last_process_time = 0;
    
    /* CRITICAL: Always increment counter first to avoid deadlock */
    hid_mouse_state.sim_counter++;
    ULONG current_time = hid_mouse_state.sim_counter;
    
    /* Already logged above if needed */
    
    /* Check if timer generated new movement */
    BOOL has_movement = check_mouse_timer();
    
    /* Only complete interrupt requests when we have real movement */
    if (!has_movement) {
        return; /* No movement, keep requests queued */
    }
    
    kprintf_good("[USB-SIM] 📡 Timer generated movement - completing INT requests\n");
    last_process_time = current_time;
    
    /* Process only ONE request at a time to prevent flooding */
    if ((ioreq = dequeue_int_request()) != NULL) {
        if (ioreq->iouh_Data && ioreq->iouh_Length >= sizeof(struct MouseReport)) {
            /* Generate mouse data */
            generate_mouse_data(&report);
            
            /* Copy to buffer */
            CopyMem(&report, ioreq->iouh_Data, sizeof(struct MouseReport));
            ioreq->iouh_Actual = sizeof(struct MouseReport);
            ioreq->iouh_Req.io_Error = 0;
            
            /* Log mouse movement for debugging */
            if (report.delta_x != 0 || report.delta_y != 0) {
                kprintf_good("[USB-SIM] ✅ Mouse movement sent: dx=%d dy=%d (counter=%lu)\n",
                            report.delta_x, report.delta_y, hid_mouse_state.sim_counter);
            } else if ((hid_mouse_state.sim_counter % 200) == 0) {
                kprintf_good("[USB-SIM] ⏸️  Mouse idle (counter=%lu)\n", hid_mouse_state.sim_counter);
            }
        } else {
            kprintf_good("[USB-SIM] INT transfer: insufficient buffer size\n");
            ioreq->iouh_Req.io_Error = IOERR_BADLENGTH;
            ioreq->iouh_Actual = 0;
        }
        
        /* Complete the request */
        ReplyMsg(&ioreq->iouh_Req.io_Message);
    }
}

/* Periodic interrupt processor - call this from a timer or main loop */
static void periodic_interrupt_processor(void) {
    /* This function can be called periodically to process queued interrupts
     * at a controlled rate instead of processing them immediately */
    process_pending_interrupts();
}

/*
 * Control Transfer command handler
 */
LONG cmdControlXfer(struct IOUsbHWReq *ioreq)
{
    UBYTE *setup = (UBYTE *)&ioreq->iouh_SetupData;
    UBYTE bmRequestType, bRequest;
    UWORD wValue, wIndex, wLength;
    UBYTE *data_buf = (UBYTE *)ioreq->iouh_Data;
    
    kprintf_good("[USB-SIM] CONTROLXFER: setup=0x%08x buffer=0x%08x len=%u\n", 
                (ULONG)setup, (ULONG)data_buf, ioreq->iouh_Length);
    struct Z3660USBUnit *unit = (struct Z3660USBUnit *)ioreq->iouh_Req.io_Unit;
    
    if (!setup) {
        kprintf_good("[USB-SIM] ERROR: No setup data provided - this shouldn't happen!\n");
        /* Try to handle this gracefully - maybe setup data is in buffer? */
        if (data_buf && ioreq->iouh_Length >= 8) {
            kprintf_good("[USB-SIM] Trying to use buffer as setup data...\n");
            setup = data_buf;
        } else {
            kprintf_good("[USB-SIM] Cannot recover - returning BADADDRESS\n");
            ioreq->iouh_Req.io_Error = IOERR_BADADDRESS;
            return RC_OK;
        }
    }

    /* Parse setup packet */
    bmRequestType = setup[0];
    bRequest = setup[1];
    wValue = (setup[3] << 8) | setup[2];
    wIndex = (setup[5] << 8) | setup[4];
    wLength = (setup[7] << 8) | setup[6];

    kprintf_good("[USB-SIM] Control transfer: dev=%u bmReq=0x%02x bReq=%u wVal=0x%04x wIdx=0x%04x wLen=%u\n",
                unit->simunit.device_address, bmRequestType, bRequest, wValue, wIndex, wLength);

    /* Handle standard USB requests */
    if (bRequest == UR_SET_ADDRESS && bmRequestType == 0x00) {
        /* SET_ADDRESS - simulate success */
        UBYTE new_addr = wValue & 0xFF;
        kprintf_good("[USB-SIM] SET_ADDRESS to %u - SUCCESS\n", new_addr);
        ((struct USBSimUnit *)unit)->device_address = new_addr;
        ioreq->iouh_Actual = 0; // Corrected from iouh_ActualLength
        ioreq->iouh_Req.io_Error = 0;
        return RC_OK;
    }
    else if (bRequest == UR_GET_DESCRIPTOR && (bmRequestType & 0x80)) {
        /* GET_DESCRIPTOR */
        UBYTE desc_type = (wValue >> 8) & 0xFF;
        UBYTE desc_index = wValue & 0xFF;
        
        kprintf_good("[USB-SIM] GET_DESCRIPTOR type=%u index=%u len=%u\n", desc_type, desc_index, wLength);
        
        if (desc_type == 0x01 && desc_index == 0) {
            /* Device Descriptor */
            UWORD copy_len = (wLength < sizeof(device_descriptor)) ? wLength : sizeof(device_descriptor);
            if (data_buf) {
                for (int i = 0; i < copy_len; i++) {
                    data_buf[i] = device_descriptor[i];
                }
            }
            ioreq->iouh_Actual = copy_len; // Corrected from iouh_ActualLength
            ioreq->iouh_Req.io_Error = 0;
            kprintf_good("[USB-SIM] Returned device descriptor (%u bytes)\n", copy_len);
            return RC_OK;
        }
        else if (desc_type == 0x02 && desc_index == 0) {
            /* Configuration Descriptor */
            UWORD copy_len = (wLength < sizeof(config_descriptor)) ? wLength : sizeof(config_descriptor);
            if (data_buf) {
                for (int i = 0; i < copy_len; i++) {
                    data_buf[i] = config_descriptor[i];
                }
            }
            ioreq->iouh_Actual = copy_len; // Corrected from iouh_ActualLength
            ioreq->iouh_Req.io_Error = 0;
            kprintf_good("[USB-SIM] Returned configuration descriptor (%u bytes)\n", copy_len);
            return RC_OK;
        }
        else if (desc_type == 0x22 && desc_index == 0) {
            /* HID Report Descriptor */
            UWORD copy_len = (wLength < sizeof(hid_report_descriptor)) ? wLength : sizeof(hid_report_descriptor);
            if (data_buf) {
                for (int i = 0; i < copy_len; i++) {
                    data_buf[i] = hid_report_descriptor[i];
                }
            }
            ioreq->iouh_Actual = copy_len;
            ioreq->iouh_Req.io_Error = 0;
            kprintf_good("[USB-SIM] Returned HID report descriptor (%u bytes)\n", copy_len);
            return RC_OK;
        }
        else if (desc_type == 0x03) {
            /* String Descriptor */
            if (desc_index == 0) {
                /* Language IDs */
                static UBYTE lang_desc[] = {4, 0x03, 0x09, 0x04}; /* English US */
                UWORD copy_len = (wLength < sizeof(lang_desc)) ? wLength : sizeof(lang_desc);
                if (data_buf) {
                    for (int i = 0; i < copy_len; i++) {
                        data_buf[i] = lang_desc[i];
                    }
                }
                ioreq->iouh_Actual = copy_len; // Corrected from iouh_ActualLength
                ioreq->iouh_Req.io_Error = 0;
                kprintf_good("[USB-SIM] Returned language descriptor\n");
                return RC_OK;
            }
            else if (desc_index == 1 && data_buf && wLength >= 2) {
                /* Manufacturer String */
                char *str = string_manufacturer;
                int str_len = 0;
                while (str[str_len]) str_len++; /* Get length */
                
                data_buf[0] = (str_len * 2) + 2; /* bLength */
                data_buf[1] = 0x03; /* bDescriptorType */
                
                int copy_chars = ((wLength - 2) / 2);
                if (copy_chars > str_len) copy_chars = str_len;
                
                for (int i = 0; i < copy_chars; i++) {
                    data_buf[2 + (i * 2)] = str[i];
                    data_buf[3 + (i * 2)] = 0; /* Unicode high byte */
                }
                
                ioreq->iouh_Actual = 2 + (copy_chars * 2); // Corrected from iouh_ActualLength
                ioreq->iouh_Req.io_Error = 0;
                kprintf_good("[USB-SIM] Returned manufacturer string\n");
                return RC_OK;
            }
            else if (desc_index == 2 && data_buf && wLength >= 2) {
                /* Product String */
                char *str = string_product;
                int str_len = 0;
                while (str[str_len]) str_len++; /* Get length */
                
                data_buf[0] = (str_len * 2) + 2; /* bLength */
                data_buf[1] = 0x03; /* bDescriptorType */
                
                int copy_chars = ((wLength - 2) / 2);
                if (copy_chars > str_len) copy_chars = str_len;
                
                for (int i = 0; i < copy_chars; i++) {
                    data_buf[2 + (i * 2)] = str[i];
                    data_buf[3 + (i * 2)] = 0; /* Unicode high byte */
                }
                
                ioreq->iouh_Actual = 2 + (copy_chars * 2); // Corrected from iouh_ActualLength
                ioreq->iouh_Req.io_Error = 0;
                kprintf_good("[USB-SIM] Returned product string\n");
                return RC_OK;
            }
        }
        
        /* Unsupported descriptor */
        kprintf_good("[USB-SIM] Unsupported descriptor type=%u index=%u\n", desc_type, desc_index);
        ioreq->iouh_Req.io_Error = IOERR_NOCMD;
        return RC_OK;
    }
    else if (bRequest == UR_SET_CONFIG && bmRequestType == 0x00) {
        /* SET_CONFIGURATION - simulate success */
        UBYTE config = wValue & 0xFF;
        kprintf_good("[USB-SIM] SET_CONFIGURATION %u - SUCCESS\n", config);
        ioreq->iouh_Actual = 0; // Corrected from iouh_ActualLength
        ioreq->iouh_Req.io_Error = 0;
        return RC_OK;
    }
    else if (bRequest == USR_GET_STATUS && (bmRequestType & 0x80)) {
        /* GET_STATUS */
        kprintf_good("[USB-SIM] GET_STATUS for %s\n", 
                    (bmRequestType & 0x03) == URTF_DEVICE ? "device" :
                    (bmRequestType & 0x03) == URTF_INTERFACE ? "interface" :
                    (bmRequestType & 0x03) == URTF_ENDPOINT ? "endpoint" : "other");
        
        if (data_buf && wLength >= 2) {
            /* Always return zeros - no special status */
            data_buf[0] = 0;
            data_buf[1] = 0;
            ioreq->iouh_Actual = 2;
            ioreq->iouh_Req.io_Error = 0;
            return RC_OK;
        }
    }
    else if (bRequest == USR_CLEAR_FEATURE && (bmRequestType & 0x03) == URTF_ENDPOINT) {
        /* CLEAR_FEATURE for endpoint */
        UWORD endpoint_idx = wIndex & 0x0F;
        UWORD endpoint_dir = wIndex & 0x80;
        UWORD feature = wValue;
        
        kprintf_good("[USB-SIM] CLEAR_FEATURE for endpoint %u %s, feature=%u\n", 
                    endpoint_idx, endpoint_dir ? "IN" : "OUT", feature);
        
        /* Just accept it - we don't actually track endpoint status */
        ioreq->iouh_Actual = 0;
        ioreq->iouh_Req.io_Error = 0;
        return RC_OK;
    }
    else if (bmRequestType == 0x21) {
        /* HID Class-specific requests (OUT) */
        if (bRequest == 0x0A) {
            /* SET_IDLE */
            UBYTE idle_rate = (wValue >> 8) & 0xFF;  /* Duration in 4ms units */
            UBYTE report_id = wValue & 0xFF;         /* Report ID (0 = all reports) */
            hid_mouse_state.idle_rate = idle_rate;
            kprintf_good("[USB-SIM] SET_IDLE: rate=%u (4ms units), report_id=%u - SUCCESS\n", idle_rate, report_id);
            ioreq->iouh_Actual = 0;
            ioreq->iouh_Req.io_Error = 0;
            return RC_OK;
        }
        else if (bRequest == 0x0B) {
            /* SET_PROTOCOL */
            UBYTE protocol = wValue & 0xFF;  /* 0=boot, 1=report */
            if (protocol <= 1) {
                hid_mouse_state.protocol = protocol;
                kprintf_good("[USB-SIM] SET_PROTOCOL: %s protocol - SUCCESS\n", 
                            protocol ? "REPORT" : "BOOT");
                ioreq->iouh_Actual = 0;
                ioreq->iouh_Req.io_Error = 0;
                return RC_OK;
            } else {
                kprintf_good("[USB-SIM] SET_PROTOCOL: Invalid protocol %u - STALL\n", protocol);
                ioreq->iouh_Req.io_Error = IOERR_NOCMD;
                return RC_OK;
            }
        }
    }
    else if (bmRequestType == 0xA1) {
        /* HID Class-specific requests (IN) */
        if (bRequest == 0x02) {
            /* GET_IDLE */
            if (data_buf && wLength >= 1) {
                data_buf[0] = hid_mouse_state.idle_rate;
                ioreq->iouh_Actual = 1;
                ioreq->iouh_Req.io_Error = 0;
                kprintf_good("[USB-SIM] GET_IDLE: returned rate=%u\n", hid_mouse_state.idle_rate);
                return RC_OK;
            }
        }
        else if (bRequest == 0x03) {
            /* GET_PROTOCOL */
            if (data_buf && wLength >= 1) {
                data_buf[0] = hid_mouse_state.protocol;
                ioreq->iouh_Actual = 1;
                ioreq->iouh_Req.io_Error = 0;
                kprintf_good("[USB-SIM] GET_PROTOCOL: returned %s protocol\n", 
                            hid_mouse_state.protocol ? "REPORT" : "BOOT");
                return RC_OK;
            }
        }
    }
    
    /* Default fallback for any unhandled request */
    kprintf_good("[USB-SIM] Unhandled control request bmReq=0x%02x bReq=%u\n", bmRequestType, bRequest);
    ioreq->iouh_Req.io_Error = IOERR_NOCMD;
    return RC_OK;
}

/*
 * USB Operation commands - put controller in operational state
 */
LONG cmdUsbOper(struct IOUsbHWReq *ioreq)
{
    kprintf_good("[USB-SIM] USB Controller going OPERATIONAL\n");
    
    /* Simulate controller becoming operational */
    struct Z3660USBUnit *unit = (struct Z3660USBUnit *)ioreq->iouh_Req.io_Unit;
    
    /* Mark controller as operational */
    unit->simunit.port_connected = FALSE;  /* No devices connected yet */
    unit->simunit.device_address = 0;
    
    kprintf_good("[USB-SIM] Controller is now OPERATIONAL and ready\n");
    return RC_OK;
}

LONG cmdUsbResume(struct IOUsbHWReq *ioreq)
{
    kprintf_good("[USB-SIM] USB Resume - controller resuming from suspend\n");
    return RC_OK;
}

LONG cmdUsbSuspend(struct IOUsbHWReq *ioreq)
{
    kprintf_good("[USB-SIM] USB Suspend - controller entering suspend mode\n");
    return RC_OK;
}

LONG cmdUsbOffline(struct IOUsbHWReq *ioreq)
{
    kprintf_good("[USB-SIM] USB Offline - controller going offline\n");
    
    /* Stop the mouse timer if running */
    stop_mouse_timer();
    
    /* Clear all pending interrupt requests */
    kprintf_good("[USB-SIM] Clearing all pending INT requests...\n");
    clear_all_int_requests();
    
    kprintf_good("[USB-SIM] All pending requests cleared, controller is offline\n");
    return RC_OK;
}

/*
 * Interrupt Transfer command handler - USB REAL BEHAVIOR
 * This simulates real USB behavior: wait for data, then respond
 */
LONG cmdIntXfer(struct IOUsbHWReq *ioreq)
{
    struct Z3660USBUnit *unit = (struct Z3660USBUnit *)ioreq->iouh_Req.io_Unit;
    struct MouseReport report;
    
    kprintf_good("[USB-SIM] INT transfer: dev=%u ep=%u dir=%u len=%u\n",
                ioreq->iouh_DevAddr, ioreq->iouh_Endpoint, 
                ioreq->iouh_Dir, ioreq->iouh_Length);
    
    /* TEMPORARY FIX: Auto-connect a device if none is connected */
    if (unit->simunit.port_connected == FALSE || unit->simunit.device_address == 0) {
        kprintf_good("[USB-SIM] Auto-connecting simulated mouse device at address 1\n");
        unit->simunit.port_connected = TRUE;
        unit->simunit.device_address = 1;
        unit->simunit.device_speed = USB_SPEED_HIGH;
    }
    
    /* Validate this is for our mouse device - accept both address 0 (initial) and 1 (assigned) */
    if (ioreq->iouh_DevAddr != 0 && ioreq->iouh_DevAddr != 1) {
        kprintf_good("[USB-SIM] INT transfer for unsupported device addr %d, rejecting\n", ioreq->iouh_DevAddr);
        ioreq->iouh_Req.io_Error = IOERR_NOCMD;
        return RC_OK;
    }
    
    /* Validate endpoint (should be endpoint 1 IN for mouse) */
    if (ioreq->iouh_Endpoint != 1 || ioreq->iouh_Dir != UHDIR_IN) {
        kprintf_good("[USB-SIM] INT transfer: wrong endpoint or direction\n");
        ioreq->iouh_Req.io_Error = IOERR_NOCMD;
        return RC_OK;
    }
    
    /* REAL USB BEHAVIOR: Wait for timer to have data, then respond */
    
    /* Start timer if not active */
    if (!hid_mouse_state.timer_active) {
        kprintf_good("[USB-SIM] 🎯 Starting mouse timer for first INT request\n");
        start_mouse_timer();
    }
    
    /* Wait for timer to expire (simulates waiting for USB interrupt) */
    if (hid_mouse_state.timer_active && hid_mouse_state.timer_req) {
        kprintf_good("[USB-SIM] ⏳ Waiting for timer to expire (simulating USB interrupt)...\n");
        
        /* Wait for timer signal */
        ULONG timer_mask = 1L << hid_mouse_state.timer_port->mp_SigBit;
        ULONG signals = Wait(timer_mask);
        
        if (signals & timer_mask) {
            kprintf_good("[USB-SIM] ⏰ Timer expired! Processing mouse data...\n");
            
            /* Get the timer message */
            GetMsg(hid_mouse_state.timer_port);
            
            /* Increment movement count */
            hid_mouse_state.movement_count++;
            
            /* Generate movement data */
            generate_mouse_data(&report);
            
            /* Copy to output buffer */
            if (ioreq->iouh_Data && ioreq->iouh_Length >= sizeof(struct MouseReport)) {
                CopyMem(&report, ioreq->iouh_Data, sizeof(struct MouseReport));
                ioreq->iouh_Actual = sizeof(struct MouseReport);
                ioreq->iouh_Req.io_Error = 0;
                
                kprintf_good("[USB-SIM] 🐭 Mouse data sent: dx=%d dy=%d (movement #%d)\n",
                            report.delta_x, report.delta_y, hid_mouse_state.movement_count);
            } else {
                ioreq->iouh_Actual = 0;
                ioreq->iouh_Req.io_Error = IOERR_BADLENGTH;
            }
            
            /* Restart timer if we haven't reached the limit */
            if (hid_mouse_state.movement_count < 20) {
                hid_mouse_state.timer_req->tr_node.io_Command = TR_ADDREQUEST;
                hid_mouse_state.timer_req->tr_time.tv_secs = 0;
                hid_mouse_state.timer_req->tr_time.tv_micro = 500000; /* 500ms */
                SendIO((struct IORequest *)hid_mouse_state.timer_req);
                kprintf_good("[USB-SIM] ⏰ Timer restarted for next movement\n");
            } else {
                kprintf_good("[USB-SIM] 🎆 Simulation complete after 20 movements\n");
                stop_mouse_timer();
            }
        }
    } else {
        /* No timer - return idle data */
        report.delta_x = 0;
        report.delta_y = 0;
        report.buttons = 0;
        report.wheel = 0;
        
        if (ioreq->iouh_Data && ioreq->iouh_Length >= sizeof(struct MouseReport)) {
            CopyMem(&report, ioreq->iouh_Data, sizeof(struct MouseReport));
            ioreq->iouh_Actual = sizeof(struct MouseReport);
            ioreq->iouh_Req.io_Error = 0;
            kprintf_good("[USB-SIM] 😴 Mouse idle - no timer\n");
        }
    }
    
    return RC_OK; /* Always respond immediately after Wait() */
}
