// This file has two main functions: mother board test, and BOOT.bin update system.
#include <inttypes.h>
#include "mobotest.h"
#include <xparameters.h>
#include "main.h"
#include "lwip.h"
#ifdef USE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/sys.h"
#include "arch/sys_arch.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "rtg/fonts.h"
#include "config_file.h"
#include "xtime_l.h"
#include "xil_mmu.h"
#include "lwip/tftp_server.h"
#include "lwip/web_utils.h"
#include <stdbool.h>
#include "sii9022_init/sii9022_init.h"
#include "usb/usb_test_standalone.h"

#include "pt/pt.h"
#include "ARM_ztop/tabs.h"
#include "usb.h"

#include "ARM_ztop/textedit.h"
#include "ARM_ztop/tabs.h"
#include "coremark/coremark_port.h"
int init_xc3sprog(void);
int i2c_finish(void);
int main_xc3sprog(void);
int measures_thread(struct pt *pt);
extern uint32_t ticks;
void b_refresh_action(void);
extern int selected_tab;
extern uint32_t fb_pitch;
#include "ARM_ztop/button.h"
extern Button *b_apply_all_timings;
void test_tab_timings(void);
void ns_repaint(void);
extern void *(memcpy_neon)(void * s1, const void * s2, u32 n);
void configure_clk(int clk, int verbose, int emu);

server_t server=GITHUB_SERVER;

int timer_thread(struct pt *pt)
{
   PT_BEGIN(pt);
   while(1)
   {
      PT_WAIT_UNTIL(pt,ticks>=1000); // ~1 second
      ticks=0;
//      printf("tick\n");
      if(selected_tab==TAB_INFO)
         b_refresh_action();
   }
   PT_END(pt);
}
int ltc2990_init(void);
extern long int task_counter;

extern struct pt pt_measures;
static struct pt pt_timer;
sFONT *Font=&Font20;
extern SHARED *shared;
#define NOP __asm(" NOP")

void NOPX_WRITE(void)
{
   for(int i=shared->nops_write;i>0;i--)
   {
      NOP;
   }
}

// EEMBC CoreMark standard benchmark for ARM performance measurement
void test_coremark(int freq_code)
{
   extern void print_hdmi_ln(int xpos, char *message, int line_inc);
   extern int core_main(int argc, char *argv[]); // CoreMark main function (renamed from main)
   
   char message[100];
   
   sprintf(message,"=== EEMBC CoreMark BENCHMARK ===");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   sprintf(message,"CoreMark standard for Cortex-A9 @ %s MHz", arm_frequency_names[config.arm_frequency]);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   sprintf(message,"Running CoreMark standard benchmark...");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   // Execute standard CoreMark - it will capture and return the score
   float return_value = 0.0f;
   int coremark_result = coremark_run(&return_value);
   
   if (return_value > 0.0f) {
      sprintf(message,"CoreMark score : %.2f", return_value);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      switch(freq_code)
      {
         case 0:
            sprintf(message,"CoreMark/MHz : %.4f", return_value/667.0);
            break;
         case 1:
            sprintf(message,"CoreMark/MHz : %.4f", return_value/767.0);
            break;
         case 2:
            sprintf(message,"CoreMark/MHz : %.4f", return_value/867.0);
            break;
         case 3:
            sprintf(message,"CoreMark/MHz : %.4f", return_value/900.0);
            break;
         case 4:
            sprintf(message,"CoreMark/MHz : %.4f", return_value/933.0);
            break;
         case 5:
            sprintf(message,"CoreMark/MHz : %.4f", return_value/967.0);
            break;
         case 6:
            sprintf(message,"CoreMark/MHz : %.4f", return_value/1000.0);
            break;
         default:
            sprintf(message,"Error, freq_code %d unknown", freq_code);
      }
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   } else {
      sprintf(message,"CoreMark score not captured");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }
   
   sprintf(message,"CoreMark benchmark completed");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   sprintf(message,"Return code: %d", coremark_result);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
}
void NOPX_READ(void)
{
   for(int i=shared->nops_read;i>0;i--)
   {
      NOP;
   }
}
#include "ff.h"

#define check_bus_error(A,B)

#define REG_BASE_ADDRESS XPAR_Z3660_0_BASEADDR

#define write_reg64(Offset,Data) (*(volatile uint64_t *)(REG_BASE_ADDRESS+(Offset)))=(Data)
#define write_reg(Offset,Data)   (*(volatile uint32_t *)(REG_BASE_ADDRESS+(Offset)))=(Data)

#define read_mem32(Offset) (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x14000000+((Offset&0x00FFFFFF)<<2)))=(0)
#define read_mem16(Offset) (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x18000000+((Offset&0x00FFFFFF)<<2)))=(0)
#define read_mem8(Offset)  (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x1C000000+((Offset&0x00FFFFFF)<<2)))=(0)

uint32_t last_bank=-1;

void arm_write_amiga_long(uint32_t address, uint32_t data, int *timeout)
{
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   write_mem32(address,data);
   NOP;
   do {
      NOPX_WRITE();
      (*timeout)--;
   }
   while((read_reg(REG5)&0x3)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   write_reg(REG4,0x0); // ARM Bus Hi-Z
}
void arm_write_long(uint32_t address, uint32_t data, int *timeout)
{
   if(address< 0x08000000)
      arm_write_amiga_long(address, data, timeout);
   else if(address<0x10000000)
      *(uint32_t*)address=data;
}
void arm_write_amiga_word(uint32_t address, uint32_t data, int *timeout)
{
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   write_mem16(address,data);
   NOP;
   do {
      NOPX_WRITE();
      (*timeout)--;
   }
   while((read_reg(REG5)&0x3)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   write_reg(REG4,0x0); // ARM Bus Hi-Z
}
void arm_write_amiga_byte(uint32_t address, uint32_t data, int *timeout)
{
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   write_mem8(address,data);
   NOP;
   do {
      NOPX_WRITE();
      (*timeout)--;
   }
   while((read_reg(REG5)&0x3)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   write_reg(REG4,0x0); // ARM Bus Hi-Z
}

uint32_t arm_read_amiga_long(uint32_t address, int *timeout)
{
//   while(read_reg(REG5)&0x80000000);        // previous write ack
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   read_mem32(address);
   NOP;
   do {
      NOPX_READ();
      (*timeout)--;
   }
   while(read_reg(REG5)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   uint32_t data_read=read_reg(REG7); // read data
   write_reg(REG4,0x0); // ARM Bus Hi-Z
   return(data_read);
}
uint32_t arm_read_amiga_word(uint32_t address, int *timeout)
{
//   while(read_reg(REG5)&0x80000000);        // previous write ack
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   read_mem16(address);
   NOP;
   do {
      NOPX_READ();
      (*timeout)--;
   }
   while(read_reg(REG5)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   write_reg(REG4,0x0); // ARM Bus Hi-Z
   uint32_t data_read=read_reg(REG7); // read data
   return(data_read);
}
uint32_t arm_read_long(uint32_t address, int *timeout)
{
   if(address< 0x08000000)
      return arm_read_amiga_long(address, timeout);
   else if(address<0x10000000)
      return *(uint32_t*)address;
   else return 0xFFFFFFFF;
}
#define ps_read_byte(A,T) ((arm_read_amiga_byte(A,T)>>((3-(A&3))*8))&0xFF)
#define ps_write_byte(A,D,T) arm_write_amiga_byte(A,(D&0xFF)<<((3-(A&3))*8),T)

uint32_t arm_read_amiga_byte(uint32_t address, int *timeout)
{
//   while(read_reg(REG5)&0x80000000);        // previous write ack
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   read_mem8(address);
   NOP;
   do {
      NOPX_READ();
      (*timeout)--;
   }
   while(read_reg(REG5)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   write_reg(REG4,0x0); // ARM Bus Hi-Z
   uint32_t data_read=read_reg(REG7); // read data
   return(data_read);
}
char message[300]={0};
void handle_cache_flush(uint32_t address,uint32_t size);
extern ZZ_VIDEO_STATE vs;
void copy_rect_mobotest( int line)
{
#define LINE_MAX_FONT ((int)((vs.vmode_vsize/Font->Height) - 1))
   int delta=(line-LINE_MAX_FONT)*Font->Height;
   uint32_t h = vs.vmode_vsize - delta;
   uint32_t w = vs.vmode_hsize;
   uint16_t *dp=(uint16_t *)(FRAMEBUFFER_ADDRESS);
   uint16_t *sp=(uint16_t *)(FRAMEBUFFER_ADDRESS+delta*w*2);
   for (uint16_t y_line = 0; y_line < h; y_line++,dp += w,sp += w)
      memcpy_neon(dp, sp, w * 2);
   for (uint16_t y_line = h; y_line < vs.vmode_vsize; y_line++,dp += w)
      memset(dp, 0, w * 2);
}
int line=0;

void print_hdmi(int xpos, char *message)
{
   displayStringAt(Font,xpos*Font->Width,line*Font->Height,(uint8_t*)message,LEFT_MODE);
//   handle_cache_flush(((uint32_t)vs.framebuffer) + vs.framebuffer_pan_offset,vs.framebuffer_size);
//   while(video_formatter_read(0)==1) //wait vblank
//   {}
//   while(video_formatter_read(0)==0)
//   {}
}
void print_hdmi_ln(int xpos, char *message, int line_inc)
{
   print_hdmi(xpos,message);
   line+=line_inc;
   if(line>LINE_MAX_FONT)
   {
      copy_rect_mobotest(line);
      line=LINE_MAX_FONT;
   }
//   handle_cache_flush(((uint32_t)vs.framebuffer) + vs.framebuffer_pan_offset,vs.framebuffer_size);
//   while(video_formatter_read(0)==1) //wait vblank
//   {}
//   while(video_formatter_read(0)==0)
//   {}
}
int screen_width=800;
void print_hdmi_ln_centered(char *message, int line_inc)
{
   int x=(screen_width/Font->Width-strlen(message))/2;
   print_hdmi_ln(x, message, line_inc);
}
char kb_tbl[256]=
   "`1234567890-=\\\x00" "0"
   "qwertyuiop[]" "\x00" "123"
   "asdfghjkl;'\x00\x00" "456"
   "\x00zxcvbnm,./\x00.789"
   " \x08\x09\x0d\x0d\x1b\x7f\x00\x00\x00-\x00\x1f\x1e\x1d\x1c" // 28-31 Cursor Keys
   "\xf9\xf8\xf7\xf6\xf5\xf4\xf3\xf2\xf1\xf0()/*+\xfa"          // F-Keys, Help
   "\xfe\xfe\xfb\xff\xfd\xfd\xfc\xfc"                           // Shift, Ctrl, Alt, Amiga
   "\x80\x81\x82\x83\x84\x85\x86\x87"
   "\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97"
   "~!@#$%^&*()_+|\x00" "0"
   "QWERTYUIOP{}\x00" "123"
   "ASDFGHJKL:\x22\x00\x00" "456"
   "\x00ZXCVBNM<>?\x00.789"
   " \x08\x09\x0d\x0d\x1b\x7f\x00\x00\x00-\x00\x1f\x1e\x1d\x1c" // 28-31 Cursor Keys
   "\xf9\xf8\xf7\xf6\xf5\xf4\xf3\xf2\xf1\xf0()/*+\xfa"          // F-Keys, Help
   "\xfe\xfe\xfb\xff\xfd\xfd\xfc\xfc"                           // Shift, Ctrl, Alt, Amiga
   "\x80\x81\x82\x83\x84\x85\x86\x87"
   "\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97"
   ;
void keyboard_handshake(void)
{
   int timeout=0;
   ps_write_byte(0xbfee01,0x50,&timeout);
   usleep(100);
   ps_write_byte(0xbfee01,0x10,&timeout);
   usleep(1000);
}
#define L_SHIFT   0x60
#define R_SHIFT   0x61
#define CAPS_LOCK 0x62
#define L_AMIGA   0x66
#define R_AMIGA   0x67
uint8_t shift_status=0;
uint8_t amiga_status=0;
int16_t mousex=0,mousey=0;
uint8_t counterx=0,countery=0;
uint8_t mouse_pressed=0,mouse_pressed_old=-1;
void do_clip_hw_sprite(int16_t offset_x, int16_t offset_y);
void do_update_hw_sprite_pos(int16_t x, int16_t y);
void update_mouse_position(void)
{
   if(mousex<0) mousex=0;
   if(mousex>(int16_t)(vs.vmode_hsize-1)) mousex=vs.vmode_hsize-1;
   if(mousey<0) mousey=0;
   if(mousey>(int16_t)(vs.vmode_vsize-1)) mousey=vs.vmode_vsize-1;
   vs.sprite_x_base=mousex;
   vs.sprite_y_base=mousey;
   do_update_hw_sprite_pos(vs.sprite_x_base, vs.sprite_y_base);
   video_formatter_write((vs.sprite_y_adj << 16) | vs.sprite_x_adj, MNTVF_OP_SPRITE_XY);

   update_hw_sprite_pos();
}
void calc_mouseposition(uint16_t joy0dat)
{
   uint8_t counter_new;
   counter_new=(joy0dat>>8)&0xFF;
   int16_t diff=counter_new-countery;
   if(diff > 127)
      mousey += diff - 256;
   else if(diff < -128)
      mousey += diff + 256;
   else
      mousey += diff;
   countery=counter_new;

   counter_new=(joy0dat   )&0xFF;
   diff=counter_new-counterx;
   if(diff > 127)
      mousex += diff - 256;
   else if(diff < -128)
      mousex += diff + 256;
   else
      mousex += diff;
   counterx=counter_new;

   update_mouse_position();
//   char message2[20];
//   sprintf(message2,"%04d,%04d",mousex,mousey);
//   displayStringAt(Font,0,0,(uint8_t*)message2,LEFT_MODE);
}
bool enable_amiga_keyboard_read=true;
uint8_t read_keyboard(uint8_t *data, int enable_mouse)
{
   if(XUartPs_IsReceiveData(STDIN_BASEADDRESS))
   {
      *data=XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
      return(1);
   }
   if(enable_amiga_keyboard_read)
   {
      if(enable_mouse)
      {
         int timeout;
         uint16_t joyy0dat=arm_read_amiga_word(0xdff00a,&timeout)&0xFFFF;
         calc_mouseposition(joyy0dat);
         uint8_t mousedat=ps_read_byte(0xbfe001,&timeout);
         mouse_pressed=mousedat&(1<<6)?0:1;
      }
      int timeout;
      uint8_t kb_int=ps_read_byte(0xbfed01,&timeout);
      if(timeout==0)
      {
         static int retry=0;
         if(retry==10)
         {
            enable_amiga_keyboard_read=false;
            sprintf(message,"Can't access to the Amiga keyboard -> Serial Debug used as keyboard");
            printf("%s\n",message);
            Font->TextColor=0x00FF0000; // red
            int line_old=line;
            line = 1; // show this at top
            print_hdmi_ln_centered(message,0);
            line=line_old;
            Font->TextColor=0x00FFFFFF; // white
         }
         else
         {
            retry++;
         }
      }
      if((kb_int&(1<<3))==0)
         return(0);
      ps_write_byte(0xbfed01,0x00,&timeout);

      uint8_t kb_data=ps_read_byte(0xbfec01,&timeout);
      uint8_t data1=(~(kb_data>>1))&0x7F;
      keyboard_handshake();
//      sprintf(message,"kb_data %02X  data1  %02X      ",kb_data,data1);
//      line++;
//      print_hdmi(0,message);
//      line--;
      uint8_t down=kb_data&0x01;
      if(data1==L_SHIFT || data1==R_SHIFT || data1==CAPS_LOCK)
      {
         shift_status=down?1:0;
//         sprintf(message,"changed shift_status %d      ",shift_status);
//         line+=2;
//         print_hdmi(0,message);
//         line-=2;
         return(0);
      }
      if(data1==L_AMIGA || data1==R_AMIGA)
      {
         amiga_status=down?1:0;
         return(0);
      }
      if(shift_status)
         data1=data1+128;
      *data=kb_tbl[data1];
      return(down);
   }
   return(0);
}
extern char version_string_export[];
extern char version_scsirom_string_export[];
extern char version_jed_string_export[];
uint32_t read_rtg_register(uint32_t zaddr);
extern DOWNLOAD_DATA download_data;

void hard_reboot(void);
void reboot(void)
{
   sprintf(message,"Reboot in 1 second...");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   usleep(1000000);
   hard_reboot();
}
int v_major=1;
int v_minor=3;
int beta=0;
int alfa=0;
int mio14=0;
extern uint32_t counts_per_second;
void test_nops(void)
{
   XTime total_time_start = 0;
   XTime total_time_stop;
   XTime debug_time_start = 0;
   XTime debug_time_stop;
   float read_mbs[10];
   float write_mbs[10];

   fpga_interrupt_connect(isr_video, isr_audio_tx, isr_usb, INT_IPL_ON_THIS_CORE);
#define TOTAL_MEM_TESTED 0x40000
#define TESTED_TIMES 8
   sprintf(message,"NOPS ticks (READ)");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   XTime_GetTime(&total_time_start);
   for(int r=0;r<10;r++)
   {
      shared->nops_read=r;
      XTime_GetTime(&debug_time_start);
      int timeout;
      for(int j=0;j<TESTED_TIMES;j++)
         for(uint32_t i=0;i<TOTAL_MEM_TESTED;i+=4)
            arm_read_amiga_long((0x200000-TOTAL_MEM_TESTED-0x10000)+i,&timeout);
      XTime_GetTime(&debug_time_stop);
      read_mbs[r]=(TESTED_TIMES*((float)TOTAL_MEM_TESTED)/1024./1024.*(counts_per_second))/((1.0 * (debug_time_stop-debug_time_start)));
      sprintf(message," %2d %6.4f MB/s",r,read_mbs[r]);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }
   float max=0;
   int opt_r=0;
   for(int r=0;r<10;r++)
   {
      if(read_mbs[r]>max)
      {
         max=read_mbs[r];
         opt_r=r;
      }
   }
   sprintf(message,"Optimal NOPS for READ: %d (%6.4f MB/s)",opt_r,read_mbs[opt_r]);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   shared->nops_read=opt_r;

   sprintf(message,"NOPS ticks (WRITE)");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   for(int w=0;w<10;w++)
   {
      shared->nops_write=w;
      XTime_GetTime(&debug_time_start);
      int timeout;
      for(int j=0;j<TESTED_TIMES;j++)
         for(uint32_t i=0;i<TOTAL_MEM_TESTED;i+=4)
            arm_write_amiga_long((0x200000-TOTAL_MEM_TESTED-0x10000)+i,i,&timeout);
      XTime_GetTime(&debug_time_stop);
      write_mbs[w]=(TESTED_TIMES*((float)TOTAL_MEM_TESTED)/1024./1024.*(counts_per_second))/((1.0 * (debug_time_stop-debug_time_start)));
      sprintf(message," %2d %6.4f MB/s",w,write_mbs[w]);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }
   max=0;
   int opt_w=0;
   for(int w=0;w<10;w++)
   {
      if(write_mbs[w]>max)
      {
         max=write_mbs[w];
         opt_w=w;
      }
   }
   XTime_GetTime(&total_time_stop);
   sprintf(message,"Optimal NOPS for WRITE: %d (%6.4f MB/s)",opt_w,write_mbs[opt_w]);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   shared->nops_read=opt_w;
   float total_time=(1.0 * (total_time_stop-total_time_start)) / (counts_per_second);
   sprintf(message,"Total time: %4.1f s (%3.1f MB)",total_time,(20.*TOTAL_MEM_TESTED*TESTED_TIMES)/1024./1024.);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
}
uint8_t keybd_data=0;

void show_options(void)
{
#define MSG_LINE(X) sprintf(message,X);print_hdmi_ln(0,message,1);printf("%s\n",message)
   MSG_LINE("Update Options:");
   MSG_LINE("'H' - Show this help menu");
   MSG_LINE("'I' - Connect to the Internet and check for updates");
   MSG_LINE("'S' - Change the update server");
   MSG_LINE("'O' - Download the latest z3660_scsi.rom (overwrites z3660scsi.rom on exFAT)");
   MSG_LINE("'T' - Test download of latest BOOT.bin (does NOT write to storage)");
   MSG_LINE("'U' - Download and overwrite Z3660.bin with latest BOOT.bin");
   MSG_LINE("'F' - Download and overwrite FAILSAFE.bin with latest BOOT.bin");
   MSG_LINE("'D' - Download and flash the latest CPLD firmware");
   MSG_LINE("      (if not connected to the Internet, flashes the previously downloaded file)");
   MSG_LINE("'P' - Start a TFTP server connected to your network");
   MSG_LINE("Test Options:");
   MSG_LINE("'X'  - Test read/write and determine optimal NOPS values for EMU (only for developers)");
   MSG_LINE("'Q'  - CoreMark performance test for ARM CPU");
   MSG_LINE("File Management Options:");
   MSG_LINE("'B' - Copy BOOT.bin to FAILSAFE.bin");
   MSG_LINE("'N' - Ccopy BOOT.bin to Z3660.bin");
   MSG_LINE("'M' - Copy Z3660.bin to BOOT.bin");
   MSG_LINE("'J' - Copy Z3660.bin to FAILSAFE.bin");
   MSG_LINE("'K' - Ccopy FAILSAFE.bin to BOOT.bin");
   MSG_LINE("'L' - Copy FAILSAFE.bin to Z3660.bin");
   MSG_LINE(" ");
   MSG_LINE("'Y' - Test USB mouse");
   MSG_LINE(" ");
   MSG_LINE("'R' to reboot");
   MSG_LINE(" ");
   MSG_LINE("Note: BOOT.bin initializes the system and then attempts to load Z3660.bin to continue booting.");
   MSG_LINE("      If Z3660.bin is invalid or missing, it will attempt to boot from FAILSAFE.bin.");
   MSG_LINE("      If both fail, BOOT.bin will continue booting on its own.");
   MSG_LINE(" ");
}
void mobo_change_jumpers(int reg6)
{
   for(int i=0x180;i<0x200;i++) // RTG force no cache
      Xil_SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);

   vs.sprite_x_base = 3000;
   vs.sprite_y_base = 3000;
   do_update_hw_sprite_pos(vs.sprite_x_base, vs.sprite_y_base);
   video_formatter_write((vs.sprite_y_adj << 16) | vs.sprite_x_adj, MNTVF_OP_SPRITE_XY);
   switch(config.bootscreen_resolution)
   {
   case RES_1920x1080:
      Font=&Font20;
      break;
   case RES_1280x720:
      Font=&Font12;
      break;
   case RES_800x600:
   default:
      Font=&Font12;
      break;
   }

   Font->TextColor=0x00FFFFFF; // white
   Font->BackColor=0x00000000; // black

   int w=1920;
   int h=1080;
//   if(config.enable_test!=_YES)
   {
      int offset=8;
      switch(config.bootscreen_resolution)
      {
      case RES_1920x1080:
         w=1920;
         h=1080;
         offset=11;
         break;
      case RES_1280x720:
         w=1280;
         h=720;
         offset=12;
         break;
      case RES_800x600:
      default:
         w=800;
         h=600;
         offset=12+5;
         break;
      }
      line=(h/Font->Height - 1)- offset; // 43 @ 1080p
   }
   screen_width=w;
   if((reg6&(FPGA_CLK90_DETECTED|FPGA_CPUCLK_DETECTED))==FPGA_CPUCLK_DETECTED)
      sprintf(message,"FPGA detected CPUCLK!!! For the love of your Amiga, change it now to EXT position !!!");
   else if((reg6&(FPGA_CLK90_DETECTED|FPGA_CPUCLK_DETECTED))==FPGA_CLK90_DETECTED)
      sprintf(message,"FPGA detected CLK90!!! For the love of your Amiga, change it now to EXT position !!!");
   else
      sprintf(message,"FPGA detected CPUCLK and CLK90!!! For the love of your Amiga, change them now to EXT position !!!");

   int x=(w/Font->Width-strlen(message))/2;
   Font->TextColor=0x00FFFFFF;
   Font->BackColor=0x00303030;
   print_hdmi_ln(x,message,2);
   printf("%s\n",message);
}
void mobotest(int sw1_is_down)
{
   for(int i=0x180;i<0x200;i++) // RTG force no cache
      Xil_SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);

   vs.sprite_x_base = 3000;
   vs.sprite_y_base = 3000;
   do_update_hw_sprite_pos(vs.sprite_x_base, vs.sprite_y_base);
   video_formatter_write((vs.sprite_y_adj << 16) | vs.sprite_x_adj, MNTVF_OP_SPRITE_XY);
   switch(config.bootscreen_resolution)
   {
   case RES_1920x1080:
      Font=&Font20;
      break;
   case RES_1280x720:
      Font=&Font12;
      break;
   case RES_800x600:
   default:
      Font=&Font12;
      break;
   }

   Font->TextColor=0x00FFFFFF; // white
   Font->BackColor=0x00000000; // black

   NBR_ARM(0);        // bus request
   usleep(100000);
   CPLD_RESET_ARM(1); // CPLD RUN
   int timeout1=0;
   printf("READ_NBG_ARM()\n");
   while(READ_NBG_ARM()!=0) // make sure that we have the bus control
   {
      usleep(1000);
      timeout1++;
      if(timeout1==1000)
         break;
   }

#ifdef CPLD_PROGRAMMING
   if(timeout1==1000
      || XGpioPs_ReadPin(&GpioPs, n040RSTI)==0
      || sw1_is_down)
   {
      if(sw1_is_down)
         sprintf(message,"SW1 pressed -> CPLD programming...");
      else
         sprintf(message,"Timeout waiting NBG_ARM (CPLD erased?)");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sw1_is_down=0;
      CPLD_RESET_ARM(0);
      Xil_ExceptionDisable();
      int err=init_xc3sprog();
      if(err==0)
      {
         main_xc3sprog();
      }
      else
      {
         //We will not have access to the Amiga keyboard, so enable serial debug as keyboard
         enable_amiga_keyboard_read=false;
         printf("We will not have access to the Amiga keyboard, so enable serial debug as keyboard\n");
      }
      i2c_finish();
      Xil_ExceptionEnable();
      CPLD_RESET_ARM(1);
   }
#endif
   usleep(1000);

   int failed=0;
   int ovl_failed=0;
   int timeout;
   if(config.enable_test==_YES || config.enable_test==_MIN)
   {
      sprintf(message,"[TEST] Simple Amiga bus test...");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);

      sprintf(message,"[TEST] 0xbfe201 %02lX",ps_read_byte(0xbfe201,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sprintf(message,"[TEST] 0xbfe001 %02lX",ps_read_byte(0xbfe001,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);

      sprintf(message,"[TEST] Setting new values");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
//#define FORCE_OVL_FAIL
#ifndef FORCE_OVL_FAIL
      ps_write_byte(0xbfe201,0x01,&timeout); // OVL as OUTPUT
#endif
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 1 to 0xbfe201");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      ps_write_byte(0xbfe001,0x00,&timeout); // OVL LOW to see the CHIP RAM
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0 to 0xbfe001");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      sprintf(message,"[TEST] 0xbfe201 %0lX",ps_read_byte(0xbfe201,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sprintf(message,"[TEST] 0xbfe001 %0lX",ps_read_byte(0xbfe001,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);

      arm_write_amiga_long(0x00000000,0xAA55AA55,&timeout);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0xAA55AA55 to 0x00000000");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      arm_write_amiga_long(0x00100000,0x55AA55AA,&timeout);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0x55AA55AA to 0x00100000");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      uint32_t data1=arm_read_amiga_long(0x00000000,&timeout);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when reading from 0x00000000");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      uint32_t data2=arm_read_amiga_long(0x00100000,&timeout);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when reading from 0x00100000");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      if(data1==0xAA55AA55 && data2==0x55AA55AA)
      {
         sprintf(message,"[TEST] Test basic Amiga CHIP RAM ok");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
      }
      else
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Test basic Amiga CHIP RAM FAILED!!!");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
         if(data1==0xAA55AA55)
         {
            sprintf(message,"[TEST] (0x00000000) -> 0x%08lX == 0xAA55AA55  OK!!!",data1);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
         }
         else if(data1==0x11144EF9)
         {
            ovl_failed=1;
            Font->TextColor=0x00FF0000; // red
            sprintf(message,"[TEST] (0x00000000) -> 0x%08lX == 0x11144EF9 This is the kickstart!!!",data1);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
            Font->TextColor=0x00FFFFFF; // white
         }
         else
         {
            Font->TextColor=0x00FF0000; // red
            sprintf(message,"[TEST] (0x00000000) -> 0x%08lX instead of 0xAA55AA55",data1);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
            Font->TextColor=0x00FFFFFF; // white
         }
         if(data2==0x55AA55AA)
         {
            sprintf(message,"[TEST] (0x00100000) -> 0x%08lX == 0x55AA55AA  OK!!!",data2);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
         }
         else
         {
            Font->TextColor=0x00FF0000; // red
            sprintf(message,"[TEST] (0x00100000) -> 0x%08lX instead of 0x55AA55AA",data2);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
            Font->TextColor=0x00FFFFFF; // white
         }
      }
   }
   if(failed || config.enable_test==_YES)
   {
      if(failed)
         sprintf(message,"[TEST] Something failed -> exhaustive test");
      else
         sprintf(message,"[TEST] Test Enabled in z3660cfg.txt file -> exhaustive test");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);

      usleep(1000000); // wait time to allow monitor to show the screen
      failed=0;
#define NUM_MEM_REGIONS 8
      struct mem_region {
         uint8_t *test_mem1;
         uint8_t *test_mem2;
         uint32_t start;
         uint32_t len;
         char name[50];
         int failed;
      };
      struct mem_region mem_regions[NUM_MEM_REGIONS];
      int max_str_length=0;
      for(int i=0;i<NUM_MEM_REGIONS;i++)
      {
         mem_regions[i].start=config.test_range[i].start;
         mem_regions[i].len=config.test_range[i].length;
         mem_regions[i].failed=0;
         int len=strlen(config.test_range[i].name);
         if(len>max_str_length) max_str_length=len;
      }
      max_str_length+=17; // "Test Region x () "
      for(int j=0;j<NUM_MEM_REGIONS;j++)
      {
         uint32_t length=mem_regions[j].len;
         if(length>2*1024*1024) length=2*1024*1024; // 2MB max
         mem_regions[j].test_mem1=malloc(length);
         mem_regions[j].test_mem2=malloc(length);
         for(uint32_t i=0;i<length;i++)
            mem_regions[j].test_mem1[i]=(uint8_t)(rand()&0xFF);
      }
      sprintf(message,"[TEST] Writing random data");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      int x;
      int steps=4*1;
      int disp=5;
      int xx_init=75;
      switch(config.bootscreen_resolution)
      {
      case RES_1920x1080:
      case RES_1280x720:
         disp=5;
         xx_init=75;
         break;
      case RES_800x600:
      default:
         disp=4;
         xx_init=75-10;
         break;
      }

      for(int j=0;j<NUM_MEM_REGIONS;j++)
      {
         if(mem_regions[j].len>=128*1024*1024) steps=4*256;
         else if(mem_regions[j].len>=64*1024*1024) steps=4*128;
         else if(mem_regions[j].len>=32*1024*1024) steps=4*64;
         else if(mem_regions[j].len>=16*1024*1024) steps=4*32;
         else if(mem_regions[j].len>=8*1024*1024) steps=4*16;
         else if(mem_regions[j].len>=4*1024*1024) steps=4*8;
         x=max_str_length;
         sprintf(message,"Test Region %d (%s)",j,config.test_range[j].name);
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         for(uint32_t i=mem_regions[j].start,k=0;i<mem_regions[j].start+mem_regions[j].len;i+=steps)
         {
            arm_write_long(i,((uint32_t *)mem_regions[j].test_mem1)[k>>2],&timeout);
            k+=steps;
            if(k>=2*1024*1024) k=0;
            if(timeout==0)
            {
               if(mem_regions[j].failed==0)
                  printf("\n");
               sprintf(message,"[TEST] Timeout when writing 0x%08lX to 0x%08lX",((uint32_t *)mem_regions[j].test_mem1)[i>>2],i);
               printf("%s\n",message);
               Font->TextColor=0x00FF0000; // red
               sprintf(message,"Timeout when writing 0x%08lX to 0x%08lX      ",((uint32_t *)mem_regions[j].test_mem1)[i>>2],i);
               int line_old=line;
               line=0;
               print_hdmi(60,message);
               line=line_old;
               Font->TextColor=0x00FFFFFF; // white
               mem_regions[j].failed=1;
            }
            if(mem_regions[j].failed==0 && (i%(mem_regions[j].len>>disp))==0)
            {
               sprintf(message,".");
               printf("%s",message);
               line--;
               print_hdmi(x++,message);
               line++;
            }
         }
         if(mem_regions[j].failed==0)
         {
            sprintf(message," done");
            printf("%s\n",message);
            line --;
            print_hdmi(x,message);
            line++;
         }
      }
      sprintf(message,"[TEST] Reading data");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      for(int j=0;j<NUM_MEM_REGIONS;j++)
      {
         if(mem_regions[j].len>=128*1024*1024) steps=4*256;
         else if(mem_regions[j].len>=64*1024*1024) steps=4*128;
         else if(mem_regions[j].len>=32*1024*1024) steps=4*64;
         else if(mem_regions[j].len>=16*1024*1024) steps=4*32;
         else if(mem_regions[j].len>=8*1024*1024) steps=4*16;
         else if(mem_regions[j].len>=4*1024*1024) steps=4*8;
         mem_regions[j].failed=0;
         x=max_str_length;
         sprintf(message,"Test Region %d (%s)",j,config.test_range[j].name);
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         for(uint32_t i=mem_regions[j].start,k=0;i<mem_regions[j].start+mem_regions[j].len;i+=steps)
         {
            ((uint32_t *)mem_regions[j].test_mem2)[k>>2]=arm_read_long(i,&timeout);
            k+=steps;
            if(k>=2*1024*1024) k=0;
            if(timeout==0)
            {
               if(mem_regions[j].failed==0)
                  printf("\n");
               sprintf(message,"[TEST] Timeout when reading from 0x%08lX",i);
               printf("%s\n",message);
               Font->TextColor=0x00FF0000; // red
               sprintf(message,"Timeout when reading from 0x%08lX               ",i);
               int line_old=line;
               line=0;
               print_hdmi(60,message);
               line=line_old;
               Font->TextColor=0x00FFFFFF; // white
               mem_regions[j].failed=1;
            }
            if(mem_regions[j].failed==0 && (i%(mem_regions[j].len>>disp))==0)
            {
               sprintf(message,".");
               printf("%s",message);
               line--;
               print_hdmi(x++,message);
               line++;
            }
         }
         if(mem_regions[j].failed==0)
         {
            sprintf(message," done");
            printf("%s\n",message);
            line--;
            print_hdmi(x++,message);
            line++;
         }
      }
      sprintf(message,"Press '1' -> 1k jump, '2' -> 2k jump, '3' -> 8k jump, ...");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sprintf(message," ..., '9' -> 256k jump, '0' -> 512 k jump, 'P' pause/resume, ESC exit");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sprintf(message,"[TEST] Comparing data");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      for(int j=0;j<NUM_MEM_REGIONS;j++)
      {
         if(mem_regions[j].len>=128*1024*1024) steps=4*256;
         else if(mem_regions[j].len>=64*1024*1024) steps=4*128;
         else if(mem_regions[j].len>=32*1024*1024) steps=4*64;
         else if(mem_regions[j].len>=16*1024*1024) steps=4*32;
         else if(mem_regions[j].len>=8*1024*1024) steps=4*16;
         else if(mem_regions[j].len>=4*1024*1024) steps=4*8;
         mem_regions[j].failed=0;
         uint32_t fail_mask=0;
         x=max_str_length;
         sprintf(message,"Test Region %d (%s)",j,config.test_range[j].name);
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         for(uint32_t i=mem_regions[j].start,k=0;i<mem_regions[j].start+mem_regions[j].len;i+=steps)
         {
            if(read_keyboard(&keybd_data,0))
            {
               if(keybd_data>='0' && keybd_data <='9')
               {
                  uint32_t jump_table[10]={512,1,2,4,8,16,32,64,128,256};
                  uint32_t jump=jump_table[keybd_data-'0']*1024;
                  i=(i+jump)&(~(jump-1));
                  if(i>=mem_regions[j].start+mem_regions[j].len)
                     break;
               }
               if(keybd_data=='\x1b') // ESC
               {
                  goto end_test;
               }
               if(keybd_data=='p' || keybd_data=='P')
               {
//                  while(!read_keyboard(&keybd_data,0));
                  while(1) // wait until resume with 'p'
                  {
                     if(read_keyboard(&keybd_data,0))
                     {
                        if(keybd_data=='p' || keybd_data=='P')
                           break;
                     }
                  }
               }
            }
            uint32_t data1=((uint32_t *)mem_regions[j].test_mem1)[k>>2];
            uint32_t data2=((uint32_t *)mem_regions[j].test_mem2)[k>>2];
            k+=steps;
            if(k>=2*1024*1024) k=0;

            if(data1!=data2)
//            if((data1&0xFFFFFFF5)!=(data2&0xFFFFFFF5))
            {
               if(failed==0)
                  printf("\n");
               sprintf(message,"[TEST] Address 0x%08lX failed: W/R %08lX/%08lX",i,data1,data2);
               printf("%s",message);
               mem_regions[j].failed=1;
               Font->TextColor=0x00FF0000; // red
               sprintf(message,"Address 0x%08lX failed: W/R %08lX/%08lX",i,data1,data2);
            }
            else
            {
               Font->TextColor=0x0000FF00; // green
               sprintf(message,"Address 0x%08lX OK:     W/R %08lX/%08lX",i,data1,data2);
            }
            {
               int line_old=line;
               line=0+3*j;
               print_hdmi(xx_init-1,message);
               Font->TextColor=0x00FFFFFF; // white
               sprintf(message,"31     24   23     16   15      8   7       0");
               line=1+3*j;
               print_hdmi(xx_init,message);
               int xx=xx_init;
               line=2+3*j;
               for(int l=31;l>=0;l--)
               {
                  int bit=1L<<l;
                  int data=0;
                  if(fail_mask&bit)
                  {
                     Font->TextColor=0x00FF0000; // red
                     data=1;
                  }
                  else
                  {
                     if((data1&bit)!=(data2&bit))
                     {
                        fail_mask|=bit;
                        Font->TextColor=0x00FF0000; // red
                        data=1;
                     }
                     else
                     {
                        Font->TextColor=0x0000FF00; // green
                     }
                  }
                  int len1=0;
                  if((l%8)==0 && l>0)
                  {
                     sprintf(message,"%s   ",data==1?"X":"O");
                     len1=4;
                  }
                  else if((l%4)==0 && l>0)
                  {
                     sprintf(message,"%s ",data==1?"X":"O");
                     len1=2;
                  }
                  else
                  {
                     sprintf(message,"%s",data==1?"X":"O");
                     len1=1;
                  }
                  print_hdmi(xx,message);
                  xx+=len1;
               }

               line=line_old;
               Font->TextColor=0x00FFFFFF; // white

            }
            if(//mem_regions[j].failed==0 &&
               (i%(mem_regions[j].len>>disp))==0)
            {
               sprintf(message,".");
               if(mem_regions[j].failed==0)
               {
                  printf("%s",message);
               }
               line--;
               print_hdmi(x++,message);
               line++;
            }
            if(ovl_failed)
            {
               if(i<0x80000)
               {
                  i+=0x100-4; // if ovl is not working this space will fail completely, so speed up
               }
            }
         }
         sprintf(message," done");
         printf("%s\n",message);
         if(mem_regions[j].failed==0) sprintf(message," done");
         else sprintf(message," failed");
         line--;
         print_hdmi_ln(x++,message,1);
         sprintf(message,"Press 'C' to continue, 'R' to retry or ESC to exit");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
//         if(mem_regions[j].failed)
         if(mem_regions[j].len>0)
         {
            int exit=0;
            do
            {
               if(read_keyboard(&keybd_data,0))
               {
                  if(keybd_data=='c' || keybd_data=='C')
                     exit=1;
                  if(keybd_data=='r' || keybd_data=='R')
                  {
                     j--;
                     exit=1;
                     fail_mask=0;
                  }
                  if(keybd_data=='\x1b') // ESC
                  {
                     j=NUM_MEM_REGIONS;
                     exit=1;
                  }
               }
            }
            while(exit==0);
         }
      }
end_test:
      for(int j=0;j<NUM_MEM_REGIONS;j++)
      {
         free(mem_regions[j].test_mem1);
         free(mem_regions[j].test_mem2);
      }
      for(int j=0;j<NUM_MEM_REGIONS;j++)
         failed|=mem_regions[j].failed;
   }

   if(failed || config.enable_test==_YES || config.enable_test==_MIN)
   {
      sprintf(message,"[TEST] Restoring...");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      ps_write_byte(0xbfe001,0xFF,&timeout); // OVL HIGH to see the ROM
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0Xff to 0xbfe001");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      ps_write_byte(0xbfe201,0x00,&timeout); // OVL as INPUT
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0 to 0xbfe201");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }

      sprintf(message,"[TEST] 0xbfe201 %02lX",ps_read_byte(0xbfe201,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sprintf(message,"[TEST] 0xbfe001 %02lX",ps_read_byte(0xbfe001,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }
}
extern const char *bootmode_names[];
uint8_t cursor_data[32*48]={
      1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      3, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 3, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 3, 1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 3, 1, 1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 3, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 3, 1, 1, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 3, 1, 0, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 3, 1, 0, 0, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
void monitor_switch(int RTG);
void hdmi_tick(int clean);
void look_for_ver(char *filename)
{
   FIL fil;
   FATFS fatfs;
   TCHAR *Path = "0:/";
   f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
   char filepath[20];
   sprintf(filepath,"0:/%s",filename);
   FRESULT res=f_open(&fil,filepath, FA_READ);
   if(res!=FR_OK)
   {
      sprintf(message,"Can't find the %s file on FAT partition",filename);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      return;
   }
   UINT NumBytesRead;
   FSIZE_t filesize=fil.obj.objsize;
   DATA=malloc(filesize+100);
   ACTIVITY_LED_ON; // ON
   f_read(&fil, DATA, filesize,&NumBytesRead);
   f_close(&fil);
   f_umount(Path);
   ACTIVITY_LED_OFF; // OFF
   uint32_t pos=0;
   while(!(DATA[pos]=='$' && DATA[pos+1]=='V' && DATA[pos+2]=='E'  && DATA[pos+3]=='R'  && DATA[pos+4]==':'))
   {
      if((pos&0xFFFFFL)==0)
         hdmi_tick(0); // roll the bar
      pos++;
      if(pos>=filesize)
         break;
   }
   hdmi_tick(1); // clean the bar
   if(pos>=filesize)
   {
      sprintf(message,"Can't find the $%s: in the %s file on FAT partition","VER",filename); // "VER" hides $VER:
   }
   else
   {
      pos+=5;
      sprintf(message,"%s v%s",filename,&DATA[pos]);
   }
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   free(DATA);
}
int main_thread();
void start_ztop(void);
int sw1_is_down=0;

void update_sd(void)
{
   uint8_t keybd_data=0;
   int data=read_rtg_register(REG_ZZ_FW_VERSION);
   v_major=(data>>8)&0xFF;
   v_minor=(data)&0xFF;
   beta=read_rtg_register(REG_ZZ_FW_BETA);
   alfa=read_rtg_register(REG_ZZ_FW_ALFA);
   int w=1920;
   int h=1080;
//   if(config.enable_test!=_YES)
   {
      int offset=8;
      switch(config.bootscreen_resolution)
      {
      case RES_1920x1080:
         w=1920;
         h=1080;
         offset=11;
         break;
      case RES_1280x720:
         w=1280;
         h=720;
         offset=12;
         break;
      case RES_800x600:
      default:
         w=800;
         h=600;
         offset=12+5;
         break;
      }
      line=(h/Font->Height - 1)- offset; // 43 @ 1080p
   }
   screen_width=w;
   if(beta==0)
      sprintf(message,"Z3660 Firmware %d.%02d (%s)",v_major,v_minor,__DATE__);
   else
   {
      if(alfa==0)
         sprintf(message,"Z3660 Firmware %d.%02d BETA %d (%s)",v_major,v_minor,beta,__DATE__);
      else
         sprintf(message,"Z3660 Firmware %d.%02d BETA %d ALFA %d (%s)",v_major,v_minor,beta,alfa,__DATE__);
   }

//   Font=&Font20;
   int x=(w/Font->Width-strlen(message))/2;
   Font->TextColor=0x00FFFFFF;
   Font->BackColor=0x00303030;
   print_hdmi_ln(x,message,2);
//   Font=&Font12;
   Font->BackColor=0x00404040;
   sprintf(message,"After Power ON, 'C' opens the SD update tool, 'Z' opens ZTop ...");
//   if(config.enable_test==_YES)
//      x=0;
//   else
      x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);

   keybd_data=0;
   int time=config.boot_delay;
   if(config.enable_test==_YES)
      time=config.boot_delay+5;
   printf("  Continuing in %d seconds...  \n",time);
   for(int i=time*1000;i>0;i--)
   {
      if(XGpioPs_ReadPin(&GpioPs, USER_SW1)==0)
      {
         sw1_is_down=1;
         goto start_console;
      }
      if(read_keyboard(&keybd_data,0))
      {
         if(keybd_data=='c' || keybd_data=='C')
         {
            Font->TextColor=0x00FFFFFF; // white
            Font->BackColor=0x00000000;
            memset(vs.framebuffer,0,w*h*4);
            line=0;
            sprintf(message,"Starting console...                 ");
            printf("Starting console... Use the Amiga keyboard\n");
            print_hdmi_ln(0,message,1);
            goto start_console;
         }
         else if(keybd_data=='z' || keybd_data=='Z')
         {
            Font->TextColor=0x00FFFFFF; // white
            Font->BackColor=0x00000000;
            memset(vs.framebuffer,0,w*h*4);
            line=0;
            sprintf(message,"Starting ZTop...                 ");
            printf("Starting ZTop... Use the Amiga keyboard and mouse\n");
            print_hdmi_ln(0,message,1);
            start_ztop();
         }
      }
      if((i%1000)==0)
      {
         sprintf(message,"   Continuing in %i %s   ",i/1000,i/1000==1?"second":"seconds");
//         if(config.enable_test==_YES)
//            x=0;
//         else
            x=(w/Font->Width-strlen(message))/2;
         print_hdmi(x,message);
      }
      usleep(1000);
   }
   sprintf(message,"          Booting...          ");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   if(preset_selected>=0 && preset_selected<PRESET_CB_MAX-1)
      sprintf(message,"Selected preset %d \"%s\"",preset_selected,env_file_vars_temp[preset_selected].preset_name);
   else
      sprintf(message,"Default preset (z3360cfg.txt file)");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   if(config.boot_mode==0)
      sprintf(message,"060 CPU");
   else
      sprintf(message,"%s CPU EMULATOR",bootmode_names[config.boot_mode]);
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   sprintf(message,"BUS Frequency %d MHz",config.cpufreq);
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   sprintf(message,"Z3 RAM %s",config.autoconfig_ram?"Enabled":"Disabled");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   sprintf(message,"Z3 RTG AUTOCONFIG %s",config.autoconfig_rtg?"Enabled":"Disabled");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   sprintf(message,"SCSI BOOT %s",config.scsiboot?"Enabled":"Disabled");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   sprintf(message,"MAC ADDRESS %02X:%02X:%02X:%02X:%02X:%02X",config.mac_address[0],
         config.mac_address[1],config.mac_address[2],
         config.mac_address[3],config.mac_address[4],config.mac_address[5]);
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

//   NBR_ARM(1);
   usleep(1000);
   CPLD_RESET_ARM(0); // CPLD RESET
   return;

start_console:
#ifndef USE_RTOS
   main_thread();
#else
#define DEFAULT_THREAD_PRIO 2
#define THREAD_STACKSIZE 1024*4

   sys_thread_new("main_thrd", (void(*)(void*))main_thread, 0,
                THREAD_STACKSIZE,
                DEFAULT_THREAD_PRIO);
   vTaskStartScheduler();
   while(1);
#endif
}
int main_thread()
{
#ifdef USE_RTOS
   printf("FreeRTOS main_thread\n");
#endif
   char commands[20]="";
   hw_sprite_show(0);
   vs.sprite_showing=0;
   if(!sw1_is_down)
   {
      show_options();
   }
   else //if(sw1_is_down)
   {
      sw1_is_down=0;
      sprintf(message,"SW1 pressed -> download and CPLD programming...");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      usleep(1000000);
      for(int i=0;i<100;i++)
      {
         if(XGpioPs_ReadPin(&GpioPs, USER_SW1)==0)
         {
            sw1_is_down=1;
            sprintf(message,"SW1 pressed again -> download ALFA and CPLD programming...");
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
            i=100;
         }
         usleep(10000);
      }
      if(sw1_is_down==0)
         sprintf(commands,"IDR");
      else
         sprintf(commands,"ADR");
      sprintf(message,"Sending commands '%c' '%c' '%c'",commands[0],commands[1],commands[2]);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }

#ifdef CPLD_PROGRAMMING
   // Init CPLD programming (and read firmware version)
   CPLD_RESET_ARM(0);
   Xil_ExceptionDisable();
   init_xc3sprog();
   Xil_ExceptionEnable();
   CPLD_RESET_ARM(1);
   monitor_switch(1); // 1=RTG
#endif
#include "alfa.txt"
#if REVISION_ALFA == 0
#define READ_FILE_VERSIONS
#endif
#ifdef READ_FILE_VERSIONS
   // here we look for the versions of the BOOT.BIN, FAILSAFE.BIN and Z3660.BIN versions
   sprintf(message,"Reading file versions");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   look_for_ver("BOOT.bin");
   look_for_ver("FAILSAFE.bin");
   look_for_ver("Z3660.bin");
#endif
   int connected=0;
   alfa=0;
   while(1)
   {
      int commands_len=strlen(commands);
      if(read_keyboard(&keybd_data,0) || commands_len>0)
      {
         if(commands_len>0)
         {
            keybd_data=commands[0];
            commands[0]=commands[1];
            commands[1]=commands[2];
            commands[2]=0;
         }
         if(keybd_data=='i' || keybd_data=='I')
         {
            alfa=0;
            if(connected==0)
            {
               sprintf(message,"Ethernet PHY auto-negotiation");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               connected=z3660_lwip_connect();
               if(connected)
               {
                  sprintf(message,"Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               else
               {
                  sprintf(message,"NOT Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
            }
            if(connected)
            {
               sprintf(message,"Getting version info");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               int ok=lwip_get_update_version("version.txt",0); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest BOOT.bin version is     %s",version_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  int data=read_rtg_register(REG_ZZ_FW_VERSION);
                  int v_major=(data>>8)&0xFF;
                  int v_minor=(data)&0xFF;
                  int beta=read_rtg_register(REG_ZZ_FW_BETA);
                  int alfa=read_rtg_register(REG_ZZ_FW_ALFA);
                  if(beta==0)
                     sprintf(message,"The loaded BOOT.bin version is %d.%02d",v_major,v_minor);
                  else
                  {
                     if(alfa==0)
                        sprintf(message,"The loaded BOOT.bin version is %d.%02d BETA %d",v_major,v_minor,beta);
                     else
                        sprintf(message,"The loaded BOOT.bin version is %d.%02d BETA %d ALFA %d",v_major,v_minor,beta,alfa);
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               ok=lwip_get_update_version_scsirom("scsirom_version.txt",0); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660_scsi.rom version is v%s",version_scsirom_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660_scsi.rom", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_umount(Path);
                  FSIZE_t pos=0;
                  while(DATA[pos]!='$')
                  {
                     pos++;
                     if(pos>=filesize)
                        break;
                  }
                  if(pos>=filesize)
                     sprintf(message,"Can't read the z3660_scsi.rom file on exFAT partition");
                  else
                  {
                     pos++;
                     if(DATA[pos]=='V' && DATA[pos+31]=='I')
                     {
//                        DATA[pos+37]=0;
                        sprintf(message,"You have z3660_scsi.rom version  v%s",&DATA[pos+34]);
                     }
                     else
                        sprintf(message,"Can't read the version number of the z3660_scsi.rom file");
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  free(DATA);
               }
#ifdef CPLD_PROGRAMMING
               ok=lwip_get_update_version_jed("jed_version.txt",0); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660.jed date is %s",version_jed_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660.jed", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_umount(Path);
                  FSIZE_t pos=0;
                  while(DATA[pos]!='D')
                  {
                     pos++;
                     if(pos>=filesize)
                        break;
                  }
                  if(pos>=filesize)
                     sprintf(message,"Can't read the z3660.jed file on exFAT partition");
                  else
                  {
                     pos++;
                     if(DATA[pos]=='a' && DATA[pos+14]==' ')
                     {
                        DATA[pos+39]=0;
                        sprintf(message,"You have z3660.jed date  %s",&DATA[pos+15]);
                     }
                     else
                        sprintf(message,"Can't read the date of the z3660.jed file");
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  free(DATA);
               }
#endif
            }
         }
         if(keybd_data=='p' || keybd_data=='P')
         {
            if(connected==0)
            {
               sprintf(message,"Ethernet PHY auto-negotiation");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               connected=z3660_lwip_connect();
               if(connected)
               {
                  sprintf(message,"Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               else
               {
                  sprintf(message,"NOT Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
            }
            if(connected)
            {
               char path[500]="0:";
               printAppHeader(TFTP_PORT);
tftp_restart:
               sprintf(message,"Current path %s (Press SPACE to change the path, R to reboot)",path);
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);

               initFileSystem(path,0);
               startApplication();
               listDirectory(path);
               createIndexFileTree(path);
               while(1)
               {
#ifndef USE_RTOS
                  lwip_run();
#endif
                  if(read_keyboard(&keybd_data,0))
                  {
                     if(keybd_data==' ')
                     {
                        strcpy(path,"1:");
                        goto tftp_restart;
                     }
                     else if(keybd_data=='r' || keybd_data=='R')
                     {
                        reboot();
                     }
                  }
               }
            }
         }
         else if(keybd_data=='s' || keybd_data=='S')
         {
            if(server==SHANSHE_SERVER)
            {
               server=GITHUB_SERVER;
               sprintf(message,"github server selected");
            }
            else //if(server==GITHUB_SERVER)
            {
               server=SHANSHE_SERVER;
               sprintf(message,"shanshe server selected");
            }
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
         }
         else if(keybd_data=='a' || keybd_data=='A')
         {
            alfa=1;
            if(connected==0)
            {
               sprintf(message,"Ethernet PHY auto-negotiation");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               connected=z3660_lwip_connect();
               if(connected)
               {
                  sprintf(message,"Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               else
               {
                  sprintf(message,"NOT Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
            }
            if(connected)
            {
               sprintf(message,"Getting version info");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               int ok=lwip_get_update_version("version.txt",1); // 1 = alfa
               if(ok)
               {
                  sprintf(message,"Latest BOOT.bin version is     %s",version_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  int data=read_rtg_register(REG_ZZ_FW_VERSION);
                  int v_major=(data>>8)&0xFF;
                  int v_minor=(data)&0xFF;
                  int beta=read_rtg_register(REG_ZZ_FW_BETA);
                  int alfa=read_rtg_register(REG_ZZ_FW_ALFA);
                  if(beta==0)
                     sprintf(message,"The loaded BOOT.bin version is %d.%02d",v_major,v_minor);
                  else
                  {
                     if(alfa==0)
                        sprintf(message,"The loaded BOOT.bin version is %d.%02d BETA %d",v_major,v_minor,beta);
                     else
                        sprintf(message,"The loaded BOOT.bin version is %d.%02d BETA %d ALFA %d",v_major,v_minor,beta,alfa);
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               ok=lwip_get_update_version_scsirom("scsirom_version.txt",1); // 1 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660_scsi.rom version is v%s",version_scsirom_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660_scsi.rom", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_umount(Path);
                  FSIZE_t pos=0;
                  while(DATA[pos]!='$')
                  {
                     pos++;
                     if(pos>=filesize)
                        break;
                  }
                  if(pos>=filesize)
                     sprintf(message,"Can't read the z3660_scsi.rom file on exFAT partition");
                  else
                  {
                     pos++;
                     if(DATA[pos]=='V' && DATA[pos+31]=='I')
                     {
//                        DATA[pos+37]=0;
                        sprintf(message,"You have z3660_scsi.rom version  v%s",&DATA[pos+34]);
                     }
                     else
                        sprintf(message,"Can't read the version number of the z3660_scsi.rom file");
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  free(DATA);
               }
#ifdef CPLD_PROGRAMMING
               ok=lwip_get_update_version_jed("jed_version.txt",1); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660.jed date is %s",version_jed_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660.jed", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_umount(Path);
                  FSIZE_t pos=0;
                  while(DATA[pos]!='D')
                  {
                     pos++;
                     if(pos>=filesize)
                        break;
                  }
                  if(pos>=filesize)
                     sprintf(message,"Can't read the z3660.jed file on exFAT partition");
                  else
                  {
                     pos++;
                     if(DATA[pos]=='a' && DATA[pos+14]==' ')
                     {
                        DATA[pos+39]=0;
                        sprintf(message,"You have z3660.jed date  %s",&DATA[pos+15]);
                     }
                     else
                        sprintf(message,"Can't read the date of the z3660.jed file");
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  free(DATA);
               }
#endif
            }
         }
         else if(keybd_data=='h' || keybd_data=='H')
         {
            show_options();
         }
         else if(keybd_data=='x' || keybd_data=='X')
         {
            test_nops();
         }
         else if(keybd_data=='q' || keybd_data=='Q')
         {
            test_coremark(config.arm_frequency);
         }
#ifdef CPLD_PROGRAMMING
         else if(keybd_data=='d' || keybd_data=='D')
         {
            if(connected==0)
            {
               sprintf(message,"NOT Connected");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
            }
            else
            {
               char filename[]="z3660.jed";
               sprintf(message,"Downloading the %s file",filename);
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               DATA=malloc(1*1024*1024);
               memset(DATA,0,1*1024*1024);
               int ok=lwip_get_update_jed(filename,alfa);
               if(ok)
               {
                  uint32_t checksum32=0;
                  uint32_t *data_u32=(uint32_t *)DATA;
                  for(uint32_t i=0;i<download_data.filesize_jed/4;i++)
                     checksum32+=__builtin_bswap32(data_u32[i]);
                  sprintf(message,"Checksum-32 %lu (%08lX) %lu (%08lX)",checksum32,checksum32,download_data.checksum32_jed,download_data.checksum32_jed);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  if(checksum32==download_data.checksum32_jed)
                  {
                     sprintf(message,"Writing the %s file...",filename);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                     ACTIVITY_LED_ON; // ON
                     FIL fil;
                     FATFS fatfs;
                     TCHAR *Path = "1:/";
                     f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                     char filename_ex[20];
                     sprintf(filename_ex,"%s%s",Path,filename);
                     f_open(&fil,filename_ex, FA_CREATE_ALWAYS | FA_WRITE);
                     UINT NumBytesWritten;
                     f_write(&fil, DATA, download_data.filesize_jed,&NumBytesWritten);
                     f_close(&fil);
                     ACTIVITY_LED_OFF; // OFF

                     sprintf(message,"File %s written (%d bytes of %ld)",filename,NumBytesWritten,download_data.filesize_jed);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                     f_umount(Path);
                  }
                  else
                  {
                     sprintf(message,"File %s NOT written (bad checksum)",filename);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                  }
               }
               free(DATA);
            }


            CPLD_RESET_ARM(0); // Reset when CPLD programming

#ifdef DIRECT_CPLD_PROGRAMMING
            printf("")
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x24C,(uint32_t)( 0 *1000));
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x240,(uint32_t)( 100 *1000)); // TDI CLK90
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x23C,(uint32_t)( 0 *1000));
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x238,(uint32_t)( 8 ));
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x248,(uint32_t)( 0 *1000));
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);
            while(XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x004)==0);
#endif
            Xil_DCacheEnable();
            Xil_ExceptionDisable();
            XIicPs_SetSClk(&IicInstance, I2C_FREQ_SII9022*4); // 400 kHz
            main_xc3sprog();
            i2c_finish();
            XIicPs_SetSClk(&IicInstance, I2C_FREQ_SII9022);
            Xil_ExceptionEnable();

            //Restore pins
#ifdef DIRECT_CPLD_PROGRAMMING
            configure_clk(config.cpufreq,0,1);
#endif
            CPLD_RESET_ARM(1);

         }
#endif // CPLD_PROGRAMMIG
         else if(keybd_data=='u' || keybd_data=='U' || keybd_data=='f' || keybd_data=='F' || keybd_data=='t' || keybd_data=='T')
         {
            if(connected==0)
            {
               sprintf(message,"NOT Connected");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
            }
            else
            {
               sprintf(message,"Downloading the BOOT.bin file");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               DATA=malloc(32*1024*1024);
               memset(DATA,0,32*1024*1024);
               int ok=lwip_get_update("BOOT.BIN",alfa);
               if(ok)
               {
                  uint32_t checksum32=0;
                  uint32_t *data_u32=(uint32_t *)DATA;
                  for(uint32_t i=0;i<download_data.filesize/4;i++)
                     checksum32+=__builtin_bswap32(data_u32[i]);
                  sprintf(message,"Checksum-32 %lu (%08lX) %lu (%08lX)",checksum32,checksum32,download_data.checksum32,download_data.checksum32);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  char filename[20];
                  if(keybd_data=='u' || keybd_data=='U' || keybd_data=='f' || keybd_data=='F')
                  {
                     if(keybd_data=='u' || keybd_data=='U')
                        strcpy(filename,"Z3660.bin");
                     else
                        strcpy(filename,"FAILSAFE.bin");
                     if(checksum32==download_data.checksum32)
                     {
                        sprintf(message,"Writing the %s file...",filename);
                        print_hdmi_ln(0,message,1);
                        printf("%s\n",message);
                        ACTIVITY_LED_ON; // ON
                        FIL fil;
                        FATFS fatfs;
                        TCHAR *Path = "0:/";
                        f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                        f_open(&fil,filename, FA_CREATE_ALWAYS | FA_WRITE);
                        UINT NumBytesWritten,NumBytesWritten_total=0;
                        int num=download_data.filesize/(1L<<20);
                        print_hdmi_ln(0,"[",0);
                        printf("[");
                        fflush(stdout);
                        for(int i=0;i<num;i++)
                        {
                           print_hdmi_ln(i+1,".",0);
                           printf(".");
                           fflush(stdout);
                        }
                        print_hdmi_ln(num+1,"]",0);
                        printf("]\r[");
                        fflush(stdout);
                        for(int i=0;i<num;i++)
                        {
                           f_write(&fil, DATA+(i*(1L<<20)), 1L<<20,&NumBytesWritten);
                           print_hdmi_ln(i+1,"=",0);
                           printf("=");
                           fflush(stdout);
                           NumBytesWritten_total+=NumBytesWritten;
                        }
                        f_write(&fil, DATA+(num*(1L<<20)), download_data.filesize-((1L<<20)*num),&NumBytesWritten);
                        print_hdmi_ln(num+1,"]",1);
                        printf("]\n");
                        fflush(stdout);
                        NumBytesWritten_total+=NumBytesWritten;
                        f_close(&fil);
                        ACTIVITY_LED_OFF; // OFF
                        sprintf(message,"File %s written (%d bytes of %ld)",filename,NumBytesWritten_total,download_data.filesize);
                        print_hdmi_ln(0,message,1);
                        printf("%s\n",message);
                        f_umount(Path);
                     }
                     else
                     {
                        sprintf(message,"File %s NOT written (bad checksum)",filename);
                        print_hdmi_ln(0,message,1);
                        printf("%s\n",message);
                     }
                  }
                  else
                  {
                     if(checksum32==download_data.checksum32)
                        sprintf(message,"Download test OK");
                     else
                        sprintf(message,"Download test NOT OK !!!");
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                  }
               }
               free(DATA);
            }
         }
         else if(keybd_data=='o' || keybd_data=='O')
         {
            if(connected==0)
            {
               sprintf(message,"NOT Connected");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
            }
            else
            {
               char filename[]="z3660_scsi.rom";
               sprintf(message,"Downloading the %s file",filename);
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               DATA=malloc(1*1024*1024);
               memset(DATA,0,1*1024*1024);
               int ok=lwip_get_update_scsirom(filename,alfa);
               if(ok)
               {
                  uint32_t checksum32=0;
                  uint32_t *data_u32=(uint32_t *)DATA;
                  for(uint32_t i=0;i<download_data.filesize_scsirom/4;i++)
                     checksum32+=__builtin_bswap32(data_u32[i]);
                  sprintf(message,"Checksum-32 %lu (%08lX) %lu (%08lX)",checksum32,checksum32,download_data.checksum32_scsirom,download_data.checksum32_scsirom);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  if(checksum32==download_data.checksum32_scsirom)
                  {
                     sprintf(message,"Writing the %s file...",filename);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                     ACTIVITY_LED_ON; // ON
                     FIL fil;
                     FATFS fatfs;
                     TCHAR *Path = "1:/";
                     f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                     char filename_ex[20];
                     sprintf(filename_ex,"%s%s",Path,filename);
                     f_open(&fil,filename_ex, FA_CREATE_ALWAYS | FA_WRITE);
                     UINT NumBytesWritten;
                     f_write(&fil, DATA, download_data.filesize_scsirom,&NumBytesWritten);
                     f_close(&fil);
                     ACTIVITY_LED_OFF; // OFF

                     sprintf(message,"File %s written (%d bytes of %ld)",filename,NumBytesWritten,download_data.filesize_scsirom);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                     f_umount(Path);
                  }
                  else
                  {
                     sprintf(message,"File %s NOT written (bad checksum)",filename);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                  }
               }
               free(DATA);
            }
         }
         else if(keybd_data=='b' || keybd_data=='B' || keybd_data=='n' || keybd_data=='N' || keybd_data=='m' || keybd_data=='M' ||
               keybd_data=='j' || keybd_data=='J' || keybd_data=='k' || keybd_data=='K' || keybd_data=='l' || keybd_data=='L')
         {
            char src[20];
            char dst[20];
            if(keybd_data=='b' || keybd_data=='B')
            {
               strcpy(src,"BOOT.bin");
               strcpy(dst,"FAILSAFE.bin");
            }
            else if(keybd_data=='n' || keybd_data=='N')
            {
               strcpy(src,"BOOT.bin");
               strcpy(dst,"Z3660.bin");
            }
            else if(keybd_data=='m' || keybd_data=='M')
            {
               strcpy(src,"Z3660.bin");
               strcpy(dst,"BOOT.bin");
            }
            else if(keybd_data=='j' || keybd_data=='J')
            {
               strcpy(src,"Z3660.bin");
               strcpy(dst,"FAILSAFE.bin");
            }
            else if(keybd_data=='k' || keybd_data=='K')
            {
               strcpy(src,"FAILSAFE.bin");
               strcpy(dst,"BOOT.bin");
            }
            else //if(keybd_data=='l' || keybd_data=='L')
            {
               strcpy(src,"FAILSAFE.bin");
               strcpy(dst,"Z3660.bin");
            }
            sprintf(message,"Copying file %s to %s",src,dst);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);

            ACTIVITY_LED_ON; // ON
            FIL fil_src,fil_dst;
            FATFS fatfs;
            TCHAR *Path = "0:/";
            f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
            FRESULT res=f_open(&fil_src,src, FA_READ);
            if(res==FR_OK)
            {
               UINT NumBytesRead,NumBytesRead_total=0;
               FSIZE_t filesize=fil_src.obj.objsize;
               sprintf(message,"Filesize to copy %lld bytes",filesize);
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               DATA=malloc(filesize+100);
               int num=filesize/(1L<<20);
               print_hdmi_ln(0,"[",0);
               printf("[");
               fflush(stdout);
               for(int i=0;i<num;i++)
               {
                  f_read(&fil_src, DATA+(i*(1L<<20)), 1L<<20,&NumBytesRead);
                  NumBytesRead_total+=NumBytesRead;
                  print_hdmi_ln(i+1,".",0);
                  printf(".");
                  fflush(stdout);
               }
               f_read(&fil_src, DATA+(num*(1L<<20)), filesize-(1L<<20)*num,&NumBytesRead);
               NumBytesRead_total+=NumBytesRead;
               print_hdmi_ln(num+1,"]",0);
               printf("]\r[");
               fflush(stdout);
               f_close(&fil_src);

               f_open(&fil_dst,dst, FA_CREATE_ALWAYS | FA_WRITE);
               UINT NumBytesWritten,NumBytesWritten_total=0;
               for(int i=0;i<num;i++)
               {
                  f_write(&fil_dst, DATA+(i*(1L<<20)), 1L<<20,&NumBytesWritten);
                  NumBytesWritten_total+=NumBytesWritten;
                  print_hdmi_ln(i+1,"=",0);
                  printf("=");
                  fflush(stdout);
               }
               f_write(&fil_dst, DATA+(num*(1L<<20)), filesize-(1L<<20)*num,&NumBytesWritten);
               NumBytesWritten_total+=NumBytesWritten;
               print_hdmi_ln(num+1,"]",1);
               printf("]\n");
               fflush(stdout);
               f_close(&fil_dst);

               if(NumBytesRead_total==NumBytesWritten_total)
               {
                  sprintf(message,"Copied file %s to %s",src,dst);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               else
               {
                  sprintf(message,"NumBytesRead %d != NumBytesWritten %d",NumBytesRead_total,NumBytesWritten_total);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  sprintf(message,"Error when copying file %s to %s",src,dst);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
            }
            else
            {
               sprintf(message,"Can't read file %s",src);
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
            }
            f_umount(Path);
            ACTIVITY_LED_OFF; // OFF
            free(DATA);
         }
         else if(keybd_data=='r' || keybd_data=='R')
         {
            reboot();
         }
         else if(keybd_data=='y' || keybd_data=='Y')
         {
            fpga_interrupt_connect(isr_video, isr_audio_tx, isr_usb, INT_IPL_ON_THIS_CORE);
            usb_test_standalone_main();
         }
      }
#ifndef USE_RTOS
      if(connected)
         lwip_run();
#endif
   }
}
void start_ztop(void)
{

   init_win();

   uint8_t* bmp_data = (uint8_t*) (((uint32_t) vs.framebuffer) + (1920*1080*2));
   uint8_t* bmp_colors = (uint8_t*) (((uint32_t) vs.framebuffer) + (1920*1080*2+32*48));

   bmp_colors[ 0]=0xFF;bmp_colors[ 1]=0xFF;bmp_colors[ 2]=0xFF; // yeah 24 bit per color
   bmp_colors[ 3]=0xF3;bmp_colors[ 4]=0x30;bmp_colors[ 5]=0x30;
   bmp_colors[ 6]=0xF0;bmp_colors[ 7]=0xEC;bmp_colors[ 8]=0xDC;
   bmp_colors[ 9]=0x00;bmp_colors[10]=0x00;bmp_colors[11]=0x00;

   for(int i=0;i<32*48;i++)
      bmp_data[i]=cursor_data[i];
   update_hw_sprite_clut(bmp_data,bmp_colors,32,48,0,config.doubled_cursor,0);
   update_hw_sprite_pos();
   hw_sprite_show(1);
   vs.sprite_showing=1;
   do_clip_hw_sprite(0, 0);

   xadc_init();
   ltc2990_init();
   PT_INIT(&pt_measures);
   PT_INIT(&pt_timer);
   fpga_interrupt_connect(isr_video, isr_audio_tx, isr_usb, INT_IPL_ON_THIS_CORE);
   for(int i=0;i<20;i++)
   {
      usleep(1000);
      task_counter=10000000;
      measures_thread(&pt_measures);
   }

   show_ztop();

   monitor_switch(1); // 1=RTG

   while(1)
   {
      task_counter+=100;
      measures_thread(&pt_measures);
      timer_thread(&pt_timer);
      if(read_keyboard(&keybd_data,1))
      {
//         if(keybd_data=='\x1b') // ESC
//         {
//            reboot();
//         }
//         printf("key = 0x%02X\n",keybd_data);
         if(amiga_status) // amiga cursors + Amiga key
         {
            if(keybd_data=='\x1f') // up
            {
               mousey-=10;
               update_mouse_position();
            }
            if(keybd_data=='\x1e') // down
            {
               mousey+=10;
               update_mouse_position();
            }
            if(keybd_data=='\x1d') // right
            {
               mousex+=10;
               update_mouse_position();
            }
            if(keybd_data=='\x1c') // left
            {
               mousex-=10;
               update_mouse_position();
            }
            if(keybd_data=='\xfd') // left alt + Amiga key = mouse click
            {
               mouse_pressed=1;
            }
         }
         if(keybd_data=='\x1b') // serial debug cursors
         {
            int timeout_key=100000;
            while(timeout_key>0 && !read_keyboard(&keybd_data,1))
            {
               timeout_key--;
            }
            if(timeout_key!=0)
            {
               if(keybd_data=='\x5b')
               {
                  timeout_key=100000;
                  while(timeout_key>0 && !read_keyboard(&keybd_data,1))
                  {
                     timeout_key--;
                  }
                  if(timeout_key!=0)
                  {
                     if(keybd_data=='\x41') // up
                     {
                        mousey-=10;
                        update_mouse_position();
                     }
                     if(keybd_data=='\x42') // down
                     {
                        mousey+=10;
                        update_mouse_position();
                     }
                     if(keybd_data=='\x43') // right
                     {
                        mousex+=10;
                        update_mouse_position();
                     }
                     if(keybd_data=='\x44') // left
                     {
                        mousex-=10;
                        update_mouse_position();
                     }
                  }
               }
            }
         }
         if(keybd_data==' ') // space = mouse click
         {
            mouse_pressed=1;
         }
         if(keybd_data=='\x09') // TAB
         {
            selected_tab++;
            if(selected_tab==NUM_TABS+1)
            {
               selected_tab=1;
            }
            tab_change(*tabs[selected_tab-1],1);
            (*tabs[selected_tab-1])->action();
         }
         if(selected_tab==TAB_MISC && mac_textedit->cursor_pos>0)
         {
            if(keybd_data=='\x1c')
            {
               mac_textedit->cursor_pos--;
               if((mac_textedit->cursor_pos%3)==0) mac_textedit->cursor_pos--; // skip the :
               if(mac_textedit->cursor_pos<=0)
                  mac_textedit->cursor_pos=17;
               paint_mac_textedit();
            }
            if(keybd_data=='\x1d')
            {
               mac_textedit->cursor_pos++;
               if((mac_textedit->cursor_pos%3)==0) mac_textedit->cursor_pos++; // skip the :
               if(mac_textedit->cursor_pos>=18)
                  mac_textedit->cursor_pos=1;
               paint_mac_textedit();
            }
            if((keybd_data>='0' && keybd_data<='9') ||
               (keybd_data>='a' && keybd_data<='f') ||
               (keybd_data>='A' && keybd_data<='F') )
            {
               int idx[19]={0,0,1,1,
                              2,3,3,
                              4,5,5,
                              6,7,7,
                              8,9,9,
                              10,11,11,
                              };
               if(keybd_data>='a' && keybd_data<='f')
                  keybd_data+='A'-'a';
               if(keybd_data>='0' && keybd_data<='9')
                  keybd_data-='0';
               else
                  keybd_data-='A'-10;
               int index=idx[mac_textedit->cursor_pos];
               if(index&1)
                  mac_textedit->mac_address[index>>1]=(mac_textedit->mac_address[index>>1]&0xF0)|keybd_data;
               else
                  mac_textedit->mac_address[index>>1]=(mac_textedit->mac_address[index>>1]&0x0F)|(keybd_data<<4);
               mac_textedit->cursor_pos++;
               if((mac_textedit->cursor_pos%3)==0) mac_textedit->cursor_pos++; // skip the :
               if(mac_textedit->cursor_pos>=18)
                  mac_textedit->cursor_pos=1;
               paint_mac_textedit();
            }
         }
         if(selected_tab==TAB_PRESET && preset_textedit[preset_selected]->cursor_pos>0)
         {
            if(keybd_data=='\x1c')
            {
//#define debug_pos printf("pos %d len %d\n",preset_textedit[preset_selected]->cursor_pos,strlen(preset_textedit[preset_selected]->text))
#define debug_pos

               preset_textedit[preset_selected]->cursor_pos--;
               if(preset_textedit[preset_selected]->cursor_pos<=1)
                  preset_textedit[preset_selected]->cursor_pos=1;
               paint_preset_textedit();

               debug_pos;
            }
            else if(keybd_data=='\x1d')
            {
               preset_textedit[preset_selected]->cursor_pos++;
               int len=strlen(preset_textedit[preset_selected]->text);
               if(preset_textedit[preset_selected]->cursor_pos>=len+1)
                  preset_textedit[preset_selected]->cursor_pos=len+1;
               paint_preset_textedit();

               debug_pos;
            }
            else if(keybd_data=='\x08')
            {
               int len=strlen(preset_textedit[preset_selected]->text);
               if(preset_textedit[preset_selected]->cursor_pos>1 && preset_textedit[preset_selected]->cursor_pos-1<=len)
               {
                  for(int i=preset_textedit[preset_selected]->cursor_pos-1;i<=len;i++)
                     preset_textedit[preset_selected]->text[i-1]=preset_textedit[preset_selected]->text[i];
                  preset_textedit[preset_selected]->cursor_pos--;
               }
               paint_preset_textedit();

               debug_pos;
            }
            else if(keybd_data=='\x7f')
            {
               int len=strlen(preset_textedit[preset_selected]->text);
               for(int i=preset_textedit[preset_selected]->cursor_pos;i<=len+1;i++)
                  preset_textedit[preset_selected]->text[i-1]=preset_textedit[preset_selected]->text[i];
               paint_preset_textedit();

               debug_pos;
            }
            else if((keybd_data>='0' && keybd_data<='9') ||
               (keybd_data>='a' && keybd_data<='z') ||
               (keybd_data>='A' && keybd_data<='Z') ||
               keybd_data=='_' || keybd_data=='-' || keybd_data==' ' )
            {
               int len=strlen(preset_textedit[preset_selected]->text);
               if(len<PRESET_MAX_LENGTH)
               {
                  int index=preset_textedit[preset_selected]->cursor_pos-1;
                  if(index<len)
                  {
                     for(int i=len;i>=index;i--)
                        preset_textedit[preset_selected]->text[i+1]=preset_textedit[preset_selected]->text[i];
                  }
                  preset_textedit[preset_selected]->text[index]=keybd_data;
                  preset_textedit[preset_selected]->cursor_pos++;
                  if(preset_textedit[preset_selected]->cursor_pos>=PRESET_MAX_LENGTH+1)
                     preset_textedit[preset_selected]->cursor_pos=PRESET_MAX_LENGTH+1;
                  paint_preset_textedit();
               }
               debug_pos;
            }
         }
         if(selected_tab==TAB_TIMINGS && (
               (timings_phase_textedit[0]->cursor_pos>0) ||
               (timings_phase_textedit[1]->cursor_pos>0) ||
               (timings_phase_textedit[2]->cursor_pos>0) ||
               (timings_phase_textedit[3]->cursor_pos>0) ||
               (timings_phase_textedit[4]->cursor_pos>0) ||
               (timings_phase_textedit[5]->cursor_pos>0) ||
               (timings_divider_textedit[0]->cursor_pos>0) ||
               (timings_divider_textedit[1]->cursor_pos>0) ||
               (timings_divider_textedit[2]->cursor_pos>0) ||
               (timings_divider_textedit[3]->cursor_pos>0) ||
               (timings_divider_textedit[4]->cursor_pos>0) ||
               (timings_muldiv_textedit[0]->cursor_pos>0) ||
               (timings_muldiv_textedit[1]->cursor_pos>0)
               ) )
         {
            if(keybd_data=='\x1c')
            {
//#define debug_pos printf("pos %d len %d\n",preset_textedit[preset_selected]->cursor_pos,strlen(preset_textedit[preset_selected]->text))
#define debug_pos
               TextEdit *b;
               if(timings_phase_textedit[0]->cursor_pos>0)
                  b=timings_phase_textedit[0];
               else if(timings_phase_textedit[1]->cursor_pos>0)
                  b=timings_phase_textedit[1];
               else if(timings_phase_textedit[2]->cursor_pos>0)
                  b=timings_phase_textedit[2];
               else if(timings_phase_textedit[3]->cursor_pos>0)
                  b=timings_phase_textedit[3];
               else if(timings_phase_textedit[4]->cursor_pos>0)
                  b=timings_phase_textedit[4];
               else if(timings_phase_textedit[5]->cursor_pos>0)
                  b=timings_phase_textedit[5];
               else if(timings_divider_textedit[0]->cursor_pos>0)
                  b=timings_divider_textedit[0];
               else if(timings_divider_textedit[1]->cursor_pos>0)
                  b=timings_divider_textedit[1];
               else if(timings_divider_textedit[2]->cursor_pos>0)
                  b=timings_divider_textedit[2];
               else if(timings_divider_textedit[3]->cursor_pos>0)
                  b=timings_divider_textedit[3];
               else if(timings_divider_textedit[4]->cursor_pos>0)
                  b=timings_divider_textedit[4];
               else if(timings_muldiv_textedit[0]->cursor_pos>0)
                  b=timings_muldiv_textedit[0];
               else if(timings_muldiv_textedit[1]->cursor_pos>0)
                  b=timings_muldiv_textedit[1];
               else
                  b=timings_phase_textedit[0];
               b->cursor_pos--;
               if(b->cursor_pos<=1)
                  b->cursor_pos=1;
               test_tab_timings();
               ns_repaint();
               paint_b_apply_timings();
               paint_b_apply_all_timings();

               debug_pos;
            }
            else if(keybd_data=='\x1d')
            {
               TextEdit *b;
               if(timings_phase_textedit[0]->cursor_pos>0)
                  b=timings_phase_textedit[0];
               else if(timings_phase_textedit[1]->cursor_pos>0)
                  b=timings_phase_textedit[1];
               else if(timings_phase_textedit[2]->cursor_pos>0)
                  b=timings_phase_textedit[2];
               else if(timings_phase_textedit[3]->cursor_pos>0)
                  b=timings_phase_textedit[3];
               else if(timings_phase_textedit[4]->cursor_pos>0)
                  b=timings_phase_textedit[4];
               else if(timings_phase_textedit[5]->cursor_pos>0)
                  b=timings_phase_textedit[5];
               else if(timings_divider_textedit[0]->cursor_pos>0)
                  b=timings_divider_textedit[0];
               else if(timings_divider_textedit[1]->cursor_pos>0)
                  b=timings_divider_textedit[1];
               else if(timings_divider_textedit[2]->cursor_pos>0)
                  b=timings_divider_textedit[2];
               else if(timings_divider_textedit[3]->cursor_pos>0)
                  b=timings_divider_textedit[3];
               else if(timings_divider_textedit[4]->cursor_pos>0)
                  b=timings_divider_textedit[4];
               else if(timings_muldiv_textedit[0]->cursor_pos>0)
                  b=timings_muldiv_textedit[0];
               else if(timings_muldiv_textedit[1]->cursor_pos>0)
                  b=timings_muldiv_textedit[1];
               else
                  b=timings_phase_textedit[0];
               b->cursor_pos++;
               int len=strlen(b->text);
               if(b->cursor_pos>=len+1)
                  b->cursor_pos=len+1;
               test_tab_timings();
               ns_repaint();
               paint_b_apply_timings();
               paint_b_apply_all_timings();

               debug_pos;
            }
            else if(keybd_data=='\x08')
            {
               TextEdit *b;
               if(timings_phase_textedit[0]->cursor_pos>0)
                  b=timings_phase_textedit[0];
               else if(timings_phase_textedit[1]->cursor_pos>0)
                  b=timings_phase_textedit[1];
               else if(timings_phase_textedit[2]->cursor_pos>0)
                  b=timings_phase_textedit[2];
               else if(timings_phase_textedit[3]->cursor_pos>0)
                  b=timings_phase_textedit[3];
               else if(timings_phase_textedit[4]->cursor_pos>0)
                  b=timings_phase_textedit[4];
               else if(timings_phase_textedit[5]->cursor_pos>0)
                  b=timings_phase_textedit[5];
               else if(timings_divider_textedit[0]->cursor_pos>0)
                  b=timings_divider_textedit[0];
               else if(timings_divider_textedit[1]->cursor_pos>0)
                  b=timings_divider_textedit[1];
               else if(timings_divider_textedit[2]->cursor_pos>0)
                  b=timings_divider_textedit[2];
               else if(timings_divider_textedit[3]->cursor_pos>0)
                  b=timings_divider_textedit[3];
               else if(timings_divider_textedit[4]->cursor_pos>0)
                  b=timings_divider_textedit[4];
               else if(timings_muldiv_textedit[0]->cursor_pos>0)
                  b=timings_muldiv_textedit[0];
               else if(timings_muldiv_textedit[1]->cursor_pos>0)
                  b=timings_muldiv_textedit[1];
               else
                  b=timings_phase_textedit[0];
               int len=strlen(b->text);
               if(b->cursor_pos>1 && b->cursor_pos-1<=len)
               {
                  for(int i=b->cursor_pos-1;i<=len;i++)
                     b->text[i-1]=b->text[i];
                  b->cursor_pos--;
               }
               test_tab_timings();
               ns_repaint();
               paint_b_apply_timings();
               paint_b_apply_all_timings();

               debug_pos;
            }
            else if(keybd_data=='\x7f')
            {
               TextEdit *b;
               if(timings_phase_textedit[0]->cursor_pos>0)
                  b=timings_phase_textedit[0];
               else if(timings_phase_textedit[1]->cursor_pos>0)
                  b=timings_phase_textedit[1];
               else if(timings_phase_textedit[2]->cursor_pos>0)
                  b=timings_phase_textedit[2];
               else if(timings_phase_textedit[3]->cursor_pos>0)
                  b=timings_phase_textedit[3];
               else if(timings_phase_textedit[4]->cursor_pos>0)
                  b=timings_phase_textedit[4];
               else if(timings_phase_textedit[5]->cursor_pos>0)
                  b=timings_phase_textedit[5];
               else if(timings_divider_textedit[0]->cursor_pos>0)
                  b=timings_divider_textedit[0];
               else if(timings_divider_textedit[1]->cursor_pos>0)
                  b=timings_divider_textedit[1];
               else if(timings_divider_textedit[2]->cursor_pos>0)
                  b=timings_divider_textedit[2];
               else if(timings_divider_textedit[3]->cursor_pos>0)
                  b=timings_divider_textedit[3];
               else if(timings_divider_textedit[4]->cursor_pos>0)
                  b=timings_divider_textedit[4];
               else if(timings_muldiv_textedit[0]->cursor_pos>0)
                  b=timings_muldiv_textedit[0];
               else if(timings_muldiv_textedit[1]->cursor_pos>0)
                  b=timings_muldiv_textedit[1];
               else
                  b=timings_phase_textedit[0];
               int len=strlen(b->text);
               for(int i=b->cursor_pos;i<=len+1;i++)
                  b->text[i-1]=b->text[i];
               test_tab_timings();
               ns_repaint();
               paint_b_apply_timings();
               paint_b_apply_all_timings();

               debug_pos;
            }
            else if((keybd_data>='0' && keybd_data<='9') ||
                     keybd_data=='-' )
            {
               TextEdit *b;
               int max_length;
               if(timings_phase_textedit[0]->cursor_pos>0)
               {
                  b=timings_phase_textedit[0];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_phase_textedit[1]->cursor_pos>0)
               {
                  b=timings_phase_textedit[1];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_phase_textedit[2]->cursor_pos>0)
               {
                  b=timings_phase_textedit[2];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_phase_textedit[3]->cursor_pos>0)
               {
                  b=timings_phase_textedit[3];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_phase_textedit[4]->cursor_pos>0)
               {
                  b=timings_phase_textedit[4];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_phase_textedit[5]->cursor_pos>0)
               {
                  b=timings_phase_textedit[5];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_divider_textedit[0]->cursor_pos>0)
               {
                  b=timings_divider_textedit[0];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_divider_textedit[1]->cursor_pos>0)
               {
                  b=timings_divider_textedit[1];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_divider_textedit[2]->cursor_pos>0)
               {
                  b=timings_divider_textedit[2];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_divider_textedit[3]->cursor_pos>0)
               {
                  b=timings_divider_textedit[3];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_divider_textedit[4]->cursor_pos>0)
               {
                  b=timings_divider_textedit[4];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_muldiv_textedit[0]->cursor_pos>0)
               {
                  b=timings_muldiv_textedit[0];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_muldiv_textedit[1]->cursor_pos>0)
               {
                  b=timings_muldiv_textedit[1];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else
               {
                  b=timings_phase_textedit[0];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               int len=strlen(b->text);
               if(len<max_length)
               {
                  int index=b->cursor_pos-1;
                  if(keybd_data!='-' || index==0) // do nothing when '-' and index!=0
                  {
                     if(index<len)
                     {
                        for(int i=len;i>=index;i--)
                           b->text[i+1]=b->text[i];
                     }
                     b->text[index]=keybd_data;
                     b->cursor_pos++;
                     if(b->cursor_pos>max_length+1)
                        b->cursor_pos=max_length+1;
                     test_tab_timings();
                     ns_repaint();
                     paint_b_apply_timings();
                     paint_b_apply_all_timings();
                  }
               }
               debug_pos;
            }
         }
      }

      if(mouse_pressed==1 && mouse_pressed_old==0)
      {
         win_run();
      }
      else if(mouse_pressed==0 && mouse_pressed_old==1)
      {
         win_actions();
      }
      else if(mouse_pressed && is_dragging())
      {
         int drag_x=get_drag_mousex_pre();
         int drag_y=get_drag_mousey_pre();
         int delta_x=mousex-drag_x;
         int delta_y=mousey-drag_y;
         if(delta_x!=0 || delta_y!=0)
         {
            calculate_drag(delta_x,delta_y);
         }
      }
      win_repaint();
      mouse_pressed_old=mouse_pressed;
      mouse_pressed=0;
   }
}
void hdmi_tick(int clean)
{
   char chars[]="\\|/-";
   static int i=0;
   if(clean==0)
   {
      sprintf(message,"%c",chars[i++]);
      i=i&3;
      print_hdmi(0,message);
   }
   else if(clean==1)
   {
      sprintf(message," ");
      print_hdmi(0,message);
   }
   else //if(clean==2)
   {
      sprintf(message,"Timeout");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }
}
