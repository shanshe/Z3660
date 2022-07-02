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

#include <proto/expansion.h>
#include <exec/libraries.h>

#define CLOCK_HZ 100000000

#include "rtg.h"

/* library functions -------------------- */

struct MNTGFXBase {
  struct Library libNode;
  BPTR segList;
  struct ExecBase* sysBase;
  struct ExpansionBase* expansionBase;
};

int FindCard(__reg("a0") struct RTGBoard* b);
int InitCard(__reg("a0") struct RTGBoard* b);

/* rtg functions ------------------------ */

void nop();

void init_dac(__reg("a0") struct RTGBoard* b, __reg("d7") uint16 format);
uint32 enable_display(__reg("a0") struct RTGBoard* b, __reg("d0") uint16 enabled);
void pan(__reg("a0") struct RTGBoard* b,__reg("a1") uint8* mem,__reg("d0") uint16 w,__reg("d1") int16 x,__reg("d2") int16 y,__reg("d7") uint16 format);
void pan_dma(__reg("a0") struct RTGBoard* b,__reg("a1") uint8* mem,__reg("d0") uint16 w,__reg("d1") int16 x,__reg("d2") int16 y,__reg("d7") uint16 format);
void set_memory_mode(__reg("a0") struct RTGBoard* b,__reg("d7") uint16 format);
void set_read_plane(__reg("a0") struct RTGBoard* b,__reg("d0") uint8 p);
void set_write_mask(__reg("a0") struct RTGBoard* b,__reg("d0") uint8 m);
void set_clear_mask(__reg("a0") struct RTGBoard* b,__reg("d0") uint8 m);
void vsync_wait(__reg("a0") struct RTGBoard* b);
int is_vsynced(__reg("a0") struct RTGBoard* b,__reg("d0") uint8 p);
void set_clock(__reg("a0") struct RTGBoard* b);
void set_palette(__reg("a0") struct RTGBoard* b,__reg("d0") uint16 idx,__reg("d1") uint16 len);
void init_mode(__reg("a0") struct RTGBoard* b,__reg("a1") struct ModeInfo* m,__reg("d0") int16 border);
uint32 is_bitmap_compatible(__reg("a0") struct RTGBoard* b,__reg("d7") uint16 format);
uint16 get_pitch(__reg("a0") struct RTGBoard* b,__reg("d0") uint16 width,__reg("d7") uint16 format);
uint32 map_address(__reg("a0") struct RTGBoard* b,__reg("a1") uint32 addr);
uint32 get_pixelclock_index(__reg("a0") struct RTGBoard* b,__reg("a1") struct ModeInfo* mode,__reg("d0") int32 clock,__reg("d7") uint16 format);
uint32 get_pixelclock_hz(__reg("a0") struct RTGBoard* b,__reg("a1") struct ModeInfo* mode,__reg("d0") int32 clock,__reg("d7") uint16 format);
uint32 monitor_switch(__reg("a0") struct RTGBoard* b,__reg("d0") uint16 state);
void rect_p2c(__reg("a0") struct RTGBoard* b,__reg("a1") struct BitMap* bm,__reg("a2") struct RenderInfo* r,__reg("d0") uint16 x,__reg("d1") uint16 y,__reg("d2") uint16 dx,__reg("d3") uint16 dy,__reg("d4") uint16 w,__reg("d5") uint16 h,__reg("d6") uint8 minterm,__reg("d7") uint8 mask);
void rect_p2c_dma(__reg("a0") struct RTGBoard* b,__reg("a1") struct BitMap* bm,__reg("a2") struct RenderInfo* r,__reg("d0") uint16 x,__reg("d1") uint16 y,__reg("d2") uint16 dx,__reg("d3") uint16 dy,__reg("d4") uint16 w,__reg("d5") uint16 h,__reg("d6") uint8 minterm,__reg("d7") uint8 mask);
void rect_p2d(__reg("a0") struct RTGBoard* b, __reg("a1") struct BitMap* bm, __reg("a2") struct RenderInfo* r, __reg("a3") struct ColorIndexMapping* cim, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 dx, __reg("d3") uint16 dy, __reg("d4") uint16 w, __reg("d5") uint16 h, __reg("d6") uint8 minterm, __reg("d7") uint8 mask);
void rect_p2d_dma(__reg("a0") struct RTGBoard* b, __reg("a1") struct BitMap* bm, __reg("a2") struct RenderInfo* r, __reg("a3") struct ColorIndexMapping* cim, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 dx, __reg("d3") uint16 dy, __reg("d4") uint16 w, __reg("d5") uint16 h, __reg("d6") uint8 minterm, __reg("d7") uint8 mask);
void rect_invert(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* r,__reg("d0") uint16 x, __reg("d1") uint16 y,__reg("d2") uint16 w,__reg("d3") uint16 h,__reg("d4") uint8 mask,__reg("d7") uint16 format);
void rect_invert_dma(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* r,__reg("d0") uint16 x, __reg("d1") uint16 y,__reg("d2") uint16 w,__reg("d3") uint16 h,__reg("d4") uint8 mask,__reg("d7") uint16 format);
void draw_line(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* r,__reg("a2") struct Line* l,__reg("d0") uint16 mask,__reg("d7") uint16 format);
void draw_line_dma(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* r,__reg("a2") struct Line* l,__reg("d0") uint16 mask,__reg("d7") uint16 format);
void rect_fill(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h, __reg("d4") uint32 color, __reg("d5") uint8 mask);
void rect_fill_dma(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h, __reg("d4") uint32 color, __reg("d5") uint8 mask);
void rect_copy(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* r,__reg("d0") uint16 x,__reg("d1") uint16 y,__reg("d2") uint16 dx,__reg("d3") uint16 dy,__reg("d4") uint16 w,__reg("d5") uint16 h,__reg("d6") uint8 m,__reg("d7") uint16 format);
void rect_copy_dma(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* r,__reg("d0") uint16 x,__reg("d1") uint16 y,__reg("d2") uint16 dx,__reg("d3") uint16 dy,__reg("d4") uint16 w,__reg("d5") uint16 h,__reg("d6") uint8 m,__reg("d7") uint16 format);
void rect_template(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("a2") struct Template* t,
                   __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h,
                   __reg("d4") uint32 mask, __reg("d7") uint32 format);
void rect_template_dma(__reg("a0") struct RTGBoard* b, __reg("a1") struct RenderInfo* r, __reg("a2") struct Template* t,
                   __reg("d0") uint16 x, __reg("d1") uint16 y, __reg("d2") uint16 w, __reg("d3") uint16 h,
                   __reg("d4") uint32 mask, __reg("d7") uint32 format);
void rect_pattern(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* r,__reg("a2") struct Pattern* pat,__reg("d0") uint16 x,__reg("d1") uint16 y,__reg("d2") uint16 w,__reg("d3") uint16 h,__reg("d4") uint8 mask,__reg("d7") uint32 format);
void rect_pattern_dma(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* r,__reg("a2") struct Pattern* pat,__reg("d0") uint16 x,__reg("d1") uint16 y,__reg("d2") uint16 w,__reg("d3") uint16 h,__reg("d4") uint8 mask,__reg("d7") uint32 format);
void rect_copy_nomask(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* sr,__reg("a2") struct RenderInfo* dr,__reg("d0") uint16 x,__reg("d1") uint16 y,__reg("d2") uint16 dx,__reg("d3") uint16 dy,__reg("d4") uint16 w,__reg("d5") uint16 h,__reg("d6") uint8 opcode,__reg("d7") uint16 format);
void rect_copy_nomask_dma(__reg("a0") struct RTGBoard* b,__reg("a1") struct RenderInfo* sr,__reg("a2") struct RenderInfo* dr,__reg("d0") uint16 x,__reg("d1") uint16 y,__reg("d2") uint16 dx,__reg("d3") uint16 dy,__reg("d4") uint16 w,__reg("d5") uint16 h,__reg("d6") uint8 opcode,__reg("d7") uint16 format);
void blitter_wait(__reg("a0") struct RTGBoard* b);

void sprite_setup(__reg("a0") struct RTGBoard* b,__reg("d0") uint8 enable);
void sprite_xy(__reg("a0") struct RTGBoard* b, __reg("d0") int16 x, __reg("d1") int16 y, __reg("d7") uint16 format);
void sprite_xy_dma(__reg("a0") struct RTGBoard* b, __reg("d0") int16 x, __reg("d1") int16 y, __reg("d7") uint16 format);
void sprite_bitmap(__reg("a0") struct RTGBoard* b,__reg("d7") uint16 format);
void sprite_bitmap_dma(__reg("a0") struct RTGBoard* b,__reg("d7") uint16 format);
void sprite_colors(__reg("a0") struct RTGBoard* b,__reg("d0") uint8 idx,__reg("d1") uint8 red,__reg("d2") uint8 green,__reg("d3") uint8 blue, __reg("d7") uint16 format);
void sprite_colors_dma(__reg("a0") struct RTGBoard* b,__reg("d0") uint8 idx,__reg("d1") uint8 red,__reg("d2") uint8 green,__reg("d3") uint8 blue, __reg("d7") uint16 format);

void set_split_pos(__reg("a0") struct RTGBoard* b, __reg("d0") int16 pos);
void set_split_pos_dma(__reg("a0") struct RTGBoard* b, __reg("d0") int16 pos);
