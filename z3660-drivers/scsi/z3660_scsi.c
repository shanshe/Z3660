// SPDX-License-Identifier: MIT

#include <exec/resident.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/alerts.h>
#include <exec/tasks.h>
#include <exec/io.h>
#include <exec/execbase.h>

#include <libraries/expansion.h>
#include <libraries/expansionbase.h>

#include <devices/trackdisk.h>
#include <devices/timer.h>
#include <devices/scsidisk.h>

#include <dos/filehandler.h>

#include <proto/exec.h>
#include <proto/disk.h>
#include <proto/expansion.h>

#include "newstyle.h"

#include "z3660_scsi_enums.h"
#include <stdint.h>
#include <string.h>

#include "z3660_scsi.h"

#pragma pack(4)
struct piscsi_base {
    struct Device* pi_dev;
    struct piscsi_unit {
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
    } units[NUM_UNITS];
};

struct ExecBase *SysBase;

//#define WRITESHORT(cmd, val) *(unsigned short *)((unsigned long)(Z3660_REGS + PISCSI_OFFSET + cmd)) = val;
#define WRITELONG(cmd, val) *(volatile unsigned long *)((unsigned long)(Z3660_REGS + PISCSI_OFFSET + (cmd))) = (val);
//#define WRITEBYTE(cmd, val) *(unsigned char *)((unsigned long)(Z3660_REGS + PISCSI_OFFSET + cmd)) = val;

#define WRITE_CMD(COMMAND,UNIT,DATA,LEN)  do{               \
            uint32_t len2=LEN;                              \
/*            CacheClearE((APTR)DATA,len,CACRF_ClearD); */  \
            CachePreDMA((APTR)(DATA),&len2,0);              \
            WRITELONG(COMMAND, UNIT);                       \
            CachePostDMA((APTR)DATA,&len2,0);               \
            }while(0)

//#define READSHORT(cmd, var) var = *(volatile unsigned short *)(Z3660_REGS + PISCSI_OFFSET + cmd);
#define READLONG(cmd, var) var = *(volatile unsigned long *)(Z3660_REGS + PISCSI_OFFSET + cmd);

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

uint8_t piscsi_perform_io(struct piscsi_unit *u, struct IORequest *io);
uint8_t piscsi_rw(struct piscsi_unit *u, struct IORequest *io);
uint8_t piscsi_scsi(struct piscsi_unit *u, struct IORequest *io);

#define uint32_t unsigned long
#define uint16_t unsigned short
#define uint8_t unsigned char

#define debug(...)
#define debugval(...)
#define debug_z3660(...)
//#define debug(c, v) WRITELONG(c, v)
//#define debugval(c, v) WRITELONG(c, v)

struct piscsi_base *dev_base = NULL;
ULONG Z3660_REGS=0;
static struct Library __attribute__((used)) *init_device(uint8_t *seg_list asm("a0"), struct Library *dev asm("d0"))
{
    struct Library* ExpansionBase;
    SysBase = *(struct ExecBase **)4L;

    debug(PISCSI_DBG_MSG, DBG_INIT);

    dev_base = AllocMem(sizeof(struct piscsi_base), MEMF_PUBLIC | MEMF_CLEAR);
    dev_base->pi_dev = (struct Device *)dev;
    LONG ok = 0;
    struct ConfigDev* cd = NULL;

    if ((ExpansionBase = (struct Library*)OpenLibrary((uint8_t*)"expansion.library", 0L)) ) {
        // Find Z2 or Z3 model of Z3660
        if ((cd = (struct ConfigDev*)FindConfigDev(cd,0x144B,0x1)) ) {
            ok = 1;
            debug_z3660("Z3660_SCSI: Z3660 found.\n");
            Z3660_REGS = (ULONG)cd->cd_BoardAddr;
            struct BootNode *bn = (struct BootNode*)((struct ExpansionBase*)ExpansionBase)->MountList.lh_Head;
            while (bn->bn_Node.ln_Succ) {
		        struct DeviceNode *dn = bn->bn_DeviceNode;
		        const UBYTE *bname2 = BADDR(dn->dn_Name);
                WRITELONG(PISCSI_CMD_EXPBASE,(uint32_t)(bname2));
                bn = (struct BootNode*)bn->bn_Node.ln_Succ;
            }
            for (int i = 0; i < NUM_UNITS; i++) {
                uint32_t r = 0;
                if(i>=8 && i<10)
                    continue;
                WRITELONG(PISCSI_CMD_DRVNUM, (i));
                dev_base->units[i].regs_ptr = Z3660_REGS + PISCSI_OFFSET;
                READLONG(PISCSI_CMD_DRVTYPE, r);
                dev_base->units[i].enabled = r;
                dev_base->units[i].present = r;
                dev_base->units[i].valid = r;
                dev_base->units[i].unit_num = i;
                if (dev_base->units[i].present) {
                    READLONG(PISCSI_CMD_CYLS, dev_base->units[i].c);
                    READLONG(PISCSI_CMD_HEADS, dev_base->units[i].h);
                    READLONG(PISCSI_CMD_SECS, dev_base->units[i].s);

                    debugval(PISCSI_DBG_VAL1, dev_base->units[i].c);
                    debugval(PISCSI_DBG_VAL2, dev_base->units[i].h);
                    debugval(PISCSI_DBG_VAL3, dev_base->units[i].s);
                    debug(PISCSI_DBG_MSG, DBG_CHS);
                }
                dev_base->units[i].change_num++;
            }
        } else {
            debug_z3660("Z3660_SCSI: Z3660 not found!\n");
        }
        CloseLibrary(ExpansionBase);
    } else {
        debug_z3660("Z3660_SCSI: failed to open expansion.library!\n");
    }
    return (ok > 0) ? dev: 0;
}

static uint8_t* __attribute__((used)) expunge(struct Library *dev asm("a6"))
{
    debug(PISCSI_DBG_MSG, DBG_CLEANUP);
    /*if (dev_base->open_count)
        return 0;
    FreeMem(dev_base, sizeof(struct piscsi_base));*/
    return 0;
}

static void __attribute__((used)) open(struct Library *dev asm("a6"), struct IOExtTD *iotd asm("a1"), uint32_t num asm("d0"), uint32_t flags asm("d1"))
{
    //struct Node* node = (struct Node*)iotd;
    int io_err = TDERR_BadUnitNum;

    //WRITELONG(PISCSI_CMD_DEBUGME, 1);

    int unit_num = num;
    //WRITELONG(PISCSI_CMD_DRVNUM, num);
    //READLONG(PISCSI_CMD_DRVNUM, unit_num);

    debugval(PISCSI_DBG_VAL1, unit_num);
    debugval(PISCSI_DBG_VAL2, flags);
    debugval(PISCSI_DBG_VAL3, num);
    debug(PISCSI_DBG_MSG, DBG_OPENDEV);

    if (iotd && unit_num < NUM_UNITS && (unit_num<8 || unit_num>=10)) {
        if (dev_base->units[unit_num].enabled && dev_base->units[unit_num].present) {
            io_err = 0;
            iotd->iotd_Req.io_Unit = (struct Unit*)&dev_base->units[unit_num].unit;
            iotd->iotd_Req.io_Unit->unit_flags = UNITF_ACTIVE;
            iotd->iotd_Req.io_Unit->unit_OpenCnt = 1;
        }
    }

    iotd->iotd_Req.io_Error = io_err;
//    int counter=
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
    
    struct piscsi_unit *u;
    struct Node* node = (struct Node*)io;
    u = (struct piscsi_unit *)io->io_Unit;

    if (node == NULL || u == NULL)
        return;

    debugval(PISCSI_DBG_VAL1, io->io_Command);
    debugval(PISCSI_DBG_VAL2, io->io_Flags);
    debugval(PISCSI_DBG_VAL3, (io->io_Flags & IOF_QUICK));
    debug(PISCSI_DBG_MSG, DBG_BEGINIO);
    io->io_Error = piscsi_perform_io(u, io);

    if (!(io->io_Flags & IOF_QUICK)) {
        ReplyMsg(&io->io_Message);
    }
}

static uint32_t __attribute__((used)) abort_io(struct Library *dev asm("a6"), struct IORequest *io asm("a1"))
{
    debug(PISCSI_DBG_MSG, DBG_ABORTIO);
    if (!io) return IOERR_NOCMD;
    io->io_Error = IOERR_ABORTED;

    return IOERR_ABORTED;
}
uint32_t get_blocksize(uint8_t unit_num)
{
    uint32_t block_size;
    switch(unit_num){
        case 0:
            READLONG(PISCSI_CMD_BLOCKSIZE0, block_size);
            break;
        case 1:
            READLONG(PISCSI_CMD_BLOCKSIZE1, block_size);
            break;
        case 2:
            READLONG(PISCSI_CMD_BLOCKSIZE2, block_size);
            break;
        case 3:
            READLONG(PISCSI_CMD_BLOCKSIZE3, block_size);
            break;
        case 4:
            READLONG(PISCSI_CMD_BLOCKSIZE4, block_size);
            break;
        case 5:
            READLONG(PISCSI_CMD_BLOCKSIZE5, block_size);
            break;
        case 6:
            READLONG(PISCSI_CMD_BLOCKSIZE6, block_size);
            break;
        case 7:
            READLONG(PISCSI_CMD_BLOCKSIZE7, block_size);
            break;
        case 10:
            READLONG(PISCSI_CMD_BLOCKSIZE10, block_size);
            break;
        case 11:
            READLONG(PISCSI_CMD_BLOCKSIZE11, block_size);
            break;
        case 12:
            READLONG(PISCSI_CMD_BLOCKSIZE12, block_size);
            break;
        case 13:
            READLONG(PISCSI_CMD_BLOCKSIZE13, block_size);
            break;
        case 14:
            READLONG(PISCSI_CMD_BLOCKSIZE14, block_size);
            break;
        case 15:
            READLONG(PISCSI_CMD_BLOCKSIZE15, block_size);
            break;
        case 16:
            READLONG(PISCSI_CMD_BLOCKSIZE16, block_size);
            break;
        case 17:
            READLONG(PISCSI_CMD_BLOCKSIZE17, block_size);
            break;
        default:
            block_size=0;
    }
    return block_size;
}
uint32_t get_blocks(uint8_t unit_num)
{
    uint32_t blocks;
    switch(unit_num){
        case 0:
            READLONG(PISCSI_CMD_BLOCKS0, blocks);
            break;
        case 1:
            READLONG(PISCSI_CMD_BLOCKS1, blocks);
            break;
        case 2:
            READLONG(PISCSI_CMD_BLOCKS2, blocks);
            break;
        case 3:
            READLONG(PISCSI_CMD_BLOCKS3, blocks);
            break;
        case 4:
            READLONG(PISCSI_CMD_BLOCKS4, blocks);
            break;
        case 5:
            READLONG(PISCSI_CMD_BLOCKS5, blocks);
            break;
        case 6:
            READLONG(PISCSI_CMD_BLOCKS6, blocks);
            break;
        case 7:
            READLONG(PISCSI_CMD_BLOCKS7, blocks);
            break;
        case 10:
            READLONG(PISCSI_CMD_BLOCKS10, blocks);
            break;
        case 11:
            READLONG(PISCSI_CMD_BLOCKS11, blocks);
            break;
        case 12:
            READLONG(PISCSI_CMD_BLOCKS12, blocks);
            break;
        case 13:
            READLONG(PISCSI_CMD_BLOCKS13, blocks);
            break;
        case 14:
            READLONG(PISCSI_CMD_BLOCKS14, blocks);
            break;
        case 15:
            READLONG(PISCSI_CMD_BLOCKS15, blocks);
            break;
        case 16:
            READLONG(PISCSI_CMD_BLOCKS16, blocks);
            break;
        case 17:
            READLONG(PISCSI_CMD_BLOCKS17, blocks);
            break;
        default:
            blocks=0;
    }
    return blocks;
}

//static unsigned char last_unit_num=-1;
uint8_t piscsi_rw(struct piscsi_unit *u, struct IORequest *io) {
    struct IOStdReq *iostd = (struct IOStdReq *)io;
    struct IOExtTD *iotd = (struct IOExtTD *)io;

    uint8_t* data;
    uint32_t len;
    uint8_t unit_num;
    ULONG io_Offset;
    ULONG io_Actual;
    //uint32_t block, num_blocks;
    uint8_t sderr = 0;
    uint32_t block_size = 512;

    unit_num = u->unit_num;
//    if(last_unit_num!=u->unit_num)
//    {
    WRITELONG(PISCSI_CMD_DRVNUMX, u->unit_num);
//    READLONG(PISCSI_CMD_BLOCKSIZE, block_size);
    block_size=get_blocksize(u->unit_num);
//        last_unit_num=u->unit_num;
//    }

    data = iotd->iotd_Req.io_Data;
    if (data == 0) {
        return IOERR_BADADDRESS;
    }
    len = iotd->iotd_Req.io_Length;
    if (len < block_size) {
        iostd->io_Actual = 0;
        return IOERR_BADLENGTH;
    }
    io_Offset=iostd->io_Offset;
    io_Actual=iostd->io_Actual;
    switch (io->io_Command) {
        case TD_WRITE64:
        case NSCMD_TD_WRITE64:
        case TD_FORMAT64:
        case NSCMD_TD_FORMAT64: {
    WRITELONG(PISCSI_CMD_WRITE_ADDR1, io_Offset);
    WRITELONG(PISCSI_CMD_WRITE_ADDR2, len);
    WRITELONG(PISCSI_CMD_WRITE_ADDR3, (uint32_t)data);
            if((ULONG)data<0x08000000)
                memcpy((uint8_t *)(Z3660_REGS + 0x80000), data, len);
            WRITELONG(PISCSI_CMD_WRITE_ADDR4, io_Actual);
            WRITE_CMD(PISCSI_CMD_WRITE64,unit_num,data,len);
            break;
        }
        case TD_READ64:
        case NSCMD_TD_READ64: {
    WRITELONG(PISCSI_CMD_READ_ADDR1, io_Offset);
    WRITELONG(PISCSI_CMD_READ_ADDR2, len);
    WRITELONG(PISCSI_CMD_READ_ADDR3, (uint32_t)data);
            WRITELONG(PISCSI_CMD_READ_ADDR4, io_Actual);
            WRITE_CMD(PISCSI_CMD_READ64,unit_num,data,len);
            ULONG dma;
            READLONG(PISCSI_CMD_USED_DMA,dma);
            if(dma!=0)
                memcpy((uint8_t *)data,(uint8_t *)(Z3660_REGS + 0x80000), len);
            break;
        }
        case TD_FORMAT:
        case CMD_WRITE: {
    WRITELONG(PISCSI_CMD_WRITE_ADDR1, io_Offset);
    WRITELONG(PISCSI_CMD_WRITE_ADDR2, len);
    WRITELONG(PISCSI_CMD_WRITE_ADDR3, (uint32_t)data);
            if((ULONG)data<0x08000000)
                memcpy((uint8_t *)(Z3660_REGS + 0x80000), data, len);
            WRITE_CMD(PISCSI_CMD_WRITEBYTES,unit_num,data,len);
            break;
        }
        case CMD_READ: {
    WRITELONG(PISCSI_CMD_READ_ADDR1, io_Offset);
    WRITELONG(PISCSI_CMD_READ_ADDR2, len);
    WRITELONG(PISCSI_CMD_READ_ADDR3, (uint32_t)data);
            WRITE_CMD(PISCSI_CMD_READBYTES,unit_num,data,len);
            ULONG dma;
            READLONG(PISCSI_CMD_USED_DMA,dma);
            if(dma!=0)
                memcpy((uint8_t *)data,(uint8_t *)(Z3660_REGS + 0x80000), len);
            break;
        }
    }

    if (sderr) {
        iostd->io_Actual = 0;

        if (sderr & SCSIERR_TIMEOUT)
            return TDERR_DiskChanged;
        if (sderr & SCSIERR_PARAM)
            return TDERR_SeekError;
        if (sderr & SCSIERR_ADDRESS)
            return TDERR_SeekError;
        if (sderr & (SCSIERR_ERASESEQ | SCSIERR_ERASERES))
            return TDERR_BadSecPreamble;
        if (sderr & SCSIERR_CRC)
            return TDERR_BadSecSum;
        if (sderr & SCSIERR_ILLEGAL)
            return TDERR_TooFewSecs;
        if (sderr & SCSIERR_IDLE)
            return TDERR_PostReset;

        return TDERR_SeekError;
    } else {
        iostd->io_Actual = iotd->iotd_Req.io_Length;
    }

    return 0;
}

#define PISCSI_ID_STRING "Z3660    SCSI DiskX     0.1 1111111111111111"
// that X in position 18 is reserved for disknumber...

uint8_t piscsi_scsi(struct piscsi_unit *u, struct IORequest *io)
{
    struct IOStdReq *iostd = (struct IOStdReq *)io;
    struct SCSICmd *scsi = iostd->io_Data;
    //uint8_t* registers = sdu->sdu_Registers;
    uint8_t *data = (uint8_t *)scsi->scsi_Data;
    uint32_t i, block = 0, blocks = 0, maxblocks = 0;
    uint8_t err = 0;
    uint8_t write = 0;
    uint32_t block_size;

//    WRITELONG(PISCSI_CMD_DRVNUMX, u->unit_num);
//    READLONG(PISCSI_CMD_BLOCKSIZE, block_size);
    block_size=get_blocksize(u->unit_num);

    debugval(PISCSI_DBG_VAL1, iostd->io_Length);
    debugval(PISCSI_DBG_VAL2, scsi->scsi_Command[0]);
    debugval(PISCSI_DBG_VAL3, scsi->scsi_Command[1]);
    debugval(PISCSI_DBG_VAL4, scsi->scsi_Command[2]);
    debugval(PISCSI_DBG_VAL5, scsi->scsi_CmdLength);
    debug(PISCSI_DBG_MSG, DBG_SCSICMD);

    //maxblocks = u->s * u->c * u->h;

    if (scsi->scsi_CmdLength < 6) {
        return IOERR_BADLENGTH;
    }

    if (scsi->scsi_Command == NULL) {
        return IOERR_BADADDRESS;
    }

    scsi->scsi_Actual = 0;
    //iostd->io_Actual = sizeof(*scsi);

    switch (scsi->scsi_Command[0]) {
        case SCSICMD_TEST_UNIT_READY:
            err = 0;
            break;
        
        case SCSICMD_INQUIRY:
            for (i = 0; i < scsi->scsi_Length; i++) {
                uint8_t val = 0;

                switch (i) {
                    case 0: // SCSI device type: direct-access device
                        val = (0 << 5) | 0;
                        break;
                    case 1: // RMB = 1 (removable medium)
                        val = (1 << 7);
//                        val = 0;
                        break;
                    case 2: // VERSION = 0
                        val = 0;
                        break;
                    case 3: // NORMACA=0, HISUP = 0, RESPONSE_DATA_FORMAT = 2
                        val = (0 << 5) | (0 << 4) | 2;
                        break;
                    case 4: // ADDITIONAL_LENGTH = 44 - 4
                        val = 44 - 4;
                        break;
                    default:
                        if (i >= 8 && i < 44)
                        {
                            if(i-8==18)
                            {
                                if(u->unit_num<10)
                                    val= '0'+u->unit_num;
                                else
                                    val= 'A'+u->unit_num-10;
                            }
                            else
                                val = PISCSI_ID_STRING[i - 8];
                        }
                        else
                            val = 0;
                        break;
                }
                data[i] = val;
            }
//            scsi->scsi_Actual = scsi->scsi_Length;
            scsi->scsi_Actual = i;
            err = 0;
            break;
        
        case SCSICMD_WRITE_6:
            write = 1;
        case SCSICMD_READ_6:
            //block = *(uint32_t *)(&scsi->scsi_Command[0]) & 0x001FFFFF;
            block = scsi->scsi_Command[1] & 0x1f;
            block = (block << 8) | scsi->scsi_Command[2];
            block = (block << 8) | scsi->scsi_Command[3];
            blocks = scsi->scsi_Command[4];
            debugval(PISCSI_DBG_VAL1, (uint32_t)scsi->scsi_Command);
            debug(PISCSI_DBG_MSG, DBG_SCSICMD_RW6);
            goto scsireadwrite;
        case SCSICMD_WRITE_10:
            write = 1;
        case SCSICMD_READ_10:
            debugval(PISCSI_DBG_VAL1, (uint32_t)scsi->scsi_Command);
            debug(PISCSI_DBG_MSG, DBG_SCSICMD_RW10);
            //block = *(uint32_t *)(&scsi->scsi_Command[2]);
            block = scsi->scsi_Command[2];
            block = (block << 8) | scsi->scsi_Command[3];
            block = (block << 8) | scsi->scsi_Command[4];
            block = (block << 8) | scsi->scsi_Command[5];

            //blocks = *(uint16_t *)(&scsi->scsi_Command[7]);
            blocks = scsi->scsi_Command[7];
            blocks = (blocks << 8) | scsi->scsi_Command[8];

scsireadwrite:;
//            WRITELONG(PISCSI_CMD_DRVNUMX, u->unit_num);
//            READLONG(PISCSI_CMD_BLOCKS, maxblocks);
//            uint8_t unit_num;
//            do{
                maxblocks=get_blocks(u->unit_num);
//                READLONG(PISCSI_CMD_DRVNUM,unit_num);
//            }while(unit_num!=u->unit_num);
            if (block > maxblocks || (block + blocks) > maxblocks || blocks == 0) {
                err = IOERR_BADADDRESS;
                break;
            }
            if (data == NULL) {
                err = IOERR_BADADDRESS;
                break;
            }
            uint32_t len=blocks << 9;
/*            if (scsi->scsi_Length < len) {
                err = IOERR_BADLENGTH;
                break;
            }
*/
            if (write == 0) {
                WRITELONG(PISCSI_CMD_READ_ADDR1, block);
                WRITELONG(PISCSI_CMD_READ_ADDR2, len);
                WRITELONG(PISCSI_CMD_READ_ADDR3, (uint32_t)data);
                WRITE_CMD(PISCSI_CMD_READ,u->unit_num,data,len);
                ULONG dma;
                READLONG(PISCSI_CMD_USED_DMA,dma);
                if(dma!=0)
                    memcpy((uint8_t *)data,(uint8_t *)(Z3660_REGS + 0x80000), len);
            }
            else {
                if((ULONG)data<0x08000000)
                    memcpy((uint8_t *)(Z3660_REGS + 0x80000), data, len);
                WRITELONG(PISCSI_CMD_WRITE_ADDR1, block);
                WRITELONG(PISCSI_CMD_WRITE_ADDR2, len);
                WRITELONG(PISCSI_CMD_WRITE_ADDR3, (uint32_t)data);
                WRITE_CMD(PISCSI_CMD_WRITE,u->unit_num,data,len);
            }

            scsi->scsi_Actual = scsi->scsi_Length;
            err = 0;
            break;
        
        case SCSICMD_READ_CAPACITY_10:
            if (scsi->scsi_CmdLength < 10) {
                err = HFERR_BadStatus;
                break;
            }
/*            if (scsi->scsi_Command[2] != 0 || scsi->scsi_Command[3] != 0 || scsi->scsi_Command[4] != 0 || scsi->scsi_Command[5] != 0 || (scsi->scsi_Command[8] & 1))
            {
                err = HFERR_BadStatus;
                break;
            }
*/
            if (scsi->scsi_Length < 8) {
                err = IOERR_BADLENGTH;
                break;
            }
            
//            WRITELONG(PISCSI_CMD_DRVNUM, (u->unit_num));
//            READLONG(PISCSI_CMD_BLOCKS, blocks);
            blocks=get_blocks(u->unit_num);
            ((uint32_t*)data)[0] = blocks - 1;
            ((uint32_t*)data)[1] = block_size;

            scsi->scsi_Actual = 8;
            err = 0;

            break;
        case SCSICMD_MODE_SENSE_6:
            data[0] = 3 + 8 + 0x16;
            data[1] = 0; // MEDIUM TYPE
            data[2] = 0;
            data[3] = 8;

            debugval(PISCSI_DBG_VAL1, ((uint32_t)scsi->scsi_Command));
            debug(PISCSI_DBG_MSG, DBG_SCSI_DEBUG_MODESENSE_6);

//            WRITELONG(PISCSI_CMD_DRVNUM, (u->unit_num));
//            READLONG(PISCSI_CMD_BLOCKS, maxblocks);
            maxblocks=get_blocks(u->unit_num);
            blocks = (maxblocks - 1) & 0xFFFFFF;
//            if (maxblocks > (1 << 24))
//                blocks = 0xffffff;
//            else
//                blocks = maxblocks;

            *((uint32_t *)&data[4]) = blocks;
            *((uint32_t *)&data[8]) = block_size;

            switch (((UWORD)scsi->scsi_Command[2] << 8) | scsi->scsi_Command[3]) {
                case 0x0300: { // Format Device Mode
                    debug(PISCSI_DBG_MSG, DBG_SCSI_FORMATDEVICE);
                    uint8_t *datext = data + 12;
                    datext[0] = 0x03; // page code
                    datext[1] = 0x16; // page length
                    datext[2] = 0x00;
                    datext[3] = 0x01; // tracks per zone (heads)
//                    *((uint16_t *)&datext[2]) = u->h;// tracks per zone (heads)
                    *((uint32_t *)&datext[4]) = 0;
                    *((uint32_t *)&datext[8]) = 0;
                    *((uint16_t *)&datext[10]) = u->s; // sectors per track
                    *((uint16_t *)&datext[12]) = block_size; // data bytes per physical sector
                    datext[14] = 0x00;
                    datext[15] = 0x01;
//                    datext[15] = 0x00;
                    *((uint32_t *)&datext[16]) = 0;
                    datext[20] = 0x80;
//                    datext[20] = (1 << 6) | (1 << 5);

                    scsi->scsi_Actual = data[0] + 1;
                    err = 0;
                    break;
                }
                case 0x0400: // Rigid Drive Geometry
                    debug(PISCSI_DBG_MSG, DBG_SCSI_RDG);
                    uint8_t *datext = data + 12;
                    datext[0] = 0x04; // page code
                    *((uint32_t *)&datext[1]) = u->c; // cylinders 3 lower bytes
                    datext[1] = 0x16; // page length
                    datext[5] = u->h; // heads
                    datext[6] = 0x00;
                    *((uint32_t *)&datext[6]) = 0;
                    *((uint32_t *)&datext[10]) = 0;
                    *((uint32_t *)&datext[13]) = u->c;
                    datext[17] = 0;
                    *((uint32_t *)&datext[18]) = 0;
                    *((uint16_t *)&datext[20]) = 5400;

                    scsi->scsi_Actual = data[0] + 1;
                    err = 0;
                    break;
                
                default:
                    debugval(PISCSI_DBG_VAL1, (((UWORD)scsi->scsi_Command[2] << 8) | scsi->scsi_Command[3]));
                    debug(PISCSI_DBG_MSG, DBG_SCSI_UNKNOWN_MODESENSE);
                    err = HFERR_BadStatus;
                    break;
            }
            break;
        
        case SCSICMD_READ_DEFECT_DATA_10:
            break;
        case SCSICMD_CHANGE_DEFINITION:
            break;

        default:
            debugval(PISCSI_DBG_VAL1, scsi->scsi_Command[0]);
            debug(PISCSI_DBG_MSG, DBG_SCSI_UNKNOWN_COMMAND);
            err = HFERR_BadStatus;
            break;
    }

    if (err != 0) {
        debugval(PISCSI_DBG_VAL1, err);
        debug(PISCSI_DBG_MSG, DBG_SCSIERR);
        scsi->scsi_Actual = 0;
    }

    return err;
}

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
	TD_GETGEOMETRY,
	TD_ADDCHANGEINT,
	TD_REMCHANGEINT,
	HD_SCSICMD,
	NSCMD_TD_READ64,
	NSCMD_TD_WRITE64,
	NSCMD_TD_SEEK64,
	NSCMD_TD_FORMAT64,
	0,
};

#define DUMMYCMD iostd->io_Actual = 0; break;

uint8_t piscsi_perform_io(struct piscsi_unit *u, struct IORequest *io) {
    struct IOStdReq *iostd = (struct IOStdReq *)io;
    struct IOExtTD *iotd = (struct IOExtTD *)io;

    //uint8_t *data;
    //uint32_t len;
    //uint32_t offset;
    uint8_t err = 0;

    if (!u->enabled) {
        return IOERR_OPENFAIL;
    }

    //data = iotd->iotd_Req.io_Data;
    //len = iotd->iotd_Req.io_Length;

    if (io->io_Error == IOERR_ABORTED) {
        return io->io_Error;
    }

    debugval(PISCSI_DBG_VAL1, io->io_Command);
    debugval(PISCSI_DBG_VAL2, io->io_Flags);
    debugval(PISCSI_DBG_VAL3, iostd->io_Length);
    debug(PISCSI_DBG_MSG, DBG_IOCMD);

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
            break;
        }
        case CMD_CLEAR:
            /* Invalidate read buffer */
            DUMMYCMD;
        case CMD_UPDATE:
            /* Flush write buffer */
            DUMMYCMD;
        case TD_PROTSTATUS:
            DUMMYCMD;
        case TD_CHANGENUM:
            iostd->io_Actual = u->change_num;
            break;
        case TD_REMOVE:
            DUMMYCMD;
        case TD_CHANGESTATE:
            DUMMYCMD;
        case TD_GETDRIVETYPE:
            iostd->io_Actual = DG_DIRECT_ACCESS;
            break;
        case TD_MOTOR:
            iostd->io_Actual = u->motor;
            u->motor = iostd->io_Length ? 1 : 0;
            break;
        case TD_GETGEOMETRY: {
            struct DriveGeometry *res = (struct DriveGeometry *)iostd->io_Data;
            WRITELONG(PISCSI_CMD_DRVNUMX, u->unit_num);
//            READLONG(PISCSI_CMD_BLOCKSIZE, res->dg_SectorSize);
            res->dg_SectorSize=get_blocksize(u->unit_num);
//            READLONG(PISCSI_CMD_BLOCKS, res->dg_TotalSectors);
            res->dg_TotalSectors=get_blocks(u->unit_num);
            res->dg_Cylinders = u->c;
            res->dg_CylSectors = u->s * u->h;
            res->dg_Heads = u->h;
            res->dg_TrackSectors = u->s;
            res->dg_BufMemType = MEMF_PUBLIC;
            res->dg_DeviceType = 0;
            res->dg_Flags = 0;

            return 0;
            break;
        }

        case TD_FORMAT:
        case TD_FORMAT64:
        case NSCMD_TD_FORMAT64:
        case TD_READ64:
        case NSCMD_TD_READ64:
        case TD_WRITE64:
        case NSCMD_TD_WRITE64:
        case CMD_WRITE:
        case CMD_READ:
            err = piscsi_rw(u, io);
            break;
        case HD_SCSICMD:
            //err = 0;
            err = piscsi_scsi(u, io);
            break;
        default: {
            //int cmd = io->io_Command;
            debug(PISCSI_DBG_MSG, DBG_IOCMD_UNHANDLED);
            err = IOERR_NOCMD;
            break;
        }
    }

    return err;
}
#undef DUMMYCMD

static uint32_t device_vectors[] = {
    (uint32_t)open,
    (uint32_t)close,
    (uint32_t)expunge,
    0, //extFunc not used here
    (uint32_t)begin_io,
    (uint32_t)abort_io,
    -1
};

const uint32_t auto_init_tables[4] = {
    sizeof(struct Library),
    (uint32_t)device_vectors,
    0,
    (uint32_t)init_device
};
