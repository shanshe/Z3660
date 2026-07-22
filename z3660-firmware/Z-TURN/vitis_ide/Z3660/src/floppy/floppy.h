// SPDX-License-Identifier: MIT
#ifndef _FLOPPY_H_
#define _FLOPPY_H_
#include <stdint.h>
#include <ff.h>
#include "z3660_floppy_enums.h"
#include "../defines.h"

// ADF geometry constants
#define ADF_SECTOR_SIZE      512
#define ADF_SECTORS_PER_TRACK_DD  11
#define ADF_SECTORS_PER_TRACK_HD  22
#define ADF_TRACKS           80
#define ADF_SIDES            2
#define ADF_TOTAL_SECTORS_DD (ADF_TRACKS * ADF_SIDES * ADF_SECTORS_PER_TRACK_DD)  // 1760
#define ADF_TOTAL_SECTORS_HD (ADF_TRACKS * ADF_SIDES * ADF_SECTORS_PER_TRACK_HD)  // 3520
#define ADF_DD_SIZE          (ADF_TOTAL_SECTORS_DD * ADF_SECTOR_SIZE)  // 901120
#define ADF_HD_SIZE          (ADF_TOTAL_SECTORS_HD * ADF_SECTOR_SIZE)  // 1802240

#define NUM_FLOPPY_UNITS 8
#define PIFLP_DRIVER_OFFSET 0x2600

// Floppy drive types
enum floppy_drive_type {
    FLOPPY_TYPE_NONE = 0,
    FLOPPY_TYPE_DD   = 1,
    FLOPPY_TYPE_HD   = 2,
};

typedef struct pifloppy_dev_ {
    FIL *fd;                    // File descriptor for ADF file (NULL if not loaded)
    uint32_t block_size;        // Sector size (512 for DD, 512 for HD)
    uint32_t total_blocks;     // Total sectors (1760 for DD, 3520 for HD)
    uint32_t cylinders;        // Number of cylinders (80)
    uint32_t heads;            // Number of heads (2)
    uint32_t sectors_per_track; // Sectors per track (11 DD, 22 HD)
    uint32_t drive_type;       // FLOPPY_TYPE_DD or FLOPPY_TYPE_HD
    uint32_t disk_inserted;    // 1 if ADF loaded, 0 if not
    uint32_t configured;       // 1 if this drive slot has an ADF configured (even if ejected)
    uint32_t write_protect;    // 1 if write protected
    uint32_t motor_on;         // Motor state (1=on, 0=off)
    uint32_t current_track;    // Current track position (0-79)
    uint32_t current_side;     // Current side (0 or 1)
    uint32_t change_count;     // Disk change counter
    FSIZE_t file_size;         // Size of the ADF file
} PIFLOPPY_DEV;

int pifloppy_init(void);
void pifloppy_shutdown(void);
void handle_pifloppy_reg_write(uint32_t addr, uint32_t val, uint8_t type);
uint32_t handle_pifloppy_read(uint32_t addr, uint8_t type);

#endif /* _FLOPPY_H_ */
