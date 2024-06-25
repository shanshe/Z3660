/*
 * MNT ZZ9000 Amiga Graphics and Coprocessor Card Operating System (ZZ9000OS)
 *
 * Copyright (C) 2020, Lukas F. Hartmann <lukas@mntre.com>
 *                     MNT Research GmbH, Berlin
 *                     https://mntre.com
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 *
 */
#ifndef ZZREGS_H

#define ZZREGS_H
// Registers offsets relative to the register base, although the offset on the ARM side is always 0.

enum zz_reg_offsets {
   REG_ZZ_MODE           = 0x100,
   REG_ZZ_CONFIG         = 0x104,
   REG_ZZ_SPRITE_X       = 0x108,
   REG_ZZ_SPRITE_Y       = 0x10C,

   REG_ZZ_X1             = 0x110,
   REG_ZZ_Y1             = 0x114,
   REG_ZZ_X2             = 0x118,
   REG_ZZ_Y2             = 0x11C,

   REG_ZZ_PAN            = 0x120,
   REG_ZZ_ROW_PITCH      = 0x124,
   REG_ZZ_X3             = 0x128,
   REG_ZZ_Y3             = 0x12C,

   REG_ZZ_RGB            = 0x130,
   REG_ZZ_FILLRECT       = 0x134,
   REG_ZZ_COPYRECT       = 0x138,
   REG_ZZ_FILLTEMPLATE   = 0x13C,

   REG_ZZ_BLIT_SRC       = 0x140,
   REG_ZZ_BLIT_DST       = 0x144,
   REG_ZZ_COLORMODE      = 0x148,
   REG_ZZ_SRC_PITCH      = 0x14C,

   REG_ZZ_RGB2           = 0x150,
   REG_ZZ_P2C            = 0x154,
   REG_ZZ_DRAWLINE       = 0x158,
   REG_ZZ_P2D            = 0x15C,

   REG_ZZ_USER1          = 0x160,
   REG_ZZ_USER2          = 0x164,
   REG_ZZ_USER3          = 0x168,
   REG_ZZ_USER4          = 0x16C,

   REG_ZZ_INVERTRECT     = 0x170,
   REG_ZZ_SPRITE_BITMAP  = 0x174,
   REG_ZZ_SPRITE_COLORS  = 0x178,
   REG_ZZ_VBLANK_STATUS  = 0x17C,

   REG_ZZ_BLITTER_DMA_OP = 0x180,
   REG_ZZ_ACC_OP         = 0x184,
   REG_ZZ_SET_SPLIT_POS  = 0x188,
   REG_ZZ_ORIG_RES       = 0x18C,

   REG_ZZ_ETH_TX         = 0x190,
   REG_ZZ_ETH_RX         = 0x194,
   REG_ZZ_ETH_MAC_HI     = 0x198,
   REG_ZZ_ETH_MAC_LO     = 0x19C,

   REG_ZZ_FW_VERSION     = 0x1A0,
   REG_ZZ_ETH_RX_ADDRESS = 0x1A4,
   REG_ZZ_INT_STATUS     = 0x1A8,
   REG_ZZ_USB_CAPACITY   = 0x1AC,

   REG_ZZ_USBBLK_TX      = 0x1B0,
   REG_ZZ_USBBLK_RX      = 0x1B4,
   REG_ZZ_USB_STATUS     = 0x1B8,
   REG_ZZ_USB_BUFSEL     = 0x1BC,

   REG_ZZ_DECODER_PARAM  = 0x1C0,
   REG_ZZ_DECODER_VAL    = 0x1C4,
   REG_ZZ_DECODE         = 0x1C8,
   REG_ZZ_DECODER_FIFOTX = 0x1CC,

   REG_ZZ_DECODER_FIFORX = 0x1D0,
   REG_ZZ_TEMPERATURE    = 0x1D4,
   REG_ZZ_VOLTAGE_AUX    = 0x1D8,
   REG_ZZ_VOLTAGE_INT    = 0x1DC,

   REG_ZZ_SET_FEATURE    = 0x1E0,
   REG_ZZ_AUDIO_SWAB     = 0x1E4,
   REG_ZZ_AUDIO_SCALE    = 0x1E8,
   REG_ZZ_AUDIO_PARAM    = 0x1EC,

   REG_ZZ_AUDIO_VAL      = 0x1F0,
   REG_ZZ_AUDIO_CONFIG   = 0x1F4,
   REG_ZZ_CPU_FREQ       = 0x1F8,
   REG_ZZ_DEBUG          = 0x1FC,

   ZZ_CUSTOM_VIDMODE     = 0x200,
   ZZ_CUSTOM_VIDMODE_DATA= 0x204,
   REG_ZZ_EMULATION_USED = 0x208,
   REG_ZZ_JIT_ENABLE     = 0x20C,

   REG_ZZ_BOOTMODE       = 0x210,
   REG_ZZ_APPLY_BOOTMODE = 0x214,
   REG_ZZ_SCSIBOOT_EN    = 0x218,
   REG_ZZ_AUTOC_RAM_EN   = 0x21C,

   REG_ZZ_LTC_TEMP       = 0x220,
   REG_ZZ_LTC_V1         = 0x224,
   REG_ZZ_LTC_V2         = 0x228,
   REG_ZZ_LTC_060_TEMP   = 0x22C,

   REG_ZZ_LTC_VCC        = 0x230,
   REG_ZZ_CPU_RAM_EN     = 0x234,
   REG_ZZ_KS_SEL         = 0x238,
   REG_ZZ_EXT_KS_SEL     = 0x23C,

   REG_ZZ_KS_SEL_TXT     = 0x240,
   REG_ZZ_EXT_KS_SEL_TXT = 0x244,
   REG_ZZ_SCSI_SEL_0     = 0x248,
   REG_ZZ_SCSI_SEL_1     = 0x24C,

   REG_ZZ_SCSI_SEL_2     = 0x250,
   REG_ZZ_SCSI_SEL_3     = 0x254,
   REG_ZZ_SCSI_SEL_4     = 0x258,
   REG_ZZ_SCSI_SEL_5     = 0x25C,

   REG_ZZ_SCSI_SEL_6     = 0x260,
   REG_ZZ_SCSI_SEL_TXT   = 0x264,
   REG_ZZ_APPLY_SCSI     = 0x268,
   REG_ZZ_APPLY_ALL      = 0x26C,

   REG_ZZ_SOFT3D_OP      = 0x270,

   REG_ZZ_FW_BETA        = 0x280,

   //NOT USED 0x284 - 0x2FC

   REG_ZZ_OP_DATA        = 0x300,
   REG_ZZ_OP             = 0x304,
   REG_ZZ_OP_NOP         = 0x308,
   REG_ZZ_OP_CAPTUREMODE = 0x30C,

   REG_ZZ_SEL_KS_TXT     = 0x500,
   REG_ZZ_SEL_SCSI_TXT   = 0x600,

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

enum zz9k_card_features {
   CARD_FEATURE_NONE,
   CARD_FEATURE_SECONDARY_PALETTE,
   CARD_FEATURE_NONSTANDARD_VSYNC,
   CARD_FEATURE_NUM,
};

enum gfx_soft3d_op {
  OP_SETBITMAP,
  OP_SETCLIPPING,
  OP_SETDRAWSTATE,
  OP_DRAWPRIMITIVE,
  OP_DOUPDATE,
  OP_FLUSH,
  OP_END,
  OP_CREATETEXTURE,
  OP_FREETEXTURE,
  OP_UPDATETEXTURE,
  OP_START,
  OP_ALLOCZBUFFER,
  OP_ALLOCIMAGEBUFFER,
  OP_CLEARZBUFFER,
  OP_READZSPAN,
  OP_WRITEZSPAN,
  OP_SOFT3D_NUM,
};
#endif
