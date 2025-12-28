/*
 * Z3660 USB Device Driver - ARM Communication Header
 * 
 * Defines the interface between Amiga USB device driver and ARM USB stack
 */

#ifndef Z3660_USB_ARM_H
#define Z3660_USB_ARM_H

#include "z3660_usb_device.h"

/* USB operation codes (must match ARM side) */
#define USB_OP_INIT_STACK       0x1001
#define USB_OP_SHUTDOWN_STACK   0x1002
#define USB_OP_CONTROL_XFER     0x1005
#define USB_OP_BULK_XFER        0x1006
#define USB_OP_INT_XFER         0x1007
#define USB_OP_INT_XFER_ASYNC   0x1008  /* Async interrupt transfer - ARM signals when data ready */
#define USB_OP_ISO_XFER         0x1009  /* Moved to avoid conflict */
#define USB_OP_RESET_PORT       0x1010
#define USB_OP_QUERY_DEVICE     0x1020
#define USB_OP_ENUMERATE_DEVS   0x1030
#define USB_OP_GET_DEVICE_INFO  0x1031
#define USB_OP_PORT_STATUS      0x1040
#define USB_OP_PORT_CONTROL     0x1041

/* USB status flags (must match ARM side) */
#define USB_STATUS_READY        0x0001
#define USB_STATUS_BUSY         0x0002
#define USB_STATUS_ERROR        0x0004
#define USB_STATUS_TIMEOUT      0x0008
#define USB_STATUS_COMPLETE     0x0010

/* ARM communication functions */

/*
 * Initialize communication with ARM USB stack
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_init(struct Z3660USBBase *base);

/*
 * Shutdown communication with ARM USB stack
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_shutdown(struct Z3660USBBase *base);

/*
 * Send a control transfer to ARM
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_control_transfer(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq);

/*
 * Send a bulk transfer to ARM
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_bulk_transfer(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq);

/*
 * Send an interrupt transfer to ARM (DEPRECATED - use async version)
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_interrupt_transfer(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq);

/*
 * Start an asynchronous interrupt transfer - ARM will signal when data is ready
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_interrupt_transfer_async(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq);

/*
 * Complete an asynchronous interrupt transfer - get the actual data
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_interrupt_transfer_complete(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq);

/*
 * Send an isochronous transfer to ARM
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_iso_transfer(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq);

/*
 * Query device information from ARM
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_query_device(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq);

/*
 * Reset USB port via ARM
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_reset_port(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq);

/*
 * Get port status from ARM
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_port_status(struct Z3660USBUnit *unit, UWORD port, UWORD *wPortStatus, UWORD *wPortChange);

/*
 * Send port control command to ARM
 * Returns: RC_OK on success, error code on failure
 */
LONG z3660_usb_arm_port_control(struct Z3660USBUnit *unit, UWORD port, UWORD feature, BOOL set);

/*
 * Check if ARM USB stack is ready
 * Returns: TRUE if ready, FALSE if not ready
 */
BOOL z3660_usb_arm_is_ready(struct Z3660USBBase *base);

/*
 * Get ARM USB status
 * Returns: Current ARM status flags
 */
ULONG z3660_usb_arm_get_status(struct Z3660USBBase *base);

#endif /* Z3660_USB_ARM_H */
