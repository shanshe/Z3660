/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * ZZ9000 USB Proxy - ARM-side USB host controller command handler
 * for the Poseidon USB stack running on the m68k Amiga.
 *
 * Copyright (C) 2026 Dimitris Panokostas <midwan@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <sleep.h>
#include "xil_cache.h"
#include "usb_proxy.h"
#include "usb_proxy_str.h"
#include "usb/usb.h"
#include "usb/ehci.h"
#include "memorymap.h"

#ifndef ALIGN
#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#endif

static unsigned int toggle_bits[128][2];
static uint8_t dma_buf[24576] __attribute__((aligned(32)));

static struct ehci_ctrl *get_ehci_ctrl(void)
{
    struct usb_device *root = usb_get_dev_index(0);
    if (!root) return NULL;
    return (struct ehci_ctrl *)root->controller;
}

static int zz_speed_to_usb(int zz)
{
    switch (zz) {
    case ZZUSB_SPEED_LOW:  return USB_SPEED_LOW;
    case ZZUSB_SPEED_HIGH: return USB_SPEED_HIGH;
    case ZZUSB_SPEED_FULL: return USB_SPEED_FULL;
    default:               return USB_SPEED_FULL;
    }
}

static int get_real_speed_from_hw(void)
{
    struct ehci_ctrl *ctrl = get_ehci_ctrl();
    if (!ctrl) return -1;
    uint32_t reg = ehci_readl(&ctrl->hcor->or_portsc[0]);
    int pspd = (reg >> 26) & 0x3;
    if (pspd == 2) return 3;  /* USB_SPEED_HIGH = 3 in enum */
    if (pspd == 1) return 1;  /* USB_SPEED_LOW = 1 in enum */
    return 2;  /* USB_SPEED_FULL = 2 in enum */
}
static void prep_dev(struct usb_device *dev, int addr, int speed_usb,
                     int maxpkt, int endpoint)
{
    struct usb_device *root;

    memset(dev, 0, sizeof(*dev));
    dev->devnum = addr;
    dev->speed = speed_usb;
    /* create_pipe() encodes maxpacketsize in bits 0-6 (max 127).
     * Keep it at EP0 default (64) to avoid overflowing into devnum bits.
     * Per-endpoint maxpkt goes into epmaxpacketin/out[] instead. */
    dev->maxpacketsize = (endpoint == 0) ? maxpkt : 64;
    dev->controller = get_ehci_ctrl();
    dev->status = USB_ST_NOT_PROC;
    root = usb_get_dev_index(0);
    if (root && (speed_usb == USB_SPEED_LOW || speed_usb == USB_SPEED_FULL)) {
        dev->parent = root;
        dev->portnr = 1;
    }
    if (addr >= 0 && addr < 128) {
        dev->toggle[0] = toggle_bits[addr][0];
        dev->toggle[1] = toggle_bits[addr][1];
    }
    dev->epmaxpacketin[0] = (endpoint == 0) ? maxpkt : 64;
    dev->epmaxpacketout[0] = (endpoint == 0) ? maxpkt : 64;
    if (endpoint > 0 && endpoint < 16) {
        dev->epmaxpacketin[endpoint] = maxpkt;
        dev->epmaxpacketout[endpoint] = maxpkt;
    }
}

static void save_toggle(int addr, struct usb_device *dev)
{
    if (addr >= 0 && addr < 128) {
        toggle_bits[addr][0] = dev->toggle[0];
        toggle_bits[addr][1] = dev->toggle[1];
    }
}

static uint16_t usb_status_to_zz(unsigned long status)
{
    if (status & USB_ST_STALLED)
        return ZZUSB_STATUS_STALL;
    if (status & USB_ST_NAK_REC)
        return ZZUSB_STATUS_NAK;
    if (status & USB_ST_CRC_ERR)
        return ZZUSB_STATUS_CRC;
    if (status & (USB_ST_BABBLE_DET | USB_ST_BUF_ERR))
        return ZZUSB_STATUS_BABBLE;
    return ZZUSB_STATUS_ERROR;
}

/* PORTSC bits (Zynq/ChipIdea extensions beyond standard EHCI) */
#define PORTSC_PFSC  (1U << 24)  /* Port Force Full Speed Connect */
#define PORTSC_PHCD  (1U << 23)  /* PHY Clock Disable (suspend) */
#define PORTSC_LS_K  (1U << 10)  /* Line status K = low-speed attach */

static uint16_t handle_reset_port(volatile struct ZZUSBCommand *cmd)
{
    struct ehci_ctrl *ctrl = get_ehci_ctrl();
    if (!ctrl)
    {
        printf("handle_reset_port ZZUSB_STATUS_ERROR\n");
        return ZZUSB_STATUS_ERROR;
    }

    uint32_t *portsc = (uint32_t *)&ctrl->hcor->or_portsc[0];
    uint32_t reg_pre, reg, reg_after_hold;
    int ret;
    int is_low_speed_pre;

    reg_pre = ehci_readl(portsc);

    /*
     * If the port already has PR=1 latched from a prior wedged attempt,
     * drop it cleanly before re-asserting. We write the same value that
     * is currently in the register with PR cleared (and W1C bits
     * zeroed so we don't inadvertently ACK status changes).
     */
    if (reg_pre & EHCI_PS_PR) {
        reg = reg_pre & ~EHCI_PS_CLEAR;
        reg &= ~EHCI_PS_PR;
        ehci_writel(portsc, reg);
        /* brief settle */
        usleep(100);
        reg_pre = ehci_readl(portsc);
    }

    /*
     * Detect pre-reset line state. On a USB 2.0 root port, LS devices
     * show line status = K (D- pull-up); FS/HS devices show J.
     *
     * Line status bits 11:10; K = 01.
     */
    is_low_speed_pre = ((reg_pre & 0x00000C00) == PORTSC_LS_K);

    /*
     * Dynamic ULPI transceiver-select switch. HS mode is the default
     * resting state for the PHY (see ehci-zynq.c init), so HS devices
     * negotiate 480 Mbit/s natively. For LS devices, HS chirp
     * negotiation wedges the HC, so we drop the PHY to FS4LS mode
     * BEFORE asserting port reset. The call gives the PHY a few
     * microseconds to settle before we kick the reset.
     */
    ehci_zynq_set_phy_mode(is_low_speed_pre);
    usleep(100);

    /*
     * Assert reset: set PR=1, clear PE, and preserve other bits.
     * W1C bits (CSC/PEC/OCC) must be written as 0 so we don't
     * accidentally acknowledge changes we haven't processed yet.
     * For LS devices also set PFSC to disable HS chirp on the HC
     * side (defensive; PHY is already in FS4LS).
     */
    reg = reg_pre;
    reg &= ~EHCI_PS_CLEAR;
    reg |= EHCI_PS_PR;
    reg &= ~EHCI_PS_PE;
    if (is_low_speed_pre)
        reg |= PORTSC_PFSC;
    ehci_writel(portsc, reg);

    /* USB 2.0 spec: hold reset for 50ms on root ports. */
    usleep(50*1000);

    reg_after_hold = ehci_readl(portsc);

    /*
     * De-assert reset. Crucially, use the VALUE WE WROTE (with W1C
     * cleared) rather than re-reading PORTSC: a re-read may pick up
     * ChipIdea / Zynq-specific sticky bits (PSPD changing as the
     * PHY redetects LS, line-status transitioning through J/K/SE0)
     * whose write-back semantics differ from HS reset and were
     * wedging the low-speed reset path. This matches the in-tree
     * U-Boot EHCI root-hub RESET handler.
     */
    reg &= ~EHCI_PS_PR;
    ehci_writel(portsc, reg);

    /*
     * EHCI spec 2.3.9: the host controller must clear PR and
     * stabilize the port within 2 ms. Give it a bit of headroom
     * but don't sit here forever - on failure we want to return
     * status to Poseidon promptly so the Zorro bus keeps flowing.
     */
    ret = -1;
    for (int i = 0; i < 800; i++) {  /* 800 * 5us = 4ms bound */
        reg = ehci_readl(portsc);
        if (!(reg & EHCI_PS_PR)) { ret = 0; break; }
        usleep(5);
    }
    if (ret < 0) {
        reg = ehci_readl(portsc);
        printf("[usb-proxy] reset: PR didn't clear "
               "(pre=%08x after_hold=%08x now=%08x)\n",
               (unsigned int)reg_pre,
               (unsigned int)reg_after_hold,
               (unsigned int)reg);
        return ZZUSB_STATUS_ERROR;
    }

    reg = ehci_readl(portsc);

    if (!(reg & EHCI_PS_CS)) {
        printf("[usb-proxy] reset: no device connected after reset "
               "(portsc=%08x)\n", (unsigned int)reg);
        return ZZUSB_STATUS_ERROR;
    }

    if (!(reg & EHCI_PS_PE)) {
        printf("[usb-proxy] reset: port not enabled "
               "(portsc=%08x pre=%08x)\n",
               (unsigned int)reg, (unsigned int)reg_pre);
        return ZZUSB_STATUS_ERROR;
    }

    int pspd = PORTSC_PSPD(reg);
    int zz_speed;
    if (pspd == PORTSC_PSPD_HS) zz_speed = ZZUSB_SPEED_HIGH;
    else if (pspd == PORTSC_PSPD_LS) zz_speed = ZZUSB_SPEED_LOW;
    else zz_speed = ZZUSB_SPEED_FULL;

    /* USB 2.0 TRSTRCY recovery. 10 ms is the spec floor. */
    usleep(10*1000);

    toggle_bits[0][0] = 0;
    toggle_bits[0][1] = 0;

    printf("[usb-proxy] reset done: speed=%d portsc=%08x%s\n",
           zz_speed, (unsigned int)reg,
           is_low_speed_pre ? " (LS/PFSC path)" : "");
    put_be16(cmd->speed, zz_speed);
    return ZZUSB_STATUS_OK;
}

static uint16_t handle_control_xfer(volatile struct ZZUSBCommand *cmd,
                                uint8_t *data_buf)
{
    struct ehci_ctrl *ctrl = get_ehci_ctrl();
    if (!ctrl)
        return ZZUSB_STATUS_ERROR;

    int dev_addr = be32(cmd->dev_addr);
    int endpoint = be16(cmd->endpoint);
    int maxpkt = be16(cmd->max_pkt_size);
    int speed_zz = be16(cmd->speed);
    int speed_usb = zz_speed_to_usb(speed_zz);
    int hw_speed = get_real_speed_from_hw();
    int data_len = be32(cmd->data_length);

    /* No per-control-xfer trace in the hot path: the UART is
     * polled-blocking and every line costs milliseconds of
     * firmware time, turning a 1-second enumeration into 10+
     * seconds and a mass-storage mount into minutes. The
     * [usb-proxy] ctrl fail / set_addr / reset prints below
     * still fire for rare events and are enough for triage. */

    if (hw_speed >= 0 && hw_speed != speed_usb) {
        speed_usb = hw_speed;
    }

    if (endpoint == 0 && speed_usb == USB_SPEED_HIGH && maxpkt < 64)
        maxpkt = 64;

    struct usb_device dev;
    prep_dev(&dev, dev_addr, speed_usb, maxpkt, endpoint);

    unsigned long pipe;
    if (cmd->setup_bRequestType & 0x80)
        pipe = usb_rcvctrlpipe(&dev, endpoint);
    else
        pipe = usb_sndctrlpipe(&dev, endpoint);

    struct devrequest setup __attribute__((aligned(32)));
    setup.requesttype = cmd->setup_bRequestType;
    setup.request = cmd->setup_bRequest;
    setup.value = le16(cmd->setup_wValue);
    setup.index = le16(cmd->setup_wIndex);
    setup.length = le16(cmd->setup_wLength);

//    Xil_DCacheFlushRange((u32)&setup, ALIGN(sizeof(setup), 32));

    int is_in = (cmd->setup_bRequestType & 0x80) != 0;
    void *buf = NULL;

    if (data_len > 0) {
        if (!is_in) {
            memcpy(dma_buf, data_buf, data_len);
//            Xil_DCacheFlushRange((u32)dma_buf, ALIGN(data_len, 32));
        } else {
//            Xil_DCacheInvalidateRange((u32)dma_buf, ALIGN(data_len, 32));
        }
        buf = dma_buf;
    }

    int result = ehci_submit_async(&dev, pipe, buf, data_len, &setup);

    save_toggle(dev_addr, &dev);

    if (result >= 0 && dev.status == 0) {
        if (is_in && data_len > 0 && dev.act_len > 0) {
//            Xil_DCacheInvalidateRange((u32)dma_buf, ALIGN(dev.act_len, 32));
            memcpy(data_buf, dma_buf, dev.act_len);
        }
        put_be32(cmd->actual_length, dev.act_len);

        if (setup.requesttype == 0x00 && setup.request == 0x05) {
            int new_addr = setup.value;
            printf("[usb-proxy] set_addr: %d -> %d\n", dev_addr, new_addr);
            if (new_addr > 0 && new_addr < 128) {
                toggle_bits[new_addr][0] = 0;
                toggle_bits[new_addr][1] = 0;
                if (new_addr != dev_addr) {
                    toggle_bits[dev_addr][0] = 0;
                    toggle_bits[dev_addr][1] = 0;
                }
            }
            /* USB spec requires 2ms recovery time after SET_ADDRESS
             * before the next transaction to the device's new address.
             * Use 10ms for margin. */
            usleep(10000);
        }

        /*
         * CLEAR_FEATURE(ENDPOINT_HALT): bmRequestType=0x02 (standard,
         * endpoint recipient, host-to-device), bRequest=0x01,
         * wValue=0x0000 (ENDPOINT_HALT), wIndex encodes endpoint
         * number in bits 0-3 and direction (IN=1) in bit 7.
         *
         * The device resets its toggle to DATA0 on a successful
         * clear-halt. If we don't mirror that in our per-address
         * toggle cache, the next bulk/interrupt transfer on the
         * endpoint will use the stale toggle value - which looks
         * like a DATA0/DATA1 mismatch and fails. Mass-storage
         * STALL recovery stops working after the first halt.
         */
        if (setup.requesttype == 0x02 && setup.request == 0x01 &&
            setup.value == 0x0000 &&
            dev_addr >= 0 && dev_addr < 128) {
            int clr_ep  = setup.index & 0x0f;
            int clr_dir = (setup.index & 0x80) ? 1 : 0;
            toggle_bits[dev_addr][clr_dir] &= ~(1U << clr_ep);
        }
        return ZZUSB_STATUS_OK;
    } else {
        printf("[usb-proxy] ctrl fail: addr=%d ep=%d req=%02x/%02x result=%d status=%lx act_len=%d\n",
               dev_addr, endpoint, setup.requesttype, setup.request, result, dev.status, dev.act_len);
        put_be32(cmd->actual_length, 0);
        return usb_status_to_zz(dev.status);
    }
}

static uint16_t handle_bulk_xfer(volatile struct ZZUSBCommand *cmd,
                             uint8_t *data_buf)
{
    int dev_addr = be32(cmd->dev_addr);
    int endpoint = be16(cmd->endpoint);
    int maxpkt = be16(cmd->max_pkt_size);
    int speed_usb = zz_speed_to_usb(be16(cmd->speed));
    int data_len = be32(cmd->data_length);
    int is_in = (be16(cmd->direction) & 0x80);

    /* Per-transfer CBW decode disabled - uncomment for deep
     * mass-storage debugging. Uses volatile to avoid GCC -O2
     * coalescing byte reads into a misaligned LDR that traps. */

    struct usb_device dev;
    prep_dev(&dev, dev_addr, speed_usb, maxpkt, endpoint);

    unsigned long pipe;
    if (is_in)
        pipe = usb_rcvbulkpipe(&dev, endpoint);
    else
        pipe = usb_sndbulkpipe(&dev, endpoint);

    /* DMA directly from/to the shared mailbox (USB_DATA_ADDRESS
     * + ZZUSB_DATA_OFFSET). The mailbox is 64-byte aligned in DDR and
     * reachable by the EHCI DMA engine, so the dma_buf bounce copy is
     * pure overhead. Skipping it saves one 8KB DDR-DDR memcpy plus one
     * cache op per bulk chunk. Fall back to dma_buf if the caller did
     * not provide a buffer (shouldn't happen for bulk, but defensive). */
    uint8_t *xfer_buf = data_buf ? data_buf : dma_buf;

    if (data_len > 0) {
        if (!is_in) {
//            Xil_DCacheFlushRange((u32)xfer_buf, ALIGN(data_len, 32));
        } else {
//            Xil_DCacheInvalidateRange((u32)xfer_buf, ALIGN(data_len, 32));
        }
    }

    int result = ehci_submit_async(&dev, pipe, data_len > 0 ? xfer_buf : NULL,
                                    data_len, NULL);

    save_toggle(dev_addr, &dev);

    if (result >= 0 && dev.status == 0) {
        if (is_in && data_len > 0 && dev.act_len > 0) {
//            Xil_DCacheInvalidateRange((u32)xfer_buf, ALIGN(dev.act_len, 32));
        }
        put_be32(cmd->actual_length, dev.act_len);
        return ZZUSB_STATUS_OK;
    } else {
        /* Keep the failure print - it's rare and tells us why a
         * bulk transfer errored. */
        printf("[bulk] FAIL addr=%d ep=%d result=%d status=%lx\n",
               dev_addr, endpoint, result, dev.status);
        put_be32(cmd->actual_length, 0);
        return usb_status_to_zz(dev.status);
    }
}

static uint16_t handle_int_xfer(volatile struct ZZUSBCommand *cmd,
                            uint8_t *data_buf)
{
    int dev_addr = be32(cmd->dev_addr);
    int endpoint = be16(cmd->endpoint);
    int maxpkt = be16(cmd->max_pkt_size);
    int speed_usb = zz_speed_to_usb(be16(cmd->speed));
    int data_len = be32(cmd->data_length);
    int is_in = (be16(cmd->direction) & 0x80);

    struct usb_device dev;
    prep_dev(&dev, dev_addr, speed_usb, maxpkt, endpoint);

    unsigned long pipe;
    if (is_in)
        pipe = usb_rcvintpipe(&dev, endpoint);
    else
        pipe = usb_sndintpipe(&dev, endpoint);

    if (!is_in && data_len > 0) {
        memcpy(dma_buf, data_buf, data_len);
//        Xil_DCacheFlushRange((u32)dma_buf, ALIGN(data_len, 32));
    } else if (is_in && data_len > 0) {
//        Xil_DCacheInvalidateRange((u32)dma_buf, ALIGN(data_len, 32));
    }

    int result = submit_int_msg(&dev, pipe, data_len > 0 ? dma_buf : NULL,
                                 data_len, be16(cmd->interval));

    save_toggle(dev_addr, &dev);

    /*
     * Interrupt IN success path covers two sub-cases:
     *   - qTD retired with data: act_len > 0, copy buffer out.
     *   - poll window elapsed with no data (idle endpoint): act_len
     *     is 0 and status is 0. Reported as ZZUSB_STATUS_OK with
     *     actual_length=0, which Poseidon treats as "no report this
     *     cycle, keep polling" (NOT as endpoint timeout).
     * Only real device/transfer errors (stall, babble, data-buffer)
     * fall through to the error branch.
     */
    if (result >= 0 && dev.status == 0) {
        if (is_in && data_len > 0 && dev.act_len > 0) {
//            Xil_DCacheInvalidateRange((u32)dma_buf, ALIGN(dev.act_len, 32));
            memcpy(data_buf, dma_buf, dev.act_len);
        }
        put_be32(cmd->actual_length, dev.act_len);
        return ZZUSB_STATUS_OK;
    } else {
        put_be32(cmd->actual_length, 0);
        printf("[usb-proxy] int fail: addr=%d ep=%d result=%d status=%lx\n",
               dev_addr, endpoint, result, dev.status);
        return usb_status_to_zz(dev.status);
    }
}

static uint16_t handle_clear_stall(volatile struct ZZUSBCommand *cmd)
{
    int dev_addr = be32(cmd->dev_addr);
    int endpoint = be16(cmd->endpoint);
    int maxpkt = be16(cmd->max_pkt_size);
    int speed_usb = zz_speed_to_usb(be16(cmd->speed));

    struct usb_device dev;
    prep_dev(&dev, dev_addr, speed_usb, maxpkt, endpoint);

    /* Direction convention matches the rest of the driver: bit 7 of
     * `direction` selects IN (0x80) vs OUT (0x00). */
    unsigned long pipe;
    if (be16(cmd->direction) & 0x80)
        pipe = usb_rcvbulkpipe(&dev, endpoint);
    else
        pipe = usb_sndbulkpipe(&dev, endpoint);

    int result = usb_clear_halt(&dev, pipe);
    /* usb_clear_halt resets dev->toggle[] for the endpoint on
     * success; persist that back to our per-address cache so the
     * next bulk/int transfer sees DATA0. */
    save_toggle(dev_addr, &dev);
    return (result == 0) ? ZZUSB_STATUS_OK : ZZUSB_STATUS_ERROR;
}

static uint16_t handle_enumerate(volatile struct ZZUSBCommand *cmd,
                             uint8_t *data_buf)
{
    (void)data_buf;
    put_be32(cmd->actual_length, 0);
    return ZZUSB_STATUS_OK;
}

static int is_port_connected(void)
{
    struct ehci_ctrl *ctrl = get_ehci_ctrl();
    if (!ctrl) return 0;
    uint32_t portsc = ehci_readl(&ctrl->hcor->or_portsc[0]);
    return (portsc & EHCI_PS_CS) ? 1 : 0;
}

static uint16_t handle_check_port(volatile struct ZZUSBCommand *cmd)
{
    struct ehci_ctrl *ctrl = get_ehci_ctrl();
    if (!ctrl)
        return ZZUSB_STATUS_OFFLINE;

    uint32_t reg = ehci_readl(&ctrl->hcor->or_portsc[0]);

    if (!(reg & EHCI_PS_CS))
        return ZZUSB_STATUS_OFFLINE;

    int pspd = PORTSC_PSPD(reg);
    int zz_speed;
    if (pspd == PORTSC_PSPD_HS) zz_speed = ZZUSB_SPEED_HIGH;
    else if (pspd == PORTSC_PSPD_LS) zz_speed = ZZUSB_SPEED_LOW;
    else zz_speed = ZZUSB_SPEED_FULL;

    put_be16(cmd->speed, zz_speed);
    return ZZUSB_STATUS_OK;
}

uint16_t usb_proxy_handle_command(volatile struct ZZUSBCommand *cmd,
                              uint8_t *data_buf)
{
    uint16_t command = be16(cmd->cmd);
    put_be32(cmd->actual_length, 0);

    if (command != ZZUSB_CMD_RESET_PORT && command != ZZUSB_CMD_CHECK_PORT && !is_port_connected()) {
        printf("usb_proxy_handle_command return ZZUSB_STATUS_OFFLINE\n");
        return ZZUSB_STATUS_OFFLINE;
    }

    switch (command) {
    case ZZUSB_CMD_CONTROL_XFER:
        return handle_control_xfer(cmd, data_buf);
    case ZZUSB_CMD_BULK_XFER:
        return handle_bulk_xfer(cmd, data_buf);
    case ZZUSB_CMD_INT_XFER:
        return handle_int_xfer(cmd, data_buf);
    case ZZUSB_CMD_CLEAR_STALL:
        return handle_clear_stall(cmd);
    case ZZUSB_CMD_RESET_PORT:
        return handle_reset_port(cmd);
    case ZZUSB_CMD_CHECK_PORT:
        return handle_check_port(cmd);
    case ZZUSB_CMD_ENUMERATE:
        return handle_enumerate(cmd, data_buf);
    default:
        printf("[usb_proxy] error command unknown %d\n",command);
        return ZZUSB_STATUS_BADPARAM;
    }
}
