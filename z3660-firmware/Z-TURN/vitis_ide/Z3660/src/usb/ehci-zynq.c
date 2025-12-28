// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014, Xilinx, Inc
 *
 * USB Low level initialization(Specific to zynq)
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

/**
 * @brief  Initializes and configures the EHCI controller for USB on the Zynq platform.
 * @param  priv Pointer to the private EHCI controller structure.
 * @return Status code of the operation (0 if successful, negative on error).
 */
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
	// XUsbPs_ResetHw(USB_BASE_ADDR);
	printf("[ehci-zynq] Skipping USB hardware reset (already done by system reset)\n");

	hccr = (struct ehci_hccr *)((uint32_t)&priv->ehci->caplength);
	hcor = (struct ehci_hcor *)((uint32_t) hccr + HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	//printf("[ehci-zynq] hccr: %p hcor: %p\n", hccr, hcor);

	priv->ehcictrl.hccr = hccr;
	priv->ehcictrl.hcor = hcor;

	ulpi_vp.viewport_addr = (u32)&priv->ehci->ulpi_viewpoint;
	ulpi_vp.port_num = 0;

	printf("[ehci-zynq] viewport_addr: %p\n", &priv->ehci->ulpi_viewpoint);

	// lifted from https://elixir.bootlin.com/u-boot/latest/source/drivers/usb/host/ehci-fsl.c#L275
	/* Set to Host mode */
	setbits_le32(&ehci->usbmode, CM_HOST);

	// FIXME: need to figure out which of these are necessary

	setbits_le32(&ehci->portsc, USB_EN);
	usleep(1000); /* delay required for PHY Clk to appear */
	out_le32(&(hcor)->or_portsc[0], PORT_PTS_ULPI);

//	out_le32(&ehci->prictrl, 0x0000000c);
//	out_le32(&ehci->age_cnt_limit, 0x00000040);
//	out_le32(&ehci->sictrl, 0x00000001);

//	in_le32(&ehci->usbmode);

	ulpi_vp.port_num = 0;

	/* ULPI set flags */
	ret = ulpi_init(&ulpi_vp);
	if (ret) {
		puts("zynq ULPI viewport init failed\n");
		return -1;
	}
	else {
	   printf("zynq ULPI viewport init OK!!!\n");
	}

	/* ULPI set flags */
	ulpi_write(&ulpi_vp, &ulpi->otg_ctrl,
		   ULPI_OTG_DP_PULLDOWN | ULPI_OTG_DM_PULLDOWN |
		   ULPI_OTG_EXTVBUSIND);
	printf("[ehci-zynq] Configuring USB3320C for HIGH SPEED operation\n");
	ulpi_write(&ulpi_vp, &ulpi->function_ctrl,
		   ULPI_FC_HIGH_SPEED | ULPI_FC_OPMODE_NORMAL |
		   ULPI_FC_SUSPENDM);
	ulpi_write(&ulpi_vp, &ulpi->iface_ctrl, 0);

	/* Set VBus */
	ulpi_write(&ulpi_vp, &ulpi->otg_ctrl_set,
		   ULPI_OTG_DRVVBUS | ULPI_OTG_DRVVBUS_EXT);

	// FIXME: This original code was commented out but may be necessary
	return ehci_register(&priv->ehcictrl, hccr, hcor, NULL, 0, USB_INIT_HOST);
	// return 0;
}
