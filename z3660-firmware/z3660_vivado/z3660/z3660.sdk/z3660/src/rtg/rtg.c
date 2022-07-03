/*
 * rtg.c
 *
 *  Created on: 26 jun. 2022
 *      Author: shanshe
 */
#include <stdio.h>
#include "stdint.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xil_cache.h"
#include "zzregs.h"
#include "../main.h"
#include "gfx.h"
#include "zz_video_modes.h"

extern u32 *frameBuf;

unsigned int cur_mem_offset = 0x3500000;

u32 gpio=0,gpio_last=0;
u32 framebuffer_pan_offset = 0;

int16_t sprite_x_offset = 0;
int16_t sprite_y_offset = 0;
int interrupt_enabled = 0;
int colormode = 0;
int scalemode = 0;
int video_mode = ZZVMODE_1920x1080_50 | 2 << 12 | MNTVA_COLOR_32BIT << 8;
int16_t sprite_x = 0, sprite_x_adj = 0, sprite_x_base = 0;
int16_t sprite_y = 0, sprite_y_adj = 0, sprite_y_base = 0;
int16_t sprite_clip_x = 0, sprite_clip_y = 0;
u8 sprite_clipped = 0;
u16 sprite_enabled = 0;
u8 sprite_width  = 16;
u8 sprite_height = 16;
u32 sprite_colors[4] = {0x00ff00ff, 0x00000000, 0x00000000, 0x00000000 };
u32 sprite_buf[32*48];

// blitter etc
u16 rect_x1 = 0;
u16 rect_x2 = 0;
u16 rect_x3 = 0;
u16 rect_y1 = 0;
u16 rect_y2 = 0;
u16 rect_y3 = 0;
u16 blitter_user1 = 0;
u16 blitter_user2 = 0;
u16 blitter_user3 = 0;
u16 blitter_user4 = 0;
u16 blitter_src_pitch = 0;
u16 blitter_dst_pitch = 0;
u32 rect_rgb = 0;
u32 rect_rgb2 = 0;
u32 debug_lowlevel = 0;

u32 blitter_src_offset = 0;
u32 blitter_dst_offset = 0;
u32 blitter_colormode = MNTVA_COLOR_32BIT;
u32 bgbuf_offset = 0;
uint16_t split_pos = 0, next_split_pos = 0;
u32 vmode_hsize = 1920, vmode_vsize = 1080, vmode_hdiv = 4, vmode_vdiv=1;
void video_formatter_valign() {
	// vertical alignment
/*	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG3, 1);
	usleep(1);
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0x80000000 + 0x5); // OP_VSYNC
	usleep(1);
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG3, 0);
	usleep(1);
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0x80000000); // NOP
	usleep(1);
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0); // NOP
	usleep(1);
*/}

#define VF_DLY ;
void sprite_hide() {
	sprite_x = 2000;
	sprite_y = 2000;
	sprite_enabled = 0;
	video_formatter_write((sprite_y << 16) | sprite_x, MNTVF_OP_SPRITE_XY);
}

void video_formatter_write(uint32_t data, uint16_t op) {
/*	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG3, data);
	VF_DLY;
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0x80000000 | op); // OP_MAX (vmax | hmax)
	VF_DLY;
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0x80000000); // NOP
	VF_DLY;
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0); // clear
	VF_DLY;
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG3, 0); // clear
	VF_DLY;
*/
}

void update_hw_sprite_pos(int16_t x, int16_t y) {
	sprite_x = x + sprite_x_offset + 1;
	// horizontally doubled mode
	if (scalemode & 1)
		sprite_x_adj = (sprite_x * 2) + 1;
	else
		sprite_x_adj = sprite_x + 2;

	sprite_y = y + split_pos + sprite_y_offset + 1;

	// vertically doubled mode
	if (scalemode & 2)
		sprite_y_adj = sprite_y *= 2;
	else
		sprite_y_adj = sprite_y;

	if (sprite_x < 0 || sprite_y < 0) {
		if (sprite_clip_x != sprite_x || sprite_clip_y != sprite_y) {
			clip_hw_sprite((sprite_x < 0) ? sprite_x : 0, (sprite_y < 0) ? sprite_y : 0);
		}
		sprite_clipped = 1;
		if (sprite_x < 0) {
			sprite_x_adj = 0;
			sprite_clip_x = sprite_x;
		}
		if (sprite_y < 0) {
			sprite_y_adj = 0;
			sprite_clip_y = sprite_y;
		}
	}
	else if (sprite_clipped && sprite_x >= 0 && sprite_y >= 0) {
		clip_hw_sprite(0, 0);
		sprite_clipped = 0;
	}
	video_formatter_write((sprite_y_adj << 16) | sprite_x_adj, MNTVF_OP_SPRITE_XY);
}

void video_formatter_init(int scalemode, int colormode, int width, int height,
		int htotal, int vtotal, int hss, int hse, int vss, int vse,
		int polarity)
{
/*
	video_formatter_write((vtotal << 16) | htotal, MNTVF_OP_MAX);
	video_formatter_write((height << 16) | width, MNTVF_OP_DIMENSIONS);
	video_formatter_write((hss << 16) | hse, MNTVF_OP_HS);
	video_formatter_write((vss << 16) | vse, MNTVF_OP_VS);
	video_formatter_write(polarity, MNTVF_OP_POLARITY);
	video_formatter_write(scalemode, MNTVF_OP_SCALE);
	video_formatter_write(colormode, MNTVF_OP_COLORMODE);

	video_formatter_valign();
*/
}
void video_system_init(struct zz_video_mode *mode, int hdiv, int vdiv) {
//	pixelclock_init_2(mode);
	sii9022_init();
	init_vdma(mode->hres, mode->vres, hdiv, vdiv, ((u32)frameBuf) + framebuffer_pan_offset);
}

void video_mode_init(int mode, int scalemode, int colormode) {
	int hdiv = 1, vdiv = 1;

	if (scalemode & 1)
		hdiv = 2;
	if (scalemode & 2)
		vdiv = 2;

	if (colormode == 0)
		hdiv *= 4;
	if (colormode == 1)
		hdiv *= 2;

	struct zz_video_mode *vmode = &preset_video_modes[mode];

	video_system_init(vmode, hdiv, vdiv);

	video_formatter_init(scalemode, colormode,
			vmode->hres, vmode->vres,
			vmode->hmax, vmode->vmax,
			vmode->hstart, vmode->hend,
			vmode->vstart, vmode->vend,
			vmode->polarity);

	vmode_hsize = vmode->hres;
	vmode_vsize = vmode->vres;
	vmode_vdiv = vdiv;
	vmode_hdiv = hdiv;
}
#define REVISION_MAJOR 1
#define REVISION_MINOR 8

void rtg_init(void)
{
	gpio=XGpio_In32((XPAR_AXI_GPIO_0_BASEADDR) + (8));
	gpio_last=gpio;

	*((u32 *)REG_ZZ_FW_VERSION)= (REVISION_MAJOR << 8 | REVISION_MINOR);
	framebuffer_pan_offset=0;
}
u32 *address;
u32 zdata;
u16 zaddr;
u32 writes_counter=0;
void rtg_loop(void)
{
	static int loop_counter=0;
	loop_counter++;
	if(loop_counter==100000)
	{
		cache_flush();
		loop_counter=0;
	}
	gpio=XGpio_In32((XPAR_AXI_GPIO_0_BASEADDR) + (8));
	if((gpio&0x80000000)!=(gpio_last&0x80000000))
	{
		gpio_last=gpio;
		address=(u32 *)(gpio&0xFFFF);
		zdata=*address;
		zaddr=gpio&0xFFFF;
		writes_counter++;
/*
		for (int i = 0; i < 108; i++)
		{
			for (int j = 0; j < 192; j++)
			{
				u8 r,g,b;
				b=(data>>16)&0xFF;
				g=(data>>8)&0xFF;
				r=(data)&0xFF;
				frameBuf[0][1920*4*i+4*j]=0x00;
				frameBuf[0][1920*4*i+4*j+1]=b;
				frameBuf[0][1920*4*i+4*j+2]=g;
				frameBuf[0][1920*4*i+4*j+3]=r;
			}
		}
		Xil_DCacheFlushRange((unsigned int) frameBuf[0], DEMO_MAX_FRAME);
*/
//		if(zaddr!=0x80)
//			printf("W %02x\r\n",zaddr);

		switch (zaddr) {
		// Various blitter/video registers
		case REG_ZZ_PAN:
			framebuffer_pan_offset = zdata;

			// cursor offset support for p96 split screen
			sprite_x_offset = rect_x1;
			sprite_y_offset = rect_y1;

			break;

		case REG_ZZ_BLIT_SRC:
			blitter_src_offset = zdata;
			break;
		case REG_ZZ_BLIT_DST:
			blitter_dst_offset = zdata;
			break;

		case REG_ZZ_COLORMODE:
			blitter_colormode = zdata;
			break;
		case REG_ZZ_CONFIG:
			// enable/disable INT6, currently used to signal incoming ethernet packets
			interrupt_enabled = zdata & 1;
			break;
		case REG_ZZ_MODE: {
			//printf("mode change: %lx\n", zdata);

			int mode = zdata & 0xff;
			colormode = (zdata & 0xf00) >> 8;
			scalemode = (zdata & 0xf000) >> 12;
//			printf("mode: %d color: %d scale: %d\n", mode, colormode, scalemode);

			video_mode_init(mode, scalemode, colormode);
			// remember selected video mode
			video_mode = zdata;
			//request_video_align = 1;
			break;
		}
		case REG_ZZ_SPRITE_X:
		case REG_ZZ_SPRITE_Y:
			if (!sprite_enabled)
				break;

			sprite_x_base = (int16_t)rect_x1;
			sprite_y_base = (int16_t)rect_y1;

			update_hw_sprite_pos(sprite_x_base, sprite_y_base);

			break;
		case REG_ZZ_SPRITE_BITMAP: {
			if (zdata == 1) { // Hardware sprite enabled
				sprite_enabled = 1;
				break;
			}
			else if (zdata == 2) { // Hardware sprite disabled
				sprite_hide();
				break;
			}

			uint8_t* bmp_data = (uint8_t*) (((u32) frameBuf) + blitter_src_offset);

			clear_hw_sprite();

			sprite_x_offset = rect_x1;
			sprite_y_offset = rect_y1;
			sprite_width  = rect_x2;
			sprite_height = rect_y2;

			update_hw_sprite(bmp_data, sprite_colors, sprite_width, sprite_height);
			update_hw_sprite_pos(sprite_x_base, sprite_y_base);
			break;
		}
		case REG_ZZ_SPRITE_COLORS: {
			sprite_colors[zdata] = (blitter_user1 << 16) | blitter_user2;
			if (zdata != 0 && sprite_colors[zdata] == 0xff00ff)
        		sprite_colors[zdata] = 0xfe00fe;
			break;
		}
		case REG_ZZ_SRC_PITCH:
			blitter_src_pitch = zdata;
			break;

		case REG_ZZ_X1:
			rect_x1 = zdata;
//			printf("rect_x1 %d\r\n",rect_x1);
			break;
		case REG_ZZ_Y1:
			rect_y1 = zdata;
//			printf("rect_y1 %d\r\n",rect_y1);
			break;
		case REG_ZZ_X2:
			rect_x2 = zdata;
//			printf("rect_x2 %d\r\n",rect_x2);
			break;
		case REG_ZZ_Y2:
			rect_y2 = zdata;
//			printf("rect_y2 %d\r\n",rect_y2);
			break;
		case REG_ZZ_ROW_PITCH:
			blitter_dst_pitch = zdata;
			break;
		case REG_ZZ_X3:
			rect_x3 = zdata;
			break;
		case REG_ZZ_Y3:
			rect_y3 = zdata;
			break;

		case REG_ZZ_USER1:
			blitter_user1 = zdata;
			break;
		case REG_ZZ_USER2:
			blitter_user2 = zdata;
			break;
		case REG_ZZ_USER3:
			blitter_user3 = zdata;
			break;
		case REG_ZZ_USER4:
			blitter_user4 = zdata;
			break;

		case REG_ZZ_RGB:
			rect_rgb  = SWAP32(zdata);
//			rect_rgb  = zdata;
			break;
		case REG_ZZ_RGB2:
			rect_rgb2  = SWAP32(zdata);
//			rect_rgb2  = zdata;
			break;

		// Generic acceleration ops
		case REG_ZZ_ACC_OP: {
			handle_acc_op(zdata);
			break;
		}
		case REG_ZZ_FW_VERSION: // this is readonly, so restore its value if it is written
			*((u32 *)REG_ZZ_FW_VERSION)= (REVISION_MAJOR << 24 | REVISION_MINOR << 16);
			break;
		// DMA RTG rendering
		case REG_ZZ_BITTER_DMA_OP: {
			handle_blitter_dma_op(zdata);
			break;
		}

		// RTG rendering
		case REG_ZZ_FILLRECT:
//			printf("FILLRECT blitter_dst_offset %lx\r\n",blitter_dst_offset);
//			printf("         %d %d %d %d \r\n",rect_x1,rect_y1,rect_x2,rect_y2);
			set_fb((uint32_t*) (((u32) frameBuf) + blitter_dst_offset),
					blitter_dst_pitch);
			uint8_t mask = zdata;

			if (mask == 0xFF)
				fill_rect_solid(rect_x1, rect_y1, rect_x2, rect_y2,
						rect_rgb, blitter_colormode);
			else
				fill_rect(rect_x1, rect_y1, rect_x2, rect_y2, rect_rgb,
						blitter_colormode, mask);
			break;

		case REG_ZZ_COPYRECT: {
			set_fb((uint32_t*) (((u32) frameBuf) + blitter_dst_offset),
					blitter_dst_pitch);
			mask = (blitter_colormode >> 8);

			switch (zdata) {
			case 1: // Regular BlitRect
				if (mask == 0xFF || (mask != 0xFF && (blitter_colormode & 0x0F)) != MNTVA_COLOR_8BIT)
					copy_rect_nomask(rect_x1, rect_y1, rect_x2, rect_y2, rect_x3,
									rect_y3, blitter_colormode & 0x0F,
									(uint32_t*) (((u32) frameBuf)
											+ blitter_dst_offset),
									blitter_dst_pitch, MINTERM_SRC);
				else
					copy_rect(rect_x1, rect_y1, rect_x2, rect_y2, rect_x3,
							rect_y3, blitter_colormode & 0x0F,
							(uint32_t*) (((u32) frameBuf)
									+ blitter_dst_offset),
							blitter_dst_pitch, mask);
				break;
			case 2: // BlitRectNoMaskComplete
				copy_rect_nomask(rect_x1, rect_y1, rect_x2, rect_y2, rect_x3,
								rect_y3, blitter_colormode & 0x0F,
								(uint32_t*) (((u32) frameBuf)
										+ blitter_src_offset),
								blitter_src_pitch, mask); // Mask in this case is minterm/opcode.
				break;
			}

			break;
		}

		case REG_ZZ_FILLTEMPLATE: {
			uint8_t draw_mode = blitter_colormode >> 8;
			uint8_t* tmpl_data = (uint8_t*) (((u32) frameBuf)
					+ blitter_src_offset);
			set_fb((uint32_t*) (((u32) frameBuf) + blitter_dst_offset),
					blitter_dst_pitch);

			uint8_t bpp = 2 * (blitter_colormode & 0xff);
			if (bpp == 0)
				bpp = 1;
			uint16_t loop_rows = 0;
			mask = zdata;

			if (zdata & 0x8000) {
				// pattern mode
				// TODO yoffset
				loop_rows = zdata & 0xff;
				mask = blitter_user1;
				blitter_src_pitch = 16;
				pattern_fill_rect((blitter_colormode & 0x0F), rect_x1,
						rect_y1, rect_x2, rect_y2, draw_mode, mask,
						rect_rgb, rect_rgb2, rect_x3, rect_y3, tmpl_data,
						blitter_src_pitch, loop_rows);
			}
			else {
				template_fill_rect((blitter_colormode & 0x0F), rect_x1,
						rect_y1, rect_x2, rect_y2, draw_mode, mask,
						rect_rgb, rect_rgb2, rect_x3, rect_y3, tmpl_data,
						blitter_src_pitch);
			}

			break;
		}
/*
		case 0x50: { // Copy crap from scratch area
			for (int i = 0; i < rect_y1; i++) {
				memcpy	((uint32_t*) (((u32) frameBuf) + frameBuf_pan_offset + (i * rect_x1)),
						 (uint32_t*) ((u32)Z3_SCRATCH_ADDR + (i * rect_x1)),
						 rect_x1);
			}
			break;
		}

		case 0x52: // Custom video mode param
			custom_vmode_param = zdata;
			break;

		case 0x54: { // Custom video mode data
			int *target = &preset_video_modes[custom_video_mode].hres;
			switch(custom_vmode_param) {
				case VMODE_PARAM_VRES: target = &preset_video_modes[custom_video_mode].vres; break;
				case VMODE_PARAM_HSTART: target = &preset_video_modes[custom_video_mode].hstart; break;
				case VMODE_PARAM_HEND: target = &preset_video_modes[custom_video_mode].hend; break;
				case VMODE_PARAM_HMAX: target = &preset_video_modes[custom_video_mode].hmax; break;
				case VMODE_PARAM_VSTART: target = &preset_video_modes[custom_video_mode].vstart; break;
				case VMODE_PARAM_VEND: target = &preset_video_modes[custom_video_mode].vend; break;
				case VMODE_PARAM_VMAX: target = &preset_video_modes[custom_video_mode].vmax; break;
				case VMODE_PARAM_POLARITY: target = &preset_video_modes[custom_video_mode].polarity; break;
				case VMODE_PARAM_MHZ: target = &preset_video_modes[custom_video_mode].mhz; break;
				case VMODE_PARAM_PHZ: target = &preset_video_modes[custom_video_mode].phz; break;
				case VMODE_PARAM_VHZ: target = &preset_video_modes[custom_video_mode].vhz; break;
				case VMODE_PARAM_HDMI: target = &preset_video_modes[custom_video_mode].hdmi; break;
				case VMODE_PARAM_MUL: target = &preset_video_modes[custom_video_mode].mul; break;
				case VMODE_PARAM_DIV: target = &preset_video_modes[custom_video_mode].div; break;
				case VMODE_PARAM_DIV2: target = &preset_video_modes[custom_video_mode].div2; break;
				default: break;
			}

			*target = zdata;
			break;
		}

		case 0x56: // Set custom video mode index
			custom_video_mode = zdata;
			break;

		case 0x58: // Set custom video mode without any questions asked.
			// This assumes that the custom video mode is 640x480 or higher resolution.
			video_mode_init(custom_video_mode, scalemode, colormode);
			break;
*/
		case REG_ZZ_P2C: {
			uint8_t draw_mode = blitter_colormode >> 8;
			uint8_t planes = (zdata & 0xFF00) >> 8;
			uint8_t mask = (zdata & 0xFF);
			uint8_t layer_mask = blitter_user2;
			uint8_t* bmp_data = (uint8_t*) (((u32) frameBuf)
					+ blitter_src_offset);

			set_fb((uint32_t*) (((u32) frameBuf) + blitter_dst_offset),
					blitter_dst_pitch);

			p2c_rect(rect_x1, 0, rect_x2, rect_y2, rect_x3,
					rect_y3, draw_mode, planes, mask,
					layer_mask, blitter_src_pitch, bmp_data);
			break;
		}

		case REG_ZZ_P2D: {
			uint8_t draw_mode = blitter_colormode >> 8;
			uint8_t planes = (zdata & 0xFF00) >> 8;
			uint8_t mask = (zdata & 0xFF);
			uint8_t layer_mask = blitter_user2;
			uint8_t* bmp_data = (uint8_t*) (((u32) frameBuf)
					+ blitter_src_offset);

			set_fb((uint32_t*) (((u32) frameBuf) + blitter_dst_offset),
					blitter_dst_pitch);
			p2d_rect(rect_x1, 0, rect_x2, rect_y2, rect_x3,
					rect_y3, draw_mode, planes, mask, layer_mask, rect_rgb,
					blitter_src_pitch, bmp_data, (blitter_colormode & 0x0F));
			break;
		}

		case REG_ZZ_DRAWLINE: {
			uint8_t draw_mode = blitter_colormode >> 8;
			set_fb((uint32_t*) (((u32) frameBuf) + blitter_dst_offset),
					blitter_dst_pitch);

			// rect_x3 contains the pattern. if all bits are set for both the mask and the pattern,
			// there's no point in passing non-essential data to the pattern/mask aware function.

			if (rect_x3 == 0xFFFF && zdata == 0xFF)
				draw_line_solid(rect_x1, rect_y1, rect_x2, rect_y2,
						blitter_user1, rect_rgb,
						(blitter_colormode & 0x0F));
			else
				draw_line(rect_x1, rect_y1, rect_x2, rect_y2,
						blitter_user1, rect_x3, rect_y3, rect_rgb,
						rect_rgb2, (blitter_colormode & 0x0F), zdata,
						draw_mode);
			break;
		}

		case REG_ZZ_INVERTRECT:
			set_fb((uint32_t*) (((u32) frameBuf) + blitter_dst_offset),
					blitter_dst_pitch);
			invert_rect(rect_x1, rect_y1, rect_x2, rect_y2,
					zdata & 0xFF, blitter_colormode);
			break;

		case REG_ZZ_SET_SPLIT_POS:
			bgbuf_offset = blitter_src_offset;
			next_split_pos = zdata;

			video_formatter_write(split_pos, MNTVF_OP_REPORT_LINE);
			break;
/*
		// Ethernet
		case REG_ZZ_ETH_TX:
			ethernet_send_result = ethernet_send_frame(zdata);
			//printf("SEND frame sz: %ld res: %d\n",zdata,ethernet_send_result);
			break;
		case REG_ZZ_ETH_RX: {
			//printf("RECV eth frame sz: %ld\n",zdata);
			int frfb = ethernet_receive_frame();
			mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG4, frfb);
			break;
		}
		case REG_ZZ_ETH_MAC_HI: {
			uint8_t* mac = ethernet_get_mac_address_ptr();
			mac[0] = (zdata & 0xff00) >> 8;
			mac[1] = (zdata & 0x00ff);
			break;
		}
		case REG_ZZ_ETH_MAC_HI2: {
			uint8_t* mac = ethernet_get_mac_address_ptr();
			mac[2] = (zdata & 0xff00) >> 8;
			mac[3] = (zdata & 0x00ff);
			break;
		}
		case REG_ZZ_ETH_MAC_LO: {
			uint8_t* mac = ethernet_get_mac_address_ptr();
			mac[4] = (zdata & 0xff00) >> 8;
			mac[5] = (zdata & 0x00ff);
			ethernet_update_mac_address();
			break;
		}
		case REG_ZZ_USBBLK_TX_HI: {
			usb_storage_write_block = ((u32) zdata) << 16;
			break;
		}
		case REG_ZZ_USBBLK_TX_LO: {
			usb_storage_write_block |= zdata;
			if (usb_storage_available) {
				usb_status = zz_usb_write_blocks(0, usb_storage_write_block, usb_read_write_num_blocks, (void*)USB_BLOCK_STORAGE_ADDRESS);
			} else {
				printf("[USB] TX but no storage available!\n");
			}
			break;
		}
		case REG_ZZ_USBBLK_RX_HI: {
			usb_storage_read_block = ((u32) zdata) << 16;
			break;
		}
		case REG_ZZ_USBBLK_RX_LO: {
			usb_storage_read_block |= zdata;
			if (usb_storage_available) {
				usb_status = zz_usb_read_blocks(0, usb_storage_read_block, usb_read_write_num_blocks, (void*)USB_BLOCK_STORAGE_ADDRESS);
			} else {
				printf("[USB] RX but no storage available!\n");
			}
			break;
		}
		case REG_ZZ_USB_STATUS: {
			//printf("[USB] write to status/blocknum register: %d\n", zdata);
			if (zdata==0) {
				// reset USB
				// FIXME memory leaks?
				//usb_storage_available = zz_usb_init();
			} else {
				// set number of blocks to read/write at once
				usb_read_write_num_blocks = zdata;
			}
			break;
		}
		case REG_ZZ_USB_BUFSEL: {
			//printf("[USB] select buffer: %d\n", zdata);
			usb_selected_buffer_block = zdata;
			mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG5, usb_selected_buffer_block);
			break;
		}
*/
		case REG_ZZ_DEBUG: {
			debug_lowlevel = zdata;

			break;
		}
/*
		// ARM core 2 execution
		case REG_ZZ_ARM_RUN_HI:
			arm_run_address = ((u32) zdata) << 16;
			break;
		case REG_ZZ_ARM_RUN_LO:
			// TODO checksum?
			arm_run_address |= zdata;

			*core1_addr = (uint32_t) core1_loop;
			core1_addr2[0] = 0xe3e0000f; // mvn	r0, #15  -- loads 0xfffffff0
			core1_addr2[1] = 0xe590f000; // ldr	pc, [r0] -- jumps to the address in that address

			printf("[ARM_RUN] %lx\n", arm_run_address);
			if (arm_run_address > 0) {
				core1_trampoline = (volatile void (*)(
						volatile struct ZZ9K_ENV*)) arm_run_address;
				printf("[ARM_RUN] signaling second core.\n");
				Xil_DCacheFlush();
				Xil_ICacheInvalidate();
				core2_execute = 1;
				Xil_DCacheFlush();
				Xil_ICacheInvalidate();
			} else {
				core1_trampoline = 0;
				core2_execute = 0;
			}

			// FIXME move this out of here
			// sequence to reset cpu1 taken from https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18842504/XAPP1079+Latest+Information

			Xil_Out32(XSLCR_UNLOCK_ADDR, XSLCR_UNLOCK_CODE);
			uint32_t RegVal = Xil_In32(A9_CPU_RST_CTRL);
			RegVal |= A9_RST1_MASK;
			Xil_Out32(A9_CPU_RST_CTRL, RegVal);
			RegVal |= A9_CLKSTOP1_MASK;
			Xil_Out32(A9_CPU_RST_CTRL, RegVal);
			RegVal &= ~A9_RST1_MASK;
			Xil_Out32(A9_CPU_RST_CTRL, RegVal);
			RegVal &= ~A9_CLKSTOP1_MASK;
			Xil_Out32(A9_CPU_RST_CTRL, RegVal);
			Xil_Out32(XSLCR_LOCK_ADDR, XSLCR_LOCK_CODE);

			dmb();
			dsb();
			isb();
			asm("sev");
			break;
		case REG_ZZ_ARM_ARGC:
			arm_run_env.argc = zdata;
			break;
		case REG_ZZ_ARM_ARGV0:
			arm_run_env.argv[0] = ((u32) zdata) << 16;
			break;
		case REG_ZZ_ARM_ARGV1:
			arm_run_env.argv[0] |= zdata;
			printf("ARG0 set: %lx\n", arm_run_env.argv[0]);
			break;
		case REG_ZZ_ARM_ARGV2:
			arm_run_env.argv[1] = ((u32) zdata) << 16;
			break;
		case REG_ZZ_ARM_ARGV3:
			arm_run_env.argv[1] |= zdata;
			printf("ARG1 set: %lx\n", arm_run_env.argv[1]);
			break;
		case REG_ZZ_ARM_ARGV4:
			arm_run_env.argv[2] = ((u32) zdata) << 16;
			break;
		case REG_ZZ_ARM_ARGV5:
			arm_run_env.argv[2] |= zdata;
			printf("ARG2 set: %lx\n", arm_run_env.argv[2]);
			break;
		case REG_ZZ_ARM_ARGV6:
			arm_run_env.argv[3] = ((u32) zdata) << 16;
			break;
		case REG_ZZ_ARM_ARGV7:
			arm_run_env.argv[3] |= zdata;
			printf("ARG3 set: %lx\n", arm_run_env.argv[3]);
			break;
		case REG_ZZ_ARM_EV_CODE:
			arm_app_input_event_code = zdata;
			arm_app_input_event_serial++;
			arm_app_input_event_ack = 0;
			break;
*/
		}
	}
}

