/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * ZZ9000 Poseidon USB Hardware Driver (zzusbhw.device)
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
 * 
 * Z3660 Poseidon USB Hardware Driver (z3660_usb.device) based on ZZ9000 work by MiDWaN
 * 
 */

#include <exec/resident.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/tasks.h>
#include <exec/io.h>
#include <exec/execbase.h>

#include <libraries/expansion.h>

#include <devices/timer.h>
#include <devices/usbhardware.h>

#include <proto/exec.h>
#include <proto/expansion.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "z3660_usb.h"
#include "z3660_usb_str.h"

struct ExecBase* SysBase;

#define STR(s) #s
#define XSTR(s) STR(s)

/*
 * lib_IdString format matches the canonical Amiga pattern
 *   "name version.revision (date) description"
 * so tools like `version` and Poseidon's internal identification
 * routines parse it consistently.
 */
#define DEVICE_ID_STRING DEVICE_NAME " " XSTR(DEVICE_VERSION) "." XSTR(DEVICE_REVISION) \
    " (17.05.2026) Poseidon USB driver for Z3660 " \
    "(C) Copyright 2026 Dimitris Panokostas"

#define USB_DATA_ADDRESS 0x07F00000

/* USB request constants (from usb.h) */
#define USR_GET_STATUS        0x00
#define USR_CLEAR_FEATURE     0x01
#define USR_SET_FEATURE       0x03
#define USR_GET_DESCRIPTOR    0x06
#define USR_SET_ADDRESS       0x05
#define USR_GET_CONFIGURATION 0x08
#define USR_SET_CONFIGURATION 0x09

#define URTF_IN               0x80
#define URTF_STANDARD         0x00
#define URTF_CLASS            0x20
#define URTF_DEVICE           0x00
#define URTF_OTHER            0x03

#define UDT_DEVICE            0x01
#define UDT_CONFIGURATION     0x02
#define UDT_STRING            0x03
#define UDT_HUB               0x29

#define UPSF_PORT_CONNECTION  0x0100
#define UPSF_PORT_ENABLE      0x0200
#define UPSF_PORT_SUSPEND     0x0400
#define UPSF_PORT_OVER_CURRENT 0x0800
#define UPSF_PORT_RESET       0x1000
#define UPSF_PORT_POWER       0x0001
#define UPSF_PORT_LOW_SPEED   0x0002
#define UPSF_PORT_HIGH_SPEED  0x0004

#define UFS_PORT_POWER        0x08
#define UFS_PORT_RESET        0x04
#define UFS_PORT_ENABLE       0x01
#define UFS_PORT_SUSPEND      0x02
#define UFS_C_PORT_CONNECTION 0x10
#define UFS_C_PORT_ENABLE     0x11
#define UFS_C_PORT_SUSPEND    0x12
#define UFS_C_PORT_OVER_CURRENT 0x13
#define UFS_C_PORT_RESET      0x14

/* wPortChange bits use same UPSF_ layout as wPortStatus (byte-swapped USB spec). */
#define UPSF_C_PORT_CONNECTION 0x0100
#define UPSF_C_PORT_ENABLE     0x0200
#define UPSF_C_PORT_SUSPEND    0x0400
#define UPSF_C_PORT_OVER_CURRENT 0x0800
#define UPSF_C_PORT_RESET      0x1000

#define SWAP16(x) ((uint16_t)((uint16_t)(x) << 8) | ((uint16_t)(x) >> 8))
#define ZZ_RH_POLL_DELAY_TICKS 10
#define ZZ_INT_IDLE_REPLY_POLLS 10
#define ZZ_INT_PENDING_SLOTS 32
#define ZZ_ABORTED_REPLY_SLOTS (ZZ_INT_PENDING_SLOTS + ZZ_NUM_PORTS + 16)
#ifndef UHCF_USB20
#define UHCF_USB20 (1UL << 0)
#endif
#ifndef UHA_Capabilities
#define UHA_Capabilities (UHA_Dummy + 0x21)
#endif
#ifndef UHA_RootHubAddr
#define UHA_RootHubAddr (UHA_Dummy + 0x22)
#endif

static struct ZZUSBBase *PollBase;

/*
 * Saved seglist for expunge. Stored as a static rather than a struct
 * member because struct ZZUSBBase has a frozen v2.0.0 layout — the
 * hand-written AddTask inline asm in begin_io and the firmware-side
 * tooling expect specific field offsets, and inserting a member
 * shifts everything after it. Single-instance driver, so a static
 * is functionally equivalent.
 */
static uint8_t *DeviceSegList;
static struct IOUsbHWReq *RootHubIntPending[ZZ_NUM_PORTS];
static uint8_t RootHubPollDelay[ZZ_NUM_PORTS];

struct ZZIntPendingSlot {
    struct ZZUSBUnit *unit;
    struct IOUsbHWReq *ior;
    uint8_t idle_polls;
};

static struct ZZIntPendingSlot IntPendingSlots[ZZ_INT_PENDING_SLOTS];

static uint16_t request_speed(struct ZZUSBUnit *unit, struct IOUsbHWReq *ior);
static int write_tag_ulong(struct TagItem *tag, ULONG value);
static int write_tag_str(struct TagItem *tag, const char *value);

static void hotplug_poll_task(void);

asm("romtag:                                \n"
    "       dc.w    "XSTR(RTC_MATCHWORD)"   \n"
    "       dc.l    romtag                  \n"
    "       dc.l    endcode                 \n"
    "       dc.b    "XSTR(RTF_AUTOINIT)"    \n"
    "       dc.b    "XSTR(DEVICE_VERSION)"  \n"
    "       dc.b    "XSTR(NT_DEVICE)"       \n"
    "       dc.b    0                       \n"
    "       dc.l    _device_name            \n"
    "       dc.l    _device_id_string       \n"
    "       dc.l    _auto_init_tables       \n"
    "endcode:                               \n");

int __attribute__((no_reorder)) _start()
{
    return -1;
}
void _exit (int32_t status)
{
    (void) status;
    while (1)
    {}
}
const char device_name[] = DEVICE_NAME;
const char device_id_string[] = DEVICE_ID_STRING;

/*
 * AmigaOS `version DEVS:USBHardware/z3660_usb.device FILE` scans the
 * binary for a `$VER:` tag and prints the version/revision that
 * follows. Without the tag, `version` falls back to a generic "v1.0"
 * which defeats
 * field verification of driver deployment. Include a (date) after
 * the version number per standard Amiga $VER convention — some
 * tools require it to parse the version correctly.
 */
static const char __attribute__((used)) version_tag[] =
    "$VER: " DEVICE_NAME " " XSTR(DEVICE_VERSION) "." XSTR(DEVICE_REVISION)
    " (17.05.2026) Poseidon USB driver for Z3660 "
    "(C) Copyright 2026 Dimitris Panokostas";

typedef char ZZUSBCommand_size_must_match_protocol[
    (sizeof(struct ZZUSBCommand) == ZZUSB_CMD_SIZE) ? 1 : -1];

static struct ExecBase *get_sysbase(void)
{
/*
    struct ExecBase *sysbase;
    __asm volatile ("move.l 4.w,%0" : "=r"(sysbase));
    return sysbase;
*/
// Es esto equivalente?? por qué usar ensamblador??
    return *(struct ExecBase **)4UL;
}

/* Push a NUL-terminated string to the Z3660 serial debug channel. */
static void dstr(void* regs, const char *fmt, ...)
{
    
    static char message_buffer[256];  /* Static to avoid stack issues during crashes */
    va_list args;

    va_start(args, fmt);
    vsprintf(message_buffer, fmt, args);
    va_end(args);
    static int initialized = 0;
    if(!initialized) {
        // Send an initial newline to flush the channel and make the output more readable
        *((volatile uint32_t*)((uint8_t*)regs + REG_ZZ_DEBUG)) = 0xDB600001;  /* Magic marker for ARM */
        initialized = 1;
    }
    static char arm_message[280];  /* Static to avoid stack issues */
    sprintf(arm_message, "[AMG] %s", message_buffer);

    /* CACHE COHERENCY: Ensure data is visible to ARM before sending pointer */
    /* This cleans the CPU cache line containing our message to main memory */
    /* so the ARM can read the correct data from its cache-coherent view */
    {
        ULONG msg_len = strlen(arm_message) + 1;
        CachePreDMA((APTR)arm_message, &msg_len, 0);
        *((volatile uint32_t*)((uint8_t*)regs + REG_ZZ_DEBUG)) = (ULONG)arm_message;
        CachePostDMA((APTR)arm_message, &msg_len, 0);
    }
}

static void trace_port_state(struct ZZUSBUnit *unit, char *tag)
{
    void *regs = unit->zz_Registers;

    dstr(regs, "[z3660_usb] %s ps=%04X ch=%04X sp=%02X pr=%d dead=%d\n",
                  tag, unit->zz_PortStatus, unit->zz_PortChange,
                  unit->zz_Speed, unit->zz_PortPresent ? 1 : 0,
                  unit->zz_PortDead ? 1 : 0);
}

static void trace_port_state_status(struct ZZUSBUnit *unit,
                                    char *tag,
                                    uint16_t status,
                                    uint16_t speed)
{
    void *regs = unit->zz_Registers;

    dstr(regs, "[z3660_usb] %s rc=%04X fwsp=%02X ps=%04X ch=%04X\n",
                  tag, status, speed, unit->zz_PortStatus, unit->zz_PortChange);
}

static void trace_control_status(struct ZZUSBUnit *unit,
                                 char *tag,
                                 struct IOUsbHWReq *ior,
                                 uint16_t status)
{
    void *regs = unit->zz_Registers;

    dstr(regs, "[z3660_usb] %s st=%04X dev=%02X req=%02X/%02X fl=%04X sp=%02X split=%02X hub=%02X port=%02X\n",
                  tag, status, ior->iouh_DevAddr, ior->iouh_SetupData.bmRequestType,
                  ior->iouh_SetupData.bRequest, ior->iouh_Flags, request_speed(unit, ior),
                  (ior->iouh_Flags & UHFF_SPLITTRANS) ? 1 : 0, ior->iouh_SplitHubAddr,
                  ior->iouh_SplitHubPort);
}

static void __attribute__((unused)) trace_int_status(struct ZZUSBUnit *unit,
                                                     char *tag,
                                                     struct IOUsbHWReq *ior)
{
    void *regs = unit->zz_Registers;

    dstr(regs, "[z3660_usb] %s dev=%02X ep=%02X len=%04X fl=%04X split=%02X\n",
                  tag, ior->iouh_DevAddr, ior->iouh_Endpoint, ior->iouh_Length,
                  ior->iouh_Flags, (ior->iouh_Flags & UHFF_SPLITTRANS) ? 1 : 0);
}

static void trace_hub_int_data(struct ZZUSBUnit *unit,
                               struct IOUsbHWReq *ior,
                               uint32_t actual,
                               volatile uint8_t *data)
{
    void *regs = unit->zz_Registers;

    if (ior->iouh_Dir != UHDIR_IN || ior->iouh_Length > 2 || actual == 0)
        return;
    for (uint32_t i = 0; i < actual && i < 2; i++) {
        if (data[i] != 0)
            goto nonzero;
    }
    return;

nonzero:
    dstr(regs, "[z3660_usb] INT_DATA dev=%02X ep=%02X act=%04X d=%02X%02X\n",
                 ior->iouh_DevAddr, ior->iouh_Endpoint, actual, data[0], actual>0?data[1]:0);
}

static int is_hid_set_idle(struct IOUsbHWReq *ior)
{
    return ior->iouh_SetupData.bmRequestType == 0x21 &&
           ior->iouh_SetupData.bRequest == 0x0a &&
           ior->iouh_Length == 0;
}

/*
 * Alignment-safe memcpy. Replaces AmigaOS CopyMem, which has been
 * observed to silently no-op on this toolchain with GCC 15.2 when
 * either src or dst is at an odd address — Poseidon's iouh_Data
 * can legitimately arrive at odd alignments.
 *
 * The byte-loop at the bottom is the safe universal path. When
 * src and dst share 4-byte or 2-byte alignment we upgrade to
 * MOVE.L / MOVE.W copies. The fast path was proven correct
 * during mass-storage bring-up: FAT32 reads work end-to-end with
 * the fast path active, and the byte-level data verification
 * showed identical contents across repeat reads.
 */
static void safe_copy(const void *src, void *dst, uint32_t n)
{
    const uint8_t *s = (const uint8_t *)src;
    uint8_t *d = (uint8_t *)dst;
//    ULONG len = n;
//    CachePreDMA((APTR)dst, &len, 0);

    /* Long-aligned fast path — both divisible by 4.
     * Use exec's CopyMemQuick (movem.l-based, ~10× faster than the C
     * long-per-move loop). Requires 4-byte-aligned src+dst and size
     * multiple of 4, which this branch already guarantees. */
    if (n >= 4 && ((((uintptr_t)s | (uintptr_t)d) & 3) == 0)) {
        uint32_t bulk = n & ~3U;
        CopyMemQuick((APTR)s, (APTR)d, bulk);
        s += bulk;
        d += bulk;
        n &= 3;
    }
    /* Word-aligned fast path — both divisible by 2. */
    else if (n >= 2 && ((((uintptr_t)s | (uintptr_t)d) & 1) == 0)) {
        const uint16_t *ws = (const uint16_t *)s;
        uint16_t *wd = (uint16_t *)d;
        uint32_t words = n >> 1;
        while (words--) *wd++ = *ws++;
        s = (const uint8_t *)ws;
        d = (uint8_t *)wd;
        n &= 1;
    }

    /* Byte tail, or unaligned-all-the-way. */
    while (n--) *d++ = *s++;

//    CachePostDMA((APTR)dst, &len, 0);

}

static void safe_zero(void *dst, uint32_t n)
{
    uint8_t *d = (uint8_t *)dst;
    while (n--) *d++ = 0;
}

static uint16_t int_endpoint_key(struct IOUsbHWReq *ior)
{
    return ior->iouh_Endpoint & 0x0f;
}

static int int_slot_matches(struct ZZIntPendingSlot *slot,
                            struct ZZUSBUnit *unit,
                            struct IOUsbHWReq *ior)
{
    struct IOUsbHWReq *pending = slot->ior;

    return pending &&
        slot->unit == unit &&
        pending->iouh_DevAddr == ior->iouh_DevAddr &&
        int_endpoint_key(pending) == int_endpoint_key(ior) &&
        pending->iouh_Dir == ior->iouh_Dir;
}

static void clear_int_slot(struct ZZIntPendingSlot *slot)
{
    slot->unit = NULL;
    slot->ior = NULL;
    slot->idle_polls = 0;
}

static void reset_int_slots(void)
{
    for (int i = 0; i < ZZ_INT_PENDING_SLOTS; i++)
        clear_int_slot(&IntPendingSlots[i]);
}

static struct ZZIntPendingSlot *find_int_slot_for_ior(struct IOUsbHWReq *ior)
{
    for (int i = 0; i < ZZ_INT_PENDING_SLOTS; i++) {
        if (IntPendingSlots[i].ior == ior)
            return &IntPendingSlots[i];
    }
    return NULL;
}

static int queue_int_ior(struct ZZUSBUnit *unit,
                         struct IOUsbHWReq *ior,
                         struct IOUsbHWReq **replaced)
{
    struct ZZIntPendingSlot *free_slot = NULL;

    if (replaced)
        *replaced = NULL;

    for (int i = 0; i < ZZ_INT_PENDING_SLOTS; i++) {
        struct ZZIntPendingSlot *slot = &IntPendingSlots[i];
        if (int_slot_matches(slot, unit, ior)) {
            if (slot->ior && slot->ior != ior) {
                if (replaced)
                    *replaced = slot->ior;
                slot->ior->iouh_Actual = 0;
                slot->ior->iouh_Req.io_Error = IOERR_ABORTED;
            }
            slot->unit = unit;
            slot->ior = ior;
            slot->idle_polls = 0;
            return 1;
        }
        if (!slot->ior && !free_slot)
            free_slot = slot;
    }

    if (!free_slot)
        return 0;

    free_slot->unit = unit;
    free_slot->ior = ior;
    free_slot->idle_polls = 0;
    return 1;
}

static int int_pending_for_unit(struct ZZUSBUnit *unit)
{
    for (int i = 0; i < ZZ_INT_PENDING_SLOTS; i++) {
        if (IntPendingSlots[i].ior && IntPendingSlots[i].unit == unit)
            return 1;
    }
    return 0;
}

static int is_addr0_ep0(struct IOUsbHWReq *ior)
{
    return ior->iouh_DevAddr == 0 && ior->iouh_Endpoint == 0;
}

static int is_direct_root_request(struct ZZUSBUnit *unit)
{
    /*
     * Address zero is also used while enumerating devices behind an
     * external high-speed hub. In that case Poseidon supplies valid split
     * information and the root port itself remains high-speed. Only force
     * root-port handling when the physical root port is already known to be
     * FS/LS; this applies to every endpoint, not just address-zero EP0.
     */
    return unit->zz_PortPresent &&
           unit->zz_Speed != ZZUSB_SPEED_HIGH;
}

static int is_direct_root_addr0(struct ZZUSBUnit *unit, struct IOUsbHWReq *ior)
{
    return is_addr0_ep0(ior) && is_direct_root_request(unit);
}

static int is_addr0_get_device_desc(struct IOUsbHWReq *ior)
{
    return is_addr0_ep0(ior) &&
           ior->iouh_SetupData.bmRequestType == 0x80 &&
           ior->iouh_SetupData.bRequest == 0x06 &&
           SWAP16(ior->iouh_SetupData.wValue) == 0x0100;
}

static uint16_t request_speed(struct ZZUSBUnit *unit, struct IOUsbHWReq *ior)
{
    if (is_direct_root_request(unit))
        return unit->zz_Speed;
    if (ior->iouh_Flags & UHFF_SPLITTRANS) {
        return (ior->iouh_Flags & UHFF_LOWSPEED)
               ? ZZUSB_SPEED_LOW : ZZUSB_SPEED_FULL;
    }
    if (ior->iouh_Flags & UHFF_LOWSPEED)
        return ZZUSB_SPEED_LOW;
    if (ior->iouh_Flags & UHFF_HIGHSPEED)
        return ZZUSB_SPEED_HIGH;
    return unit->zz_Speed;
}

static void fill_split_fields(struct ZZUSBCommand *cmd,
                              struct ZZUSBUnit *unit,
                              struct IOUsbHWReq *ior)
{
    if (is_direct_root_request(unit))
        return;

    if ((ior->iouh_Flags & UHFF_SPLITTRANS) &&
        ior->iouh_SplitHubAddr != 0 &&
        ior->iouh_SplitHubPort != 0) {
        cmd->split_hub_addr = ior->iouh_SplitHubAddr;
        cmd->split_hub_port = ior->iouh_SplitHubPort;
        cmd->flags |= ZZUSB_FLAG_SPLIT;
    }
}

static void fill_root_reset_hint(struct ZZUSBCommand *cmd,
                                 struct ZZUSBUnit *unit)
{
    cmd->speed = unit->zz_Speed;
    /*
     * Full-speed and high-speed devices both present J before reset. Forcing
     * PFSC on a full-speed pre-reset hint prevents high-speed chirp, so only
     * force the FS/LS reset path for a confirmed low-speed root attach.
     */
    if (unit->zz_PortPresent && unit->zz_Speed == ZZUSB_SPEED_LOW)
        cmd->flags |= ZZUSB_FLAG_RESET_FSLS;
}

static void mark_direct_low_speed_unsupported(struct ZZUSBUnit *unit,
                                              char *tag)
{
    unit->zz_PortDead = TRUE;
    unit->zz_PortPresent = FALSE;
    unit->zz_Speed = ZZUSB_SPEED_LOW;
    unit->zz_PortStatus = UPSF_PORT_POWER;
    unit->zz_PortChange = UPSF_C_PORT_CONNECTION;
    unit->zz_BulkErrCount = 0;
    trace_port_state(unit, tag);
}

static int is_zero_report(volatile uint8_t *buf, uint32_t len)
{
    while (len--) {
        if (*buf++ != 0)
            return 0;
    }
    return 1;
}

static int send_usb_cmd(volatile uint8_t *base, struct ZZUSBCommand *cmd,
                        void *data_out, uint32_t data_out_len)
{
    volatile struct ZZUSBCommand *result =
        (volatile struct ZZUSBCommand*)(base + USB_DATA_ADDRESS);
    volatile int delay;

    cmd->status = ZZUSB_STATUS_PENDING;
    /*
     * Use safe_copy for everything. AmigaOS CopyMem has been observed
     * to silently no-op on this toolchain when either src or dst is
     * at an odd address. Poseidon can hand us iouh_Data buffers at
     * arbitrary alignments, so play it safe across the board.
     */
    safe_copy(cmd, (void*)(base + USB_DATA_ADDRESS), ZZUSB_CMD_SIZE);

    if (data_out && data_out_len > 0 && data_out_len <= ZZUSB_MAX_XFER) {
        safe_copy(data_out, (void*)(base + USB_DATA_ADDRESS + ZZUSB_DATA_OFFSET), data_out_len);
    }

//    CacheClearU();

//    dstr(base,"Sending command %s\n",zz_usb_cmd_str[cmd->cmd]);
     *((volatile uint32_t*)(base + REG_ZZ_USB_PROXY_CMD)) = cmd->cmd;

    /*
     * Tight busy-wait poll for firmware response.
     *
     * Prior revisions used an inner delay loop of 1000 empty
     * iterations between status reads (~200us on 68020 per check).
     * For a typical 1-5ms bulk transfer that wastes dozens of
     * 200us windows, adding milliseconds of per-round-trip
     * overhead that compounds over every bulk chunk. Benchmark
     * showed this as the dominant throughput bottleneck vs the
     * Deneb reference.
     *
     * Replaced with a short inner delay (~10us) so the poll rate
     * matches the firmware response timing without hammering the
     * Zorro bus. Outer loop still bounded by cmd->timeout_ms so
     * a wedged firmware releases zz_Lock within the specified
     * deadline.
     *
     * Inner delay ~10us + status-read overhead ~5us = ~15us per
     * iteration. Outer count = timeout_ms * 66 gives us roughly
     * the right wall-clock bound. timeout_ms=0 falls back to
     * ~3 sec (200,000 iterations).
     */
    {
        uint32_t outer_limit = cmd->timeout_ms
                               ? (uint32_t)cmd->timeout_ms * 66
                               : 200000;
        uint32_t timeout = outer_limit;
        while (timeout-- > 0) {
            if (result->status != ZZUSB_STATUS_PENDING) break;
            delay = 50;        /* ~10us on 68020 */
            while (delay--);
        }
    }
//    dstr(base,"Received status %s\n",zz_usb_status_str[result->status]);
    
    CacheClearU();

    if (result->status == ZZUSB_STATUS_PENDING) {
        dstr((void*)base, "[z3660_usb] CMD_TIMEOUT cmd=%02X dev=%02X ep=%02X wait=%04X\n",
                             cmd->cmd, cmd->dev_addr, cmd->endpoint, cmd->timeout_ms);
    }

    return result->status;
}

static void ensure_poll_task(struct ZZUSBBase *ZZBase)
{
    if (ZZBase->zz_PollTask)
        return;

    Forbid();
    if (!ZZBase->zz_PollTask) {
        struct Task *poll = &ZZBase->zz_PollTaskStorage;

        uint8_t *tp = (uint8_t*)poll;
        for (unsigned i = 0; i < sizeof(struct Task); i++)
            tp[i] = 0;

        poll->tc_Node.ln_Type = NT_TASK;
        poll->tc_Node.ln_Pri  = -1;
        poll->tc_Node.ln_Name = (char *)"z3660_usb.poll";

        poll->tc_SPLower = (APTR)&ZZBase->zz_PollStack[0];
        poll->tc_SPUpper = (APTR)&ZZBase->zz_PollStack[1024];
        poll->tc_SPReg   = (APTR)&ZZBase->zz_PollStack[1024];

        poll->tc_MemEntry.lh_Head =
            (struct Node *)&poll->tc_MemEntry.lh_Tail;
        poll->tc_MemEntry.lh_Tail = NULL;
        poll->tc_MemEntry.lh_TailPred =
            (struct Node *)&poll->tc_MemEntry.lh_Head;
        poll->tc_MemEntry.lh_Type = NT_MEMORY;

        /*
         * Reserve system signals 0..15 so AllocSignal(-1) in the task
         * body returns a user bit (16..31), and initialize nest counts
         * to the normal task state for exec variants that do not fix
         * them up inside AddTask().
         */
        poll->tc_SigAlloc  = 0xFFFF;
        poll->tc_IDNestCnt = -1;
        poll->tc_TDNestCnt = -1;

        PollBase = ZZBase;
        ZZBase->zz_PollTask = poll;

        {
/*
            struct Task *const _t = poll;
            const APTR _ipc = (APTR)hotplug_poll_task;
            const APTR _fpc = NULL;
            register struct Task *_a1 __asm("a1") = _t;
            register APTR _a2 __asm("a2") = _ipc;
            register APTR _a3 __asm("a3") = _fpc;
            register void *_a6 __asm("a6") = SysBase;
            __asm volatile ("jsr a6@(-0x11a:W)"
                : "+r"(_a1), "+r"(_a2), "+r"(_a3)
                : "r"(_a6)
                : "d0","d1","a0","cc","memory");
*/
// Esto es sustituible por esto?
            struct Task *poll = &ZZBase->zz_PollTaskStorage;
            AddTask(poll, hotplug_poll_task, NULL);
        }
    }
    Permit();
}

static struct Library* __attribute__((used)) init_device(uint8_t *seg_list asm("a0"), struct Library *dev asm("d0"))
{
    struct Library* ExpansionBase;
    struct ConfigDev* cd = NULL;
    uint8_t* registers = NULL;

    SysBase = get_sysbase();

    if (!(ExpansionBase = (struct Library*)OpenLibrary((uint8_t*)"expansion.library", 0L))) {
        return 0;
    }

#define Z3660_VENDOR        0x144B
#define Z3660_PRODUCT       1

    /* Prefer Zorro III (product 0x4) when available — Z3 bus is ~2× the
     * bandwidth of Z2 and dominates bulk throughput, since the mailbox
     * read/write at base+USB_DATA_ADDRESS is bus-bound rather than CPU-bound.
     * Fall back to Z2 (product 0x3) for cards installed in Z2 slots. */
    if ((cd = (struct ConfigDev*)FindConfigDev(cd, Z3660_VENDOR, Z3660_PRODUCT))) {
        registers = ((uint8_t*)cd->cd_BoardAddr);
    } else {
        CloseLibrary(ExpansionBase);
        return 0;
    }

    struct ZZUSBBase* ZZBase = (struct ZZUSBBase*)dev;
    if (!ZZBase) {
        CloseLibrary(ExpansionBase);
        return 0;
    }

    /* Saved for expunge so the loader can release our segments. */
    DeviceSegList = seg_list;

    dev->lib_Node.ln_Type = NT_DEVICE;
    dev->lib_Node.ln_Name = (char *)device_name;
    dev->lib_Version = DEVICE_VERSION;
    dev->lib_Revision = DEVICE_REVISION;
    dev->lib_IdString = (char *)device_id_string;

    InitSemaphore(&ZZBase->zz_Lock);

    dstr(registers, "[z3660_usb] " XSTR(DEVICE_VERSION) "." XSTR(DEVICE_REVISION) "\r\n");

    struct ZZUSBUnit* unit = &ZZBase->zz_Units[0];
    unit->zz_Registers = registers;
    unit->zz_Enabled = TRUE;
    unit->zz_PortPresent = FALSE;
    unit->zz_PortDead = FALSE;
    unit->zz_RootHubAddr = 0;
    unit->zz_PortChange = 0;
    unit->zz_PortStatus = UPSF_PORT_POWER;
    unit->zz_Speed = 0;
    unit->zz_BulkErrCount = 0;
    for (int ep = 0; ep < 16; ep++)
        unit->zz_IntPending[ep] = NULL;
    RootHubIntPending[0] = NULL;
    RootHubPollDelay[0] = 0;
    reset_int_slots();

    PollBase = ZZBase;
    /*
     * The poll task is created lazily on the first begin_io call,
     * not here. init_device runs during library AutoInit, before
     * MakeLibrary has sealed lib_Sum via SumLibrary; calling AddTask
     * in that window corrupts exec state and gurus with 80000004
     * (library checksum failure). See the matching block in
     * begin_io for the deferred creation.
     */
    ZZBase->zz_PollTask = NULL;

    unit->zz_Unit.unit_MsgPort.mp_MsgList.lh_Head = (struct Node *)&unit->zz_Unit.unit_MsgPort.mp_MsgList.lh_Tail;
    unit->zz_Unit.unit_MsgPort.mp_MsgList.lh_Tail = NULL;
    unit->zz_Unit.unit_MsgPort.mp_MsgList.lh_TailPred = (struct Node *)&unit->zz_Unit.unit_MsgPort.mp_MsgList.lh_Head;
    unit->zz_Unit.unit_flags = 0;
    unit->zz_Unit.unit_OpenCnt = 0;

    CloseLibrary(ExpansionBase);
    return dev;
}

/*
 * Unload helper, callable from both expunge and close. Caller must
 * ensure lib_OpenCnt is zero before invoking. Returns 0 (and re-asserts
 * LIBF_DELEXP) if the poll task is still alive — its Task struct and
 * stack live inside ZZBase, so freeing the base out from under it
 * would crash on the next wake-up. The poll task is created lazily
 * on the first INT transfer and never torn down in current builds,
 * so once it exists the driver effectively becomes non-unloadable;
 * acceptable for a hardware driver normally only unloaded at reboot.
 */
static uint8_t *unload_device(struct Library *dev)
{
    struct ZZUSBBase *ZZBase = (struct ZZUSBBase *)dev;

    if (ZZBase->zz_PollTask) {
        dev->lib_Flags |= LIBF_DELEXP;
        return 0;
    }

    uint8_t *seg = DeviceSegList;

    /*
     * Forbid() prevents another task from FindDevice'ing us between
     * the Remove and FreeMem and dereferencing a half-freed base.
     */
    Forbid();
    Remove(&dev->lib_Node);
    FreeMem((char *)dev - dev->lib_NegSize,
            dev->lib_NegSize + dev->lib_PosSize);
    Permit();
    return seg;
}

static uint8_t* __attribute__((used)) expunge(struct Library *dev asm("a6"))
{
    /*
     * Defer the unload if any unit is still open. Without this guard
     * the caller would FreeMem our base while live IORequests still
     * point at it, crashing on the next BeginIO.
     */
    if (dev->lib_OpenCnt) {
        dev->lib_Flags |= LIBF_DELEXP;
        return 0;
}
    return unload_device(dev);
}

static void __attribute__((used)) open(struct Library *dev asm("a6"), struct IOUsbHWReq *ior asm("a1"),
                 uint32_t unitnum asm("d0"), uint32_t flags asm("d1"))
{
    struct ZZUSBBase* ZZBase = (struct ZZUSBBase*)dev;
    int io_err = IOERR_OPENFAIL;

    if (!ior) {
        return;
    }

    if (unitnum < ZZ_NUM_PORTS) {
        struct ZZUSBUnit* unit = &ZZBase->zz_Units[unitnum];
        if (unit->zz_Enabled) {
            io_err = 0;
            ior->iouh_Req.io_Unit = (struct Unit*)unit;
            ior->iouh_Req.io_Unit->unit_flags = UNITF_ACTIVE;
            ior->iouh_Req.io_Unit->unit_OpenCnt++;
            dev->lib_OpenCnt++;
            /*
             * A pending expunge is being cancelled by this open — clear
             * only on success, otherwise a failed open would silently
             * lose a previous deferred-expunge request.
             */
            dev->lib_Flags &= ~LIBF_DELEXP;
        }
    }

    ior->iouh_Req.io_Error = io_err;
}

static uint8_t* __attribute__((used)) close(struct Library *dev asm("a6"), struct IOUsbHWReq *ior asm("a1"))
{
    if (ior) {
        struct Unit *unit = ior->iouh_Req.io_Unit;
        if (unit && unit != (struct Unit *)-1 && unit->unit_OpenCnt) {
            unit->unit_OpenCnt--;
        }
        /* Sentinel values let exec catch use-after-close on this IOR. */
        ior->iouh_Req.io_Unit = (struct Unit *)-1;
        ior->iouh_Req.io_Device = (struct Device *)-1;
    }

    if (dev->lib_OpenCnt) {
        dev->lib_OpenCnt--;
    }

    if (dev->lib_OpenCnt == 0 && (dev->lib_Flags & LIBF_DELEXP)) {
        return unload_device(dev);
    }
    return 0;
}

/*
 * Abort every queued downstream interrupt IOR for this unit with
 * UHIOERR_USBOFFLINE. Used on hot-unplug from both the hub-INT
 * path and any other future context that detects device loss
 * before poll_int_pending's own offline-mapping sees it.
 *
 * The aborted IORs are appended to the caller's pending-reply
 * array; the actual ReplyMsg happens after zz_Lock is released.
 */
static void abort_int_iors_offline(struct ZZUSBUnit *unit,
                                    struct IOUsbHWReq **aborted,
                                    int *aborted_count,
                                    int aborted_max)
{
    for (int i = 0; i < ZZ_INT_PENDING_SLOTS; i++) {
        struct ZZIntPendingSlot *slot = &IntPendingSlots[i];
        struct IOUsbHWReq *p = slot->ior;
        if (!p || slot->unit != unit) continue;
        clear_int_slot(slot);
        p->iouh_Actual = 0;
        p->iouh_Req.io_Error = UHIOERR_USBOFFLINE;
        if (aborted && aborted_count && *aborted_count < aborted_max) {
            aborted[(*aborted_count)++] = p;
        }
    }

    for (int ep = 1; ep < 16; ep++) {
        struct IOUsbHWReq *p = unit->zz_IntPending[ep];
        if (!p) continue;
        unit->zz_IntPending[ep] = NULL;
        p->iouh_Actual = 0;
        p->iouh_Req.io_Error = UHIOERR_USBOFFLINE;
        if (aborted && aborted_count && *aborted_count < aborted_max) {
            aborted[(*aborted_count)++] = p;
        }
    }
}

static void update_port_state(struct ZZUSBUnit *unit,
                              volatile uint8_t *base,
                              struct IOUsbHWReq **aborted,
                              int *aborted_count,
                              int aborted_max)
{
    struct ZZUSBCommand chk;
    memset(&chk, 0, sizeof(chk));
    chk.cmd = ZZUSB_CMD_CHECK_PORT;

    if (send_usb_cmd(base, &chk, NULL, 0) == ZZUSB_STATUS_OK) {
        /*
         * Firmware says the port is still physically connected.
         * If we previously marked the device dead after an
         * unrecoverable error, refuse to re-enumerate: keep the
         * port reported as empty to Poseidon until the user
         * physically unplugs (which firmware detects as CCS=0
         * and takes the else-branch below). Without this sticky
         * state, a device that keeps babbling would trigger an
         * infinite disconnect/reconnect/babble loop.
         */
        if (unit->zz_PortDead) {
            /* Stay in the "not present" state. No state change. */
            return;
        }

        volatile struct ZZUSBCommand *r =
            (volatile struct ZZUSBCommand*)(base + USB_DATA_ADDRESS);
        if (r->speed == ZZUSB_SPEED_LOW) {
            /*
             * The Zynq/ChipIdea root port does not complete direct
             * low-speed EP0. Hide these devices from Poseidon before it
             * starts enumeration; low-speed devices behind a high-speed
             * hub are still handled through split transactions because
             * the root port speed remains HIGH in that topology.
             */
            if (!unit->zz_PortDead)
                mark_direct_low_speed_unsupported(unit, "LS_ROOT_IGNORE");
            return;
        }

        UWORD port_status = UPSF_PORT_POWER | UPSF_PORT_CONNECTION;
        if (unit->zz_PortPresent && unit->zz_Speed == r->speed) {
            port_status |= unit->zz_PortStatus &
                           (UPSF_PORT_ENABLE | UPSF_PORT_SUSPEND);
        }
        if (r->speed == ZZUSB_SPEED_HIGH) {
            port_status |= UPSF_PORT_HIGH_SPEED;
        } else if (r->speed == ZZUSB_SPEED_LOW) {
            port_status |= UPSF_PORT_LOW_SPEED;
        }
        if (!unit->zz_PortPresent || unit->zz_Speed != r->speed) {
            unit->zz_PortPresent = TRUE;
            unit->zz_Speed = r->speed;
            unit->zz_PortStatus = port_status;
            unit->zz_PortChange = UPSF_C_PORT_CONNECTION;
            trace_port_state(unit, "PORT_CONNECT");
        } else {
            unit->zz_PortStatus = port_status;
        }
    } else {
        /*
         * Firmware says no device on the port (PHYSICAL disconnect).
         * Clear the PortDead sticky state — if the user replugs, we
         * want to re-enumerate normally. Then finish the disconnect
         * bookkeeping if we thought the device was present.
         */
        unit->zz_PortDead = FALSE;
        if (unit->zz_PortPresent) {
            unit->zz_PortPresent = FALSE;
            unit->zz_Speed = 0;
            unit->zz_PortStatus = UPSF_PORT_POWER;
            unit->zz_PortChange = UPSF_C_PORT_CONNECTION;
            unit->zz_BulkErrCount = 0;
            abort_int_iors_offline(unit, aborted, aborted_count, aborted_max);
            trace_port_state(unit, "PORT_DISCONNECT");
        }
    }
}

/*
 * Poll pending interrupt IORs on one unit. For each endpoint slot
 * with a pending IOR, issue an INT_XFER command to the firmware and
 * complete the request when firmware returns report data or a real
 * transfer error. Idle IN polls stay queued, matching Deneb-style
 * hardware-driver behaviour: Poseidon should not see a fresh reply
 * for every NAK/no-change interval.
 *
 * IMPORTANT: send_usb_cmd uses the single firmware mailbox, so the
 * poll task holds zz_Lock while the command is in flight. Keep the
 * firmware timeout bounded here so foreground Poseidon I/O is not held
 * indefinitely behind idle interrupt endpoints.
 */
static void poll_int_pending(struct ZZUSBBase *base_dev,
                             struct ZZUSBUnit *unit)
{
    volatile uint8_t *base = (volatile uint8_t*)unit->zz_Registers;

    for (int slot_index = 0; slot_index < ZZ_INT_PENDING_SLOTS; slot_index++) {
        struct ZZIntPendingSlot *slot = &IntPendingSlots[slot_index];
        struct IOUsbHWReq *reply_now = NULL;

        /*
         * Hold zz_Lock for the entire send_usb_cmd + result decode.
         * The shared firmware buffer is the serialized resource;
         * without this lock, begin_io and the poll task race each
         * other on the command mailbox. ReplyMsg is done AFTER the
         * lock is released to avoid re-entrancy deadlocks.
         */
        ObtainSemaphore(&base_dev->zz_Lock);

        if (!slot->ior || slot->unit != unit) {
            ReleaseSemaphore(&base_dev->zz_Lock);
            continue;
        }
        struct IOUsbHWReq *ior = slot->ior;

        struct ZZUSBCommand cmd;
        memset(&cmd, 0, sizeof(cmd));
        cmd.cmd = ZZUSB_CMD_INT_XFER;
        cmd.dev_addr = ior->iouh_DevAddr;
        cmd.endpoint = ior->iouh_Endpoint;
        cmd.direction = (ior->iouh_Dir == UHDIR_IN) ? 0x80 : 0x00;
        cmd.max_pkt_size = ior->iouh_MaxPktSize;
        cmd.speed = request_speed(unit, ior);
        cmd.data_length = ior->iouh_Length;
        cmd.interval = ior->iouh_Interval;
        fill_split_fields(&cmd, unit, ior);
        /*
         * Firmware's current interrupt helper polls the EHCI queue for
         * up to 16 ms before reporting an idle IN endpoint as a clean
         * zero-byte completion, then tears down the periodic queue with
         * a defensive 500 ms guard. A high-speed hub with no downstream
         * changes exercises this idle path continuously, so the mailbox
         * wait must cover the firmware's whole bounded path. Timing out
         * locally while firmware is still using the shared command buffer
         * races the next Poseidon request and can wedge the stack.
         */
        cmd.timeout_ms = 1000;

        uint16_t status = send_usb_cmd(base, &cmd,
                                       (ior->iouh_Dir == UHDIR_OUT) ? ior->iouh_Data : NULL,
                                       (ior->iouh_Dir == UHDIR_OUT) ? ior->iouh_Length : 0);

        if (status == ZZUSB_STATUS_OK ||
            (ior->iouh_Dir == UHDIR_IN &&
             (status == ZZUSB_STATUS_NAK ||
              status == ZZUSB_STATUS_TIMEOUT))) {
            volatile struct ZZUSBCommand *result =
                (volatile struct ZZUSBCommand*)(base + USB_DATA_ADDRESS);
            uint32_t actual = (status == ZZUSB_STATUS_OK)
                              ? result->actual_length : 0;

            /* Clamp against the caller's buffer length in case the
             * firmware returns a bogus actual_length after a data
             * buffer error — prevents heap corruption in Poseidon. */
            if (actual > ior->iouh_Length) actual = ior->iouh_Length;

            trace_hub_int_data(unit, ior, actual,
                (volatile uint8_t*)(base + USB_DATA_ADDRESS + ZZUSB_DATA_OFFSET));

            int idle_in = (ior->iouh_Dir == UHDIR_IN && actual == 0);
            if (ior->iouh_Dir == UHDIR_IN && actual > 0 && actual <= 2 &&
                is_zero_report((volatile uint8_t*)(base + USB_DATA_ADDRESS + ZZUSB_DATA_OFFSET),
                               actual)) {
                idle_in = 1;
            }
            int complete_idle = 0;

            if (idle_in) {
                /*
                 * Idle interrupt IN. Deneb leaves this IOR queued and
                 * only replies when the endpoint produces data. Some
                 * high-speed hubs return a one-byte all-zero status
                 * bitmap instead of NAKing when no downstream port
                 * changed; treat that as idle too. This driver still
                 * has to emulate the queue with synchronous firmware
                 * mailbox polls, so periodically complete an idle IOR
                 * to let Poseidon's bring-online code make progress.
                 * Keep the zero-report test limited to <=2 bytes so
                 * HID release reports are still delivered.
                 */
                if (slot->idle_polls < 0xff)
                    slot->idle_polls++;
                if (slot->idle_polls >= ZZ_INT_IDLE_REPLY_POLLS)
                    complete_idle = 1;
            } else if (actual > 0 || ior->iouh_Dir == UHDIR_OUT) {
                if (ior->iouh_Dir == UHDIR_IN && ior->iouh_Data) {
                    safe_copy((void*)(base + USB_DATA_ADDRESS + ZZUSB_DATA_OFFSET),
                              ior->iouh_Data, actual);
                }
            }

            if (complete_idle) {
                if (ior->iouh_Dir == UHDIR_IN && ior->iouh_Data) {
                    if (actual > 0) {
                        safe_copy((void*)(base + USB_DATA_ADDRESS + ZZUSB_DATA_OFFSET),
                                  ior->iouh_Data, actual);
                    } else if (ior->iouh_Length > 0) {
                        safe_zero(ior->iouh_Data, ior->iouh_Length);
                    }
                }
                ior->iouh_Actual = actual;
                ior->iouh_Req.io_Error = 0;
                clear_int_slot(slot);
                reply_now = ior;
            } else if (!idle_in) {
                ior->iouh_Actual = actual;
                ior->iouh_Req.io_Error = 0;
                clear_int_slot(slot);
                reply_now = ior;
            }
        } else {
            /*
             * Interrupt IN errors on idle HID-style endpoints can be
             * transient EHCI/poll artifacts. Replying UHIOERR_TIMEOUT
             * here makes Poseidon clear endpoint halt immediately and
             * retry, which we observed as an endless CLEAR_FEATURE loop
             * on EP2 IN that starves the whole OS. Treat non-offline
             * IN errors like an idle poll and periodically complete a
             * zero-byte result to keep Poseidon moving.
             */
            ior->iouh_Actual = 0;
            if (status != ZZUSB_STATUS_OFFLINE &&
                ior->iouh_Dir == UHDIR_IN) {
                int complete_idle = 0;
                if (slot->idle_polls < 0xff)
                    slot->idle_polls++;
                if (slot->idle_polls >= ZZ_INT_IDLE_REPLY_POLLS)
                    complete_idle = 1;
                if (complete_idle) {
                    if (ior->iouh_Data && ior->iouh_Length > 0)
                        safe_zero(ior->iouh_Data, ior->iouh_Length);
                    ior->iouh_Req.io_Error = 0;
                    clear_int_slot(slot);
                    reply_now = ior;
                }
            } else if (status == ZZUSB_STATUS_OFFLINE) {
                ior->iouh_Req.io_Error = UHIOERR_USBOFFLINE;
                clear_int_slot(slot);
                reply_now = ior;
            } else {
                ior->iouh_Req.io_Error = UHIOERR_TIMEOUT;
                clear_int_slot(slot);
                reply_now = ior;
            }
        }

        ReleaseSemaphore(&base_dev->zz_Lock);

        if (reply_now && !(reply_now->iouh_Req.io_Flags & IOF_QUICK)) {
            ReplyMsg(&reply_now->iouh_Req.io_Message);
        }
        return;
    }
}

static int poll_roothub_pending(struct ZZUSBBase *base_dev,
                                struct ZZUSBUnit *unit,
                                int unit_index)
{
    volatile uint8_t *base = (volatile uint8_t*)unit->zz_Registers;
    struct IOUsbHWReq *reply_now = NULL;
    struct IOUsbHWReq *aborted_replies[ZZ_ABORTED_REPLY_SLOTS];
    int aborted_count = 0;
    int still_pending;

    for (int i = 0; i < ZZ_ABORTED_REPLY_SLOTS; i++)
        aborted_replies[i] = NULL;

    ObtainSemaphore(&base_dev->zz_Lock);

    struct IOUsbHWReq *ior = RootHubIntPending[unit_index];
    if (!ior) {
        ReleaseSemaphore(&base_dev->zz_Lock);
        return 0;
    }

    if (unit->zz_PortChange == 0) {
        if (RootHubPollDelay[unit_index] > 0) {
            RootHubPollDelay[unit_index]--;
            ReleaseSemaphore(&base_dev->zz_Lock);
            return 1;
        }
        RootHubPollDelay[unit_index] = ZZ_RH_POLL_DELAY_TICKS;
        update_port_state(unit, base, aborted_replies,
                          &aborted_count, ZZ_ABORTED_REPLY_SLOTS);
    }

    if (unit->zz_PortChange != 0) {
        uint8_t change_bitmap[2] = { 0x02, 0x00 };
        uint16_t len = (ior->iouh_Length < 2) ? ior->iouh_Length : 2;

        trace_port_state(unit, "HUB_INT");
        safe_copy(change_bitmap, ior->iouh_Data, len);
        ior->iouh_Actual = len;
        ior->iouh_Req.io_Error = 0;
        RootHubIntPending[unit_index] = NULL;
        RootHubPollDelay[unit_index] = 0;
            reply_now = ior;
        }

    still_pending = RootHubIntPending[unit_index] != NULL;

        ReleaseSemaphore(&base_dev->zz_Lock);

        if (reply_now && !(reply_now->iouh_Req.io_Flags & IOF_QUICK)) {
            ReplyMsg(&reply_now->iouh_Req.io_Message);
        }
    for (int i = 0; i < aborted_count; i++) {
        struct IOUsbHWReq *p = aborted_replies[i];
        if (p && !(p->iouh_Req.io_Flags & IOF_QUICK)) {
            ReplyMsg(&p->iouh_Req.io_Message);
        }
    }

    return still_pending;
}

static void hotplug_poll_task(void)
{
    /*
     * Async interrupt-delivery loop. The v1.52 design:
     *  - begin_io UHCMD_INTXFER stashes downstream IORs in
     *    IntPendingSlots[] and root-hub IORs in RootHubIntPending[].
     *  - Signal() from begin_io wakes us up.
     *  - We scan pending IORs and reply on report data, root-hub
     *    changes, or hard errors.
     *  - Idle loop Wait()s on our signal; zero CPU when no pending.
     */
    BYTE sig = AllocSignal(-1);
    ULONG mask = (sig >= 0) ? (1UL << sig) : (1UL << 16);
    BYTE timer_sig = AllocSignal(-1);
    ULONG timer_mask = (timer_sig >= 0) ? (1UL << timer_sig) : 0;
    struct MsgPort timer_port;
    struct timerequest timer_req;
    BOOL timer_open = FALSE;

    PollBase->zz_PollSignal = mask;

    if (timer_mask) {
        memset(&timer_port, 0, sizeof(timer_port));
        memset(&timer_req, 0, sizeof(timer_req));
        timer_port.mp_Node.ln_Type = NT_MSGPORT;
        timer_port.mp_Flags = PA_SIGNAL;
        timer_port.mp_SigBit = timer_sig;
        timer_port.mp_SigTask = FindTask(NULL);
        timer_port.mp_MsgList.lh_Head =
            (struct Node *)&timer_port.mp_MsgList.lh_Tail;
        timer_port.mp_MsgList.lh_Tail = NULL;
        timer_port.mp_MsgList.lh_TailPred =
            (struct Node *)&timer_port.mp_MsgList.lh_Head;
        timer_port.mp_MsgList.lh_Type = NT_MESSAGE;
        timer_req.tr_node.io_Message.mn_Node.ln_Type = NT_MESSAGE;
        timer_req.tr_node.io_Message.mn_ReplyPort = &timer_port;
        timer_open = (OpenDevice((CONST_STRPTR)"timer.device", UNIT_MICROHZ,
                                 (struct IORequest*)&timer_req, 0) == 0);
    }

    for (;;) {
        int any_pending = 0;

        for (int u = 0; u < ZZ_NUM_PORTS; u++) {
            struct ZZUSBUnit *unit = &PollBase->zz_Units[u];
            if (!unit->zz_Enabled) continue;

            if (poll_roothub_pending(PollBase, unit, u)) {
                any_pending = 1;
            }

            poll_int_pending(PollBase, unit);

            if (int_pending_for_unit(unit))
                any_pending = 1;
        }

        if (!any_pending) {
            Wait(mask);
        } else if (timer_open) {
            /*
             * Pending interrupt IORs are normal. Do not spin in a CPU
             * delay loop here: that made the whole OS feel stuck while
             * Poseidon held a hub-status request open. Sleep on
             * timer.device and wake early if begin_io signals new work.
             */
            SetSignal(0, timer_mask);
            timer_req.tr_node.io_Command = TR_ADDREQUEST;
            timer_req.tr_time.tv_secs = 0;
            timer_req.tr_time.tv_micro = 100000;
            SendIO((struct IORequest*)&timer_req);
            Wait(mask | timer_mask);
            if (!CheckIO((struct IORequest*)&timer_req)) {
                AbortIO((struct IORequest*)&timer_req);
            }
            WaitIO((struct IORequest*)&timer_req);
        } else {
            Wait(mask);
        }
    }
}

static void handle_roothub_control(struct ZZUSBUnit *unit,
                                   struct IOUsbHWReq *ior)
{
    uint8_t reqtype = ior->iouh_SetupData.bmRequestType;
    uint8_t request = ior->iouh_SetupData.bRequest;
    uint16_t wValue = SWAP16(ior->iouh_SetupData.wValue);
    uint16_t wIndex = SWAP16(ior->iouh_SetupData.wIndex);
    uint16_t port = wIndex;

    if ((reqtype & 0x60) == URTF_STANDARD) {
        switch (request) {
        case USR_GET_STATUS:
            if ((reqtype & 0x1f) == URTF_DEVICE) {
                uint8_t status[2] = { 0x01, 0x00 };
                uint16_t len = (ior->iouh_Length < 2) ? ior->iouh_Length : 2;
                if (ior->iouh_Data) {
                    safe_copy(status, ior->iouh_Data, len);
                }
                ior->iouh_Actual = len;
                ior->iouh_Req.io_Error = 0;
                return;
            }
            break;
        case USR_GET_DESCRIPTOR:
            dstr(unit->zz_Registers, "[z3660_usb] GET_DESCRIPTOR: wValue=%04X\n", wValue);
            if (wValue == (UDT_DEVICE << 8)) {
                dstr(unit->zz_Registers, "[z3660_usb] DEVICE DESCRIPTOR REQUEST\n");
                static const uint8_t devdesc[18] = {
                    18, 0x01,             /* bLength, bDescriptorType = DEVICE */
                    0x00, 0x02,           /* bcdUSB = 2.0 */
                    0x09,                 /* bDeviceClass = HUB */
                    0x00,                 /* bDeviceSubClass */
                    0x01,                 /* bDeviceProtocol = single TT */
//                    0x02,                 /* bDeviceProtocol = multiple TT (per USB 2.0 hub spec) */
                    64,                   /* bMaxPacketSize0 */
                    (Z3660_VENDOR>>8)&0xFF,
                    (Z3660_VENDOR )&0xFF, /* idVendor */
                    0x00, 0x01,           /* idProduct */
                    0x01, 0x00,           /* bcdDevice */
                    0x01,                 /* iManufacturer */
                    0x02,                 /* iProduct */
                    0x00,                 /* iSerialNumber */
                    0x01                  /* bNumConfigurations */
                };
                uint16_t len = (ior->iouh_Length < 18) ? ior->iouh_Length : 18;
                if (ior->iouh_Data) {
                    safe_copy(devdesc, ior->iouh_Data, len);
                }
                ior->iouh_Actual = len;
                ior->iouh_Req.io_Error = 0;
                return;
            }
            if (wValue == (UDT_CONFIGURATION << 8)) {
                dstr(unit->zz_Registers, "[z3660_usb] CONFIG DESCRIPTOR REQUEST: wValue=%04X\n", wValue);
                static const uint8_t cfgdesc[25] = {
                    9, 0x02,              /* Config descriptor */
                    25, 0x00,             /* wTotalLength */
                    0x01,                 /* bNumInterfaces */
                    0x01,                 /* bConfigurationValue */
                    0x00,                 /* iConfiguration */
                    0xe0,                 /* bmAttributes: bus powered */
                    0x01,                 /* bMaxPower = 2mA */
                    /* Interface descriptor */
                    9, 0x04,              /* bLength, bDescriptorType = INTERFACE */
                    0x00,                 /* bInterfaceNumber */
                    0x00,                 /* bAlternateSetting */
                    0x01,                 /* bNumEndpoints */
                    0x09,                 /* bInterfaceClass = HUB */
                    0x00,                 /* bInterfaceSubClass */
                    0x00,                 /* bInterfaceProtocol */
                    0x00,                 /* iInterface */
                    /* Endpoint descriptor (interrupt IN) */
                    7, 0x05,              /* bLength, bDescriptorType = ENDPOINT */
                    0x81,                 /* bEndpointAddress = EP1 IN */
                    0x03,                 /* bmAttributes = INTERRUPT */
                    0x08, 0x00,
                    12
                };
                uint16_t len = (ior->iouh_Length < 25) ? ior->iouh_Length : 25;
                if (ior->iouh_Data) {
                    safe_copy((void*)cfgdesc, ior->iouh_Data, len);
                }
                ior->iouh_Actual = len;
                ior->iouh_Req.io_Error = 0;
                return;
            }
            if ((wValue >> 8) == UDT_HUB) {
                dstr(unit->zz_Registers, "[z3660_usb] HUB DESCRIPTOR REQUEST: wValue=%04X\n", wValue);
                uint8_t hubdesc[9] = {
                    9,                    /* bLength */
                    0x29,                 /* bDescriptorType = HUB */
                    ZZ_NUM_PORTS,         /* bNbrPorts */
                    0x00, 0x00,           /* wHubCharacteristics */
//                    0x08, 0x00,           /* wHubCharacteristics: multiple TTs, no power switching */
                    0x01,                 /* bPwrOn2PwrGood = 2ms */
                    0x00,                 /* bHubContrCurrent */
                    0x00,                 /* DeviceRemovable */
                    0xff                  /* PortPwrCtrlMask */
                };
                uint16_t len = (ior->iouh_Length < 9) ? ior->iouh_Length : 9;
                if (ior->iouh_Data) {
                    safe_copy((void*)hubdesc, ior->iouh_Data, len);
                }
                ior->iouh_Actual = len;
                ior->iouh_Req.io_Error = 0;
                return;
            }
            if ((wValue >> 8) == UDT_STRING) {
                uint8_t string_index = wValue & 0xff;
                dstr(unit->zz_Registers, "[z3660_usb] STRING DESCRIPTOR REQUEST: string_index=%d\n", string_index);
                if (string_index == 0) {
                    dstr(unit->zz_Registers, "[z3660_usb] STRING LANGID REQUEST\n");
                    uint8_t lang_desc[4] = {
                        4, 0x03, 0x09, 0x04
                    };
                    uint16_t len = (ior->iouh_Length < 4) ? ior->iouh_Length : 4;
                    if (ior->iouh_Data) {
                        safe_copy((void*)lang_desc, ior->iouh_Data, len);
                    }
                    ior->iouh_Actual = len;
                    ior->iouh_Req.io_Error = 0;
                    return;
                } else if (string_index == 1) {
                    /* Manufacturer: "MNT Research GmbH" — 13 chars
                     * UTF-16LE = 26 bytes + 2-byte header = 28. */
                    static const uint8_t mfr_desc[] = {
                        28, 0x03,
                        'D', 0, 'o', 0, 'u', 0, 'b', 0, 'l', 0, 'e', 0, ' ', 0,
                        'H', 0, ' ', 0,
                        'T', 0, 'e', 0, 'c', 0, 'h', 0,
                    };
                    uint16_t len = (ior->iouh_Length < mfr_desc[0]) ? ior->iouh_Length : mfr_desc[0];
                    if (ior->iouh_Data) {
                        safe_copy((void*)mfr_desc, ior->iouh_Data, len);
                    }
                    ior->iouh_Actual = len;
                    ior->iouh_Req.io_Error = 0;
                    return;
                } else if (string_index == 2) {
                     dstr(unit->zz_Registers, "[z3660_usb] STRING DESCRIPTOR: Product (index 2)\n");
                    /* Product: "Z3660 USB Root Hub" — 18 chars
                     * UTF-16LE = 36 bytes + 2-byte header = 38.
                     * Previous array declared bLength=24 but held
                     * only 22 bytes, so Poseidon read 2 bytes of
                     * garbage past the end and displayed the
                     * product as "ZZ9000 USB?". */
                    static const uint8_t prod_desc[] = {
                        38, 0x03,
                        'Z', 0, '3', 0, '6', 0, '6', 0,'0', 0, ' ', 0,
                        'U', 0, 'S', 0, 'B', 0, ' ', 0,
                        'R', 0, 'o', 0, 'o', 0, 't', 0, ' ', 0,
                        'H', 0, 'u', 0, 'b', 0
                    };
                    uint16_t len = (ior->iouh_Length < prod_desc[0]) ? ior->iouh_Length : prod_desc[0];
                    if (ior->iouh_Data) {
                        safe_copy((void*)prod_desc, ior->iouh_Data, len);
                    }
                    ior->iouh_Actual = len;
                    ior->iouh_Req.io_Error = 0;
                    return;
                }
            }
            break;

        case USR_SET_ADDRESS:
            unit->zz_RootHubAddr = wValue;
            ior->iouh_Actual = 0;
            ior->iouh_Req.io_Error = 0;
            return;

        case USR_GET_CONFIGURATION:
            {
                uint8_t cfg_val = 1;
                uint16_t len = (ior->iouh_Length < 1) ? ior->iouh_Length : 1;
                if (ior->iouh_Data) {
                    safe_copy(&cfg_val, ior->iouh_Data, len);
                }
                ior->iouh_Actual = len;
                ior->iouh_Req.io_Error = 0;
                return;
            }

        case USR_SET_CONFIGURATION:
            ior->iouh_Actual = 0;
            ior->iouh_Req.io_Error = 0;
            return;
        }
    }

    if ((reqtype & 0x60) == URTF_CLASS) {
        switch (request) {
        case USR_GET_DESCRIPTOR:
            if ((reqtype & 0x1f) == URTF_DEVICE && (wValue >> 8) == UDT_HUB) {
                uint8_t hubdesc[9] = {
                    9,
                    0x29,
                    ZZ_NUM_PORTS,
                    0x00, 0x00,
//                    0x08, 0x00,           /* wHubCharacteristics: multiple TTs, no power switching */
                    0x01,                 /* bPwrOn2PwrGood = 2ms */
                    0x00,                 /* bHubContrCurrent */
                    0x00,                 /* DeviceRemovable */
                    0xff                  /* PortPwrCtrlMask */
                };
                uint16_t len = (ior->iouh_Length < 9) ? ior->iouh_Length : 9;
                if (ior->iouh_Data) {
                    safe_copy((void*)hubdesc, ior->iouh_Data, len);
                }
                ior->iouh_Actual = len;
                ior->iouh_Req.io_Error = 0;
                return;
            }
            break;

        case USR_GET_STATUS:
            if ((reqtype & 0x1f) == URTF_DEVICE) {
                uint16_t hub_status[2] = { 0, 0 };
                uint16_t len = (ior->iouh_Length < 4) ? ior->iouh_Length : 4;
                if (ior->iouh_Data) {
                    safe_copy(hub_status, ior->iouh_Data, len);
                }
                ior->iouh_Actual = len;
                ior->iouh_Req.io_Error = 0;
                return;
            }
            if ((reqtype & 0x1f) == URTF_OTHER && port > 0 && port <= ZZ_NUM_PORTS) {
                uint16_t ps[2];
                ps[0] = unit->zz_PortStatus;
                ps[1] = unit->zz_PortChange;
                trace_port_state(unit, "GET_STATUS");
                uint16_t len = (ior->iouh_Length < 4) ? ior->iouh_Length : 4;
                if (ior->iouh_Data) {
                    safe_copy(ps, ior->iouh_Data, len);
                }
                ior->iouh_Actual = len;
                ior->iouh_Req.io_Error = 0;
                return;
            }
            break;

        case USR_SET_FEATURE:
            if ((reqtype & 0x1f) == URTF_DEVICE) {
                ior->iouh_Actual = 0;
                ior->iouh_Req.io_Error = 0;
                return;
            }
            if ((reqtype & 0x1f) == URTF_OTHER && port > 0 && port <= ZZ_NUM_PORTS) {
                switch (wValue) {
                case UFS_PORT_RESET:
                    {
                        if (unit->zz_PortDead) {
                            /*
                             * Direct root-port low-speed failed and was
                             * deliberately hidden until physical unplug.
                             * Poseidon may still have a queued hub-reset
                             * request from the failed enumeration path;
                             * answer it locally as an empty powered port.
                             * Do not touch firmware or re-enable speed bits.
                             */
                            unit->zz_PortPresent = FALSE;
                            unit->zz_PortStatus = UPSF_PORT_POWER;
                            unit->zz_PortChange &= ~UPSF_C_PORT_RESET;
                            trace_port_state(unit, "SET_RESET_DEAD");
                            ior->iouh_Actual = 0;
                            ior->iouh_Req.io_Error = 0;
                            return;
                        }

                        unit->zz_PortStatus |= UPSF_PORT_RESET;
                        unit->zz_PortChange &= ~UPSF_C_PORT_RESET;
                        trace_port_state(unit, "SET_RESET_START");

                        struct ZZUSBCommand rcmd;
                        volatile uint8_t *rbase = (volatile uint8_t*)unit->zz_Registers;
                        memset(&rcmd, 0, sizeof(rcmd));
                        rcmd.cmd = ZZUSB_CMD_RESET_PORT;
                        rcmd.timeout_ms = 5000;
                        fill_root_reset_hint(&rcmd, unit);

                        uint16_t rstatus = send_usb_cmd(rbase, &rcmd, NULL, 0);
                        uint16_t fw_speed = 0;
                        volatile struct ZZUSBCommand *rresult =
                            (volatile struct ZZUSBCommand*)(rbase + USB_DATA_ADDRESS);
                        fw_speed = rresult->speed;

                        unit->zz_PortStatus &= ~UPSF_PORT_RESET;
                        if ((rstatus == ZZUSB_STATUS_OK ||
                             rstatus == ZZUSB_STATUS_OFFLINE) &&
                            fw_speed == ZZUSB_SPEED_LOW) {
                            mark_direct_low_speed_unsupported(unit,
                                                              "LS_ROOT_IGNORE");
                            trace_port_state_status(unit, "SET_RESET_FW",
                                                    rstatus, fw_speed);
                            trace_port_state(unit, "SET_RESET_DONE");
                            ior->iouh_Actual = 0;
                            ior->iouh_Req.io_Error = 0;
                            return;
                        } else if (rstatus == ZZUSB_STATUS_OK) {
                            unit->zz_Speed = rresult->speed;
                            unit->zz_PortStatus |= UPSF_PORT_ENABLE;
                            unit->zz_PortStatus &= ~(UPSF_PORT_HIGH_SPEED |
                                                     UPSF_PORT_LOW_SPEED);
                            if (unit->zz_Speed == ZZUSB_SPEED_HIGH) {
                                unit->zz_PortStatus |= UPSF_PORT_HIGH_SPEED;
                            } else if (unit->zz_Speed == ZZUSB_SPEED_LOW) {
                                unit->zz_PortStatus |= UPSF_PORT_LOW_SPEED;
                            }
                        } else {
                            unit->zz_PortStatus &= ~(UPSF_PORT_ENABLE |
                                                     UPSF_PORT_HIGH_SPEED |
                                                     UPSF_PORT_LOW_SPEED);
                        }
                        unit->zz_PortChange |= UPSF_C_PORT_RESET;
                        trace_port_state_status(unit, "SET_RESET_FW",
                                                rstatus, fw_speed);
                        trace_port_state(unit, "SET_RESET_DONE");
                        ior->iouh_Actual = 0;
                        ior->iouh_Req.io_Error = 0;
                        return;
                    }
                case UFS_PORT_POWER:
                    unit->zz_PortStatus |= UPSF_PORT_POWER;
                    ior->iouh_Actual = 0;
                    ior->iouh_Req.io_Error = 0;
                    return;
                case UFS_PORT_SUSPEND:
                    unit->zz_PortStatus |= UPSF_PORT_SUSPEND;
                    ior->iouh_Actual = 0;
                    ior->iouh_Req.io_Error = 0;
                    return;
                }
            }
            break;

        case USR_CLEAR_FEATURE:
            if ((reqtype & 0x1f) == URTF_DEVICE) {
                ior->iouh_Actual = 0;
                ior->iouh_Req.io_Error = 0;
                return;
            }
            if ((reqtype & 0x1f) == URTF_OTHER && port > 0 && port <= ZZ_NUM_PORTS) {
                switch (wValue) {
                case UFS_PORT_ENABLE:
                    unit->zz_PortStatus &= ~UPSF_PORT_ENABLE;
                    trace_port_state(unit, "CLR_ENABLE");
                    ior->iouh_Actual = 0;
                    ior->iouh_Req.io_Error = 0;
                    return;
                case UFS_C_PORT_CONNECTION:
                    unit->zz_PortChange &= ~UPSF_C_PORT_CONNECTION;
                    trace_port_state(unit, "CLR_C_CONN");
                    ior->iouh_Actual = 0;
                    ior->iouh_Req.io_Error = 0;
                    return;
                case UFS_C_PORT_ENABLE:
                    unit->zz_PortChange &= ~UPSF_C_PORT_ENABLE;
                    trace_port_state(unit, "CLR_C_ENABLE");
                    ior->iouh_Actual = 0;
                    ior->iouh_Req.io_Error = 0;
                    return;
                case UFS_C_PORT_RESET:
                    unit->zz_PortChange &= ~UPSF_C_PORT_RESET;
                    trace_port_state(unit, "CLR_C_RESET");
                    ior->iouh_Actual = 0;
                    ior->iouh_Req.io_Error = 0;
                    return;
                case UFS_C_PORT_SUSPEND:
                    unit->zz_PortChange &= ~UPSF_C_PORT_SUSPEND;
                    trace_port_state(unit, "CLR_C_SUSP");
                    ior->iouh_Actual = 0;
                    ior->iouh_Req.io_Error = 0;
                    return;
                case UFS_C_PORT_OVER_CURRENT:
                    unit->zz_PortChange &= ~UPSF_C_PORT_OVER_CURRENT;
                    trace_port_state(unit, "CLR_C_OC");
                    ior->iouh_Actual = 0;
                    ior->iouh_Req.io_Error = 0;
                    return;
                }
            }
            break;
        }
    }

    ior->iouh_Req.io_Error = UHIOERR_STALL;
    ior->iouh_Actual = 0;
}

static int handle_roothub_int(struct ZZUSBUnit *unit,
                              int unit_index,
                              struct IOUsbHWReq *ior,
                              volatile uint8_t *base,
                              struct IOUsbHWReq **aborted,
                              int *aborted_count,
                              int aborted_max)
{
    if (ior->iouh_Endpoint == 1 && ior->iouh_Data) {
        /*
         * Re-check port state on every hub-INT poll unless
         * Poseidon hasn't yet acked the previous transition
         * (zz_PortChange != 0). This is how we pick up both
         * hot-plug and hot-unplug — without refreshing here,
         * once the port was known occupied we'd never see the
         * C_PORT_CONNECTION transition going the other way.
         */
        if (unit->zz_PortChange == 0) {
            update_port_state(unit, base, aborted, aborted_count, aborted_max);
        }
        if (unit->zz_PortChange == 0) {
            struct IOUsbHWReq *old = RootHubIntPending[unit_index];
            if (old && old != ior) {
                old->iouh_Actual = 0;
                old->iouh_Req.io_Error = IOERR_ABORTED;
                if (aborted && aborted_count && *aborted_count < aborted_max) {
                    aborted[(*aborted_count)++] = old;
                }
            }
            ior->iouh_Req.io_Flags &= ~IOF_QUICK;
            RootHubIntPending[unit_index] = ior;
            RootHubPollDelay[unit_index] = ZZ_RH_POLL_DELAY_TICKS;
            trace_int_status(unit, "RH_INT_WAIT", ior);
            return 1;
        }
        uint8_t change_bitmap[2] = { 0x02, 0x00 };
        uint16_t len = (ior->iouh_Length < 2) ? ior->iouh_Length : 2;
        trace_port_state(unit, "HUB_INT");
        safe_copy(change_bitmap, ior->iouh_Data, len);
        ior->iouh_Actual = len;
        ior->iouh_Req.io_Error = 0;
    } else {
        ior->iouh_Actual = 0;
        ior->iouh_Req.io_Error = UHIOERR_STALL;
    }
    return 0;
}

/*
 * Returns the number of UHA_* tags actually populated. Caller writes
 * this into iouh_Actual so NSD-aware Poseidon tools can detect tag
 * coverage.
 */
static int fill_querydevice_tags(struct ZZUSBUnit *unit, struct TagItem *tags)
{
    /*
     * Poseidon normally sends a flat taglist, but TagItem control
     * tags are legal API input. Handle them here so a TAG_MORE or
     * TAG_SKIP list cannot make the driver walk unrelated memory.
     * Like Deneb, ti_Data points at the caller's output storage.
     * Leave unknown newer Poseidon tags untouched.
     */
    int guard = 64;
    int count = 0;

    while (tags && guard-- > 0) {
        switch (tags->ti_Tag) {
        case TAG_DONE:
            return count;
        case TAG_IGNORE:
            tags++;
            continue;
        case TAG_MORE:
            tags = (struct TagItem *)tags->ti_Data;
            continue;
        case TAG_SKIP:
            tags += tags->ti_Data + 1;
            continue;
        case UHA_DriverVersion:
            count += write_tag_ulong(tags, 0x0200);
            break;
        case UHA_Version:
            count += write_tag_ulong(tags, DEVICE_VERSION);
            break;
        case UHA_Revision:
            count += write_tag_ulong(tags, DEVICE_REVISION);
            break;
        case UHA_State:
            count += write_tag_ulong(tags, UHSF_OPERATIONAL);
            break;
        case UHA_Manufacturer:
            count += write_tag_str(tags, "Double H Tech");
            break;
        case UHA_ProductName:
            count += write_tag_str(tags, "Z3660 USB Host Controller");
            break;
        case UHA_Description:
            count += write_tag_str(tags,
                "Poseidon USB hardware driver for the Z3660 "
                "ZTurn board (Zynq 7020 ChipIdea EHCI)");
            break;
        case UHA_Copyright:
            count += write_tag_str(tags,
                "(C) Copyright 2026 Dimitris Panokostas. "
                "Licensed under GNU GPL v3 or later.");
            break;
        case UHA_Capabilities:
            count += write_tag_ulong(tags, UHCF_USB20);
            break;
        case UHA_RootHubAddr:
            count += write_tag_ulong(tags, unit ? unit->zz_RootHubAddr : 0);
            break;
        default:
            break;
        }
        tags++;
    }
    return count;
}

/*
 * NSD (NewStyleDevice) capability table. Listed in the order Poseidon
 * tends to query. UHCMD_ISOXFER is intentionally absent — we return
 * UHIOERR_BADPARAMS for it, so advertising support would be a lie.
 */
#ifndef NSCMD_DEVICEQUERY
#define NSCMD_DEVICEQUERY 0x4000
#endif
#ifndef NSDEVTYPE_USBHARDWARE
#define NSDEVTYPE_USBHARDWARE 14
#endif
static int write_tag_ulong(struct TagItem *tag, ULONG value)
{
    ULONG *out = (ULONG *)(uintptr_t)tag->ti_Data;
    if (!out)
        return 0;
    *out = value;
    return 1;
}

static int write_tag_str(struct TagItem *tag, const char *value)
{
    STRPTR *out = (STRPTR *)(uintptr_t)tag->ti_Data;
    if (!out)
        return 0;
    *out = (STRPTR)value;
    return 1;
}

static const UWORD NSDSupportedCommands[] = {
    CMD_RESET,
    CMD_FLUSH,
    NSCMD_DEVICEQUERY,
    UHCMD_QUERYDEVICE,
    UHCMD_USBRESET,
    UHCMD_USBRESUME,
    UHCMD_USBSUSPEND,
    UHCMD_USBOPER,
    UHCMD_CONTROLXFER,
    UHCMD_BULKXFER,
    UHCMD_INTXFER,
    0
};

struct ZZNSDeviceQueryResult {
    ULONG  DevQueryFormat;
    ULONG  SizeAvailable;
    UWORD  DeviceType;
    UWORD  DeviceSubType;
    const UWORD *SupportedCommands;
};

static void __attribute__((used)) begin_io(struct Library *dev asm("a6"), struct IOUsbHWReq *ior asm("a1"))
{
    struct ZZUSBBase* ZZBase = (struct ZZUSBBase*)dev;
    struct ZZUSBUnit* unit;
    int deferred = 0;
    /*
     * IORs that we need to ReplyMsg AFTER releasing zz_Lock.
     * Calling ReplyMsg while holding a semaphore can cause
     * scheduling issues if the receiving task tries to re-enter
     * our driver. These are collected inside the switch while
     * the lock is held and replied once the lock is released.
     */
    struct IOUsbHWReq *deferred_old_ior = NULL;
    enum { ABORTED_REPLY_MAX = ZZ_ABORTED_REPLY_SLOTS };
    struct IOUsbHWReq *aborted_replies[ABORTED_REPLY_MAX];
    int aborted_count = 0;
    for (int _i = 0; _i < ABORTED_REPLY_MAX; _i++) aborted_replies[_i] = NULL;

    if (!ZZBase || !ior) return;

    unit = (struct ZZUSBUnit*)ior->iouh_Req.io_Unit;
    if (!unit) {
        ior->iouh_Req.io_Error = IOERR_NOCMD;
        if (!(ior->iouh_Req.io_Flags & IOF_QUICK)) {
            ReplyMsg(&ior->iouh_Req.io_Message);
        }
        return;
    }

    volatile uint8_t* base = (volatile uint8_t*)unit->zz_Registers;

    ObtainSemaphore(&ZZBase->zz_Lock);

    switch (ior->iouh_Req.io_Command) {
    case UHCMD_QUERYDEVICE:
        {
            int filled = fill_querydevice_tags(unit,
                (struct TagItem *)ior->iouh_Data);
            ior->iouh_Actual = filled;
            ior->iouh_Req.io_Error = 0;
        }
        break;

    case NSCMD_DEVICEQUERY:
        {
            /*
             * NSD probe. The IOR is only guaranteed to be sized as
             * IOStdReq (callers may not pass an IOUsbHWReq); use the
             * IOStdReq overlay for io_Data / io_Length / io_Actual.
             */
            struct IOStdReq *std = (struct IOStdReq *)ior;
            struct ZZNSDeviceQueryResult *q =
                (struct ZZNSDeviceQueryResult *)std->io_Data;
            /*
             * SizeAvailable is an output field per the NSD spec, but
             * real callers reuse the buffer between probes — strict
             * "must be zero on entry" enforcement (as Deneb does) makes
             * the second probe spuriously fail. We only validate the
             * fields the caller is unambiguously responsible for: a
             * non-null buffer, sufficient length, and DevQueryFormat
             * being the only format we know how to fill (0).
             */
            if (!q ||
                std->io_Length < sizeof(struct ZZNSDeviceQueryResult) ||
                q->DevQueryFormat != 0) {
                std->io_Error = IOERR_NOCMD;
                break;
            }
            q->SizeAvailable     = sizeof(struct ZZNSDeviceQueryResult);
            q->DeviceType        = NSDEVTYPE_USBHARDWARE;
            q->DeviceSubType     = 0;
            q->SupportedCommands = NSDSupportedCommands;
            std->io_Actual       = sizeof(struct ZZNSDeviceQueryResult);
            std->io_Error        = 0;
        }
        break;

    case UHCMD_USBRESET:
        {
            struct ZZUSBCommand cmd;
            memset(&cmd, 0, sizeof(cmd));
            cmd.cmd = ZZUSB_CMD_RESET_PORT;
            cmd.timeout_ms = 5000;
            fill_root_reset_hint(&cmd, unit);

            uint16_t status = send_usb_cmd(base, &cmd, NULL, 0);
            uint16_t fw_speed = 0;
            volatile struct ZZUSBCommand *result =
                (volatile struct ZZUSBCommand*)(base + USB_DATA_ADDRESS);
            fw_speed = result->speed;

            if (status == ZZUSB_STATUS_OK) {
                if (result->speed == ZZUSB_SPEED_LOW) {
                    mark_direct_low_speed_unsupported(unit, "LS_ROOT_IGNORE");
                    trace_port_state_status(unit, "USBRESET_FW",
                                            status, fw_speed);
                    trace_port_state(unit, "USBRESET_DONE");
                    ior->iouh_Req.io_Error = 0;
                    ior->iouh_State = UHSF_OPERATIONAL;
                    break;
                }
                /*
                 * Only flag POWER + CONNECTION + speed here.
                 * Poseidon's hub class drives the enable / C_RESET
                 * transitions via the subsequent hub SET_FEATURE
                 * and GET_STATUS control transfers handled in
                 * handle_roothub_control. Setting PE or C_RESET at
                 * this point triggers a Poseidon recovery path
                 * that can reboot the Amiga on device-online.
                 */
                UWORD port_status = UPSF_PORT_POWER | UPSF_PORT_CONNECTION;
                if (result->speed == ZZUSB_SPEED_HIGH) {
                    port_status |= UPSF_PORT_HIGH_SPEED;
                } else {
                    port_status |= UPSF_PORT_ENABLE;
                    if (result->speed == ZZUSB_SPEED_LOW) {
                        port_status |= UPSF_PORT_LOW_SPEED;
                    }
                }
                unit->zz_Speed = result->speed;
                unit->zz_PortPresent = TRUE;
                unit->zz_PortStatus = port_status;
                unit->zz_PortChange = UPSF_C_PORT_CONNECTION;
            } else {
                if (status == ZZUSB_STATUS_OFFLINE &&
                    fw_speed == ZZUSB_SPEED_LOW) {
                    mark_direct_low_speed_unsupported(unit, "LS_ROOT_IGNORE");
                } else {
                    unit->zz_PortPresent = FALSE;
                    unit->zz_PortStatus = UPSF_PORT_POWER;
                    unit->zz_PortChange = 0;
                    unit->zz_Speed = 0;
                }
            }
            trace_port_state_status(unit, "USBRESET_FW", status, fw_speed);
            trace_port_state(unit, "USBRESET_DONE");

            /*
             * USBRESET always-OK reporting (matches v1.53 behaviour).
             *
             * v1.58 tried to report UHIOERR_HOSTERROR on firmware
             * error for strictness, but that set up a contradiction
             * (io_Error = HOSTERROR + iouh_State = UHSF_OPERATIONAL)
             * that sent Poseidon down a recovery code path which
             * in turn tripped the poll task — net result: crash.
             *
             * "Empty port" is a normal operational state for a root
             * hub, not a hardware error. Report success; downstream
             * port-status / port-connection bits (set above) tell
             * Poseidon the port is simply unoccupied.
             */
            ior->iouh_Req.io_Error = 0;
            ior->iouh_State = UHSF_OPERATIONAL;
        }
        break;

    case UHCMD_CONTROLXFER:
        {
            uint16_t rh_addr = unit->zz_RootHubAddr;

            if ((rh_addr == 0 && ior->iouh_DevAddr == 0) || ior->iouh_DevAddr == rh_addr) {
                handle_roothub_control(unit, ior);
            } else {
                struct ZZUSBCommand cmd;
                uint16_t status;
                int setup_in;

                if (ior->iouh_Length > ZZUSB_MAX_XFER) {
                    ior->iouh_Req.io_Error = UHIOERR_PKTTOOLARGE;
                    break;
                }

                if (unit->zz_PortDead) {
                    ior->iouh_Actual = 0;
                    ior->iouh_Req.io_Error = UHIOERR_USBOFFLINE;
                    break;
                }

                setup_in = (ior->iouh_SetupData.bmRequestType & 0x80) != 0;

                memset(&cmd, 0, sizeof(cmd));
                cmd.cmd = ZZUSB_CMD_CONTROL_XFER;
                cmd.dev_addr = ior->iouh_DevAddr;
                cmd.endpoint = ior->iouh_Endpoint;
                cmd.direction = setup_in ? 0x80 : 0x00;
                cmd.max_pkt_size = ior->iouh_MaxPktSize;
                cmd.speed = request_speed(unit, ior);
                if (is_direct_root_addr0(unit, ior)) {
                    /*
                     * During address-0 enumeration Poseidon may not yet
                     * have reliable per-device speed flags. The root hub
                     * reset result is authoritative here; using a split
                     * or high-speed flag from the IOR can mis-drive a
                     * low-speed mouse as full-speed before it even has
                     * an address.
                     */
                    cmd.speed = unit->zz_Speed;
                }
                cmd.data_length = ior->iouh_Length;
                cmd.timeout_ms = (ior->iouh_Flags & UHFF_NAKTIMEOUT)
                                 ? (ior->iouh_NakTimeout ? ior->iouh_NakTimeout : 5000)
                                 : 0;
                if (is_direct_root_addr0(unit, ior) &&
                    unit->zz_Speed == ZZUSB_SPEED_LOW &&
                    is_addr0_get_device_desc(ior)) {
                    /*
                     * Firmware has its own short internal bound for
                     * this unsupported direct-LS root-port probe. Keep
                     * the Amiga-side mailbox wait comfortably longer so
                     * debug printing and EHCI cleanup cannot race the
                     * next Poseidon command into the shared mailbox.
                     */
                    cmd.timeout_ms = 250;
                }
                fill_split_fields(&cmd, unit, ior);

                cmd.setup_bRequestType = ior->iouh_SetupData.bmRequestType;
                cmd.setup_bRequest = ior->iouh_SetupData.bRequest;
                cmd.setup_wValue = ior->iouh_SetupData.wValue;
                cmd.setup_wIndex = ior->iouh_SetupData.wIndex;
                cmd.setup_wLength = ior->iouh_SetupData.wLength;

                status = send_usb_cmd(base, &cmd,
                                      (!setup_in) ? ior->iouh_Data : NULL,
                                      (!setup_in) ? ior->iouh_Length : 0);

                if (status == ZZUSB_STATUS_OK) {
                    volatile struct ZZUSBCommand *result =
                        (volatile struct ZZUSBCommand*)(base + USB_DATA_ADDRESS);
                    uint32_t actual = result->actual_length;

                    /*
                     * Clamp actual against the caller's iouh_Length
                     * in case firmware returns a bogus length after
                     * an EHCI data-buffer or transaction error. A
                     * garbage actual would make safe_copy overflow
                     * iouh_Data and corrupt Poseidon's heap.
                     */
                    if (actual > ior->iouh_Length) actual = ior->iouh_Length;

                    if (setup_in && ior->iouh_Data && actual > 0) {
                        /*
                         * safe_copy not CopyMem — Poseidon's iouh_Data
                         * can be odd-aligned on small descriptor reads,
                         * and CopyMem silently no-ops on this toolchain
                         * with odd endpoints, leaving caller with stale
                         * bytes (e.g. MaxPktSize0=178).
                         */
                        safe_copy((void*)(base + USB_DATA_ADDRESS + ZZUSB_DATA_OFFSET),
                                  ior->iouh_Data, actual);
                    }
                    ior->iouh_Actual = actual;
                    ior->iouh_Req.io_Error = 0;
                } else if (status == ZZUSB_STATUS_STALL && is_hid_set_idle(ior)) {
                    /*
                     * Several HID receivers accept enumeration but stall
                     * SET_IDLE. Treat that specific optional idle-rate
                     * request as non-fatal; otherwise Poseidon's HID
                     * recovery path can keep re-enumerating the device
                     * and eventually wedge the stack.
                     */
                    trace_control_status(unit, "HID_IDLE_STALL_IGN", ior, status);
                    ior->iouh_Actual = 0;
                    ior->iouh_Req.io_Error = 0;
                } else {
                    /*
                     * Control error handling. Let explicit STALL and
                     * OFFLINE through, but keep generic transaction
                     * failures retryable. The v1.53 "mark port dead +
                     * fake connection-change" recovery was implicated
                     * in HID-bring-up crashes and has been removed.
                     */
                    trace_control_status(unit, "CTRL_FAIL", ior, status);
                    ior->iouh_Actual = 0;
                    if (is_direct_root_addr0(unit, ior) &&
                        unit->zz_Speed == ZZUSB_SPEED_LOW &&
                        is_addr0_get_device_desc(ior)) {
                        /*
                         * Zynq/ChipIdea EHCI is not completing direct
                         * root-port LS EP0. Hide this device until
                         * physical unplug so Poseidon stops retrying and
                         * the Amiga remains responsive. LS devices behind
                         * a HS hub still use split transactions above.
                         */
                        unit->zz_PortDead = TRUE;
                        unit->zz_PortPresent = FALSE;
                        unit->zz_PortStatus = UPSF_PORT_POWER;
                        unit->zz_PortChange = UPSF_C_PORT_CONNECTION;
                        ior->iouh_Req.io_Error = UHIOERR_USBOFFLINE;
                        trace_port_state(unit, "LS_ROOT_OFFLINE");
                        break;
                    }
                    switch (status) {
                    case ZZUSB_STATUS_STALL:
                        ior->iouh_Req.io_Error = UHIOERR_STALL; break;
                    case ZZUSB_STATUS_TIMEOUT:
                    case ZZUSB_STATUS_NAK:
                        ior->iouh_Req.io_Error = UHIOERR_TIMEOUT; break;
                    case ZZUSB_STATUS_OFFLINE:
                        ior->iouh_Req.io_Error = UHIOERR_USBOFFLINE; break;
                    default:
                        /*
                         * Poseidon class drivers have repeatedly shown
                         * fragile recovery behaviour for specific host
                         * controller errors during enumeration. Treat
                         * non-stall/non-offline control failures as a
                         * retryable timeout; the UART trace still keeps
                         * the firmware status for diagnosis.
                         */
                        ior->iouh_Req.io_Error = UHIOERR_TIMEOUT; break;
                    }
                }
            }
        }
        break;

    case UHCMD_USBRESUME:
    case UHCMD_USBSUSPEND:
    case UHCMD_USBOPER:
        ior->iouh_Req.io_Error = 0;
        ior->iouh_State = UHSF_OPERATIONAL;
        break;

    case UHCMD_ISOXFER:
        /*
         * The mailbox protocol reserves ZZUSB_CMD_ISO_XFER, but the
         * current firmware dispatcher has no isochronous handler.
         * Fail explicitly instead of falling through to IOERR_NOCMD,
         * which makes this look like a broken hardware-driver command
         * rather than an unsupported transfer type.
         */
        ior->iouh_Actual = 0;
        ior->iouh_Req.io_Error = UHIOERR_BADPARAMS;
        break;

    case UHCMD_INTXFER:
        {
            uint16_t rh_addr = unit->zz_RootHubAddr;

            if (((rh_addr == 0 && ior->iouh_DevAddr == 0) || ior->iouh_DevAddr == rh_addr)
                && ior->iouh_Endpoint == 1) {
                int unit_index = (int)(unit - &ZZBase->zz_Units[0]);
                if (handle_roothub_int(unit, unit_index, ior, base,
                                       aborted_replies, &aborted_count,
                                       ABORTED_REPLY_MAX)) {
                    deferred = 1;
                    ensure_poll_task(ZZBase);
                    if (ZZBase->zz_PollTask && ZZBase->zz_PollSignal) {
                        Signal(ZZBase->zz_PollTask, ZZBase->zz_PollSignal);
                    }
                }
            } else {
                /*
                 * Async delivery for downstream interrupt endpoints.
                 * Stash the IOR, Signal the poll task, defer the
                 * reply. The task replies on report data, occasional
                 * idle completion, or real offline. This avoids a
                 * tight Poseidon-side interrupt-poll loop while the
                 * firmware-side EHCI poll remains bounded.
                 */
                if (ior->iouh_Length > ZZUSB_MAX_XFER) {
                    ior->iouh_Req.io_Error = UHIOERR_PKTTOOLARGE;
                    break;
                }

                uint16_t ep = ior->iouh_Endpoint & 0x0f;
                if (ep == 0) {
                    /* EP0 is not valid for interrupt transfers. */
                    ior->iouh_Req.io_Error = UHIOERR_STALL;
                    break;
                }

                ior->iouh_Req.io_Flags &= ~IOF_QUICK;
                if (!queue_int_ior(unit, ior, &deferred_old_ior)) {
                    ior->iouh_Req.io_Error = UHIOERR_OUTOFMEMORY;
                    break;
                }
                deferred = 1;      /* do NOT ReplyMsg at bottom */
                ensure_poll_task(ZZBase);

                if (ZZBase->zz_PollTask && ZZBase->zz_PollSignal) {
                    Signal(ZZBase->zz_PollTask, ZZBase->zz_PollSignal);
                }
            }
        }
        break;

    case UHCMD_BULKXFER:
        {
            /*
             * USB 2.0 forbids low-speed bulk; firmware behaviour is
             * undefined for this combination. Reject up front so a
             * misconfigured class driver gets a clear answer instead
             * of a silent stall later.
             */
            if (ior->iouh_Flags & UHFF_LOWSPEED) {
                ior->iouh_Actual = 0;
                ior->iouh_Req.io_Error = UHIOERR_BADPARAMS;
                break;
            }
            /*
             * Chunked bulk-transfer loop.
             *
             * Our firmware mailbox is a single 24KB shared buffer
             * (ZZUSB_MAX_XFER ~= 24512 bytes after the header). For
             * efficient large-file transfers, Poseidon's massstorage
             * class submits much larger bulk chunks (32KB / 64KB or
             * more). Earlier revisions rejected anything over that
             * limit with UHIOERR_PKTTOOLARGE, and Poseidon's bulk-
             * error recovery state machine didn't handle that code
             * gracefully — after a few such rejections Poseidon's
             * internal state would shred and hard-lock the Amiga,
             * without ever surfacing a guru (MuForce wasn't seeing
             * anything because it wasn't a trap-catchable fault —
             * it was exec state getting corrupted).
             *
             * Solution: loop, sending the transfer to firmware in
             * ZZUSB_MAX_XFER chunks, and return a single combined
             * reply. Short-read on BULK IN (firmware returns less
             * than requested) signals end-of-transfer per USB spec;
             * we stop the loop there.
             */
            struct ZZUSBCommand cmd;
            uint16_t status = ZZUSB_STATUS_OK;
            uint32_t remaining = ior->iouh_Length;
            uint32_t total_actual = 0;
            uint8_t *user_buf = (uint8_t *)ior->iouh_Data;

            /*
             * Bulk chunk size: 16 KB per EHCI transaction.
             *
             * Sweet spot found empirically. 8 KB (historical default)
             * left ~37% write / ~28% read throughput on the table due
             * to per-chunk fixed overhead (cache ops, EHCI queue setup,
             * mailbox round-trip). 16 KB amortises that overhead while
             * fitting in a single EHCI QTD (5 × 4 KB pages = 20 KB max
             * per QTD). 24 KB forces a 2-QTD chain whose second QTD
             * starves on AXI contention and returns EHCI Data Buffer
             * Errors (status=0x20), wedging the Amiga via Poseidon's
             * recovery path. Raising this again requires Vivado AXI QoS
             * tuning (lever #5) or a larger shared buffer (lever #3).
             */
            enum { BULK_CHUNK = 16384 };

            while (remaining > 0) {
                uint32_t chunk = (remaining > BULK_CHUNK)
                                 ? BULK_CHUNK : remaining;

                memset(&cmd, 0, sizeof(cmd));
                cmd.cmd = ZZUSB_CMD_BULK_XFER;
                cmd.dev_addr = ior->iouh_DevAddr;
                cmd.endpoint = ior->iouh_Endpoint;
                cmd.direction = (ior->iouh_Dir == UHDIR_IN) ? 0x80 : 0x00;
                cmd.max_pkt_size = ior->iouh_MaxPktSize;
                cmd.speed = request_speed(unit, ior);
                cmd.data_length = chunk;
                cmd.timeout_ms = (ior->iouh_Flags & UHFF_NAKTIMEOUT)
                                 ? (ior->iouh_NakTimeout ? ior->iouh_NakTimeout : 500)
                                 : 500;
                fill_split_fields(&cmd, unit, ior);

                status = send_usb_cmd(base, &cmd,
                                      (ior->iouh_Dir == UHDIR_OUT && user_buf) ? user_buf : NULL,
                                      (ior->iouh_Dir == UHDIR_OUT) ? chunk : 0);

                if (status != ZZUSB_STATUS_OK) {
                    break;      /* error — fall through to error handling */
                }

                {
                    volatile struct ZZUSBCommand *result =
                        (volatile struct ZZUSBCommand*)(base + USB_DATA_ADDRESS);
                    uint32_t actual = result->actual_length;

                    if (actual > chunk) actual = chunk;

                    if (ior->iouh_Dir == UHDIR_IN && user_buf && actual > 0) {
                        safe_copy((void*)(base + USB_DATA_ADDRESS + ZZUSB_DATA_OFFSET),
                                  user_buf, actual);
                    }

                    total_actual += actual;
                    user_buf += actual;
                    remaining -= actual;

                    /* Short packet on IN signals end-of-transfer
                     * per USB spec — stop looping, don't report an
                     * error. SCSI data-IN transfers routinely end
                     * on a short packet when the device is done. */
                    if (ior->iouh_Dir == UHDIR_IN && actual < chunk) {
                        break;
                    }

                    /* For OUT, if firmware reported 0 or partial
                     * we should also stop — can't push more if the
                     * device isn't accepting. */
                    if (ior->iouh_Dir == UHDIR_OUT && actual < chunk) {
                        break;
                    }
                }
            }

            if (status == ZZUSB_STATUS_OK) {
                ior->iouh_Actual = total_actual;
                unit->zz_BulkErrCount = 0;
                ior->iouh_Req.io_Error = 0;
            } else {
                /*
                 * Error path. Explicitly zero iouh_Actual so the
                 * caller doesn't misread a stale value as valid
                 * transfer length and over-read iouh_Data (which
                 * we did NOT populate on the error path). Observed:
                 * USB ethernet devices that glitch mid-transfer
                 * would return HOSTERROR with stale iouh_Actual,
                 * Poseidon's class driver would walk past the end
                 * of iouh_Data, and AmigaDOS would guru.
                 */
                /*
                 * Bulk error handling — bounded-retry policy.
                 *
                 * Default mapping: non-offline → UHIOERR_TIMEOUT so
                 * the class driver retries normally (avoids specific
                 * error codes that have historically crashed
                 * Poseidon class drivers).
                 *
                 * Escalation: after N consecutive failures on this
                 * unit, report UHIOERR_USBOFFLINE. That forces
                 * Poseidon to tear the device down cleanly instead
                 * of retrying forever. Sustained-retry loops
                 * (4000+ failures over a large file copy) were
                 * observed to eventually shred Poseidon's internal
                 * state and hard-lock the Amiga.
                 *
                 * Counter resets on any successful bulk, so a
                 * transient glitch doesn't cause a spurious
                 * detach.
                 */
                ior->iouh_Actual = 0;
                if (status == ZZUSB_STATUS_OFFLINE) {
                    ior->iouh_Req.io_Error = UHIOERR_USBOFFLINE;
                } else {
                    if (unit->zz_BulkErrCount < 0xFFFF)
                        unit->zz_BulkErrCount++;
                    if (unit->zz_BulkErrCount >= 16) {
                        /* Too many consecutive errors — give up on
                         * this device cleanly. */
                        ior->iouh_Req.io_Error = UHIOERR_USBOFFLINE;
                        unit->zz_PortDead = TRUE;
                    } else {
                        ior->iouh_Req.io_Error = UHIOERR_TIMEOUT;
                    }
                }
            }
        }
        break;

    case CMD_RESET:
    case CMD_FLUSH:
        /*
         * Abort every queued root/downstream interrupt IOR. Collect them
         * here; actual ReplyMsg happens AFTER zz_Lock is released
         * to avoid scheduling issues if a replied task immediately
         * re-enters our driver.
         */
        {
            int unit_index = (int)(unit - &ZZBase->zz_Units[0]);
            struct IOUsbHWReq *pending = RootHubIntPending[unit_index];
            if (pending) {
                RootHubIntPending[unit_index] = NULL;
                RootHubPollDelay[unit_index] = 0;
                pending->iouh_Actual = 0;
                pending->iouh_Req.io_Error = IOERR_ABORTED;
                if (aborted_count < ABORTED_REPLY_MAX)
                    aborted_replies[aborted_count++] = pending;
            }
        }
        for (int i = 0; i < ZZ_INT_PENDING_SLOTS; i++) {
            struct ZZIntPendingSlot *slot = &IntPendingSlots[i];
            struct IOUsbHWReq *pending = slot->ior;
            if (!pending || slot->unit != unit) continue;
            clear_int_slot(slot);
            pending->iouh_Actual = 0;
            pending->iouh_Req.io_Error = IOERR_ABORTED;
            if (aborted_count < ABORTED_REPLY_MAX)
                aborted_replies[aborted_count++] = pending;
        }
        ior->iouh_Req.io_Error = 0;
        break;
    default:
        ior->iouh_Req.io_Error = IOERR_NOCMD;
        break;
    }

    ReleaseSemaphore(&ZZBase->zz_Lock);

    /* Reply any IORs that were aborted during CMD_RESET/CMD_FLUSH
     * or pre-empted by a re-queue in UHCMD_INTXFER. Done after
     * the lock release to avoid re-entrancy issues. */
    if (deferred_old_ior &&
        !(deferred_old_ior->iouh_Req.io_Flags & IOF_QUICK)) {
        ReplyMsg(&deferred_old_ior->iouh_Req.io_Message);
    }
    for (int _i = 0; _i < aborted_count; _i++) {
        struct IOUsbHWReq *p = aborted_replies[_i];
        if (p && !(p->iouh_Req.io_Flags & IOF_QUICK)) {
            ReplyMsg(&p->iouh_Req.io_Message);
        }
    }

    if (!deferred) {
        if (!(ior->iouh_Req.io_Flags & IOF_QUICK)) {
            ReplyMsg(&ior->iouh_Req.io_Message);
        }
    }
}

static uint32_t __attribute__((used)) abort_io(struct Library *dev asm("a6"), struct IOUsbHWReq *ior asm("a1"))
{
    /*
     * Abort a queued downstream interrupt IOR. All access is
     * serialised by zz_Lock.
     */
    struct ZZUSBBase *ZZBase = (struct ZZUSBBase*)dev;
    if (!ior || !ZZBase) return IOERR_NOCMD;
    struct ZZUSBUnit *unit = (struct ZZUSBUnit *)ior->iouh_Req.io_Unit;
    if (!unit) {
        ior->iouh_Req.io_Error = IOERR_ABORTED;
        return IOERR_ABORTED;
    }

    ObtainSemaphore(&ZZBase->zz_Lock);
    int found = 0;
    int unit_index = (int)(unit - &ZZBase->zz_Units[0]);
    if (unit_index >= 0 && unit_index < ZZ_NUM_PORTS &&
        RootHubIntPending[unit_index] == ior) {
        RootHubIntPending[unit_index] = NULL;
        RootHubPollDelay[unit_index] = 0;
        found = 1;
    }
    {
        struct ZZIntPendingSlot *slot = find_int_slot_for_ior(ior);
        if (slot && slot->unit == unit) {
            clear_int_slot(slot);
            found = 1;
        }
    }
    ReleaseSemaphore(&ZZBase->zz_Lock);

    if (found) {
        ior->iouh_Actual = 0;
        ior->iouh_Req.io_Error = IOERR_ABORTED;
        if (!(ior->iouh_Req.io_Flags & IOF_QUICK)) {
            ReplyMsg(&ior->iouh_Req.io_Message);
        }
        return IOERR_ABORTED;
    }
    ior->iouh_Req.io_Error = IOERR_ABORTED;
    return IOERR_ABORTED;
}

static uint32_t device_vectors[] = {
    (uint32_t)open,
    (uint32_t)close,
    (uint32_t)expunge,
    0,
    (uint32_t)begin_io,
    (uint32_t)abort_io,
    -1
};

const uint32_t auto_init_tables[4] = {
    sizeof(struct ZZUSBBase),
    (uint32_t)device_vectors,
    0,
    (uint32_t)init_device
};
