/*
 * Z3660 USB Device Driver - ARM Communication Implementation
 * 
 * Implementation of communication between Amiga USB device driver and ARM USB stack
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <exec/devices.h>
#include <exec/errors.h>
#include <libraries/expansion.h>

#include "z3660_usb_device.h"
#include "z3660_usb_arm.h"

/*
 * Send command to ARM and wait for completion
 * Returns: RC_OK on success, error code on failure
 */
static LONG arm_send_command_and_wait(struct Z3660USBBase *base, ULONG command, ULONG timeout_ms)
{
    if (!base || !base->z3660_regs) {
        kprintf_good("[ARM] ERROR: Invalid base or z3660_regs\n");
        return IOERR_OPENFAIL;
    }

    volatile ULONG *reg_cmd_op = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_CMD_OP);
    volatile ULONG *reg_status = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_STATUS);
    
    /* Send command */
    *reg_cmd_op = command;
    
    /* Wait for completion with timeout */
//    ULONG start_time = 0; /* TODO: Get actual system time */
    ULONG elapsed = 0;
    
    while (elapsed < timeout_ms) {
        ULONG status = *reg_status;
        
        /* Check if operation is complete */
        if (status & USB_STATUS_COMPLETE) {
            if (status & USB_STATUS_ERROR) {
                kprintf_good("[ARM] Command 0x%08lx completed with error, status=0x%08lx\n", command, status);
                return IOERR_BADADDRESS;
            }
            /* Success - reduced logging */
            return RC_OK;
        }
        
        /* Small delay to avoid hammering the register */
        /* TODO: Use proper delay function */
        for (ULONG i = 0; i < 1000; i++) {
            /* Busy wait */
        }
        
        elapsed++; /* Rough approximation */
    }
    
    kprintf_good("[ARM] Command 0x%08lx timed out after %ld ms\n", command, timeout_ms);
    return IOERR_OPENFAIL;
}

/*
 * Set parameter registers for ARM communication
 */
static void arm_set_parameters(struct Z3660USBBase *base, ULONG param0, ULONG param1, ULONG param2, ULONG param3)
{
    if (!base || !base->z3660_regs) return;
    
    volatile ULONG *reg_param0 = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_PARAM0);
    volatile ULONG *reg_param1 = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_PARAM1);
    volatile ULONG *reg_param2 = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_PARAM2);
    volatile ULONG *reg_param3 = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_PARAM3);
    
    *reg_param0 = param0;
    *reg_param1 = param1;
    *reg_param2 = param2;
    *reg_param3 = param3;
    
    /* Parameter logging disabled to reduce spam */
}

/*
 * Get data length from ARM response
 */
static ULONG arm_get_data_length(struct Z3660USBBase *base)
{
    if (!base || !base->z3660_regs) return 0;
    
    /* Use READ0 register for data length/count */
    volatile ULONG *reg_read0 = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_READ0);
    return *reg_read0;
}

/*
 * Initialize communication with ARM USB stack
 */
LONG z3660_usb_arm_init(struct Z3660USBBase *base)
{
    kprintf_good("[ARM] Initializing ARM USB stack\n");
    
    if (!base) {
        kprintf_good("[ARM] ERROR: Invalid base pointer\n");
        return IOERR_OPENFAIL;
    }
    
    /* Send initialization command to ARM */
    LONG result = arm_send_command_and_wait(base, USB_OP_INIT_STACK, 5000);
    if (result != RC_OK) {
        kprintf_good("[ARM] ERROR: Failed to initialize ARM USB stack\n");
        return result;
    }
    
    kprintf_good("[ARM] ARM USB stack initialized successfully\n");
    return RC_OK;
}

/*
 * Shutdown communication with ARM USB stack
 */
LONG z3660_usb_arm_shutdown(struct Z3660USBBase *base)
{
    kprintf_good("[ARM] Shutting down ARM USB stack\n");
    
    if (!base) {
        kprintf_good("[ARM] ERROR: Invalid base pointer\n");
        return IOERR_OPENFAIL;
    }
    
    /* Send shutdown command to ARM */
    LONG result = arm_send_command_and_wait(base, USB_OP_SHUTDOWN_STACK, 2000);
    if (result != RC_OK) {
        kprintf_good("[ARM] ERROR: Failed to shutdown ARM USB stack\n");
        return result;
    }
    
    kprintf_good("[ARM] ARM USB stack shutdown successfully\n");
    return RC_OK;
}

/*
 * Send a control transfer to ARM
 */
LONG z3660_usb_arm_control_transfer(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq)
{
    if (!unit || !unit->base || !ioreq) {
        kprintf_good("[ARM] ERROR: Invalid parameters for control transfer\n");
        return IOERR_BADADDRESS;
    }
    
    struct Z3660USBBase *base = unit->base;
    
    /* Set up parameters for ARM - match ARM's bit layout expectations */
    ULONG param0 = (ioreq->iouh_DevAddr << 24) | (ioreq->iouh_Endpoint << 16) | (ioreq->iouh_Dir << 8);
    ULONG param1 = (ULONG)&ioreq->iouh_SetupData; /* Setup packet pointer */
    ULONG param2 = (ULONG)ioreq->iouh_Data;      /* Data buffer pointer */
    ULONG param3 = ioreq->iouh_Length;           /* Data length */
    
    /* Cache coherency: Always prepare setup data for ARM to read */
    {
        ULONG setup_len = sizeof(ioreq->iouh_SetupData);
        CachePreDMA((APTR)&ioreq->iouh_SetupData, &setup_len, 0);
    }
    
    /* For transfers with data buffer, prepare based on direction */
    if (ioreq->iouh_Data && ioreq->iouh_Length > 0) {
        if (ioreq->iouh_Dir == UHDIR_OUT) {
            /* OUT: ARM reads from our buffer - prepare for DMA read */
            ULONG data_len = ioreq->iouh_Length;
            CachePreDMA(ioreq->iouh_Data, &data_len, 0);
        }
        /* IN transfers: Cache will be invalidated after ARM writes data */
    }
    
    arm_set_parameters(base, param0, param1, param2, param3);
    
    /* Send control transfer command */
    LONG result = arm_send_command_and_wait(base, USB_OP_CONTROL_XFER, 5000);
    if (result != RC_OK) {
        kprintf_good("[ARM] ERROR: Control transfer failed\n");
        ioreq->iouh_Actual = 0;
        return result;
    }
    
    /* Cache coherency: Invalidate data buffer to read ARM's response */
    if (ioreq->iouh_Data && ioreq->iouh_Dir == UHDIR_IN && ioreq->iouh_Length > 0) {
        /* Get actual length first */
        ioreq->iouh_Actual = arm_get_data_length(base);
        
        /* Then invalidate cache for the actual transferred data */
        if (ioreq->iouh_Actual > 0) {
            ULONG actual_len = ioreq->iouh_Actual;
            /* Invalidate cache so we read fresh data written by ARM */
            CachePostDMA(ioreq->iouh_Data, &actual_len, 0);
        }
    } else {
        /* For non-IN transfers, just get the length */
        ioreq->iouh_Actual = arm_get_data_length(base);
    }
    
    return RC_OK;
}

/*
 * Send a bulk transfer to ARM
 */
LONG z3660_usb_arm_bulk_transfer(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq)
{
    if (!unit || !unit->base || !ioreq) {
        kprintf_good("[ARM] ERROR: Invalid parameters for bulk transfer\n");
        return IOERR_BADADDRESS;
    }
    
    struct Z3660USBBase *base = unit->base;
    
    kprintf_good("[ARM] Bulk transfer: dev=%ld ep=%ld len=%ld\n", 
            ioreq->iouh_DevAddr, ioreq->iouh_Endpoint, ioreq->iouh_Length);
    
    /* Set up parameters for ARM - match ARM's bit layout expectations */
    ULONG param0 = (ioreq->iouh_DevAddr << 24) | (ioreq->iouh_Endpoint << 16) | (ioreq->iouh_Dir << 8);
    ULONG param1 = 0; /* No setup data for bulk transfers */
    ULONG param2 = (ULONG)ioreq->iouh_Data;      /* Data buffer pointer */
    ULONG param3 = ioreq->iouh_Length;           /* Data length */
    
    arm_set_parameters(base, param0, param1, param2, param3);
    
    /* Send bulk transfer command */
    LONG result = arm_send_command_and_wait(base, USB_OP_BULK_XFER, 5000);
    if (result != RC_OK) {
        kprintf_good("[ARM] ERROR: Bulk transfer failed\n");
        ioreq->iouh_Actual = 0;
        return result;
    }
    
    /* Get actual transfer length */
    ioreq->iouh_Actual = arm_get_data_length(base);
    
    kprintf_good("[ARM] Bulk transfer completed: actual=%ld\n", ioreq->iouh_Actual);
    return RC_OK;
}

/*
 * Send an interrupt transfer to ARM (DEPRECATED - use async version)
 */
LONG z3660_usb_arm_interrupt_transfer(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq)
{
    if (!unit || !unit->base || !ioreq) {
        kprintf_good("[ARM] ERROR: Invalid parameters for interrupt transfer\n");
        return IOERR_BADADDRESS;
    }
    
    struct Z3660USBBase *base = unit->base;
    
    /* Set up parameters for ARM - match ARM's bit layout expectations */
    ULONG param0 = (ioreq->iouh_DevAddr << 24) | (ioreq->iouh_Endpoint << 16) | (ioreq->iouh_Dir << 8);
    ULONG param1 = 0; /* No setup data for interrupt transfers */
    ULONG param2 = (ULONG)ioreq->iouh_Data;      /* Data buffer pointer */
    ULONG param3 = ioreq->iouh_Length;           /* Data length */
    
    arm_set_parameters(base, param0, param1, param2, param3);
    
    /* Send interrupt transfer command */
    LONG result = arm_send_command_and_wait(base, USB_OP_INT_XFER, 5000);
    if (result != RC_OK) {
        kprintf_good("[ARM] ERROR: Interrupt transfer failed\n");
        ioreq->iouh_Actual = 0;
        return result;
    }
    
    /* Get actual transfer length */
    ioreq->iouh_Actual = arm_get_data_length(base);
    return RC_OK;
}

/*
 * Start an asynchronous interrupt transfer - ARM will signal when data is ready
 * This implements the correct USB behavior: don't complete until data is available
 */
LONG z3660_usb_arm_interrupt_transfer_async(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq)
{
    if (!unit || !unit->base || !ioreq) {
        kprintf_good("[ARM] ERROR: Invalid parameters for async interrupt transfer\n");
        return IOERR_BADADDRESS;
    }
    
    struct Z3660USBBase *base = unit->base;
    
//    kprintf_good("[ARM] Starting async interrupt transfer: dev=%u ep=%u len=%u\n",
//                ioreq->iouh_DevAddr, ioreq->iouh_Endpoint, ioreq->iouh_Length);
    
    /* Cache coherency: Prepare data buffer if it's an OUT transfer */
    if (ioreq->iouh_Data && ioreq->iouh_Length > 0 && ioreq->iouh_Dir == UHDIR_OUT) {
        ULONG data_len = ioreq->iouh_Length;
        CachePreDMA(ioreq->iouh_Data, &data_len, 0);
    }
    
    /* Set up parameters for ARM - match ARM's bit layout expectations */
    ULONG param0 = (ioreq->iouh_DevAddr << 24) | (ioreq->iouh_Endpoint << 16) | (ioreq->iouh_Dir << 8);
    ULONG param1 = ((ULONG)FindTask(NULL) << 8) | unit->arm_int_signal;  /* Task pointer + signal number */
    ULONG param2 = (ULONG)ioreq->iouh_Data;      /* Data buffer pointer */
    ULONG param3 = ioreq->iouh_Length;           /* Data length */
    
    arm_set_parameters(base, param0, param1, param2, param3);
    
    /* CRITICAL: Send async interrupt transfer command to ARM
     * ARM should NOT respond immediately. Instead, it should:
     * 1. Queue this request internally
     * 2. Wait for real USB interrupt from hardware
     * 3. Only when USB device has data -> Signal the Amiga task
     * 4. Amiga wakes up from Wait() and completes the request
     */
    volatile ULONG *reg_cmd_op = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_CMD_OP);
    *reg_cmd_op = USB_OP_INT_XFER_ASYNC; /* Use async command - ARM queues and waits */
    
//    kprintf_good("[ARM] Async interrupt transfer queued, ARM will signal task when data ready\n");
    return RC_OK;  /* Return immediately - ARM will signal later */
}

/*
 * Complete an asynchronous interrupt transfer - get the actual data  
 * This is called after Wait() receives the signal from ARM
 */
LONG z3660_usb_arm_interrupt_transfer_complete(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq)
{
    if (!unit || !unit->base || !ioreq) {
        kprintf_good("[ARM] ERROR: Invalid parameters for interrupt transfer completion\n");
        return IOERR_BADADDRESS;
    }
    
    struct Z3660USBBase *base = unit->base;
    
    /* ARM has already signaled us that data is ready, so we don't need to poll */
//    kprintf_good("[ARM] Getting completed interrupt transfer data from ARM\n");
    
    /* Check ARM status (should be COMPLETE since ARM signaled us) */
    volatile ULONG *reg_status = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_STATUS);
    ULONG status = *reg_status;
    
    if (status & USB_STATUS_ERROR) {
        kprintf_good("[ARM] ERROR: Interrupt transfer completed with error, status=0x%08lx\n", status);
        ioreq->iouh_Actual = 0;
        return IOERR_BADADDRESS;
    }
    
    if (!(status & USB_STATUS_COMPLETE)) {
        kprintf_good("[ARM] WARNING: ARM signaled but status not complete (0x%08lx), continuing anyway\n", status);
    }
    
    /* Get actual transfer length from ARM */
    ioreq->iouh_Actual = arm_get_data_length(base);
    
//    kprintf_good("[ARM] ARM reports %lu bytes available\n", ioreq->iouh_Actual);
    
    /* Cache coherency: Invalidate data buffer to read ARM's response */
    if (ioreq->iouh_Data && ioreq->iouh_Actual > 0 && ioreq->iouh_Dir == UHDIR_IN) {
        ULONG actual_len = ioreq->iouh_Actual;
        /* Invalidate cache so we read fresh data written by ARM */
        CachePostDMA(ioreq->iouh_Data, &actual_len, 0);
        
//        kprintf_good("[ARM] Cache invalidated for %lu bytes of IN data\n", actual_len);
    }
    
    /* Clear the ARM status to prepare for next transfer */
    volatile ULONG *reg_cmd_op = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_CMD_OP);
    *reg_cmd_op = 0;  /* Clear command register */
    
//    kprintf_good("[ARM] Async interrupt transfer completed successfully: actual=%lu\n", ioreq->iouh_Actual);
    return RC_OK;
}

/*
 * Send an isochronous transfer to ARM
 */
LONG z3660_usb_arm_iso_transfer(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq)
{
    if (!unit || !unit->base || !ioreq) {
        kprintf_good("[ARM] ERROR: Invalid parameters for isochronous transfer\n");
        return IOERR_BADADDRESS;
    }
    
    /* For now, return not supported */
    kprintf_good("[ARM] Isochronous transfers not yet implemented\n");
    ioreq->iouh_Actual = 0;
    return IOERR_NOCMD;
}

/*
 * Query device information from ARM
 */
LONG z3660_usb_arm_query_device(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq)
{
    if (!unit || !unit->base || !ioreq) {
        kprintf_good("[ARM] ERROR: Invalid parameters for query device\n");
        return IOERR_BADADDRESS;
    }
    
    kprintf_good("[ARM] Query device not yet implemented\n");
    return RC_OK;
}

/*
 * Reset USB port via ARM
 */
LONG z3660_usb_arm_reset_port(struct Z3660USBUnit *unit, struct IOUsbHWReq *ioreq)
{
    if (!unit || !unit->base) {
        kprintf_good("[ARM] ERROR: Invalid parameters for port reset\n");
        return IOERR_BADADDRESS;
    }
    
    struct Z3660USBBase *base = unit->base;
    
    kprintf_good("[ARM] Resetting USB port\n");
    
    /* Set up parameters for ARM */
    arm_set_parameters(base, 1, 4, 1, 0); /* Port 1, feature PORT_RESET, set=TRUE */
    
    /* Send port control command */
    LONG result = arm_send_command_and_wait(base, USB_OP_PORT_CONTROL, 3000);
    if (result != RC_OK) {
        kprintf_good("[ARM] ERROR: Port reset failed\n");
        return result;
    }
    
    kprintf_good("[ARM] Port reset completed successfully\n");
    return RC_OK;
}

/*
 * Get port status from ARM
 */
LONG z3660_usb_arm_port_status(struct Z3660USBUnit *unit, UWORD port, UWORD *wPortStatus, UWORD *wPortChange)
{
    if (!unit || !unit->base || !wPortStatus || !wPortChange) {
        kprintf_good("[ARM] ERROR: Invalid parameters for port status\n");
        return IOERR_BADADDRESS;
    }
    
    struct Z3660USBBase *base = unit->base;
    
    kprintf_good("[ARM] Getting port status for port %ld\n", (LONG)port);
    
    /* Set up parameters for ARM */
    arm_set_parameters(base, port, 0, 0, 0);
    
    /* Send port status command */
    LONG result = arm_send_command_and_wait(base, USB_OP_PORT_STATUS, 2000);
    if (result != RC_OK) {
        kprintf_good("[ARM] ERROR: Port status query failed\n");
        *wPortStatus = 0;
        *wPortChange = 0;
        return result;
    }
    
    /* For now, return default values - TODO: read from ARM response */
    *wPortStatus = 0x0103; /* Connected, enabled, high speed */
    *wPortChange = 0x000A; /* Connection and enable change */
    
    kprintf_good("[ARM] Port status: status=0x%04lx change=0x%04lx\n", 
            (LONG)*wPortStatus, (LONG)*wPortChange);
    
    return RC_OK;
}

/*
 * Send port control command to ARM
 */
LONG z3660_usb_arm_port_control(struct Z3660USBUnit *unit, UWORD port, UWORD feature, BOOL set)
{
    if (!unit || !unit->base) {
        kprintf_good("[ARM] ERROR: Invalid parameters for port control\n");
        return IOERR_BADADDRESS;
    }
    
    struct Z3660USBBase *base = unit->base;
    
    kprintf_good("[ARM] Port control: port=%ld feature=%ld set=%ld\n", 
            (LONG)port, (LONG)feature, (LONG)set);
    
    /* Set up parameters for ARM */
    arm_set_parameters(base, port, feature, set ? 1 : 0, 0);
    
    /* Send port control command */
    LONG result = arm_send_command_and_wait(base, USB_OP_PORT_CONTROL, 2000);
    if (result != RC_OK) {
        kprintf_good("[ARM] ERROR: Port control failed\n");
        return result;
    }
    
    kprintf_good("[ARM] Port control completed successfully\n");
    return RC_OK;
}

/*
 * Check if ARM USB stack is ready
 */
BOOL z3660_usb_arm_is_ready(struct Z3660USBBase *base)
{
    if (!base || !base->z3660_regs) {
        return FALSE;
    }
    
    volatile ULONG *reg_status = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_STATUS);
    ULONG status = *reg_status;
    
    return (status & USB_STATUS_READY) ? TRUE : FALSE;
}

/*
 * Get ARM USB status
 */
ULONG z3660_usb_arm_get_status(struct Z3660USBBase *base)
{
    if (!base || !base->z3660_regs) {
        return 0;
    }
    
    volatile ULONG *reg_status = (volatile ULONG *)((ULONG)base->z3660_regs + REG_ZZ_USB_STATUS);
    return *reg_status;
}
