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
#include "math.h"
#include "sleep.h"
#include "../config_file.h"
#include "../scsi/scsi.h"
#include "../ltc2990/ltc2990.h"
#include "../config_clk.h"
#include "../debug_console.h"
#include "str_zzregs.h"
#include "../lwip.h"
#include "../ARM_ztop/slider.h"

typedef enum {
   MA_DECODE_INIT,
   MA_DECODE_RUN,
   MA_DECODE_CLEAR_FIFO,
   MA_DECODE_INIT_FIFO,
   MA_DECODE_NUM_PAPARAMS
} MA_DECODE_COMMAND;
static const char ma_decode_command_str[MA_DECODE_NUM_PAPARAMS][30]={
      "MA_DECODE_INIT",
      "MA_DECODE_RUN",
      "MA_DECODE_CLEAR_FIFO",
      "MA_DECODE_INIT_FIFO",
};
unsigned long ma_fifo_get_read_index(void);
void ma_fifo_set_write_index(unsigned long aWriteIndex);
void ma_fifo_clear(void);
int decode_ma_init_fifo(uint8_t* input_buffer, size_t input_buffer_size);
int decode_ma_init(uint8_t* input_buffer, size_t input_buffer_size);
int ma_get_hz();
int decode_ma_samples(void* output_buffer, int max_samples);
int delete_selected_preset(void);

extern DEBUG_CONSOLE debug_console;
void DEBUG_AUDIO(const char *format, ...)
{
   if(debug_console.debug_audio==0)
      return;
   va_list args;
   va_start(args, format);
   vprintf(format,args);
   va_end(args);
}

#define inline

uint32_t cur_mem_offset = 0x03500000;
typedef struct {
   uint32_t Core_temp;
   uint32_t Aux_volt;
   uint32_t Core_volt;
   uint32_t LTC_temp;
   uint32_t LTC_3V3;
   uint32_t LTC_5V;
   uint32_t LTC_060_temp;
   uint32_t LTC_VCC;
} Measures;
Measures measures;

//#define XPAR_PROCESSING_AV_SYSTEM_AUDIO_VIDEO_ENGINE_VIDEO_VIDEO_FORMATTER_0_BASEADDR 0x78C20000
#define VIDEO_FORMATTER_BASEADDR XPAR_PROCESSING_AV_SYSTEM_AUDIO_VIDEO_ENGINE_VIDEO_VIDEO_FORMATTER_0_BASEADDR
void write_rtg_register(uint32_t zaddr,uint32_t zdata);
uint32_t read_rtg_register(uint32_t zaddr);
extern uint32_t revision_alfa;
uint32_t gpio=0;
uint16_t ack_request=0;
uint32_t custom_vmode_param=0;
uint32_t custom_video_mode=ZZVMODE_CUSTOM;
ENV_FILE_VARS env_file_vars_temp[9]; // really size 8
int preset_selected=-1;
//int bm=0,sb=0,ar=0,cr=0,ks=0,ext_ks=0;
void hard_reboot(void);

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
int frfb=0;

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
   printf("set_palette(%d) idx=%d color=%08lx\n",op_palette==19,idx,data);
*/
   Xil_ExceptionDisable();
   video_formatter_write(zdata, op_palette);
   Xil_ExceptionEnable();
}
void read_preset_name(void)
{
   if(preset_selected>=0 && preset_selected<=7)
   {
      int j=0;
      char c=-1;
      while(c!=0)
      {
         c=*(char*)(RTG_BASE+REG_ZZ_SEL_PRESET_TXT+j);
         env_file_vars_temp[preset_selected].preset_name[j]=c;
         j++;
      }
   }
   printf("Apply PRESET %d name %s\n",preset_selected,env_file_vars_temp[preset_selected].preset_name);
}
ZZ_VIDEO_STATE* video_state;
void rtg_init(void)
{
   printf("RTG init...\n");
   video_state->framebuffer_pan_offset=0;

   ethernet_send_result = 0;
   frfb=0;
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

   *(uint32_t *)(RTG_BASE+REG_ZZ_SEL_KS_TXT)=0;
   *(uint32_t *)(RTG_BASE+REG_ZZ_SEL_SCSI_TXT)=0;
   *(uint32_t *)(RTG_BASE+REG_ZZ_SEL_PRESET_TXT)=0;
}
uint32_t *address;
uint32_t zdata;
uint32_t op_data=0;
uint32_t zaddr;


int eth_backlog_nag_counter_max=ETH_BACKLOG_NAG_COUNTER_MAX;
#ifdef CPU_EMULATOR
#define IDLE_TASK_COUNT_MAX 300000
#else
#define IDLE_TASK_COUNT_MAX 3000000
#endif
int idle_task_count_max=IDLE_TASK_COUNT_MAX;
long int task_counter=0;
#include "../pt/pt.h"
extern uint32_t ticks;
uint32_t last_ticks=-1;
void load_preset_to_config(void)
{
   if(preset_selected>0 && preset_selected<=7)
   {
      temp_config.boot_mode=env_file_vars_temp[preset_selected].boot_mode;
      temp_config.scsiboot=env_file_vars_temp[preset_selected].scsiboot;
      temp_config.autoconfig_ram=env_file_vars_temp[preset_selected].autoconfig_ram;
      temp_config.autoconfig_rtg=env_file_vars_temp[preset_selected].autoconfig_rtg;
      temp_config.enable_test=env_file_vars_temp[preset_selected].enable_test;
      temp_config.cpu_ram=env_file_vars_temp[preset_selected].cpu_ram;
      temp_config.cpufreq=env_file_vars_temp[preset_selected].cpufreq;
      temp_config.kickstart=env_file_vars_temp[preset_selected].kickstart;
      temp_config.ext_kickstart=env_file_vars_temp[preset_selected].ext_kickstart;
      temp_config.bootscreen_resolution=env_file_vars_temp[preset_selected].bootscreen_resolution;
      for(int i=0;i<7;i++)
      {
         temp_config.scsi_num[i]=env_file_vars_temp[preset_selected].scsi_num[i];
      }
      for(int i=0;i<6;i++)
         temp_config.mac_address[i]=env_file_vars_temp[preset_selected].mac_address[i];

      temp_config.bp_ton=env_file_vars_temp[preset_selected].bp_ton;
      temp_config.bp_toff=env_file_vars_temp[preset_selected].bp_toff;
   }
   else
   {
      memcpy(&temp_config,&default_config,sizeof(CONFIG));
   }
}

int interrupt_thread(struct pt *pt)
{
   PT_BEGIN(pt);
   while(1)
   {
      PT_WAIT_UNTIL(pt,last_interrupt!=(current_interrupt=amiga_interrupt_get()));
      last_interrupt=current_interrupt;
      *((uint32_t *)(RTG_BASE+REG_ZZ_INT_STATUS))=swap32(current_interrupt);
      //      Xil_DCacheFlushRange(RTG_BASE+REG_ZZ_INT_STATUS,4);
   }
   PT_END(pt);
}
int ethernet_thread(struct pt *pt)
//void rtg_loop(void)
{
   PT_BEGIN(pt);
   while(1)
   {
      // check for queued up ethernet frames and interrupt amiga
      PT_WAIT_UNTIL(pt,eth_backlog_nag_counter > eth_backlog_nag_counter_max);
      eth_backlog_nag_counter = 0;
//               if((amiga_interrupt_get()&AMIGA_INTERRUPT_ETH)==0)
//               {
      amiga_interrupt_set(AMIGA_INTERRUPT_ETH);
//               }
//            }
//         }
//         else if(ethernet_backlog == 0)
//         {
//            if((amiga_interrupt_get()&AMIGA_INTERRUPT_ETH)!=0)
//               amiga_interrupt_clear(AMIGA_INTERRUPT_ETH);
//         }
//      }

   }
   PT_END(pt);

}
void other_tasks(void)
{
   static int idle_task_count=0;
   if(idle_task_count++> idle_task_count_max)
   {
      idle_task_count=0;
      ethernet_task();
   }

   if(audio_request_init) {
      audio_debug_timer(0);
      audio_silence();
      audio_init_i2s();
      audio_request_init = 0;
      audio_debug_timer(1);
   }

   if(interrupt_enabled_ethernet && ethernet_get_backlog()>0)
   {
      eth_backlog_nag_counter++;
   }

}
#define WAIT_COUNTER PT_WAIT_UNTIL(pt,task_counter>=1000000);task_counter=0
inline float __fsat(float X,float XMAX, float XMIN)
{
   if(X<XMIN)X=XMIN;
   else if(X>XMAX) X=XMAX;
   return(X);
}
int measures_thread(struct pt *pt)
{
   PT_BEGIN(pt);
   while(1)
   {
      static float value3=0;
      WAIT_COUNTER;
      measures.Core_temp=(uint32_t)(xadc_get_temperature()*10.0);
      PT_WAIT_UNTIL(pt,iic_write_ltc2990(LTC_TRIGGER_REG,0)); // any value trigger a conversion
      float value=xadc_get_aux_voltage();
      value=__fsat(value,200,0);
      measures.Aux_volt=(uint32_t)(value*100.0);
      WAIT_COUNTER;
      value=xadc_get_int_voltage();
      value=__fsat(value,200,0);
      measures.Core_volt=(uint32_t)(value*100.0);
      WAIT_COUNTER;
      PT_WAIT_UNTIL(pt,iic_read_ltc2990(LTC_TINT_MSB));
      {
         int data_in = ((ReadBuffer_ltc2990[0]&0x1F)<<8) | ReadBuffer_ltc2990[1];
         value=data_in*0.0625;
         value=__fsat(value,200,0);
         measures.LTC_temp=(uint32_t)(value*100.0);

      }
      WAIT_COUNTER;
      PT_WAIT_UNTIL(pt,iic_read_ltc2990(LTC_V1_MSB));
      {
         int data_in = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
         value=data_in*(305.8e-6)*10.;
         value=__fsat(value,200,0);
         measures.LTC_3V3=(uint32_t)(value*100.0);
      }
      WAIT_COUNTER;
      PT_WAIT_UNTIL(pt,iic_read_ltc2990(LTC_V2_MSB));
      {
         int data_in = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
         value=data_in*(305.8e-6)*10.;
         value=__fsat(value,200,0);
         measures.LTC_5V=(uint32_t)(value*100.0);
      }
      WAIT_COUNTER;
      PT_WAIT_UNTIL(pt,iic_read_ltc2990(LTC_V3_MSB));
      {
         int data_in = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
         value3=data_in*(305.8e-6);
      }
      WAIT_COUNTER;
      PT_WAIT_UNTIL(pt,iic_read_ltc2990(LTC_V4_MSB));
      {
         int data_in = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
         float value4=data_in*(305.8e-6)*10.;
         // resistor calculation
         value=value3/(value4-value3)*9000.;
         float real_resistor_calibrated=config.resistor*9000/(9000-config.resistor);
         float offset=real_resistor_calibrated*((float)(1./2.8))-config.temperature;
         value=value*((float)(1./2.8))-offset;
         if(value>10000) value=0; // higher values overflow
         value=__fsat(value,200,0);
         measures.LTC_060_temp=(uint32_t)(value*100.);
      }
      WAIT_COUNTER;
      PT_WAIT_UNTIL(pt,iic_read_ltc2990(LTC_VCC_MSB));
      {
         int data_in = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
         value=(2.5+data_in*305.18e-6);
         value=__fsat(value,200,0);
         measures.LTC_VCC=(uint32_t)(value*100.0);
      }
   }
   PT_END(pt);
}
int rtg_thread(struct pt *pt)
//void rtg_loop(void)
{
   PT_BEGIN(pt);
   while(1)
   {

      PT_WAIT_UNTIL(pt,((gpio=read_reg_s01(REG1))&0xC0000000)!=0);
      if((gpio&0x80000000)!=0) //write
      {
         if((gpio&0xF80000)==0xF80000)
         {
            //write to ROM!!!!
            address=(uint32_t *)(gpio&0xFFFFFC);
            printf("Write ROM 0x%08lX gpio 0x%08lX\n",(uint32_t)address,gpio);
         }
         else
         {
            address=(uint32_t *)((gpio&0x1FFFFF) + RTG_BASE);
            zaddr=gpio&0x1FFFFF;

            if(zaddr<0x2000)
            {
               zdata=swap32(*address);
               int add_bits=gpio&0x3;
               if(add_bits && zaddr<0x700 && zaddr>=0x800)
                  printf("write RTG regs add 0x%08lX\n",zaddr);
               write_rtg_register(zaddr,zdata);
            }
            else
            {
#define SCSI_ADDR_MAX 0x6000 //0x80000 //0x6000
               if(zaddr<SCSI_ADDR_MAX)
               {
                  int add_bits=gpio&0x3;
                  if(add_bits)
                     printf("write SCSI add 0x%08lX\n",zaddr);
                  zaddr-=0x2000;
                  int type=OP_TYPE_BYTE;
                  if(((gpio&0x30000000)==0x00000000)  //long
                        ||((gpio&0x30000000)==0x30000000)) //line
                  {
                     type=OP_TYPE_LONGWORD;
                     zdata=swap32(*address);
                  }
                  else if((gpio&0x30000000)==0x20000000) //word
                  {
                     type=OP_TYPE_WORD;
                     zdata=swap16(*(uint16_t*)address);
                  }
                  else // byte
                  {
                     zdata=*(uint8_t*)address;
                  }
                  handle_piscsi_reg_write(zaddr, zdata, type);
               }
#define SCSI_ROM_MAX (SCSI_NO_DMA_ADDRESS-RTG_BASE)
               else if(zaddr>=SCSI_ROM_MAX)
               {
                  int add_bits=gpio&0x3;
                  if(add_bits)
                     printf("write SCSI add 0x%08lX\n",zaddr);
                  //                  printf("Write to zaddr= 0x%08lX\n",zaddr);
                  zdata=swap32(*address);
               }
               else
               {
                  printf("write SCSI ROM add 0x%08lX\n",zaddr);
               }
            }
         }
         ack_request=1;
      }
      else if((gpio&0x40000000)!=0) //read
      {
         if((gpio&0xF80000)==0xF80000)
         {
            address=(uint32_t *)(gpio&0xFFFFFC);
            zdata=swap32(*address);
            write_reg_s01(REG5,zdata);
         }
         else
         {
            zaddr=gpio&0x1FFFFF;
            if(zaddr<0x2000)
            {
               zdata=read_rtg_register(zaddr);
//               zdata=swap32(*address);
/*
               int add_bits=gpio&0x3;
               if(add_bits)
                  printf("read RTG add 0x%08lX\n",zaddr);
*/
               write_reg_s01(REG5,zdata);
            }
            else
            {
               if(zaddr<SCSI_ADDR_MAX)
               {
                  zaddr-=0x2000;
                  zdata=handle_piscsi_read(zaddr&0x1FFFFC, 2);
                  int add_bits=gpio&0x3;
                  if(add_bits)
                     printf("read SCSI add 0x%08lX\n",zaddr);
                  write_reg_s01(REG5,zdata);
               }
               else
               {
//                  int size_bits=(gpio>>28)&0x3;
/*
                  int add_bits=gpio&0x3;
                  if(add_bits)
                     printf("read add 0x%08lX\n",zaddr);
*/
                  zdata=swap32(*((uint32_t *)(RTG_BASE+(zaddr&0x1FFFFC))));
                  write_reg_s01(REG5,zdata);
               }
            }
         }
         ack_request=1;
      }

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
   }
   PT_END(pt);
}
uint32_t read_rtg_register(uint32_t zaddr)
{
   uint32_t data=0;
   /*
   if(zaddr&3)
      printf("read unaligned to 0x%08X\n",zaddr);
    */
   if(debug_console.debug_rtg)
   {
      printf("READ RTG reg 0x%lX %s\n",zaddr,zz_reg_offsets_string[zaddr]);
   }
   int address=(zaddr&0x1FFFFC);
   if((address>=REG_ZZ_SEL_KS_TXT     && address<REG_ZZ_SEL_KS_TXT+   150) ||
      (address>=REG_ZZ_SEL_SCSI_TXT   && address<REG_ZZ_SEL_SCSI_TXT+ 150) ||
      (address>=REG_ZZ_SEL_PRESET_TXT && address<REG_ZZ_SEL_PRESET_TXT+150))
   {
      data=swap32(*(uint32_t *)(RTG_BASE+address));
      return(data);
   }
   switch (address)
   {
      case REG_ZZ_SOFT3D_OP:
         data=swap32(*(uint32_t*)(RTG_BASE+REG_ZZ_SOFT3D_OP));
         //      printf("data read soft3d %08lx\n",data);
         break;
      case REG_ZZ_INT_STATUS:
         data=amiga_interrupt_get();
         break;
      case REG_ZZ_FW_VERSION:
         //      data=(REVISION_MINOR << 24) | (REVISION_MAJOR << 16);
         data=(REVISION_MAJOR << 8 ) | (REVISION_MINOR      );
         break;
      case REG_ZZ_FW_BETA:
         data=REVISION_BETA;
         //      printf("Read beta version number: %d\n",REVISION_BETA);
         break;
      case REG_ZZ_FW_ALFA:
         data=revision_alfa;
         //      printf("Read alfa version number: %d\n",revision_alfa);
         break;
      case REG_ZZ_ETH_TX:
         data=ethernet_send_result;
         break;
      case REG_ZZ_ETH_RX_ADDRESS: {
         //      data=RX_BACKLOG_ADDRESS+(frfb<<11); // <<11 = 2048 (FRAME_SIZE)
         data=(uint32_t)ethernet_current_receive_ptr();
         break;
      }
      case REG_ZZ_AUDIO_SWAB:
         data=audio_buffer_collision;
         break;
      case REG_ZZ_AUDIO_VAL:
         data=audio_params[audio_param]; // read param
         break;
      case REG_ZZ_AUDIO_CONFIG:
         data=1; // AX is present
         break;
      case REG_ZZ_DECODER_FIFORX:
         data=ma_fifo_get_read_index();
         break;
      case REG_ZZ_DECODE:
         data=decoder_bytes_decoded;
         break;
         //   case REG_ZZ_VBLANK_STATUS:
         //      return(0); // this is now read directly from FPGA, so this value is not used anymore
      case REG_ZZ_CPU_FREQ:
         data=env_file_vars_temp[preset_selected].cpufreq;
         break;
      case REG_ZZ_EMULATION_USED:
         data=env_file_vars_temp[preset_selected].boot_mode==UAEJIT || env_file_vars_temp[preset_selected].boot_mode==UAE || env_file_vars_temp[preset_selected].boot_mode==MUSASHI;
         break;
      case REG_ZZ_SCSIBOOT_EN:
         data=env_file_vars_temp[preset_selected].scsiboot==YES;
         break;
      case REG_ZZ_AUTOC_RAM_EN:
         data=env_file_vars_temp[preset_selected].autoconfig_ram==YES;
         break;
      case REG_ZZ_AUTOC_RTG_EN:
         data=env_file_vars_temp[preset_selected].autoconfig_rtg==YES;
         break;
      case REG_ZZ_CPU_RAM_EN:
         data=env_file_vars_temp[preset_selected].cpu_ram==YES;
         break;
      case REG_ZZ_TEST_ENABLE:
         data=env_file_vars_temp[preset_selected].enable_test;
         break;

      case REG_ZZ_ETH_MAC_HI: {
         uint8_t* mac = ethernet_get_mac_address_ptr();
         data=(((uint32_t)mac[0])<<8)|mac[1];
         break;
      }
      case REG_ZZ_ETH_MAC_LO: {
         uint8_t* mac = ethernet_get_mac_address_ptr();
         data=(((uint32_t)mac[2])<<24)|(((uint32_t)mac[3])<<16)|(((uint32_t)mac[4])<<8)|mac[5];
         break;
      }
      case REG_ZZ_JIT_ENABLE:
         data=shared->jit_enabled;
         break;
      case REG_ZZ_BOOTMODE:
         data=env_file_vars_temp[preset_selected].boot_mode;
         break;
      case REG_ZZ_TEMPERATURE:
         data=measures.Core_temp;
         break;
      case REG_ZZ_VOLTAGE_AUX:
         data=measures.Aux_volt;
         break;
      case REG_ZZ_VOLTAGE_INT:
         data=measures.Core_volt;
         break;
      case REG_ZZ_LTC_TEMP:
         data=measures.LTC_temp;
         break;
      case REG_ZZ_LTC_V1:
         data=measures.LTC_3V3;
         break;
      case REG_ZZ_LTC_V2:
         data=measures.LTC_5V;
         break;
      case REG_ZZ_LTC_060_TEMP:
         data=measures.LTC_060_temp;
         break;
      case REG_ZZ_LTC_VCC:
         data=measures.LTC_VCC;
         break;
      case REG_ZZ_KS_SEL:
         data=env_file_vars_temp[preset_selected].kickstart;
         //       printf("KICKSTART SEL READ %ld\n",data);
         break;
      case REG_ZZ_EXT_KS_SEL:
         data=env_file_vars_temp[preset_selected].ext_kickstart;
         //       printf("EXT KICKSTART SEL READ %ld\n",data);
         break;
      case REG_ZZ_SCSI_SEL_0:
      case REG_ZZ_SCSI_SEL_1:
      case REG_ZZ_SCSI_SEL_2:
      case REG_ZZ_SCSI_SEL_3:
      case REG_ZZ_SCSI_SEL_4:
      case REG_ZZ_SCSI_SEL_5:
      case REG_ZZ_SCSI_SEL_6:
         data=env_file_vars_temp[preset_selected].scsi_num[(address-REG_ZZ_SCSI_SEL_0)>>2];
         break;
      case REG_ZZ_BPTON:
         data=(env_file_vars_temp[preset_selected].bp_ton+0.5e-6)*(SLIDER_BPTON_MAX/TON_MAX);
         break;
      case REG_ZZ_BPTOFF:
         data=(env_file_vars_temp[preset_selected].bp_toff+0.5e-3)*(SLIDER_BPTOFF_MAX/TOFF_MAX);
         break;
      case REG_ZZ_PRESET_SEL:
         data=preset_selected;
         printf("read preset_selected %ld\n",data);
         break;
      default:
         if(debug_console.debug_rtg)
            printf("Read from unknown 0x%X RTG register\n",address);
         data=0xFFFFFFFF; // swap32(*((uint32_t *)(RTG_BASE+address)));
   }
   return(data);
}
void write_rtg_register(uint32_t zaddr,uint32_t zdata)
{
   if(zaddr>=REG_ZZ_SEL_PRESET_TXT && zaddr<REG_ZZ_SEL_PRESET_TXT+150)
   {
      // unaligned write
      *(uint8_t *)(RTG_BASE+zaddr)=swap32(zdata)&0xff;
//      printf("%c %08lx,%08lX\n",(uint8_t)(swap32(zdata)&0xFF),zaddr,zdata);
      return;
   }
   if(zaddr&3)
      printf("write unaligned to 0x%08lX\n",zaddr);
   if(debug_console.debug_rtg)
   {
      printf("WRITE RTG reg 0x%lX = %ld (0x%lX) %s\n",zaddr,zdata,zdata,zz_reg_offsets_string[zaddr]);
      if(debug_console.step)
      {
         while(!XUartPs_IsReceiveData(STDIN_BASEADDRESS)){}
         char c = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
         if(c=='C' || c=='c')
            debug_console.step=0;
      }
   }
   uint32_t address=zaddr&0x1FFFFC;
   switch (address) {
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
         //            printf("[enable] eth:0x%lx\n", zdata);
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
         *((uint32_t *)(RTG_BASE+REG_ZZ_INT_STATUS))=swap32(amiga_interrupt_get());
         break;
      case REG_ZZ_MODE: {
         //printf("mode change: %lx\n", zdata);

         int mode = zdata & 0xff;
         if(mode < ZZVMODE_NUM)
         {
            int colormode = (zdata & 0xf00) >> 8;
            int scalemode = (zdata & 0xf000) >> 12;
            printf("mode: %d color: %d scale: %d\n", mode, colormode, scalemode);
            video_mode_init(mode, scalemode, colormode);
         }
         else
         {
            printf("[RTG] Error mode: %d\n", mode);
         }
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
         //         printf("rect_x1 %d\n",rect_x1);
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
         //            rect_rgb  = zdata;
         break;
      case REG_ZZ_RGB2:
         rect_rgb2  = swap32(zdata);
         //            rect_rgb2  = zdata;
         break;

         // Generic acceleration ops
      case REG_ZZ_ACC_OP: {
         handle_acc_op(zdata);
         break;
      }
      // DMA RTG rendering
      case REG_ZZ_BLITTER_DMA_OP: {
         handle_blitter_dma_op(video_state,zdata);
         break;
      }
      // Soft3D rendering
      case REG_ZZ_SOFT3D_OP: {
         handle_soft3d_op(zdata);
         break;
      }

      // RTG rendering
      case REG_ZZ_FILLRECT:
         //         printf("FILLRECT blitter_dst_offset %lx\n",blitter_dst_offset);
         //         printf("         %d %d %d %d \n",rect_x1,rect_y1,rect_x2,rect_y2);
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
            memcpy   ((uint32_t*) (((uint32_t) video_state->framebuffer) + video_state->frameBuf_pan_offset + (i * rect_x1)),
               (uint32_t*) ((uint32_t)Z3_SCRATCH_ADDR + (i * rect_x1)),rect_x1);
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
            case VMODE_PARAM_HRES:        preset_video_modes[custom_video_mode].hres=zdata;     break;
            case VMODE_PARAM_VRES:        preset_video_modes[custom_video_mode].vres=zdata;     break;
            case VMODE_PARAM_HSTART:      preset_video_modes[custom_video_mode].hstart=zdata;   break;
            case VMODE_PARAM_HEND:        preset_video_modes[custom_video_mode].hend=zdata;     break;
            case VMODE_PARAM_HMAX:        preset_video_modes[custom_video_mode].hmax=zdata;     break;
            case VMODE_PARAM_VSTART:      preset_video_modes[custom_video_mode].vstart=zdata;   break;
            case VMODE_PARAM_VEND:        preset_video_modes[custom_video_mode].vend=zdata;     break;
            case VMODE_PARAM_VMAX:        preset_video_modes[custom_video_mode].vmax=zdata;     break;
            case VMODE_PARAM_POLARITY:    preset_video_modes[custom_video_mode].polarity=zdata; break;
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
/*
            case CARD_FEATURE_NONSTANDARD_VSYNC:
               printf("[feature] NONSTANDARD_VSYNC: %lu\n",zdata);
               // Enables/disables the nonstandard refresh rates for scandoubled PAL/NTSC HDMI output modes.
               if (zdata == 2) {
                  video_state->scandoubler_mode_adjust = 2;
               } else {
                  video_state->scandoubler_mode_adjust = 0;
               }
               video_state->card_feature_enabled[CARD_FEATURE_NONSTANDARD_VSYNC] = zdata;
               break;
*/
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
         //            printf("SEND frame sz: %ld res: %ld\n",zdata,ethernet_send_result);
         break;
      case REG_ZZ_ETH_RX: {
         //            printf("RECV eth frame sz: %ld\n",zdata);
         frfb = ethernet_receive_frame();
         //            printf("REG_ZZ_ETH_RX_ADDRESS=0x%08x\n",RX_BACKLOG_ADDRESS+(frfb<<11));
         //            mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG4, frfb);
         if(0)
         {
                  uint8_t * frame_bl_ptr=(uint8_t *)ethernet_current_receive_ptr()+RTG_BASE;
                  uint16_t rx_bytes,frame_serial;
                  rx_bytes =(*(frame_bl_ptr))<<8;
                  rx_bytes|=(*(frame_bl_ptr+1))&0xFF;
                  frame_serial =(*(frame_bl_ptr+2))<<8;
                  frame_serial|=(*(frame_bl_ptr+3))&0xFF;
                  if(frame_serial!=0)
                     printf("rx_b %d f_serial %d frfb %d\n",rx_bytes,frame_serial,frfb);
                  else
                     printf("serial 0 0x%08lX frfb %d\n",(uint32_t)frame_bl_ptr,frfb);
         }
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
         //               DEBUG_AUDIO("audio_offset 0x%08lx\n",audio_offset);
         break;
      }
      case REG_ZZ_AUDIO_SCALE:
         audio_scale = zdata;
         break;
/*
      case REG_ZZ_UNUSED_REG8C:
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
         DEBUG_AUDIO("[REG_ZZ_AUDIO_PARAM] %ld\n", zdata);

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
         } else {
            audio_param = -1;
         }
         break;
      case REG_ZZ_AUDIO_VAL:
         if(audio_param>=0) {
            DEBUG_AUDIO("[REG_ZZ_AUDIO_VAL] %lx\n", zdata);

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
                  DEBUG_AUDIO("[audio] reprogramming from 0x%p and 0x%p\n", program_ptr, params_ptr);
                  //                     audio_program_adau(program_ptr, 5120);
                  //                     audio_program_adau_params(params_ptr, 4096);
               } else {
                  DEBUG_AUDIO("[audio] programming %ld params from 0x%p\n", zdata, params_ptr);
                  //                     audio_program_adau_params(params_ptr, zdata);
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
         ma_fifo_set_write_index(zdata);
         //               DEBUG_AUDIO("[decode:fifotx:%ld]\n",zdata);
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
            case MA_DECODE_CLEAR_FIFO:
               DEBUG_AUDIO("[decode:clear]\n");
               ma_fifo_clear();
               break;
            case MA_DECODE_INIT: // this is used by axmp3
               if(decode_ma_init(input_buffer, input_buffer_size))
               {
                  DEBUG_AUDIO("[decode_init:%s] %p (length %d) -> %p (%d)\n", ma_decode_command_str[(int)zdata], input_buffer, input_buffer_size,
                        output_buffer, output_buffer_size);
               }
               else
               {
                  DEBUG_AUDIO("[decode_init: FAILED] \n");
               }
               decoder_bytes_decoded = -1;
               break;
            case MA_DECODE_INIT_FIFO: // this is used by mhi
               if(decode_ma_init_fifo(input_buffer, input_buffer_size))
               {
                  DEBUG_AUDIO("[decode_init_fifo:%s] %p (length %d) -> %p (%d)\n", ma_decode_command_str[(int)zdata], input_buffer, input_buffer_size,
                        output_buffer, output_buffer_size);
               }
               else
               {
                  DEBUG_AUDIO("[decode_init_fifo: FAILED] \n");
               }
               decoder_bytes_decoded = -1;
               break;
            case MA_DECODE_RUN:
               max_samples = output_buffer_size;
               {
                  int ma_freq = ma_get_hz();
                  if (ma_freq != 48000) {
                     uint8_t* temp_buffer = output_buffer + AUDIO_TX_BUFFER_SIZE; // FIXME hack
                     max_samples = ma_freq/50 * 2;

                     decoder_bytes_decoded = decode_ma_samples(temp_buffer, max_samples);

                     // resample
                     if(decoder_bytes_decoded>0)
                     {
                        resample_s16((int16_t*)temp_buffer, (int16_t*)output_buffer,
                              ma_freq, 48000, AUDIO_BYTES_PER_PERIOD / 4);
                     }
                  } else {
                     decoder_bytes_decoded = decode_ma_samples(output_buffer, max_samples);
                  }
               }
               //                     if(decoder_bytes_decoded>0)
               //                        DEBUG_AUDIO("[decode:mp3:%s] %p (%d) -> %p (%d) %ld %ld\n", decode_command_str[(int)zdata], input_buffer, input_buffer_size,
               //                           output_buffer, output_buffer_size,fifo_get_read_index(),swap32(*((uint32_t *)(RTG_BASE+REG_ZZ_DECODER_FIFOTX))));
               break;
         }
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
         if((zdata>=CPUFREQ_MIN) && (zdata<=CPUFREQ_MAX))
         {
            //            configure_clk(zdata, 0, 1, 0);
            if(preset_selected>=0)
               env_file_vars_temp[preset_selected].cpufreq=zdata;
         }
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
         core1_addr2[0] = 0xe3e0000f; // mvn   r0, #15  -- loads 0xfffffff0
         core1_addr2[1] = 0xe590f000; // ldr   pc, [r0] -- jumps to the address in that address

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
         //        printf("1000 <= 0x%08X\n",zdata);
      case REG_ZZ_OP:
         //        printf("1004 <= %d\n",zdata);
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
         //        printf("1008 <= %d\n",zdata);
         break;
      case REG_ZZ_OP_CAPTUREMODE:
         printf("CAPTUREMODE <= %ld\n",zdata);
         //            zz_set_monswitch(!zdata);
         break;
      case REG_ZZ_JIT_ENABLE:
         printf("[REG_ZZ_JIT_ENABLE] %ld\n", zdata);
         shared->jit_enabled=zdata;
         break;
      case REG_ZZ_BOOTMODE:
         if(preset_selected>=0)
         {
            if(zdata>=0 && zdata<BOOTMODE_NUM)
            {
               printf("BOOTMODE %ld (%s)\n",zdata,bootmode_names[zdata]);
               if(preset_selected>=0)
                  env_file_vars_temp[preset_selected].boot_mode=zdata;
            }
            else
            {
               printf("BOOTMODE %ld unknown\n",zdata);
               env_file_vars_temp[preset_selected].boot_mode=0;
            }
         }
         break;
      case REG_ZZ_APPLY_BOOTMODE:
         if(zdata==0x55AA)
         {
            if(preset_selected>=0)
            {
               read_preset_name();
               printf("Apply BOOTMODE %d (%s)\n",env_file_vars_temp[preset_selected].boot_mode,bootmode_names[env_file_vars_temp[preset_selected].boot_mode]);
               piscsi_shutdown();
               if(write_env_files_boot(&env_file_vars_temp[preset_selected])==1)
                  hard_reboot();
            }
         }
         else if(zdata==0x5A5A)
         {
            piscsi_shutdown();
            hard_reboot();
         }
         else
         {
            printf("Apply BOOTMODE magic code not valid: 0x%lx\n",zdata);
         }
         break;
      case REG_ZZ_APPLY_SCSI:
         if(zdata==0x55AA)
         {
            if(preset_selected>=0)
            {
               read_preset_name();
               printf("Apply SCSI\n");
               piscsi_shutdown();
               if(write_env_files_scsi(&env_file_vars_temp[preset_selected])==1)
                  hard_reboot();
            }
         }
         else
         {
            printf("Apply SCSI magic code not valid: 0x%lx\n",zdata);
         }
         break;
      case REG_ZZ_APPLY_MISC:
         if(zdata==0x55AA)
         {
            if(preset_selected>=0)
            {
               read_preset_name();
               printf("Apply MISC\n");
               piscsi_shutdown();
               if(write_env_files_misc(&env_file_vars_temp[preset_selected])==1)
                  hard_reboot();
            }
         }
         else
         {
            printf("Apply BOOTMODE magic code not valid: 0x%lx\n",zdata);
         }
         break;
      case REG_ZZ_APPLY_PRESET:
         if(zdata==0x55AA)
         {
            if(preset_selected>=0)
            {
               read_preset_name();
               piscsi_shutdown();
               if(write_env_files_preset(&env_file_vars_temp[preset_selected])==1)
                  hard_reboot();
            }
         }
         else
         {
            printf("Apply BOOTMODE magic code not valid: 0x%lx\n",zdata);
         }
         break;
      case REG_ZZ_PRESET_SEL:
         if(zdata>=0 && zdata<=7)
         {
            preset_selected=zdata;
            printf("[ENV] Preset %d selected\n",preset_selected);
            load_preset_to_config();

         }
         else if(zdata==8)
         {
            preset_selected=8;
            memcpy(&temp_config,&default_config,sizeof(CONFIG));
            printf("[ENV] Default preset selected (z3660cfg.txt file)\n");
         }
         else
            printf("[ENV] ERROR preset selected %ld out of range\n",zdata);
         break;
      case REG_ZZ_APPLY_ALL:
         if(zdata==0x55AA)
         {
            if(preset_selected>=0)
            {
               read_preset_name();
               piscsi_shutdown();
               if(write_env_files(&env_file_vars_temp[preset_selected])==1)
                  hard_reboot();
            }
         }
         else
         {
            printf("Apply ALL magic code not valid: 0x%lx\n",zdata);
         }
         break;
      case REG_ZZ_SCSIBOOT_EN:
         if(preset_selected>=0)
         {
            env_file_vars_temp[preset_selected].scsiboot=zdata;
            printf("SCSI BOOT %s\n",zdata?"enabled":"disabled");
         }
         break;
      case REG_ZZ_AUTOC_RAM_EN:
         if(preset_selected>=0)
         {
            env_file_vars_temp[preset_selected].autoconfig_ram=zdata;
            printf("AUTOCONFIG RAM %s\n",zdata?"enabled":"disabled");
         }
         break;
      case REG_ZZ_AUTOC_RTG_EN:
         if(preset_selected>=0)
         {
            env_file_vars_temp[preset_selected].autoconfig_rtg=zdata;
            printf("AUTOCONFIG RTG %s\n",zdata?"enabled":"disabled");
         }
         break;
      case REG_ZZ_KS_SEL:
         if(preset_selected>=0)
         {
            env_file_vars_temp[preset_selected].kickstart=zdata;
            //printf("KICKSTART SELECT %ld\n",zdata);
         }
         break;
      case REG_ZZ_EXT_KS_SEL:
         if(preset_selected>=0)
         {
            env_file_vars_temp[preset_selected].ext_kickstart=zdata;
            //printf("EXT KICKSTART SELECT %ld\n",zdata);
         }
         break;
      case REG_ZZ_SCSI_SEL_0:
      case REG_ZZ_SCSI_SEL_1:
      case REG_ZZ_SCSI_SEL_2:
      case REG_ZZ_SCSI_SEL_3:
      case REG_ZZ_SCSI_SEL_4:
      case REG_ZZ_SCSI_SEL_5:
      case REG_ZZ_SCSI_SEL_6:
         if(preset_selected>=0)
         {
            env_file_vars_temp[preset_selected].scsi_num[(address-REG_ZZ_SCSI_SEL_0)>>2]=zdata-1;
            //printf("SCSI SELECT %ld\n",zdata);
         }
         break;
      case REG_ZZ_KS_SEL_TXT:
      {
         int j=0;
         char c=-1;
         switch(zdata)
         {
            case 0:
            {
               char string[]="Amiga mother board kickstart";
               while(c!=0)
               {
                  c=string[j];
                  *(char*)(RTG_BASE+REG_ZZ_SEL_KS_TXT+j)=c;
                  j++;
                  /*printf("%c",c);*/
               }
               for(c=0;c<4-(j&3);c++)
                  *(char*)(RTG_BASE+REG_ZZ_SEL_KS_TXT+j+c)=0;
               /*printf("\n");*/
               break;
            }
#define KICKSTART(X) case X:\
      while(c!=0)\
      {\
         c=temp_config.kickstart ## X[j];\
         *(char*)(RTG_BASE+REG_ZZ_SEL_KS_TXT+j)=c;\
         j++;\
         /*printf("%c",c);*/\
      }\
      for(c=0;c<4-(j&3);c++)\
      *(char*)(RTG_BASE+REG_ZZ_SEL_KS_TXT+j+c)=0;\
      /*if(j>1)\
                        printf("\n");*/\
            break;
            KICKSTART(1)
            KICKSTART(2)
            KICKSTART(3)
            KICKSTART(4)
            KICKSTART(5)
            KICKSTART(6)
            KICKSTART(7)
            KICKSTART(8)
            KICKSTART(9)
            }
         }
         break;
         case REG_ZZ_EXT_KS_SEL_TXT:
         {
            int j=0;
            char c=-1;
            switch(zdata)
            {
            case 0:
            {
               char string[]="Amiga mother board ext kickstart";
               while(c!=0)
               {
                  c=string[j];
                  *(char*)(RTG_BASE+REG_ZZ_SEL_KS_TXT+j)=c;
                  j++;
                  /*printf("%c",c);*/
               }
               for(c=0;c<4-(j&3);c++)
                  *(char*)(RTG_BASE+REG_ZZ_SEL_KS_TXT+j+c)=0;
               /*printf("\n");*/
               break;
            }
#define EXT_KICKSTART(X) case X:\
      while(c!=0)\
      {\
         c=temp_config.ext_kickstart ## X[j];\
         *(char*)(RTG_BASE+REG_ZZ_SEL_KS_TXT+j)=c;\
         j++;\
         /*printf("%c",c);*/\
      }\
      for(c=0;c<4-(j&3);c++)\
      *(char*)(RTG_BASE+REG_ZZ_SEL_KS_TXT+j+c)=0;\
      /*if(j>1)\
                        printf("\n");*/\
            break;
            EXT_KICKSTART(1)
            EXT_KICKSTART(2)
            EXT_KICKSTART(3)
            EXT_KICKSTART(4)
            EXT_KICKSTART(5)
            EXT_KICKSTART(6)
            EXT_KICKSTART(7)
            EXT_KICKSTART(8)
            EXT_KICKSTART(9)
         }
      }
      break;
      case REG_ZZ_SCSI_SEL_TXT:
      {
         int j=0;
         char c=-1;
         if(zdata>=0 && zdata<=19)
         {
            while(c!=0)
            {
               c=temp_config.hdf[zdata][j];
               *(char*)(RTG_BASE+REG_ZZ_SEL_SCSI_TXT+j)=c;
               j++;
            }
            for(c=0;c<4-(j&3);c++)
               *(char*)(RTG_BASE+REG_ZZ_SEL_SCSI_TXT+j+c)=0;
         }
      }
      break;
      case REG_ZZ_PRESET_SEL_TXT:
      {
         int j=0;
         char c=-1;
         if(zdata>=0 && zdata<=7)
         {
            while(c!=0)
            {
               c=env_file_vars_temp[zdata].preset_name[j];
               *(char*)(RTG_BASE+REG_ZZ_SEL_PRESET_TXT+j)=c;
               j++;
            }
            for(c=0;c<4-(j&3);c++)
               *(char*)(RTG_BASE+REG_ZZ_SEL_PRESET_TXT+j+c)=0;
         }
      }
      break;
      case REG_ZZ_TEST_ENABLE:
         if(preset_selected>=0)
            env_file_vars_temp[preset_selected].enable_test=zdata;
         printf("TEST ENABLE %s\n",zdata?"enabled":"disabled");
         break;
      case REG_ZZ_BPTON:
         env_file_vars_temp[preset_selected].bp_ton=zdata*(TON_MAX/SLIDER_BPTON_MAX);
         printf("BP_TON = %7.1f us\n",env_file_vars_temp[preset_selected].bp_ton*1e6);
         test_beeper(ENV_INT_TON,ENV_INT_TOFF);
         break;
      case REG_ZZ_BPTOFF:
         env_file_vars_temp[preset_selected].bp_toff=zdata*(TOFF_MAX/SLIDER_BPTOFF_MAX);
         printf("BP_TOFF = %7.1f ms\n",env_file_vars_temp[preset_selected].bp_toff*1e3);
         test_beeper(ENV_INT_TON,ENV_INT_TOFF);
         break;
      case REG_ZZ_DELETE_PRESET:
         if(zdata==0x55AA)
         {
            if(preset_selected>=0 && preset_selected<=7)
            {
               printf("Delete Preset %d\n",preset_selected);
               if(delete_selected_preset())
               {
                  preset_selected=8;
                  printf("[ENV] Preset %d selected\n",preset_selected);
                  load_preset_to_config();

                  read_preset_name();
                  piscsi_shutdown();
                  if(write_env_files(&env_file_vars_temp[preset_selected])==1)
                     hard_reboot();
               }
            }
         }
         else
         {
            printf("Delete Preset magic code not valid: 0x%lx\n",zdata);
         }
         break;
      default:
         if(debug_console.debug_rtg)
            printf("Write to unknown %08lx RTG register\n",address); // write to an unknown RTG register
         break;
   }
}

void audio_reset(void)
{
   audio_set_interrupt_enabled(0);
   audio_silence();
   audio_init_i2s();
}
