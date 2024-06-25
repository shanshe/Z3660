#include <stdio.h>

#include "platform.h"
#include "xi2stx.h"
#include "xaudioformatter.h"
#include "interrupt.h"
#include "sleep.h"
#include <stdlib.h>
#include "ax.h"
#include "memorymap.h"
#include "xtime_l.h"
#include <math.h>
#include "ax.h"

#include "debug_console.h"
#include "rtg/gfx.h"
#include "main.h"

extern DEBUG_CONSOLE debug_console;
void DEBUG_AUDIO(const char *format, ...);

float cut_freq=2.*M_PI*23900.;
float a1[11]={0};
float a2[11]={0};

float b0[11]={0};
float b1[11]={0};
float b2[11]={0};
void lowpass_filter(int16_t *output, int output_samples);
void equalizer(int16_t *output, int output_samples);

int32_t preamp=64;
float vl=1.0,vr=1.0;

XI2s_Tx i2s;
XAudioFormatter audio_formatter;

static uint8_t* audio_tx_buffer = (uint8_t*)(RTG_BASE+AUDIO_TX_BUFFER_ADDRESS);
//static uint8_t* audio_rx_buffer = (uint8_t*)(RTG_BASE+AUDIO_RX_BUFFER_ADDRESS);

static __inline__ __attribute__((always_inline)) int32_t ssat16(int32_t a)
{
    int32_t x = 0;
    __asm__ ("ssat %0, #16, %1" : "=r"(x) : "r"(a));
    return(x);
}

void audio_init_i2s() {
   XI2stx_Config* i2s_config = XI2s_Tx_LookupConfig(XPAR_XI2STX_0_DEVICE_ID);
   int status = XI2s_Tx_CfgInitialize(&i2s, i2s_config, i2s_config->BaseAddress);

   DEBUG_AUDIO("[adau] I2S_TX cfg status: %d\n", status);
   DEBUG_AUDIO("[adau] I2S Dwidth: %d\n", i2s.Config.DWidth);
   DEBUG_AUDIO("[adau] I2S MaxNumChannels: %d\n", i2s.Config.MaxNumChannels);

   XI2s_Tx_JustifyEnable(&i2s, JUSTIFY_ENABLE);
   XI2s_Tx_Justify(&i2s, XI2S_TX_JUSTIFY_LEFT);

   XI2s_Tx_SetSclkOutDiv(&i2s, (384*48)>>JUSTIFY_ENABLE, 48);

   XAudioFormatter_Config* af_config = XAudioFormatter_LookupConfig(XPAR_XAUDIOFORMATTER_0_DEVICE_ID);
   audio_formatter.BaseAddress = af_config->BaseAddress;

   status = XAudioFormatter_CfgInitialize(&audio_formatter, af_config);

   DEBUG_AUDIO("[adau] AudioFormatter cfg status: %d\n", status);

   // reset the goddamn register
   XAudioFormatter_WriteReg(audio_formatter.BaseAddress,
         XAUD_FORMATTER_CTRL + XAUD_FORMATTER_MM2S_OFFSET, 0);

   XAudioFormatterHwParams af_params;
   af_params.buf_addr = (uint32_t)audio_tx_buffer;
   af_params.bits_per_sample = BIT_DEPTH_16;
   af_params.periods = AUDIO_NUM_PERIODS; // 1 second = 192000 bytes
   af_params.active_ch = 2;
   // must be multiple of 32*channels = 64
   af_params.bytes_per_period = AUDIO_BYTES_PER_PERIOD;

   XAudioFormatterSetFsMultiplier(&audio_formatter, (384*48)>>JUSTIFY_ENABLE, 48); // mclk = 256 * Fs // this doesn't really seem to change anything?!
   XAudioFormatterSetHwParams(&audio_formatter, &af_params);
//   XAudioFormatter_InterruptDisable(&audio_formatter, 1<<14); // timeout
//   XAudioFormatter_InterruptDisable(&audio_formatter, XAUD_CTRL_IOC_IRQ_MASK);

   DEBUG_AUDIO("[adau] XAudioFormatter_InterruptEnable...\n");

   XAudioFormatter_InterruptEnable(&audio_formatter, XAUD_CTRL_IOC_IRQ_MASK); // Interrupt On Complete

   DEBUG_AUDIO("[adau] XI2s_Tx_Enable...\n");
   XI2s_Tx_Enable(&i2s, 1);

   DEBUG_AUDIO("[adau] XAudioFormatterDMAStart...\n");
   XAudioFormatterDMAStart(&audio_formatter);
   DEBUG_AUDIO("[adau] XAudioFormatterDMAStart done.\n");
}

// returns 1 if adau1701 found, otherwise 0
// set audio_tx_buffer before!
int audio_adau_init(int program_dsp) {

   return(1);
}

extern int interrupt_enabled_audio;

XTime debug_time_start = 0;

void audio_debug_timer(int zdata) {
   if (zdata == 0) {
      XTime_GetTime(&debug_time_start);
   } else {
      XTime debug_time_stop;
      XTime_GetTime(&debug_time_stop);
      DEBUG_AUDIO("%x;%09.2f us\n", (uint8_t)zdata,
            1.0 * (debug_time_stop-debug_time_start) / (COUNTS_PER_SECOND/1000000));
      XTime_GetTime(&debug_time_start);
   }
}

//int isra_count = 0;

// audio formatter interrupt, triggered whenever a period is completed
void isr_audio_tx(void *dummy) {
   uint32_t val = XAudioFormatter_ReadReg(XPAR_XAUDIOFORMATTER_0_BASEADDR, XAUD_FORMATTER_STS + XAUD_FORMATTER_MM2S_OFFSET);
   val |= (1<<31); // clear irq
   XAudioFormatter_WriteReg(XPAR_XAUDIOFORMATTER_0_BASEADDR,
      XAUD_FORMATTER_STS + XAUD_FORMATTER_MM2S_OFFSET, val);
/*
   if (isra_count++>100) {
      DEBUG_AUDIO("[isra]\n");
      isra_count = 0;
   }
*/
   if (interrupt_enabled_audio) {
      amiga_interrupt_set(AMIGA_INTERRUPT_AUDIO);
   }
}

uint32_t audio_get_dma_transfer_count() {
   return(XAudioFormatterGetDMATransferCount(&audio_formatter));
}

void audio_set_interrupt_enabled(int en) {
   DEBUG_AUDIO("[audio] enable irq: %d\n", en);
   interrupt_enabled_audio = en;

   if (!en) {
      amiga_interrupt_clear(AMIGA_INTERRUPT_AUDIO);
   }

   audio_silence();
}

// offset = offset from audio tx buffer
// returns audio_buffer_collision (1 or 0)
int audio_swab(uint16_t audio_buf_samples, uint32_t offset, int byteswap) {
   int audio_buffer_collision = 0;
//   uint16_t* data = (uint16_t*)(audio_tx_buffer + offset);
   int16_t* sdata = (int16_t*)(audio_tx_buffer + offset);
   int audio_freq = audio_buf_samples * 50;

   //DEBUG_AUDIO("[audio:%d] play: %d +%lu\n", byteswap, audio_freq, offset);

   // byteswap and preamp
   if (byteswap) {
      for (int i=0; i < audio_buf_samples * 2; i++) {
         sdata[i]=__builtin_bswap16(sdata[i]);
      }
   }
   // only preamp
   if(preamp!=64) // preamp == 64 is gain 1.0
   {
      for (int i=0; i < audio_buf_samples * 2; i++) {
         sdata[i]=ssat16((sdata[i]*preamp)>>6);
      }
   }
   // FIXME missing filter, wonky address calculation
   // resample if other freq

   if (audio_freq != 48000) {
      resample_s16((int16_t*)(audio_tx_buffer + offset),
            (int16_t*)((uint8_t*)audio_tx_buffer+AUDIO_TX_BUFFER_SIZE*2),
            audio_freq,
            48000,
            AUDIO_BYTES_PER_PERIOD/4);
      equalizer((int16_t*)((uint8_t*)audio_tx_buffer+AUDIO_TX_BUFFER_SIZE*2),AUDIO_BYTES_PER_PERIOD/4);
      lowpass_filter((int16_t*)((uint8_t*)audio_tx_buffer+AUDIO_TX_BUFFER_SIZE*2),AUDIO_BYTES_PER_PERIOD/4);
      memcpy(audio_tx_buffer + offset, (uint8_t*)audio_tx_buffer+AUDIO_TX_BUFFER_SIZE*2, AUDIO_BYTES_PER_PERIOD);
   }
   else
   {
      equalizer((int16_t*)((uint32_t)audio_tx_buffer + offset),audio_buf_samples);
      lowpass_filter((int16_t*)((uint32_t)audio_tx_buffer + offset),audio_buf_samples);
   }

   uint32_t txcount = audio_get_dma_transfer_count();

   // is the distance of reader (audio dma) and writer (amiga) in the ring buffer too small?
   // then signal this condition so amiga can adjust
   if (abs(txcount-offset) < AUDIO_BYTES_PER_PERIOD) {
      audio_buffer_collision = 1;
      //DEBUG_AUDIO("[aswap] ring collision %d\n", abs(txcount-offset));
   } else {
      audio_buffer_collision = 0;
   }

   if (audio_buffer_collision) {
      DEBUG_AUDIO("[ax]:audio_buffer_collision\n");
//      DEBUG_AUDIO("[aswap] d-a: %ld\n",txcount-offset);
//      DEBUG_AUDIO("offset: %ld\n",offset);
//      DEBUG_AUDIO("txcount: %ld\n",txcount);
   }

   return(audio_buffer_collision);
}

double resample_cur = 0;
double resample_psampl = 0;
double resample_psampr = 0;

void resample_s16(int16_t *input, int16_t *output, int in_sample_rate,
      int out_sample_rate, int output_samples) {
   double step_dist = ((double) in_sample_rate / (double) out_sample_rate);
   double cur = resample_cur;
   int in_pos1 = 0, in_pos2 = 0;
   double sample1l = 0, sample2l = 0, sample1r = 0, sample2r = 0;

   int inmax = (int) (step_dist * 960.0) - 1;

   for (int i = 0; i < output_samples; i++) {
      cur=i*step_dist+resample_cur;
      in_pos1 = ((int) cur) - 1;
      in_pos2 = ((int) cur);

      // FIXME hack
      if (in_pos2 > inmax) {
         in_pos2 = inmax;
         in_pos1 = inmax - 1;
      }

      double frac2 = cur - (1 + in_pos1);
      double frac1 = (double) 1.0 - frac2;

      if (in_pos1 == -1) {
         sample1l = frac1 * resample_psampl;
         sample1r = frac1 * resample_psampr;
      } else {
         sample1l = frac1 * (double) (input[in_pos1 * 2 + 0]);
         sample1r = frac1 * (double) (input[in_pos1 * 2 + 1]);
      }
      sample2l = frac2 * (double) (input[in_pos2 * 2 + 0]);
      sample2r = frac2 * (double) (input[in_pos2 * 2 + 1]);

      output[i * 2 + 0] = ((int16_t) (sample1l + sample2l));
      output[i * 2 + 1] = ((int16_t) (sample1r + sample2r));
   }
   cur += step_dist;

   resample_cur = cur - (int) cur;
   resample_psampl = (double) (input[in_pos2 * 2 + 0]);
   resample_psampr = (double) (input[in_pos2 * 2 + 1]);
}
void lowpass_filter(int16_t *output, int output_samples)
{
   // low pass filter
//#define DIRECT_FORM_I
#define DIRECT_FORM_II
#ifdef DIRECT_FORM_I
   static float el1=0.,el2=0.,sl=0.,sl1=0.,sl2=0.;
   static float er1=0.,er2=0.,sr=0.,sr1=0.,sr2=0.;
   float el,er;
   for (uint32_t i = 0; i < output_samples; i++)
   {
      el =output[2*i+0];
      er =output[2*i+1];

      sl=b0[11]*el+b1[11]*el1+b2[11]*el2+a1[11]*sl1+a2[11]*sl2;
      sr=b0[11]*er+b1[11]*er1+b2[11]*er2+a1[11]*sr1+a2[11]*sr2;

      // output gain
      int32_t ol=sl*vl;
      int32_t or=sr*vr;

      //saturation to int16
      ol=ssat16(ol);
      or=ssat16(or);

      output[2*i+0]=((int16_t)ol);
      output[2*i+1]=((int16_t)or);

      el2=el1;
      el1=el;
      er2=er1;
      er1=er;
      sl2=sl1;
      sl1=sl;
      sr2=sr1;
      sr1=sr;
   }
#endif
#ifdef DIRECT_FORM_II
   static float wl1=0.,wl2=0.;
   static float wr1=0.,wr2=0.;
   float el,er;
   float sl,sr;
   for (uint32_t i = 0; i < output_samples; i++)
   {
      el =output[2*i+0];
      er =output[2*i+1];

      sl = b0[10]*el          +wl1;
      wl1= b1[10]*el+a1[10]*sl+wl2;
      wl2= b2[10]*el+a2[10]*sl;
      sr = b0[10]*er          +wr1;
      wr1= b1[10]*er+a1[10]*sr+wr2;
      wr2= b2[10]*er+a2[10]*sr;

      // output gain
      int32_t ol=sl*vl;
      int32_t or=sr*vr;

      //saturation to int16
      ol=ssat16(ol);
      or=ssat16(or);

      output[2*i+0]=((int16_t)ol);
      output[2*i+1]=((int16_t)or);
   }
#endif
}
void bandpass_filter(int band, int16_t *output, int output_samples)
{
   // low pass filter
//#define DIRECT_FORM_I
#define DIRECT_FORM_II
#ifdef DIRECT_FORM_I
   static float el1=0.,el2=0.,sl=0.,sl1=0.,sl2=0.;
   static float er1=0.,er2=0.,sr=0.,sr1=0.,sr2=0.;
   float el,er;
   for (uint32_t i = 0; i < output_samples; i++)
   {
      el =output[2*i+0];
      er =output[2*i+1];

      sl=b0[band]*el+b1[band]*el1+b2[band]*el2+a1[band]*sl1+a2[band]*sl2;
      sr=b0[band]*er+b1[band]*er1+b2[band]*er2+a1[band]*sr1+a2[band]*sr2;

      // output gain
      int32_t ol=sl*vl;
      int32_t or=sr*vr;

      //saturation to int16
      ol=ssat16(ol);
      or=ssat16(or);

      output[2*i+0]=((int16_t)ol);
      output[2*i+1]=((int16_t)or);

      el2=el1;
      el1=el;
      er2=er1;
      er1=er;
      sl2=sl1;
      sl1=sl;
      sr2=sr1;
      sr1=sr;
   }
#endif
#ifdef DIRECT_FORM_II
   static float wl1[10]={0.},wl2[10]={0.};
   static float wr1[10]={0.},wr2[10]={0.};
   float el,er;
   float sl,sr;
   for (uint32_t i = 0; i < output_samples; i++)
   {
      el =output[2*i+0];
      er =output[2*i+1];

      sl       = b0[band]*el            +wl1[band];
      wl1[band]= b1[band]*el+a1[band]*sl+wl2[band];
      wl2[band]= b2[band]*el+a2[band]*sl;
      sr       = b0[band]*er            +wr1[band];
      wr1[band]= b1[band]*er+a1[band]*sr+wr2[band];
      wr2[band]= b2[band]*er+a2[band]*sr;

      // output gain is 1.0
      int32_t ol=sl;
      int32_t or=sr;

      //saturation to int16
      ol=ssat16(ol);
      or=ssat16(or);

      output[2*i+0]=((int16_t)ol);
      output[2*i+1]=((int16_t)or);
   }
#endif
}
extern uint32_t audio_params[];
void equalizer(int16_t *output, int output_samples)
{
   for(int i=0;i<10;i++)
   {
      if(audio_params[AP_DSP_SET_EQ_BAND1+i]!=50)
         bandpass_filter(i,output,output_samples);
   }
}
void reset_resampling() {
   resample_cur = 0;
   resample_psampl = 0;
   resample_psampr = 0;
}

void audio_set_tx_buffer(uint8_t* addr) {
   DEBUG_AUDIO("[audio] set tx buffer: %p\n", addr);
   audio_tx_buffer = addr;
   reset_resampling();
}
/*
void audio_set_rx_buffer(uint8_t* addr) {
   DEBUG_AUDIO("[audio] set rx buffer: %p\n", addr);
   audio_rx_buffer = addr;
}
*/
void audio_silence() {
   memset(audio_tx_buffer, 0, AUDIO_TX_BUFFER_SIZE);
   reset_resampling();
}

// sources:
// https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
// https://wiki.analog.com/resources/tools-software/sigmastudio/usingsigmastudio/systemimplementation
// https://ez.analog.com/dsp/sigmadsp/f/q-a/104470/nth-order-filter-coefficient-calculations
// https://wiki.analog.com/resources/tools-software/sigmastudio/toolbox/filters/general2ndorder
// https://ez.analog.com/dsp/sigmadsp/f/q-a/65510/parameters-with-adau1701


double flt_omega(double fs, double f0) {
   return(2.0 * M_PI * (f0 / fs));
}

double flt_alpha(double fs, double f0) {
   double omega = flt_omega(fs, f0);
   double Q = 1.0 / sqrt(2.0);
   return(sin(omega) / (2.0 * Q));
}
double eq_omega(double fs, double f0) {
   return(2.0 * M_PI * (f0 / fs));
}

double eq_alpha(double fs, double f0) {
   double omega = eq_omega(fs, f0);
   double Q = 1.2247449;
   return(sin(omega) / (2.0 * Q));
}

void audio_adau_set_eq_gain(int band, double gain)
{
   if(band > 9) return;
   // These are the classic
   static const double BandFreqs[10] = {
      31.25, 62.5, 125.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0, 16000.0
   };
   double dBBoost = ((float)gain-50.0f)*12.0/50.0;
   double gainLinear = 1.0;
   double A= pow(10.0, dBBoost / 20.0);
   double fs = 48000.0f;
   double f0 = BandFreqs[band];

   double omega = eq_omega(fs, f0);
   double alpha = eq_alpha(fs, f0);

   double _a0 = 1.0 + alpha/A;
   double _a1 = -2.0 * cos(omega);
   double _a2 = 1.0 - alpha/A;
   double _b0 = (1 + alpha*A) * gainLinear;
   double _b1 = -(2.0 * cos(omega)) * gainLinear;
   double _b2 = (1.0 - alpha*A) * gainLinear;

   _a1 /= _a0;
   _a2 /= _a0;
   _b0 /= _a0;
   _b1 /= _a0;
   _b2 /= _a0;

   _a1 = -_a1;
   _a2 = -_a2;

   b0[band]=_b0;
   b1[band]=_b1;
   b2[band]=_b2;
   a1[band]=_a1;
   a2[band]=_a2;

   DEBUG_AUDIO("[bpf] f0: %8.2f Hz, band=%d\n", (float)f0, band);

}
void audio_adau_set_lpf_params(int f0) {
//   double gain = 1; // unused in general, but there is a preamp gain when using MHI
   double fs = 48000.0;
   if(f0<10) f0 = 10;
//   if(f0>22050) f0 = 22050;
   cut_freq=2.*M_PI*f0;
   DEBUG_AUDIO("[lpf] f0: %8.2f Hz\n", (float)f0);
//#define SHANSHE_FILTER
#ifdef SHANSHE_FILTER
   double w=cut_freq;

   double tanwTm2=tan(w/fs/2.); // 2nd order lpf equations with prewarp :)
   double a0=tanwTm2*tanwTm2+2.*tanwTm2+1.0;

   a1=(2.0-2.0*tanwTm2*tanwTm2        )/a0;
   a2=(2.0*tanwTm2-tanwTm2*tanwTm2-1.0)/a0;

   b0=   tanwTm2*tanwTm2/a0;
   b1=2.*tanwTm2*tanwTm2/a0;
   b2=   tanwTm2*tanwTm2/a0;

#else
   double omega = flt_omega(fs, f0);
   double alpha = flt_alpha(fs, f0);

   double _a0 = 1.0 + alpha;
   double _a1 = -2.0 * cos(omega);
   double _a2 = 1.0 - alpha;
   double _b0 = (1.0 - cos(omega)) / 2.0;
   double _b1 = 1.0 - cos(omega);
   double _b2 = _b0;

   _a1 /= _a0;
   _a2 /= _a0;
   _b0 /= _a0;
   _b1 /= _a0;
   _b2 /= _a0;

   _a1 = -_a1;
   _a2 = -_a2;
#endif
   b0[10]=_b0;
   b1[10]=_b1;
   b2[10]=_b2;
   a1[10]=_a1;
   a2[10]=_a2;

}

int Xil_AssertNonVoid(){return(0);}

// vol range: 0-255. 127 = 0db
// vol1: paula
// vol2: i2s
void audio_adau_set_mixer_vol(int vol1, int vol2) {
   // FIXME not applied
   double v1 = ((double)vol1)/127.0;
   double v2 = ((double)vol2)/127.0;

   DEBUG_AUDIO("[vol] v1: %f v2: %f\n", v1, v2);
}
void audio_adau_set_vol_pan(int vol, int pan) {
   vl=vr=vol*0.02;
   if(pan>50) vl=(vl*(100-pan))*0.02;
   if(pan<50) vr=(vr*(pan-0  ))*0.02;
   DEBUG_AUDIO("[vol+pan] vl: %f vr: %f\n", vl, vr);
}
void audio_adau_set_prefactor(int vol) {
   preamp=(int32_t)(pow(10,0.012*vol-0.6)*64); // 0 <-> 100 == -12dB <-> 12dB
   DEBUG_AUDIO("[preamp] p: %f\n", preamp/64.);
}
