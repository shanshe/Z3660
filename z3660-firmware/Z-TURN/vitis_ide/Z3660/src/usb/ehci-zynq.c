// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014, Xilinx, Inc
 *
 * USB Low level initialization(Specific to zynq)
 *
 * ZZ9000 modifications (ULPI FS4LS XCVR mode for low-speed root-port
 * support, SICTRL / CONTROL register setup for ChipIdea TDI).
 *
 * Copyright (C) 2026 Dimitris Panokostas <midwan@gmail.com>
 */

#include "usb.h"
#include "ehci-ci.h"
#include "ulpi.h"
#include "ehci.h"
#include <stdio.h>
#include <xusbps_hw.h>
#include <sleep.h>
#include <xparameters.h>

#define EINVAL 1

// see zynq TRM
#define USB_BASE_ADDR XPS_USB0_BASEADDR

int ehci_hcd_init(int index, enum usb_init_type init,
      struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
   (void)init;
   if (index > 0) {
      printf("[ehci_hcd_init] index out of range: %d\n",index);
      return -EINVAL;
   }

   struct usb_ehci *ehci = (struct usb_ehci *)(USB_BASE_ADDR);

   if (hccr && hcor) {
      *hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
      *hcor = (struct ehci_hcor *)((uint32_t)*hccr + HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
   }

   return 0;
}

int ehci_zynq_probe(struct zynq_ehci_priv *priv)
{
   struct ehci_hccr *hccr;
   struct ehci_hcor *hcor;
   struct ulpi_viewport ulpi_vp;
   /* Used for writing the ULPI data address */
   struct ulpi_regs *ulpi = (struct ulpi_regs *)0;
   int ret;

   struct usb_ehci *ehci = (struct usb_ehci *)(USB_BASE_ADDR);
   priv->ehci = ehci;

	// USB hardware reset is not needed - PS_501_RESET_OUTn signal is shared
	// between all devices and reset was already performed during system init.
	// Calling XUsbPs_ResetHw() here interferes with Ethernet functionality.
	XUsbPs_ResetHw(USB_BASE_ADDR);

   hccr = (struct ehci_hccr *)((uint32_t)&priv->ehci->caplength);
   hcor = (struct ehci_hcor *)((uint32_t) hccr + HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

   //printf("[ehci-zynq] hccr: %p hcor: %p\n", hccr, hcor);

   priv->ehcictrl.hccr = hccr;
   priv->ehcictrl.hcor = hcor;

   ulpi_vp.viewport_addr = (u32)&priv->ehci->ulpi_viewpoint;
   ulpi_vp.port_num = 0;

   //printf("[ehci-zynq] viewport_addr: %p\n", &priv->ehci->ulpi_viewpoint);
// lifted from https://elixir.bootlin.com/u-boot/latest/source/drivers/usb/host/ehci-fsl.c#L275
/* Set to Host mode + Stream Disable.
 *
 * SDIS (bit 4 on Zynq PS USB) disables the EHCI controller's
 * streaming/prefetch mode. Streaming mode expects AXI reads/writes
 * to keep up with the USB transfer rate; on a Zorro III card where
 * AXI is shared with the RTG graphics engine, AXI bandwidth drops
 * during frame updates and streaming reads hit qTD data-buffer
 * errors (status 0x20) on HS bulk IN — exactly what we saw on the
 * SanDisk USB stick. With SDIS set, the EHCI prefills its FIFO
 * fully before starting the transaction, trading a small amount of
 * throughput for reliability across the AXI bus.
 *
 * This is the standard Zynq / FSL / ChipIdea EHCI workaround;
 * Xilinx's XUsbPs driver also recommends SDIS for host mode.
 */
   setbits_le32(&ehci->usbmode, CM_HOST | SDIS);

// Enable controller and select ULPI interface in CONTROL register
// bit 10: ULPI_SEL, bit 2: USB_EN
   setbits_le32(&ehci->portsc, USB_EN);
   usleep(1000); /* delay required for PHY Clk to appear */
   out_le32(&(hcor)->or_portsc[0], PORT_PTS_ULPI);
//   out_le32(&ehci->control, PHY_CLK_SEL_ULPI);

//   out_le32(&ehci->prictrl, 0x0000000c);
//   out_le32(&ehci->age_cnt_limit, 0x00000040);

/*
 * AXI burst-size tuning. EHCI DMA shares the AXI bus with the RTG
 * graphics engine; when graphics is busy, AXI arbitration can stall
 * small bursts and trigger EHCI Data Buffer Errors on HS bulk
 * transfers. Raising burst size amortises arbitration overhead:
 *   TXPBURST = 16 DWORDs (64 bytes per AXI burst)
 *   RXPBURST = 16 DWORDs (64 bytes per AXI burst)
 * This is the Xilinx-recommended default for Zynq PS USB; higher
 * values (32/64 DWORDs) may be explored if bulk throughput is still
 * contested by graphics.
 */
   out_le32(&ehci->burstsize, 0x00001010);

   // SICTRL bit 0 (SITP) should be 0 for ULPI
//   out_le32(&ehci->sictrl, 0);

   // Select ULPI interface in PORTSC
   clrsetbits_le32(&ehci->portsc, PORT_PTS_MSK, PORT_PTS_ULPI);

   usleep(10000); /* delay required for PHY Clk to appear */

//   in_le32(&ehci->usbmode);

   /* ULPI set flags */

   ret = ulpi_init(&ulpi_vp);
   if (ret) {
      puts("zynq ULPI viewport init failed\n");
      return -1;
   }
   else {
      printf("zynq ULPI viewport init OK!!!\n");
   }

// dp and dm pulldown = host mode
// extvbusind = vbus indicator input
   ulpi_write(&ulpi_vp, &ulpi->otg_ctrl,
      ULPI_OTG_DP_PULLDOWN | ULPI_OTG_DM_PULLDOWN |
      ULPI_OTG_EXTVBUSIND);

/*
 * Start the ULPI PHY in HS (XCVR_SELECT = 00) mode so high-speed
 * devices (mass storage, hubs) get full 480 Mbit/s throughput.
 *
 * Earlier revisions hard-coded FS4LS as a workaround for LS devices
 * wedging the reset path: PHY in HS mode wouldn't complete LS
 * chirp negotiation, PR stayed latched, and the HC hung.
 *
 * The fix is dynamic mode switching (see ehci_zynq_set_phy_mode_ls
 * below), called by usb_proxy's reset handler when it detects a
 * low-speed device by line state. HS is the default resting state;
 * we only dip into FS4LS for LS attach, then switch back.
 */
   ulpi_write(&ulpi_vp, &ulpi->function_ctrl,
      ULPI_FC_HIGH_SPEED | ULPI_FC_OPMODE_NORMAL |
      ULPI_FC_SUSPENDM);

   ulpi_write(&ulpi_vp, &ulpi->iface_ctrl, 0);

/* drive external vbus switch */
   ulpi_write(&ulpi_vp, &ulpi->otg_ctrl_set,
      ULPI_OTG_DRVVBUS | ULPI_OTG_DRVVBUS_EXT);

   usleep(10000);


   // FIXME removing this made it work! probably because there is another ehci_reset in there?
   //return ehci_register(&priv->ehcictrl, hccr, hcor, NULL, 0, USB_INIT_HOST);
   return 0;
}

/*
 * Runtime ULPI transceiver-select switch. Called from the reset
 * handler (usb_proxy.c handle_reset_port) right after it detects
 * the attached device's line state, BEFORE it asserts port reset.
 *
 *   for_low_speed = 1  -> ULPI_FC_FS4LS (LS/FS composite mode)
 *   for_low_speed = 0  -> ULPI_FC_HIGH_SPEED (HS mode)
 *
 * This lets us keep the PHY in HS mode by default (so HS devices
 * get native 480 Mbit/s throughput) while temporarily dropping to
 * FS4LS for LS attachments, which won't complete chirp negotiation
 * in HS mode on this ChipIdea controller.
 */
int ehci_zynq_set_phy_mode(int for_low_speed)
{
   struct usb_ehci *ehci = (struct usb_ehci *)USB_BASE_ADDR;
   struct ulpi_viewport ulpi_vp;
   struct ulpi_regs *ulpi = (struct ulpi_regs *)0;
   unsigned xcvr;

   ulpi_vp.viewport_addr = (u32)&ehci->ulpi_viewpoint;
   ulpi_vp.port_num = 0;

   xcvr = for_low_speed ? ULPI_FC_FS4LS : ULPI_FC_HIGH_SPEED;

   return ulpi_write(&ulpi_vp, &ulpi->function_ctrl,
                     xcvr | ULPI_FC_OPMODE_NORMAL | ULPI_FC_SUSPENDM);
}
