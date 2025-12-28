/*
 * Z3660 USB Hardware Device Driver for Amiga
 * Header file with device structures and definitions
 * 
 * This device provides USB hardware abstraction for poseidon.library
 * targeting the Z3660 accelerator card's ARM-based USB subsystem.
 */

#ifndef Z3660_USB_SIM_H
#define Z3660_USB_SIM_H

#include <exec/semaphores.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <exec/errors.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/tasks.h>
#include <exec/interrupts.h>

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/dos.h>

#include <libraries/configregs.h>

#include <stdlib.h>
#include <string.h>
#include "devices/usbhardware.h"

/* Kernel printf for debugging */
extern void kprintf(const char *fmt, ...);

/* Missing macros from AROS */
#ifndef NEWLIST
#define NEWLIST(list) (((struct List *)(list))->lh_Head = (struct Node *)&((struct List *)(list))->lh_Tail, \
                       ((struct List *)(list))->lh_Tail = NULL, \
                       ((struct List *)(list))->lh_TailPred = (struct Node *)(list))
#endif

#ifndef ForeachNode
#define ForeachNode(list, node) \
    for (node = (void *)((struct List *)(list))->lh_Head; \
         ((struct Node *)(node))->ln_Succ; \
         node = (void *)((struct Node *)(node))->ln_Succ)
#endif

/* Force byte packing for all USB structures */
#if defined(__GNUC__)
# pragma pack(push, 1)
#endif

#include "devices/usb.h"
#include "devices/usb_hub.h"
#include "devices/newstyle.h"
#include "devices/usbhardware.h"

#if defined(__GNUC__)
# pragma pack(pop)
#endif

#include "z3660_regs.h"

/* Z3660 Hardware Detection */
#define Z3660_VENDOR_ID    0x144B
#define Z3660_PRODUCT_ID   0x0001

/* USB Device Classes */
#define HUB_CLASSCODE      0x09

/* USB Device Speed Constants */
#define USB_SPEED_LOW       0  /* Low Speed (1.5 Mbps) */
#define USB_SPEED_FULL      1  /* Full Speed (12 Mbps) */
#define USB_SPEED_HIGH      2  /* High Speed (480 Mbps) */
#define USB_SPEED_SUPER     3  /* Super Speed (5 Gbps) - future */

/* Z3660 USB Operation Codes for ARM communication */
#define USB_OP_INIT_STACK       0x1001
#define USB_OP_SHUTDOWN_STACK   0x1002
#define USB_OP_CONTROL_XFER     0x1005
#define USB_OP_BULK_XFER        0x1006
#define USB_OP_INT_XFER         0x1007
#define USB_OP_ISO_XFER         0x1008
#define USB_OP_RESET_PORT       0x1010
#define USB_OP_RESUME           0x1011
#define USB_OP_SUSPEND          0x1012
#define USB_OP_QUERY_DEVICE     0x1020
#define USB_OP_ENUMERATE_DEVS   0x1030
#define USB_OP_GET_DEVICE_INFO  0x1031
#define USB_OP_PORT_STATUS      0x1040
#define USB_OP_PORT_CONTROL     0x1041

/* USB Status flags for Z3660 operations */
#define USB_STATUS_READY        0x0001
#define USB_STATUS_BUSY         0x0002
#define USB_STATUS_ERROR        0x0004
#define USB_STATUS_TIMEOUT      0x0008
#define USB_STATUS_COMPLETE     0x0010

/* USB Address Constants */
#define RESERVED_ROOT_HUB_ADDR  127  /* Reserved address for root hub to avoid conflicts with real devices */

/* Debug macros for Amiga using KPrintF for kernel debugging */
//#define DEBUG

void kprintf_good(const char *fmt, ...);
void z3660_usb_debug_init_regs(volatile ULONG *z3660_regs);

/* Logging control
 * LOG_LEVEL: 0=errors only, 1=info, 2=verbose
 */
#ifndef LOG_LEVEL
#define LOG_LEVEL 2
#endif

#if LOG_LEVEL >= 2
#define mybug(l, x) do { kprintf_good x; } while(0)
#define mybug_unit(l, x) do { kprintf_good("%s: ", unit->name); kprintf x; } while(0)
#else
#define mybug(l, x) do { } while(0)
#define mybug_unit(l, x) do { } while(0)
#endif

/* Return codes */
#define RC_OK         0
#define RC_DONTREPLY -1

/* USB simulator unit structure for simple simulation */
struct USBSimUnit {
    UBYTE port_connected;
    UBYTE device_address;
    UBYTE device_speed;
    ULONG unit_num;
};

/* Forward declarations */
struct Z3660USBUnit;
struct Z3660USBBase;

/* Z3660 USB Unit structure - represents a USB host controller */
struct Z3660USBUnit {
    struct Node                  node;
    CONST_STRPTR                 name;
    ULONG                        unit_num;
    ULONG                        state;
    BOOL                         allocated;
    
    struct Z3660USBBase         *base;

    /* Timer for timeouts and periodic operations */
    struct timerequest          *tr;
    struct MsgPort              *mp;

    /* Hardware communication */
    volatile ULONG              *z3660_regs;
    struct ConfigDev            *z3660_cd;

    /* IORequest queues for different transfer types */
    struct IOUsbHWReq           *ioreq_ctrl;
    struct IOUsbHWReq           *ioreq_intr;
    struct IOUsbHWReq           *ioreq_bulk;
    struct IOUsbHWReq           *ioreq_isoc;

    /* Transfer processing task */
    struct Task                 *handler_task;
    ULONG                        handler_task_sig_run;
    ULONG                        handler_task_sig_term;
    
    /* ARM response signals - for efficient Wait() instead of busy polling */
    ULONG                        arm_response_sig;     /* Signal cuando ARM complete operación */
    ULONG                        arm_ready_sig;        /* Signal cuando ARM esté listo para comando */

    /* Transfer status flags */
    BOOL                         ctrlxfer_pending;
    BOOL                         intrxfer_pending;
    BOOL                         bulkxfer_pending;
    BOOL                         isocxfer_pending;

    /* ARM command state */
    BOOL                         arm_cmd_inflight;

    /* INT6 interrupt server for USB wakeups */
    struct Interrupt             usb_int;
    BOOL                         usb_int_added;

    /* Diagnostics and safety controls */
    BOOL                         panic_on_error;   /* if TRUE, trigger fatal stop on first error */
    BOOL                         fatal_stop;       /* once set, all future IOs are immediately failed */
    BOOL                         raw_trace;        /* enable verbose RAW dumps and alignment checks */
    
    /* Hub device tracking for polling workaround */
    UBYTE                        detected_hubs[4]; /* Bitmap for 32 device addresses (4 bytes * 8 bits) */
    ULONG                        hub_poll_counter; /* Counter for hub polling intervals */
    
    /* USB Topology Management */
    struct Z3660USBTopologyNode {
        UBYTE                    device_address;     /* USB device address (1-127) */
        UBYTE                    parent_hub_addr;    /* Address of parent hub (0 = root hub) */
        UBYTE                    parent_hub_port;    /* Port on parent hub (1-based) */
        UBYTE                    device_speed;       /* USB speed: 0=LS, 1=FS, 2=HS */
        UBYTE                    device_class;       /* USB device class */
        UBYTE                    is_hub;             /* 1 if this device is a hub */
        UBYTE                    hub_port_count;     /* Number of downstream ports (if hub) */
        UBYTE                    has_tt;             /* Has Transaction Translator */
        UBYTE                    tt_think_time;      /* TT think time configuration */
        UWORD                    vendor_id;          /* USB vendor ID */
        UWORD                    product_id;         /* USB product ID */
        struct Node              node;               /* For linking in lists */
    }                            topology_nodes[32]; /* Support up to 32 devices */
    struct List                  topology_list;      /* Linked list of active topology nodes */
    struct SignalSemaphore       topology_lock;      /* Protection for topology data */

    /* Transfer queue locks */
    struct SignalSemaphore       ctrlxfer_queue_lock;
    struct SignalSemaphore       intrxfer_queue_lock;
    struct SignalSemaphore       bulkxfer_queue_lock;
    struct SignalSemaphore       isocxfer_queue_lock;

    /* Transfer queues */
    struct List                  ctrlxfer_queue;
    struct List                  intrxfer_queue;
    struct List                  bulkxfer_queue;
    struct List                  isocxfer_queue;

    /* Root Hub emulation */
    struct Z3660USBRootHub {
        struct SignalSemaphore   intrxfer_queue_lock;
        struct List              intrxfer_queue; /* Status Change endpoint */

        UWORD                    addr;
        UWORD                    port_count;

        struct UsbStdDevDesc     devdesc;

        struct RHConfig {
            struct UsbStdCfgDesc cfgdesc;
            struct UsbStdIfDesc  ifdesc;
            struct UsbStdEPDesc  epdesc;
        }                        config;

        union {
            struct UsbHubDesc        hubdesc;
            struct UsbSSHubDesc     sshubdesc;
        };

        struct UsbHubStatus      hubstatus;
        struct UsbPortStatus     portstatus[16]; /* Up to 16 ports */

        /* Minimal state to emulate port connect/change and interrupt bitmap */
        UWORD                    port_connected_bitmap; /* bit per port (1=connected) */
        UWORD                    port_change_bitmap;    /* bit per port (1=changed) */

    }                            roothub;
    struct USBSimUnit simunit;
};

/* Z3660 USB Device base structure */
struct Z3660USBBase {
    struct Device                device;
    
    struct Library              *SysBase;
    struct Library              *ExpansionBase;
    struct Library              *UtilityBase;

    /* Memory management */
    APTR                         MemPool;
    
    /* Hardware access */
    volatile ULONG              *z3660_regs;
    struct ConfigDev            *z3660_cd;
    
    /* Units */
    struct Z3660USBUnit         *usbunit200;  /* USB 2.0 controller */
    struct Z3660USBUnit         *usbunit300;  /* USB 3.0 controller (future) */
    
    /* Unit management */
    struct List                  Units;
    struct SignalSemaphore       unit_lock;
};

/* Global variables (extern) - remove from header to avoid conflicts */

/* Function prototypes */

/* Device functions */
BOOL Z3660USB_InitHardware(struct Z3660USBBase *base);
void Z3660USB_CleanupHardware(struct Z3660USBBase *base);

/* Unit management */
struct Z3660USBUnit *Z3660USB_CreateUnit(struct Z3660USBBase *base, ULONG unit_num);
void Z3660USB_DeleteUnit(struct Z3660USBUnit *unit);
struct Z3660USBUnit *Z3660USB_OpenUnit(struct IOUsbHWReq *ioreq, ULONG unit, struct Z3660USBBase *base);
void Z3660USB_CloseUnit(struct Z3660USBUnit *unit);

/* Command processing */
BOOL cmdAbortIO(struct IOUsbHWReq *ioreq);
WORD cmdFlush(struct IOUsbHWReq *ioreq);
LONG cmdUsbReset(struct IOUsbHWReq *ioreq);
WORD cmdNSDeviceQuery(struct IOStdReq *ioreq);
LONG cmdQueryDevice(struct IOUsbHWReq *ioreq);

/* Transfer commands */
WORD cmdControlXFerRootHub(struct IOUsbHWReq *ioreq);
WORD cmdIntXFerRootHub(struct IOUsbHWReq *ioreq);
LONG cmdControlXfer(struct IOUsbHWReq *ioreq);
WORD cmdBulkXFer(struct IOUsbHWReq *ioreq);
WORD cmdIntXFer(struct IOUsbHWReq *ioreq);
WORD cmdISOXFer(struct IOUsbHWReq *ioreq);

/* USB state commands */
LONG cmdUsbOper(struct IOUsbHWReq *ioreq);
LONG cmdUsbResume(struct IOUsbHWReq *ioreq);
LONG cmdUsbSuspend(struct IOUsbHWReq *ioreq);
LONG cmdUsbOffline(struct IOUsbHWReq *ioreq);

/* Root hub management */
void uhwCheckRootHubChanges(struct Z3660USBUnit *unit);
void uhwInitRootHub(struct Z3660USBUnit *unit);

/* Z3660 hardware communication */
BOOL Z3660USB_FindHardware(struct Z3660USBBase *base);
LONG Z3660USB_SendCommand(struct Z3660USBUnit *unit, ULONG cmd);
LONG Z3660USB_SendCommandForce(struct Z3660USBUnit *unit, ULONG cmd);
LONG Z3660USB_WaitResponse(struct Z3660USBUnit *unit);
BOOL Z3660USB_IsARMReady(struct Z3660USBUnit *unit);
void Z3660USB_WriteReg(struct Z3660USBUnit *unit, ULONG offset, ULONG value);
ULONG Z3660USB_ReadReg(struct Z3660USBUnit *unit, ULONG offset);

/* Global logging and fatal control */
void Z3660USB_SetLogSilence(BOOL on);
void Z3660USB_FatalStop(struct Z3660USBUnit *unit, const char *reason);

/* Error recovery functions */
void z3660_usb_recover_after_error(struct Z3660USBUnit *unit);

/* USB Topology management functions */
void z3660_usb_topology_init(struct Z3660USBUnit *unit);
void z3660_usb_topology_cleanup(struct Z3660USBUnit *unit);
struct Z3660USBTopologyNode *z3660_usb_topology_add_device(struct Z3660USBUnit *unit, 
    UBYTE device_addr, UBYTE parent_hub_addr, UBYTE parent_hub_port, 
    UBYTE device_speed, UBYTE device_class, UWORD vendor_id, UWORD product_id);
void z3660_usb_topology_remove_device(struct Z3660USBUnit *unit, UBYTE device_addr);
struct Z3660USBTopologyNode *z3660_usb_topology_find_device(struct Z3660USBUnit *unit, UBYTE device_addr);
void z3660_usb_topology_device_addressed(struct Z3660USBUnit *unit, UBYTE old_addr, UBYTE new_addr);
void z3660_usb_topology_mark_hub(struct Z3660USBUnit *unit, UBYTE device_addr, UBYTE port_count);
void z3660_usb_topology_print(struct Z3660USBUnit *unit);
BOOL z3660_usb_topology_needs_split_transaction(struct Z3660USBUnit *unit, UBYTE device_addr, 
    UBYTE *hub_addr, UBYTE *hub_port);

/* Hub descriptor processing */
void enhanced_hub_descriptor_processing(struct Z3660USBUnit *unit, UBYTE device_addr, UBYTE *desc, ULONG len);

/* ARM communication functions */
BOOL z3660_usb_init(struct Z3660USBBase *base);
void z3660_usb_cleanup(struct Z3660USBBase *base);
int z3660_usb_ctrl_transfer(struct IOUsbHWReq *ioreq);
int z3660_usb_intr_transfer(struct IOUsbHWReq *ioreq);
int z3660_usb_bulk_transfer(struct IOUsbHWReq *ioreq);
int z3660_usb_isoc_transfer(struct IOUsbHWReq *ioreq);

/* Per-port status/control */
LONG z3660_usb_port_status_ex(struct Z3660USBUnit *unit, UWORD port, UWORD *wPortStatus, UWORD *wPortChange);
LONG z3660_usb_port_control(struct Z3660USBUnit *unit, UWORD port, UWORD feature, BOOL set);
LONG z3660_usb_port_control_force(struct Z3660USBUnit *unit, UWORD port, UWORD feature, BOOL set);
LONG z3660_usb_port_reset(struct Z3660USBUnit *unit);

/* Task management */
void z3660_usb_handler_task(struct Task *parent, struct Z3660USBUnit *unit);

/* Start/stop the async USB handler task (INT6 + timer) */
BOOL z3660_usb_start_handler(struct Z3660USBUnit *unit);
void z3660_usb_stop_handler(struct Z3660USBUnit *unit);

/* External kick from INT6 dispatcher (to be called from your INT6 bottom-half) */
void z3660_usb_on_int6(void);

/* USB Topology Management Functions */
void z3660_usb_topology_init(struct Z3660USBUnit *unit);
void z3660_usb_topology_cleanup(struct Z3660USBUnit *unit);
struct Z3660USBTopologyNode *z3660_usb_topology_add_device(struct Z3660USBUnit *unit, 
    UBYTE device_addr, UBYTE parent_hub_addr, UBYTE parent_hub_port, 
    UBYTE device_speed, UBYTE device_class, UWORD vendor_id, UWORD product_id);
void z3660_usb_topology_remove_device(struct Z3660USBUnit *unit, UBYTE device_addr);
struct Z3660USBTopologyNode *z3660_usb_topology_find_device(struct Z3660USBUnit *unit, UBYTE device_addr);
void z3660_usb_topology_mark_hub(struct Z3660USBUnit *unit, UBYTE device_addr, UBYTE port_count);
void z3660_usb_topology_print(struct Z3660USBUnit *unit);
void z3660_usb_configure_split_transaction(struct IOUsbHWReq *ioreq);
BOOL z3660_usb_topology_needs_split_transaction(struct Z3660USBUnit *unit, UBYTE device_addr, 
    UBYTE *hub_addr, UBYTE *hub_port);

#endif /* Z3660_USB_SIM_H */
