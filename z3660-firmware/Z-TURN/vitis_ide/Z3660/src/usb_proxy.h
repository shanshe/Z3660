/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * ZZ9000 USB Proxy — command mailbox protocol between the m68k
 * Poseidon USB driver and the ARM firmware's EHCI host stack.
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

#ifndef USB_PROXY_H
#define USB_PROXY_H

#include <xil_types.h>

/* Command types */
enum zz_usb_cmd {
    ZZUSB_CMD_NO_COMMAND   = 0x00,
    ZZUSB_CMD_CONTROL_XFER = 0x01,
    ZZUSB_CMD_BULK_XFER    = 0x02,
    ZZUSB_CMD_INT_XFER     = 0x03,
    ZZUSB_CMD_ISO_XFER     = 0x04,
    ZZUSB_CMD_RESET_PORT   = 0x05,
    ZZUSB_CMD_RESUME_PORT  = 0x06,
    ZZUSB_CMD_SUSPEND_PORT = 0x07,
    ZZUSB_CMD_ENUMERATE    = 0x08,
    ZZUSB_CMD_QUERY_DEVICE = 0x09,
    ZZUSB_CMD_SET_ADDRESS  = 0x0A,
    ZZUSB_CMD_CLEAR_STALL  = 0x0B,
    ZZUSB_CMD_CHECK_PORT   = 0x0C,
};

/* Command status codes (returned by ARM firmware) */
enum zz_usb_status {
   ZZUSB_STATUS_OK        = 0x00,
   ZZUSB_STATUS_PENDING   = 0x01,
   ZZUSB_STATUS_ERROR     = 0xFF,
   ZZUSB_STATUS_TIMEOUT   = 0xFE,
   ZZUSB_STATUS_STALL     = 0xFD,
   ZZUSB_STATUS_NAK       = 0xFC,
   ZZUSB_STATUS_CRC       = 0xFB,
   ZZUSB_STATUS_BABBLE    = 0xFA,
   ZZUSB_STATUS_OVERRUN   = 0xF9,
   ZZUSB_STATUS_UNDERRUN  = 0xF8,
   ZZUSB_STATUS_OFFLINE   = 0xF7,
   ZZUSB_STATUS_BADPARAM  = 0xF6,
};

#define ZZUSB_SPEED_LOW         0
#define ZZUSB_SPEED_FULL        1
#define ZZUSB_SPEED_HIGH        2

#define ZZUSB_CMD_SIZE          48
#define ZZUSB_DATA_OFFSET       64
#define ZZUSB_MAX_XFER          (24576 - ZZUSB_DATA_OFFSET)

struct ZZUSBCommand {
    uint16_t cmd;           /* ZZUSB_CMD_* */
    uint16_t status;        /* ZZUSB_STATUS_* (written by ARM on completion) */
    uint32_t dev_addr;      /* USB device address (0-127) */
    uint16_t endpoint;      /* endpoint number (0-15) */
    uint16_t direction;     /* 0=OUT, 0x80=IN */
    uint16_t xfer_type;     /* control/bulk/int/iso */
    uint16_t max_pkt_size;  /* max packet size for endpoint */
    uint32_t data_length;   /* total transfer length */
    uint32_t actual_length; /* actual bytes transferred (written by ARM) */
    uint32_t timeout_ms;    /* timeout in milliseconds */
    uint16_t speed;         /* device speed */
    uint16_t interval;      /* interrupt interval */
    /* Setup data for control transfers (8 bytes) */
    uint8_t  setup_bRequestType;
    uint8_t  setup_bRequest;
    uint16_t setup_wValue;
    uint16_t setup_wIndex;
    uint16_t setup_wLength;
    uint8_t  reserved[8];
} __attribute__((packed));

static inline uint16_t be16(const volatile uint16_t p) {
    return (__builtin_bswap16(p));
}

/* Read a 16-bit value stored in USB little-endian byte order.
 * The m68k Poseidon driver copies setup packet fields (wValue, wIndex,
 * wLength) directly from its IoUsbHW API structure, which stores them
 * in USB-native little-endian format.  Using be16() on these would
 * incorrectly byte-swap them. */
static inline uint16_t le16(const volatile uint16_t p) {
    return(p);
}

static inline uint32_t be32(const volatile uint32_t p) {
    return(__builtin_bswap32(p));
}

#define put_be32(p,v) do{p=__builtin_bswap32(v);}while(0)
#define put_be16(p,v) do{p=__builtin_bswap16(v);}while(0)

uint16_t usb_proxy_handle_command(volatile struct ZZUSBCommand *cmd, uint8_t *data_buf);

#endif
