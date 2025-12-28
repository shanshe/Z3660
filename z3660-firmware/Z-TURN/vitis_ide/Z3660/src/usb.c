/*
 * MNT ZZ9000 Amiga Graphics and Coprocessor Card Operating System (ZZ9000OS)
 *
 * Copyright (C) 2019, Lukas F. Hartmann <lukas@mntre.com>
 *                     MNT Research GmbH, Berlin
 *                     https://mntre.com
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 *
 * Some portions (from example code, weird custom license) Copyright (C) 2010 - 2015 Xilinx, Inc.
 *
*/

#include <stdio.h>
#include "platform.h"
#include <xil_printf.h>
#include <xil_cache.h>
#include <xil_mmu.h>
#include "sleep.h"
#include "xparameters.h"
#include <xscugic.h>
#include "usb/ehci.h"
#include "usb/usb.h"

#include "ethernet.h"


#include "xil_cache.h"
#include <xusbps.h>
#include "interrupt.h"

// Forward declaration for EHCI interrupt handler
int ehci_intr_handler(struct ehci_ctrl *ehci_ctrl);

struct zynq_ehci_priv _zynq_ehci;
extern struct ehci_ctrl ehcic[];

/*
 * https://www.cypress.com/file/134171/download
 * https://elixir.bootlin.com/u-boot/latest/source/common/usb_storage.c#L653
 *
 * "Enumeration directly follows the device detection, and is the process
of assigning a unique address to a newly attached device. Configuration is the process of determining a device's
capabilities by an exchange of device requests. The requests that the host uses to learn about a device are called
standard requests and must support these requests on all USB devices."
 */
#define USB_USBCMD              0x140   // USB Command Register
#define USB_USBSTS              0x144   // USB Status Register (Assumed offset based on common Xilinx Zynq USB register maps)
#define USBCMD_CTRL_RESET       (1 << 1)
static inline uint32_t usb_read_reg(uint32_t offset) {
    return *(volatile uint32_t*)(XPS_USB0_BASEADDR + offset);
}
static inline void usb_write_reg(uint32_t offset, uint32_t value) {
    *(volatile uint32_t*)(XPS_USB0_BASEADDR + offset) = value;
}
// returns 1 if USB system is working (regardless of storage devices)
int zz_usb_init(void) {
//   zynq_usb_clock_init();
//   usb_hardware_reset();
    printf("[USB OTG] Initializing USB PHY...\n");
    Xil_DCacheEnable();
    // Reset the USB controller first
    usb_write_reg(USB_USBCMD, USBCMD_CTRL_RESET);

    // Wait for reset to complete
    uint32_t timeout = 10000;
    while ((usb_read_reg(USB_USBCMD) & USBCMD_CTRL_RESET) && timeout--) {
        usleep(10);
    }

    if (!timeout) {
        printf("[USB OTG] PHY reset timeout!\n");
        return 0;
    }
    printf("[USB OTG] USB PHY initialized\n");

    printf("[USB] trying to probe zynq ehci...\n");
    if(ehci_zynq_probe(&_zynq_ehci)==0)
        printf("[USB] probed!\n");

    if(usb_init()==0)
    {
        printf("[USB] initialized!\n");
        
        // Enable XUsbPs interrupts
        printf("[USB] Enabling XUsbPs interrupts...\n");
        XUsbPs_WriteReg(XPS_USB0_BASEADDR, XUSBPS_IER_OFFSET, XUSBPS_IXR_ALL);
        
        // Verify XUsbPs interrupt enable register
        uint32_t ier_val = XUsbPs_ReadReg(XPS_USB0_BASEADDR, XUSBPS_IER_OFFSET);
        printf("[USB] XUsbPs IER register: 0x%08lx (expected: 0x%08lx)\n", 
               (unsigned long)ier_val, (unsigned long)XUSBPS_IXR_ALL);
        
        // Also enable EHCI interrupts if EHCI controller is available
        struct ehci_ctrl *ehci_ctrl = &ehcic[0];
        if (ehci_ctrl && ehci_ctrl->hcor) {
            printf("[USB] Enabling EHCI interrupts...\n");
            // Enable all relevant USB interrupts
            uint32_t ehci_intr_enable = 
                0x01 |  // USB Interrupt (Transaction Complete)
                0x02 |  // USB Error Interrupt
                0x04 |  // Port Change Detect
                0x08 |  // Frame List Rollover
                0x10 |  // Host System Error
                0x20;   // Interrupt on Async Advance
            
            // Clear any pending interrupts first
            ehci_writel(&ehci_ctrl->hcor->or_usbsts, 0x3F);
            
            // Now enable the interrupts
            ehci_writel(&ehci_ctrl->hcor->or_usbintr, ehci_intr_enable);
            
            uint32_t usbintr_val = ehci_readl(&ehci_ctrl->hcor->or_usbintr);
            uint32_t usbsts_val = ehci_readl(&ehci_ctrl->hcor->or_usbsts);
            printf("[USB] EHCI USBINTR=0x%08lx USBSTS=0x%08lx\n", 
                   (unsigned long)usbintr_val, (unsigned long)usbsts_val);
            
            // Force a Frame List Rollover interrupt to test interrupt path
            ehci_writel(&ehci_ctrl->hcor->or_usbsts, 0x08);
        } else {
            printf("[USB] WARNING: EHCI controller not available for interrupt setup\n");
        }

#if 0
        // Always scan for storage devices, but don't fail if none found
        if (!usb_stor_scan(1)) {
            printf("[USB] USB storage devices found\n");
        } else {
            printf("[USB] No USB storage devices found (this is normal)\n");
        }
        // USB system is working regardless of storage devices
#endif
        return 1;
    }

    printf("[USB] USB initialization failed\n");
    return 0;
}
#if 0
unsigned long zz_usb_read_blocks(int dev_index, unsigned long blknr, unsigned long blkcnt, void *buffer) {
    int res = usb_stor_read_direct(dev_index, blknr, blkcnt, buffer);
    //printf("[USB] RD %lu# at %lu: ",blkcnt,blknr);
    /*for (int i=0;i<8*4;i++) {
            printf("%02x", ((uint8_t*)buffer)[i]);
    }*/
    return res;
}

unsigned long zz_usb_write_blocks(int dev_index, unsigned long blknr, unsigned long blkcnt, void *buffer) {
    int res = usb_stor_write_direct(dev_index, blknr, blkcnt, buffer);
    //printf("[USB] WR %lu# at %lu: [%08lx %08lx]\n",blkcnt,blknr,((uint32_t*)buffer)[0],((uint32_t*)buffer)[1]);
    return res;
}

unsigned long zz_usb_storage_capacity(int dev_index) {
    unsigned long cap = usb_stor_get_capacity(dev_index);
    printf("[USB] get capacity: %lx\n",cap);
    return cap;
}
#endif

void isr_usb(void *dummy) {
    (void)dummy;
    static bool hse_recovery_active = false;
    
    static int isr_call_count = 0;
    static int auto_call_count = 0;
    static int manual_call_count = 0;
    isr_call_count++;
    
    // Determine if pending in GIC before reading/clearing EHCI
    extern XScuGic int_handler;
    const int usb_interrupt_id = INT_INTERRUPT_ID_2; // Use define from interrupt.h
    uint32_t pending_reg_offset = XSCUGIC_PENDING_SET_OFFSET + (usb_interrupt_id / 32) * 4; // GICD_ISPENDR
    uint32_t dist_pending = XScuGic_DistReadReg(&int_handler, pending_reg_offset);
    uint32_t bit_mask = 1U << (usb_interrupt_id % 32);
    int is_pending_in_gic = (dist_pending & bit_mask) ? 1 : 0;
    
    if (is_pending_in_gic) {
        auto_call_count++;
        USB_DEBUG("USB INTERRUPT #%d (AUTOMATIC #%d) - Pending in GIC\n", 
               isr_call_count, auto_call_count);
    } else {
        manual_call_count++;
        USB_DEBUG("USB INTERRUPT #%d (MANUAL/MISSED #%d) - Polled path\n", 
               isr_call_count, manual_call_count);
    }

    /* Get EHCI controller state */
    struct ehci_ctrl *ehci_ctrl = &ehcic[0];
    if (!ehci_ctrl || !ehci_ctrl->hcor) {
        USB_DEBUG_ERROR("Invalid EHCI controller state\n");
        return;
    }

    /* Read registers before processing */
    uint32_t usbsts = ehci_readl(&ehci_ctrl->hcor->or_usbsts);
    uint32_t usbintr = ehci_readl(&ehci_ctrl->hcor->or_usbintr);
    uint32_t portsc = ehci_readl(&ehci_ctrl->hcor->or_portsc[0]);
    (void)portsc; // Silencia advertencia de variable no utilizada
    USB_DEBUG("USBSTS=0x%08lx USBINTR=0x%08lx PORTSC=0x%08lx (Pending in GIC: %s)\n",
           (unsigned long)usbsts, (unsigned long)usbintr, (unsigned long)portsc,
           is_pending_in_gic ? "YES" : "NO");

    /* Check which interrupt sources are active */
    if (usbsts & usbintr) {
        USB_DEBUG("Active interrupt sources: 0x%08lx\n", (unsigned long)(usbsts & usbintr));
        
        /* Check specific interrupt types */
        if (usbsts & 0x01) USB_DEBUG("[USB ISR] - USB Transaction Complete\n");
        if (usbsts & 0x02) USB_DEBUG("[USB ISR] - USB Error\n");
        if (usbsts & 0x04) USB_DEBUG("[USB ISR] - Port Change Detect\n");
        if (usbsts & 0x08) USB_DEBUG("[USB ISR] - Frame List Rollover\n");
        if (usbsts & 0x10) USB_DEBUG("[USB ISR] - Host System Error\n");
        if (usbsts & 0x20) USB_DEBUG("[USB ISR] - Interrupt on Async Advance\n");
    } else {
        printf("[USB ISR] WARNING: No active interrupt sources found!\n");
    }

    /* Call EHCI interrupt handler */
    int ret = ehci_intr_handler(ehci_ctrl);
    if (ret) {
        USB_DEBUG("[USB ISR] EHCI handler returned %d\n", ret);
    }

    /* Read USBSTS after handling to see what changed */
    uint32_t usbsts_after = ehci_readl(&ehci_ctrl->hcor->or_usbsts);
    USB_DEBUG("[USB ISR] After handling: USBSTS=0x%08lx (was 0x%08lx)\n", 
           (unsigned long)usbsts_after, (unsigned long)usbsts);

    // Check for Host System Error and recover if needed
    if ((usbsts & 0x10) && !hse_recovery_active) { // HSE bit set and not already recovering
        hse_recovery_active = true;
        printf("[USB ISR] HSE Recovery: Host System Error detected, attempting recovery...\n");
        
        // Re-initialize key registers
        usb_write_reg(USB_USBCMD, usb_read_reg(USB_USBCMD) | 0x1); // Set RUN bit
        usleep(100); // Let controller stabilize
        
        // Clear any error conditions
        usb_write_reg(USB_USBSTS, usb_read_reg(USB_USBSTS) | 0x3f);
        
        printf("[USB ISR] HSE Recovery: EHCI controller restarted\n");
        hse_recovery_active = false;
    }

    /* Clear both XUsbPs and EHCI pending interrupts */
    
    // Clear XUsbPs interrupts
    uint32_t xusbps_isr = XUsbPs_ReadReg(XPS_USB0_BASEADDR, XUSBPS_ISR_OFFSET);
    if (xusbps_isr) {
        USB_DEBUG("[USB ISR] Clearing XUsbPs interrupts: 0x%08lx\n", (unsigned long)xusbps_isr);
        XUsbPs_WriteReg(XPS_USB0_BASEADDR, XUSBPS_ISR_OFFSET, xusbps_isr);
    }
    
    // Clear any remaining EHCI interrupts
    uint32_t remaining_usbsts = ehci_readl(&ehci_ctrl->hcor->or_usbsts);
    if (remaining_usbsts & 0x3F) {
        printf("[USB ISR] Clearing remaining EHCI interrupts: 0x%08lx\n", (unsigned long)remaining_usbsts);
        ehci_writel(&ehci_ctrl->hcor->or_usbsts, remaining_usbsts & 0x3F);
    }
    
    // Verify everything is cleared
    uint32_t final_xusbps = XUsbPs_ReadReg(XPS_USB0_BASEADDR, XUSBPS_ISR_OFFSET);
    uint32_t final_usbsts = ehci_readl(&ehci_ctrl->hcor->or_usbsts);
    USB_DEBUG("[USB ISR] Final state - XUsbPs=0x%08lx EHCI=0x%08lx\n", 
           (unsigned long)final_xusbps, (unsigned long)final_usbsts);
    
    USB_DEBUG("[USB ISR] *** USB INTERRUPT #%d PROCESSING COMPLETE ***\n", isr_call_count);
}

int ehci_intr_handler(struct ehci_ctrl *ehci_ctrl) {
    if (!ehci_ctrl || !ehci_ctrl->hcor) {
        printf("[USB ISR] ERROR: Invalid EHCI controller in ehci_intr_handler\n");
        return -1;
    }
    
    /* Read current EHCI status */
    uint32_t usbsts = ehci_readl(&ehci_ctrl->hcor->or_usbsts);
    uint32_t usbintr = ehci_readl(&ehci_ctrl->hcor->or_usbintr);
    
    /* Only handle interrupts that are both pending and enabled */
    uint32_t pending_intrs = usbsts & usbintr;
    
    if (pending_intrs == 0) {
        printf("[USB ISR] No pending EHCI interrupts to handle\n");
        return 0;
    }
    
    USB_DEBUG("[USB ISR] Processing EHCI interrupts: 0x%08lx\n", (unsigned long)pending_intrs);
    
    /* Handle specific interrupt types */
    if (pending_intrs & 0x01) { // USB Transaction Complete
        USB_DEBUG("[USB ISR] Handling USB Transaction Complete interrupt\n");
        
        // Delegate HID completion processing to handler layer
        extern void z3660_usb_on_transaction_complete(void);
        z3660_usb_on_transaction_complete();
    }
    
    if (pending_intrs & 0x02) { // USB Error
        USB_DEBUG("[USB ISR] Handling USB Error interrupt\n");
        // USB error occurred - log but continue operation
    }
    
    if (pending_intrs & 0x04) { // Port Change Detect
        printf("[USB ISR] Handling Port Change Detect interrupt\n");
        // Port status changed - device connect/disconnect
        // TODO: Could trigger device re-enumeration if needed
    }
    
    if (pending_intrs & 0x08) { // Frame List Rollover
        USB_DEBUG("[USB ISR] Handling Frame List Rollover interrupt\n");
        // Frame list rollover - normal operation, just acknowledge
    }
    
    if (pending_intrs & 0x10) { // Host System Error
        printf("[USB ISR] Handling Host System Error interrupt\n");
        // Host system error - serious condition, but continue operation
    }
    
    if (pending_intrs & 0x20) { // Interrupt on Async Advance
        USB_DEBUG("[USB ISR] Handling Interrupt on Async Advance interrupt\n");
        // Async advance doorbell completed - acknowledge
    }
    
    /* CRITICAL: Clear the interrupt status bits by writing 1 to them */
    /* This is the W1C (Write-1-to-Clear) mechanism for EHCI USBSTS register */
    uint32_t clear_mask = pending_intrs & 0x3F; // Only clear bits 0-5 which are interrupt status bits
    ehci_writel(&ehci_ctrl->hcor->or_usbsts, clear_mask);
    
    /* Verify the bits were cleared */
    uint32_t usbsts_after = ehci_readl(&ehci_ctrl->hcor->or_usbsts);
    USB_DEBUG("[USB ISR] USBSTS before clear: 0x%08lx, after clear: 0x%08lx\n",
           (unsigned long)usbsts, (unsigned long)usbsts_after);
    
    /* Return the number of interrupts we handled */
    int handled_count = __builtin_popcount(pending_intrs);
    USB_DEBUG("[USB ISR] ehci_intr_handler completed - handled %d interrupt types\n", handled_count);
    
    return handled_count;
}
