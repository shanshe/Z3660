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
 *													MNT Research GmbH, Berlin
 *													https://mntre.com
 * Copyright (C) 2021,			Bjorn Astrom <beeanyew@gmail.com>
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
#include <string.h>
#include <stdint.h>

#include "gfx.h"
#include "z3660_regs.h"

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

//#define KPrintF(...)
#define __saveds__

#define DEVICE_VERSION 1
#define DEVICE_REVISION 03
#define DEVICE_PRIORITY 0
#define DEVICE_ID_STRING "$VER Z3660.card " XSTR(DEVICE_VERSION) "." XSTR(DEVICE_REVISION) " " DEVICE_DATE
#define DEVICE_NAME "Z3660.card"
#define DEVICE_DATE "(18.02.2024)"

int __attribute__((no_reorder)) _start()
{
    return -1;
}

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

//char device_name[] = DEVICE_NAME;
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

struct ExecBase *SysBase;
//static LONG scandoubler_800x600 = 0;
static LONG secondary_palette_enabled = 0;

#ifdef DMARTG
static volatile struct GFXData *gfxdata;
#endif

uint16_t rtg_to_mnt[21] = {
	MNTVA_COLOR_8BIT,		// 0x00 -- None
	MNTVA_COLOR_8BIT,		// 0x01 -- 8BPP CLUT
	MNTVA_COLOR_NO_USE,		// 0x02 -- 24BPP RGB
	MNTVA_COLOR_NO_USE,		// 0x03 -- 24BPP BGR
	MNTVA_COLOR_NO_USE,		// 0x04 -- 16BPP R5G6B5PC
	MNTVA_COLOR_15BIT,		// 0x05 -- 15BPP R5G5B5PC
	MNTVA_COLOR_32BIT,		// 0x06 -- 32BPP ARGB
	MNTVA_COLOR_32BIT,		// 0x07 -- 32BPP ABGR
	MNTVA_COLOR_32BIT,		// 0x08 -- 32BPP RGBA
	MNTVA_COLOR_32BIT,		// 0x09 -- 32BPP BGRA
	MNTVA_COLOR_16BIT565,	// 0x0A -- 16BPP R5G6B5
	MNTVA_COLOR_15BIT,		// 0x0B -- 15BPP R5G5B5
	MNTVA_COLOR_NO_USE,		// 0x0C -- 16BPP B5G6R5PC
	MNTVA_COLOR_15BIT,		// 0x0D -- 15BPP B5G5R5PC
	MNTVA_COLOR_NO_USE,		// 0x0E -- YUV 4:2:2
	MNTVA_COLOR_NO_USE,		// 0x0F -- YUV 4:1:1
	MNTVA_COLOR_NO_USE,		// 0x10 -- YUV 4:1:1PC
	MNTVA_COLOR_NO_USE,		// 0x11 -- YUV 4:2:2 (Duplicate for some reason)
	MNTVA_COLOR_NO_USE,		// 0x12 -- YUV 4:2:2PC
	MNTVA_COLOR_NO_USE,		// 0x13 -- YUV 4:2:2 Planar
	MNTVA_COLOR_NO_USE,		// 0x14 -- YUV 4:2:2PC Planar
};

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
// This can't be cached here because the firmware doesn't cache it either!
//	static UWORD old = 0;
//	if(srcpitch != old) {
		zzwrite16(&registers->blitter_src_pitch, srcpitch);
//		old = srcpitch;
//	}
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
	_gfxbase = exb;
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

		negsize	 = exb->libNode.lib_NegSize;
		possize	 = exb->libNode.lib_PosSize;
		fullsize = negsize + possize;
		negptr	-= negsize;

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
		return 0; \
	} \


int __attribute__((used)) FindCard(__REGA0(struct BoardInfo* b)) {
	struct ConfigDev* cd = NULL;
	struct ExpansionBase *ExpansionBase = NULL;
	struct DOSBase *DOSBase = NULL;
	struct IntuitionBase *IntuitionBase = NULL;
	struct ExecBase *SysBase = *(struct ExecBase **)4L;
	LONG zorro_version = 0;
	LONG hwrev = 0;
	LONG fwrev_major = 0;
	LONG fwrev_minor = 0;
	LONG fwrev = 0;

	LOADLIB(ExpansionBase, "expansion.library");
	LOADLIB(DOSBase, "dos.library");
	LOADLIB(IntuitionBase, "intuition.library");

	zorro_version = 0;
	b->CardFlags = 0;
	if ((cd = (struct ConfigDev*)FindConfigDev(cd,0x144B,0x1))) zorro_version = 3;

	// Find Z3 or Z2 model
	if (zorro_version>=2) {

		b->MemoryBase = (uint8*)(cd->cd_BoardAddr)+Z3660_MEMBASE_ADDR;
		KPrintF((CONST_STRPTR)"BoardAddr 0x%lx\n",cd->cd_BoardAddr);
		if (zorro_version == 2) {
			b->MemorySize = cd->cd_BoardSize-0x30000;
		} else {
			// 13.8 MB for Z3 (safety, will be expanded later)
			// one full HD screen @8bit ~ 2MB
			b->MemorySize = 0x3000000 - Z3660_MEMBASE_ADDR;
			b->CardFlags |= CARDFLAG_ZORRO_3;
#ifdef DMARTG
			gfxdata = (struct GFXData*)(((uint32_t)b->MemoryBase) + (uint32_t)Z3_GFXDATA_ADDR);
			KPrintF((CONST_STRPTR)"GFXData   0x%lx\n",gfxdata);
			memset((void *)gfxdata, 0x00, sizeof(struct GFXData));
#endif
		}
		b->RegisterBase = (uint8*)(cd->cd_BoardAddr);
#ifdef DMARTG
		uint32_t* registers = (uint32_t *)b->RegisterBase;
#endif

//    test write
//    ZZ_REGS_WRITE(REG_ZZ_DEBUG,(u32)cd->cd_BoardAddr);
//    ZZ_REGS_WRITE(REG_ZZ_DEBUG,(u32)cd->cd_BoardAddr);

		(void)hwrev;
#ifdef DMARTG
		fwrev = registers[REG_ZZ_FW_VERSION>>2];
#else
		fwrev = ((uint16_t*)b->RegisterBase)[0xC0/2];
#endif
		fwrev_major = fwrev >> 8;
		fwrev_minor = fwrev & 0xff;

		KPrintF((CONST_STRPTR)"%s\n",device_id_string);
		KPrintF((CONST_STRPTR)"Z3660.card: Z3660 found. Zorro version %ld.\n", zorro_version);
		KPrintF((CONST_STRPTR)"Z3660.card: HW Revision: %ld.\n", hwrev);
		KPrintF((CONST_STRPTR)"Z3660.card: FW Revision Major: %ld.\n", fwrev_major);
		KPrintF((CONST_STRPTR)"Z3660.card: FW Revision Minor: %ld.\n", fwrev_minor);

		if (fwrev_major <= 1 && fwrev_minor < 3) {
			char *alert = "\x00\x14\x14 vX.XX: Z3660.card v1.03 needs at least firmware (BOOT.bin) v1.03.\x00\x00";
			alert[5]='0'+fwrev_major;
			alert[7]='0'+(fwrev_minor/10);
			alert[8]='0'+(fwrev_minor%10);
			DisplayAlert(RECOVERY_ALERT, (unsigned char*)alert, 52);
			return 0;
		}
/* Z3660 -> no scandoubler :(
		MNTZZ9KRegs* registers = (MNTZZ9KRegs *)b->RegisterBase;
*/		BPTR f;
/*		if ((f = Open((APTR)"ENV:ZZ9000-VCAP-800x600", MODE_OLDFILE))) {
			Close(f);
			KPrintF("ZZ9000.card: 800x600 60hz scandoubler mode.\n");
			scandoubler_800x600 = 1;
			registers->videocap_vmode = ZZVMODE_800x600; // 60hz
		} else {
			KPrintF("ZZ9000.card: 720x576 50hz scandoubler mode.\n");
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

		return 1;
	} else {
		KPrintF((CONST_STRPTR)"Z3660.card: Z3660 not found!\n");
		return 0;
	}
}

int __attribute__((used)) InitCard(__REGA0(struct BoardInfo* b)) {
	int i;

	KPrintF((CONST_STRPTR)"InitCard()\n");
	b->CardBase = (struct CardBase *)_gfxbase;
	b->ExecBase = SysBase;
	b->BoardName = "Z3660";
//	b->BoardType = BT_MNT_ZZ9000;
//	b->PaletteChipType = PCT_MNT_ZZ9000;
//	b->GraphicsControllerType = GCT_MNT_ZZ9000;
	b->BoardType = BT_uaegfx;
	b->PaletteChipType = PCT_S3ViRGE;
	b->GraphicsControllerType = GCT_S3ViRGE;


	b->Flags |= BIF_GRANTDIRECTACCESS |
				BIF_HARDWARESPRITE |
//				BIF_FLICKERFIXER |
				BIF_VGASCREENSPLIT |
				BIF_PALETTESWITCH |
				BIF_BLITTER |
				0;

	b->RGBFormats = RTG_COLOR_FORMAT_CLUT |   //  8bit
					RTG_COLOR_FORMAT_RGB565 | // 16bit
					RTG_COLOR_FORMAT_BGRA |   // 32bit
//					RTG_COLOR_FORMAT_BGR888 | // 24bit
					RTG_COLOR_FORMAT_RGB555 | // 15bit
					0;
	b->SoftSpriteFlags = 0;
	b->BitsPerCannon = 8;

	for(i = 0; i < MAXMODES; i++) {
		b->MaxHorValue[i] = 8192;
		b->MaxVerValue[i] = 8192;
		b->MaxHorResolution[i] = 8192;
		b->MaxVerResolution[i] = 8192;
		b->PixelClockCount[i] = 1;
	}

	b->MemoryClock = CLOCK_HZ;

	//b->AllocCardMem = (void *)NULL;
	//b->FreeCardMem = (void *)NULL;
//	b->SetSwitch = (void *)SetSwitch;
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
	//b->SetInterrupt = (void *)NULL;

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
	//b->EnableSoftSprite = (void *)NULL;

	//b->AllocCardMemAbs = (void *)NULL;
	b->SetSplitPosition = (void *)SetSplitPosition;
	//b->ReInitMemory = (void *)NULL;
	//b->WriteYUVRect = (void *)NULL;
	b->GetVSyncState = (void *)GetVSyncState;
	//b->GetVBeamPos = (void *)NULL;
	//b->SetDPMSLevel = (void *)NULL;
	//b->ResetChip = (void *)NULL;
	//b->GetFeatureAttrs = (void *)NULL;
	//b->AllocBitMap = (void *)NULL;
	//b->FreeBitMap = (void *)NULL;
	//b->GetBitMapAttr = (void *)NULL;

	b->SetSprite = (void *)SetSprite;
	b->SetSpritePosition = (void *)SetSpritePosition;
	b->SetSpriteImage = (void *)SetSpriteImage;
	b->SetSpriteColor = (void *)SetSpriteColor;

	//b->CreateFeature = (void *)NULL;
	//b->SetFeatureAttrs = (void *)NULL;
	//b->DeleteFeature = (void *)NULL;

	return 1;
}

// None of these five really have to do anything.
void SetDAC (__REGA0(struct BoardInfo *b), __REGD7(RGBFTYPE format)) { }
void WaitBlitter (__REGA0(struct BoardInfo *b)) { }
void SetClock (__REGA0(struct BoardInfo *b)) { }
void SetMemoryMode (__REGA0(struct BoardInfo *b), __REGD7(RGBFTYPE format)) { }
void SetWriteMask (__REGA0(struct BoardInfo *b), __REGD0(UBYTE mask)) { }
void SetClearMask (__REGA0(struct BoardInfo *b), __REGD0(UBYTE mask)) { }
void SetReadPlane (__REGA0(struct BoardInfo *b), __REGD0(UBYTE plane)) { }

// Optional dummy read for tricking the 68k cache on processors with occasional garbage output on screen
#ifdef DUMMY_CACHE_READ
	#define dmy_cache memcpy(dummies, (uint32_t *)(uint32_t)0x7F00000, 4);
#else
	#define dmy_cache
#endif

void init_modeline(uint32_t* registers, uint16 w, uint16 h, uint8 colormode, uint8 scalemode, struct ModeInfo *mode_info) {
	uint16_t mode = 0;

	if (w == 1280 && h == 720) {
		mode = 0;
	} else if (w == 800 && h == 600) {
		mode = 1;
	} else if (w == 640 && h == 480) {
		mode = 2;
	} else if (w == 640 && h == 400) {
		mode = 16;
	} else if (w == 1024 && h == 768) {
		mode = 3;
	} else if (w == 1280 && h == 1024) {
		mode = 4;
	} else if (w == 1920 && h == 1080) {
		mode = 5;
	} else if (w == 720 && h == 576) {
		mode = 6;
	} else if (w == 720 && h == 480) {
		mode = 8;
	} else if (w == 640 && h == 512) {
		mode = 9;
	} else if (w == 1600 && h == 1200) {
		mode = 10;
	} else if (w == 2560 && h == 1440) {
		mode = 11;
	}

/* TODO: custom video mode
    else {
		mode = 12; // custom mode
	}
    if(mode==12)
    {
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE,VMODE_PARAM_HRES);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE_DATA,mode_info->Width);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE,VMODE_PARAM_VRES);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE_DATA,mode_info->Height);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE,VMODE_PARAM_HSTART);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE_DATA,mode_info->HorSyncStart);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE,VMODE_PARAM_HEND);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE_DATA,mode_info->HorSyncStart+mode_info->HorSyncSize);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE,VMODE_PARAM_HMAX);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE_DATA,mode_info->HorTotal);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE,VMODE_PARAM_VSTART);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE_DATA,mode_info->VerSyncStart);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE,VMODE_PARAM_VEND);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE_DATA,mode_info->VerSyncStart+mode_info->VerSyncSize);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE,VMODE_PARAM_VMAX);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE_DATA,mode_info->VerTotal);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE,VMODE_PARAM_POLARITY);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE_DATA,0);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE,VMODE_PARAM_PHZ);
        ZZ_REGS_WRITE(REG_ZZ_VIDMODE_DATA,mode_info->PixelClock);
    }
*/
    ZZ_REGS_WRITE(REG_ZZ_ORIG_RES, (mode_info->Width<<16)|mode_info->Height);
    ZZ_REGS_WRITE(REG_ZZ_MODE, ((u32)mode)|(((u32)colormode)<<8)|(((u32)scalemode)<<12));
}

void SetGC(__REGA0(struct BoardInfo *b), __REGA1(struct ModeInfo *mode_info), __REGD0(BOOL border)) {
	uint32_t* registers = (uint32_t*)b->RegisterBase;
	uint16_t scale = 0;
	uint16_t w;
	uint16_t h;
	uint16_t colormode;

	b->ModeInfo = mode_info;
	b->Border = border;

	if (mode_info->Width < 320 || mode_info->Height < 200)
		return;

	colormode = rtg_to_mnt[b->RGBFormat];

	if (mode_info->Height >= 400 || mode_info->Width >= 640) {
		scale = 0;

		w = mode_info->Width;
		h = mode_info->Height;
	} else {
		// small doublescan modes are scaled 2x
		// and output as 640x480 wrapped in 800x600 sync
		scale = 3;

		w = 2 * mode_info->Width;
		h = 2 * mode_info->Height;
	}

	init_modeline(registers, w, h, colormode, scale, mode_info);
}

//z3660 -> no scandoubler :(
#if 0 
int setswitch = -1;
UWORD SetSwitch(__REGA0(struct BoardInfo *b), __REGD0(UWORD enabled)) {
    uint32_t* registers = (uint32_t*)b->RegisterBase;

	if (enabled == 0) {
		// capture 24 bit amiga video to 0xe00000

		if (scandoubler_800x600) {
			// slightly adjusted centering
			ZZ_REGS_WRITE(REG_ZZ_PAN, 0x00dff2f8);
		} else {
			ZZ_REGS_WRITE(REG_ZZ_PAN, 0x00e00000);
		}
*/

		// firmware will detect that we are capturing and viewing the capture area
		// and switch to the appropriate video mode (VCAP_MODE)
		ZZ_REGS_WRITE(REG_ZZ_OP_CAPTUREMODE, 1); // capture mode
	} else {
		// rtg mode
		ZZ_REGS_WRITE(REG_ZZ_OP_CAPTUREMODE, 0); // capture mode

		SetGC(b, b->ModeInfo, b->Border);
	}

	return 1 - enabled;
}
#endif

void SetPanning(__REGA0(struct BoardInfo *b), __REGA1(UBYTE *addr), __REGD0(UWORD width), __REGD1(WORD x_offset), __REGD2(WORD y_offset), __REGD7(RGBFTYPE format)) {
	b->XOffset = x_offset;
	b->YOffset = y_offset;
    uint32_t* registers = (uint32_t *)b->RegisterBase;

	if (b->CardFlags & CARDFLAG_ZORRO_3) {
		dmy_cache
		gfxdata->offset[0] = ((uint32_t)addr - (uint32_t)b->MemoryBase);

		gfxdata->x[0] = x_offset;
		gfxdata->y[0] = y_offset;
		gfxdata->x[1] = width;
		gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8)rtg_to_mnt[format & 0xFF];
		ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_PAN);
	} else {
		uint32_t offset = ((uint32_t)addr - (uint32_t)b->MemoryBase);

		ZZ_REGS_WRITE(REG_ZZ_X1, x_offset);
		ZZ_REGS_WRITE(REG_ZZ_Y1, y_offset);
		ZZ_REGS_WRITE(REG_ZZ_X2, width);
		ZZ_REGS_WRITE(REG_ZZ_COLORMODE, rtg_to_mnt[format & 0xFF]);
		ZZ_REGS_WRITE(REG_ZZ_PAN, offset);
	}
}

void SetColorArray(__REGA0(struct BoardInfo *b), __REGD0(UWORD start), __REGD1(UWORD num)) {
	if (!b->CLUT)
		return;

	uint32_t* registers = (uint32_t*)b->RegisterBase;
	int j = start + num;
	int op = 3; // OP_PALETTE

	if (start >= 256) {
		// Select secondary palette if start index is above 255
		if (!secondary_palette_enabled) {
			ZZ_REGS_WRITE(REG_ZZ_USER1, CARD_FEATURE_SECONDARY_PALETTE);
			ZZ_REGS_WRITE(REG_ZZ_SET_FEATURE, 1);
			secondary_palette_enabled = 1;
		}
		op = 19; // OP_PALETTE_HI
	}

	for (int i = start; i < j; i++) {
		uint32_t xrgb = ((uint32_t)(i & 0xFF) << 24) | ((uint32_t)b->CLUT[(i & 0xFF)].Red << 16) | ((uint32_t)b->CLUT[(i & 0xFF)].Green << 8) | ((uint32_t)b->CLUT[(i & 0xFF)].Blue);

		ZZ_REGS_WRITE(REG_ZZ_OP_DATA,xrgb);
		ZZ_REGS_WRITE(REG_ZZ_OP,op);
//		ZZ_REGS_WRITE(REG_ZZ_OP_NOP,0); // NOP
	}
}

uint16_t calc_pitch_bytes(uint16_t w, uint16_t colormode) {
	uint16_t pitch = w;

	if (colormode == MNTVA_COLOR_15BIT) {
		pitch = w<<1;
	} else {
		pitch = w<<colormode;
	}
	return pitch;
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

UWORD CalculateBytesPerRow(__REGA0(struct BoardInfo *b), __REGD0(UWORD width), __REGD7(RGBFTYPE format)) {
	if (!b)
		return 0;

	return calc_pitch_bytes(width, rtg_to_mnt[format]);
}

APTR CalculateMemory(__REGA0(struct BoardInfo *b), __REGA1(unsigned long addr), __REGD7(RGBFTYPE format)) {
	return (APTR)addr;
}

ULONG GetCompatibleFormats(__REGA0(struct BoardInfo *b), __REGD7(RGBFTYPE format)) {
	return 0xFFFFFFFF;
}

UWORD SetDisplay(__REGA0(struct BoardInfo *b), __REGD0(UWORD enabled)) {
	return 1;
}

LONG ResolvePixelClock(__REGA0(struct BoardInfo *b), __REGA1(struct ModeInfo *mode_info), __REGD0(ULONG pixel_clock), __REGD7(RGBFTYPE format)) {
	mode_info->PixelClock = CLOCK_HZ;
	mode_info->pll1.Clock = 0;
	mode_info->pll2.ClockDivide = 1;

	return 0;
}

ULONG GetPixelClock(__REGA0(struct BoardInfo *b), __REGA1(struct ModeInfo *mode_info), __REGD0(ULONG index), __REGD7(RGBFTYPE format)) {
	return CLOCK_HZ;
}

void WaitVerticalSync(__REGA0(struct BoardInfo *b), __REGD0(BOOL toggle)) {
	volatile uint32_t* registers =(volatile uint32_t*)b->RegisterBase;
	uint32 vblank_state = !!ZZ_REGS_READ(REG_ZZ_VBLANK_STATUS);

	while(vblank_state != 0) {
		vblank_state = !!ZZ_REGS_READ(REG_ZZ_VBLANK_STATUS);
	}

	while(vblank_state == 0) {
		vblank_state = !!ZZ_REGS_READ(REG_ZZ_VBLANK_STATUS);
	}
}

BOOL GetVSyncState(__REGA0(struct BoardInfo *b), __REGD0(BOOL toggle)) {
	volatile uint32_t* registers =(volatile uint32_t*)b->RegisterBase;

	return !!ZZ_REGS_READ(REG_ZZ_VBLANK_STATUS);
}

void FillRect(__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *r), __REGD0(WORD x), __REGD1(WORD y), __REGD2(WORD w), __REGD3(WORD h), __REGD4(ULONG color), __REGD5(UBYTE mask), __REGD7(RGBFTYPE format)) {
	if (!r) return;
	if (w<1 || h<1) return;

	uint32_t* registers =(uint32_t*)b->RegisterBase;
	if (b->CardFlags & CARDFLAG_ZORRO_3) {
		dmy_cache
		gfxdata->offset[GFXDATA_DST] = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
		gfxdata->pitch[GFXDATA_DST] = (r->BytesPerRow >> 2);

		gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)rtg_to_mnt[r->RGBFormat];
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
		ZZ_REGS_WRITE(REG_ZZ_COLORMODE, rtg_to_mnt[r->RGBFormat]);
		ZZ_REGS_WRITE(REG_ZZ_X1, x);
		ZZ_REGS_WRITE(REG_ZZ_Y1, y);
		ZZ_REGS_WRITE(REG_ZZ_X2, w);
		ZZ_REGS_WRITE(REG_ZZ_Y2, h);
		ZZ_REGS_WRITE(REG_ZZ_FILLRECT, mask);
	}
}

void InvertRect(__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *r), __REGD0(WORD x), __REGD1(WORD y), __REGD2(WORD w), __REGD3(WORD h), __REGD4(UBYTE mask), __REGD7(RGBFTYPE format)) {
	if (!b || !r)
		return;

	if (b->CardFlags & CARDFLAG_ZORRO_3) {
		dmy_cache
    	uint32_t* registers =(uint32_t*)b->RegisterBase;
		gfxdata->offset[GFXDATA_DST] = (uint32_t)r->Memory - (uint32_t)b->MemoryBase;
		gfxdata->pitch[GFXDATA_DST] = (r->BytesPerRow >> 2);

		gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)rtg_to_mnt[r->RGBFormat];
		gfxdata->mask = mask;

		gfxdata->x[0] = x;
		gfxdata->x[1] = w;
		gfxdata->y[0] = y;
		gfxdata->y[1] = h;

    	ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_INVERTRECT);
	} else {
	    uint32_t* registers =(uint32_t*)b->RegisterBase;
		uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);

		ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);
		ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow >> 2);
		ZZ_REGS_WRITE(REG_ZZ_COLORMODE, rtg_to_mnt[r->RGBFormat]);

		ZZ_REGS_WRITE(REG_ZZ_X1, x);
		ZZ_REGS_WRITE(REG_ZZ_Y1, y);
		ZZ_REGS_WRITE(REG_ZZ_X2, w);
		ZZ_REGS_WRITE(REG_ZZ_Y2, h);

		ZZ_REGS_WRITE(REG_ZZ_INVERTRECT, mask);
	}
}

// This function shall copy a rectangular image region in the video RAM.
void BlitRect(__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *r), __REGD0(WORD x), __REGD1(WORD y), __REGD2(WORD dx), __REGD3(WORD dy), __REGD4(WORD w), __REGD5(WORD h), __REGD6(UBYTE mask), __REGD7(RGBFTYPE format)) {
	if (w<1 || h<1) return;
	if (!r) return;

	if (b->CardFlags & CARDFLAG_ZORRO_3) {
		uint32_t* registers =(uint32_t*)b->RegisterBase;
		dmy_cache

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
		gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)rtg_to_mnt[r->RGBFormat];

		// Mask is a bitmask that defines which (logical) planes are affected by the copy for planar or chunky bitmaps. It can be ignored for direct color modes.
		gfxdata->mask = mask;

		// Source and destination rectangle may be overlapping, a proper copy operation shall be performed in either case.
		ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_COPYRECT);
	} else {
		uint32_t* registers =(uint32_t*)b->RegisterBase;

		ZZ_REGS_WRITE(REG_ZZ_Y1, dy);
		ZZ_REGS_WRITE(REG_ZZ_Y2, h);
		ZZ_REGS_WRITE(REG_ZZ_Y3, y);

		ZZ_REGS_WRITE(REG_ZZ_X1, dx);
		ZZ_REGS_WRITE(REG_ZZ_X2, w);
		ZZ_REGS_WRITE(REG_ZZ_X3, x);

		ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow >> 2);
		ZZ_REGS_WRITE(REG_ZZ_COLORMODE, rtg_to_mnt[r->RGBFormat] | (mask << 8));

		uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
		ZZ_REGS_WRITE(REG_ZZ_BLIT_SRC, offset);
		ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);

		ZZ_REGS_WRITE(REG_ZZ_COPYRECT, 1);
	}
}

// This function shall copy one rectangle in video RAM to another rectangle in video RAM using a mode given in d6. A mask is not applied.
void BlitRectNoMaskComplete(__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *rs), __REGA2(struct RenderInfo *rt), __REGD0(WORD x), __REGD1(WORD y), __REGD2(WORD dx), __REGD3(WORD dy), __REGD4(WORD w), __REGD5(WORD h), __REGD6(UBYTE minterm), __REGD7(RGBFTYPE format)) {
	if (w<1 || h<1) return;
	if (!rs || !rt) return;

	// b->BlitRectNoMaskCompleteDefault(b, rs, rt, x, y, dx, dy, w, h, minterm, format);
	// return;

	if (b->CardFlags & CARDFLAG_ZORRO_3) {
		uint32_t* registers =(uint32_t*)b->RegisterBase;
		dmy_cache

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
		gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)rtg_to_mnt[rt->RGBFormat];

		ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_COPYRECT_NOMASK);
	} else {
		uint32_t* registers =(uint32_t*)b->RegisterBase;

		ZZ_REGS_WRITE(REG_ZZ_Y1, dy);
		ZZ_REGS_WRITE(REG_ZZ_Y2, h);
		ZZ_REGS_WRITE(REG_ZZ_Y3, y);

		ZZ_REGS_WRITE(REG_ZZ_X1, dx);
		ZZ_REGS_WRITE(REG_ZZ_X2, w);
		ZZ_REGS_WRITE(REG_ZZ_X3, x);

		ZZ_REGS_WRITE(REG_ZZ_COLORMODE, rtg_to_mnt[rt->RGBFormat] | (minterm << 8));

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
	if (!r) return;
	if (w<1 || h<1) return;
	if (!t) return;

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
		dmy_cache
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

		gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)rtg_to_mnt[r->RGBFormat];
		gfxdata->u8_user[GFXDATA_U8_DRAWMODE] = t->DrawMode;
		gfxdata->mask = mask;

		ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_RECT_TEMPLATE);
	} else {
		ZZ_REGS_WRITE(REG_ZZ_BLIT_SRC, zz_template_addr);

		ZZ_REGS_WRITE(REG_ZZ_RGB, t->FgPen);
		ZZ_REGS_WRITE(REG_ZZ_RGB2, t->BgPen);

		ZZ_REGS_WRITE(REG_ZZ_SRC_PITCH, t->BytesPerRow);
		ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow);
		ZZ_REGS_WRITE(REG_ZZ_COLORMODE, rtg_to_mnt[r->RGBFormat] | (t->DrawMode << 8));
		ZZ_REGS_WRITE(REG_ZZ_X1, x);
		ZZ_REGS_WRITE(REG_ZZ_Y1, y);
		ZZ_REGS_WRITE(REG_ZZ_X2, w);
		ZZ_REGS_WRITE(REG_ZZ_Y2, h);
		ZZ_REGS_WRITE(REG_ZZ_X3, t->XOffset);

		ZZ_REGS_WRITE(REG_ZZ_FILLTEMPLATE, mask);
	}
}

void BlitPattern(__REGA0(struct BoardInfo *b), __REGA1(struct RenderInfo *r), __REGA2(struct Pattern *pat), __REGD0(WORD x), __REGD1(WORD y), __REGD2(WORD w), __REGD3(WORD h), __REGD4(UBYTE mask), __REGD7(RGBFTYPE format)) {
	if (!r) return;
	if (w<1 || h<1) return;
	if (!pat) return;
	uint32_t* registers =(uint32_t*)b->RegisterBase;

	if (!(b->CardFlags & CARDFLAG_ZORRO_3)) {
		uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
		ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);
	}

	uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
	if (!(b->CardFlags & CARDFLAG_ZORRO_3)) {
		zz_template_addr = b->MemorySize;
	}

	memcpy((uint8_t*)(((uint32_t)b->MemoryBase) + zz_template_addr), pat->Memory, 2 * (1 << pat->Size));

	if (b->CardFlags & CARDFLAG_ZORRO_3) {
		dmy_cache
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

		gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)rtg_to_mnt[r->RGBFormat];
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
		ZZ_REGS_WRITE(REG_ZZ_COLORMODE, rtg_to_mnt[r->RGBFormat] | (pat->DrawMode << 8));
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
	if (!l || !r)
		return;

	if (b->CardFlags & CARDFLAG_ZORRO_3) {
		uint32_t* registers =(uint32_t*)b->RegisterBase;
		dmy_cache
		gfxdata->offset[GFXDATA_DST] = (uint32_t)r->Memory - (uint32_t)b->MemoryBase;
		gfxdata->pitch[GFXDATA_DST] = (r->BytesPerRow >> 2);

		gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)rtg_to_mnt[r->RGBFormat];
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
		uint32_t* registers =(uint32_t*)b->RegisterBase;
		uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);

		ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);
		ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow >> 2);

		ZZ_REGS_WRITE(REG_ZZ_COLORMODE, rtg_to_mnt[r->RGBFormat] | (l->DrawMode << 8));

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

// This function shall blit a planar bitmap anywhere in the 68K address space into a chunky bitmap in video RAM.
// The source bitmap that contains the data to be blitted is in the bm argument.
// If one of its plane pointers is 0x0, the source data of that bitplane shall be considered to consist of all-zeros.
// If one of its plane pointers is 0xffffffff, the data in this bitplane shall be considered to be all ones.
void BlitPlanar2Chunky(__REGA0(struct BoardInfo *b), __REGA1(struct BitMap *bm), __REGA2(struct RenderInfo *r), __REGD0(SHORT x), __REGD1(SHORT y), __REGD2(SHORT dx), __REGD3(SHORT dy), __REGD4(SHORT w), __REGD5(SHORT h), __REGD6(UBYTE minterm), __REGD7(UBYTE mask)) {
	if (!b || !r)
		return;

	// b->BlitPlanar2ChunkyDefault(b, bm, r, x, y, dx, dy, w, h, minterm, mask);
	// return;

	uint32_t* registers = (uint32_t*)b->RegisterBase;
	uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
	uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
	uint16_t zz_mask = mask;
	uint8_t cur_plane = 0x01;

	uint32_t plane_size = bm->BytesPerRow * bm->Rows;

	if (plane_size * bm->Depth > 0xFFFF && (!(b->CardFlags & CARDFLAG_ZORRO_3))) {
		b->BlitPlanar2ChunkyDefault(b, bm, r, x, y, dx, dy, w, h, minterm, mask);
		return;
	}

	uint16_t line_size = (w >> 3) + 2;
	uint32_t output_plane_size = line_size * h;

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
		ZZ_REGS_WRITE(REG_ZZ_COLORMODE, rtg_to_mnt[r->RGBFormat] | (minterm << 8));
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
	if (!b || !r)
		return;

	// b->BlitPlanar2DirectDefault(b, bm, r, clut, x, y, dx, dy, w, h, minterm, mask);
	// return;

	uint32_t* registers = (uint32_t*)b->RegisterBase;
	uint32_t offset = ((uint32_t)r->Memory - (uint32_t)b->MemoryBase);
	uint32_t zz_template_addr = Z3_TEMPLATE_ADDR;
	uint16_t zz_mask = mask;
	uint8_t cur_plane = 0x01;

	uint32_t plane_size = bm->BytesPerRow * bm->Rows;

	if (plane_size * bm->Depth > 0xFFFF && (!(b->CardFlags & CARDFLAG_ZORRO_3))) {
		b->BlitPlanar2DirectDefault(b, bm, r, clut, x, y, dx, dy, w, h, minterm, mask);
		return;
	}

	uint16_t line_size = (w >> 3) + 2;
	uint32_t output_plane_size = line_size * h;

	if (!(b->CardFlags & CARDFLAG_ZORRO_3)) {
		zz_template_addr = b->MemorySize;
	}

	if (b->CardFlags & CARDFLAG_ZORRO_3) {
		gfxdata->offset[GFXDATA_DST] = (uint32_t)r->Memory - (uint32_t)b->MemoryBase;
		gfxdata->offset[GFXDATA_SRC] = zz_template_addr;
		gfxdata->pitch[GFXDATA_DST] = (r->BytesPerRow >> 2);
		gfxdata->pitch[GFXDATA_SRC] = line_size;
		gfxdata->rgb[0] = clut->ColorMask;

		gfxdata->u8_user[GFXDATA_U8_COLORMODE] = (uint8_t)rtg_to_mnt[r->RGBFormat];
		gfxdata->mask = mask;
		gfxdata->minterm = minterm;
	} else {
		ZZ_REGS_WRITE(REG_ZZ_BLIT_DST, offset);
		ZZ_REGS_WRITE(REG_ZZ_ROW_PITCH, r->BytesPerRow >> 2);
		ZZ_REGS_WRITE(REG_ZZ_COLORMODE, rtg_to_mnt[r->RGBFormat] | (minterm << 8));
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

//		unsigned long mi=minterm;
//		unsigned long ma=mask;
//		unsigned long zm=zz_mask;
//		unsigned long cm=clut->ColorMask;
//		unsigned long cf=r->RGBFormat;
//		KPrintF("minterm = %ld, mask=%ld, zzmask=0x%08lX, colormask=0x%08lX, colorformat=%ld\n", mi, ma, zm, cm, cf);

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

void SetSprite(__REGA0(struct BoardInfo *b), __REGD0(BOOL what), __REGD7(RGBFTYPE format)) {
	uint32_t* registers = (uint32_t*)b->RegisterBase;
	ZZ_REGS_WRITE(REG_ZZ_SPRITE_BITMAP, 1);
}

void SetSpritePosition(__REGA0(struct BoardInfo *b), __REGD0(WORD x), __REGD1(WORD y), __REGD7(RGBFTYPE format)) {
	// see http://wiki.icomp.de/wiki/P96_Driver_Development#SetSpritePosition
	b->MouseX = x;
	b->MouseY = y;

	uint32_t* registers = (uint32_t*)b->RegisterBase;
	if(b->CardFlags & CARDFLAG_ZORRO_3) {
		dmy_cache
		gfxdata->x[0] = x;
		gfxdata->y[0] = y + b->YSplit;

		ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_SPRITE_XY);
	} else {
		ZZ_REGS_WRITE(REG_ZZ_X1, x);
		ZZ_REGS_WRITE(REG_ZZ_Y1, y + b->YSplit);
		ZZ_REGS_WRITE(REG_ZZ_SPRITE_Y, 1);
	}
}

void SetSpriteImage(__REGA0(struct BoardInfo *b), __REGD7(RGBFTYPE format)) {
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
		dmy_cache
		gfxdata->offset[1] = zz_template_addr;
		gfxdata->x[2] = doubledsprite;
		gfxdata->x[0] = b->XOffset;
		gfxdata->x[1] = b->MouseWidth;
		gfxdata->y[0] = b->YOffset;
		gfxdata->y[1] = b->MouseHeight;

		ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_SPRITE_BITMAP);
	} else {
		ZZ_REGS_WRITE(REG_ZZ_BLIT_SRC, zz_template_addr);

		ZZ_REGS_WRITE(REG_ZZ_X3, doubledsprite);
		ZZ_REGS_WRITE(REG_ZZ_X1, b->XOffset);
		ZZ_REGS_WRITE(REG_ZZ_X2, b->MouseWidth);
		ZZ_REGS_WRITE(REG_ZZ_Y1, b->YOffset);
		ZZ_REGS_WRITE(REG_ZZ_Y2, b->MouseHeight);

		ZZ_REGS_WRITE(REG_ZZ_SPRITE_BITMAP, 0);
	}
}

void SetSpriteColor (__REGA0(struct BoardInfo *b), __REGD0(UBYTE idx), __REGD1(UBYTE R), __REGD2(UBYTE G), __REGD3(UBYTE B), __REGD7(RGBFTYPE format)) {
	uint32_t* registers =(uint32_t*)b->RegisterBase;
	if (b->CardFlags & CARDFLAG_ZORRO_3) {
		dmy_cache
		((char *)&gfxdata->rgb[0])[0] = B;
		((char *)&gfxdata->rgb[0])[1] = G;
		((char *)&gfxdata->rgb[0])[2] = R;
		((char *)&gfxdata->rgb[0])[3] = 0x00;
		gfxdata->u8offset = idx + 1;

		ZZ_REGS_WRITE(REG_ZZ_BLITTER_DMA_OP, OP_SPRITE_COLOR);
	} else {
		ZZ_REGS_WRITE(REG_ZZ_USER1, R);
		ZZ_REGS_WRITE(REG_ZZ_USER2, B | (G << 8));
		ZZ_REGS_WRITE(REG_ZZ_SPRITE_COLORS, idx + 1);
	}
}

void SetSplitPosition(__REGA0(struct BoardInfo *b),__REGD0(SHORT pos)) {
	b->YSplit = pos;

	uint32_t offset = ((uint32_t)b->VisibleBitMap->Planes[0]) - ((uint32_t)b->MemoryBase);
	uint32_t* registers =(uint32_t*)b->RegisterBase;
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
	sizeof(struct Library),
	(uint32_t)device_vectors,
	0,
	(uint32_t)InitLib,
};
