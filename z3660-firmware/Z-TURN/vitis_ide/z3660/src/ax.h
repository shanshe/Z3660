/*
 * ax.h
 *
 *  Created on: 27 sept. 2022
 *      Author: shanshe
 */

#ifndef SRC_AX_H_
#define SRC_AX_H_

#include <stdint.h>
enum {
	AP_TX_BUF_OFFS,           // 0
	AP_RX_BUS_OFFS,           // 1
	AP_DSP_PROG_OFFS,         // 2
	AP_DSP_PARAM_OFFS,        // 3
	AP_DSP_UPLOAD=8,          // 8
	AP_DSP_SET_LOWPASS,       // 9
	AP_DSP_SET_VOLUMES,       // 10
	AP_DSP_SET_PREFACTOR,     // 11
	AP_DSP_SET_EQ_BAND1,      // 12
	AP_DSP_SET_EQ_BAND2,      // 13
	AP_DSP_SET_EQ_BAND3,      // 14
	AP_DSP_SET_EQ_BAND4,      // 15
	AP_DSP_SET_EQ_BAND5,      // 16
	AP_DSP_SET_EQ_BAND6,      // 17
	AP_DSP_SET_EQ_BAND7,      // 18
	AP_DSP_SET_EQ_BAND8,      // 19
	AP_DSP_SET_EQ_BAND9,      // 20
	AP_DSP_SET_EQ_BAND10,     // 21
	AP_DSP_SET_STEREO_VOLUME, // 22
	ZZ_NUM_AUDIO_PARAMS       // 23

};
int audio_adau_init(int program_dsp);
void audio_init_i2s();
void isr_audio_tx(void *dummy);

void audio_set_interrupt_enabled(int en);
void audio_clear_interrupt();
uint32_t audio_get_interrupt();
uint32_t audio_get_dma_transfer_count();
int audio_swab(uint16_t audio_buf_samples, uint32_t offset, int byteswap);
void audio_set_tx_buffer(uint8_t* addr);
void audio_set_rx_buffer(uint8_t* addr);

void resample_s16(int16_t *input, int16_t *output,
		int in_sample_rate, int out_sample_rate, int output_samples);
void audio_silence();
void audio_debug_timer(int zdata);

void audio_program_adau(u8* program, uint32_t program_len);
void audio_program_adau_params(u8* params, uint32_t param_len);
void audio_adau_set_lpf_params(int f0);

// vol range: 0-100. 50 = 0db
void audio_adau_set_mixer_vol(int vol1, int vol2);
void audio_adau_set_prefactor(int vol);
void audio_adau_set_vol_pan(int vol, int pan);
void audio_adau_set_eq_gain(int band, double gain);

void audio_reset(void);

#define JUSTIFY_ENABLE 1
#define JUSTIFY_DISABLE 0

#endif /* SRC_AX_H_ */
