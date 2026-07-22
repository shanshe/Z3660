// SPDX-License-Identifier: MIT
//
// Z3660 Floppy/ADF Driver - Register definitions
// Reads .ADF disk image files via Z3660 FPGA registers
//
// Registers are at Z3660_REGS + PIFLOPPY_OFFSET (0x2000)
// Sharing the same base offset as SCSI but with different command space

#ifndef Z3660_FLOPPY_ENUMS_H
#define Z3660_FLOPPY_ENUMS_H

#define NUM_FLOPPY_UNITS  8    // DF0:-DF3: (up to 4 floppy drives)

// Register offsets for floppy/ADF access
// These share the Z3660 register space starting at PIFLOPPY_OFFSET
// The FPGA will handle ADF file access on the ARM side
#define PIFLOPPY_OFFSET   0x00002000
#define PIFLOPPY_REGSIZE  0x00010000

// Floppy command registers (offsets from PIFLOPPY_OFFSET base)
enum floppy_cmds {
    PIFLP_CMD_READ         = 0x400,  // Read sector(s) from ADF
    PIFLP_CMD_WRITE        = 0x404,  // Write sector(s) to ADF
    PIFLP_CMD_DRVNUM       = 0x408,  // Select drive number (0-3)
    PIFLP_CMD_DRVTYPE      = 0x40C,  // Get drive type (0=none, 1=DD, 2=HD)
    PIFLP_CMD_TRACK        = 0x410,  // Set/get current track
    PIFLP_CMD_SIDE         = 0x414,  // Set/get current side (0 or 1)
    PIFLP_CMD_SECTOR       = 0x418,  // Set/get current sector
    PIFLP_CMD_MOTOR        = 0x41C,  // Motor on/off control
    PIFLP_CMD_SEEK         = 0x420,  // Seek to track
    PIFLP_CMD_STATUS       = 0x424,  // Get drive status
    PIFLP_CMD_CHANGE       = 0x428,  // Disk change status
    PIFLP_CMD_PROTECT      = 0x42C,  // Write protect status
    PIFLP_CMD_FORMAT       = 0x430,  // Format track
    PIFLP_CMD_READ_ADDR1   = 0x434,  // Read data address (offset)
    PIFLP_CMD_READ_ADDR2   = 0x438,  // Read data address (length)
    PIFLP_CMD_READ_ADDR3   = 0x43C,  // Read data address (buffer ptr)
    PIFLP_CMD_READ_ADDR4   = 0x440,  // Read data address (actual)
    PIFLP_CMD_WRITE_ADDR1  = 0x444,  // Write data address (offset)
    PIFLP_CMD_WRITE_ADDR2  = 0x448,  // Write data address (length)
    PIFLP_CMD_WRITE_ADDR3  = 0x44C,  // Write data address (buffer ptr)
    PIFLP_CMD_WRITE_ADDR4  = 0x450,  // Write data address (actual)
    PIFLP_CMD_DRVNUMX      = 0x454,  // Extended drive select (with side)
    PIFLP_CMD_READ64       = 0x458,  // 64-bit read command
    PIFLP_CMD_WRITE64      = 0x45C,  // 64-bit write command
    PIFLP_CMD_USED_DMA     = 0x460,  // DMA used flag
    PIFLP_CMD_BLOCKSIZE    = 0x464,  // Sector size (512 for DD, 1024 for HD)
    PIFLP_CMD_TOTALTRACKS  = 0x468,  // Total tracks (80 for DD, 80 for HD)
    PIFLP_CMD_TOTALSECTORS = 0x46C,  // Total sectors per track (11 DD, 22 HD)
    PIFLP_CMD_TOTALSIDES   = 0x470,  // Total sides (1 or 2)
    PIFLP_CMD_CYLINDERS    = 0x474,  // Number of cylinders
    PIFLP_CMD_HEADS        = 0x478,  // Number of heads
    PIFLP_CMD_SECPERTRACK  = 0x47C,  // Sectors per track
    PIFLP_CMD_DISKINSERTED = 0x480,  // Disk inserted flag (ADF loaded)
    PIFLP_CMD_ADFNAME      = 0x484,  // ADF filename pointer (ARM side)
    PIFLP_CMD_EJECT        = 0x488,
    PIFLP_CMD_DRIVER       = 0x48C,
    
    // Per-unit block size registers (like SCSI)
    PIFLP_CMD_BLOCKSIZE0   = 0x4A0,
    PIFLP_CMD_BLOCKSIZE1   = 0x4A4,
    PIFLP_CMD_BLOCKSIZE2   = 0x4A8,
    PIFLP_CMD_BLOCKSIZE3   = 0x4AC,
    PIFLP_CMD_BLOCKSIZE4   = 0x4B0,
    PIFLP_CMD_BLOCKSIZE5   = 0x4B4,
    PIFLP_CMD_BLOCKSIZE6   = 0x4B8,
    PIFLP_CMD_BLOCKSIZE7   = 0x4BC,

    // Per-unit total blocks registers
    PIFLP_CMD_BLOCKS0      = 0x4C0,
    PIFLP_CMD_BLOCKS1      = 0x4C4,
    PIFLP_CMD_BLOCKS2      = 0x4C8,
    PIFLP_CMD_BLOCKS3      = 0x4CC,
    PIFLP_CMD_BLOCKS4      = 0x4D0,
    PIFLP_CMD_BLOCKS5      = 0x4D4,
    PIFLP_CMD_BLOCKS6      = 0x4D8,
    PIFLP_CMD_BLOCKS7      = 0x4DC,

    // Debug registers
    PIFLP_DBG_MSG          = 0x500,
    PIFLP_DBG_VAL1         = 0x510,
    PIFLP_DBG_VAL2         = 0x514,
    PIFLP_DBG_VAL3         = 0x518,
    PIFLP_DBG_VAL4         = 0x51C,
    PIFLP_DBG_VAL5         = 0x520,
};

// Floppy drive types
enum floppy_drive_type {
    FLOPPY_TYPE_NONE = 0,   // No drive / no ADF
    FLOPPY_TYPE_DD   = 1,   // 3.5" DD (880KB)
    FLOPPY_TYPE_HD   = 2,   // 3.5" HD (1760KB)
};

// Floppy error codes (mapped to AmigaOS TDERR_xxx)
enum floppy_errors {
    FLOPPY_ERR_OK          = 0,
    FLOPPY_ERR_TIMEOUT     = (1 << 7),
    FLOPPY_ERR_PARAM       = (1 << 6),
    FLOPPY_ERR_ADDRESS     = (1 << 5),
    FLOPPY_ERR_ERASESEQ    = (1 << 4),
    FLOPPY_ERR_CRC         = (1 << 3),
    FLOPPY_ERR_ILLEGAL     = (1 << 2),
    FLOPPY_ERR_ERASERES    = (1 << 1),
    FLOPPY_ERR_IDLE        = (1 << 0),
};

// Debug message IDs
enum floppy_dbg_msgs {
    DBG_FLP_INIT,
    DBG_FLP_OPENDEV,
    DBG_FLP_CLOSEDEV,
    DBG_FLP_CHS,
    DBG_FLP_IOCMD,
    DBG_FLP_CLEANUP,
    DBG_FLP_BEGINIO,
    DBG_FLP_ABORTIO,
    DBG_FLP_READ,
    DBG_FLP_WRITE,
    DBG_FLP_SEEK,
    DBG_FLP_MOTOR,
    DBG_FLP_FORMAT,
    DBG_FLP_ERR,
    DBG_FLP_UNHANDLED,
};

#endif // Z3660_FLOPPY_ENUMS_H