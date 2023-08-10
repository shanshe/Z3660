/*
 * rtg.c
 *
 *  Created on: 26 jun. 2022
 *      Author: shanshe
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "zzregs.h"
#include "../main.h"
#include "gfx.h"
#include "zz_video_modes.h"
#include "../video.h"
#include "../interrupt.h"
#include "../ethernet.h"
#include "../adc.h"
#include "../ax.h"
#include "../mp3/mp3.h"
#include "math.h"
#include "sleep.h"
#include "../config_file.h"
#include "../scsi/scsi.h"

#define inline

unsigned int cur_mem_offset = 0x03500000;

//#define XPAR_PROCESSING_AV_SYSTEM_AUDIO_VIDEO_ENGINE_VIDEO_VIDEO_FORMATTER_0_BASEADDR 0x78C20000
#define VIDEO_FORMATTER_BASEADDR XPAR_PROCESSING_AV_SYSTEM_AUDIO_VIDEO_ENGINE_VIDEO_VIDEO_FORMATTER_0_BASEADDR
void write_rtg_register(uint16_t zaddr,uint32_t zdata);

uint32_t gpio=0;
uint16_t ack_request=0;
extern uint16_t flag_cache_flush;
extern int cpu_freq;
uint32_t custom_vmode_param=0;
uint32_t custom_video_mode=ZZVMODE_CUSTOM;
int bm=0,sb=0,ar=0;
void hard_reboot(void);
void write_env_files(int bootmode, int scsiboot, int autoconfig_ram);

// ethernet state
uint32_t ethernet_send_result = 0;
uint32_t eth_backlog_nag_counter=0;
int interrupt_enabled_ethernet = 0;
uint32_t last_interrupt=-1;
uint32_t current_interrupt=0;
// audio state (ZZ9000AX)
static int audio_buffer_collision = 0;
static uint32_t audio_scale = 48000/50;
static uint32_t audio_offset = 0;
int interrupt_enabled_audio = 0;
// audio parameters (buffer locations)

uint32_t audio_params[ZZ_NUM_AUDIO_PARAMS];
int audio_param = 0; // selected parameter
int audio_request_init = 0;

// decoder parameters (mp3 etc)
#define ZZ_NUM_DECODER_PARAMS 4
uint32_t decoder_params[ZZ_NUM_DECODER_PARAMS];
int decoder_param = 0; // selected parameter
int decoder_bytes_decoded = 0;
int max_samples = 0;

//int video_mode = ZZVMODE_1920x1080_50 | 2 << 12 | MNTVA_COLOR_32BIT << 8;
//int video_mode;// = ZZVMODE_800x600 | 2 << 12 | MNTVA_COLOR_32BIT << 8;

// blitter etc
uint16_t rect_x1 = 0;
uint16_t rect_x2 = 0;
uint16_t rect_x3 = 0;
uint16_t rect_y1 = 0;
uint16_t rect_y2 = 0;
uint16_t rect_y3 = 0;
uint16_t blitter_user1 = 0;
uint16_t blitter_user2 = 0;
uint16_t blitter_user3 = 0;
uint16_t blitter_user4 = 0;
uint16_t blitter_src_pitch = 0;
uint16_t blitter_dst_pitch = 0;
uint32_t rect_rgb = 0;
uint32_t rect_rgb2 = 0;
uint32_t debug_lowlevel = 0;
uint16_t original_w = 0;
uint16_t original_h = 0;

uint32_t blitter_src_offset = 0;
uint32_t blitter_dst_offset = 0;
uint32_t blitter_colormode = MNTVA_COLOR_32BIT;
uint32_t blitter_colormode_hibyte = 0;
inline void video_formatter_valign(void) {
	// vertical alignment
	Xil_Out32(VIDEO_FORMATTER_BASEADDR+(MNTVF_OP_VSYNC<<2),0); // OP_VSYNC
}

#define VF_DLY ;

inline void video_formatter_write(uint32_t data, uint16_t op) {
	Xil_Out32(VIDEO_FORMATTER_BASEADDR+(op<<2),data);
}
inline uint32_t video_formatter_read(uint16_t op)
{
	return (Xil_In32(VIDEO_FORMATTER_BASEADDR+(op<<2)));
}

inline void set_palette(uint32_t zdata,uint16_t op_palette)
{
	/*
	uint32_t data;
	uint8_t idx;
	idx=(uint8_t)((zdata>>24)&0xFF);
	data=zdata&0x00FFFFFF;
	printf("set_palette(%d) idx=%d color=%08lx\r\n",op_palette==19,idx,data);
	 */
	Xil_ExceptionDisable();
	video_formatter_write(zdata, op_palette);
	Xil_ExceptionEnable();
}
ZZ_VIDEO_STATE* video_state;
void rtg_init(void)
{
	//	*((uint32_t *)REG_ZZ_FW_VERSION)= (REVISION_MAJOR << 8 | REVISION_MINOR);
	*((uint32_t *)(RTG_BASE+REG_ZZ_FW_VERSION))= swap32(REVISION_MAJOR << 8 | REVISION_MINOR); // BS
	//    *((uint32_t *)(RTG_BASE+REG_ZZ_VBLANK_STATUS))= swap32(0); // this is now read directly from FPGA, so this value is not used anymore
	*((uint32_t *)(RTG_BASE+REG_ZZ_AUDIO_CONFIG))= swap32(1); // AX is present
	*((uint32_t *)(RTG_BASE+REG_ZZ_DECODER_FIFORX))= swap32(0);
	*((uint32_t *)(RTG_BASE+REG_ZZ_CPU_FREQ))= swap32(cpu_freq); // FIXME this is for now fixed at 100MHz
	*((uint32_t *)(RTG_BASE+REG_ZZ_EMULATION_USED))= swap32(config.boot_mode==UAEJIT || config.boot_mode==UAE);
	*((uint32_t *)(RTG_BASE+REG_ZZ_SCSIBOOT_EN))= swap32(config.scsiboot==YES);

	uint8_t* mac = ethernet_get_mac_address_ptr();
	*((uint32_t *)(RTG_BASE+REG_ZZ_ETH_MAC_HI))= swap32((((uint32_t)mac[0])<<8)|mac[1]);
	*((uint32_t *)(RTG_BASE+REG_ZZ_ETH_MAC_LO))= swap32((((uint32_t)mac[2])<<24)|(((uint32_t)mac[3])<<16)|(((uint32_t)mac[4])<<8)|mac[5]);

	printf("RTG init...\r\n");
	video_state->framebuffer_pan_offset=0;

	ethernet_send_result = 0;
	*((uint32_t *)(RTG_BASE+REG_ZZ_ETH_TX))=swap32(ethernet_send_result);
	int frfb=0;
	*((uint32_t *)(RTG_BASE+REG_ZZ_ETH_RX_ADDRESS))= swap32(RX_BACKLOG_ADDRESS+(frfb<<11)); // <<11 = 2048 (FRAME_SIZE)
	eth_backlog_nag_counter = 0;
	interrupt_enabled_ethernet=0;
	interrupt_enabled_audio=0;

	amiga_interrupt_clear(0xFFFFFFFF);
	audio_set_tx_buffer((uint8_t*)(RTG_BASE+AUDIO_TX_BUFFER_ADDRESS));
	/*
    int16_t* adata = (int16_t *)(((void*)RTG_BASE+AUDIO_TX_BUFFER_ADDRESS));
    memset((void*)RTG_BASE+AUDIO_TX_BUFFER_ADDRESS, 0, AUDIO_TX_BUFFER_SIZE);
    float f=1;
    for(int i=0;i<AUDIO_TX_BUFFER_SIZE/2;i++)
    {
    	adata[i] = (sin((float)i/200.0)*65536)*f;
    	f-=0.0001;
    }
	 */
	audio_adau_set_lpf_params(23900);
	for(int i=0;i<10;i++)
		audio_adau_set_eq_gain(i,50);

	*((uint32_t *)(RTG_BASE+REG_ZZ_BOOTMODE))= swap32(config.boot_mode);
}
uint32_t *address;
uint32_t zdata;
uint32_t op_data=0;
uint16_t zaddr;

#ifdef CPU_EMULATOR
#define IDLE_TASK_COUNT_MAX 30000
#else
#define IDLE_TASK_COUNT_MAX 3000000
#endif
int idle_task_count_max=IDLE_TASK_COUNT_MAX;
void other_tasks(void)
{
	static int adc_time_counter=0;
	adc_time_counter++;
	if(adc_time_counter==100000)
		*((uint32_t *)(RTG_BASE+REG_ZZ_TEMPERATURE))= swap32((uint32_t)(xadc_get_temperature()*10.0));
	if(adc_time_counter==200000)
		*((uint32_t *)(RTG_BASE+REG_ZZ_VOLTAGE_AUX))= swap32((uint32_t)(xadc_get_aux_voltage()*100.0));
	if(adc_time_counter==300000)
	{
		*((uint32_t *)(RTG_BASE+REG_ZZ_VOLTAGE_INT))= swap32((uint32_t)(xadc_get_int_voltage()*100.0));
		adc_time_counter=0;
	}
	{
		current_interrupt=amiga_interrupt_get();
		if(last_interrupt != current_interrupt)
		{
			last_interrupt=current_interrupt;
			*((uint32_t *)(RTG_BASE+REG_ZZ_INT_STATUS))= swap32(last_interrupt);
		}
	}
	static int idle_task_count=0;
	if(idle_task_count++> idle_task_count_max)
	{
		idle_task_count=0;
		ethernet_task();
	}

	if(audio_request_init) {
		audio_debug_timer(0);
		audio_init_i2s();
		audio_request_init = 0;
		audio_debug_timer(1);
	}

	// check for queued up ethernet frames and interrupt amiga
	int ethernet_backlog = ethernet_get_backlog();
	if (ethernet_backlog > 0 && eth_backlog_nag_counter > 5000) {
		amiga_interrupt_set(AMIGA_INTERRUPT_ETH);
		eth_backlog_nag_counter = 0;
	}

	if (interrupt_enabled_ethernet && ethernet_backlog > 0) {
		eth_backlog_nag_counter++;
	}

}
void rtg_loop(void)
{
	gpio=read_reg_s01(REG1);
	//	if((gpio&0xC0000000)!=0)
	if((gpio&0x80000000)!=0) //write
	{
		//		if((gpio&0x80000000)!=0) // write cycle
		address=(uint32_t *)((gpio&0xFFFF) + RTG_BASE);

		//			if(zaddr!=0x80)
		//				printf("W %02x %08lx\r\n",zaddr,zdata);
		zaddr=gpio&0xFFFF;

		if(zaddr<0x2000)
		{
			zdata=swap32(*address);
			write_rtg_register(zaddr,zdata);
		}
		else
		{
			zaddr-=0x2000;
			int type=0;
			if((gpio&0x30000000)==0x00000000)
			{
				type=2;
				zdata=swap32(*address);
			}
			else if((gpio&0x30000000)==0x20000000)
			{
				type=1;
				zdata=swap16(*(uint16_t*)address);
			}
			else
				zdata=*(uint8_t*)address;
			handle_piscsi_write(zaddr, zdata, type);
		}
		ack_request=1;
	}
#if 0
	else if((gpio&0x40000000)!=0) //read
	{
		//		if((gpio&0x80000000)!=0) // write cycle
		address=(uint32_t *)((gpio&0xFFFF) + RTG_BASE);

		//			if(zaddr!=0x80)
		//				printf("W %02x %08lx\r\n",zaddr,zdata);
		zaddr=gpio&0xFFFF;
		if(zaddr<0x2000)
		{
			//zdata=swap32(*address);
			//write_rtg_register(zaddr,zdata);
		}
		else
		{
			zaddr-=0x2000;
			int type=0;
			if((gpio&0x30000000)==0x00000000)
			{
				type=2;
			}
			else if((gpio&0x30000000)==0x20000000)
			{
				type=1;
			}

			zdata=handle_piscsi_read(zaddr, type);
			if(type==2)
				*address=swap32(zdata);
			else if(type==1)
				*(uint16_t*)address=swap16(zdata);
			else
				*(uint8_t*)address=zdata;
		}
		ack_request=1;
	}
#endif
	if(ack_request==1)
	{
		DiscreteSet(REG0,READ_WRITE_ACK);

		while(read_reg_s01(REG1)!=0)
		{
			DiscreteClear(REG0,READ_WRITE_ACK);
			DiscreteSet(REG0,READ_WRITE_ACK);
		}
		DiscreteClear(REG0,READ_WRITE_ACK);
		ack_request=0;
	}
	other_tasks();
}

void write_rtg_register(uint16_t zaddr,uint32_t zdata)
{
	switch (zaddr) {
	// Various blitter/video registers
	case REG_ZZ_PAN:
		video_state->framebuffer_pan_offset = zdata;

		// cursor offset support for p96 panning
		video_state->sprite_x_offset = rect_x1;
		video_state->sprite_y_offset = rect_y1;

		video_state->framebuffer_pan_width = rect_x2;
		uint32_t framebuffer_color_format = blitter_colormode;
		video_state->framebuffer_pan_offset += (rect_x1 << blitter_colormode);
		video_state->framebuffer_pan_offset += (rect_y1 *(video_state->framebuffer_pan_width<<framebuffer_color_format));
		break;
	case REG_ZZ_ORIG_RES:
		original_w=(zdata>>16)&0xFFFF;
		original_h=(zdata    )&0xFFFF;
		break;
	case REG_ZZ_BLIT_SRC:
		blitter_src_offset = zdata;
		break;
	case REG_ZZ_BLIT_DST:
		blitter_dst_offset = zdata;
		break;

	case REG_ZZ_COLORMODE:
		blitter_colormode = zdata & 0x0f;
		blitter_colormode_hibyte = zdata >> 8;
		// hack to use 16 bit gfx ops with 15 bit
		if (blitter_colormode == MNTVA_COLOR_15BIT) {
			blitter_colormode = MNTVA_COLOR_16BIT565;
		}
		break;
	case REG_ZZ_CONFIG:
		// enable/disable INT6, currently used to signal incoming ethernet packets
		//				printf("[enable] eth:0x%lx\n", zdata);
		if (zdata & 8) {
			// clear/ack
			if (zdata & 16) {
				amiga_interrupt_clear(AMIGA_INTERRUPT_ETH);
			}
			if (zdata & 32) {
				amiga_interrupt_clear(AMIGA_INTERRUPT_AUDIO);
			}
		} else {
			//printf("[enable] eth: %d\n", (int)zdata);
			interrupt_enabled_ethernet = zdata & 1;

			if (!interrupt_enabled_ethernet) {
				amiga_interrupt_clear(AMIGA_INTERRUPT_ETH);
			}
		}
		*((uint32_t *)(RTG_BASE+REG_ZZ_INT_STATUS))= swap32(amiga_interrupt_get());
		break;
	case REG_ZZ_MODE: {
		//printf("mode change: %lx\n", zdata);

		int mode = zdata & 0xff;
		int colormode = (zdata & 0xf00) >> 8;
		int scalemode = (zdata & 0xf000) >> 12;
		printf("mode: %d color: %d scale: %d\n", mode, colormode, scalemode);
		video_mode_init(mode, scalemode, colormode);

		// FIXME
		// remember selected video mode
		// video_mode = zdata;
		break;
	}
	case REG_ZZ_SPRITE_X:
	case REG_ZZ_SPRITE_Y:
		if (!video_state->sprite_showing)
			break;

		video_state->sprite_x_base = (int16_t)rect_x1;
		video_state->sprite_y_base = (int16_t)rect_y1;
		update_hw_sprite_pos();

		break;
	case REG_ZZ_SPRITE_BITMAP: {
		if (zdata == 1) { // Hardware sprite enabled
			hw_sprite_show(1);
			break;
		}
		else if (zdata == 2) { // Hardware sprite disabled
			hw_sprite_show(0);
			break;
		}
//double
		uint8_t* bmp_data = (uint8_t*) (((uint32_t) video_state->framebuffer) + blitter_src_offset);

		video_state->sprite_x_offset = rect_x1;
		video_state->sprite_y_offset = rect_y1;
		video_state->sprite_width  = rect_x2;
		video_state->sprite_height = rect_y2;
		int double_sprite = rect_x3;
		clear_hw_sprite();
		update_hw_sprite(bmp_data, double_sprite);
		update_hw_sprite_pos();
		break;
	}
	case REG_ZZ_SPRITE_COLORS: {
		video_state->sprite_colors[zdata] = (blitter_user1 << 16) | blitter_user2;
		if (zdata != 0 && video_state->sprite_colors[zdata] == 0xff00ff)
			video_state->sprite_colors[zdata] = 0xfe00fe;
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
		break;
	case REG_ZZ_X2:
		rect_x2 = zdata;
		break;
	case REG_ZZ_Y2:
		rect_y2 = zdata;
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
		rect_rgb  = swap32(zdata);
		//				rect_rgb  = zdata;
		break;
	case REG_ZZ_RGB2:
		rect_rgb2  = swap32(zdata);
		//				rect_rgb2  = zdata;
		break;

		// Generic acceleration ops
	case REG_ZZ_ACC_OP: {
		handle_acc_op(zdata);
		break;
	}
	case REG_ZZ_FW_VERSION: // this is readonly, so restore its value if it is written
		*((uint32_t *)REG_ZZ_FW_VERSION)= (REVISION_MAJOR << 24 | REVISION_MINOR << 16);
		break;
		// DMA RTG rendering
	case REG_ZZ_BLITTER_DMA_OP: {
		handle_blitter_dma_op(video_state,zdata);
		break;
	}

	// RTG rendering
	case REG_ZZ_FILLRECT:
		//			printf("FILLRECT blitter_dst_offset %lx\r\n",blitter_dst_offset);
		//			printf("         %d %d %d %d \r\n",rect_x1,rect_y1,rect_x2,rect_y2);
		set_fb((uint32_t*) (((uint32_t) video_state->framebuffer) + blitter_dst_offset),
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
		set_fb((uint32_t*) (((uint32_t) video_state->framebuffer) + blitter_dst_offset),
				blitter_dst_pitch);
		mask = blitter_colormode_hibyte;

		switch (zdata) {
		case 1: // Regular BlitRect
			if (mask == 0xFF || (mask != 0xFF && (blitter_colormode != MNTVA_COLOR_8BIT)))
				copy_rect_nomask(rect_x1, rect_y1, rect_x2, rect_y2, rect_x3,
						rect_y3, blitter_colormode,
						(uint32_t*) (((uint32_t) video_state->framebuffer)
								+ blitter_dst_offset),
								blitter_dst_pitch, MINTERM_SRC);
			else
				copy_rect(rect_x1, rect_y1, rect_x2, rect_y2, rect_x3,
						rect_y3, blitter_colormode,
						(uint32_t*) (((uint32_t) video_state->framebuffer)
								+ blitter_dst_offset),
								blitter_dst_pitch, mask);
			break;
		case 2: // BlitRectNoMaskComplete
			copy_rect_nomask(rect_x1, rect_y1, rect_x2, rect_y2, rect_x3,
					rect_y3, blitter_colormode,
					(uint32_t*) (((uint32_t) video_state->framebuffer)
							+ blitter_src_offset),
							blitter_src_pitch, mask); // Mask in this case is minterm/opcode.
			break;
		}

		break;
	}

	case REG_ZZ_FILLTEMPLATE: {
		uint8_t draw_mode = blitter_colormode_hibyte;
		uint8_t* tmpl_data = (uint8_t*) (((uint32_t) video_state->framebuffer)
				+ blitter_src_offset);
		set_fb((uint32_t*) (((uint32_t) video_state->framebuffer) + blitter_dst_offset),
				blitter_dst_pitch);

		uint8_t bpp = 2 * blitter_colormode;
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
			pattern_fill_rect(blitter_colormode, rect_x1,
					rect_y1, rect_x2, rect_y2, draw_mode, mask,
					rect_rgb, rect_rgb2, rect_x3, rect_y3, tmpl_data,
					blitter_src_pitch, loop_rows);
		}
		else {
			template_fill_rect(blitter_colormode, rect_x1,
					rect_y1, rect_x2, rect_y2, draw_mode, mask,
					rect_rgb, rect_rgb2, rect_x3, rect_y3, tmpl_data,
					blitter_src_pitch);
		}

		break;
	}
	/*
				case REG_ZZ_SCRATCH_COPY: { // Copy crap from scratch area
					for (int i = 0; i < rect_y1; i++) {
						memcpy	((uint32_t*) (((uint32_t) video_state->framebuffer) + video_state->frameBuf_pan_offset + (i * rect_x1)),
								 (uint32_t*) ((uint32_t)Z3_SCRATCH_ADDR + (i * rect_x1)),
								 rect_x1);
					}
					break;
				}
*/
/* TODO: custom video mode
	case ZZ_CUSTOM_VIDMODE: // Custom video mode param
		custom_vmode_param = zdata;
		break;

	case ZZ_CUSTOM_VIDMODE_DATA: { // Custom video mode data
		switch(custom_vmode_param) {
			case VMODE_PARAM_HRES:     	preset_video_modes[custom_video_mode].hres=zdata;     break;
			case VMODE_PARAM_VRES:     	preset_video_modes[custom_video_mode].vres=zdata;     break;
			case VMODE_PARAM_HSTART:   	preset_video_modes[custom_video_mode].hstart=zdata;   break;
			case VMODE_PARAM_HEND:     	preset_video_modes[custom_video_mode].hend=zdata;     break;
			case VMODE_PARAM_HMAX:     	preset_video_modes[custom_video_mode].hmax=zdata;     break;
			case VMODE_PARAM_VSTART:   	preset_video_modes[custom_video_mode].vstart=zdata;   break;
			case VMODE_PARAM_VEND:     	preset_video_modes[custom_video_mode].vend=zdata;     break;
			case VMODE_PARAM_VMAX:     	preset_video_modes[custom_video_mode].vmax=zdata;     break;
			case VMODE_PARAM_POLARITY: 	preset_video_modes[custom_video_mode].polarity=zdata; break;
			case VMODE_PARAM_PHZ: {
				float phz=zdata;
				if(phz<1e6)
					return;
				uint32_t mul=50,mul_temp=50;
				uint32_t div=1;
				uint32_t div2=1;
				float min_error=1e6;
				// explore best solution
				do {
					for(int div1=1;div1<=4;div1++)
					{
						int div2_temp=mul_temp*100.e6/(div1*phz);
						if((div2_temp>0)&&(div2_temp<64))
						{
							float error=fabs(phz-mul_temp*100.e6/(div1*div2_temp));
							if(error<min_error)
							{
								min_error=error;
								mul=mul_temp;
								div=div1;
								div2=div2_temp;
							}
						}
					}
				}
				while(--mul_temp>5);
				uint32_t phz_int=mul*100.e6/(div*div2);
				printf("Best mode: PixelClock=%d,mul=%d, div=%d, div2=%d\n",phz_int,mul,div,div2);
				preset_video_modes[custom_video_mode].phz=phz_int;
				preset_video_modes[custom_video_mode].mhz=(int16_t)(phz_int*1.e-6+0.5);
				preset_video_modes[custom_video_mode].vhz=60;
				preset_video_modes[custom_video_mode].hdmi=0;

				preset_video_modes[custom_video_mode].mul=mul;
				preset_video_modes[custom_video_mode].div=div;
				preset_video_modes[custom_video_mode].div2=div2;
			}
			break;
			default: break;
		}
		break;
	}
*/
/*
				case 0x56: // Set custom video mode index
					custom_video_mode = zdata;
					break;

				case 0x58: // Set custom video mode without any questions asked.
					// This assumes that the custom video mode is 640x480 or higher resolution.
					video_mode_init(custom_video_mode, scalemode, colormode);
					break;
*/
	case REG_ZZ_SET_FEATURE:
		switch (blitter_user1) {
		case CARD_FEATURE_SECONDARY_PALETTE:
			printf("[feature] SECONDARY_PALETTE: %lu\n",zdata);
			// Enables/disables the secondary palette on screen split with P96 3.10+
			video_state->card_feature_enabled[CARD_FEATURE_SECONDARY_PALETTE] = zdata;
			break;
			/*                case CARD_FEATURE_NONSTANDARD_VSYNC:
							printf("[feature] NONSTANDARD_VSYNC: %lu\n",zdata);
							// Enables/disables the nonstandard refresh rates for scandoubled PAL/NTSC HDMI output modes.
							if (zdata == 2) {
								video_state->scandoubler_mode_adjust = 2;
							} else {
								video_state->scandoubler_mode_adjust = 0;
							}
							video_state->card_feature_enabled[CARD_FEATURE_NONSTANDARD_VSYNC] = zdata;
							break;*/
		default:
			break;
		}
		break;
		case REG_ZZ_P2C: {
			uint8_t draw_mode = blitter_colormode_hibyte;
			uint8_t planes = (zdata & 0xFF00) >> 8;
			uint8_t mask = (zdata & 0xFF);
			uint8_t layer_mask = blitter_user2;
			uint8_t* bmp_data = (uint8_t*) (((uint32_t) video_state->framebuffer)
					+ blitter_src_offset);

			set_fb((uint32_t*) (((uint32_t) video_state->framebuffer) + blitter_dst_offset),
					blitter_dst_pitch);

			p2c_rect(rect_x1, 0, rect_x2, rect_y2, rect_x3,
					rect_y3, draw_mode, planes, mask,
					layer_mask, blitter_src_pitch, bmp_data);
			break;
		}

		case REG_ZZ_P2D: {
			uint8_t draw_mode = blitter_colormode_hibyte;
			uint8_t planes = (zdata & 0xFF00) >> 8;
			uint8_t mask = (zdata & 0xFF);
			uint8_t layer_mask = blitter_user2;
			uint8_t* bmp_data = (uint8_t*) (((uint32_t) video_state->framebuffer)
					+ blitter_src_offset);

			set_fb((uint32_t*) (((uint32_t) video_state->framebuffer) + blitter_dst_offset),
					blitter_dst_pitch);
			p2d_rect(rect_x1, 0, rect_x2, rect_y2, rect_x3,
					rect_y3, draw_mode, planes, mask, layer_mask, rect_rgb,
					blitter_src_pitch, bmp_data, blitter_colormode);
			break;
		}

		case REG_ZZ_DRAWLINE: {
			uint8_t draw_mode = blitter_colormode_hibyte;
			set_fb((uint32_t*) (((uint32_t) video_state->framebuffer) + blitter_dst_offset),
					blitter_dst_pitch);

			// rect_x3 contains the pattern. if all bits are set for both the mask and the pattern,
			// there's no point in passing non-essential data to the pattern/mask aware function.

			if (rect_x3 == 0xFFFF && zdata == 0xFF)
				draw_line_solid(rect_x1, rect_y1, rect_x2, rect_y2,
						blitter_user1, rect_rgb,
						blitter_colormode);
			else
				draw_line(rect_x1, rect_y1, rect_x2, rect_y2,
						blitter_user1, rect_x3, rect_y3, rect_rgb,
						rect_rgb2, blitter_colormode, zdata,
						draw_mode);
			break;
		}

		case REG_ZZ_INVERTRECT:
			set_fb((uint32_t*) (((uint32_t) video_state->framebuffer) + blitter_dst_offset),
					blitter_dst_pitch);
			invert_rect(rect_x1, rect_y1, rect_x2, rect_y2,
					zdata & 0xFF, blitter_colormode);
			break;

		case REG_ZZ_SET_SPLIT_POS:
			video_state->bgbuf_offset = blitter_src_offset;
			video_state->split_request_pos = zdata;
			break;

			// Ethernet
		case REG_ZZ_ETH_TX:
			ethernet_send_result = ethernet_send_frame(zdata);
			*((uint32_t *)(RTG_BASE+REG_ZZ_ETH_TX))=swap32(ethernet_send_result);
			//				printf("SEND frame sz: %ld res: %ld\n",zdata,ethernet_send_result);
			break;
		case REG_ZZ_ETH_RX: {
			//				printf("RECV eth frame sz: %ld\n",zdata);
			int frfb = ethernet_receive_frame();
			*((uint32_t *)(RTG_BASE+REG_ZZ_ETH_RX_ADDRESS))= swap32(RX_BACKLOG_ADDRESS+(frfb<<11)); // <<11 = 2048 (FRAME_SIZE)
			//				printf("REG_ZZ_ETH_RX_ADDRESS=0x%08x\n",RX_BACKLOG_ADDRESS+(frfb<<11));
			//				mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG4, frfb);
			break;
		}
		case REG_ZZ_ETH_MAC_HI: {
			uint8_t* mac = ethernet_get_mac_address_ptr();
			mac[0] = (zdata & 0xff00) >> 8;
			mac[1] = (zdata & 0x00ff);
			break;
		}
		case REG_ZZ_ETH_MAC_LO: {
			uint8_t* mac = ethernet_get_mac_address_ptr();
			mac[2] = (zdata & 0xff000000) >>24;
			mac[3] = (zdata & 0x00ff0000) >>16;
			mac[4] = (zdata & 0x0000ff00) >> 8;
			mac[5] = (zdata & 0x000000ff);
			ethernet_update_mac_address();
			break;
		}
		case REG_ZZ_AUDIO_SWAB:
		{
			int byteswap = 1;
			if (zdata&(1<<15)) byteswap = 0;
			audio_offset = (zdata&0x7fff)<<8; // *256
			audio_buffer_collision = audio_swab(audio_scale, audio_offset, byteswap);
			*((uint32_t *)(RTG_BASE+REG_ZZ_AUDIO_SWAB))= swap32(audio_buffer_collision);
			//					printf("audio_offset 0x%08lx\n",audio_offset);
			break;
		}
		case REG_ZZ_AUDIO_SCALE:
			audio_scale = zdata;
			break;
			/*			case REG_ZZ_UNUSED_REG8C:
					// set up a test (set sleep time, and set counter to 0)
					zz_debug_test_ms = zdata;
					zz_debug_test_counter = 0;
					zz_debug_test_prev = 0;
					printf("[zzdebug] test reset, time: %lu\n", zz_debug_test_ms);
					break;

				case REG_ZZ_UNUSED_REG8E:
					// increase counter by one and compare with the number we are sent
					if (zdata > 0 && zz_debug_test_prev != zdata-1) {
						printf("[zzdebug] loss! zdata: %lu prev: %lu counter: %lu\n", zdata, zz_debug_test_prev, zz_debug_test_counter);
					}
					usleep(zz_debug_test_ms*1000);
					zz_debug_test_counter++;
					zz_debug_test_prev = zdata;
					break;
			 */
		case REG_ZZ_AUDIO_PARAM:
			printf("[REG_ZZ_AUDIO_PARAM] %ld\n", zdata);

			// AUDIO PARAMS:
			// 0: tx buffer offset
			// 1: rx buffer offset
			// 2: dsp program offset
			// 3: dsp params offset
			// 8: dsp upload program + params or params only (length in zdata)
			// 9: dsp set lowpass filter
			// 10: dsp set volumes

			if (zdata<ZZ_NUM_AUDIO_PARAMS) {
				audio_param = zdata;
				*((uint32_t *)(RTG_BASE+REG_ZZ_AUDIO_VAL))= swap32(audio_params[audio_param]); // read param
			} else {
				audio_param = -1;
			}
			break;
		case REG_ZZ_AUDIO_VAL:
			if(audio_param>=0) {
				printf("[REG_ZZ_AUDIO_VAL] %lx\n", zdata);

				audio_params[audio_param] = zdata;
				if (audio_param == AP_TX_BUF_OFFS) {

					uint8_t* addr = (uint8_t*)(audio_params[AP_TX_BUF_OFFS]);
					if (((uint32_t)addr)>=(AUDIO_TX_BUFFER_ADDRESS*0+0x06000000) && ((uint32_t)addr)<TX_BD_LIST_START_ADDRESS) {
						audio_set_tx_buffer(RTG_BASE+addr);
						audio_request_init = 1;
					} else {
						printf("[audio] illegal tx address: %p\n", addr);
					}
					/*
						} else if (audio_param == AP_RX_BUS_OFFS) {

							uint8_t* addr = (uint8_t*)(audio_params[AP_RX_BUS_OFFS]);
							if (((uint32_t)addr)>=(AUDIO_RX_BUFFER_ADDRESS*0+0x06000000) && ((uint32_t)addr)<TX_BD_LIST_START_ADDRESS) {
								audio_set_rx_buffer((RTG_BASE+addr);
								audio_request_init = 1;
							} else {
								printf("[audio] illegal rx address: 0x%p\n", addr);
							}
					 */
				} else if (audio_param == AP_DSP_UPLOAD) {
					uint8_t* program_ptr = (uint8_t*)audio_params[AP_DSP_PROG_OFFS];


					uint8_t* params_ptr = (uint8_t*)audio_params[AP_DSP_PARAM_OFFS];

					if (zdata == 0) {
						printf("[audio] reprogramming from 0x%p and 0x%p\n", program_ptr, params_ptr);
						//							audio_program_adau(program_ptr, 5120);
						//							audio_program_adau_params(params_ptr, 4096);
					} else {
						printf("[audio] programming %ld params from 0x%p\n", zdata, params_ptr);
						//							audio_program_adau_params(params_ptr, zdata);
					}
				} else if (audio_param == AP_DSP_SET_LOWPASS) {
					// set lowpass filter params by cutoff freq (works only if default program is loaded!)
					audio_adau_set_lpf_params(zdata);
				} else if (audio_param == AP_DSP_SET_VOLUMES) {
					audio_adau_set_mixer_vol(zdata&0xff, (zdata>>8)&0xff);
				} else if (audio_param == AP_DSP_SET_PREFACTOR) {
					audio_adau_set_prefactor(zdata);
				} else if ((audio_param >= AP_DSP_SET_EQ_BAND1) && (audio_param <= AP_DSP_SET_EQ_BAND10)) {
					audio_adau_set_eq_gain(audio_param-AP_DSP_SET_EQ_BAND1, zdata);
				} else if (audio_param == AP_DSP_SET_STEREO_VOLUME) {
					audio_adau_set_vol_pan(zdata&0xff, (zdata>>8)&0xff);
				}
			}
			break;
		case REG_ZZ_AUDIO_CONFIG: {
			// audio config
			audio_set_interrupt_enabled((int)(zdata&1));
			*((uint32_t *)(RTG_BASE+REG_ZZ_AUDIO_CONFIG))= swap32(1); // AX is present
			break;
		}
		case REG_ZZ_DECODER_PARAM:
			if (zdata<ZZ_NUM_DECODER_PARAMS) {
				decoder_param = zdata;
			} else {
				decoder_param = -1;
			}
			break;
		case REG_ZZ_DECODER_VAL:
			if(decoder_param>=0)
			{
				decoder_params[decoder_param] = zdata;
			}
			break;
		case REG_ZZ_DECODER_FIFOTX:
		{
			fifo_set_write_index(zdata);
			//					printf("[decode:fifotx:%ld]\n",zdata);
		}
		break;
		case REG_ZZ_DECODE:
		{
			// DECODER PARAMS:
			// 0: input buffer offset
			// 1: input buffer size
			// 2: output buffer offset
			// 3: output buffer size

			uint8_t* input_buffer = (uint8_t*)(RTG_BASE+decoder_params[0]);

			size_t input_buffer_size = decoder_params[1];

			uint8_t* output_buffer = (uint8_t*)(RTG_BASE+decoder_params[2]);

			size_t output_buffer_size = decoder_params[3];

			switch(zdata) {
			case DECODE_CLEAR_FIFO:
				printf("[decode:clear]\n");
				fifo_clear();
				break;
			case DECODE_INIT: // this is used by axmp3
				printf("[decode:mp3:%s] %p (%d) -> %p (%d)\n", decode_command_str[(int)zdata], input_buffer, input_buffer_size,
						output_buffer, output_buffer_size);
				decode_mp3_init(input_buffer, input_buffer_size);
				decoder_bytes_decoded = -1;
				break;
			case DECODE_INIT_FIFO: // this is used by mhi
				printf("[decode:mp3:%s] %p (%d) -> %p (%d)\n", decode_command_str[(int)zdata], input_buffer, input_buffer_size,
						output_buffer, output_buffer_size);
				decode_mp3_init_fifo(input_buffer, input_buffer_size);
				decoder_bytes_decoded = -1;
				break;
			case DECODE_RUN: {
				max_samples = output_buffer_size;
				int mp3_freq = mp3_get_hz();
				if (mp3_freq != 48000) {
					uint8_t* temp_buffer = output_buffer + AUDIO_TX_BUFFER_SIZE; // FIXME hack
					max_samples = mp3_freq/50 * 2;

					decoder_bytes_decoded = decode_mp3_samples(temp_buffer, max_samples);

					// resample
					if(decoder_bytes_decoded>0)
					{
						resample_s16((int16_t*)temp_buffer, (int16_t*)output_buffer,
								mp3_freq, 48000, AUDIO_BYTES_PER_PERIOD / 4);
					}
				} else {
					decoder_bytes_decoded = decode_mp3_samples(output_buffer, max_samples);
				}
				//							if(decoder_bytes_decoded>0)
				//								printf("[decode:mp3:%s] %p (%d) -> %p (%d) %ld %ld\n", decode_command_str[(int)zdata], input_buffer, input_buffer_size,
				//									output_buffer, output_buffer_size,fifo_get_read_index(),swap32(*((uint32_t *)(RTG_BASE+REG_ZZ_DECODER_FIFOTX))));
			}
			break;
			}
			*((uint32_t *)(RTG_BASE+REG_ZZ_DECODER_FIFORX))= swap32(fifo_get_read_index());
			//					printf("[fifo:fiforx:%d]\n",fifo_get_read_index());
			// this is used only by axmp3 app
			*((uint32_t *)(RTG_BASE+REG_ZZ_DECODE))= swap32(decoder_bytes_decoded);
			break;
		}
		/*
				case REG_ZZ_USBBLK_TX_HI: {
					usb_storage_write_block = ((uint32_t) zdata) << 16;
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
					usb_storage_read_block = ((uint32_t) zdata) << 16;
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
		case REG_ZZ_CPU_FREQ:
			printf("[REG_ZZ_CPU_FREQ] %ld MHz\n", zdata);
			if((zdata>50) && (zdata<100))
				configure_clk(zdata, 0, 1, 1);
			break;
			/*
				// ARM core 2 execution
				case REG_ZZ_ARM_RUN_HI:
					arm_run_address = ((uint32_t) zdata) << 16;
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
					arm_run_env.argv[0] = ((uint32_t) zdata) << 16;
					break;
				case REG_ZZ_ARM_ARGV1:
					arm_run_env.argv[0] |= zdata;
					printf("ARG0 set: %lx\n", arm_run_env.argv[0]);
					break;
				case REG_ZZ_ARM_ARGV2:
					arm_run_env.argv[1] = ((uint32_t) zdata) << 16;
					break;
				case REG_ZZ_ARM_ARGV3:
					arm_run_env.argv[1] |= zdata;
					printf("ARG1 set: %lx\n", arm_run_env.argv[1]);
					break;
				case REG_ZZ_ARM_ARGV4:
					arm_run_env.argv[2] = ((uint32_t) zdata) << 16;
					break;
				case REG_ZZ_ARM_ARGV5:
					arm_run_env.argv[2] |= zdata;
					printf("ARG2 set: %lx\n", arm_run_env.argv[2]);
					break;
				case REG_ZZ_ARM_ARGV6:
					arm_run_env.argv[3] = ((uint32_t) zdata) << 16;
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

		case REG_ZZ_OP_DATA: // idx + RGB data
			op_data=zdata;
			//        printf("1000 <= 0x%08X\r\n",zdata);
			break;
		case REG_ZZ_OP:
			//        printf("1004 <= %d\r\n",zdata);
			if(zdata==OP_PALETTE)
			{
				set_palette(op_data,OP_PALETTE);
			}
			else if(zdata==OP_PALETTE_HI)
			{
				set_palette(op_data,OP_PALETTE_HI);
			}
			break;
		case REG_ZZ_OP_NOP:
			//        printf("1008 <= %d\r\n",zdata);
			break;
		case REG_ZZ_OP_CAPTUREMODE:
			printf("CAPTUREMODE <= %ld\r\n",zdata);
			//				zz_set_monswitch(!zdata);
			break;
		case REG_ZZ_JIT_ENABLE:
			printf("[REG_ZZ_JIT_ENABLE] %ld\n", zdata);
			shared->jit_enabled=zdata;
			break;
		case REG_ZZ_BOOTMODE:
			if(zdata>=0 && zdata<BOOTMODE_NUM)
			{
				printf("BOOTMODE %ld (%s)\r\n",zdata,bootmode_names[zdata]);
				bm=zdata;
			}
			else
			{
				printf("BOOTMODE %ld unknown\r\n",zdata);
				bm=0;
			}
			break;
		case REG_ZZ_APPLY_BOOTMODE:
			if(zdata>=0x55AA)
			{
				printf("Apply BOOTMODE %d (%s)\r\n",bm,bootmode_names[bm]);
				write_env_files(bm,sb,ar);
				hard_reboot();
			}
			else
			{
				printf("Apply BOOTMODE magic code not valid: 0x%lx\r\n",zdata);
			}
			break;
		case REG_ZZ_SCSIBOOT_EN:
			sb=zdata;
			printf("SCSI BOOT %s\r\n",zdata?"enabled":"disabled");
			break;
		case REG_ZZ_AUTOC_RAM_EN:
			ar=zdata;
			printf("AUTOCONFIG RAM %s\r\n",zdata?"enabled":"disabled");
			break;
		default:
			printf("W %02x\r\n",zaddr); // write to an unknown RTG register
			break;
	}
}

void audio_reset(void)
{
	audio_set_interrupt_enabled(0);
	audio_silence();
	audio_init_i2s();
}
