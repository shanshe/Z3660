/*
 * Z3660 USB Device Handler for ARM side - Header file
 * 
 * Function prototypes for Z3660 USB device handling
 */

#ifndef Z3660_USB_HANDLER_H
#define Z3660_USB_HANDLER_H

#include <stdint.h>
#include "pt/pt.h"

/* Function prototypes */

/*
 * Initialize USB handler system
 */
void z3660_usb_handler_init(void);

/*
 * Handle USB register writes from Amiga
 */
void usb_handle_register_write(uint32_t reg, uint32_t value);

/*
 * Handle USB register reads to Amiga
 */
uint32_t usb_handle_register_read(uint32_t reg);

/*
 * Process USB commands from Amiga
 */
void z3660_usb_process_command(uint32_t command);

/*
 * Get current USB status for external queries
 */
uint32_t z3660_usb_get_status(void);

/*
 * Check if USB system is offline/shutdown
 */
int z3660_usb_is_offline(void);

/*
 * Process pending ASYNC USB interrupt requests
 * Should be called periodically from main loop to complete ASYNC requests when data is available
 */
void z3660_usb_process_async_requests(struct pt *pt);

#endif /* Z3660_USB_HANDLER_H */
