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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gfx.h"
#include "../main.h"


uint32_t* fb=0;
uint32_t fb_pitch=0;
static void *(memcpy_rom1)(void * s1, const void * s2, u32 n)
{
	char *dst = (char *)s1;
	const char *src = (char *)s2;

	/*
	 * Loop and copy
	 */
	while (n-- != 0)
		*dst++ = *src++;
	return s1;
}
static void *(memmove_rom1)(void * s1, const void * s2, u32 n)
{
	char *dst = (char *)s1+n;
	const char *src = (char *)s2+n;

	/*
	 * Loop and copy
	 */
	while (n-- != 0)
		*dst-- = *src--;
	return s1;
}

void set_fb(uint32_t* fb_, uint32_t pitch) {
	fb=fb_;
	fb_pitch=pitch;
//	if(((uint32_t)fb)>=0x08000000U-0x00200000U)
//		printf("set_fb 0x%08lX\n\r",(uint32_t)fb);
}



uint8_t color_map_16_to_8[65536];

void *get_color_conversion_table(int index)
{
	switch (index) {
		case 0:
			return ((void *)color_map_16_to_8);
		default:
			return (0);
	}
}

void fill_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint32_t fg_color, uint32_t color_format, uint8_t mask)
{
	uint32_t* dp = fb + (rect_y1 * fb_pitch);
	uint8_t u8_fg = fg_color >> 24;
	uint16_t rect_y2 = rect_y1 + h, rect_x2 = rect_x1 + w;
	uint16_t x;

	for (uint16_t cur_y = rect_y1; cur_y < rect_y2; cur_y++) {
		x = rect_x1;
		switch(color_format) {
			case MNTVA_COLOR_8BIT:
				while(x < rect_x2) {
					SET_FG_PIXEL8_MASK(0);
					x++;
				}
				break;
			case MNTVA_COLOR_32BIT:
			case MNTVA_COLOR_16BIT565:
			case MNTVA_COLOR_15BIT:
				while(x < rect_x2) {
					// The mask isn't used at all for 16/32-bit
					SET_FG_PIXEL;
					x++;
				}
				break;
			default:
				// Unknown/unhandled color format.
				printf("fillrect Unknown/unhandled color format.\n");
				break;
		}
		dp += fb_pitch;
	}
}

void fill_rect_solid(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint32_t rect_rgb, uint32_t color_format)
{
	uint32_t* p = fb + (rect_y1 * fb_pitch);
	uint16_t* p16;
	uint16_t rect_y2 = rect_y1 + h, rect_x2 = rect_x1 + w;
	uint16_t x;

	for (uint16_t cur_y = rect_y1; cur_y < rect_y2; cur_y++) {
		switch(color_format) {
			case MNTVA_COLOR_8BIT:
				memset((uint8_t *)p + rect_x1, (uint8_t)(rect_rgb >> 24), w);
				break;
			case MNTVA_COLOR_16BIT565:
			case MNTVA_COLOR_15BIT:
				x = rect_x1;
				p16 = (uint16_t *)p;
				while(x < rect_x2) {
					p16[x++] = rect_rgb;
				}
				break;
			case MNTVA_COLOR_32BIT:
				x = rect_x1;
				while(x < rect_x2) {
					p[x++] = rect_rgb;
				}
				break;
			default:
				// Unknown/unhandled color format.
				printf("fillrectsolid Unknown/unhandled color format.\n");
				break;
		}
		p += fb_pitch;
	}
}

void invert_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint8_t mask, uint32_t color_format)
{
	uint32_t* dp = fb + (rect_y1 * fb_pitch);
	uint16_t x;

	uint16_t rect_y2 = rect_y1 + h, rect_x2 = rect_x1 + w;

	for (uint16_t cur_y = rect_y1; cur_y < rect_y2; cur_y++) {
		x = rect_x1;
		while (x < rect_x2) {
			INVERT_PIXEL;
			x++;
		}
		dp += fb_pitch;
	}
}

void copy_rect_nomask(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint16_t rect_sx, uint16_t rect_sy, uint32_t color_format, uint32_t* sp_src, uint32_t src_pitch, uint8_t draw_mode)
{
	uint32_t* dp = fb + (rect_y1 * fb_pitch);
	uint32_t* sp = sp_src + (rect_sy * src_pitch);
	uint16_t rect_y2 = rect_y1 + h - 1;
	uint8_t mask = 0xFF; // Perform mask handling, just in case we get a FillRectComplete at some point.
	uint32_t color_mask = 0x00FFFFFF;

	uint8_t u8_fg = 0;
	uint32_t fg_color = 0;

	int32_t line_step_d = fb_pitch, line_step_s = src_pitch;
	int8_t x_reverse = 0;

	if (rect_sy < rect_y1) {
		line_step_d = -fb_pitch;
		dp = fb + (rect_y2 * fb_pitch);
		line_step_s = -src_pitch;
		sp = sp_src + ((rect_sy + h - 1) * src_pitch);
	}

	if (rect_sx < rect_x1) {
		x_reverse = 1;
	}

	if (draw_mode == MINTERM_SRC) {
		switch(color_format) {
		case MNTVA_COLOR_8BIT:
			if (!x_reverse)
			{
				for (uint16_t y_line = 0; y_line < h; y_line++,dp += line_step_d,sp += line_step_s)
					memcpy((uint8_t *)dp + rect_x1, (uint8_t *)sp + rect_sx, w);
			}
			else
			{
				for (uint16_t y_line = 0; y_line < h; y_line++,dp += line_step_d,sp += line_step_s)
					memmove((uint8_t *)dp + rect_x1, (uint8_t *)sp + rect_sx, w);
			}
			break;

		case MNTVA_COLOR_16BIT565:
		case MNTVA_COLOR_15BIT:
			if (!x_reverse)
			{
				for (uint16_t y_line = 0; y_line < h; y_line++,dp += line_step_d,sp += line_step_s)
					memcpy((uint16_t *)dp + rect_x1, (uint16_t *)sp + rect_sx, w * 2);
			}
			else
			{
				for (uint16_t y_line = 0; y_line < h; y_line++,dp += line_step_d,sp += line_step_s)
					memmove((uint16_t *)dp + rect_x1, (uint16_t *)sp + rect_sx, w * 2);
			}
			break;
		case MNTVA_COLOR_32BIT:
			if (!x_reverse)
			{
				for (uint16_t y_line = 0; y_line < h; y_line++,dp += line_step_d,sp += line_step_s)
					memcpy(dp + rect_x1, sp + rect_sx, w * 4);
			}
			else
			{
				for (uint16_t y_line = 0; y_line < h; y_line++,dp += line_step_d,sp += line_step_s)
					memmove(dp + rect_x1, sp + rect_sx, w * 4);
			}
			break;
		}
	}
	else {
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			if (x_reverse) {
				for (int16_t x = w-1; x >= 0; x--) {
					if (color_format == MNTVA_COLOR_8BIT) {
						u8_fg = ((uint8_t *)sp)[rect_sx + x];
						HANDLE_MINTERM_PIXEL_8(u8_fg, ((uint8_t *)dp)[rect_x1 + x]);
					}
					else {
						if (color_format == MNTVA_COLOR_16BIT565 || color_format == MNTVA_COLOR_15BIT) {
							fg_color = ((uint16_t *)sp)[rect_sx + x];
							uint16_t* dpx1 = (uint16_t*)dp + rect_x1;
							HANDLE_MINTERM_PIXEL_16_32(fg_color, dpx1);
						}
						else {
							fg_color = sp[rect_sx + x];
							uint32_t* dpx1 = dp + rect_x1;
							HANDLE_MINTERM_PIXEL_16_32(fg_color, dpx1);
						}
					}
				}
			}
			else {
				for (int16_t x = 0; x < w; x++) {
					if (color_format == MNTVA_COLOR_8BIT) {
						u8_fg = ((uint8_t *)sp)[rect_sx + x];
						HANDLE_MINTERM_PIXEL_8(u8_fg, ((uint8_t *)dp)[rect_x1 + x]);
					}
					else {
						if (color_format == MNTVA_COLOR_16BIT565 || color_format == MNTVA_COLOR_15BIT) {
							fg_color = ((uint16_t *)sp)[rect_sx + x];
							uint16_t* dpx1 = (uint16_t*)dp + rect_x1;
							HANDLE_MINTERM_PIXEL_16_32(fg_color, dpx1);
						}
						else {
							fg_color = sp[rect_sx + x];
							uint32_t* dpx1 = dp + rect_x1;
							HANDLE_MINTERM_PIXEL_16_32(fg_color, dpx1);
						}
					}
				}
			}
			dp += line_step_d;
			sp += line_step_s;
		}
	}
}

void copy_rect(uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h, uint16_t rect_sx, uint16_t rect_sy, uint32_t color_format, uint32_t* sp_src, uint32_t src_pitch, uint8_t mask)
{
	uint32_t* dp = fb + (rect_y1 * fb_pitch);
	uint32_t* sp = sp_src + (rect_sy * src_pitch);
	uint16_t rect_y2 = rect_y1 + h - 1;//, rect_x2 = rect_x1 + h - 1;

	int32_t line_step_d = fb_pitch, line_step_s = src_pitch;
	int8_t x_reverse = 0;

	if (rect_sy < rect_y1) {
		line_step_d = -fb_pitch;
		dp = fb + (rect_y2 * fb_pitch);
		line_step_s = -src_pitch;
		sp = sp_src + ((rect_sy + h - 1) * src_pitch);
	}

	if (rect_sx < rect_x1) {
		x_reverse = 1;
	}

	for (uint16_t y_line = 0; y_line < h; y_line++) {
		if (x_reverse) {
			for (int16_t x = w; x >= 0; x--) {
				((uint8_t *)dp)[rect_x1 + x] = (((uint8_t *)dp)[rect_x1 + x] & (mask ^ 0xFF)) | (((uint8_t *)sp)[rect_sx + x] & mask);
			}
		}
		else {
			for (int16_t x = 0; x < w; x++) {
				((uint8_t *)dp)[rect_x1 + x] = (((uint8_t *)dp)[rect_x1 + x] & (mask ^ 0xFF)) | (((uint8_t *)sp)[rect_sx + x] & mask);
			}
		}
		dp += line_step_d;
		sp += line_step_s;
	}
}

#define DRAW_LINE_PIXEL \
	if (draw_mode == JAM1) { \
		if(pattern & cur_bit) { \
			if (!inversion) { \
				if (mask == 0xFF || color_format == MNTVA_COLOR_16BIT565 || color_format == MNTVA_COLOR_15BIT || color_format == MNTVA_COLOR_32BIT) { SET_FG_PIXEL; } \
				else { SET_FG_PIXEL8_MASK(0) } \
			} \
			else { INVERT_PIXEL; } \
		} \
	} \
	else { \
		if(pattern & cur_bit) { \
			if (!inversion) { \
				if (mask == 0xFF || color_format == MNTVA_COLOR_16BIT565 || color_format == MNTVA_COLOR_15BIT || color_format == MNTVA_COLOR_32BIT) { SET_FG_PIXEL; } \
				else { SET_FG_PIXEL8_MASK(0); } \
			} \
			else { INVERT_PIXEL; } /* JAM2 and complement is kind of useless, as it ends up being the same visual result as JAM1 and a pattern of 0xFFFF */ \
		} \
		else { \
			if (!inversion) { \
				if (mask == 0xFF || color_format == MNTVA_COLOR_16BIT565 || color_format == MNTVA_COLOR_15BIT || color_format == MNTVA_COLOR_32BIT) { SET_BG_PIXEL; } \
				else { SET_BG_PIXEL8_MASK(0); } \
			} \
			else { INVERT_PIXEL; } \
		} \
	} \
	if ((cur_bit >>= 1) == 0) \
		cur_bit = 0x8000; \

// Sneakily adapted version of the good old Bresenham algorithm
void draw_line(int16_t rect_x1, int16_t rect_y1, int16_t rect_x2, int16_t rect_y2, uint16_t len,
	uint16_t pattern, uint16_t pattern_offset,
	uint32_t fg_color, uint32_t bg_color, uint32_t color_format,
	uint8_t mask, uint8_t draw_mode)
{
	int16_t x1 = rect_x1, y1 = rect_y1;
	int16_t x2 = rect_x1 + rect_x2, y2 = rect_y1 + rect_y2;

	uint8_t u8_fg = fg_color >> 24;
	uint8_t u8_bg = bg_color >> 24;

	uint32_t* dp = fb + (y1 * fb_pitch);
	int32_t line_step = fb_pitch;
	int8_t x_reverse = 0, inversion = 0;

	uint16_t cur_bit = 0x8000;

	int16_t dx, dy, dx_abs, dy_abs, ix, iy, x = x1;

	if (x2 < x1)
		x_reverse = 1;
	if (y2 < y1)
		line_step = -fb_pitch;

	if (draw_mode & INVERSVID)
		pattern ^= 0xFFFF;
	if (draw_mode & COMPLEMENT) {
		inversion = 1;
		fg_color = 0xFFFF0000;
	}
	draw_mode &= 0x01;

	dx = x2 - x1;
	dy = y2 - y1;
	dx_abs = abs(dx);
	dy_abs = abs(dy);
	ix = dy_abs >> 1;
	iy = dx_abs >> 1;

	// This can't be used for now, as Flags from the current RastPort struct is not exposed by [ P96 2.4.2 ]
	/*if ((pattern_offset >> 8) & 0x01) { // Is FRST_DOT set?
		cur_bit = 0x8000;
		fg_color = 0xFFFF0000;
	}
	else {
		fg_color = 0xFF00FF00;
		cur_bit >>= ((pattern_offset & 0xFF) % 16);
	}

	if (cur_bit == 0)
		cur_bit = 0x8000;*/


	DRAW_LINE_PIXEL;

	if (dx_abs >= dy_abs) {
		if (!len) len = dx_abs;
		for (uint16_t i = 0; i < len; i++) {
			iy += dy_abs;
			if (iy >= dx_abs) {
				iy -= dx_abs;
				dp += line_step;
			}
			x += (x_reverse) ? -1 : 1;

			DRAW_LINE_PIXEL;
		}
	}
	else {
		if (!len) len = dy_abs;
		for(uint16_t i = 0; i < len; i++) {
			ix += dx_abs;
			if (ix >= dy_abs) {
				ix -= dy_abs;
				x += (x_reverse) ? -1 : 1;
			}
			dp += line_step;

			DRAW_LINE_PIXEL;
		}
	}
}

void draw_line_solid(int16_t rect_x1, int16_t rect_y1, int16_t rect_x2, int16_t rect_y2, uint16_t len,
	uint32_t fg_color, uint32_t color_format)
{
	int16_t x1 = rect_x1, y1 = rect_y1;
	int16_t x2 = rect_x1 + rect_x2, y2 = rect_y1 + rect_y2;

	uint8_t u8_fg = fg_color >> 24;

	uint32_t* dp = fb + (y1 * fb_pitch);
	int32_t line_step = fb_pitch;
	int8_t x_reverse = 0;

	int16_t dx, dy, dx_abs, dy_abs, ix, iy, x = x1;

	if (x2 < x1)
		x_reverse = 1;
	if (y2 < y1)
		line_step = -fb_pitch;

	dx = x2 - x1;
	dy = y2 - y1;
	dx_abs = abs(dx);
	dy_abs = abs(dy);
	ix = dy_abs >> 1;
	iy = dx_abs >> 1;

	SET_FG_PIXEL;

	if (dx_abs >= dy_abs) {
		if (!len) len = dx_abs;
		for (uint16_t i = 0; i < len; i++) {
			iy += dy_abs;
			if (iy >= dx_abs) {
				iy -= dx_abs;
				dp += line_step;
			}
			x += (x_reverse) ? -1 : 1;

			SET_FG_PIXEL;
		}
	}
	else {
		if (!len) len = dy_abs;
		for (uint16_t i = 0; i < len; i++) {
			ix += dx_abs;
			if (ix >= dy_abs) {
				ix -= dy_abs;
				x += (x_reverse) ? -1 : 1;
			}
			dp += line_step;

			SET_FG_PIXEL;
		}
	}
}

#define DECODE_PLANAR_PIXEL(a) \
	switch (planes) { \
		case 8: if ((layer_mask & 0x80) && (bmp_data[(plane_size * 7) + cur_byte] & cur_bit)) a |= 0x80; \
		case 7: if ((layer_mask & 0x40) && (bmp_data[(plane_size * 6) + cur_byte] & cur_bit)) a |= 0x40; \
		case 6: if ((layer_mask & 0x20) && (bmp_data[(plane_size * 5) + cur_byte] & cur_bit)) a |= 0x20; \
		case 5: if ((layer_mask & 0x10) && (bmp_data[(plane_size * 4) + cur_byte] & cur_bit)) a |= 0x10; \
		case 4: if ((layer_mask & 0x08) && (bmp_data[(plane_size * 3) + cur_byte] & cur_bit)) a |= 0x08; \
		case 3: if ((layer_mask & 0x04) && (bmp_data[(plane_size * 2) + cur_byte] & cur_bit)) a |= 0x04; \
		case 2: if ((layer_mask & 0x02) && (bmp_data[plane_size + cur_byte] & cur_bit)) a |= 0x02; \
		case 1: if ((layer_mask & 0x01) && (bmp_data[cur_byte] & cur_bit)) a |= 0x01; \
			break; \
	}

#define DECODE_INVERTED_PLANAR_PIXEL(a) \
	switch (planes) { \
		case 8: if ((layer_mask & 0x80) && ((bmp_data[(plane_size * 7) + cur_byte] ^ 0xFF) & cur_bit)) a |= 0x80; \
		case 7: if ((layer_mask & 0x40) && ((bmp_data[(plane_size * 6) + cur_byte] ^ 0xFF) & cur_bit)) a |= 0x40; \
		case 6: if ((layer_mask & 0x20) && ((bmp_data[(plane_size * 5) + cur_byte] ^ 0xFF) & cur_bit)) a |= 0x20; \
		case 5: if ((layer_mask & 0x10) && ((bmp_data[(plane_size * 4) + cur_byte] ^ 0xFF) & cur_bit)) a |= 0x10; \
		case 4: if ((layer_mask & 0x08) && ((bmp_data[(plane_size * 3) + cur_byte] ^ 0xFF) & cur_bit)) a |= 0x08; \
		case 3: if ((layer_mask & 0x04) && ((bmp_data[(plane_size * 2) + cur_byte] ^ 0xFF) & cur_bit)) a |= 0x04; \
		case 2: if ((layer_mask & 0x02) && ((bmp_data[plane_size + cur_byte] ^ 0xFF) & cur_bit)) a |= 0x02; \
		case 1: if ((layer_mask & 0x01) && ((bmp_data[cur_byte] ^ 0xFF) & cur_bit)) a |= 0x01; \
			break; \
	}

void p2c_rect(int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h, uint8_t draw_mode, uint8_t planes, uint8_t mask, uint8_t layer_mask, uint16_t src_line_pitch, uint8_t *bmp_data_src)
{
	uint32_t *dp = fb + (dy * fb_pitch);

	uint8_t cur_bit, base_bit, base_byte;
	uint16_t cur_byte = 0, u8_fg = 0;

	uint32_t plane_size = src_line_pitch * h;
	uint8_t *bmp_data = bmp_data_src;

	cur_bit = base_bit = (0x80 >> (sx % 8));
	cur_byte = base_byte = ((sx / 8) % src_line_pitch);

	for (int16_t line_y = 0; line_y < h; line_y++) {
		for (int16_t x = dx; x < dx + w; x++) {
			u8_fg = 0;
			if (draw_mode & 0x01) // If bit 1 is set, the inverted planar data is always used.
				DECODE_INVERTED_PLANAR_PIXEL(u8_fg)
			else
				DECODE_PLANAR_PIXEL(u8_fg)

			if (mask == 0xFF && (draw_mode == MINTERM_SRC || draw_mode == MINTERM_NOTSRC)) {
				((uint8_t *)dp)[x] = u8_fg;
				goto skip;
			}

			HANDLE_MINTERM_PIXEL_8(u8_fg, ((uint8_t *)dp)[x]);

			skip:;
			if ((cur_bit >>= 1) == 0) {
				cur_bit = 0x80;
				cur_byte++;
				cur_byte %= src_line_pitch;
			}

		}
		dp += fb_pitch;
		if ((line_y + sy + 1) % h)
			bmp_data += src_line_pitch;
		else
			bmp_data = bmp_data_src;
		cur_bit = base_bit;
		cur_byte = base_byte;
	}
}

uint8_t reverse_lookup(uint32_t *bmp_pal, uint8_t planes, uint32_t fg_color) {
	uint8_t num_colors = (1<<planes) - 1;

	for(uint8_t i=0; i<num_colors; i++) {
		if(bmp_pal[i] == fg_color) {
			return (i);
		}
	}
	return (0);
}

void p2d_rect(int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h, uint8_t draw_mode, uint8_t planes, uint8_t mask, uint8_t layer_mask, uint32_t color_mask, uint16_t src_line_pitch, uint8_t *bmp_data_src, uint32_t color_format) {
	uint32_t *dp = fb + (dy * fb_pitch);

	uint8_t cur_bit, base_bit, base_byte;
	uint16_t cur_byte = 0;

	uint32_t plane_size = src_line_pitch * h;
	uint32_t *bmp_pal = (uint32_t *)bmp_data_src;
	uint8_t *bmp_data = bmp_data_src + (256 * 4);

	cur_bit = base_bit = (0x80 >> (sx % 8));
	cur_byte = base_byte = ((sx / 8) % src_line_pitch);

	for (int16_t line_y = 0; line_y < h; line_y++) {
		for (int16_t x = dx; x < dx + w; x++) {

			uint8_t b=0,nb=0,c,d=0;
			switch(draw_mode) {
				case MINTERM_FALSE:
					d = 0;
				break;
				case MINTERM_NOR:
					DECODE_PLANAR_PIXEL(b);
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = ~(c | b);
				break;
				case MINTERM_ONLYDST:
					DECODE_INVERTED_PLANAR_PIXEL(nb);
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = c & nb;
				break;
				case MINTERM_NOTSRC:
					DECODE_INVERTED_PLANAR_PIXEL(nb);
					d = nb;
				break;
				case MINTERM_ONLYSRC:
					DECODE_PLANAR_PIXEL(b);
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = (~c) & b;
				break;
				case MINTERM_INVERT:
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = ~c;
				break;
				case MINTERM_EOR:
					DECODE_PLANAR_PIXEL(b);
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = c ^ b;
				break;
				case MINTERM_NAND:
					DECODE_PLANAR_PIXEL(b);
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = ~(c & b);
				break;
				case MINTERM_AND:
					DECODE_PLANAR_PIXEL(b);
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = c & b;
				break;
				case MINTERM_NEOR:
					DECODE_PLANAR_PIXEL(b);
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = ~(c ^ b);
				break;
				case MINTERM_DST:
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = c;
				break;
				case MINTERM_NOTONLYSRC:
					DECODE_INVERTED_PLANAR_PIXEL(nb);
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = c | nb;
				break;
				case MINTERM_SRC:
					DECODE_PLANAR_PIXEL(b);
					d = b;
				break;
				case MINTERM_NOTONLYDST:
					DECODE_PLANAR_PIXEL(b);
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = (~c) | b;
				break;
				case MINTERM_OR:
					DECODE_PLANAR_PIXEL(b);
					c = reverse_lookup(bmp_pal, planes, dp[x]);
					d = c | b;
				break;
				case MINTERM_TRUE:
					d = (1<<planes) - 1;
				break;
			}

			switch (color_format) {
				case MNTVA_COLOR_16BIT565:
				case MNTVA_COLOR_15BIT:
					((uint16_t *)dp)[x] = bmp_pal[d];
					break;
				case MNTVA_COLOR_32BIT:
					dp[x] = bmp_pal[d];
					break;
			}

			if ((cur_bit >>= 1) == 0) {
				cur_bit = 0x80;
				cur_byte++;
				cur_byte %= src_line_pitch;
			}

		}
		dp += fb_pitch;
		if ((line_y + sy + 1) % h)
			bmp_data += src_line_pitch;
		else
			bmp_data = bmp_data_src;
		cur_bit = base_bit;
		cur_byte = base_byte;
	}
}

void orig_p2d_rect(int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h, uint8_t draw_mode, uint8_t planes, uint8_t mask, uint8_t layer_mask, uint32_t color_mask, uint16_t src_line_pitch, uint8_t *bmp_data_src, uint32_t color_format) {
	uint32_t *dp = fb + (dy * fb_pitch);

	uint8_t cur_bit, base_bit, base_byte;
	uint16_t cur_byte = 0, cur_pixel = 0;
	uint32_t fg_color = 0;

	uint32_t plane_size = src_line_pitch * h;
	uint32_t *bmp_pal = (uint32_t *)bmp_data_src;
	uint8_t *bmp_data = bmp_data_src + (256 * 4);

	cur_bit = base_bit = (0x80 >> (sx % 8));
	cur_byte = base_byte = ((sx / 8) % src_line_pitch);

	for (int16_t line_y = 0; line_y < h; line_y++) {
		for (int16_t x = dx; x < dx + w; x++) {
			cur_pixel = 0;
			if (draw_mode & 0x01)
				DECODE_INVERTED_PLANAR_PIXEL(cur_pixel)
			else
				DECODE_PLANAR_PIXEL(cur_pixel)
			fg_color = bmp_pal[cur_pixel];

			if (mask == 0xFF && (draw_mode == 0x0C || draw_mode == 0x03)) {
				switch (color_format) {
					case MNTVA_COLOR_16BIT565:
					case MNTVA_COLOR_15BIT:
						((uint16_t *)dp)[x] = fg_color;
						break;
					case MNTVA_COLOR_32BIT:
						dp[x] = fg_color;
						break;
				}
				goto skip;
			}

			HANDLE_MINTERM_PIXEL_16_32(fg_color, dp);

			skip:;
			if ((cur_bit >>= 1) == 0) {
				cur_bit = 0x80;
				cur_byte++;
				cur_byte %= src_line_pitch;
			}

		}
		dp += fb_pitch;
		if ((line_y + sy + 1) % h)
			bmp_data += src_line_pitch;
		else
			bmp_data = bmp_data_src;
		cur_bit = base_bit;
		cur_byte = base_byte;
	}
}

#define PATTERN_FILLRECT_LOOPX \
	tmpl_x ^= 0x01; \
	cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

#define PATTERN_FILLRECT_LOOPY \
	tmpl_data += 2 ; \
	if ((y_line + y_offset + 1) % loop_rows == 0) \
		tmpl_data = tmpl_base; \
	tmpl_x = tmpl_x_base; \
	cur_bit = base_bit; \
	dp += fb_pitch / 4;

void pattern_fill_rect(uint32_t color_format, uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h,
	uint8_t draw_mode, uint8_t mask, uint32_t fg_color, uint32_t bg_color,
	uint16_t x_offset, uint16_t y_offset,
	uint8_t *tmpl_data, uint16_t tmpl_pitch, uint16_t loop_rows)
{
	uint32_t rect_x2 = rect_x1 + w;
	uint32_t *dp = fb + (rect_y1 * (fb_pitch / 4));
	uint8_t* tmpl_base = tmpl_data;

	uint16_t tmpl_x, tmpl_x_base;

	uint8_t cur_bit, base_bit, inversion = 0;
	uint8_t u8_fg = fg_color >> 24;
	uint8_t u8_bg = bg_color >> 24;
	uint8_t cur_byte = 0;

	uint8_t cur_line = 0;
	uint16_t cheat_y = 0;

	tmpl_x = (x_offset / 8) % 2;
	tmpl_data += (y_offset % loop_rows) * 2;
	tmpl_x_base = tmpl_x;

	cur_bit = base_bit = (0x80 >> (x_offset % 8));

	if (draw_mode & INVERSVID) inversion = 1;
	draw_mode &= 0x03;

	if (draw_mode == JAM1) {
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			uint16_t x = rect_x1;

			cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

			while (x < rect_x2) {
				if (w >= 8 && cur_bit == 0x80 && x < rect_x2 - 8) {
					if (mask == 0xFF) {
						SET_FG_PIXELS;
					}
					else {
						SET_FG_PIXELS_MASK;
					}
					x += 8;
				}
				else {
					while (cur_bit > 0 && x < rect_x2) {
						if (cur_byte & cur_bit) {
							SET_FG_PIXEL_MASK;
						}
						x++;
						cur_bit >>= 1;
					}
					cur_bit = 0x80;
				}
				PATTERN_FILLRECT_LOOPX;
			}
			PATTERN_FILLRECT_LOOPY;
		}

		return;
	}
	else if (draw_mode == JAM2) {
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			uint16_t x = rect_x1;

			cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

			while (x < rect_x2) {
				if (w >= 8 && cur_bit == 0x80 && x < rect_x2 - 8) {
					if (mask == 0xFF) {
						SET_FG_OR_BG_PIXELS;
					}
					else {
						SET_FG_OR_BG_PIXELS_MASK;
					}
					x += 8;
				}
				else {
					while (cur_bit > 0 && x < rect_x2) {
						if (cur_byte & cur_bit) {
							SET_FG_PIXEL_MASK;
						}
						else {
							SET_BG_PIXEL_MASK;
						}
						x++;
						cur_bit >>= 1;
					}
					cur_bit = 0x80;
				}
				PATTERN_FILLRECT_LOOPX;
			}
			if (mask == 0xFF && loop_rows <= 64) {
				cur_line++;
				if (cur_line == loop_rows) {
					cheat_y = y_line + 1;
					goto engage_cheat_codes;
				}
			}
			PATTERN_FILLRECT_LOOPY;
		}

		return;

engage_cheat_codes:;
		dp += (fb_pitch / 4);
		uint32_t *sp = dp - (cur_line * (fb_pitch / 4));
		for (uint16_t y_line = cheat_y; y_line < h; y_line++) {
			switch (color_format) {
				case MNTVA_COLOR_8BIT:
					memcpy(&((uint8_t *)dp)[rect_x1], &((uint8_t *)sp)[rect_x1], w);
					break;
				case MNTVA_COLOR_16BIT565:
				case MNTVA_COLOR_15BIT:
					memcpy(&((uint16_t *)dp)[rect_x1], &((uint16_t *)sp)[rect_x1], w * 2);
					break;
				case MNTVA_COLOR_32BIT:
					memcpy(&dp[rect_x1], &sp[rect_x1], w * 4);
					break;
			}
			dp += fb_pitch / 4;
			sp += fb_pitch / 4;
		}
		return;
	}
	else { // COMPLEMENT
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			uint16_t x = rect_x1;

			cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

			while (x < rect_x2) {
				if (w >= 8 && cur_bit == 0x80 && x < rect_x2 - 8) {
					INVERT_PIXELS;
					x += 8;
				}
				else {
					while (cur_bit > 0 && x < rect_x2) {
						if (cur_byte & cur_bit) {
							INVERT_PIXEL;
						}
						x++;
						cur_bit >>= 1;
					}
					cur_bit = 0x80;
				}
				PATTERN_FILLRECT_LOOPX;
			}
			PATTERN_FILLRECT_LOOPY;
		}
	}
}

#define TEMPLATE_FILLRECT_LOOPX \
	tmpl_x++; \
	cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

#define TEMPLATE_FILLRECT_LOOPY \
	tmpl_data += tmpl_pitch; \
	tmpl_x = tmpl_x_base; \
	cur_bit = base_bit; \
	dp += fb_pitch / 4;

void template_fill_rect(uint32_t color_format, uint16_t rect_x1, uint16_t rect_y1, uint16_t w, uint16_t h,
	uint8_t draw_mode, uint8_t mask, uint32_t fg_color, uint32_t bg_color,
	uint16_t x_offset, uint16_t y_offset,
	uint8_t *tmpl_data, uint16_t tmpl_pitch)
{
	uint32_t rect_x2 = rect_x1 + w;
	uint32_t *dp = fb + (rect_y1 * (fb_pitch / 4));

	uint16_t tmpl_x, tmpl_x_base;

	uint8_t cur_bit, base_bit, inversion = 0;
	uint8_t u8_fg = fg_color >> 24;
	uint8_t u8_bg = bg_color >> 24;
	uint8_t cur_byte = 0;

	tmpl_x = x_offset / 8;
	tmpl_x_base = tmpl_x;

	cur_bit = base_bit = (0x80 >> (x_offset % 8));

	if (draw_mode & INVERSVID) inversion = 1;
	draw_mode &= 0x03;

	if (draw_mode == JAM1) {
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			uint16_t x = rect_x1;

			cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

			while (x < rect_x2) {
				if (w >= 8 && cur_bit == 0x80 && x < rect_x2 - 8) {
					if (mask == 0xFF) {
						SET_FG_PIXELS;
					}
					else {
						SET_FG_PIXELS_MASK;
					}
					x += 8;
				}
				else {
					while (cur_bit > 0 && x < rect_x2) {
						if (cur_byte & cur_bit) {
							SET_FG_PIXEL_MASK;
						}
						x++;
						cur_bit >>= 1;
					}
					cur_bit = 0x80;
				}
				TEMPLATE_FILLRECT_LOOPX;
			}
			TEMPLATE_FILLRECT_LOOPY;
		}

		return;
	}
	else if (draw_mode == JAM2) {
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			uint16_t x = rect_x1;

			cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

			while (x < rect_x2) {
				if (w >= 8 && cur_bit == 0x80 && x < rect_x2 - 8) {
					if (mask == 0xFF) {
						SET_FG_OR_BG_PIXELS;
					}
					else {
						SET_FG_OR_BG_PIXELS_MASK;
					}
					x += 8;
				}
				else {
					while (cur_bit > 0 && x < rect_x2) {
						if (cur_byte & cur_bit) {
							SET_FG_PIXEL_MASK;
						}
						else {
							SET_BG_PIXEL_MASK;
						}
						x++;
						cur_bit >>= 1;
					}
					cur_bit = 0x80;
				}
				TEMPLATE_FILLRECT_LOOPX;
			}
			TEMPLATE_FILLRECT_LOOPY;
		}

		return;
	}
	else { // COMPLEMENT
		for (uint16_t y_line = 0; y_line < h; y_line++) {
			uint16_t x = rect_x1;

			cur_byte = (inversion) ? tmpl_data[tmpl_x] ^ 0xFF : tmpl_data[tmpl_x];

			while (w >= 8 && x < rect_x2) {
				if (cur_bit == 0x80 && x < rect_x2 - 8) {
					INVERT_PIXELS;
					x += 8;
				}
				else {
					while (cur_bit > 0 && x < rect_x2) {
						if (cur_byte & cur_bit) {
							INVERT_PIXEL;
						}
						x++;
						cur_bit >>= 1;
					}
					cur_bit = 0x80;
				}
				TEMPLATE_FILLRECT_LOOPX;
			}
			TEMPLATE_FILLRECT_LOOPY;
		}
	}
}

#define MNTVA_FROM_BPP(d, s) \
	if (s == 2) { \
		d = MNTVA_COLOR_16BIT565; \
	} else if (s == 3) { \
			d = MNTVA_COLOR_15BIT; \
	} else if (s == 4) { \
		d = MNTVA_COLOR_32BIT; \
	}

// Generic graphics acceleration functionality
void acc_clear_buffer(uint32_t addr, uint16_t w, uint16_t h, uint16_t pitch_, uint32_t fg_color, uint32_t color_format_)
{
	if (!w || !h || !addr)
		return;

	uint16_t pitch = pitch_ * color_format_;
	uint8_t* dp = (uint8_t*)((uint32_t)addr);
	uint8_t u8_fg = fg_color >> 24;

	uint8_t color_format = MNTVA_COLOR_8BIT;
	MNTVA_FROM_BPP(color_format, color_format_)

	switch(color_format) {
		case MNTVA_COLOR_8BIT:
			memset(dp, u8_fg, h * pitch);
			break;
		case MNTVA_COLOR_16BIT565:
		case MNTVA_COLOR_15BIT:
		case MNTVA_COLOR_32BIT:
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; y++) {
					SET_FG_PIXEL;
					x++;
				}
				dp += pitch;
			}
			break;
		default:
			// Unknown/unhandled color format.
			break;
	}
}

void acc_flip_to_fb(uint32_t src, uint32_t dest, uint16_t w, uint16_t h, uint16_t pitch_, uint32_t color_format)
{
	// This function assumes a flip of a surface with the same dimensions as the frame buffer.
	if (!w || !h || !src || !dest)
		return;

	uint16_t pitch = pitch_ * color_format;
	uint8_t* sp = (uint8_t*)((uint32_t)src);
	uint8_t* dp = (uint8_t *)((uint32_t)dest);

	memcpy(dp, sp, h * pitch);
}

void acc_blit_rect(uint32_t src, uint32_t dest, uint16_t dx, uint16_t dy, uint16_t w, uint16_t h, uint16_t src_pitch, uint16_t dest_pitch, uint8_t draw_mode, uint8_t mask_color)
{
	if (!w || !h || !src || !dest)
		return;

	uint8_t* sp = (uint8_t*)((uint32_t)src);
	uint8_t* dp = (uint8_t *)((uint32_t)dest);
	dp += (dx + (dy * dest_pitch));

	switch (draw_mode) {
		case 1: // Reverse direction
			sp = (uint8_t*)((uint32_t)src) + (h-1) * src_pitch;
			dp = (uint8_t*)((uint32_t)src) + (dy) * src_pitch;

			for (int y = 0; y < 0; y++) {
				memmove(dp, sp, w);
				dp -= dest_pitch;
				sp -= src_pitch;
			}
			return;
			break;
		case 2: // Masked blit
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					if (sp[x] != mask_color)
						dp[x] = sp[x];
				}
				dp += dest_pitch;
				sp += src_pitch;
			}
			return;
			break;
		default:
			break;
	}

	for (int i = 0; i < h; i++) {
		memcpy(dp, sp, w);
		dp += dest_pitch;
		sp += src_pitch;
	}
}

void acc_blit_rect_16to8(uint32_t src, uint32_t dest, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t src_pitch, uint16_t dest_pitch)
{
	if (!w || !h || !src || !dest)
		return;

	uint16_t* sp = (uint16_t*)((uint32_t)src);
	uint8_t* dp = (uint8_t *)((uint32_t)dest);
	dp += (x + (y * dest_pitch));

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			dp[x] = color_map_16_to_8[SWAP16(sp[x])];
//			dp[x] = color_map_16_to_8[sp[x]];
		}
		dp += dest_pitch;
		sp += src_pitch;
	}
}

#define ACC_DRAW_LINE_PIXELS \
	for (int y = 0; y < pen_width; y++) { \
		for (int x2 = 0; x2 < pen_width; x2++) { \
			switch(color_format) { \
				case MNTVA_COLOR_8BIT: \
					dp[x + x2 + (y * pitch)] = u8_fg; break; \
				case MNTVA_COLOR_16BIT565: \
				case MNTVA_COLOR_15BIT: \
					((uint16_t *)dp)[x + x2 + (y * pitch)] = fg_color; break; \
				case MNTVA_COLOR_32BIT: \
					((uint32_t *)dp)[x + x2 + (y * pitch)] = fg_color; break; \
			} \
		} \
	}

void acc_draw_line(uint32_t dest, uint16_t pitch, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t fg_color, uint8_t bpp, uint8_t pen_width, uint8_t pen_height)
{
	uint8_t color_format = MNTVA_COLOR_8BIT;
	MNTVA_FROM_BPP(color_format, bpp)
	uint8_t u8_fg = fg_color >> 24;

	uint8_t* dp = (uint8_t *)((uint32_t)dest + (y1 * pitch));
	int32_t line_step = pitch;
	int8_t x_reverse = 0;

	int16_t dx, dy, dx_abs, dy_abs, ix, iy, x = x1, len;

	if (x2 < x1)
		x_reverse = 1;
	if (y2 < y1)
		line_step = -pitch;

	dx = x2 - x1;
	dy = y2 - y1;
	dx_abs = abs(dx);
	dy_abs = abs(dy);
	ix = dy_abs >> 1;
	iy = dx_abs >> 1;

	ACC_DRAW_LINE_PIXELS;

	if (dx_abs >= dy_abs) {
		len = dx_abs;
		for (uint16_t i = 0; i < len; i++) {
			iy += dy_abs;
			if (iy >= dx_abs) {
				iy -= dx_abs;
				dp += line_step;
			}
			x += (x_reverse) ? -1 : 1;

			ACC_DRAW_LINE_PIXELS;
		}
	}
	else {
		len = dy_abs;
		for (uint16_t i = 0; i < len; i++) {
			ix += dx_abs;
			if (ix >= dy_abs) {
				ix -= dy_abs;
				x += (x_reverse) ? -1 : 1;
			}
			dp += line_step;

			ACC_DRAW_LINE_PIXELS;
		}
	}
}

void acc_fill_rect(uint32_t dest, uint16_t pitch, int16_t x, int16_t y, int16_t w, int16_t h, uint32_t fg_color, uint8_t bpp)
{
	uint8_t color_format = MNTVA_COLOR_8BIT;
	MNTVA_FROM_BPP(color_format, bpp)
	uint8_t u8_fg = fg_color >> 24;

	uint8_t* dp = (uint8_t *)((uint32_t)dest + (x * bpp) + (y * pitch));
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			switch(color_format) {
				case MNTVA_COLOR_8BIT:
					memset(dp, u8_fg, w);
					j = w;
					break;
				case MNTVA_COLOR_16BIT565:
				case MNTVA_COLOR_15BIT:
					((uint16_t *)dp)[x] = fg_color;
					break;
				case MNTVA_COLOR_32BIT:
					((uint32_t *)dp)[x] = fg_color;
					break;
				default:
					break;
			}
		}
		dp += pitch;
	}
}

#define CHKBLOT(a, b) \
	if (a >= 0 && b >= 0 && a < w && b < h)

//	DrawPixel(surface, x + x1, y + y1, colour);
//	DrawPixel(surface, x - x1, y + y1, colour);
//	DrawPixel(surface, x + x1, y - y1, colour);
//	DrawPixel(surface, x - x1, y - y1, colour);
//	DrawPixel(surface, x + y1, y + x1, colour);
//	DrawPixel(surface, x - y1, y + x1, colour);
//	DrawPixel(surface, x + y1, y - x1, colour);
//	DrawPixel(surface, x - y1, y - x1, colour);

#define BLOTCIRCLE(a, b) \
	CHKBLOT((x + x1),(y + y1)) a[(x + x1) + ((y + y1) * pitch)] = b; \
	CHKBLOT((x - x1),(y + y1)) a[(x - x1) + ((y + y1) * pitch)] = b; \
	CHKBLOT((x + x1),(y - y1)) a[(x + x1) + ((y - y1) * pitch)] = b; \
	CHKBLOT((x - x1),(y - y1)) a[(x - x1) + ((y - y1) * pitch)] = b; \
	CHKBLOT((x + y1),(y + x1)) a[(x + y1) + ((y + x1) * pitch)] = b; \
	CHKBLOT((x - y1),(y + x1)) a[(x - y1) + ((y + x1) * pitch)] = b; \
	CHKBLOT((x + y1),(y - x1)) a[(x + y1) + ((y - x1) * pitch)] = b; \
	CHKBLOT((x - y1),(y - x1)) a[(x - y1) + ((y - x1) * pitch)] = b;

void acc_draw_circle(uint32_t dest, uint16_t pitch, int16_t x, int16_t y, int16_t r, int16_t w, int16_t h, uint32_t fg_color, uint8_t bpp)
{
	uint8_t color_format = MNTVA_COLOR_8BIT;
	MNTVA_FROM_BPP(color_format, bpp)
	uint8_t u8_fg = fg_color >> 24;

	int x1 = r;
	int y1 = 0;
	int d = 3 - 2 * r;

	uint8_t* dp = (uint8_t *)(uint32_t)dest;

	while (x1 >= y1) {
		y1++;

		if (d > 0) {
			x1--;
			d = d + 4 * (y1 - x1) + 10;
		}
		else {
			d = d + 4 * y1 + 6;
		}

		switch(color_format) {
			case MNTVA_COLOR_8BIT:
				BLOTCIRCLE(dp, u8_fg);
				break;
			case MNTVA_COLOR_16BIT565:
			case MNTVA_COLOR_15BIT:
				BLOTCIRCLE(((uint16_t *)dp), fg_color);
				break;
			case MNTVA_COLOR_32BIT:
				BLOTCIRCLE(((uint32_t *)dp), fg_color);
				break;
			default:
				break;
		}
	}
}

void acc_fill_circle(uint32_t dest, uint16_t pitch, int16_t x0, int16_t y0, int16_t r, int16_t w, int16_t h, uint32_t fg_color, uint8_t bpp)
{
	uint8_t color_format = MNTVA_COLOR_8BIT;
	MNTVA_FROM_BPP(color_format, bpp)
	uint8_t u8_fg = fg_color >> 24;

	uint8_t* dp = (uint8_t *)(uint32_t)dest;
	float radius_sqr = r * r;

	for (int x = -r; x < r ; x++)
	{
		int hh = (int)sqrt(radius_sqr - x * x);
		int rx = x0 + x;
		int ph = y0 + hh;

		for (int y = y0 - hh; y < ph; y++) {
			switch(color_format) {
				case MNTVA_COLOR_8BIT:
					CHKBLOT(rx, y)
						dp[rx + (y * pitch)] = u8_fg;
					break;
				case MNTVA_COLOR_16BIT565:
				case MNTVA_COLOR_15BIT:
					CHKBLOT(rx, y)
						((uint16_t *)dp)[rx + (y * pitch)] = fg_color;
					break;
				case MNTVA_COLOR_32BIT:
					CHKBLOT(rx, y)
						((uint32_t *)dp)[rx + (y * pitch)] = fg_color;
					break;
				default:
					break;
			}
		}
	}
}

uint8_t *tri_array = 0;

void TriTexLine(int32_t x1, int32_t x2, int32_t y, int32_t tx1, int32_t tx2, int32_t ty1, int32_t ty2, uint16_t w, uint16_t h, uint32_t fg_color)
{
	// Round to ensure that problems caused by rounding errors don't occur (jumping lines)
	x2 &= 0xFFFF0000;
	x1 &= 0xFFFF0000;
	uint8_t u8_fg = fg_color >> 24;

	// Sort values to make drawing from left to right possible
	if (x2 < x1) {
		int32_t temp = x2;
		x2 = x1;
		x1 = temp;
		temp = tx2;
		tx2 = tx1;
		tx1 = temp;
		temp = ty2;
		ty2 = ty1;
		ty1 = temp;
	}

	int32_t xdelta = (x2 - x1) >>16;
	if (xdelta <1)
		return;

	int xd = xdelta;

	//Calculate start Tex-X and Tex-X-Increment
	int txi = tx1; //fixed point
	int32_t txd = (tx2 - tx1) / xdelta;
	int txdi = txd; //same here

	//Same for Tex-Y and Tex-Y-Increment
	int tyi = ty1; //fixed point
	int32_t tyd = (ty2 - ty1) / xdelta;
	int tydi = tyd; //same here


	//Clipping begin
	//If line isn't inside screen -> outta here
	x1 >>= 16;
	x2 >>= 16;

	if (x1 > ((w - 1)) || (x2 < 0))
		return;

	/*If the line is clipped at the left screen border (where we start), the left out
	gouraud and texture steps have to be calculated; x is set to 0 */
	if (x1 < 0) {
		//int xm=-x1;
		x1 = 0;
	}
	/* x is simply clipped at the right border. That's where the loop is going to end
	then */
	if (x2 > (w - 1))
		x2= (w - 1);
	//End of clipping and calculation of screen start address
		int arrayptr = (y * w) + (x1);

	//Recalculate X-Delta because of clipping
	xdelta = (x2 - x1);
	if (xdelta <= 0)
		return;
	xd = (int)(xdelta);

	for (int x = 0; x <= xd; x++) {
		//Fetch a pixel from the texture (256*256)
		*(tri_array + (arrayptr++)) = u8_fg;
		//*(tri_array + (arrayptr++)) = array_tex1[(txi >> 16) + ((tyi >> 8) & 0xff00)];

		//Increase Texture- and Gouraud-counter
		txi += txdi;
		tyi += tydi;
	}
}

// filled tri code viciously stolen from mntmn...!
void acc_fill_flat_tri(uint32_t dest, TriangleDef *d, uint16_t w, uint16_t h, uint32_t fg_color, uint8_t bpp)
{
	uint8_t u8_fg = fg_color >> 24;
	int32_t *dataa, *datab, *datac;

	int32_t xs1, xs2, xs3, txs1, txs2, txs3, tys1, tys2, tys3;
	int32_t *tempdata;

	dataa = d->a;
	datab = d->b;
	datac = d->c;

	tri_array = (uint8_t *)dest;

	// Very simple sorting of the three y coordinates
	if (dataa[1] > datab[1]) {
		tempdata = dataa;
		dataa = datab;
		datab = tempdata;
	}
	if (datab[1] > datac[1]) {
		tempdata = datab;
		datab = datac;
		datac = tempdata;
	}
	if (dataa[1] > datab[1]) {
		tempdata = dataa;
		dataa = datab;
		datab = tempdata;
	}

	// Calculate some deltas
	int32_t xd1 = datab[0] - dataa[0];
	int32_t xd2 = datac[0] - dataa[0];
	int32_t xd3 = datac[0] - datab[0];
	int32_t yd1 = datab[1] - dataa[1];
	int32_t yd2 = datac[1] - dataa[1];
	int32_t yd3 = datac[1] - datab[1];
	int32_t txd1 = datab[2] - dataa[2];
	int32_t txd2 = datac[2] - dataa[2];
	int32_t txd3 = datac[2] - datab[2];
	int32_t tyd1 = datab[3] - dataa[3];
	int32_t tyd2 = datac[3] - dataa[3];
	int32_t tyd3 = datac[3] - datab[3];

	// Calculate steps per line while taking care of division by 0
	if(yd1 != 0) {
		xs1 = xd1 / yd1;
		txs1 = txd1 / yd1;
		tys1 = tyd1 / yd1;
	}
	else {
		xs1 = xd1;
		txs1 = txd1;
		tys1 = tyd1;
	}
	if(yd2 != 0) {
		xs2 = xd2 / yd2;
		txs2 = txd2 / yd2;
		tys2 = tyd2 / yd2;
	}
	else {
		xs2 = xd2;
		txs2 = txd2;
		tys2 = tyd2;
	}
	if(yd3 != 0) {
		xs3 = xd3 / yd3;
		txs3 = txd3 / yd3;
		tys3 = tyd3 / yd3;
	}
	else  {
		xs3 = xd3;
		txs3 = txd3;
		tys3 = tyd3;
	}

	/*
	 Variable meanings:

	 xs? xstep=delta x
	 txs? delta tx
	 tys? delta ty
	 xd? xdelta
	 yd? dunno
	 txd?  "
	 tyd?  "
	 xw? current x-value used in loop
	 txw? for tx
	 tyw? for ty
	*/
	/*
	 Start values for the first part (up to y of point 2)
	 xw1 and xw2 are x-values for the current line. The triangle is drawn from
	 top to bottom line after line...
	 txw, tyw and gw are values for texture and brightness
	 always for start- and ending-point of the current line
	 A line is also called "Span".
	*/

	int32_t xw1 = dataa[0]; //pax
	int32_t xw2 = dataa[0];
	int32_t txw1 = dataa[2]; //tax
	int32_t txw2 = dataa[2];
	int32_t tyw1 = dataa[3]; //tay
	int32_t tyw2 = dataa[3];

	if (yd1) {
		for (int sz = dataa[1]; sz <= datab[1]; sz++) {
			// draw if y is inside the screen (clipping)
			if (sz >=h )
				break;
			if (sz >= 0 && sz < h) {
				uint32_t xed = (xw1 < xw2) ? xw1 : xw2;
				uint32_t xed2 = (xw1 < xw2) ? xw2 : xw1;
				xed = ((xed & 0xFFFF0000) >> 16);
				xed2 = ((xed2 & 0xFFFF0000) >> 16);
				if ((xed < 0 && xed2 < 0) || (xed >= w && xed2 >= w) || (xed == xed2))
					goto skip_span;
				if (xed < 0) xed = 0;
				if (xed2 >= w) xed2 = w - 1;

				int clear_w = (xed == xed2) ? 1 : xed2 - xed;
				memset((uint8_t *)(uint32_t)(dest + xed + (sz * w)), u8_fg, clear_w);
				skip_span:;
			}
			xw1 += xs1;
			xw2 += xs2;
			txw1 += txs1;
			txw2 += txs2;
			tyw1 += tys1;
			tyw2 += tys2;
		}
	}

	/*
	 New start values for the second part of the triangle
	*/
	xw1 = datab[0] + xs3;
	txw1 = datab[2] + txs3;
	tyw1 = datab[3] + tys3;

	if (yd3) { //If Span-Height 1 or higher
		for (int sz=datab[1] + 1; sz < datac[1]; sz++)
		{
			if (sz >=h )
				break;

			if (sz >= 0 && sz < (h - 1)) {
				int xed = (xw1 < xw2) ? xw1 : xw2;
				int xed2 = (xw1 < xw2) ? xw2 : xw1;
				xed = ((xed & 0xFFFF0000) >> 16);
				xed2 = ((xed2 & 0xFFFF0000) >> 16);
				if ((xed < 0 && xed2 < 0) || (xed >= w && xed2 >= w) || (xed == xed2))
					goto skip_span2;
				if (xed < 0) xed = 0;
				if (xed2 >= w) xed2 = w - 1;

				int clear_w = (xed == xed2) ? 1 : xed2 - xed;
				memset((uint8_t *)(uint32_t)(dest + xed + (sz * w)), u8_fg, clear_w);
				skip_span2:;
			}
			xw1 += xs3;
			xw2 += xs2;
			txw1 += txs3;
			txw2 += txs2;
			tyw1 += tys3;
			tyw2 += tys2;
		}
	}
};
