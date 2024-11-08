#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hunk-reloc.h"
#include "xpseudo_asm_gcc.h"
#include "z3660_scsi_enums.h"
#include "scsi.h"
#include <ff.h>
#include <diskio.h>

#ifdef FAKESTORM
#define lseek64 lseek
#endif

#define DEBUG_SPAMMY(...)
#define DEBUG(...)
#define DEBUGME(...)
//#define DEBUGME printf
//#define DEBUG_SPAMMY printf
//#define DEBUG printf

#define BE(val) be32toh(val)
#define BE16(val) be16toh(val)

#define READLW(a, b) f_read(b, &a, 4, &n_bytes); a = be32toh(a);
#define READW(a, b) f_read(b, &a, 2, &n_bytes); a = be16toh(a);

uint32_t lw = 0;
static uint32_t file_offset = 0, add_size = 0;

char *hunk_id_name(uint32_t index) {
   switch (index) {
   case HUNKTYPE_HEADER:
      return "HUNK_HEADER";
   case HUNKTYPE_CODE:
      return "HUNK_CODE";
   case HUNKTYPE_HUNK_RELOC32:
      return "HUNK_RELOC32";
   case HUNKTYPE_SYMBOL:
      return "HUNK_SYMBOL";
   case HUNKTYPE_BSS:
      return "HUNK_BSS";
   case HUNKTYPE_DATA:
      return "HUNK_DATA";
   case HUNKTYPE_END:
      return "HUNK_END";
   default:
      return "UNKNOWN HUNK TYPE";
   }
}

int process_hunk(uint32_t index, HUNK_INFO *info, FIL *f, HUNK_RELOC *hreloc) {
   if (!f)
      return -1;

   uint32_t discard = 0, cur_hunk = 0, offs32 = 0;
   unsigned int n_bytes;
   switch (index) {
   case HUNKTYPE_HEADER:
      DEBUG("[HUNK_RELOC] Processing hunk HEADER.\n");
      do {
         READLW(discard, f);
         if (discard) {
            if (info->libnames[info->num_libs]) {
               free(info->libnames[info->num_libs]);
               info->libnames[info->num_libs] = NULL;
            }
            info->libnames[info->num_libs] = malloc(discard * 4);
            f_read(f,info->libnames[info->num_libs], discard * 4, &n_bytes);
            info->num_libs++;
         }
      } while (discard && !f_eof(f));

      READLW(info->table_size, f);
      DEBUG("[HUNK_RELOC] [HEADER] Table size: %ld\n", info->table_size);
      READLW(info->first_hunk, f);
      READLW(info->last_hunk, f);
      info->num_hunks = (info->last_hunk - info->first_hunk) + 1;
      DEBUG("[HUNK_RELOC] [HEADER] First: %ld Last: %ld Num: %ld\n", info->first_hunk, info->last_hunk, info->num_hunks);
      if (info->hunk_sizes) {
         free(info->hunk_sizes);
         info->hunk_sizes = NULL;
      }
      if (info->hunk_offsets) {
         free(info->hunk_offsets);
         info->hunk_offsets = NULL;
      }
      dsb();
      dmb();
      info->hunk_sizes = malloc(info->num_hunks * 4);
      info->hunk_offsets = malloc(info->num_hunks * 4);
      for (uint32_t i = 0; i < info->table_size; i++) {
         READLW(info->hunk_sizes[i], f);
         DEBUG("[HUNK_RELOC] [HEADER] Hunk %ld: size %ld (%.8lX)\n", i, info->hunk_sizes[i] * 4, info->hunk_sizes[i] * 4);
      }
      info->header_size = (uint32_t)f_tell(f) - file_offset;
      DEBUG("[HUNK_RELOC] [HEADER] ~~~~~~~~~~~ Hunk HEADER size is %ld ~~~~~~~~~~~~.\n", info->header_size);
      return 0;
      break;
   case HUNKTYPE_CODE:
      DEBUG("[HUNK_RELOC] Hunk %d: CODE.\n", info->current_hunk);
      READLW(discard, f);
      info->hunk_offsets[info->current_hunk] = f_tell(f) - file_offset;
      //            discard+=123;
      DEBUG("[HUNK_RELOC] [CODE] Code hunk size: %ld (%.8lX)\n", discard * 4, discard * 4);
      if((((FSIZE_t)discard) * 4 + f->fptr) >= f_size(f))
         printf("Error in Code hunk size\n");
      f_lseek(f, ((FSIZE_t)discard) * 4 + f->fptr);
      return 0;
      break;
   case HUNKTYPE_DATA:
      DEBUG("[HUNK_RELOC] Hunk %d: DATA.\n", info->current_hunk);
      READLW(discard, f);
      info->hunk_offsets[info->current_hunk] = f_tell(f) - file_offset;
      DEBUG("[HUNK_RELOC] [CODE] Data hunk size: %ld (%.8lX)\n", discard * 4, discard * 4);
      if((((FSIZE_t)discard) * 4 + f->fptr) >= f_size(f))
         printf("Error in Data hunk size\n");
      f_lseek(f, ((FSIZE_t)discard) * 4 + f->fptr);
      return 0;
      break;
   case HUNKTYPE_HUNK_RELOC32:
      DEBUG("[HUNK_RELOC] Hunk %d: RELOC32.\n", info->current_hunk);
      DEBUG("Processing Reloc32 hunk.\n");
      do {
         READLW(discard, f);
         if (discard && discard != 0xFFFFFFFF) {
            READLW(cur_hunk, f);
            DEBUG("[HUNK_RELOC] [RELOC32] Relocating %ld offsets pointing to hunk %ld.\n", discard, cur_hunk);
            if(discard>4096)
            {
               printf("Warning!!!!! Relocating %ld offsets > 4096 pointing to hunk %ld.\n",discard, cur_hunk);
            }
            for(uint32_t i = 0; i < discard; i++) {
               READLW(offs32, f);
               DEBUG_SPAMMY("[HUNK_RELOC] [RELOC32] #%d: @%.8X in hunk %d\n", i + 1, offs32, cur_hunk);
               hreloc[info->reloc_hunks].offset = offs32;
               hreloc[info->reloc_hunks].src_hunk = info->current_hunk;
               hreloc[info->reloc_hunks].target_hunk = cur_hunk;
               info->reloc_hunks++;
            }
         }
      } while(discard && !f_eof(f));
      return 0;
      break;
   case HUNKTYPE_SYMBOL:
      DEBUG("[HUNK_RELOC] Hunk %d: SYMBOL.\n", info->current_hunk);
      DEBUG("[HUNK_RELOC] [SYMBOL] Processing Symbol hunk.\n");
      READLW(discard, f);
      do {
         if (discard) {
            char sstr[256];
            memset(sstr, 0x00, 256);
            f_read(f,sstr, discard * 4, &n_bytes);
            READLW(discard, f);
            DEBUG("[HUNK_RELOC] [SYMBOL] Symbol: %s - %.8lX\n", sstr, discard);
         }
         READLW(discard, f);
      } while (discard && !f_eof(f));
      return 0;
      break;
   case HUNKTYPE_BSS:
      DEBUG("[HUNK_RELOC] Hunk %d: BSS.\n", info->current_hunk);
      READLW(discard, f);
      info->hunk_offsets[info->current_hunk] = f_tell(f) - file_offset;
      DEBUG("[HUNK_RELOC] [BSS] Skipping BSS hunk. Size: %ld\n", discard * 4);
      add_size += (discard * 4);
      return 0;
      break;
   case HUNKTYPE_END:
      DEBUG("[HUNK_RELOC] END: Ending hunk %d.\n", info->current_hunk);
      info->current_hunk++;
      return 0;
      break;
   default:
      DEBUG("[!!!HUNK_RELOC] Unknown hunk type %.8lX! Can't process!\n", index);
      break;
   }

   return -1;
}

void reloc_hunk(HUNK_RELOC *h, uint8_t *buf, HUNK_INFO *i) {
   uint32_t rel = i->hunk_offsets[h->target_hunk];
   uint32_t *src_ptr = (uint32_t *)(&buf[i->hunk_offsets[h->src_hunk] + h->offset]);

   uint32_t src = be32toh(*src_ptr);
   uint32_t dst = src + i->base_offset + rel;
   DEBUG("[HUNK-RELOC] %.8lX -> %.8lX\n", src, dst);
   *src_ptr = htobe32(dst);
}

void process_hunks(FIL *in, HUNK_INFO *h_info, HUNK_RELOC *r, uint32_t offset) {
   unsigned int n_bytes;
   READLW(lw, in);
   DEBUG_SPAMMY("Hunk ID: %.8X (%s)\n", lw, hunk_id_name(lw));

   file_offset = offset;
   add_size = 0;
   while (!f_eof(in) && process_hunk(lw, h_info, in, r) != -1) {
      READLW(lw, in);
      if (f_eof(in)) goto end_parse;
      DEBUG("Hunk ID: %.8lX (%s)\n", lw, hunk_id_name(lw));
      DEBUG("File pos: %.8llX\n", f_tell(in));// - file_offset);
   }
   end_parse:;
   DEBUG("Done processing hunks.\n");
}

void reloc_hunks(HUNK_RELOC *r, uint8_t *buf, HUNK_INFO *h_info) {
   DEBUG("[HUNK-RELOC] Relocating %ld offsets.\n", h_info->reloc_hunks);
   for (uint32_t i = 0; i < h_info->reloc_hunks; i++) {
      DEBUG("[HUNK-RELOC] Relocating offset %ld.\n", i);
      reloc_hunk(&r[i], buf, h_info);
   }
   DEBUG("[HUNK-RELOC] Done relocating offsets.\n");
}

struct LoadSegBlock {
   uint32_t   lsb_ID;
   uint32_t   lsb_SummedLongs;
   int32_t    lsb_ChkSum;
   uint32_t   lsb_HostID;
   uint32_t   lsb_Next;
   uint32_t   lsb_LoadData[PISCSI_MAX_BLOCK_SIZE / 4];
};
#define	LOADSEG_IDENTIFIER 0x4C534547
void check_exist(char *path);

int load_lseg(PISCSI_DEV *d, uint8_t **buf_p, HUNK_INFO *i, HUNK_RELOC *relocs, uint32_t block_size) {
   if (d->fd == 0)
      return -1;

   if (block_size == 0)
      block_size = 512;

   uint8_t *block = malloc(block_size);
   uint32_t next_blk = 0;
   struct LoadSegBlock *lsb = (struct LoadSegBlock *)block;
   unsigned int n_w_bytes;
   unsigned int n_r_bytes;
   if(d->fd>(FIL *)1)
   {
      f_read(d->fd, block, block_size,&n_r_bytes);
   }
   else
   {
      //DRESULT res =
      disk_read(0,block,d->fshd_offs/d->block_size+1 + d->start_block,1);
   }
   if (BE(lsb->lsb_ID) != LOADSEG_IDENTIFIER) {
      DEBUG("[LOAD_LSEG] Attempted to load a non LSEG-block: %.8lX\n", BE(lsb->lsb_ID));
      goto fail;
   }
   check_exist(DEFAULT_ROOT "data");

   char *filename = DEFAULT_ROOT "data/lsegout.bin";
   FIL out;
   f_open(&out,filename, FA_READ|FA_WRITE|FA_CREATE_ALWAYS);
   //    f_open(&out,filename, FA_READ|FA_WRITE|FA_OPEN_APPEND);
   uint32_t totalbytes=0;
   do {
      DEBUG("[LOAD_LSEG] LSEG ID:%08lX (4C534547=LSEG)\n",BE(lsb->lsb_ID));
      DEBUG("[LOAD_LSEG] Longs: %ld HostID: %ld\n", BE(lsb->lsb_SummedLongs), BE(lsb->lsb_HostID));
      DEBUG("[LOAD_LSEG] Next: %ld LoadData: %p\n", BE(lsb->lsb_Next), (void *)lsb->lsb_LoadData);
      //    next_blk = BE(lsb->lsb_Next);
      f_write(&out,lsb->lsb_LoadData, block_size - 20, &n_w_bytes);
      if(n_w_bytes!=block_size - 20)
         printf("[LSEG] Error write n_w_bytes=%d != %ld (%s)\n",n_w_bytes,block_size - 20,filename);
      totalbytes+=n_w_bytes;
      next_blk = BE(lsb->lsb_Next);
      if(next_blk == 0xFFFFFFFF)
         break;
      if(d->fd>(FIL *)1)
      {
         if((((FSIZE_t)next_blk) * block_size) >= f_size(d->fd))
            printf("Error in LSEG data hunk size\n");
         f_lseek(d->fd, ((FSIZE_t)next_blk) * block_size);
         f_read(d->fd, block, block_size,&n_r_bytes);
         if(f_eof(d->fd))
            break;
      }
      else
      {
         //DRESULT res =
         disk_read(0,block,next_blk + d->start_block,1);
      }
   } while (next_blk != 0xFFFFFFFF);

   uint32_t file_size = f_tell(&out);
   DEBUG("lsegout.bin file size %ld %ld\n",file_size,totalbytes);
   f_lseek(&out, 0);
   uint8_t *buf = malloc(file_size + 1024);
   unsigned int n_bytes;
   f_read(&out,buf, file_size, &n_bytes);

   f_lseek(&out, 0);
   process_hunks(&out, i, relocs, 0x0);
   f_close(&out);
   *buf_p = buf;
   i->byte_size = file_size;
   i->alloc_size = file_size + add_size;

   return 0;

   fail:;
   if (block)
      free(block);

   return -1;
}

int load_fs(PISCSI_FS *fs, char *dosID) {
   char filename[256];
   memset(filename, 0x00, 256);
   sprintf(filename, DEFAULT_ROOT "data/fs/%c%c%c.%d", dosID[0], dosID[1], dosID[2], dosID[3]);

   FIL in;
   int ret = f_open(&in,filename, FA_READ);
   if (ret != FR_OK)
      return -1;

   //    fseek(in, 0, SEEK_END);
   uint32_t file_size = f_size(&in);//f_tell(in);
   f_lseek(&in, 0);

   fs->binary_data = malloc(file_size);
   unsigned int n_bytes;
   f_read(&in,fs->binary_data, file_size, &n_bytes);
   f_lseek(&in, 0);
   process_hunks(&in, &fs->h_info, fs->relocs, 0x0);
   fs->h_info.byte_size = file_size;
   fs->h_info.alloc_size = file_size + add_size;

   f_close(&in);
   return 0;
}
