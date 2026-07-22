// SPDX-License-Identifier: MIT
//
// Z3660 Floppy/ADF Driver - Main header
// Based on z3660_scsi.h pattern
//

#ifndef Z3660_FLOPPY_H
#define Z3660_FLOPPY_H

#include <devices/trackdisk.h>

#define STR(s) #s
#define XSTR(s) STR(s)

#define DEVICE_NAME "z3660_floppy.device"
#define DEVICE_DATE "(19 May 2026)"
#define DEVICE_ID_STRING "Z3660FLP " XSTR(DEVICE_VERSION) "." XSTR(DEVICE_REVISION) " " DEVICE_DATE
#define DEVICE_VERSION 1
#define DEVICE_REVISION 1
#define DEVICE_PRIORITY 10

#define ARRAY_SIZE(x) ((sizeof(x) / sizeof((x)[0])))
#define BIT(x) (1 << (x))

// ADF disk geometry constants
#define ADF_SECTOR_SIZE     512
#define ADF_SECTORS_PER_TRACK_DD  11   // DD: 11 sectors/track
#define ADF_SECTORS_PER_TRACK_HD  22   // HD: 22 sectors/track
#define ADF_TRACKS_PER_DISK  80
#define ADF_SIDES_DD        2
#define ADF_SIDES_HD        2
#define ADF_TOTAL_SECTORS_DD  (ADF_SECTORS_PER_TRACK_DD * ADF_TRACKS_PER_DISK * ADF_SIDES_DD)  // 1760
#define ADF_TOTAL_SECTORS_HD  (ADF_SECTORS_PER_TRACK_HD * ADF_TRACKS_PER_DISK * ADF_SIDES_HD)  // 3520
#define ADF_DD_SIZE         (ADF_TOTAL_SECTORS_DD * ADF_SECTOR_SIZE)   // 880KB = 901120 bytes
#define ADF_HD_SIZE         (ADF_TOTAL_SECTORS_HD * ADF_SECTOR_SIZE)   // 1760KB = 1802240 bytes

// DMA buffer offset in Z3660 register space (same as SCSI)
#define Z3660_DMA_BUFFER    0x80000

#endif // Z3660_FLOPPY_H