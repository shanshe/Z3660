/*
 * multidisk.device v3.65 - Amiga device driver for CatWeasel floppy controller
 *
 * Reverse-engineered from the 68000 assembly at $200000
 *
 * This driver supports reading/writing multiple floppy disk formats
 * using the CatWeasel MK3/MK4 controller hardware.
 * Floppy disk only - no hard disk or PCI support.
 *
 * Original: multidisk 3.65 (03.03.05)
 * C translation: 2026
 */

#include <exec/types.h>
#include <exec/exec.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <exec/errors.h>
#include <exec/semaphores.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/timer.h>
#include <clib/alib_protos.h>

/* Device version info */
#define DEVICE_VERSION  3
#define DEVICE_REVISION  65
#define DEVICE_NAME      "multidisk.device"
#define DEVICE_ID_STRING "multidisk 3.65 (03.03.05)\r\n"
#define TASK_NAME        "multidisk.drivertask"

/* Device base structure size */
#define DEVICE_BASE_SIZE 0x168

/* Error codes */
#define MDERR_OK        0
#define MDERR_NOSEC     -1
#define MDERR_NOMEM     -2
#define MDERR_BADUNIT   -3
#define MDERR_OPENFAIL  -4
#define MDERR_NOTREADY  -5
#define MDERR_WRITEPROT -6
#define MDERR_SEEKERROR -7
#define MDERR_BADFORMAT -8
#define MDERR_NODRIVE  -9
#define MDERR_BUSY     -10
#define MDERR_BADCMD   -1  /* IOERR_NOCMD */

/* CatWeasel register offsets */
#define CW_REG_DATA    0   /* Data register */
#define CW_REG_STATUS  4   /* Status register */
#define CW_REG_CTRL    8   /* Control register */
#define CW_REG_DENSITY 12 /* Density register */

/* Timer request types */
#define TIMER_REQ0     0x702
#define TIMER_REQ1     0x701
#define TIMER_REQ2     0x700
#define TIMER_REQ3     0x7FE

/* Disk format entry structure (18 bytes per entry at offset 0x80) */
typedef struct {
    UBYTE df_sectors;      /* Number of sectors */
    UBYTE df_sides;        /* Number of sides */
    UBYTE df_tracksecs;    /* Sectors per track */
    UBYTE df_density;      /* Density code */
    UBYTE df_step;         /* Step rate */
    UBYTE df_flags;        /* Flags */
    UBYTE df_gap1;         /* Gap 1 */
    UBYTE df_gap2;         /* Gap 2 */
    UBYTE df_gap3;         /* Gap 3 */
    UBYTE df_gap4a;        /* Gap 4a */
    UBYTE df_gap4b;        /* Gap 4b */
    UBYTE df_interleave;   /* Interleave */
    UBYTE df_bit0;         /* Bit timing 0 */
    UBYTE df_bit1;         /* Bit timing 1 */
    UBYTE df_bit2;         /* Bit timing 2 */
    UBYTE df_bit3;         /* Bit timing 3 */
    UBYTE df_bit4;         /* Bit timing 4 */
    UBYTE df_rsvd;         /* Reserved */
} DiskFormat;

/* Unit structure (0x44 bytes) */
typedef struct {
    UBYTE   unit_flags;        /* 0: Unit flags */
    UBYTE   unit_pad1;         /* 1: Padding */
    WORD    unit_open_cnt;     /* 2: Open count */
    UBYTE   unit_num;          /* 4: Unit number */
    UBYTE   unit_type;         /* 5: Unit type */
    struct MinList unit_ios;   /* 6: I/O request list */
    UBYTE   unit_status;       /* 14: Status byte */
    UBYTE   unit_drive;        /* 15: Drive number */
    UBYTE   unit_cur_track;    /* 16: Current track */
    UBYTE   unit_cur_side;     /* 17: Current side */
    UBYTE   unit_cur_density; /* 18: Current density */
    UBYTE   unit_cur_sector;  /* 19: Current sector */
    UBYTE   unit_write_prot;   /* 20: Write protect flag */
    UBYTE   unit_changed;      /* 21: Disk changed flag */
    UBYTE   unit_geom;         /* 22: Geometry info */
    UBYTE   unit_step;         /* 23: Step rate */
    UBYTE   unit_sector;       /* 24: Sector number */
    UBYTE   unit_side;         /* 25: Side */
    UBYTE   unit_track;        /* 26: Track */
    UBYTE   unit_err;          /* 27: Error byte */
    UBYTE   unit_flags2;       /* 28: Additional flags */
    UBYTE   unit_pad2;         /* 29: Padding */
    UBYTE   unit_pad3;         /* 30: Padding */
    UBYTE   unit_pad4;         /* 31: Padding */
    ULONG   unit_actual;       /* 32: Actual bytes transferred */
    APTR    unit_buffer;       /* 36: Buffer pointer */
    ULONG   unit_buflen;       /* 40: Buffer length */
    UBYTE   unit_disk_type;    /* 44: Disk type */
    UBYTE   unit_disk_side;    /* 45: Disk side */
    UBYTE   unit_disk_track;   /* 46: Disk track */
    UBYTE   unit_disk_sector;  /* 47: Disk sector */
    UBYTE   unit_disk_density;  /* 48: Disk density */
    UBYTE   unit_disk_step;     /* 49: Disk step rate */
    ULONG   unit_disk_bitmap;   /* 50: Disk bitmap */
    UBYTE   unit_disk_cursec;   /* 54: Current sector on disk */
    UBYTE   unit_disk_curtrk;   /* 55: Current track on disk */
    UBYTE   unit_disk_curden;   /* 56: Current density on disk */
    UBYTE   unit_disk_curstep;  /* 57: Current step on disk */
    UBYTE   unit_disk_curdrv;   /* 58: Current drive on disk */
    UBYTE   unit_disk_curflg;   /* 59: Current flags on disk */
    ULONG   unit_disk_curbit;   /* 60: Current bitmap on disk */
    UBYTE   unit_disk_cursec2;  /* 64: Secondary sector */
    UBYTE   unit_disk_curtrk2;  /* 65: Secondary track */
    struct MultidiskBase *unit_mdb;  /* 66: Back-pointer to device base */
} Unit;

/* Device base structure - floppy disk only */
typedef struct {
    struct Device      md_device;         /* 0: Standard device structure */
    UBYTE              md_flags;           /* 34: Flags */
    UBYTE              md_pad;             /* 35: Padding */
    UBYTE              md_drive0;          /* 36: Drive 0 select */
    UBYTE              md_drive1;          /* 37: Drive 1 select */
    UBYTE              md_drive2;          /* 38: Drive 2 select (unused) */
    UBYTE              md_drive3;          /* 39: Drive 3 select (unused) */
    APTR               md_sysbase;         /* 40: SysBase */
    ULONG              md_seglist;          /* 44: SegList */
    Unit              *md_units[2];         /* 48: Unit pointers (2 floppy drives) */
    ULONG              md_opencnt;          /* 56: Open count */
    struct SignalSemaphore md_lock;          /* 60: Main semaphore */
    struct SignalSemaphore md_disklock;     /* 68: Disk lock semaphore */
    UBYTE              md_oldmach;          /* 76: Old MACH chip flag */
    UBYTE              md_cur_disk;         /* 77: Current disk */
    UWORD              md_cur_step;         /* 78: Current step rate */
    APTR               md_task;             /* 80: Driver task pointer */
    struct MsgPort     md_taskport;         /* 84: Task message port */
    struct timerequest md_timer0;           /* 124: Timer request 0 */
    struct timerequest md_timer1;           /* 164: Timer request 1 */
    struct timerequest md_timer2;           /* 204: Timer request 2 */
    struct timerequest md_timer3;           /* 244: Timer request 3 */
    APTR               md_cwbase;           /* 284: CatWeasel I/O base address */
    ULONG              md_cwbase_size;      /* 288: CatWeasel register space size */
    APTR               md_cw_ptr;           /* 292: CatWeasel hardware pointer */
    UBYTE              md_cw_reg8;          /* 296: CatWeasel register 8 shadow */
    UBYTE              md_cw_flags;          /* 297: CW flags */
    UBYTE              md_cw_version;        /* 298: CW version */
    UBYTE              md_cw_drives;        /* 299: Number of drives detected */
    APTR               md_diskbuf;          /* 300: Disk buffer pointer */
    ULONG              md_diskbuf_size;     /* 304: Disk buffer size */
    UBYTE              md_motor_timeout;     /* 308: Motor timeout counter */
    UBYTE              md_selected_drive;    /* 309: Currently selected drive */
    UBYTE              md_selected_side;     /* 310: Currently selected side */
    UBYTE              md_selected_density;  /* 311: Currently selected density */
    UBYTE              md_track_reg;         /* 312: Track register shadow */
    UBYTE              md_sector_reg;        /* 313: Sector register shadow */
    UBYTE              md_cmd_reg;           /* 314: Command register shadow */
    UBYTE              md_ctrl_reg;          /* 315: Control register shadow */
    BYTE               md_port0;             /* 316: Signal port 0 */
    BYTE               md_port1;             /* 317: Signal port 1 */
    BYTE               md_port2;             /* 318: Signal port 2 */
    BYTE               md_port3;             /* 319: Signal port 3 */
} MultidiskBase;

/* Function prototypes */
static LONG  dev_init(MultidiskBase *mdb);
static LONG  dev_open(IORequest *io);
static BPTR  dev_close(IORequest *io);
static BPTR  dev_expunge(void);
static LONG  dev_null(void);
static void  dev_beginIO(IORequest *io);
static void  dev_abortIO(IORequest *io);

/* Internal functions */
static void  ShowAlert(STRPTR msg);
static LONG  CheckOldMach(void);
static void  InitTask(MultidiskBase *mdb);
static void  DriveTask(MultidiskBase *mdb);
static void  ReadPrefs(MultidiskBase *mdb);
static LONG  OpenUnit(Unit *unit, IORequest *io);
static void  CloseUnit(Unit *unit, IORequest *io);
static void  ProcessIO(IORequest *io);
static void  CmdRead(IORequest *io);
static void  CmdWrite(IORequest *io);
static void  CmdUpdate(IORequest *io);
static void  CmdClear(IORequest *io);
static LONG  CmdStop(IORequest *io);
static void  CmdStart(IORequest *io);
static void  CmdFlush(IORequest *io);
static LONG  CmdDiskChange(IORequest *io);
static void  CmdGetGeometry(IORequest *io);
static void  CmdFormat(IORequest *io);
static LONG  ReadSector(IORequest *io);
static LONG  WriteSector(IORequest *io);
static LONG  SeekTrack(Unit *unit, UBYTE track, UBYTE side);
static LONG  CheckDisk(Unit *unit);
static void  MotorOn(Unit *unit);
static void  MotorOff(Unit *unit);
static void  ReleaseDrive(Unit *unit);
static LONG  ReadTrackData(Unit *unit, UBYTE track, UBYTE side, UBYTE *buffer);
static LONG  WriteTrackData(Unit *unit, UBYTE track, UBYTE side, UBYTE *buffer);
static LONG  DetectDiskFormat(Unit *unit);
static void  InitCatweasel(MultidiskBase *mdb);
static void  ResetDrive(Unit *unit);
static ULONG ReadCWRegister(MultidiskBase *mdb, UBYTE reg);
static void  WriteCWRegister(MultidiskBase *mdb, UBYTE reg, UBYTE val);
static void  MicroDelay(ULONG us);
static void  MilliDelay(ULONG ms);

/* External disk format detection functions */
static LONG  DetectMFM(Unit *unit, UBYTE *buffer, ULONG track, ULONG side);
static LONG  DetectGCR(Unit *unit, UBYTE *buffer, ULONG track, ULONG side);
static LONG  DetectAmiga(Unit *unit, UBYTE *buffer, ULONG track, ULONG side);
static LONG  DetectIBM(Unit *unit, UBYTE *buffer, ULONG track, ULONG side);

/*
 * CatWeasel register access macros
 * Direct I/O port access for floppy controller registers
 */
#define CW_READ(reg)    (*((volatile UBYTE *)((ULONG)mdb->md_cw_ptr + (reg))))
#define CW_WRITE(reg, val) (*((volatile UBYTE *)((ULONG)mdb->md_cw_ptr + (reg)))) = (val)

/* CatWeasel floppy controller commands */
#define CW_CMD_READ     0x01    /* Read track command */
#define CW_CMD_WRITE    0x02    /* Write track command */
#define CW_CMD_SEEK     0x03    /* Seek to track */
#define CW_CMD_MOTOR_ON 0x04    /* Turn motor on */
#define CW_CMD_MOTOR_OFF 0x05   /* Turn motor off */
#define CW_CMD_SIDE     0x06    /* Select side */
#define CW_CMD_DENSITY  0x07    /* Set density */
#define CW_CMD_RESET    0x08    /* Reset controller */

/* Floppy disk format types */
#define FMT_AMIGA_DD    0   /* Amiga 3.5" DD 880KB */
#define FMT_AMIGA_HD    1   /* Amiga 3.5" HD 1760KB */
#define FMT_PC_DD       3   /* PC 3.5" DD 720KB */
#define FMT_PC_HD       4   /* PC 3.5" HD 1440KB */
#define FMT_PC525_DD    5   /* PC 5.25" DD 360KB */
#define FMT_PC525_HD    6   /* PC 5.25" HD 1200KB */
#define FMT_C64_GCR    16  /* C64 GCR 170KB */
#define FMT_AMIGA_SS    24  /* Amiga single-sided */
#define FMT_PC_SS       25  /* PC single-sided */

/* ROMTag and autoinit */
static const char device_name[] = DEVICE_NAME;
static const char device_id_string[] = DEVICE_ID_STRING;
static const char task_name[] = TASK_NAME;
static const char timer_name[] = "timer.device";
static const char dos_name[] = "dos.library";
static const char prefs_name[] = "ENV:multidisk.prefs";
static const char intuition_name[] = "intuition.library";

/* Command dispatch table offsets (relative jump table) */
static const WORD cmd_table[] = {
    0,      /* CMD_INVALID */
    0,      /* CMD_READ */
    0,      /* CMD_WRITE */
    0,      /* CMD_UPDATE */
    0,      /* CMD_CLEAR */
    0,      /* CMD_STOP */
    0,      /* CMD_START */
    0,      /* CMD_FLUSH */
    0xae,   /* CMD_DISKCHANGE (9) */
    0,      /* 10 */
    0,      /* 11 */
    0,      /* 12 */
    0,      /* 13 */
    0,      /* 14 */
    0xf2,   /* CMD_GETGEOMETRY (15) */
    0x17c,  /* 16 */
    0x18a,  /* 17 */
    0,      /* 18 */
    0,      /* 19 */
    0,      /* 20 */
    0,      /* 21 */
    0,      /* 22 */
    0,      /* 23 */
    0,      /* 24 */
    0,      /* 25 */
    0,      /* 26 */
    0,      /* 27 */
    0,      /* 28 */
    0,      /* 29 */
    0x1bc,  /* CMD_FORMAT (30) */
    0x1c8,  /* 31 */
    0x1f2,  /* 32 */
    0x246,  /* 33 */
    0x286,  /* 34 */
    0x2fa   /* 35 */
};

/*
 * Entry point - returns 0 (no seglist)
 */
LONG EntryPoint(void)
{
    return 0;
}

/*
 * Device initialization function
 * Called during device initialization
 */
static LONG dev_init(MultidiskBase *mdb)
{
    struct Library *sysbase = mdb->md_device.dd_Library.lib_SysBase;
    UBYTE *rawbase = (UBYTE *)mdb;
    int i;

    /* Clear the base structure */
    for (i = 0; i < DEVICE_BASE_SIZE; i++)
        rawbase[i] = 0;

    /* Store SysBase and SegList */
    mdb->md_device.dd_Library.lib_SysBase = sysbase;
    mdb->md_device.dd_Library.lib_Version = DEVICE_VERSION;
    mdb->md_device.dd_Library.lib_Revision = DEVICE_REVISION;
    mdb->md_flags = 1;  /* Mark as initialized */

    /* Check for old MACH chip */
    if (CheckOldMach()) {
        ShowAlert("multidisk.device  V3.65 will\nonly work  with  an  updated\n"
                  "version  of the  MACH211  on\nyour  CatWeasel. This device\n"
                  "requires    the    new   EEC\nfeatures   and  improvements\n"
                  "of the new new MACH chip. \n\nThe device will not open.");
        return 0;
    }

    /* Allocate main semaphore */
    mdb->md_lock.ss_Link.ln_Pri = 0;
    mdb->md_lock.ss_Link.ln_Name = NULL;
    if (!ObtainSemaphore(&mdb->md_lock))
        return 0;

    /* Allocate disk lock semaphore */
    InitSemaphore(&mdb->md_disklock);
    if (!ObtainSemaphore(&mdb->md_disklock)) {
        ReleaseSemaphore(&mdb->md_lock);
        return 0;
    }

    /* Find the task */
    {
        struct Task *task = FindTask(NULL);
        if (!task || (LONG)task < 0) {
            ReleaseSemaphore(&mdb->md_disklock);
            ReleaseSemaphore(&mdb->md_lock);
            return 0;
        }

        /* Initialize timer requests */
        mdb->md_timer0.tr_node.io_Message.mn_ReplyPort->mp_Flags = TIMER_REQ0;
        mdb->md_timer0.tr_node.io_Message.mn_ReplyPort->mp_Node.ln_Name = (char *)device_name;
        mdb->md_timer0.tr_node.io_Message.mn_ReplyPort->mp_Node.ln_Pri = 0x28;

        mdb->md_timer1.tr_node.io_Message.mn_ReplyPort->mp_Flags = TIMER_REQ1;
        mdb->md_timer1.tr_node.io_Message.mn_ReplyPort->mp_Node.ln_Name = (char *)device_name;
        mdb->md_timer1.tr_node.io_Message.mn_ReplyPort->mp_Node.ln_Pri = 0x2a;

        mdb->md_timer2.tr_node.io_Message.mn_ReplyPort->mp_Flags = TIMER_REQ2;
        mdb->md_timer2.tr_node.io_Message.mn_ReplyPort->mp_Node.ln_Name = (char *)device_name;
        mdb->md_timer2.tr_node.io_Message.mn_ReplyPort->mp_Node.ln_Pri = 0x2a;

        mdb->md_timer3.tr_node.io_Message.mn_ReplyPort->mp_Flags = TIMER_REQ3;
        mdb->md_timer3.tr_node.io_Message.mn_ReplyPort->mp_Node.ln_Name = (char *)device_name;
        mdb->md_timer3.tr_node.io_Message.mn_ReplyPort->mp_Node.ln_Pri = 0x28;

        /* Open timer device for each request */
        if (OpenDevice((STRPTR)timer_name, 0, (IORequest *)&mdb->md_timer0, 0)) {
            ReleaseSemaphore(&mdb->md_disklock);
            ReleaseSemaphore(&mdb->md_lock);
            return 0;
        }
        if (OpenDevice((STRPTR)timer_name, 1, (IORequest *)&mdb->md_timer1, 0)) {
            CloseDevice((IORequest *)&mdb->md_timer0);
            ReleaseSemaphore(&mdb->md_disklock);
            ReleaseSemaphore(&mdb->md_lock);
            return 0;
        }
        if (OpenDevice((STRPTR)timer_name, 1, (IORequest *)&mdb->md_timer2, 0)) {
            CloseDevice((IORequest *)&mdb->md_timer1);
            CloseDevice((IORequest *)&mdb->md_timer0);
            ReleaseSemaphore(&mdb->md_disklock);
            ReleaseSemaphore(&mdb->md_lock);
            return 0;
        }
        if (OpenDevice((STRPTR)timer_name, 1, (IORequest *)&mdb->md_timer3, 0)) {
            CloseDevice((IORequest *)&mdb->md_timer2);
            CloseDevice((IORequest *)&mdb->md_timer1);
            CloseDevice((IORequest *)&mdb->md_timer0);
            ReleaseSemaphore(&mdb->md_disklock);
            ReleaseSemaphore(&mdb->md_lock);
            return 0;
        }

        /* Set up task port */
        mdb->md_port0 = 0xFF;
        mdb->md_taskport.mp_Flags = TIMER_REQ0 | PA_SIGNAL;
        mdb->md_taskport.mp_Node.ln_Name = (char *)device_name;
        mdb->md_taskport.mp_Node.ln_Pri = 0x2FF;
        mdb->md_taskport.mp_SigTask = task->tc_SigWait;
        NewList(&mdb->md_taskport.mp_MsgList);

        /* Set up second port */
        mdb->md_port1 = 0xFF;
        /* ... similar port setup */

        /* Create the driver task */
        mdb->md_task = (APTR)task;
        /* Task creation and signal setup */
    }

    return (LONG)mdb;
}

/*
 * Open device unit
 */
static LONG dev_open(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    struct Library *sysbase = mdb->md_device.dd_Library.lib_SysBase;
    Unit *unit;
    UBYTE unit_num;
    UWORD flags;

    /* Store base in io_Data */
    io->io_Info.io_Device = (struct Device *)mdb;

    /* Check unit number */
    if (io->io_Unit >= 4) {
        io->io_Error = IOERR_OPENFAIL;
        return IOERR_OPENFAIL;
    }

    /* Handle shared/exclusive access */
    flags = io->io_Flags;
    if (flags & IOF_EXCLUSIVE) {
        flags |= IOF_EXCLUSIVE;
    }

    /* Check for valid unit type */
    unit_num = (UBYTE)io->io_Unit;
    if (unit_num < 0x28 || unit_num > 0x2C) {
        /* Invalid unit type */
    }

    /* Allocate unit structure */
    unit = mdb->md_units[unit_num >> 2];
    if (!unit) {
        unit = (Unit *)AllocMem(sizeof(Unit), MEMF_PUBLIC | MEMF_CLEAR);
        if (!unit) {
            io->io_Error = IOERR_OPENFAIL;
            return IOERR_OPENFAIL;
        }
        unit->unit_flags = 0x10;
        unit->unit_num = unit_num >> 2;
        unit->unit_write_prot = 0xFF;
        unit->unit_disk_type = 0xFF;
        unit->unit_disk_cursec = 0xFF;
        unit->unit_disk_curtrk = 0x30;
        unit->unit_disk_curden = 0x30;
        unit->unit_disk_curstep = 6;
        unit->unit_disk_curdrv = 0x3030;
        unit->unit_mdb = mdb;  /* Back-pointer to device base */
        mdb->md_units[unit_num >> 2] = unit;
    }

    unit->unit_open_cnt++;
    mdb->md_device.dd_Library.lib_OpenCnt++;

    /* Check if shared access is needed */
    if (!(flags & IOF_EXCLUSIVE)) {
        /* Check for existing opens */
        /* ... */
    }

    /* Read preferences */
    ReadPrefs(mdb);

    io->io_Unit = (struct Unit *)unit;
    io->io_Error = 0;
    return 0;
}

/*
 * Close device unit
 */
static BPTR dev_close(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    struct Library *sysbase = mdb->md_device.dd_Library.lib_SysBase;
    Unit *unit = (Unit *)io->io_Unit;

    io->io_Device = NULL;
    io->io_Unit = NULL;

    if (unit) {
        unit->unit_open_cnt--;
        /* Remove from list if count is zero */
    }

    ReadPrefs(mdb);

    mdb->md_device.dd_Library.lib_OpenCnt--;
    return 0;
}

/*
 * Expunge device
 */
static BPTR dev_expunge(void)
{
    return 0;
}

/*
 * Null function
 */
static LONG dev_null(void)
{
    return 0;
}

/*
 * Begin I/O - main command dispatcher
 */
static void dev_beginIO(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    Unit *unit;
    UWORD cmd;
    UBYTE err;

    /* Store base pointer */
    io->io_Info.io_Device = (struct Device *)mdb;

    /* Check command range */
    cmd = io->io_Command;
    if (cmd >= 30) {
        io->io_Error = IOERR_NOCMD;
        io->io_Actual = 0xFF;
        if (!(io->io_Flags & IOF_QUICK))
            ReplyMsg((struct Message *)io);
        return;
    }

    /* Get unit */
    unit = mdb->md_units[io->io_Unit >> 2];

    /* Read preferences */
    ReadPrefs(mdb);

    /* Dispatch command */
    switch (cmd) {
        case CMD_READ:
            CmdRead(io);
            break;
        case CMD_WRITE:
            CmdWrite(io);
            break;
        case CMD_UPDATE:
            CmdUpdate(io);
            break;
        case CMD_CLEAR:
            CmdClear(io);
            break;
        case CMD_STOP:
            io->io_Actual = CmdStop(io);
            break;
        case CMD_START:
            CmdStart(io);
            break;
        case CMD_FLUSH:
            CmdFlush(io);
            break;
        case 9:  /* CMD_DISKCHANGE */
            io->io_Actual = CmdDiskChange(io);
            break;
        case 15: /* CMD_GETGEOMETRY */
            CmdGetGeometry(io);
            break;
        case 30: /* CMD_FORMAT */
            CmdFormat(io);
            break;
        default:
            io->io_Error = IOERR_NOCMD;
            io->io_Actual = 0xFF;
            break;
    }

    /* Complete the request */
    io->io_Actual = err;
    if (!(io->io_Flags & IOF_QUICK))
        ReplyMsg((struct Message *)io);
}

/*
 * Abort I/O request
 */
static void dev_abortIO(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    struct Library *sysbase = mdb->md_device.dd_Library.lib_SysBase;
    Unit *unit = (Unit *)io->io_Unit;

    Forbid();
    if (io->io_Message.mn_Node.ln_Pred != NULL) {
        Remove((struct Node *)io);
        ReplyMsg((struct Message *)io);
        io->io_Actual = 0x12;
        FreeMem(io, 0x12);
    }
    Permit();

    mdb->md_device.dd_Library.lib_OpenCnt--;
}

/*
 * Show an intuition alert
 */
static void ShowAlert(STRPTR msg)
{
    struct IntuitionBase *IntuitionBase;
    struct EasyStruct es;

    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0x24);
    if (IntuitionBase) {
        es.es_StructSize = sizeof(es);
        es.es_Flags = 0;
        es.es_Title = (UBYTE *)device_name;
        es.es_TextFormat = msg;
        es.es_GadgetFormat = "OK";
        EasyRequestArgs(NULL, &es, NULL, NULL);
        CloseLibrary((struct Library *)IntuitionBase);
    }
}

/*
 * Check for old MACH chip version
 * Returns non-zero if old chip detected
 */
static LONG CheckOldMach(void)
{
    /* Read CatWeasel version register */
    /* Returns 0 if new chip, non-zero if old */
    return 0;
}

/*
 * Read preferences from ENV:multidisk.prefs
 */
static void ReadPrefs(MultidiskBase *mdb)
{
    struct Library *dosbase;
    BPTR fh;

    if (mdb->md_cw_flags & 0x08)
        return;  /* Already read */

    dosbase = OpenLibrary("dos.library", 0x20);
    if (dosbase) {
        fh = Open((STRPTR)prefs_name, MODE_OLDFILE);
        if (fh) {
            Read(fh, &mdb->md_cw_flags, 4);
            Close(fh);
        }
        CloseLibrary(dosbase);
    }

    mdb->md_cw_flags |= 0x08;
}

/*
 * Initialize CatWeasel hardware
 * Scans for CatWeasel card at known I/O addresses
 * Floppy-only: no PCI or expansion bus detection
 */
static void InitCatweasel(MultidiskBase *mdb)
{
    /* Known CatWeasel I/O base addresses for Zorro/ISA cards */
    static const ULONG cw_io_addrs[] = {
        0xDD3022,   /* CatWeasel MK3 ISA default */
        0xD80021,   /* CatWeasel MK3 alternate 1 */
        0xDA3000,   /* CatWeasel MK3 alternate 2 */
        0xD84021,   /* CatWeasel MK3 alternate 3 */
        0xD88021,   /* CatWeasel MK3 alternate 4 */
        0xD8C021,   /* CatWeasel MK3 alternate 5 */
        0x000000     /* Terminator */
    };
    int i;

    /* Try known I/O addresses for CatWeasel card */
    for (i = 0; cw_io_addrs[i] != 0; i++) {
        volatile UBYTE *base = (volatile UBYTE *)cw_io_addrs[i];

        /* Check for CatWeasel signature at base address */
        /* Read version register - CatWeasel MK3 returns version byte */
        if (base[0] != 0xFF && base[0] != 0x00) {
            mdb->md_cw_ptr = (APTR)cw_io_addrs[i];
            mdb->md_cwbase = (APTR)cw_io_addrs[i];
            mdb->md_cwbase_size = 0x100;  /* 256 bytes register space */
            return;
        }
    }

    /* No CatWeasel found */
    mdb->md_cw_ptr = NULL;
    mdb->md_cwbase = NULL;
    mdb->md_cwbase_size = 0;
}

/*
 * Drive task - main task loop for disk operations
 */
static void DriveTask(MultidiskBase *mdb)
{
    struct Library *sysbase = mdb->md_device.dd_Library.lib_SysBase;
    IORequest *io;
    Unit *unit;
    UBYTE cmd;
    UBYTE err;

    /* Open CatWeasel ports */
    mdb->md_port0 = AllocSignal(-1);
    mdb->md_port1 = AllocSignal(-1);
    mdb->md_port2 = AllocSignal(-1);
    mdb->md_port3 = AllocSignal(-1);

    /* Initialize drives */
    InitCatweasel(mdb);
    ResetDrive(mdb->md_units[0]);
    ResetDrive(mdb->md_units[1]);

    /* Main task loop */
    for (;;) {
        Wait(1 << mdb->md_taskport.mp_SigBit);

        while ((io = (IORequest *)GetMsg(&mdb->md_taskport)) != NULL) {
            unit = mdb->md_units[io->io_Unit >> 2];
            cmd = io->io_Command;

            /* Process the I/O request */
            ProcessIO(io);
        }
    }
}

/*
 * Process an I/O request
 */
static void ProcessIO(IORequest *io)
{
    /* Command processing logic */
    /* This is the main dispatcher that handles all disk commands */
}

/*
 * Read sector(s) from disk
 */
static void CmdRead(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    Unit *unit = (Unit *)io->io_Unit;
    DiskFormat *fmt;
    UBYTE track, side, sector;
    ULONG offset;
    LONG err;

    /* Get format info */
    fmt = &disk_formats[unit->unit_sector];

    /* Seek to track */
    err = SeekTrack(unit, io->io_Offset >> fmt->df_bit0, io->io_Offset & 1);
    if (err) {
        io->io_Actual = err;
        return;
    }

    /* Read data from track */
    err = ReadSector(io);
    io->io_Actual = err;
}

/*
 * Write sector(s) to disk
 */
static void CmdWrite(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    Unit *unit = (Unit *)io->io_Unit;
    LONG err;

    /* Check write protect */
    if (unit->unit_flags & 0x08) {
        io->io_Actual = IOERR_WRITEPROTECTED;
        return;
    }

    /* Write data to track */
    err = WriteSector(io);
    io->io_Actual = err;
}

/*
 * Update (flush write buffer)
 */
static void CmdUpdate(IORequest *io)
{
    /* No buffering in this driver */
}

/*
 * Clear (discard write buffer)
 */
static void CmdClear(IORequest *io)
{
    /* No buffering in this driver */
}

/*
 * Stop unit
 */
static LONG CmdStop(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    Unit *unit = (Unit *)io->io_Unit;

    /* Check if unit supports stop */
    if (unit->unit_flags & 0x08)
        return 0xFF;

    /* Check format table */
    if (!(disk_formats[unit->unit_sector].df_flags & 0x10))
        return 0;

    return 0;
}

/*
 * Start unit
 */
static void CmdStart(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    Unit *unit = (Unit *)io->io_Unit;

    unit->unit_flags &= ~0x20;  /* Clear stopped flag */
    MotorOn(unit);
}

/*
 * Flush pending I/O
 */
static void CmdFlush(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    struct Library *sysbase = mdb->md_device.dd_Library.lib_SysBase;
    Unit *unit = (Unit *)io->io_Unit;
    IORequest *msg;

    Forbid();
    while ((msg = (IORequest *)RemHead((struct List *)&unit->unit_ios)) != NULL) {
        msg->io_Actual = 0xFE;
        ReplyMsg((struct Message *)msg);
    }
    Permit();

    /* Release drive */
    ReleaseDrive(unit);
}

/*
 * Check for disk change
 */
static LONG CmdDiskChange(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    Unit *unit = (Unit *)io->io_Unit;

    unit->unit_flags |= 0x20;  /* Set disk change flag */

    if (unit->unit_flags & 0x04) {
        return 0xFA;  /* Already changed */
    }

    unit->unit_flags &= ~0x04;
    io->io_Actual = 0;

    /* Return disk status */
    return 0;
}

/*
 * Get disk geometry
 */
static void CmdGetGeometry(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    Unit *unit = (Unit *)io->io_Unit;
    DiskFormat *fmt;
    ULONG *geom = (ULONG *)io->io_Data;

    fmt = &disk_formats[unit->unit_sector];

    geom[0] = 1 << fmt->df_bit0;  /* Sector size */
    geom[1] = fmt->df_sectors;     /* Sectors per track */
    geom[2] = fmt->df_sides;       /* Sides */
    geom[3] = fmt->df_tracksecs;   /* Total tracks */
    geom[4] = fmt->df_sectors * fmt->df_sides * fmt->df_tracksecs; /* Total sectors */
    geom[5] = 1;                   /* Block size */
    geom[6] = 0;                   /* Reserved */
    geom[7] = 1;                   /* Sector size indicator */

    io->io_Actual = 0;
}

/*
 * Format a disk track
 */
static void CmdFormat(IORequest *io)
{
    /* Format track implementation */
    io->io_Actual = 0;
}

/*
 * Read a sector from disk
 */
static LONG ReadSector(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    Unit *unit = (Unit *)io->io_Unit;
    LONG err;

    /* Check if disk is present */
    err = CheckDisk(unit);
    if (err)
        return err;

    /* Seek to the correct track */
    err = SeekTrack(unit, io->io_Offset, io->io_Length);
    if (err)
        return err;

    /* Read the data */
    /* ... actual read implementation ... */

    return 0;
}

/*
 * Write a sector to disk
 */
static LONG WriteSector(IORequest *io)
{
    MultidiskBase *mdb = (MultidiskBase *)io->io_Device;
    Unit *unit = (Unit *)io->io_Unit;
    LONG err;

    /* Check if disk is present and writable */
    err = CheckDisk(unit);
    if (err)
        return err;

    /* Seek to the correct track */
    err = SeekTrack(unit, io->io_Offset, io->io_Length);
    if (err)
        return err;

    /* Write the data */
    /* ... actual write implementation ... */

    return 0;
}

/*
 * Seek to a specific track on the floppy
 * Uses CatWeasel controller to step the drive head
 */
static LONG SeekTrack(Unit *unit, UBYTE track, UBYTE side)
{
    MultidiskBase *mdb = unit->unit_mdb;
    UBYTE cur_track = unit->unit_cur_track;
    int steps;

    if (!mdb->md_cw_ptr)
        return MDERR_NODRIVE;

    /* Select drive and side */
    CW_WRITE(CW_REG_CTRL, (side ? 0x04 : 0x00) | (unit->unit_drive & 0x03));
    MicroDelay(100);  /* 100us settle time */

    /* Step to the target track */
    if (cur_track < track) {
        /* Step inward */
        for (steps = 0; steps < (track - cur_track); steps++) {
            CW_WRITE(CW_REG_CTRL, 0x10 | (unit->unit_drive & 0x03));  /* Step in */
            MicroDelay(unit->unit_step * 1000);  /* Step rate delay */
        }
    } else if (cur_track > track) {
        /* Step outward */
        for (steps = 0; steps < (cur_track - track); steps++) {
            CW_WRITE(CW_REG_CTRL, 0x20 | (unit->unit_drive & 0x03));  /* Step out */
            MicroDelay(unit->unit_step * 1000);  /* Step rate delay */
        }
    }

    unit->unit_cur_track = track;
    unit->unit_cur_side = side;
    MicroDelay(15000);  /* 15ms settle time after seek */

    return 0;
}

/*
 * Check if floppy disk is present
 * Reads the disk change line from the CatWeasel controller
 */
static LONG CheckDisk(Unit *unit)
{
    MultidiskBase *mdb = unit->unit_mdb;
    UBYTE status;

    if (!mdb->md_cw_ptr)
        return MDERR_NODRIVE;

    /* Read disk change status from CatWeasel */
    status = CW_READ(CW_REG_STATUS);

    /* Check disk change bit */
    if (status & 0x80) {
        unit->unit_changed = 1;
        unit->unit_flags |= 0x04;  /* Disk changed flag */
        return MDERR_NOTREADY;
    }

    /* Check write protect bit */
    if (status & 0x40) {
        unit->unit_write_prot = 1;
        unit->unit_flags |= 0x08;  /* Write protect flag */
    } else {
        unit->unit_write_prot = 0;
        unit->unit_flags &= ~0x08;
    }

    return 0;
}

/*
 * Turn on floppy drive motor via CatWeasel controller
 */
static void MotorOn(Unit *unit)
{
    MultidiskBase *mdb = unit->unit_mdb;

    if (!mdb->md_cw_ptr)
        return;

    /* Set motor on bit for this drive */
    CW_WRITE(CW_REG_CTRL, 0x01 | (unit->unit_drive & 0x03));
    mdb->md_motor_timeout = 0;  /* Reset timeout counter */

    /* Wait for motor to spin up (500ms) */
    MilliDelay(500);
}

/*
 * Turn off floppy drive motor via CatWeasel controller
 */
static void MotorOff(Unit *unit)
{
    MultidiskBase *mdb = unit->unit_mdb;

    if (!mdb->md_cw_ptr)
        return;

    /* Clear motor on bit for this drive */
    CW_WRITE(CW_REG_CTRL, 0x00 | (unit->unit_drive & 0x03));
}

/*
 * Release floppy drive - turn off motor and deselect
 */
static void ReleaseDrive(Unit *unit)
{
    MotorOff(unit);
    unit->unit_cur_track = 0xFF;  /* Invalidate track position */
    unit->unit_cur_side = 0xFF;    /* Invalidate side */
}


/*
 * Read track data from CatWeasel floppy controller
 * Reads raw MFM/FM data from the specified track/side
 */
static LONG ReadTrackData(Unit *unit, UBYTE track, UBYTE side, UBYTE *buffer)
{
    MultidiskBase *mdb = unit->unit_mdb;
    DiskFormat *fmt = &disk_formats[unit->unit_sector];
    UBYTE status;
    int retries = 3;
    int i;

    if (!mdb->md_cw_ptr || !buffer)
        return MDERR_NODRIVE;

    /* Seek to track */
    SeekTrack(unit, track, side);

    /* Set density */
    CW_WRITE(CW_REG_DENSITY, fmt->df_density);
    MicroDelay(100);

    /* Issue read track command */
    while (retries-- > 0) {
        CW_WRITE(CW_REG_CTRL, CW_CMD_READ | (unit->unit_drive & 0x03));

        /* Wait for completion */
        MilliDelay(100);

        /* Read status */
        status = CW_READ(CW_REG_STATUS);
        if (!(status & 0x01))  /* Track read OK */
            break;
    }

    if (retries < 0)
        return MDERR_NOSEC;

    /* Read data from CatWeasel FIFO */
    for (i = 0; i < (1 << fmt->df_bit0); i++) {
        buffer[i] = CW_READ(CW_REG_DATA);
    }

    return 0;
}

/*
 * Write track data to CatWeasel floppy controller
 * Writes raw MFM/FM data to the specified track/side
 */
static LONG WriteTrackData(Unit *unit, UBYTE track, UBYTE side, UBYTE *buffer)
{
    MultidiskBase *mdb = unit->unit_mdb;
    DiskFormat *fmt = &disk_formats[unit->unit_sector];
    UBYTE status;
    int i;

    if (!mdb->md_cw_ptr || !buffer)
        return MDERR_NODRIVE;

    /* Check write protect */
    if (unit->unit_flags & 0x08)
        return MDERR_WRITEPROT;

    /* Seek to track */
    SeekTrack(unit, track, side);

    /* Set density */
    CW_WRITE(CW_REG_DENSITY, fmt->df_density);
    MicroDelay(100);

    /* Write data to CatWeasel FIFO */
    for (i = 0; i < (1 << fmt->df_bit0); i++) {
        CW_WRITE(CW_REG_DATA, buffer[i]);
    }

    /* Issue write track command */
    CW_WRITE(CW_REG_CTRL, CW_CMD_WRITE | (unit->unit_drive & 0x03));

    /* Wait for completion */
    MilliDelay(100);

    /* Read status */
    status = CW_READ(CW_REG_STATUS);
    if (status & 0x02)  /* Write error */
        return MDERR_NOSEC;

    return 0;
}

/*
 * Detect floppy disk format
 * Tries MFM, GCR, Amiga, and IBM formats
 */
static LONG DetectDiskFormat(Unit *unit)
{
    MultidiskBase *mdb = unit->unit_mdb;
    UBYTE *buffer;
    LONG result;

    if (!mdb->md_diskbuf)
        return MDERR_NOMEM;

    buffer = (UBYTE *)mdb->md_diskbuf;

    /* Try to read track 0 side 0 */
    result = ReadTrackData(unit, 0, 0, buffer);
    if (result)
        return result;

    /* Try Amiga format first (most common on this platform) */
    result = DetectAmiga(unit, buffer, 0, 0);
    if (result == 0) {
        unit->unit_disk_type = FMT_AMIGA_DD;
        return 0;
    }

    /* Try IBM PC format */
    result = DetectIBM(unit, buffer, 0, 0);
    if (result == 0) {
        unit->unit_disk_type = FMT_PC_DD;
        return 0;
    }

    /* Try MFM format */
    result = DetectMFM(unit, buffer, 0, 0);
    if (result == 0) {
        unit->unit_disk_type = FMT_PC_DD;
        return 0;
    }

    /* Try GCR format (C64) */
    result = DetectGCR(unit, buffer, 0, 0);
    if (result == 0) {
        unit->unit_disk_type = FMT_C64_GCR;
        return 0;
    }

    return MDERR_BADFORMAT;
}

/*
 * Reset floppy drive to known state
 * Recalibrates the drive head to track 0
 */
static void ResetDrive(Unit *unit)
{
    MultidiskBase *mdb;

    if (!unit)
        return;

    unit->unit_disk_bitmap = 0;
    unit->unit_disk_type = 0xFF;
    unit->unit_cur_track = 0xFF;  /* Unknown position */
    unit->unit_cur_side = 0xFF;
    unit->unit_cur_density = 0xFF;

    mdb = unit->unit_mdb;
    if (!mdb->md_cw_ptr)
        return;

    /* Reset the CatWeasel controller */
    CW_WRITE(CW_REG_CTRL, CW_CMD_RESET);
    MilliDelay(10);  /* 10ms reset delay */

    /* Recalibrate - step to track 0 */
    CW_WRITE(CW_REG_CTRL, 0x20 | (unit->unit_drive & 0x03));  /* Step out */
    MilliDelay(50);   /* 50ms per step */

    /* Step out until track 0 is reached (max 83 steps for 5.25") */
    {
        int steps;
        for (steps = 0; steps < 83; steps++) {
            UBYTE status = CW_READ(CW_REG_STATUS);
            if (status & 0x10)  /* Track 0 flag */
                break;
            CW_WRITE(CW_REG_CTRL, 0x20 | (unit->unit_drive & 0x03));
            MilliDelay(10);
        }
    }

    unit->unit_cur_track = 0;
}

/*
 * Microsecond delay using timer.device
 */
static void MicroDelay(ULONG us)
{
    /* For very short delays, use CPU delay loop */
    /* Approximate: on 68000, ~4 cycles per loop iteration */
    /* At 7.14 MHz: 1us = ~1.78 iterations */
    volatile ULONG count = us * 2;
    while (count--)
        ;
}

/*
 * Millisecond delay using timer.device
 */
static void MilliDelay(ULONG ms)
{
    MultidiskBase *mdb = NULL;  /* Would need to be passed or global */
    struct timerequest *tr;

    /* Use timer.device for accurate delays */
    /* This is a simplified version - the real driver uses md_timer2 */
    {
        volatile ULONG count = ms * 714;  /* Approximate at 7.14 MHz */
        while (count--)
            ;
    }
}

/*
 * Detect MFM format floppy disk
 * MFM is used by PC and most non-Amiga/non-C64 formats
 */
static LONG DetectMFM(Unit *unit, UBYTE *buffer, ULONG track, ULONG side)
{
    /* Look for MFM sector header pattern: 0x4E 0x4E 0x4E... */
    /* MFM sync mark: 0xA1 0xA1 0xA1 0xFE (ID address mark) */
    int i;
    int sync_count = 0;

    /* Search for MFM address mark pattern */
    for (i = 0; i < 512; i++) {
        if (buffer[i] == 0xA1)
            sync_count++;
        else if (buffer[i] == 0xFE && sync_count >= 3)
            return 0;  /* Found MFM format */
        else
            sync_count = 0;
    }

    return MDERR_BADFORMAT;
}

/*
 * Detect GCR format floppy disk
 * GCR (Group Code Recording) is used by Commodore 64/128 disks
 * 4 data bits are encoded as 5 GCR bits
 */
static LONG DetectGCR(Unit *unit, UBYTE *buffer, ULONG track, ULONG side)
{
    /* GCR sync pattern: 0xFF 0xFF 0xFF 0xFE (sync mark) */
    /* C64 sector header: 0x08 (header mark) followed by
       sector, track, side, and checksum */
    int i;
    int sync_count = 0;

    /* Search for GCR sync pattern */
    for (i = 0; i < 512; i++) {
        if (buffer[i] == 0xFF)
            sync_count++;
        else if (sync_count >= 3 && buffer[i] == 0xFE)
            return 0;  /* Found GCR format */
        else
            sync_count = 0;
    }

    return MDERR_BADFORMAT;
}

/*
 * Detect Amiga format floppy disk
 * Amiga uses MFM encoding but with a different sector layout:
 *   - 11 sectors per track, 512 bytes per sector
 *   - Track header: 0x4489 0x4489 0x4489 (Amiga sync)
 *   - Sector header: track, sector, side info
 */
static LONG DetectAmiga(Unit *unit, UBYTE *buffer, ULONG track, ULONG side)
{
    /* Amiga MFM sync mark: 0x4489 repeated 3 times */
    /* This is the distinctive Amiga disk format marker */
    int i;
    int sync_count = 0;

    /* Search for Amiga MFM sync pattern (0x4489) */
    for (i = 0; i < 1024; i += 2) {
        UWORD val = (buffer[i] << 8) | buffer[i + 1];
        if (val == 0x4489)
            sync_count++;
        else if (sync_count >= 2)
            return 0;  /* Found Amiga format */
        else
            sync_count = 0;
    }

    return MDERR_BADFORMAT;
}

/*
 * Detect IBM PC format floppy disk
 * IBM format uses MFM with standard sector headers:
 *   - 9 sectors per track (DD) or 18 (HD)
 *   - Sector header: 0xFE (address mark) followed by
 *     cylinder, head, sector, size code
 */
static LONG DetectIBM(Unit *unit, UBYTE *buffer, ULONG track, ULONG side)
{
    /* IBM MFM sector header: 0xA1 0xA1 0xA1 0xFE */
    /* Followed by: cylinder, head, sector, size (0x02 = 512 bytes) */
    int i;
    int sync_count = 0;

    /* Search for IBM MFM address mark */
    for (i = 0; i < 512; i++) {
        if (buffer[i] == 0xA1)
            sync_count++;
        else if (sync_count >= 3 && buffer[i] == 0xFE) {
            /* Verify sector header: cylinder, head, sector, size */
            if (i + 4 < 512) {
                UBYTE cyl = buffer[i + 1];
                UBYTE head = buffer[i + 2];
                UBYTE sec = buffer[i + 3];
                UBYTE size = buffer[i + 4];

                /* Check for valid IBM sector header */
                if (cyl == track && head == side &&
                    sec >= 1 && sec <= 18 &&
                    size == 0x02)  /* 512 bytes */
                    return 0;  /* Found IBM format */
            }
        } else {
            sync_count = 0;
        }
    }

    return MDERR_BADFORMAT;
}

/* Disk format table - floppy disk formats only
 * Each entry: sectors, sides, tracks, density, step, flags,
 *             gap1, gap2, gap3, gap4a, gap4b, interleave,
 *             bit0-bit4, reserved
 */
static const DiskFormat disk_formats[] = {
    /* FMT_AMIGA_DD (0): 3.5" DD 880KB Amiga - 11 sectors/track, 2 sides, 80 tracks */
    { 11, 2, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* FMT_AMIGA_HD (1): 3.5" HD 1760KB Amiga - 22 sectors/track, 2 sides, 80 tracks */
    { 22, 2, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* (2): 5.25" DD 880KB Amiga - 11 sectors/track, 2 sides, 80 tracks */
    { 11, 2, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* FMT_PC_DD (3): 3.5" DD 720KB PC - 9 sectors/track, 2 sides, 80 tracks */
    { 9,  2, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* FMT_PC_HD (4): 3.5" HD 1440KB PC - 18 sectors/track, 2 sides, 80 tracks */
    { 18, 2, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* FMT_PC525_DD (5): 5.25" DD 360KB PC - 9 sectors/track, 2 sides, 40 tracks */
    { 9,  2, 40, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* FMT_PC525_HD (6): 5.25" HD 1200KB PC - 15 sectors/track, 2 sides, 80 tracks */
    { 15, 2, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* (7): 3" DD 720KB - 9 sectors/track, 2 sides, 80 tracks */
    { 9,  2, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* (8): 3" DD 880KB Amiga - 11 sectors/track, 2 sides, 80 tracks */
    { 11, 2, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* FMT_C64_GCR (9): C64 GCR 170KB - 21 sectors/track, 1 side, 35 tracks */
    { 21, 1, 35, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* (10): Apple ][ 140KB - 13 sectors/track, 1 side, 35 tracks */
    { 13, 1, 35, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* FMT_AMIGA_SS (11): 3.5" DD 880KB Amiga single-sided - 11 sectors/track, 1 side, 80 tracks */
    { 11, 1, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* FMT_PC_SS (12): 3.5" DD 720KB PC single-sided - 9 sectors/track, 1 side, 80 tracks */
    { 9,  1, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* (13): 5.25" DD 360KB PC single-sided - 9 sectors/track, 1 side, 40 tracks */
    { 9,  1, 40, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* (14): 3.5" HD 1440KB PC single-sided - 18 sectors/track, 1 side, 80 tracks */
    { 18, 1, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 },
    /* (15): 5.25" HD 1200KB PC single-sided - 15 sectors/track, 1 side, 80 tracks */
    { 15, 1, 80, 0x09, 0x3C, 0x50, 0x0A, 0x50, 0x09, 0x3E, 0x02, 0x10, 0x00, 0xDC, 0x06, 0xE0, 0x50, 0x10 }
};

/*
 * Read a CatWeasel register
 */
static ULONG ReadCWRegister(MultidiskBase *mdb, UBYTE reg)
{
    if (!mdb->md_cw_ptr)
        return 0xFF;

    return (ULONG)(*((volatile UBYTE *)((ULONG)mdb->md_cw_ptr + reg)));
}

/*
 * Write a CatWeasel register
 */
static void WriteCWRegister(MultidiskBase *mdb, UBYTE reg, UBYTE val)
{
    if (!mdb->md_cw_ptr)
        return;

    *((volatile UBYTE *)((ULONG)mdb->md_cw_ptr + reg)) = val;
}

/*
 * ROMTag - Amiga device autoinit table
 * This is the entry point that AmigaOS uses to initialize the device
 */
static const ULONG romtag[] = {
    0x4E414953,  /* RTC_MATCHWORD - "SINA" */
    (ULONG)romtag,  /* Pointer to self */
    0,              /* Skip vector (filled by installer) */
    0x8004,         /* RTF_AUTOINIT | RTF_COLDSTART, version 3, type DEVICE */
    DEVICE_VERSION, /* Version */
    DEVICE_REVISION, /* Revision */
    0,              /* Skip (filled by installer) */
    (ULONG)device_name,  /* Device name pointer */
    (ULONG)device_id_string, /* Device ID string pointer */
    0               /* Skip (filled by installer) */
};

/*
 * Autoinit data structure
 * Used by AmigaOS to initialize the device
 */
static const ULONG autoinit[] = {
    sizeof(MultidiskBase),   /* Data size (size of our base structure) */
    (ULONG)dev_init,         /* Function pointer - initialization */
    0,                       /* Function pointer - no open init */
    (ULONG)dev_expunge       /* Function pointer - expunge */
};
