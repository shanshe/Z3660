// SPDX-License-Identifier: GPL-2.0+
/*
 * Most of this source has been derived from the Linux USB
 * project:
 * (C) Copyright Linus Torvalds 1999
 * (C) Copyright Johannes Erdfelt 1999-2001
 * (C) Copyright Andreas Gal 1999
 * (C) Copyright Gregory P. Smith 1999
 * (C) Copyright Deti Fliegl 1999 (new USB architecture)
 * (C) Copyright Randy Dunlap 2000
 * (C) Copyright David Brownell 2000 (kernel hotplug, usb_device_id)
 * (C) Copyright Yggdrasil Computing, Inc. 2000
 *     (usb_device_id matching changes by Adam J. Richter)
 *
 * Adapted for U-Boot:
 * (C) Copyright 2001 Denis Peter, MPL AG Switzerland
 */

/*
 * How it works:
 *
 * Since this is a bootloader, the devices will not be automatic
 * (re)configured on hotplug, but after a restart of the USB the
 * device should work.
 *
 * For each transfer (except "Interrupt") we wait for completion.
 */
#include "memalign.h"
#include <ctype.h>
#include "asm/byteorder.h"
#include <errno.h>
#include "usb.h"
#include "ehci.h"
#include <stdio.h>
#include <string.h>
#include <sleep.h>
#include "pt/pt.h"

void mdelay(int ms);

static inline void *malloc_cache_aligned(size_t size)
{
	return memalign(ARCH_DMA_MINALIGN, ALIGN(size, ARCH_DMA_MINALIGN));
}

#define USB_BUFSIZ	512

static int asynch_allowed;
char usb_started; /* flag for the started/stopped USB status */

static struct usb_device usb_dev[USB_MAX_DEVICE];
static int dev_index;
static void *poseidon_ctrl = NULL;

#define CONFIG_USB_MAX_CONTROLLER_COUNT 1

/***************************************************************************
 * Init USB Device
 */
int usb_init(void)
{
	void *ctrl;
	struct usb_device *dev;
	int i, start_index = 0;
	int controllers_initialized = 0;
	int ret;

	dev_index = 0;
	asynch_allowed = 1;
	usb_hub_reset();

	/* first make all devices unknown */
	for (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, sizeof(struct usb_device));
		usb_dev[i].devnum = -1;
	}

	/* init low_level USB */
	for (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++) {
		/* init low_level USB */
		printf("USB%d:   ", i);
		ret = usb_lowlevel_init(i, USB_INIT_HOST, &ctrl);
		if (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			continue;
		}

		if (ret) {		/* Other error. */
			puts("lowlevel init failed\n");
			continue;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		start_index = dev_index;
		printf("[usb] scanning bus %d for devices... ", i);
		ret = usb_alloc_new_device(ctrl, &dev);
		if (ret)
			break;

		/*
		 * device 0 is always present
		 * (root hub, so let it analyze)
		 */
		ret = usb_new_device(dev);
		if (ret)
			usb_free_device(dev->controller);

		if (start_index == dev_index) {
			puts("[usb] No USB Device found\n");
			continue;
		} else {
			printf("[usb] %d USB Device(s) found\n",
				dev_index - start_index);
		}

		usb_started = 1;
	}

	/* if we were not able to find at least one working bus, bail out */
	if (controllers_initialized == 0)
		puts("USB error: all controllers failed lowlevel init\n");

	return usb_started ? 0 : -ENODEV;
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
int usb_stop(void)
{
	int i;

	if (usb_started) {
		asynch_allowed = 1;
		usb_started = 0;
		usb_hub_reset();

		for (i = 0; i < CONFIG_USB_MAX_CONTROLLER_COUNT; i++) {
			if (usb_lowlevel_stop(i))
				printf("failed to stop USB controller %d\n", i);
		}
	}

	return 0;
}

/******************************************************************************
 * Detect if a USB device has been plugged or unplugged.
 */
int usb_detect_change(void)
{
	int i, j;
	int change = 0;

	for (j = 0; j < USB_MAX_DEVICE; j++) {
		for (i = 0; i < usb_dev[j].maxchild; i++) {
			struct usb_port_status status;

			if (usb_get_port_status(&usb_dev[j], i + 1,
						&status) < 0)
				/* USB request failed */
				continue;

			if (le16_to_cpu(status.wPortChange) &
			    USB_PORT_STAT_C_CONNECTION)
				change++;
		}
	}

	return change;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
int usb_disable_asynch(int disable)
{
	int old_value = asynch_allowed;

	asynch_allowed = !disable;
	return old_value;
}

/*-------------------------------------------------------------------
 * Message wrappers.
 *
 */

/*
 * submits an Interrupt Message
 */
int usb_int_msg(struct pt *pt, struct usb_device *dev, unsigned long pipe,
			void *buffer, int transfer_len, int interval)
{
	return submit_int_msg(pt, dev, pipe, buffer, transfer_len, interval);
}

/*
 * submits a control message and waits for completion (at least timeout * 1ms)
 * If timeout is 0, we don't wait for completion (used as example to set and
 * clear keyboards LEDs). For data transfers, (storage transfers) we don't
 * allow control messages with 0 timeout, by previously resetting the flag
 * asynch_allowed (usb_disable_asynch(1)).
 * returns the transferred length if OK or -1 if error. The transferred length
 * and the current status are stored in the dev->act_len and dev->status.
 */
int usb_control_msg(struct usb_device *dev, unsigned int pipe,
			unsigned char request, unsigned char requesttype,
			unsigned short value, unsigned short index,
			void *data, unsigned short size, int timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct devrequest, setup_packet, 1);
	int err;
	int retries = 0;
	const int max_retries = 3;
	const int retry_delay = 20; /* ms */
	const int settle_delay = 5; /* ms */

	if ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		return -EINVAL;
	}

	/* Ensure any previous transfer is fully completed */
	if (dev->status & USB_ST_NOT_PROC) {
		printf("[USB] Warning: Previous transfer not complete (status=0x%lx)\n",
		       dev->status);
		mdelay(settle_delay);
		dev->status &= ~USB_ST_NOT_PROC;
	}

	do {
		/* Clear status before transfer */
		dev->status = USB_ST_NOT_PROC;


		/* set setup command */
		setup_packet->requesttype = requesttype;
		setup_packet->request = request;
		setup_packet->value = cpu_to_le16(value);
		setup_packet->index = cpu_to_le16(index);
		setup_packet->length = cpu_to_le16(size);

		/* Submit the control message */
		err = submit_control_msg(dev, pipe, data, size, setup_packet);

		/* Check submission result */
		if (err < 0) {
			printf("[USB] Control transfer submission failed on try %d/%d (err=%d)\n",
			       retries + 1, max_retries, err);
			if (retries < max_retries - 1) {
				mdelay(retry_delay);
				retries++;
				continue;
			}
			return err;
		}

		/* Wait for completion with timeout */
		int wait = timeout;
		while (wait > 0 && (dev->status & USB_ST_NOT_PROC)) {
			mdelay(1);
			wait--;
		}

		/* Check completion status */
		if (!(dev->status & USB_ST_NOT_PROC)) {
			/* Transfer completed successfully */
			break;
		}

		/* Timeout occurred - retry if possible */
		if (retries < max_retries - 1) {
			
			printf("[USB] Control transfer timeout on try %d/%d\n",
			       retries + 1, max_retries);
			mdelay(retry_delay);
			retries++;
			continue;
		}

		/* All retries exhausted */
		printf("[USB] Control transfer failed after %d attempts\n", max_retries);
		return -ETIMEDOUT;

	} while (retries < max_retries);

	/* Allow hardware to settle after transfer */
	mdelay(settle_delay);


	/* DEBUG: Show VID/PID for GET_DESCRIPTOR(DEVICE) requests */
	if ((requesttype & 0x80) && request == 0x06 && (value >> 8) == 0x01 && data && size >= 18) {
		unsigned char *raw_data = (unsigned char *)data;
		
		/* Show raw bytes to diagnose endianness issues */
		printf("[ARM USB DEBUG] Raw descriptor bytes 8-11: %02X %02X %02X %02X\n",
		       raw_data[8], raw_data[9], raw_data[10], raw_data[11]);
		       
		/* Try both endianness interpretations */
		unsigned short raw_vid_le = (unsigned short)(raw_data[8] | (raw_data[9] << 8));
		unsigned short raw_pid_le = (unsigned short)(raw_data[10] | (raw_data[11] << 8));
		unsigned short raw_vid_be = (unsigned short)(raw_data[9] | (raw_data[8] << 8));
		unsigned short raw_pid_be = (unsigned short)(raw_data[11] | (raw_data[10] << 8));
		
		printf("[ARM USB] GET_DESCRIPTOR(DEVICE) dev=%d: LE=VID:0x%04X PID:0x%04X BE=VID:0x%04X PID:0x%04X\n", 
		       dev->devnum, raw_vid_le, raw_pid_le, raw_vid_be, raw_pid_be);
	}

	/* Enhanced SET_ADDRESS processing with timing */
	if (request == USB_REQ_SET_ADDRESS && requesttype == 0x00) {
		uint8_t new_address = (uint8_t)value;
		printf("[USB] SET_ADDRESS processing: %d->%d\n", dev->devnum, new_address);
		
		/* Update device address */
		dev->devnum = new_address;
		
		/* Reset endpoint toggles for new address */
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		
		/* Allow device time to complete address transition */
		printf("[USB] SET_ADDRESS completed, allowing settling time...\n");
		mdelay(10); /* Final settling time */
	}

	return dev->act_len;
}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * negative if Error.
 * synchronous behavior
 */
int usb_bulk_msg(struct usb_device *dev, unsigned int pipe,
			void *data, int len, int *actual_length, int timeout)
{
	if (len < 0)
		return -EINVAL;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	if (submit_bulk_msg(dev, pipe, data, len) < 0)
		return -EIO;
	while (timeout--) {
		if (!((volatile unsigned long)dev->status & USB_ST_NOT_PROC))
			break;
		mdelay(1);
	}
	*actual_length = dev->act_len;
	if (dev->status == 0)
		return 0;
	else
		return -EIO;
}


/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
int usb_maxpacket(struct usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	if ((pipe & USB_DIR_IN) == 0)
		return dev->epmaxpacketout[((pipe>>15) & 0xf)];
	else
		return dev->epmaxpacketin[((pipe>>15) & 0xf)];
}

// FIXME: LOL

/*
 * The routine usb_set_maxpacket_ep() is extracted from the loop of routine
 * usb_set_maxpacket(), because the optimizer of GCC 4.x chokes on this routine
 * when it is inlined in 1 single routine. What happens is that the register r3
 * is used as loop-count 'i', but gets overwritten later on.
 * This is clearly a compiler bug, but it is easier to workaround it here than
 * to update the compiler (Occurs with at least several GCC 4.{1,2},x
 * CodeSourcery compilers like e.g. 2007q3, 2008q1, 2008q3 lite editions on ARM)
 *
 * NOTE: Similar behaviour was observed with GCC4.6 on ARMv5.
 */
static void usb_set_maxpacket_ep(struct usb_device *dev, int if_idx, int ep_idx)
{
	int b;
	struct usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize;

	ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	if ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep_wMaxPacketSize;
		dev->epmaxpacketin[b] = ep_wMaxPacketSize;
		printf("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} else {
		if ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			if (ep_wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep_wMaxPacketSize;
				//printf("##EP epmaxpacketout[%d] = %d\n",
				//      b, dev->epmaxpacketout[b]);
			}
		} else {
			/* IN Endpoint */
			if (ep_wMaxPacketSize > dev->epmaxpacketin[b]) {
				dev->epmaxpacketin[b] = ep_wMaxPacketSize;
				//printf("##EP epmaxpacketin[%d] = %d\n",
				//      b, dev->epmaxpacketin[b]);
			}
		} /* if out */
	} /* if control */
}

/*
 * set the max packed value of all endpoints in the given configuration
 */
static int usb_set_maxpacket(struct usb_device *dev)
{
	int i, ii;

	for (i = 0; i < dev->config.desc.bNumInterfaces; i++)
		for (ii = 0; ii < dev->config.if_desc[i].desc.bNumEndpoints; ii++)
			usb_set_maxpacket_ep(dev, i, ii);

	return 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
static int usb_parse_config(struct usb_device *dev,
			unsigned char *buffer, int cfgno)
{
	struct usb_descriptor_header *head;
	int index, ifno, epno, curr_if_num;
	u16 ep_wMaxPacketSize;
	struct usb_interface *if_desc = NULL;

	ifno = -1;
	epno = -1;
	curr_if_num = -1;

	dev->configno = cfgno;
	head = (struct usb_descriptor_header *) &buffer[0];
	if (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		return -EINVAL;
	}
	if (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		return -EINVAL;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (struct usb_descriptor_header *) &buffer[index];
	while (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		// printf("[usb_parse_config] DescriptorType: 0x%x, Length: %d\n", head->bDescriptorType, head->bLength);
		switch (head->bDescriptorType) {
		case USB_DT_INTERFACE:
			if (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				break;
			}
			if (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				break;
			}
			if (((struct usb_interface_descriptor *) \
			     head)->bInterfaceNumber != curr_if_num) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				if (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					return -EINVAL;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->num_altsetting = 1;
				curr_if_num =
				     if_desc->desc.bInterfaceNumber;
				// printf("[usb_parse_config] New interface: %d, Class: %d, SubClass: %d, Protocol: %d\n",
				//        if_desc->desc.bInterfaceNumber, if_desc->desc.bInterfaceClass,
				//        if_desc->desc.bInterfaceSubClass, if_desc->desc.bInterfaceProtocol);
			} else {
				/* found alternate setting for the interface */
				if (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->num_altsetting++;
				}
			}
			break;
		case USB_DT_ENDPOINT:
			if (head->bLength != USB_DT_ENDPOINT_SIZE &&
			    head->bLength != USB_DT_ENDPOINT_AUDIO_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				break;
			}
			if (index + head->bLength >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				break;
			}
			if (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				break;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			if (epno >= USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.bInterfaceNumber);
				return -EINVAL;
			}
			/* found an endpoint */
			if_desc->no_of_ep++;
			memcpy(&if_desc->ep_desc[epno], head,
				USB_DT_ENDPOINT_SIZE);
			ep_wMaxPacketSize = get_unaligned(&dev->config.\
							if_desc[ifno].\
							ep_desc[epno].\
							wMaxPacketSize);
			put_unaligned(le16_to_cpu(ep_wMaxPacketSize),
					&dev->config.\
					if_desc[ifno].\
					ep_desc[epno].\
					wMaxPacketSize);
			//printf("if %d, ep %d\n", ifno, epno);
			break;
		case USB_DT_SS_ENDPOINT_COMP:
			if (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				break;
			}
			if (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				break;
			}
			if (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				break;
			}
			if_desc = &dev->config.if_desc[ifno];
		memcpy(&if_desc->ss_ep_comp_desc[epno], head,
			USB_DT_SS_EP_COMP_SIZE);
		break;
	case 0x21: /* HID Class Descriptor */
		if (head->bLength < 6) {
			printf("ERROR: Invalid HID descriptor length (%d)\n",
				head->bLength);
			break;
		}
		if (index + head->bLength >
		    dev->config.desc.wTotalLength) {
			puts("HID descriptor overflowed buffer!\n");
			break;
		}
		/* HID Class Descriptor - just skip it as we don't need to store it */
		/* This descriptor contains HID version, country code, and report descriptor info */
		break;
	default:
		if (head->bLength == 0)
			return -EINVAL;

		printf("unknown Description Type : %x\n",
		      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned char *ch = (unsigned char *)head;
				int i;

				for (i = 0; i < head->bLength; i++)
					printf("%02X ", *ch++);
				printf("\n\n\n");
			}
#endif
			break;
		}
		index += head->bLength;
		head = (struct usb_descriptor_header *)&buffer[index];
	}
	return 0;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
int usb_clear_halt(struct usb_device *dev, int pipe)
{
	int result;
	int endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, NULL, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	if (result < 0)
		return result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	return 0;
}


/**********************************************************************
 * get_descriptor type
 */
static int usb_get_descriptor(struct usb_device *dev, unsigned char type,
			unsigned char index, void *buf, int size)
{
	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			       USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			       (type << 8) + index, 0, buf, size,
			       USB_CNTL_TIMEOUT);
}

/**********************************************************************
 * gets len of configuration cfgno
 */
int usb_get_configuration_len(struct usb_device *dev, int cfgno)
{
	int result;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, 9);
	struct usb_config_descriptor *config;

	/* Clear buffer to avoid residual data from previous transfers */
	memset(buffer, 0, 9);
	config = (struct usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	if (result < 9) {
		if (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		else
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		return -EIO;
	}
	return le16_to_cpu(config->wTotalLength);
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
int usb_get_configuration_no(struct usb_device *dev, int cfgno,
			     unsigned char *buffer, int length)
{
	int result;
	struct usb_config_descriptor *config;

	/* Clear destination buffer to avoid stale bytes on short/partial reads */
	memset(buffer, 0, length);
	config = (struct usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	//printf("get_conf_no %d Result %d, wLength %d\n", cfgno, result,
	//      le16_to_cpu(config->wTotalLength));
	config->wTotalLength = result; /* validated, with CPU byte order */

	return result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
static int usb_set_address(struct usb_device *dev)
{
	printf("[usb] set address %d\n", dev->devnum);

	return usb_control_msg(dev, usb_snddefctrl(dev), USB_REQ_SET_ADDRESS,
			       0, (dev->devnum), 0, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set interface number to interface
 */
int usb_set_interface(struct usb_device *dev, int interface, int alternate)
{
	struct usb_interface *if_face = NULL;
	int ret, i;

	for (i = 0; i < dev->config.desc.bNumInterfaces; i++) {
		if (dev->config.if_desc[i].desc.bInterfaceNumber == interface) {
			if_face = &dev->config.if_desc[i];
			break;
		}
	}
	if (!if_face) {
		printf("selecting invalid interface %d", interface);
		return -EINVAL;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	if (if_face->num_altsetting == 1)
		return 0;

	ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, NULL, 0,
				USB_CNTL_TIMEOUT * 5);
	if (ret < 0)
		return ret;

	return 0;
}

/********************************************************************
 * set configuration number to configuration
 */
int usb_set_configuration(struct usb_device *dev, int configuration)
{
	int res;
	printf("[usb] set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				NULL, 0, USB_CNTL_TIMEOUT);
	if (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		return 0;
	} else
		return -EIO;
}

/********************************************************************
 * set protocol to protocol
 */
int usb_set_protocol(struct usb_device *dev, int ifnum, int protocol)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
int usb_set_idle(struct usb_device *dev, int ifnum, int duration, int report_id)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
int usb_get_report(struct usb_device *dev, int ifnum, unsigned char type,
		   unsigned char id, void *buf, int size)
{
	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
int usb_get_class_descriptor(struct usb_device *dev, int ifnum,
		unsigned char type, unsigned char id, void *buf, int size)
{
	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
static int usb_get_string(struct usb_device *dev, unsigned short langid,
		   unsigned char index, void *buf, int size)
{
	int i;
	int result;

	for (i = 0; i < 3; ++i) {
		/* some devices are flaky */
		result = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

		if (result > 0)
			break;
	}

	return result;
}


static void usb_try_string_workarounds(unsigned char *buf, int *length)
{
	int newlength, oldlength = *length;

	for (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		if (!isprint(buf[newlength]) || buf[newlength + 1])
			break;

	if (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}


static int usb_string_sub(struct usb_device *dev, unsigned int langid,
		unsigned int index, unsigned char *buf)
{
	int rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	if (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		if (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	if (rc >= 2) {
		if (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		if (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	if (rc < 2)
		rc = -EINVAL;

	return rc;
}


/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
int usb_string(struct usb_device *dev, int index, char *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, mybuf, USB_BUFSIZ);
	unsigned char *tbuf;
	int err;
	unsigned int u, idx;

	if (size <= 0 || !buf || !index)
		return -EINVAL;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	if (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		if (err < 0) {
			printf("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			return -EIO;
		} else if (tbuf[0] < 4) {
			printf("string descriptor 0 too short\n");
			return -EIO;
		} else {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			printf("[usb] USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	if (err < 0)
		return err;

	size--;		/* leave room for trailing NULL char in output buffer */
	for (idx = 0, u = 2; ((int)u) < err; u += 2) {
		if (idx >= size)
			break;
		if (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		else
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	return err;
}


/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */

/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
struct usb_device *usb_get_dev_index(int index)
{
	if (usb_dev[index].devnum == -1)
		return NULL;
	else
		return &usb_dev[index];
}

int usb_alloc_new_device(void *controller, struct usb_device **devp)
{
	int i;
	printf("[usb] new device %d\n", dev_index);
	if (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		return -ENOSPC;
	}
	/* Reserve address 127 for root hub (matches Amiga scheme), real device addresses start from 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	for (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = NULL;
	usb_dev[dev_index].parent = NULL;
	usb_dev[dev_index].controller = controller;
	dev_index++;
	*devp = &usb_dev[dev_index - 1];

	return 0;
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
void usb_free_device(void *controller)
{
	(void)controller;
	dev_index--;
	printf("Freeing device node: %d\n", dev_index);
	memset(&usb_dev[dev_index], 0, sizeof(struct usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
int usb_alloc_device(struct usb_device *udev)
{
	(void)udev;
	return 0;
}

static int usb_hub_port_reset(struct usb_device *dev, struct usb_device *hub)
{
	(void)dev;
	if (!hub)
	{
		usb_reset_root_port(dev);
	}
	return 0;
}

static int get_descriptor_len(struct usb_device *dev, int len, int expect_len)
{
	struct usb_device_descriptor *desc;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, tmpbuf, USB_BUFSIZ);
	int err;

	desc = (struct usb_device_descriptor *)tmpbuf;

	/* Clear temp buffer to avoid reading stale data on hardware retries */
	memset(tmpbuf, 0, USB_BUFSIZ);
	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, len);
	if (err < expect_len) {
		if (err < 0) {
			printf("unable to get device descriptor (error=%d)\n",
				err);
			return err;
		} else {
			printf("USB device descriptor short read (expected %i, got %i)\n",
				expect_len, err);
			return -EIO;
		}
	}
	memcpy(&dev->descriptor, tmpbuf, sizeof(dev->descriptor));

	return 0;
}

static int usb_setup_descriptor(struct usb_device *dev, bool do_read)
{
	/*
	 * This is a Windows scheme of initialization sequence, with double
	 * reset of the device (Linux uses the same sequence)
	 * Some equipment is said to work only with such init sequence; this
	 * patch is based on the work by Alan Stern:
	 * http://sourceforge.net/mailarchive/forum.php?
	 * thread_id=5729457&forum_id=5398
	 */

	/*
	 * send 64-byte GET-DEVICE-DESCRIPTOR request.  Since the descriptor is
	 * only 18 bytes long, this will terminate with a short packet.  But if
	 * the maxpacket size is 8 or 16 the device may be waiting to transmit
	 * some more, or keeps on retransmitting the 8 byte header.
	 */

	if (dev->speed == USB_SPEED_LOW) {
		dev->descriptor.bMaxPacketSize0 = 8;
		dev->maxpacketsize = PACKET_SIZE_8;
	} else {
		dev->descriptor.bMaxPacketSize0 = 64;
		dev->maxpacketsize = PACKET_SIZE_64;
	}
	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;

	if (do_read && dev->speed == USB_SPEED_FULL) {
		int err;

		/*
		 * Validate we've received only at least 8 bytes, not that
		 * we've received the entire descriptor. The reasoning is:
		 * - The code only uses fields in the first 8 bytes, so
		 *   that's all we need to have fetched at this stage.
		 * - The smallest maxpacket size is 8 bytes. Before we know
		 *   the actual maxpacket the device uses, the USB controller
		 *   may only accept a single packet. Consequently we are only
		 *   guaranteed to receive 1 packet (at least 8 bytes) even in
		 *   a non-error case.
		 *
		 * At least the DWC2 controller needs to be programmed with
		 * the number of packets in addition to the number of bytes.
		 * A request for 64 bytes of data with the maxpacket guessed
		 * as 64 (above) yields a request for 1 packet.
		 */
		err = get_descriptor_len(dev, 64, 8);
		if (err)
			return err;
	}

	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;
	switch (dev->descriptor.bMaxPacketSize0) {
	case 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		break;
	case 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		break;
	case 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		break;
	case 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		break;
	default:
		printf("%s: invalid max packet size\n", __func__);
		return -EIO;
	}

	return 0;
}

static int usb_prepare_device(struct usb_device *dev, int addr, bool do_read,
			      struct usb_device *parent)
{
	int err;

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	err = usb_alloc_device(dev);
	if (err) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		return err;
	}
	err = usb_setup_descriptor(dev, do_read);
	if (err)
		return err;
	err = usb_hub_port_reset(dev, parent);
	if (err)
		return err;

	dev->devnum = addr;

	err = usb_set_address(dev); /* set address */

	if (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		return err;
	}

	mdelay(10);	/* Let the SET_ADDRESS settle */

	/*
	 * If we haven't read device descriptor before, read it here
	 * after device is assigned an address. This is only applicable
	 * to xHCI so far.
	 */
	if (!do_read) {
		err = usb_setup_descriptor(dev, true);
		if (err)
			return err;
	}

	return 0;
}

int usb_select_config(struct usb_device *dev)
{
	unsigned char *tmpbuf = NULL;
	int err;

	err = get_descriptor_len(dev, USB_DT_DEVICE_SIZE, USB_DT_DEVICE_SIZE);
	if (err)
		return err;

	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);

	/*
	 * Kingston DT Ultimate 32GB USB 3.0 seems to be extremely sensitive
	 * about this first Get Descriptor request. If there are any other
	 * requests in the first microframe, the stick crashes. Wait about
	 * one microframe duration here (1mS for USB 1.x , 125uS for USB 2.0).
	 */
	mdelay(1);

	/* only support for one config for now */
	err = usb_get_configuration_len(dev, 0);
	if (err >= 0) {
		tmpbuf = (unsigned char *)malloc_cache_aligned(err);
		if (!tmpbuf)
			err = -ENOMEM;
		else {
			/* CRITICAL FIX: Reset EP0 state between CONFIG transfers */
			/* Clear toggles to ensure clean state for full CONFIG read */
			dev->toggle[0] = 0;
			dev->toggle[1] = 0;
			/* Small delay to let EP0 settle after the 9-byte CONFIG read */
			mdelay(2);
			err = usb_get_configuration_no(dev, 0, tmpbuf, err);
		}
	}
	if (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		free(tmpbuf);
		return err;
	}
	usb_parse_config(dev, tmpbuf, 0);
	free(tmpbuf);
	usb_set_maxpacket(dev);
	/*
	 * we set the default configuration here
	 * This seems premature. If the driver wants a different configuration
	 * it will need to select itself.
	 */
	err = usb_set_configuration(dev, dev->config.desc.bConfigurationValue);
	if (err < 0) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		return err;
	}

	/*
	 * Wait until the Set Configuration request gets processed by the
	 * device. This is required by at least SanDisk Cruzer Pop USB 2.0
	 * and Kingston DT Ultimate 32GB USB 3.0 on DWC2 OTG controller.
	 */
	mdelay(10);

	/*printf("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.iSerialNumber);*/
	memset(dev->mf, 0, sizeof(dev->mf));
	memset(dev->prod, 0, sizeof(dev->prod));
	memset(dev->serial, 0, sizeof(dev->serial));
	if (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, sizeof(dev->mf));
	if (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, sizeof(dev->prod));
	if (dev->descriptor.iSerialNumber)
		usb_string(dev, dev->descriptor.iSerialNumber,
			   dev->serial, sizeof(dev->serial));
	printf("[usb] Manufacturer %s\n", dev->mf);
	printf("[usb] Product      %s\n", dev->prod);
	printf("[usb] SerialNumber %s\n", dev->serial);

	// Call ARM device detection logging with RAW descriptor data
	extern void log_device_detected_arm(unsigned char dev, const void *data_ptr, int data_len,
	                                   unsigned short vendor_id, unsigned short product_id);
	log_device_detected_arm(dev->devnum, &dev->descriptor, sizeof(dev->descriptor),
	                       dev->descriptor.idVendor, dev->descriptor.idProduct);

	return 0;
}

int usb_setup_device(struct usb_device *dev, bool do_read,
		     struct usb_device *parent)
{
	int addr;
	int ret;

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

	ret = usb_prepare_device(dev, addr, do_read, parent);
	if (ret)
		return ret;
	ret = usb_select_config(dev);

	return ret;
}

/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
int usb_new_device(struct usb_device *dev)
{
	bool do_read = true;
	int err;

	printf("[usb_new_device] Probing new device...\n");
	err = usb_setup_device(dev, do_read, dev->parent);
	if (err) {
		printf("[usb_new_device] usb_setup_device failed with error %d\n", err);
		return err;
	}

	/* Now probe if the device is a hub */
	printf("[usb_new_device] Probing for hub...\n");
	err = usb_hub_probe(dev, 0);
	if (err < 0) {
		printf("[usb_new_device] usb_hub_probe failed with error %d\n", err);
		return err;
	}

	printf("[usb_new_device] Device setup complete.\n");
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	(void)index;
	(void)init;
	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	(void)index;
	(void)init;
	return 0;
}

bool usb_device_has_child_on_port(struct usb_device *parent, int port)
{
	return parent->children[port] != NULL;
}

void usb_find_usb2_hub_address_port(struct usb_device *udev,
			       uint8_t *hub_address, uint8_t *hub_port)
{
	/* Default to no TT context */
	if (hub_address)
		*hub_address = 0;
	if (hub_port)
		*hub_port = 0;

	/* Safety check for null pointers */
	if (!udev || !hub_address || !hub_port)
		return;

	/* Check cache first - avoid recalculation for same device */
	static struct tt_cache {
		int devnum;
		int hub_address;
		int hub_port;
		int valid;
	} cache = {0};
	
	if (cache.valid && cache.devnum == udev->devnum) {
		*hub_address = cache.hub_address;
		*hub_port = cache.hub_port;
		#ifdef USB_DEBUG_VERBOSE
		USB_DEBUG("[USB TT] Using cached TT info for device %d: hub_addr=%d hub_port=%d\n", 
		       udev->devnum, *hub_address, *hub_port);
		#endif
		return;
	}

	#ifdef USB_DEBUG_VERBOSE
	printf("[USB TT] Analyzing device %d (speed=%s parent=%p)\n", udev->devnum,
	       (udev->speed == USB_SPEED_HIGH) ? "HIGH" :
	       (udev->speed == USB_SPEED_FULL) ? "FULL" :
	       (udev->speed == USB_SPEED_LOW) ? "LOW" : "UNKNOWN",
	       udev->parent);
	#endif

	/* High-Speed devices never need split transactions */
	if (udev->speed == USB_SPEED_HIGH) {
		#ifdef USB_DEBUG_VERBOSE
		printf("[USB TT] Device %d is HIGH-speed - NO split transaction needed\n", udev->devnum);
		#endif
		return;
	}
	
	/* For Full/Low-Speed devices, we need to find the nearest High-Speed hub */
	struct usb_device *ttdev = udev;  /* The device that will use the TT */
	struct usb_device *p = udev->parent;
	int guard = 0;
	
	while (p && guard++ < 16) {
		#ifdef USB_DEBUG_VERBOSE
		printf("[USB TT] Checking parent: devnum=%d speed=%s parent=%p\n",
		       p->devnum,
		       (p->speed == USB_SPEED_HIGH) ? "HIGH" :
		       (p->speed == USB_SPEED_FULL) ? "FULL" :
		       (p->speed == USB_SPEED_LOW) ? "LOW" : "UNKNOWN",
		       p->parent);
		#endif
		
		/* Found a High-Speed hub */
		if (p->speed == USB_SPEED_HIGH) {
			*hub_address = p->devnum;
			*hub_port = ttdev->portnr;
			
			/* Cache the result */
			cache.devnum = udev->devnum;
			cache.hub_address = *hub_address;
			cache.hub_port = *hub_port;
			cache.valid = 1;
			
			#ifdef USB_DEBUG_VERBOSE
			printf("[USB TT] Split transaction required: hub_addr=%d hub_port=%d (CACHED)\n", *hub_address, *hub_port);
			#endif
			return;
		}
		
		/* Move up the chain */
		ttdev = p;
		p = p->parent;
	}
	
	#ifdef USB_DEBUG_VERBOSE
	printf("[USB TT] No High-Speed hub found - direct communication (hub_addr=%d hub_port=%d)\n", *hub_address, *hub_port);
	#endif
	/* If we didn't find a HS hub ancestor, leave hub_address/port = 0 */
}

/* Poseidon-driven minimal init: initialize controller without bus scan */
int usb_poseidon_init(void)
{
    void *ctrl;
    int ret;

    dev_index = 0;
    asynch_allowed = 1;
    usb_hub_reset();

    /* Clear device table */
    for (int i = 0; i < USB_MAX_DEVICE; i++) {
        memset(&usb_dev[i], 0, sizeof(struct usb_device));
        usb_dev[i].devnum = -1;
    }

    /* Init low-level USB host controller only */
    printf("[usb_poseidon] lowlevel init...\n");
    ret = usb_lowlevel_init(0, USB_INIT_HOST, &ctrl);
    if (ret) {
        printf("[usb_poseidon] lowlevel init failed: %d\n", ret);
        return ret;
    }
    poseidon_ctrl = ctrl;
    usb_started = 1;

    /* CRITICAL FIX: Do NOT pre-allocate DEV=0!
     * ADDR=0 should be temporary and created on-demand during enumeration.
     * According to USB spec:
     * - ADDR=0 is used only during device enumeration
     * - After SET_ADDRESS, ADDR=0 should be freed for next enumeration
     * - Multiple ADDR=0 should not coexist (violates USB standard)
     * 
     * The old approach was creating a persistent ADDR=0 which caused:
     * - "Device addr=0 already exists" messages
     * - Memory waste
     * - Violation of USB enumeration standard
     */

    /* CRITICAL FIX: Power on root hub ports!
     * In standalone mode, usb_hub_probe() -> usb_hub_configure() -> usb_hub_power_on() 
     * powers on all root hub ports. Without this, external hubs get no power.
     * We need to do the same minimal power-on sequence here without full enumeration. */
    printf("[usb_poseidon] Powering on root hub ports...\n");
    
    /* Get root hub descriptor to find number of ports */
    extern struct ehci_ctrl ehcic[];
    struct ehci_ctrl *ehci_ctrl = &ehcic[0];
    if (ehci_ctrl && ehci_ctrl->hccr) {
        uint32_t reg = ehci_readl(&ehci_ctrl->hccr->cr_hcsparams);
        int num_ports = reg & 0xf; /* HCS_N_PORTS(reg) */
        printf("[usb_poseidon] Root hub has %d ports\n", num_ports);
        
        /* Power on each port using EHCI root hub emulation */
        for (int port = 0; port < num_ports; port++) {
            uint32_t *portsc = &ehci_ctrl->hcor->or_portsc[port];
            uint32_t status = ehci_readl(portsc);
            printf("[usb_poseidon] Port %d before: 0x%08lx\n", port, (unsigned long)status);
            
            /* Set Port Power (PP) bit - this is what usb_hub_power_on() effectively does */
            status |= (1 << 12); /* EHCI_PS_PP - Port Power */
            ehci_writel(portsc, status);
            
            status = ehci_readl(portsc);
            printf("[usb_poseidon] Port %d after power-on: 0x%08lx\n", port, (unsigned long)status);
        }
        
        /* Give ports time to power up - same delay as usb_hub_power_on() */
        printf("[usb_poseidon] Waiting for ports to power up...\n");
        mdelay(100); /* Minimum 100ms power stabilization delay */
        
        printf("[usb_poseidon] Port power-on completed\n");
    } else {
        printf("[usb_poseidon] WARNING: Could not access EHCI controller for port power-on\n");
    }
    
    printf("[usb_poseidon] ready, default dev at address 0\n");
    return 0;
}

int usb_poseidon_reset(void)
{
    int ret;
    /* Stop controller if started */
    if (usb_started) {
        usb_stop();
        usb_started = 0;
    }
    /* Re-init low-level only */
    ret = usb_poseidon_init();
    if (ret == 0)
        return 0;
    return ret;
}

struct usb_device *usb_poseidon_get_dev(int addr)
{
    for (int i = 0; i < USB_MAX_DEVICE; i++) {
        if (usb_dev[i].devnum == addr)
            return &usb_dev[i];
    }
    return NULL;
}


struct usb_device *usb_poseidon_alloc_dev(int addr)
{
    struct usb_device *dev=NULL;
    int i;

    if (!poseidon_ctrl) {
        printf("[usb_poseidon] controller not initialized\n");
        return NULL;
    }

    /* Handle address 0 specially - always create new instance */
    if (addr == 0) {
        /* Never reuse address 0 devices - each enumeration needs fresh state */
        if (dev_index >= USB_MAX_DEVICE) {
            printf("[USB] ERROR: No free device slots\n");
            return NULL;
        }
        
        /* Initialize new device at address 0 */
        dev = &usb_dev[dev_index++];
        memset(dev, 0, sizeof(struct usb_device));
        dev->devnum = 0;
        dev->controller = poseidon_ctrl;
        
        /* CRITICAL HIGH SPEED FIX: Force port reset for HS negotiation FIRST */
        printf("[USB HS] Forcing port reset for High Speed negotiation...\n");
        extern struct ehci_ctrl ehcic[];
        struct ehci_ctrl *ehci_ctrl = &ehcic[0];
        
        /* CRITICAL: Validate ALL pointers before register access to avoid crash */
        if (!ehci_ctrl) {
            printf("[USB HS] ERROR: ehci_ctrl is NULL - skipping HS reset\n");
        } else if (!ehci_ctrl->hcor) {
            printf("[USB HS] ERROR: ehci_ctrl->hcor is NULL - skipping HS reset\n");
        } else {
            printf("[USB HS] EHCI pointers valid: ehci_ctrl=%p hcor=%p\n", ehci_ctrl, ehci_ctrl->hcor);
            printf("[USB HS] PORTSC register address: %p\n", &ehci_ctrl->hcor->or_portsc[0]);
            
            /* Safely read current PORTSC */
            uint32_t portsc = ehci_readl(&ehci_ctrl->hcor->or_portsc[0]);
            printf("[USB HS] Before HS reset: PORTSC=0x%08lx PSPD=%ld\n", 
                   (unsigned long)portsc, (portsc >> 26) & 0x3);
            
            /* Force port reset for HS negotiation - FIXED parameter order */
            printf("[USB HS] Performing port reset with CORRECT parameter order...\n");
            portsc |= 0x100;  /* Set PR bit */
            ehci_writel(&ehci_ctrl->hcor->or_portsc[0], portsc);  /* CORRECT: (addr, value) */
            printf("[USB HS] Reset bit set, waiting 50ms...\n");
            usleep(50 * 1000);  /* 50ms reset pulse */
            
            /* Clear reset and wait for negotiation */
            portsc &= ~0x100;  /* Clear PR bit */
            ehci_writel(&ehci_ctrl->hcor->or_portsc[0], portsc);  /* CORRECT: (addr, value) */
            printf("[USB HS] Reset bit cleared, waiting 100ms for HS negotiation...\n");
            usleep(100 * 1000);  /* 100ms for HS negotiation */
            
            /* Read final status */
            portsc = ehci_readl(&ehci_ctrl->hcor->or_portsc[0]);
            printf("[USB HS] After HS reset: PORTSC=0x%08lx PSPD=%ld (%s)\n", 
                   (unsigned long)portsc, (portsc >> 26) & 0x3,
                   ((portsc >> 26) & 0x3) == 2 ? "HIGH_SPEED" : 
                   ((portsc >> 26) & 0x3) == 1 ? "LOW_SPEED" : "FULL_SPEED");
        }
        
        /* CRITICAL FIX: Detect device speed for address 0 devices too!
         * Previously we used safe defaults, but this caused "speed=UNKNOWN" 
         * which led to timeouts in EHCI transfers. We need to detect the actual
         * device speed from PORTSC to configure proper transfer parameters. */
        
        /* Get current port status to determine device speed */
        extern struct ehci_ctrl ehcic[];
        uint32_t portsc_status = 0;
        int is_low_speed = 0;
        int device_speed = 0;
        const char *speed_name = "UNKNOWN";
        
        if (ehci_ctrl && ehci_ctrl->hcor) {
            portsc_status = ehci_readl(&ehci_ctrl->hcor->or_portsc[0]);
            
            /* Extract PORTSC_PSPD field (bits 27-26) - the correct speed field */
            device_speed = (portsc_status >> 26) & 0x3;
            
            switch (device_speed) {
                case 0: /* PORTSC_PSPD_FS */
                    is_low_speed = 0;
                    speed_name = "FULL_SPEED";
                    break;
                case 1: /* PORTSC_PSPD_LS */
                    is_low_speed = 1;
                    speed_name = "LOW_SPEED";
                    break;
                case 2: /* PORTSC_PSPD_HS */
                    is_low_speed = 0;
                    speed_name = "HIGH_SPEED";
                    break;
                default:
                    is_low_speed = 0;
                    speed_name = "UNKNOWN";
                    break;
            }
            
            printf("[USB SPEED] Device addr=0 PORTSC=0x%08lx PSPD=%d (%s)\n", 
                   (unsigned long)portsc_status, device_speed, speed_name);
        }
        
        /* Configure EP0 packet size AND device speed based on detected speed */
        if (is_low_speed) {
            /* Low Speed devices: Must use 8 bytes EP0 packet size */
            dev->epmaxpacketin[0] = 8;
            dev->epmaxpacketout[0] = 8;
            dev->maxpacketsize = PACKET_SIZE_8;
            dev->speed = USB_SPEED_LOW;
            printf("[USB SPEED] Device addr=0 configured for LOW SPEED (8 bytes EP0)\n");
        } else {
            /* Full/High Speed devices: Start with 64 bytes for proper enumeration */
            dev->epmaxpacketin[0] = 64;
            dev->epmaxpacketout[0] = 64;
            dev->maxpacketsize = PACKET_SIZE_64;
            
            /* Set correct speed based on PORTSC detection */
            if (device_speed == 2) {
                dev->speed = USB_SPEED_HIGH;
                printf("[USB SPEED] Device addr=0 configured for HIGH SPEED (64 bytes EP0)\n");
            } else {
                dev->speed = USB_SPEED_FULL;
                printf("[USB SPEED] Device addr=0 configured for FULL SPEED (64 bytes EP0)\n");
            }
        }
        
        printf("[USB] Created new device at address 0 for enumeration\n");
        return dev;
    }
    
    /* For non-zero addresses, check if it already exists */
    dev = usb_poseidon_get_dev(addr);
    if (dev) {
        /* CRITICAL FIX: This should be rare for non-zero addresses
         * Log it as debug info, not a persistent problem */
//        printf("[USB ALLOC] Device addr=%d already exists, returning existing (normal for multiple operations)\n", addr);
        return dev;
    }

    /* CRITICAL FIX: Allocate device manually instead of using usb_alloc_new_device()
     * because usb_alloc_new_device() assigns devnum = dev_index + 1, which creates
     * confusion when Poseidon requests specific device addresses.
     * 
     * We need direct control over device allocation to ensure:
     * - dev->devnum exactly matches the requested addr
     * - Array position maps correctly to device address
     * - No confusion between internal dev_index and USB device addresses
     */
    
    if (dev_index >= USB_MAX_DEVICE) {
        printf("[usb_poseidon] ERROR: too many USB devices, max=%d\n", USB_MAX_DEVICE);
        return NULL;
    }
    
    /* Use next available slot in device array */
    dev = &usb_dev[dev_index];
    
    /* Initialize device structure with correct address */
    memset(dev, 0, sizeof(struct usb_device));
    dev->devnum = addr;  /* Use exactly the address Poseidon requested */
    dev->maxchild = 0;
    for (i = 0; i < USB_MAXCHILDREN; i++)
        dev->children[i] = NULL;
    dev->parent = NULL;
    dev->controller = poseidon_ctrl;
    
    /* CRITICAL FIX: Configure EP0 based on device speed detection from PORTSC
     * EHCI PORTSC LS field (bits 11-10):
     * - 00: Full/High Speed device (use 64 bytes for initial probe)
     * - 01: Low Speed device (use 8 bytes)
     * - 10: Full Speed device (use 64 bytes for initial probe) 
     * - 11: Reserved
     * 
     * For Poseidon compatibility, we start with reasonable defaults and let
     * the GET_DESCRIPTOR response update the actual packet size.
     */
    
    
    /* Get current port status to determine device speed
     * CRITICAL FIX: Use correct PORTSC_PSPD field (bits 27-26) not LS field (bits 11-10)
     * Based on U-Boot ehci-hcd.c implementation:
     * - PORTSC_PSPD_FS = 0x0 (Full Speed)
     * - PORTSC_PSPD_LS = 0x1 (Low Speed) 
     * - PORTSC_PSPD_HS = 0x2 (High Speed)
     */
    extern struct ehci_ctrl ehcic[];
    struct ehci_ctrl *ehci_ctrl = &ehcic[0];
    uint32_t portsc_status = 0;
    int is_low_speed = 0;
    int device_speed = 0;
    const char *speed_name = "UNKNOWN";
    
    if (ehci_ctrl && ehci_ctrl->hcor) {
        portsc_status = ehci_readl(&ehci_ctrl->hcor->or_portsc[0]);
        
        /* Extract PORTSC_PSPD field (bits 27-26) - the correct speed field */
        device_speed = (portsc_status >> 26) & 0x3;
        
        switch (device_speed) {
            case 0: /* PORTSC_PSPD_FS */
                is_low_speed = 0;
                speed_name = "FULL_SPEED";
                break;
            case 1: /* PORTSC_PSPD_LS */
                is_low_speed = 1;
                speed_name = "LOW_SPEED";
                break;
            case 2: /* PORTSC_PSPD_HS */
                is_low_speed = 0;
                speed_name = "HIGH_SPEED";
                break;
            default:
                is_low_speed = 0;
                speed_name = "UNKNOWN";
                break;
        }
        
        printf("[USB SPEED] Device addr=%d PORTSC=0x%08lx PSPD=%d (%s)\n", 
               addr, (unsigned long)portsc_status, device_speed, speed_name);
        
        /* Additional debug: show LS field (bits 11-10) for comparison */
        int line_status = (portsc_status >> 10) & 0x3;
        printf("[USB SPEED] Line Status (bits 11-10): %d (was incorrectly using this before)\n", line_status);
    }
    
    /* Configure EP0 packet size AND device speed based on detected speed */
    if (is_low_speed) {
        /* Low Speed devices: Must use 8 bytes EP0 packet size */
        dev->epmaxpacketin[0] = 8;
        dev->epmaxpacketout[0] = 8;
        dev->maxpacketsize = PACKET_SIZE_8;
        dev->speed = USB_SPEED_LOW;  /* CRITICAL FIX: Set speed field */
        printf("[USB SPEED] Device addr=%d configured for LOW SPEED (8 bytes EP0)\n", addr);
    } else {
        /* CRITICAL FIX: Use conservative EP0 sizing for better compatibility
         * Many FS devices (especially mice) use 8-byte EP0 even though spec allows 8,16,32,64
         * Start conservative, let EP0 probing determine actual size */
        
        if (device_speed == 2) {
            /* High Speed devices can reliably use 64 bytes */
            dev->epmaxpacketin[0] = 64;
            dev->epmaxpacketout[0] = 64;
            dev->maxpacketsize = PACKET_SIZE_64;
            dev->speed = USB_SPEED_HIGH;
            printf("[USB SPEED] Device addr=%d configured for HIGH SPEED (64 bytes EP0)\n", addr);
        } else {
            /* Full Speed: Start conservative with 8 bytes to avoid transfer failures
             * EP0 probing will increase this if the device actually supports larger packets */
            dev->epmaxpacketin[0] = 8;
            dev->epmaxpacketout[0] = 8;
            dev->maxpacketsize = PACKET_SIZE_8;
            dev->speed = USB_SPEED_FULL;
            printf("[USB SPEED] Device addr=%d configured for FULL SPEED (8 bytes EP0 - conservative)\n", addr);
            printf("[USB SPEED] EP0 probing will determine if larger packets are supported\n");
        }
    }
    
    /* Advance device index for next allocation */
    dev_index++;
    
    printf("[USB ALLOC] Allocated device addr=%d at array index %d\n", addr, dev_index - 1);

    /* DO NOT pre-assign topology or speeds for Poseidon devices!
     * Poseidon will discover the actual topology through GET_DESCRIPTOR.
     * Let the natural USB enumeration process determine:
     * - Device speeds via descriptor analysis
     * - Device relationships via hub enumeration
     * - Split transaction requirements via usb_find_usb2_hub_address_port()
     * 
     * The previous logic was incorrectly assuming:
     * - addr=1 is always a HIGH SPEED hub (WRONG - could be FULL SPEED external hub)
     * - addr>1 devices are always children of addr=1 (WRONG - topology discovered via GET_DESCRIPTOR)
     */
    
    printf("[USB ALLOC] Allocated device addr=%d - speeds and topology will be discovered via enumeration\n", addr);

    return dev;
}

/* Poseidon helper: apply full configuration descriptor to device state
 * Parses the provided config buffer (must contain an entire configuration
 * descriptor blob starting with USB_DT_CONFIG) and updates dev->config and
 * endpoint max packet sizes, matching standalone behavior.
 */
int usb_poseidon_apply_config(struct usb_device *dev, unsigned char *buffer, int length)
{
	int ret;
	if (!dev || !buffer || length < 9)
		return -1;

	/* Basic sanity: first header must be CONFIG and not overflow */
	struct usb_config_descriptor *cfg = (struct usb_config_descriptor *)buffer;
	if (cfg->bLength != USB_DT_CONFIG_SIZE || cfg->bDescriptorType != USB_DT_CONFIG)
		return -1;
	if (le16_to_cpu(cfg->wTotalLength) > length)
		return -1;

	/* Reuse internal parser and endpoint sizing helpers */
	ret = usb_parse_config(dev, buffer, 0);
	if (ret)
		return ret;
	ret = usb_set_maxpacket(dev);
	return ret;
}

/* EOF */
