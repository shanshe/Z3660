// This file has two main functions: mother board test, and BOOT.bin update system.
#include <inttypes.h>
#include "mobotest.h"
#include <xparameters.h>
#include "main.h"
#include "lwip.h"
#include <stdio.h>
#include <stdlib.h>
#include "rtg/fonts.h"
#include "config_file.h"
#include "xtime_l.h"
#include "xil_mmu.h"
#include "pt/pt.h"

#include "ARM_ztop/textedit.h"
#include "ARM_ztop/tabs.h"
int init_xc3sprog(void);
int i2c_finish(void);
int main_xc3sprog(void);
int measures_thread(struct pt *pt);
extern uint32_t ticks;
void b_refresh_action(void);
extern int selected_tab;
extern uint32_t fb_pitch;
extern void *(memcpy_neon)(void * s1, const void * s2, u32 n);
void configure_clk(int clk, int verbose, int nbr);

int timer_thread(struct pt *pt)
{
   PT_BEGIN(pt);
   while(1)
   {
      PT_WAIT_UNTIL(pt,ticks>=1000); // ~1 second
      ticks=0;
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
int timeout=0;

void arm_write_amiga_long(uint32_t address, uint32_t data)
{
   timeout=10000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
   if(bank!=last_bank)
   {
      write_reg(0x18,bank);
      last_bank=bank;
   }
   NOP;
   write_mem32(address,data);
   NOP;
   do {
      NOPX_WRITE();
      timeout--;
   }
   while(read_reg(0x14)==0 && timeout>0);        // read ack
   check_bus_error(read_reg(0x14),address);
   write_reg(0x10,0x0); // ARM Bus Hi-Z
}
void arm_write_amiga_word(uint32_t address, uint32_t data)
{
   timeout=10000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
   if(bank!=last_bank)
   {
      write_reg(0x18,bank);
      last_bank=bank;
   }
   NOP;
   write_mem16(address,data);
   NOP;
   do {
      NOPX_WRITE();
      timeout--;
   }
   while(read_reg(0x14)==0 && timeout>0);        // read ack
   check_bus_error(read_reg(0x14),address);
   write_reg(0x10,0x0); // ARM Bus Hi-Z
}
void arm_write_amiga_byte(uint32_t address, uint32_t data)
{
   timeout=10000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
   if(bank!=last_bank)
   {
      write_reg(0x18,bank);
      last_bank=bank;
   }
   NOP;
   write_mem8(address,data);
   NOP;
   do {
      NOPX_WRITE();
      timeout--;
   }
   while(read_reg(0x14)==0 && timeout>0);        // read ack
   check_bus_error(read_reg(0x14),address);
   write_reg(0x10,0x0); // ARM Bus Hi-Z
}
uint32_t arm_read_amiga_long(uint32_t address)
{
   timeout=10000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
   if(bank!=last_bank)
   {
      write_reg(0x18,bank);
      last_bank=bank;
   }
   NOP;
   read_mem32(address);
   NOP;
   do {
      NOPX_READ();
      timeout--;
   }
   while(read_reg(0x14)==0 && timeout>0);        // read ack
   check_bus_error(read_reg(0x14),address);
   uint32_t data_read=read_reg(0x1C); // read data
   write_reg(0x10,0x0); // ARM Bus Hi-Z
   return(data_read);
}
uint32_t arm_read_amiga_word(uint32_t address)
{
   timeout=1000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
   if(bank!=last_bank)
   {
      write_reg(0x18,bank);
      last_bank=bank;
   }
   NOP;
   read_mem16(address);
   NOP;
   do {
      NOPX_READ();
      timeout--;
   }
   while(read_reg(0x14)==0 && timeout>0);        // read ack
   check_bus_error(read_reg(0x14),address);
   write_reg(0x10,0x0); // ARM Bus Hi-Z
   uint32_t data_read=read_reg(0x1C); // read data
   return(data_read);
}
uint32_t arm_read_amiga_byte(uint32_t address)
{
   timeout=1000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
   if(bank!=last_bank)
   {
      write_reg(0x18,bank);
      last_bank=bank;
   }
   NOP;
   read_mem8(address);
   NOP;
   do {
      NOPX_READ();
      timeout--;
   }
   while(read_reg(0x14)==0 && timeout>0);        // read ack
   check_bus_error(read_reg(0x14),address);
   write_reg(0x10,0x0); // ARM Bus Hi-Z
   uint32_t data_read=read_reg(0x1C); // read data
   return(data_read);
}
char message[300]={0};
void handle_cache_flush(uint32_t address,uint32_t size);
extern ZZ_VIDEO_STATE vs;
void copy_rect_mobotest( int line)
{
#define LINE_MAX ((vs.vmode_vsize/Font->Height) - 1)
   int delta=(line-LINE_MAX)*Font->Height;
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
   if(line>LINE_MAX)
   {
      copy_rect_mobotest(line);
      line=LINE_MAX;
   }
//   handle_cache_flush(((uint32_t)vs.framebuffer) + vs.framebuffer_pan_offset,vs.framebuffer_size);
//   while(video_formatter_read(0)==1) //wait vblank
//   {}
//   while(video_formatter_read(0)==0)
//   {}
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
   arm_write_amiga_byte(0xbfee01,0x50<<16);
   usleep(100);
   arm_write_amiga_byte(0xbfee01,0x10<<16);
   usleep(1000);
}
#define L_SHIFT   0x60
#define R_SHIFT   0x61
#define CAPS_LOCK 0x62
uint8_t shift_status=0;
int16_t mousex=0,mousey=0;
uint8_t counterx=0,countery=0;
uint8_t mouse_pressed=0,mouse_pressed_old=-1;
void do_clip_hw_sprite(int16_t offset_x, int16_t offset_y);
void do_update_hw_sprite_pos(int16_t x, int16_t y);

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

   if(mousex<0) mousex=0;
   if(mousex>vs.vmode_hsize-1) mousex=vs.vmode_hsize-1;
   if(mousey<0) mousey=0;
   if(mousey>vs.vmode_vsize-1) mousey=vs.vmode_vsize-1;
   vs.sprite_x_base=mousex;
   vs.sprite_y_base=mousey;
   do_update_hw_sprite_pos(vs.sprite_x_base, vs.sprite_y_base);
   video_formatter_write((vs.sprite_y_adj << 16) | vs.sprite_x_adj, MNTVF_OP_SPRITE_XY);

   update_hw_sprite_pos();
//   char message2[20];
//   sprintf(message2,"%04d,%04d",mousex,mousey);
//   displayStringAt(Font,0,0,(uint8_t*)message2,LEFT_MODE);
}
uint8_t read_keyboard(uint8_t *data, int enable_mouse)
{
   static int a=0;
   if(enable_mouse)
   {
      uint16_t joyy0dat=arm_read_amiga_word(0xdff00a)&0xFFFF;
      calc_mouseposition(joyy0dat);
      uint8_t mousedat=(arm_read_amiga_byte(0xbfe001)>>16)&0xFF;
      mouse_pressed=mousedat&(1<<6)?0:1;
   }
   uint8_t kb_int=(arm_read_amiga_byte(0xbfed01)>>16)&0xFF;
   if((kb_int&(1<<3))==0)
      return(0);
   arm_write_amiga_byte(0xbfed01,0x00<<16);
   a++;
   uint8_t kb_data=(arm_read_amiga_byte(0xbfec01)>>16)&0xFF;
   uint8_t data1=(~(kb_data>>1))&0x7F;
   keyboard_handshake();
//   sprintf(message,"kb_data %02X  data1  %02X      ",kb_data,data1);
//   line++;
//   print_hdmi(0,message);
//   line--;
   uint8_t down=kb_data&0x01;
   if(data1==L_SHIFT || data1==R_SHIFT || data1==CAPS_LOCK)
   {
      shift_status=down?1:0;
//      sprintf(message,"changed shift_status %d      ",shift_status);
//      line+=2;
//      print_hdmi(0,message);
//      line-=2;
      return(0);
   }
   if(shift_status)
      data1=data1+128;
   *data=kb_tbl[data1];
   return(down);
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
   usleep(1000000);
   hard_reboot();
}
int v_major=1;
int v_minor=3;
int beta=0;
int alfa=0;
int mio14=0;
void test_nops(void)
{
   XTime total_time_start = 0;
   XTime total_time_stop;
   XTime debug_time_start = 0;
   XTime debug_time_stop;
   float read_mbs[10];
   float write_mbs[10];

   fpga_interrupt_connect(isr_video,isr_audio_tx,INT_IPL_ON_THIS_CORE);
#define TOTAL_MEM_TESTED 0x40000
#define TESTED_TIMES 8
   sprintf(message,"NOPS ticks (READ)");
   printf("%s\n",message);
   print_hdmi_ln(0,message,1);
   XTime_GetTime(&total_time_start);
   for(int r=0;r<10;r++)
   {
      shared->nops_read=r;
      XTime_GetTime(&debug_time_start);
      for(int j=0;j<TESTED_TIMES;j++)
         for(uint32_t i=0;i<TOTAL_MEM_TESTED;i+=4)
            arm_read_amiga_long((0x200000-TOTAL_MEM_TESTED-0x10000)+i);
      XTime_GetTime(&debug_time_stop);
      read_mbs[r]=(TESTED_TIMES*((float)TOTAL_MEM_TESTED)/1024./1024.*(COUNTS_PER_SECOND))/((1.0 * (debug_time_stop-debug_time_start)));
      sprintf(message," %2d %6.4f MB/s",r,read_mbs[r]);
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);
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
   printf("%s\n",message);
   print_hdmi_ln(0,message,1);
   shared->nops_read=opt_r;

   sprintf(message,"NOPS ticks (WRITE)");
   printf("%s\n",message);
   print_hdmi_ln(0,message,1);
   for(int w=0;w<10;w++)
   {
      shared->nops_write=w;
      XTime_GetTime(&debug_time_start);
      for(int j=0;j<TESTED_TIMES;j++)
         for(uint32_t i=0;i<TOTAL_MEM_TESTED;i+=4)
            arm_write_amiga_long((0x200000-TOTAL_MEM_TESTED-0x10000)+i,i);
      XTime_GetTime(&debug_time_stop);
      write_mbs[w]=(TESTED_TIMES*((float)TOTAL_MEM_TESTED)/1024./1024.*(COUNTS_PER_SECOND))/((1.0 * (debug_time_stop-debug_time_start)));
      sprintf(message," %2d %6.4f MB/s",w,write_mbs[w]);
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);
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
   printf("%s\n",message);
   print_hdmi_ln(0,message,1);
   shared->nops_read=opt_w;
   float total_time=(1.0 * (total_time_stop-total_time_start)) / (COUNTS_PER_SECOND);
   sprintf(message,"Total time: %4.1f s (%3.1f MB)",total_time,(20.*TOTAL_MEM_TESTED*TESTED_TIMES)/1024./1024.);
   printf("%s\n",message);
   print_hdmi_ln(0,message,1);
}

void show_options(void)
{
#define MSG_LINE(X) sprintf(message,X);print_hdmi_ln(0,message,1)
   MSG_LINE("Update Options:");
   MSG_LINE("'H' to show these options");
   MSG_LINE("'I' to connect to Internet and check if there is an update version");
   MSG_LINE("'O' to download latest z3660_scsi.rom (it will overwrite the z3660scsi.rom file on exFAT)");
   MSG_LINE("'T' to test download latest BOOT.bin (it will NOT write anything)");
   MSG_LINE("'U' to download latest BOOT.bin (it will overwrite the Z3660.bin file)");
   MSG_LINE("'F' to download latest BOOT.bin (it will overwrite the FAILSAFE.bin file)");
   MSG_LINE("'D' to download latest CPLD firmware and flash it");
   MSG_LINE("Test Options:");
   MSG_LINE("'X' to test read/write NOPS optimal values for EMU");
   MSG_LINE("Other Options:");
   MSG_LINE("'B' to copy BOOT.bin to FAILSAFE.bin");
   MSG_LINE("'N' to copy BOOT.bin to Z3660.bin");
   MSG_LINE("'M' to copy Z3660.bin to BOOT.bin");
   MSG_LINE("'J' to copy Z3660.bin to FAILSAFE.bin");
   MSG_LINE("'K' to copy FAILSAFE.bin to BOOT.bin");
   MSG_LINE("'L' to copy FAILSAFE.bin to Z3660.bin");
   MSG_LINE("'R' to reboot");
   MSG_LINE(" ");
   MSG_LINE("Note: BOOT.bin boots  the  system  and  it  will  search  for  Z3660.bin  to  continue");
   MSG_LINE("      booting. If the Z3660.bin file is bad, then it will search for FAILSAFE.bin file");
   MSG_LINE("      to boot. If FAILSAFE.bin is bad then BOOT.bin will continue booting from itself.");
}
void mobotest(void)
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
   usleep(1000);
   CPLD_RESET_ARM(1); // CPLD RUN
   int timeout1=0;
   while(READ_NBG_ARM()==1) // make sure that we have the bus control
   {
      usleep(1000);
      timeout1++;
      if(timeout1==1000)
         break;
   }
#ifdef CPLD_PROGRAMMING
   if(timeout1==1000 || XGpioPs_ReadPin(&GpioPs, n040RSTI)==0)
   {
      printf("Timeout waiting NBG_ARM (CPLD erased?)\n");
      CPLD_RESET_ARM(0);
      Xil_ExceptionDisable();
      init_xc3sprog();
      main_xc3sprog();
      i2c_finish();
      Xil_ExceptionEnable();
      CPLD_RESET_ARM(1);
   }
#endif
   usleep(1000);

   int failed=0;
   int ovl_failed=0;

   if(config.enable_test==_YES || config.enable_test==_MIN)
   {
      sprintf(message,"[TEST] Simple Amiga bus test...");
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);

      sprintf(message,"[TEST] 0xbfe201 %08lX",arm_read_amiga_byte(0xbfe201));
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);
      sprintf(message,"[TEST] 0xbfe001 %08lX",arm_read_amiga_byte(0xbfe001));
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);

      sprintf(message,"[TEST] Setting new values");
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);
//#define FORCE_OVL_FAIL
#ifndef FORCE_OVL_FAIL
      arm_write_amiga_byte(0xbfe201,0x01<<16); // OVL as OUTPUT
#endif
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 1 to 0xbfe201");
         printf("%s\n",message);
         print_hdmi_ln(0,message,1);
         Font->TextColor=0x00FFFFFF; // white
      }
      arm_write_amiga_byte(0xbfe001,0x00<<16); // OVL LOW to see the CHIP RAM
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0 to 0xbfe001");
         printf("%s\n",message);
         print_hdmi_ln(0,message,1);
         Font->TextColor=0x00FFFFFF; // white
      }
      sprintf(message,"[TEST] 0xbfe201 %08lX",arm_read_amiga_byte(0xbfe201));
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);
      sprintf(message,"[TEST] 0xbfe001 %08lX",arm_read_amiga_byte(0xbfe001));
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);

      arm_write_amiga_long(0x00000000,0xAA55AA55);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0xAA55AA55 to 0x00000000");
         printf("%s\n",message);
         print_hdmi_ln(0,message,1);
         Font->TextColor=0x00FFFFFF; // white
      }
      arm_write_amiga_long(0x00100000,0x55AA55AA);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0x55AA55AA to 0x00100000");
         printf("%s\n",message);
         print_hdmi_ln(0,message,1);
         Font->TextColor=0x00FFFFFF; // white
      }
      uint32_t data1=arm_read_amiga_long(0x00000000);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when reading from 0x00000000");
         printf("%s\n",message);
         print_hdmi_ln(0,message,1);
         Font->TextColor=0x00FFFFFF; // white
      }
      uint32_t data2=arm_read_amiga_long(0x00100000);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when reading from 0x00100000");
         printf("%s\n",message);
         print_hdmi_ln(0,message,1);
         Font->TextColor=0x00FFFFFF; // white
      }
      if(data1==0xAA55AA55 && data2==0x55AA55AA)
      {
         sprintf(message,"[TEST] Test basic Amiga CHIP RAM ok");
         printf("%s\n",message);
         print_hdmi_ln(0,message,1);
      }
      else
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Test basic Amiga CHIP RAM FAILED!!!");
         printf("%s\n",message);
         print_hdmi_ln(0,message,1);
         Font->TextColor=0x00FFFFFF; // white
         if(data1==0xAA55AA55)
         {
            sprintf(message,"[TEST] (0x00000000) -> 0x%08lX == 0xAA55AA55  OK!!!",data1);
            printf("%s\n",message);
            print_hdmi_ln(0,message,1);
         }
         else if(data1==0x11144EF9)
         {
            ovl_failed=1;
            Font->TextColor=0x00FF0000; // red
            sprintf(message,"[TEST] (0x00000000) -> 0x%08lX == 0x11144EF9 This is the kickstart!!!",data1);
            printf("%s\n",message);
            print_hdmi_ln(0,message,1);
            Font->TextColor=0x00FFFFFF; // white
         }
         else
         {
            Font->TextColor=0x00FF0000; // red
            sprintf(message,"[TEST] (0x00000000) -> 0x%08lX instead of 0xAA55AA55",data1);
            printf("%s\n",message);
            print_hdmi_ln(0,message,1);
            Font->TextColor=0x00FFFFFF; // white
         }
         if(data2==0x55AA55AA)
         {
            sprintf(message,"[TEST] (0x00100000) -> 0x%08lX == 0x55AA55AA  OK!!!",data2);
            printf("%s\n",message);
            print_hdmi_ln(0,message,1);
         }
         else
         {
            Font->TextColor=0x00FF0000; // red
            sprintf(message,"[TEST] (0x00100000) -> 0x%08lX instead of 0x55AA55AA",data2);
            printf("%s\n",message);
            print_hdmi_ln(0,message,1);
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
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);

      usleep(1000000); // wait time to allow monitor to show the screen
      failed=0;
      uint8_t * test_mem1=malloc(0x200000);
      uint8_t * test_mem2=malloc(0x200000);
      for(uint32_t i=0;i<0x200000;i++)
         test_mem1[i]=(uint8_t)(rand()&0xFF);
      sprintf(message,"[TEST] Writing 2MB of random data");
      printf("%s",message);
      print_hdmi_ln(0,message,1);
      int x=34;
      for(uint32_t i=0;i<0x200000;i+=4)
      {
         arm_write_amiga_long(i,((uint32_t *)test_mem1)[i>>2]);
         if(timeout==0)
         {
            if(failed==0)
               printf("\n");
            sprintf(message,"[TEST] Timeout when writing 0x%08lX to 0x%08lX",((uint32_t *)test_mem1)[i>>2],i);
            printf("%s\n",message);
            Font->TextColor=0x00FF0000; // red
            sprintf(message,"Timeout when writing 0x%08lX to 0x%08lX      ",((uint32_t *)test_mem1)[i>>2],i);
            int line_old=line;
            line=0;
            print_hdmi(60,message);
            line=line_old;
            Font->TextColor=0x00FFFFFF; // white
            failed=1;
         }
         if(failed==0 && (i%0x10000)==0)
         {

            sprintf(message,".");
            printf("%s",message);
            line--;
            print_hdmi(x++,message);
            line++;
         }
      }
      if(failed==0)
      {
         sprintf(message,"done");
         printf("%s\n",message);
         line --;
         print_hdmi(x,message);
         line++;
      }
      failed=0;
      x=34;
      sprintf(message,"[TEST] Reading 2MB of data       ");
      printf("%s",message);
      print_hdmi_ln(0,message,1);
      for(uint32_t i=0;i<0x200000;i+=4)
      {
         ((uint32_t *)test_mem2)[i>>2]=arm_read_amiga_long(i);
         if(timeout==0)
         {
            if(failed==0)
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
            failed=1;
         }
         if(failed==0 && (i%0x10000)==0)
         {
            sprintf(message,".");
            printf("%s",message);
            line--;
            print_hdmi(x++,message);
            line++;
         }
      }
      if(failed==0)
      {
         sprintf(message,"done");
         printf("%s\n",message);
         line--;
         print_hdmi(x++,message);
         line++;
      }
      sprintf(message,"[TEST] Comparing 2MB of data     ");
      printf("%s",message);
      print_hdmi_ln(0,message,1);
      failed=0;
      x=34;
      for(uint32_t i=0;i<0x200000;i+=4)
      {
         uint32_t data1=((uint32_t *)test_mem1)[i>>2];
         uint32_t data2=((uint32_t *)test_mem2)[i>>2];
         if(data1!=data2)
         {
            if(failed==0)
               printf("\n");
            sprintf(message,"[TEST] Address 0x%08lX failed: W/R %08lX/%08lX",i,data1,data2);
            printf("%s\n",message);
            Font->TextColor=0x00FF0000; // red
            sprintf(message,"Address 0x%08lX failed: W/R %08lX/%08lX",i,data1,data2);
            int line_old=line;
            line=0;
            print_hdmi(60,message);
            line=line_old;
            Font->TextColor=0x00FFFFFF; // white
            failed=1;
         }
         if(failed==0 && (i%0x10000)==0)
         {
            sprintf(message,".");
            printf("%s",message);
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
      if(failed==0)
      {
         sprintf(message,"done");
         printf("%s\n",message);
         line--;
         print_hdmi(x++,message);
         line++;
      }
      free(test_mem1);
      free(test_mem2);
   }

   if(failed || config.enable_test==_YES || config.enable_test==_MIN)
   {
      sprintf(message,"[TEST] Restoring...");
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);
      arm_write_amiga_byte(0xbfe001,0xFF<<16); // OVL HIGH to see the ROM
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0Xff to 0xbfe001");
         printf("%s\n",message);
         print_hdmi_ln(0,message,1);
         Font->TextColor=0x00FFFFFF; // white
      }
      arm_write_amiga_byte(0xbfe201,0x00<<16); // OVL HIGH to see the ROM
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0 to 0xbfe201");
         printf("%s\n",message);
         print_hdmi_ln(0,message,1);
         Font->TextColor=0x00FFFFFF; // white
      }

      sprintf(message,"[TEST] 0xbfe201 %08lX",arm_read_amiga_byte(0xbfe201));
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);
      sprintf(message,"[TEST] 0xbfe001 %08lX",arm_read_amiga_byte(0xbfe001));
      printf("%s\n",message);
      print_hdmi_ln(0,message,1);
   }
}
extern const char *bootmode_names[];
uint8_t cursor_data[]={
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
   int time=2;
   if(config.enable_test==_YES)
      time=10;
   printf("  Continuing in %d seconds...  \n",time);
   for(int i=time*1000;i>0;i--)
   {
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
            goto start_ztop;
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

   if(preset_selected>=0 && preset_selected<PRESET_CB_MAX-1)
      sprintf(message,"Selected preset %d \"%s\"",preset_selected,env_file_vars_temp[preset_selected].preset_name);
   else
      sprintf(message,"Default preset (z3360cfg.txt file)");
   printf("%s\n",message);
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);

   if(config.boot_mode==0)
      sprintf(message,"060 CPU");
   else
      sprintf(message,"%s CPU EMULATOR",bootmode_names[config.boot_mode]);
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);

   sprintf(message,"BUS Frequency %d MHz",config.cpufreq);
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);

   sprintf(message,"Z3 RAM %s",config.autoconfig_ram?"Enabled":"Disabled");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);

   sprintf(message,"Z3 RTG AUTOCONFIG %s",config.autoconfig_rtg?"Enabled":"Disabled");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);

   sprintf(message,"SCSI BOOT %s",config.scsiboot?"Enabled":"Disabled");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);

   sprintf(message,"MAC ADDRESS %02X:%02X:%02X:%02X:%02X:%02X",config.mac_address[0],
         config.mac_address[1],config.mac_address[2],
         config.mac_address[3],config.mac_address[4],config.mac_address[5]);
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);

   NBR_ARM(1);
   usleep(1000);
   CPLD_RESET_ARM(0); // CPLD RESET
   return;

start_console:
   hw_sprite_show(0);
   vs.sprite_showing=0;
   show_options();
#ifdef CPLD_PROGRAMMING
   CPLD_RESET_ARM(0);
   Xil_ExceptionDisable();
   init_xc3sprog();
   Xil_ExceptionEnable();
   CPLD_RESET_ARM(1);
#endif
   int connected=0;
   alfa=0;
   while(1)
   {
      if(read_keyboard(&keybd_data,0))
      {
         if(keybd_data=='i' || keybd_data=='I')
         {
            alfa=0;
            if(connected==0)
            {
               sprintf(message,"Ethernet PHY auto-negotiation");
               print_hdmi_ln(0,message,1);
               connected=lwip_connect();
               if(connected)
               {
                  sprintf(message,"Connected");
                  print_hdmi_ln(0,message,1);
               }
               else
               {
                  sprintf(message,"NOT Connected");
                  print_hdmi_ln(0,message,1);
               }
            }
            if(connected)
            {
               sprintf(message,"Getting version info");
               print_hdmi_ln(0,message,1);
               int ok=lwip_get_update_version("version.txt",0); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest BOOT.bin version is     %s",version_string_export);
                  print_hdmi_ln(0,message,1);
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
               }
               ok=lwip_get_update_version_scsirom("scsirom_version.txt",0); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660_scsi.rom version is v%s",version_scsirom_string_export);
                  print_hdmi_ln(0,message,1);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660_scsi.rom", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_mount(0,Path,1); // unmount
                  int pos=0;
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
                  free(DATA);
               }
#ifdef CPLD_PROGRAMMING
               ok=lwip_get_update_version_jed("jed_version.txt",0); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660.jed date is %s",version_jed_string_export);
                  print_hdmi_ln(0,message,1);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660.jed", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_mount(0,Path,1); // unmount
                  int pos=0;
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
                  free(DATA);
               }
#endif
            }
         }
         else if(keybd_data=='a' || keybd_data=='A')
         {
            alfa=1;
            if(connected==0)
            {
               sprintf(message,"Ethernet PHY auto-negotiation");
               print_hdmi_ln(0,message,1);
               connected=lwip_connect();
               if(connected)
               {
                  sprintf(message,"Connected");
                  print_hdmi_ln(0,message,1);
               }
               else
               {
                  sprintf(message,"NOT Connected");
                  print_hdmi_ln(0,message,1);
               }
            }
            if(connected)
            {
               sprintf(message,"Getting version info");
               print_hdmi_ln(0,message,1);
               int ok=lwip_get_update_version("version.txt",1); // 1 = alfa
               if(ok)
               {
                  sprintf(message,"Latest BOOT.bin version is     %s",version_string_export);
                  print_hdmi_ln(0,message,1);
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
               }
               ok=lwip_get_update_version_scsirom("scsirom_version.txt",1); // 1 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660_scsi.rom version is v%s",version_scsirom_string_export);
                  print_hdmi_ln(0,message,1);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660_scsi.rom", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_mount(0,Path,1); // unmount
                  int pos=0;
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
                  free(DATA);
               }
#ifdef CPLD_PROGRAMMING
               ok=lwip_get_update_version_jed("jed_version.txt",1); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660.jed date is %s",version_jed_string_export);
                  print_hdmi_ln(0,message,1);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660.jed", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_mount(0,Path,1); // unmount
                  int pos=0;
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
#ifdef CPLD_PROGRAMMING
         else if(keybd_data=='d' || keybd_data=='D')
         {
            if(connected==0)
            {
               sprintf(message,"Not connected");
               print_hdmi_ln(0,message,1);
            }
            else
            {
               char filename[]="z3660.jed";
               sprintf(message,"Downloading the %s file",filename);
               print_hdmi_ln(0,message,1);
               DATA=malloc(1*1024*1024);
               int ok=lwip_get_update_jed(filename,alfa);
               if(ok)
               {
                  uint32_t checksum32=0;
                  uint32_t *data_u32=(uint32_t *)DATA;
                  for(int i=0;i<download_data.filesize_jed/4;i++)
                     checksum32+=__builtin_bswap32(data_u32[i]);
                  sprintf(message,"Checksum-32 %lu (%08lX) %lu (%08lX)",checksum32,checksum32,download_data.checksum32_jed,download_data.checksum32_jed);
                  print_hdmi_ln(0,message,1);
                  if(checksum32==download_data.checksum32_jed)
                  {
                     sprintf(message,"Writing the %s file...",filename);
                     print_hdmi_ln(0,message,1);
                     ACTIVITY_LED_ON; // ON
                     FIL fil;
                     FATFS fatfs;
                     TCHAR *Path = "1:/";
                     f_mount(&fatfs, Path, 1); // 1 mount immediately
                     char filename_ex[20];
                     sprintf(filename_ex,"%s%s",Path,filename);
                     f_open(&fil,filename_ex, FA_CREATE_ALWAYS | FA_WRITE);
                     UINT NumBytesWritten;
                     f_write(&fil, DATA, download_data.filesize_jed,&NumBytesWritten);
                     f_close(&fil);
                     ACTIVITY_LED_OFF; // OFF

                     sprintf(message,"File %s written (%d bytes of %ld)",filename,NumBytesWritten,download_data.filesize_jed);
                     print_hdmi_ln(0,message,1);
                     f_mount(0,Path,1); // unmount
                  }
                  else
                  {
                     sprintf(message,"File %s NOT written (bad checksum)",filename);
                     print_hdmi_ln(0,message,1);
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
            main_xc3sprog();
            i2c_finish();
            Xil_ExceptionEnable();

            //Restore pins
#ifdef DIRECT_CPLD_PROGRAMMING
            configure_clk(config.cpufreq,0,0);
#endif
            CPLD_RESET_ARM(1);

         }
#endif // CPLD_PROGRAMMIG
         else if(keybd_data=='u' || keybd_data=='U' || keybd_data=='f' || keybd_data=='F' || keybd_data=='t' || keybd_data=='T')
         {
            if(connected==0)
            {
               sprintf(message,"Not connected");
               print_hdmi_ln(0,message,1);
            }
            else
            {
               sprintf(message,"Downloading the BOOT.bin file");
               print_hdmi_ln(0,message,1);
               DATA=malloc(16*1024*1024);
               int ok=lwip_get_update("BOOT.BIN",alfa);
               if(ok)
               {
                  uint32_t checksum32=0;
                  uint32_t *data_u32=(uint32_t *)DATA;
                  for(int i=0;i<download_data.filesize/4;i++)
                     checksum32+=__builtin_bswap32(data_u32[i]);
                  sprintf(message,"Checksum-32 %lu (%08lX) %lu (%08lX)",checksum32,checksum32,download_data.checksum32,download_data.checksum32);
                  print_hdmi_ln(0,message,1);
                  char filename[20];
                  if(keybd_data=='u' || keybd_data=='U' || keybd_data=='f' || keybd_data=='F')
                  {
                     if(checksum32==download_data.checksum32)
                     {
                        if(keybd_data=='u' || keybd_data=='U')
                           strcpy(filename,"Z3660.bin");
                        else
                           strcpy(filename,"FAILSAFE.bin");
                        sprintf(message,"Writting the %s file...",filename);
                        print_hdmi_ln(0,message,1);
                        ACTIVITY_LED_ON; // ON
                        FIL fil;
                        FATFS fatfs;
                        TCHAR *Path = "0:/";
                        f_mount(&fatfs, Path, 1); // 1 mount immediately
                        f_open(&fil,filename, FA_CREATE_ALWAYS | FA_WRITE);
                        UINT NumBytesWritten;
                        f_write(&fil, DATA, download_data.filesize,&NumBytesWritten);
                        f_close(&fil);
                        ACTIVITY_LED_OFF; // OFF
                        sprintf(message,"File %s written (%d bytes of %ld)",filename,NumBytesWritten,download_data.filesize);
                        print_hdmi_ln(0,message,1);
                        f_mount(0,Path,1); // unmount
                     }
                     else
                     {
                        sprintf(message,"File %s NOT written (bad checksum)",filename);
                        print_hdmi_ln(0,message,1);
                     }
                  }
                  else
                  {
                     if(checksum32==download_data.checksum32)
                     {
                        sprintf(message,"Download test OK");
                        print_hdmi_ln(0,message,1);
                     }
                  }
               }
               free(DATA);
            }
         }
         else if(keybd_data=='o' || keybd_data=='O')
         {
            if(connected==0)
            {
               sprintf(message,"Not connected");
               print_hdmi_ln(0,message,1);
            }
            else
            {
               char filename[]="z3660_scsi.rom";
               sprintf(message,"Downloading the %s file",filename);
               print_hdmi_ln(0,message,1);
               DATA=malloc(1*1024*1024);
               int ok=lwip_get_update_scsirom(filename,alfa);
               if(ok)
               {
                  uint32_t checksum32=0;
                  uint32_t *data_u32=(uint32_t *)DATA;
                  for(int i=0;i<download_data.filesize_scsirom/4;i++)
                     checksum32+=__builtin_bswap32(data_u32[i]);
                  sprintf(message,"Checksum-32 %lu (%08lX) %lu (%08lX)",checksum32,checksum32,download_data.checksum32_scsirom,download_data.checksum32_scsirom);
                  print_hdmi_ln(0,message,1);
                  if(checksum32==download_data.checksum32_scsirom)
                  {
                     sprintf(message,"Writing the %s file...",filename);
                     print_hdmi_ln(0,message,1);
                     ACTIVITY_LED_ON; // ON
                     FIL fil;
                     FATFS fatfs;
                     TCHAR *Path = "1:/";
                     f_mount(&fatfs, Path, 1); // 1 mount immediately
                     char filename_ex[20];
                     sprintf(filename_ex,"%s%s",Path,filename);
                     f_open(&fil,filename_ex, FA_CREATE_ALWAYS | FA_WRITE);
                     UINT NumBytesWritten;
                     f_write(&fil, DATA, download_data.filesize_scsirom,&NumBytesWritten);
                     f_close(&fil);
                     ACTIVITY_LED_OFF; // OFF

                     sprintf(message,"File %s written (%d bytes of %ld)",filename,NumBytesWritten,download_data.filesize_scsirom);
                     print_hdmi_ln(0,message,1);
                     f_mount(0,Path,1); // unmount
                  }
                  else
                  {
                     sprintf(message,"File %s NOT written (bad checksum)",filename);
                     print_hdmi_ln(0,message,1);
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
            else if(keybd_data=='K' || keybd_data=='K')
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

            ACTIVITY_LED_ON; // ON
            FIL fil_src,fil_dst;
            FATFS fatfs;
            TCHAR *Path = "0:/";
            f_mount(&fatfs, Path, 1); // 1 mount immediately
            f_open(&fil_src,src, FA_READ);
            UINT NumBytesRead;
            FSIZE_t filesize=fil_src.obj.objsize;
            sprintf(message,"Filesize to copy %lld bytes",filesize);
            print_hdmi_ln(0,message,1);
            DATA=malloc(16*1024*1024);
            f_read(&fil_src, DATA, filesize,&NumBytesRead);
            f_close(&fil_src);

            f_open(&fil_dst,dst, FA_CREATE_ALWAYS | FA_WRITE);
            UINT NumBytesWritten;
            f_write(&fil_dst, DATA,filesize,&NumBytesWritten);
            f_close(&fil_dst);
            if(NumBytesRead==NumBytesWritten)
            {
               sprintf(message,"Copied file %s to %s",src,dst);
               print_hdmi_ln(0,message,1);
            }
            else
            {
               sprintf(message,"NumBytesRead %d != NumBytesWritten %d",NumBytesRead,NumBytesWritten);
               print_hdmi_ln(0,message,1);
               sprintf(message,"Error when copying file %s to %s",src,dst);
               print_hdmi_ln(0,message,1);
            }
            f_mount(0,Path,1); // unmount
            ACTIVITY_LED_OFF; // OFF
            free(DATA);
         }
         else if(keybd_data=='r' || keybd_data=='R')
         {
            reboot();
         }
//         sprintf(message,"Detected key %02X %c               ",keybd_data,keybd_data);
//         print_hdmi_ln(0,message);
//         line--;
      }
      if(connected)
         lwip_run();
   }
   return;

start_ztop:

   init_win();

   uint8_t* bmp_data = (uint8_t*) (((uint32_t) vs.framebuffer) + (1920*1080*2));
   uint8_t* bmp_colors = (uint8_t*) (((uint32_t) vs.framebuffer) + (1920*1080*2+32*48));

   bmp_colors[ 0]=0xFF;bmp_colors[ 1]=0xFF;bmp_colors[ 2]=0xFF; // yeah 24 bit per color
   bmp_colors[ 3]=0xF3;bmp_colors[ 4]=0x30;bmp_colors[ 5]=0x30;
   bmp_colors[ 6]=0xF0;bmp_colors[ 7]=0xEC;bmp_colors[ 8]=0xDC;
   bmp_colors[ 9]=0x00;bmp_colors[10]=0x00;bmp_colors[11]=0x00;

   for(int i=0;i<32*48;i++)
      bmp_data[i]=cursor_data[i];
   update_hw_sprite_clut(bmp_data,bmp_colors,32,48,0,0);
   update_hw_sprite_pos();
   hw_sprite_show(1);
   vs.sprite_showing=1;
   do_clip_hw_sprite(0, 0);

   xadc_init();
   ltc2990_init();
   PT_INIT(&pt_measures);
   PT_INIT(&pt_timer);
   fpga_interrupt_connect(isr_video,isr_audio_tx,INT_IPL_ON_THIS_CORE);
   for(int i=0;i<20;i++)
   {
      usleep(1000);
      task_counter=10000000;
      measures_thread(&pt_measures);
   }

   show_ztop();

   while(1)
   {
      task_counter+=100;
      measures_thread(&pt_measures);
      timer_thread(&pt_timer);
      if(read_keyboard(&keybd_data,1))
      {
/*
         if(keybd_data=='r' || keybd_data=='R')
         {
            reboot();
         }
*/
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
   }
}
