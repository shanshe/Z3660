/*
 * video.h
 *
 *  Created on: 10 ago. 2022
 *      Author: shanshe
 */

#ifndef SRC_VIDEO_H_
#define SRC_VIDEO_H_

#include "xscugic.h"
#include "xil_cache.h"
#include "xil_cache_l.h"
#include "rtg/zzregs.h"

#define L1_CACHE_ENABLED
#define L2_CACHE_ENABLED
//#define NO_L1_CACHE_FLUSH
//#define NO_L2_CACHE_FLUSH

inline void handle_cache_flush(void)
{
#ifndef NO_L1_CACHE_FLUSH
#ifdef L1_CACHE_ENABLED
	Xil_L1DCacheFlush();
#endif
#endif
#ifndef NO_L2_CACHE_FLUSH
#ifdef L2_CACHE_ENABLED
	Xil_L2CacheFlush();
#endif
#endif
}

typedef struct {
	uint32_t* framebuffer;

	uint32_t framebuffer_size;

	uint16_t video_mode;
	uint16_t colormode;
	uint16_t scalemode;

	uint32_t vmode_hsize;
	uint32_t vmode_vsize;
	uint32_t vmode_hdiv;
	uint32_t vmode_vdiv;

	uint16_t videocap_video_mode;

	uint16_t interlace_old;
	uint16_t videocap_ntsc_old;
	uint16_t videocap_enabled_old;
	uint16_t split_request_pos;
	uint16_t split_pos;
	uint32_t bgbuf_offset;

	uint32_t framebuffer_pan_offset;
	uint32_t framebuffer_pan_width;
	uint8_t scandoubler_mode_adjust;

	uint16_t sprite_showing;
	int16_t sprite_x;
	int16_t sprite_x_adj;
	int16_t sprite_x_base;
	int16_t sprite_y;
	int16_t sprite_y_adj;
	int16_t sprite_y_base;
	int16_t sprite_x_offset;
	int16_t sprite_y_offset;
	uint8_t sprite_width;
	uint8_t sprite_height;
	uint32_t sprite_colors[4];

	uint8_t card_feature_enabled[CARD_FEATURE_NUM];
} ZZ_VIDEO_STATE;

ZZ_VIDEO_STATE* video_init();

void isr_video(void *dummy);
int set_framebuffer_address(int hsize, int vsize, int hdiv, int vdiv, uint32_t bufpos);
void video_formatter_write(uint32_t data, uint16_t op);
uint32_t video_formatter_read(uint16_t op);
void video_formatter_valign(void);
void hw_sprite_show(int show);
void update_hw_sprite(uint8_t *data, int double_sprite);
void update_hw_sprite_clut(uint8_t *data_, uint8_t *colors, uint16_t w, uint16_t h, uint8_t keycolor, int double_sprite);
void update_hw_sprite_pos();
void clip_hw_sprite(int16_t offset_x, int16_t offset_y);
void clear_hw_sprite();
void video_reset(void);



#endif /* SRC_VIDEO_H_ */
