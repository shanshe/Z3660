// SD-card file manager for the boot-time serial console.
//
// A small "SD>" command loop reachable from the boot menu (key 'E'). It works
// on both mounted volumes, 0: (FAT boot) and 1: (exFAT data, hdf images live
// under 1:/hdf/), and always expects an explicit volume prefix in paths.
//
// The COPY command is a generalized, streamed version of the fixed BOOT.bin /
// FAILSAFE.bin / Z3660.bin copy shortcuts in mobotest.c: it copies arbitrary
// files of any size through a chunk buffer (so multi-GB hdf images work), shows
// progress, verifies read-total against write-total, asks before overwriting an
// existing destination and can be aborted with ESC between chunks.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "ff.h"
#include "sd_fileops.h"

// Provided by mobotest.c (serial/Amiga keyboard poll) and main.c (SD mount
// helpers). Declared here so this module stays self-contained, mirroring the
// extern of read_keyboard() already used inside mobotest.c.
extern uint8_t read_keyboard(uint8_t *data, int enable_mouse);
extern FRESULT f_clk_mount(FATFS* fs, const TCHAR* path, BYTE opt);
extern FRESULT f_umount(const TCHAR* path);

// Bytes per sector on the SD card (FF_MIN_SS == FF_MAX_SS == 512).
#define SD_SECTOR_SIZE 512

// Human-readable FatFs result code, so errors are reported plainly.
static const char *fr_str(FRESULT r)
{
   switch(r)
   {
   case FR_OK:                  return "OK";
   case FR_DISK_ERR:            return "disk error";
   case FR_INT_ERR:             return "internal error";
   case FR_NOT_READY:           return "drive not ready";
   case FR_NO_FILE:             return "no such file";
   case FR_NO_PATH:             return "no such path";
   case FR_INVALID_NAME:        return "invalid name";
   case FR_DENIED:              return "denied (directory full or read-only)";
   case FR_EXIST:               return "already exists";
   case FR_INVALID_OBJECT:      return "invalid object";
   case FR_WRITE_PROTECTED:     return "write protected";
   case FR_INVALID_DRIVE:       return "invalid drive";
   case FR_NOT_ENABLED:         return "volume not mounted";
   case FR_NO_FILESYSTEM:       return "no filesystem";
   case FR_MKFS_ABORTED:        return "mkfs aborted";
   case FR_TIMEOUT:             return "timeout";
   case FR_LOCKED:              return "locked";
   case FR_NOT_ENOUGH_CORE:     return "out of memory";
   case FR_TOO_MANY_OPEN_FILES: return "too many open files";
   case FR_INVALID_PARAMETER:   return "invalid parameter";
   default:                     return "unknown error";
   }
}

// Grab the largest copy/CRC buffer we can, trying 4 MB down to 64 KB. The size
// actually obtained is returned through size_out. NULL means allocation failed.
static uint8_t *alloc_chunk(UINT *size_out)
{
   UINT sz;
   for(sz = 4U << 20; sz >= (64U * 1024); sz >>= 1)
   {
      uint8_t *b = malloc(sz);
      if(b != NULL)
      {
         *size_out = sz;
         return(b);
      }
   }
   return(NULL);
}

// Read one line from the serial UART with echo and backspace handling. Returns
// the line length; the buffer is always NUL terminated.
static int sd_readline(char *buf, int maxlen)
{
   int len = 0;
   uint8_t c;
   for(;;)
   {
      if(!read_keyboard(&c, 0))
         continue;
      if(c == '\r' || c == '\n')
      {
         printf("\n");
         fflush(stdout);
         buf[len] = 0;
         return(len);
      }
      if(c == 0x08 || c == 0x7F) // backspace or DEL
      {
         if(len > 0)
         {
            len--;
            printf("\b \b"); // erase the last echoed character
            fflush(stdout);
         }
         continue;
      }
      if(c >= 0x20 && c < 0x7F && len < maxlen - 1)
      {
         buf[len++] = (char)c;
         printf("%c", c); // echo
         fflush(stdout);
      }
   }
}

// Block for a single key and return 1 for 'y'/'Y', 0 otherwise.
static int sd_confirm(void)
{
   uint8_t c;
   while(!read_keyboard(&c, 0))
      ;
   printf("%c\n", (c >= 0x20 && c < 0x7F) ? c : '?');
   fflush(stdout);
   return(c == 'y' || c == 'Y');
}

// DIR <path> - list a directory with sizes and modification dates.
static void cmd_dir(const char *path)
{
   DIR dir;
   FILINFO fno;
   FRESULT res = f_opendir(&dir, path);
   if(res != FR_OK)
   {
      printf("DIR: cannot open %s (%s)\n", path, fr_str(res));
      return;
   }
   unsigned long nfiles = 0, ndirs = 0;
   uint64_t total = 0;
   printf("Directory of %s\n", path);
   for(;;)
   {
      res = f_readdir(&dir, &fno);
      if(res != FR_OK)
      {
         printf("DIR: read error (%s)\n", fr_str(res));
         break;
      }
      if(fno.fname[0] == 0) // end of directory
         break;
      int yr = 1980 + ((fno.fdate >> 9) & 0x7F);
      int mo = (fno.fdate >> 5) & 0x0F;
      int dy = (fno.fdate) & 0x1F;
      int hh = (fno.ftime >> 11) & 0x1F;
      int mi = (fno.ftime >> 5) & 0x3F;
      if(fno.fattrib & AM_DIR)
      {
         printf("%04d-%02d-%02d %02d:%02d      <DIR> %s\n", yr, mo, dy, hh, mi, fno.fname);
         ndirs++;
      }
      else
      {
         printf("%04d-%02d-%02d %02d:%02d %10llu %s\n", yr, mo, dy, hh, mi,
                (unsigned long long)fno.fsize, fno.fname);
         nfiles++;
         total += fno.fsize;
      }
   }
   f_closedir(&dir);
   printf("  %lu file(s), %llu bytes; %lu dir(s)\n", nfiles, (unsigned long long)total, ndirs);
}

// COPY <src> <dst> - streamed copy with progress, overwrite confirm, ESC abort
// and a read-total vs write-total integrity check.
static void cmd_copy(const char *src, const char *dst)
{
   FIL fsrc, fdst;
   FILINFO fno;
   FRESULT res;

   if(strcmp(src, dst) == 0)
   {
      printf("COPY: source and destination are the same file\n");
      return;
   }

   res = f_open(&fsrc, src, FA_READ);
   if(res != FR_OK)
   {
      printf("COPY: cannot open source %s (%s)\n", src, fr_str(res));
      return;
   }
   FSIZE_t total = f_size(&fsrc);

   // Ask before clobbering an existing destination.
   if(f_stat(dst, &fno) == FR_OK)
   {
      printf("%s already exists (%llu bytes). Overwrite? (y/n): ", dst, (unsigned long long)fno.fsize);
      fflush(stdout);
      if(!sd_confirm())
      {
         printf("COPY cancelled\n");
         f_close(&fsrc);
         return;
      }
   }

   res = f_open(&fdst, dst, FA_CREATE_ALWAYS | FA_WRITE);
   if(res != FR_OK)
   {
      printf("COPY: cannot create destination %s (%s)\n", dst, fr_str(res));
      f_close(&fsrc);
      return;
   }

   UINT chunk;
   uint8_t *buf = alloc_chunk(&chunk);
   if(buf == NULL)
   {
      printf("COPY: out of memory for copy buffer\n");
      f_close(&fsrc);
      f_close(&fdst);
      return;
   }

   printf("Copying %s -> %s (%llu bytes, %lu KB buffer)\n",
          src, dst, (unsigned long long)total, (unsigned long)(chunk / 1024));

   FSIZE_t done = 0;
   uint64_t rd_total = 0, wr_total = 0;
   int next_pct = 10;
   int aborted = 0, failed = 0;
   printf("[");
   fflush(stdout);
   while(done < total)
   {
      UINT want = ((total - done) > (FSIZE_t)chunk) ? chunk : (UINT)(total - done);
      UINT nr, nw;
      res = f_read(&fsrc, buf, want, &nr);
      if(res != FR_OK)
      {
         printf("\nCOPY: read error (%s)\n", fr_str(res));
         failed = 1;
         break;
      }
      rd_total += nr;
      if(nr > 0)
      {
         res = f_write(&fdst, buf, nr, &nw);
         if(res != FR_OK)
         {
            printf("\nCOPY: write error (%s)\n", fr_str(res));
            failed = 1;
            break;
         }
         wr_total += nw;
         if(nw != nr)
         {
            printf("\nCOPY: short write (disk full?)\n");
            failed = 1;
            break;
         }
      }
      done += nr;
      // Progress roughly every 10%.
      if(total > 0)
      {
         int pct = (int)((done * 100) / total);
         while(pct >= next_pct && next_pct <= 100)
         {
            printf(" %d%%", next_pct);
            fflush(stdout);
            next_pct += 10;
         }
      }
      if(nr < want) // short read => end of file
         break;
      // Let the user bail out between chunks with ESC.
      uint8_t c;
      if(read_keyboard(&c, 0) && c == 0x1B)
      {
         aborted = 1;
         break;
      }
   }
   printf(" ]\n");

   f_close(&fsrc);
   res = f_close(&fdst); // flush the write
   if(res != FR_OK)
      printf("COPY: error closing %s (%s)\n", dst, fr_str(res));

   if(aborted)
   {
      printf("COPY aborted by user (ESC): %llu of %llu bytes written, %s is incomplete\n",
             (unsigned long long)wr_total, (unsigned long long)total, dst);
      return;
   }
   if(failed)
      return;
   if(rd_total == (uint64_t)total && wr_total == rd_total)
      printf("COPY OK: %llu bytes (read == write, verified)\n", (unsigned long long)wr_total);
   else
      printf("COPY FAILED integrity check: read %llu, wrote %llu, expected %llu\n",
             (unsigned long long)rd_total, (unsigned long long)wr_total, (unsigned long long)total);
}

// REN <old> <new> - rename/move within a volume.
static void cmd_ren(const char *oldp, const char *newp)
{
   FRESULT res = f_rename(oldp, newp);
   if(res == FR_OK)
      printf("Renamed %s -> %s\n", oldp, newp);
   else
      printf("REN failed: %s (FatFs code %d)\n", fr_str(res), res);
}

// DEL <file> - delete after a y/n confirm.
static void cmd_del(const char *path)
{
   FILINFO fno;
   if(f_stat(path, &fno) != FR_OK)
   {
      printf("DEL: %s not found\n", path);
      return;
   }
   printf("Delete %s? (y/n): ", path);
   fflush(stdout);
   if(!sd_confirm())
   {
      printf("DEL cancelled\n");
      return;
   }
   FRESULT res = f_unlink(path);
   if(res == FR_OK)
      printf("Deleted %s\n", path);
   else
      printf("DEL failed: %s (FatFs code %d)\n", fr_str(res), res);
}

// MKDIR <dir> - create a directory.
static void cmd_mkdir(const char *path)
{
   FRESULT res = f_mkdir(path);
   if(res == FR_OK)
      printf("Created directory %s\n", path);
   else
      printf("MKDIR failed: %s (FatFs code %d)\n", fr_str(res), res);
}

// CRC <file> - standard CRC-32 (poly 0xEDB88320 reflected, init 0xFFFFFFFF,
// final XOR 0xFFFFFFFF), i.e. identical to zlib / python zlib.crc32. Streamed
// over the file through the chunk buffer, printed as 8 hex digits + byte count.
static void cmd_crc(const char *path)
{
   FIL fp;
   FRESULT res = f_open(&fp, path, FA_READ);
   if(res != FR_OK)
   {
      printf("CRC: cannot open %s (%s)\n", path, fr_str(res));
      return;
   }

   uint32_t table[256];
   for(uint32_t i = 0; i < 256; i++)
   {
      uint32_t c = i;
      for(int k = 0; k < 8; k++)
         c = (c & 1) ? (0xEDB88320UL ^ (c >> 1)) : (c >> 1);
      table[i] = c;
   }

   UINT chunk;
   uint8_t *buf = alloc_chunk(&chunk);
   if(buf == NULL)
   {
      printf("CRC: out of memory for read buffer\n");
      f_close(&fp);
      return;
   }

   uint32_t crc = 0xFFFFFFFFUL;
   uint64_t nbytes = 0;
   int aborted = 0;
   for(;;)
   {
      UINT br;
      res = f_read(&fp, buf, chunk, &br);
      if(res != FR_OK)
      {
         printf("CRC: read error (%s)\n", fr_str(res));
         break;
      }
      if(br == 0)
         break;
      for(UINT i = 0; i < br; i++)
         crc = table[(crc ^ buf[i]) & 0xFF] ^ (crc >> 8);
      nbytes += br;
      if(br < chunk) // end of file
         break;
      uint8_t c;
      if(read_keyboard(&c, 0) && c == 0x1B)
      {
         aborted = 1;
         break;
      }
   }
   crc ^= 0xFFFFFFFFUL;
   free(buf);
   f_close(&fp);

   if(aborted)
      printf("CRC aborted by user (ESC) after %llu bytes\n", (unsigned long long)nbytes);
   else
      printf("CRC-32 %08lX  %llu bytes  %s\n", (unsigned long)crc, (unsigned long long)nbytes, path);
}

// Print free/total space for one mounted volume.
static void free_one(const char *vol)
{
   DWORD nclst;
   FATFS *fs;
   FRESULT res = f_getfree(vol, &nclst, &fs);
   if(res != FR_OK)
   {
      printf("FREE %s: %s\n", vol, fr_str(res));
      return;
   }
   uint64_t freeb = (uint64_t)nclst * fs->csize * SD_SECTOR_SIZE;
   uint64_t totb = (uint64_t)(fs->n_fatent - 2) * fs->csize * SD_SECTOR_SIZE;
   uint64_t usedb = totb - freeb;
   printf("%s total %llu MB, used %llu MB, free %llu MB\n", vol,
          (unsigned long long)(totb / (1024 * 1024)),
          (unsigned long long)(usedb / (1024 * 1024)),
          (unsigned long long)(freeb / (1024 * 1024)));
}

// FREE [vol] - free/total space for the given volume, or both if omitted.
static void cmd_free(const char *vol)
{
   if(vol != NULL && vol[0] != 0)
      free_one(vol);
   else
   {
      free_one("0:/");
      free_one("1:/");
   }
}

static void sd_help(void)
{
   printf("\n");
   printf("SD-card file manager. Volumes: 0: (FAT boot), 1: (exFAT data).\n");
   printf("Paths need an explicit volume prefix, e.g. 1:/hdf/amix.hdf\n");
   printf("Commands (case-insensitive; file names may not contain spaces):\n");
   printf("  DIR <path>          list a directory with sizes and dates\n");
   printf("  COPY <src> <dst>    copy a file (progress, verify, ESC aborts)\n");
   printf("  REN <old> <new>     rename/move within a volume\n");
   printf("  DEL <file>          delete a file (asks to confirm)\n");
   printf("  MKDIR <dir>         create a directory\n");
   printf("  CRC <file>          CRC-32 (zlib) of a file\n");
   printf("  FREE [vol]          free/total space (both volumes if omitted)\n");
   printf("  HELP                show this help\n");
   printf("  EXIT                return to the boot menu\n");
}

void sd_fileops_menu(void)
{
   // FATFS objects are large; keep them out of the caller's stack. Both volumes
   // live on the same physical SD card (multi-partition), mounted on entry and
   // unmounted on exit, mirroring the f_clk_mount/f_umount use in mobotest.c.
   static FATFS fs0, fs1;
   FRESULT r0 = f_clk_mount(&fs0, "0:/", 1);
   FRESULT r1 = f_clk_mount(&fs1, "1:/", 1);

   printf("\n=== SD-card file manager ===\n");
   printf("Volume 0: (FAT)   %s\n", (r0 == FR_OK) ? "mounted" : fr_str(r0));
   printf("Volume 1: (exFAT) %s\n", (r1 == FR_OK) ? "mounted" : fr_str(r1));
   sd_help();

   char line[512];
   for(;;)
   {
      printf("\nSD> ");
      fflush(stdout);
      sd_readline(line, sizeof(line));

      // Split into a command word and up to two whitespace-separated arguments.
      char *cmd = strtok(line, " \t");
      if(cmd == NULL)
         continue;
      char *a1 = strtok(NULL, " \t");
      char *a2 = strtok(NULL, " \t");

      char up[12];
      int i;
      for(i = 0; cmd[i] != 0 && i < (int)sizeof(up) - 1; i++)
         up[i] = (char)toupper((unsigned char)cmd[i]);
      up[i] = 0;

      if(strcmp(up, "EXIT") == 0)
         break;
      else if(strcmp(up, "HELP") == 0)
         sd_help();
      else if(strcmp(up, "DIR") == 0)
      {
         if(a1 == NULL) printf("usage: DIR <path>\n");
         else cmd_dir(a1);
      }
      else if(strcmp(up, "COPY") == 0)
      {
         if(a1 == NULL || a2 == NULL) printf("usage: COPY <src> <dst>\n");
         else cmd_copy(a1, a2);
      }
      else if(strcmp(up, "REN") == 0)
      {
         if(a1 == NULL || a2 == NULL) printf("usage: REN <old> <new>\n");
         else cmd_ren(a1, a2);
      }
      else if(strcmp(up, "DEL") == 0)
      {
         if(a1 == NULL) printf("usage: DEL <file>\n");
         else cmd_del(a1);
      }
      else if(strcmp(up, "MKDIR") == 0)
      {
         if(a1 == NULL) printf("usage: MKDIR <dir>\n");
         else cmd_mkdir(a1);
      }
      else if(strcmp(up, "CRC") == 0)
      {
         if(a1 == NULL) printf("usage: CRC <file>\n");
         else cmd_crc(a1);
      }
      else if(strcmp(up, "FREE") == 0)
         cmd_free(a1); // a1 == NULL means both volumes
      else
         printf("Unknown command '%s'. Type HELP.\n", cmd);
   }

   f_umount("0:/");
   f_umount("1:/");
   printf("Returning to the boot menu.\n");
}
