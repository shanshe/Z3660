// SPDX-License-Identifier: MIT
//
// Z3660 Floppy/ADF driver - ARM side
// Handles reading/writing .ADF disk image files via Z3660 FPGA registers
// Equivalent to scsi.c but for floppy disk images instead of hard disk images
//

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

#include "floppy.h"
#include "../config_file.h"
#include "../debug_console.h"
#include "../scsi/scsi.h"

extern CONFIG config;
extern DEBUG_CONSOLE debug_console;
extern uint8_t *piscsi_rom_ptr;

#define BE(val) be32toh(val)
#define BE16(val) be16toh(val)

//#define PIFLOPPY_DEBUG

#ifdef PIFLOPPY_DEBUG
#define DEBUG printf
#define DEBUG_TRIVIAL printf
#else
#ifdef NO_DEBUG_CONSOLE
#define DEBUG(...)
#define DEBUG_TRIVIAL(...)
#else
void pifloppy_debug(const char *format, ...)
{
   if(debug_console.debug_scsi==0)
      return;
   va_list args;
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
}
#define DEBUG pifloppy_debug
#define DEBUG_TRIVIAL(...)
#endif
#endif

HUNK_INFO pifloppy_hinfo;
HUNK_RELOC pifloppy_hreloc[2048];

uint32_t pifloppy_used_dma = 0;

PIFLOPPY_DEV pifloppy_devs[NUM_FLOPPY_UNITS];

uint8_t pifloppy_cur_drive = 0;
uint32_t pifloppy_u32_read[4];
uint32_t pifloppy_u32_write[4];
uint32_t pifloppy_dbg[8];

FIL pifloppy_fd[NUM_FLOPPY_UNITS];

//static FATFS pifloppy_fatfs;
//static int pifloppy_fatfs_mounted = 0;

// ADF filenames from config (up to 4 drives)
// These will be loaded from the config file
static char pifloppy_adf_names[NUM_FLOPPY_UNITS][150];

// SCSI_NO_DMA_ADDRESS equivalent for floppy DMA
#define FLOPPY_NO_DMA_ADDRESS (RTG_BASE + 0x180000)

// ============================================================
// Initialization
// ============================================================

int pifloppy_init(void)
{
    memset(pifloppy_devs, 0, sizeof(pifloppy_devs));
    memset(pifloppy_adf_names, 0, sizeof(pifloppy_adf_names));

    // Initialize all drives as empty
    for (int i = 0; i < NUM_FLOPPY_UNITS; i++) {
        pifloppy_devs[i].fd = NULL;
        pifloppy_devs[i].block_size = ADF_SECTOR_SIZE;
        pifloppy_devs[i].total_blocks = 0;
        pifloppy_devs[i].cylinders = ADF_TRACKS;
        pifloppy_devs[i].heads = ADF_SIDES;
        pifloppy_devs[i].sectors_per_track = ADF_SECTORS_PER_TRACK_DD;
        pifloppy_devs[i].drive_type = FLOPPY_TYPE_NONE;
        pifloppy_devs[i].disk_inserted = 0;
        pifloppy_devs[i].write_protect = 1; // Default write-protected
        pifloppy_devs[i].motor_on = 0;
        pifloppy_devs[i].current_track = 0;
        pifloppy_devs[i].current_side = 0;
        pifloppy_devs[i].change_count = 0;
        pifloppy_devs[i].file_size = 0;
    }
/* Already mounted in scsi.c
    // Mount filesystem if not already mounted
    if (!pifloppy_fatfs_mounted) {
        TCHAR *Path = DEFAULT_ROOT;
        f_clk_mount(&pifloppy_fatfs, Path, 1);
        pifloppy_fatfs_mounted = 1;
    }
*/
    // Try to load ADF files for each drive
    // ADF files are named: adf0.adf, adf1.adf, adf2.adf, adf3.adf
    // Located in the root of the SD card
    for (int i = 0; i < NUM_FLOPPY_UNITS; i++) {
        if(config.adf_num[i]>=0 && config.adf_num[i]<=19) {
            if(config.adf[config.adf_num[i]][0]!=0) {
                char adf_path[256];
                sprintf(adf_path, "%s", config.adf[config.adf_num[i]]);

                FIL *tmp_fd = &pifloppy_fd[i];
                int ret = f_open(tmp_fd, adf_path, FA_READ | FA_WRITE | FA_OPEN_EXISTING);
                if (ret == FR_OK) {
                    FSIZE_t file_size = f_size(tmp_fd);
                    pifloppy_devs[i].fd = tmp_fd;
                    pifloppy_devs[i].file_size = file_size;
                    pifloppy_devs[i].disk_inserted = 1;
                    pifloppy_devs[i].configured = 1;
                    pifloppy_devs[i].write_protect = 0; // Opened for write

                    // Determine drive type based on file size
                    if (file_size == ADF_DD_SIZE) {
                        pifloppy_devs[i].drive_type = FLOPPY_TYPE_DD;
                        pifloppy_devs[i].total_blocks = ADF_TOTAL_SECTORS_DD;
                        pifloppy_devs[i].sectors_per_track = ADF_SECTORS_PER_TRACK_DD;
                        printf("[PIFLOPPY] ADF%d: DD floppy image loaded (%lld bytes, %d sectors) %s\n",
                            i, (uint64_t)file_size, ADF_TOTAL_SECTORS_DD,&adf_path[3]);
                    } else if (file_size == ADF_HD_SIZE) {
                        pifloppy_devs[i].drive_type = FLOPPY_TYPE_HD;
                        pifloppy_devs[i].total_blocks = ADF_TOTAL_SECTORS_HD;
                        pifloppy_devs[i].sectors_per_track = ADF_SECTORS_PER_TRACK_HD;
                        printf("[PIFLOPPY] ADF%d: HD floppy image loaded (%lld bytes, %d sectors) %s\n",
                            i, (uint64_t)file_size, ADF_TOTAL_SECTORS_HD,&adf_path[3]);
                    } else {
                        // Unknown size - treat as DD with whatever size
                        pifloppy_devs[i].drive_type = FLOPPY_TYPE_DD;
                        pifloppy_devs[i].total_blocks = file_size / ADF_SECTOR_SIZE;
                        pifloppy_devs[i].sectors_per_track = ADF_SECTORS_PER_TRACK_DD;
                        printf("[PIFLOPPY] ADF%d: Unknown size floppy image (%lld bytes, %ld sectors) %s\n",
                            i, (uint64_t)file_size, (long)pifloppy_devs[i].total_blocks,&adf_path[3]);
                    }

                    pifloppy_devs[i].change_count = 1;
                    f_lseek(tmp_fd, 0);
                } else {
                    // Try read-only
                    ret = f_open(tmp_fd, adf_path, FA_READ | FA_OPEN_EXISTING);
                    if (ret == FR_OK) {
                        FSIZE_t file_size = f_size(tmp_fd);
                        pifloppy_devs[i].fd = tmp_fd;
                        pifloppy_devs[i].file_size = file_size;
                        pifloppy_devs[i].disk_inserted = 1;
                        pifloppy_devs[i].configured = 1;
                        pifloppy_devs[i].write_protect = 1; // Read-only

                        if (file_size == ADF_DD_SIZE) {
                            pifloppy_devs[i].drive_type = FLOPPY_TYPE_DD;
                            pifloppy_devs[i].total_blocks = ADF_TOTAL_SECTORS_DD;
                            pifloppy_devs[i].sectors_per_track = ADF_SECTORS_PER_TRACK_DD;
                        } else if (file_size == ADF_HD_SIZE) {
                            pifloppy_devs[i].drive_type = FLOPPY_TYPE_HD;
                            pifloppy_devs[i].total_blocks = ADF_TOTAL_SECTORS_HD;
                            pifloppy_devs[i].sectors_per_track = ADF_SECTORS_PER_TRACK_HD;
                        } else {
                            pifloppy_devs[i].drive_type = FLOPPY_TYPE_DD;
                            pifloppy_devs[i].total_blocks = file_size / ADF_SECTOR_SIZE;
                            pifloppy_devs[i].sectors_per_track = ADF_SECTORS_PER_TRACK_DD;
                        }

                        pifloppy_devs[i].change_count = 1;
                        f_lseek(tmp_fd, 0);
                        printf("[PIFLOPPY] ADF%d: Floppy image loaded (read-only, %lld bytes) %s\n",
                            i, (uint64_t)file_size,&adf_path[3]);
                    } else {
                        printf("[PIFLOPPY] ADF%d: No ADF image found (%s)\n", i, &adf_path[3]);
                    }
                }
            }
        }
    }

    FIL in;
    f_open(&in,DEFAULT_ROOT "z3660_scsi.rom", FA_READ | FA_OPEN_EXISTING);

    f_lseek(&in, 0x2600);
    int debug=1;
    process_hunks(&in, &pifloppy_hinfo, pifloppy_hreloc, 0x2600, debug);
    f_close(&in);

    Xil_L1DCacheFlush();
    Xil_L2CacheFlush();

    return 1;
}

void pifloppy_shutdown(void)
{
    for (int i = 0; i < NUM_FLOPPY_UNITS; i++) {
        if (pifloppy_devs[i].fd != NULL) {
            f_close(pifloppy_devs[i].fd);
            pifloppy_devs[i].fd = NULL;
            pifloppy_devs[i].disk_inserted = 0;
        }
    }
}

// ============================================================
// Debug message names
// ============================================================
static const char *pifloppy_cmd_name(uint32_t cmd)
{
    switch (cmd) {
    case PIFLP_CMD_READ:        return "READ";
    case PIFLP_CMD_WRITE:       return "WRITE";
    case PIFLP_CMD_DRVNUM:      return "DRVNUM";
    case PIFLP_CMD_DRVTYPE:     return "DRVTYPE";
    case PIFLP_CMD_TRACK:       return "TRACK";
    case PIFLP_CMD_SIDE:        return "SIDE";
    case PIFLP_CMD_SECTOR:      return "SECTOR";
    case PIFLP_CMD_MOTOR:       return "MOTOR";
    case PIFLP_CMD_SEEK:        return "SEEK";
    case PIFLP_CMD_STATUS:      return "STATUS";
    case PIFLP_CMD_CHANGE:      return "CHANGE";
    case PIFLP_CMD_PROTECT:     return "PROTECT";
    case PIFLP_CMD_FORMAT:      return "FORMAT";
    case PIFLP_CMD_READ64:      return "READ64";
    case PIFLP_CMD_WRITE64:     return "WRITE64";
    case PIFLP_CMD_USED_DMA:    return "USED_DMA";
    case PIFLP_CMD_BLOCKSIZE:   return "BLOCKSIZE";
    case PIFLP_CMD_TOTALTRACKS: return "TOTALTRACKS";
    case PIFLP_CMD_TOTALSECTORS:return "TOTALSECTORS";
    case PIFLP_CMD_TOTALSIDES:  return "TOTALSIDES";
    case PIFLP_CMD_CYLINDERS:   return "CYLINDERS";
    case PIFLP_CMD_HEADS:       return "HEADS";
    case PIFLP_CMD_SECPERTRACK: return "SECPERTRACK";
    case PIFLP_CMD_DISKINSERTED:return "DISKINSERTED";
    case PIFLP_CMD_ADFNAME:     return "ADFNAME";
    case PIFLP_CMD_EJECT:       return "EJECT";
    default:                    return "UNKNOWN";
    }
}

// ============================================================
// Debug message handler
// ============================================================
static void pifloppy_debug_msg(uint32_t index)
{
    switch (index) {
    case DBG_FLP_INIT:
        DEBUG("[PIFLOPPY] Initializing devices.\n");
        break;
    case DBG_FLP_OPENDEV:
        DEBUG("[PIFLOPPY] Opening device %ld.\n", pifloppy_dbg[0]);
        break;
    case DBG_FLP_CLEANUP:
        DEBUG("[PIFLOPPY] Cleaning up.\n");
        break;
    case DBG_FLP_BEGINIO:
        DEBUG("[PIFLOPPY] BeginIO: command %ld (%s)\n", pifloppy_dbg[0], pifloppy_cmd_name(pifloppy_dbg[0]));
        break;
    case DBG_FLP_ABORTIO:
        DEBUG("[PIFLOPPY] AbortIO!\n");
        break;
    case DBG_FLP_READ:
        DEBUG("[PIFLOPPY] Read: drive %ld, block %ld, length %ld\n",
              pifloppy_dbg[0], pifloppy_dbg[1], pifloppy_dbg[2]);
        break;
    case DBG_FLP_WRITE:
        DEBUG("[PIFLOPPY] Write: drive %ld, block %ld, length %ld\n",
              pifloppy_dbg[0], pifloppy_dbg[1], pifloppy_dbg[2]);
        break;
    case DBG_FLP_ERR:
        DEBUG("[PIFLOPPY] Error: %ld\n", pifloppy_dbg[0]);
        break;
    default:
        DEBUG("[PIFLOPPY] Debug message %ld\n", index);
        break;
    }
}

// ============================================================
// Register write handler
// ============================================================
void handle_pifloppy_reg_write(uint32_t addr, uint32_t val, uint8_t type)
{
    (void)type; // unused for now
    PIFLOPPY_DEV *d = &pifloppy_devs[pifloppy_cur_drive];

    switch (addr) {
    case PIFLP_CMD_EJECT:
        if (d->fd != NULL) {
            f_close(d->fd);
            d->fd = NULL;
        }
        d->disk_inserted = 0;
        d->change_count++;
        d->write_protect = 1;
        d->total_blocks = 0;
        d->current_track = 0;
        d->current_side = 0;
        d->motor_on = 0;
        printf("[PIFLOPPY] ADF%d: Disk ejected.\n", pifloppy_cur_drive);
        break;
    // ---- Read commands ----
    case PIFLP_CMD_READ64:
    case PIFLP_CMD_READ: {
        d = &pifloppy_devs[val];
        if (d->fd == NULL || !d->disk_inserted) {
            DEBUG("[PIFLOPPY] Read from empty drive %ld.\n", val);
            break;
        }

        uint64_t offset;
        uint32_t length = pifloppy_u32_read[1];
        uint32_t buffer_addr = pifloppy_u32_read[2];

        if (addr == PIFLP_CMD_READ) {
            // 32-bit read: block number in u32_read[0]
            offset = (uint64_t)pifloppy_u32_read[0];
            DEBUG("[PIFLOPPY-%ld] %ld byte READ from block %ld to address %.8lX\n",
                  val, length, pifloppy_u32_read[0], buffer_addr);
        } else {
            // 64-bit read: high 32 bits in u32_read[3], low in u32_read[0]
            uint64_t block64 = ((uint64_t)pifloppy_u32_read[3] << 32) | pifloppy_u32_read[0];
            offset = block64;
            DEBUG("[PIFLOPPY-%ld] %ld byte READ64 from block %lld to address %.8lX\n",
                  val, length, block64, buffer_addr);
        }

        // Seek to position in ADF file
        f_lseek(d->fd, offset);

        uint32_t map = buffer_addr;
        if ((config.cpu_ram && (map >= 0x08000000) && (map < 0x10000000)) ||
            (config.autoconfig_ram && (map >= 0x40000000) && (map < 0x50000000)))
        {
            // DMA directly to Amiga RAM
            if (map >= 0x40000000) map -= (0x40000000 - 0x20000000);
            DEBUG("[PIFLOPPY-%ld] DMA Read to mapped range 0x%08lX.\n", val, map);
            pifloppy_used_dma = 0;
            unsigned int n_bytes;
            FRESULT res = f_read(d->fd, (uint8_t *)map, length, &n_bytes);
            if (res != FR_OK) {
                printf("[PIFLOPPY] ERROR: f_read result=%d\n", res);
            }
            if (n_bytes != length) {
                printf("[PIFLOPPY] ERROR: bytes_to_read=%ld, bytes_read=%d\n", length, n_bytes);
            }
        } else {
            // No DMA - use buffer area
            DEBUG("[PIFLOPPY-%ld] Read to buffer address %.8lX\n", val, buffer_addr);
            if (length >= 0x80000)
                printf("[PIFLOPPY] ERROR: read length > 0x80000 (0x%08lX)\n", length);
            uint8_t *buffer = (uint8_t *)FLOPPY_NO_DMA_ADDRESS;
            pifloppy_used_dma = buffer_addr;
            unsigned int n_bytes;
            FRESULT res = f_read(d->fd, buffer, length, &n_bytes);
            if (res != FR_OK) {
                printf("[PIFLOPPY] ERROR: f_read result=%d\n", res);
            }
            if (n_bytes != length) {
                printf("[PIFLOPPY] ERROR: bytes_to_read=%ld, bytes_read=%d\n", length, n_bytes);
            }
        }
        Xil_L1DCacheFlush();
        break;
    }

    // ---- Write commands ----
    case PIFLP_CMD_WRITE64:
    case PIFLP_CMD_WRITE: {
        d = &pifloppy_devs[val];
        if (d->fd == NULL || !d->disk_inserted) {
            printf("[PIFLOPPY] Write to empty drive %ld.\n", val);
            break;
        }
        if (d->write_protect) {
            printf("[PIFLOPPY] Write to write-protected drive %ld.\n", val);
            break;
        }

        uint64_t offset;
        uint32_t length = pifloppy_u32_write[1];
        uint32_t buffer_addr = pifloppy_u32_write[2];

        if (addr == PIFLP_CMD_WRITE) {
            offset = (uint64_t)pifloppy_u32_write[0];
            DEBUG("[PIFLOPPY-%ld] %ld byte WRITE to block %ld from address %.8lX\n",
                  val, length, pifloppy_u32_write[0], buffer_addr);
        } else {
            uint64_t block64 = ((uint64_t)pifloppy_u32_write[3] << 32) | pifloppy_u32_write[0];
            offset = block64;
            DEBUG("[PIFLOPPY-%ld] %ld byte WRITE64 to block %lld from address %.8lX\n",
                  val, length, block64, buffer_addr);
        }

        f_lseek(d->fd, offset);

        uint32_t map = buffer_addr;
        if ((config.cpu_ram && (map >= 0x08000000) && (map < 0x10000000)) ||
            (config.autoconfig_ram && (map >= 0x40000000) && (map < 0x50000000)))
        {
            // DMA from Amiga RAM
            if (map >= 0x40000000) map -= 0x20000000;
            DEBUG("[PIFLOPPY-%ld] DMA Write from mapped range 0x%08lX.\n", val, map);
            pifloppy_used_dma = 0;
            unsigned int n_bytes;
            FRESULT res = f_write(d->fd, (uint8_t *)map, length, &n_bytes);
            if (res != FR_OK) {
                printf("[PIFLOPPY] ERROR: f_write result=%d\n", res);
            }
            if (n_bytes != length) {
                printf("[PIFLOPPY] ERROR: bytes_to_write=%ld, bytes_written=%d\n", length, n_bytes);
            }
        } else {
            // No DMA - use buffer area
            DEBUG("[PIFLOPPY-%ld] Write from buffer address %.8lX\n", val, buffer_addr);
            if (length >= 0x80000)
                printf("[PIFLOPPY] ERROR: write length > 0x80000 (0x%08lX)\n", length);
            uint8_t *buffer = (uint8_t *)FLOPPY_NO_DMA_ADDRESS;
            pifloppy_used_dma = buffer_addr;
            unsigned int n_bytes;
            FRESULT res = f_write(d->fd, buffer, length, &n_bytes);
            if (res != FR_OK) {
                printf("[PIFLOPPY] ERROR: f_write result=%d\n", res);
            }
            if (n_bytes != length) {
                printf("[PIFLOPPY] ERROR: bytes_to_write=%ld, bytes_written=%d\n", length, n_bytes);
            }
        }
        Xil_L1DCacheFlush();
        break;
    }

    // ---- Read address registers ----
    case PIFLP_CMD_READ_ADDR1:
    case PIFLP_CMD_READ_ADDR2:
    case PIFLP_CMD_READ_ADDR3:
    case PIFLP_CMD_READ_ADDR4: {
        int i = (addr - PIFLP_CMD_READ_ADDR1) / 4;
        DEBUG("PIFLP_CMD_READ_ADDR%d %ld\n",i+1,val);
        pifloppy_u32_read[i] = val;
        break;
    }

    // ---- Write address registers ----
    case PIFLP_CMD_WRITE_ADDR1:
    case PIFLP_CMD_WRITE_ADDR2:
    case PIFLP_CMD_WRITE_ADDR3:
    case PIFLP_CMD_WRITE_ADDR4: {
        int i = (addr - PIFLP_CMD_WRITE_ADDR1) / 4;
        pifloppy_u32_write[i] = val;
        break;
    }

    // ---- Drive select ----
    case PIFLP_CMD_DRVNUM:
        if (val >= NUM_FLOPPY_UNITS) {
            pifloppy_cur_drive = 0;
            DEBUG("[PIFLOPPY] Invalid drive number %ld, resetting to 0.\n", val);
        } else {
            pifloppy_cur_drive = val;
            DEBUG("[PIFLOPPY] Drive number set to %d.\n", pifloppy_cur_drive);
        }
        break;

    case PIFLP_CMD_DRVNUMX:
        pifloppy_cur_drive = val & 0x07;
        DEBUG("[PIFLOPPY] DRVNUMX: %ld.\n", val);
        break;

    // ---- Track/side/sector control ----
    case PIFLP_CMD_TRACK:
        d->current_track = val;
        DEBUG("[PIFLOPPY] Track set to %ld.\n", val);
        break;

    case PIFLP_CMD_SIDE:
        d->current_side = val & 1;
        DEBUG("[PIFLOPPY] Side set to %ld.\n", val);
        break;

    case PIFLP_CMD_SECTOR:
        DEBUG("[PIFLOPPY] Sector set to %ld (ignored in block mode).\n", val);
        break;

    // ---- Motor control ----
    case PIFLP_CMD_MOTOR:
        d->motor_on = val & 1;
        DEBUG("[PIFLOPPY] Motor %s for drive %d.\n", d->motor_on ? "ON" : "OFF", pifloppy_cur_drive);
        break;

    // ---- Seek ----
    case PIFLP_CMD_SEEK:
        d->current_track = val;
        DEBUG("[PIFLOPPY] Seek to track %ld.\n", val);
        break;

    // ---- Format (no-op for ADF, just acknowledge) ----
    case PIFLP_CMD_FORMAT:
        if (d->fd != NULL && d->disk_inserted && !d->write_protect) {
            DEBUG("[PIFLOPPY] Format track %ld side %ld.\n", d->current_track, d->current_side);
        }
        break;

    // ---- Debug registers ----
    case PIFLP_DBG_VAL1:
    case PIFLP_DBG_VAL2:
    case PIFLP_DBG_VAL3:
    case PIFLP_DBG_VAL4:
    case PIFLP_DBG_VAL5: {
        int i = (addr - PIFLP_DBG_VAL1) / 4;
        pifloppy_dbg[i] = val;
        break;
    }

    case PIFLP_DBG_MSG:
        pifloppy_debug_msg(val);
        break;

    case PIFLP_CMD_DRIVER:
       printf("[PIFLP] Driver copy/patch called, destination address %.8lX.\n", val);
       //            r = 0;//get_mapped_item_by_address(cfg, val);
       if ( ((val>=0x08000000) && (val<0x10000000) && config.cpu_ram)
             ||((val>=0x40000000) && (val<0x50000000) && config.autoconfig_ram)
       )
       {
          if(val>=0x40000000) val-=0x20000000;

          //            if (r != -1) {
          uint32_t addr = val;// - cfg->map_offset[r];
          uint8_t *dst_data = 0;//cfg->map_data[r];

          memcpy(dst_data + addr, piscsi_rom_ptr + PIFLP_DRIVER_OFFSET, BOOT_ROM_SIZE - PIFLP_DRIVER_OFFSET);
          pifloppy_hinfo.base_offset = val;
          reloc_hunks(pifloppy_hreloc, dst_data + addr, &pifloppy_hinfo, 1);
  
          //                uint32_t data_addr = addr + 0x3F00;
          uint32_t data_addr = addr + BOOT_ROM_SIZE - 0x100;
          sprintf((char *)dst_data + data_addr, "z3660_floppy.device");

          Xil_L1DCacheFlush();
          Xil_L2CacheFlush();

       }
       else
       {
          printf("[PISCSI] DRIVER Address 0x%08lX not mapped in FPGA RAM...\n",val);
       }

       break;

    default:
        printf("[PIFLOPPY] WARN: Unhandled register write to %.8lX: %ld\n", addr, val);
        break;
    }
}

// ============================================================
// Register read handler
// ============================================================
uint32_t handle_pifloppy_read(uint32_t addr, uint8_t type)
{
    (void)type;
//    printf("handle_pifloppy_read add 0x%0lX\n",addr);

    PIFLOPPY_DEV *d = &pifloppy_devs[pifloppy_cur_drive];

    switch (addr) {
    // ---- Read address registers (echo back what was written) ----
    case PIFLP_CMD_READ_ADDR1:
    case PIFLP_CMD_READ_ADDR2:
    case PIFLP_CMD_READ_ADDR3:
    case PIFLP_CMD_READ_ADDR4: {
        int i = (addr - PIFLP_CMD_READ_ADDR1) / 4;
        return pifloppy_u32_read[i];
    }

    // ---- Write address registers (echo back what was written) ----
    case PIFLP_CMD_WRITE_ADDR1:
    case PIFLP_CMD_WRITE_ADDR2:
    case PIFLP_CMD_WRITE_ADDR3:
    case PIFLP_CMD_WRITE_ADDR4: {
        int i = (addr - PIFLP_CMD_WRITE_ADDR1) / 4;
        return pifloppy_u32_write[i];
    }

    // ---- Drive type ----
    case PIFLP_CMD_DRVTYPE:
        // Return drive type even when no disk is inserted
        // The drive type is a property of the drive, not the disk
        // Only return NONE if this drive slot was never configured
        if (!d->configured) {
            DEBUG("[PIFLOPPY] DRVTYPE: drive %d not configured.\n", pifloppy_cur_drive);
            return FLOPPY_TYPE_NONE;
        }
        DEBUG("[PIFLOPPY] DRVTYPE: drive %d type %ld.\n", pifloppy_cur_drive, d->drive_type);
        return d->drive_type;

    // ---- Current drive number ----
    case PIFLP_CMD_DRVNUM:
        return pifloppy_cur_drive;

    // ---- Geometry: cylinders ----
    case PIFLP_CMD_CYLINDERS:
        if (d->fd == NULL || !d->disk_inserted) return 0;
        return d->cylinders;

    // ---- Geometry: heads ----
    case PIFLP_CMD_HEADS:
        if (d->fd == NULL || !d->disk_inserted) return 0;
        return d->heads;

    // ---- Geometry: sectors per track ----
    case PIFLP_CMD_SECPERTRACK:
        if (d->fd == NULL || !d->disk_inserted) return 0;
        return d->sectors_per_track;

    // ---- Geometry: total tracks ----
    case PIFLP_CMD_TOTALTRACKS:
        if (d->fd == NULL || !d->disk_inserted) return 0;
        return d->cylinders;

    // ---- Geometry: total sectors per track ----
    case PIFLP_CMD_TOTALSECTORS:
        if (d->fd == NULL || !d->disk_inserted) return 0;
        return d->sectors_per_track;

    // ---- Geometry: total sides ----
    case PIFLP_CMD_TOTALSIDES:
        if (d->fd == NULL || !d->disk_inserted) return 0;
        return d->heads;

    // ---- Block size ----
    case PIFLP_CMD_BLOCKSIZE:
        if (d->fd == NULL || !d->disk_inserted) return 0;
        return d->block_size;

    // ---- Disk inserted flag ----
    case PIFLP_CMD_DISKINSERTED:
        return d->disk_inserted;

    // ---- Drive status ----
    case PIFLP_CMD_STATUS:
        if (d->fd == NULL || !d->disk_inserted) return 0;
        // Bit 0: disk in drive, Bit 1: motor on, Bit 2: write protected
        return (d->disk_inserted & 1) |
               ((d->motor_on & 1) << 1) |
               ((d->write_protect & 1) << 2);

    // ---- Disk change count ----
    case PIFLP_CMD_CHANGE:
        return d->change_count;

    // ---- Write protect status ----
    case PIFLP_CMD_PROTECT:
        return d->write_protect;

    // ---- Current track ----
    case PIFLP_CMD_TRACK:
        return d->current_track;

    // ---- Current side ----
    case PIFLP_CMD_SIDE:
        return d->current_side;

    // ---- DMA used flag ----
    case PIFLP_CMD_USED_DMA: {
        uint32_t temp = pifloppy_used_dma;
        pifloppy_used_dma = 0;
        return temp;
    }

    // ---- Per-unit block size registers ----
    case PIFLP_CMD_BLOCKSIZE0:
    case PIFLP_CMD_BLOCKSIZE1:
    case PIFLP_CMD_BLOCKSIZE2:
    case PIFLP_CMD_BLOCKSIZE3:
    case PIFLP_CMD_BLOCKSIZE4:
    case PIFLP_CMD_BLOCKSIZE5:
    case PIFLP_CMD_BLOCKSIZE6:
    case PIFLP_CMD_BLOCKSIZE7: {
        int unit = (addr - PIFLP_CMD_BLOCKSIZE0) / 4;
        if (unit < NUM_FLOPPY_UNITS)
            return pifloppy_devs[unit].block_size;
        return 0;
    }

    // ---- Per-unit total blocks registers ----
    case PIFLP_CMD_BLOCKS0:
    case PIFLP_CMD_BLOCKS1:
    case PIFLP_CMD_BLOCKS2:
    case PIFLP_CMD_BLOCKS3:
    case PIFLP_CMD_BLOCKS4:
    case PIFLP_CMD_BLOCKS5:
    case PIFLP_CMD_BLOCKS6:
    case PIFLP_CMD_BLOCKS7: {
        int unit = (addr - PIFLP_CMD_BLOCKS0) / 4;
        if (unit < NUM_FLOPPY_UNITS)
            return pifloppy_devs[unit].total_blocks;
        return 0;
    }

    default:
        printf("[PIFLOPPY] WARN: Unhandled register read from %.8lX\n", addr);
        break;
    }

    return 0;
}
