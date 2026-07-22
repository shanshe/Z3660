// SPDX-License-Identifier: MIT
//
// z3660_floppy.device - AmigaOS trackdisk-compatible driver
// Reads/writes .ADF disk image files via Z3660 FPGA registers
//
// Based on z3660_scsi.device pattern
// Implements trackdisk.device commands for floppy disk access
// through ADF image files stored on the ARM side of the Z3660

#include <exec/resident.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/alerts.h>
#include <exec/tasks.h>
#include <exec/io.h>
#include <exec/execbase.h>
#include "debug.h"

#include <libraries/expansion.h>
#include <libraries/expansionbase.h>

#include <devices/trackdisk.h>
#include <devices/timer.h>

#include <dos/filehandler.h>

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/dos.h>

void WaitTOF(void);

#include "newstyle.h"

#include "z3660_floppy_enums.h"
#include <stdint.h>
#include <string.h>

#include "z3660_floppy.h"

#pragma pack(4)

struct pifloppy_base {
    struct Device* pi_dev;
    struct pifloppy_unit {
        struct Unit unit;
        uint32_t regs_ptr;

        uint8_t enabled;
        uint8_t present;
        uint8_t valid;
        uint8_t read_only;
        uint8_t motor;
        uint8_t unit_num;
        uint16_t h, s;
        uint32_t c;

        uint32_t change_num;
        uint8_t disk_type;      // FLOPPY_TYPE_DD or FLOPPY_TYPE_HD
        uint8_t disk_inserted;  // 1 if ADF is loaded
        uint32_t block_size;   // 512 for DD, 512 for HD
        uint32_t total_blocks;  // 1760 for DD, 3520 for HD
        struct List change_int;  // List of change interrupts
    } units[NUM_FLOPPY_UNITS];
};

struct ExecBase *SysBase;

#define WRITELONG(cmd, val) *(volatile uint32_t *)((uint32_t)(Z3660_REGS + PIFLOPPY_OFFSET + (cmd))) = (val);
#define READLONG(cmd, var) var = *(volatile uint32_t *)((uint32_t)(Z3660_REGS + PIFLOPPY_OFFSET + (cmd)));

#define WRITE_CMD(COMMAND,UNIT,DATA,LEN)  do{               \
            ULONG len2=LEN;                                 \
            CachePreDMA((APTR)(DATA),&len2,0);              \
            WRITELONG(COMMAND, UNIT);                       \
            CachePostDMA((APTR)(DATA),&len2,0);             \
            }while(0)

asm("romtag:                                \n"
    "       dc.w    "XSTR(RTC_MATCHWORD)"   \n"
    "       dc.l    romtag                  \n"
    "       dc.l    endcode                 \n"
    "       dc.b    "XSTR(RTF_AUTOINIT)"    \n"
    "       dc.b    "XSTR(DEVICE_VERSION)"  \n"
    "       dc.b    "XSTR(NT_DEVICE)"       \n"
    "       dc.b    "XSTR(DEVICE_PRIORITY)" \n"
    "       dc.l    _device_name            \n"
    "       dc.l    _device_id_string       \n"
    "       dc.l    _auto_init_tables       \n"
    "endcode:                               \n");

int __attribute__((no_reorder)) _start()
{
    return -1;
}

char device_name[] = DEVICE_NAME;
char device_id_string[] = DEVICE_ID_STRING;

uint8_t pifloppy_perform_io(struct pifloppy_unit *u, struct IORequest *io);
uint8_t pifloppy_rw(struct pifloppy_unit *u, struct IORequest *io);
void pifloppy_cause_change_int(struct pifloppy_unit *u);

#if 0
#define debug(...)
#define debug_z3660(...)
#else
#define debug KPrintF
#define debug_z3660 KPrintF
#endif

struct pifloppy_base *dev_base = NULL;
ULONG Z3660_REGS = 0;

static struct Library __attribute__((used)) *init_device(uint8_t *seg_list asm("a0"), struct Library *dev asm("d0"))
{
    struct Library* ExpansionBase;
    SysBase = *(struct ExecBase **)4L;

    dev_base = AllocMem(sizeof(struct pifloppy_base), MEMF_PUBLIC | MEMF_CLEAR);
    dev_base->pi_dev = (struct Device *)dev;
    LONG ok = 0;
    struct ConfigDev* cd = NULL;

    if ((ExpansionBase = (struct Library*)OpenLibrary((uint8_t*)"expansion.library", 0L))) {
        // Find Z3660
        if ((cd = (struct ConfigDev*)FindConfigDev(cd, 0x144B, 0x1))) {
            ok = 1;
            debug_z3660("Z3660_Floppy: Z3660 found.\n");
            Z3660_REGS = (ULONG)cd->cd_BoardAddr;

            for (int i = 0; i < NUM_FLOPPY_UNITS; i++) {
                uint32_t r = 0;
                dev_base->units[i].change_int.lh_Head = (struct Node *)&dev_base->units[i].change_int.lh_Tail;
                dev_base->units[i].change_int.lh_Tail = NULL;
                dev_base->units[i].change_int.lh_TailPred = (struct Node *)&dev_base->units[i].change_int.lh_Head;
                WRITELONG(PIFLP_CMD_DRVNUM, i);
                dev_base->units[i].regs_ptr = Z3660_REGS + PIFLOPPY_OFFSET;
                READLONG(PIFLP_CMD_DRVTYPE, r);
                dev_base->units[i].enabled = r;
                dev_base->units[i].present = r;
                dev_base->units[i].valid = r;
                dev_base->units[i].unit_num = i;

                if (dev_base->units[i].present) {
                    READLONG(PIFLP_CMD_CYLINDERS, dev_base->units[i].c);
                    READLONG(PIFLP_CMD_HEADS, dev_base->units[i].h);
                    READLONG(PIFLP_CMD_SECPERTRACK, dev_base->units[i].s);

                    // Read block size and total blocks
                    uint32_t block_size, total_blocks;
                    switch (i) {
                        case 0: READLONG(PIFLP_CMD_BLOCKSIZE0, block_size); break;
                        case 1: READLONG(PIFLP_CMD_BLOCKSIZE1, block_size); break;
                        case 2: READLONG(PIFLP_CMD_BLOCKSIZE2, block_size); break;
                        case 3: READLONG(PIFLP_CMD_BLOCKSIZE3, block_size); break;
                        case 4: READLONG(PIFLP_CMD_BLOCKSIZE4, block_size); break;
                        case 5: READLONG(PIFLP_CMD_BLOCKSIZE5, block_size); break;
                        case 6: READLONG(PIFLP_CMD_BLOCKSIZE6, block_size); break;
                        case 7: READLONG(PIFLP_CMD_BLOCKSIZE7, block_size); break;
                        default: block_size = 512; break;
                    }
                    switch (i) {
                        case 0: READLONG(PIFLP_CMD_BLOCKS0, total_blocks); break;
                        case 1: READLONG(PIFLP_CMD_BLOCKS1, total_blocks); break;
                        case 2: READLONG(PIFLP_CMD_BLOCKS2, total_blocks); break;
                        case 3: READLONG(PIFLP_CMD_BLOCKS3, total_blocks); break;
                        case 4: READLONG(PIFLP_CMD_BLOCKS4, total_blocks); break;
                        case 5: READLONG(PIFLP_CMD_BLOCKS5, total_blocks); break;
                        case 6: READLONG(PIFLP_CMD_BLOCKS6, total_blocks); break;
                        case 7: READLONG(PIFLP_CMD_BLOCKS7, total_blocks); break;
                        default: total_blocks = 0; break;
                    }
                    dev_base->units[i].block_size = block_size ? block_size : 512;
                    dev_base->units[i].total_blocks = total_blocks;

                    // Determine disk type from geometry
                    if (dev_base->units[i].s >= 22) {
                        dev_base->units[i].disk_type = FLOPPY_TYPE_HD;
                    } else {
                        dev_base->units[i].disk_type = FLOPPY_TYPE_DD;
                    }

                    // Check if ADF is loaded
                    uint32_t inserted;
                    READLONG(PIFLP_CMD_DISKINSERTED, inserted);
                    dev_base->units[i].disk_inserted = inserted ? 1 : 0;
                }
                dev_base->units[i].change_num++;
            }
        } else {
            debug_z3660("Z3660_Floppy: Z3660 not found!\n");
        }
        CloseLibrary(ExpansionBase);
    } else {
        debug_z3660("Z3660_Floppy: failed to open expansion.library!\n");
    }
    return (ok > 0) ? dev : 0;
}

static uint8_t* __attribute__((used)) expunge(struct Library *dev asm("a6"))
{
    return 0;
}

static void __attribute__((used)) open(struct Library *dev asm("a6"), struct IOExtTD *iotd asm("a1"), uint32_t num asm("d0"), uint32_t flags asm("d1"))
{
    int io_err = TDERR_BadUnitNum;
    int unit_num = num;

    if (iotd && unit_num < NUM_FLOPPY_UNITS) {
        if (dev_base->units[unit_num].enabled && dev_base->units[unit_num].present) {
            io_err = 0;
            iotd->iotd_Req.io_Unit = (struct Unit*)&dev_base->units[unit_num].unit;
            iotd->iotd_Req.io_Unit->unit_flags = UNITF_ACTIVE;
            iotd->iotd_Req.io_Unit->unit_OpenCnt = 1;
            debug("Z3660_Floppy: open unit %d OK\n", unit_num);
        } else {
            debug("Z3660_Floppy: open unit %d FAILED enabled=%d present=%d\n",
                  unit_num, dev_base->units[unit_num].enabled, dev_base->units[unit_num].present);
        }
    }

    iotd->iotd_Req.io_Error = io_err;
    ((struct Library *)dev_base->pi_dev)->lib_OpenCnt++;
}

static uint8_t* __attribute__((used)) close(struct Library *dev asm("a6"), struct IOExtTD *iotd asm("a1"))
{
    ((struct Library *)dev_base->pi_dev)->lib_OpenCnt--;
    return 0;
}

static void __attribute__((used)) begin_io(struct Library *dev asm("a6"), struct IORequest *io asm("a1"))
{
    if (dev_base == NULL || io == NULL)
        return;

    struct pifloppy_unit *u;
    u = (struct pifloppy_unit *)io->io_Unit;

    if (u == NULL) {
        debug("Z3660_Floppy: begin_io u=NULL cmd=%ld\n", io->io_Command);
        return;
    }

    debug("Z3660_Floppy: begin_io unit=%ld cmd=%ld\n", (ULONG)u->unit_num, (ULONG)io->io_Command);

    io->io_Error = pifloppy_perform_io(u, io);

    // TD_ADDCHANGEINT returns a special value to indicate "don't reply"
    // The IORequest is kept by the driver and will be replied on disk change
    if (io->io_Error != IOERR_NOCMD) {
        if (!(io->io_Flags & IOF_QUICK)) {
            ReplyMsg(&io->io_Message);
        }
    }
}

static uint32_t __attribute__((used)) abort_io(struct Library *dev asm("a6"), struct IORequest *io asm("a1"))
{
    if (!io) return IOERR_NOCMD;
    io->io_Error = IOERR_ABORTED;
    return IOERR_ABORTED;
}

uint32_t get_blocksize(uint8_t unit_num)
{
    uint32_t block_size;
    switch (unit_num) {
        case 0: READLONG(PIFLP_CMD_BLOCKSIZE0, block_size); break;
        case 1: READLONG(PIFLP_CMD_BLOCKSIZE1, block_size); break;
        case 2: READLONG(PIFLP_CMD_BLOCKSIZE2, block_size); break;
        case 3: READLONG(PIFLP_CMD_BLOCKSIZE3, block_size); break;
        case 4: READLONG(PIFLP_CMD_BLOCKSIZE4, block_size); break;
        case 5: READLONG(PIFLP_CMD_BLOCKSIZE5, block_size); break;
        case 6: READLONG(PIFLP_CMD_BLOCKSIZE6, block_size); break;
        case 7: READLONG(PIFLP_CMD_BLOCKSIZE7, block_size); break;
        default: block_size = 512; break;
    }
    return block_size;
}

uint32_t get_blocks(uint8_t unit_num)
{
    uint32_t blocks;
    switch (unit_num) {
        case 0: READLONG(PIFLP_CMD_BLOCKS0, blocks); break;
        case 1: READLONG(PIFLP_CMD_BLOCKS1, blocks); break;
        case 2: READLONG(PIFLP_CMD_BLOCKS2, blocks); break;
        case 3: READLONG(PIFLP_CMD_BLOCKS3, blocks); break;
        case 4: READLONG(PIFLP_CMD_BLOCKS4, blocks); break;
        case 5: READLONG(PIFLP_CMD_BLOCKS5, blocks); break;
        case 6: READLONG(PIFLP_CMD_BLOCKS6, blocks); break;
        case 7: READLONG(PIFLP_CMD_BLOCKS7, blocks); break;
        default: blocks = 0; break;
    }
    return blocks;
}

/*
 * Read/write sectors from/to ADF image via Z3660 FPGA
 * This is the core function that transfers data between Amiga memory
 * and the ADF image file on the ARM side
 */
uint8_t pifloppy_rw(struct pifloppy_unit *u, struct IORequest *io)
{
    struct IOStdReq *iostd = (struct IOStdReq *)io;
    struct IOExtTD *iotd = (struct IOExtTD *)io;

    uint8_t *data;
    uint32_t len;
    uint8_t unit_num;
    ULONG io_Offset;
    ULONG io_Actual;
    uint8_t sderr = 0;
    uint32_t block_size = 512;

    unit_num = u->unit_num;
    WRITELONG(PIFLP_CMD_DRVNUMX, u->unit_num);
    block_size = get_blocksize(u->unit_num);

    data = iotd->iotd_Req.io_Data;
    if (data == 0) {
        return IOERR_BADADDRESS;
    }
    len = iotd->iotd_Req.io_Length;
    if (len < block_size) {
        iostd->io_Actual = 0;
        return IOERR_BADLENGTH;
    }
    io_Offset = iostd->io_Offset;
    io_Actual = iostd->io_Actual;

    switch (io->io_Command) {
        case TD_WRITE64:
        case NSCMD_TD_WRITE64:
        case TD_FORMAT64:
        case NSCMD_TD_FORMAT64:
            WRITELONG(PIFLP_CMD_WRITE_ADDR1, io_Offset);
            WRITELONG(PIFLP_CMD_WRITE_ADDR2, len);
            WRITELONG(PIFLP_CMD_WRITE_ADDR3, (uint32_t)data);
            if ((ULONG)data < 0x08000000)
                memcpy((uint8_t *)(Z3660_REGS + Z3660_DMA_BUFFER), data, len);
            WRITELONG(PIFLP_CMD_WRITE_ADDR4, io_Actual);
            WRITE_CMD(PIFLP_CMD_WRITE64, unit_num, data, len);
            break;

        case TD_READ64:
        case NSCMD_TD_READ64:
            WRITELONG(PIFLP_CMD_READ_ADDR1, io_Offset);
            WRITELONG(PIFLP_CMD_READ_ADDR2, len);
            WRITELONG(PIFLP_CMD_READ_ADDR3, (uint32_t)data);
            WRITELONG(PIFLP_CMD_READ_ADDR4, io_Actual);
            WRITE_CMD(PIFLP_CMD_READ64, unit_num, data, len);
            {
                ULONG dma;
                READLONG(PIFLP_CMD_USED_DMA, dma);
                if (dma != 0)
                    memcpy((uint8_t *)data, (uint8_t *)(Z3660_REGS + Z3660_DMA_BUFFER), len);
            }
            break;

        case TD_FORMAT:
        case CMD_WRITE:
            WRITELONG(PIFLP_CMD_WRITE_ADDR1, io_Offset);
            WRITELONG(PIFLP_CMD_WRITE_ADDR2, len);
            WRITELONG(PIFLP_CMD_WRITE_ADDR3, (uint32_t)data);
            if ((ULONG)data < 0x08000000)
                memcpy((uint8_t *)(Z3660_REGS + Z3660_DMA_BUFFER), data, len);
            WRITE_CMD(PIFLP_CMD_WRITE, unit_num, data, len);
            break;

        case CMD_READ:
            WRITELONG(PIFLP_CMD_READ_ADDR1, io_Offset);
            WRITELONG(PIFLP_CMD_READ_ADDR2, len);
            WRITELONG(PIFLP_CMD_READ_ADDR3, (uint32_t)data);
            WRITE_CMD(PIFLP_CMD_READ, unit_num, data, len);
            {
                ULONG dma;
                READLONG(PIFLP_CMD_USED_DMA, dma);
                if (dma != 0)
                    memcpy((uint8_t *)data, (uint8_t *)(Z3660_REGS + Z3660_DMA_BUFFER), len);
            }
            break;
    }

    if (sderr) {
        iostd->io_Actual = 0;
        if (sderr & FLOPPY_ERR_TIMEOUT)
            return TDERR_DiskChanged;
        if (sderr & FLOPPY_ERR_PARAM)
            return TDERR_SeekError;
        if (sderr & FLOPPY_ERR_ADDRESS)
            return TDERR_SeekError;
        if (sderr & FLOPPY_ERR_CRC)
            return TDERR_BadSecSum;
        if (sderr & FLOPPY_ERR_ILLEGAL)
            return TDERR_TooFewSecs;
        return TDERR_SeekError;
    } else {
        iostd->io_Actual = iotd->iotd_Req.io_Length;
    }

    return 0;
}

#define PIFLOPPY_ID_STRING "Z3660    Floppy  DF0   1.1 1111111111111111"

uint16_t ns_support[] = {
    NSCMD_DEVICEQUERY,
    CMD_RESET,
    CMD_READ,
    CMD_WRITE,
    CMD_UPDATE,
    CMD_CLEAR,
    CMD_START,
    CMD_STOP,
    CMD_FLUSH,
    TD_MOTOR,
    TD_SEEK,
    TD_FORMAT,
    TD_REMOVE,
    TD_CHANGENUM,
    TD_CHANGESTATE,
    TD_PROTSTATUS,
    TD_GETDRIVETYPE,
    TD_ADDCHANGEINT,
    TD_REMCHANGEINT,
    TD_GETGEOMETRY,
    TD_EJECT,
    NSCMD_TD_READ64,
    NSCMD_TD_WRITE64,
    NSCMD_TD_SEEK64,
    NSCMD_TD_FORMAT64,
    TAG_END,
};

#define DUMMYCMD iostd->io_Actual = 0; break;

uint8_t pifloppy_perform_io(struct pifloppy_unit *u, struct IORequest *io)
{
    struct IOStdReq *iostd = (struct IOStdReq *)io;
    struct IOExtTD *iotd = (struct IOExtTD *)io;
    uint8_t err = 0;

    if (!u->enabled) {
        return IOERR_OPENFAIL;
    }

    if (io->io_Error == IOERR_ABORTED) {
        return io->io_Error;
    }

    switch (io->io_Command) {
        case NSCMD_DEVICEQUERY: {
            struct NSDeviceQueryResult *res = (struct NSDeviceQueryResult *)iotd->iotd_Req.io_Data;
            res->DevQueryFormat = 0;
            res->SizeAvailable = 16;
            res->DeviceType = NSDEVTYPE_TRACKDISK;
            res->DeviceSubType = 0;
            res->SupportedCommands = ns_support;
            iostd->io_Actual = 16;
            return 0;
        }

        case CMD_CLEAR:
            DUMMYCMD;
        case CMD_UPDATE:
            DUMMYCMD;
        case TD_PROTSTATUS:
            iostd->io_Actual = u->read_only ? 1 : 0;
            break;
        case TD_CHANGENUM:
            iostd->io_Actual = u->change_num;
            break;
        case TD_REMOVE:
            DUMMYCMD;
        case TD_CHANGESTATE: {
            // Check if disk is inserted via FPGA register
            uint32_t inserted;
            WRITELONG(PIFLP_CMD_DRVNUMX, u->unit_num);
            READLONG(PIFLP_CMD_DISKINSERTED, inserted);
            iostd->io_Actual = inserted ? 0 : 1;  // 0=disk in, 1=no disk
            break;
        }
        case TD_GETDRIVETYPE:
            // Return drive type - all floppy drives are direct access
            iostd->io_Actual = DG_DIRECT_ACCESS;
            break;

        case TD_MOTOR:
            iostd->io_Actual = u->motor;
            u->motor = iostd->io_Length ? 1 : 0;
            // Motor on/off via FPGA register
            WRITELONG(PIFLP_CMD_DRVNUMX, u->unit_num);
            WRITELONG(PIFLP_CMD_MOTOR, u->motor);
            break;

        case TD_SEEK:
            // Seek to track - for ADF this is just updating position
            WRITELONG(PIFLP_CMD_DRVNUMX, u->unit_num);
            WRITELONG(PIFLP_CMD_TRACK, iostd->io_Offset);
            break;

        case TD_GETGEOMETRY: {
            struct DriveGeometry *res = (struct DriveGeometry *)iostd->io_Data;
            WRITELONG(PIFLP_CMD_DRVNUMX, u->unit_num);
            res->dg_SectorSize = get_blocksize(u->unit_num);
            res->dg_TotalSectors = get_blocks(u->unit_num);
            res->dg_Cylinders = u->c;
            res->dg_CylSectors = u->s * u->h;
            res->dg_Heads = u->h;
            res->dg_TrackSectors = u->s;
            res->dg_BufMemType = MEMF_PUBLIC;
            res->dg_DeviceType = DG_DIRECT_ACCESS;
            res->dg_Flags = DGF_REMOVABLE;
            return 0;
        }

        case TD_EJECT:
            debug("Z3660_Floppy: TD_EJECT unit %d\n", u->unit_num);
            WRITELONG(PIFLP_CMD_DRVNUMX, u->unit_num);
            WRITELONG(PIFLP_CMD_EJECT, u->unit_num);
            u->change_num++;
            u->disk_inserted = 0;
            u->read_only = 1;
            pifloppy_cause_change_int(u);
            break;

        case TD_ADDCHANGEINT:
            Forbid();
            Enqueue(&u->change_int, &((struct IOExtTD *)io)->iotd_Req.io_Message.mn_Node);
            Permit();
            return IOERR_NOCMD;  // Special: begin_io will not reply this

        case TD_REMCHANGEINT:
            Forbid();
            Remove(&((struct IOExtTD *)io)->iotd_Req.io_Message.mn_Node);
            Permit();
            break;

        case TD_FORMAT:
        case TD_FORMAT64:
        case NSCMD_TD_FORMAT64:
        case TD_READ64:
        case NSCMD_TD_READ64:
        case TD_WRITE64:
        case NSCMD_TD_WRITE64:
        case CMD_WRITE:
        case CMD_READ:
            err = pifloppy_rw(u, io);
            break;

        default:
            debug("Z3660_Floppy: unknkown cmd %d\n", io->io_Command);
            err = IOERR_NOCMD;
            break;
    }

    return err;
}
#undef DUMMYCMD

/*
 * Cause all registered change interrupts for this unit
 * Called when a disk change is detected (eject, insert)
 */
void pifloppy_cause_change_int(struct pifloppy_unit *u)
{
    if (IsListEmpty(&u->change_int))
        return;

    struct Node *node;
    struct List tempList;
    tempList.lh_Head = (struct Node *)&tempList.lh_Tail;
    tempList.lh_Tail = NULL;
    tempList.lh_TailPred = (struct Node *)&tempList.lh_Head;

    // Move all entries to temp list to avoid race conditions
    Forbid();
    while ((node = RemHead(&u->change_int)) != NULL) {
        AddTail(&tempList, node);
    }
    Permit();

    // Reply each IORequest to signal disk change
    // The mn_Node is inside io_Message which is inside IORequest
    while ((node = RemHead(&tempList)) != NULL) {
        struct Message *msg = (struct Message *)((char *)node - offsetof(struct Message, mn_Node));
        struct IORequest *io = (struct IORequest *)((char *)msg - offsetof(struct IORequest, io_Message));
        io->io_Error = 0;
        ReplyMsg(&io->io_Message);
    }
}

static uint32_t device_vectors[] = {
    (uint32_t)open,
    (uint32_t)close,
    (uint32_t)expunge,
    0,  // extFunc not used
    (uint32_t)begin_io,
    (uint32_t)abort_io,
    (uint32_t)-1
};

const uint32_t auto_init_tables[4] = {
    sizeof(struct Library),
    (uint32_t)device_vectors,
    0,
    (uint32_t)init_device
};