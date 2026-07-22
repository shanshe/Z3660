/*
 * Z3660 Graphics Card Driver based on MNT ZZ9000 rev 1.13
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

#include <exec/lists.h>
#include <exec/interrupts.h>
#include <graphics/gfx.h>

#define int32 long
#define int16 short
#define int8  char

#define uint32 unsigned long
#define uint16 unsigned short
#define uint8  unsigned char

#define RTG_COLOR_FORMAT_PLANAR        RGBFF_NONE
#define RTG_COLOR_FORMAT_CLUT          RGBFF_CLUT
#define RTG_COLOR_FORMAT_RGB888        RGBFF_R8G8B8
#define RTG_COLOR_FORMAT_BGR888        RGBFF_B8G8R8
#define RTG_COLOR_FORMAT_RGB565_WEIRD1 RGBFF_R5G6B5PC
#define RTG_COLOR_FORMAT_RGB555_WEIRD1 RGBFF_R5G5B5PC
#define RTG_COLOR_FORMAT_ARGB          RGBFF_A8R8G8B8
#define RTG_COLOR_FORMAT_ABGR          RGBFF_A8B8G8R8
#define RTG_COLOR_FORMAT_RGBA          RGBFF_R8G8B8A8
#define RTG_COLOR_FORMAT_BGRA          RGBFF_B8G8R8A8
#define RTG_COLOR_FORMAT_RGB565        RGBFF_R5G6B5
#define RTG_COLOR_FORMAT_RGB555        RGBFF_R5G5B5
#define RTG_COLOR_FORMAT_BGR565_WEIRD2 RGBFF_B5G6R5PC
#define RTG_COLOR_FORMAT_BGR555_WEIRD2 RGBFF_B5G5R5PC
#define RTG_COLOR_FORMAT_32BIT         (RTG_COLOR_FORMAT_ARGB|RTG_COLOR_FORMAT_ABGR|RTG_COLOR_FORMAT_RGBA|RTG_COLOR_FORMAT_BGRA)

typedef enum zz_video_modes {
   ZZVMODE_1280x720,
   ZZVMODE_800x600,
   ZZVMODE_640x480,
   ZZVMODE_1024x768,
   ZZVMODE_1280x1024,
   ZZVMODE_1920x1080_60,
   ZZVMODE_720x576,           // 50Hz
   ZZVMODE_1920x1080_50,      // 50Hz
   ZZVMODE_720x480,
   ZZVMODE_640x512,
   ZZVMODE_1600x1200,
   ZZVMODE_2560x1440_30,
   ZZVMODE_720x576_NS_PAL,    // Non-standard "50Hz" (PAL Amiga)
   ZZVMODE_720x480_NS_PAL,    // Non-standard "60Hz" (PAL Amiga)
   ZZVMODE_720x576_NS_NTSC,   // Non-standard "50Hz" (NTSC Amiga)
   ZZVMODE_720x480_NS_NTSC,   // Non-standard "60Hz" (NTSC Amiga)
   ZZVMODE_640x400,
   ZZVMODE_1280x800,
   ZZVMODE_1920x1200,
   ZZVMODE_1600x900,
   ZZVMODE_1680x1050,
   ZZVMODE_1366x768,
   ZZVMODE_CUSTOM,
   ZZVMODE_NUM,
} zz_video_modes_t;