/*
 * MNT ZZ9000 Amiga Graphics and Coprocessor Card Operating System (ZZ9000OS)
 *
 * Copyright (C) 2019, Lukas F. Hartmann <lukas@mntre.com>
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
#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include "../video.h"
#include "../memorymap.h"

#define MNTVF_OP_UNUSED 12
#define MNTVF_OP_SPRITE_XY 13
#define MNTVF_OP_SPRITE_ADDR 14
#define MNTVF_OP_SPRITE_DATA 15
#define MNTVF_OP_VSYNC 5
#define MNTVF_OP_MAX 6
#define MNTVF_OP_HS 7
#define MNTVF_OP_VS 8
#define MNTVF_OP_POLARITY 10
#define MNTVF_OP_SCALE 4
#define MNTVF_OP_DIMENSIONS 2
#define MNTVF_OP_COLORMODE 1
#define MNTVF_OP_REPORT_LINE 17
#define MNTVF_OP_PALETTE_SEL 18
#define MNTVF_OP_PALETTE_HI 19

typedef struct Vec2 {
	float x;
	float y;
} Vec2;

typedef struct {
	int32_t	a[5];
	int32_t	b[5];
	int32_t	c[5];
} TriangleDef;

typedef struct {
	int32_t a[2];
} vec2_i32;

void video_formatter_write(uint32_t data, uint16_t op);
void handle_blitter_dma_op(ZZ_VIDEO_STATE* vs,uint16_t zdata);
void handle_soft3d_op(ZZ_VIDEO_STATE* vs,uint16_t zdata);
void handle_acc_op(uint16_t zdata);

void set_fb(uint32_t* fb_, uint32_t pitch);

void fill_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint32_t rect_rgb, uint32_t color_format, uint8_t mask);
void fill_rect_solid(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint32_t rect_rgb, uint32_t color_format);

void copy_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint16_t rect_sx, uint16_t rect_sy, uint32_t color_format, uint32_t* sp_src, uint32_t src_pitch, uint8_t mask);
void copy_rect_nomask(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint16_t rect_sx, uint16_t rect_sy, uint32_t color_format, uint32_t* sp_src, uint32_t src_pitch, uint8_t draw_mode);

void template_fill_rect(uint32_t color_format, uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h,
	uint8_t draw_mode, uint8_t mask, uint32_t fg_color, uint32_t bg_color,
	uint16_t x_offset, uint16_t y_offset,
	uint8_t *tmpl_data, uint16_t tmpl_pitch);
void pattern_fill_rect(uint32_t color_format, uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h,
	uint8_t draw_mode, uint8_t mask, uint32_t fg_color, uint32_t bg_color,
	uint16_t x_offset, uint16_t y_offset,
	uint8_t *tmpl_data, uint16_t tmpl_pitch, uint16_t loop_rows);

void draw_line(int16_t rect_x1, int16_t rect_y1, int16_t rect_x2, int16_t rect_y2, uint16_t len, uint16_t pattern, uint16_t pattern_offset, uint32_t fg_color, uint32_t bg_color, uint32_t color_format, uint8_t mask, uint8_t draw_mode);
void draw_line_solid(int16_t rect_x1, int16_t rect_y1, int16_t rect_x2, int16_t rect_y2, uint16_t len, uint32_t fg_color, uint32_t color_format);

void p2c_rect(int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h, uint8_t draw_mode, uint8_t planes, uint8_t mask, uint8_t layer_mask, uint16_t src_line_pitch, uint8_t *bmp_data_src);
void p2d_rect(int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h, uint8_t draw_mode, uint8_t planes, uint8_t mask, uint8_t layer_mask, uint32_t color_mask, uint16_t src_line_pitch, uint8_t *bmp_data_src, uint32_t color_format);
void invert_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint8_t mask, uint32_t color_format);

void acc_clear_buffer(uint32_t addr, uint16_t w, uint16_t h, uint16_t pitch_, uint32_t fg_color, uint32_t color_format);
void acc_flip_to_fb(uint32_t src, uint32_t dest, uint16_t w, uint16_t h, uint16_t pitch_, uint32_t color_format);
void acc_blit_rect(uint32_t src, uint32_t dest, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t src_pitch, uint16_t dest_pitch, uint8_t draw_mode, uint8_t mask_color);
void acc_blit_rect_16to8(uint32_t src, uint32_t dest, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t src_pitch, uint16_t dest_pitch);

void acc_draw_line(uint32_t dest, uint16_t pitch, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color, uint8_t bpp, uint8_t pen_width, uint8_t pen_height);
void acc_fill_rect(uint32_t dest, uint16_t pitch, int16_t x, int16_t y, int16_t w, int16_t h, uint32_t fg_color, uint8_t bpp);
void acc_draw_circle(uint32_t dest, uint16_t pitch, int16_t x, int16_t y, int16_t r, int16_t w, int16_t h, uint32_t fg_color, uint8_t bpp);
void acc_fill_circle(uint32_t dest, uint16_t pitch, int16_t x0, int16_t y0, int16_t r, int16_t w, int16_t h, uint32_t fg_color, uint8_t bpp);

void acc_fill_flat_tri(uint32_t dest, TriangleDef *d, uint16_t w, uint16_t h, uint32_t fg_color, uint8_t bpp);

void *get_color_conversion_table(int index);

enum color_formats {
	MNTVA_COLOR_8BIT,
	MNTVA_COLOR_16BIT565,
	MNTVA_COLOR_32BIT,
//	MNTVA_COLOR_1BIT,
	MNTVA_COLOR_15BIT,
	MNTVA_COLOR_NUM,
};

#define SWAP16(a) a = __builtin_bswap16(a)
#define SWAP32(a) a = __builtin_bswap32(a)

#define swap32(a) __builtin_bswap32(a)
#define swap16(a) __builtin_bswap16(a)

// see http://amigadev.elowar.com/read/ADCD_2.1/Libraries_Manual_guide/node0351.html
#define JAM1	    0	      /* jam 1 color into raster */
#define JAM2	    1	      /* jam 2 colors into raster */
#define COMPLEMENT  2	      /* XOR bits into raster */
#define INVERSVID   4	      /* inverse video for drawing modes */

// Graphics minterm drawing modes
enum gfx_minterm_modes {
	MINTERM_FALSE,
	MINTERM_NOR,
	MINTERM_ONLYDST,
	MINTERM_NOTSRC,
	MINTERM_ONLYSRC,
	MINTERM_INVERT,
	MINTERM_EOR,
	MINTERM_NAND,
	MINTERM_AND,
	MINTERM_NEOR,
	MINTERM_DST,
	MINTERM_NOTONLYSRC,
	MINTERM_SRC,
	MINTERM_NOTONLYDST,
	MINTERM_OR,
	MINTERM_TRUE,
};

/* Macros for keeping gfx.c a bit more tidy */
#define SET_FG_PIXEL8(a) \
	((uint8_t *)dp)[x+a] = u8_fg;
#define SET_FG_PIXEL16(a) \
	((uint16_t *)dp)[x+a] = fg_color;
#define SET_FG_PIXEL32(a) \
	dp[x+a] = fg_color;

#define SET_BG_PIXEL8(a) \
	((uint8_t *)dp)[x+a] = u8_bg;
#define SET_BG_PIXEL16(a) \
	((uint16_t *)dp)[x+a] = bg_color;
#define SET_BG_PIXEL32(a) \
	dp[x+a] = bg_color;

#define SET_FG_PIXEL8_MASK(a) \
	((uint8_t *)dp)[x + a] = u8_fg ^ (((uint8_t *)dp)[x + a] & (mask ^ 0xFF));
#define SET_BG_PIXEL8_MASK(a) \
	((uint8_t *)dp)[x + a] = u8_bg ^ (((uint8_t *)dp)[x + a] & (mask ^ 0xFF));
#define SET_FG_PIXEL16_MASK(a) \
	((uint16_t *)dp)[x + a] = fg_color ^ (((uint16_t *)dp)[x + a] & (color_mask ^ 0xFFFF));
#define SET_FG_PIXEL32_MASK(a) \
	dp[x + a] = fg_color ^ (dp[x + a] & (color_mask ^ 0xFFFFFFFF));

#define SET_FG_PIXEL \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			SET_FG_PIXEL8(0); break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			SET_FG_PIXEL16(0); break; \
		case MNTVA_COLOR_32BIT: \
			SET_FG_PIXEL32(0); break; \
	}

#define SET_FG_PIXEL_MASK \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			SET_FG_PIXEL8_MASK(0); break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			SET_FG_PIXEL16(0); break; \
		case MNTVA_COLOR_32BIT: \
			SET_FG_PIXEL32(0); break; \
	}

#define SET_BG_PIXEL \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			SET_BG_PIXEL8(0); break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			SET_BG_PIXEL16(0); break; \
		case MNTVA_COLOR_32BIT: \
			SET_BG_PIXEL32(0); break; \
	}

#define SET_BG_PIXEL_MASK \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			SET_BG_PIXEL8_MASK(0); break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			SET_BG_PIXEL16(0); break; \
		case MNTVA_COLOR_32BIT: \
			SET_BG_PIXEL32(0); break; \
	}

#define SET_FG_PIXELS \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL8(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL8(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL8(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL8(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL8(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL8(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL8(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL8(7); \
			break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL16(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL16(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL16(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL16(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL16(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL16(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL16(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL16(7); \
			break; \
		case MNTVA_COLOR_32BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL32(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL32(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL32(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL32(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL32(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL32(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL32(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL32(7); \
			break; \
	}

#define SET_FG_PIXELS_MASK \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL8_MASK(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL8_MASK(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL8_MASK(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL8_MASK(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL8_MASK(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL8_MASK(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL8_MASK(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL8_MASK(7); \
			break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL16(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL16(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL16(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL16(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL16(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL16(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL16(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL16(7); \
			break; \
		case MNTVA_COLOR_32BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL32(0); \
			if (cur_byte & 0x40) SET_FG_PIXEL32(1); \
			if (cur_byte & 0x20) SET_FG_PIXEL32(2); \
			if (cur_byte & 0x10) SET_FG_PIXEL32(3); \
			if (cur_byte & 0x08) SET_FG_PIXEL32(4); \
			if (cur_byte & 0x04) SET_FG_PIXEL32(5); \
			if (cur_byte & 0x02) SET_FG_PIXEL32(6); \
			if (cur_byte & 0x01) SET_FG_PIXEL32(7); \
			break; \
	}

#define SET_FG_OR_BG_PIXELS \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL8(0) else SET_BG_PIXEL8(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL8(1) else SET_BG_PIXEL8(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL8(2) else SET_BG_PIXEL8(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL8(3) else SET_BG_PIXEL8(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL8(4) else SET_BG_PIXEL8(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL8(5) else SET_BG_PIXEL8(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL8(6) else SET_BG_PIXEL8(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL8(7) else SET_BG_PIXEL8(7) \
			break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL16(0) else SET_BG_PIXEL16(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL16(1) else SET_BG_PIXEL16(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL16(2) else SET_BG_PIXEL16(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL16(3) else SET_BG_PIXEL16(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL16(4) else SET_BG_PIXEL16(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL16(5) else SET_BG_PIXEL16(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL16(6) else SET_BG_PIXEL16(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL16(7) else SET_BG_PIXEL16(7) \
			break; \
		case MNTVA_COLOR_32BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL32(0) else SET_BG_PIXEL32(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL32(1) else SET_BG_PIXEL32(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL32(2) else SET_BG_PIXEL32(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL32(3) else SET_BG_PIXEL32(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL32(4) else SET_BG_PIXEL32(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL32(5) else SET_BG_PIXEL32(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL32(6) else SET_BG_PIXEL32(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL32(7) else SET_BG_PIXEL32(7) \
			break; \
	}

#define SET_FG_OR_BG_PIXELS_MASK \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL8_MASK(0) else SET_BG_PIXEL8_MASK(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL8_MASK(1) else SET_BG_PIXEL8_MASK(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL8_MASK(2) else SET_BG_PIXEL8_MASK(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL8_MASK(3) else SET_BG_PIXEL8_MASK(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL8_MASK(4) else SET_BG_PIXEL8_MASK(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL8_MASK(5) else SET_BG_PIXEL8_MASK(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL8_MASK(6) else SET_BG_PIXEL8_MASK(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL8_MASK(7) else SET_BG_PIXEL8_MASK(7) \
			break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL16(0) else SET_BG_PIXEL16(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL16(1) else SET_BG_PIXEL16(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL16(2) else SET_BG_PIXEL16(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL16(3) else SET_BG_PIXEL16(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL16(4) else SET_BG_PIXEL16(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL16(5) else SET_BG_PIXEL16(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL16(6) else SET_BG_PIXEL16(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL16(7) else SET_BG_PIXEL16(7) \
			break; \
		case MNTVA_COLOR_32BIT: \
			if (cur_byte & 0x80) SET_FG_PIXEL32(0) else SET_BG_PIXEL32(0) \
			if (cur_byte & 0x40) SET_FG_PIXEL32(1) else SET_BG_PIXEL32(1) \
			if (cur_byte & 0x20) SET_FG_PIXEL32(2) else SET_BG_PIXEL32(2) \
			if (cur_byte & 0x10) SET_FG_PIXEL32(3) else SET_BG_PIXEL32(3) \
			if (cur_byte & 0x08) SET_FG_PIXEL32(4) else SET_BG_PIXEL32(4) \
			if (cur_byte & 0x04) SET_FG_PIXEL32(5) else SET_BG_PIXEL32(5) \
			if (cur_byte & 0x02) SET_FG_PIXEL32(6) else SET_BG_PIXEL32(6) \
			if (cur_byte & 0x01) SET_FG_PIXEL32(7) else SET_BG_PIXEL32(7) \
			break; \
	}

#define INVERT_PIXEL \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			((uint8_t *)dp)[x] ^= mask; break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			((uint16_t *)dp)[x] ^= 0xFFFF; break; \
		case MNTVA_COLOR_32BIT: \
			dp[x] ^= 0xFFFFFFFF; break; \
	}

#define INVERT_PIXEL_FG \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			((uint8_t *)dp)[x] = u8_fg ^ 0xFF; break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			((uint16_t *)dp)[x] ^= fg_color; break; \
		case MNTVA_COLOR_32BIT: \
			dp[x] ^= fg_color; break; \
	}

#define INVERT_PIXEL_BG \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			((uint8_t *)dp)[x] ^= u8_bg; break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			((uint16_t *)dp)[x] ^= bg_color; break; \
		case MNTVA_COLOR_32BIT: \
			dp[x] ^= bg_color; break; \
	}

#define INVERT_PIXELS \
	switch (color_format) { \
		case MNTVA_COLOR_8BIT: \
			if (cur_byte & 0x80) ((uint8_t *)dp)[x] ^= mask; \
			if (cur_byte & 0x40) ((uint8_t *)dp)[x+1] ^= mask; \
			if (cur_byte & 0x20) ((uint8_t *)dp)[x+2] ^= mask; \
			if (cur_byte & 0x10) ((uint8_t *)dp)[x+3] ^= mask; \
			if (cur_byte & 0x08) ((uint8_t *)dp)[x+4] ^= mask; \
			if (cur_byte & 0x04) ((uint8_t *)dp)[x+5] ^= mask; \
			if (cur_byte & 0x02) ((uint8_t *)dp)[x+6] ^= mask; \
			if (cur_byte & 0x01) ((uint8_t *)dp)[x+7] ^= mask; \
			break; \
		case MNTVA_COLOR_16BIT565: \
		case MNTVA_COLOR_15BIT: \
			if (cur_byte & 0x80) ((uint16_t *)dp)[x] ^= 0xFFFF; \
			if (cur_byte & 0x40) ((uint16_t *)dp)[x+1] ^= 0xFFFF; \
			if (cur_byte & 0x20) ((uint16_t *)dp)[x+2] ^= 0xFFFF; \
			if (cur_byte & 0x10) ((uint16_t *)dp)[x+3] ^= 0xFFFF; \
			if (cur_byte & 0x08) ((uint16_t *)dp)[x+4] ^= 0xFFFF; \
			if (cur_byte & 0x04) ((uint16_t *)dp)[x+5] ^= 0xFFFF; \
			if (cur_byte & 0x02) ((uint16_t *)dp)[x+6] ^= 0xFFFF; \
			if (cur_byte & 0x01) ((uint16_t *)dp)[x+7] ^= 0xFFFF; \
			break; \
		case MNTVA_COLOR_32BIT: \
			if (cur_byte & 0x80) dp[x] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x40) dp[x+1] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x20) dp[x+2] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x10) dp[x+3] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x08) dp[x+4] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x04) dp[x+5] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x02) dp[x+6] ^= 0xFFFFFFFF; \
			if (cur_byte & 0x01) dp[x+7] ^= 0xFFFFFFFF; \
			break; \
	}

#define HANDLE_MINTERM_PIXEL_8(s, d) \
	switch(draw_mode) {\
		case MINTERM_NOR: \
			s &= ~(d); \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_ONLYDST: \
			d = d & ~(s); break; \
		case MINTERM_NOTSRC: \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_ONLYSRC: \
			s &= (d ^ 0xFF); \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_INVERT: \
			d ^= 0xFF; break; \
		case MINTERM_EOR: \
			d ^= s; break; \
		case MINTERM_NAND: \
			s = ~(d & ~(s)) & mask; \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_AND: \
			s &= d; \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_NEOR: \
			d ^= ~(s & mask); break; \
		case MINTERM_DST: /* This one does nothing. */ \
			return; break; \
		case MINTERM_NOTONLYSRC: \
			d |= (s & mask); break; \
		case MINTERM_SRC: \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_NOTONLYDST: \
			s = ~(d & s) & mask; \
			SET_FG_PIXEL8_MASK(0); break; \
		case MINTERM_OR: \
			d |= (s & mask); break; \
	}

#define HANDLE_MINTERM_PIXEL_16(s, d) \
	switch (draw_mode) { \
		case MINTERM_NOR: \
			s &= ~(d); \
			SET_FG_PIXEL16_MASK(0); break; \
		case MINTERM_ONLYDST: \
			d = d & ~(s); break; \
		case MINTERM_ONLYSRC: \
			s &= (d ^ 0xFFFF); \
			SET_FG_PIXEL16_MASK(0); break; \
		case MINTERM_INVERT: \
			d ^= 0xFFFF; break; \
		case MINTERM_EOR: \
			d ^= s; break; \
		case MINTERM_NAND: \
			s = ~(d | ~(s)) & color_mask; \
			SET_FG_PIXEL16_MASK(0); break; \
		case MINTERM_AND: \
			s &= d; \
			SET_FG_PIXEL16_MASK(0); break; \
		case MINTERM_NEOR: \
			d ^= ~(s & color_mask); break; \
		case MINTERM_DST: /* This one does nothing. */ \
			return; break; \
		case MINTERM_NOTONLYSRC: \
			d |= (s & color_mask); break; \
		case MINTERM_NOTSRC: \
		case MINTERM_SRC: \
			SET_FG_PIXEL16_MASK(0); break; \
		case MINTERM_NOTONLYDST: \
			d = ~(d & s) & color_mask; \
			SET_FG_PIXEL16_MASK(0); break; \
		case MINTERM_OR: \
			d |= (s & color_mask); break; \
	}

#define HANDLE_MINTERM_PIXEL_32(s, d) \
	switch (draw_mode) { \
		case MINTERM_NOR: \
			s &= ~(d); \
			SET_FG_PIXEL32_MASK(0); break; \
		case MINTERM_ONLYDST: \
			d = d & ~(s); break; \
		case MINTERM_ONLYSRC: \
			s &= (d ^ 0x00FFFFFF); \
			SET_FG_PIXEL32_MASK(0); break; \
		case MINTERM_INVERT: \
			d ^= 0x00FFFFFF; break; \
		case MINTERM_EOR: \
			d ^= s; break; \
		case MINTERM_NAND: \
			s = ~(d | ~(s)) & color_mask; \
			SET_FG_PIXEL32_MASK(0); break; \
		case MINTERM_AND: \
			s &= d; \
			SET_FG_PIXEL32_MASK(0); break; \
		case MINTERM_NEOR: \
			d ^= ~(s & color_mask);	break; \
		case MINTERM_DST: /* This one does nothing. */ \
			return; break; \
		case MINTERM_NOTONLYSRC: \
			d |= (s & color_mask); break; \
		case MINTERM_NOTSRC: \
		case MINTERM_SRC: \
			SET_FG_PIXEL32_MASK(0); break; \
		case MINTERM_NOTONLYDST: \
			d = ~(d & s) & color_mask; \
			SET_FG_PIXEL32_MASK(0); break; \
		case MINTERM_OR: \
			d |= (s & color_mask); \
			break; \
	}

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
  ACC_OP_DRAW_CIRCLE,
  ACC_OP_FILL_CIRCLE,
  ACC_OP_DRAW_FLAT_TRI,
  ACC_OP_DRAW_TEX_TRI,
  ACC_OP_DECOMPRESS,
  ACC_OP_COMPRESS,
  ACC_OP_CODEC_OP,
  ACC_OP_NUM,
};

enum compression_types {
  ACC_CMPTYPE_SMUSH_CODEC1,
  ACC_CMPTYPE_SMUSH_CODEC37,
  ACC_CMPTYPE_SMUSH_CODEC47,
  ACC_CMPTYPE_IMA_ADPCM_VBR,
  ACC_CMPTYPE_NUM,
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

#endif
