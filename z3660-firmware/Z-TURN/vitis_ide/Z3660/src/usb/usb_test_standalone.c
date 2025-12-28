#include <stdio.h>
#include "usb.h"
#include "sleep.h"
#include "usb_test_standalone.h"
#include "xil_cache.h"

/*
 * Enhanced USB test function with detailed diagnostics and troubleshooting
 */
int zz_usb_init(void);
int main_example(void);

int usb_test_standalone_main(void)
{
    Xil_DCacheEnable();
    printf("\n");
    printf("=================================\n");
    printf("  Z3660 USB Standalone Test\n");
    printf("=================================\n");
    printf("\n");
    printf("[TEST] Initializing USB...\n");
    
    // Single initialization attempt
    
    printf("[USB INIT] Initializing USB controller...\n");
    zz_usb_init();
    
    // Brief stabilization delay
    printf("[USB INIT] Stabilizing USB PHY...\n");
    usleep(500000); // 500ms delay
    
    // USB initialization completed
    printf("[USB INIT] ✅ USB initialization completed\n");

    // Run HID diagnostic first to understand the device
    printf("\n=== Starting USB HID Diagnostic ===\n");
    int reportid = usb_hid_diagnostic();

    // Try advanced mouse activation techniques
    printf("\n=== Starting Improved USB Mouse Test ===\n");
    usb_mouse_test_improved(reportid);

    return 0;
}
