/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * ZZ9000 Poseidon USB Hardware Driver
 *
 * Copyright (C) 2026 Dimitris Panokostas <midwan@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This driver implements the Poseidon usbhardware.device interface,
 * allowing the Poseidon USB stack to use the ZZ9000's USB port for
 * all USB device types (HID, storage, networking, etc.).
 *
 * Architecture:
 *   The ZZ9000's USB controller (Xilinx Zynq PS7 EHCI) runs on the
 *   ARM core. The m68k Amiga side communicates with it via a
 *   register-based command protocol through Zorro address space.
 *
 *   m68k Poseidon driver <--registers--> ARM firmware <--> EHCI <--> USB devices
 *
 *   The ARM firmware already has a full EHCI/USB stack. This driver
 *   sends USB operation requests through a command mailbox protocol,
 *   and the ARM firmware executes them and returns results via
 *   shared memory.
 */

#ifndef ZZUSBHW_H
#define ZZUSBHW_H

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <devices/usbhardware.h>

#define DEVICE_NAME      "z3660_usb.device"
#define DEVICE_VERSION   2
#define DEVICE_REVISION  1

#define ZZ_NUM_PORTS     1

/*
 * USB Command Mailbox Protocol
 *
 * The m68k driver writes a command structure to the shared buffer
 * at card_base + 0xa000, then triggers the ARM firmware via a
 * register write. The ARM firmware processes the command and
 * writes the response back to the same buffer.
 *
 * This protocol requires matching ARM firmware support in
 * zz9000-firmware/ZZ9000_proto.sdk/ZZ9000OS/src/main.c
 */

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

/* USB speed types */
#define ZZUSB_SPEED_LOW         0
#define ZZUSB_SPEED_FULL        1
#define ZZUSB_SPEED_HIGH        2

#define ZZUSB_FLAG_SPLIT       0x0001
#define ZZUSB_FLAG_RESET_FSLS  0x0002

/*
 * Command structure layout in shared buffer.
 * Written at card_base + 0xa000 via Zorro II bus.
 *
 * Most fields are big-endian (m68k native), read by ARM with be16()/be32().
 * Setup packet fields (wValue, wIndex, wLength) are in USB little-endian,
 * read by ARM with le16().
 * uint8_t fields need no conversion.
 */
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
    uint16_t split_hub_addr; /* HS hub address for FS/LS split transactions */
    uint16_t split_hub_port; /* downstream hub port for split transactions */
    uint16_t flags;          /* ZZUSB_FLAG_* */
    uint16_t reserved;
    /* Data follows at ZZUSB_DATA_OFFSET. */
} __attribute__((packed));

#define ZZUSB_CMD_SIZE    48   /* command header including setup data */
#define ZZUSB_DATA_OFFSET 64   /* data starts at this offset (cache-line aligned) */

/* Size of shared buffer minus command header = max data per transfer */
#define ZZUSB_MAX_XFER    (24576 - ZZUSB_DATA_OFFSET)

#include "common/z3660_regs.h"

/*
 * Device base structure (extends struct Device/library)
 */
struct ZZUSBBase {
    struct Device      zz_Device;
    struct SignalSemaphore zz_Lock;
    struct Task       *zz_PollTask;
    ULONG              zz_PollSignal;       /* signal-mask the poll task waits on */
    struct Task        zz_PollTaskStorage;
    ULONG              zz_PollStack[1024];  /* match v2.0.0 layout exactly */
    struct ZZUSBUnit {
        struct Unit    zz_Unit;
        void*          zz_Registers;
        BOOL           zz_Enabled;
        BOOL           zz_PortPresent;
        BOOL           zz_PortDead;    /* device marked unusable after
                                        * unrecoverable error (babble,
                                        * stuck qTD). Sticky until
                                        * physical disconnect — prevents
                                        * infinite re-enumerate loops on
                                        * a device that keeps failing. */
        UWORD          zz_RootHubAddr;
        UWORD          zz_PortChange;
        UWORD          zz_PortStatus;
        UWORD          zz_Speed;
        /*
         * Reserved legacy async interrupt slots. The actual pending
         * table is driver-static and keyed by device address, endpoint,
         * and direction; this field stays here to preserve the frozen
         * device-base layout.
         */
        struct IOUsbHWReq *zz_IntPending[16];
        /*
         * Consecutive bulk-failure counter. Incremented when a
         * UHCMD_BULKXFER comes back non-OK from firmware; reset to
         * 0 on any successful bulk. If it crosses the threshold we
         * report UHIOERR_USBOFFLINE instead of TIMEOUT so Poseidon
         * tears the device down instead of looping forever. A
         * genuinely incompatible USB stick would otherwise keep
         * Poseidon retrying indefinitely, accumulating state until
         * something in the class driver shreds.
         */
        UWORD          zz_BulkErrCount;
    } zz_Units[ZZ_NUM_PORTS];
};

#endif /* ZZUSBHW_H */
