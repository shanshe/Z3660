// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ffconf.h>
#include <ff.h>
#include <diskio.h>
#include "../main.h"

#include <xparameters.h>
#include "../memorymap.h"
#include <xil_cache.h>

#include "z3660_scsi_enums.h"
#include "scsi.h"
#include "../config_file.h"
#include "../debug_console.h"

extern CONFIG config;
#define BE(val) be32toh(val)
#define BE16(val) be16toh(val)

// Uncomment the line below to enable debug output
//#define PISCSI_DEBUG
uint32_t used_dma=0;
//#define MEMCPY memcpy
#define MEMCPY memcpy_neon
extern void *(memcpy_neon)(void * s1, const void * s2, u32 n);

#ifdef PISCSI_DEBUG
#define read8(a) *(uint8_t*)(a)
#define DEBUG printf
//#define DEBUG_TRIVIAL printf
#define DEBUG_TRIVIAL(...)

static const char *op_type_names[4] = {
      "BYTE",
      "WORD",
      "LONGWORD",
      "MEM",
};
#else
#define DEBUG_TRIVIAL(...)
//#define NO_DEBUG_CONSOLE
#ifdef NO_DEBUG_CONSOLE
#define DEBUG(...)
#else
extern DEBUG_CONSOLE debug_console;
void DEBUG(const char *format, ...)
{
   if(debug_console.debug_scsi==0)
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

PISCSI_DEV devs[NUM_SCSI_UNITS_MAX];
PISCSI_FS filesystems[NUM_FILESYSTEMS];

uint8_t piscsi_num_fs = 0;

uint8_t piscsi_cur_drive = 0;
uint32_t piscsi_u32_read[4];
uint32_t piscsi_u32_write[4];
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

HUNK_INFO piscsi_hinfo;
HUNK_RELOC piscsi_hreloc[2048];
static FATFS fatfs;

int piscsi_init() {
   memset(filesystems, 0x00, sizeof(filesystems));
   if(config.scsiboot==0)
   {
      ACTIVITY_LED_OFF; // OFF
      return(0);
   }
   ACTIVITY_LED_ON; // ON

   for (int i = 0; i < NUM_SCSI_UNITS_MAX; i++) {
      devs[i].fd = 0;
      devs[i].lba = 0;
      devs[i].c = devs[i].h = devs[i].s = 0;
      devs[i].SeekTbl[0]=0;
   }
   for (int i = 0; i < 128; i++) {
      memset(partition_names[i], 0x00, 32);
      times_used[i] = 0;
   }
   num_partition_names = 0;
   TCHAR *Path = DEFAULT_ROOT;
   f_mount(&fatfs, Path, 1); // 1 mount immediately
   // force to load piscsi always
   piscsi_rom_ptr = NULL;
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
         ACTIVITY_LED_OFF; // OFF
         return(0); // Boot ROM disabled
      }
      piscsi_rom_size = f_size(&in);
      f_lseek(&in, 0);
      piscsi_rom_ptr =(uint8_t*) BOOT_ROM_ADDRESS;//malloc(piscsi_rom_size);
      unsigned int n_bytes;
      memset((uint8_t*) piscsi_rom_ptr,0,BOOT_ROM_SIZE);
      f_read(&in,piscsi_rom_ptr, piscsi_rom_size, &n_bytes);
      uint32_t* ptr_src=(uint32_t*)BOOT_ROM_ADDRESS;
      uint32_t* ptr_dst1=(uint32_t*)0x10006000;
      uint32_t* ptr_dst2=(uint32_t*)0x00E96000;
      for(int i=0;i<BOOT_ROM_SIZE/4;i++)
      {
         ptr_dst2[i]=
         ptr_dst1[i]= (ptr_src[i]);
//         ptr_dst2[i]=
//         ptr_dst1[i]= swap32(ptr_src[i]);
      }

      f_lseek(&in, PISCSI_DRIVER_OFFSET);
      process_hunks(&in, &piscsi_hinfo, piscsi_hreloc, PISCSI_DRIVER_OFFSET);
      f_close(&in);

      Xil_DCacheFlush();
      Xil_DCacheInvalidate();

      printf("[PISCSI] Loaded SCSI Boot ROM.\n");
   } else {
      printf("[PISCSI] Boot ROM already loaded.\n");
   }
   //    fflush(stdout);
   // first the hdfs
   for(int i=0;i<7;i++)
   {
      if(config.scsi_num[i]>=0 && config.scsi_num[i]<=19)
         if(config.hdf[config.scsi_num[i]][0]!=0)
            piscsi_map_drive(config.hdf[config.scsi_num[i]], i, 0, 0);
   }
   // and now the SD partitions
   uint8_t buff[512];
   DRESULT ret = disk_read(0,buff,0,1);
   if(ret==0)
   {
      for(int i=0;i<4;i++)
      {
         uint8_t *b = &buff[0x1be + 16 *  i];
         uint8_t p0_Type = b[4];
         uint32_t p0_Start = b[8] | (b[9] << 8) | (b[10] << 16) | (b[11] << 24);
         uint32_t p0_Len = b[12] | (b[13] << 8) | (b[14] << 16) | (b[15] << 24);
         if(p0_Type != 0) {
            printf("Partition %d: type 0x%02X, start 0x%08lX, length 0x%08lx\n",i,p0_Type, p0_Start, p0_Len);
            if(p0_Type == 0x76)// || p0_Type == 0x0E || p0_Type == 0x07)
            {
               piscsi_map_drive("",10+i,p0_Start,p0_Len);
            }
         }
      }
   }
   Xil_L1DCacheFlush();
   Xil_L2CacheFlush();

   ACTIVITY_LED_OFF; // OFF
   return(1); // Boot ROM loaded
}

void piscsi_shutdown() {
   if(config.scsiboot==0)
   {
      ACTIVITY_LED_OFF; // OFF
      return;
   }
   Xil_ExceptionDisable();

   printf("[PISCSI] Shutting down PiSCSI...");
   // first the hdfs
   for (int i = 0; i < 8; i++) {
      if (devs[i].fd != 0) {
         //            FRESULT res=
         f_close(devs[i].fd);
         //printf("\nresult %d %d",i,res);
         devs[i].fd = 0;
         devs[i].block_size = 0;
      }
   }
   // and now the SD partitions
   for (int i = 10; i < NUM_SCSI_UNITS_MAX; i++) {
      if (devs[i].fd != 0) {
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
   Xil_ExceptionEnable();
}

void piscsi_find_partitions(PISCSI_DEV *d) {
   ACTIVITY_LED_ON; // ON
   FIL *fd = d->fd;
   int cur_partition = 0;
   uint8_t tmp;

   if (!d->rdb || d->rdb->rdb_PartitionList == 0) {
      DEBUG("[PISCSI] No partitions on disk.\n");
      ACTIVITY_LED_OFF; // OFF
      return;
   }

   unsigned char *block = malloc(d->block_size);
   DEBUG("Malloc block=0x%08lX\n",(uint32_t)block);
   uint64_t next = ((uint64_t)BE(d->rdb->rdb_PartitionList));
   if(fd>(FIL *)1)
      f_lseek(fd, next * d->block_size);
   next_partition:;
   if(fd>(FIL *)1)
   {
      unsigned int n_bytes;
      f_read(fd, block, d->block_size,&n_bytes);
   }
   else
   {
      //DRESULT res =
      disk_read(0,block,next + d->start_block,1);
   }
   Xil_L1DCacheFlush();

   uint32_t first = be32toh(*((uint32_t *)&block[0]));
   if (first != PART_IDENTIFIER) {
      DEBUG("Entry at block %ld is not a valid partition. Aborting.\n", BE(d->rdb->rdb_PartitionList));
      DEBUG("Freeing block=0x%08lX\n",(uint32_t)block);
      free(block);
      ACTIVITY_LED_OFF; // OFF
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
      next = be32toh(pb->pb_Next);
      block = malloc(d->block_size);
      DEBUG("Malloc block=0x%08lX\n",(uint32_t)block);
//        lseek64(fd, next * d->block_size, SEEK_SET);
      if(fd>(FIL *)1)
         f_lseek(fd, next * d->block_size);
      cur_partition++;
      DEBUG("[PISCSI] Next partition at block %ld.\n", be32toh(pb->pb_Next));
      goto next_partition;
   }
   DEBUG("[PISCSI] No more partitions on disk.\n");
   d->num_partitions = cur_partition + 1;
   //    d->fshd_offs = lseek64(fd, 0, SEEK_CUR);
   if(fd>(FIL *)1)
      d->fshd_offs = fd->fptr;
   else
      d->fshd_offs = next*d->block_size;
   ACTIVITY_LED_OFF; // OFF
   return;
}

int piscsi_parse_rdb(PISCSI_DEV *d) {
   ACTIVITY_LED_ON; // ON
   FIL *fd = d->fd;
   int i = 0;
   uint8_t *block = malloc(PISCSI_MAX_BLOCK_SIZE);

   if(fd>(FIL *)1)
      f_lseek(fd, 0);
   for (i = 0; i < RDB_BLOCK_LIMIT; i++) {
      if(fd>(FIL *)1)
      {
         unsigned int n_bytes;
         f_read(fd, block, PISCSI_MAX_BLOCK_SIZE,&n_bytes);
      }
      else
      {
         //DRESULT res = 
         disk_read(0,block,i * 512 + d->start_block,PISCSI_MAX_BLOCK_SIZE/512);
      }
      Xil_L1DCacheFlush();
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
   ACTIVITY_LED_OFF; // OFF
   return 0;

   no_rdb_found:;
   if (block)
   {
      free(block);
   }

   ACTIVITY_LED_OFF; // OFF
   return -1;
}

void piscsi_refresh_drives() {
   piscsi_num_fs = 0;

   for (int i = 0; i < NUM_FILESYSTEMS; i++) {
      if (filesystems[i].binary_data) {
         DEBUG("Freeing filesystems[%d].binary_data=0x%08lX\n",i,(uint32_t)filesystems[i].binary_data);
         free(filesystems[i].binary_data);
         //            filesystems[i].binary_data = NULL;
      }
      if (filesystems[i].fhb) {
         DEBUG("Freeing filesystems[%d].fhb=0x%08lX\n",i,(uint32_t)filesystems[i].fhb);
         free(filesystems[i].fhb);
         //            filesystems[i].fhb = NULL;
      }
      //        filesystems[i].h_info.current_hunk = 0;
      //        filesystems[i].h_info.reloc_hunks = 0;
      //        filesystems[i].FS_ID = 0;
      //        filesystems[i].handler = 0;
   }
   memset(filesystems, 0x00, sizeof(filesystems));

   rom_cur_fs = 0;

   for (int i = 0; i < 128; i++) {
      memset(partition_names[i], 0x00, 32);
      times_used[i] = 0;
   }
   num_partition_names = 0;

   for (int i = 0; i < NUM_SCSI_UNITS_MAX; i++) {
      for (int j = 0; j < 16; j++) {
         if (devs[i].pb[j]) {
            DEBUG("Freeing devs[%d].pb[%d]=0x%08lX\n",i,j,(uint32_t)devs[i].pb[j]);
            free(devs[i].pb[j]);
            devs[i].pb[j] = NULL;
         }
      }
   }

   for (int i = 0; i < NUM_SCSI_UNITS_MAX; i++) {
      if (devs[i].fd != 0) {
         piscsi_parse_rdb(&devs[i]);
         piscsi_find_partitions(&devs[i]);
         piscsi_find_filesystems(&devs[i]);
      }
   }
}
void check_exist(char *path)
{
   FRESULT ret=f_stat(path,NULL);
   if(ret==FR_NO_FILE)
   {
      if(f_mkdir(path)==FR_OK)
         printf("Created directory %s\n",path);
      else
         printf("Can't create directory %s\n",path);
   }
}
void piscsi_find_filesystems(PISCSI_DEV *d) {
   if (!d->num_partitions)
      return;

   uint8_t fs_found = 0;

   uint8_t *fhb_block = malloc(d->block_size);
   DEBUG("Malloc fhb block=0x%08lX\n",(uint32_t)fhb_block);

   struct FileSysHeaderBlock *fhb = (struct FileSysHeaderBlock *)fhb_block;
   if(d->fd>(FIL *)1)
   {
      f_lseek(d->fd, d->fshd_offs);
      unsigned int n_bytes;
      f_read(d->fd, fhb_block, d->block_size,&n_bytes);
      DEBUG("[FSHD] Read %d bytes (should be %ld)\n",n_bytes,d->block_size);
   }
   else
   {
      //DRESULT res = 
      disk_read(0,fhb_block,d->fshd_offs/d->block_size + d->start_block,1);
   }
   Xil_L1DCacheFlush();
   while ((BE(fhb->fhb_ID) == FS_IDENTIFIER)) {// && (!f_eof(d->fd))) {
      if(d->fd>(FIL *)1)
      {
         if(f_eof(d->fd))
            break;
      }
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
         //           printf("Filesystem %d\n",i);
         //#define DOS7_HACK
#ifdef DOS7_HACK
         //FIXME HACK -> Skip DOS7 because it hangs z3660_scsi.device
#define DOS7 ((uint32_t)0x07534f44L)
         if (DOS7 == fhb->fhb_DosType) {
            DEBUG("[FSHD] File system %c%c%c/%d skipped.\n", dosID[0], dosID[1], dosID[2], dosID[3]);
            if (BE(fhb->fhb_Next) == 0xFFFFFFFF)
               goto fs_done;

            goto skip_fs_load_lseg;
         }
#endif
         if (filesystems[i].FS_ID == fhb->fhb_DosType) {
            DEBUG("[FSHD] File system %c%c%c/%d already loaded. Skipping.\n", dosID[0], dosID[1], dosID[2], dosID[3]);
            if (BE(fhb->fhb_Next) == 0xFFFFFFFF)
               goto fs_done;

            goto skip_fs_load_lseg;
         }
      }
#define ENABLE_LOAD_SEG
#ifdef ENABLE_LOAD_SEG
//      if (load_lseg(d->fshd_offs/d->block_size+1, &filesystems[piscsi_num_fs].binary_data, &filesystems[piscsi_num_fs].h_info, filesystems[piscsi_num_fs].relocs, d->block_size) != -1) {
      if (load_lseg(d, &filesystems[piscsi_num_fs].binary_data, &filesystems[piscsi_num_fs].h_info, filesystems[piscsi_num_fs].relocs, d->block_size) != -1) {
         filesystems[piscsi_num_fs].FS_ID = fhb->fhb_DosType;
         filesystems[piscsi_num_fs].fhb = fhb;
         printf("[FSHD] Loaded and set up file system %d: %c%c%c/%d\n", piscsi_num_fs + 1, dosID[0], dosID[1], dosID[2], dosID[3]);
         //            if(0) // enable or disable filesystem save
         {
            char fs_save_filename[256];
            memset(fs_save_filename, 0x00, 256);
            sprintf(fs_save_filename,DEFAULT_ROOT "data/fs/%c%c%c.%d", dosID[0], dosID[1], dosID[2], dosID[3]);
            FIL save_fs;
            check_exist(DEFAULT_ROOT "data");
            check_exist(DEFAULT_ROOT "data/fs");
            int ret = f_open(&save_fs,fs_save_filename, FA_READ|FA_OPEN_EXISTING);
            if (ret != FR_OK) {
               printf("[FSHD] File system %c%c%c/%d not found in fs storage.\n", dosID[0], dosID[1], dosID[2], dosID[3]);
               ret = f_open(&save_fs,fs_save_filename, FA_READ|FA_WRITE|FA_CREATE_NEW);
               if (ret == FR_OK) {
                  unsigned int n_bytes;
                  f_write(&save_fs,filesystems[piscsi_num_fs].binary_data, filesystems[piscsi_num_fs].h_info.byte_size, &n_bytes);
                  printf("[FSHD] File system pointer %08lX size %ld bytes written %ld.\n", (uint32_t)filesystems[piscsi_num_fs].binary_data, (uint32_t)filesystems[piscsi_num_fs].h_info.byte_size, (uint32_t)n_bytes);
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
      if(d->fd>(FIL *)1)
      {
         if((((FSIZE_t)BE(fhb->fhb_Next)) * d->block_size)>=d->fd->obj.objsize)
         {
            printf("lseek error!!!\n");
            goto fs_done;
         }
         f_lseek(d->fd, ((FSIZE_t)BE(fhb->fhb_Next)) * d->block_size);
         fhb_block = malloc(d->block_size);
         unsigned int n_bytes;
         f_read(d->fd, fhb_block, d->block_size,&n_bytes);
      }
      else
      {
         fhb_block = malloc(d->block_size);
         //DRESULT res = 
         disk_read (0,fhb_block,((uint64_t)BE(fhb->fhb_Next)) + d->start_block,1);
      }
      fhb = (struct FileSysHeaderBlock *)fhb_block;
      Xil_L1DCacheFlush();
      Xil_L2CacheFlush();
   }

   if (!fs_found) {
      DEBUG("[!!!FSHD] No file systems found on hard drive!\n");
   }

   fs_done:;
   /* They will be free on piscsi_refresh_drives()
   if (fhb_block)
    {
       DEBUG("Freeing fhb block=0x%08lX\n",(uint32_t)fhb_block);
        free(fhb_block);
    }
    */
}

PISCSI_DEV *piscsi_get_dev(uint8_t index) {
   return &devs[index];
}

FIL fd[8];

void piscsi_map_drive(char *filename, uint8_t index, uint64_t p0_Start, uint64_t p0_Len) {
   if (index > NUM_SCSI_UNITS_MAX) {
      if(index<8)
         printf("[PISCSI] Drive index %d out of range.\nUnable to map file %s to drive.\n", index, filename);
      else
         printf("[PISCSI] Drive index %d out of range.\n", index);
      return;
   }

   PISCSI_DEV *d = &devs[index];

   FIL *tmp_fd=&fd[index];
   if(index<8)
   {
      int ret = f_open(tmp_fd,filename, FA_READ|FA_WRITE|FA_OPEN_EXISTING);
      if (ret != FR_OK) {
         printf("[PISCSI] Failed to open file %s, could not map drive %d.\n", filename, index);
         return;
      }

      char hdfID[512];
      memset(hdfID, 0x00, 512);
      unsigned int n_bytes;
      f_read(tmp_fd, hdfID, 512,&n_bytes);
      Xil_L1DCacheFlush();
      hdfID[3] = '\0';
      if (strcmp(hdfID, "DOS") == 0 || strcmp(hdfID, "PFS") == 0 || strcmp(hdfID, "PDS") == 0 || strcmp(hdfID, "SFS") == 0) {
         printf("[!!!PISCSI] The disk image %s is a UAE Single Partition Hardfile!\n", filename);
         printf("[!!!PISCSI] WARNING: PiSCSI does NOT support UAE Single Partition Hardfiles!\n");
         printf("[!!!PISCSI] PLEASE check the PiSCSI readme file in the GitHub repo for more information.\n");
         printf("[!!!PISCSI] If this is merely an empty or placeholder file you've created to partition and format on the Amiga, please disregard this warning message.\n");
      }

      FSIZE_t file_size = f_size(tmp_fd);//lseek(tmp_fd, 0, SEEK_END);
      d->fs = file_size;
      d->fd = tmp_fd;
      f_lseek(d->fd, 0);
      printf("[PISCSI] Map %d: [%s] - %llu bytes.\n", index, filename, file_size);

      if (piscsi_parse_rdb(d) == -1) {
         printf("[PISCSI] No RDB found on disk, making up some CHS values.\n");
         d->h = 16;
         d->s = 63;
         d->c = (file_size / 512) / (d->s * d->h);
         d->block_size = 512;
      }
   }
   else
   {
      d->fs = p0_Len * 512;
      d->start_block = p0_Start;
      d->fd = (FIL *)1;
      printf("[PISCSI] Map %d: %llu bytes.\n", index, p0_Len*512);

      if (piscsi_parse_rdb(d) == -1) {
         printf("[PISCSI] No RDB found on disk, making up some CHS values.\n");
         d->h = 16;
         d->s = 63;
         d->c = p0_Len / (d->s * d->h);
         d->block_size = 512;
      }
   }
   DEBUG("[PISCSI] CHS: %ld %d %d\n", d->c, d->h, d->s);

   DEBUG("Finding partitions.\n");
   piscsi_find_partitions(d);
   DEBUG("Finding file systems.\n");
   piscsi_find_filesystems(d);
   DEBUG("Done file systems.\n");

   if(d->fd>(FIL *)1)
   {
      DEBUG("Enabling fast seek.\n");
      d->fd->cltbl = d->SeekTbl;         // Enable fast seek (set address of buffer)

      d->SeekTbl[0] = sizeof d->SeekTbl / sizeof d->SeekTbl[0];   // Buffer size
      int res = f_lseek(d->fd, CREATE_LINKMAP);   // Create link map table
      if (res == FR_OK) {
         DEBUG("[FASTSEEK %d] %lu clusters, ",index, (d->fd->obj.fs->n_fatent - 1) );
         DEBUG((d->SeekTbl[0] > 4) ? "fragmented in %ld.\n" : "contiguous.\n", d->SeekTbl[0] / 2 - 1);
         DEBUG("[FASTSEEK %d] %lu items used.\n",index, d->SeekTbl[0]);
      }
      if (res == FR_NOT_ENOUGH_CORE) {
         printf("[FASTSEEK %d] %lu items required to create the link map table.\n",index, d->SeekTbl[0]);
      }
   }
   DEBUG("Done partitions.\n");

}

void piscsi_unmap_drive(uint8_t index) {
   if (devs[index].fd != 0) {
      DEBUG("[PISCSI] Unmapped drive %d.\n", index);
      if(index<8)
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
      DEBUG_TRIVIAL("[PISCSI] IO Command %ld (%s)\n", piscsi_dbg[0], io_cmd_name(piscsi_dbg[0]));
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
         DEBUG_TRIVIAL("[RW10] scsiData: %.8lX\n", piscsi_dbg[0]);
         for (int i = 0; i < 10; i++) {
            data[i] = read8(piscsi_dbg[0] + i);
         }
         rwdat = (struct SCSICmd_RW10 *)data;
      }
      if (rwdat) {
         DEBUG_TRIVIAL("[RW10] CMD: %.2X\n", rwdat->opcode);
         DEBUG_TRIVIAL("[RW10] RDP: %.2X\n", rwdat->rdprotect_flags);
         DEBUG_TRIVIAL("[RW10] Block: %ld (%ld)\n", rwdat->block, BE(rwdat->block));
         DEBUG_TRIVIAL("[RW10] Res_Group: %.2X\n", rwdat->res_groupnum);
         DEBUG_TRIVIAL("[RW10] Len: %d (%d)\n", rwdat->len, BE16(rwdat->len));
      }
#endif
      break;
   case DBG_SCSI_DEBUG_MODESENSE_6:
      DEBUG_TRIVIAL("[PISCSI] SCSI ModeSense debug. Data: %.8lX\n", piscsi_dbg[0]);
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
   DEBUGME_SIMPLE(38, "[PISCSI-DEBUGME] AddDosNode\n");
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
      DEBUGME("[PISCSI-DEBUGME] OpenResource result: %ld (%08lX)\n", piscsi_u32_read[1], piscsi_u32_read[1]);
      break;
   case 32:
      DEBUGME("[PISCSI-DEBUGME] Could not open FileSystem.resource!\n");
      break;
   case 35:
      DEBUGME("[PISCSI-DEBUGME] stuff output\n");
      break;
   case 36:
      DEBUGME("[PISCSI-DEBUGME] Debug pointers: %.8lX %.8lX %.8lX %.8lX\n", piscsi_u32_read[0], piscsi_u32_read[1], piscsi_u32_read[2], piscsi_u32_read[3]);
      break;
   default:
      DEBUGME("[!!!PISCSI-DEBUGME] No debugme message for index %ld!\n", index);
      break;
   }
}

void handle_piscsi_reg_write(uint32_t addr, uint32_t val, uint8_t type) {
   ACTIVITY_LED_ON; // ON
   uint32_t map;
#ifndef PISCSI_DEBUG
   if (type) {}
#endif

   PISCSI_DEV *d = &devs[piscsi_cur_drive];

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
         DEBUG("[PISCSI-%ld] %ld byte READBYTES from block %ld to address %.8lX\n", val, piscsi_u32_read[1], piscsi_u32_read[0] / d->block_size, piscsi_u32_read[2]);
         uint32_t src = piscsi_u32_read[0];
         d->lba = (src / d->block_size);
         if(d->fd>(FIL *)1)
            f_lseek(d->fd, src);
      }
      else if (cmd == PISCSI_CMD_READ) {
         DEBUG("[PISCSI-%ld] %ld byte READ from block %ld to address %.8lX\n", val, piscsi_u32_read[1], piscsi_u32_read[0], piscsi_u32_read[2]);
         uint32_t src = piscsi_u32_read[0];
         d->lba = src;
         if(d->fd>(FIL *)1)
            f_lseek(d->fd, (((FSIZE_t)src) * d->block_size));
      }
      else {
         FSIZE_t src = piscsi_u32_read[3];
         src = (src << 32) | piscsi_u32_read[0];
         DEBUG("[PISCSI-%ld] %ld byte READ64 from block %lld to address %.8lX\n", val, piscsi_u32_read[1], (src / d->block_size), piscsi_u32_read[2]);
         d->lba = (src / d->block_size);
         if(d->fd>(FIL *)1)
            f_lseek(d->fd, src);
      }

      map = piscsi_u32_read[2];//get_mapped_data_pointer_by_address(cfg, piscsi_u32_read[2]);
      if ( (config.cpu_ram        && (map>=0x08000000) && (map<0x10000000))
            ||(config.autoconfig_ram && (map>=0x40000000) && (map<0x50000000))
      )
      {
         if(map>=0x40000000) map-=(0x40000000-0x20000000);
         DEBUG("[PISCSI-%ld] \"DMA\" Read goes to mapped range 0x%08lX.\n", val, map);
         used_dma=0;
         if(d->fd>(FIL *)1)
         {
            unsigned int n_bytes;
            f_read(d->fd, (uint8_t *)map, piscsi_u32_read[1],&n_bytes);
            DEBUG("            Bytes read %d\n",n_bytes);
            if(n_bytes!=piscsi_u32_read[1])
            {
               printf("SCSI ERROR!!! bytes_to_read=%ld, bytes_read=%d\n",piscsi_u32_read[1],n_bytes);
            }
         }
         else
         {
            //DRESULT res = 
            disk_read(0,(uint8_t *)map,d->lba + d->start_block,piscsi_u32_read[1]/ d->block_size);
         }
      } else {
         DEBUG("[PISCSI-%ld] No mapped range found for read.\n", val);
         DEBUG("Begin data read from disk: 0x%08lX to 0x%08lX\n",piscsi_u32_read[0],piscsi_u32_read[2]);
         if(piscsi_u32_read[1]>=0x180000)
            printf("ERROR SCSI read length>0x180000 (0x%08lX)\n",piscsi_u32_read[1]);
         uint8_t *buffer=(uint8_t *)SCSI_NO_DMA_ADDRESS;
         used_dma = piscsi_u32_read[2];
         if(d->fd>(FIL *)1)
         {
            unsigned int n_bytes;
            f_read(d->fd, buffer, piscsi_u32_read[1], &n_bytes);
            DEBUG("            Bytes read %d\n",n_bytes);
            if(n_bytes!=piscsi_u32_read[1])
            {
               printf("SCSI ERROR!!! bytes_to_read=%ld, bytes_read=%d\n",piscsi_u32_read[1],n_bytes);
            }
         }
         else
         {
            //DRESULT res = 
            disk_read(0,buffer,d->lba + d->start_block,piscsi_u32_read[1]/ d->block_size);
         }
      }
      Xil_L1DCacheFlush();
      break;
   case PISCSI_CMD_WRITE64:
   case PISCSI_CMD_WRITE:
   case PISCSI_CMD_WRITEBYTES:
      d = &devs[val];
      if(val != piscsi_cur_drive)
      {
         printf("[PISCSI] Warning val=%ld piscsi_cur_drive=%d\n",val,piscsi_cur_drive);
         printf("[PISCSI] Command cmd=%d\n",cmd);
      }
      if (d->fd == 0) {
         DEBUG("[!!!PISCSI] BUG: Attempted write to unmapped drive %ld.\n", val);
         break;
      }

      if (cmd == PISCSI_CMD_WRITEBYTES) {
         DEBUG("[PISCSI-%ld] %ld byte WRITEBYTES to block %ld from address %.8lX\n", val, piscsi_u32_write[1], piscsi_u32_write[0] / d->block_size, piscsi_u32_write[2]);
         uint32_t src = piscsi_u32_write[0];
         d->lba = (src / d->block_size);
         if(d->fd>(FIL *)1)
            f_lseek(d->fd, src);
      }
      else if (cmd == PISCSI_CMD_WRITE) {
         DEBUG("[PISCSI-%ld] %ld byte WRITE to block %ld from address %.8lX\n", val, piscsi_u32_write[1], piscsi_u32_write[0], piscsi_u32_write[2]);
         d->lba = piscsi_u32_write[0];
         if(d->fd>(FIL *)1)
         {
            FSIZE_t fpos=(((FSIZE_t)piscsi_u32_write[0]) * d->block_size);
            if(fpos>=d->fd->obj.objsize)
            {
               printf("Error on File Offset: %016llX (File size %016llX)\n",fpos,d->fd->obj.objsize);
            }
            f_lseek(d->fd, fpos);
         }
      }
      else {
         FSIZE_t src = piscsi_u32_write[3];
         src = (src << 32) | piscsi_u32_write[0];
         DEBUG("[PISCSI-%ld] %ld byte WRITE64 to block %lld from address %.8lX\n", val, piscsi_u32_write[1], (src / d->block_size), piscsi_u32_write[2]);
         d->lba = (src / d->block_size);
         if(d->fd>(FIL *)1)
            f_lseek(d->fd, src);
      }

      map = piscsi_u32_write[2];//get_mapped_data_pointer_by_address(cfg, piscsi_u32_write[2]);
      if ( ((map>=0x08000000) && (map<0x10000000) && config.cpu_ram)
            ||((map>=0x40000000) && (map<0x50000000) && config.autoconfig_ram)
      )
      {
         if(map>=0x40000000) map-=0x20000000;
         DEBUG("[PISCSI-%ld] \"DMA\" Write comes from mapped range 0x%08lX.\n", val, map);
         used_dma=0;
         if(d->fd>(FIL *)1)
         {
            unsigned int n_bytes;
            f_write(d->fd, (uint8_t *)map, piscsi_u32_write[1],&n_bytes);
            DEBUG("             Bytes written %d\n",n_bytes);
            if(n_bytes!=piscsi_u32_write[1])
            {
               printf("SCSI ERROR!!! bytes_to_write=%ld, bytes_written=%d\n",piscsi_u32_write[1],n_bytes);
            }
         }
         else
         {
            //DRESULT res = 
            disk_write(0,(uint8_t *)map,d->lba + d->start_block,piscsi_u32_write[1]/d->block_size);
         }
      }
      else {
         DEBUG("[PISCSI-%ld] No mapped range found for write.\n", val);
         DEBUG("          Begin data write to disk: 0x%08lX to 0x%08lX\n",piscsi_u32_write[0],piscsi_u32_write[2]);
         if(piscsi_u32_write[1]>=0x180000)
            printf("ERROR SCSI write length>0x180000 (0x%08lX)\n",piscsi_u32_write[1]);
         uint8_t *buffer=(uint8_t *)SCSI_NO_DMA_ADDRESS;
         used_dma = piscsi_u32_write[2];
         if(d->fd>(FIL *)1)
         {
            unsigned int n_bytes;
            f_write(d->fd, buffer, piscsi_u32_write[1], &n_bytes);
            DEBUG("             Bytes written %d\n",n_bytes);
            if(n_bytes!=piscsi_u32_write[1])
            {
               printf("SCSI ERROR!!! bytes_to_write=%ld, bytes_written=%d dma=0x%08lX\n",piscsi_u32_write[1],n_bytes,used_dma);
            }
         }
         else
         {
            //DRESULT res = 
            disk_write(0,buffer,d->lba + d->start_block,piscsi_u32_write[1]/d->block_size);
         }
      }
      Xil_L1DCacheFlush();
      break;
   case PISCSI_CMD_READ_ADDR1:
   case PISCSI_CMD_READ_ADDR2:
   case PISCSI_CMD_READ_ADDR3:
   case PISCSI_CMD_READ_ADDR4: {
      int i = (addr - PISCSI_CMD_READ_ADDR1) / 4;
      piscsi_u32_read[i] = val;
      break;
   }
   case PISCSI_CMD_WRITE_ADDR1:
   case PISCSI_CMD_WRITE_ADDR2:
   case PISCSI_CMD_WRITE_ADDR3:
   case PISCSI_CMD_WRITE_ADDR4: {
      int i = (addr - PISCSI_CMD_WRITE_ADDR1) / 4;
      piscsi_u32_write[i] = val;
      break;
   }
   case PISCSI_CMD_DRVNUM:
      if (val > 16) {
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
      //            r = 0;//get_mapped_item_by_address(cfg, val);
      if ( ((val>=0x08000000) && (val<0x10000000) && config.cpu_ram)
            ||((val>=0x40000000) && (val<0x50000000) && config.autoconfig_ram)
      )
      {
         if(val>=0x40000000) val-=0x20000000;
         //            if (r != -1) {
         uint32_t addr = val;// - cfg->map_offset[r];
         uint8_t *dst_data = 0;//cfg->map_data[r];
         uint8_t cur_partition = 0;
         MEMCPY(dst_data + addr, piscsi_rom_ptr + PISCSI_DRIVER_OFFSET, BOOT_ROM_SIZE - PISCSI_DRIVER_OFFSET);

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
         for (int i = 0; i < NUM_SCSI_UNITS_MAX; i++) {
            if (devs[i].fd == 0)
               goto skip_disk;

            if (devs[i].num_partitions) {
               uint32_t p_offs = addr2;
               DEBUG("[PISCSI] Adding %ld partitions for unit %d\n", devs[i].num_partitions, i);
               for (uint32_t j = 0; j < devs[i].num_partitions; j++) {
                  DEBUG("Partition %ld: %s\n", j, devs[i].pb[j]->pb_DriveName + 1);
                  sprintf((char *)dst_data + p_offs, "%s", devs[i].pb[j]->pb_DriveName + 1);
                  devs[i].pb_addres[j]=(uint32_t)(dst_data + p_offs);
                  p_offs += 0x20;
                  PUTNODELONG(addr2);// + cfg->map_offset[r]);
                  PUTNODELONG(data_addr);// + cfg->map_offset[r]);
                  PUTNODELONG(i);
                  PUTNODELONG(0);
                  uint32_t nodesize = (be32toh(devs[i].pb[j]->pb_Environment[0]) + 1) * 4;
                  MEMCPY(dst_data + p_offs, devs[i].pb[j]->pb_Environment, nodesize);

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
         Xil_L1DCacheFlush();
         Xil_L2CacheFlush();
      }
      else
      {
         printf("[PISCSI] DRIVER Address 0x%08lX not mapped in FPGA RAM...\n",val);
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
   case PISCSI_CMD_COPYFS: {
      uint32_t addr=val;
      if ( ((addr>=0x08000000) && (addr<0x10000000) && config.cpu_ram)
            ||((addr>=0x40000000) && (addr<0x50000000) && config.autoconfig_ram)
      )
      {
         if(addr>=0x40000000) addr-=0x20000000;
         printf("[PISCSI] Copy file system %ld to %.8lX and reloc.\n", rom_cur_fs + 1, addr);
         //            r = 0;//get_mapped_item_by_address(cfg, addr);
         //            if (r != -1) {
         MEMCPY((uint8_t *)addr, filesystems[rom_cur_fs].binary_data, filesystems[rom_cur_fs].h_info.byte_size);
         filesystems[rom_cur_fs].h_info.base_offset = addr;
         reloc_hunks(filesystems[rom_cur_fs].relocs, (uint8_t*) addr, &filesystems[rom_cur_fs].h_info);
         filesystems[rom_cur_fs].handler = addr;
         Xil_L1DCacheFlush();
         Xil_L2CacheFlush();
         //            }
      }
      else
      {
         printf("[PISCSI] COPYFS Address 0x%08lX not mapped in FPGA RAM...\n",addr);
      }
   }
   break;
   case PISCSI_CMD_SETFSH: {
      int i = 0;
      DEBUG("[PISCSI] Set handler for partition %ld (DeviceNode: %.8lX)\n", rom_cur_partition, val);
      uint32_t addr = val;// - cfg->map_offset[r];
      if ( ((addr>=0x08000000) && (addr<0x10000000) && config.cpu_ram)
            ||((addr>=0x40000000) && (addr<0x50000000) && config.autoconfig_ram)
      )
      {
         if(addr>=0x40000000) addr-=0x20000000;
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
         Xil_L1DCacheFlush();
         Xil_L2CacheFlush();
         break;
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
      else
      {
         printf("[PISCSI] SETFSH Address 0x%08lX not mapped in FPGA RAM...\n",val);
      }
      break;
   }
   case PISCSI_CMD_LOADFS: {
      DEBUG("[PISCSI] Attempt to load file system for partition %ld from disk.\n", rom_cur_partition);
      printf("->>>>>>>>>>>>>>>>>>> a1 %08lX\n",val);
      uint32_t addr = val;
      if ( ((addr>=0x08000000) && (addr<0x10000000) && config.cpu_ram)
            ||((addr>=0x40000000) && (addr<0x50000000) && config.autoconfig_ram)
      )
      {
         if(addr>=0x40000000) addr-=0x20000000;
         char *dosID = (char *)&rom_partition_dostype[rom_cur_partition];
         filesystems[piscsi_num_fs].binary_data = NULL;
         filesystems[piscsi_num_fs].fhb = NULL;
         filesystems[piscsi_num_fs].FS_ID = rom_partition_dostype[rom_cur_partition];
         filesystems[piscsi_num_fs].handler = 0;
         if (load_fs(&filesystems[piscsi_num_fs], dosID) != -1) {
            printf("[FSHD-Late] Loaded file system %c%c%c/%d from fs storage.\n", dosID[0], dosID[1], dosID[2], dosID[3]);
            piscsi_u32_read[3] = piscsi_num_fs;
            rom_cur_fs = piscsi_num_fs;
            piscsi_num_fs++;
         } else {
            printf("[FSHD-Late] Failed to load file system %c%c%c/%d from fs storage.\n", dosID[0], dosID[1], dosID[2], dosID[3]);
            piscsi_u32_read[3] = 0xFFFFFFFF;
         }
      }
      else
      {
         printf("[PISCSI] LOADFS Address 0x%08lX not mapped in FPGA RAM...\n",val);
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
   case PISCSI_CMD_EXPBASE: {
      if(val>=0x08000000 && val<0x10000000)
      {
         char *str=(char*)val+1;
         char drivename[20];
         int i;
         for(i=0;i<strlen(str);i++)
         {
            drivename[i]=str[i];
            if(drivename[i]>='a' && drivename[i]<='z')
               drivename[i]+='A'-'a';
         }
         drivename[i]=0;
         printf("%s 0x%08lX\n",drivename,val);
         for(int k=0;k<NUM_SCSI_UNITS_MAX;k++)
         {
            PISCSI_DEV *d = &devs[k];
            for (int i = 0; i < d->num_partitions; i++) {
               char tempname[20];
               strcpy(tempname,(char *)(d->pb[i]->pb_DriveName + 1));
               int j;
               for(j=0;j<strlen(tempname);j++)
               {
                  if(tempname[j]>='a' && tempname[j]<='z')
                     tempname[j]+='A'-'a';
               }
               if (strcmp(drivename, tempname) == 0) {
                  if(tempname[strlen(tempname)-3]=='.')
                  {
                     int current=tempname[strlen(tempname)-1]-'0';
                     current+=(tempname[strlen(tempname)-2]-'0')*10;
                     tempname[strlen(tempname)-3]=0;
                     printf("[PISCSI] Duplicate partition name %s. Temporarily renaming to %s.%d\n", d->pb[i]->pb_DriveName + 1, tempname, current + 1);
                     current++;
                     sprintf((char *)d->pb[i]->pb_DriveName + 1 + d->pb[i]->pb_DriveName[0]-2, ".%d", current);
                     if (current > 99)
                        d->pb[i]->pb_DriveName[0]++;
                     sprintf((char *)d->pb_addres[i], "%s", d->pb[i]->pb_DriveName + 1);
                  }
                  else if(tempname[strlen(tempname)-2]=='.')
                  {
                     int current=tempname[strlen(tempname)-1]-'0';
                     tempname[strlen(tempname)-2]=0;
                     printf("[PISCSI] Duplicate partition name %s. Temporarily renaming to %s.%d\n", d->pb[i]->pb_DriveName + 1, tempname, current + 1);
                     current++;
                     sprintf((char *)d->pb[i]->pb_DriveName + 1 + d->pb[i]->pb_DriveName[0]-2, ".%d", current);
                     if (current > 9)
                        d->pb[i]->pb_DriveName[0]++;
                     sprintf((char *)d->pb_addres[i], "%s", d->pb[i]->pb_DriveName + 1);
                  }
                  else
                  {
                     printf("[PISCSI] Duplicate partition name %s. Temporarily renaming to %s.%d\n", d->pb[i]->pb_DriveName + 1, d->pb[i]->pb_DriveName + 1, 1);
                     sprintf((char *)d->pb[i]->pb_DriveName + 1 + d->pb[i]->pb_DriveName[0], ".%d", 1);
                     d->pb[i]->pb_DriveName[0] += 2;
                     sprintf((char *)d->pb_addres[i], "%s", d->pb[i]->pb_DriveName + 1);
                  }
                  goto partition_renamed2;
               }
            }
         }
      }
      else
      {
         printf("[PISCSI] Error Mountlist device at 0x%08lX !!!\n",val);
      }
      partition_renamed2:
      break;
   }

   default:
      //            DEBUG("[!!!PISCSI] WARN: Unhandled %s register write to %.8lX: %ld\n", op_type_names[type], addr, val);
      printf("[!!!PISCSI] WARN: Unhandled register write to %.8lX: %ld\n", addr, val);
      break;
   }
   ACTIVITY_LED_OFF; // OFF
}

#define PIB 0x00

uint32_t handle_piscsi_read(uint32_t addr, uint8_t type) {
   ACTIVITY_LED_ON; // ON
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
         ACTIVITY_LED_OFF; // OFF
         return v;
      }
      ACTIVITY_LED_OFF; // OFF
      return 0;
   }

   switch (addr) {
   case PISCSI_CMD_READ_ADDR1:
   case PISCSI_CMD_READ_ADDR2:
   case PISCSI_CMD_READ_ADDR3:
   case PISCSI_CMD_READ_ADDR4: {
      int i = (addr - PISCSI_CMD_READ_ADDR1) / 4;
      ACTIVITY_LED_OFF; // OFF
      return piscsi_u32_read[i];
      break;
   }
   case PISCSI_CMD_WRITE_ADDR1:
   case PISCSI_CMD_WRITE_ADDR2:
   case PISCSI_CMD_WRITE_ADDR3:
   case PISCSI_CMD_WRITE_ADDR4: {
      int i = (addr - PISCSI_CMD_WRITE_ADDR1) / 4;
      ACTIVITY_LED_OFF; // OFF
      return piscsi_u32_write[i];
      break;
   }
   case PISCSI_CMD_DRVTYPE:
      if (devs[piscsi_cur_drive].fd == 0) {
         DEBUG("[PISCSI] %s Read from DRVTYPE %d, drive not attached.\n", op_type_names[type], piscsi_cur_drive);
         ACTIVITY_LED_OFF; // OFF
         return 0;
      }
      DEBUG("[PISCSI] %s Read from DRVTYPE %d, drive attached.\n", op_type_names[type], piscsi_cur_drive);
      ACTIVITY_LED_OFF; // OFF
      return 1;
      break;
   case PISCSI_CMD_DRVNUM:
      ACTIVITY_LED_OFF; // OFF
      return piscsi_cur_drive;
      break;
   case PISCSI_CMD_CYLS:
      DEBUG("[PISCSI] %s Read from CYLS %d: %ld\n", op_type_names[type], piscsi_cur_drive, devs[piscsi_cur_drive].c);
      ACTIVITY_LED_OFF; // OFF
      return devs[piscsi_cur_drive].c;
      break;
   case PISCSI_CMD_HEADS:
      DEBUG("[PISCSI] %s Read from HEADS %d: %d\n", op_type_names[type], piscsi_cur_drive, devs[piscsi_cur_drive].h);
      ACTIVITY_LED_OFF; // OFF
      return devs[piscsi_cur_drive].h;
      break;
   case PISCSI_CMD_SECS:
      DEBUG("[PISCSI] %s Read from SECS %d: %d\n", op_type_names[type], piscsi_cur_drive, devs[piscsi_cur_drive].s);
      ACTIVITY_LED_OFF; // OFF
      return devs[piscsi_cur_drive].s;
      break;
   case PISCSI_CMD_BLOCKS: {
      uint32_t blox = devs[piscsi_cur_drive].fs / devs[piscsi_cur_drive].block_size;
      DEBUG("[PISCSI] %s Read from BLOCKS %d: %ld\n", op_type_names[type], piscsi_cur_drive, blox);
      DEBUG("filesize: %lld (%lld blocks*block_size)\n", devs[piscsi_cur_drive].fs, ((FSIZE_t)blox)*devs[piscsi_cur_drive].block_size);
      ACTIVITY_LED_OFF; // OFF
      return blox;
      break;
   }

   case PISCSI_CMD_BLOCKS0:
   case PISCSI_CMD_BLOCKS1:
   case PISCSI_CMD_BLOCKS2:
   case PISCSI_CMD_BLOCKS3:
   case PISCSI_CMD_BLOCKS4:
   case PISCSI_CMD_BLOCKS5:
   case PISCSI_CMD_BLOCKS6:
   case PISCSI_CMD_BLOCKS7: {
      piscsi_cur_drive=(addr-PISCSI_CMD_BLOCKS0) / 4;
      uint32_t blox = devs[piscsi_cur_drive].fs / devs[piscsi_cur_drive].block_size;
      DEBUG("[PISCSI] %s Read from BLOCKS %d: %ld\n", op_type_names[type], piscsi_cur_drive, blox);
      DEBUG("filesize: %lld (%lld blocks*block_size)\n", devs[piscsi_cur_drive].fs, ((FSIZE_t)blox)*devs[piscsi_cur_drive].block_size);
      ACTIVITY_LED_OFF; // OFF
      return blox;
   }

   case PISCSI_CMD_BLOCKS10:
   case PISCSI_CMD_BLOCKS11:
   case PISCSI_CMD_BLOCKS12:
   case PISCSI_CMD_BLOCKS13:
   case PISCSI_CMD_BLOCKS14:
   case PISCSI_CMD_BLOCKS15:
   case PISCSI_CMD_BLOCKS16:
   case PISCSI_CMD_BLOCKS17: {
      piscsi_cur_drive=10+(addr-PISCSI_CMD_BLOCKS10) / 4;
      uint32_t blox = devs[piscsi_cur_drive].fs / devs[piscsi_cur_drive].block_size;
      DEBUG("[PISCSI] %s Read from BLOCKS %d: %ld\n", op_type_names[type], piscsi_cur_drive, blox);
      DEBUG("filesize: %lld (%lld blocks*block_size)\n", devs[piscsi_cur_drive].fs, ((FSIZE_t)blox)*devs[piscsi_cur_drive].block_size);
      ACTIVITY_LED_OFF; // OFF
      return blox;
   }
   case PISCSI_CMD_GETPART: {
      DEBUG("[PISCSI] Get ROM partition %ld offset: %.8lX\n", rom_cur_partition, rom_partitions[rom_cur_partition]);
      ACTIVITY_LED_OFF; // OFF
      return rom_partitions[rom_cur_partition];
      break;
   }
   case PISCSI_CMD_GETPRIO:
      DEBUG("[PISCSI] Get partition %ld boot priority: %ld\n", rom_cur_partition, rom_partition_prio[rom_cur_partition]);
      ACTIVITY_LED_OFF; // OFF
      return rom_partition_prio[rom_cur_partition];
      break;
   case PISCSI_CMD_GETFS:
      DEBUG("[PISCSI] Get current loaded file system: %.8lX\n", filesystems[rom_cur_fs].FS_ID);
      ACTIVITY_LED_OFF; // OFF
      return filesystems[rom_cur_fs].FS_ID;
   case PISCSI_CMD_FSSIZE:
      DEBUG("[PISCSI] Get alloc size of loaded file system: %ld\n", filesystems[rom_cur_fs].h_info.alloc_size);
      ACTIVITY_LED_OFF; // OFF
      return filesystems[rom_cur_fs].h_info.alloc_size;
   case PISCSI_CMD_BLOCKSIZE:
      DEBUG("[PISCSI] Get block size of drive %d: %ld\n", piscsi_cur_drive, devs[piscsi_cur_drive].block_size);
      ACTIVITY_LED_OFF; // OFF
      return devs[piscsi_cur_drive].block_size;
   case PISCSI_CMD_BLOCKSIZE0:
   case PISCSI_CMD_BLOCKSIZE1:
   case PISCSI_CMD_BLOCKSIZE2:
   case PISCSI_CMD_BLOCKSIZE3:
   case PISCSI_CMD_BLOCKSIZE4:
   case PISCSI_CMD_BLOCKSIZE5:
   case PISCSI_CMD_BLOCKSIZE6:
   case PISCSI_CMD_BLOCKSIZE7:
      piscsi_cur_drive=(addr-PISCSI_CMD_BLOCKSIZE0) / 4;
      DEBUG("[PISCSI] Get block size of drive %d: %ld\n", piscsi_cur_drive, devs[piscsi_cur_drive].block_size);
      ACTIVITY_LED_OFF; // OFF
      return devs[piscsi_cur_drive].block_size;
   case PISCSI_CMD_BLOCKSIZE10:
   case PISCSI_CMD_BLOCKSIZE11:
   case PISCSI_CMD_BLOCKSIZE12:
   case PISCSI_CMD_BLOCKSIZE13:
   case PISCSI_CMD_BLOCKSIZE14:
   case PISCSI_CMD_BLOCKSIZE15:
   case PISCSI_CMD_BLOCKSIZE16:
   case PISCSI_CMD_BLOCKSIZE17:
      piscsi_cur_drive=10+(addr-PISCSI_CMD_BLOCKSIZE10) / 4;
      DEBUG("[PISCSI] Get block size of drive %d: %ld\n", piscsi_cur_drive, devs[piscsi_cur_drive].block_size);
      ACTIVITY_LED_OFF; // OFF
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
               ACTIVITY_LED_OFF; // OFF
               return 0;
            }
         }
      }
      ACTIVITY_LED_OFF; // OFF
      return 1;
   }
   case PISCSI_CMD_USED_DMA: {
      ACTIVITY_LED_OFF; // OFF
      uint32_t temp=used_dma;
      //           DEBUG("Read used_dma=%08lX\n",used_dma);
      used_dma=0;
      return(temp);
   }
   default:
      DEBUG("[!!!PISCSI] WARN: Unhandled %s register read from %.8lX\n", op_type_names[type], addr);
      break;
   }

   ACTIVITY_LED_OFF; // OFF
   return 0;
}
