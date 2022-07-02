/*
 * Z3660 Graphics Card Driver based on MNT ZZ9000 rev 1.6
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 */

/*
 * MNT ZZ9000 Amiga Graphics Card Driver (ZZ9000.card)
 * Copyright (C) 2016-2019, Lukas F. Hartmann <lukas@mntre.com>
 *                          MNT Research GmbH, Berlin
 *                          https://mntre.com
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 */

/* REVISION 1.6 */

#include "gfx.h"
#include "z3660.h"

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/dos.h>
#include <proto/intuition.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <exec/initializers.h>
#include <clib/debug_protos.h>

#include <string.h>

#define KPrintF(...)

static ULONG LibStart(void) {
  return(-1);
}

static const char LibraryName[] = "Z3660.card";
static const char LibraryID[]   = "$VER: Z3660.card 1.6.0 (2022-06-29)\r\n";

__saveds struct MNTGFXBase* OpenLib( __reg("a6") struct MNTGFXBase *MNTGFXBase);
BPTR __saveds CloseLib( __reg("a6") struct MNTGFXBase *MNTGFXBase);
BPTR __saveds ExpungeLib( __reg("a6") struct MNTGFXBase *exb);
ULONG ExtFuncLib(void);
__saveds struct MNTGFXBase* InitLib(__reg("a6") struct ExecBase    *sysbase,
                                    __reg("a0") BPTR          seglist,
                                    __reg("d0") struct MNTGFXBase *exb);

static const APTR FuncTab[] = {
  (APTR)OpenLib,
  (APTR)CloseLib,
  (APTR)ExpungeLib,
  (APTR)ExtFuncLib,

  (APTR)FindCard,
  (APTR)InitCard,
  (APTR)((LONG)-1)
};

struct InitTable
{
  ULONG LibBaseSize;
  APTR  FunctionTable;
  APTR  DataTable;
  APTR  InitLibTable;
};

static struct InitTable InitTab = {
  (ULONG) sizeof(struct MNTGFXBase),
  (APTR) FuncTab,
  (APTR) NULL,
  (APTR) InitLib
};

static const struct Resident ROMTag = {
  RTC_MATCHWORD,
  (struct Resident*)&ROMTag,
  (struct Resident*)&ROMTag + 1,
  RTF_AUTOINIT,
  83,
  NT_LIBRARY,
  0,
  (char *)LibraryName,
  (char *)LibraryID,
  (APTR)&InitTab
};

// Place scratch area right after framebuffer? Might be a horrible idea.
#define Z3_GFXDATA_ADDR  (0x3200000 - 0x10000) //0x31F0000
#define Z3_TEMPLATE_ADDR (0x3210000 - 0x10000)
#define ZZVMODE_800x600 1
#define ZZVMODE_720x576 6
/*
#define zzwrite16(a, b) *a = b;
#define ZZWRITE32(b, c) \
  zzwrite16(b##_hi, ((uint16 *)&c)[0]); \
  zzwrite16(b##_lo, ((uint16 *)&c)[1]);
*/
#define ZZ_WRITE32(b, c) do{(b) = (c);}while(0)

// useful for debugging
void waitclick() {
#define CIAAPRA ((volatile uint8*)0xbfe001)
  // bfe001 http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node012E.html
  while (!(*CIAAPRA & (1<<6))) {
    // wait for left mouse button pressed
  }
  while ((*CIAAPRA & (1<<6))) {
    // wait for left mouse button released
  }
}

static struct MNTGFXBase *MNTGFXBase;
char dummies[128];

__saveds struct MNTGFXBase* InitLib(__reg("a6") struct ExecBase      *sysbase,
                                          __reg("a0") BPTR            seglist,
                                          __reg("d0") struct MNTGFXBase   *exb)
{
  MNTGFXBase = exb;
  return MNTGFXBase;
}

__saveds struct MNTGFXBase* OpenLib(__reg("a6") struct MNTGFXBase *MNTGFXBase)
{
  MNTGFXBase->libNode.lib_OpenCnt++;
  MNTGFXBase->libNode.lib_Flags &= ~LIBF_DELEXP;

  return MNTGFXBase;
}

BPTR __saveds CloseLib(__reg("a6") struct MNTGFXBase *MNTGFXBase)
{
  MNTGFXBase->libNode.lib_OpenCnt--;

  if (!MNTGFXBase->libNode.lib_OpenCnt) {
    if (MNTGFXBase->libNode.lib_Flags & LIBF_DELEXP) {
      return (ExpungeLib(MNTGFXBase));
    }
  }
  return 0;
}

BPTR __saveds ExpungeLib(__reg("a6") struct MNTGFXBase *exb)
{
  struct MNTGFXBase *MNTGFXBase = exb;
  BPTR seglist;
  struct ExecBase *SysBase = *(struct ExecBase **)4L;
  
  if(!MNTGFXBase->libNode.lib_OpenCnt) {
    ULONG negsize, possize, fullsize;
    UBYTE *negptr = (UBYTE *) MNTGFXBase;

    seglist = MNTGFXBase->segList;

    Remove((struct Node *)MNTGFXBase);

    negsize  = MNTGFXBase->libNode.lib_NegSize;
    possize  = MNTGFXBase->libNode.lib_PosSize;
    fullsize = negsize + possize;
    negptr  -= negsize;

    FreeMem(negptr, fullsize);
    return(seglist);
  }

  MNTGFXBase->libNode.lib_Flags |= LIBF_DELEXP;
  return 0;
}

int new_vsync_reg = 0;

ULONG ExtFuncLib(void)
{
  return 0;
}

static LONG zorro_version = 0;
static LONG hwrev = 0;
static LONG fwrev_major = 0;
static LONG fwrev_minor = 0;
static LONG fwrev = 0;
static LONG scandoubler_800x600 = 0;

static struct GFXData *gfxdata;
MNTZZ9KRegs* registers;

#define LOADLIB(a, b) if ((a = (struct a*)OpenLibrary(b,0L))==NULL) { \
    KPrintF("ZZ9000.card: Failed to open %s!\n", b); \
    return 0; \
  } \

int FindCard(__reg("a0") struct RTGBoard* b) {
  struct ConfigDev* cd = NULL;
  struct ExpansionBase *ExpansionBase = NULL;
  struct DOSBase *DOSBase = NULL;
  struct IntuitionBase *IntuitionBase = NULL;
  struct ExecBase *SysBase = *(struct ExecBase **)4L;

  LOADLIB(ExpansionBase, "expansion.library");
  LOADLIB(DOSBase, "dos.library");
  LOADLIB(IntuitionBase, "intuition.library");

  zorro_version = 0;
  if ((cd = (struct ConfigDev*)FindConfigDev(cd,0x6d6e,0x4))) zorro_version = 3;
  else if ((cd = (struct ConfigDev*)FindConfigDev(cd,0x6d6e,0x3))) zorro_version = 2;
  
  // Find Z3 or Z2 model
  if (zorro_version>=2) {

    b->memory = (uint8*)(cd->cd_BoardAddr)+0x100000;

    if (zorro_version==2) {
      b->memory_size = cd->cd_BoardSize-0x20000;
    } else {
      // 13.8 MB for Z3 (safety, will be expanded later)
      // one full HD screen @8bit ~ 2MB
      b->memory_size = 0x3000000 - 0x100000;
      gfxdata = (struct GFXData*)(((uint32)b->memory)+(uint32)Z3_GFXDATA_ADDR);
    }
    b->registers = (uint8*)(cd->cd_BoardAddr);
    registers = b->registers;

    ZZ_WRITE32(registers->zz_debug,(u32)cd->cd_BoardAddr);
    ZZ_WRITE32(registers->zz_debug,(u32)cd->cd_BoardAddr);

//    hwrev = ((uint16*)b->registers)[0];
    fwrev = *(&registers->zz_fw_version);

    fwrev_major = fwrev>>8;
    fwrev_minor = fwrev&0xff;

    KPrintF(LibraryID);
    KPrintF("ZZ9000.card: MNT ZZ9000 found. Zorro version %ld.\n", zorro_version);
    KPrintF("ZZ9000.card: HW Revision: %ld.\n", hwrev);
    KPrintF("ZZ9000.card: FW Revision Major: %ld.\n", fwrev_major);
    KPrintF("ZZ9000.card: FW Revision Minor: %ld.\n", fwrev_minor);

    if (fwrev_major<=1 && fwrev_minor<6) {
      char *alert = "\x00\x14\x14 vX.X: ZZ9000.card v1.6 needs at least firmware (BOOT.bin) v1.6.\x00\x00";
      alert[5]='0'+fwrev_major;
      alert[7]='0'+fwrev_minor;
      DisplayAlert(RECOVERY_ALERT, alert, 52);
      return 0;
    }
/*    else
    {
       char *alert = "\x00\x14\x14 vX.X: OK ZZ9000.card v1.6\x00\x00";
       alert[5]='0'+fwrev_major;
       alert[7]='0'+fwrev_minor;
       DisplayAlert(RECOVERY_ALERT, alert, 52);
    }
*/
    MNTZZ9KRegs* registers = b->registers;
    BPTR f;
    if ((f = Open("ENV:ZZ9000-VCAP-800x600", MODE_OLDFILE))) {
      Close(f);
      KPrintF("ZZ9000.card: 800x600 60hz scandoubler mode.\n");
      scandoubler_800x600 = 1;
//      registers->videocap_vmode = ZZVMODE_800x600; // 60hz
    } else {
      KPrintF("ZZ9000.card: 720x576 50hz scandoubler mode.\n");
      scandoubler_800x600 = 0;
//      registers->videocap_vmode = ZZVMODE_720x576; // 50hz
    }
    
    return 1;
  } else {
    KPrintF("ZZ9000.card: MNT ZZ9000 not found!\n");
    return 0;
  }
}

#define HWSPRITE 1
#define VGASPLIT (1 << 6)
#define FLICKERFIXER (1 << 12)
#define INDISPLAYCHAIN (1 << 20)
#define DIRECTACCESS (1 << 26)

// Values assigned to the ZZ9000 card since P96 3.0
#define ZZ9K_BOARDTYPE 27
#define ZZ9K_CHIPTYPE 19
#define ZZ9K_GCTYPE 16

int InitCard(__reg("a0") struct RTGBoard* b) {
  int max;
  struct ExecBase *SysBase = *(struct ExecBase **)4L;

  b->self = MNTGFXBase;
  b->exec = SysBase;
  b->name = "ZZ9000";
  b->type = ZZ9K_BOARDTYPE;
  b->chip_type = ZZ9K_CHIPTYPE;
  b->controller_type = ZZ9K_GCTYPE;

  b->flags = HWSPRITE | VGASPLIT | INDISPLAYCHAIN | FLICKERFIXER | DIRECTACCESS;
  b->color_formats = RTG_COLOR_FORMAT_CLUT |
//                     RTG_COLOR_FORMAT_RGB888 |
//                     RTG_COLOR_FORMAT_RGB565 |
//                     RTG_COLOR_FORMAT_RGB555 |
//                     RTG_COLOR_FORMAT_BGR565_WEIRD3 |
                     RTG_COLOR_FORMAT_BGRA
//                     RTG_COLOR_FORMAT_ABGR
//                     RTG_COLOR_FORMAT_RGBA
                     ;
  b->sprite_flags = 0;
  b->bits_per_channel = 8;

  for (int i = 0; i < rtg_color_num; i++) {
    b->max_bitmap_w[i] = 8192;
    b->max_bitmap_h[i] = 8192;
    b->max_res_w[i] = 2560;
    b->max_res_h[i] = 1440;
    b->num_pixelclocks[i] = 1;
  }
  b->clock_ram = CLOCK_HZ;

  // no alloc yet
  //b->max_alloc = 0;
  //b->max_alloc_part = 0;

  b->fn_init_dac = (void*)init_dac;
  b->fn_init_mode = (void*)init_mode;

  b->fn_get_pitch = (void*)get_pitch;
  b->fn_map_address = (void*)map_address;

  b->fn_is_bitmap_compatible = (void*)is_bitmap_compatible;
  b->fn_set_palette = (void*)set_palette;
  b->fn_enable_display = (void*)enable_display;
  // FIXME: I've commented this because function acceleration is not working (I have to work more on FPGA-amiga interface)
/*
  b->fn_rect_fill = (void*)rect_fill;
  b->fn_rect_copy = (void*)rect_copy;

  if (fwrev >= (1 << 8 | 3)) {
    // introduced in fw 1.1 (z3) / fw 1.3b (z2)
    // accelerated text drawing
    b->fn_rect_template = (void*)rect_template;
    // accelerated pattern drawing
    b->fn_rect_pattern = (void*)rect_pattern;
  }
  if (fwrev >= (1 << 8 | 4)) {
    // Accelerated line drawing added in FW 1.4
    b->fn_line = (void *)draw_line;
    // Accelerated Planar2Chunky/Direct and
    // InvertRect added in FW 1.4/1.5
    b->fn_p2c = (void *)rect_p2c;
    b->fn_p2d = (void *)rect_p2d;
    b->fn_rect_invert = (void *)rect_invert;
    b->fn_rect_copy_nomask = (void *)rect_copy_nomask;
  }
  */
  b->fn_blitter_wait = (void*)blitter_wait;

  b->fn_get_pixelclock_index = (void*)get_pixelclock_index;
  b->fn_get_pixelclock_hz = (void*)get_pixelclock_hz;
  b->fn_set_clock = (void*)set_clock;

  b->fn_monitor_switch = (void*)monitor_switch;

  b->fn_vsync_wait = (void*)vsync_wait;
  b->fn_is_vsynced = (void*)is_vsynced;
  b->fn_pan = (void*)pan;
  b->fn_set_memory_mode = (void*)set_memory_mode;
  b->fn_set_write_mask = (void*)set_write_mask;
  b->fn_set_clear_mask = (void*)set_clear_mask;
  b->fn_set_read_plane = (void*)set_read_plane;
  
  b->fn_sprite_setup = (void*)sprite_setup;
  b->fn_sprite_xy = (void*)sprite_xy;
  b->fn_sprite_bitmap = (void*)sprite_bitmap;
  b->fn_sprite_colors = (void*)sprite_colors;

  // FIXME: I've commented this because function acceleration is not working (I have to work more on FPGA-amiga interface)
/*
  if (fwrev >= (1 << 8 | 7)) {
    new_vsync_reg = 1;
    if (zorro_version == 3) {
      b->fn_line = (void *)draw_line_dma;
      b->fn_rect_fill = (void *)rect_fill_dma;
      b->fn_rect_copy = (void *)rect_copy_dma;
      b->fn_rect_copy_nomask = (void *)rect_copy_nomask_dma;
      b->fn_rect_template = (void *)rect_template_dma;
      b->fn_rect_pattern = (void *)rect_pattern_dma;
      b->fn_p2c = (void *)rect_p2c_dma;
      b->fn_p2d = (void *)rect_p2d_dma;
      b->fn_rect_invert = (void *)rect_invert_dma;
      b->fn_pan = (void *)pan_dma;
      b->fn_sprite_xy = (void*)sprite_xy_dma;
      b->fn_sprite_bitmap = (void*)sprite_bitmap_dma;
      b->fn_sprite_colors = (void*)sprite_colors_dma;
      b->fn_set_split_pos = (void*)set_split_pos_dma;
    } else {
      b->fn_set_split_pos = (void*)set_split_pos;
    }
  }
*/
  return 1;
}

void init_dac(__reg("a0") struct RTGBoard* b, __reg("d7") uint16 format) {
}
void set_memory_mode(__reg("a0") struct RTGBoard* b, __reg("d7") uint16 format) {
}
void set_read_plane(__reg("a0") struct RTGBoard* b, __reg("d0") uint8 p) {
}
void set_write_mask(__reg("a0") struct RTGBoard* b, __reg("d0") uint8 m) {
}
void set_clear_mask(__reg("a0") struct RTGBoard* b, __reg("d0") uint8 m) {
}
void memory_alloc(__reg("a0") struct RTGBoard* b, __reg("d0") uint32 len, __reg("d1") uint16 s1, __reg("d2") uint16 s2) {
}
void set_clock(__reg("a0") struct RTGBoard* b) {
}

uint32 enable_display(__reg("a0") struct RTGBoard* b, __reg("d0") uint16 enabled) {
  return 1;
}

void fix_vsync(MNTZZ9KRegs* registers) {  
  // video control op: vsync
  *(u16*)((uint32)registers+0x1000) = 0;
  *(u16*)((uint32)registers+0x1002) = 1;
  *(u16*)((uint32)registers+0x1004) = 5; // OP_VSYNC
  *(u16*)((uint32)registers+0x1004) = 0;
  *(u16*)((uint32)registers+0x1002) = 0;
}

uint16_t rtg_to_mnt[16] = {
  MNTVA_COLOR_8BIT,     // 0x00
  MNTVA_COLOR_8BIT,     // 0x01
  0,                    // 0x02
  0,                    // 0x03
  0,                    // 0x04
  MNTVA_COLOR_15BIT,    // 0x05
  0,                    // 0x06
  0,                    // 0x07
  MNTVA_COLOR_32BIT,    // 0x08
  MNTVA_COLOR_32BIT,    // 0x09
  MNTVA_COLOR_16BIT565, // 0x0A
  MNTVA_COLOR_15BIT,    // 0x0B
  0,                    // 0x0C
  MNTVA_COLOR_15BIT,    // 0x0D
  0,                    // 0x0E
  0,                    // 0x0F
};

// Optional dummy read for tricking the 68k cache on processors with occasional garbage output on screen
#ifdef DUMMY_CACHE_READ
  #define dmy_cache memcpy(dummies, (uint32_t *)(uint32_t)0x7F00000, 4);
#else
  #define dmy_cache
#endif

void pan(__reg("a0") struct RTGBoard* b, __reg("a1") uint8* mem, __reg("d0") uint16 w, __reg("d1") int16 x, __reg("d2") int16 y, __reg("d7") uint16 format) {
  MNTZZ9KRegs* registers = b->registers;
  uint32 offset = (mem - (b->memory)) & 0xFFFFFC00;

  b->offset_x = x;
  b->offset_y = y;

  ZZ_WRITE32(registers->pan_ptr, offset);
}

void pan_dma(__reg("a0") struct RTGBoard* b, __reg("a1") uint8* mem, __reg("d0") uint16 w, __reg("d1") int16 x, __reg("d2") int16 y, __reg("d7") uint16 format) {
  dmy_cache
  gfxdata->offset[0] = (mem - (b->memory)) & 0xFFFFFC00;

  b->offset_x = x;
  b->offset_y = y;

  gfxdata->x[0] = x;
  gfxdata->y[0] = y;
  ZZ_WRITE32(registers->blitter_dma_op, OP_PAN);
}

static int toggle = 0;

int is_vsynced(__reg("a0") struct RTGBoard* b, __reg("d0") uint8 p) {
  uint32 vblank_state;

  if (!new_vsync_reg) {
    vblank_state = ((uint32*)b->registers)[REG_ZZ_VBLANK_STATUS];
  }
  else {
    vblank_state = ((uint32*)b->registers)[0x800];
  }

  return vblank_state;  
}

void vsync_wait(__reg("a0") struct RTGBoard* b) {
	/* FIXME
  if (!new_vsync_reg) {
    uint32 vblank_state = ((volatile uint32*)b->registers)[REG_ZZ_VBLANK_STATUS];
    while(vblank_state == 0) {
      vblank_state = ((volatile uint32*)b->registers)[REG_ZZ_VBLANK_STATUS];
    }
    return;
  }

  uint32 vblank_state = ((volatile uint32*)b->registers)[0x800];

  while(vblank_state != 0) {
    vblank_state = ((volatile uint32*)b->registers)[0x800];
  }

  while(vblank_state == 0) {
    vblank_state = ((volatile uint32*)b->registers)[0x800];
  }
  */
}

uint16 calc_pitch_bytes(uint16 w, uint16 colormode) {
  uint16 pitch = w;

  if (colormode == MNTVA_COLOR_15BIT) {
    pitch = w<<1;
  } else {
    pitch = w<<colormode;
  }
  return pitch;
}

uint16 pitch_to_shift(uint16 p) {
  if (p==8192) return 13;
  if (p==4096) return 12;
  if (p==2048) return 11;
  if (p==1024) return 10;
  if (p==512)  return 9;
  if (p==256)  return 8;
  return 0;
}

uint16 get_pitch(__reg("a0") struct RTGBoard* b, __reg("d0") uint16 width, __reg("d7") uint16 format) {
  return calc_pitch_bytes(width, rtg_to_mnt[format]);
}

void init_modeline(MNTZZ9KRegs* registers, uint16 w, uint16 h, uint8 colormode, uint8 scalemode) {
  int hmax,vmax,hstart,hend,vstart,vend;
  uint16 mode = 0;
  uint16 polarity = 0;
  
  if (w==1280 && h==720) {
    mode = 0;
  } else if (w==800 && h==600) {
    mode = 1;
  } else if (w==640 && h==480) {
    mode = 2;
  } else if (w==1024 && h==768) {
    mode = 3;
  } else if (w==1280 && h==1024) {
    mode = 4;
  } else if (w==1920 && h==1080) {
    mode = 5;
  } else if (w==720 && h==576) {
    mode = 6;
  } else if (w==640 && h==512) {
    mode = 9;
  } else if (w==1600 && h==1200 && new_vsync_reg) {
    mode = 10;
  } else if (w==2560 && h==1440 && new_vsync_reg) {
    mode = 11;
  }

  ZZ_WRITE32(registers->mode, mode|(colormode<<8)|(scalemode<<12));
}

void init_mode(__reg("a0") struct RTGBoard* b, __reg("a1") struct ModeInfo* m, __reg("d0") int16 border) {
  MNTZZ9KRegs* registers = b->registers;
  uint16 scale = 0;
  uint16 w;
  uint16 h;
  uint16 colormode;
  
  b->mode_info = m;
  b->border = border;

  if (m->width<320 || m->height<200) return;

  colormode = rtg_to_mnt[b->color_format];
  
  if (m->height>=480 || m->width>=640) {
    scale = 0;
    
    w = m->width;
    h = m->height;
  } else {
    // small doublescan modes are scaled 2x
    // and output as 640x480 wrapped in 800x600 sync
    scale = 3;
    
    w = 2*m->width;
    h = 2*m->height;
    if (h<480) h=480;
  }
  
  init_modeline(registers, w, h, colormode, scale);
}

void set_palette(__reg("a0") struct RTGBoard* b, __reg("d0") uint16 idx, __reg("d1") uint16 len) {
  MNTZZ9KRegs* registers = b->registers;
  int i;
  int j;
  
  len+=idx;
  for (i=idx, j=idx*3; i<len; i++) {
    u32 ctrldata = ((u32)i<<24)|(((u32)b->palette[j])<<16)|(((u32)b->palette[j+1])<<8)|(u32)b->palette[j+2];
    
    *(u32*)((uint32)registers+0x1000) = ctrldata;
    *(u32*)((uint32)registers+0x1004) = 3; // OP_PALETTE
    *(u32*)((uint32)registers+0x1008) = 0; // NOP
    j+=3;
  }
}

uint32 is_bitmap_compatible(__reg("a0") struct RTGBoard* b, __reg("d7") uint16 format) {
  return 0xffffffff;
}

uint32 map_address(__reg("a0") struct RTGBoard* b, __reg("a1") uint32 addr) {
  // align screen buffers
  if (addr>(uint32)b->memory && addr < (((uint32)b->memory) + b->memory_size)) {
    addr=(addr+0x1000)&0xfffff000;
  }
  return addr;
}

uint32 get_pixelclock_index(__reg("a0") struct RTGBoard* b, __reg("a1") struct ModeInfo* mode, __reg("d0") int32 clock, __reg("d7") uint16 format) {
  mode->pixel_clock_hz = CLOCK_HZ;
  mode->clock = 0;
  mode->clock_div = 1;
  return 0;
}

uint32 get_pixelclock_hz(__reg("a0") struct RTGBoard* b, __reg("a1") struct ModeInfo* mode, __reg("d0") int32 clock, __reg("d7") uint16 format) {
  return CLOCK_HZ;
}

uint32 monitor_switch(__reg("a0") struct RTGBoard* b, __reg("d0") uint16 state) {
  MNTZZ9KRegs* registers = b->registers;
  
  if (state==0) {
/*
  // capture 24 bit amiga video to 0xe00000
//    zzwrite16(&registers->pan_ptr_hi, 0xe0);
    
    if (scandoubler_800x600) {
      // slightly adjusted centering
       ZZ_WRITE32(registers->pan_ptr, 0xe00bd0);
    } else {
       ZZ_WRITE32(registers->pan_ptr, 0xe00000);
    }
*/

    int w = 1920;
    int h = 1080;
    int colormode = MNTVA_COLOR_32BIT;
    int scalemode = 1; // vertical line doubling
/*
    if (scandoubler_800x600) {
      w = 800;
      h = 600;
    }
*/
    init_modeline(registers, w, h, colormode, scalemode);

    // firmware will remember the selected mode
    *(u32*)((uint32)registers+0x100C) = 1; // capture mode
    
  } else {
    // rtg mode
    *(u32*)((uint32)registers+0x100C) = 0; // capture mode
    
    init_mode(b, b->mode_info, b->border);
  }

  // FIXME
  for (volatile int i=0; i<100; i++) {
    fix_vsync(registers);
  }
  
  return 1-state;
}

void rect_invert(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h, __reg("d4") uint8 mask, __reg("d7") uint16 format)
{
  if (!b || !r)
    return;
  
  MNTZZ9KRegs* registers = b->registers;
  uint32_t offset = (r->memory - b->memory);

  ZZ_WRITE32(registers->blitter_dst, offset);
  ZZ_WRITE32(registers->blitter_row_pitch, r->pitch >> 2);
  ZZ_WRITE32(registers->blitter_colormode, rtg_to_mnt[r->color_format]);

  ZZ_WRITE32(registers->blitter_x1, x);
  ZZ_WRITE32(registers->blitter_y1, y);
  ZZ_WRITE32(registers->blitter_x2, w);
  ZZ_WRITE32(registers->blitter_y2, h);
  
  ZZ_WRITE32(registers->blitter_op_invertrect, mask);
}

void rect_invert_dma(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h, __reg("d4") uint8 mask, __reg("d7") uint16 format)
{
  if (!b || !r)
    return;

  dmy_cache
  gfxdata->offset[GFXDATA_DST] = (r->memory - b->memory);
  gfxdata->pitch[GFXDATA_DST] = (r->pitch >> 2);

  gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8)rtg_to_mnt[r->color_format];
  gfxdata->mask = mask;

  gfxdata->x[0] = x;
  gfxdata->x[1] = w;
  gfxdata->y[0] = y;
  gfxdata->y[1] = h;

  ZZ_WRITE32(registers->blitter_dma_op, OP_INVERTRECT);
}

void rect_p2d(__reg("a0") struct RTGBoard* b, __reg("a1") struct BitMap* bm, __reg("a2") struct RenderInfo* r, __reg("a3") struct ColorIndexMapping* cim, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 dx, __reg("d3") uint16 dy, __reg("d4") uint16 w, __reg("d5") uint16 h, __reg("d6") uint8 minterm, __reg("d7") uint8 mask)
{
  if (!b || !r)
    return;

  uint32 plane_size = bm->BytesPerRow * bm->Rows;

  if (plane_size * bm->Depth > 0xFFFF && zorro_version != 3) {
    b->fn_p2d_fallback(b, bm, r, cim, x, y, dx, dy, w, h, minterm, mask);
    return;
  }

  uint32_t offset = (r->memory - b->memory);
  uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
  MNTZZ9KRegs* registers = b->registers;
  uint16_t zz_mask = mask;
  uint8_t cur_plane = 0x01;

  ZZ_WRITE32(registers->blitter_dst, offset);
  ZZ_WRITE32(registers->blitter_row_pitch, r->pitch >> 2);
  ZZ_WRITE32(registers->blitter_colormode, rtg_to_mnt[r->color_format] | (minterm << 8));

  uint16 line_size = (w >> 3) + 2;
  uint32 output_plane_size = line_size * h;
  uint16 x_offset = (x >> 3);

  if (zorro_version != 3) {
    zz_template_addr = b->memory_size;
  }
  ZZ_WRITE32(registers->blitter_src, zz_template_addr);
  ZZ_WRITE32(registers->blitter_src_pitch, line_size);

  if (zorro_version != 3) {
    zz_template_addr = b->memory_size;
  }
  ZZ_WRITE32(registers->blitter_src, zz_template_addr);
  ZZ_WRITE32(registers->blitter_src_pitch, line_size);

  memcpy((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), cim->colors, (256 << 2));
  zz_template_addr += (256 << 2);
  ZZ_WRITE32(registers->blitter_rgb, cim->mask);

  for (int16 i = 0; i < bm->Depth; i++) {
    uint16 x_offset = (x >> 3);
    if ((uint32_t)bm->Planes[i] == 0xFFFFFFFF) {
      memset((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), 0xFF, output_plane_size);
    }
    else if (bm->Planes[i] != NULL) {
      uint8* bmp_mem = (uint8*)bm->Planes[i] + (y * bm->BytesPerRow) + x_offset;
      uint8* zz_dest = (uint8*)(((uint32_t)b->memory)+zz_template_addr);
      for (int16 y_line = 0; y_line < h; y_line++) {
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

  ZZ_WRITE32(registers->blitter_x1, x & 0x07);
  ZZ_WRITE32(registers->blitter_x2, dx);
  ZZ_WRITE32(registers->blitter_y2, dy);
  ZZ_WRITE32(registers->blitter_x3, w);
  ZZ_WRITE32(registers->blitter_y3, h);

  ZZ_WRITE32(registers->blitter_user2, zz_mask);

  ZZ_WRITE32(registers->blitter_op_p2d, mask | bm->Depth << 8);
}

void rect_p2d_dma(__reg("a0") struct RTGBoard* b, __reg("a1") struct BitMap* bm, __reg("a2") struct RenderInfo* r, __reg("a3") struct ColorIndexMapping* cim, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 dx, __reg("d3") uint16 dy, __reg("d4") uint16 w, __reg("d5") uint16 h, __reg("d6") uint8 minterm, __reg("d7") uint8 mask)
{
  if (!b || !r)
    return;

  dmy_cache
  uint32 plane_size = bm->BytesPerRow * bm->Rows;

  if (plane_size * bm->Depth > 0xFFFF && zorro_version != 3) {
    b->fn_p2d_fallback(b, bm, r, cim, x, y, dx, dy, w, h, minterm, mask);
    return;
  }

  uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
  if (zorro_version != 3) {
    zz_template_addr = b->memory_size;
  }

  uint16_t zz_mask = mask;
  uint8_t cur_plane = 0x01;

  uint16 line_size = (w >> 3) + 2;
  uint32 output_plane_size = line_size * h;
  uint16 x_offset = (x >> 3);

  gfxdata->offset[GFXDATA_DST] = (r->memory - b->memory);
  gfxdata->offset[GFXDATA_SRC] = zz_template_addr;
  gfxdata->pitch[GFXDATA_DST] = (r->pitch >> 2);
  gfxdata->pitch[GFXDATA_SRC] = line_size;
  gfxdata->rgb[0] = cim->mask;

  gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8)rtg_to_mnt[r->color_format];
  gfxdata->mask = mask;
  gfxdata->minterm = minterm;

  memcpy((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), cim->colors, (256 << 2));
  zz_template_addr += (256 << 2);
  ZZ_WRITE32(registers->blitter_rgb, cim->mask);

  for (int16 i = 0; i < bm->Depth; i++) {
    uint16 x_offset = (x >> 3);
    if ((uint32_t)bm->Planes[i] == 0xFFFFFFFF) {
      memset((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), 0xFF, output_plane_size);
    }
    else if (bm->Planes[i] != NULL) {
      uint8* bmp_mem = (uint8*)bm->Planes[i] + (y * bm->BytesPerRow) + x_offset;
      uint8* zz_dest = (uint8*)(((uint32_t)b->memory)+zz_template_addr);
      for (int16 y_line = 0; y_line < h; y_line++) {
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

  gfxdata->x[0] = (x & 0x07);
  gfxdata->x[1] = dx;
  gfxdata->x[2] = w;
  gfxdata->y[1] = dy;
  gfxdata->y[2] = h;

  gfxdata->user[0] = zz_mask;
  gfxdata->user[1] = bm->Depth;

  ZZ_WRITE32(registers->blitter_dma_op, OP_P2D);
}

void rect_p2c(__reg("a0") struct RTGBoard* b, __reg("a1") struct BitMap* bm, __reg("a2") struct RenderInfo* r, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 dx, __reg("d3") uint16 dy, __reg("d4") uint16 w, __reg("d5") uint16 h, __reg("d6") uint8 minterm, __reg("d7") uint8 mask)
{
  if (!b || !r)
    return;

  uint32 plane_size = bm->BytesPerRow * bm->Rows;
  
  if (plane_size * bm->Depth > 0xFFFF && zorro_version != 3) {
    b->fn_p2c_fallback(b, bm, r, x, y, dx, dy, w, h, minterm, mask);
    return;
  }

  uint32_t offset = (r->memory - b->memory);
  uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
  MNTZZ9KRegs* registers = b->registers;
  uint16_t zz_mask = mask;
  uint8_t cur_plane = 0x01;

  ZZ_WRITE32(registers->blitter_dst, offset);
  ZZ_WRITE32(registers->blitter_row_pitch, r->pitch >> 2);
  ZZ_WRITE32(registers->blitter_colormode, rtg_to_mnt[r->color_format] | (minterm << 8));

  uint16 line_size = (w >> 3) + 2;
  uint32 output_plane_size = line_size * h;
  uint16 x_offset = (x >> 3);

  if (zorro_version != 3) {
    zz_template_addr = b->memory_size;
  }
  ZZ_WRITE32(registers->blitter_src, zz_template_addr);
  ZZ_WRITE32(registers->blitter_src_pitch, line_size);

  for (int16 i = 0; i < bm->Depth; i++) {
    uint16 x_offset = (x >> 3);
    if ((uint32_t)bm->Planes[i] == 0xFFFFFFFF) {
      memset((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), 0xFF, output_plane_size);
    }
    else if (bm->Planes[i] != NULL) {
      uint8* bmp_mem = (uint8*)bm->Planes[i] + (y * bm->BytesPerRow) + x_offset;
      uint8* zz_dest = (uint8*)(((uint32_t)b->memory)+zz_template_addr);
      for (int16 y_line = 0; y_line < h; y_line++) {
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

  ZZ_WRITE32(registers->blitter_x1, x & 0x07);
  ZZ_WRITE32(registers->blitter_x2, dx);
  ZZ_WRITE32(registers->blitter_y2, dy);
  ZZ_WRITE32(registers->blitter_x3, w);
  ZZ_WRITE32(registers->blitter_y3, h);

  ZZ_WRITE32(registers->blitter_user2, zz_mask);

  ZZ_WRITE32(registers->blitter_op_p2c, mask | bm->Depth << 8);
}

void rect_p2c_dma(__reg("a0") struct RTGBoard* b, __reg("a1") struct BitMap* bm, __reg("a2") struct RenderInfo* r, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 dx, __reg("d3") uint16 dy, __reg("d4") uint16 w, __reg("d5") uint16 h, __reg("d6") uint8 minterm, __reg("d7") uint8 mask)
{
  if (!b || !r)
    return;

  dmy_cache
  uint32 plane_size = bm->BytesPerRow * bm->Rows;
  
  if (plane_size * bm->Depth > 0xFFFF && zorro_version != 3) {
    b->fn_p2c_fallback(b, bm, r, x, y, dx, dy, w, h, minterm, mask);
    return;
  }

  uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
  if (zorro_version != 3) {
    zz_template_addr = b->memory_size;
  }

  uint16_t zz_mask = mask;
  uint8_t cur_plane = 0x01;

  uint16 line_size = (w >> 3) + 2;
  uint32 output_plane_size = line_size * h;
  uint16 x_offset = (x >> 3);

  gfxdata->offset[GFXDATA_DST] = (r->memory - b->memory);
  gfxdata->offset[GFXDATA_SRC] = zz_template_addr;
  gfxdata->pitch[GFXDATA_DST] = (r->pitch >> 2);
  gfxdata->pitch[GFXDATA_SRC] = line_size;

  //gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8)rtg_to_mnt[r->color_format];
  gfxdata->mask = mask;
  gfxdata->minterm = minterm;

  for (int16 i = 0; i < bm->Depth; i++) {
    uint16 x_offset = (x >> 3);
    if ((uint32_t)bm->Planes[i] == 0xFFFFFFFF) {
      memset((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), 0xFF, output_plane_size);
    }
    else if (bm->Planes[i] != NULL) {
      uint8* bmp_mem = (uint8*)bm->Planes[i] + (y * bm->BytesPerRow) + x_offset;
      uint8* zz_dest = (uint8*)(((uint32_t)b->memory)+zz_template_addr);
      for (int16 y_line = 0; y_line < h; y_line++) {
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

  gfxdata->x[0] = (x & 0x07);
  gfxdata->x[1] = dx;
  gfxdata->x[2] = w;
  gfxdata->y[1] = dy;
  gfxdata->y[2] = h;

  gfxdata->user[0] = zz_mask;
  gfxdata->user[1] = bm->Depth;

  ZZ_WRITE32(registers->blitter_dma_op, OP_P2C);
}

void draw_line(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("a2") struct Line* l, __reg("d0") uint16 mask, __reg("d7") uint16 format)
{
  if (!l || !r)
    return;

  uint32_t offset;
  MNTZZ9KRegs* registers = b->registers;

  offset = (r->memory - b->memory);

  ZZ_WRITE32(registers->blitter_dst, offset);
  ZZ_WRITE32(registers->blitter_row_pitch, r->pitch >> 2);

  ZZ_WRITE32(registers->blitter_colormode, rtg_to_mnt[r->color_format] | (l->draw_mode << 8));

  ZZ_WRITE32(registers->blitter_rgb, l->fg_pen);

  ZZ_WRITE32(registers->blitter_rgb2, l->bg_pen);

  ZZ_WRITE32(registers->blitter_x1, l->x);
  ZZ_WRITE32(registers->blitter_y1, l->y);
  ZZ_WRITE32(registers->blitter_x2, l->dx);
  ZZ_WRITE32(registers->blitter_y2, l->dy);
  ZZ_WRITE32(registers->blitter_user1, l->len);
  ZZ_WRITE32(registers->blitter_x3, l->pattern);
  ZZ_WRITE32(registers->blitter_y3, l->pattern_offset | (l->padding << 8));

  ZZ_WRITE32(registers->blitter_op_draw_line, mask);
}

void draw_line_dma(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("a2") struct Line* l, __reg("d0") uint16 mask, __reg("d7") uint16 format)
{
  if (!l || !r)
    return;

  dmy_cache
  gfxdata->offset[GFXDATA_DST] = (r->memory - b->memory);
  gfxdata->pitch[GFXDATA_DST] = (r->pitch >> 2);

  gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8)rtg_to_mnt[r->color_format];
  gfxdata->u8_user[GFXDATA_U8_DRAWMODE] = l->draw_mode;
  gfxdata->u8_user[GFXDATA_U8_LINE_PATTERN_OFFSET] = l->pattern_offset;
  gfxdata->u8_user[GFXDATA_U8_LINE_PADDING] = l->padding;

  gfxdata->rgb[0] = l->fg_pen;
  gfxdata->rgb[1] = l->bg_pen;

  gfxdata->x[0] = l->x;
  gfxdata->x[1] = l->dx;
  gfxdata->y[0] = l->y;
  gfxdata->y[1] = l->dy;

  gfxdata->user[0] = l->len;
  gfxdata->user[1] = l->pattern;
  gfxdata->user[2] = ((l->pattern_offset << 8) | l->padding);

  gfxdata->mask = mask;

  ZZ_WRITE32(registers->blitter_dma_op, OP_DRAWLINE);
}

void rect_fill(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h, __reg("d4") uint32 color, __reg("d5") uint8 mask) {
  MNTZZ9KRegs* registers = b->registers;

  if (!r) return;
  if (w<1 || h<1) return;
  
  uint32 offset = (r->memory - (b->memory));
  ZZ_WRITE32(registers->blitter_dst, offset);

  ZZ_WRITE32(registers->blitter_rgb, color);
  ZZ_WRITE32(registers->blitter_row_pitch, r->pitch >> 2);
  ZZ_WRITE32(registers->blitter_colormode, rtg_to_mnt[r->color_format]);
  ZZ_WRITE32(registers->blitter_x1, x);
  ZZ_WRITE32(registers->blitter_y1, y);
  ZZ_WRITE32(registers->blitter_x2, w);
  ZZ_WRITE32(registers->blitter_y2, h);
  ZZ_WRITE32(registers->blitter_op_fillrect, mask);
}

void rect_fill_dma(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h, __reg("d4") uint32 color, __reg("d5") uint8 mask) {
  if (!r) return;
  if (w<1 || h<1) return;

  dmy_cache
  gfxdata->offset[GFXDATA_DST] = (r->memory - b->memory);
  gfxdata->pitch[GFXDATA_DST] = (r->pitch >> 2);

  gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8)rtg_to_mnt[r->color_format];
  gfxdata->mask = mask;

  gfxdata->rgb[0] = color;
  gfxdata->x[0] = x;
  gfxdata->x[1] = w;
  gfxdata->y[0] = y;
  gfxdata->y[1] = h;

  ZZ_WRITE32(registers->blitter_dma_op, OP_FILLRECT);
}

void rect_copy(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 dx, __reg("d3") uint16 dy, __reg("d4") uint16 w, __reg("d5") uint16 h, __reg("d6") uint8 m, __reg("d7") uint16 format) {
  MNTZZ9KRegs* registers = b->registers;
  
  if (w<1 || h<1) return;
  if (!r) return;

  ZZ_WRITE32(registers->blitter_y1, dy);
  ZZ_WRITE32(registers->blitter_y2, h);
  ZZ_WRITE32(registers->blitter_y3, y);
  
  ZZ_WRITE32(registers->blitter_x1, dx);
  ZZ_WRITE32(registers->blitter_x2, w);
  ZZ_WRITE32(registers->blitter_x3, x);

  ZZ_WRITE32(registers->blitter_row_pitch, r->pitch >> 2);
  ZZ_WRITE32(registers->blitter_colormode, rtg_to_mnt[r->color_format] | (m << 8));
  
  uint32 offset = (r->memory - (b->memory));
  ZZ_WRITE32(registers->blitter_src, offset);

  offset = (r->memory - (b->memory));
  ZZ_WRITE32(registers->blitter_dst, offset);

  ZZ_WRITE32(registers->blitter_op_copyrect, 1);
}

void rect_copy_dma(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 dx, __reg("d3") uint16 dy, __reg("d4") uint16 w, __reg("d5") uint16 h, __reg("d6") uint8 m, __reg("d7") uint16 format) {
  if (w<1 || h<1) return;
  if (!r) return;

  dmy_cache
  gfxdata->x[0] = dx;
  gfxdata->x[1] = w;
  gfxdata->x[2] = x;
  gfxdata->y[0] = dy;
  gfxdata->y[1] = h;
  gfxdata->y[2] = y;

  gfxdata->offset[GFXDATA_DST] = (r->memory - b->memory);
  gfxdata->offset[GFXDATA_SRC] = (r->memory - b->memory);
  gfxdata->pitch[GFXDATA_DST] = (r->pitch >> 2);

  gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8)rtg_to_mnt[r->color_format];
  gfxdata->mask = m;

  ZZ_WRITE32(registers->blitter_dma_op, OP_COPYRECT);
}

void rect_copy_nomask(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* sr,__reg("a2") struct RenderInfo* dr,__reg("d0") uint16 x,__reg("d1") uint16 y,__reg("d2") uint16 dx,__reg("d3") uint16 dy,__reg("d4") uint16 w,__reg("d5") uint16 h,__reg("d6") uint8 opcode,__reg("d7") uint16 format)
{
  MNTZZ9KRegs* registers = b->registers;
  
  if (w<1 || h<1) return;
  if (!sr || !dr) return;

  ZZ_WRITE32(registers->blitter_y1, dy);
  ZZ_WRITE32(registers->blitter_y2, h);
  ZZ_WRITE32(registers->blitter_y3, y);
  
  ZZ_WRITE32(registers->blitter_x1, dx);
  ZZ_WRITE32(registers->blitter_x2, w);
  ZZ_WRITE32(registers->blitter_x3, x);

  ZZ_WRITE32(registers->blitter_colormode, rtg_to_mnt[dr->color_format] | (opcode << 8));
  
  ZZ_WRITE32(registers->blitter_src_pitch, sr->pitch >> 2);
  uint32 offset = (sr->memory - (b->memory));
  ZZ_WRITE32(registers->blitter_src, offset);

  ZZ_WRITE32(registers->blitter_row_pitch, dr->pitch >> 2);
  offset = (dr->memory - (b->memory));
  ZZ_WRITE32(registers->blitter_dst, offset);

  ZZ_WRITE32(registers->blitter_op_copyrect, 2);
}

void rect_copy_nomask_dma(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* sr,__reg("a2") struct RenderInfo* dr,__reg("d0") uint16 x,__reg("d1") uint16 y,__reg("d2") uint16 dx,__reg("d3") uint16 dy,__reg("d4") uint16 w,__reg("d5") uint16 h,__reg("d6") uint8 opcode,__reg("d7") uint16 format)
{
  if (w<1 || h<1) return;
  if (!sr || !dr) return;

  dmy_cache

  gfxdata->x[0] = dx;
  gfxdata->x[1] = w;
  gfxdata->x[2] = x;
  gfxdata->y[0] = dy;
  gfxdata->y[1] = h;
  gfxdata->y[2] = y;

  gfxdata->offset[GFXDATA_DST] = (dr->memory - b->memory);
  gfxdata->offset[GFXDATA_SRC] = (sr->memory - b->memory);
  gfxdata->pitch[GFXDATA_DST] = (dr->pitch >> 2);
  gfxdata->pitch[GFXDATA_SRC] = (sr->pitch >> 2);

  gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8)rtg_to_mnt[dr->color_format];
  gfxdata->minterm = opcode;

  ZZ_WRITE32(registers->blitter_dma_op, OP_COPYRECT_NOMASK);
}

void rect_template(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("a2") struct Template* t,
                   __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h,
                   __reg("d4") uint32 mask, __reg("d7") uint32 format) {
  MNTZZ9KRegs* registers = b->registers;

  if (!r) return;
  if (w<1 || h<1) return;
  if (!t) return; // something about special ptrs?
  
  
  uint32 offset = (r->memory - (b->memory));
  ZZ_WRITE32(registers->blitter_dst, offset);
  
  // FIXME magic numbers and no limits
  uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;

  if (zorro_version != 3) {
    zz_template_addr = b->memory_size;
  }

  memcpy((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), t->memory, t->pitch*h);
  
  ZZ_WRITE32(registers->blitter_src, zz_template_addr);

  ZZ_WRITE32(registers->blitter_rgb, t->fg_pen);
  ZZ_WRITE32(registers->blitter_rgb2, t->bg_pen);
  
  ZZ_WRITE32(registers->blitter_src_pitch, t->pitch);
  ZZ_WRITE32(registers->blitter_row_pitch, r->pitch);
  ZZ_WRITE32(registers->blitter_colormode, rtg_to_mnt[r->color_format] | (t->draw_mode << 8));
  ZZ_WRITE32(registers->blitter_x1, x);
  ZZ_WRITE32(registers->blitter_y1, y);
  ZZ_WRITE32(registers->blitter_x2, w);
  ZZ_WRITE32(registers->blitter_y2, h);
  ZZ_WRITE32(registers->blitter_x3, t->xo);
  
  ZZ_WRITE32(registers->blitter_op_filltemplate, mask);
}

void rect_template_dma(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("a2") struct Template* t,
                       __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h,
                       __reg("d4") uint32 mask, __reg("d7") uint32 format) {
  if (!r) return;
  if (w<1 || h<1) return;
  if (!t) return; // something about special ptrs?
  
  uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;

  if (zorro_version != 3) {
    zz_template_addr = b->memory_size;
  }

  dmy_cache
  gfxdata->x[0] = x;
  gfxdata->x[1] = w;
  gfxdata->x[2] = t->xo;
  gfxdata->y[0] = y;
  gfxdata->y[1] = h;

  gfxdata->offset[GFXDATA_DST] = (r->memory - b->memory);
  gfxdata->offset[GFXDATA_SRC] = zz_template_addr;
  gfxdata->pitch[GFXDATA_DST] = r->pitch;
  gfxdata->pitch[GFXDATA_SRC] = t->pitch;

  gfxdata->rgb[0] = t->fg_pen;
  gfxdata->rgb[1] = t->bg_pen;

  gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8)rtg_to_mnt[r->color_format];
  gfxdata->u8_user[GFXDATA_U8_DRAWMODE] = t->draw_mode;
  gfxdata->mask = mask;

  memcpy((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), t->memory, t->pitch*h);
  
  ZZ_WRITE32(registers->blitter_dma_op, OP_RECT_TEMPLATE);
}

void rect_pattern(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("a2") struct Pattern* pat,
                  __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h,
                  __reg("d4") uint8 mask, __reg("d7") uint32 format) {
  MNTZZ9KRegs* registers = b->registers;
  
  if (!r) return;
  if (w<1 || h<1) return;
  if (!pat) return; // something about special ptrs?
  
  uint32 offset = (r->memory - (b->memory));
  ZZ_WRITE32(registers->blitter_dst, offset);
  
  // FIXME magic numbers and no limits
  uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;

  if (zorro_version != 3) {
    zz_template_addr = b->memory_size;
  }

  memcpy((uint8_t*)(((uint32_t)b->memory) + zz_template_addr), pat->memory, 2 * (1 << pat->size));
  
  ZZ_WRITE32(registers->blitter_src, zz_template_addr);

  ZZ_WRITE32(registers->blitter_rgb, pat->fg_pen);
  ZZ_WRITE32(registers->blitter_rgb2, pat->bg_pen);

  ZZ_WRITE32(registers->blitter_user1, mask);
  ZZ_WRITE32(registers->blitter_row_pitch, r->pitch);
  ZZ_WRITE32(registers->blitter_colormode, rtg_to_mnt[r->color_format] | (pat->draw_mode << 8));
  ZZ_WRITE32(registers->blitter_x1, x);
  ZZ_WRITE32(registers->blitter_y1, y);
  ZZ_WRITE32(registers->blitter_x2, w);
  ZZ_WRITE32(registers->blitter_y2, h);
  ZZ_WRITE32(registers->blitter_x3, pat->xo);
  ZZ_WRITE32(registers->blitter_y3, pat->yo);
  
  ZZ_WRITE32(registers->blitter_op_filltemplate, (1 << pat->size) | 0x8000);
}

void rect_pattern_dma(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("a2") struct Pattern* pat,
                      __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h,
                      __reg("d4") uint8 mask, __reg("d7") uint32 format) {
  if (!r) return;
  if (w<1 || h<1) return;
  if (!pat) return; // something about special ptrs?

  uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;

  if (zorro_version != 3) {
    zz_template_addr = b->memory_size;
  }

  dmy_cache
  gfxdata->x[0] = x;
  gfxdata->x[1] = w;
  gfxdata->x[2] = pat->xo;
  gfxdata->y[0] = y;
  gfxdata->y[1] = h;
  gfxdata->y[2] = pat->yo;

  gfxdata->offset[GFXDATA_DST] = (r->memory - b->memory);
  gfxdata->offset[GFXDATA_SRC] = zz_template_addr;
  gfxdata->pitch[GFXDATA_DST] = r->pitch;

  gfxdata->rgb[0] = pat->fg_pen;
  gfxdata->rgb[1] = pat->bg_pen;

  gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8)rtg_to_mnt[r->color_format];
  gfxdata->u8_user[GFXDATA_U8_DRAWMODE] = pat->draw_mode;
  gfxdata->user[0] = (1 << pat->size);
  gfxdata->mask = mask;

  memcpy((uint8_t*)(((uint32_t)b->memory) + zz_template_addr), pat->memory, 2 * (1 << pat->size));
  
  ZZ_WRITE32(registers->blitter_dma_op, OP_RECT_PATTERN);
}

void blitter_wait(__reg("a0") struct RTGBoard* b) {
}

void sprite_xy(__reg("a0") struct RTGBoard* b, __reg("d0") int16 x, __reg("d1") int16 y, __reg("d7") uint16 format) {
  MNTZZ9KRegs* registers = b->registers;

  ZZ_WRITE32(registers->sprite_x, x);
  ZZ_WRITE32(registers->sprite_y, y);
}

void sprite_xy_dma(__reg("a0") struct RTGBoard* b, __reg("d0") int16 x, __reg("d1") int16 y, __reg("d7") uint16 format) {
  dmy_cache
  gfxdata->x[0] = x;
  gfxdata->y[0] = y;

  ZZ_WRITE32(registers->blitter_dma_op, OP_SPRITE_XY);
}

void sprite_setup(__reg("a0") struct RTGBoard* b, __reg("d0") uint8 enable) {
  MNTZZ9KRegs* registers = b->registers;
  ZZ_WRITE32(registers->sprite_bitmap, 1);
}

void sprite_bitmap(__reg("a0") struct RTGBoard* b, __reg("d7") uint16 format)
{
  MNTZZ9KRegs* registers = b->registers;
  uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
  if (zorro_version != 3) {
    zz_template_addr = b->memory_size;
  }

  ZZ_WRITE32(registers->blitter_src, zz_template_addr);

  uint16_t data_size = ((b->cursor_w >> 3) * 2) * (b->cursor_h);
  if (b->cursor_w > 16)
    memcpy((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), b->cursor_sprite_bitmap+4, data_size);
  else
    memcpy((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), b->cursor_sprite_bitmap+2, data_size);

  ZZ_WRITE32(registers->blitter_x1, b->cursor_xo);
  ZZ_WRITE32(registers->blitter_y1, b->cursor_yo);
  ZZ_WRITE32(registers->blitter_x2, b->cursor_w);
  ZZ_WRITE32(registers->blitter_y2, b->cursor_h);

  ZZ_WRITE32(registers->sprite_bitmap, 0);
}

void sprite_bitmap_dma(__reg("a0") struct RTGBoard* b, __reg("d7") uint16 format)
{
  dmy_cache
  uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
  if (zorro_version != 3) {
    zz_template_addr = b->memory_size;
  }

  gfxdata->offset[1] = zz_template_addr;

  uint16_t data_size = ((b->cursor_w >> 3) * 2) * (b->cursor_h);
  if (b->cursor_w > 16)
    memcpy((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), b->cursor_sprite_bitmap+4, data_size);
  else
    memcpy((uint8_t*)(((uint32_t)b->memory)+zz_template_addr), b->cursor_sprite_bitmap+2, data_size);

  gfxdata->x[0] = b->offset_x;
  gfxdata->x[1] = b->cursor_w;
  gfxdata->y[0] = b->offset_y;
  gfxdata->y[1] = b->cursor_h;

  ZZ_WRITE32(registers->blitter_dma_op, OP_SPRITE_BITMAP);
}

void sprite_colors(__reg("a0") struct RTGBoard* b, __reg("d0") uint8 idx, __reg("d1") uint8 red, __reg("d2") uint8 green, __reg("d3") uint8 blue, __reg("d7") uint16 format)
{
  MNTZZ9KRegs* registers = b->registers;

  ZZ_WRITE32(registers->blitter_user1, red);
  ZZ_WRITE32(registers->blitter_user2, blue | (green << 8));
  ZZ_WRITE32(registers->sprite_colors, idx + 1);
}

void sprite_colors_dma(__reg("a0") struct RTGBoard* b, __reg("d0") uint8 idx, __reg("d1") uint8 red, __reg("d2") uint8 green, __reg("d3") uint8 blue, __reg("d7") uint16 format)
{
  dmy_cache
  ((char *)&gfxdata->rgb[0])[0] = blue;
  ((char *)&gfxdata->rgb[0])[1] = green;
  ((char *)&gfxdata->rgb[0])[2] = red;
  ((char *)&gfxdata->rgb[0])[3] = 0x00;
  gfxdata->u8offset = idx + 1;

  ZZ_WRITE32(registers->blitter_dma_op, OP_SPRITE_COLOR);
}

void set_split_pos(__reg("a0") struct RTGBoard* b, __reg("d0") int16 pos)
{
  b->y_split = pos;
  u32 dat=(u32)b->current_bitmap->Planes[0];
  ZZ_WRITE32(registers->blitter_src, dat);
  ZZ_WRITE32(registers->blitter_set_split_pos, pos);
}

void set_split_pos_dma(__reg("a0") struct RTGBoard* b, __reg("d0") int16 pos)
{
  b->y_split = pos;

  gfxdata->y[0] = pos;
  gfxdata->offset[0] = (uint32)b->current_bitmap->Planes[0];
  ZZ_WRITE32(registers->blitter_dma_op, OP_SET_SPLIT_POS);
}
