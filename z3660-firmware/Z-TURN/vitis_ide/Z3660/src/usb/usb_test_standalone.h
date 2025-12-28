/*
 * USB OTG Test Standalone Header for Z3660
 * Declarations for USB testing functions
 */

#ifndef USB_TEST_STANDALONE_H
#define USB_TEST_STANDALONE_H

#include <stdint.h>

/*
 * Main USB test function - comprehensive test with mouse monitoring
 * Returns: 0 on success, -1 on failure
 */
int usb_test_standalone_main(void);

/*
 * USB Mouse test function - tests HID mouse functionality
 */
void usb_mouse_test(void);

/*
 * USB HID diagnostic function - detailed analysis of HID devices
 */
int usb_hid_diagnostic(void);

/*
 * Improved USB Mouse test function - tests with proper HID protocols and alignment
 */
void usb_mouse_test_improved(int reportid);

#endif /* USB_TEST_STANDALONE_H */
