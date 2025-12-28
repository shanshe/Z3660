// SPDX-License-Identifier: GPL-2.0
/*-
 * Copyright (c) 2007-2008, Juniper Networks, Inc. */

#include <errno.h>
#include "asm/byteorder.h"
#include "usb.h"
#include "io.h"
#include <malloc.h>
#include <stdio.h>
//#include <watchdog.h>
#include <xil_cache.h>
#include <string.h>
#include <sleep.h>

#include "ehci.h"
#include "memalign.h"
#include "xtime_l.h"
#include "../pt/pt.h"


/* Tuning / debug controls */
#define EHCI_WRAP_REARM_DELAY_US 10
#define EHCI_WRAP_REARM_IOC      1

extern uint32_t counts_per_second;

void flush_dcache_range(unsigned long start, unsigned long stop) {
	Xil_DCacheFlushRange(start, stop-start);
}

void invalidate_dcache_range(unsigned long start, unsigned long stop) {
	Xil_DCacheInvalidateRange(start, stop-start);
}

uint32_t virt_to_phys(void* addr) {
	return (uint32_t)addr;
}

// FIXME
void udelay(int us) {
	usleep(us);
}
void mdelay(int ms) {
	usleep(1000*ms);
}

/* External function from main system */
void other_tasks(void);

/* Forward declarations needed before first use */
struct int_queue; /* defined later in this file */
static void *_ehci_poll_int_queue(struct usb_device *dev, struct int_queue *queue);
static int _ehci_submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int length);
static int _ehci_submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer, int length, struct devrequest *setup);
unsigned long get_timer(unsigned long base_time) {
	XTime current_time;
	XTime_GetTime(&current_time);
	/* Convert from ticks to milliseconds */
	unsigned long current_ms = (unsigned long)(current_time / (counts_per_second / 1000));
	return current_ms - base_time;
}

#ifndef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#endif

/*
 * EHCI spec page 20 says that the HC may take up to 16 uFrames (= 4ms) to halt.
 * Let's time out after 8 to have a little safety margin on top of that.
 */
#define HCHALT_TIMEOUT (8 * 1000)

struct ehci_ctrl ehcic[CONFIG_USB_MAX_CONTROLLER_COUNT];

/* Simple single-context USB management - no protothreads */
#define USB_SIMPLE_TIMEOUT_MS 100

static struct descriptor {
	struct usb_hub_descriptor hub;
	struct usb_device_descriptor device;
	struct usb_linux_config_descriptor config;
	struct usb_linux_interface_descriptor interface;
	struct usb_endpoint_descriptor endpoint;
}  __attribute__ ((packed)) descriptor = {
	{
		0x8,		/* bDescLength */
		0x29,		/* bDescriptorType: hub descriptor */
		2,		/* bNrPorts -- runtime modified */
		0,		/* wHubCharacteristics */
		50,		/* bPwrOn2PwrGood (units of 2ms) -> 100ms */
		0,		/* bHubCntrCurrent */
		{		/* Device removable */
		}		/* at most 7 ports! XXX */
	},
	{
		0x12,		/* bLength */
		1,		/* bDescriptorType: UDESC_DEVICE */
		cpu_to_le16(0x0200), /* bcdUSB: v2.0 */
		9,		/* bDeviceClass: UDCLASS_HUB */
		0,		/* bDeviceSubClass: UDSUBCLASS_HUB */
		1,		/* bDeviceProtocol: UDPROTO_HSHUBSTT */
		64,		/* bMaxPacketSize: 64 bytes */
		cpu_to_le16(0x3660),	/* idVendor: Z3660 identifier */
		cpu_to_le16(0xE4C1),	/* idProduct: EHCI root hub marker */
		cpu_to_le16(0x0100), /* bcdDevice */
		1,		/* iManufacturer */
		2,		/* iProduct */
		0,		/* iSerialNumber */
		1		/* bNumConfigurations: 1 */
	},
	{
		0x9,
		2,		/* bDescriptorType: UDESC_CONFIG */
		cpu_to_le16(0x19),
		1,		/* bNumInterface */
		1,		/* bConfigurationValue */
		0,		/* iConfiguration */
		0x40,		/* bmAttributes: UC_SELF_POWER */
		0		/* bMaxPower */
	},
	{
		0x9,		/* bLength */
		4,		/* bDescriptorType: UDESC_INTERFACE */
		0,		/* bInterfaceNumber */
		0,		/* bAlternateSetting */
		1,		/* bNumEndpoints */
		9,		/* bInterfaceClass: UICLASS_HUB */
		0,		/* bInterfaceSubClass: UISUBCLASS_HUB */
		0,		/* bInterfaceProtocol: UIPROTO_HSHUBSTT */
		0		/* iInterface */
	},
	{
		0x7,		/* bLength */
		5,		/* bDescriptorType: UDESC_ENDPOINT */
		0x81,		/* bEndpointAddress: UE_DIR_IN | EHCI_INTR_ENDPT */
		3,		/* bmAttributes: UE_INTERRUPT */
		8,		/* wMaxPacketSize */
		255,		/* bInterval */
		0,		/* bRefresh */
		0		/* bSynchAddress */
	},
};

// Enable TDI mode for Zynq EHCI to handle low-speed devices directly
// But with improved low-speed device handling
#define CONFIG_EHCI_IS_TDI

#if defined(CONFIG_EHCI_IS_TDI)
#define ehci_is_TDI()	(1)
#else
#define ehci_is_TDI()	(0)
#endif

static struct ehci_ctrl *ehci_get_ctrl(struct usb_device *udev)
{
	return udev->controller;
}

static int ehci_get_port_speed(struct ehci_ctrl *ctrl, uint32_t reg)
{
	(void)ctrl;
    uint32_t pspd_bits = PORTSC_PSPD(reg);  /* bits 26-27 */
    uint32_t ls_bits = (reg & EHCI_PS_LS) >> 10;  /* bits 10-11 */
    uint32_t port_enabled = (reg & EHCI_PS_PE) ? 1 : 0;
    uint32_t port_connected = (reg & EHCI_PS_CS) ? 1 : 0;
    uint32_t port_reset = (reg & EHCI_PS_PR) ? 1 : 0;
    
    /* Handle K-state (ls_bits=1) as definite indicator of LOW speed at any time */
    if (ls_bits == 1) {
        USB_DEBUG("[EHCI SPEED] K-state detected: Device is LOW speed\n");
        return PORTSC_PSPD_LS;
    }
    
    /* For other line states, interpret based on connection phase */
    if (!port_connected) {
        USB_DEBUG("[EHCI SPEED] Port not connected\n");
        return PORTSC_PSPD_FS; /* Default when no connection */
    }
    
    /* During reset or when port disabled: use line state */
    if (port_reset || !port_enabled) {
        USB_DEBUG("[EHCI SPEED] Port in reset/disabled (PORTSC=0x%08lx LS=%lu)\n", reg, ls_bits);
        
        /* During reset/disabled, line state is authoritative */
        switch (ls_bits) {
            case 0: /* SE0 - common during reset, could be HS */
                return PORTSC_PSPD_HS;
                
            case 2: /* J-state indicates potential FS */
				USB_DEBUG("[EHCI SPEED] J-state detected during reset: Tentative FULL speed\n");
                return PORTSC_PSPD_FS;
                
            /* K-state (1) handled above */
            
            default:
				USB_DEBUG("[EHCI SPEED] Warning: Invalid line state %lu\n", ls_bits);
                return PORTSC_PSPD_FS;
        }
    }
    
    /* Port enabled - verify PSPD agrees with line state history */
    int port_speed;
    switch (pspd_bits) {
        case PORTSC_PSPD_LS: /* 01 = Low speed */
            port_speed = PORTSC_PSPD_LS;
            /* Warn if line state doesn't match */
            if (ls_bits != 1) {
                USB_DEBUG("[EHCI SPEED] Warning: PSPD=LS but line state=%lu\n", ls_bits);
            }
            break;
            
        case PORTSC_PSPD_HS: /* 10 = High speed */
            port_speed = PORTSC_PSPD_HS;
            break;
            
        default: /* PSPD = 00 */
            if (ls_bits == 2) {
                /* J-state confirms FULL speed */
                port_speed = PORTSC_PSPD_FS;
                USB_DEBUG("[EHCI SPEED] J-state confirms FULL speed\n");
            } else {
                /* If not J-state, check history */
                if (ls_bits == 0) {
                    /* SE0 - probably still resetting */
                    port_speed = PORTSC_PSPD_HS;
                    USB_DEBUG("[EHCI SPEED] SE0 state with PSPD=0: Assuming HIGH speed\n");
                } else {
                    /* Default conservatively to FULL speed */
                    port_speed = PORTSC_PSPD_FS;
                    USB_DEBUG("[EHCI SPEED] Unknown state: Defaulting to FULL speed\n");
                }
            }
            break;
    }
    
    /* Complete debug output */
    USB_DEBUG("[EHCI SPEED] PORTSC=0x%08lx PSPD=%lu LS=%lu PE=%lu CS=%lu PR=%lu -> %s speed\n", 
           reg, pspd_bits, ls_bits, port_enabled, port_connected, port_reset,
           (port_speed == PORTSC_PSPD_LS) ? "LOW" :
           (port_speed == PORTSC_PSPD_HS) ? "HIGH" : "FULL");
    
    return port_speed;
}

static void ehci_set_usbmode(struct ehci_ctrl *ctrl)
{
	uint32_t tmp;
	uint32_t *reg_ptr;

	reg_ptr = (uint32_t *)((u8 *)&ctrl->hcor->or_usbcmd + USBMODE);
	tmp = ehci_readl(reg_ptr);
	tmp |= USBMODE_CM_HC;
#if defined(CONFIG_EHCI_MMIO_BIG_ENDIAN)
	tmp |= USBMODE_BE;
#else
	tmp &= ~USBMODE_BE;
#endif
	ehci_writel(reg_ptr, tmp);
}

static void ehci_powerup_fixup(struct ehci_ctrl *ctrl, uint32_t *status_reg,
			       uint32_t *reg)
{
	(void)ctrl;
	(void)status_reg;
	(void)reg;
	mdelay(50); // FIXME
}

static uint32_t *ehci_get_portsc_register(struct ehci_ctrl *ctrl, int port)
{
	int max_ports = HCS_N_PORTS(ehci_readl(&ctrl->hccr->cr_hcsparams));

	if (port < 0 || port >= max_ports) {
		/* Printing the message would cause a scan failure! */
		printf("The request port(%u) exceeds maximum port number\n", port);
		return NULL;
	}

	return (uint32_t *)&ctrl->hcor->or_portsc[port];
}

static int handshake(uint32_t *ptr, uint32_t mask, uint32_t done, int usec)
{
	uint32_t result;
	do {
		result = ehci_readl(ptr);
		udelay(5);
		if (result == ~(uint32_t)0)
			return -1;
		result &= mask;
		if (result == done)
			return 0;
		usec--;
	} while (usec > 0);
	return -1;
}

static int ehci_reset(struct ehci_ctrl *ctrl)
{
	uint32_t cmd;
	int ret = 0;

	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd = (cmd & ~CMD_RUN) | CMD_RESET_HC;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);
	ret = handshake((uint32_t *)&ctrl->hcor->or_usbcmd,
			CMD_RESET_HC, 0, 250 * 1000);
	if (ret < 0) {
		printf("EHCI fail to reset\n");
		goto out;
	}

	if (ehci_is_TDI())
		ctrl->ops.set_usb_mode(ctrl);

#ifdef CONFIG_USB_EHCI_TXFIFO_THRESH
	cmd = ehci_readl(&ctrl->hcor->or_txfilltuning);
	cmd &= ~TXFIFO_THRESH_MASK;
	cmd |= TXFIFO_THRESH(CONFIG_USB_EHCI_TXFIFO_THRESH);
	ehci_writel(&ctrl->hcor->or_txfilltuning, cmd);
#endif
out:
	return ret;
}

static int ehci_shutdown(struct ehci_ctrl *ctrl)
{
	int i, ret = 0;
	uint32_t cmd, reg;
	int max_ports = HCS_N_PORTS(ehci_readl(&ctrl->hccr->cr_hcsparams));

	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	/* If not run, directly return */
	if (!(cmd & CMD_RUN))
		return 0;
	cmd &= ~(CMD_PSE | CMD_ASE);
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);
	ret = handshake(&ctrl->hcor->or_usbsts, STS_ASS | STS_PSS, 0,
		100 * 1000);

	if (!ret) {
		for (i = 0; i < max_ports; i++) {
			reg = ehci_readl(&ctrl->hcor->or_portsc[i]);
			reg |= EHCI_PS_SUSP;
			ehci_writel(&ctrl->hcor->or_portsc[i], reg);
		}

		cmd &= ~CMD_RUN;
		ehci_writel(&ctrl->hcor->or_usbcmd, cmd);
		ret = handshake(&ctrl->hcor->or_usbsts, STS_HALT, STS_HALT,
			HCHALT_TIMEOUT);
	}

	if (ret)
		puts("EHCI failed to shut down host controller.\n");

	return ret;
}

static int ehci_td_buffer(struct qTD *td, void *buf, size_t sz)
{
	uint32_t delta, next;
	unsigned long addr = (unsigned long)buf;
	int idx;

	/* Note: Buffer may not be aligned, but EHCI can handle it */

	flush_dcache_range(addr, ALIGN(addr + sz, ARCH_DMA_MINALIGN));

	idx = 0;
	while (idx < QT_BUFFER_CNT) {
		td->qt_buffer[idx] = cpu_to_hc32(virt_to_phys((void *)addr));
		td->qt_buffer_hi[idx] = 0;
		next = (addr + EHCI_PAGE_SIZE) & ~(EHCI_PAGE_SIZE - 1);
		delta = next - addr;
		if (delta >= sz)
			break;
		sz -= delta;
		addr = next;
		idx++;
	}

	if (idx == QT_BUFFER_CNT) {
		printf("out of buffer pointers (%zu bytes left)\n", sz);
		return -1;
	}

	return 0;
}

static inline u8 ehci_encode_speed(enum usb_device_speed speed)
{
	#define QH_HIGH_SPEED	2
	#define QH_FULL_SPEED	0
	#define QH_LOW_SPEED	1
	if (speed == USB_SPEED_HIGH)
		return QH_HIGH_SPEED;
	if (speed == USB_SPEED_LOW)
		return QH_LOW_SPEED;
	return QH_FULL_SPEED;
}

static void ehci_update_endpt2_dev_n_port(struct usb_device *udev,
					  struct QH *qh)
{
	uint8_t portnr = 0;
	uint8_t hubaddr = 0;

	if (udev->speed != USB_SPEED_LOW && udev->speed != USB_SPEED_FULL)
		return;

	usb_find_usb2_hub_address_port(udev, &hubaddr, &portnr);

//	printf("[SPLIT] Device speed=%s, hub_addr=%d, hub_port=%d\n",
//	       (udev->speed == USB_SPEED_LOW) ? "LOW" : "FULL",
//	       hubaddr, portnr);

	qh->qh_endpt2 |= cpu_to_hc32(QH_ENDPT2_PORTNUM(portnr) |
				     QH_ENDPT2_HUBADDR(hubaddr));
	
//	printf("[SPLIT] qh_endpt2 configured: 0x%08x\n", hc32_to_cpu(qh->qh_endpt2));
}

static int
ehci_submit_async(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int length, struct devrequest *req)
{
	/* Log CONFIG descriptor requests for debugging */
	if (req && (req->requesttype & 0x80) && req->request == USB_REQ_GET_DESCRIPTOR && 
	    ((le16_to_cpu(req->value) >> 8) == USB_DT_CONFIG)) {
		uint8_t config_index = le16_to_cpu(req->value) & 0xff;
		printf("[EHCI] GET_DESCRIPTOR(CONFIG) dev=%d config_index=%u length=%d\n", 
		       dev->devnum, config_index, length);
	}
	ALLOC_ALIGN_BUFFER(struct QH, qh, 1, USB_DMA_MINALIGN);
	struct qTD *qtd;
	int qtd_count = 0;
	int qtd_counter = 0;
	volatile struct qTD *vtd;
	unsigned long ts;
	uint32_t *tdp;
	uint32_t endpt, maxpacket, token, usbsts;
	uint32_t c, toggle;
	uint32_t cmd;
	unsigned long timeout;
	int ret = 0;
	struct ehci_ctrl *ctrl = ehci_get_ctrl(dev);

	//printf("dev=%p, pipe=%lx, buffer=%p, length=%d, req=%p\n", dev, pipe,
	//      buffer, length, req);
	/*if (req != NULL)
		printf("req=%u (%#x), type=%u (%#x), value=%u (%#x), index=%u\n",
		      req->request, req->request,
		      req->requesttype, req->requesttype,
		      le16_to_cpu(req->value), le16_to_cpu(req->value),
		      le16_to_cpu(req->index));*/

#define PKT_ALIGN	512
	/*
	 * The USB transfer is split into qTD transfers. Eeach qTD transfer is
	 * described by a transfer descriptor (the qTD). The qTDs form a linked
	 * list with a queue head (QH).
	 *
	 * Each qTD transfer starts with a new USB packet, i.e. a packet cannot
	 * have its beginning in a qTD transfer and its end in the following
	 * one, so the qTD transfer lengths have to be chosen accordingly.
	 *
	 * Each qTD transfer uses up to QT_BUFFER_CNT data buffers, mapped to
	 * single pages. The first data buffer can start at any offset within a
	 * page (not considering the cache-line alignment issues), while the
	 * following buffers must be page-aligned. There is no alignment
	 * constraint on the size of a qTD transfer.
	 */
	if (req != NULL)
		/* 1 qTD will be needed for SETUP, and 1 for ACK. */
		qtd_count += 1 + 1;
	if (length > 0 || req == NULL) {
		/*
		 * Determine the qTD transfer size that will be used for the
		 * data payload (not considering the first qTD transfer, which
		 * may be longer or shorter, and the final one, which may be
		 * shorter).
		 *
		 * In order to keep each packet within a qTD transfer, the qTD
		 * transfer size is aligned to PKT_ALIGN, which is a multiple of
		 * wMaxPacketSize (except in some cases for interrupt transfers,
		 * see comment in submit_int_msg()).
		 *
		 * By default, i.e. if the input buffer is aligned to PKT_ALIGN,
		 * QT_BUFFER_CNT full pages will be used.
		 */
		int xfr_sz = QT_BUFFER_CNT;
		/*
		 * However, if the input buffer is not aligned to PKT_ALIGN, the
		 * qTD transfer size will be one page shorter, and the first qTD
		 * data buffer of each transfer will be page-unaligned.
		 */
		if ((unsigned long)buffer & (PKT_ALIGN - 1))
			xfr_sz--;
		/* Convert the qTD transfer size to bytes. */
		xfr_sz *= EHCI_PAGE_SIZE;
		/*
		 * Approximate by excess the number of qTDs that will be
		 * required for the data payload. The exact formula is way more
		 * complicated and saves at most 2 qTDs, i.e. a total of 128
		 * bytes.
		 */
		qtd_count += 2 + length / xfr_sz;
	}
/*
 * Threshold value based on the worst-case total size of the allocated qTDs for
 * a mass-storage transfer of 65535 blocks of 512 bytes.
 */
//#if CONFIG_SYS_MALLOC_LEN <= 64 + 128 * 1024
//#warning CONFIG_SYS_MALLOC_LEN may be too small for EHCI
//#endif

	// FIXME needs 128kB ram?

	qtd = memalign(USB_DMA_MINALIGN, qtd_count * sizeof(struct qTD)); // FIXME dynamic allocation
	if (qtd == NULL) {
		printf("unable to allocate TDs\n");
		return -1;
	}

	memset(qh, 0, sizeof(struct QH));
	memset(qtd, 0, qtd_count * sizeof(*qtd));

	toggle = usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/*
	 * Setup QH (3.6 in ehci-r10.pdf)
	 *
	 *   qh_link ................. 03-00 H
	 *   qh_endpt1 ............... 07-04 H
	 *   qh_endpt2 ............... 0B-08 H
	 * - qh_curtd
	 *   qh_overlay.qt_next ...... 13-10 H
	 * - qh_overlay.qt_altnext
	 */
	qh->qh_link = cpu_to_hc32(virt_to_phys(&ctrl->qh_list) | QH_LINK_TYPE_QH);
	c = (dev->speed != USB_SPEED_HIGH) && !usb_pipeendpoint(pipe);
	maxpacket = usb_maxpacket(dev, pipe);
	endpt = QH_ENDPT1_RL(8) | QH_ENDPT1_C(c) |
		QH_ENDPT1_MAXPKTLEN(maxpacket) | QH_ENDPT1_H(0) |
		QH_ENDPT1_DTC(QH_ENDPT1_DTC_DT_FROM_QTD) |
		QH_ENDPT1_ENDPT(usb_pipeendpoint(pipe)) | QH_ENDPT1_I(0) |
		QH_ENDPT1_DEVADDR(usb_pipedevice(pipe));

	/* Force FS for fsl HS quirk */
	if (!ctrl->has_fsl_erratum_a005275)
		endpt |= QH_ENDPT1_EPS(ehci_encode_speed(dev->speed));
	else
		endpt |= QH_ENDPT1_EPS(ehci_encode_speed(QH_FULL_SPEED));

	qh->qh_endpt1 = cpu_to_hc32(endpt);
	endpt = QH_ENDPT2_MULT(1);
	
	/* Configure S-mask and C-mask for split transactions */
	if (dev->speed == USB_SPEED_LOW || dev->speed == USB_SPEED_FULL) {
		/* For FS/LS devices, configure split transaction microframes */
		/* Use SPECIFIC microframes for better reliability */
		uint8_t s_mask = 0x01;  /* Start in microframe 0 only */
		
		/* Try different C-mask strategies for FS devices */
		uint8_t c_mask;
		
		if (dev->speed == USB_SPEED_LOW) {
			/* LOW SPEED: Use wider completion window */
			c_mask = 0xFE;  /* Complete in microframes 1-7 */
			#ifdef USB_DEBUG_VERBOSE
			USB_DEBUG("[EHCI SPLIT] Device %d LOW SPEED: S-mask=0x%02x C-mask=0x%02x\n", 
			       dev->devnum, s_mask, c_mask);
			#endif
		} else {
			/* FULL SPEED: Try MUCH more conservative timing for stability */
			/* Start split immediately and allow completion in most microframes */
			s_mask = 0x01;  /* Start in microframe 0 */
			c_mask = 0xFF;  /* Complete in ALL microframes 0-7 (most conservative) */
			
			#ifdef USB_DEBUG_VERBOSE
			USB_DEBUG("[EHCI SPLIT] Device %d FULL SPEED: S-mask=0x%02x C-mask=0x%02x (U-BOOT STYLE)\n", 
			       dev->devnum, s_mask, c_mask);
			#endif
		}
		
		endpt |= QH_ENDPT2_UFSMASK(s_mask);  
		endpt |= QH_ENDPT2_UFCMASK(c_mask);  
	} else {
		/* High-Speed devices: no split transactions needed */
		endpt |= QH_ENDPT2_UFSMASK(0);     /* S-mask: 0 (no split) */
		endpt |= QH_ENDPT2_UFCMASK(0);     /* C-mask: 0 (no split) */
		USB_DEBUG("[EHCI SPLIT] Device %d HIGH SPEED: no split transactions\n", dev->devnum);
	}
	
	qh->qh_endpt2 = cpu_to_hc32(endpt);
	ehci_update_endpt2_dev_n_port(dev, qh);
	
	#ifdef USB_DEBUG_VERBOSE
	USB_DEBUG("[EHCI QH] Device %d: qh_endpt1=0x%08x qh_endpt2=0x%08lx (speed=%s)\n",
	       dev->devnum, hc32_to_cpu(qh->qh_endpt1), hc32_to_cpu(qh->qh_endpt2),
	       (dev->speed == USB_SPEED_HIGH) ? "HIGH" :
	       (dev->speed == USB_SPEED_FULL) ? "FULL" :
	       (dev->speed == USB_SPEED_LOW) ? "LOW" : "UNKNOWN");
	#endif
	qh->qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);

	tdp = &qh->qh_overlay.qt_next;
	if (req != NULL) {
		/*
		 * Setup request qTD (3.5 in ehci-r10.pdf)
		 *
		 *   qt_next ................ 03-00 H
		 *   qt_altnext ............. 07-04 H
		 *   qt_token ............... 0B-08 H
		 *
		 *   [ buffer, buffer_hi ] loaded with "req".
		 */
		qtd[qtd_counter].qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		qtd[qtd_counter].qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		/* Robust error handling: allow up to 3 transaction errors (EHCI CERR=3) */
		uint8_t cerr_count = 3;
		token = QT_TOKEN_DT(0) | QT_TOKEN_TOTALBYTES(sizeof(*req)) |
			QT_TOKEN_IOC(0) | QT_TOKEN_CPAGE(0) | QT_TOKEN_CERR(cerr_count) |
			QT_TOKEN_PID(QT_TOKEN_PID_SETUP) |
			QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
		qtd[qtd_counter].qt_token = cpu_to_hc32(token);
		if (ehci_td_buffer(&qtd[qtd_counter], req, sizeof(*req))) {
			printf("unable to construct SETUP TD\n");
			goto fail;
		}
		/* Update previous qTD! */
		*tdp = cpu_to_hc32(virt_to_phys(&qtd[qtd_counter]));
		tdp = &qtd[qtd_counter++].qt_next;
		toggle = 1;
	}

	if (length > 0 || req == NULL) {
		uint8_t *buf_ptr = buffer;
		int left_length = length;

		do {
			/*
			 * Determine the size of this qTD transfer. By default,
			 * QT_BUFFER_CNT full pages can be used.
			 */
			int xfr_bytes = QT_BUFFER_CNT * EHCI_PAGE_SIZE;
			/*
			 * However, if the input buffer is not page-aligned, the
			 * portion of the first page before the buffer start
			 * offset within that page is unusable.
			 */
			xfr_bytes -= (unsigned long)buf_ptr & (EHCI_PAGE_SIZE - 1);
			/*
			 * In order to keep each packet within a qTD transfer,
			 * align the qTD transfer size to PKT_ALIGN.
			 */
			xfr_bytes &= ~(PKT_ALIGN - 1);
			/*
			 * This transfer may be shorter than the available qTD
			 * transfer size that has just been computed.
			 */
			xfr_bytes = min(xfr_bytes, left_length);

			/*
			 * Setup request qTD (3.5 in ehci-r10.pdf)
			 *
			 *   qt_next ................ 03-00 H
			 *   qt_altnext ............. 07-04 H
			 *   qt_token ............... 0B-08 H
			 *
			 *   [ buffer, buffer_hi ] loaded with "buffer".
			 */
			qtd[qtd_counter].qt_next =
					cpu_to_hc32(QT_NEXT_TERMINATE);
			qtd[qtd_counter].qt_altnext =
					cpu_to_hc32(QT_NEXT_TERMINATE);
			/* Robust error handling: allow up to 3 transaction errors (EHCI CERR=3) */
			uint8_t cerr_count = 3;
			token = QT_TOKEN_DT(toggle) |
				QT_TOKEN_TOTALBYTES(xfr_bytes) |
				QT_TOKEN_IOC(req == NULL) | QT_TOKEN_CPAGE(0) |
				QT_TOKEN_CERR(cerr_count) |
				QT_TOKEN_PID(usb_pipein(pipe) ?
					QT_TOKEN_PID_IN : QT_TOKEN_PID_OUT) |
				QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
			qtd[qtd_counter].qt_token = cpu_to_hc32(token);
			if (ehci_td_buffer(&qtd[qtd_counter], buf_ptr,
						xfr_bytes)) {
				printf("unable to construct DATA TD\n");

				goto fail;
			}
			/* Update previous qTD! */
			*tdp = cpu_to_hc32(virt_to_phys(&qtd[qtd_counter]));
			tdp = &qtd[qtd_counter++].qt_next;
			/*
			 * Data toggle has to be adjusted since the qTD transfer
			 * size is not always an even multiple of
			 * wMaxPacketSize.
			 */
			if ((xfr_bytes / maxpacket) & 1)
				toggle ^= 1;
			buf_ptr += xfr_bytes;
			left_length -= xfr_bytes;
		} while (left_length > 0);
	}

	if (req != NULL) {
		/*
		 * Setup request qTD (3.5 in ehci-r10.pdf)
		 *
		 *   qt_next ................ 03-00 H
		 *   qt_altnext ............. 07-04 H
		 *   qt_token ............... 0B-08 H
		 */
		qtd[qtd_counter].qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		qtd[qtd_counter].qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		token = QT_TOKEN_DT(1) | QT_TOKEN_TOTALBYTES(0) |
			QT_TOKEN_IOC(1) | QT_TOKEN_CPAGE(0) | QT_TOKEN_CERR(3) |
			QT_TOKEN_PID(usb_pipein(pipe) ?
				QT_TOKEN_PID_OUT : QT_TOKEN_PID_IN) |
			QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
		qtd[qtd_counter].qt_token = cpu_to_hc32(token);
		/* Update previous qTD! */
		*tdp = cpu_to_hc32(virt_to_phys(&qtd[qtd_counter]));
		tdp = &qtd[qtd_counter++].qt_next;
	}

	ctrl->qh_list.qh_link = cpu_to_hc32(virt_to_phys(qh) | QH_LINK_TYPE_QH);

	/* Flush dcache */
	uint32_t end = ALIGN_END_ADDR(struct QH, &ctrl->qh_list, 1);
	flush_dcache_range((unsigned long)&ctrl->qh_list, end);
	flush_dcache_range((unsigned long)qh, ALIGN_END_ADDR(struct QH, qh, 1));
	flush_dcache_range((unsigned long)qtd,
			   ALIGN_END_ADDR(struct qTD, qtd, qtd_count));

	/* Set async. queue head pointer. */
	ehci_writel(&ctrl->hcor->or_asynclistaddr, virt_to_phys(&ctrl->qh_list));

	usbsts = ehci_readl(&ctrl->hcor->or_usbsts);
	ehci_writel(&ctrl->hcor->or_usbsts, (usbsts & 0x3f));

	/* Enable async. schedule. */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd |= CMD_ASE;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&ctrl->hcor->or_usbsts, STS_ASS, STS_ASS,
			100 * 1000);
	if (ret < 0) {
		printf("EHCI fail timeout STS_ASS set\n");
		goto fail;
	}

	/* Wait for TDs to be processed. */
	ts = get_timer(0);  // FIXME
	vtd = &qtd[qtd_counter - 1];
	timeout = USB_TIMEOUT_MS(pipe);
	
	
	do {
		/* Invalidate dcache */
		invalidate_dcache_range((unsigned long)&ctrl->qh_list,
			ALIGN_END_ADDR(struct QH, &ctrl->qh_list, 1));
		invalidate_dcache_range((unsigned long)qh,
			ALIGN_END_ADDR(struct QH, qh, 1));
		invalidate_dcache_range((unsigned long)qtd,
			ALIGN_END_ADDR(struct qTD, qtd, qtd_count));

		token = hc32_to_cpu(vtd->qt_token);
		if (!(QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE))
			break;
		//WATCHDOG_RESET(); // FIXME
	} while (get_timer(ts) < timeout);


	/*
	 * Invalidate the memory area occupied by buffer
	 * Don't try to fix the buffer alignment, if it isn't properly
	 * aligned it's upper layer's fault so let invalidate_dcache_range()
	 * vow about it. But we have to fix the length as it's actual
	 * transfer length and can be unaligned. This is potentially
	 * dangerous operation, it's responsibility of the calling
	 * code to make sure enough space is reserved.
	 */
	if (buffer != NULL && length > 0)
		invalidate_dcache_range((unsigned long)buffer,
			ALIGN((unsigned long)buffer + length, ARCH_DMA_MINALIGN));


	/* Check that the TD processing happened */
	if (QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE) {
		USB_DEBUG_WARN("TD timeout - token=%#lx dev=%u\n", token, dev->devnum);
		
		/* For string descriptor requests that timeout, try to provide graceful handling */
		if (req && (req->requesttype & USB_DIR_IN) &&
		    req->request == USB_REQ_GET_DESCRIPTOR &&
		    ((le16_to_cpu(req->value) >> 8) == USB_DT_STRING)) {
			uint8_t string_index = le16_to_cpu(req->value) & 0xff;
			printf("[EHCI] Timeout on string descriptor index %u - device may not support it\n", string_index);
			
			/* Set a more specific error for string descriptor timeouts */
			dev->status = USB_ST_CRC_ERR;
			dev->act_len = 0;
		} else {
			/* For other requests, set timeout error */
			dev->status = USB_ST_CRC_ERR;
			dev->act_len = 0;
		}
	}

	/* Disable async schedule. */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd &= ~CMD_ASE;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&ctrl->hcor->or_usbsts, STS_ASS, 0,
			100 * 1000);
	if (ret < 0) {
		printf("EHCI fail timeout STS_ASS reset\n");
		goto fail;
	}

	token = hc32_to_cpu(qh->qh_overlay.qt_token);
	if (!(QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE)) {
		//printf("TOKEN=%#lx\n", token);
		switch (QT_TOKEN_GET_STATUS(token) &
			~(QT_TOKEN_STATUS_SPLITXSTATE | QT_TOKEN_STATUS_PERR)) {
		case 0:
			toggle = QT_TOKEN_GET_DT(token);
			usb_settoggle(dev, usb_pipeendpoint(pipe),
				       usb_pipeout(pipe), toggle);
			dev->status = 0;
			break;
		case QT_TOKEN_STATUS_HALTED:
			dev->status = USB_ST_STALLED;
			break;
		case QT_TOKEN_STATUS_ACTIVE | QT_TOKEN_STATUS_DATBUFERR:
		case QT_TOKEN_STATUS_DATBUFERR:
			dev->status = USB_ST_BUF_ERR;
			break;
		case QT_TOKEN_STATUS_HALTED | QT_TOKEN_STATUS_BABBLEDET:
		case QT_TOKEN_STATUS_BABBLEDET:
			dev->status = USB_ST_BABBLE_DET;
			break;
		default:
			dev->status = USB_ST_CRC_ERR;
			if (QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_HALTED)
				dev->status |= USB_ST_STALLED;
			break;
		}
		dev->act_len = length - QT_TOKEN_GET_TOTALBYTES(token);
	} else {
		dev->act_len = 0;
		//printf("dev=%u, usbsts=%#lx, p[1]=%#lx, p[2]=%#lx\n",
		//      dev->devnum, ehci_readl(&ctrl->hcor->or_usbsts),
		//      ehci_readl(&ctrl->hcor->or_portsc[0]),
		//      ehci_readl(&ctrl->hcor->or_portsc[1]));
	}

	/* Debug logging on error/status != 0 */
	if (dev->status != 0) {
		uint32_t usbsts_dbg = ehci_readl(&ctrl->hcor->or_usbsts);
		uint32_t portsc0_dbg = ehci_readl(&ctrl->hcor->or_portsc[0]);
		USB_DEBUG_ERROR("dev=%u pipe=0x%lx status=%ld token=0x%08lx act_len=%d usbsts=0x%08lx portsc0=0x%08lx\n",
		       dev->devnum, pipe, dev->status, token, dev->act_len,
		       usbsts_dbg, portsc0_dbg);
		if (req != NULL) {
			USB_DEBUG_VERBOSE("bmReq=0x%02x bReq=0x%02x wValue=0x%04x wIndex=0x%04x wLength=%u\n",
			       req->requesttype, req->request,
			       le16_to_cpu(req->value), le16_to_cpu(req->index),
			       le16_to_cpu(req->length));
		}
	}

	free(qtd);
	return (dev->status != USB_ST_NOT_PROC) ? 0 : -1;

fail:
	free(qtd);
	return -1;
}

/* Configuration: Use USB3320C physical hub as root hub instead of software emulation */
// #define USE_USB3320C_AS_ROOT_HUB  // DISABLED - use standard U-Boot root hub emulation

static int ehci_submit_root(struct usb_device *dev, unsigned long pipe,
			    void *buffer, int length, struct devrequest *req)
{
	(void)pipe;
	(void)buffer;
	uint8_t tmpbuf[4];
	u16 typeReq;
	void *srcptr = NULL;
	int len, srclen;
	uint32_t reg;
	uint32_t *status_reg;
	int port = le16_to_cpu(req->index) & 0xff;
	struct ehci_ctrl *ctrl = ehci_get_ctrl(dev);

	srclen = 0;

	/*printf("req=%u (%#x), type=%u (%#x), value=%u, index=%u\n",
	      req->request, req->request,
	      req->requesttype, req->requesttype,
	      le16_to_cpu(req->value), le16_to_cpu(req->index));*/

	typeReq = req->request | req->requesttype << 8;

	/* Root hub request trace - Note: Only used when root hub emulation is enabled */
#if defined(USB_DEBUG_EHCI_ROOT)
	USB_DEBUG("[ehci-root] typeReq=0x%04x bmReq=0x%02x bReq=0x%02x wValue=0x%04x wIndex=0x%04x wLength=%u\n",
	       typeReq, req->requesttype, req->request,
	       le16_to_cpu(req->value), le16_to_cpu(req->index),
	       le16_to_cpu(req->length));
//	printf("[ehci-root] NOTE: Root hub emulation active - USB3320C mode should disable this\n");
#endif

	switch (typeReq) {
	case USB_REQ_GET_STATUS | ((USB_RT_PORT | USB_DIR_IN) << 8):
	case USB_REQ_SET_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
	case USB_REQ_CLEAR_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
		status_reg = ctrl->ops.get_portsc_register(ctrl, port - 1);
		if (!status_reg)
			return -1;
		break;
	default:
		status_reg = NULL;
		break;
	}

	switch (typeReq) {
	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_DEVICE:
			//printf("USB_DT_DEVICE request\n");
			srcptr = &descriptor.device;
			srclen = descriptor.device.bLength;
			break;
		case USB_DT_CONFIG:
			{
				/* CRITICAL FIX: Only return CONFIG 0, STALL for non-existent configs */
				uint8_t config_index = le16_to_cpu(req->value) & 0xff;
				USB_DEBUG("[ehci-root] GET_DESCRIPTOR(CONFIG) index=%u\n", config_index);
				
				if (config_index == 0) {
					/* CONFIG 0: Return valid root hub configuration */
					//printf("USB_DT_CONFIG config\n");
					srcptr = &descriptor.config;
					srclen = descriptor.config.bLength +
							descriptor.interface.bLength +
							descriptor.endpoint.bLength;
				} else {
					/* CONFIG >= 1: Invalid configuration - STALL */
					printf("[ehci-root] ERROR: Invalid config index %u - STALLing\n", config_index);
					goto unknown;  /* This will cause STALL response */
				}
			}
			break;
		case USB_DT_STRING:
			//printf("USB_DT_STRING config\n");
			switch (le16_to_cpu(req->value) & 0xff) {
			case 0:	/* Language */
				srcptr = "\4\3\1\0";
				srclen = 4;
				break;
			case 1:	/* Vendor */
				srcptr = "\16\3u\0-\0b\0o\0o\0t\0";
				srclen = 14;
				break;
			case 2:	/* Product */
				srcptr = "\52\3E\0H\0C\0I\0 "
					 "\0H\0o\0s\0t\0 "
					 "\0C\0o\0n\0t\0r\0o\0l\0l\0e\0r\0";
				srclen = 42;
				break;
			default:
				printf("unknown value DT_STRING %x\n",
					le16_to_cpu(req->value));
				goto unknown;
			}
			break;
		default:
			printf("unknown value %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
	case USB_REQ_GET_DESCRIPTOR | ((USB_DIR_IN | USB_RT_HUB) << 8):
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_HUB:
			//printf("[ehci] USB_DT_HUB config\n");
			srcptr = &descriptor.hub;
			srclen = descriptor.hub.bLength;
			break;
		default:
			printf("unknown value %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
    case USB_REQ_SET_ADDRESS | (USB_RECIP_DEVICE << 8):
        //printf("USB_REQ_SET_ADDRESS\n");
        ctrl->rootdev = le16_to_cpu(req->value);
        break;
	case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
		//printf("USB_REQ_SET_CONFIGURATION\n");
		/* Nothing to do */
		break;
	case USB_REQ_GET_STATUS | ((USB_DIR_IN | USB_RT_HUB) << 8):
		tmpbuf[0] = 1;	/* USB_STATUS_SELFPOWERED */
		tmpbuf[1] = 0;
		srcptr = tmpbuf;
		srclen = 2;
		break;
	case USB_REQ_GET_STATUS | ((USB_DIR_IN | USB_RECIP_DEVICE) << 8):
		/* Device status - bit 0: self-powered, bit 1: remote wakeup */
		tmpbuf[0] = 1;	/* USB_STATUS_SELFPOWERED (root hub is self-powered) */
		tmpbuf[1] = 0;	/* No remote wakeup capability */
		srcptr = tmpbuf;
		srclen = 2;
		USB_DEBUG("[ehci-root] GET_STATUS(DEVICE): returning self-powered status\n");
		break;
	case USB_REQ_GET_STATUS | ((USB_RT_PORT | USB_DIR_IN) << 8):
		memset(tmpbuf, 0, 4);
		reg = ehci_readl(status_reg);
		if (reg & EHCI_PS_CS)
			tmpbuf[0] |= USB_PORT_STAT_CONNECTION;
		if (reg & EHCI_PS_PE)
			tmpbuf[0] |= USB_PORT_STAT_ENABLE;
		if (reg & EHCI_PS_SUSP)
			tmpbuf[0] |= USB_PORT_STAT_SUSPEND;
		if (reg & EHCI_PS_OCA)
			tmpbuf[0] |= USB_PORT_STAT_OVERCURRENT;
		if (reg & EHCI_PS_PR)
			tmpbuf[0] |= USB_PORT_STAT_RESET;
		if (reg & EHCI_PS_PP)
			tmpbuf[1] |= USB_PORT_STAT_POWER >> 8;
#if defined(USB_DEBUG_EHCI_ROOT)
		USB_DEBUG("[ehci-root] GET_STATUS: port=%d PORTSC=0x%08lx -> status_lo=0x%02x status_hi=0x%02x change=0x%02x\n",
		       port, (unsigned long)reg, tmpbuf[0], tmpbuf[1], tmpbuf[2]);
#endif

		if (ehci_is_TDI()) {
			switch (ctrl->ops.get_port_speed(ctrl, reg)) {
			case PORTSC_PSPD_FS:
				break;
			case PORTSC_PSPD_LS:
				tmpbuf[1] |= USB_PORT_STAT_LOW_SPEED >> 8;
				break;
			case PORTSC_PSPD_HS:
			default:
				tmpbuf[1] |= USB_PORT_STAT_HIGH_SPEED >> 8;
				break;
			}
		} else {
			tmpbuf[1] |= USB_PORT_STAT_HIGH_SPEED >> 8;
		}

		if (reg & EHCI_PS_CSC)
			tmpbuf[2] |= USB_PORT_STAT_C_CONNECTION;
		if (reg & EHCI_PS_PEC)
			tmpbuf[2] |= USB_PORT_STAT_C_ENABLE;
		if (reg & EHCI_PS_OCC)
			tmpbuf[2] |= USB_PORT_STAT_C_OVERCURRENT;
		if (ctrl->portreset & (1 << port))
			tmpbuf[2] |= USB_PORT_STAT_C_RESET;

		srcptr = tmpbuf;
		srclen = 4;
		break;
	case USB_REQ_SET_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
		reg = ehci_readl(status_reg);
		reg &= ~EHCI_PS_CLEAR;
#if defined(USB_DEBUG_EHCI_ROOT)
		USB_DEBUG("[ehci-root] SET_FEATURE: port=%d feat=%u PORTSC(before)=0x%08lx\n",
		       port, le16_to_cpu(req->value), (unsigned long)reg);
#endif
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_ENABLE:
			reg |= EHCI_PS_PE;
			ehci_writel(status_reg, reg);
			break;
		case USB_PORT_FEAT_POWER:
			if (HCS_PPC(ehci_readl(&ctrl->hccr->cr_hcsparams))) {
				reg |= EHCI_PS_PP;
				ehci_writel(status_reg, reg);
				USB_DEBUG("[ehci-root] SET_FEATURE POWER: port=%d -> PORTSC(after)=0x%08lx\n",
				       port, (unsigned long)ehci_readl(status_reg));
			}
			break;
		case USB_PORT_FEAT_RESET:
			if ((reg & (EHCI_PS_PE | EHCI_PS_CS)) == EHCI_PS_CS &&
			    !ehci_is_TDI() &&
			    EHCI_PS_IS_LOWSPEED(reg)) {
				/* Low speed device, give up ownership. */
				USB_DEBUG("port %d low speed --> companion\n",
				      port - 1);
				reg |= EHCI_PS_PO;
				ehci_writel(status_reg, reg);
				return -ENXIO;
			} else {
				int ret;

				/* Disable chirp for HS erratum */
				if (ctrl->has_fsl_erratum_a005275)
					reg |= PORTSC_FSL_PFSC;

				reg |= EHCI_PS_PR;
				reg &= ~EHCI_PS_PE;
				ehci_writel(status_reg, reg);
				/*
				 * caller must wait, then call GetPortStatus
				 * usb 2.0 specification say 50 ms resets on
				 * root
				 */
				ctrl->ops.powerup_fixup(ctrl, status_reg, &reg);

				ehci_writel(status_reg, reg & ~EHCI_PS_PR);
				/*
				 * A host controller must terminate the reset
				 * and stabilize the state of the port within
				 * 2 milliseconds
				 */
				ret = handshake(status_reg, EHCI_PS_PR, 0,
						2 * 1000);
				if (!ret) {
					reg = ehci_readl(status_reg);
					if ((reg & (EHCI_PS_PE | EHCI_PS_CS))
					    == EHCI_PS_CS && !ehci_is_TDI()) {
						printf("port %d full speed --> companion\n", port - 1);
						reg &= ~EHCI_PS_CLEAR;
						reg |= EHCI_PS_PO;
						ehci_writel(status_reg, reg);
#if defined(USB_DEBUG_EHCI_ROOT)
						USB_DEBUG("[ehci-root] RESET complete (FS no-enable) -> handoff, PORTSC=0x%08lx\n",
					       (unsigned long)ehci_readl(status_reg));
#endif
						return -ENXIO;
					} else {
						ctrl->portreset |= 1 << port;
#if defined(USB_DEBUG_EHCI_ROOT)
						USB_DEBUG("[ehci-root] RESET complete: PORTSC=0x%08lx\n",
					       (unsigned long)reg);
#endif
					}
				} else {
					printf("port(%d) reset error\n",
					       port - 1);
				}
			}
			break;
		case USB_PORT_FEAT_TEST:
			ehci_shutdown(ctrl);
			reg &= ~(0xf << 16);
			reg |= ((le16_to_cpu(req->index) >> 8) & 0xf) << 16;
			ehci_writel(status_reg, reg);
			break;
		default:
			printf("unknown feature %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		/* unblock posted writes */
		(void) ehci_readl(&ctrl->hcor->or_usbcmd);
		break;
	case USB_REQ_CLEAR_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
		reg = ehci_readl(status_reg);
		reg &= ~EHCI_PS_CLEAR;
#if defined(USB_DEBUG_EHCI_ROOT)
		USB_DEBUG("[ehci-root] CLEAR_FEATURE: port=%d feat=%u PORTSC(before)=0x%08lx\n",
		       port, le16_to_cpu(req->value), (unsigned long)reg);
#endif
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_ENABLE:
			/* Disable port (clear PE) */
			reg &= ~EHCI_PS_PE;
			break;
		case USB_PORT_FEAT_C_ENABLE:
			/* Ack Port Enable/Disable Change (W1C) */
			reg |= EHCI_PS_PEC;
			break;
		case USB_PORT_FEAT_POWER:
			/* Some stacks may send CLEAR_FEATURE(POWER); accept by clearing PP if PPC */
			if (HCS_PPC(ehci_readl(&ctrl->hccr->cr_hcsparams)))
				reg &= ~EHCI_PS_PP;
			break;
		case USB_PORT_FEAT_C_CONNECTION:
			/* Ack Connect Status Change (W1C) */
			reg |= EHCI_PS_CSC;
			break;
		case USB_PORT_FEAT_C_OVER_CURRENT:
			/* Ack Over-current Change (W1C) */
			reg |= EHCI_PS_OCC;
			break;
		case USB_PORT_FEAT_C_SUSPEND:
			/* No dedicated EHCI W1C for C_SUSPEND; accept as no-op */
			break;
		case USB_PORT_FEAT_C_RESET:
			/* Clear software portreset latch */
			ctrl->portreset &= ~(1 << port);
			break;
		default:
			printf("unknown feature %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		ehci_writel(status_reg, reg);
#if defined(USB_DEBUG_EHCI_ROOT)
		USB_DEBUG("[ehci-root] CLEAR_FEATURE done: port=%d PORTSC(after)=0x%08lx\n",
		       port, (unsigned long)ehci_readl(status_reg));
#endif
		/* unblock posted write */
		(void) ehci_readl(&ctrl->hcor->or_usbcmd);
		break;
	default:
		printf("Unknown request\n");
		goto unknown;
	}

	mdelay(1);
	len = min3(srclen, (int)le16_to_cpu(req->length), length);
	if (srcptr != NULL && len > 0)
		memcpy(buffer, srcptr, len);
	//else
	//	printf("[ehci] Len is 0\n");

	dev->act_len = len;
	dev->status = 0;
	return 0;

unknown:
	printf("[ehci unknown] requesttype=%x, request=%x, value=%x, index=%x, length=%x\n",
	      req->requesttype, req->request, le16_to_cpu(req->value),
	      le16_to_cpu(req->index), le16_to_cpu(req->length));

	dev->act_len = 0;
	dev->status = USB_ST_STALLED;
	return -1;
}

static const struct ehci_ops default_ehci_ops = {
	.set_usb_mode		= ehci_set_usbmode,
	.get_port_speed		= ehci_get_port_speed,
	.powerup_fixup		= ehci_powerup_fixup,
	.get_portsc_register	= ehci_get_portsc_register,
};

static void ehci_setup_ops(struct ehci_ctrl *ctrl, const struct ehci_ops *ops)
{
	if (!ops) {
		ctrl->ops = default_ehci_ops;
	} else {
		ctrl->ops = *ops;
		if (!ctrl->ops.set_usb_mode)
			ctrl->ops.set_usb_mode = ehci_set_usbmode;
		if (!ctrl->ops.get_port_speed)
			ctrl->ops.get_port_speed = ehci_get_port_speed;
		if (!ctrl->ops.powerup_fixup)
			ctrl->ops.powerup_fixup = ehci_powerup_fixup;
		if (!ctrl->ops.get_portsc_register)
			ctrl->ops.get_portsc_register =
					ehci_get_portsc_register;
	}
}

void ehci_set_controller_priv(int index, void *priv, const struct ehci_ops *ops)
{
	struct ehci_ctrl *ctrl = &ehcic[index];

	ctrl->priv = priv;
	ehci_setup_ops(ctrl, ops);
}

void *ehci_get_controller_priv(int index)
{
	return ehcic[index].priv;
}

static int ehci_common_init(struct ehci_ctrl *ctrl, unsigned int tweaks)
{
	struct QH *qh_list;
	struct QH *periodic;
	uint32_t reg;
	uint32_t cmd;
	int i;

	/* Set the high address word (aka segment) for 64-bit controller */
	if (ehci_readl(&ctrl->hccr->cr_hccparams) & 1)
		ehci_writel(&ctrl->hcor->or_ctrldssegment, 0);

	qh_list = &ctrl->qh_list;

	/* Set head of reclaim list */
	memset(qh_list, 0, sizeof(*qh_list));
	qh_list->qh_link = cpu_to_hc32(virt_to_phys(qh_list) | QH_LINK_TYPE_QH);
	qh_list->qh_endpt1 = cpu_to_hc32(QH_ENDPT1_H(1) |
						QH_ENDPT1_EPS(USB_SPEED_HIGH));
	qh_list->qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list->qh_overlay.qt_token =
			cpu_to_hc32(QT_TOKEN_STATUS(QT_TOKEN_STATUS_HALTED));

	flush_dcache_range((unsigned long)qh_list,
			   ALIGN_END_ADDR(struct QH, qh_list, 1));

	/* Set async. queue head pointer. */
	ehci_writel(&ctrl->hcor->or_asynclistaddr, virt_to_phys(qh_list));

	/*
	 * Set up periodic list
	 * Step 1: Parent QH for all periodic transfers.
	 */
	ctrl->periodic_schedules = 0;
	periodic = &ctrl->periodic_queue;
	memset(periodic, 0, sizeof(*periodic));
	periodic->qh_link = cpu_to_hc32(QH_LINK_TERMINATE);
	periodic->qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
	periodic->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);

	flush_dcache_range((unsigned long)periodic,
			   ALIGN_END_ADDR(struct QH, periodic, 1));

	/*
	 * Step 2: Setup frame-list: Every microframe, USB tries the same list.
	 *         In particular, device specifications on polling frequency
	 *         are disregarded. Keyboards seem to send NAK/NYet reliably
	 *         when polled with an empty buffer.
	 *
	 *         Split Transactions will be spread across microframes using
	 *         S-mask and C-mask.
	 */
	if (ctrl->periodic_list == NULL)
		ctrl->periodic_list = memalign(4096, 1024 * 4); // FIXME dynamic allocation

	if (!ctrl->periodic_list) {
		printf("[ehci_common_init] ENOMEM\n");
		return -ENOMEM;
	}
	for (i = 0; i < 1024; i++) {
		ctrl->periodic_list[i] = cpu_to_hc32((unsigned long)periodic
						| QH_LINK_TYPE_QH);
	}

	flush_dcache_range((unsigned long)ctrl->periodic_list,
			   ALIGN_END_ADDR(uint32_t, ctrl->periodic_list,
					  1024));

	/* Set periodic list base address */
	ehci_writel(&ctrl->hcor->or_periodiclistbase,
		(unsigned long)ctrl->periodic_list);

	reg = ehci_readl(&ctrl->hccr->cr_hcsparams);
	descriptor.hub.bNbrPorts = HCS_N_PORTS(reg);
#if defined(USB_DEBUG_EHCI_ROOT)
	USB_DEBUG("Register %lx NbrPorts %d\n", reg, descriptor.hub.bNbrPorts);
#endif
	/* Port Indicators */
	if (HCS_INDICATOR(reg))
		put_unaligned(get_unaligned(&descriptor.hub.wHubCharacteristics)
				| 0x80, &descriptor.hub.wHubCharacteristics);
	/* Port Power Control */
	if (HCS_PPC(reg))
		put_unaligned(get_unaligned(&descriptor.hub.wHubCharacteristics)
				| 0x01, &descriptor.hub.wHubCharacteristics);

	/* Start the host controller. */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	/*
	 * Philips, Intel, and maybe others need CMD_RUN before the
	 * root hub will detect new devices (why?); NEC doesn't
	 */
	cmd &= ~(CMD_LRESET|CMD_IAAD|CMD_PSE|CMD_ASE|CMD_RESET_HC);
	cmd |= CMD_RUN;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);

	if (!(tweaks & EHCI_TWEAK_NO_INIT_CF)) {
		/* take control over the ports */
		cmd = ehci_readl(&ctrl->hcor->or_configflag);
		cmd |= FLAG_CF;
		ehci_writel(&ctrl->hcor->or_configflag, cmd);
	}

	/* unblock posted write */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	mdelay(5);
	reg = HC_VERSION(ehci_readl(&ctrl->hccr->cr_capbase));
#if defined(USB_DEBUG_EHCI_ROOT)
	printf("USB EHCI %lx.%02lx\n", reg >> 8, reg & 0xff);
#endif

	return 0;
}

int ehci_hcd_stop(int index)
{
	(void)index;
	return 0;
}

int usb_lowlevel_stop(int index)
{
	ehci_shutdown(&ehcic[index]);
	return ehci_hcd_stop(index);
}

int usb_lowlevel_init(int index, enum usb_init_type init, void **controller)
{
	struct ehci_ctrl *ctrl = &ehcic[index];
	unsigned int tweaks = 0;
	int rc;

	/**
	 * Set ops to default_ehci_ops, ehci_hcd_init should call
	 * ehci_set_controller_priv to change any of these function pointers.
	 */
	ctrl->ops = default_ehci_ops;

	rc = ehci_hcd_init(index, init, &ctrl->hccr, &ctrl->hcor);
	if (rc) {
		USB_DEBUG("[usb_lowlevel_init] rc: %d\n",rc);
		return rc;
	}
	if (!ctrl->hccr || !ctrl->hcor) {
		printf("[usb_lowlevel_init] hccr or hcor missing: %p %p\n",ctrl->hccr,ctrl->hcor);
		return -1;
	}
	if (init == USB_INIT_DEVICE)
		goto done;

	/* Zynq+ULPI PHY bring-up: ensure ULPI (USB3320) is awake and in host mode before EHCI reset */
	{
		static struct zynq_ehci_priv zynq_priv;
		int zret = ehci_zynq_probe(&zynq_priv);
		if (zret) {
			printf("[usb_lowlevel_init] ehci_zynq_probe (ULPI init) failed: %d\n", zret);
		} else {
			/* Optionally record priv for later use */
			ctrl->priv = &zynq_priv;
		}
	}

	/* EHCI spec section 4.1 */
	if (ehci_reset(ctrl)) {
		printf("[usb_lowlevel_init] ehci_reset failed\n");
		return -1;
	}

#if defined(CONFIG_EHCI_HCD_INIT_AFTER_RESET)
	rc = ehci_hcd_init(index, init, &ctrl->hccr, &ctrl->hcor);
	if (rc)
		return rc;
#endif
	rc = ehci_common_init(ctrl, tweaks);
	if (rc) {
		printf("[usb_lowlevel_init] ehci_common_init failed\n");
		return rc;
	}

	ctrl->rootdev = 0;
done:
	*controller = &ehcic[index];
	return 0;
}

static int _ehci_submit_bulk_msg(struct usb_device *dev, unsigned long pipe,
				 void *buffer, int length)
{

	if (usb_pipetype(pipe) != PIPE_BULK) {
		printf("non-bulk pipe (type=%lu)", usb_pipetype(pipe));
		return -1;
	}
	return ehci_submit_async(dev, pipe, buffer, length, NULL);
}

static int _ehci_submit_control_msg(struct usb_device *dev, unsigned long pipe,
				    void *buffer, int length,
				    struct devrequest *setup)
{
	struct ehci_ctrl *ctrl = ehci_get_ctrl(dev);

	if (usb_pipetype(pipe) != PIPE_CONTROL) {
		printf("non-control pipe (type=%lu)", usb_pipetype(pipe));
		return -1;
	}

  /* Check if this is a request for the root hub */
  if (usb_pipedevice(pipe) == ctrl->rootdev) {
		if (!ctrl->rootdev)
			dev->speed = USB_SPEED_HIGH;
		return ehci_submit_root(dev, pipe, buffer, length, setup);
	}

	/* Intercept problematic string descriptor requests for non-root devices */
	if (setup && (setup->requesttype & USB_DIR_IN) &&
	    setup->request == USB_REQ_GET_DESCRIPTOR &&
	    ((le16_to_cpu(setup->value) >> 8) == USB_DT_STRING)) {
		
		uint8_t string_index = le16_to_cpu(setup->value) & 0xff;
		
		/* Check for potentially problematic high string descriptor indices */
		if (string_index >= 8) {
			printf("[EHCI] Intercepting risky string descriptor request: dev=%lu index=%u\n",
			       usb_pipedevice(pipe), string_index);
			
			/* Return a minimal valid empty string descriptor */
			if (buffer && length >= 2) {
				uint8_t *buf = (uint8_t *)buffer;
				buf[0] = 2;    /* bLength: minimal descriptor length */
				buf[1] = USB_DT_STRING;  /* bDescriptorType: STRING */
				/* No string data, just the header */
				
				/* Flush cache to ensure 68k sees the data */
				flush_dcache_range((unsigned long)buffer,
						   ALIGN((unsigned long)buffer + 2, ARCH_DMA_MINALIGN));
				
				dev->act_len = 2;
				dev->status = 0;
				return 0;
			} else {
				/* Buffer too small or NULL, return error */
				dev->act_len = 0;
				dev->status = USB_ST_BUF_ERR;
				return -1;
			}
		}
	}

	return ehci_submit_async(dev, pipe, buffer, length, setup);
}

struct int_queue {
	int elementsize;
	unsigned long pipe;
	struct QH *first;
	struct QH *current;
	struct QH *last;
	struct qTD *tds;
	int queuesize;            /* total TD/QH entries in circular queue */
	void *qh_raw_ptr;         /* raw pointer for manual alignment cleanup */
	void *tds_raw_ptr;        /* raw pointer for TDs manual alignment cleanup */
	int owns_td_buffers;      /* 1 if this queue allocated TD buffers internally */
};

#define NEXT_QH(qh) (struct QH *)((unsigned long)hc32_to_cpu((qh)->qh_link) & ~0x1f)

static int
enable_periodic(struct ehci_ctrl *ctrl)
{
	uint32_t cmd;
	struct ehci_hcor *hcor = ctrl->hcor;
	int ret;

	cmd = ehci_readl(&hcor->or_usbcmd);
	cmd |= CMD_PSE;
	ehci_writel(&hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&hcor->or_usbsts,
			STS_PSS, STS_PSS, 100 * 1000);
	if (ret < 0) {
		printf("EHCI failed: timeout when enabling periodic list\n");
		return -ETIMEDOUT;
	}
	udelay(1000);
	return 0;
}

static int
disable_periodic(struct ehci_ctrl *ctrl)
{
	uint32_t cmd;
	struct ehci_hcor *hcor = ctrl->hcor;
	int ret;

	cmd = ehci_readl(&hcor->or_usbcmd);
	cmd &= ~CMD_PSE;
	ehci_writel(&hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&hcor->or_usbsts,
			STS_PSS, 0, 100 * 1000);
	if (ret < 0) {
		printf("EHCI failed: timeout when disabling periodic list\n");
		return -ETIMEDOUT;
	}
	return 0;
}

static struct int_queue *_ehci_create_int_queue(struct usb_device *dev,
		unsigned long pipe, int queuesize, int elementsize,
		void *buffer, int interval)
{
	struct ehci_ctrl *ctrl = ehci_get_ctrl(dev);
	struct int_queue *result = NULL;
	uint32_t i, toggle;

	/* DEBUG: Show why interrupt queue creation might fail */
    printf("[EHCI DEBUG] _ehci_create_int_queue: dev=%u elementsize=%d queuesize=%d interval=%d\n", dev->devnum, elementsize, queuesize, interval);

    /* Clamp invalid/zero interval to 1 to avoid edge-case behavior */
    if (interval <= 0)
        interval = 1;
    
    /* For full-speed devices with split transactions, ensure proper timing */
    if (dev->speed != USB_SPEED_HIGH && interval < 8) {
        printf("[EHCI SPLIT] FS/LS device interval adjusted from %d to 8 for split timing\n", interval);
        interval = 8;  /* Minimum interval for reliable split transactions */
    }


	/*
	 * Interrupt transfers requiring several transactions are not supported
	 * because bInterval is ignored.
	 *
	 * Also, ehci_submit_async() relies on wMaxPacketSize being a power of 2
	 * <= PKT_ALIGN if several qTDs are required, while the USB
	 * specification does not constrain this for interrupt transfers. That
	 * means that ehci_submit_async() would support interrupt transfers
	 * requiring several transactions only as long as the transfer size does
	 * not require more than a single qTD.
	 */
	int size=usb_maxpacket(dev, pipe);
//	printf("[EHCI DEBUG] maxpacket=%d elementsize=%d pipe_type=%lu\n", size, elementsize, usb_pipetype(pipe));
	if (elementsize > size) {
//		printf("[EHCI ADJUST] elementsize %d > maxpacket %d - adjusting to %d\n", elementsize, size, size);
		elementsize = size;  /* Use maxpacket instead of failing */
	}

	if (usb_pipetype(pipe) != PIPE_INTERRUPT) {
		printf("[EHCI ERROR] non-interrupt pipe (type=%lu) - FAILING\n", usb_pipetype(pipe));
		return NULL;
	}

	/* limit to 4 full pages worth of data -
	 * we can safely fit them in a single TD,
	 * no matter the alignment
	 */
	if (elementsize >= 16384) {
		printf("too large elements for interrupt transfers\n");
		return NULL;
	}

	result = malloc(sizeof(*result));  // FIXME dynamic allocation
	if (!result) {
		printf("ehci intr queue: out of memory\n");
		goto fail1;
	}
	
	result->elementsize = elementsize;
	result->pipe = pipe;
	result->owns_td_buffers = (buffer == NULL) ? 1 : 0;
	/* Allocate extra space for alignment */
	size_t qh_size = sizeof(struct QH) * queuesize;
	void *qh_raw = malloc(qh_size + USB_DMA_MINALIGN - 1);
	if (!qh_raw) {
		printf("ehci intr queue: out of memory for QH raw buffer\n");
		goto fail2;
	}
	/* Manually align the pointer */
	result->first = (struct QH *)ALIGN((unsigned long)qh_raw, USB_DMA_MINALIGN);
	/* Store raw pointer for later free() */
	result->qh_raw_ptr = qh_raw;
	if (!result->first) {
		printf("ehci intr queue: out of memory\n");
		goto fail2;
	}
	
	result->current = result->first;
	result->last = result->first + queuesize - 1;
	result->queuesize = queuesize;
	/* Allocate extra space for alignment */
	size_t tds_size = sizeof(struct qTD) * queuesize;
	void *tds_raw = malloc(tds_size + USB_DMA_MINALIGN - 1);
	if (!tds_raw) {
		printf("ehci intr queue: out of memory for TDs raw buffer\n");
		goto fail3;
	}
	/* Manually align the pointer */
	result->tds = (struct qTD *)ALIGN((unsigned long)tds_raw, USB_DMA_MINALIGN);
	/* Store raw pointer for later free() */
	result->tds_raw_ptr = tds_raw;
	if (!result->tds) {
		printf("ehci intr queue: out of memory\n");
		goto fail3;
	}
	
	memset(result->first, 0, sizeof(struct QH) * queuesize);
	memset(result->tds, 0, sizeof(struct qTD) * queuesize);

	toggle = usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	for (i = 0; i < (unsigned int)queuesize; i++) {
		struct QH *qh = result->first + i;
		struct qTD *td = result->tds + i;
		void **buf = &qh->buffer;

		qh->qh_link = cpu_to_hc32((unsigned long)(qh+1) | QH_LINK_TYPE_QH);
		if (i == (unsigned int)(queuesize - 1))
			qh->qh_link = cpu_to_hc32((unsigned long)result->first | QH_LINK_TYPE_QH);

		qh->qh_overlay.qt_next = cpu_to_hc32((unsigned long)td);
		qh->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		qh->qh_endpt1 =
			cpu_to_hc32((0 << 28) | /* No NAK reload (ehci 4.9) */
			(usb_maxpacket(dev, pipe) << 16) | /* MPS */
			(1 << 14) |
			QH_ENDPT1_EPS(ehci_encode_speed(dev->speed)) |
			(usb_pipeendpoint(pipe) << 8) | /* Endpoint Number */
			(usb_pipedevice(pipe) << 0));
		qh->qh_endpt2 = cpu_to_hc32((1 << 30)); /* 1 Tx per mframe */
		
		/* Configure S-mask and C-mask for split transactions */
		/* Reduced logging: only show first time for each device */
		static int logged_devices[128] = {0}; /* Track which devices we've logged */
		if (dev->devnum < 128 && !logged_devices[dev->devnum]) {
			USB_DEBUG("[EHCI INT] Device %d speed=%d configured for interrupts\n", dev->devnum, dev->speed);
			logged_devices[dev->devnum] = 1;
		}
		if (dev->speed == USB_SPEED_LOW || dev->speed == USB_SPEED_FULL) {
			/* For FS/LS devices, configure split transaction microframes */
			/* S-mask: start split in microframe 0 (bit 0 = 0x01) */
			/* C-mask: complete split in microframes 2-4 (bits 2-4 = 0x1c) for FS/LS */
			qh->qh_endpt2 |= cpu_to_hc32(0x01 << 0);   /* S-mask: microframe 0 */
			qh->qh_endpt2 |= cpu_to_hc32(0x1c << 8);   /* C-mask: microframes 2,3,4 */
		} else {
			/* High-Speed devices: no split transactions needed */
			qh->qh_endpt2 |= cpu_to_hc32(0x00 << 0);   /* S-mask: 0 (no split) */
			qh->qh_endpt2 |= cpu_to_hc32(0x00 << 8);   /* C-mask: 0 (no split) */
		}
		ehci_update_endpt2_dev_n_port(dev, qh);

		td->qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		td->qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
//		printf("communication direction is '%s'\n",
//		      usb_pipein(pipe) ? "in" : "out");
		td->qt_token = cpu_to_hc32(
			QT_TOKEN_DT(toggle) |
			QT_TOKEN_TOTALBYTES(elementsize) |
			QT_TOKEN_PID(usb_pipein(pipe) ? QT_TOKEN_PID_IN : QT_TOKEN_PID_OUT) |
			QT_TOKEN_CERR(3) |
			QT_TOKEN_STATUS_ACTIVE);

		/* Allocate aligned buffer for this queue entry if not provided */
		void *td_buffer;
		if (buffer) {
			/* For provided buffers, use offset into the buffer for each entry */
			td_buffer = buffer + i * elementsize;
		} else {
			/* Use simple malloc for individual TD buffers - alignment will be handled by ehci_td_buffer */
			td_buffer = malloc(elementsize);
			if (!td_buffer) {
				printf("ehci intr queue: unable to allocate TD buffer\n");
				goto fail3;
			}
			/* Store the buffer pointer in the QH for later cleanup */
			memset(td_buffer, 0, elementsize);
		}
		
		if (ehci_td_buffer(td, td_buffer, elementsize)) {
			printf("ehci intr queue: unable to construct TD\n");
			if (!buffer) free(td_buffer); /* Free if we allocated it */
			goto fail3;
		}

		td->qt_buffer[0] = cpu_to_hc32((unsigned long)td_buffer);
		td->qt_buffer[1] = cpu_to_hc32((td->qt_buffer[0] + 0x1000) & ~0xfff);
		td->qt_buffer[2] = cpu_to_hc32((td->qt_buffer[0] + 0x2000) & ~0xfff);
		td->qt_buffer[3] = cpu_to_hc32((td->qt_buffer[0] + 0x3000) & ~0xfff);
		td->qt_buffer[4] = cpu_to_hc32((td->qt_buffer[0] + 0x4000) & ~0xfff);

		*buf = td_buffer;
		toggle ^= 1;
	}

	/* Only flush if we have a provided buffer */
	if (buffer) {
		flush_dcache_range((unsigned long)buffer,
				   ALIGN_END_ADDR(char, buffer,
						  queuesize * elementsize));
	}
	flush_dcache_range((unsigned long)result->first,
			   ALIGN_END_ADDR(struct QH, result->first,
					  queuesize));
	flush_dcache_range((unsigned long)result->tds,
			   ALIGN_END_ADDR(struct qTD, result->tds,
					  queuesize));

	if (ctrl->periodic_schedules > 0) {
		if (disable_periodic(ctrl) < 0) {
			printf("FATAL: periodic should never fail, but did");
			goto fail3;
		}
	}

	/* hook up to periodic list */
	struct QH *list = &ctrl->periodic_queue;
	/* Insert our circular queue into the periodic schedule */
	list->qh_link = cpu_to_hc32((unsigned long)result->first | QH_LINK_TYPE_QH);
	/* Note: result->last->qh_link already points to result->first (circular) */

	flush_dcache_range((unsigned long)result->last,
			   ALIGN_END_ADDR(struct QH, result->last, 1));
	flush_dcache_range((unsigned long)list,
			   ALIGN_END_ADDR(struct QH, list, 1));

	if (enable_periodic(ctrl) < 0) {
		printf("FATAL: periodic should never fail, but did");
		goto fail3;
	}
	ctrl->periodic_schedules++;

//	printf("Exit create_int_queue\n");
	return result;
fail3:
	/* CRITICAL: Free individual TD buffers that were allocated before failure */
	if (result && result->first && result->owns_td_buffers) {
		/* We need to free any TD buffers that were allocated before the failure */
		/* Count how many QH entries we successfully created */
		int created_qhs = 0;
		for (int j = 0; j < queuesize; j++) {
			struct QH *qh = result->first + j;
			if (qh->buffer) {
				created_qhs++;
				free(qh->buffer);
				qh->buffer = NULL;
			} else {
				break; /* No more buffers allocated */
			}
		}
		USB_DEBUG("[EHCI] Freed %d TD buffers during error cleanup\n", created_qhs);
	}
	
	if (result->tds_raw_ptr)
		free(result->tds_raw_ptr);  /* Free the raw pointer for TDs */
	else if (result->tds)
		free(result->tds);         /* Fallback if tds_raw_ptr is NULL */
fail2:
	if (result->qh_raw_ptr)
		free(result->qh_raw_ptr);  /* Free the raw pointer, not the aligned one */
	else if (result->first)
		free(result->first);       /* Fallback if qh_raw_ptr is NULL */
	if (result)
		free(result);
fail1:
	return NULL;
}

static void *_ehci_poll_int_queue(struct usb_device *dev,
				  struct int_queue *queue)
{
	struct QH *cur = queue->current;
	struct qTD *cur_td;
	uint32_t token, toggle;
	unsigned long pipe = queue->pipe;
	int td_index;
	void *buffer_data;

	/* depleted queue */
	if (cur == NULL) {
		return NULL;
	}

	td_index = cur - queue->first;
	cur_td = &queue->tds[td_index];

	invalidate_dcache_range((unsigned long)cur_td, ALIGN_END_ADDR(struct qTD, cur_td, 1));
	token = hc32_to_cpu(cur_td->qt_token);
/*
	printf("[DEBUG] TD[%d]: token=0x%08x status=0x%02x active=%d bytes=%d DT=%d\n",
	       td_index, token, QT_TOKEN_GET_STATUS(token),
	       (QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE) ? 1 : 0,
	       QT_TOKEN_GET_TOTALBYTES(token), QT_TOKEN_GET_DT(token));
*/
	/* Check if TD completed */
	if (QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE) {
		return NULL;
	}

	toggle = QT_TOKEN_GET_DT(token);
	/* Check TD completion status */
	uint32_t status = QT_TOKEN_GET_STATUS(token);
	
	if (status & QT_TOKEN_STATUS_ACTIVE) {
		/* TD still active - not completed yet */
		return NULL;
	}
	
	/* Check for serious errors that would prevent further transfers */
	if (status & (QT_TOKEN_STATUS_HALTED | QT_TOKEN_STATUS_BABBLEDET | QT_TOKEN_STATUS_DATBUFERR)) {
		#ifdef USB_DEBUG_VERBOSE
		printf("[EHCI POLL] TD[%d] FATAL ERROR: status=0x%02lx\n", td_index, (unsigned long)status);
		#endif
		dev->status = USB_ST_STALLED; /* Serious error */
		dev->act_len = 0;
		return NULL; /* Return NULL to indicate error */
	}
	
	/* For MISSEDUFRAME and other non-fatal conditions, continue processing */
	if (status & QT_TOKEN_STATUS_MISSEDUFRAME) {
		#ifdef USB_DEBUG_VERBOSE
		printf("[EHCI POLL] TD[%d] MISSEDUFRAME (0x%02lx) - continuing anyway\n", 
		       td_index, (unsigned long)status);
		#endif
		/* MISSEDUFRAME is often recoverable - don't treat as fatal */
	}
	
	/* TD completed successfully - even if it returned zero data */
	#ifdef USB_DEBUG_VERBOSE
	printf("[EHCI POLL] TD[%d] COMPLETED SUCCESSFULLY: status=0x%02lx bytes_transferred=%ld\n",
	       td_index, (unsigned long)status, 
	       (long)(queue->elementsize - QT_TOKEN_GET_TOTALBYTES(token)));
	#endif

	/* TD completed successfully */
	#ifdef USB_DEBUG_VERBOSE
	printf("[EHCI POLL] TD[%d] COMPLETED: DT=%ld status=0x%02lx bytes_transferred=%ld\n",
	       td_index, (unsigned long)toggle, (unsigned long)status, 
	       (long)(queue->elementsize - QT_TOKEN_GET_TOTALBYTES(token)));
	#endif
	
	/* Update device toggle based on completed transfer */
	uint32_t next_device_toggle = toggle ^ 1;
	#ifdef USB_DEBUG_VERBOSE
	printf("[EHCI POLL] TD[%d] toggle: was %ld, setting device to %ld\n",
	       td_index, (unsigned long)toggle, (unsigned long)next_device_toggle);
	#endif
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), next_device_toggle);

	/* Report completion to upper layers */
	int transferred = queue->elementsize - QT_TOKEN_GET_TOTALBYTES(token);
	if (transferred < 0)
		transferred = 0;
	dev->act_len = transferred;
	dev->status = 0;

	/* Read the data */
	invalidate_dcache_range((unsigned long)cur->buffer, 
			   ALIGN_END_ADDR(char, cur->buffer, queue->elementsize));

	/* Buffer data debugging disabled to reduce log spam */
	#ifdef USB_DEBUG_TD_BUFFER
	uint8_t *raw_data = (uint8_t *)cur->buffer;
	USB_DEBUG("[DEBUG] TD[%d] buffer data: %02x %02x %02x %02x %02x %02x\n",
	       td_index, raw_data[0], raw_data[1], raw_data[2], 
	       raw_data[3], raw_data[4], raw_data[5]);
	#endif
	/* Store the buffer data to return */
	buffer_data = cur->buffer;
	
	/* IMPORTANT: Advance to next TD for multi-element queues */
	queue->current = cur == queue->last ? NULL : (cur + 1);
	
	return buffer_data;
}

static int _ehci_destroy_int_queue(struct usb_device *dev,
				   struct int_queue *queue)
{
	struct ehci_ctrl *ctrl = ehci_get_ctrl(dev);
	int result = -1;
	unsigned long timeout;
	int i;

	/* Validate input parameters */
	if (!dev || !queue || !ctrl) {
		printf("[EHCI] Invalid parameters in _ehci_destroy_int_queue\n");
		return -1;
	}

	if (disable_periodic(ctrl) < 0) {
		USB_DEBUG("[EHCI] Warning: periodic disable failed, continuing cleanup\n");
		/* Don't abort - continue with cleanup */
	}
	ctrl->periodic_schedules--;

	/* Try to find and remove from periodic list - but don't get stuck */
	struct QH *cur = &ctrl->periodic_queue;
    timeout = get_timer(0) + 5000; /* Increased timeout to 5000ms (5 seconds) for USB3320C */
	int iterations = 0;
	const int max_iterations = 32; /* Prevent infinite loops */
	
	while (!(cur->qh_link & cpu_to_hc32(QH_LINK_TERMINATE)) && iterations < max_iterations) {
		if (NEXT_QH(cur) == queue->first) {
			/* Found it - remove from chain */
			cur->qh_link = queue->last->qh_link;
			flush_dcache_range((unsigned long)cur,
					   ALIGN_END_ADDR(struct QH, cur, 1));
			result = 0;
			break;
		}
		cur = NEXT_QH(cur);
		iterations++;
		
		if (get_timer(0) > timeout) {
			printf("[EHCI] Timeout finding queue in periodic list, forcing cleanup\n");
			result = 0; /* Force success to continue cleanup */
			break;
		}
	}

	if (iterations >= max_iterations) {
//		printf("[EHCI] Hit iteration limit finding queue, forcing cleanup\n");
		result = 0; /* Force success */
	}

	if (ctrl->periodic_schedules > 0) {
		int enable_result = enable_periodic(ctrl);
		if (enable_result < 0) {
			printf("[EHCI] Warning: periodic enable failed\n");
			/* Don't change result - continue with cleanup */
		}
	}

	/* Free individual TD buffers ONLY if they were allocated internally */
	if (queue && queue->owns_td_buffers && queue->first && queue->queuesize > 0) {
		for (i = 0; i < queue->queuesize; i++) {
			struct QH *qh = queue->first + i;
			/* ONLY free buffers that were malloc'd internally */
			/* If owns_td_buffers=1, then buffer was NOT provided externally */
			if (qh && qh->buffer) {
				free(qh->buffer);
				qh->buffer = NULL;
			}
		}
	}
	
	/* Safe cleanup of TDs */
	if (queue) {
		if (queue->tds_raw_ptr) {
			free(queue->tds_raw_ptr);  /* Free the raw pointer for TDs */
			queue->tds_raw_ptr = NULL;
		} else if (queue->tds) {
			free(queue->tds);         /* Fallback if tds_raw_ptr is NULL */
			queue->tds = NULL;
		}
		
		/* Safe cleanup of QHs */
		if (queue->qh_raw_ptr) {
			free(queue->qh_raw_ptr);  /* Free the raw pointer, not the aligned one */
			queue->qh_raw_ptr = NULL;
		} else if (queue->first) {
			free(queue->first);       /* Fallback if qh_raw_ptr is NULL */
			queue->first = NULL;
		}
		
		/* Clear all pointers before freeing queue */
		queue->current = NULL;
		queue->last = NULL;
		free(queue);
	}

	return result;
}

static int _ehci_submit_int_msg(struct pt *pt, struct usb_device *dev, unsigned long pipe,
				void *buffer, int length, int interval)
{
	struct int_queue *queue;
	void *poll_result;
	unsigned long start_time;
	int ret = 0;

	/* Create interrupt queue */
	queue = _ehci_create_int_queue(dev, pipe, 1, length, buffer, interval);
	if (!queue)
		return -1;

	/* Simple polling with timeout - no complex protothreads */
	start_time = get_timer(0);
	poll_result = NULL;
	PT_BEGIN(pt);
	/* Poll for data with timeout */
	PT_WAIT_UNTIL(pt, (
		(poll_result = _ehci_poll_int_queue(dev, queue)) != NULL ||
		(get_timer(start_time) > USB_SIMPLE_TIMEOUT_MS)
	));
	PT_END(pt);

	if (poll_result != NULL) {
		/* Success - data received */
		ret = 0;
	} else {
		/* Timeout - treat as success with no data */
		dev->act_len = 0;
		dev->status = 0;
		ret = 0;
	}

	/* Cleanup queue */
	_ehci_destroy_int_queue(dev, queue);
	
	return ret;
}

int submit_bulk_msg(struct usb_device *dev, unsigned long pipe,
			    void *buffer, int length)
{
	return _ehci_submit_bulk_msg(dev, pipe, buffer, length);
}

int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int length, struct devrequest *setup)
{
	return _ehci_submit_control_msg(dev, pipe, buffer, length, setup);
}

int submit_int_msg(struct pt *pt, struct usb_device *dev, unsigned long pipe,
		   void *buffer, int length, int interval)
{
	return _ehci_submit_int_msg(pt, dev, pipe, buffer, length, interval);
}

struct int_queue *create_int_queue(struct usb_device *dev,
		unsigned long pipe, int queuesize, int elementsize,
		void *buffer, int interval)
{
//   printf("dev=%p, pipe=%lu, buffer=%p, elementsize=%d, interval=%d\n",
//         dev, pipe, buffer, elementsize, interval);

	return _ehci_create_int_queue(dev, pipe, queuesize, elementsize,
				      buffer, interval);
}

void *poll_int_queue(struct usb_device *dev, struct int_queue *queue)
{
	return _ehci_poll_int_queue(dev, queue);
}

int destroy_int_queue(struct usb_device *dev, struct int_queue *queue)
{
	return _ehci_destroy_int_queue(dev, queue);
}

int ehci_register(struct ehci_ctrl *ctrl, struct ehci_hccr *hccr,
		  struct ehci_hcor *hcor, const struct ehci_ops *ops,
		  unsigned int tweaks, enum usb_init_type init)
{
	//struct usb_bus_priv *priv = dev_get_uclass_priv(dev);
	//struct ehci_ctrl *ctrl = dev_get_priv(dev);
	(void)tweaks;	

	int ret = -1;

	USB_DEBUG("%s: dev='%s', ctrl=%p, hccr=%p, hcor=%p, init=%d\n", __func__,
	      "zynq-ehci", ctrl, hccr, hcor, init);

	if (!ctrl || !hccr || !hcor)
		goto err;

	// FIXME
	//priv->desc_before_addr = true;

	ehci_setup_ops(ctrl, ops);
	ctrl->hccr = hccr;
	ctrl->hcor = hcor;
	ctrl->priv = ctrl;

	ctrl->init = init;
	if (ctrl->init == USB_INIT_DEVICE)
		goto done;

	ret = ehci_reset(ctrl);
	if (ret)
		goto err;

	if (ctrl->ops.init_after_reset) {
		ret = ctrl->ops.init_after_reset(ctrl);
		if (ret)
			goto err;
	}

	//ret = ehci_common_init(ctrl, tweaks);
	if (ret)
		goto err;
done:
	return 0;
err:
	free(ctrl);
	printf("%s: failed, ret=%d\n", __func__, ret);
	return ret;
}

#ifdef CONFIG_PHY
int ehci_setup_phy(struct udevice *dev, struct phy *phy, int index)
{
	int ret;

	if (!phy)
		return 0;

	ret = generic_phy_get_by_index(dev, index, phy);
	if (ret) {
		if (ret != -ENOENT) {
			dev_err(dev, "failed to get usb phy\n");
			return ret;
		}
	} else {
		ret = generic_phy_init(phy);
		if (ret) {
			dev_err(dev, "failed to init usb phy\n");
			return ret;
		}

		ret = generic_phy_power_on(phy);
		if (ret) {
			dev_err(dev, "failed to power on usb phy\n");
			return generic_phy_exit(phy);
		}
	}

	return 0;
}

int ehci_shutdown_phy(struct udevice *dev, struct phy *phy)
{
	int ret = 0;

	if (!phy)
		return 0;

	if (generic_phy_valid(phy)) {
		ret = generic_phy_power_off(phy);
		if (ret) {
			dev_err(dev, "failed to power off usb phy\n");
			return ret;
		}

		ret = generic_phy_exit(phy);
		if (ret) {
			dev_err(dev, "failed to power off usb phy\n");
			return ret;
		}
	}

	return 0;
}
#else
int ehci_setup_phy(struct udevice *dev, struct phy *phy, int index)
{
	(void)dev;
	(void)phy;
	(void)index;
	return 0;
}

int ehci_shutdown_phy(struct udevice *dev, struct phy *phy)
{
	(void)dev;
	(void)phy;
	return 0;
}
#endif
