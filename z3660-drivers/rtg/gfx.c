/*
 * Z3660 Graphics Card Driver based on MNT ZZ9000 rev 1.11
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 */

/*
 * MNT ZZ9000 Amiga Graphics Card Driver (ZZ9000.card)
 *
 * Copyright (C) 2016-2023, Lukas F. Hartmann <lukas@mntre.com>
 *                                       MNT Research GmbH, Berlin
 *                                       https://mntre.com
 * Copyright (C) 2021,         Bjorn Astrom <beeanyew@gmail.com>
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <exec/initializers.h>
#include <exec/nodes.h>
#include <clib/debug_protos.h>
#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/input.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <exec/initializers.h>
#include <clib/debug_protos.h>
#include <devices/inputevent.h>
#include <proto/utility.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "gfx.h"
#include "../common/z3660_regs.h"

#define STR(s) #s
#define XSTR(s) STR(s)

struct GFXBase {
    struct Library libNode;
    UBYTE Flags;
    UBYTE pad;
    struct ExecBase *ExecBase;
    struct ExpansionBase *ExpansionBase;
    BPTR segList;
    char *Name;
};

#define KPrintF(...)
#define __saveds__

#define DEVICE_VERSION 1
#define DEVICE_REVISION 4
#define DEVICE_PRIORITY 0
#define DEVICE_ID_STRING "$VER Z3660.card " XSTR(DEVICE_VERSION) "." XSTR(DEVICE_REVISION) " " DEVICE_DATE
#define DEVICE_NAME "Z3660.card"
#define DEVICE_DATE "(17.07.2025)"

int __attribute__((no_reorder)) _start()
{
    return -1;
}

#ifdef DEBUG
void exit(int status)
{
   (void)status;
   while(1);
}
#endif

extern const uint32_t auto_init_tables[4];

const struct Resident RomTag = {
    RTC_MATCHWORD,                       /* Marker value. */
    (struct Resident *)&RomTag,          /* This points back to itself. */
    (struct Resident *)auto_init_tables, /* This points somewhere behind this marker. */
    RTF_AUTOINIT,                        /* The Library should be set up according to the given table. */
    DEVICE_VERSION,                      /* The version of this Library. */
    NT_LIBRARY,                          /* This defines this module as a Library. */
    DEVICE_PRIORITY,                     /* Initialization priority of this Library; unused. */
    DEVICE_NAME,                         /* Points to the name of the Library. */
    DEVICE_ID_STRING,                    /* The identification string of this Library. */
    (APTR)&auto_init_tables              /* This table is for initializing the Library. */
};

char device_name[] = DEVICE_NAME;
char device_id_string[] = DEVICE_ID_STRING;

__saveds struct GFXBase* OpenLib(__REGA6(struct GFXBase *gfxbase));
BPTR __saveds CloseLib(__REGA6(struct GFXBase *gfxbase));
BPTR __saveds ExpungeLib(__REGA6(struct GFXBase *exb));
ULONG ExtFuncLib(void);
__saveds struct GFXBase* InitLib(__REGA6(struct ExecBase *sysbase),
                                                 __REGA0(BPTR seglist),
                                                 __REGD0(struct GFXBase *exb));

#define CLOCK_HZ 100000000

static struct GFXBase *_gfxbase;
char *gfxname = "Z3660";
char dummies[128];

#define CARDFLAG_ZORRO_3 1
// Place scratch area right after framebuffer? Might be a horrible idea.
#define Z3660_FRAMEBUF_ADDR 0x00200000
#define Z3660_MEMBASE_ADDR 0x00200000
#define Z3_GFXDATA_ADDR    (0x03200000 - Z3660_MEMBASE_ADDR)
#define Z3_TEMPLATE_ADDR   (0x03210000 - Z3660_MEMBASE_ADDR)
#define ZZVMODE_800x600 1
#define ZZVMODE_720x576 6
#define ZZ_CARD_DATA_GFXDATA 0
#define ZZ_CARD_DATA_SCANDBL_800X600 1
#define ZZ_CARD_DATA_NSVSYNC 2
#define ZZ_CARD_DATA_MONITOR_SWITCH 3
#define ZZ_CARD_DATA_DISPLAY_ENABLED 4
#define ZZ_CARD_DATA_SECONDARY_PALETTE 5

/*
 * Only advertise formats the firmware/video formatter interprets natively.
 * RGBA and the PC/BGR 15-bit variants use different byte or component order,
 * but the firmware only has one 32-bit mode and one RGB555 mode.
 */
#define ZZ_SUPPORTED_RGB_FORMATS (  \
          RTG_COLOR_FORMAT_CLUT   | \
          RTG_COLOR_FORMAT_RGB555 | \
          RTG_COLOR_FORMAT_RGB565 | \
          RTG_COLOR_FORMAT_BGRA    )

struct ExecBase *SysBase;

static inline volatile struct GFXData *zz_gfxdata(struct BoardInfo *b) {
   return (volatile struct GFXData *)b->CardData[ZZ_CARD_DATA_GFXDATA];
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const uint16_t rtg_to_mnt[21] = {
   MNTVA_COLOR_8BIT,       // 0x00 -- None
   MNTVA_COLOR_8BIT,       // 0x01 -- 8BPP CLUT
   MNTVA_COLOR_NO_USE,     // 0x02 -- 24BPP RGB
   MNTVA_COLOR_NO_USE,     // 0x03 -- 24BPP BGR
   MNTVA_COLOR_NO_USE,     // 0x04 -- 16BPP R5G6B5PC
   MNTVA_COLOR_15BIT,      // 0x05 -- 15BPP R5G5B5PC
   MNTVA_COLOR_32BIT,      // 0x06 -- 32BPP ARGB
   MNTVA_COLOR_32BIT,      // 0x07 -- 32BPP ABGR
   MNTVA_COLOR_32BIT,      // 0x08 -- 32BPP RGBA
   MNTVA_COLOR_32BIT,      // 0x09 -- 32BPP BGRA
   MNTVA_COLOR_16BIT565,   // 0x0A -- 16BPP R5G6B5
   MNTVA_COLOR_15BIT,      // 0x0B -- 15BPP R5G5B5
   MNTVA_COLOR_NO_USE,     // 0x0C -- 16BPP B5G6R5PC
   MNTVA_COLOR_15BIT,      // 0x0D -- 15BPP B5G5R5PC
   MNTVA_COLOR_NO_USE,     // 0x0E -- YUV 4:2:2
   MNTVA_COLOR_NO_USE,     // 0x0F -- YUV 4:1:1
   MNTVA_COLOR_NO_USE,     // 0x10 -- YUV 4:1:1PC
   MNTVA_COLOR_NO_USE,     // 0x11 -- YUV 4:2:2 (Duplicate for some reason)
   MNTVA_COLOR_NO_USE,     // 0x12 -- YUV 4:2:2PC
   MNTVA_COLOR_NO_USE,     // 0x13 -- YUV 4:2:2 Planar
   MNTVA_COLOR_NO_USE,     // 0x14 -- YUV 4:2:2PC Planar
};

static inline uint16_t mnt_colormode(UWORD format) {
   if (format < ARRAY_SIZE(rtg_to_mnt))
      return rtg_to_mnt[format];

   return MNTVA_COLOR_NO_USE;
}

static inline uint16_t panning_colormode(uint16_t colormode) {
   return (colormode == MNTVA_COLOR_15BIT) ? MNTVA_COLOR_16BIT565 : colormode;
}

static inline UBYTE direct_color_mask(uint16_t colormode, UBYTE mask) {
   return (colormode == MNTVA_COLOR_32BIT) ? 0xFF : mask;
}

static inline BOOL supported_rgb_format(RGBFTYPE format) {
   if (format >= 32)
      return FALSE;

   return (ZZ_SUPPORTED_RGB_FORMATS & (1UL << format)) != 0;
}

static inline uint16_t planar_line_bytes(SHORT x, SHORT w) {
	return ((((UWORD)x) & 0x07) + (UWORD)w + 7) >> 3;
}

static inline uint16_t planar_line_bytes_padded(SHORT w) {
   // P2C keeps the historical two-byte padding; the firmware's
   // source byte wrap check otherwise fires at the end of most scanlines.
   return (((UWORD)w) >> 3) + 2;
}

#define ZZ_REGS_WRITE(b, c) do{registers[(b)>>2]=(c);}while(0)
#define ZZ_REGS_READ(b) registers[(b)>>2]

/*
// Assuming that it takes longer to write the same value through slow ZorroII register access again
// than comparing it with a cached value in FAST RAM, these routines should speed up things a lot.
static inline void writeBlitterSrcOffset(MNTZZ9KRegs* registers, ULONG offset) {
   static ULONG old = 0;
   if (offset != old) {
      zzwrite32(&registers->blitter_src_hi, offset);
      old = offset;
   }
}

static inline void writeBlitterDstOffset(MNTZZ9KRegs* registers, ULONG offset) {
   static ULONG old = 0;
   if (offset != old) {
      zzwrite32(&registers->blitter_dst_hi, offset);
      old = offset;
   }
}

static inline void writeBlitterRGB(MNTZZ9KRegs* registers, ULONG color) {
   static ULONG old = 0;
   if (color != old) {
      zzwrite32(&registers->blitter_rgb_hi, color);
      old = color;
   }
}

static inline void writeBlitterSrcPitch(MNTZZ9KRegs* registers, UWORD srcpitch) {
   if (blitter_cache_src_pitch_write_needed(&blitter_register_cache,
         registers, srcpitch)) {
      zzwrite16(&registers->blitter_src_pitch, srcpitch);
   }
}

static inline void writeBlitterRGB2(MNTZZ9KRegs* registers, ULONG color) {
   if (blitter_cache_write32_needed(&blitter_register_cache, registers,
         BLITTER_CACHE_RGB2, &blitter_register_cache.rgb2, color)) {
      zzwrite32(&registers->blitter_rgb2_hi, color);
   }
}

static inline void writeBlitterUser1(MNTZZ9KRegs* registers, UWORD value) {
   if (blitter_cache_write16_needed(&blitter_register_cache, registers,
         BLITTER_CACHE_USER1, &blitter_register_cache.user1, value)) {
      zzwrite16(&registers->blitter_user1, value);
   }
}

static inline void writeBlitterUser2(MNTZZ9KRegs* registers, UWORD value) {
   if (blitter_cache_write16_needed(&blitter_register_cache, registers,
         BLITTER_CACHE_USER2, &blitter_register_cache.user2, value)) {
      zzwrite16(&registers->blitter_user2, value);
   }
}

static inline void writeBlitterDstPitch(MNTZZ9KRegs* registers, UWORD dstpitch) {
   static UWORD old = 0;
   if (dstpitch != old) {
      zzwrite16(&registers->blitter_row_pitch, dstpitch);
      old = dstpitch;
   }
}

static inline void writeBlitterColorMode(MNTZZ9KRegs* registers, UWORD colormode) {
   static UWORD old = MNTVA_COLOR_32BIT;
   if (colormode != old) {
      zzwrite16(&registers->blitter_colormode, colormode);
      old = colormode;
   }
}

static inline void writeBlitterX1(MNTZZ9KRegs* registers, UWORD x) {
   static UWORD old = 0;
   if (x != old) {
      zzwrite16(&registers->blitter_x1, x);
      old = x;
   }
}

static inline void writeBlitterY1(MNTZZ9KRegs* registers, UWORD y) {
   static UWORD old = 0;
   if (y != old) {
      zzwrite16(&registers->blitter_y1, y);
      old = y;
   }
}

static inline void writeBlitterX2(MNTZZ9KRegs* registers, UWORD x) {
   static UWORD old = 0;
   if (x != old) {
      zzwrite16(&registers->blitter_x2, x);
      old = x;
   }
}

static inline void writeBlitterY2(MNTZZ9KRegs* registers, UWORD y) {
   static UWORD old = 0;
   if (y != old) {
      zzwrite16(&registers->blitter_y2, y);
      old = y;
   }
}

static inline void writeBlitterX3(MNTZZ9KRegs* registers, UWORD x) {
   static UWORD old = 0;
   if (x != old) {
      zzwrite16(&registers->blitter_x3, x);
      old = x;
   }
}

static inline void writeBlitterY3(MNTZZ9KRegs* registers, UWORD y) {
   static UWORD old = 0;
   if (y != old) {
      zzwrite16(&registers->blitter_y3, y);
      old = y;
   }
}
*/

// useful for debugging
void waitclick() {
#define CIAAPRA ((volatile uint8_t*)0xbfe001)
   // bfe001 http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node012E.html
   while (!(*CIAAPRA & (1<<6))) {
      // wait for left mouse button pressed
   }
   while ((*CIAAPRA & (1<<6))) {
      // wait for left mouse button released
   }
}

__saveds struct GFXBase* __attribute__((used)) InitLib(__REGA6(struct ExecBase *sysbase),
                                           __REGA0(BPTR seglist),
                                           __REGD0(struct GFXBase *exb))
{
   (void)sysbase;
   (void)seglist;
   _gfxbase = exb;
   _gfxbase->libNode.lib_Node.ln_Name = device_name;
   _gfxbase->libNode.lib_Version = DEVICE_VERSION;
   _gfxbase->libNode.lib_Revision = DEVICE_REVISION;
   _gfxbase->libNode.lib_IdString = device_id_string;
   _gfxbase->Name = gfxname;
   SysBase = *(struct ExecBase **)4L;
   return _gfxbase;
}

__saveds struct GFXBase* __attribute__((used)) OpenLib(__REGA6(struct GFXBase *gfxbase))
{
   gfxbase->libNode.lib_OpenCnt++;
   gfxbase->libNode.lib_Flags &= ~LIBF_DELEXP;

   return gfxbase;
}

BPTR __saveds __attribute__((used)) CloseLib(__REGA6(struct GFXBase *gfxbase))
{
   gfxbase->libNode.lib_OpenCnt--;

   if (!gfxbase->libNode.lib_OpenCnt) {
      if (gfxbase->libNode.lib_Flags & LIBF_DELEXP) {
         return (ExpungeLib(gfxbase));
      }
   }
   return 0;
}

BPTR __saveds __attribute__((used)) ExpungeLib(__REGA6(struct GFXBase *exb))
{
   BPTR seglist;
   struct ExecBase *SysBase = *(struct ExecBase **)4L;

   if(!exb->libNode.lib_OpenCnt) {
      ULONG negsize, possize, fullsize;
      UBYTE *negptr = (UBYTE *)exb;

      seglist = exb->segList;

      Remove((struct Node *)exb);

      negsize    = exb->libNode.lib_NegSize;
      possize    = exb->libNode.lib_PosSize;
      fullsize = negsize + possize;
      negptr   -= negsize;

      FreeMem(negptr, fullsize);
      return(seglist);
   }

   exb->libNode.lib_Flags |= LIBF_DELEXP;
   return 0;
}

ULONG ExtFuncLib(void)
{
   return 0;
}

#define LOADLIB(a, b) if ((a = (struct a*)OpenLibrary((STRPTR)b,0L))==NULL) { \
      KPrintF((STRPTR)"Z3660.card: Failed to load %s.\n", b); \
      goto cleanup; \
   } \


int __attribute__((used)) FindCard(__REGA0(struct BoardInfo* b)) {
   struct ConfigDev* cd = NULL;
   struct ExpansionBase *ExpansionBase = NULL;
   struct DOSBase *DOSBase = NULL;
   struct IntuitionBase *IntuitionBase = NULL;
   struct ExecBase *SysBase = *(struct ExecBase **)4L;
   LONG zorro_version = 0;
   LONG fwrev_major = 0;
   LONG fwrev_minor = 0;
   LONG fwrev = 0;
//   int result = 0;
#ifdef DEBUG
   LONG hwrev = 0;
#endif

   LOADLIB(ExpansionBase, "expansion.library");
   LOADLIB(DOSBase, "dos.library");
   LOADLIB(IntuitionBase, "intuition.library");

   zorro_version = 0;
   b->CardFlags = 0;
   b->CardData[ZZ_CARD_DATA_GFXDATA] = 0;
   b->CardData[ZZ_CARD_DATA_SCANDBL_800X600] = 0;
   b->CardData[ZZ_CARD_DATA_NSVSYNC] = 0;
   b->CardData[ZZ_CARD_DATA_MONITOR_SWITCH] = 1;
   b->CardData[ZZ_CARD_DATA_DISPLAY_ENABLED] = 1;
   b->CardData[ZZ_CARD_DATA_SECONDARY_PALETTE] = 0;

   if ((cd = (struct ConfigDev*)FindConfigDev(cd,0x144B,0x1))) zorro_version = 3;
   if(zorro_version==0)
   {
      struct ConfigDev *last_CD = NULL;
      struct ConfigDev *new_zz_cd = NULL;
//      KPrintF((CONST_STRPTR)"Z3660 not found. Looking at $10000000 if it is there...\n");
      new_zz_cd = (struct ConfigDev*)malloc(sizeof(struct ConfigDev));
      memset(new_zz_cd, 0, sizeof(struct ConfigDev));
      while(last_CD=FindConfigDev(last_CD,-1L,-1L)) /* search for all ConfigDevs */
      {
         if(last_CD->cd_NextCD==NULL)
         {
            break;
         }
      }
      KPrintF((CONST_STRPTR)"[rtg driver] Z3660 not found! Setting 0x10000000\n");
      new_zz_cd->cd_Node.ln_Type = NT_DEVICE;
      new_zz_cd->cd_Node.ln_Name = "Z3660";
      new_zz_cd->cd_Node.ln_Pri = 0;
      new_zz_cd->cd_Node.ln_Succ = NULL;
      new_zz_cd->cd_Node.ln_Pred = &last_CD->cd_Node;
      new_zz_cd->cd_Flags = 0;
      new_zz_cd->cd_BoardAddr=(APTR)0x10000000; /* where in memory the board was placed */
      new_zz_cd->cd_BoardSize=0x8000000;   /* 128MB size of board in bytes */
      new_zz_cd->cd_Rom.er_Type = ERT_ZORROIII | 2; // ZorroIII and 128 MB
      new_zz_cd->cd_Rom.er_Manufacturer = 0x144B;
      new_zz_cd->cd_Rom.er_Product = 0x1;
      new_zz_cd->cd_Rom.er_Flags = 0;
      new_zz_cd->cd_Rom.er_InitDiagVec = 0;
      //UWORD      cd_SlotAddr;   /* which slot number (PRIVATE) */
      //UWORD      cd_SlotSize;   /* number of slots (PRIVATE) */
      //APTR      cd_Driver;   /* pointer to node of driver */
      //struct ConfigDev *   cd_NextCD;   /* linked list of drivers to config */
      //ULONG      cd_Unused[4];   /* for whatever the driver wants */
      AddConfigDev(new_zz_cd);
      cd = new_zz_cd;
   }

   volatile uint32_t* registers = (uint32_t *)(cd->cd_BoardAddr);

   {
      uint32_t fwrev = registers[REG_ZZ_FW_VERSION>>2];

      int fwrev_major = fwrev>>8;
      if(fwrev_major!=1)
      {
         KPrintF((CONST_STRPTR)"Z3660 not found. %d\n",fwrev_major);
         return(0);
      }
      else
      {
         KPrintF((CONST_STRPTR)"Z3660 found. %d\n",fwrev_major);
      }
   }
   // Find Z3 or Z2 model
   if (zorro_version>=2) {

      b->MemoryBase = (uint8_t *)(cd->cd_BoardAddr)+Z3660_MEMBASE_ADDR;
      KPrintF((CONST_STRPTR)"BoardAddr 0x%lx\n",cd->cd_BoardAddr);
      if (zorro_version == 2) {
         b->MemorySize = cd->cd_BoardSize-0x30000;
      } else {
         // 13.8 MB for Z3 (safety, will be expanded later)
         // one full HD screen @8bit ~ 2MB
         b->MemorySize = 0x3000000 - Z3660_MEMBASE_ADDR;
         b->CardFlags |= CARDFLAG_ZORRO_3;
         volatile struct GFXData *gd = (struct GFXData*)(((uint32_t)b->MemoryBase) + (uint32_t)Z3_GFXDATA_ADDR);
         b->CardData[ZZ_CARD_DATA_GFXDATA] = (ULONG)gd;
         KPrintF((CONST_STRPTR)"GFXData   0x%lx\n",gd);
         memset((void *)gd, 0x00, sizeof(struct GFXData));
      }
      b->MemorySpaceBase = b->MemoryBase;
      b->MemorySpaceSize = b->MemorySize;
      b->RegisterBase = (void*)(cd->cd_BoardAddr);
#ifdef DMARTG
      volatile uint32_t* registers = (uint32_t *)b->RegisterBase;
#endif

//    test write
//    ZZ_REGS_WRITE(REG_ZZ_DEBUG,(u32)cd->cd_BoardAddr);
//    ZZ_REGS_WRITE(REG_ZZ_DEBUG,(u32)cd->cd_BoardAddr);

#ifdef DMARTG
      fwrev = registers[REG_ZZ_FW_VERSION>>2];
#else
      fwrev = ((uint16_t*)b->RegisterBase)[REG_ZZ_FW_VERSION/2];
#endif
      fwrev_major = fwrev >> 8;
      fwrev_minor = fwrev & 0xff;

      KPrintF((CONST_STRPTR)"%s\n",device_id_string);
      KPrintF((CONST_STRPTR)"Z3660.card: Z3660 found. Zorro version %ld.\n", zorro_version);
      KPrintF((CONST_STRPTR)"Z3660.card: HW Revision: %ld.\n", hwrev);
      KPrintF((CONST_STRPTR)"Z3660.card: FW Revision Major: %ld.\n", fwrev_major);
      KPrintF((CONST_STRPTR)"Z3660.card: FW Revision Minor: %ld.\n", fwrev_minor);

      if (fwrev_major <= 1 && fwrev_minor < 3) {
         char *alert = "\x00\x14\x14 vX.XX: Z3660.card v" XSTR(DEVICE_VERSION) "." XSTR(DEVICE_REVISION) " needs at least firmware (BOOT.bin) v1.03.\x00\x00";
         alert[5]='0'+fwrev_major;
         alert[7]='0'+(fwrev_minor/10);
         alert[8]='0'+(fwrev_minor%10);
         DisplayAlert(RECOVERY_ALERT, (unsigned char*)alert, 52);
         return 1;
      }
/*
      if (fwrev_major <= 1 && fwrev_minor < 3) {
         KPrintF((CONST_STRPTR)"Z3660 FW v%c.%c%c. Z3660.card v1.03 needs at least firmware (BOOT.bin) v1.03.\n",'0'+fwrev_major,'0'+(fwrev_minor/10),'0'+(fwrev_minor%10));
         return 1;
      }
*/
/* Z3660 -> no scandoubler :(
      MNTZZ9KRegs* registers = (MNTZZ9KRegs *)b->RegisterBase;
*/      BPTR f;
/*      if ((f = Open((APTR)"ENV:ZZ9000-VCAP-800x600", MODE_OLDFILE))) {
         Close(f);
         KPrintF((CONST_STRPTR)"ZZ9000.card: 800x600 60hz scandoubler mode.\n");
         scandoubler_800x600 = 1;
         registers->videocap_vmode = ZZVMODE_800x600; // 60hz
      } else {
         KPrintF((CONST_STRPTR)"ZZ9000.card: 720x576 50hz scandoubler mode.\n");
         scandoubler_800x600 = 0;
         registers->videocap_vmode = ZZVMODE_720x576; // 50hz
      }
*/

      if ((f = Open((APTR)"ENV:Z3660-NS-VSYNC", MODE_OLDFILE))) {
         Close(f);
         ZZ_REGS_WRITE(REG_ZZ_USER1, CARD_FEATURE_NONSTANDARD_VSYNC);
         ZZ_REGS_WRITE(REG_ZZ_SET_FEATURE, 1);
      } else if ((f = Open((APTR)"ENV:Z3660-NS-VSYNC-NTSC", MODE_OLDFILE))) {
         Close(f);
         ZZ_REGS_WRITE(REG_ZZ_USER1, CARD_FEATURE_NONSTANDARD_VSYNC);
         ZZ_REGS_WRITE(REG_ZZ_SET_FEATURE, 2);
      } else {
         ZZ_REGS_WRITE(REG_ZZ_USER1, CARD_FEATURE_NONSTANDARD_VSYNC);
         ZZ_REGS_WRITE(REG_ZZ_SET_FEATURE, 0);
      }

   } else {
      KPrintF((CONST_STRPTR)"Z3660.card: Z3660 not found!\n");
//      DisplayAlert(RECOVERY_ALERT, (unsigned char*)"\x01\x04\x10Z3660 not found\x00\x00", 30);
   }
   
   cleanup:
   if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
   if (DOSBase) CloseLibrary((struct Library *)DOSBase);
   if (ExpansionBase) CloseLibrary((struct Library *)ExpansionBase);
   
   return(1);
}

#define gfxdata zz_gfxdata(b)

int __attribute__((used)) InitCard(__REGA0(struct BoardInfo* b)) {
   int i;

   KPrintF((CONST_STRPTR)"InitCard()\n");
   b->CardBase = (struct CardBase *)_gfxbase;
   b->ExecBase = SysBase;
   b->BoardName = "Z3660";
//   b->BoardType = BT_MNT_ZZ9000;
//   b->PaletteChipType = PCT_MNT_ZZ9000;
//   b->GraphicsControllerType = GCT_MNT_ZZ9000;
   b->BoardType = BT_uaegfx;
   b->PaletteChipType = PCT_S3ViRGE;
   b->GraphicsControllerType = GCT_S3ViRGE;

   b->Flags |= BIF_GRANTDIRECTACCESS |
               BIF_HARDWARESPRITE |
//               BIF_FLICKERFIXER |
               BIF_VGASCREENSPLIT |
               BIF_PALETTESWITCH |
               BIF_BLITTER |
//               BIF_INTERNALMODESONLY | // this doesn't work as expected
//               BIF_VIDEOCAPTURE |
               BIF_VIDEOWINDOW |
               BIF_OVERCLOCK|
               BIF_CACHEMODECHANGE|
               0;

   b->RGBFormats = ZZ_SUPPORTED_RGB_FORMATS;
   b->SoftSpriteFlags = 0;
   b->BitsPerCannon = 8;
#define PIXELCLOCK_MHZ_MAX 220
#define PIXELCLOCK_MHZ_MIN   3
   for(i = 0; i < MAXMODES; i++) {
      b->MaxHorValue[i] = 8192;
      b->MaxVerValue[i] = 8192;
      b->MaxHorResolution[i] = 8192;
      b->MaxVerResolution[i] = 8192;
      b->PixelClockCount[i] = (PIXELCLOCK_MHZ_MAX-PIXELCLOCK_MHZ_MIN)*4+1;
   }

   b->MemoryClock = 0;//CLOCK_HZ;

   //b->AllocCardMem = (void *)NULL;
   //b->FreeCardMem = (void *)NULL;
   b->SetSwitch = (void *)SetSwitch;
   b->SetColorArray = (void *)SetColorArray;
   b->SetDAC = (void *)SetDAC;
   b->SetGC = (void *)SetGC;
   b->SetPanning = (void *)SetPanning;
   b->CalculateBytesPerRow = (void *)CalculateBytesPerRow;
   b->CalculateMemory = (void *)CalculateMemory;
   b->GetCompatibleFormats = (void *)GetCompatibleFormats;
   b->SetDisplay = (void *)SetDisplay;

   b->ResolvePixelClock = (void *)ResolvePixelClock;
   b->GetPixelClock = (void *)GetPixelClock;
   b->SetClock = (void *)SetClock;

   b->SetMemoryMode = (void *)SetMemoryMode;
   b->SetWriteMask = (void *)SetWriteMask;
   b->SetClearMask = (void *)SetClearMask;
   b->SetReadPlane = (void *)SetReadPlane;

   b->WaitVerticalSync = (void *)WaitVerticalSync;

   b->WaitBlitter = (void *)WaitBlitter;

   //b->ScrollPlanar = (void *)NULL;
   //b->UpdatePlanar = (void *)NULL;

   b->BlitPlanar2Chunky = (void *)BlitPlanar2Chunky;
   b->BlitPlanar2Direct = (void *)BlitPlanar2Direct;

   b->FillRect = (void *)FillRect;
   b->InvertRect = (void *)InvertRect;
   b->BlitRect = (void *)BlitRect;
   b->BlitTemplate = (void *)BlitTemplate;
   b->BlitPattern = (void *)BlitPattern;
   b->DrawLine = (void *)DrawLine;
   b->BlitRectNoMaskComplete = (void *)BlitRectNoMaskComplete;
   b->EnableSoftSprite = (void *)EnableSoftSprite;

   //b->AllocCardMemAbs = (void *)NULL;
   b->SetSplitPosition = (void *)SetSplitPosition;
   //b->ReInitMemory = (void *)NULL;
   //b->WriteYUVRect = (void *)NULL;
   b->GetVSyncState = (void *)GetVSyncState;
   //b->GetVBeamPos = (void *)NULL;
   //b->SetDPMSLevel = (void *)NULL;
   //b->ResetChip = (void *)NULL;
   //b->AllocBitMap = (void *)NULL;
   //b->FreeBitMap = (void *)NULL;
   //b->GetBitMapAttr = (void *)NULL;

   b->SetSprite = (void *)SetSprite;
   b->SetSpritePosition = (void *)SetSpritePosition;
   b->SetSpriteImage = (void *)SetSpriteImage;
   b->SetSpriteColor = (void *)SetSpriteColor;

   b->CreateFeature = (void *)CreateFeature;
   b->SetFeatureAttrs = (void *)SetFeatureAttrs;
   b->GetFeatureAttrs = (void *)GetFeatureAttrs;
   b->DeleteFeature = (void *)DeleteFeature;

   return 1;
}

struct Z3660Overlay {
   struct Z3660Overlay *next;  // For linking in the list
   ULONG type;                 // SFT_VIDEOWINDOW or SFT_MEMORYWINDOW
   BOOL active;                // Whether the overlay is active
   BOOL occlusion;             // Whether it is being hidden by a layer
   ULONG left, top;            // Position on screen
   ULONG width, height;        // Window size
   ULONG max_width, max_height;// Maximum allowed size
   ULONG min_width, min_height;// Minimum allowed size
   ULONG source_width;         // Source size
   ULONG source_height;
   RGBFTYPE format;            // RGB format of the overlay
   struct BitMap *bitmap;      // Source bitmap
   BOOL free_bitmap;           // Whether the bitmap should be freed
   ULONG ClipLeft, ClipTop;    // Clipping area
   ULONG ClipWidth, ClipHeight;
   BOOL onboard;
};

ULONG GetMemoryOffset(struct BoardInfo *bi, ULONG *offset)
{
    if (!offset || !bi->GetBitMapAttr)
        return 0;
#define BMA_BASEADDRESS 0x1001
    // Get the base address of the bitmap
    ULONG base_addr = (ULONG)bi->GetBitMapAttr(bi, (struct BitMap *)offset, BMA_BASEADDRESS);
    if (!base_addr)
        return 0;

    // Calculate the offset within the framebuffer memory
    *offset = base_addr - (ULONG)bi->MemoryBase;
    return 1;
}
ULONG IsOnBoardMemory(struct BoardInfo *bi, ULONG address)
{
    if (address >= (ULONG)bi->MemoryBase &&
        address < ((ULONG)bi->MemoryBase + bi->MemorySize)) {
        return 1;
    }
    return 0;
}

APTR CreateFeature(__REGA0(struct BoardInfo *bi), __REGD0(ULONG type), __REGA1(struct TagItem *tags))
{
   struct Z3660Overlay *overlay = NULL;
   KPrintF((CONST_STRPTR)"CreateFeature type %lu\n", type);
   KPrintF((CONST_STRPTR)"tags 0x%08lX\n", (ULONG)tags);
    
   // Verificar que el tipo de feature es soportado
   if (type != SFT_VIDEOWINDOW && type != SFT_MEMORYWINDOW)
      return NULL;

   // Reservar memoria para la estructura interna del overlay
   overlay = (struct Z3660Overlay *)AllocVec(sizeof(struct Z3660Overlay), MEMF_CLEAR);
   if (!overlay)
      return NULL;

   KPrintF((CONST_STRPTR)"Overlay struct allocated at 0x%lx\n", overlay);

   // Inicializar la estructura
   memset(overlay, 0, sizeof(struct Z3660Overlay));
   overlay->type = type;
   overlay->active = FALSE;
   overlay->occlusion = FALSE;

   struct TagItem *tagPtr = (struct TagItem *)tags;
   KPrintF((CONST_STRPTR)"tags 0x%08lX\n", (ULONG)tags);
   KPrintF((CONST_STRPTR)"tagPtr 0x%08lX\n", (ULONG)tagPtr);
   uint32_t tag_counter = 0;
   while (tagPtr->ti_Tag != TAG_DONE && tagPtr->ti_Tag != TAG_END) {
      switch (tagPtr->ti_Tag) {
         case TAG_MORE:
            tagPtr=(struct TagItem *)tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: TAG_MORE to 0x%08lX\n", (ULONG)tagPtr);
            tagPtr--;
            break;
         case FA_Left:
            overlay->left = tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_Left %ld\n", tagPtr->ti_Data);
            break;
         case FA_Top:
            overlay->top = tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_Top %ld\n", tagPtr->ti_Data);
            break;
         case FA_Width:
            overlay->width = tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_Width %ld\n", tagPtr->ti_Data);
            break;
         case FA_Height:
            overlay->height = tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_Height %ld\n", tagPtr->ti_Data);
            break;
         case FA_Format:
            overlay->format = (RGBFTYPE)tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_Format %ld\n", tagPtr->ti_Data);
            break;
         case FA_BitMap:
            overlay->bitmap = (struct BitMap *)tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_BitMap 0x%08lX\n", (ULONG)overlay->bitmap);
            break;
         case FA_MaxWidth:
            overlay->max_width = tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_MaxWidth %ld\n", tagPtr->ti_Data);
            break;
         case FA_MaxHeight:
            overlay->max_height = tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_MaxHeight %ld\n", tagPtr->ti_Data);
            break;
         case FA_MinWidth:
            overlay->min_width = tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_MinWidth %ld\n", tagPtr->ti_Data);
            break;
         case FA_MinHeight:
            overlay->min_height = tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_MinHeight %ld\n", tagPtr->ti_Data);
            break;
         case FA_SourceWidth:
            overlay->source_width = tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_SourceWidth %ld\n", tagPtr->ti_Data);
            break;
         case FA_SourceHeight:
            overlay->source_height = tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_SourceHeight %ld\n", tagPtr->ti_Data);
            break;
         case FA_Occlusion:
            overlay->occlusion = (BOOL)tagPtr->ti_Data;
            KPrintF((CONST_STRPTR)"CreateFeature: FA_Occlusion %ld\n", tagPtr->ti_Data);
            break;
         case FA_ModeInfo:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_ModeInfo not implemented yet. 0x%08lX\n", tagPtr->ti_Data);
            break;
         case FA_ModeFormat:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_ModeFormat not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_ModeMemorySize:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_ModeMemorySize not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_BitmapWidth:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_BitmapWidth not implemented yet. %ld\n", tagPtr->ti_Data); 
            break;
         case FA_BitmapHeight:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_BitmapHeight not implemented yet. %ld\n", tagPtr->ti_Data);  
            break;
         case FA_Colors:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_Colors not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_Colors32:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_Colors32 not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_NoMemory:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_NoMemory not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_RenderFunc:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_RenderFunc not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_SaveFunc:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_SaveFunc not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_UserData:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_UserData not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_Alignment:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_Alignment not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_ConstantBytesPerRow:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_ConstantBytesPerRow not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_DoubleBuffer:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_DoubleBuffer not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_Pen:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_Pen not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_ConstantByteSwapping:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_ConstantByteSwapping not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case FA_Color:
            KPrintF((CONST_STRPTR)"CreateFeature: FA_Color not implemented yet. 0x%08lX\n", tagPtr->ti_Data);
            break;

         case P96PIP_SourceFormat:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_SourceFormat not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_SourceBitMap:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_SourceBitMap not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_SourceRPort:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_SourceRPort not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_SourceWidth:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_SourceWidth not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_SourceHeight:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_SourceHeight not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_Type:
            if(tagPtr->ti_Data == PIPT_VideoWindow)
            {
               overlay->type = SFT_VIDEOWINDOW;
               KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Type PIPT_VideoWindow\n");
            }
            else if(tagPtr->ti_Data == PIPT_MemoryWindow)
            {
               overlay->type = SFT_MEMORYWINDOW;
               KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Type PIPT_MemoryWindow\n");
            }
            else
            {
               KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Type unknown type %ld\n", tagPtr->ti_Data);
            }
            break;
         case P96PIP_ErrorCode:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_ErrorCode not implemented yet. 0x%08lX\n", tagPtr->ti_Data);
            break;
         case P96PIP_Brightness:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Brightness not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_Left:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Left not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_Top:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Top not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_Width:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Width not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_Height:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Height not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_Relativity:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Relativity not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_Colors:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Colors not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_Colors32:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Colors32 not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_NoMemory:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_NoMemory not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_RenderFunc:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_RenderFunc not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_SaveFunc:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_SaveFunc not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_UserData:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_UserData not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_Alignment:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_Alignment not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_ConstantBytesPerRow:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_ConstantBytesPerRow not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_AllowCropping:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_AllowCropping not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_InitialIntScaling:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_InitialIntScaling not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_ClipLeft:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_ClipLeft not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_ClipTop:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_ClipTop not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_ClipWidth:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_ClipWidth not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_ClipHeight:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_ClipHeight not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         case P96PIP_ConstantByteSwapping:
            KPrintF((CONST_STRPTR)"CreateFeature: P96PIP_ConstantByteSwapping not implemented yet. %ld\n", tagPtr->ti_Data);
            break;
         default:
            KPrintF((CONST_STRPTR)"CreateFeature: Unknown tag 0x%08lX data 0x%08lX\n", (ULONG)tagPtr->ti_Tag, (ULONG)tagPtr->ti_Data);
            break;
      }
      tagPtr++;
      tag_counter++;
   }
   KPrintF((CONST_STRPTR)"tag_counter %ld\n", tag_counter);

   // Guardar la estructura en la data privada del driver
   overlay->next = (struct Z3660Overlay *)bi->CardData[0];
   bi->CardData[0] = (ULONG) overlay;
   
   // Marcar que tenemos un overlay activo
   bi->Flags |= BIF_VIDEOWINDOW;
   KPrintF((CONST_STRPTR)"Overlay created successfully.\n");

   return (APTR)overlay;
}
int ConvertToZ3660Format(RGBFTYPE format)
{
    switch (format) {
        case RGBFB_CLUT:
            return 0x00; // Formato Z3660 para 8bpp CLUT
        case RGBFB_R5G6B5:
            return 0x05; // Formato Z3660 para 16bpp R5G6B5
        case RGBFB_R5G5B5:
            return 0x04; // Formato Z3660 para 15bpp R5G5B5
        case RGBFB_A8R8G8B8:
            return 0x0A; // Formato Z3660 para 32bpp ARGB
        default:
            return 0x00; // Por defecto, usar 8bpp CLUT
    }
}
ULONG SetFeatureAttrs(__REGA0(struct BoardInfo *bi), __REGA1(APTR feature), __REGD0(ULONG type), __REGA2(struct TagItem *tags))
{
   volatile uint32_t* registers = (volatile uint32_t*)bi->RegisterBase;
   struct Z3660Overlay *overlay = (struct Z3660Overlay *)feature;
   ULONG result = 0;
   BOOL reconfigure = FALSE;
//   ULONG old_left, old_top, old_width, old_height;
    
   if (!overlay || overlay->type != type)
      return 0;
/*    
   // Guardar valores actuales para detectar cambios
   old_left = overlay->left;
   old_top = overlay->top;
   old_width = overlay->width;
   old_height = overlay->height;
*/
   // Procesar cada tag
   while (tags->ti_Tag != TAG_DONE && tags->ti_Tag != TAG_END) {
      result++;
      switch ((ULONG)tags->ti_Tag) {
         case TAG_MORE:
            tags = (struct TagItem *)tags->ti_Data;
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: TAG_MORE to 0x%08lX\n", (ULONG)tags);
            tags--;
            break;
         case FA_Active:
            overlay->active = (BOOL)tags->ti_Data;
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_Active %lu\n", overlay->active);
            reconfigure = TRUE;
            break;
               
         case FA_Left:
            overlay->left = (ULONG)tags->ti_Data;
//            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_Left %lu\n", overlay->left);
            reconfigure = TRUE;
            break;
               
         case FA_Top:
            overlay->top = (ULONG)tags->ti_Data;
//            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_Top %lu\n", overlay->top);
            reconfigure = TRUE;
            break;
               
         case FA_Width:
            overlay->width = (ULONG)tags->ti_Data;
//            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_Width %lu\n", overlay->width);
            reconfigure = TRUE;
            break;
               
         case FA_Height:
            overlay->height = (ULONG)tags->ti_Data;
//            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_Height %lu\n", overlay->height);
            reconfigure = TRUE;
            break;
               
         case FA_SourceWidth:
            overlay->source_width = (ULONG)tags->ti_Data;
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_SourceWidth %lu\n", overlay->source_width);
            reconfigure = TRUE;
            break;
               
         case FA_SourceHeight:
            overlay->source_height = (ULONG)tags->ti_Data;
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_SourceHeight %lu\n", overlay->source_height);
            reconfigure = TRUE;
            break;
               
         case FA_Format:
            overlay->format = (RGBFTYPE)tags->ti_Data;
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_Format %lu\n", overlay->format);
            reconfigure = TRUE;
            break;
               
         case FA_BitMap:
            overlay->bitmap = (struct BitMap *)tags->ti_Data;
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_BitMap 0x%08lX\n", (ULONG)overlay->bitmap);
            if (overlay->bitmap && bi->SetMemoryMode) {
               bi->SetMemoryMode(bi, overlay->format);
            }
            reconfigure = TRUE;
            break;
               
         case FA_Occlusion:
            overlay->occlusion = (BOOL)tags->ti_Data;
            // Si la ventana est?? siendo ocultada, desactivar temporalmente
            if (overlay->occlusion && overlay->active) {
               // Desactivar overlay
               KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_Occlusion TRUE, disabling overlay\n");               
               ZZ_REGS_WRITE(REG_ZZ_OVERLAY_ENABLE, 0);
               overlay->active = FALSE;
            }
            else if (!overlay->occlusion && !overlay->active) {
               // Reactivar overlay
               KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_Occlusion FALSE, enabling overlay\n");
               ZZ_REGS_WRITE(REG_ZZ_OVERLAY_ENABLE, 1);
               overlay->active = TRUE;
               reconfigure = TRUE;
            }
            break;
         case FA_ClipLeft:
            overlay->ClipLeft = (ULONG)tags->ti_Data;
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_ClipLeft %lu\n", overlay->ClipLeft);
            break;
         case FA_ClipTop:
            overlay->ClipTop = (ULONG)tags->ti_Data;
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_ClipTop %lu\n", overlay->ClipTop);
            break;
         case FA_ClipWidth:
            overlay->ClipWidth = (ULONG)tags->ti_Data;
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_ClipWidth %lu\n", overlay->ClipWidth);
            break;
         case FA_ClipHeight:
            overlay->ClipHeight = (ULONG)tags->ti_Data;
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_ClipHeight %lu\n", overlay->ClipHeight);
            break;
         case FA_ModeInfo:
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_ModeInfo not implemented yet 0x%08lX\n", (ULONG)tags->ti_Data);
            break;
         case FA_Onboard:
            overlay->onboard = tags->ti_Data;
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_Onboard not implemented yet %lu\n", tags->ti_Data);
            break;
         case FA_Brightness:
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: FA_Brightness not implemented yet %lu\n", tags->ti_Data);
            break;
         default:
            KPrintF((CONST_STRPTR)"SetFeatureAttrs: Unknown tag 0x%08lX data 0x%08lX\n", (ULONG)tags->ti_Tag, (ULONG)tags->ti_Data);
            break;
      }
      tags++;
   }
    
   // If there are changes that require hardware reconfiguration
   if (reconfigure && overlay->active) {
      // Get the current mode for comparison
      struct ModeInfo *mi = bi->ModeInfo;
      
      // Calculate hardware values
      ULONG h_start = overlay->left;
      ULONG v_start = overlay->top;
      ULONG h_size = overlay->width;
      ULONG v_size = overlay->height;
      
      // Adjust according to screen size
      if (h_start + h_size > mi->Width)
         h_size = mi->Width - h_start;
      if (v_start + v_size > mi->Height)
         v_size = mi->Height - v_start;
/*            
        // Calculate scale if necessary
        FLOAT x_scale = 1.0;
        FLOAT y_scale = 1.0;
        if (overlay->source_width != h_size || overlay->source_height != v_size) {
            x_scale = (FLOAT)overlay->source_width / h_size;
            y_scale = (FLOAT)overlay->source_height / v_size;
        }
*/        
      // Configure Z3660 hardware registers
      // Here goes the specific code for your card
      // Generic example:
      ZZ_REGS_WRITE(REG_ZZ_OVERLAY_XSTART, h_start);
      ZZ_REGS_WRITE(REG_ZZ_OVERLAY_YSTART, v_start);
      ZZ_REGS_WRITE(REG_ZZ_OVERLAY_WIDTH, h_size);
      ZZ_REGS_WRITE(REG_ZZ_OVERLAY_HEIGHT, v_size);
      ZZ_REGS_WRITE(REG_ZZ_OVERLAY_SOURCE_WIDTH, overlay->source_width);
      ZZ_REGS_WRITE(REG_ZZ_OVERLAY_SOURCE_HEIGHT, overlay->source_height);
      
      // Configure color mode
      ZZ_REGS_WRITE(REG_ZZ_OVERLAY_FORMAT, ConvertToZ3660Format(overlay->format));
      
      // If there is a bitmap, configure memory address
      if (overlay->bitmap) {
         ULONG mem_offset = GetMemoryOffset(bi, (ULONG *) overlay->bitmap);
         ZZ_REGS_WRITE(REG_ZZ_OVERLAY_BASE, mem_offset);
      }
        
      // Activar el overlay si no est?? activo
      if (!overlay->active) {
         overlay->active = TRUE;
         ZZ_REGS_WRITE(REG_ZZ_OVERLAY_ENABLE, 1);
      }
   }
    
   return result;
}

ULONG GetFeatureAttrs(__REGA0(struct BoardInfo *bi), __REGA1(APTR feature), __REGD0(ULONG type), __REGA2(struct TagItem *tags))
{
   struct Z3660Overlay *overlay = (struct Z3660Overlay *)feature;
   ULONG result = 0;
   KPrintF((CONST_STRPTR)"Z3660Overlay at 0x%lx type %ld\n", overlay,overlay->type);
   KPrintF((CONST_STRPTR)"GetFeatureAttrs type %ld\n", type);

   if (!overlay || overlay->type != type)
      return 0;
      
   // Process each tag to return information
   while (tags->ti_Tag != TAG_DONE && tags->ti_Tag != TAG_END) {
      result++;
      switch ((ULONG)tags->ti_Tag) {
         case TAG_MORE:
            tags = (struct TagItem *)tags->ti_Data;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: TAG_MORE to 0x%08lX\n", (ULONG)tags);
            tags--;
            break;
         case FA_Active:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->active;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay active: %lu\n",  *(ULONG*)tags->ti_Data);
            break;
               
         case FA_Left:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->left;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay left: %lu\n",  *(ULONG*)tags->ti_Data);
            break;
               
         case FA_Top:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->top;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay top: %lu\n",  *(ULONG*)tags->ti_Data);
            break;
               
         case FA_Width:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->width;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay width: %lu\n",  *(ULONG*)tags->ti_Data);
            break;
               
         case FA_Height:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->height;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay height: %lu\n", *(ULONG*)tags->ti_Data);
            break;
               
         case FA_SourceWidth:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->source_width;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay source width: %lu\n", *(ULONG*)tags->ti_Data);
            break;
         
         case FA_SourceHeight:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->source_height;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay source height: %lu\n", *(ULONG*)tags->ti_Data);
            break;
         
         case FA_Format:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->format;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay format: %lu\n", *(ULONG*)tags->ti_Data);
            break;
         
         case FA_BitMap:
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: data 0x%08lX\n", tags->ti_Data);
//            *(ULONG*)tags->ti_Data = (ULONG)overlay->bitmap;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: FA_BitMap 0x%08lX\n", (ULONG)overlay->bitmap);
            break;
         
         case FA_Occlusion:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->occlusion;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay occlusion: %lu\n", *(ULONG*)tags->ti_Data);
            break;
         
         case FA_Onboard: {
            // Check if bitmap is in card memory
            //int onboard = (overlay->bitmap && IsOnBoardMemory(bi, (ULONG)overlay->bitmap));
            *(ULONG*)tags->ti_Data = (ULONG)overlay->onboard;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay onboard: %lu\n", *(ULONG*)tags->ti_Data);
         }
            break;
               
         case FA_MinWidth:
            *(ULONG*)tags->ti_Data = 32; // Minimum value in pixels
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay min width: %lu\n", *(ULONG*)tags->ti_Data);
            break;
               
         case FA_MinHeight:
            *(ULONG*)tags->ti_Data = 32; // Minimum value in pixels
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay min height: %lu\n", *(ULONG*)tags->ti_Data);
            break;
               
         case FA_MaxWidth:
            *(ULONG*)tags->ti_Data = bi->MaxHorResolution[0]; // Card maximum
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay max width: %lu\n", *(ULONG*)tags->ti_Data);
            break;
               
         case FA_MaxHeight:
            *(ULONG*)tags->ti_Data = bi->MaxVerResolution[0]; // Card maximum
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Overlay max height: %lu\n", *(ULONG*)tags->ti_Data);
            break;
         case P96PIP_Type:
            if(overlay->type == SFT_VIDEOWINDOW)
               *(ULONG*)tags->ti_Data = PIPT_VideoWindow;
            else if(overlay->type == SFT_MEMORYWINDOW)
               *(ULONG*)tags->ti_Data = PIPT_MemoryWindow;
            else
               *(ULONG*)tags->ti_Data = 0;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: P96PIP_Type %lu\n", *(ULONG*)tags->ti_Data);
            break;
case P96PIP_SourceBitMap:
    KPrintF((CONST_STRPTR)"P96PIP_SourceBitMap RECIBIDO\n");
    KPrintF((CONST_STRPTR)"Direcci??n destino REAL: 0x%08lX\n", (ULONG)tags->ti_Data);
    KPrintF((CONST_STRPTR)"Contenido ANTES: 0x%08lX\n", tags->ti_Data ? *((ULONG*)tags->ti_Data) : 0xFFFFFFFF);
    
    if (tags->ti_Data) {
        *((APTR *)tags->ti_Data) = (APTR)0x50200000L;
        KPrintF((CONST_STRPTR)"Contenido DESPUES: 0x%08lX\n", *((ULONG*)tags->ti_Data));
    }
    
    break;
            
         case P96PIP_SourceRPort:
            KPrintF((CONST_STRPTR)"P96PIP_SourceRPort - address: 0x%08lX\n", (ULONG)tags->ti_Data);
//            if (tags->ti_Data) {
//                *((APTR *)tags->ti_Data) = (APTR)0x50200000L;
//                KPrintF((CONST_STRPTR)"Written 0x50200000 using APTR*\n");
//            }
            break;
        case FA_ModeFormat:
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: FA_ModeFormat not implemented yet.\n");
            break;
         case FA_Colors:
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: FA_Colors not implemented yet.\n");
            break;
         case FA_Colors32:
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: FA_Colors32 not implemented yet.\n");
            break;
         case FA_NoMemory:
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: FA_NoMemory not implemented yet.\n");
            break;
         case FA_ClipLeft:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->ClipLeft;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: FA_ClipLeft %lu\n", *(ULONG*)tags->ti_Data);
            break;
         case FA_ClipTop:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->ClipTop;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: FA_ClipTop %lu\n", *(ULONG*)tags->ti_Data);
            break;
         case FA_ClipWidth:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->ClipWidth;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: FA_ClipWidth %lu\n", *(ULONG*)tags->ti_Data);
            break;
         case FA_ClipHeight:
            *(ULONG*)tags->ti_Data = (ULONG)overlay->ClipHeight;
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: FA_ClipHeight %lu\n", *(ULONG*)tags->ti_Data);
            break;

         default:
            KPrintF((CONST_STRPTR)"GetFeatureAttrs: Unknown tag 0x%08lX data 0x%08lX\n", (ULONG)tags->ti_Tag, (ULONG)tags->ti_Data);
            break;
      }
      tags++;
   }
   
   return result;
}

UWORD DeleteFeature(__REGA0(struct BoardInfo *bi), __REGA1(APTR feature), __REGD0(ULONG type))
{
   volatile uint32_t* registers = (volatile uint32_t*)bi->RegisterBase;
   struct Z3660Overlay *overlay = (struct Z3660Overlay *)feature;
   struct Z3660Overlay **ptr;
   KPrintF((CONST_STRPTR)"DeleteFeature called for overlay at 0x%lx type %ld\n", overlay, type);
   if (!overlay || overlay->type != type)
      return 0;
   KPrintF((CONST_STRPTR)"Deleting overlay feature.\n");

   if (overlay->active) {
      overlay->active = FALSE;
      KPrintF((CONST_STRPTR)"Disabling overlay in hardware.\n");
      ZZ_REGS_WRITE(REG_ZZ_OVERLAY_ENABLE, 0);
   }
   
   // Eliminar de la lista de overlays activos
   ptr = (struct Z3660Overlay **)&bi->CardData[0];
   while (*ptr) {
      if (*ptr == overlay) {
         *ptr = overlay->next;
         break;
      }
      ptr = &((*ptr)->next);
   }
   
   // Free resources
   if (overlay->bitmap && overlay->free_bitmap) {
      bi->FreeCardMem(bi, overlay->bitmap->Planes[0]);
      overlay->bitmap = NULL;
   }
   
   // Free overlay struct
   FreeVec(overlay);
   overlay = NULL;
   KPrintF((CONST_STRPTR)"Overlay struct freed.\n");
   
   if ((APTR)bi->CardData[0] == NULL)
      bi->Flags &= ~BIF_VIDEOWINDOW;
   
   return 1; // ??xito
}

// None of these five really have to do anything.
void SetDAC (__REGA0(struct BoardInfo *b), __REGD0(UWORD d0_), __REGD7(RGBFTYPE format)) {
   (void)b;
   (void)d0_;
   (void)format;
}
void WaitBlitter (__REGA0(struct BoardInfo *b)) {
   (void)b;
}
void SetMemoryMode (__REGA0(struct BoardInfo *b), __REGD7(RGBFTYPE format)) {
   (void)b;
   (void)format;
}
void SetWriteMask (__REGA0(struct BoardInfo *b), __REGD0(UBYTE mask)) {
   (void)b;
   (void)mask;
}
void SetClearMask (__REGA0(struct BoardInfo *b), __REGD0(UBYTE mask)) {
   (void)b;
   (void)mask;
}
void SetReadPlane (__REGA0(struct BoardInfo *b), __REGD0(UBYTE plane)) {
   (void)b;
   (void)plane;
 }
#define abs(x) (x)>0?(x):(-(x))

void init_modeline(volatile uint32_t* registers, uint16 w, uint16 h, uint8 colormode, uint8 scalemode, struct ModeInfo *mode_info) {
   uint16_t mode = ZZVMODE_CUSTOM;
   // We need these 3 modes to be fixed, to force x2 resolutions
   // Some monitors support directly 320x240 and 320x256, but they seem to apply some blur filter to the image.
   // Also 320x200 is not compatible directly with any of my HDMI monitors...
   if (w == 320 && h == 200) {
      mode = ZZVMODE_640x400;
      w = mode_info->Width * 2;
      h = mode_info->Height * 2;
   } else if (w == 320 && h == 240) {
      mode = ZZVMODE_640x480;
      w = mode_info->Width * 2;
      h = mode_info->Height * 2;
   } else if (w == 320 && h == 256) {
      mode = ZZVMODE_640x512;
      w = mode_info->Width * 2;
      h = mode_info->Height * 2;
   }
/*
   if (w == 1280 && h == 720) {
      mode = ZZVMODE_1280x720;
   } else if (w == 800 && h == 600) {
      mode = ZZVMODE_800x600;
   } else if (w == 640 && h == 480) {
      mode = ZZVMODE_640x480;
   } else if (w == 1024 && h == 768) {
      mode = ZZVMODE_1024x768;
   } else if (w == 1280 && h == 1024) {
      mode = ZZVMODE_1280x1024;
   } else if (w == 1920 && h == 1080 && hz == 60) {
      mode = ZZVMODE_1920x1080_60;
   } else if (w == 720 && h == 576) {
      mode = ZZVMODE_720x576;
   } else if (w == 1920 && h == 1080 && hz == 50) {
      mode = ZZVMODE_1920x1080_50;
   } else if (w == 720 && h == 480) {
      mode = ZZVMODE_720x480;
   } else if (w == 640 && h == 512) {
      mode = ZZVMODE_640x512;
   } else if (w == 1600 && h == 1200) {
      mode = ZZVMODE_1600x1200;
   } else if (w == 2560 && h == 1440) {
      mode = ZZVMODE_2560x1440_30;
   } else if (w == 720 && h == 576 && hz == 50) {
      mode = ZZVMODE_720x576_NS_PAL;
   } else if (w == 720 && h == 480 && hz == 50) {
      mode = ZZVMODE_720x480_NS_PAL;
   } else if (w == 720 && h == 576 && hz == 60) {
      mode = ZZVMODE_720x576_NS_NTSC;
   } else if (w == 720 && h == 480 && hz == 60) {
      mode = ZZVMODE_720x480_NS_NTSC;
   } else if (w == 640 && h == 400) {
      mode = ZZVMODE_640x400;
   } else if (w == 1280 && h == 800) {
      mode = ZZVMODE_1280x800;
   } else if (w == 1920 && h == 1200) {
      mode = ZZVMODE_1920x1200;
   } else if (w == 1600 && h == 900) {
      mode = ZZVMODE_1600x900;
   } else if (w == 1680 && h == 1050) {
      mode = ZZVMODE_1680x1050;
   } else if (w == 1366 && h == 768) {
      mode = ZZVMODE_1366x768;
   }*/
   else {
      w = mode_info->Width;
      h = mode_info->Height;
      mode = ZZVMODE_CUSTOM;
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE,VMODE_PARAM_HRES);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE_DATA,mode_info->Width);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE,VMODE_PARAM_VRES);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE_DATA,mode_info->Height);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE,VMODE_PARAM_HSTART);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE_DATA,mode_info->Width+mode_info->HorSyncStart);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE,VMODE_PARAM_HEND);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE_DATA,mode_info->Width+mode_info->HorSyncStart+mode_info->HorSyncSize);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE,VMODE_PARAM_HMAX);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE_DATA,mode_info->HorTotal);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE,VMODE_PARAM_VSTART);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE_DATA,mode_info->Height+mode_info->VerSyncStart);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE,VMODE_PARAM_VEND);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE_DATA,mode_info->Height+mode_info->VerSyncStart+mode_info->VerSyncSize);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE,VMODE_PARAM_VMAX);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE_DATA,mode_info->VerTotal);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE,VMODE_PARAM_POLARITY);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE_DATA,0);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE,VMODE_PARAM_PHZ);
      ZZ_REGS_WRITE(ZZ_CUSTOM_VIDMODE_DATA,mode_info->PixelClock);
   }

    ZZ_REGS_WRITE(REG_ZZ_ORIG_RES, (((uint32_t)w)<<16)|((uint32_t)h));
    ZZ_REGS_WRITE(REG_ZZ_MODE, ((u32)mode)|(((u32)colormode)<<8)|(((u32)scalemode)<<12));
}

// SetGC sets the graphics mode
// This function shall reprogram the hardware to display video data according to the video mode encoded in the
// ModeInfo structure, and shall adjust the border blank according to the value in d0. This value shall also be
// copied into bi->Border. Also, the ModeInfo structure may potentially be modified to reflect the requirements
// of the mode, e.g. mi->Flags may be adjusted.
// Typically, this will reprogram the CRTC and TS (sequencer) registers of a VGA chip to generate the timing for
// the requested mode. The RAMDAC should not be touched, but will be reprogrammed by SetDAC().
// The start address of the bitmap to be displayed will be neither installed by this function, but by SetPanning()
void SetGC(__REGA0(struct BoardInfo *b), __REGA1(struct ModeInfo *mode_info), __REGD0(BOOL border)) {
   uint16_t scale = 0;
   uint16_t colormode;

   if (!b || !mode_info)
      return;

   volatile uint32_t* registers = (volatile uint32_t*)b->RegisterBase;

   b->ModeInfo = mode_info;
   b->Border = border;
   if (mode_info->Width < 320 || mode_info->Height < 200)
      return;

   colormode = mnt_colormode(b->RGBFormat);
   if (colormode == MNTVA_COLOR_NO_USE)
      return;

   if (mode_info->Height >= 400 || mode_info->Width >= 640) {
      scale = 0;
      mode_info->Flags&=~GMF_DOUBLESCAN;
   } else {
      // small doublescan modes are scaled 2x
      // and output as 640x480 wrapped in 800x600 sync
      scale = 3;
      mode_info->Flags|=GMF_DOUBLESCAN;
   }

   init_modeline(registers, mode_info->Width, mode_info->Height, colormode, scale, mode_info);
}

//z3660 -> no scandoubler :(

UWORD SetSwitch(__REGA0(struct BoardInfo *b), __REGD0(UWORD enabled)) {
   UWORD old = b ? (UWORD)b->CardData[ZZ_CARD_DATA_MONITOR_SWITCH] : 1;

   if (!b || !b->RegisterBase)
      return old;

   b->CardData[ZZ_CARD_DATA_MONITOR_SWITCH] = enabled ? 1 : 0;

   if(b->MoniSwitch == enabled)
      return enabled;

   UWORD last=b->MoniSwitch;
   b->MoniSwitch=enabled;

   uint32_t* registers = (uint32_t*)b->RegisterBase;
   // Read the monitor switch config selected
   uint32_t monswitch=ZZ_REGS_READ(REG_ZZ_MONITOR_SWITCH);
#define CIAB_PRA  0xBFD000
#define CIAB_DDRA 0xBFD200
   // Read the DDRA CIAB register
   uint8_t ciab_ddra=*(uint8_t *)CIAB_DDRA;
   if(monswitch&1)
      ciab_ddra|=0x10; // CTS
   if(monswitch&2)
      ciab_ddra|=0x04; // SEL
   if(monswitch&3) // if we have to change CTS and/or SEL
   {
      // Write the DDRA CIAB register
      *(uint8_t *)CIAB_DDRA=ciab_ddra;
      // Read the PRA CIAB register
      uint8_t ciab_pra=*(uint8_t *)CIAB_PRA;
      if (enabled != 0) // RTG
      {
         if(monswitch&1)
         {
            if(monswitch&0x10)
               ciab_pra&=~0x10;// CTS high level
            else
               ciab_pra|=0x10;
         }
         if(monswitch&2)
         {
            if(monswitch&0x20)
               ciab_pra&=~0x04;// SEL high level
            else
               ciab_pra|=0x04;
         }
      }
      else
      {
         if(monswitch&1)
         {
            if(monswitch&0x10)
               ciab_pra|=0x10;
            else
               ciab_pra&=~0x10;// CTS high level
         }
         if(monswitch&2)
         {
            if(monswitch&0x20)
               ciab_pra|=0x04;
            else
               ciab_pra&=~0x04;// SEL high level
         }
      }
      // Write the PRA CIAB register
      *(uint8_t *)CIAB_PRA=ciab_pra;
   }

   if (enabled == 0) // native video
   {

   // capture 24 bit amiga video to 0xe00000
/*
      if (scandoubler_800x600) {
         // slightly adjusted centering
         ZZ_REGS_WRITE(REG_ZZ_PAN, 0x00dff2f8);
      } else {
         ZZ_REGS_WRITE(REG_ZZ_PAN, 0x00e00000);
      }
*/

      // firmware will detect that we are capturing and viewing the capture area
      // and switch to the appropriate video mode (VCAP_MODE)
      ZZ_REGS_WRITE(REG_ZZ_OP_CAPTUREMODE, 1); // capture mode on
   } else { // RTG
      // rtg mode
      ZZ_REGS_WRITE(REG_ZZ_OP_CAPTUREMODE, 0); // capture mode off

      if(b->ModeInfo) {
         SetGC(b, b->ModeInfo, b->Border);
      } else {
         KPrintF((CONST_STRPTR)"SetSwitch: mode_info ptr is null");
      }
   }
   return last;
}

void SetPanning(__REGA0(struct BoardInfo *b), __REGA1(UBYTE *addr), __REGD0(UWORD width), __REGD3(UWORD height), __REGD1(WORD x_offset), __REGD2(WORD y_offset), __REGD7(RGBFTYPE format)) {
   (void)height;
   if (!b) return;
   b->XOffset = x_offset;
   b->YOffset = y_offset;
   uint32_t* registers = (uint32_t *)b->RegisterBase;

//   if(b->CardBase->LibBase.lib_Version >= 44)
//   {
//    ... make something with height?  
//   }
   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->offset[0] = ((uint32_t)addr - (uint32_t)b->MemoryBase);

      gfxdata->x[0] = x_offset;
      gfxdata->y[0] = y_offset;
      gfxdata->x[1] = width;
      gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8)panning_colormode(mnt_colormode(format));
      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_PAN);
   } else {
      uint32_t offset = ((uint32_t)addr - (uint32_t)b->MemoryBase);

      ZZ_REGS_WRITE(REG_ZZ_X1, x_offset);
      ZZ_REGS_WRITE(REG_ZZ_Y1, y_offset);
      ZZ_REGS_WRITE(REG_ZZ_X2, width);
      ZZ_REGS_WRITE(REG_ZZ_COLORMODE, panning_colormode(mnt_colormode(format)));
      ZZ_REGS_WRITE(REG_ZZ_PAN, offset);
   }
}

void SetColorArray(__REGA0(struct BoardInfo *b), __REGD0(UWORD start), __REGD1(UWORD num)) {
   if (!b || !num)
      return;

   uint32_t* registers = (uint32_t*)b->RegisterBase;
   struct CLUTEntry *clut = (start >= 256) ? b->SecondaryCLUT : b->CLUT;
   UWORD count = (num > 256) ? 256 : num;
   KPrintF((CONST_STRPTR)"SetColorArray start %ld num %ld\n", (int32_t)start, (int32_t)num);
   KPrintF((CONST_STRPTR)"j %ld\n", (int32_t)j);

   if (start >= 256) {
      // Select secondary palette if start index is above 255
      if (!b->CardData[ZZ_CARD_DATA_SECONDARY_PALETTE]) {
         ZZ_REGS_WRITE(REG_ZZ_USER1, CARD_FEATURE_SECONDARY_PALETTE);
         ZZ_REGS_WRITE(REG_ZZ_SET_FEATURE, 1);
         b->CardData[ZZ_CARD_DATA_SECONDARY_PALETTE] = 1;
      }
   }
   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      for (int i = 0; i < count; i++) {
         int ci = (start + i) & 0xFF;
         gfxdata->clut1[i * 3]     = clut[ci].Red;
         gfxdata->clut1[i * 3 + 1] = clut[ci].Green;
         gfxdata->clut1[i * 3 + 2] = clut[ci].Blue;
      }
      gfxdata->user[0] = start;
      gfxdata->user[1] = count;
      gfxdata->u8_user[0] = (start >= 256) ? 1 : 0;
      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_SET_PALETTE);
   } else {
      int op = (start >= 256) ? 19 : 3; // 3 OP_PALETTE, 19 OP_PALETTE_HI
      for (int i = 0; i < count; i++) {
         int ci = (start + i) & 0xFF;
         uint32_t xrgb = ((uint32_t)ci << 24) | ((uint32_t)clut[ci].Red << 16) | ((uint32_t)clut[ci].Green << 8) | ((uint32_t)clut[ci].Blue);
         KPrintF((CONST_STRPTR)"Setting color index %ld: R=%d G=%d B=%d\n", (int32_t)ci, clut[ci].Red, clut[ci].Green, clut[ci].Blue);
         ZZ_REGS_WRITE(REG_ZZ_OP_DATA,xrgb);
         ZZ_REGS_WRITE(REG_ZZ_OP,op);
//         ZZ_REGS_WRITE(REG_ZZ_OP_NOP,0); // NOP
      }
   }
}

uint16_t calc_pitch_bytes(uint16_t w, uint16_t colormode) {
   switch (colormode) {
      case MNTVA_COLOR_8BIT:
         return w;
      case MNTVA_COLOR_16BIT565:
      case MNTVA_COLOR_15BIT:
         return w << 1;
      case MNTVA_COLOR_32BIT:
         return w << 2;
   }

	return 0;
}

uint16_t pitch_to_shift(uint16_t p) {
   if (p == 8192) return 13;
   if (p == 4096) return 12;
   if (p == 2048) return 11;
   if (p == 1024) return 10;
   if (p == 512)  return 9;
   if (p == 256)  return 8;
   return 0;
}

UWORD CalculateBytesPerRow(__REGA0(struct BoardInfo *b), __REGD0(UWORD width), __REGD1(UWORD height), __REGA1(struct ModeInfo *mode_info), __REGD7(RGBFTYPE format)) {
   (void)height;
   (void)mode_info;
   if (!b)
      return 0;
	if (!supported_rgb_format(format))
		return 0;

   return calc_pitch_bytes(width, mnt_colormode(format));
}

APTR CalculateMemory(__REGA0(struct BoardInfo *b), __REGA1(APTR addr), __REGD0(struct RenderInfo *ri), __REGD7(RGBFTYPE format)) {
   (void)ri;
   if (!b || !supported_rgb_format(format))
      return NULL;
   return addr;
}

ULONG GetCompatibleFormats(__REGA0(struct BoardInfo *b), __REGD7(RGBFTYPE format)) {
   if (!supported_rgb_format(format))
      return 0;

   return b ? b->RGBFormats : ZZ_SUPPORTED_RGB_FORMATS;
}

UWORD SetDisplay(__REGA0(struct BoardInfo *b), __REGD0(UWORD enabled)) {
	UWORD old = b ? (UWORD)b->CardData[ZZ_CARD_DATA_DISPLAY_ENABLED] : 1;

	if (b)
		b->CardData[ZZ_CARD_DATA_DISPLAY_ENABLED] = enabled ? 1 : 0;
	// No firmware display-blanking register is exposed; keep sync and report
	// the previous logical state as required by the P96 callback contract.
	return old;
}
// ResolvePixelClock - computes a pixel clock index from a target frequency
// Inputs:
//          a0  struct BoardInfo *bi
//          a1  struct ModeInfo *mi
//          d0  LONG pixelclock
//          d7  RGBFTYPE RGBFormat
// Result:   
//          d0  ULONG index
// This function shall convert a pixel clock in Hz to an index in an internal lookup table that can later be used
// to check for the actual pixel clock. It shall also fill in the Numerator, Denominator and PixelCLock members of
// the ModeInfo structure passed in. It may also clear GMF_INTERLACE and/or GMF_DOUBLECLOCK flags if the board 
// does not support the corresponding feature, or set GMF_DOUBLECLOCK if a particular pixel clock requires double-clocking.
// It shall also install the effective pixel clock (in Hz) into mi->PixelClock of the ModeInfo structure.
//  The information in the ModeInfo structure is later used to adjust the timing of the VGA chip through SetPixelClock.
LONG ResolvePixelClock(__REGA0(struct BoardInfo *b), __REGA1(struct ModeInfo *mode_info), __REGD0(ULONG pixel_clock), __REGD7(RGBFTYPE format)) {
   if (!b || !mode_info || !supported_rgb_format(format))
      return -1;

   mode_info->PixelClock = pixel_clock;
   mode_info->Flags&=~(GMF_INTERLACE|GMF_DOUBLESCAN);
   mode_info->pll1.Clock = 1;
   mode_info->pll2.ClockDivide = 1;
   return (mode_info->PixelClock-PIXELCLOCK_MHZ_MIN*1000000)/250000;
}

ULONG GetPixelClock(__REGA0(struct BoardInfo *b), __REGA1(struct ModeInfo *mode_info), __REGD0(ULONG index), __REGD7(RGBFTYPE format)) {
   if (!b || !mode_info || !supported_rgb_format(format))
      return 0;

   KPrintF((CONST_STRPTR)"GetPixelClock() index %ld,mode_info->PixelClock %ld\n", index, mode_info->PixelClock);
   return (index*250000+PIXELCLOCK_MHZ_MIN*1000000);
}
// SetClock sets the pixel clock from a clock index
// This function shall program the pixel clock of the chip according to the mode stored in bi->ModeInfo and, potentially,
// other settings such as bi->RGBFormat.
// This function will typically program the VGA sequencer registers according to the numerator and denominator members of
// the ModeInfo structure which is currently active in the BoardInfo, namely the mi->Numerator and mi->Denominator members
// of the ModeInfo structure. This clock information is deposited there by ResolvePixelClock.
void SetClock (__REGA0(struct BoardInfo *b)) {
   (void)b;
   // Do nothing as the pixel clock will be programmed when changing the video mode...
}

#define VBLANK_WAIT_LIMIT 200000UL

static inline BOOL vblank_state(struct BoardInfo *b) {
   volatile uint32_t* registers =(volatile uint32_t*)b->RegisterBase;
   return (ZZ_REGS_READ(REG_ZZ_VBLANK_STATUS) != 0);
}

void WaitVerticalSync(__REGA0(struct BoardInfo *b), __REGD0(BOOL end)) {
   (void)end;
   ULONG guard = VBLANK_WAIT_LIMIT;

   if (!b || !b->RegisterBase)
      return;

   if (end) {
      while (vblank_state(b) && --guard) { }
      } else {
      while (!vblank_state(b) && --guard) { }
   }
}

BOOL GetVSyncState(__REGA0(struct BoardInfo *b), __REGD0(BOOL expected)) {
   if (!b || !b->RegisterBase)
      return expected;

   return vblank_state(b);
}

void FillRect(__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *r), __REGD0(WORD x), __REGD1(WORD y), __REGD2(WORD w), __REGD3(WORD h), __REGD4(ULONG color), __REGD5(UBYTE mask), __REGD7(RGBFTYPE format)) {
   (void)format;
   if (!r) return;
   if (w<1 || h<1) return;

   uint32_t* registers =(uint32_t*)b->RegisterBase;
   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->offset[GFXDATA_DST] = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
      gfxdata->pitch[GFXDATA_DST] = (r->BytesPerRow >> 2);

      gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)mnt_colormode(r->RGBFormat);
      gfxdata->mask = mask;

      gfxdata->rgb[0] = color;
      gfxdata->x[0] = x;
      gfxdata->x[1] = w;
      gfxdata->y[0] = y;
      gfxdata->y[1] = h;

      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_FILLRECT);
   } else {
      uint32_t* registers =(uint32_t*)b->RegisterBase;
      uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);

      ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);
      ZZ_REGS_WRITE(REG_ZZ_RGB, color);
      ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow >> 2);
      ZZ_REGS_WRITE(REG_ZZ_COLORMODE, mnt_colormode(r->RGBFormat));
      ZZ_REGS_WRITE(REG_ZZ_X1, x);
      ZZ_REGS_WRITE(REG_ZZ_Y1, y);
      ZZ_REGS_WRITE(REG_ZZ_X2, w);
      ZZ_REGS_WRITE(REG_ZZ_Y2, h);
      ZZ_REGS_WRITE(REG_ZZ_FILLRECT, mask);
   }
}

void InvertRect(__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *r), __REGD0(WORD x), __REGD1(WORD y), __REGD2(WORD w), __REGD3(WORD h), __REGD4(UBYTE mask), __REGD7(RGBFTYPE format)) {
   (void)format;
   if (!b || !r)
      return;

   uint16_t colormode = mnt_colormode(format);
   if (colormode == MNTVA_COLOR_NO_USE) {
      if (b->InvertRectDefault)
         b->InvertRectDefault(b, r, x, y, w, h, mask, format);
      return;
   }
   mask = direct_color_mask(colormode, mask);
   uint32_t* registers =(uint32_t*)b->RegisterBase;
   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->offset[GFXDATA_DST] = (uint32_t)r->Memory - (uint32_t)b->MemoryBase;
      gfxdata->pitch[GFXDATA_DST] = (r->BytesPerRow >> 2);

      gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)colormode;
      gfxdata->mask = mask;

      gfxdata->x[0] = x;
      gfxdata->x[1] = w;
      gfxdata->y[0] = y;
      gfxdata->y[1] = h;

      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_INVERTRECT);
   } else {
      uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);

      ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);
      ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow >> 2);
      ZZ_REGS_WRITE(REG_ZZ_COLORMODE, mnt_colormode(r->RGBFormat));

      ZZ_REGS_WRITE(REG_ZZ_X1, x);
      ZZ_REGS_WRITE(REG_ZZ_Y1, y);
      ZZ_REGS_WRITE(REG_ZZ_X2, w);
      ZZ_REGS_WRITE(REG_ZZ_Y2, h);

      ZZ_REGS_WRITE(REG_ZZ_INVERTRECT, mask);
   }
}

// This function shall copy a rectangular image region in the video RAM.
void BlitRect(__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *r), __REGD0(WORD x), __REGD1(WORD y), __REGD2(WORD dx), __REGD3(WORD dy), __REGD4(WORD w), __REGD5(WORD h), __REGD6(UBYTE mask), __REGD7(RGBFTYPE format)) {
   (void)format;
   if (w<1 || h<1) return;
   if (!b || !r) return;

   uint16_t colormode = mnt_colormode(format);
   if (colormode == MNTVA_COLOR_NO_USE) {
      if (b->BlitRectDefault)
         b->BlitRectDefault(b, r, x, y, dx, dy, w, h, mask, format);
      return;
   }
   uint32_t* registers =(uint32_t*)b->RegisterBase;

   if (b->CardFlags & CARDFLAG_ZORRO_3) {

      // RenderInfo describes the video RAM containing the source and target rectangle,
      gfxdata->offset[GFXDATA_DST] = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
      gfxdata->offset[GFXDATA_SRC] = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
      gfxdata->pitch[GFXDATA_DST] = (r->BytesPerRow >> 2);

      // x/y are the top-left edge of the source rectangle,
      gfxdata->x[2] = x;
      gfxdata->y[2] = y;

      // dx/dy the top-left edge of the destination rectangle,
      gfxdata->x[0] = dx;
      gfxdata->y[0] = dy;

      // and w/h the dimensions of the rectangle to copy.
      gfxdata->x[1] = w;
      gfxdata->y[1] = h;

      // RGBFormat is the format of the source (and destination); this format shall not be taken from the RenderInfo.
      gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)colormode;

      // Mask is a bitmask that defines which (logical) planes are affected by the copy for planar or chunky bitmaps. It can be ignored for direct color modes.
      gfxdata->mask = mask;

      // Source and destination rectangle may be overlapping, a proper copy operation shall be performed in either case.
      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_COPYRECT);
   } else {
      ZZ_REGS_WRITE(REG_ZZ_Y1, dy);
      ZZ_REGS_WRITE(REG_ZZ_Y2, h);
      ZZ_REGS_WRITE(REG_ZZ_Y3, y);

      ZZ_REGS_WRITE(REG_ZZ_X1, dx);
      ZZ_REGS_WRITE(REG_ZZ_X2, w);
      ZZ_REGS_WRITE(REG_ZZ_X3, x);

      ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow >> 2);
      ZZ_REGS_WRITE(REG_ZZ_COLORMODE, colormode | (mask << 8));

      uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
      ZZ_REGS_WRITE(REG_ZZ_BLIT_SRC, offset);
      ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);

      ZZ_REGS_WRITE(REG_ZZ_COPYRECT, 1);
   }
}

// This function shall copy one rectangle in video RAM to another rectangle in video RAM using a mode given in d6. A mask is not applied.
void BlitRectNoMaskComplete(__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *rs), __REGA2(struct RenderInfo *rt), __REGD0(WORD x), __REGD1(WORD y), __REGD2(WORD dx), __REGD3(WORD dy), __REGD4(WORD w), __REGD5(WORD h), __REGD6(UBYTE minterm), __REGD7(RGBFTYPE format)) {
   (void)format;
   if (w<1 || h<1) return;
   if (!b || !rs || !rt) return;

   uint16_t colormode = mnt_colormode(format);
   if (colormode == MNTVA_COLOR_NO_USE) {
      if (b->BlitRectNoMaskCompleteDefault)
         b->BlitRectNoMaskCompleteDefault(b, rs, rt, x, y, dx, dy, w, h, minterm, format);
      return;
   }
   uint32_t* registers =(uint32_t*)b->RegisterBase;

   // b->BlitRectNoMaskCompleteDefault(b, rs, rt, x, y, dx, dy, w, h, minterm, format);
   // return;

   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      // The source region in video RAM is given by the source RenderInfo in a1 and a position within it in x and y.
      gfxdata->x[2] = x;
      gfxdata->y[2] = y;
      gfxdata->offset[GFXDATA_SRC] = ((uint32_t)rs->Memory - (uint32_t)b->MemoryBase);
      gfxdata->pitch[GFXDATA_SRC] = (rs->BytesPerRow >> 2);

      // The destination region in video RAM is given by the destinaton RenderInfo in a2 and a position within it in dx and dy.
      gfxdata->x[0] = dx;
      gfxdata->y[0] = dy;
      gfxdata->offset[GFXDATA_DST] = ((uint32_t)rt->Memory - (uint32_t)b->MemoryBase);
      gfxdata->pitch[GFXDATA_DST] = (rt->BytesPerRow >> 2);

      // The dimension of the rectangle to copy is in w and h.
      gfxdata->x[1] = w;
      gfxdata->y[1] = h;

      // The mode is in register d6, it uses the Amiga Blitter MinTerms encoding of the graphics.library.
      gfxdata->minterm = minterm;

      // The common RGBFormat of source and destination is in register d7, it shall not be taken from the source or destination RenderInfo.
      gfxdata->u8_user[GFXDATA_U8_COLORMODE] = colormode;

      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_COPYRECT_NOMASK);
   } else {
      ZZ_REGS_WRITE(REG_ZZ_Y1, dy);
      ZZ_REGS_WRITE(REG_ZZ_Y2, h);
      ZZ_REGS_WRITE(REG_ZZ_Y3, y);

      ZZ_REGS_WRITE(REG_ZZ_X1, dx);
      ZZ_REGS_WRITE(REG_ZZ_X2, w);
      ZZ_REGS_WRITE(REG_ZZ_X3, x);

      ZZ_REGS_WRITE(REG_ZZ_COLORMODE, colormode | (minterm << 8));

      ZZ_REGS_WRITE(REG_ZZ_SRC_PITCH, rs->BytesPerRow >> 2);
      uint32_t offset = ((uint32_t)rs->Memory - (uint32_t)b->MemoryBase);
      ZZ_REGS_WRITE(REG_ZZ_BLIT_SRC, offset);

      ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, rt->BytesPerRow >> 2);
      offset = ((uint32_t)rt->Memory - (uint32_t)b->MemoryBase);
      ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);

      ZZ_REGS_WRITE(REG_ZZ_COPYRECT, 2);
   }
}

void BlitTemplate (__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *r), __REGA2(struct Template *t), __REGD0(WORD x), __REGD1(WORD y), __REGD2(WORD w), __REGD3(WORD h), __REGD4(UBYTE mask), __REGD7(RGBFTYPE format)) {
   (void)format;
   if (!b || !r) return;
   if (w<1 || h<1) return;
   if (!t) return;

   uint16_t colormode = mnt_colormode(format);
   if (colormode == MNTVA_COLOR_NO_USE) {
      if (b->BlitTemplateDefault)
         b->BlitTemplateDefault(b, r, t, x, y, w, h, mask, format);
      return;
   }
   mask = direct_color_mask(colormode, mask);
   uint32_t* registers = (uint32_t *)b->RegisterBase;

   if (!(b->CardFlags & CARDFLAG_ZORRO_3)) {
      uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
       ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);
   }

   uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
   if (!(b->CardFlags & CARDFLAG_ZORRO_3)) {
      zz_template_addr = b->MemorySize;
   }

   memcpy((uint8_t*)(((uint32_t)b->MemoryBase)+zz_template_addr), t->Memory, t->BytesPerRow * h);

   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->x[0] = x;
      gfxdata->x[1] = w;
      gfxdata->x[2] = t->XOffset;
      gfxdata->y[0] = y;
      gfxdata->y[1] = h;

      gfxdata->offset[GFXDATA_DST] = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
      gfxdata->offset[GFXDATA_SRC] = zz_template_addr;
      gfxdata->pitch[GFXDATA_DST] = r->BytesPerRow;
      gfxdata->pitch[GFXDATA_SRC] = t->BytesPerRow;

      gfxdata->rgb[0] = t->FgPen;
      gfxdata->rgb[1] = t->BgPen;

      gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)colormode;
      gfxdata->u8_user[GFXDATA_U8_DRAWMODE] = t->DrawMode;
      gfxdata->mask = mask;

      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_RECT_TEMPLATE);
   } else {
      ZZ_REGS_WRITE(REG_ZZ_BLIT_SRC, zz_template_addr);

      ZZ_REGS_WRITE(REG_ZZ_RGB, t->FgPen);
      ZZ_REGS_WRITE(REG_ZZ_RGB2, t->BgPen);

      ZZ_REGS_WRITE(REG_ZZ_SRC_PITCH, t->BytesPerRow);
      ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow);
      ZZ_REGS_WRITE(REG_ZZ_COLORMODE, colormode | (t->DrawMode << 8));
      ZZ_REGS_WRITE(REG_ZZ_X1, x);
      ZZ_REGS_WRITE(REG_ZZ_Y1, y);
      ZZ_REGS_WRITE(REG_ZZ_X2, w);
      ZZ_REGS_WRITE(REG_ZZ_Y2, h);
      ZZ_REGS_WRITE(REG_ZZ_X3, t->XOffset);

      ZZ_REGS_WRITE(REG_ZZ_FILLTEMPLATE, mask);
   }
}

void BlitPattern(__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *r), __REGA2(struct Pattern *pat), __REGD0(WORD x), __REGD1(WORD y), __REGD2(WORD w), __REGD3(WORD h), __REGD4(UBYTE mask), __REGD7(RGBFTYPE format)) {
   (void)format;
   if (!b || !r) return;
   if (w<1 || h<1) return;
   if (!pat) return;

   uint16_t colormode = mnt_colormode(format);
   if (colormode == MNTVA_COLOR_NO_USE) {
      if (b->BlitPatternDefault)
         b->BlitPatternDefault(b, r, pat, x, y, w, h, mask, format);
      return;
   }
   mask = direct_color_mask(colormode, mask);
   uint32_t* registers =(uint32_t*)b->RegisterBase;

   if (!(b->CardFlags & CARDFLAG_ZORRO_3)) {
      uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
      ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);
   }

   uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
   if (!(b->CardFlags & CARDFLAG_ZORRO_3)) {
      zz_template_addr = b->MemorySize;
   }

   if (pat->Size > 15) return;
   memcpy((uint8_t*)(((uint32_t)b->MemoryBase) + zz_template_addr), pat->Memory, 2 * (1 << pat->Size));

   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->x[0] = x;
      gfxdata->x[1] = w;
      gfxdata->x[2] = pat->XOffset;
      gfxdata->y[0] = y;
      gfxdata->y[1] = h;
      gfxdata->y[2] = pat->YOffset;

      gfxdata->offset[GFXDATA_DST] = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
      gfxdata->offset[GFXDATA_SRC] = zz_template_addr;
      gfxdata->pitch[GFXDATA_DST] = r->BytesPerRow;

      gfxdata->rgb[0] = pat->FgPen;
      gfxdata->rgb[1] = pat->BgPen;

      gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)colormode;
      gfxdata->u8_user[GFXDATA_U8_DRAWMODE] = pat->DrawMode;
      gfxdata->user[0] = (1 << pat->Size);
      gfxdata->mask = mask;

      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_RECT_PATTERN);
   } else {
      ZZ_REGS_WRITE(REG_ZZ_BLIT_SRC, zz_template_addr);

      ZZ_REGS_WRITE(REG_ZZ_RGB, pat->FgPen);
      ZZ_REGS_WRITE(REG_ZZ_RGB2, pat->BgPen);

      ZZ_REGS_WRITE(REG_ZZ_USER1, mask);
      ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow);
      ZZ_REGS_WRITE(REG_ZZ_COLORMODE, colormode | (pat->DrawMode << 8));
      ZZ_REGS_WRITE(REG_ZZ_X1, x);
      ZZ_REGS_WRITE(REG_ZZ_Y1, y);
      ZZ_REGS_WRITE(REG_ZZ_X2, w);
      ZZ_REGS_WRITE(REG_ZZ_Y2, h);
      ZZ_REGS_WRITE(REG_ZZ_X3, pat->XOffset);
      ZZ_REGS_WRITE(REG_ZZ_Y3, pat->YOffset);

      ZZ_REGS_WRITE(REG_ZZ_FILLTEMPLATE, (1 << pat->Size) | 0x8000);
   }
}

void DrawLine(__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *r), __REGA2(struct Line *l), __REGD0(UBYTE mask), __REGD7(RGBFTYPE format)) {
   (void)format;
   if (!b || !l || !r)
      return;

   uint16_t colormode = mnt_colormode(format);
   if (colormode == MNTVA_COLOR_NO_USE) {
      if (b->DrawLineDefault)
         b->DrawLineDefault(b, r, l, mask, format);
      return;
   }
   mask = direct_color_mask(colormode, mask);
   uint32_t* registers =(uint32_t*)b->RegisterBase;

   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->offset[GFXDATA_DST] = (uint32_t)r->Memory - (uint32_t)b->MemoryBase;
      gfxdata->pitch[GFXDATA_DST] = (r->BytesPerRow >> 2);

      gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)colormode;
      gfxdata->u8_user[GFXDATA_U8_DRAWMODE] = l->DrawMode;
      gfxdata->u8_user[GFXDATA_U8_LINE_PATTERN_OFFSET] = l->PatternShift;
      gfxdata->u8_user[GFXDATA_U8_LINE_PADDING] = l->pad;

      gfxdata->rgb[0] = l->FgPen;
      gfxdata->rgb[1] = l->BgPen;

      gfxdata->x[0] = l->X;
      gfxdata->x[1] = l->dX;
      gfxdata->y[0] = l->Y;
      gfxdata->y[1] = l->dY;

      gfxdata->user[0] = l->Length;
      gfxdata->user[1] = l->LinePtrn;
      gfxdata->user[2] = ((l->PatternShift << 8) | l->pad);

      gfxdata->mask = mask;

      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_DRAWLINE);
   } else {
      uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);

      ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);
      ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow >> 2);

      ZZ_REGS_WRITE(REG_ZZ_COLORMODE, colormode | (l->DrawMode << 8));

      ZZ_REGS_WRITE(REG_ZZ_RGB, l->FgPen);

      ZZ_REGS_WRITE(REG_ZZ_RGB2, l->BgPen);

      ZZ_REGS_WRITE(REG_ZZ_X1, l->X);
      ZZ_REGS_WRITE(REG_ZZ_Y1, l->Y);
      ZZ_REGS_WRITE(REG_ZZ_X2, l->dX);
      ZZ_REGS_WRITE(REG_ZZ_Y2, l->dY);
      ZZ_REGS_WRITE(REG_ZZ_USER1, l->Length);
      ZZ_REGS_WRITE(REG_ZZ_X3, l->LinePtrn);
      ZZ_REGS_WRITE(REG_ZZ_Y3, l->PatternShift | (l->pad << 8));

      ZZ_REGS_WRITE(REG_ZZ_DRAWLINE, mask);
   }
}
/*
struct BitMap * ZZ_AllocBitMap(__REGA0(struct BoardInfo *b), __REGD0(ULONG width), __REGD1(ULONG height), __REGA1(struct TagItem *tags)) {
	if (!(b->CardFlags & CARDFLAG_ZORRO_3))
		return NULL;

	MNTZZ9KRegs* registers = (MNTZZ9KRegs*)b->RegisterBase;
	ULONG rgbformat = RGBFB_CLUT;
	ULONG bytesperrow_override = 0;

	struct TagItem *tag = tags;
	while (tag && tag->ti_Tag != TAG_DONE) {
		switch (tag->ti_Tag) {
			case TAG_IGNORE: break;
			case TAG_SKIP: tag += tag->ti_Data; break;
			case TAG_MORE: tag = (struct TagItem *)tag->ti_Data; continue;
			case ABMA_RGBFormat: rgbformat = tag->ti_Data; break;
			case ABMA_NoMemory: return NULL;
			case ABMA_ConstantBytesPerRow: bytesperrow_override = tag->ti_Data; break;
			default: break;
		}
		tag++;
	}

	uint16_t bytesperrow = bytesperrow_override ? bytesperrow_override :
		CalculateBytesPerRow(b, NULL, width, height, rgbformat);
	uint32_t size = (uint32_t)bytesperrow * height;
	if (!size) return NULL;

	dmy_cache
	gfxdata->u8_user[1] = 1;
	gfxdata->offset[1] = size;
	zzwrite16(&registers->blitter_acc_op, ACC_OP_ALLOC_SURFACE);

	uint32_t card_offset = gfxdata->offset[0];
	if (!card_offset) return NULL;

	struct BitMap *bm = (struct BitMap *)AllocMem(sizeof(struct BitMap), MEMF_PUBLIC | MEMF_CLEAR);
	if (!bm) {
		gfxdata->offset[0] = card_offset;
		gfxdata->u8_user[0] = 0;
		zzwrite16(&registers->blitter_acc_op, ACC_OP_FREE_SURFACE);
		return NULL;
	}

	bm->BytesPerRow = bytesperrow;
	bm->Rows = height;
	bm->Depth = 1;
	bm->Planes[0] = (PLANEPTR)((uint32_t)b->MemoryBase + card_offset);

	return bm;
}

BOOL ZZ_FreeBitMap(__REGA0(struct BoardInfo *b), __REGA1(struct BitMap *bm), __REGA2(struct TagItem *tags)) {
	if (!bm) return FALSE;

	if (b->CardFlags & CARDFLAG_ZORRO_3) {
		MNTZZ9KRegs* registers = (MNTZZ9KRegs*)b->RegisterBase;
		uint32_t card_offset = (uint32_t)bm->Planes[0] - (uint32_t)b->MemoryBase;
		dmy_cache
		gfxdata->offset[0] = card_offset;
		gfxdata->u8_user[0] = 0;
		zzwrite16(&registers->blitter_acc_op, ACC_OP_FREE_SURFACE);
	}

	FreeMem(bm, sizeof(struct BitMap));
	return TRUE;
}

 */
// This function shall blit a planar bitmap anywhere in the 68K address space into a chunky bitmap in video RAM.
// The source bitmap that contains the data to be blitted is in the bm argument.
// If one of its plane pointers is 0x0, the source data of that bitplane shall be considered to consist of all-zeros.
// If one of its plane pointers is 0xffffffff, the data in this bitplane shall be considered to be all ones.
void BlitPlanar2Chunky(__REGA0(struct BoardInfo *b), __REGA1(struct BitMap *bm), __REGA2(struct RenderInfo *r), __REGD0(SHORT x), __REGD1(SHORT y), __REGD2(SHORT dx), __REGD3(SHORT dy), __REGD4(SHORT w), __REGD5(SHORT h), __REGD6(UBYTE minterm), __REGD7(UBYTE mask)) {
   if (!b || !r || !bm)
      return;
   if (w<1 || h<1)
      return;

   // b->BlitPlanar2ChunkyDefault(b, bm, r, x, y, dx, dy, w, h, minterm, mask);
   // return;

   uint32_t* registers = (uint32_t*)b->RegisterBase;
   uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
   uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
   uint16_t zz_mask = mask;
   uint8_t cur_plane = 0x01;

   uint16_t line_size = planar_line_bytes_padded(w);
   uint32_t output_plane_size = line_size * h;

   if (output_plane_size * bm->Depth > 0xFFFF && (!(b->CardFlags & CARDFLAG_ZORRO_3))) {
      b->BlitPlanar2ChunkyDefault(b, bm, r, x, y, dx, dy, w, h, minterm, mask);
      return;
   }

   if (!(b->CardFlags & CARDFLAG_ZORRO_3)) {
      zz_template_addr = b->MemorySize;
   }

   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->offset[GFXDATA_DST] = (uint32_t)r->Memory - (uint32_t)b->MemoryBase;
      gfxdata->offset[GFXDATA_SRC] = zz_template_addr;
      gfxdata->pitch[GFXDATA_DST] = (r->BytesPerRow >> 2);
      gfxdata->pitch[GFXDATA_SRC] = line_size;

      gfxdata->mask = mask;
      gfxdata->minterm = minterm;
   } else {
      ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);
      ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow >> 2);
      ZZ_REGS_WRITE(REG_ZZ_COLORMODE, mnt_colormode(r->RGBFormat) | (minterm << 8));
      ZZ_REGS_WRITE(REG_ZZ_BLIT_SRC, zz_template_addr);
      ZZ_REGS_WRITE(REG_ZZ_SRC_PITCH, line_size);
   }

   for (int16_t i = 0; i < bm->Depth; i++) {
      uint16_t x_offset = (x >> 3);
      if ((uint32_t)bm->Planes[i] == 0xFFFFFFFF) {
         memset((uint8_t*)(((uint32_t)b->MemoryBase)+zz_template_addr), 0xFF, output_plane_size);
      }
      else if (bm->Planes[i] != NULL) {
         uint8_t* bmp_mem = (uint8_t*)bm->Planes[i] + (y * bm->BytesPerRow) + x_offset;
         uint8_t* zz_dest = (uint8_t*)(((uint32_t)b->MemoryBase)+zz_template_addr);
         for (int16_t y_line = 0; y_line < h; y_line++) {
            memcpy(zz_dest, bmp_mem, line_size);
            zz_dest += line_size;
            bmp_mem += bm->BytesPerRow;
         }
      }
      else {
         zz_mask &= (cur_plane ^ 0xFF);
      }
      cur_plane <<= 1;
      zz_template_addr += output_plane_size;
   }

   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->x[0] = (x & 0x07);
      gfxdata->x[1] = dx;
      gfxdata->x[2] = w;
      gfxdata->y[1] = dy;
      gfxdata->y[2] = h;

      gfxdata->user[0] = zz_mask;
      gfxdata->user[1] = bm->Depth;

      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_P2C);
   } else {
      ZZ_REGS_WRITE(REG_ZZ_X1, x & 0x07);
      ZZ_REGS_WRITE(REG_ZZ_X2, dx);
      ZZ_REGS_WRITE(REG_ZZ_Y2, dy);
      ZZ_REGS_WRITE(REG_ZZ_X3, w);
      ZZ_REGS_WRITE(REG_ZZ_Y3, h);

      ZZ_REGS_WRITE(REG_ZZ_USER2, zz_mask);

      ZZ_REGS_WRITE(REG_ZZ_P2C, mask | bm->Depth << 8);
   }
}

void BlitPlanar2Direct(__REGA0(struct BoardInfo *b), __REGA1(struct BitMap *bm), __REGA2(struct RenderInfo *r), __REGA3(struct ColorIndexMapping *clut), __REGD0(SHORT x), __REGD1(SHORT y), __REGD2(SHORT dx), __REGD3(SHORT dy), __REGD4(SHORT w), __REGD5(SHORT h), __REGD6(UBYTE minterm), __REGD7(UBYTE mask)) {
   if (!b || !r || !bm || !clut)
      return;
   if (w<1 || h<1)
      return;

   // b->BlitPlanar2DirectDefault(b, bm, r, clut, x, y, dx, dy, w, h, minterm, mask);
   // return;

   uint32_t* registers = (uint32_t*)b->RegisterBase;
   uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
   uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
   uint16_t zz_mask = mask;
   uint8_t cur_plane = 0x01;

   uint16_t line_size = planar_line_bytes(x, w);
   uint32_t output_plane_size = line_size * h;
   uint32_t staged_size = (256 << 2) + (output_plane_size * bm->Depth);

   if (staged_size > 0xFFFF && (!(b->CardFlags & CARDFLAG_ZORRO_3))) {
      b->BlitPlanar2DirectDefault(b, bm, r, clut, x, y, dx, dy, w, h, minterm, mask);
      return;
   }

   if (!(b->CardFlags & CARDFLAG_ZORRO_3)) {
      zz_template_addr = b->MemorySize;
   }

   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->offset[GFXDATA_DST] = (uint32_t)r->Memory - (uint32_t)b->MemoryBase;
      gfxdata->offset[GFXDATA_SRC] = zz_template_addr;
      gfxdata->pitch[GFXDATA_DST] = (r->BytesPerRow >> 2);
      gfxdata->pitch[GFXDATA_SRC] = line_size;
      gfxdata->rgb[0] = clut->ColorMask;

      gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)mnt_colormode(r->RGBFormat);
      gfxdata->mask = mask;
      gfxdata->minterm = minterm;
   } else {
      ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);
      ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow >> 2);
      ZZ_REGS_WRITE(REG_ZZ_COLORMODE, mnt_colormode(r->RGBFormat) | (minterm << 8));
      ZZ_REGS_WRITE(REG_ZZ_BLIT_SRC, zz_template_addr);
      ZZ_REGS_WRITE(REG_ZZ_SRC_PITCH, line_size);
      ZZ_REGS_WRITE(REG_ZZ_RGB, clut->ColorMask);
   }

   memcpy((uint8_t*)(((uint32_t)b->MemoryBase)+zz_template_addr), clut->Colors, (256 << 2));
   zz_template_addr += (256 << 2);

   for (int16_t i = 0; i < bm->Depth; i++) {
      uint16_t x_offset = (x >> 3);
      if ((uint32_t)bm->Planes[i] == 0xFFFFFFFF) {
         memset((uint8_t*)(((uint32_t)b->MemoryBase)+zz_template_addr), 0xFF, output_plane_size);
      }
      else if (bm->Planes[i] != NULL) {
         uint8_t* bmp_mem = (uint8_t*)bm->Planes[i] + (y * bm->BytesPerRow) + x_offset;
         uint8_t* zz_dest = (uint8_t*)(((uint32_t)b->MemoryBase)+zz_template_addr);
         for (int16_t y_line = 0; y_line < h; y_line++) {
            memcpy(zz_dest, bmp_mem, line_size);
            zz_dest += line_size;
            bmp_mem += bm->BytesPerRow;
         }
      }
      else {
         zz_mask &= (cur_plane ^ 0xFF);
      }
      cur_plane <<= 1;
      zz_template_addr += output_plane_size;
   }

   if(b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->x[0] = (x & 0x07);
      gfxdata->x[1] = dx;
      gfxdata->x[2] = w;
      gfxdata->y[1] = dy;
      gfxdata->y[2] = h;

      gfxdata->user[0] = zz_mask;
      gfxdata->user[1] = bm->Depth;

//      unsigned long mi=minterm;
//      unsigned long ma=mask;
//      unsigned long zm=zz_mask;
//      unsigned long cm=clut->ColorMask;
//      unsigned long cf=r->RGBFormat;
//      KPrintF((CONST_STRPTR)"minterm = %ld, mask=%ld, zzmask=0x%08lX, colormask=0x%08lX, colorformat=%ld\n", mi, ma, zm, cm, cf);

      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_P2D);
   } else {
      ZZ_REGS_WRITE(REG_ZZ_X1, x & 0x07);
      ZZ_REGS_WRITE(REG_ZZ_X2, dx);
      ZZ_REGS_WRITE(REG_ZZ_Y2, dy);
      ZZ_REGS_WRITE(REG_ZZ_X3, w);
      ZZ_REGS_WRITE(REG_ZZ_Y3, h);

      ZZ_REGS_WRITE(REG_ZZ_USER2, zz_mask);

      ZZ_REGS_WRITE(REG_ZZ_P2D, mask | bm->Depth << 8);
   }
}

BOOL SetSprite(__REGA0(struct BoardInfo *b), __REGD0(BOOL what), __REGD7(RGBFTYPE format)) {
   (void)what;
   (void)format;
	if (!b || !b->RegisterBase)
		return FALSE;
   uint32_t* registers = (uint32_t*)b->RegisterBase;
   /*
    * Keep the legacy ZZ9000 behavior here: the existing firmware/driver
    * path expects SetSprite() to assert hardware-sprite visibility even
    * when P96 passes FALSE during setup or screen transitions.
    */
      ZZ_REGS_WRITE(REG_ZZ_SPRITE_BITMAP, 1);
   return TRUE;
}

void SetSpritePosition(__REGA0(struct BoardInfo *b), __REGD0(WORD x), __REGD1(WORD y), __REGD7(RGBFTYPE format)) {
   (void)format;
   if (!b || !b->RegisterBase)
      return;

   uint32_t* registers = (uint32_t*)b->RegisterBase;
   // The firmware stores b->XOffset/YOffset through SetPanning/SetSpriteImage
   // and applies the P96 sprite offset adjustment when positioning the sprite.
   b->MouseX = x;
   b->MouseY = y;

   if(b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->x[0] = x - b->XOffset;
      gfxdata->y[0] = y - b->YOffset + b->YSplit;

      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_SPRITE_XY);
   } else {
      ZZ_REGS_WRITE(REG_ZZ_X1, x - b->XOffset);
      ZZ_REGS_WRITE(REG_ZZ_Y1, y - b->YOffset + b->YSplit);
      ZZ_REGS_WRITE(REG_ZZ_SPRITE_Y, 1);
   }
}

void SetSpriteImage(__REGA0(struct BoardInfo *b), __REGD7(RGBFTYPE format)) {
   (void)format;
   uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
   uint32_t* registers = (uint32_t*)b->RegisterBase;
   if (!(b->CardFlags & CARDFLAG_ZORRO_3)) {
      zz_template_addr = b->MemorySize;
   }
   uint32_t flags = b->Flags;
   int hiressprite = 1;
   int doubledsprite = 0;
   if (flags & BIF_HIRESSPRITE)
      hiressprite = 2;
   if (flags & BIF_BIGSPRITE)
      doubledsprite = 1;
   uint16_t data_size = ((b->MouseWidth >> 3) * 2 * hiressprite) * (b->MouseHeight);
   memcpy((uint8_t*)(((uint32_t)b->MemoryBase)+zz_template_addr), b->MouseImage+2*hiressprite, data_size);
   if(b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->offset[1] = zz_template_addr;
      gfxdata->x[2] = doubledsprite;
      gfxdata->y[2] = hiressprite-1;
      gfxdata->x[0] = b->XOffset;
      gfxdata->x[1] = b->MouseWidth;
      gfxdata->y[0] = b->YOffset;
      gfxdata->y[1] = b->MouseHeight;

      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_SPRITE_BITMAP);
   } else {
      ZZ_REGS_WRITE(REG_ZZ_BLIT_SRC, zz_template_addr);

      ZZ_REGS_WRITE(REG_ZZ_X3, doubledsprite);
      ZZ_REGS_WRITE(REG_ZZ_Y3, hiressprite-1);
      ZZ_REGS_WRITE(REG_ZZ_X1, b->XOffset);
      ZZ_REGS_WRITE(REG_ZZ_X2, b->MouseWidth);
      ZZ_REGS_WRITE(REG_ZZ_Y1, b->YOffset);
      ZZ_REGS_WRITE(REG_ZZ_Y2, b->MouseHeight);

      ZZ_REGS_WRITE(REG_ZZ_SPRITE_BITMAP, 0);
   }
}

void SetSpriteColor (__REGA0(struct BoardInfo *b), __REGD0(UBYTE idx), __REGD1(UBYTE R), __REGD2(UBYTE G), __REGD3(UBYTE B), __REGD7(RGBFTYPE format)) {
   (void)format;
   if (!b) return;
   uint32_t* registers =(uint32_t*)b->RegisterBase;

   if (b->CardFlags & CARDFLAG_ZORRO_3) {
//      ((char *)&gfxdata->rgb[0])[0] = B;
//      ((char *)&gfxdata->rgb[0])[1] = G;
//      ((char *)&gfxdata->rgb[0])[2] = R;
//      ((char *)&gfxdata->rgb[0])[3] = 0x00;
      union rgba {
         struct bgraW {
            uint32_t BGRA32;
         }  l;
         struct bgraB{
            uint8_t B,G,R,A;
         }  b;
      } color;
      color.b.B=B;
      color.b.G=G;
      color.b.R=R;
      color.b.A=0;
      gfxdata->rgb[0]=color.l.BGRA32;
      gfxdata->u8offset = idx + 1;

      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_SPRITE_COLOR);
   } else {
      ZZ_REGS_WRITE(REG_ZZ_USER1, R);
      ZZ_REGS_WRITE(REG_ZZ_USER2, B | (G << 8));
      ZZ_REGS_WRITE(REG_ZZ_SPRITE_COLORS, idx + 1);
   }
}

BOOL EnableSoftSprite(__REGA0(struct BoardInfo *b),__REGD0(ULONG fmtflags),__REGA1(struct ModeInfo *mode_info))
{
   (void)b;
   (void)fmtflags;
   (void)mode_info;
   // software sprite shall be enabled for a particular mode?
   return(FALSE);
}
void SetSplitPosition(__REGA0(struct BoardInfo *b),__REGD0(SHORT pos)) {
   if (!b)
      return;

   b->YSplit = pos;
   if (!b->RegisterBase)
      return;

   uint32_t* registers =(uint32_t*)b->RegisterBase;
   uint32_t offset = 0;
   
   if (pos != 0) {
      if (!b->VisibleBitMap || !b->VisibleBitMap->Planes[0]) {
         pos = 0;
         b->YSplit = 0;
      } else {
         offset = ((uint32_t)b->VisibleBitMap->Planes[0]) - ((uint32_t)b->MemoryBase);
      }
   }

   if (b->CardFlags & CARDFLAG_ZORRO_3) {
      gfxdata->offset[0] = offset;
      gfxdata->y[0] = pos;
      ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_SET_SPLIT_POS);
   } else {
      ZZ_REGS_WRITE(REG_ZZ_BLIT_SRC, offset);
      ZZ_REGS_WRITE(REG_ZZ_SET_SPLIT_POS, pos);
   }
}

static uint32_t device_vectors[] = {
   (uint32_t)OpenLib,
   (uint32_t)CloseLib,
   (uint32_t)ExpungeLib,
   0,
   (uint32_t)FindCard,
   (uint32_t)InitCard,
   -1
};

struct InitTable
{
   ULONG LibBaseSize;
   APTR FunctionTable;
   APTR DataTable;
   APTR InitLibTable;
};

const uint32_t auto_init_tables[4] = {
   sizeof(struct GFXBase),
   (uint32_t)device_vectors,
   0,
   (uint32_t)InitLib,
};
