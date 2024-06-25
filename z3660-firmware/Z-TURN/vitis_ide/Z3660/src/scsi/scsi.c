// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
//#include <unistd.h>
//#include <endian.h>
//#include "../mpg/ffconf.h"
//#include "../mpg/ff.h"
#include <ffconf.h>
#include <ff.h>
#include "../main.h"

#include <xparameters.h>
#include "../memorymap.h"
#include <xil_cache.h>

//#include "config_file/config_file.h"
//#include "gpio/ps_protocol.h"
#include "z3660_scsi_enums.h"
#include "scsi.h"
#include "../config_file.h"
#include "../console.h"
//#include "platforms/amiga/hunk-reloc.h"
extern CONFIG config;
#define BE(val) be32toh(val)
#define BE16(val) be16toh(val)

// Uncomment the line below to enable debug output
//#define PISCSI_DEBUG
uint32_t used_dma=0;

#ifdef PISCSI_DEBUG
#define read8(a) *(uint8_t*)(a)
#define DEBUG printf
//#define DEBUG_TRIVIAL printf
#define DEBUG_TRIVIAL(...)

//extern void stop_cpu_emulation(uint8_t disasm_cur);
#define stop_cpu_emulation(...)

static const char *op_type_names[4] = {
    "BYTE",
    "WORD",
    "LONGWORD",
    "MEM",
};
#else
#define DEBUG_TRIVIAL(...)
#define stop_cpu_emulation(...)
//#define NO_DEBUG_CONSOLE
#ifdef NO_DEBUG_CONSOLE
#define DEBUG(...)
#else
extern CONSOLE con;
void DEBUG(const char *format, ...)
{
	if(con.debug_scsi==0)
		return;
	va_list args;
	va_start(args, format);
	vprintf(format,args);
	va_end(args);
}
static const char *op_type_names[4] = {
    "BYTE",
    "WORD",
    "LONGWORD",
    "MEM",
};
#endif
#endif

#ifdef FAKESTORM
#define lseek64 lseek
#endif

struct emulator_config *cfg;

struct piscsi_dev devs[8];
struct piscsi_fs filesystems[NUM_FILESYSTEMS];

uint8_t piscsi_num_fs = 0;

uint8_t piscsi_cur_drive = 0;
uint32_t piscsi_u32[4];
uint32_t piscsi_dbg[8];
uint32_t piscsi_rom_size = 0;
uint8_t *piscsi_rom_ptr=NULL;

uint32_t rom_partitions[128];
uint32_t rom_partition_prio[128];
uint32_t rom_partition_dostype[128];
uint32_t rom_cur_partition = 0, rom_cur_fs = 0;

extern uint8_t ac_piscsi_rom[];

char partition_names[128][32];
unsigned int times_used[128];
unsigned int num_partition_names = 0;

struct hunk_info piscsi_hinfo;
struct hunk_reloc piscsi_hreloc[2048];
static FATFS fatfs;

int piscsi_init() {
	XGpioPs_WritePin(&GpioPs, LED1, 0); // ON

    for (int i = 0; i < 8; i++) {
        devs[i].fd = 0;
        devs[i].lba = 0;
        devs[i].c = devs[i].h = devs[i].s = 0;
        devs[i].SeekTbl[0]=0;
    }

	TCHAR *Path = DEFAULT_ROOT;
	f_mount(&fatfs, Path, 1); // 1 mount immediately

    if (piscsi_rom_ptr == NULL) {
        FIL in;
        int ret = f_open(&in,DEFAULT_ROOT "z3660_scsi.rom", FA_READ | FA_OPEN_EXISTING);
        if (ret != FR_OK) {
            printf("[PISCSI] Could not open PISCSI Boot ROM file for reading!\n");
            // Zero out the boot ROM offset from the autoconfig ROM.
//            ac_piscsi_rom[20] = 0;
//            ac_piscsi_rom[21] = 0;
//            ac_piscsi_rom[22] = 0;
//            ac_piscsi_rom[23] = 0;
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return(0); // Boot ROM disabled
        }
        piscsi_rom_size = f_size(&in);
        f_lseek(&in, 0);
        piscsi_rom_ptr =(uint8_t*) BOOT_ROM_ADDRESS;//malloc(piscsi_rom_size);
        unsigned int n_bytes;
        memset((uint8_t*) piscsi_rom_ptr,0,BOOT_ROM_SIZE);
        f_read(&in,piscsi_rom_ptr, piscsi_rom_size, &n_bytes);

        f_lseek(&in, PISCSI_DRIVER_OFFSET);
        process_hunks(&in, &piscsi_hinfo, piscsi_hreloc, PISCSI_DRIVER_OFFSET);
        f_close(&in);

        Xil_L1DCacheFlush();
        Xil_L2CacheFlush();
//        Xil_DCacheFlushRange((INTPTR)piscsi_rom_ptr,BOOT_ROM_SIZE);
//        Xil_DCacheInvalidateRange((INTPTR)piscsi_rom_ptr,BOOT_ROM_SIZE);

        printf("[PISCSI] Loaded SCSI Boot ROM.\n");
    } else {
        printf("[PISCSI] Boot ROM already loaded.\n");
    }
//    fflush(stdout);
	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
    return(1); // Boot ROM loaded
}

void piscsi_shutdown() {
    printf("[PISCSI] Shutting down PiSCSI...");
    for (int i = 0; i < 8; i++) {
        if (devs[i].fd != 0) {
//            FRESULT res=
            f_close(devs[i].fd);
//printf("\nresult %d %d",i,res);
            devs[i].fd = 0;
            devs[i].block_size = 0;
        }
    }

    for (int i = 0; i < NUM_FILESYSTEMS; i++) {
        if (filesystems[i].binary_data) {
            free(filesystems[i].binary_data);
            filesystems[i].binary_data = NULL;
        }
        if (filesystems[i].fhb) {
            free(filesystems[i].fhb);
            filesystems[i].fhb = NULL;
        }
        filesystems[i].h_info.current_hunk = 0;
        filesystems[i].h_info.reloc_hunks = 0;
        filesystems[i].FS_ID = 0;
        filesystems[i].handler = 0;
    }
    f_mount(NULL,DEFAULT_ROOT,1);
    printf("...Done\n");
}

void piscsi_find_partitions(struct piscsi_dev *d) {
	XGpioPs_WritePin(&GpioPs, LED1, 0); // ON
    FIL *fd = d->fd;
    int cur_partition = 0;
    uint8_t tmp;

    for (int i = 0; i < 16; i++) {
        if (d->pb[i]) {
            free(d->pb[i]);
            d->pb[i] = NULL;
        }
    }

    if (!d->rdb || d->rdb->rdb_PartitionList == 0) {
        DEBUG("[PISCSI] No partitions on disk.\n");
    	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
        return;
    }

    char *block = malloc(d->block_size);

    f_lseek(fd, ((uint64_t)BE(d->rdb->rdb_PartitionList)) * d->block_size);
next_partition:;
    unsigned int n_bytes;
    f_read(fd, block, d->block_size,&n_bytes);

    uint32_t first = be32toh(*((uint32_t *)&block[0]));
    if (first != PART_IDENTIFIER) {
        DEBUG("Entry at block %ld is not a valid partition. Aborting.\n", BE(d->rdb->rdb_PartitionList));
    	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
        return;
    }

    struct PartitionBlock *pb = (struct PartitionBlock *)block;
    tmp = pb->pb_DriveName[0];
    pb->pb_DriveName[tmp + 1] = 0x00;
    printf("[PISCSI] Partition %d: %s (%d)\n", cur_partition, pb->pb_DriveName + 1, pb->pb_DriveName[0]);
    DEBUG("Checksum: %.8lX HostID: %ld\n", BE(pb->pb_ChkSum), BE(pb->pb_HostID));
    DEBUG("Flags: %ld (%.8lX) Devflags: %ld (%.8lX)\n", BE(pb->pb_Flags), BE(pb->pb_Flags), BE(pb->pb_DevFlags), BE(pb->pb_DevFlags));
    d->pb[cur_partition] = pb;

    for (int i = 0; i < 128; i++) {
        if (strcmp((char *)pb->pb_DriveName + 1, partition_names[i]) == 0) {
            DEBUG("[PISCSI] Duplicate partition name %s. Temporarily renaming to %s_%d.\n", pb->pb_DriveName + 1, pb->pb_DriveName + 1, times_used[i] + 1);
            times_used[i]++;
            sprintf((char *)pb->pb_DriveName + 1 + pb->pb_DriveName[0], "_%d", times_used[i]);
            pb->pb_DriveName[0] += 2;
            if (times_used[i] > 9)
                pb->pb_DriveName[0]++;
            goto partition_renamed;
        }
    }
    sprintf(partition_names[num_partition_names], "%s", pb->pb_DriveName + 1);
    num_partition_names++;

partition_renamed:
    if (d->pb[cur_partition]->pb_Next != 0xFFFFFFFF) {
        uint64_t next = be32toh(pb->pb_Next);
        block = malloc(d->block_size);
//        lseek64(fd, next * d->block_size, SEEK_SET);
        f_lseek(fd, next * d->block_size);
        cur_partition++;
        DEBUG("[PISCSI] Next partition at block %ld.\n", be32toh(pb->pb_Next));
        goto next_partition;
    }
    DEBUG("[PISCSI] No more partitions on disk.\n");
    d->num_partitions = cur_partition + 1;
//    d->fshd_offs = lseek64(fd, 0, SEEK_CUR);
    d->fshd_offs = fd->fptr;
	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
    return;
}

int piscsi_parse_rdb(struct piscsi_dev *d) {
	XGpioPs_WritePin(&GpioPs, LED1, 0); // ON
    FIL *fd = d->fd;
    int i = 0;
    uint8_t *block = malloc(PISCSI_MAX_BLOCK_SIZE);

    f_lseek(fd, 0);
    for (i = 0; i < RDB_BLOCK_LIMIT; i++) {
    	unsigned int n_bytes;
        f_read(fd, block, PISCSI_MAX_BLOCK_SIZE,&n_bytes);
        uint32_t first = be32toh(*((uint32_t *)&block[0]));
        if (first == RDB_IDENTIFIER)
            goto rdb_found;
    }
    goto no_rdb_found;
rdb_found:;
    struct RigidDiskBlock *rdb = (struct RigidDiskBlock *)block;
    DEBUG("[PISCSI] RDB found at block %d.\n", i);
    d->c = be32toh(rdb->rdb_Cylinders);
    d->h = be32toh(rdb->rdb_Heads);
    d->s = be32toh(rdb->rdb_Sectors);
    d->num_partitions = 0;
    DEBUG("[PISCSI] RDB - first partition at block %ld.\n", be32toh(rdb->rdb_PartitionList));
    d->block_size = be32toh(rdb->rdb_BlockBytes);
    DEBUG("[PISCSI] Block size: %ld. (%ld)\n", be32toh(rdb->rdb_BlockBytes), d->block_size);
    if (d->rdb)
        free(d->rdb);
    d->rdb = rdb;
    sprintf(d->rdb->rdb_DriveInitName, "z3660_scsi.device");
	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
    return 0;

no_rdb_found:;
    if (block)
    {
    	free(block);
    }

	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
    return -1;
}

void piscsi_refresh_drives() {
    piscsi_num_fs = 0;

    for (int i = 0; i < NUM_FILESYSTEMS; i++) {
        if (filesystems[i].binary_data) {
            free(filesystems[i].binary_data);
            filesystems[i].binary_data = NULL;
        }
        if (filesystems[i].fhb) {
            free(filesystems[i].fhb);
            filesystems[i].fhb = NULL;
        }
        filesystems[i].h_info.current_hunk = 0;
        filesystems[i].h_info.reloc_hunks = 0;
        filesystems[i].FS_ID = 0;
        filesystems[i].handler = 0;
    }

    rom_cur_fs = 0;

    for (int i = 0; i < 128; i++) {
        memset(partition_names[i], 0x00, 32);
        times_used[i] = 0;
    }
    num_partition_names = 0;

    for (int i = 0; i < NUM_UNITS; i++) {
        if (devs[i].fd != 0) {
            piscsi_parse_rdb(&devs[i]);
            piscsi_find_partitions(&devs[i]);
            piscsi_find_filesystems(&devs[i]);
        }
    }
}

void piscsi_find_filesystems(struct piscsi_dev *d) {
    if (!d->num_partitions)
        return;

    uint8_t fs_found = 0;

    uint8_t *fhb_block = malloc(d->block_size);

//    lseek64(d->fd, d->fshd_offs, SEEK_SET);
    f_lseek(d->fd, d->fshd_offs);
    DEBUG("[FSHD] FSHD offset 0x%llX.\n",d->fshd_offs);
    struct FileSysHeaderBlock *fhb = (struct FileSysHeaderBlock *)fhb_block;
    unsigned int n_bytes;
    f_read(d->fd, fhb_block, d->block_size,&n_bytes);

    while ((BE(fhb->fhb_ID) == FS_IDENTIFIER) && (!f_eof(d->fd))) {
        char *dosID = (char *)&fhb->fhb_DosType;
#ifdef PISCSI_DEBUG
        uint16_t *fsVer = (uint16_t *)&fhb->fhb_Version;

        DEBUG("[FSHD] FSHD Block found.\n");
        DEBUG("[FSHD] HostID: %ld Next: %ld Size: %ld\n", BE(fhb->fhb_HostID), BE(fhb->fhb_Next), BE(fhb->fhb_SummedLongs));
        DEBUG("[FSHD] Flags: %.8lX DOSType: %c%c%c/%d\n", BE(fhb->fhb_Flags), dosID[0], dosID[1], dosID[2], dosID[3]);
        DEBUG("[FSHD] Version: %d.%d\n", BE16(fsVer[0]), BE16(fsVer[1]));
        DEBUG("[FSHD] Patchflags: %ld Type: %ld\n", BE(fhb->fhb_PatchFlags), BE(fhb->fhb_Type));
        DEBUG("[FSHD] Task: %ld Lock: %ld\n", BE(fhb->fhb_Task), BE(fhb->fhb_Lock));
        DEBUG("[FSHD] Handler: %ld StackSize: %ld\n", BE(fhb->fhb_Handler), BE(fhb->fhb_StackSize));
        DEBUG("[FSHD] Prio: %ld Startup: %ld (%.8lX)\n", BE(fhb->fhb_Priority), BE(fhb->fhb_Startup), BE(fhb->fhb_Startup));
        DEBUG("[FSHD] SegListBlocks: %ld GlobalVec: %ld\n", BE(fhb->fhb_Priority), BE(fhb->fhb_Startup));
        DEBUG("[FSHD] FileSysName: %s\n", fhb->fhb_FileSysName + 1);
#endif

        for (int i = 0; i < NUM_FILESYSTEMS; i++) {
//FIXME HACK -> Skip DOS7 because it hangs z3660_scsi.device
#define DOS7 ((uint32_t)0x07534f44L)
            if (DOS7 == fhb->fhb_DosType) {
            	DEBUG("[FSHD] File system %c%c%c/%d skipped.\n", dosID[0], dosID[1], dosID[2], dosID[3]);
            	if (BE(fhb->fhb_Next) == 0xFFFFFFFF)
            		goto fs_done;

            	goto skip_fs_load_lseg;
            }
            if (filesystems[i].FS_ID == fhb->fhb_DosType) {
                DEBUG("[FSHD] File system %c%c%c/%d already loaded. Skipping.\n", dosID[0], dosID[1], dosID[2], dosID[3]);
                if (BE(fhb->fhb_Next) == 0xFFFFFFFF)
                    goto fs_done;

                goto skip_fs_load_lseg;
            }
        }
//#define ENABLE_LOAD_SEG
#ifdef ENABLE_LOAD_SEG
        if (load_lseg(d->fd, &filesystems[piscsi_num_fs].binary_data, &filesystems[piscsi_num_fs].h_info, filesystems[piscsi_num_fs].relocs, d->block_size) != -1) {
            filesystems[piscsi_num_fs].FS_ID = fhb->fhb_DosType;
            filesystems[piscsi_num_fs].fhb = fhb;
            printf("[FSHD] Loaded and set up file system %d: %c%c%c/%d\n", piscsi_num_fs + 1, dosID[0], dosID[1], dosID[2], dosID[3]);
            if(0) // enable or disable filesystem save
            {
                char fs_save_filename[256];
                memset(fs_save_filename, 0x00, 256);
                sprintf(fs_save_filename,DEFAULT_ROOT "data/fs/%c%c%c.%d", dosID[0], dosID[1], dosID[2], dosID[3]);
                FIL save_fs;
                int ret = f_open(&save_fs,fs_save_filename, FA_READ|FA_WRITE|FA_OPEN_EXISTING);
                if (ret != FR_OK) {
                    ret = f_open(&save_fs,fs_save_filename, FA_READ|FA_WRITE|FA_CREATE_NEW);
                    if (ret == FR_OK) {
                    	unsigned int n_bytes;
                        f_write(&save_fs,filesystems[piscsi_num_fs].binary_data, filesystems[piscsi_num_fs].h_info.byte_size, &n_bytes);
                        f_close(&save_fs);
                        printf("[FSHD] File system %c%c%c/%d saved to fs storage.\n", dosID[0], dosID[1], dosID[2], dosID[3]);
                    } else {
                        printf("[FSHD] Failed to save file system to fs storage. (Permission issues?)\n");
                    }
                } else {
                    f_close(&save_fs);
                }
            }
            piscsi_num_fs++;
        }
#endif
        if (BE(fhb->fhb_Next) == 0xFFFFFFFF)
        	goto fs_done;
skip_fs_load_lseg:;
        fs_found++;
        f_lseek(d->fd, ((uint64_t)BE(fhb->fhb_Next)) * d->block_size);
        fhb_block = malloc(d->block_size);
        fhb = (struct FileSysHeaderBlock *)fhb_block;
        unsigned int n_bytes;
        f_read(d->fd, fhb_block, d->block_size,&n_bytes);
    }

    if (!fs_found) {
        DEBUG("[!!!FSHD] No file systems found on hard drive!\n");
    }

fs_done:;
    if (fhb_block)
        free(fhb_block);
}

struct piscsi_dev *piscsi_get_dev(uint8_t index) {
    return &devs[index];
}

FIL fd[8];

void piscsi_map_drive(char *filename, uint8_t index) {
    if (index > 7) {
        printf("[PISCSI] Drive index %d out of range.\nUnable to map file %s to drive.\n", index, filename);
        return;
    }
    FIL *tmp_fd=&fd[index];
	int ret = f_open(tmp_fd,filename, FA_READ|FA_WRITE|FA_OPEN_EXISTING);
    if (ret != FR_OK) {
        printf("[PISCSI] Failed to open file %s, could not map drive %d.\n", filename, index);
        return;
    }

    char hdfID[512];
    memset(hdfID, 0x00, 512);
    unsigned int n_bytes;
    f_read(tmp_fd, hdfID, 512,&n_bytes);

    hdfID[3] = '\0';
    if (strcmp(hdfID, "DOS") == 0 || strcmp(hdfID, "PFS") == 0 || strcmp(hdfID, "PDS") == 0 || strcmp(hdfID, "SFS") == 0) {
        printf("[!!!PISCSI] The disk image %s is a UAE Single Partition Hardfile!\n", filename);
        printf("[!!!PISCSI] WARNING: PiSCSI does NOT support UAE Single Partition Hardfiles!\n");
        printf("[!!!PISCSI] PLEASE check the PiSCSI readme file in the GitHub repo for more information.\n");
        printf("[!!!PISCSI] If this is merely an empty or placeholder file you've created to partition and format on the Amiga, please disregard this warning message.\n");
    }

    struct piscsi_dev *d = &devs[index];

    FSIZE_t file_size = f_size(tmp_fd);//lseek(tmp_fd, 0, SEEK_END);
    d->fs = file_size;
    d->fd = tmp_fd;
    f_lseek(tmp_fd, 0);
    printf("[PISCSI] Map %d: [%s] - %llu bytes.\n", index, filename, file_size);

    if (piscsi_parse_rdb(d) == -1) {
        printf("[PISCSI] No RDB found on disk, making up some CHS values.\n");
        d->h = 16;
        d->s = 63;
        d->c = (file_size / 512) / (d->s * d->h);
        d->block_size = 512;
    }
    DEBUG("[PISCSI] CHS: %ld %d %d\n", d->c, d->h, d->s);

    DEBUG("Finding partitions.\n");
    piscsi_find_partitions(d);
    DEBUG("Finding file systems.\n");
    piscsi_find_filesystems(d);
    DEBUG("Done.\n");

    DEBUG("Enabling fast seek.\n");
    d->fd->cltbl = d->SeekTbl;			// Enable fast seek (set address of buffer)

    d->SeekTbl[0] = sizeof d->SeekTbl / sizeof d->SeekTbl[0];	// Buffer size
    int res = f_lseek(d->fd, CREATE_LINKMAP);	// Create link map table
    if (res == FR_OK) {
    	DEBUG("[FASTSEEK %d] %lu clusters, ",index, (d->fd->obj.fs->n_fatent - 1) );
    	DEBUG((d->SeekTbl[0] > 4) ? "fragmented in %ld.\n" : "contiguous.\n", d->SeekTbl[0] / 2 - 1);
    	DEBUG("[FASTSEEK %d] %lu items used.\n",index, d->SeekTbl[0]);
    }
    if (res == FR_NOT_ENOUGH_CORE) {
        printf("[FASTSEEK %d] %lu items required to create the link map table.\n",index, d->SeekTbl[0]);
    }
    DEBUG("Done.\n");

}

void piscsi_unmap_drive(uint8_t index) {
    if (devs[index].fd != 0) {
        DEBUG("[PISCSI] Unmapped drive %d.\n", index);
        f_close (devs[index].fd);
        devs[index].fd = 0;
    }
}

char *io_cmd_name(int index) {
    switch (index) {
        case CMD_INVALID: return "INVALID";
        case CMD_RESET: return "RESET";
        case CMD_READ: return "READ";
        case CMD_WRITE: return "WRITE";
        case CMD_UPDATE: return "UPDATE";
        case CMD_CLEAR: return "CLEAR";
        case CMD_STOP: return "STOP";
        case CMD_START: return "START";
        case CMD_FLUSH: return "FLUSH";
        case TD_MOTOR: return "TD_MOTOR";
        case TD_SEEK: return "SEEK";
        case TD_FORMAT: return "FORMAT";
        case TD_REMOVE: return "REMOVE";
        case TD_CHANGENUM: return "CHANGENUM";
        case TD_CHANGESTATE: return "CHANGESTATE";
        case TD_PROTSTATUS: return "PROTSTATUS";
        case TD_RAWREAD: return "RAWREAD";
        case TD_RAWWRITE: return "RAWWRITE";
        case TD_GETDRIVETYPE: return "GETDRIVETYPE";
        case TD_GETNUMTRACKS: return "GETNUMTRACKS";
        case TD_ADDCHANGEINT: return "ADDCHANGEINT";
        case TD_REMCHANGEINT: return "REMCHANGEINT";
        case TD_GETGEOMETRY: return "GETGEOMETRY";
        case TD_EJECT: return "EJECT";
        case TD_LASTCOMM: return "LASTCOMM/READ64";
        case TD_WRITE64: return "WRITE64";
        case HD_SCSICMD: return "HD_SCSICMD";
        case NSCMD_DEVICEQUERY: return "NSCMD_DEVICEQUERY";
        case NSCMD_TD_READ64: return "NSCMD_TD_READ64";
        case NSCMD_TD_WRITE64: return "NSCMD_TD_WRITE64";
        case NSCMD_TD_FORMAT64: return "NSCMD_TD_FORMAT64";

        default:
            return "[!!!PISCSI] Unhandled IO command";
    }
}

#define GETSCSINAME(a) case a: return ""#a"";
#define SCSIUNHANDLED(a) return "[!!!PISCSI] Unhandled SCSI command "#a"";

char *scsi_cmd_name(int index) {
    switch(index) {
        GETSCSINAME(SCSICMD_TEST_UNIT_READY);
        GETSCSINAME(SCSICMD_INQUIRY);
        GETSCSINAME(SCSICMD_READ_6);
        GETSCSINAME(SCSICMD_WRITE_6);
        GETSCSINAME(SCSICMD_READ_10);
        GETSCSINAME(SCSICMD_WRITE_10);
        GETSCSINAME(SCSICMD_READ_CAPACITY_10);
        GETSCSINAME(SCSICMD_MODE_SENSE_6);
        GETSCSINAME(SCSICMD_READ_DEFECT_DATA_10);
        default:
            return "[!!!PISCSI] Unhandled SCSI command";
    }
}

void print_piscsi_debug_message(int index) {
    int32_t r = 0;

    switch (index) {
        case DBG_INIT:
            DEBUG("[PISCSI] Initializing devices.\n");
            break;
        case DBG_OPENDEV:
            if (piscsi_dbg[0] != 255) {
                DEBUG("[PISCSI] Opening device %ld (%ld). Flags: %ld (%.2lX)\n", piscsi_dbg[0], piscsi_dbg[2], piscsi_dbg[1], piscsi_dbg[1]);
            }
            break;
        case DBG_CLEANUP:
            DEBUG("[PISCSI] Cleaning up.\n");
            break;
        case DBG_CHS:
            DEBUG("[PISCSI] C/H/S: %ld / %ld / %ld\n", piscsi_dbg[0], piscsi_dbg[1], piscsi_dbg[2]);
            break;
        case DBG_BEGINIO:
            DEBUG("[PISCSI] BeginIO: io_Command: %ld (%s) - io_Flags = %ld - quick: %ld\n", piscsi_dbg[0], io_cmd_name(piscsi_dbg[0]), piscsi_dbg[1], piscsi_dbg[2]);
            break;
        case DBG_ABORTIO:
            DEBUG("[PISCSI] AbortIO!\n");
            break;
        case DBG_SCSICMD:
            DEBUG("[PISCSI] SCSI Command %ld (%s)\n", piscsi_dbg[1], scsi_cmd_name(piscsi_dbg[1]));
            DEBUG("Len: %ld - %.2lX %.2lX %.2lX - Command Length: %ld\n", piscsi_dbg[0], piscsi_dbg[1], piscsi_dbg[2], piscsi_dbg[3], piscsi_dbg[4]);
            break;
        case DBG_SCSI_UNKNOWN_MODESENSE:
            DEBUG("[!!!PISCSI] SCSI: Unknown modesense %.4lX\n", piscsi_dbg[0]);
            break;
        case DBG_SCSI_UNKNOWN_COMMAND:
            DEBUG("[!!!PISCSI] SCSI: Unknown command %.4lX\n", piscsi_dbg[0]);
            break;
        case DBG_SCSIERR:
            DEBUG("[!!!PISCSI] SCSI: An error occured: %.4lX\n", piscsi_dbg[0]);
            break;
        case DBG_IOCMD:
            DEBUG_TRIVIAL("[PISCSI] IO Command %d (%s)\n", piscsi_dbg[0], io_cmd_name(piscsi_dbg[0]));
            break;
        case DBG_IOCMD_UNHANDLED:
            DEBUG("[!!!PISCSI] WARN: IO command %.4lX (%s) is unhandled by driver.\n", piscsi_dbg[0], io_cmd_name(piscsi_dbg[0]));
            break;
        case DBG_SCSI_FORMATDEVICE:
            DEBUG("[PISCSI] Get SCSI FormatDevice MODE SENSE.\n");
            break;
        case DBG_SCSI_RDG:
            DEBUG("[PISCSI] Get SCSI RDG MODE SENSE.\n");
            break;
        case DBG_SCSICMD_RW10:
#ifdef PISCSI_DEBUG
            r = 0;//get_mapped_item_by_address(cfg, piscsi_dbg[0]);
            struct SCSICmd_RW10 *rwdat = NULL;
            char data[10];
            if (r != -1) {
                uint32_t addr = piscsi_dbg[0];// - cfg->map_offset[r];
                rwdat = (struct SCSICmd_RW10 *)addr;//(&cfg->map_data[r][addr]);
            }
            else {
                DEBUG_TRIVIAL("[RW10] scsiData: %.8X\n", piscsi_dbg[0]);
                for (int i = 0; i < 10; i++) {
                    data[i] = read8(piscsi_dbg[0] + i);
                }
                rwdat = (struct SCSICmd_RW10 *)data;
            }
            if (rwdat) {
                DEBUG_TRIVIAL("[RW10] CMD: %.2X\n", rwdat->opcode);
                DEBUG_TRIVIAL("[RW10] RDP: %.2X\n", rwdat->rdprotect_flags);
                DEBUG_TRIVIAL("[RW10] Block: %d (%d)\n", rwdat->block, BE(rwdat->block));
                DEBUG_TRIVIAL("[RW10] Res_Group: %.2X\n", rwdat->res_groupnum);
                DEBUG_TRIVIAL("[RW10] Len: %d (%d)\n", rwdat->len, BE16(rwdat->len));
            }
#endif
            break;
        case DBG_SCSI_DEBUG_MODESENSE_6:
            DEBUG_TRIVIAL("[PISCSI] SCSI ModeSense debug. Data: %.8X\n", piscsi_dbg[0]);
            r = 0;//get_mapped_item_by_address(cfg, piscsi_dbg[0]);
            if (r != -1) {
#ifdef PISCSI_DEBUG
                uint32_t addr = piscsi_dbg[0];// - cfg->map_offset[r];
                struct SCSICmd_ModeSense6 *sense = (struct SCSICmd_ModeSense6 *)addr;//(&cfg->map_data[r][addr]);
                DEBUG_TRIVIAL("[SenseData] CMD: %.2X\n", sense->opcode);
                DEBUG_TRIVIAL("[SenseData] DBD: %d\n", sense->reserved_dbd & 0x04);
                DEBUG_TRIVIAL("[SenseData] PC: %d\n", (sense->pc_pagecode & 0xC0 >> 6));
                DEBUG_TRIVIAL("[SenseData] PageCodes: %.2X %.2X\n", (sense->pc_pagecode & 0x3F), sense->subpage_code);
                DEBUG_TRIVIAL("[SenseData] AllocLen: %d\n", sense->alloc_len);
                DEBUG_TRIVIAL("[SenseData] Control: %.2X (%d)\n", sense->control, sense->control);
#endif
            }
            else {
                DEBUG("[!!!PISCSI] ModeSense data not immediately available.\n");
            }
            break;
        default:
            printf("[!!!PISCSI] No debug message available for index %d.\n", index);
            break;
    }
}

#define DEBUGME_SIMPLE(i, s) case i: DEBUGME(s); break;
//#define DEBUGME printf
#define DEBUGME(...)
void piscsi_debugme(uint32_t index) {
    switch (index) {
        DEBUGME_SIMPLE(1, "[PISCSI-DEBUGME] Arrived at DiagEntry.\n");
        DEBUGME_SIMPLE(2, "[PISCSI-DEBUGME] Arrived at BootEntry, for some reason.\n");
        DEBUGME_SIMPLE(3, "[PISCSI-DEBUGME] Init: Interrupt disable.\n");
        DEBUGME_SIMPLE(4, "[PISCSI-DEBUGME] Init: Copy/reloc driver.\n");
        DEBUGME_SIMPLE(5, "[PISCSI-DEBUGME] Init: InitResident.\n");
        DEBUGME_SIMPLE(7, "[PISCSI-DEBUGME] Init: Begin partition loop.\n");
        DEBUGME_SIMPLE(8, "[PISCSI-DEBUGME] Init: Partition loop done. Cleaning up and returning to Exec.\n");
        DEBUGME_SIMPLE(9, "[PISCSI-DEBUGME] Init: Load file systems.\n");
        DEBUGME_SIMPLE(10, "[PISCSI-DEBUGME] Init: AllocMem for resident.\n");
        DEBUGME_SIMPLE(11, "[PISCSI-DEBUGME] Init: Checking if resident is loaded.\n");
        DEBUGME_SIMPLE(22, "[PISCSI-DEBUGME] Arrived at BootEntry.\n");
        DEBUGME_SIMPLE(33, "[PISCSI-DEBUGME] FileSystem.resource not available, creating.\n");
        DEBUGME_SIMPLE(34, "[PISCSI-DEBUGME] FSLoop.\n");
        DEBUGME_SIMPLE(37, "[PISCSI-DEBUGME] FSDone.\n");
        DEBUGME_SIMPLE(39, "[PISCSI-DEBUGME] AllocMem size\n");
        DEBUGME_SIMPLE(40, "[PISCSI-DEBUGME] AllocMem attributes\n");
        DEBUGME_SIMPLE(41, "[PISCSI-DEBUGME] AllocMem()\n");
        DEBUGME_SIMPLE(45, "[PISCSI-DEBUGME] FSNext.\n");
        DEBUGME_SIMPLE(390, "[PISCSI-DEBUGME] No entries.\n");
        DEBUGME_SIMPLE(480, "[PISCSI-DEBUGME] AlreadyLoaded.\n");
        DEBUGME_SIMPLE(800, "[PISCSI-DEBUGME] Cleaning Load exec base.\n");
        DEBUGME_SIMPLE(801, "[PISCSI-DEBUGME] Closing library.\n");
        DEBUGME_SIMPLE(802, "[PISCSI-DEBUGME] Library closed.\n");
        DEBUGME_SIMPLE(803, "[PISCSI-DEBUGME] Delay.\n");
        DEBUGME_SIMPLE(804, "[PISCSI-DEBUGME] Re-enable Interrupts.\n");
        DEBUGME_SIMPLE(805, "[PISCSI-DEBUGME] Return Success.\n");
        case 30:
            DEBUGME("[PISCSI-DEBUGME] LoadFileSystems: Opening FileSystem.resource.\n");
            rom_cur_fs = 0;
            break;
        case 31:
            DEBUGME("[PISCSI-DEBUGME] OpenResource result: %ld\n", piscsi_u32[0]);
            break;
        case 32:
            DEBUGME("[PISCSI-DEBUGME] Could not open FileSystem.resource!\n");
            break;
        case 35:
            DEBUGME("[PISCSI-DEBUGME] stuff output\n");
            break;
        case 36:
            DEBUGME("[PISCSI-DEBUGME] Debug pointers: %.8lX %.8lX %.8lX %.8lX\n", piscsi_u32[0], piscsi_u32[1], piscsi_u32[2], piscsi_u32[3]);
            break;
        default:
            DEBUGME("[!!!PISCSI-DEBUGME] No debugme message for index %ld!\n", index);
            break;
    }

    if (index == 8) {
        stop_cpu_emulation(1);
    }
}

void handle_piscsi_reg_write(uint32_t addr, uint32_t val, uint8_t type) {
	XGpioPs_WritePin(&GpioPs, LED1, 0); // ON
    int32_t r;
    uint32_t map;
#ifndef PISCSI_DEBUG
    if (type) {}
#endif

    struct piscsi_dev *d = &devs[piscsi_cur_drive];

    uint16_t cmd = addr;
    switch (cmd) {
        case PISCSI_CMD_READ64:
        case PISCSI_CMD_READ:
        case PISCSI_CMD_READBYTES:
            d = &devs[val];
            if (d->fd == 0) {
                DEBUG("[!!!PISCSI] BUG: Attempted read from unmapped drive %ld.\n", val);
                break;
            }

            if (cmd == PISCSI_CMD_READBYTES) {
                DEBUG("[PISCSI-%ld] %ld byte READBYTES from block %ld to address %.8lX\n", val, piscsi_u32[1], piscsi_u32[0] / d->block_size, piscsi_u32[2]);
                uint32_t src = piscsi_u32[0];
                d->lba = (src / d->block_size);
                f_lseek(d->fd, src);
            }
            else if (cmd == PISCSI_CMD_READ) {
                DEBUG("[PISCSI-%ld] %ld byte READ from block %ld to address %.8lX\n", val, piscsi_u32[1], piscsi_u32[0], piscsi_u32[2]);
                d->lba = piscsi_u32[0];
                f_lseek(d->fd, (((uint64_t)piscsi_u32[0]) * d->block_size));
            }
            else {
                uint64_t src = piscsi_u32[3];
                src = (src << 32) | piscsi_u32[0];
                DEBUG("[PISCSI-%ld] %ld byte READ64 from block %lld to address %.8lX\n", val, piscsi_u32[1], (src / d->block_size), piscsi_u32[2]);
                d->lba = (src / d->block_size);
                f_lseek(d->fd, src);
            }

            map = piscsi_u32[2];//get_mapped_data_pointer_by_address(cfg, piscsi_u32[2]);
            if ( (config.cpu_ram        && (map>=0x08000000) && (map<0x10000000))
               ||(config.autoconfig_ram && (map>=0x40000000) && (map<(0x40000000+0x10000000)))
			   )
            {
            	if(map>=0x40000000) map-=(0x40000000-0x20000000);
                DEBUG("[PISCSI-%ld] \"DMA\" Read goes to mapped range 0x%08lX.\n", val, map);
                unsigned int n_bytes;
                FRESULT res=f_read(d->fd, (uint8_t *)map, piscsi_u32[1],&n_bytes);
                Xil_L1DCacheFlushRange((INTPTR)map,n_bytes);
                Xil_L2CacheFlushRange((INTPTR)map,n_bytes);
                used_dma=0;
                DEBUG("            Bytes read %d\n",n_bytes);
            	if(n_bytes!=piscsi_u32[1])
            	{
            		printf("SCSI ERROR!!! bytes_to_read=%ld, bytes_read=%d\n",piscsi_u32[1],n_bytes);
            		printf("SCSI ERROR!!! res = %d, d = %8lX, fd = %8lX\n",res,(uint32_t)d,(uint32_t)d->fd);
            	}
            } else {
            	DEBUG("[PISCSI-%ld] No mapped range found for read.\n", val);
            	DEBUG("Begin data read from disk: 0x%08lX to 0x%08lX\n",piscsi_u32[0],piscsi_u32[2]);
            	unsigned int n_bytes;
            	if(piscsi_u32[1]>=0x180000)
                    printf("ERROR SCSI read length>0x180000 (0x%08lX)\n",piscsi_u32[1]);
            	uint8_t *buffer=(uint8_t *)SCSI_NO_DMA_ADDRESS;
            	FRESULT res=f_read(d->fd, buffer, piscsi_u32[1], &n_bytes);
                Xil_L1DCacheFlushRange((INTPTR)buffer,n_bytes);
                Xil_L2CacheFlushRange((INTPTR)map,n_bytes);
            	used_dma = piscsi_u32[2];
                DEBUG("            Bytes read %d\n",n_bytes);
            	if(n_bytes!=piscsi_u32[1])
            	{
            		printf("SCSI ERROR!!! bytes_to_read=%ld, bytes_read=%d\n",piscsi_u32[1],n_bytes);
            		printf("SCSI ERROR!!! res = %d, d = %8lX, fd = %8lX\n",res,(uint32_t)d,(uint32_t)d->fd);
            	}
            }
            break;
        case PISCSI_CMD_WRITE64:
        case PISCSI_CMD_WRITE:
        case PISCSI_CMD_WRITEBYTES:
            d = &devs[val];
            if (d->fd == 0) {
                DEBUG("[PISCSI] BUG: Attempted write to unmapped drive %ld.\n", val);
                break;
            }

            if (cmd == PISCSI_CMD_WRITEBYTES) {
                DEBUG("[PISCSI-%ld] %ld byte WRITEBYTES to block %ld from address %.8lX\n", val, piscsi_u32[1], piscsi_u32[0] / d->block_size, piscsi_u32[2]);
                uint32_t src = piscsi_u32[0];
                d->lba = (src / d->block_size);
                f_lseek(d->fd, src);
            }
            else if (cmd == PISCSI_CMD_WRITE) {
                DEBUG("[PISCSI-%ld] %ld byte WRITE to block %ld from address %.8lX\n", val, piscsi_u32[1], piscsi_u32[0], piscsi_u32[2]);
                d->lba = piscsi_u32[0];
                f_lseek(d->fd, (((uint64_t)piscsi_u32[0]) * d->block_size));
            }
            else {
                uint64_t src = piscsi_u32[3];
                src = (src << 32) | piscsi_u32[0];
                DEBUG("[PISCSI-%ld] %ld byte WRITE64 to block %lld from address %.8lX\n", val, piscsi_u32[1], (src / d->block_size), piscsi_u32[2]);
                d->lba = (src / d->block_size);
                f_lseek(d->fd, src);
            }

            map = piscsi_u32[2];//get_mapped_data_pointer_by_address(cfg, piscsi_u32[2]);
            if ( ((map>=0x08000000) && (map<0x10000000) && config.cpu_ram)
               ||((map>=0x40000000) && (map<0x50000000) && config.autoconfig_ram)
               )
            {
            	if(map>=0x40000000) map-=0x20000000;
            	DEBUG("[PISCSI-%ld] \"DMA\" Write comes from mapped range 0x%08lX.\n", val, map);
                unsigned int n_bytes;
                f_write(d->fd, (uint8_t *)map, piscsi_u32[1],&n_bytes);
                DEBUG("             Bytes written %d\n",n_bytes);
                used_dma=0;
            	if(n_bytes!=piscsi_u32[1])
            		printf("SCSI ERROR!!! bytes_to_write=%ld, bytes_read=%d\n",piscsi_u32[1],n_bytes);
            }
            else {
            	DEBUG("[PISCSI-%ld] No mapped range found for write.\n", val);
            	DEBUG("          Begin data write to disk: 0x%08lX to 0x%08lX\n",piscsi_u32[0],piscsi_u32[2]);
            	unsigned int n_bytes;
            	if(piscsi_u32[1]>=0x180000)
                    printf("ERROR SCSI write length>0x180000 (0x%08lX)\n",piscsi_u32[1]);
            	uint8_t *buffer=(uint8_t *)SCSI_NO_DMA_ADDRESS;
                f_write(d->fd, buffer, piscsi_u32[1], &n_bytes);
                used_dma = piscsi_u32[2];
                DEBUG("             Bytes written %d\n",n_bytes);
            	if(n_bytes!=piscsi_u32[1])
            		printf("SCSI ERROR!!! bytes_to_write=%ld, bytes_read=%d\n",piscsi_u32[1],n_bytes);
            }
            break;
        case PISCSI_CMD_ADDR1:
        case PISCSI_CMD_ADDR2:
        case PISCSI_CMD_ADDR3:
        case PISCSI_CMD_ADDR4: {
            int i = (addr - PISCSI_CMD_ADDR1) / 4;
            piscsi_u32[i] = val;
            break;
        }
        case PISCSI_CMD_DRVNUM:
            if (val > 6) {
                piscsi_cur_drive = 255;
            }
            else {
                piscsi_cur_drive = val;
            }
            if (piscsi_cur_drive != 255) {
                DEBUG("[PISCSI] (%s) Drive number set to %d (%ld)\n", op_type_names[type], piscsi_cur_drive, val);
            }
            break;
        case PISCSI_CMD_DRVNUMX:
            piscsi_cur_drive = val;
            DEBUG("[PISCSI] DRVNUMX: %ld.\n", val);
            break;
        case PISCSI_CMD_DEBUGME:
            piscsi_debugme(val);
//            printf("piscsi debugme %ld\n",val);
            break;
        case PISCSI_CMD_DRIVER:
            DEBUG("[PISCSI] Driver copy/patch called, destination address %.8lX.\n", val);
            r = 0;//get_mapped_item_by_address(cfg, val);
            if ( ((val>=0x08000000) && (val<0x10000000) && config.cpu_ram)
               ||((val>=0x40000000) && (val<0x50000000) && config.autoconfig_ram)
               )
            {
            	if(val>=0x40000000) val-=0x20000000;
//            if (r != -1) {
                uint32_t addr = val;// - cfg->map_offset[r];
                uint8_t *dst_data = 0;//cfg->map_data[r];
                uint8_t cur_partition = 0;
                memcpy(dst_data + addr, piscsi_rom_ptr + PISCSI_DRIVER_OFFSET, BOOT_ROM_SIZE - PISCSI_DRIVER_OFFSET);

                piscsi_hinfo.base_offset = val;

                reloc_hunks(piscsi_hreloc, dst_data + addr, &piscsi_hinfo);

                #define PUTNODELONG(val) *(uint32_t *)&dst_data[p_offs] = htobe32(val); p_offs += 4;
                #define PUTNODELONGBE(val) *(uint32_t *)&dst_data[p_offs] = val; p_offs += 4;

                for (int i = 0; i < 128; i++) {
                    rom_partitions[i] = 0;
                    rom_partition_prio[i] = 0;
                    rom_partition_dostype[i] = 0;
                }
                rom_cur_partition = 0;

//                uint32_t data_addr = addr + 0x3F00;
                uint32_t data_addr = addr + BOOT_ROM_SIZE -0x100;
                sprintf((char *)dst_data + data_addr, "z3660_scsi.device");
                uint32_t addr2 = addr + BOOT_ROM_SIZE;
                for (int i = 0; i < NUM_UNITS; i++) {
                    if (devs[i].fd == 0)
                        goto skip_disk;

                    if (devs[i].num_partitions) {
                        uint32_t p_offs = addr2;
                        DEBUG("[PISCSI] Adding %ld partitions for unit %d\n", devs[i].num_partitions, i);
                        for (uint32_t j = 0; j < devs[i].num_partitions; j++) {
                            DEBUG("Partition %ld: %s\n", j, devs[i].pb[j]->pb_DriveName + 1);
                            sprintf((char *)dst_data + p_offs, "%s", devs[i].pb[j]->pb_DriveName + 1);
                            p_offs += 0x20;
                            PUTNODELONG(addr2);// + cfg->map_offset[r]);
                            PUTNODELONG(data_addr);// + cfg->map_offset[r]);
                            PUTNODELONG(i);
                            PUTNODELONG(0);
                            uint32_t nodesize = (be32toh(devs[i].pb[j]->pb_Environment[0]) + 1) * 4;
                            memcpy(dst_data + p_offs, devs[i].pb[j]->pb_Environment, nodesize);

                            struct pihd_dosnode_data *dat = (struct pihd_dosnode_data *)(&dst_data[addr2+0x20]);

                            if (BE(devs[i].pb[j]->pb_Flags) & 0x01) {
                                DEBUG("Partition is bootable.\n");
                                rom_partition_prio[cur_partition] = BE(dat->priority);
                            }
                            else {
                                DEBUG("Partition is not bootable.\n");
                                rom_partition_prio[cur_partition] = -128;
                            }

                            DEBUG("DOSNode Data:\n");
                            DEBUG("Name: %s Device: %s\n", dst_data + addr2, dst_data + data_addr);
                            DEBUG("Unit: %ld Flags: %ld Pad1: %ld\n", BE(dat->unit), BE(dat->flags), BE(dat->pad1));
                            DEBUG("Node len: %ld Block len: %ld\n", BE(dat->node_len) * 4, BE(dat->block_len) * 4);
                            DEBUG("H: %ld SPB: %ld BPS: %ld\n", BE(dat->surf), BE(dat->secs_per_block), BE(dat->blocks_per_track));
                            DEBUG("Reserved: %ld Prealloc: %ld\n", BE(dat->reserved_blocks), BE(dat->pad2));
                            DEBUG("Interleaved: %ld Buffers: %ld Memtype: %ld\n", BE(dat->interleave), BE(dat->buffers), BE(dat->mem_type));
                            DEBUG("Lowcyl: %ld Highcyl: %ld Prio: %ld\n", BE(dat->lowcyl), BE(dat->highcyl), BE(dat->priority));
                            DEBUG("Maxtransfer: %.8lX Mask: %.8lX\n", BE(dat->maxtransfer), BE(dat->transfer_mask));
                            DEBUG("DOSType: %.8lX\n", BE(dat->dostype));

                            rom_partitions[cur_partition] = addr2 + 0x20;// + cfg->map_offset[r];
                            rom_partition_dostype[cur_partition] = dat->dostype;
                            cur_partition++;
                            addr2 += 0x100;
                            p_offs = addr2;
                        }
                    }
skip_disk:;
                }
            }
            else
            {
            	printf("[PISCSI] Address 0x%08lX not mapped in FPGA RAM...\n",val);
            }

            break;
        case PISCSI_CMD_NEXTPART:
            DEBUG("[PISCSI] Switch partition %ld -> %ld\n", rom_cur_partition, rom_cur_partition + 1);
            rom_cur_partition++;
            break;
        case PISCSI_CMD_NEXTFS:
            DEBUG("[PISCSI] Switch file system %ld -> %ld\n", rom_cur_fs, rom_cur_fs + 1);
            rom_cur_fs++;
            break;
        case PISCSI_CMD_COPYFS:
            DEBUG("[PISCSI] Copy file system %ld to %.8lX and reloc.\n", rom_cur_fs + 1, piscsi_u32[2]);
            r = 0;//get_mapped_item_by_address(cfg, piscsi_u32[2]);
            if (r != -1) {
                uint32_t addr = piscsi_u32[2];// - cfg->map_offset[r];
                memcpy(/*cfg->map_data[r] +*/ (uint8_t *)addr, filesystems[rom_cur_fs].binary_data, filesystems[rom_cur_fs].h_info.byte_size);
                filesystems[rom_cur_fs].h_info.base_offset = piscsi_u32[2];
                reloc_hunks(filesystems[rom_cur_fs].relocs, /*cfg->map_data[r] +*/(uint8_t*) addr, &filesystems[rom_cur_fs].h_info);
                filesystems[rom_cur_fs].handler = piscsi_u32[2];
            }
            break;
        case PISCSI_CMD_SETFSH: {
            int i = 0;
            DEBUG("[PISCSI] Set handler for partition %ld (DeviceNode: %.8lX)\n", rom_cur_partition, val);
            r = 0;//get_mapped_item_by_address(cfg, val);
            if (r != -1) {
                uint32_t addr = val;// - cfg->map_offset[r];
                struct DeviceNode *node = (struct DeviceNode *)(/*cfg->map_data[r] +*/ addr);
                char *dosID = (char *)&rom_partition_dostype[rom_cur_partition];

                DEBUG("[PISCSI] Partition DOSType is %c%c%c/%d\n", dosID[0], dosID[1], dosID[2], dosID[3]);
                for (i = 0; i < piscsi_num_fs; i++) {
                    if (rom_partition_dostype[rom_cur_partition] == filesystems[i].FS_ID) {
                        node->dn_SegList = htobe32((((filesystems[i].handler) + filesystems[i].h_info.header_size) >> 2));
                        node->dn_GlobalVec = 0xFFFFFFFF;
                        goto fs_found;
                    }
                }
                node->dn_GlobalVec = 0xFFFFFFFF;
                node->dn_SegList = 0;
                printf("[!!!PISCSI] Found no handler for file system %c%c%c/%d\n", dosID[0], dosID[1], dosID[2], dosID[3]);
fs_found:;
                DEBUG("[FS-HANDLER] Next: %ld Type: %.8lX\n", BE(node->dn_Next), BE(node->dn_Type));
                DEBUG("[FS-HANDLER] Task: %ld Lock: %ld\n", BE(node->dn_Task), BE(node->dn_Lock));
                DEBUG("[FS-HANDLER] Handler: %ld Stacksize: %ld\n", BE((uint32_t)node->dn_Handler), BE(node->dn_StackSize));
                DEBUG("[FS-HANDLER] Priority: %ld Startup: %ld (%.8lX)\n", BE((uint32_t)node->dn_Priority), BE(node->dn_Startup), BE(node->dn_Startup));
                DEBUG("[FS-HANDLER] SegList: %.8lX GlobalVec: %ld\n", BE((uint32_t)node->dn_SegList), BE(node->dn_GlobalVec));
                DEBUG("[PISCSI] Handler for partition %.8lX set to %.8lX (%.8lX).\n", BE((uint32_t)node->dn_Name), filesystems[i].FS_ID, filesystems[i].handler);
                Xil_L1DCacheFlush();
                Xil_L2CacheFlush();
            }
            break;
        }
        case PISCSI_CMD_LOADFS: {
            DEBUG("[PISCSI] Attempt to load file system for partition %ld from disk.\n", rom_cur_partition);
            r = 0;//get_mapped_item_by_address(cfg, val);
            if (r != -1) {
                char *dosID = (char *)&rom_partition_dostype[rom_cur_partition];
                filesystems[piscsi_num_fs].binary_data = NULL;
                filesystems[piscsi_num_fs].fhb = NULL;
                filesystems[piscsi_num_fs].FS_ID = rom_partition_dostype[rom_cur_partition];
                filesystems[piscsi_num_fs].handler = 0;
                if (load_fs(&filesystems[piscsi_num_fs], dosID) != -1) {
                    printf("[FSHD-Late] Loaded file system %c%c%c/%d from fs storage.\n", dosID[0], dosID[1], dosID[2], dosID[3]);
                    piscsi_u32[3] = piscsi_num_fs;
                    rom_cur_fs = piscsi_num_fs;
                    piscsi_num_fs++;
                } else {
                    printf("[FSHD-Late] Failed to load file system %c%c%c/%d from fs storage.\n", dosID[0], dosID[1], dosID[2], dosID[3]);
                    piscsi_u32[3] = 0xFFFFFFFF;
                }
            }
            break;
        }
        case PISCSI_DBG_VAL1:
        case PISCSI_DBG_VAL2:
        case PISCSI_DBG_VAL3:
        case PISCSI_DBG_VAL4:
        case PISCSI_DBG_VAL5:
        case PISCSI_DBG_VAL6:
        case PISCSI_DBG_VAL7:
        case PISCSI_DBG_VAL8: {
            int i = (addr - PISCSI_DBG_VAL1) / 4;
            piscsi_dbg[i] = val;
            break;
        }
        case PISCSI_DBG_MSG:
            print_piscsi_debug_message(val);
            break;
        default:
//            DEBUG("[!!!PISCSI] WARN: Unhandled %s register write to %.8lX: %ld\n", op_type_names[type], addr, val);
            printf("[!!!PISCSI] WARN: Unhandled register write to %.8lX: %ld\n", addr, val);
            break;
    }
	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
}

#define PIB 0x00

uint32_t handle_piscsi_read(uint32_t addr, uint8_t type) {
	XGpioPs_WritePin(&GpioPs, LED1, 0); // ON
    if (type) {}

    if (addr >= PISCSI_CMD_ROM) {
        uint32_t romoffs = addr - PISCSI_CMD_ROM;
        if (romoffs < (piscsi_rom_size + PIB)) {
//            DEBUG("[PISCSI] %s read from Boot ROM @$%.4X (%.8X): ", op_type_names[type], romoffs, addr);
            uint32_t v = 0;
            switch (type) {
                case OP_TYPE_BYTE:
                    v = piscsi_rom_ptr[romoffs - PIB];
//                    DEBUG("%.2X\n", v);
//                    printf("[PISCSI] Read from Boot ROM @$%.4lX (%.8lX): 0x%02lX\n", romoffs, addr, v);
                    break;
                case OP_TYPE_WORD:
                    v = be16toh(*((uint16_t *)(piscsi_rom_ptr + romoffs - PIB)));
//                    DEBUG("%.4X\n", v);
//                    printf("[PISCSI] Read from Boot ROM @$%.4lX (%.8lX): 0x%04lX\n", romoffs, addr, v);
                    break;
                case OP_TYPE_LONGWORD:
                    v = be32toh(*((uint32_t *)(piscsi_rom_ptr + romoffs - PIB)));
//                    DEBUG("%.8X\n", v);
//                    printf("[PISCSI] Read from Boot ROM @$%.4lX (%.8lX): 0x%08lX\n", romoffs, addr, v);
                    break;
            }
            Xil_L1DCacheFlushRange((INTPTR)piscsi_rom_ptr,BOOT_ROM_SIZE);
            Xil_L2CacheFlushRange((INTPTR)piscsi_rom_ptr,BOOT_ROM_SIZE);
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return v;
        }
    	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
        return 0;
    }

    switch (addr) {
        case PISCSI_CMD_ADDR1:
        case PISCSI_CMD_ADDR2:
        case PISCSI_CMD_ADDR3:
        case PISCSI_CMD_ADDR4: {
            int i = (addr - PISCSI_CMD_ADDR1) / 4;
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return piscsi_u32[i];
            break;
        }
        case PISCSI_CMD_DRVTYPE:
            if (devs[piscsi_cur_drive].fd == 0) {
                DEBUG("[PISCSI] %s Read from DRVTYPE %d, drive not attached.\n", op_type_names[type], piscsi_cur_drive);
            	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
                return 0;
            }
            DEBUG("[PISCSI] %s Read from DRVTYPE %d, drive attached.\n", op_type_names[type], piscsi_cur_drive);
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return 1;
            break;
        case PISCSI_CMD_DRVNUM:
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return piscsi_cur_drive;
            break;
        case PISCSI_CMD_CYLS:
            DEBUG("[PISCSI] %s Read from CYLS %d: %ld\n", op_type_names[type], piscsi_cur_drive, devs[piscsi_cur_drive].c);
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return devs[piscsi_cur_drive].c;
            break;
        case PISCSI_CMD_HEADS:
            DEBUG("[PISCSI] %s Read from HEADS %d: %d\n", op_type_names[type], piscsi_cur_drive, devs[piscsi_cur_drive].h);
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return devs[piscsi_cur_drive].h;
            break;
        case PISCSI_CMD_SECS:
            DEBUG("[PISCSI] %s Read from SECS %d: %d\n", op_type_names[type], piscsi_cur_drive, devs[piscsi_cur_drive].s);
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return devs[piscsi_cur_drive].s;
            break;
        case PISCSI_CMD_BLOCKS: {
            uint32_t blox = devs[piscsi_cur_drive].fs / devs[piscsi_cur_drive].block_size;
            DEBUG("[PISCSI] %s Read from BLOCKS %d: %ld\n", op_type_names[type], piscsi_cur_drive, blox);
            DEBUG("filesize: %lld (%lld blocks*block_size)\n", devs[piscsi_cur_drive].fs, ((uint64_t)blox)*devs[piscsi_cur_drive].block_size);
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return blox;
            break;
        }
        case PISCSI_CMD_GETPART: {
            DEBUG("[PISCSI] Get ROM partition %ld offset: %.8lX\n", rom_cur_partition, rom_partitions[rom_cur_partition]);
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return rom_partitions[rom_cur_partition];
            break;
        }
        case PISCSI_CMD_GETPRIO:
            DEBUG("[PISCSI] Get partition %ld boot priority: %ld\n", rom_cur_partition, rom_partition_prio[rom_cur_partition]);
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return rom_partition_prio[rom_cur_partition];
            break;
        case PISCSI_CMD_CHECKFS:
            DEBUG("[PISCSI] Get current loaded file system: %.8lX\n", filesystems[rom_cur_fs].FS_ID);
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return filesystems[rom_cur_fs].FS_ID;
        case PISCSI_CMD_FSSIZE:
            DEBUG("[PISCSI] Get alloc size of loaded file system: %ld\n", filesystems[rom_cur_fs].h_info.alloc_size);
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return filesystems[rom_cur_fs].h_info.alloc_size;
        case PISCSI_CMD_BLOCKSIZE:
            DEBUG("[PISCSI] Get block size of drive %d: %ld\n", piscsi_cur_drive, devs[piscsi_cur_drive].block_size);
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return devs[piscsi_cur_drive].block_size;
        case PISCSI_CMD_GET_FS_INFO: {
            int i = 0;
//            uint32_t val = piscsi_u32[1];
            int32_t r = 0;//get_mapped_item_by_address(cfg, val);
            if (r != -1) {
#ifdef PISCSI_DEBUG
//                uint32_t addr = val;// - cfg->map_offset[r];
                char *dosID = (char *)&rom_partition_dostype[rom_cur_partition];
                DEBUG("[PISCSI-GET-FS-INFO] Partition DOSType is %c%c%c/%d\n", dosID[0], dosID[1], dosID[2], dosID[3]);
#endif
                for (i = 0; i < piscsi_num_fs; i++) {
                    if (rom_partition_dostype[rom_cur_partition] == filesystems[i].FS_ID) {
                    	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
                        return 0;
                    }
                }
            }
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
            return 1;
        }
        case PISCSI_CMD_USED_DMA: {
        	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
        	uint32_t temp=used_dma;
//        	DEBUG("Read used_dma=%08lX\n",used_dma);
        	used_dma=0;
        	return(temp);
        }
        default:
            DEBUG("[!!!PISCSI] WARN: Unhandled %s register read from %.8lX\n", op_type_names[type], addr);
            break;
    }

	XGpioPs_WritePin(&GpioPs, LED1, 1); // OFF
    return 0;
}
