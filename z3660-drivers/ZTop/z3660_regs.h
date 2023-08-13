/*
 * Z3660 Graphics Card Driver based on MNT ZZ9000 rev 1.11
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 */

/*
 * MNT ZZ9000 Amiga Graphics Card Driver (ZZ9000.card)
 * Copyright (C) 2016-2020, Lukas F. Hartmann <lukas@mntre.com>
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
#define MNTVA_COLOR_NO_USE   255

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

   REG_ZZ_BLITTER_DMA_OP = 0x080,
   REG_ZZ_ACC_OP         = 0x084,
   REG_ZZ_SET_SPLIT_POS  = 0x088,
   REG_ZZ_ORIG_RES       = 0x08C,

   REG_ZZ_ETH_TX         = 0x090,
   REG_ZZ_ETH_RX         = 0x094,
   REG_ZZ_ETH_MAC_HI     = 0x098,
   REG_ZZ_ETH_MAC_LO     = 0x09C,

   REG_ZZ_FW_VERSION     = 0x0A0,
   REG_ZZ_ETH_RX_ADDRESS = 0x0A4,
   REG_ZZ_INT_STATUS     = 0x0A8,
   REG_ZZ_USB_CAPACITY   = 0x0AC,

   REG_ZZ_USBBLK_TX      = 0x0B0,
   REG_ZZ_USBBLK_RX      = 0x0B4,
   REG_ZZ_USB_STATUS     = 0x0B8,
   REG_ZZ_USB_BUFSEL     = 0x0BC,

   REG_ZZ_DECODER_PARAM  = 0x0C0,
   REG_ZZ_DECODER_VAL    = 0x0C4,
   REG_ZZ_DECODE         = 0x0C8,
   REG_ZZ_DECODER_FIFOTX = 0x0CC,

   REG_ZZ_DECODER_FIFORX = 0x0D0,
   REG_ZZ_TEMPERATURE    = 0x0D4,
   REG_ZZ_VOLTAGE_AUX    = 0x0D8,
   REG_ZZ_VOLTAGE_INT    = 0x0DC,

   REG_ZZ_SET_FEATURE    = 0x0E0,
   REG_ZZ_AUDIO_SWAB     = 0x0E4,
   REG_ZZ_AUDIO_SCALE    = 0x0E8,
   REG_ZZ_AUDIO_PARAM    = 0x0EC,

   REG_ZZ_AUDIO_VAL      = 0x0F0,
   REG_ZZ_AUDIO_CONFIG   = 0x0F4,
   REG_ZZ_CPU_FREQ       = 0x0F8,
   REG_ZZ_DEBUG          = 0x0FC,

   ZZ_CUSTOM_VIDMODE     = 0x100,
   ZZ_CUSTOM_VIDMODE_DATA= 0x104,
   REG_ZZ_EMULATION_USED = 0x108,
   REG_ZZ_JIT_ENABLE     = 0x10C,

   REG_ZZ_BOOTMODE       = 0x110,
   REG_ZZ_APPLY_BOOTMODE = 0x114,
   REG_ZZ_SCSIBOOT_EN    = 0x118,
   REG_ZZ_AUTOC_RAM_EN   = 0x11C,

   REG_ZZ_LTC_TEMP       = 0x120,
   REG_ZZ_LTC_V1         = 0x124,
   REG_ZZ_LTC_V2         = 0x128,
   REG_ZZ_LTC_RESISTOR   = 0x12C,

   REG_ZZ_LTC_VCC        = 0x130,

   //NOT USED 0x134 - 0xFFC

   REG_ZZ_OP_DATA        = 0x1000,
   REG_ZZ_OP             = 0x1004,
   REG_ZZ_OP_NOP         = 0x1008,
   REG_ZZ_OP_CAPTUREMODE = 0x100C,

//   REG_ZZ_RX_BUFF        = 0x2000,
//   REG_ZZ_TX_BUFF        = 0x8000
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
};

#define AUDIO_TX_FRAME_ADDRESS 0x07DE0000
#define AUDIO_RX_FRAME_ADDRESS 0x07E00000
#define TX_FRAME_ADDRESS 0x07F00000

enum zz9k_card_features {
    CARD_FEATURE_NONE,
    CARD_FEATURE_SECONDARY_PALETTE,
    CARD_FEATURE_NONSTANDARD_VSYNC,
    CARD_FEATURE_NUM,
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
    uint32_t offset[2];
    uint32_t rgb[2];
    uint16_t x[4], y[4];
    uint16_t user[4];
    uint16_t pitch[4];
    uint8_t u8_user[8];
    uint8_t op, mask, minterm, u8offset;
    uint32_t u32_user[8];
    uint8_t clut1[768];
    uint8_t clut2[768];
    uint8_t clut3[768];
    uint8_t clut4[768];
};
