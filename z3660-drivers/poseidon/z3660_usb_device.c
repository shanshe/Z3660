/*
 * Z3660 USB Device Driver for Amiga
 */

// Standard USB Request Codes (UR_*)
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

/* USB Hardware commands */
#ifndef UHCMD_USBOFFLINE
#define UHCMD_USBOFFLINE 8
#endif

/* USB Direction constants */
#ifndef UHDIR_IN
#define UHDIR_IN 2
#endif

/* Assembly function macros for interrupt handlers */
#include <include/compiler.h>

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

#include "z3660_usb_arm.h"

/* Static global device base pointer */
static struct Z3660USBBase *g_Z3660USBBase = NULL;

/* HID protocol modes */
#define HID_PROTOCOL_BOOT   0
#define HID_PROTOCOL_REPORT 1

// Assuming maximum 16 units as per devOpen logic
#define MAX_USB_UNITS 16

/*
 * INT6 Interrupt Service Routine for USB events from ARM
 * Called when ARM activates amiga_interrupt_set(AMIGA_INTERRUPT_USB)
 */
ASM ULONG z3660_usb_isr(ASMR(a1) struct Z3660USBUnit *unit ASMREG(a1))
{
    volatile ULONG *z3660_regs = unit->z3660_regs;
    ULONG int_status, handled = 0;
//    kprintf_good("[USB ISR] *** z3660_regs %p ***\n",z3660_regs);
    
    if (!z3660_regs) {
        kprintf_good("[USB ISR] *** Not our interrupt ***\n");
        return 0;  /* Not our interrupt */
    }
    
    /* Read interrupt status register */
    int_status = z3660_regs[REG_ZZ_INT_STATUS >> 2];
//    kprintf_good("[USB ISR] *** int_status %d ***\n",int_status);
    
    /* Check if USB interrupt bit is set */
    if (int_status & 4) {  /* AMIGA_INTERRUPT_USB = 4 */
//        kprintf_good("[USB ISR] *** USB INTERRUPT DETECTED *** int_status=0x%08lx\n", int_status);
        
        /* Clear USB interrupt bit FIRST to prevent infinite loop */
        z3660_regs[REG_ZZ_CONFIG >> 2] = 8 | 64;  /* Clear USB interrupt */
        
        /* Signal the task waiting for interrupt data - ALWAYS signal on any USB interrupt */
        if (unit->arm_int_signal != -1 && unit->waiting_task) {
//            kprintf_good("[USB ISR] Signaling task 0x%08lx with signal %u\n", 
//                        (ULONG)unit->waiting_task, unit->arm_int_signal);
            Signal(unit->waiting_task, 1L << unit->arm_int_signal);
            handled = 1;
        } else {
            kprintf_good("[USB ISR] WARNING: No waiting task or invalid signal (signal=%d, task=0x%08lx)\n", 
                        unit->arm_int_signal, (ULONG)unit->waiting_task);
        }
    }
    
    return handled;
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
    /* Clear any latched interrupt bits to avoid storm when installing server */
    if (real_base->usbunit && real_base->usbunit->z3660_regs) {
        kprintf_good("***** devOpen STEP 8 - About to clear latched interrupt bits *****\n");
        ULONG st = real_base->usbunit->z3660_regs[REG_ZZ_INT_STATUS >> 2];
        if (st & 1UL) real_base->usbunit->z3660_regs[REG_ZZ_CONFIG >> 2] = (ULONG)(8 | 16); /* eth ack */
        if (st & 2UL) real_base->usbunit->z3660_regs[REG_ZZ_CONFIG >> 2] = (ULONG)(8 | 32); /* audio ack */
        if (st & 4UL) real_base->usbunit->z3660_regs[REG_ZZ_CONFIG >> 2] = (ULONG)(8 | 64); /* usb ack */
        kprintf_good("***** devOpen STEP 8.1 - Interrupt bits cleared *****\n");
    }

    /* Register INT6 interrupt server for USB wakeups (deferred from init) */
    if (real_base->usbunit && !real_base->usbunit->usb_int_added) {
        kprintf_good("***** devOpen STEP 9 - About to register INT6 interrupt server *****\n");
        struct Z3660USBUnit *u = real_base->usbunit;
        u->usb_int.is_Node.ln_Type = NT_INTERRUPT;
        u->usb_int.is_Node.ln_Pri = -70;
        u->usb_int.is_Node.ln_Name = "z3660.usb.int6";
        u->usb_int.is_Data = u;
        kprintf_good("***** usbunit %p *****\n",u);
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
    /* BeginIO logging disabled to reduce spam */
    #ifdef USB_DEBUG_BEGINOI
    kprintf_good("[USB-SIM] BeginIO: Command %ld\n", ioreq->iouh_Req.io_Command);
    #endif
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
            /* CONTROLXFER logging disabled to reduce spam */
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
            /* INTXFER logging disabled to reduce spam */
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
    unit->port_connected = TRUE;
    unit->device_address = 0;  /* Device starts at address 0 */
    unit->device_speed   = USB_SPEED_HIGH;  /* Simulate high-speed device */

    kprintf_good("[USB-SIM] Simulated device connected at high speed\n");
    return RC_OK;
}

/*
 * Query Device command handler
 */

// Z3660USB_InitHardware - Initialize ARM USB communication
BOOL Z3660USB_InitHardware(struct Z3660USBBase *dev_base)
{
    kprintf_good("Z3660USB_InitHardware: Initializing ARM USB communication\n");
    
    if (!dev_base) {
        kprintf_good("ERROR: Z3660USB_InitHardware - Invalid base pointer\n");
        return FALSE;
    }
    
    // Initialize ARM debug system for crash-safe logging
    if (dev_base->z3660_regs) {
        z3660_usb_debug_init_regs(dev_base->z3660_regs);
        kprintf_good("ARM debug logging initialized\n");
    }
    
    // Initialize ARM USB stack
    if (z3660_usb_arm_init(dev_base) != RC_OK) {
        kprintf_good("ERROR: Failed to initialize ARM USB stack\n");
        return FALSE;
    }
    
    kprintf_good("Z3660USB_InitHardware: ARM USB communication initialized successfully\n");
    return TRUE;
}

// Z3660USB_OpenUnit - properly create and initialize Z3660USBUnit
struct Z3660USBUnit *Z3660USB_OpenUnit(struct IOUsbHWReq *ioreq, ULONG unit_num, struct Z3660USBBase *dev_base)
{
    kprintf_good("Z3660USB_OpenUnit: Opening unit %lu with base=0x%08lx\n", unit_num, (uint32_t)dev_base);
    
    if (!dev_base) {
        kprintf_good("ERROR: Z3660USB_OpenUnit - Invalid base pointer\n");
        return NULL;
    }
    
    if (unit_num >= MAX_USB_UNITS) {
        kprintf_good("Z3660USB_OpenUnit: Unit %lu out of bounds.\n", unit_num);
        return NULL;
    }
    
    // For now, use the existing usbunit if unit_num == 0, or allocate from pool
    struct Z3660USBUnit *unit = NULL;
    
    if (unit_num == 0) {
        // Use the pre-allocated unit for unit 0
        if (!dev_base->usbunit) {
            // Allocate unit from memory pool
            unit = (struct Z3660USBUnit *)AllocPooled(dev_base->MemPool, sizeof(struct Z3660USBUnit));
            if (!unit) {
                kprintf_good("ERROR: Failed to allocate Z3660USBUnit from pool\n");
                return NULL;
            }
            
            // Initialize the unit
            memset(unit, 0, sizeof(struct Z3660USBUnit));
            unit->base = dev_base;
            unit->unit_num = unit_num;
            unit->name = "z3660.usb.0";
            unit->z3660_regs = dev_base->z3660_regs;
            unit->z3660_cd = dev_base->z3660_cd;
            
            // Initialize ARM signals for Wait() pattern
            unit->arm_response_sig = AllocSignal(-1);  /* -1 = any available signal */
            unit->arm_ready_sig = AllocSignal(-1);
            unit->arm_int_signal = AllocSignal(-1);    /* NEW: Signal for interrupt data ready */
//            unit->waiting_task = NULL;                 /* No task waiting initially */
            unit->waiting_task = FindTask(NULL);

            if (unit->arm_response_sig == -1 || unit->arm_ready_sig == -1 || unit->arm_int_signal == -1) {
                kprintf_good("ERROR: Failed to allocate ARM signals for Wait() pattern\n");
                if (unit->arm_response_sig != -1) FreeSignal(unit->arm_response_sig);
                if (unit->arm_ready_sig != -1) FreeSignal(unit->arm_ready_sig);
                if (unit->arm_int_signal != -1) FreeSignal(unit->arm_int_signal);
                FreePooled(dev_base->MemPool, unit, sizeof(struct Z3660USBUnit));
                return NULL;
            }
            
            kprintf_good("Z3660USB_OpenUnit: ARM signals allocated: response=%u ready=%u int=%u\n",
                        unit->arm_response_sig, unit->arm_ready_sig, unit->arm_int_signal);
            
            // Initialize simulation unit for compatibility
            unit->unit_num = unit_num;
            unit->port_connected = FALSE;
            unit->device_address = 0;
            unit->device_speed = USB_SPEED_HIGH;
            
            dev_base->usbunit = unit;
            kprintf_good("Z3660USB_OpenUnit: Created and stored new unit 0 at 0x%08lx\n", (uint32_t)unit);
        } else {
            unit = dev_base->usbunit;
            kprintf_good("Z3660USB_OpenUnit: Using existing unit 0 at 0x%08lx\n", (uint32_t)unit);
        }
    } else {
        kprintf_good("Z3660USB_OpenUnit: Unit %lu not supported yet\n", unit_num);
        return NULL;
    }
    
    // Set the IORequest unit pointer
    ioreq->iouh_Req.io_Unit = (struct Unit *)unit;
    
    kprintf_good("Z3660USB_OpenUnit: Success! Unit %lu opened, base=0x%08lx\n", 
                unit_num, (uint32_t)unit->base);
    
    return unit;
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

/*
 * Control Transfer command handler - ARM Communication Version
 */
LONG cmdControlXfer(struct IOUsbHWReq *ioreq)
{
    /* Reduced logging - only log significant control transfers */
    static int log_counter = 0;
    if ((++log_counter % 10) == 1 || ioreq->iouh_Length > 18) {
        kprintf_good("[ARM] CTRL: dev=%ld len=%ld\n", ioreq->iouh_DevAddr, ioreq->iouh_Length);
    }
    
    struct Z3660USBUnit *unit = (struct Z3660USBUnit *)ioreq->iouh_Req.io_Unit;
    if (!unit || !unit->base) {
        kprintf_good("[ARM] ERROR: Invalid unit or base in control transfer\n");
        ioreq->iouh_Req.io_Error = IOERR_BADADDRESS;
        return RC_OK;
    }
    
    /* Call ARM control transfer function */
    LONG result = z3660_usb_arm_control_transfer(unit, ioreq);
    if (result != RC_OK) {
        kprintf_good("[ARM] Control transfer failed with error %ld\n", result);
        ioreq->iouh_Req.io_Error = IOERR_ABORTED;
        return RC_OK;
    }
    
    /* Success - ARM function sets iouh_Actual */
    ioreq->iouh_Req.io_Error = 0;
    return RC_OK;
}
LONG cmdIntXfer(struct IOUsbHWReq *ioreq)
{
    struct Z3660USBUnit *unit = (struct Z3660USBUnit *)ioreq->iouh_Req.io_Unit;
    
    /* Reduced logging - only log non-hub interrupt transfers */
    if (ioreq->iouh_DevAddr != 1 || ioreq->iouh_Endpoint != 1) {
        kprintf_good("[ARM] INT: dev=%u ep=%u len=%u\n",
                    ioreq->iouh_DevAddr, ioreq->iouh_Endpoint, ioreq->iouh_Length);
    }
    
    if (!unit || !unit->base) {
        kprintf_good("[ARM] ERROR: Invalid unit or base in interrupt transfer\n");
        ioreq->iouh_Req.io_Error = IOERR_BADADDRESS;
        return RC_OK;
    }
    
    /* CRITICAL: Use synchronous Wait() pattern for USB interrupt transfers
     * This pattern follows the correct USB hardware behavior where the driver
     * waits for an interrupt from the USB device before completing the transfer.
     * 
     * 1. Send INT request to ARM (async mode)
     * 2. Wait() for ARM to signal when data is ready
     * 3. Complete transfer and return to Poseidon
     * 
     * This prevents 100% CPU usage by avoiding busy polling.
     */
    
//    kprintf_good("[ARM] Using Wait() pattern for interrupt transfer\n");
    
    /* Step 1: Start async interrupt transfer - ARM will queue it */
    LONG result = z3660_usb_arm_interrupt_transfer_async(unit, ioreq);
    if (result != RC_OK) {
        kprintf_good("[ARM] Failed to start async interrupt transfer: %ld\n", result);
        ioreq->iouh_Req.io_Error = IOERR_ABORTED;
        ioreq->iouh_Actual = 0;
        return RC_OK;
    }
    
    /* Step 2: CRITICAL - Wait for ARM to signal data ready
     * This blocks the thread until ARM signals data is available from USB device */
//    kprintf_good("[ARM] Waiting for interrupt data from USB device...\n");
    
    /* Store current task so ISR can signal us */
    unit->waiting_task = FindTask(NULL);
//    kprintf_good("[ARM] Waiting task unit->waiting_task %p\n", unit->waiting_task);
    
    ULONG arm_mask = 1L << unit->arm_int_signal;  /* Convert signal number to bitmask */
//    kprintf_good("[ARM] About to Wait() for signal %u (mask=0x%08lx)\n", unit->arm_int_signal, arm_mask);
    ULONG signals = Wait(arm_mask);  /* BLOCKS HERE until data ready */
    
//    kprintf_good("[ARM] *** WOKE UP FROM Wait() *** received signals=0x%08lx (expected=0x%08lx)\n", 
//                signals, arm_mask);
    
    /* Clear waiting task - we're no longer waiting */
//    unit->waiting_task = NULL;
    
    /* Step 3: ARM signaled data ready → read result and return */
    if (signals & arm_mask) {
//        kprintf_good("[ARM] Correct signal received! Getting interrupt transfer data\n");
        result = z3660_usb_arm_interrupt_transfer_complete(unit, ioreq);
        
        if (result != RC_OK) {
            kprintf_good("[ARM] Interrupt transfer completion failed: %ld\n", result);
            ioreq->iouh_Req.io_Error = IOERR_ABORTED;
            ioreq->iouh_Actual = 0;
        } else {
            /* Success */
            ioreq->iouh_Req.io_Error = 0;
//            kprintf_good("[ARM] Interrupt transfer completed successfully, actual=%lu\n", ioreq->iouh_Actual);
        }
    } else {
        kprintf_good("[ARM] Wait() returned unexpected signals: 0x%08lx\n", signals);
        ioreq->iouh_Req.io_Error = IOERR_ABORTED;
        ioreq->iouh_Actual = 0;
    }
    
    return RC_OK;  /* Always respond after Wait() */
}

/*
 * USB Operation commands - put controller in operational state
 */
LONG cmdUsbOper(struct IOUsbHWReq *ioreq)
{
    kprintf_good("[ARM] USB Controller going OPERATIONAL\n");
    
    struct Z3660USBUnit *unit = (struct Z3660USBUnit *)ioreq->iouh_Req.io_Unit;
    if (!unit || !unit->base) {
        kprintf_good("[ARM] ERROR: Invalid unit in USB Oper\n");
        ioreq->iouh_Req.io_Error = IOERR_BADADDRESS;
        return RC_OK;
    }
    
    /* Initialize ARM USB stack if needed */
    LONG result = z3660_usb_arm_init(unit->base);
    if (result != RC_OK) {
        kprintf_good("[ARM] Failed to initialize USB stack\n");
        ioreq->iouh_Req.io_Error = IOERR_ABORTED;
        return RC_OK;
    }
    
    kprintf_good("[ARM] Controller is now OPERATIONAL and ready\n");
    ioreq->iouh_Req.io_Error = 0;
    return RC_OK;
}

LONG cmdUsbResume(struct IOUsbHWReq *ioreq)
{
    kprintf_good("[ARM] USB Resume - controller resuming from suspend\n");
    ioreq->iouh_Req.io_Error = 0;
    return RC_OK;
}

LONG cmdUsbSuspend(struct IOUsbHWReq *ioreq)
{
    kprintf_good("[ARM] USB Suspend - controller entering suspend mode\n");
    ioreq->iouh_Req.io_Error = 0;
    return RC_OK;
}

LONG cmdUsbOffline(struct IOUsbHWReq *ioreq)
{
    kprintf_good("[ARM] USB Offline - controller going offline\n");
    
    struct Z3660USBUnit *unit = (struct Z3660USBUnit *)ioreq->iouh_Req.io_Unit;
    if (unit && unit->base) {
        /* Shutdown ARM USB stack */
        z3660_usb_arm_shutdown(unit->base);
    }

    /* Clear all pending interrupt requests */
    kprintf_good("[ARM] Clearing all pending INT requests...\n");
    clear_all_int_requests();
    
    kprintf_good("[ARM] All pending requests cleared, controller is offline\n");
    ioreq->iouh_Req.io_Error = 0;
    return RC_OK;
}
