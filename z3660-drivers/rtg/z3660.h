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

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned long

#define u16 uint16_t
#define u32 uint32_t

#define MNTVA_COLOR_8BIT     0
#define MNTVA_COLOR_16BIT565 1
#define MNTVA_COLOR_32BIT    2
#define MNTVA_COLOR_15BIT    3

typedef volatile struct MNTZZ9KRegs {
//  u16 fw_version; // 00
  u32 mode;       // 00
  u32 config;     // 04 misc config bits
  u32 sprite_x;   // 08
  u32 sprite_y;   // 0C

  u32 blitter_x1; // 10
  u32 blitter_y1; // 14
  u32 blitter_x2; // 18
  u32 blitter_y2; // 1C

  u32 pan_ptr;    // 20
  u32 blitter_row_pitch; // 24 destination pitch
  u32 blitter_x3; // 28
  u32 blitter_y3; // 2C

  u32 blitter_rgb;             // 30
  u32 blitter_op_fillrect;     // 34
  u32 blitter_op_copyrect;     // 38
  u32 blitter_op_filltemplate; // 3C

  u32 blitter_src; // 40
  u32 blitter_dst; // 44
  u32 blitter_colormode; // 48 destination colormode
  u32 blitter_src_pitch; // 4C

  u32 blitter_rgb2;   // 50 background/secondary color
  u32 blitter_op_p2c; // 54
  u32 blitter_op_draw_line; // 58
  u32 blitter_op_p2d; // 5C

  // Reusing other register-accessible variables was getting a bit cluttered, and somewhat
  // of a coding hazard. Four additional user values should help for the time being.
  u32 blitter_user1; // 60
  u32 blitter_user2; // 64
  u32 blitter_user3; // 68
  u32 blitter_user4; // 6C

  u32 blitter_op_invertrect; // 70
  u32 sprite_bitmap; // 74
  u32 sprite_colors; // 78
  u32 vblank_status; // 7C
  
  u32 blitter_dma_op; // 80
  u32 blitter_acc_op; // 84
  u32 blitter_set_split_pos; // 88
  u32 un_8C; // 8C

  u32 eth_tx; // 90
  u32 eth_rx; // 94
  u32 un_9[2]; // 98,9C

  u32 zz_fw_version;
  u32 un_A[3]; // A4,A8,AC

  u32 un_B[4]; // B0,B4,B8,BC

  u32 un_C[4]; // C0,C4,C8,CC

  u32 un_D[4]; // D0,D4,D8,DC

  u32 un_E[4]; // E0,E4,E8,EC

  u32 un_F[3]; // F0,F4,F8
  u32 zz_debug; // FC

/*
  u32 un_4[6]; // 84,86,88,8a,8c,8e

  u32 arm_run; // 90
  u32 arm_argc;   // 94
  u32 arm_arg[8]; // 96,98,9a,9c..a4

  u32 un_5[5]; // a6..ae
  
  u32 arm_event_serial; // b0
  u32 arm_event_code; // b2*/
} MNTZZ9KRegs;
/*
typedef volatile struct MNTZZ9KCXRegs {
  u16 video_control_data_hi; // 00
  u16 video_control_data_lo; // 02
  u16 video_control_op;      // 04
  u16 videocap_mode;         // 06
} MNTZZ9KCXRegs;
*/
/*
enum zz_reg_offsets {
  REG_ZZ_HW_VERSION     = 0x00,
  REG_ZZ_MODE           = 0x02,
  REG_ZZ_CONFIG         = 0x04,
  REG_ZZ_SPRITE_X       = 0x06,
  REG_ZZ_SPRITE_Y       = 0x08,
  REG_ZZ_PAN_HI         = 0x0A,
  REG_ZZ_PAN_LO         = 0x0C,
  REG_ZZ_VCAP_MODE      = 0x0E,
  
  REG_ZZ_X1             = 0x10,
  REG_ZZ_Y1             = 0x12,
  REG_ZZ_X2             = 0x14,
  REG_ZZ_Y2             = 0x16,
  REG_ZZ_ROW_PITCH      = 0x18,
  REG_ZZ_X3             = 0x1A,
  REG_ZZ_Y3             = 0x1C,
  REG_ZZ_RGB_HI         = 0x1E,

  REG_ZZ_RGB_LO         = 0x20,
  REG_ZZ_FILLRECT       = 0x22,
  REG_ZZ_COPYRECT       = 0x24,
  REG_ZZ_FILLTEMPLATE   = 0x26,
  REG_ZZ_BLIT_SRC_HI    = 0x28,
  REG_ZZ_BLIT_SRC_LO    = 0x2A,
  REG_ZZ_BLIT_DST_HI    = 0x2C,
  REG_ZZ_BLIT_DST_LO    = 0x2E,

  REG_ZZ_COLORMODE      = 0x30,
  REG_ZZ_SRC_PITCH      = 0x32,
  REG_ZZ_RGB2_HI        = 0x34,
  REG_ZZ_RGB2_LO        = 0x36,
  REG_ZZ_P2C            = 0x38,
  REG_ZZ_DRAWLINE       = 0x3A,
  REG_ZZ_P2D            = 0x3C,
  REG_ZZ_INVERTRECT     = 0x3E,

  REG_ZZ_USER1          = 0x40,
  REG_ZZ_USER2          = 0x42,
  REG_ZZ_USER3          = 0x44,
  REG_ZZ_USER4          = 0x46,
  REG_ZZ_SPRITE_BITMAP  = 0x48,
  REG_ZZ_SPRITE_COLORS  = 0x4A,
  REG_ZZ_VBLANK_STATUS  = 0x4C,
  REG_ZZ_UNUSED_REG4E   = 0x4E,

  REG_ZZ_UNUSED_REG50   = 0x50,
  REG_ZZ_UNUSED_REG52   = 0x52,
  REG_ZZ_UNUSED_REG54   = 0x54,
  REG_ZZ_UNUSED_REG56   = 0x56,
  REG_ZZ_UNUSED_REG58   = 0x58,
  REG_ZZ_DMA_OP         = 0x5A,
  REG_ZZ_ACC_OP         = 0x5C,
  REG_ZZ_SET_SPLIT_POS  = 0x5E,

  REG_ZZ_UNUSED_REG60   = 0x60,
  REG_ZZ_UNUSED_REG62   = 0x62,
  REG_ZZ_UNUSED_REG64   = 0x64,
  REG_ZZ_UNUSED_REG66   = 0x66,
  REG_ZZ_UNUSED_REG68   = 0x68,
  REG_ZZ_UNUSED_REG6A   = 0x6A,
  REG_ZZ_UNUSED_REG6C   = 0x6C,
  REG_ZZ_UNUSED_REG6E   = 0x6E,

  REG_ZZ_UNUSED_REG70   = 0x70,
  REG_ZZ_UNUSED_REG72   = 0x72,
  REG_ZZ_UNUSED_REG74   = 0x74,
  REG_ZZ_UNUSED_REG76   = 0x76,
  REG_ZZ_UNUSED_REG78   = 0x78,
  REG_ZZ_UNUSED_REG7A   = 0x7A,
  REG_ZZ_UNUSED_REG7C   = 0x7C,
  REG_ZZ_UNUSED_REG7E   = 0x7E,

  REG_ZZ_ETH_TX         = 0x80,
  REG_ZZ_ETH_RX         = 0x82,
  REG_ZZ_ETH_MAC_HI     = 0x84,
  REG_ZZ_ETH_MAC_HI2    = 0x86,
  REG_ZZ_ETH_MAC_LO     = 0x88,
  REG_ZZ_UNUSED_REG8A   = 0x8A,
  REG_ZZ_UNUSED_REG8C   = 0x8C,
  REG_ZZ_UNUSED_REG8E   = 0x8E,

  REG_ZZ_ARM_RUN_HI     = 0x90,
  REG_ZZ_ARM_RUN_LO     = 0x92,
  REG_ZZ_ARM_ARGC       = 0x94,
  REG_ZZ_ARM_ARGV0      = 0x96,
  REG_ZZ_ARM_ARGV1      = 0x98,
  REG_ZZ_ARM_ARGV2      = 0x9A,
  REG_ZZ_ARM_ARGV3      = 0x9C,
  REG_ZZ_ARM_ARGV4      = 0x9E,

  REG_ZZ_ARM_ARGV5      = 0xA0,
  REG_ZZ_ARM_ARGV6      = 0xA2,
  REG_ZZ_ARM_ARGV7      = 0xA4,
  REG_ZZ_UNUSED_REGA6   = 0xA6,
  REG_ZZ_UNUSED_REGA8   = 0xA8,
  REG_ZZ_UNUSED_REGAA   = 0xAA,
  REG_ZZ_UNUSED_REGAC   = 0xAC,
  REG_ZZ_UNUSED_REGAE   = 0xAE,

  REG_ZZ_ARM_EV_SERIAL  = 0xB0,
  REG_ZZ_ARM_EV_CODE    = 0xB2,
  REG_ZZ_UNUSED_REGB4   = 0xB4,
  REG_ZZ_UNUSED_REGB6   = 0xB6,
  REG_ZZ_UNUSED_REGB8   = 0xB8,
  REG_ZZ_UNUSED_REGBA   = 0xBA,
  REG_ZZ_UNUSED_REGBC   = 0xBC,
  REG_ZZ_UNUSED_REGBE   = 0xBE,

  REG_ZZ_FW_VERSION     = 0xC0,
  REG_ZZ_UNUSED_REGC2   = 0xC2,
  REG_ZZ_UNUSED_REGC4   = 0xC4,
  REG_ZZ_UNUSED_REGC6   = 0xC6,
  REG_ZZ_UNUSED_REGC8   = 0xC8,
  REG_ZZ_UNUSED_REGCA   = 0xCA,
  REG_ZZ_UNUSED_REGCC   = 0xCC,
  REG_ZZ_UNUSED_REGCE   = 0xCE,

  REG_ZZ_USBBLK_TX_HI   = 0xD0,
  REG_ZZ_USBBLK_TX_LO   = 0xD2,
  REG_ZZ_USBBLK_RX_HI   = 0xD4,
  REG_ZZ_USBBLK_RX_LO   = 0xD6,
  REG_ZZ_USB_STATUS     = 0xD8,
  REG_ZZ_USB_BUFSEL     = 0xDA,
  REG_ZZ_USB_CAPACITY   = 0xDC,
  REG_ZZ_UNUSED_REGDE   = 0xDE,

  REG_ZZ_UNUSED_REGE0   = 0xE0,
  REG_ZZ_UNUSED_REGE2   = 0xE2,
  REG_ZZ_UNUSED_REGE4   = 0xE4,
  REG_ZZ_UNUSED_REGE6   = 0xE6,
  REG_ZZ_UNUSED_REGE8   = 0xE8,
  REG_ZZ_UNUSED_REGEA   = 0xEA,
  REG_ZZ_UNUSED_REGEC   = 0xEC,
  REG_ZZ_UNUSED_REGEE   = 0xEE,

  REG_ZZ_UNUSED_REGF0   = 0xF0,
  REG_ZZ_UNUSED_REGF2   = 0xF2,
  REG_ZZ_UNUSED_REGF4   = 0xF4,
  REG_ZZ_UNUSED_REGF6   = 0xF6,
  REG_ZZ_UNUSED_REGF8   = 0xF8,
  REG_ZZ_UNUSED_REGFA   = 0xFA,
  REG_ZZ_DEBUG          = 0xFC,
  REG_ZZ_UNUSED_REGFE   = 0xFE,
};
*/
enum zz_reg_offsets {
  REG_ZZ_MODE           = 0x000,
  REG_ZZ_CONFIG         = 0x004,
  REG_ZZ_SPRITE_X       = 0x008,
  REG_ZZ_SPRITE_Y       = 0x00C,

  REG_ZZ_X1             = 0x010,
  REG_ZZ_Y1             = 0x014,
  REG_ZZ_X2             = 0x018,
  REG_ZZ_Y2             = 0x01C,

  REG_ZZ_PAN            = 0x020,
  REG_ZZ_ROW_PITCH      = 0x024,
  REG_ZZ_X3             = 0x028,
  REG_ZZ_Y3             = 0x02C,

  REG_ZZ_RGB            = 0x030,
  REG_ZZ_FILLRECT       = 0x034,
  REG_ZZ_COPYRECT       = 0x038,
  REG_ZZ_FILLTEMPLATE   = 0x03C,

  REG_ZZ_BLIT_SRC       = 0x040,
  REG_ZZ_BLIT_DST       = 0x044,
  REG_ZZ_COLORMODE      = 0x048,
  REG_ZZ_SRC_PITCH      = 0x04C,

  REG_ZZ_RGB2           = 0x050,
  REG_ZZ_P2C            = 0x054,
  REG_ZZ_DRAWLINE       = 0x058,
  REG_ZZ_P2D            = 0x05C,

  REG_ZZ_USER1          = 0x060,
  REG_ZZ_USER2          = 0x064,
  REG_ZZ_USER3          = 0x068,
  REG_ZZ_USER4          = 0x06C,

  REG_ZZ_INVERTRECT     = 0x070,
  REG_ZZ_SPRITE_BITMAP  = 0x074,
  REG_ZZ_SPRITE_COLORS  = 0x078,
  REG_ZZ_VBLANK_STATUS  = 0x07C,

  /*
  REG_ZZ_UNUSED_REG4E   = 0x4E,
  REG_ZZ_UNUSED_REG50   = 0x50,
  REG_ZZ_UNUSED_REG52   = 0x52,
  REG_ZZ_UNUSED_REG54   = 0x54,
  REG_ZZ_UNUSED_REG56   = 0x56,
  REG_ZZ_UNUSED_REG58   = 0x58,
*/
  REG_ZZ_BITTER_DMA_OP  = 0x080,
  REG_ZZ_ACC_OP         = 0x084,
  REG_ZZ_SET_SPLIT_POS  = 0x088,
  REG_ZZ_UNUSED_REG08C  = 0x08C,
/*
  REG_ZZ_UNUSED_REG60   = 0x60,
  REG_ZZ_UNUSED_REG62   = 0x62,
  REG_ZZ_UNUSED_REG64   = 0x64,
  REG_ZZ_UNUSED_REG66   = 0x66,
  REG_ZZ_UNUSED_REG68   = 0x68,
  REG_ZZ_UNUSED_REG6A   = 0x6A,
  REG_ZZ_UNUSED_REG6C   = 0x6C,
  REG_ZZ_UNUSED_REG6E   = 0x6E,

  REG_ZZ_UNUSED_REG70   = 0x70,
  REG_ZZ_UNUSED_REG72   = 0x72,
  REG_ZZ_UNUSED_REG74   = 0x74,
  REG_ZZ_UNUSED_REG76   = 0x76,
  REG_ZZ_UNUSED_REG78   = 0x78,
  REG_ZZ_UNUSED_REG7A   = 0x7A,
  REG_ZZ_UNUSED_REG7C   = 0x7C,
  REG_ZZ_UNUSED_REG7E   = 0x7E,
*/
  REG_ZZ_ETH_TX         = 0x090,
  REG_ZZ_ETH_RX         = 0x094,
  REG_ZZ_ETH_MAC_HI     = 0x098,
  REG_ZZ_ETH_MAC_LO     = 0x09C,
/*
  REG_ZZ_ARM_RUN_HI     = 0x90,
  REG_ZZ_ARM_RUN_LO     = 0x92,
  REG_ZZ_ARM_ARGC       = 0x94,
  REG_ZZ_ARM_ARGV0      = 0x96,
  REG_ZZ_ARM_ARGV1      = 0x98,
  REG_ZZ_ARM_ARGV2      = 0x9A,
  REG_ZZ_ARM_ARGV3      = 0x9C,
  REG_ZZ_ARM_ARGV4      = 0x9E,

  REG_ZZ_ARM_ARGV5      = 0xA0,
  REG_ZZ_ARM_ARGV6      = 0xA2,
  REG_ZZ_ARM_ARGV7      = 0xA4,
  REG_ZZ_UNUSED_REGA6   = 0xA6,
  REG_ZZ_UNUSED_REGA8   = 0xA8,
  REG_ZZ_UNUSED_REGAA   = 0xAA,
  REG_ZZ_UNUSED_REGAC   = 0xAC,
  REG_ZZ_UNUSED_REGAE   = 0xAE,

  REG_ZZ_ARM_EV_SERIAL  = 0xB0,
  REG_ZZ_ARM_EV_CODE    = 0xB2,
  REG_ZZ_UNUSED_REGB4   = 0xB4,
  REG_ZZ_UNUSED_REGB6   = 0xB6,
  REG_ZZ_UNUSED_REGB8   = 0xB8,
  REG_ZZ_UNUSED_REGBA   = 0xBA,
  REG_ZZ_UNUSED_REGBC   = 0xBC,
  REG_ZZ_UNUSED_REGBE   = 0xBE,
*/
  REG_ZZ_FW_VERSION     = 0x0A0,
  REG_ZZ_UNUSED_REG0A4  = 0x0A4,
  REG_ZZ_UNUSED_REG0A8  = 0x0A8,
  REG_ZZ_UNUSED_REG0AC  = 0x0AC,

  REG_ZZ_USBBLK_TX      = 0x0B0,
  REG_ZZ_USBBLK_RX      = 0x0B4,
  REG_ZZ_USB_STATUS     = 0x0B8,
  REG_ZZ_USB_BUFSEL     = 0x0BC,

  REG_ZZ_USB_CAPACITY   = 0x0C0,
  REG_ZZ_UNUSED_REGDE   = 0x0C4,
  REG_ZZ_UNUSED_REG0C8  = 0x0C8,
  REG_ZZ_UNUSED_REG0CC  = 0x0CC,

  REG_ZZ_TEMPERATURE    = 0x0D0,
  REG_ZZ_VOLTAGE_AUX    = 0x0D4,
  REG_ZZ_VOLTAGE_INT    = 0x0D8,
  REG_ZZ_UNUSED_REG0DC  = 0x0DC,

  REG_ZZ_UNUSED_REG0E0  = 0x0E0,
  REG_ZZ_UNUSED_REG0E4  = 0x0E4,
  REG_ZZ_UNUSED_REG0E8  = 0x0E8,
  REG_ZZ_UNUSED_REG0EC  = 0x0EC,

  REG_ZZ_UNUSED_REG0F0  = 0x0F0,
  REG_ZZ_UNUSED_REG0F4  = 0x0F4,
  REG_ZZ_UNUSED_REG0F8  = 0x0F8,
  REG_ZZ_DEBUG          = 0x0FC,
};

enum gfx_dma_op {
  OP_NONE,
  OP_DRAWLINE,
  OP_FILLRECT,
  OP_COPYRECT,
  OP_COPYRECT_NOMASK,
  OP_RECT_TEMPLATE,
  OP_RECT_PATTERN,
  OP_P2C,
  OP_P2D,
  OP_INVERTRECT,
  OP_PAN,
  OP_SPRITE_XY,
  OP_SPRITE_COLOR,
  OP_SPRITE_BITMAP,
  OP_SPRITE_CLUT_BITMAP,
  OP_ETH_USB_OFFSETS,
  OP_SET_SPLIT_POS,
  OP_NUM,
};

enum gfx_acc_op {
  ACC_OP_NONE,
  ACC_OP_BUFFER_FLIP,
  ACC_OP_BUFFER_CLEAR,
  ACC_OP_BLIT_RECT,
  ACC_OP_ALLOC_SURFACE,
  ACC_OP_FREE_SURFACE,
  ACC_OP_SET_BPP_CONVERSION_TABLE,
  ACC_OP_DRAW_LINE,
  ACC_OP_FILL_RECT,
  ACC_OP_NUM,
};

enum gfxdata_offsets {
  GFXDATA_DST,
  GFXDATA_SRC,
};

enum gfxdata_u8_types {
  GFXDATA_U8_COLORMODE,
  GFXDATA_U8_DRAWMODE,
  GFXDATA_U8_LINE_PATTERN_OFFSET,
  GFXDATA_U8_LINE_PADDING,
};

#pragma pack(4)
struct GFXData {
  uint32 offset[2];
  uint32 rgb[2];
  uint16 x[4], y[4];
  uint16 user[4];
  uint16 pitch[4];
  uint8 u8_user[8];
  uint8 op, mask, minterm, u8offset;
  uint32_t u32_user[8];
  uint8 clut1[768];
  uint8 clut2[768];
  uint8 clut3[768];
  uint8 clut4[768];
};

//#define ZZWRITE16(a, r, v) *(volatile short*)((unsigned int)a+r) = (volatile short)v;
