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
 *
 * ZZ9000 modifications:
 *
 * Copyright (C) 2026 Dimitris Panokostas <midwan@gmail.com>
 */

/****************************************************************************
 * HUB "Driver"
 * Probes device for being a hub and configurate it
 */

//#include <common.h>
//#include <command.h>
#include <errno.h>
#include "usb/memalign.h"
//#include <asm/processor.h>
//#include <asm/unaligned.h>
//#include <linux/ctype.h>
#include "list.h"
#include <usb/asm/byteorder.h>
//#include <asm/unaligned.h>
#include <stdio.h>
#include <string.h>

#include "usb.h"

unsigned long get_timer(unsigned long i);
void mdelay(int ms);

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

#define USB_BUFSIZ	512

#define HUB_SHORT_RESET_TIME	20
#define HUB_LONG_RESET_TIME	200

#define PORT_OVERCURRENT_MAX_SCAN_COUNT		3

struct usb_device_scan {
	struct usb_device *dev;		/* USB hub device to scan */
	struct usb_hub_device *hub;	/* USB hub struct */
	int port;			/* USB port to scan */
	struct list_head list;
};

static LIST_HEAD(usb_scan_list);

void usb_hub_reset_devices(struct usb_hub_device *hub, int port)
{
	(void)hub;
	(void)port;
	return;
}

static inline bool usb_hub_is_superspeed(struct usb_device *hdev)
{
	(void)hdev;
	return hdev->descriptor.bDeviceProtocol == 3;
}

static int usb_get_hub_descriptor(struct usb_device *dev, void *data, int size)
{
	unsigned short dtype = USB_DT_HUB;

	if (usb_hub_is_superspeed(dev))
		dtype = USB_DT_SS_HUB;

	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
		dtype << 8, 0, data, size, USB_CNTL_TIMEOUT);
}

static int usb_clear_port_feature(struct usb_device *dev, int port, int feature)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature,
				port, NULL, 0, USB_CNTL_TIMEOUT);
}

static int usb_set_port_feature(struct usb_device *dev, int port, int feature)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_FEATURE, USB_RT_PORT, feature,
				port, NULL, 0, USB_CNTL_TIMEOUT);
}

static int usb_get_hub_status(struct usb_device *dev, void *data)
{
	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
			data, sizeof(struct usb_hub_status), USB_CNTL_TIMEOUT);
}

int usb_get_port_status(struct usb_device *dev, int port, void *data)
{
	int ret;

	ret = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
			data, sizeof(struct usb_port_status), USB_CNTL_TIMEOUT);

	return ret;
}

static void usb_hub_power_on(struct usb_hub_device *hub)
{
	int i;
	struct usb_device *dev;
	unsigned pgood_delay = hub->desc.bPwrOn2PwrGood * 2;

	dev = hub->pusb_dev;

	//printf("[usb-hub] enabling power on all ports\n");
	for (i = 0; i < dev->maxchild; i++) {
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		//printf("port %d returns %lX\n", i + 1, dev->status);
	}

	/*
	 * Wait for power to become stable,
	 * plus spec-defined max time for device to connect
	 * but allow this time to be increased via env variable as some
	 * devices break the spec and require longer warm-up times
	 */
	//env = env_get("usb_pgood_delay");
	//if (env)
	//	pgood_delay = max(pgood_delay,
	//		          (unsigned)simple_strtol(env, NULL, 0));
	//printf("pgood_delay=%dms\n", pgood_delay);

	/*
	 * Do a minimum delay of the larger value of 100ms or pgood_delay
	 * so that the power can stablize before the devices are queried
	 */
	hub->query_delay = get_timer(0) + max(100, (int)pgood_delay);

	/*
	 * Record the power-on timeout here. The max. delay (timeout)
	 * will be done based on this value in the USB port loop in
	 * usb_hub_configure() later.
	 */
	hub->connect_timeout = hub->query_delay + 1000;
	//printf("devnum=%d poweron: query_delay=%d connect_timeout=%d\n",
	//      dev->devnum, max(100, (int)pgood_delay),
	//      max(100, (int)pgood_delay) + 1000);
}

static struct usb_hub_device hub_dev[USB_MAX_HUB];
static int usb_hub_index;

void usb_hub_reset(void)
{
	usb_hub_index = 0;

	/* Zero out global hub_dev in case its re-used again */
	memset(hub_dev, 0, sizeof(hub_dev));
}

static struct usb_hub_device *usb_hub_allocate(void)
{
	if (usb_hub_index < USB_MAX_HUB)
		return &hub_dev[usb_hub_index++];

	printf("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	return NULL;
}

#define MAX_TRIES 5

static inline char *portspeed(int portstatus)
{
	char *speed_str;

	switch (portstatus & USB_PORT_STAT_SPEED_MASK) {
	case USB_PORT_STAT_SUPER_SPEED:
		speed_str = "5 Gb/s";
		break;
	case USB_PORT_STAT_HIGH_SPEED:
		speed_str = "480 Mb/s";
		break;
	case USB_PORT_STAT_LOW_SPEED:
		speed_str = "1.5 Mb/s";
		break;
	default:
		speed_str = "12 Mb/s";
		break;
	}

	return speed_str;
}

/**
 * usb_hub_port_reset() - reset a port given its usb_device pointer
 *
 * Reset a hub port and see if a device is present on that port, providing
 * sufficient time for it to show itself. The port status is returned.
 *
 * @dev:	USB device to reset
 * @port:	Port number to reset (note ports are numbered from 0 here)
 * @portstat:	Returns port status
 */
int usb_hub_port_reset(struct usb_device *dev, int port,
		      unsigned short *portstat)
{
	int err, tries;
	ALLOC_CACHE_ALIGN_BUFFER(struct usb_port_status, portsts, 1);
	unsigned short portstatus, portchange;
	int delay = HUB_SHORT_RESET_TIME; /* start with short reset delay */

	printf("[usb-hub] %s: resetting port %d...\n", __func__, port + 1);

	for (tries = 0; tries < MAX_TRIES; tries++) {
		err = usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
		if (err < 0)
			return err;

		mdelay(delay);

		if (usb_get_port_status(dev, port + 1, portsts) < 0) {
			printf("[usb-hub] get_port_status failed status %lX\n",
			      dev->status);
			return -1;
		}
		portstatus = le16_to_cpu(portsts->wPortStatus);
		portchange = le16_to_cpu(portsts->wPortChange);

		printf("[usb-hub] portstatus %x, change %x, %s\n", portstatus, portchange,
							portspeed(portstatus));

		/*printf("STAT_C_CONNECTION = %d STAT_CONNECTION = %d" \
		      "  USB_PORT_STAT_ENABLE %d\n",
		      (portchange & USB_PORT_STAT_C_CONNECTION) ? 1 : 0,
		      (portstatus & USB_PORT_STAT_CONNECTION) ? 1 : 0,
		      (portstatus & USB_PORT_STAT_ENABLE) ? 1 : 0);*/

		/*
		 * Perhaps we should check for the following here:
		 * - C_CONNECTION hasn't been set.
		 * - CONNECTION is still set.
		 *
		 * Doing so would ensure that the device is still connected
		 * to the bus, and hasn't been unplugged or replaced while the
		 * USB bus reset was going on.
		 *
		 * However, if we do that, then (at least) a San Disk Ultra
		 * USB 3.0 16GB device fails to reset on (at least) an NVIDIA
		 * Tegra Jetson TK1 board. For some reason, the device appears
		 * to briefly drop off the bus when this second bus reset is
		 * executed, yet if we retry this loop, it'll eventually come
		 * back after another reset or two.
		 */

		if (portstatus & USB_PORT_STAT_ENABLE)
			break;

		/* Switch to long reset delay for the next round */
		delay = HUB_LONG_RESET_TIME;
	}

	if (tries == MAX_TRIES) {
		printf("Cannot enable port %i after %i retries, " \
		      "disabling port.\n", port + 1, MAX_TRIES);
		printf("Maybe the USB cable is bad?\n");
		return -1;
	}

	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
	*portstat = portstatus;
	return 0;
}

int usb_hub_port_connect_change(struct usb_device *dev, int port)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct usb_port_status, portsts, 1);
	unsigned short portstatus;
	int ret, speed;

	/* Check status */
	ret = usb_get_port_status(dev, port + 1, portsts);
	if (ret < 0) {
		printf("get_port_status failed\n");
		return ret;
	}

	portstatus = le16_to_cpu(portsts->wPortStatus);
	/*printf("[usb-hub] portstatus %x, change %x, %s\n",
	      portstatus,
	      le16_to_cpu(portsts->wPortChange),
	      portspeed(portstatus));*/

	/* Clear the connection change status */
	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	if (((!(portstatus & USB_PORT_STAT_CONNECTION)) &&
	     (!(portstatus & USB_PORT_STAT_ENABLE))) ||
	    usb_device_has_child_on_port(dev, port)) {
		printf("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		if (!(portstatus & USB_PORT_STAT_CONNECTION))
			return -ENOTCONN;
	}

	/* Reset the port */
	ret = usb_hub_port_reset(dev, port, &portstatus);
	if (ret < 0) {
		if (ret != -ENXIO)
			printf("cannot reset port %i!?\n", port + 1);
		return ret;
	}

	switch (portstatus & USB_PORT_STAT_SPEED_MASK) {
	case USB_PORT_STAT_SUPER_SPEED:
		speed = USB_SPEED_SUPER;
		break;
	case USB_PORT_STAT_HIGH_SPEED:
		speed = USB_SPEED_HIGH;
		break;
	case USB_PORT_STAT_LOW_SPEED:
		speed = USB_SPEED_LOW;
		break;
	default:
		speed = USB_SPEED_FULL;
		break;
	}

	struct usb_device *usb;

	ret = usb_alloc_new_device(dev->controller, &usb);
	if (ret) {
		printf("cannot create new device: ret=%d", ret);
		return ret;
	}

	dev->children[port] = usb;
	usb->speed = speed;
	usb->parent = dev;
	usb->portnr = port + 1;
	/* Run it through the hoops (find a driver, etc) */
	ret = usb_new_device(usb);
	if (ret < 0) {
		/* Woops, disable the port */
		usb_free_device(dev->controller);
		dev->children[port] = NULL;
	}

	if (ret < 0) {
		printf("hub: disabling port %d\n", port + 1);
		usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_ENABLE);
	}

	return ret;
}

static int usb_scan_port(struct usb_device_scan *usb_scan)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct usb_port_status, portsts, 1);
	unsigned short portstatus;
	unsigned short portchange;
	struct usb_device *dev;
	struct usb_hub_device *hub;
	int ret = 0;
	int i;

	dev = usb_scan->dev;
	hub = usb_scan->hub;
	i = usb_scan->port;

	/*
	 * Don't talk to the device before the query delay is expired.
	 * This is needed for voltages to stabalize.
	 */
	if (get_timer(0) < hub->query_delay)
		return 0;

	ret = usb_get_port_status(dev, i + 1, portsts);
	if (ret < 0) {
		printf("get_port_status failed\n");
		if (get_timer(0) >= hub->connect_timeout) {
			printf("[usb-hub] devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			free(usb_scan); // FIXME dynamic allocation
			return 0;
		}
		return 0;
	}

	portstatus = le16_to_cpu(portsts->wPortStatus);
	portchange = le16_to_cpu(portsts->wPortChange);
//	printf("[usb-hub] Port %d Status %X Change %X\n", i + 1, portstatus, portchange);

	/*
	 * No connection change happened, wait a bit more.
	 *
	 * For some situation, the hub reports no connection change but a
	 * device is connected to the port (eg: CCS bit is set but CSC is not
	 * in the PORTSC register of a root hub), ignore such case.
	 */
	if (!(portchange & USB_PORT_STAT_C_CONNECTION) &&
	    !(portstatus & USB_PORT_STAT_CONNECTION)) {
		if (get_timer(0) >= hub->connect_timeout) {
			printf("[usb-hub] devnum=%d port=%d: timeout\n",
			      dev->devnum, i + 1);
			/* Remove this device from scanning list */
			list_del(&usb_scan->list);
			free(usb_scan); // FIXME dynamic allocation
			return 0;
		}
		return 0;
	}

	if (portchange & USB_PORT_STAT_C_RESET) {
		printf("[usb-hub] port %d reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_RESET);
	}

	if ((portchange & USB_SS_PORT_STAT_C_BH_RESET) &&
	    usb_hub_is_superspeed(dev)) {
		printf("[usb-hub] port %d BH reset change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_SS_PORT_FEAT_C_BH_RESET);
	}

	/* A new USB device is ready at this point */
	printf("[usb-hub] devnum=%d port=%d: USB dev found\n", dev->devnum, i + 1);

	usb_hub_port_connect_change(dev, i);

	if (portchange & USB_PORT_STAT_C_ENABLE) {
		printf("[usb-hub] port %d enable change, status %x\n", i + 1, portstatus);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_C_ENABLE);
		/*
		 * The following hack causes a ghost device problem
		 * to Faraday EHCI
		 */
#ifndef CONFIG_USB_EHCI_FARADAY
		/*
		 * EM interference sometimes causes bad shielded USB
		 * devices to be shutdown by the hub, this hack enables
		 * them again. Works at least with mouse driver
		 */
		if (!(portstatus & USB_PORT_STAT_ENABLE) &&
		    (portstatus & USB_PORT_STAT_CONNECTION) &&
		    usb_device_has_child_on_port(dev, i)) {
			printf("already running port %i disabled by hub (EMI?), re-enabling...\n",
			      i + 1);
			usb_hub_port_connect_change(dev, i);
		}
#endif
	}

	if (portstatus & USB_PORT_STAT_SUSPEND) {
		printf("port %d suspend change\n", i + 1);
		usb_clear_port_feature(dev, i + 1, USB_PORT_FEAT_SUSPEND);
	}

	if (portchange & USB_PORT_STAT_C_OVERCURRENT) {
		printf("port %d over-current change\n", i + 1);
		usb_clear_port_feature(dev, i + 1,
				       USB_PORT_FEAT_C_OVER_CURRENT);
		/* Only power-on this one port */
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		hub->overcurrent_count[i]++;

		/*
		 * If the max-scan-count is not reached, return without removing
		 * the device from scan-list. This will re-issue a new scan.
		 */
		if (hub->overcurrent_count[i] <=
		    PORT_OVERCURRENT_MAX_SCAN_COUNT)
			return 0;

		/* Otherwise the device will get removed */
		printf("Port %d over-current occurred %d times\n", i + 1,
		       hub->overcurrent_count[i]);
	}

	/*
	 * We're done with this device, so let's remove this device from
	 * scanning list
	 */
	list_del(&usb_scan->list);
	free(usb_scan); // FIXME dynamic allocation

	return 0;
}

static int usb_device_list_scan(void)
{
	struct usb_device_scan *usb_scan;
	struct usb_device_scan *tmp;
	static int running=0;
	int ret = 0;

	/* Only run this loop once for each controller */
	if (running)
		return 0;

	running = 1;

	while (1) {
		/* We're done, once the list is empty again */
		if (list_empty(&usb_scan_list))
			goto out;

		list_for_each_entry_safe(usb_scan, tmp, &usb_scan_list, list) {
			int ret;

			/* Scan this port */
			ret = usb_scan_port(usb_scan);
			if (ret)
				goto out;
		}
	}

out:
	/*
	 * This USB controller has finished scanning all its connected
	 * USB devices. Set "running" back to 0, so that other USB controllers
	 * will scan their devices too.
	 */
	running = 0;

	return ret;
}

static struct usb_hub_device *usb_get_hub_device(struct usb_device *dev)
{
	(void)dev;
	struct usb_hub_device *hub;

	/* "allocate" Hub device */
	hub = usb_hub_allocate();

	return hub;
}

static int usb_hub_configure(struct usb_device *dev)
{
	int i, length;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, USB_BUFSIZ);
	unsigned char *bitmap;
	short hubCharacteristics;
	struct usb_hub_descriptor *descriptor;
	struct usb_hub_device *hub;
//	struct usb_hub_status *hubsts;
	int ret;

	hub = usb_get_hub_device(dev);
	if (hub == NULL)
		return -ENOMEM;
	hub->pusb_dev = dev;

	/* Get the the hub descriptor */
	ret = usb_get_hub_descriptor(dev, buffer, 4);
	if (ret < 0) {
		printf("usb_hub_configure: failed to get hub " \
		      "descriptor, giving up %lX\n", dev->status);
		return ret;
	}
	descriptor = (struct usb_hub_descriptor *)buffer;

	length = min_t(int, descriptor->bLength,
		       sizeof(struct usb_hub_descriptor));

	ret = usb_get_hub_descriptor(dev, buffer, length);
	if (ret < 0) {
		printf("usb_hub_configure: failed to get hub " \
		      "descriptor 2nd giving up %lX\n", dev->status);
		return ret;
	}
	memcpy((unsigned char *)&hub->desc, buffer, length);
	/* adjust 16bit values */
	put_unaligned(le16_to_cpu(get_unaligned(
			&descriptor->wHubCharacteristics)),
			&hub->desc.wHubCharacteristics);
	/* set the bitmap */
	bitmap = (unsigned char *)&hub->desc.u.hs.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (unsigned char *)&hub->desc.u.hs.PortPowerCtrlMask[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	for (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.DeviceRemovable[i] =
			descriptor->u.hs.DeviceRemovable[i];

	for (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.u.hs.PortPowerCtrlMask[i] =
			descriptor->u.hs.PortPowerCtrlMask[i];

	dev->maxchild = descriptor->bNbrPorts;
	//printf("[usb-hub] %d ports detected\n", dev->maxchild);

	hubCharacteristics = get_unaligned(&hub->desc.wHubCharacteristics);
	switch (hubCharacteristics & HUB_CHAR_LPSM) {
	case 0x00:
		//printf("[usb-hub] ganged power switching\n");
		break;
	case 0x01:
		//printf("[usb-hub] individual port power switching\n");
		break;
	case 0x02:
	case 0x03:
		printf("[usb-hub] unknown reserved power switching mode\n");
		break;
	}

	//if (hubCharacteristics & HUB_CHAR_COMPOUND)
	//	printf("[usb-hub] part of a compound device\n");
	//else
	//	printf("[usb-hub] standalone hub\n");

	/*switch (hubCharacteristics & HUB_CHAR_OCPM) {
	case 0x00:
		printf("[usb-hub] global over-current protection\n");
		break;
	case 0x08:
		printf("[usb-hub] individual port over-current protection\n");
		break;
	case 0x10:
	case 0x18:
		printf("[usb-hub] no over-current protection\n");
		break;
	}*/

	switch (dev->descriptor.bDeviceProtocol) {
	case USB_HUB_PR_FS:
		break;
	case USB_HUB_PR_HS_SINGLE_TT:
		//printf("Single TT\n");
		break;
	case USB_HUB_PR_HS_MULTI_TT:
		ret = usb_set_interface(dev, 0, 1);
		if (ret == 0) {
			//printf("TT per port\n");
			hub->tt.multi = true;
		} else {
			//printf("Using single TT (err %d)\n", ret);
		}
		break;
	case USB_HUB_PR_SS:
		/* USB 3.0 hubs don't have a TT */
		break;
	default:
		printf("Unrecognized hub protocol %d\n",
		      dev->descriptor.bDeviceProtocol);
		break;
	}

	/* Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns */
	switch (hubCharacteristics & HUB_CHAR_TTTT) {
	case HUB_TTTT_8_BITS:
		if (dev->descriptor.bDeviceProtocol != 0) {
			hub->tt.think_time = 666;
			//printf("[usb-hub] TT requires at most %d FS bit times (%d ns)\n",
			//      8, hub->tt.think_time);
		}
		break;
	case HUB_TTTT_16_BITS:
		hub->tt.think_time = 666 * 2;
		//printf("[usb-hub] TT requires at most %d FS bit times (%d ns)\n",
		//      16, hub->tt.think_time);
		break;
	case HUB_TTTT_24_BITS:
		hub->tt.think_time = 666 * 3;
		//printf("[usb-hub] TT requires at most %d FS bit times (%d ns)\n",
		//      24, hub->tt.think_time);
		break;
	case HUB_TTTT_32_BITS:
		hub->tt.think_time = 666 * 4;
		//printf("[usb-hub] TT requires at most %d FS bit times (%d ns)\n",
		//      32, hub->tt.think_time);
		break;
	}

	/*printf("[usb-hub] power on to power good time: %dms\n",
	      descriptor->bPwrOn2PwrGood * 2);
	printf("[usb-hub] hub controller current requirement: %dmA\n",
	      descriptor->bHubContrCurrent);*/

	/*for (i = 0; i < dev->maxchild; i++)
		printf("[usb-hub] port %d is%s removable\n", i + 1,
		      hub->desc.u.hs.DeviceRemovable[(i + 1) / 8] & \
		      (1 << ((i + 1) % 8)) ? " not" : "");*/

	if (sizeof(struct usb_hub_status) > USB_BUFSIZ) {
		printf("usb_hub_configure: failed to get Status - " \
		      "too long: %d\n", descriptor->bLength);
		return -EFBIG;
	}

	ret = usb_get_hub_status(dev, buffer);
	if (ret < 0) {
		printf("usb_hub_configure: failed to get Status %lX\n",
		      dev->status);
		return ret;
	}

//	hubsts = (struct usb_hub_status *)buffer;

	/*printf("get_hub_status returned status %X, change %X\n",
	      le16_to_cpu(hubsts->wHubStatus),
	      le16_to_cpu(hubsts->wHubChange));*/
	/*printf("local power source is %s\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_LOCAL_POWER) ? \
	      "lost (inactive)" : "good");*/
	/*printf("%sover-current condition exists\n",
	      (le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_OVERCURRENT) ? \
	      "" : "no ");*/

	usb_hub_power_on(hub);

	/*
	 * Reset any devices that may be in a bad state when applying
	 * the power.  This is a __weak function.  Resetting of the devices
	 * should occur in the board file of the device.
	 */
	for (i = 0; i < dev->maxchild; i++)
		usb_hub_reset_devices(hub, i + 1);

	/*
	 * Only add the connected USB devices, including potential hubs,
	 * to a scanning list. This list will get scanned and devices that
	 * are detected (either via port connected or via port timeout)
	 * will get removed from this list. Scanning of the devices on this
	 * list will continue until all devices are removed.
	 */
	for (i = 0; i < dev->maxchild; i++) {
		struct usb_device_scan *usb_scan;

		usb_scan = calloc(1, sizeof(*usb_scan)); // FIXME dynamic allocation
		if (!usb_scan) {
			printf("Can't allocate memory for USB device!\n");
			return -ENOMEM;
		}
		usb_scan->dev = dev;
		usb_scan->hub = hub;
		usb_scan->port = i;
		list_add_tail(&usb_scan->list, &usb_scan_list);
	}

	/*
	 * And now call the scanning code which loops over the generated list
	 */
	ret = usb_device_list_scan();

	return ret;
}

static int usb_hub_check(struct usb_device *dev, int ifnum)
{
	struct usb_interface *iface;
	struct usb_endpoint_descriptor *ep = NULL;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	if (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		goto err;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	if ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		goto err;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	if (iface->desc.bNumEndpoints != 1)
		goto err;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	if (!(ep->bEndpointAddress & USB_DIR_IN))
		goto err;
	/* If it's not an interrupt endpoint, we'd better punt! */
	if ((ep->bmAttributes & 3) != 3)
		goto err;
	/* We found a hub */
	printf("[usb-hub] USB hub found\n");
	return 0;

err:
	printf("[usb-hub] not a USB hub: bInterfaceClass=%d, bInterfaceSubClass=%d, bNumEndpoints=%d\n",
	      iface->desc.bInterfaceClass, iface->desc.bInterfaceSubClass,
	      iface->desc.bNumEndpoints);
	if (ep) {
		printf("   bEndpointAddress=%#x, bmAttributes=%d",
		      ep->bEndpointAddress, ep->bmAttributes);
	}

	return -ENOENT;
}

int usb_hub_probe(struct usb_device *dev, int ifnum)
{
	int ret;

	ret = usb_hub_check(dev, ifnum);
	if (ret)
		return 0;
	ret = usb_hub_configure(dev);
	return ret;
}
