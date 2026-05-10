#if 0 // Not used
/*
 * Z3660 USB Device Handler for ARM side
 * 
 * This file implements the ARM-side handling of USB operations
 * commanded from the Amiga side via memory-mapped registers.
 * 
 * Communication Protocol:
 * - Amiga writes commands to REG_ZZ_USB_CMD_OP
 * - Amiga provides data via REG_ZZ_USB_DATA_PTR
 * - ARM processes and updates REG_ZZ_USB_STATUS  
 * - ARM provides response data via REG_ZZ_USB_DATA_PTR
 */

#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "usb.h"
#include "usb/usb.h"
#include "usb/ehci.h"
#include <xil_cache.h>
#include <sleep.h>
#include "rtg/zzregs.h"
#include "xparameters.h"
#include "xpseudo_asm_gcc.h"
#include "interrupt.h"
#include "xtime_l.h"  // For XTime timer functions

// Define errno constants that may not be available in standalone BSP
#ifndef ENODEV
#define ENODEV 19  /* No such device */
#endif
#ifndef ENXIO
#define ENXIO  6   /* No such device or address */
#endif

// USB operation codes (matching the Amiga device driver)
#define USB_OP_INIT_STACK       0x1001
#define USB_OP_SHUTDOWN_STACK   0x1002
#define USB_OP_CONTROL_XFER     0x1005
#define USB_OP_BULK_XFER        0x1006
#define USB_OP_INT_XFER         0x1007
#define USB_OP_INT_XFER_ASYNC   0x1008  /* NEW: Async interrupt transfer - ARM signals when data ready */
#define USB_OP_ISO_XFER         0x1009  /* Moved to avoid conflict */
#define USB_OP_RESET_PORT       0x1010
#define USB_OP_QUERY_DEVICE     0x1020
#define USB_OP_ENUMERATE_DEVS   0x1030
#define USB_OP_GET_DEVICE_INFO  0x1031
#define USB_OP_PORT_STATUS      0x1040
#define USB_OP_PORT_CONTROL     0x1041
#define USB_OP_ADD_POLLING_DEVICE    0x1050
#define USB_OP_REMOVE_POLLING_DEVICE 0x1051
#define USB_OP_LIST_POLLING_DEVICES  0x1052

// USB status flags (matching the Amiga device driver)
#define USB_STATUS_READY        0x0001
#define USB_STATUS_BUSY         0x0002
#define USB_STATUS_ERROR        0x0004
#define USB_STATUS_TIMEOUT      0x0008
#define USB_STATUS_COMPLETE     0x0010

// Device types for ARM polling (matching Amiga side)
#define POLLING_DEVICE_MOUSE      1
#define POLLING_DEVICE_KEYBOARD   2
#define POLLING_DEVICE_JOYSTICK   3
#define POLLING_DEVICE_GENERIC    4

// Maximum number of devices that can be polled simultaneously
#define MAX_POLLING_DEVICES       16

// Structure for storing polling device information
typedef struct {
    uint8_t device_addr;     // USB device address
    uint8_t endpoint;        // Endpoint number
    uint8_t interval_ms;     // Polling interval in milliseconds
    uint8_t device_type;     // Device type (mouse, keyboard, etc.)
    uint8_t active;          // 1 if active, 0 if inactive
    uint32_t last_poll_time; // Last time this device was polled (in ms)
    uint8_t data_buffer[64]; // Buffer for storing last poll data
    uint32_t data_length;    // Length of data in buffer
    uint32_t poll_count;     // Number of successful polls
    uint32_t error_count;    // Number of poll errors
    
    // NEW: Persistent interrupt queue to avoid memory leaks
    struct int_queue *int_queue; // Persistent interrupt queue
    struct usb_device *udev;     // Cached USB device pointer
    uint32_t queue_create_time;  // When the queue was created
    uint32_t queue_recreate_count; // How many times we've recreated the queue
} polling_device_t;
/* Device address 0 is special - just pass commands through without tracking state */

/* Global state for USB operations */
static struct {
    uint32_t status;           // Current USB system status
    uint32_t operation;        // Current operation being processed
    uint32_t data_buffer;      // Pointer to data buffer in shared memory (legacy)
    uint32_t data_length;      // Length of data in buffer or response count
    uint32_t buffer_select;    // Current buffer selection (parameter index)
    uint32_t params[8];        // Small parameter array filled via BUFSEL+CMD_DATA
    uint32_t param_index;      // Current parameter index set by BUFSEL
    uint32_t portsc1;          // Pointer to data buffer in shared memory (legacy)
    int initialized;           // Whether USB stack is initialized
    int offline;               // Whether USB is intentionally offline/shutdown
    uint8_t hub_protocol;      // Hub protocol (0=FS,1=HS Single TT,2=HS Multi TT,3=SS)
} usb_state = {
    .status = USB_STATUS_READY,
    .operation = 0,
    .data_buffer = 0,
    .data_length = 0,
    .buffer_select = 0,
    .params = {0},
    .param_index = 0,
    .initialized = 0,
    .offline = 1
};

// Global polling device table
static polling_device_t polling_devices[MAX_POLLING_DEVICES];
static uint8_t polling_device_count = 0;

// Global ASYNC interrupt request storage
static struct {
    uint8_t pending;
    uint8_t dev_addr;
    uint8_t ep;
    uint8_t dir;
    uint32_t data_ptr;
    uint32_t data_len;
    uint32_t request_time;
} async_int_request = {0};

// Function prototypes
static void z3660_usb_process_command(uint32_t command);
static void log_portsc(const char *tag);
static void log_core_regs(const char *tag);
static int arm_add_polling_device(uint8_t device_addr, uint8_t endpoint, uint8_t interval_ms, uint8_t device_type);
static int arm_remove_polling_device(uint8_t device_addr, uint8_t endpoint);
static int arm_list_polling_devices(void);
static uint32_t get_system_time_ms(void);
int z3660_usb_polling_data_available(struct pt *pt, uint8_t device_addr, uint8_t endpoint);
int z3660_usb_get_polling_data(uint8_t device_addr, uint8_t endpoint, void *buffer, uint32_t max_len);

// USB interrupt queue function declarations
extern struct int_queue *create_int_queue(struct usb_device *dev, unsigned long pipe, 
                                         int queuesize, int elementsize, void *buffer, int interval);
extern void *poll_int_queue(struct usb_device *dev, struct int_queue *queue);
extern int destroy_int_queue(struct usb_device *dev, struct int_queue *queue);
/* Forward declaration to avoid implicit non-static declaration before definition */
static void check_for_hub_device(const unsigned char *descriptor_data, int data_len);
static int usb_control_msg_with_split_retry(struct usb_device *udev, unsigned long pipe,
                                           unsigned char request, unsigned char requesttype,
                                           unsigned short value, unsigned short index,
                                           void *data, unsigned short size, int timeout);

extern uint32_t counts_per_second;

#ifndef DEBUG_DESC_DUMP
#define DEBUG_DESC_DUMP 1  // Enable device detection logging
#endif

// Forward declarations for HID interface parsing
static void parse_hid_interfaces_and_register(unsigned char dev, const void *config_data, int config_len);

/*
 * Parse USB Configuration Descriptor to find HID interfaces and register them for polling
 * This properly detects HID devices regardless of VID/PID
 */
static void parse_hid_interfaces_and_register(unsigned char dev, const void *config_data, int config_len)
{
    if (!config_data || config_len < 9 || dev == 0) {
        return; // Invalid data or root hub
    }
    
    const unsigned char *data = (const unsigned char *)config_data;
    int offset = 0;
    
    printf("[ARM POLLING] Parsing configuration descriptor for device %u (len=%d)\n", dev, config_len);
    
    // Skip configuration descriptor (9 bytes)
    if (data[0] >= 9 && data[1] == 0x02) {
        offset = data[0];
    } else {
        printf("[ARM POLLING] ERROR: Invalid configuration descriptor\n");
        return;
    }
    
    // Parse all descriptors in the configuration
    while (offset < config_len) {
        if (offset + 2 > config_len) break; // Need at least bLength and bDescriptorType
        
        unsigned char bLength = data[offset];
        unsigned char bDescriptorType = data[offset + 1];
        
        if (bLength == 0 || offset + bLength > config_len) {
            printf("[ARM POLLING] ERROR: Invalid descriptor length %u at offset %d\n", bLength, offset);
            break;
        }
        
        // Check for Interface Descriptor (type 0x04)
        if (bDescriptorType == 0x04 && bLength >= 9) {
            unsigned char bInterfaceNumber = data[offset + 2];
            unsigned char bAlternateSetting = data[offset + 3];
            unsigned char bNumEndpoints = data[offset + 4];
            unsigned char bInterfaceClass = data[offset + 5];
            unsigned char bInterfaceSubClass = data[offset + 6];
            unsigned char bInterfaceProtocol = data[offset + 7];
            
            printf("[ARM POLLING] INTERFACE: if=%u alt=%u eps=%u class=0x%02X subclass=0x%02X protocol=0x%02X\n",
                   bInterfaceNumber, bAlternateSetting, bNumEndpoints, 
                   bInterfaceClass, bInterfaceSubClass, bInterfaceProtocol);
            
            // Check if this is a HID interface (class 0x03)
            if (bInterfaceClass == 0x03) {
                printf("[ARM POLLING] HID INTERFACE DETECTED: class=0x03 subclass=0x%02X protocol=0x%02X\n",
                       bInterfaceSubClass, bInterfaceProtocol);
                
                // Determine device type based on interface protocol
                uint8_t device_type = POLLING_DEVICE_GENERIC;
                const char *type_name = "Generic HID";
                
                if (bInterfaceSubClass == 0x01) { // Boot Interface
                    if (bInterfaceProtocol == 0x01) {
                        device_type = POLLING_DEVICE_KEYBOARD;
                        type_name = "HID Keyboard";
                    } else if (bInterfaceProtocol == 0x02) {
                        device_type = POLLING_DEVICE_MOUSE;
                        type_name = "HID Mouse";
                    }
                } else {
                    // Non-boot interface - try to detect based on endpoints
                    device_type = POLLING_DEVICE_MOUSE; // Default to mouse for now
                    type_name = "HID Device (assuming mouse)";
                }
                
                printf("[ARM POLLING] %s DETECTED: Auto-registering dev=%u ep=1 for polling\n", type_name, dev);
                
                // Register device for polling: endpoint 1, 10ms interval
                int result = arm_add_polling_device(dev, 1, 10, device_type);
                if (result == 0) {
                    printf("[ARM POLLING] SUCCESS: %s dev=%u registered for polling\n", type_name, dev);
                } else {
                    printf("[ARM POLLING] ERROR: Failed to register %s dev=%u (result=%d)\n", type_name, dev, result);
                }
                
                // Only register one HID interface per device for now
                return;
            }
        }
        
        // Move to next descriptor
        offset += bLength;
    }
    
    printf("[ARM POLLING] No HID interfaces found in device %u\n", dev);
}

// Device detection logging - show critical device info with RAW data and memory pointers
void log_device_detected_arm(unsigned char dev, const void *data_ptr, int data_len,
                                   unsigned short vendor_id, unsigned short product_id)
{
    printf("[ARM] DEVICE DETECTED: dev=%u ptr=0x%08lx len=%d VID=0x%04X PID=0x%04X\n",
           dev, (unsigned long)data_ptr, data_len, vendor_id, product_id);
    
    // Show RAW data with pointer verification
    if (data_ptr && data_len > 0) {
        const unsigned char *raw = (const unsigned char *)data_ptr;
        printf("[ARM] RAW DATA at 0x%08lx: ", (unsigned long)data_ptr);
        for (int i = 0; i < (data_len > 32 ? 32 : data_len); i++) {
            printf("%02x ", raw[i]);
        }
        printf("\n");
        
        // Verify critical fields for device descriptor
        if (data_len >= 18) {
            printf("[ARM] PARSED: len=%d type=%d vid=0x%04x pid=0x%04x class=%d\n",
                   raw[0], raw[1], 
                   (unsigned short)(raw[8] | (raw[9] << 8)),
                   (unsigned short)(raw[10] | (raw[11] << 8)),
                   raw[4]);
        }
    }
    
    printf("[ARM POLLING] DEVICE DESCRIPTOR: dev=%u VID=0x%04X PID=0x%04X DeviceClass=%d\n", 
           dev, vendor_id, product_id, data_ptr ? ((const unsigned char*)data_ptr)[4] : -1);
    
    if (dev > 0) { /* Only register real devices, not device 0 */
        printf("[ARM POLLING] Device check passed (dev > 0) - will check interfaces when config descriptor arrives\n");
        
        // Note: HID detection will happen when we get the configuration descriptor
        // in dump_parsed_descriptors_arm() function
    } else {
        printf("[ARM POLLING] DEBUG: Skipping device 0 (root hub)\n");
    }
}
#if DEBUG_DESC_DUMP
static void dump_hex_arm(const char *prefix, const unsigned char *buf, int n)
{
    int i;
    printf("%s", prefix ? prefix : "");
    for (i = 0; i < n; ++i) {
        printf("%02x ", buf[i] & 0xFF);
    }
    printf("\n");
}
#endif

static void dump_parsed_descriptors_arm(unsigned char dev,
                                        const struct devrequest *req,
                                        const void *data_ptr,
                                        int act_len)
{
#if DEBUG_DESC_DUMP
    if (!req || !data_ptr || act_len <= 0)
        return;
    unsigned char bmReq = req->requesttype;
    unsigned char bReq  = req->request;
    unsigned short wValue = req->value;
    unsigned char dtype = (unsigned char)(wValue >> 8); /* descriptor type */

    /* Only trace IN GET_DESCRIPTOR */
    if ((bmReq & 0x80) && bReq == 0x06) {
        const unsigned char *db = (const unsigned char *)data_ptr;
        if (dtype == 0x01) { /* DEVICE */
            int n = (act_len < 18) ? act_len : 18;
            char tag[96];
            snprintf(tag, sizeof(tag), "[ARM] DEV %u GET_DESCRIPTOR(DEVICE) %d bytes: ", (unsigned)dev, act_len);
            dump_hex_arm(tag, db, n);
            if (act_len >= 18) {
                unsigned short idVendor  = (unsigned short)db[8]  | ((unsigned short)db[9]  << 8);
                unsigned short idProduct = (unsigned short)db[10] | ((unsigned short)db[11] << 8);
                unsigned char  iManu     = db[14];
                unsigned char  iProd     = db[15];
                printf("[ARM] Parsed DevDesc: idVendor=0x%04x idProduct=0x%04x iManu=%u iProd=%u\n",
                       idVendor, idProduct, iManu, iProd);

                // Call ARM device detection logging with RAW descriptor data
                log_device_detected_arm(dev, data_ptr, act_len, idVendor, idProduct);
                
                // Check if this device is a USB hub for conditional auto-reset
                check_for_hub_device(db, act_len);
            }
        } else if (dtype == 0x02) { /* CONFIGURATION */
//            int n = (act_len < 9) ? act_len : 9;
            char tag[96];
            snprintf(tag, sizeof(tag), "[ARM] DEV %u GET_DESCRIPTOR(CONFIG) %d bytes: ", (unsigned)dev, act_len);
            dump_hex_arm(tag, db, act_len);
            if (act_len >= 5) {
                unsigned short wTotalLen = (unsigned short)db[2] | ((unsigned short)db[3] << 8);
                unsigned char bNumIf = db[4];
                printf("[ARM] Parsed CfgDesc: wTotalLength=%u bNumInterfaces=%u\n", wTotalLen, bNumIf);
                
                // CRITICAL: Parse interfaces to detect HID devices
                parse_hid_interfaces_and_register(dev, db, act_len);
            }
        } else if (dtype == 0x03) { /* STRING */
//            int n = (act_len < 16) ? act_len : 16;
            char tag[96];
            snprintf(tag, sizeof(tag), "[ARM] DEV %u GET_DESCRIPTOR(STRING) %d bytes: ", (unsigned)dev, act_len);
            dump_hex_arm(tag, db, act_len);
            if (act_len >= 2) {
                unsigned char bLen  = db[0];
                unsigned char bType = db[1];
                printf("[ARM] Parsed StrDesc: bLength=%u bDescriptorType=0x%02x\n", bLen, bType);
            }
        }
    }
#else
    (void)dev; (void)req; (void)data_ptr; (void)act_len;
#endif
}

/* Software change-bit latches for host expectations (e.g., C_PORT_ENABLE via PEDC) */
static int sw_pedc_latch = 0; /* Set when PED changes 0->1; cleared on CLEAR_FEATURE(C_PORT_ENABLE=17) */
static int sw_creset_latch = 0; /* Set when PR transitions 1->0; cleared on CLEAR_FEATURE(C_PORT_RESET=20) */

/* Hub detection and conditional auto-reset state */
static int hub_detected = 0;       /* Set when a hub device is detected via descriptor (bDeviceClass=9) */
static int hub_auto_reset_done = 0; /* Set when auto-reset has been applied to prevent repeated resets */
static uint32_t last_ccs_state = 0; /* Track connection state to detect new connections and reset hub state */

/* Function to detect if connected device is a USB hub based on device descriptor */
/* Enhanced control message function with split transaction retry logic */
static int usb_control_msg_with_split_retry(struct usb_device *udev, unsigned long pipe,
                                           unsigned char request, unsigned char requesttype,
                                           unsigned short value, unsigned short index,
                                           void *data, unsigned short size, int timeout)
{
    int ret = -1;
    int retry_count = 0;
    int max_retries = 2;  // Default retries
    int retry_timeout = timeout;
    
    /* Enhanced retry logic for split transactions */
    if (udev && udev->parent && udev->parent->speed == USB_SPEED_HIGH && 
        (udev->speed == USB_SPEED_FULL || udev->speed == USB_SPEED_LOW)) {
        max_retries = 5;  // More retries for split transactions
        retry_timeout = timeout * 2;  // Longer timeout for split transactions
        printf("[USB SPLIT RETRY] Device %d behind HS hub - using enhanced retry (max=%d timeout=%dms)\n", 
               udev->devnum, max_retries, retry_timeout);
    }
    
    for (retry_count = 0; retry_count <= max_retries; retry_count++) {
        if (retry_count > 0) {
            printf("[USB SPLIT RETRY] Attempt %d/%d for dev=%d req=0x%02x\n", 
                   retry_count + 1, max_retries + 1, udev->devnum, request);
            
            /* Progressive delay between retries */
            int delay_ms = 5 + (retry_count * 10);  // 5ms, 15ms, 25ms, etc.
            usleep(delay_ms * 1000);
            
            /* Reset endpoint toggles on retry */
            if (udev) {
                udev->toggle[0] = 0;
                udev->toggle[1] = 0;
            }
        }
        
        ret = usb_control_msg(udev, pipe, request, requesttype, value, index, 
                             data, size, retry_timeout);
        
        if (ret >= 0) {
            if (retry_count > 0) {
                printf("[USB SPLIT RETRY] SUCCESS on retry %d for dev=%d\n", 
                       retry_count, udev->devnum);
            }
            break;  // Success - exit retry loop
        }
        
        /* Log the error and analyze if it's worth retrying */
        printf("[USB SPLIT RETRY] Attempt %d failed: ret=%d dev=%d status=0x%lx\n", 
               retry_count + 1, ret, udev->devnum, udev ? udev->status : 0);
        
        /* Check EHCI status for split transaction specific errors */
        extern struct ehci_ctrl ehcic[];
        struct ehci_ctrl *ehci_ctrl = &ehcic[0];
        if (ehci_ctrl && ehci_ctrl->hcor) {
            uint32_t usbsts = ehci_readl(&ehci_ctrl->hcor->or_usbsts);
            uint32_t portsc = ehci_readl(&ehci_ctrl->hcor->or_portsc[0]);
            
            if (retry_count == 0) {  // Only log detailed status on first failure
                printf("[USB SPLIT RETRY] EHCI Status: USBSTS=0x%08lx PORTSC=0x%08lx\n",
                       (unsigned long)usbsts, (unsigned long)portsc);
            }
            
            /* Check for specific error patterns that indicate split transaction issues */
            if (udev && udev->status) {
                uint32_t status = (uint32_t)udev->status;
                if ((status & 0x22) == 0x22) {  // Status 34 = 0x22 (Data Buffer Error + Split State Error)
                    printf("[USB SPLIT RETRY] Detected split transaction error (status=0x%lx)\n", udev->status);
                    /* These errors are often transient - continue retrying */
                }
            }
        }
        
        /* Don't retry certain permanent errors */
        if (ret == -ENODEV || ret == -ENXIO) {
            printf("[USB SPLIT RETRY] Permanent error detected (%d) - stopping retries\n", ret);
            break;
        }
    }
    
    if (ret < 0) {
        printf("[USB SPLIT RETRY] All attempts failed for dev=%d req=0x%02x (final ret=%d)\n", 
               udev ? udev->devnum : 0, request, ret);
    }
    
    return ret;
}

static void check_for_hub_device(const unsigned char *descriptor_data, int data_len)
{
    if (!descriptor_data || data_len < 5) {
        return;
    }
    
    /* Check if this is a device descriptor (bDescriptorType=1) and has bDeviceClass field */
    if (descriptor_data[1] == 0x01 && data_len >= 5) {
        unsigned char bDeviceClass = descriptor_data[4];
        if (bDeviceClass == 9) { /* USB_CLASS_HUB */
            /* Check USB protocol to determine hub speed */
            unsigned char bDeviceProtocol = descriptor_data[6];
            const char *speed_str = "UNKNOWN";
            
            switch (bDeviceProtocol) {
                case 0: speed_str = "FULL SPEED"; break;
                case 1: speed_str = "SINGLE TT HIGH SPEED"; break;
                case 2: speed_str = "MULTI TT HIGH SPEED"; break;
                case 3: speed_str = "SUPER SPEED"; break;
            }
            
            printf("[USB Handler] HUB DETECTED: bDeviceClass=0x%02x Protocol=0x%02x (%s) - will apply auto-reset after SET_ADDRESS\n", 
                   bDeviceClass, bDeviceProtocol, speed_str);
                   
            /* Always configure as high-speed hub for proper split transaction handling */
            hub_detected = 1;
            hub_auto_reset_done = 0;
            
            /* Store hub protocol for transaction translator setup */
            usb_state.hub_protocol = bDeviceProtocol;
        } else {
            printf("[USB Handler] NON-HUB DETECTED: bDeviceClass=0x%02x - no auto-reset needed\n", bDeviceClass);
            hub_detected = 0;
        }
    }
}

/* Helper: compute recommended interrupt poll backoff from endpoint bInterval.
 * Returns milliseconds to wait, or -1 if unknown. */
#if 0
static int get_intr_bInterval_ms(struct usb_device *udev, uint8_t ep, uint8_t dir)
{
    if (!udev)
        return -1;
    /* Walk configured interfaces and endpoints to find a match */
    int ifc_count = udev->config.no_of_if;
    if (ifc_count <= 0 || ifc_count > USB_MAXINTERFACES)
        return -1;
    for (int i = 0; i < ifc_count; ++i) {
        struct usb_interface *iface = &udev->config.if_desc[i];
        int nep = iface->no_of_ep;
        if (nep <= 0 || nep > USB_MAXENDPOINTS)
            continue;
        for (int e = 0; e < nep; ++e) {
            struct usb_endpoint_descriptor *ed = &iface->ep_desc[e];
            uint8_t eaddr = ed->bEndpointAddress;
            uint8_t e_num = eaddr & 0x0f;
            uint8_t e_dir = (eaddr & 0x80) ? 1 : 0; /* 1=IN */
            if (e_num == ep && e_dir == (dir ? 1 : 0)) {
                uint8_t bint = ed->bInterval;
                if (bint == 0)
                    return -1;
                /* FS/LS: bInterval in frames (ms). HS: 2^(bInterval-1) microframes (125us units) */
                if (udev->speed == USB_SPEED_HIGH) {
                    if (bint > 16) bint = 16; /* spec max */
                    unsigned int microframes = 1u << (bint - 1); /* 1,2,4,...,32768 microframes */
                    unsigned int us = microframes * 125u;
                    unsigned int ms = (us + 999u) / 1000u; /* ceil to ms */
                    if (ms == 0) ms = 1;
                    if (ms > 32) ms = 32; /* clamp */
                    return (int)ms;
                } else {
                    int ms = (int)bint;
                    if (ms < 1) ms = 1;
                    if (ms > 32) ms = 32; /* reasonable clamp */
                    return ms;
                }
            }
        }
    }
    return -1;
}
#endif
/*
 * Initialize USB handler system
 * 
 * Note: This initializes the USB command handler but does NOT initialize
 * the USB hardware. The hardware initialization happens when the Amiga
 * sends the USB_OP_INIT_STACK command, which calls zz_usb_init().
 * 
 * Hardware initialization sequence (when USB_OP_INIT_STACK is called):
 * 1. zz_usb_init() - Main hardware init function
 * 2. ehci_zynq_probe() - Zynq-specific EHCI controller setup
 * 3. usb_init() - USB stack initialization
 * 4. usb_lowlevel_init() - Low-level EHCI initialization
 * 5. ehci_common_init() - EHCI common setup (queues, lists, etc.)
 */
void z3660_usb_handler_init(void)
{
    printf("[USB Handler] Initializing Z3660 USB handler\n");
    
    // Initialize USB state - READY to accept commands immediately
    usb_state.status = USB_STATUS_READY;
    usb_state.operation = 0;
    usb_state.initialized = 0;
    usb_state.offline = 1;  // Start in offline state
    
    printf("[USB Handler] Z3660 USB handler initialized\n");
    printf("[USB Handler] Status: 0x%04lX (READY for commands)\n", usb_state.status);
    printf("[USB Handler] Hardware will be initialized when Amiga sends INIT_STACK command\n");
    
    // Poseidon-driven mode: Do not auto-initialize USB here.
    // The Amiga side will trigger initialization via USB_OP_INIT_STACK.
}

/*
 * Called when USB transaction is complete - handle any pending operations
 * This function is called from the USB interrupt handler to process
 * completed USB transactions and update polling device data.
 */
void z3660_usb_on_transaction_complete(void)
{
    printf("[USB Handler] Transaction complete notification\n");
    
    // Check if we have any pending polling devices that may have received data
    for (int i = 0; i < polling_device_count; i++) {
        if (polling_devices[i].active) {
            uint32_t current_time = get_system_time_ms();
            
            // If enough time has passed since last poll, try to get new data
            if ((current_time - polling_devices[i].last_poll_time) >= polling_devices[i].interval_ms) {
                // This will be handled by the main USB polling thread
                // Just update the timestamp to avoid immediate re-polling
                polling_devices[i].last_poll_time = current_time;
            }
        }
    }
    
    // Signal that USB data may be available to the Amiga
    if (polling_device_count > 0) {
        amiga_interrupt_set(AMIGA_INTERRUPT_USB);
    }
}

/*
 * Handle USB register writes from Amiga
 */
void usb_handle_register_write(uint32_t reg, uint32_t value)
{
    // Debug verbose disabled - only show critical device information
    
    switch (reg) {
        case REG_ZZ_USB_CMD_OP: {
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] *** COMMAND RECEIVED: 0x%lx ***\n", value);
#endif
            usb_state.operation = value;
            usb_state.status |= USB_STATUS_BUSY;
            usb_state.status &= ~(USB_STATUS_COMPLETE | USB_STATUS_ERROR);
            
// Process the command
            z3660_usb_process_command(value);
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] *** COMMAND PROCESSED, new status=0x%lx, initialized=%d ***\n", 
                   usb_state.status, usb_state.initialized);
#endif
            break;
        }
        
#if 0
        case REG_ZZ_USB_CMD_DATA: {
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] Command data: 0x%lx (param_index=%lu)\n", value, (unsigned long)usb_state.param_index);
#endif
            /* Store into parameter array if index is valid */
            if (usb_state.param_index < (sizeof(usb_state.params)/sizeof(usb_state.params[0]))) {
                usb_state.params[usb_state.param_index] = value;
            }
            break;
        }
#endif
        
        case REG_ZZ_USB_STATUS: {
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] Status control: 0x%lx\n", value);
#endif
            // Allow Amiga to control certain status bits
            if (value & USB_STATUS_READY) {
                usb_state.status |= USB_STATUS_READY;
            }
            break;
        }
#if 0
        case REG_ZZ_USB_BU FSEL: {
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] Buffer select: 0x%lx\n", value);
#endif
            usb_state.buffer_select = value;
            usb_state.param_index = (value & 0xFF);
            break;
        }
#endif
        case REG_ZZ_USB_READ0: {
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] Data count: %ld\n", value);
#endif
            usb_state.data_length = value;
            break;
        }
        // Dedicated USB parameter registers (REG_ZZ_USB_PARAM0 through REG_ZZ_USB_PARAM7)
        case REG_ZZ_USB_PARAM0:
        case REG_ZZ_USB_PARAM1:
        case REG_ZZ_USB_PARAM2:
        case REG_ZZ_USB_PARAM3:
        case REG_ZZ_USB_PARAM4:
        case REG_ZZ_USB_PARAM5:
        case REG_ZZ_USB_PARAM6:
        case REG_ZZ_USB_PARAM7: {
            // Calculate parameter index from register address
            uint32_t param_index = (reg - REG_ZZ_USB_PARAM0) / 4;
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] USB param[%lu] write: 0x%lx (reg=0x%lx)\n", param_index, value, reg);
#endif
            if (param_index < (sizeof(usb_state.params)/sizeof(usb_state.params[0]))) {
                usb_state.params[param_index] = value;
#if defined(USB_DEBUG_VERBOSE)
                printf("[USB Handler] Stored param[%lu] = 0x%08lx\n", param_index, usb_state.params[param_index]);
#endif
            } else {
                printf("[USB Handler] ERROR: param_index %lu is out of range!\n", param_index);
            }
            break;
        }
    }
}

/*
 * Handle USB register reads to Amiga
 */
uint32_t usb_handle_register_read(uint32_t reg)
{
    uint32_t result = 0;
    
    switch (reg) {
       case REG_ZZ_USB_PORTSC1:
            result = usb_state.portsc1;
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] Command PORTSC1 read: 0x%lx\n", result);
#endif
            break;
       case REG_ZZ_USB_STATUS:
            // Return current status
            result = usb_state.status;
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] Status read: 0x%lx (READY=%s, BUSY=%s, ERROR=%s, TIMEOUT=%s, COMPLETE=%s)\n", 
                   result,
                   (result & USB_STATUS_READY) ? "YES" : "NO",
                   (result & USB_STATUS_BUSY) ? "YES" : "NO", 
                   (result & USB_STATUS_ERROR) ? "YES" : "NO",
                   (result & USB_STATUS_TIMEOUT) ? "YES" : "NO",
                   (result & USB_STATUS_COMPLETE) ? "YES" : "NO");
#endif
            break;
        case REG_ZZ_USB_READ0:
            // Return data length or device count
            result = usb_state.data_length;
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] Count read: %ld\n", result);
#endif
            break;
#if 0
        case REG_ZZ_USB_BU FSEL:
            // Return current buffer selection
            result = usb_state.buffer_select;
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] Buffer select read: 0x%lx\n", result);
#endif
            break;
#endif            
        // Dedicated USB parameter registers (REG_ZZ_USB_PARAM0 through REG_ZZ_USB_PARAM7)
        case REG_ZZ_USB_PARAM0:
        case REG_ZZ_USB_PARAM1:
        case REG_ZZ_USB_PARAM2:
        case REG_ZZ_USB_PARAM3:
        case REG_ZZ_USB_PARAM4:
        case REG_ZZ_USB_PARAM5:
        case REG_ZZ_USB_PARAM6:
        case REG_ZZ_USB_PARAM7: {
            // Calculate parameter index from register address
            uint32_t param_index = (reg - REG_ZZ_USB_PARAM0) / 4;
            if (param_index < (sizeof(usb_state.params)/sizeof(usb_state.params[0]))) {
                result = usb_state.params[param_index];
#if defined(USB_DEBUG_VERBOSE)
                printf("[USB Handler] USB param[%lu] read: 0x%lx\n", param_index, result);
#endif
            } else {
                result = 0xFFFFFFFF; // Invalid parameter index
            }
            break;
        }
    }
    
    return result;
}

/*
 * Process USB commands from Amiga
 */
static inline uint32_t ehci_read_reg(uint32_t offset)
{
    return *(volatile uint32_t *)(XPS_USB0_BASEADDR + offset);
}
static inline void ehci_write_reg(uint32_t offset, uint32_t value)
{
    *(volatile uint32_t *)(XPS_USB0_BASEADDR + offset) = value;
}
#define EHCI_USBCMD     0x140
#define EHCI_USBSTS     0x144
#define EHCI_CONFIGFLAG 0x180
#define EHCI_PORTSC1    0x184

/* EHCI PORTSC1 bit definitions (subset) */
#define EHCI_PS_CCS    (1u << 0)   /* Current Connect Status */
#define EHCI_PS_CSC    (1u << 1)   /* Connect Status Change (W1C) */
#define EHCI_PS_PED    (1u << 2)   /* Port Enabled/Disabled */
#define EHCI_PS_PEDC   (1u << 3)   /* Port Enable/Disable Change (W1C) */
#define EHCI_PS_OCA    (1u << 4)   /* Over-current Active */
#define EHCI_PS_OCC    (1u << 5)   /* Over-current Change (W1C) */
#define EHCI_PS_FPR    (1u << 6)   /* Force Port Resume */
#define EHCI_PS_SUSP   (1u << 7)   /* Suspend */
#define EHCI_PS_PR     (1u << 8)   /* Port Reset */
#define EHCI_PS_PP     (1u << 12)  /* Port Power */

/* Implement core command processing - removed (obsolete duplicate). */

/*
 * Get system time in milliseconds using ARM hardware timer
 * Uses XTime which provides high-resolution timer access
 */
static uint32_t get_system_time_ms(void)
{
    XTime current_time;
    XTime_GetTime(&current_time);
    
    // Convert XTime ticks to milliseconds
    // COUNTS_PER_SECOND is typically defined in xtime_l.h
    return (uint32_t)(current_time / (counts_per_second / 1000));
}

/*
 * Add a device to ARM polling table
 * Returns: 0 on success, -1 on error
 */
static int arm_add_polling_device(uint8_t device_addr, uint8_t endpoint, uint8_t interval_ms, uint8_t device_type)
{
    printf("[ARM Polling] Adding device: addr=%u ep=%u interval=%ums type=%u\n",
           device_addr, endpoint, interval_ms, device_type);
    
    // Check if device already exists
    for (int i = 0; i < polling_device_count; i++) {
        if (polling_devices[i].active && 
            polling_devices[i].device_addr == device_addr &&
            polling_devices[i].endpoint == endpoint) {
            printf("[ARM Polling] Device already exists, updating parameters\n");
            polling_devices[i].interval_ms = interval_ms;
            polling_devices[i].device_type = device_type;
            polling_devices[i].last_poll_time = get_system_time_ms();
            return 0;
        }
    }
    
    // Find empty slot
    int slot = -1;
    for (int i = 0; i < MAX_POLLING_DEVICES; i++) {
        if (!polling_devices[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        printf("[ARM Polling] ERROR: No free slots available (max %d)\n", MAX_POLLING_DEVICES);
        return -1;
    }
    
    // Initialize polling device
    polling_devices[slot].device_addr = device_addr;
    polling_devices[slot].endpoint = endpoint;
    polling_devices[slot].interval_ms = (interval_ms > 0) ? interval_ms : 10; // Default 10ms
    polling_devices[slot].device_type = device_type;
    polling_devices[slot].active = 1;
    polling_devices[slot].last_poll_time = get_system_time_ms();
    polling_devices[slot].data_length = 0;
    polling_devices[slot].poll_count = 0;
    polling_devices[slot].error_count = 0;
    
    // Initialize new persistent queue fields
    polling_devices[slot].int_queue = NULL;
    polling_devices[slot].udev = NULL;
    polling_devices[slot].queue_create_time = 0;
    polling_devices[slot].queue_recreate_count = 0;
    
    // Clear data buffer
    memset(polling_devices[slot].data_buffer, 0, sizeof(polling_devices[slot].data_buffer));
    
    if (slot >= polling_device_count) {
        polling_device_count = slot + 1;
    }
    
    printf("[ARM Polling] Device added successfully in slot %d (total: %u)\n", slot, polling_device_count);
    return 0;
}

/*
 * Remove a device from ARM polling table
 * Returns: 0 on success, -1 if not found
 */
static int arm_remove_polling_device(uint8_t device_addr, uint8_t endpoint)
{
    printf("[ARM Polling] Removing device: addr=%u ep=%u\n", device_addr, endpoint);
    
    for (int i = 0; i < polling_device_count; i++) {
        if (polling_devices[i].active && 
            polling_devices[i].device_addr == device_addr &&
            polling_devices[i].endpoint == endpoint) {
            
            printf("[ARM Polling] Found device in slot %d, removing\n", i);
            
            // Clean up interrupt queue if it exists
            if (polling_devices[i].int_queue && polling_devices[i].udev) {
                printf("[ARM Polling] Cleaning up interrupt queue for dev=%u\n", device_addr);
                destroy_int_queue(polling_devices[i].udev, polling_devices[i].int_queue);
                polling_devices[i].int_queue = NULL;
                polling_devices[i].udev = NULL;
            }
            
            // Mark as inactive
            polling_devices[i].active = 0;
            
            // Clear device data
            memset(&polling_devices[i], 0, sizeof(polling_device_t));
            
            // Compact the table if this was the last device
            if (i == polling_device_count - 1) {
                while (polling_device_count > 0 && !polling_devices[polling_device_count - 1].active) {
                    polling_device_count--;
                }
            }
            
            printf("[ARM Polling] Device removed successfully (remaining: %u)\n", polling_device_count);
            return 0;
        }
    }
    
    printf("[ARM Polling] Device not found in polling table\n");
    return -1;
}

/*
 * List all active polling devices
 * Returns: number of active devices
 */
static int arm_list_polling_devices(void)
{
    printf("[ARM Polling] Listing active polling devices:\n");
    
    int active_count = 0;
    for (int i = 0; i < polling_device_count; i++) {
        if (polling_devices[i].active) {
            active_count++;
            const char *type_name;
            switch (polling_devices[i].device_type) {
                case POLLING_DEVICE_MOUSE: type_name = "MOUSE"; break;
                case POLLING_DEVICE_KEYBOARD: type_name = "KEYBOARD"; break;
                case POLLING_DEVICE_JOYSTICK: type_name = "JOYSTICK"; break;
                case POLLING_DEVICE_GENERIC: type_name = "GENERIC"; break;
                default: type_name = "UNKNOWN"; break;
            }
            
            printf("[ARM Polling]   Slot %d: addr=%u ep=%u interval=%ums type=%s\n",
                   i, polling_devices[i].device_addr, polling_devices[i].endpoint,
                   polling_devices[i].interval_ms, type_name);
            printf("[ARM Polling]          polls=%lu errors=%lu data_len=%lu\n",
                   polling_devices[i].poll_count, polling_devices[i].error_count,
                   polling_devices[i].data_length);
        }
    }
    
    printf("[ARM Polling] Total active devices: %d\n", active_count);
    return active_count;
}

void z3660_usb_process_command(uint32_t command)
{
    /* Track whether we've performed the very first addr0 GET_DESCRIPTOR(DEVICE,8)
     * so we can insert a one-shot settle before it */
    static int did_first_addr0_getdev8 = 0;
#if defined(USB_DEBUG_VERBOSE)
    printf("[USB Handler] Processing command: 0x%lx\n", command);
#endif
    
    switch (command) {
        case USB_OP_INIT_STACK: {
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] Initialize USB stack (Poseidon mode)\n");
#endif
            int ret = usb_poseidon_init();
            if (ret == 0) {
                log_portsc("after INIT");
                log_core_regs("after INIT");
                /* Ensure downstream port is powered to match root-hub report */
                uint32_t reg = ehci_read_reg(EHCI_PORTSC1);
                reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC); /* avoid writing W1C bits as 1 */
                reg |= EHCI_PS_PP; /* Port Power ON */
                ehci_write_reg(EHCI_PORTSC1, reg);
                log_portsc("after INIT: forced PP=1");
                
                /* USB3320C MODE: Reset port to enable communication with physical hub */
                printf("[USB Handler] USB3320C Mode: Performing port reset to enable physical hub\n");
                
                /* Wait for port power to stabilize */
                usleep(10 * 1000); /* 10ms */
                
                /* Initiate port reset sequence */
                reg = ehci_read_reg(EHCI_PORTSC1);
                reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC); /* Clear W1C bits */
                reg |= EHCI_PS_PR; /* Assert Port Reset */
                ehci_write_reg(EHCI_PORTSC1, reg);
                log_portsc("after USB3320C reset: PR asserted");
                
                /* Hold reset for 50ms as per USB spec */
                usleep(50 * 1000);
                
                /* Deassert Port Reset */
                reg = ehci_read_reg(EHCI_PORTSC1);
                reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC | EHCI_PS_PR); /* Clear PR and W1C bits */
                ehci_write_reg(EHCI_PORTSC1, reg);
                
                /* Wait for port to stabilize and become enabled */
                int reset_timeout = 100; /* 100ms timeout */
                int enabled = 0;
                while (reset_timeout > 0) {
                    usleep(2 * 1000); /* 2ms intervals */
                    reg = ehci_read_reg(EHCI_PORTSC1);
                    
                    if (!(reg & EHCI_PS_PR) && (reg & EHCI_PS_PED)) {
                        /* Reset completed and port is enabled */
                        enabled = 1;
                        break;
                    }
                    
                    reset_timeout -= 2;
                }
                
                if (enabled) {
                    log_portsc("after USB3320C reset: PORT ENABLED");
                    printf("[USB Handler] USB3320C hub port reset completed successfully - ready for enumeration\n");
                } else {
                    log_portsc("after USB3320C reset: TIMEOUT - port not enabled");
                    printf("[USB Handler] WARNING: USB3320C port reset timeout - device may not respond correctly\n");
                }

                if(usb_state.offline)
                {
                   printf("[USB] Going ONLINE!!!\n");
                   usb_state.offline = 0;     // Clear OFFLINE state - USB is now online
                }
                else
                {
                   printf("[USB] YA estabamos ONLINE!!!\n");
                }
                usb_state.initialized = 1;
                usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
#if defined(USB_DEBUG_VERBOSE)
                printf("[USB Handler] USB stack (Poseidon) initialized successfully - ONLINE mode activated\n");
#endif
            } else {
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
#if defined(USB_DEBUG_VERBOSE)
                printf("[USB Handler] USB stack (Poseidon) initialization failed: %d\n", ret);
#endif
            }
            break;
        }
        
        case USB_OP_SHUTDOWN_STACK: {
            printf("[USB Handler] Shutdown USB stack - stopping all interrupts\n");
            
            // Clean up all interrupt queues before clearing polling devices
            for (int i = 0; i < MAX_POLLING_DEVICES; i++) {
                if (polling_devices[i].active && polling_devices[i].int_queue && polling_devices[i].udev) {
                    printf("[USB Handler] Cleaning up queue for dev=%u during shutdown\n", polling_devices[i].device_addr);
                    destroy_int_queue(polling_devices[i].udev, polling_devices[i].int_queue);
                    polling_devices[i].int_queue = NULL;
                    polling_devices[i].udev = NULL;
                }
                polling_devices[i].active = 0;
            }
            
            // Clear all polling devices to stop generating interrupts
            polling_device_count = 0;
            
            // Clear any pending USB interrupt to Amiga
            amiga_interrupt_clear(AMIGA_INTERRUPT_USB);
            
            // Set OFFLINE state to prevent other_tasks() from calling USB polling
            usb_state.offline = 1;
            usb_state.initialized = 0;
            usb_state.status = USB_STATUS_COMPLETE;
            printf("[USB Handler] USB stack shutdown complete - OFFLINE mode activated\n");
            break;
        }
        
        case USB_OP_RESET_PORT: {
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] Reset USB port (Poseidon mode)\n");
#endif
            if (usb_state.initialized) {
                int r = usb_poseidon_reset();
                if (r == 0) {
                    usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
                } else {
                    usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                }
            } else {
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
            }
            break;
        }
        
        case USB_OP_PORT_STATUS: {
#if defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] PORT_STATUS (Poseidon)\n");
#endif
            if (!usb_state.initialized) {
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }
            /* Optional: params[0] may contain port index; single port only */
            uint32_t port = usb_state.params[0] & 0xFF;
            (void)port;
            log_portsc("PORT_STATUS");
            uint32_t ps_raw = ehci_read_reg(EHCI_PORTSC1);
            uint32_t ps = ps_raw;
            
            /* CONDITIONAL AUTO-RESET: apply only for hubs that need it. */
            {
                uint32_t current_ccs = (ps_raw & EHCI_PS_CCS) ? 1 : 0;
                if (current_ccs != last_ccs_state) {
                    if (!current_ccs) {
                        printf("[USB Handler] Device disconnected - resetting hub detection state\n");
                        hub_detected = 0;
                        hub_auto_reset_done = 0;
                    } else {
                        printf("[USB Handler] New device connected - hub detection will occur during descriptor read\n");
                    }
                    last_ccs_state = current_ccs;
                }

                if (hub_detected && !hub_auto_reset_done && current_ccs) {
                    printf("[USB Handler] HUB AUTO-RESET: initiating reset sequence for detected hub\n");
                    uint32_t reg = ehci_read_reg(EHCI_PORTSC1);
                    int before_ped = (reg & EHCI_PS_PED) ? 1 : 0;

                    /* Clear W1C bits and ensure PED cleared before asserting PR */
                    reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC | EHCI_PS_PED);
                    ehci_write_reg(EHCI_PORTSC1, reg);

                    reg = ehci_read_reg(EHCI_PORTSC1);
                    reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC);
                    reg |= EHCI_PS_PR;
                    ehci_write_reg(EHCI_PORTSC1, reg);

                    usleep(50 * 1000);

                    /* Deassert PR (W1C) after delay */
                    reg = ehci_read_reg(EHCI_PORTSC1);
                    reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC | EHCI_PS_PR);
                    ehci_write_reg(EHCI_PORTSC1, reg);

                    /* Latch PEDC if PED transitioned from 0->1 */
                    reg = ehci_read_reg(EHCI_PORTSC1);
                    int after_ped = (reg & EHCI_PS_PED) ? 1 : 0;
                    if (!before_ped && after_ped) {
                        sw_pedc_latch = 1;
                    }
                    sw_creset_latch = 1;

                    hub_auto_reset_done = 1;
                    ps_raw = ehci_read_reg(EHCI_PORTSC1);
                    ps = ps_raw;
                    log_portsc("after HUB AUTO-RESET");
                }
            }
            
            /* Inject software-latched change bits so host proceeds */
            if (sw_pedc_latch) {
                ps |= EHCI_PS_PEDC; /* Present as if hardware set it */
                printf("[USB Handler] PORT_STATUS SW: injecting PEDC (latch=1) - RAW=0x%08lx -> INJECTED=0x%08lx\n", 
                       (unsigned long)ps_raw, (unsigned long)ps);
            } else {
                printf("[USB Handler] PORT_STATUS SW: NOT injecting PEDC (latch=0)\n");
            }
            if (sw_creset_latch) {
                printf("[USB Handler] PORT_STATUS SW: C_RESET latch set (no PORTSC bit)\n");
            }
            
            printf("[USB Handler] PORT_STATUS FINAL: returning PORTSC1=0x%08lx to Poseidon\n", (unsigned long)ps);
            usb_state.portsc1 = ps; /* Return raw PORTSC1 with SW change-bits (Amiga translates) */
            usb_state.data_length = 0;
            usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
            break;
        }
        
        case USB_OP_PORT_CONTROL: {
            if (!usb_state.initialized) {
                usb_state.status = USB_STATUS_ERROR;
                break;
            }
            uint32_t port    = usb_state.params[0] & 0xFF;   /* 1-based */
            uint32_t feature = usb_state.params[1];          /* hub feature selector */
            uint32_t value   = usb_state.params[2] & 0x1;    /* set(1)/clear(0) */
            (void)port; /* single downstream port */
            
#if defined(USB_DEBUG_VERBOSE)
            uint32_t before = ehci_read_reg(EHCI_PORTSC1);
            uint32_t after  = before;
            printf("[USB Handler] PORT_CONTROL (Poseidon): port=%lu feature=%lu value=%lu BEFORE=0x%08lx\n",
                   (unsigned long)port, (unsigned long)feature, (unsigned long)value, (unsigned long)before);
#endif

            log_portsc("PORT_CONTROL BEFORE");

            if (feature == 8) { /* PORT_POWER */
                if (value) {
                    uint32_t reg = ehci_read_reg(EHCI_PORTSC1);
                    /* Never write W1C bits as 1 unintentionally */
                    reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC);
                    reg |= EHCI_PS_PP;
                    ehci_write_reg(EHCI_PORTSC1, reg);
#if defined(USB_DEBUG_VERBOSE)
                    log_portsc("after PP=1");
#endif
                    /* Do NOT auto-reset here; let hub.class drive PORT_RESET */
                } else {
                    uint32_t reg = ehci_read_reg(EHCI_PORTSC1);
                    reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC | EHCI_PS_PP);
                    ehci_write_reg(EHCI_PORTSC1, reg);
#if defined(USB_DEBUG_VERBOSE)
                    log_portsc("after PP=0");
#endif
                }
            } else if (feature == 4) { /* PORT_RESET */
                if (value) {
                    uint32_t reg = ehci_read_reg(EHCI_PORTSC1);
                    int before_ped = (reg & EHCI_PS_PED) ? 1 : 0;
                    /* If port power is off, ignore reset request */
                    if ((reg & EHCI_PS_PP) == 0) {
                        printf("[USB Handler] Ignoring PORT_RESET: PP=0\n");
                    } else {
                        /* Clear W1C bits and ensure PED cleared before asserting PR */
                        reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC | EHCI_PS_PED);
                        ehci_write_reg(EHCI_PORTSC1, reg);

                        reg = ehci_read_reg(EHCI_PORTSC1);
                        reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC);
                        reg |= EHCI_PS_PR;
                        ehci_write_reg(EHCI_PORTSC1, reg);
#if defined(USB_DEBUG_VERBOSE)
                        log_portsc("after PR=1");
#endif

                        /* Hold reset asserted per EHCI spec (~50ms) */
                        usleep(50 * 1000);

                        /* Deassert PR (W1C) after delay */
                        reg = ehci_read_reg(EHCI_PORTSC1);
                        reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC | EHCI_PS_PR);
                        ehci_write_reg(EHCI_PORTSC1, reg);
#if defined(USB_DEBUG_VERBOSE)
                        log_portsc("after PR=0");
#endif

                        /* Latch PEDC if PED transitioned from 0->1 */
                        reg = ehci_read_reg(EHCI_PORTSC1);
                        int after_ped = (reg & EHCI_PS_PED) ? 1 : 0;
                        if (!before_ped && after_ped) {
                            sw_pedc_latch = 1;
#if defined(USB_DEBUG_VERBOSE)
                            printf("[USB Handler] SW PEDC latch set (PED 0->1)\n");
#endif
                        }
                        /* Latch C_RESET to signal reset completion to host logic */
                        sw_creset_latch = 1;
#if defined(USB_DEBUG_VERBOSE)
                        printf("[USB Handler] SW C_RESET latch set (PR 1->0)\n");
#endif

                        /* Hint when port is ready for enumeration */
                        if ((reg & (EHCI_PS_PP | EHCI_PS_PED | EHCI_PS_CCS)) == (EHCI_PS_PP | EHCI_PS_PED | EHCI_PS_CCS)) {
#if defined(USB_DEBUG_VERBOSE)
                            printf("[USB Handler] Port ready for enumeration (PP=1, CCS=1, PED=1)\n");
#endif
                        }
                    }
                } else {
                    uint32_t reg = ehci_read_reg(EHCI_PORTSC1);
                    reg &= ~(EHCI_PS_CSC | EHCI_PS_PEDC | EHCI_PS_OCC | EHCI_PS_PR);
                    ehci_write_reg(EHCI_PORTSC1, reg);
                    log_portsc("after PR cleared");
                }
            } else if (value == 0) {
                /* CLEAR_FEATURE handling */
                if (feature == 1) { /* PORT_ENABLE (clear): ack change without disabling port */
                    /* Write-1-to-clear PEDC to acknowledge, do not clear PED */
                    uint32_t reg = ehci_read_reg(EHCI_PORTSC1);
                    reg |= EHCI_PS_PEDC; /* W1C */
                    ehci_write_reg(EHCI_PORTSC1, reg);
                } else if (feature == 16) { /* C_PORT_CONNECTION -> CSC (W1C) */
                    ehci_write_reg(EHCI_PORTSC1, ehci_read_reg(EHCI_PORTSC1) | EHCI_PS_CSC);
                } else if (feature == 17) { /* C_PORT_ENABLE -> PEDC (W1C) */
                    /* Clear SW latch and W1C in HW */
                    sw_pedc_latch = 0;
                    ehci_write_reg(EHCI_PORTSC1, ehci_read_reg(EHCI_PORTSC1) | EHCI_PS_PEDC);
                } else if (feature == 18) { /* C_PORT_SUSPEND -> no dedicated change bit in EHCI; noop */
                    /* no-op */
                } else if (feature == 19) { /* C_PORT_OVER_CURRENT -> OCC (W1C) */
                    ehci_write_reg(EHCI_PORTSC1, ehci_read_reg(EHCI_PORTSC1) | EHCI_PS_OCC);
                } else if (feature == 20) { /* C_PORT_RESET */
                    /* Clear SW latch; no dedicated EHCI W1C for reset completion */
                    sw_creset_latch = 0;
                } else {
                    /* Unhandled clear feature */
                }
#if defined(USB_DEBUG_VERBOSE)
                after = ehci_read_reg(EHCI_PORTSC1);
                log_portsc("after CLEAR_FEATURE");
#endif
            } else {
               printf("[USB Handler] PORT_CONTROL: unknown feature=%lu value=%lu\n",
                      (unsigned long)feature, (unsigned long)value);
            }

#if defined(USB_DEBUG_VERBOSE)
            log_portsc("PORT_CONTROL AFTER");
            after = ehci_read_reg(EHCI_PORTSC1);
            printf("[USB Handler] PORT_CONTROL AFTER=0x%08lx\n", (unsigned long)after);
#endif
            usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
            break;
        }
        
        case USB_OP_ENUMERATE_DEVS: {
            printf("[USB Handler] Enumerate USB devices (Poseidon mode noop)\n");
            if (usb_state.initialized) {
                /* Poseidon-mode: ARM does not enumerate. Report 0 and COMPLETE */
                usb_state.data_length = 0;
                usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
            } else {
                usb_state.status = USB_STATUS_ERROR;
            }
            break;
        }
        
        case USB_OP_CONTROL_XFER: {
            /* Parameters (filled by BUFSEL+CMD_DATA):
               params[0] = d0 (dev/ep/dir)
               params[1] = setup_ptr (shared memory)
               params[2] = data_ptr  (shared memory)
               params[3] = data_len
               REG_ZZ_USB_COUNT = expected length (IN) or actual length (OUT) */
               
            /* EXTRA DEBUG ESPECIFICO PARA DEV=0 - ARM SIDE */
            uint8_t dev_addr_check = ((usb_state.params[0] >> 24) & 0xFF);
            if (dev_addr_check == 0) {
                printf("\n*** ARM DEBUG DEV=0 *** CONTROL_XFER RECEIVED FROM AMIGA ***\n");
                printf("[ARM] Device Address: %u (USB3320C HUB)\n", dev_addr_check);
                printf("[ARM] Command: USB_OP_CONTROL_XFER (0x1005)\n");
                printf("[ARM] ARM Status when command received: 0x%08lx\n", usb_state.status);
                printf("[ARM] ARM initialized flag: %d\n", usb_state.initialized);
                printf("[ARM] ARM offline flag: %d\n", usb_state.offline);
            }
               
            /* Parameter debugging disabled to reduce log spam */
            #ifdef USB_DEBUG_PARAMS
            printf("[USB Handler] CONTROL_XFER: params[0-3] = 0x%08lx 0x%08lx 0x%08lx 0x%08lx\n", 
                   usb_state.params[0], usb_state.params[1], usb_state.params[2], usb_state.params[3]);
            #endif
               
            uint32_t d0        = usb_state.params[0];
            uint32_t setup_ptr = usb_state.params[1];
            uint32_t data_ptr  = usb_state.params[2];
            uint32_t data_len  = usb_state.params[3];
            uint32_t hub_addr  = usb_state.params[4]; /* optional: HS hub address */
            uint32_t hub_port  = usb_state.params[5]; /* optional: downstream port on hub */
            uint32_t sp_hint   = usb_state.params[6]; /* optional: 1=LOW,2=FULL,3=HIGH */
            uint8_t dev_addr  = (d0 >> 24) & 0xFF;
//            uint8_t ep        = (d0 >> 16) & 0xFF;
//            uint8_t dir       = (d0 >> 8)  & 0xFF;
            uint32_t count    = usb_state.data_length; /* matches REG_ZZ_USB_COUNT set by 68k */

            /* Reduced logging for control transfers */
            #ifdef USB_DEBUG_VERBOSE
            printf("[USB Handler] CTRL: dev=%u len=%lu\n", dev_addr, (unsigned long)data_len);
            #endif

            /* Read setup packet early for later checks */
            struct devrequest req;
            uint8_t *setup_bytes = NULL;
            
            if (setup_ptr != 0 && setup_ptr >= 0x08000000 && setup_ptr < 0x10000000) {
                /* Setup pointer is valid, read the setup packet */
                setup_bytes = (uint8_t *)setup_ptr;
                
                req.requesttype = setup_bytes[0];
                req.request = setup_bytes[1];
                req.value = (uint16_t)setup_bytes[2] | ((uint16_t)setup_bytes[3] << 8);
                req.index = (uint16_t)setup_bytes[4] | ((uint16_t)setup_bytes[5] << 8);
                req.length = (uint16_t)setup_bytes[6] | ((uint16_t)setup_bytes[7] << 8);
            }
            else {
                req.requesttype = 0;
                req.request = 0;
                req.value = 0;
                req.index = 0;
                req.length = 0;
                printf("[USB Handler] ERROR: Invalid setup_ptr=0x%08lx - cannot read setup packet\n", (unsigned long)setup_ptr);
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }
            
            struct usb_device *udev = usb_poseidon_alloc_dev(dev_addr);
            if (!udev) {
                printf("[USB Handler] ERROR: usb_poseidon_alloc_dev(%u) returned NULL\n", dev_addr);
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }
            
            /* CRITICAL FIX: Probe for actual EP0 max packet size before large transfers */
            if (dev_addr != 0) {
                /* Check if this is a GET_DESCRIPTOR(DEVICE) or GET_DESCRIPTOR(CONFIG) request
                 * that might need EP0 packet size determination */
                if ((req.requesttype & 0x80) && req.request == USB_REQ_GET_DESCRIPTOR) {
                    uint8_t desc_type = (req.value >> 8) & 0xFF;
                    
                    /* Only probe for DEVICE descriptor requests, or CONFIG requests on devices 
                     * where we haven't determined the actual EP0 size yet */
                    int should_probe = (desc_type == USB_DT_DEVICE) || 
                                      (desc_type == USB_DT_CONFIG && udev->epmaxpacketin[0] <= 8);
                    
                    if (should_probe) {
                        printf("[EP0 PROBE] Device %u: Current EP0 size %u bytes, desc_type=%u\n", 
                               dev_addr, udev->epmaxpacketin[0], desc_type);
                        
                        /* Always probe with 8 bytes first to be safe */
//                        int orig_ep0_size = udev->epmaxpacketin[0];
                        udev->epmaxpacketin[0] = 8;
                        udev->epmaxpacketout[0] = 8;
                        
                        /* Perform 8-byte device descriptor probe to get bMaxPacketSize0 */
                        ALLOC_CACHE_ALIGN_BUFFER(unsigned char, probe_buf, 18);
                        memset(probe_buf, 0xAA, 18);  /* Initialize with pattern */
                        
                        int probe_result = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
                                                          USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
                                                          (USB_DT_DEVICE << 8), 0,
                                                          probe_buf, 8, 1000);
                        
                        if (probe_result >= 8) {
                            uint8_t actual_ep0_size = probe_buf[7];  /* bMaxPacketSize0 is at offset 7 */
                            printf("[EP0 PROBE] Device %u: Probe successful, actual EP0 size = %u bytes\n", 
                                   dev_addr, actual_ep0_size);
                            
                            /* Validate the EP0 size is reasonable */
                            if (actual_ep0_size == 8 || actual_ep0_size == 16 || 
                                actual_ep0_size == 32 || actual_ep0_size == 64) {
                                udev->epmaxpacketin[0] = actual_ep0_size;
                                udev->epmaxpacketout[0] = actual_ep0_size;
                                
                                /* Update maxpacketsize enum if needed */
                                if (actual_ep0_size == 8) {
                                    udev->maxpacketsize = PACKET_SIZE_8;
                                } else if (actual_ep0_size == 16) {
                                    udev->maxpacketsize = PACKET_SIZE_16;
                                } else if (actual_ep0_size == 32) {
                                    udev->maxpacketsize = PACKET_SIZE_32;
                                } else if (actual_ep0_size == 64) {
                                    udev->maxpacketsize = PACKET_SIZE_64;
                                }
                                
                                printf("[EP0 PROBE] Device %u: Updated EP0 max packet size to %u bytes\n", 
                                       dev_addr, actual_ep0_size);
                            } else {
                                printf("[EP0 PROBE] Device %u: Invalid EP0 size %u, keeping 8 bytes\n", 
                                       dev_addr, actual_ep0_size);
                                /* Keep 8 bytes - already set above */
                            }
                        } else {
                            printf("[EP0 PROBE] Device %u: Probe failed (%d), keeping 8-byte assumption\n", 
                                   dev_addr, probe_result);
                            /* Keep 8 bytes - already set above */
                        }
                        
                        /* Small delay to let device settle after probe */
                        usleep(5000);
                    }
                }
            }

            /* Enhanced Split Transaction Support: Use hub_addr/hub_port from Poseidon driver */
            if (dev_addr != 0) {
                /* Check if Poseidon provided split transaction info */
                if (hub_addr != 0 && hub_port != 0) {
                    #ifdef USB_DEBUG_VERBOSE
                    printf("[USB Handler] SPLIT TX: hub=%lu port=%lu dev=%u\n",
                           (unsigned long)hub_addr, (unsigned long)hub_port, (unsigned)dev_addr);
                    #endif
                    
                    /* Set up hub parent for split transactions */
                    struct usb_device *hubdev = usb_poseidon_alloc_dev((int)hub_addr);
                    if (hubdev) {
                        /* Configure hub device as High Speed */
                        hubdev->speed = USB_SPEED_HIGH;
                        hubdev->devnum = (int)hub_addr;
                        
                        /* Link this device to the hub */
                        udev->parent = hubdev;
                        udev->portnr = (unsigned char)hub_port;
                        
                        printf("[USB Handler] SPLIT TX: Configured dev=%u -> hub=%u port=%u\n",
                               (unsigned)dev_addr, (unsigned)hub_addr, (unsigned)hub_port);
                    } else {
                        printf("[USB Handler] SPLIT TX: WARNING - Failed to allocate hub device %u\n", (unsigned)hub_addr);
                    }
                } else {
                    /* Auto-detect hub topology if not provided by Poseidon */
                    printf("[USB Handler] SPLIT TX: Auto-detecting hub topology for dev=%u...\n", (unsigned)dev_addr);
                    
                    /* Try to find if this device should be behind a hub */
                    uint8_t auto_hub_addr = 0;
                    uint8_t auto_hub_port = 0;
                    usb_find_usb2_hub_address_port(udev, &auto_hub_addr, &auto_hub_port);
                    
                    if (auto_hub_addr != 0 && auto_hub_port != 0) {
                        struct usb_device *hubdev = usb_poseidon_alloc_dev((int)auto_hub_addr);
                        if (hubdev) {
                            hubdev->speed = USB_SPEED_HIGH;
                            hubdev->devnum = (int)auto_hub_addr;
                            udev->parent = hubdev;
                            udev->portnr = auto_hub_port;
                            printf("[USB Handler] SPLIT TX: Auto-configured dev=%u -> hub=%u port=%u\n",
                                   (unsigned)dev_addr, (unsigned)auto_hub_addr, (unsigned)auto_hub_port);
                        }
                    }
                }
                
                /* Configure device speed based on split transaction hint */
                if (sp_hint == 1) {
                    udev->speed = USB_SPEED_LOW;
                    printf("[USB Handler] SPLIT TX: Device %u configured as LOW SPEED\n", (unsigned)dev_addr);
                } else if (sp_hint == 2) {
                    udev->speed = USB_SPEED_FULL;
                    printf("[USB Handler] SPLIT TX: Device %u configured as FULL SPEED\n", (unsigned)dev_addr);
                } else if (sp_hint == 3) {
                    udev->speed = USB_SPEED_HIGH;
                    printf("[USB Handler] SPLIT TX: Device %u configured as HIGH SPEED\n", (unsigned)dev_addr);
                }
                
                /* For FS/LS devices behind a hub, configure EP0 with conservative packet size */
                if ((udev->speed == USB_SPEED_LOW || udev->speed == USB_SPEED_FULL) && udev->parent) {
                    udev->epmaxpacketin[0]  = 8;  /* Start with 8 bytes for first probe */
                    udev->epmaxpacketout[0] = 8;
#ifdef PACKET_SIZE_8
                    udev->maxpacketsize     = PACKET_SIZE_8;
#else
                    udev->maxpacketsize     = 8;
#endif
                    printf("[USB Handler] SPLIT TX: Device %u EP0 max packet size set to 8 bytes\n", (unsigned)dev_addr);
                }
                
                /* Log split transaction configuration */
                if (hub_addr != 0 && hub_port != 0) {
                    const char *speed_name = (udev->speed == USB_SPEED_LOW) ? "LOW" :
                                           (udev->speed == USB_SPEED_FULL) ? "FULL" : "HIGH";
                    printf("[USB Handler] SPLIT TX CONFIG: dev=%u speed=%s hub=%u port=%u\n",
                           (unsigned)dev_addr, speed_name, (unsigned)hub_addr, (unsigned)hub_port);
                }
            }

            /* Additional setup validation and debugging */
            if (setup_ptr == 0) {
                printf("[USB Handler] ERROR: setup_ptr is NULL!\n");
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }
            
            if (setup_bytes == NULL) {
                printf("[USB Handler] ERROR: Failed to read setup packet - invalid pointer range!\n");
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }
            
            #ifdef USB_DEBUG_VERBOSE
            printf("[USB Handler] RAW setup bytes at 0x%08lx: ", (unsigned long)setup_ptr);
            for (int i = 0; i < 8; i++) {
                printf("%02X ", setup_bytes[i]);
            }
            printf("\n");
            printf("[USB Handler] PARSED: reqtype=0x%02X req=0x%02X value=0x%04X index=0x%04X length=%u\n", 
                   req.requesttype, req.request, req.value, req.index, req.length);
            #endif
            
            /* Check if the memory appears to be zeroed */
            int all_zero = 1;
            for (int i = 0; i < 8; i++) {
                if (setup_bytes[i] != 0) {
                    all_zero = 0;
                    break;
                }
            }
            if (all_zero) {
                printf("[USB Handler] ERROR: All setup bytes are ZERO! Possible causes:\n");
                printf("[USB Handler]   1. Amiga didn't write to this memory location\n");
                printf("[USB Handler]   2. Cache coherency issue between Amiga and ARM\n");
                printf("[USB Handler]   3. Wrong pointer passed from Amiga\n");
                printf("[USB Handler] Retrying after delay and cache invalidation...\n");
                
                /* Try again after a small delay and another cache invalidation */
                usleep(1000);  /* 1ms delay */
                printf("[USB Handler] RETRY after delay: ");
                for (int i = 0; i < 8; i++) {
                    printf("%02X ", setup_bytes[i]);
                }
                printf("\n");
                
                /* Check if it's still all zeros after retry */
                all_zero = 1;
                for (int i = 0; i < 8; i++) {
                    if (setup_bytes[i] != 0) {
                        all_zero = 0;
                        break;
                    }
                }
                if (all_zero) {
                    printf("[USB Handler] STILL ALL ZEROS after retry - this indicates a fundamental problem\n");
                }
            }

/* Handle SET_ADDRESS specially */
            if (dev_addr == 0 && req.request == USB_REQ_SET_ADDRESS) {
                /* Track the new address that's being set */
                uint8_t new_addr = (uint8_t)(req.value & 0xFF);
                printf("[USB Handler] SET_ADDRESS detected: Device 0 -> %u\n", new_addr);
                
                /* Log current port status to see what device is being enumerated */
                extern struct ehci_ctrl ehcic[];
                struct ehci_ctrl *ehci_ctrl = &ehcic[0];
                if (ehci_ctrl && ehci_ctrl->hcor) {
                    uint32_t portsc = ehci_readl(&ehci_ctrl->hcor->or_portsc[0]);
                    printf("[USB Handler] PORTSC during SET_ADDRESS: 0x%08lx CCS=%d PE=%d PR=%d LS=%lu\n", 
                           (unsigned long)portsc, 
                           (portsc & (1<<0)) ? 1 : 0,  /* Current Connect Status */
                           (portsc & (1<<2)) ? 1 : 0,  /* Port Enabled */
                           (portsc & (1<<8)) ? 1 : 0,  /* Port Reset */
                           (portsc >> 10) & 0x3);      /* Line Status */
                }
                
                /* Device 0 SET_ADDRESS - just pass through to hardware */
                printf("[USB Handler] Device 0 SET_ADDRESS to %u - passing through\n", new_addr);
            }
            
            /* For device 0, we don't track state - just pass commands through */
            if (dev_addr == 0) {
                printf("[USB Handler] Device 0 command - passing through without state tracking\n");
            }
            
            /* Build pipe for control endpoint 0 */
            unsigned long pipe = (req.requesttype & 0x80) ? usb_rcvctrlpipe(udev, 0)
                                                          : usb_sndctrlpipe(udev, 0);

            /* Use data_len from Amiga parameters for IN transfers, count for OUT transfers */
            int transfer_len;
            if (req.requesttype & 0x80) {
                /* IN transfer: use data_len from Amiga request */
                transfer_len = (int)data_len;
                #ifdef USB_DEBUG_VERBOSE
                printf("[USB Handler] IN transfer: using data_len=%d from Amiga\n", transfer_len);
                #endif
            } else {
                /* OUT transfer: use count (actual data available) */
                transfer_len = (int)count;
                #ifdef USB_DEBUG_VERBOSE
                printf("[USB Handler] OUT transfer: using count=%d\n", transfer_len);
                #endif
            }
            
            int setup_len = (int)req.length;
            /* Fallback: if IN and transfer_len==0 but setup asks for data, use setup length (cap at 64) */
            if ((req.requesttype & 0x80) && transfer_len <= 0 && setup_len > 0) {
                int fb = setup_len;
                if (fb > 64) fb = 64;
                printf("[USB Handler] CTRL fallback: IN transfer_len=0 but setup_len=%d -> using %d bytes\n", setup_len, fb);
                transfer_len = fb;
            }

            /* Pre-settle: after SET_ADDRESS, some FS devices behind a TT need extra time
             * before a multi-packet GET_DESCRIPTOR(Device, >=18) completes fully. */
            if ((dev_addr != 0) && ((req.requesttype & 0x80) != 0) && (req.request == 0x06) &&
                (((unsigned)req.value >> 8) == 0x01) && (transfer_len >= 18)) {
                /* 20ms settle */
                usleep(20 * 1000);
            }

            /* Decode pipe for debug */
            #ifdef USB_DEBUG_VERBOSE
            {
                unsigned ptype = usb_pipetype(pipe);
                const char *ptname = (ptype==0)?"ISO":(ptype==1)?"INT":(ptype==2)?"CTRL":"BULK";
                unsigned pin = usb_pipein(pipe);
                unsigned pdev = usb_pipedevice(pipe);
                unsigned pep = usb_pipeendpoint(pipe);
                unsigned pmps = (unsigned)(pipe & 0x3);
                printf("[USB Handler] PIPE decode: type=%u(%s) dir=%s dev=%u ep=%u mps_code=%u\n",
                       ptype, ptname, pin?"IN":"OUT", pdev, pep, pmps);
            }
            #endif
            
            /* Validate parameters for string descriptors to prevent infinite loops */
            if ((req.requesttype & 0x80) && req.request == USB_REQ_GET_DESCRIPTOR && 
                ((req.value >> 8) == USB_DT_STRING)) {
                uint8_t string_index = req.value & 0xFF;
                uint16_t length = req.length;
                
                /* Check for unreasonably high index or length values that could indicate corruption */
                if (string_index > 32) {
                    printf("[USB Handler] WARNING: Unusually high string descriptor index %u\n", string_index);
                }
                
                if (length > 255) {
                    printf("[USB Handler] ERROR: Invalid string descriptor length %u (max 255)\n", length);
                    printf("[USB Handler] This indicates parameter corruption - limiting to 255 bytes\n");
                    req.length = 255;
                    transfer_len = (transfer_len > 255) ? 255 : transfer_len;
                }
                
                /* Track string requests to prevent infinite loops */
                static int string_fail_count = 0;
                static uint8_t last_string_index = 0;
                
                if (last_string_index == string_index) {
                    string_fail_count++;
                    if (string_fail_count > 3) {
                        printf("[USB Handler] ERROR: Too many retries for string index %u - preventing infinite loop\n", string_index);
                        usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                        break;
                    }
                } else {
                    string_fail_count = 0;
                    last_string_index = string_index;
                }
            }

            /* CRITICAL FIX: Validate CONFIG descriptor requests to prevent infinite loops */
            if ((req.requesttype & 0x80) && req.request == USB_REQ_GET_DESCRIPTOR && 
                ((req.value >> 8) == USB_DT_CONFIG)) {
                uint8_t config_index = req.value & 0xff;
                printf("[USB Handler] GET_DESCRIPTOR(CONFIG) dev_addr=%u config_index=%u length=%u\n", 
                   dev_addr, config_index, req.length);
                   
                /* Most USB devices only have configuration 0. Reject requests for invalid configs */
                if (config_index > 0) {
                    printf("[USB Handler] ERROR: Invalid CONFIG index %u - preventing infinite loop\n", config_index);
                    printf("[USB Handler] Device should only have CONFIG 0 - returning STALL\n");
                    usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                    break;
                }
            }

            /* Cache coherence:
               - OUT: ARM will read from data_ptr -> invalidate before read
               - IN: ARM will write to data_ptr -> flush after write */
            if ((req.requesttype & 0x80) == 0) { /* OUT */
                if (data_ptr && transfer_len > 0) {
                    /* OUT: ensure memory is written back so controller can read it */
                    Xil_DCacheFlushRange((INTPTR)data_ptr, (UINTPTR)transfer_len);
                }
            }

            /* Use synchronous control wrapper with timeout to avoid indefinite stalls */
            /* Base timeout */
            int timeout_ms = USB_CNTL_TIMEOUT; /* 100ms default */
            /* CRITICAL: Increase timeout to match U-Boot proven values for descriptor requests */
            if ((dev_addr == 0) && ((req.requesttype & 0x80) != 0) && (req.request == 0x06) &&
                ((((unsigned)req.value >> 8) == 0x01) || (((unsigned)req.value >> 8) == 0x02)) && (transfer_len <= 64)) {
                timeout_ms = 1000;  /* U-Boot uses 1000ms for control messages */
            }
            /* Also increase timeout for the first probe at a newly addressed device (dev>0, 8-byte header) */
            if ((dev_addr != 0) && ((req.requesttype & 0x80) != 0) && (req.request == 0x06) &&
                (((unsigned)req.value >> 8) == 0x01) && (transfer_len <= 8)) {
                timeout_ms = 1000;  /* U-Boot uses 1000ms for control messages */
            }
            #ifdef USB_DEBUG_VERBOSE
            printf("[USB Handler] CTRL submit: pipe=%lu dir=%s len=%d timeout=%dms dev=%u ep=%u bmReq=0x%02X bReq=0x%02X wValue=0x%04X wIndex=0x%04X\n",
                   pipe, (req.requesttype & 0x80) ? "IN" : "OUT", transfer_len, timeout_ms,
                   (unsigned)dev_addr, 0U,  /* ep=0 for control transfers */
                   (unsigned)req.requesttype, (unsigned)req.request,
                   (unsigned)req.value, (unsigned)req.index);
            #endif

            /* One-shot settle before the very first addr0 GET_DESCRIPTOR(DEVICE, <=8) */
            if (!did_first_addr0_getdev8 &&
                dev_addr == 0 && (req.requesttype & 0x80) && req.request == USB_REQ_GET_DESCRIPTOR &&
                ((req.value >> 8) == USB_DT_DEVICE) && (transfer_len <= 8)) {
                usleep(10 * 1000);
                did_first_addr0_getdev8 = 1;
            }
            
            /* USB3320C AS ROOT HUB:
             * Device address 0 is used for enumerating NEW devices, including the USB3320C hub itself.
             * When dev_addr == 0, the system is trying to enumerate a device that could be:
             * 1. The USB3320C hub (first enumeration after system startup), OR
             * 2. A device connected to any port of the USB3320C hub or downstream hubs
             * 
             * The previous validation was incorrect because it only checked the root hub port,
             * but new devices can be enumerated on external hub ports as well.
             * 
             * USB Enumeration Process with USB3320C as Root Hub:
             * 1. USB3320C hub is enumerated first (dev_addr=0 -> SET_ADDRESS -> dev_addr=1)
             * 2. Devices connected to USB3320C ports are then enumerated (dev_addr=0 -> SET_ADDRESS -> dev_addr=2,3,...)
             * 3. Each enumeration starts with GET_DESCRIPTOR(DEVICE) to learn about the device
             * 4. SET_ADDRESS assigns permanent address, then continue with full enumeration
             * 
             * The USB3320C acts as a physical root hub, eliminating the need for software root hub emulation.
             * All dev=0 transactions are passed through to the hardware - no validation against port status needed.
             * 
             * Timeouts during enumeration are normal when no device is present and are handled by the USB stack.
             */
            #ifdef USB_DEBUG_VERBOSE
            printf("[USB Handler] DEV=0: Device enumeration in progress (this could be on any hub port)\n");
            #endif

            /* DEBUG: Initialize memory with a known pattern before transfer */
            if (data_ptr && transfer_len > 0) {
                unsigned char *debug_ptr = (unsigned char *)data_ptr;
                printf("[USB DEBUG] BEFORE TRANSFER: Initializing buffer with pattern 0xAA\n");
                for (int i = 0; i < transfer_len; i++) {
                    debug_ptr[i] = 0xAA;  /* Known patttern */
                }
                printf("[USB DEBUG] Buffer initialized: ");
                for (int i = 0; i < (transfer_len > 18 ? 18 : transfer_len); i++) {
                    printf("%02x ", debug_ptr[i]);
                }
                printf("\n");
            }
            
            /* Enhanced logging for EP0 validation before transfer */
            if ((req.requesttype & 0x80) && req.request == USB_REQ_GET_DESCRIPTOR) {
                uint8_t desc_type = (req.value >> 8) & 0xFF;
                if (desc_type == USB_DT_CONFIG) {
                    printf("[EP0 VALIDATION] GET_DESCRIPTOR(CONFIG) dev=%u: EP0=%u bytes, transfer_len=%u\n", 
                           dev_addr, udev->epmaxpacketin[0], transfer_len);
                    printf("[EP0 VALIDATION] Expected packets: %u (assuming %u-byte packets)\n",
                           (transfer_len + udev->epmaxpacketin[0] - 1) / udev->epmaxpacketin[0],
                           udev->epmaxpacketin[0]);
                }
            }
            
            int retlen = usb_control_msg_with_split_retry(udev, pipe,
                                    req.request, req.requesttype,
                                    (unsigned short)req.value, (unsigned short)req.index,
                                    (void *)data_ptr, transfer_len, timeout_ms);
        
            /* DEBUG: GET_DESCRIPTOR(CONFIG) result */
            if ((req.requesttype & 0x80) && req.request == USB_REQ_GET_DESCRIPTOR && 
                ((req.value >> 8) == USB_DT_CONFIG)) {
                uint8_t config_index = req.value & 0xff;
                if (retlen < 0) {
                    printf("[USB Handler] GET_DESCRIPTOR(CONFIG) dev_addr=%u config_index=%u FAILED: %d\n", 
                       dev_addr, config_index, retlen);
                } else {
                    printf("[USB Handler] GET_DESCRIPTOR(CONFIG) dev_addr=%u config_index=%u SUCCESS: %d bytes\n", 
                       dev_addr, config_index, retlen);
                    
                    /* Enhanced validation: Check for 0xAA pattern indicating incomplete transfer */
                    if (data_ptr && retlen > 0) {
                        unsigned char *data_bytes = (unsigned char *)data_ptr;
                        int aa_count = 0;
                        int valid_bytes = 0;
                        
                        for (int i = 0; i < retlen && i < 34; i++) {
                            if (data_bytes[i] == 0xAA) {
                                aa_count++;
                            } else {
                                valid_bytes++;
                            }
                        }
                        
                        printf("[EP0 VALIDATION] Transfer result: %d valid bytes, %d bytes with 0xAA pattern\n", 
                               valid_bytes, aa_count);
                        
                        if (aa_count > 0) {
                            printf("[EP0 VALIDATION] WARNING: 0xAA pattern detected - likely incomplete transfer!\n");
                            printf("[EP0 VALIDATION] This suggests EP0 max packet size mismatch\n");
                        }
                        
                        /* Show first 18 bytes for analysis */
                        printf("[EP0 VALIDATION] First 18 bytes: ");
                        for (int i = 0; i < (retlen > 18 ? 18 : retlen); i++) {
                            printf("%02X ", data_bytes[i]);
                        }
                        printf("\n");
                    }
                }
            }
            if (retlen < 0) {
                /* Enhanced retry logic for devices behind hubs */
                int is_first_dev_desc_probe = (dev_addr != 0) && ((req.requesttype & 0x80) != 0) &&
                                              (req.request == 0x06) &&
                                              ((((unsigned)req.value >> 8) == 0x01)) &&
                                              (transfer_len <= 8);
                if (is_first_dev_desc_probe) {
                    printf("[USB Handler] First DEV DESC 8B to dev %u failed, attempting hub topology detection...\n", (unsigned)dev_addr);
                    
                    /* Try to auto-configure hub topology if device seems to be behind a hub */
                    if (!udev->parent && dev_addr > 1) {
                        /* Look for potential hub device (typically device 2) */
                        struct usb_device *potential_hub = usb_poseidon_get_dev(2);
                        if (potential_hub && potential_hub->maxchild > 0) {
                            printf("[USB Handler] Found potential hub at device 2 with %d ports\n", potential_hub->maxchild);
                            
                            /* Try linking this device to the hub on different ports */
                            for (int port = 1; port <= potential_hub->maxchild && retlen < 0; port++) {
                                printf("[USB Handler] Attempting to link dev %u -> hub 2 port %d\n", (unsigned)dev_addr, port);
                                
                                /* Configure the device as being behind the hub */
                                udev->parent = potential_hub;
                                udev->portnr = port;
                                
                                /* Try both Full-Speed and Low-Speed configurations */
                                int speeds[] = {USB_SPEED_FULL, USB_SPEED_LOW};
                                for (int s = 0; s < 2 && retlen < 0; s++) {
                                    udev->speed = speeds[s];
                                    
                                    /* Set appropriate EP0 max packet size */
                                    if (udev->speed == USB_SPEED_LOW) {
                                        udev->epmaxpacketin[0] = 8;
                                        udev->epmaxpacketout[0] = 8;
                                        udev->maxpacketsize = PACKET_SIZE_8;
                                    } else {
                                        udev->epmaxpacketin[0] = 8;  /* Conservative for first probe */
                                        udev->epmaxpacketout[0] = 8;
                                        udev->maxpacketsize = PACKET_SIZE_8;
                                    }
                                    
                                    printf("[USB Handler] Trying dev %u hub topology: hub=2 port=%d speed=%s\n",
                                           (unsigned)dev_addr, port, 
                                           (udev->speed == USB_SPEED_LOW) ? "LOW" : "FULL");
                                    
                                    /* Reset toggles and retry */
                                    udev->toggle[0] = 0;
                                    udev->toggle[1] = 0;
                                    usleep(10 * 1000);
                                    
                                    retlen = usb_control_msg_with_split_retry(udev, pipe,
                                                                     req.request, req.requesttype,
                                                                     (unsigned short)req.value, (unsigned short)req.index,
                                                                     (void *)data_ptr, transfer_len, 1000);
                                    
                                    if (retlen >= 0) {
                                        printf("[USB Handler] SUCCESS! Device %u now properly configured behind hub\n", (unsigned)dev_addr);
                                        break;
                                    }
                                }
                            }
                            
                            if (retlen < 0) {
                                /* Restore device to original state if hub detection failed */
                                udev->parent = NULL;
                                udev->portnr = 0;
                                printf("[USB Handler] Hub topology detection failed, restoring original configuration\n");
                            }
                        }
                    }
                    
                    /* If still failing, try the original retry logic */
                    if (retlen < 0) {
                        printf("[USB Handler] Falling back to original retry logic...\n");
                        /* small settle delay */
                        usleep(10 * 1000);
                        /* Reset EP0 toggles */
                        udev->toggle[0] = 0;
                        udev->toggle[1] = 0;
                        /* Retry once */
                        retlen = usb_control_msg_with_split_retry(udev, pipe,
                                                 req.request, req.requesttype,
                                                 (unsigned short)req.value, (unsigned short)req.index,
                                                 (void *)data_ptr, transfer_len, 1000);

                        /* If still failing and device is behind a HS hub, try split fallbacks:
                        * - Guess downstream port (1..4) on the HS hub
                        * - Try both FULL and LOW as device speed hints for split-transaction setup
                        */
                        if (retlen < 0 && udev->parent && udev->parent->speed == USB_SPEED_HIGH) {
                            unsigned char orig_speed = udev->speed;
                            int speeds[2];
                            /* Try current speed first, then alternate */
                            speeds[0] = (orig_speed == USB_SPEED_LOW) ? USB_SPEED_LOW : USB_SPEED_FULL;
                            speeds[1] = (orig_speed == USB_SPEED_LOW) ? USB_SPEED_FULL : USB_SPEED_LOW;
                            for (int si = 0; si < 2 && retlen < 0; ++si) {
                                udev->speed = (unsigned char)speeds[si];
                                for (int p = 1; p <= 8 && retlen < 0; ++p) {
                                    udev->portnr = (unsigned char)p;
                                    printf("[USB Handler] DEV %u split-fallback: try port=%d speed=%s for DEV DESC 8B...\n",
                                        (unsigned)dev_addr, p, (udev->speed == USB_SPEED_LOW) ? "LOW" : "FULL");
                                    /* For FS/LS, EP0 MPS=8 during first probe */
                                    if (udev->speed == USB_SPEED_LOW || udev->speed == USB_SPEED_FULL) {
                                        udev->epmaxpacketin[0]  = 8;
                                        udev->epmaxpacketout[0] = 8;
#ifdef PACKET_SIZE_8
                                        udev->maxpacketsize     = PACKET_SIZE_8;
#else
                                        udev->maxpacketsize     = 8;
#endif
                                    }
                                    /* Reset toggles and small settle */
                                    udev->toggle[0] = 0; udev->toggle[1] = 0;
                                    usleep(5 * 1000);
                                    retlen = usb_control_msg(udev, pipe,
                                                            req.request, req.requesttype,
                                                            (unsigned short)req.value, (unsigned short)req.index,
                                                            (void *)data_ptr, transfer_len, 300);
                                }
                            }
                            if (retlen < 0) {
                                /* Restore original hint to avoid side-effects */
                                udev->speed = orig_speed;
                            } else {
                                printf("[USB Handler] DEV %u split-fallback succeeded (port=%u speed=%s)\n",
                                    (unsigned)dev_addr, (unsigned)udev->portnr,
                                    (udev->speed == USB_SPEED_LOW) ? "LOW" : "FULL");
                            }
                        }
                    }
                }
            }

            /* If we got a full device descriptor length but VID/PID are zero, retry once with settle */
            if (retlen >= 18 && ((req.requesttype & 0x80) != 0) && (req.request == 0x06) && (((unsigned)req.value >> 8) == 0x01)) {
                unsigned char *db_chk = (unsigned char *)data_ptr;
                unsigned short vid_chk = (unsigned short)db_chk[8] | ((unsigned short)db_chk[9] << 8);
                unsigned short pid_chk = (unsigned short)db_chk[10] | ((unsigned short)db_chk[11] << 8);
                if (vid_chk == 0x0000 && pid_chk == 0x0000) {
                    printf("[USB Handler] DEV DESC %dB for dev %u has VID/PID=0, retrying after 20ms with EP0 toggle reset...\n",
                            retlen, (unsigned)dev_addr);
                    usleep(20 * 1000);
                    /* Reset EP0 toggles */
                    udev->toggle[0] = 0;
                    udev->toggle[1] = 0;
                    /* Retry once */
                    retlen = usb_control_msg(udev, pipe,
                                                req.request, req.requesttype,
                                                (unsigned short)req.value, (unsigned short)req.index,
                                                (void *)data_ptr, transfer_len, 300);
                }
            }
            if (retlen < 0) {
                printf("[USB Handler] usb_control_msg failed: %d (status=%ld act_len=%d)\n",
                       retlen, udev->status, udev->act_len);
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }

            int act = retlen;
            if (act < 0) act = 0;

            if (dev_addr == 0 && req.request == USB_REQ_SET_ADDRESS && retlen >= 0) {
                uint8_t new_addr = (uint8_t)(req.value & 0xFF);
                printf("[USB Handler] SET_ADDRESS successful. Updating udev->devnum for new addr %u\n", new_addr);
                if (udev) {
                    udev->devnum = new_addr;
                }
            }

            if (req.requesttype & 0x80) { /* IN */
                /* Ensure CPU sees freshly DMA-written data */
                if (data_ptr && act > 0) {
                    printf("[USB CACHE] BEFORE invalidate: ptr=0x%08lx len=%d\n", (unsigned long)data_ptr, act);
                    
                    /* Show data BEFORE cache invalidation */
                    unsigned char *raw_before = (unsigned char *)data_ptr;
                    printf("[USB CACHE] BEFORE: ");
                    for (int i = 0; i < (act > 18 ? 18 : act); i++) {
                        printf("%02x ", raw_before[i]);
                    }
                    printf("\n");
                    
                    Xil_DCacheInvalidateRange((INTPTR)data_ptr, (UINTPTR)act);
                    
                    /* Show data AFTER cache invalidation */
                    unsigned char *raw_after = (unsigned char *)data_ptr;
                    printf("[USB CACHE] AFTER:  ");
                    for (int i = 0; i < (act > 18 ? 18 : act); i++) {
                        printf("%02x ", raw_after[i]);
                    }
                    printf("\n");
                }
                /* If we just fetched a full CONFIG descriptor, apply it so ep maxpacket is correct */
                if (data_ptr && act >= 9 && req.request == USB_REQ_GET_DESCRIPTOR &&
                    ((req.value >> 8) == USB_DT_CONFIG)) {
                    /* Read wTotalLength (little-endian) */
                    unsigned char *cfgb = (unsigned char *)data_ptr;
                    unsigned short wTot = (unsigned short)cfgb[2] | ((unsigned short)cfgb[3] << 8);
                    if (act >= wTot) {
                        int ap_ret = usb_poseidon_apply_config(udev, cfgb, act);
                        if (ap_ret == 0) {
                            printf("[USB Handler] Applied CONFIG to device %u: endpoints sized\n", (unsigned)dev_addr);
                        } else {
                            printf("[USB Handler] WARNING: Failed to apply CONFIG for device %u (ret=%d)\n", (unsigned)dev_addr, ap_ret);
                        }
                    }
                }
                /* ARM-side descriptor dump for debugging */
                dump_parsed_descriptors_arm(dev_addr, &req, (const void *)data_ptr, act);
                usb_state.data_length = (uint32_t)act;
            } else {
                usb_state.data_length = (uint32_t)transfer_len;
            }

            dsb();
            usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
            
            /* EXTRA DEBUG ESPECIICO PARA DEV=0 - COMPLETION */
            if (dev_addr == 0) {
                printf("\n*** ARM DEBUG DEV=0 *** CONTROL_XFER COMPLETED ***\n");
                printf("[ARM] Command processing completed successfully\n");
                printf("[ARM] Final status: 0x%08lx (READY=%s, COMPLETE=%s)\n", 
                       usb_state.status,
                       (usb_state.status & USB_STATUS_READY) ? "YES" : "NO",
                       (usb_state.status & USB_STATUS_COMPLETE) ? "YES" : "NO");
                printf("[ARM] Data length returned: %lu bytes\n", (unsigned long)act);
                printf("[ARM] ARM is now ready for next command from Amiga\n");
            }
            
            printf("[USB Handler] CTRL done: act=%d status=0x%lx\n", act, usb_state.status);
            break;
        }
        
        case USB_OP_BULK_XFER: {
            printf("[USB Handler] BULK transfer using params[]\n");
            /* Parameters (filled by BUFSEL+CMD_DATA):
               params[0] = d0 (dev/ep/dir)
               params[1] = reserved/unused for bulk
               params[2] = data_ptr  (shared memory)
               params[3] = data_len  (optional)
               params[4] = hub_addr (optional: HS hub address)
               params[5] = hub_port (optional: downstream port on hub)
               params[6] = sp_hint  (optional: 1=LOW,2=FULL,3=HIGH)
               REG_ZZ_USB_COUNT = transfer length */
            uint32_t d0        = usb_state.params[0];
            uint32_t data_ptr  = usb_state.params[2];
            uint32_t data_len  = usb_state.params[3];
            uint32_t hub_addr  = usb_state.params[4]; /* optional: HS hub address */
            uint32_t hub_port  = usb_state.params[5]; /* optional: downstream port on hub */
            uint32_t sp_hint   = usb_state.params[6]; /* optional: 1=LOW,2=FULL,3=HIGH */
            uint8_t dev_addr  = (d0 >> 24) & 0xFF;
            uint8_t ep        = (d0 >> 16) & 0xFF;
            uint8_t dir       = (d0 >> 8)  & 0xFF; /* 0=OUT, nonzero=IN */
            uint32_t count    = usb_state.data_length; /* transfer length */
            int transfer_len  = (int)count;

            printf("[USB Handler] BULK params: dev=%u ep=%u dir=%u data_ptr=0x%lx data_len=%lu count=%lu\n",
                   dev_addr, ep, dir, (unsigned long)data_ptr,
                   (unsigned long)data_len, (unsigned long)count);

            struct usb_device *udev = usb_poseidon_alloc_dev(dev_addr);
            if (!udev) {
                printf("[USB Handler] ERROR: usb_poseidon_alloc_dev(%u) returned NULL\n", dev_addr);
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }

            /* Configure split transaction for BULK if needed */
            if (dev_addr != 0 && hub_addr != 0 && hub_port != 0) {
                printf("[USB Handler] BULK SPLIT TX: hub_addr=%lu hub_port=%lu for dev=%u\n",
                       (unsigned long)hub_addr, (unsigned long)hub_port, (unsigned)dev_addr);
                
                /* Set up hub parent for split transactions */
                struct usb_device *hubdev = usb_poseidon_alloc_dev((int)hub_addr);
                if (hubdev) {
                    hubdev->speed = USB_SPEED_HIGH;
                    hubdev->devnum = (int)hub_addr;
                    udev->parent = hubdev;
                    udev->portnr = (unsigned char)hub_port;
                    
                    /* Configure device speed */
                    if (sp_hint == 1) udev->speed = USB_SPEED_LOW;
                    else if (sp_hint == 2) udev->speed = USB_SPEED_FULL;
                    else if (sp_hint == 3) udev->speed = USB_SPEED_HIGH;
                    
                    printf("[USB Handler] BULK SPLIT TX: Configured dev=%u -> hub=%u port=%u speed=%u\n",
                           (unsigned)dev_addr, (unsigned)hub_addr, (unsigned)hub_port, (unsigned)udev->speed);
                }
            }

            unsigned long pipe = dir ? usb_rcvbulkpipe(udev, ep) : usb_sndbulkpipe(udev, ep);

            /* Cache coherence */
            if (dir == 0) { /* OUT */
                if (data_ptr && transfer_len > 0) {
                    /* OUT: write back cache so controller reads updated memory */
                    Xil_DCacheFlushRange((INTPTR)data_ptr, (UINTPTR)transfer_len);
                }
            }

            int actual = 0;
            int ret = usb_bulk_msg(udev, pipe, (void *)data_ptr, transfer_len, &actual, USB_TIMEOUT_MS(pipe));
            if (ret < 0) {
                printf("[USB Handler] usb_bulk_msg failed: %d\n", ret);
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }

            int act = actual;
            if (act < 0) act = 0;

            if (dir != 0) { /* IN */
                if (data_ptr && act > 0) {
                    /* IN: invalidate to discard stale cache and see DMA data */
//                    Xil_DCacheInvalidateRange((INTPTR)data_ptr, (UINTPTR)act);
                }
                usb_state.data_length = (uint32_t)act;
            } else {
                usb_state.data_length = (uint32_t)act;
            }

            dsb();
            usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
            printf("[USB Handler] BULK done: act=%d status=0x%lx\n", act, usb_state.status);
            break;
        }
        
        case USB_OP_INT_XFER: {
#if 1 //defined(USB_DEBUG_VERBOSE)
//            printf("[USB Handler] INTERRUPT transfer using params[]\n");
            printf("[USB Handler] USB_OP_INT_XFER start\n");
#endif
            /* Parameters (filled by BUFSEL+CMD_DATA):
               params[0] = d0 (dev/ep/dir)
               params[1] = interval (optional, in frames) or 0 for default
               params[2] = data_ptr  (shared memory)
               params[3] = data_len  (optional)
               params[4] = hub_addr (optional: HS hub address)
               params[5] = hub_port (optional: downstream port on hub)
               params[6] = sp_hint  (optional: 1=LOW,2=FULL,3=HIGH)
               REG_ZZ_USB_COUNT = transfer length */
            uint32_t d0        = usb_state.params[0];
            uint32_t interval  = usb_state.params[1];
            uint32_t data_ptr  = usb_state.params[2];
            uint32_t data_len  = usb_state.params[3];
            uint32_t hub_addr  = usb_state.params[4]; /* optional: HS hub address */
            uint32_t hub_port  = usb_state.params[5]; /* optional: downstream port on hub */
            uint32_t sp_hint   = usb_state.params[6]; /* optional: 1=LOW,2=FULL,3=HIGH */
            uint8_t dev_addr  = (d0 >> 24) & 0xFF;
            uint8_t ep        = (d0 >> 16) & 0xFF;
            uint8_t dir       = (d0 >> 8)  & 0xFF; /* 0=OUT, nonzero=IN */
            uint32_t count    = usb_state.data_length; /* transfer length */
            int transfer_len  = (int)count;

            /* Reduced logging - only show essential info for interrupt transfers */
            if ((dev_addr != 1) || (ep != 1)) {
                printf("[USB Handler] INT: dev=%u ep=%u len=%lu\n", dev_addr, ep, (unsigned long)data_len);
            }
            
#if 0
            /* PRIORITY 1: Try to get data from ARM polling first (much faster!) */
            if (dir != 0) { /* Only for IN transfers - polling is for input devices like mouse/keyboard */
                int available = z3660_usb_polling_data_available(dev_addr, ep & 0x0F); /* Strip direction bit from endpoint */
                if (available > 0) {
                    /* Use cached data from ARM background polling */
                    int retrieved = z3660_usb_get_polling_data(dev_addr, ep & 0x0F, (void*)data_ptr, transfer_len);
                    
                    if (retrieved > 0) {
                        /* Cache coherence - flush to ensure Amiga sees the data */
                        Xil_DCacheFlushRange((INTPTR)data_ptr, (UINTPTR)retrieved);
                        
                        usb_state.data_length = (uint32_t)retrieved;
                        usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
                        
                        printf("[USB Handler] INT_XFER: Used ARM polling data: %d bytes for dev=%u ep=%u (FAST PATH)\n",
                               retrieved, dev_addr, ep & 0x0F);
                        
                        /* Signal Amiga again to ensure it sees the completion */
                        amiga_interrupt_set(AMIGA_INTERRUPT_USB);
                        
                        break; /* Done - data retrieved from ARM polling cache */
                    }
                }
                printf("[USB Handler] INT_XFER: No ARM polling data available, falling back to direct USB transfer\n");
            }
#endif
            struct usb_device *udev = usb_poseidon_alloc_dev(dev_addr);
            if (!udev) {
                printf("[USB Handler] ERROR: usb_poseidon_alloc_dev(%u) returned NULL\n", dev_addr);
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }

            /* Configure split transaction for INT if needed */
            if (dev_addr != 0 && hub_addr != 0 && hub_port != 0) {
                printf("[USB Handler] INT SPLIT TX: hub_addr=%lu hub_port=%lu for dev=%u\n",
                       (unsigned long)hub_addr, (unsigned long)hub_port, (unsigned)dev_addr);
                
                /* Set up hub parent for split transactions */
                struct usb_device *hubdev = usb_poseidon_alloc_dev((int)hub_addr);
                if (hubdev) {
                    hubdev->speed = USB_SPEED_HIGH;
                    hubdev->devnum = (int)hub_addr;
                    udev->parent = hubdev;
                    udev->portnr = (unsigned char)hub_port;
                    
                    /* Configure device speed */
                    if (sp_hint == 1) udev->speed = USB_SPEED_LOW;
                    else if (sp_hint == 2) udev->speed = USB_SPEED_FULL;
                    else if (sp_hint == 3) udev->speed = USB_SPEED_HIGH;
                    
                    printf("[USB Handler] INT SPLIT TX: Configured dev=%u -> hub=%u port=%u speed=%u\n",
                           (unsigned)dev_addr, (unsigned)hub_addr, (unsigned)hub_port, (unsigned)udev->speed);
                }
            }

            unsigned long pipe = dir ? usb_rcvintpipe(udev, ep) : usb_sndintpipe(udev, ep);
            int intr_interval = interval ? (int)interval : 1; /* default minimal interval */

            /* Cache coherence */
            if (dir == 0) { /* OUT */
                if (data_ptr && transfer_len > 0) {
                    /* OUT: write back cache so controller reads updated memory */
                    Xil_DCacheFlushRange((INTPTR)data_ptr, (UINTPTR)transfer_len);
                }
            }

            /* NEW ASYNCHRONOUS APPROACH: Non-blocking polling with immediate response */
#if 1 //defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] DEBUG INT_XFER: dev_addr=%u ep=%u dir=%u\n", dev_addr, ep, dir);
            printf("[USB Handler] DEBUG INT_XFER: data_ptr=0x%08lx transfer_len=%d intr_interval=%d\n", 
                   (unsigned long)data_ptr, transfer_len, intr_interval);
            printf("[USB Handler] DEBUG INT_XFER: pipe=0x%08lx udev->speed=%u\n", 
                   (unsigned long)pipe, (unsigned)udev->speed);
            printf("[USB Handler] About to call usb_int_msg()...\n");
#endif
            int ret = usb_int_msg(NULL, udev, pipe, (void *)data_ptr, transfer_len, intr_interval);
#if 1 //defined(USB_DEBUG_VERBOSE)
            printf("[USB Handler] usb_int_msg returned: %d\n", ret);
#endif
            if (ret < 0) {
                printf("[USB Handler] usb_int_msg failed: %d\n", ret);
                printf("[USB Handler] ERROR DETAILS: udev=0x%08lx pipe=0x%08lx data_ptr=0x%08lx len=%d interval=%d\n",
                       (unsigned long)udev, (unsigned long)pipe, (unsigned long)data_ptr, transfer_len, intr_interval);
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }

            int act = udev->act_len;
            if (act < 0) act = 0;

            if (dir) { /* IN */
                if (data_ptr && act > 0) {
                    /* IN: invalidate to discard stale cache and see DMA data */
//                    Xil_DCacheInvalidateRange((INTPTR)data_ptr, (UINTPTR)act);
                }
                usb_state.data_length = (uint32_t)act;
            } else {
                usb_state.data_length = (uint32_t)transfer_len;
            }

            dsb();
            usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
            
            /* OPTIMIZED INTERRUPT SIGNALING: Only signal when we have actual data */
            if (act > 0) {
                /* Always signal immediately when we have mouse data */
                amiga_interrupt_set(AMIGA_INTERRUPT_USB);
//                printf("[USB Handler] Mouse data available: %d bytes - signaling Amiga\n", act);
            } else {
                /* NO DATA - DO NOT SIGNAL - let Amiga timer handle resubmission */
                /* This eliminates the constant interrupt traffic that causes high CPU */
//                static int no_signal_counter = 0;
//                no_signal_counter++;
//                if ((no_signal_counter % 100) == 0) {
//                    printf("[USB Handler] No mouse data - NOT signaling (count: %d)\n", no_signal_counter);
//                }
            }
            
            /* Comentado para reducir spam de mensajes 
            if (act > 0) {
                printf("[USB INT] Mouse data interrupt: %d bytes\n", act);
            } else {
                printf("[USB INT] Keep-alive interrupt (no data)\n");
            }
            */
            printf("[USB Handler] USB_OP_INT_XFER end\n");
            break;
        }
        
        case USB_OP_INT_XFER_ASYNC: {
//            printf("[USB Handler] ASYNC Interrupt transfer (NEW IMPLEMENTATION)\n");
            
            /* ASYNC PATTERN: Don't do usb_int_msg() immediately!
             * Instead, queue the request and only complete when device has real data */
            
            /* Extract parameters same as regular INT_XFER */
            uint32_t d0        = usb_state.params[0];
            uint32_t data_ptr  = usb_state.params[2];
            uint32_t data_len  = usb_state.params[3];
            uint8_t dev_addr  = (d0 >> 24) & 0xFF;
            uint8_t ep        = (d0 >> 16) & 0xFF;
            uint8_t dir       = (d0 >> 8)  & 0xFF;
//            uint32_t count    = usb_state.data_length;
            
//            printf("[USB Handler] ASYNC INT: dev=%u ep=%u dir=%u len=%lu - QUEUING for later processing\n", 
//                   dev_addr, ep, dir, (unsigned long)data_len);
            
            /* CRITICAL: Store request details for background processing */
            /* This simulates what the ARM polling system should do */
            
            /* Store the request in global variable */
            async_int_request.pending = 1;
            async_int_request.dev_addr = dev_addr;
            async_int_request.ep = ep;
            async_int_request.dir = dir;
            async_int_request.data_ptr = data_ptr;
            async_int_request.data_len = data_len;
            async_int_request.request_time = get_system_time_ms();
#if 0            
            /* DON'T set COMPLETE yet! This is the key difference.
             * The request stays PENDING until background task processes it */
            usb_state.status = USB_STATUS_READY; /* NOT COMPLETE! */
            
            printf("[USB Handler] ASYNC INT: Request queued, will be processed in background\n");
            
            /* CRITICAL: Do NOT complete immediately to avoid race condition!
             * The Amiga hasn't reached Wait() yet, so we must NOT signal now.
             * The background task z3660_usb_process_async_requests() will handle completion. */
#endif            
            /* Always keep request BUSY - background task will complete it */
            usb_state.status = USB_STATUS_BUSY; /* Keep BUSY - request is pending for background processing */
//            printf("[USB Handler] ASYNC INT: Keeping request BUSY for background processing\n");
            return; /* Return early to prevent clearing BUSY flag */
            
            break;
        }
        
        case USB_OP_ISO_XFER: {
            printf("[USB Handler] Isochronous transfer (not implemented yet)\n");
            // TODO: Implement isochronous transfers
            usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR; // Not implemented
            break;
        }
        
        case USB_OP_ADD_POLLING_DEVICE: {
            printf("[USB Handler] Add polling device\n");
            if (!usb_state.initialized) {
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }
            
            /* Parameters:
               params[0] = device_addr
               params[1] = endpoint
               params[2] = interval_ms
               params[3] = device_type */
            uint8_t device_addr = (uint8_t)(usb_state.params[0] & 0xFF);
            uint8_t endpoint = (uint8_t)(usb_state.params[1] & 0xFF);
            uint8_t interval_ms = (uint8_t)(usb_state.params[2] & 0xFF);
            uint8_t device_type = (uint8_t)(usb_state.params[3] & 0xFF);
            
            int result = arm_add_polling_device(device_addr, endpoint, interval_ms, device_type);
            if (result == 0) {
                usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
            } else {
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
            }
            usb_state.data_length = (uint32_t)result;
            break;
        }
        
        case USB_OP_REMOVE_POLLING_DEVICE: {
            printf("[USB Handler] Remove polling device\n");
            if (!usb_state.initialized) {
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }
            
            /* Parameters:
               params[0] = device_addr
               params[1] = endpoint */
            uint8_t device_addr = (uint8_t)(usb_state.params[0] & 0xFF);
            uint8_t endpoint = (uint8_t)(usb_state.params[1] & 0xFF);
            
            int result = arm_remove_polling_device(device_addr, endpoint);
            if (result == 0) {
                usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
            } else {
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
            }
            usb_state.data_length = (uint32_t)result;
            break;
        }
        
        case USB_OP_LIST_POLLING_DEVICES: {
            printf("[USB Handler] List polling devices\n");
            if (!usb_state.initialized) {
                usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
                break;
            }
            
            int result = arm_list_polling_devices();
            usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
            usb_state.data_length = (uint32_t)result; // Number of active devices
            break;
        }
        case 0:
//            printf("[USB Handler] 0 command received. Clear the ARM status to prepare for next transfer\n");
            usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
            usb_state.data_length = 0;
            break;
        default: {
            printf("[USB Handler] Unknown command: 0x%lx\n", command);
            usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
            break;
        }
    }
    
    // Clear busy flag when command processing is complete
    usb_state.status &= ~USB_STATUS_BUSY;
}

void log_portsc(const char *tag) {
    uint32_t ps = ehci_read_reg(EHCI_PORTSC1);
    unsigned ccs = (ps >> 0) & 1;
    unsigned csc = (ps >> 1) & 1;
    unsigned ped = (ps >> 2) & 1;
    unsigned pedc= (ps >> 3) & 1;
    unsigned oca = (ps >> 4) & 1;
    unsigned occ = (ps >> 5) & 1;
    unsigned fpr = (ps >> 6) & 1;
    unsigned susp= (ps >> 7) & 1;
    unsigned pr  = (ps >> 8) & 1;
    unsigned ls  = (ps >> 10) & 0x3; /* 00=SE0, 01=FS, 10=LS */
    unsigned pp  = (ps >> 12) & 1;
    unsigned po  = (ps >> 13) & 1;   /* Port Owner: handoff to companion */
    printf("[USB Handler] %s PORTSC1=0x%08lx CCS=%u CSC=%u PED=%u PEDC=%u OCA=%u OCC=%u FPR=%u SUSP=%u PR=%u LS=%u PP=%u PO=%u\n",
           tag, (unsigned long)ps, ccs, csc, ped, pedc, oca, occ, fpr, susp, pr, ls, pp, po);
}

/*
 * Get current USB status for external queries
 */
uint32_t z3660_usb_get_status(void)
{
    return usb_state.status;
}

/*
 * Check if USB system is offline/shutdown
 */
int z3660_usb_is_offline(void)
{
    return usb_state.offline;
}
void log_core_regs(const char *tag) {
    uint32_t usbcmd  = ehci_read_reg(EHCI_USBCMD);
    uint32_t usbsts  = ehci_read_reg(EHCI_USBSTS);     /* define EHCI_USBSTS offset */
    uint32_t conf    = ehci_read_reg(EHCI_CONFIGFLAG); /* define EHCI_CONFIGFLAG offset */
    printf("[USB Handler] %s USBCMD=0x%08lx USBSTS=0x%08lx CONFIGFLAG=0x%08lx\n",
           tag, (unsigned long)usbcmd, (unsigned long)usbsts, (unsigned long)conf);
}

/*
 * Retrieve data from ARM polling buffer for Amiga
 * This is called when Amiga receives INT6 and wants to get the cached data
 */
int z3660_usb_get_polling_data(uint8_t device_addr, uint8_t endpoint, void *buffer, uint32_t max_len)
{
    if (!buffer || max_len <= 0) {
        return -1;
    }
    
    // Find the polling device
    for (int i = 0; i < polling_device_count; i++) {
        polling_device_t *dev = &polling_devices[i];
        
        if (dev->active && 
            dev->device_addr == device_addr &&
            dev->endpoint == endpoint &&
            dev->data_length > 0) {
            
            // Copy data to Amiga buffer
            int copy_len = (dev->data_length > max_len) ? max_len : dev->data_length;
            memcpy(buffer, dev->data_buffer, copy_len);
            
            // Clear the data (mark as consumed)
            dev->data_length = 0;
            
//            printf("[ARM Polling] Amiga retrieved %d bytes from device %u ep=%u\n", 
//                   copy_len, device_addr, endpoint);
            
            return copy_len;
        }
    }
    
    // No data available
    return 0;
}

/*
 * Check if ARM polling has data available for a specific device
 * Returns: data length available, or 0 if no data
 * SAFE VERSION: Just check buffer, no active polling to prevent lockups
 */
int z3660_usb_polling_data_available(struct pt *pt, uint8_t device_addr, uint8_t endpoint)
{
    (void)pt;
    for (int i = 0; i < polling_device_count; i++) {
        polling_device_t *dev = &polling_devices[i];
        
        if (dev->active && 
            dev->device_addr == device_addr &&
            dev->endpoint == endpoint) {
            
            /* FAKE DATA COMPLETELY DISABLED - ANY fake data causes immediate system lockup */
            /* The problem is that ANY successful interrupt transfer creates an infinite loop */
            /* Poseidon immediately sends another request when it gets data */
            /* This happens faster than the system can handle, causing deadlock */
            /*
            if (dev->device_type == POLLING_DEVICE_MOUSE && dev->data_length == 0) {
                // ANY fake data generation code causes lockup
            }
            */
            
            /* EFFICIENT USB POLLING: Use persistent interrupt queues */
            
            /* Create persistent interrupt queue if not exists */
            if (!dev->int_queue || !dev->udev) {
                /* Get or create USB device structure */
                dev->udev = usb_poseidon_alloc_dev(dev->device_addr);
                if (!dev->udev) {
                    printf("[ARM POLLING] ERROR: Cannot get USB device %u\n", device_addr);
                    return 0;
                }
                
                /* Create persistent interrupt queue - reuse until device is removed */
                unsigned long pipe = usb_rcvintpipe(dev->udev, dev->endpoint);
                printf("[ARM POLLING] About to create queue: dev=%u ep=%u pipe=0x%lx interval=%u\n", 
                       device_addr, dev->endpoint, pipe, dev->interval_ms);
                       
                dev->int_queue = create_int_queue(dev->udev, pipe, 2, 8, NULL, dev->interval_ms);
                if (!dev->int_queue) {
                    printf("[ARM POLLING] ERROR: Failed to create interrupt queue for dev=%u\n", device_addr);
                    return 0;
                }
                
                dev->queue_create_time = get_system_time_ms();
                dev->queue_recreate_count++;
                printf("[ARM POLLING] Created persistent queue for dev=%u (attempt %lu) - SUCCESS\n", 
                       device_addr, dev->queue_recreate_count);
                printf("[ARM POLLING] Queue details: int_queue=%p udev=%p\n", 
                       dev->int_queue, dev->udev);
            }
            
            /* Poll the persistent queue for data */
            void *buffer_data = poll_int_queue(dev->udev, dev->int_queue);
            static int poll_debug_counter = 0;
            if ((++poll_debug_counter % 1000) == 0) {
                printf("[ARM POLLING] Polling cycle #%d: dev=%u ep=%u buffer=%p act_len=%d\n", 
                       poll_debug_counter, device_addr, endpoint, buffer_data, 
                       dev->udev ? (int)dev->udev->act_len : -1);
            }
            
            /* CRITICAL FIX: Even if buffer_data is NULL, we might have data if act_len > 0 */
            /* This can happen if the poll_int_queue implementation has issues with buffer return */
            if (dev->udev->act_len > 0) {
                if (buffer_data) {
                    /* Normal case - buffer pointer is valid */
                    /* Copy and process the data */
                    memcpy(dev->data_buffer, buffer_data, min(dev->udev->act_len, sizeof(dev->data_buffer)));
                    dev->data_length = dev->udev->act_len;
                    
                    /* Process based on device type - just signal completion */
                    if (dev->device_type == POLLING_DEVICE_MOUSE) {
                        /* Parse mouse data for logging/debugging */
                        uint8_t buttons = ((uint8_t*)dev->data_buffer)[0];
                        int8_t dx = ((int8_t*)dev->data_buffer)[1];
                        int8_t dy = ((int8_t*)dev->data_buffer)[2];
                        
                        /* Only signal completion if there's actual movement or button change */
                        if (buttons != 0 || dx != 0 || dy != 0) {
                            z3660_usb_on_transaction_complete();
                        }
                    } else {
                        /* For other devices, just signal transaction complete */
                        z3660_usb_on_transaction_complete();
                    }
                } else {
                    /* FALLBACK: poll_int_queue returned NULL but act_len > 0 */
                    /* This might happen if EHCI implementation has buffer handling issues */
                    printf("[ARM WARNING] poll_int_queue returned NULL but act_len=%d\n", dev->udev->act_len);
                    
                    /* Try to access the buffer directly from the USB device structure */
                    /* This is a fallback that might work depending on EHCI implementation */
                    if (dev->udev->status == 0) {  /* No error status */
                        /* For mouse, assume typical 3-byte HID report format */
                        if (dev->device_type == POLLING_DEVICE_MOUSE && dev->udev->act_len >= 3) {
                            /* Create a zero buffer - this prevents phantom movement */
                            uint8_t zero_data[8] = {0};
                            memcpy(dev->data_buffer, zero_data, sizeof(zero_data));
                            dev->data_length = 0;  /* Mark as no data to prevent movement */
                            
                            printf("[ARM FALLBACK] Mouse data cleared to prevent phantom movement\n");
                        }
                    }
                }
                
                /* Return success - data has been processed */
                dev->poll_count++;
                return dev->data_length;
            } else {
                /* No data available - normal for most polling cycles */
            }
            
            /* Recreate queue if it becomes stale (every 30 seconds) */
            uint32_t current_time = get_system_time_ms();
            if (dev->int_queue && (current_time - dev->queue_create_time) > 30000) {
                printf("[ARM POLLING] Refreshing stale queue for dev=%u (age=%lums)\n", 
                       device_addr, current_time - dev->queue_create_time);
                       
                destroy_int_queue(dev->udev, dev->int_queue);
                dev->int_queue = NULL;
                /* Will be recreated on next call */
            }
        }
    }
    
    return 0;
}

/*
 * Initialize ARM USB polling system
 * This should be called after USB stack initialization
 */
void z3660_usb_init_polling(void)
{
    printf("[ARM Polling] Initializing ARM USB polling system\n");
    
    // Clear polling device table
    memset(polling_devices, 0, sizeof(polling_devices));
    polling_device_count = 0;
    
    printf("[ARM Polling] ARM USB polling system initialized (max devices: %d)\n", 
           MAX_POLLING_DEVICES);
}

/*
 * Shutdown ARM USB polling system
 */
void z3660_usb_shutdown_polling(void)
{
    printf("[ARM Polling] Shutting down ARM USB polling system\n");
    
    // Deactivate all polling devices
    for (int i = 0; i < polling_device_count; i++) {
        if (polling_devices[i].active) {
            printf("[ARM Polling] Deactivating device %u ep=%u\n",
                   polling_devices[i].device_addr, polling_devices[i].endpoint);
            polling_devices[i].active = 0;
        }
    }
    
    // Clear the table
    memset(polling_devices, 0, sizeof(polling_devices));
    polling_device_count = 0;
    
    printf("[ARM Polling] ARM USB polling system shutdown complete\n");
}

/*
 * Background task to process ASYNC USB interrupt requests
 * This is called periodically from other_tasks() to check for pending ASYNC requests
 * and complete them when polling data becomes available
 */
void z3660_usb_process_async_requests(struct pt *pt)
{
    static uint32_t task_counter = 0;
//    printf("[USB Handler] z3660_usb_process_async_requests start\n");
    /* Check if we have a pending ASYNC interrupt request */
    if (usb_state.status != USB_STATUS_BUSY || !async_int_request.pending) {
        return; /* No pending ASYNC request */
    }
    task_counter++;
    if(task_counter < 100) {
        return; // Run this check only every 100 calls to reduce overhead
    }
    
    /* Check if polling data is available for this device */
    if (async_int_request.dir != 0) { /* IN transfers only */
        int available = z3660_usb_polling_data_available(pt, async_int_request.dev_addr, async_int_request.ep & 0x0F);
//        printf("[USB Handler] z3660_usb_polling_data_available %d\n", available);
        if (available > 0) {
            int retrieved = z3660_usb_get_polling_data(async_int_request.dev_addr, async_int_request.ep & 0x0F, 
                                                     (void*)async_int_request.data_ptr, async_int_request.data_len);
            
            if (retrieved > 0) {
                /* SUCCESS! We have data - complete the ASYNC request */
//                printf("[USB Handler] ASYNC INT: Data available! Completing request with %d bytes\n", retrieved);
                
                /* Flush cache for data written to Amiga memory */
                Xil_DCacheFlushRange((INTPTR)async_int_request.data_ptr, (UINTPTR)retrieved);
                
                /* Update USB state */
                usb_state.data_length = (uint32_t)retrieved;
                usb_state.status = USB_STATUS_READY | USB_STATUS_COMPLETE;
                
                /* Clear pending request */
                async_int_request.pending = 0;
                
                /* CRITICAL: Signal Amiga that data is ready */
                amiga_interrupt_set(AMIGA_INTERRUPT_USB);
//                printf("[USB Handler] ASYNC INT: Signaled Amiga - USB interrupt activated\n");
                
                return;
            }
        }
    }
#if 0    
    /* Still no data available - check for timeout */
    uint32_t current_time = get_system_time_ms();
    if (current_time - async_int_request.request_time > 10000) { /* 10 second timeout */
        printf("[USB Handler] ASYNC INT: Request timed out after %lu ms\n", 
               current_time - async_int_request.request_time);
        
        /* Timeout - complete with error */
        usb_state.data_length = 0;
        usb_state.status = USB_STATUS_READY | USB_STATUS_ERROR;
        async_int_request.pending = 0;
        
        /* Signal Amiga even on timeout so it doesn't wait forever */
        printf("[USB Handler] TIMEOUT: About to signal Amiga with amiga_interrupt_set(AMIGA_INTERRUPT_USB)\n");
        amiga_interrupt_set(AMIGA_INTERRUPT_USB);
        printf("[USB Handler] TIMEOUT: Amiga interrupt signal sent - Amiga should wake up now\n");
    }
#endif
    /* Request is still pending - will be checked again next time */
}
#endif