
/* ------------------------------------------------------------ */
/*            Include File Definitions                  */
/* ------------------------------------------------------------ */

#include "sii9022_init/sii9022_init.h"
#include "xuartps.h"
#include "main.h"
#include "interrupt.h"
#include "xil_types.h"
#include "xil_cache.h"
#include "xil_cache_l.h"
//#include "xparameters.h"
#include <stdio.h>
#include "config_file.h"

#include "video.h"
#include "ax.h"
#include "rtg/fonts.h"

#include "rtg/zz_video_modes.h"
#include "rtg/zzregs.h"
#include "rtg/gfx.h"
#include "xaxivdma.h"
#include "xclk_wiz.h"
#include <sleep.h>
#include "memorymap.h"
#include "render/gfx2.h"

//#include "mpg/ff.h"
#include <ff.h>
#include "mpg/pl_mpeg_player.h"

#define VDMA_DEVICE_ID   XPAR_AXIVDMA_0_DEVICE_ID

extern const char *bootmode_names[];

void write_rtg_register(uint16_t zaddr,uint32_t zdata);
uint32_t read_rtg_register(uint16_t zaddr);

extern ZZ_VIDEO_STATE vs;
extern XAxiVdma vdma;
extern XClk_Wiz clkwiz;
extern XClk_Wiz_Config conf;
extern uint16_t original_h;
uint32_t sprite_buf[32 * 48];
uint8_t sprite_clipped = 0;
int16_t sprite_clip_x = 0, sprite_clip_y = 0;
uint8_t color_reset=0;
uint8_t stride_div = 1;

uint16_t sprite_request_update_pos = 0;
uint16_t sprite_request_update_data = 0;
uint16_t sprite_request_show = 0;
uint16_t sprite_request_hide = 0;
uint16_t sprite_request_pos_x = 0;
uint16_t sprite_request_pos_y = 0;

uint16_t color16[8]={(uint16_t)((0x0f<<11)|(0x1f<<5)|(0x0f)),
                     (uint16_t)((0x0f<<11)|(0x00<<5)|(0x00)),
                     (uint16_t)((0x00<<11)|(0x1f<<5)|(0x00)),
                     (uint16_t)((0x00<<11)|(0x00<<5)|(0x0f)),
                     (uint16_t)((0x0f<<11)|(0x1f<<5)|(0x00)),
                     (uint16_t)((0x00<<11)|(0x1f<<5)|(0x0f)),
                     (uint16_t)((0x0f<<11)|(0x00<<5)|(0x0f)),
                     (uint16_t)((0x04<<11)|(0x08<<5)|(0x04)),
};
uint32_t color32[8]={(uint32_t)((0xf0L<<16)|(0xf0L<<8)|(0xf0L)),
                     (uint32_t)((0xf0L<<16)|(0x00L<<8)|(0x00L)),
                     (uint32_t)((0x00L<<16)|(0xf0L<<8)|(0x00L)),
                     (uint32_t)((0x00L<<16)|(0x00L<<8)|(0xf0L)),
                     (uint32_t)((0xf0L<<16)|(0xf0L<<8)|(0x00L)),
                     (uint32_t)((0x00L<<16)|(0xf0L<<8)|(0xf0L)),
                     (uint32_t)((0xf0L<<16)|(0x00L<<8)|(0xf0L)),
                     (uint32_t)((0x40L<<16)|(0x40L<<8)|(0x40L)),
};
zz_video_mode preset_video_modes[ZZVMODE_NUM] = {
    //   HRES       VRES    HSTART  HEND    HMAX    VSTART  VEND    VMAX    POLARITY    MHZ     PIXELCLOCK HZ   VERTICAL HZ     HDMI    MUL/DIV/DIV2
    {    1280,      720,    1390,   1430,   1650,    725,    730,    750,    0,          74,     74250000,       60,             0,      49, 2, 33 },
    {     800,      600,     840,    968,   1056,    601,    605,    628,    0,          40,     40000000,       60,             0,      14, 1, 35 },
    {     640,      480,     656,    752,    800,    490,    492,    525,    0,          25,     25175000,       60,             0,      15, 1, 60 },
    {    1024,      768,    1048,   1184,   1344,    771,    777,    806,    0,          65,     65000000,       60,             0,      13, 1, 20 },
    {    1280,     1024,    1328,   1440,   1688,   1025,   1028,   1066,    0,         108,    108000000,       60,             0,      54, 5, 10 },
    {    1920,     1080,    2008,   2052,   2200,   1084,   1089,   1125,    0,         148,    148500000,       60,             0,      27, 1, 18 },
    {     720,      576,     732,    796,    864,    581,    586,    625,    1,          27,     27000000,       50,             0,      45, 2, 83 },
    {    1920,     1080,    2448,   2492,   2640,   1084,   1089,   1125,    0,         148,    148500000,       50,             0,      27, 1, 18 },
    {     720,      480,     720,    752,    800,    490,    492,    525,    0,          25,     25175000,       60,             0,      19, 1, 75 },
    {     640,      512,     840,    968,   1056,    601,    605,    628,    0,          40,     40000000,       60,             0,      14, 1, 35 },
    {    1600,     1200,    1704,   1880,   2160,   1201,   1204,   1242,    0,         161,     16089999,       60,             0,      21, 1, 13 },
    {    2560,     1440,    2680,   2944,   3328,   1441,   1444,   1465,    0,         146,     15846000,       30,             0,      41, 2, 14 },
    {     720,      576,     732,    796,    864,    581,    586,    625,    1,          27,     27000000,       50,             0,      31, 1,115 }, // 720x576 non-standard VSync (PAL Amiga)
    {     720,      480,     720,    752,    800,    490,    492,    525,    0,          25,     25175000,       60,             0,      61, 5, 49 }, // 720x480 non-standard VSync (PAL Amiga)
    {     720,      576,     732,    796,    864,    581,    586,    625,    1,          27,     27000000,       50,             0,      59, 7, 31 }, // 720x576 non-standard VSync (NTSC Amiga)
    {     720,      480,     720,    752,    800,    490,    492,    525,    0,          25,     25175000,       60,             0,      37, 3, 49 }, // 720x480 non-standard VSync (NTSC Amiga)
    {     640,      400,     656,    752,    800,    490,    492,    525,    0,          25,     25175000,       60,             0,      15, 1, 60 },
    // The final entry here is the custom video mode, accessible through registers for debug purposes.
    {    1280,      720,    1390,   1430,   1650,    725,    730,    750,    0,          75,     75000000,       60,             0,      15, 1, 20 },
};

void do_update_hw_sprite_pos(int16_t x, int16_t y);
void do_clip_hw_sprite(int16_t offset_x, int16_t offset_y);


int interrupt_init(void* isr_video);

ZZ_VIDEO_STATE* video_init() {
#define NO_SCALE 0
#define SCALEXY 3
   vs.framebuffer = (uint32_t*) FRAMEBUFFER_ADDRESS;
   vs.video_mode = ZZVMODE_800x600 | NO_SCALE << 12 | MNTVA_COLOR_16BIT565 << 8;
   vs.colormode = 0;
   vs.framebuffer_size=800*600*1;
   interrupt_init(isr_video);

//   video_reset();

   return(&vs);
}
void set_pixelclock(zz_video_mode *mode);
void set_palette(uint32_t zdata,uint16_t op_palette);
void handle_cache_flush(uint32_t address,uint32_t size)
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
//	Xil_DCacheFlushRange((INTPTR) address, size);
}

void video_reset(void) {
//   if(reset_frame_buffer)
//      memset((uint32_t*)vs.framebuffer,0,1920*1080*2);
//   Xil_ExceptionDisable();
//   video_mode_init(ZZVMODE_1920x1080_60, 0, MNTVA_COLOR_16BIT565);
//   Xil_ExceptionEnable();
//   set_pixelclock(&preset_video_modes[ZZVMODE_800x600]);
//   sii9022_init(&preset_video_modes[ZZVMODE_800x600]);
//   init_vdma(800,600, 2, 1, (uint32_t)vs.framebuffer);
//   if(reset_frame_buffer)
//      memset((uint32_t*)frameBuf,0,800*600*4);
//   set_pixelclock(&preset_video_modes[ZZVMODE_1920x1080_60]);
//   sii9022_init(&preset_video_modes[ZZVMODE_1920x1080_60]);
//   init_vdma(1920,1080, 1, 1, (uint32_t)frameBuf,&preset_video_modes[ZZVMODE_1920x1080_60]);
   rtg_init();
//   dump_vdma_status(&vdma);

   vs.framebuffer_pan_width = 0;
   vs.framebuffer_pan_offset = 0;
   vs.split_request_pos = 0;
   vs.split_pos = 1; // force update slpit_pos from split_request_pos and write to videoformatter

   vs.sprite_colors[0] = 0x00ff00ff;
   vs.sprite_colors[1] = 0x00000000;
   vs.sprite_colors[2] = 0x00000000;
   vs.sprite_colors[3] = 0x00000000;

   vs.sprite_width = 16;
   vs.sprite_height = 16;

   sprite_request_hide = 1;
//#define MPG_VIDEO_TEST
#ifdef MPG_VIDEO_TEST
//   static FIL fil;      /* File object */
   static FATFS fatfs;
   TCHAR *Path = DEFAULT_ROOT;
   TCHAR *filename = DEFAULT_ROOT "Bailando.mpg";
//   TCHAR *filename = DEFAULT_ROOT "BraculaCondemorII.mpg";
   f_mount(&fatfs, Path, 1); // 1 mount immediately

   f_open(&fil,filename, FA_OPEN_ALWAYS | FA_READ);

   f_lseek(&fil, 0);

//   UINT NumBytesRead;
//   printf("Reading %s file\n",filename);
//   f_read(&fil, (void*)((uint32_t)vs.framebuffer), 1080,&NumBytesRead);

//   f_close(&fil);
//   printf("\r\nFile read %d\n",NumBytesRead);
//   reset_video(NO_RESET_FRAMEBUFFER);
//   f_lseek(&fil, 0);

   original_h=256;
   video_mode_init(ZZVMODE_640x480, NO_SCALE, MNTVA_COLOR_32BIT);
   set_fb((uint32_t*) (((uint32_t) vs.framebuffer) + 0), vs.vmode_hsize/vs.vmode_hdiv);
   memset(vs.framebuffer,0,vs.framebuffer_size);
//   fill_rect_solid(40, 40, 800-80, 600-80, swap16(color[color_reset&3]), MNTVA_COLOR_16BIT565);
//   color_reset++;
//   color_reset&=3;
   set_pixelclock(&preset_video_modes[vs.video_mode]);
   sii9022_init(&preset_video_modes[vs.video_mode]);

   fpga_interrupt_connect(isr_video,isr_audio_tx,INT_IPL_ON_THIS_CORE);
   Xil_ExceptionEnable();
   player_mpeg(&fil,filename);
//   Xil_ExceptionDisable();
   f_close(&fil);
#endif
#define BOOT_SCREEN
#ifdef BOOT_SCREEN

//   memset((uint32_t*)vs.framebuffer,0,800*600*2);
   while(video_formatter_read(0)==1);
   while(video_formatter_read(0)==0);
   original_h=256;
   video_mode_init(ZZVMODE_800x600, NO_SCALE, MNTVA_COLOR_16BIT565);
   set_fb((uint32_t*) (((uint32_t) vs.framebuffer) + 0), vs.vmode_hsize/vs.vmode_hdiv);
   memset(vs.framebuffer,0,vs.framebuffer_size);
//   fill_rect_solid(0, 0, 800, 600, swap16(0x8410), MNTVA_COLOR_16BIT565);
//   color_reset++;
//   color_reset&=3;
   set_pixelclock(&preset_video_modes[vs.video_mode]);
   sii9022_init(&preset_video_modes[vs.video_mode]);
   while(video_formatter_read(0)==1);
   while(video_formatter_read(0)==0);
   init_vdma(vs.vmode_hsize,vs.vmode_vsize, 2, 1, (uint32_t)vs.framebuffer);
#else
   memset(vs.framebuffer,0,vs.size);
#endif
}
void reset_init(void)
{
   audio_silence();
   set_fb((uint32_t*) (((uint32_t) vs.framebuffer) + (uint32_t) vs.framebuffer_pan_offset), vs.vmode_hsize/vs.vmode_hdiv);
   if(vs.colormode==MNTVA_COLOR_8BIT)
   {
      set_palette((0<<24)|0,OP_PALETTE);
      for(int i=0;i<8;i++)
         set_palette(((i+2)<<24)|color32[i],OP_PALETTE);
      set_palette((1<<24)|0xFFFFFF,OP_PALETTE);
   }
   sprite_request_hide=1;
   vs.split_request_pos=0;
   vs.framebuffer_pan_offset=0;
}
void min_distance(Point o,TriPoint d,Color color)
{
   int32_t dt0,dt1,dt2;
   dt0=(o.x-d.P0.x)*(o.x-d.P0.x)+(o.y-d.P0.y)*(o.y-d.P0.y);
   dt1=(o.x-d.P1.x)*(o.x-d.P1.x)+(o.y-d.P1.y)*(o.y-d.P1.y);
   dt2=(o.x-d.P2.x)*(o.x-d.P2.x)+(o.y-d.P2.y)*(o.y-d.P2.y);
   if(dt0<dt1)
   {
      if(dt0<dt2)
      {
         drawline(o, d.P0,color);
      }
      else
      {
         drawline(o, d.P2,color);
      }
   }
   else
   {
      if(dt1<dt2)
      {
         drawline(o, d.P1,color);
      }
      else
      {
         drawline(o, d.P2,color);
      }
   }
}
int sound_active=0;
int buff_offset=0;
uint8_t *ENCODED=(uint8_t *)0x06000000;
uint8_t *DECODED=(uint8_t *)0x07000000;
void play_init(int bm)
{
   sound_active=0;
   buff_offset=0;
   static FIL fil;      /* File object */
   static FATFS fatfs;
   int chunk_size=1920;
   TCHAR *Path = DEFAULT_ROOT;
   f_mount(&fatfs, Path, 1); // 1 mount immediately
   char Filenames[4][100]={DEFAULT_ROOT "sound/spa/1_060_CPU_selected.mp3",
                           DEFAULT_ROOT "sound/spa/2_musashi_emulator_selected.mp3",
                           DEFAULT_ROOT "sound/spa/3_UAE_emulator_selected.mp3",
                           DEFAULT_ROOT "sound/spa/4_JIT_emulator_selected.mp3"};
   int ret=f_open(&fil, Filenames[bm], FA_OPEN_EXISTING | FA_READ);
   if(ret!=0)
 	  printf("Audio File open failed!!!!\n");
   unsigned int NumBytesRead;
   int filesize=f_size(&fil);
   f_rewind(&fil);
   f_read(&fil, ENCODED + RTG_BASE, filesize, &NumBytesRead);

   Xil_L1DCacheFlush();
   Xil_L2CacheFlush();
   f_close(&fil);
   f_mount(NULL, Path, 1); // NULL unmount, 0 delayed
   write_rtg_register(REG_ZZ_AUDIO_PARAM, 0);
   write_rtg_register(REG_ZZ_AUDIO_VAL, (uint32_t)DECODED);

   audio_debug_timer(0);
   audio_init_i2s();
   audio_debug_timer(1);

   write_rtg_register(REG_ZZ_AUDIO_PARAM, 9);
   write_rtg_register(REG_ZZ_AUDIO_VAL, 20000);
   write_rtg_register(REG_ZZ_DECODER_PARAM, 0);
   write_rtg_register(REG_ZZ_DECODER_VAL, (uint32_t)ENCODED);
   write_rtg_register(REG_ZZ_DECODER_PARAM, 1);
   write_rtg_register(REG_ZZ_DECODER_VAL, filesize);
   write_rtg_register(REG_ZZ_DECODER_PARAM, 2);
   write_rtg_register(REG_ZZ_DECODER_VAL, (uint32_t)DECODED);
   write_rtg_register(REG_ZZ_DECODER_PARAM, 3);
   write_rtg_register(REG_ZZ_DECODER_VAL, chunk_size);

   write_rtg_register(REG_ZZ_DECODE, 0);
   buff_offset=0;
   write_rtg_register(REG_ZZ_AUDIO_CONFIG, 1);
}
void play_sound(void)
{
   if(sound_active==0)
   {
      if(read_rtg_register(REG_ZZ_INT_STATUS)&2)
      {
         write_rtg_register(REG_ZZ_CONFIG, 8 | 32);
         sound_active=1;
      }
   }
   else if(sound_active==1)
   {
      write_rtg_register(REG_ZZ_AUDIO_SCALE, 3840/4);
      write_rtg_register(REG_ZZ_DECODER_PARAM, 2);
      write_rtg_register(REG_ZZ_DECODER_VAL, ((uint32_t)DECODED)+buff_offset);
      write_rtg_register(REG_ZZ_DECODER_PARAM, 0);
      write_rtg_register(REG_ZZ_DECODE, 1);

      write_rtg_register(REG_ZZ_AUDIO_SWAB, (1<<15) | (buff_offset>>8)); // no byteswap, offset/256
      int overrun=read_rtg_register(REG_ZZ_AUDIO_SWAB);

      if(overrun==1)
         buff_offset=0;
      else
      {
         buff_offset+=3840;
         if(buff_offset >= 3840*8)
            buff_offset=0;
      }
      if(!read_rtg_register(REG_ZZ_DECODE))
         sound_active=2;
      else
         sound_active=0;
   }
   else if(sound_active==2)
   {
      write_rtg_register(REG_ZZ_AUDIO_CONFIG, 0);
      sound_active=3;
   }
}
int no_init=0;
void reset_run(int cpu_boot_mode, int counter, int counter_max,int long_reset)
{
   static int16_t iteration=0;
   iteration++;
   int h=vs.vmode_vsize;

//   static int i=0,j=0,k=1;
//   int bar_w=10;
   int w=vs.vmode_hsize;
   if(vs.scalemode)
   {
      w=w>>1;
      h=h>>1;
      if(original_h<256)
         h=original_h;
   }
/*
   int jmax=80*4-20;
   int imax=80*4;
   if(original_h<=256)
   {
      bar_w=5;
      jmax=80*2-20;
      imax=80*2;
   }
   i+=bar_w;
   if(i>=imax)
   {
      i=0;
      while(video_formatter_read(0)==1); //wait vblank
      while(video_formatter_read(0)==0);
      j+=k;
      if(j>=jmax)
         k=-1;
      if(j<=0)
         k=1;
   }
   int delta=40+i+j;
   if(original_h<=256)
   {
      delta=15+i+j;
   }
   uint32_t rgb=0;
   if(vs.colormode==MNTVA_COLOR_32BIT)
      rgb=color32[color_reset&7];
   else if(vs.colormode==MNTVA_COLOR_16BIT565)
      rgb=swap16(color16[color_reset&7]);
   else if(vs.colormode==MNTVA_COLOR_8BIT)
      rgb=((color_reset&7)+2)<<24;
   if(delta<=h/2-bar_w)
   {
      fill_rect_solid(        delta,         delta, w-2*delta,           bar_w, rgb, vs.colormode);
      fill_rect_solid(        delta, h-delta-bar_w, w-2*delta,           bar_w, rgb, vs.colormode);
      fill_rect_solid(        delta,         delta,     bar_w, h-2*delta-bar_w, rgb, vs.colormode);
      fill_rect_solid(w-delta-bar_w,         delta,     bar_w, h-2*delta-bar_w, rgb, vs.colormode);
   }
   color_reset++;
*/
/*
   Point P0,P1;
   Color color;
   P0.x=400;
   P0.y=20;
   P1.x=200;
   P1.y=200;
   color.argb=0x0000FF00;
   drawline(P0,P1, color);
*/
#define scalex(X) ((((X)*vs.vmode_hsize)>>(vs.scalemode?1:0))/800)
#define scaley(Y) ((((Y)*vs.vmode_vsize)>>(vs.scalemode?1:0))/600)
   Color grey;
   grey.argb=0x003F3F3F;
   Triangle rT,gT,bT;
   {
      // Blue Triangle
      bT.P.P0.x=scalex(300);
      bT.P.P0.y=scaley(350);
      bT.P.P1.x=scalex(400);
      bT.P.P1.y=scaley(200);
      bT.P.P2.x=scalex(200);
      bT.P.P2.y=scaley(300);
      Point origin={scalex(400),scaley(300)};
      Point origin2={scalex(300),scaley(275)};
      bT.color.argb=0x000000FF;
      static uint8_t dangle=0,dangle2=0;
      dangle-=2;
      dangle2+=2;
      rotate(origin2,&bT.P.P0,dangle2);
      rotate(origin2,&bT.P.P1,dangle2);
      rotate(origin2,&bT.P.P2,dangle2);
      rotate(origin,&bT.P.P0,dangle);
      rotate(origin,&bT.P.P1,dangle);
      rotate(origin,&bT.P.P2,dangle);
      bT.H.h0=255;
      bT.H.h1=128;
      bT.H.h2=64;
//      drawFilledTriangle(P,T.color.argb);
       if((iteration&0x100)==0x100)
         drawShadedTriangle(bT);
      else
         drawWireframeTriangle(bT.P,grey);//bT.color);
   }
   {
      // Green Triangle
      gT.P.P0.x=scalex(250);
      gT.P.P0.y=scaley(250);
      gT.P.P1.x=scalex(400);
      gT.P.P1.y=scaley(200);
      gT.P.P2.x=scalex(300);
      gT.P.P2.y=scaley(320);
      Point origin={scalex(400),scaley(300)};
      Point origin2={scalex(325),scaley(260)};
      gT.color.argb=0x0000FF00;
      static uint8_t dangle=0,dangle2=0;
      dangle+=2;
      dangle2+=1;
      rotate(origin2,&gT.P.P0,dangle2);
      rotate(origin2,&gT.P.P1,dangle2);
      rotate(origin2,&gT.P.P2,dangle2);
      rotate(origin,&gT.P.P0,dangle);
      rotate(origin,&gT.P.P1,dangle);
      rotate(origin,&gT.P.P2,dangle);
      gT.H.h0=64;
      gT.H.h1=128;
      gT.H.h2=255;
//      drawFilledTriangle(P,T.color.argb);
      if((iteration&0x100)==0x100)
         drawShadedTriangle(gT);
      else
         drawWireframeTriangle(gT.P,grey);//gT.color);
   }
   {
      // Red Triangle
      rT.P.P0.x=scalex(200);
      rT.P.P0.y=scaley(200);
      rT.P.P1.x=scalex(350);
      rT.P.P1.y=scaley(220);
      rT.P.P2.x=scalex(400);
      rT.P.P2.y=scaley(350);
      Point origin={scalex(400),scaley(300)};
      rT.color.argb=0x00FF0000;
      static uint8_t dangle=0;
      dangle++;
      rotate(origin,&rT.P.P0,dangle);
      rotate(origin,&rT.P.P1,dangle);
      rotate(origin,&rT.P.P2,dangle);
      rT.H.h0=255;
      rT.H.h1=128;
      rT.H.h2=64;
//      drawFilledTriangle(P,T.color.argb);
      if((iteration&0x100)==0x100)
         drawShadedTriangle(rT);
      else
         drawWireframeTriangle(rT.P,grey);//rT.color);
   }
   min_distance(rT.P.P0,gT.P,grey);
   min_distance(rT.P.P1,gT.P,grey);
   min_distance(rT.P.P2,gT.P,grey);
   min_distance(rT.P.P0,bT.P,grey);
   min_distance(rT.P.P1,bT.P,grey);
   min_distance(rT.P.P2,bT.P,grey);
   min_distance(gT.P.P0,bT.P,grey);
   min_distance(gT.P.P1,bT.P,grey);
   min_distance(gT.P.P2,bT.P,grey);
   min_distance(gT.P.P0,rT.P,grey);
   min_distance(gT.P.P1,rT.P,grey);
   min_distance(gT.P.P2,rT.P,grey);
   min_distance(bT.P.P0,rT.P,grey);
   min_distance(bT.P.P1,rT.P,grey);
   min_distance(bT.P.P2,rT.P,grey);
   min_distance(bT.P.P0,gT.P,grey);
   min_distance(bT.P.P1,gT.P,grey);
   min_distance(bT.P.P2,gT.P,grey);

   Font20.BackColor=CL_TRANSPARENT;
   displayStringAt(&Font20,0,h/2-12,(uint8_t*)"Z3660 reset...",CENTER_MODE);
   char message[50]={0};
   message[0]=0;
   Font20.TextColor=0x00FF2020; // red
   strcat(message,"Selected >> ");
   strcat(message,bootmode_names[cpu_boot_mode]);
   strcat(message," <<<<<<<<<<<");
   displayStringAt(&Font20,0,h/2+9,(uint8_t*)message,CENTER_MODE);
   Point P0;
   Point P1;
   Color white;
   white.argb=0x00FFFFFF;

    P0.x=40-2;
   P1.x=w-40+2;
   P0.y=h/2+30-2;
   P1.y=h/2+30-2;
   drawline(P0,P1,white);

   P0.x=40-2;
   P1.x=w-40+2;
   P0.y=h/2+30+11;
   P1.y=h/2+30+11;
   drawline(P0,P1,white);

   P0.x=40-2;
   P1.x=40-2;
   P0.y=h/2+30-2;
   P1.y=h/2+30+11;
   drawline(P0,P1,white);

   P0.x=w-40+2;
   P1.x=w-40+2;
   P0.y=h/2+30-2;
   P1.y=h/2+30+11;
   drawline(P0,P1,white);

   for(int i=0;i<10;i++)
   {
      P0.x=40;
      P1.x=40+counter*(w-40-40)/counter_max;
      P0.y=h/2+30+i;
      P1.y=h/2+30+i;
      drawline(P0,P1,white);
   }
   int bm=cpu_boot_mode+1;
   if(bm>=BOOTMODE_NUM) bm=0;
   message[0]=0;
   Font20.TextColor=0x00FFFFFF; // white
   if(long_reset==0)
   {
      strcat(message,"Will change to ");
      strcat(message,bootmode_names[bm]);
   }
   else
   {
      strcat(message,"Will delete all env files");
   }
   strcat(message," after full bar");
   displayStringAt(&Font20,0,h/2+42,(uint8_t*)message,CENTER_MODE);

   if(no_init)
   {
      play_init(cpu_boot_mode);
      no_init=0;
   }

//   usleep(10000);
   handle_cache_flush(((uint32_t)vs.framebuffer) + vs.framebuffer_pan_offset,vs.framebuffer_size);
   while(video_formatter_read(0)==1) //wait vblank
   {
      play_sound();
   }
   while(video_formatter_read(0)==0)
   {
      play_sound();
   }
#if 0
   Color black;
   black.argb=0;
   drawWireframeTriangle(P,black);
   drawFilledTriangle(P,black);
#else
   memset(vs.framebuffer,0,vs.framebuffer_size);
#endif
}

extern XAxiVdma_DmaSetup ReadCfg;
extern XAxiVdma_Config *Config;

// 32bit: hdiv=1, 16bit: hdiv=2, 8bit: hdiv=4, ...
int init_vdma(int hsize, int vsize, int hdiv, int vdiv, uint32_t bufpos) {

   int status;

   if(Config==NULL)
   {
      Config = XAxiVdma_LookupConfig(VDMA_DEVICE_ID);

      if (!Config) {
         printf("VDMA not found for ID %d\r\n", VDMA_DEVICE_ID);
         return(XST_FAILURE);
      }
   }
      status = XAxiVdma_CfgInitialize(&vdma, Config, Config->BaseAddress);
      if (status != XST_SUCCESS) {
         printf("VDMA Configuration Initialization failed, status: %d\r\n", status);
         printf("Halted\n");
         while(1);
         return(status);
      }
   //printf("VDMA MM2S DRE: %d\n", vdma.HasMm2SDRE);
   //printf("VDMA Config MM2S DRE: %d\n", Config->HasMm2SDRE);

   uint32_t stride = hsize * (Config->Mm2SStreamWidth >> 3);
   if (vs.framebuffer_pan_width != 0 && vs.framebuffer_pan_width != (hsize / hdiv)) {
      stride = (vs.framebuffer_pan_width * (Config->Mm2SStreamWidth >> 3)) * stride_div;
   }

   //printf("VDMA HDIV: %d VDIV: %d\n", hdiv, vdiv);

   ReadCfg.VertSizeInput = vsize / vdiv;
   ReadCfg.HoriSizeInput = (hsize * (Config->Mm2SStreamWidth >> 3)) / hdiv; // note: changing this breaks the output
   ReadCfg.Stride = stride / hdiv; // note: changing this is not a problem
   ReadCfg.FrameDelay = 0; /* This example does not test frame delay */
   ReadCfg.EnableCircularBuf = 1; /* Only 1 buffer, continuous loop */
   ReadCfg.EnableSync = 0; /* Gen-Lock */
   ReadCfg.PointNum = 0;
   ReadCfg.EnableFrameCounter = 0; /* Endless transfers */
   ReadCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */

   ReadCfg.FrameStoreStartAddr[0] = bufpos;

   //printf("VDMA Framebuffer at 0x%x\n", ReadCfg.FrameStoreStartAddr[0]);

   status = XAxiVdma_DmaConfig(&vdma, XAXIVDMA_READ, &ReadCfg);
   if (status != XST_SUCCESS) {
      printf("VDMA Read channel config failed, status: %d\r\n", status);
      return(status);
   }

   status = XAxiVdma_DmaSetBufferAddr(&vdma, XAXIVDMA_READ, ReadCfg.FrameStoreStartAddr);
   if (status != XST_SUCCESS) {
      printf("VDMA Read channel set buffer address failed, status: 0x%X\r\n", status);
      return(status);
   }

   status = XAxiVdma_DmaStart(&vdma, XAXIVDMA_READ);
   if (status != XST_SUCCESS) {
      printf("VDMA Failed to start DMA engine (read channel), status: 0x%X\r\n", status);
      return(status);
   }
   return(XST_SUCCESS);
}
int init_vdma_irq(int hsize, int vsize, int hdiv, int vdiv, uint32_t bufpos) {

   int status;

   if(Config==NULL)
   {
      Config = XAxiVdma_LookupConfig(VDMA_DEVICE_ID);

      if (!Config) {
         printf("VDMA not found for ID %d\r\n", VDMA_DEVICE_ID);
         return(XST_FAILURE);
      }
   }
      status = XAxiVdma_CfgInitialize(&vdma, Config, Config->BaseAddress);
      if (status != XST_SUCCESS) {
         printf("VDMA Configuration Initialization failed, status: %d\r\n", status);
         printf("Halted\n");
         while(1);
         return(status);
      }
   uint32_t stride = hsize * (Config->Mm2SStreamWidth >> 3);
   if (vs.framebuffer_pan_width != 0 && vs.framebuffer_pan_width != (hsize / hdiv)) {
      stride = (vs.framebuffer_pan_width * (Config->Mm2SStreamWidth >> 3)) * stride_div;
   }

   ReadCfg.VertSizeInput = vsize / vdiv;
   ReadCfg.HoriSizeInput = (hsize * (Config->Mm2SStreamWidth >> 3)) / hdiv; // note: changing this breaks the output
   ReadCfg.Stride = stride / hdiv; // note: changing this is not a problem
   ReadCfg.FrameDelay = 0; /* This example does not test frame delay */
   ReadCfg.EnableCircularBuf = 1; /* Only 1 buffer, continuous loop */
   ReadCfg.EnableSync = 0; /* Gen-Lock */
   ReadCfg.PointNum = 0;
   ReadCfg.EnableFrameCounter = 0; /* Endless transfers */
   ReadCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */

   ReadCfg.FrameStoreStartAddr[0] = bufpos;

   //printf("VDMA Framebuffer at 0x%x\n", ReadCfg.FrameStoreStartAddr[0]);

   status = XAxiVdma_DmaConfig(&vdma, XAXIVDMA_READ, &ReadCfg);
   if (status != XST_SUCCESS) {
      printf("VDMA Read channel config failed, status: %d\r\n", status);
      return(status);
   }

   status = XAxiVdma_DmaSetBufferAddr(&vdma, XAXIVDMA_READ, ReadCfg.FrameStoreStartAddr);
   if (status != XST_SUCCESS) {
      printf("VDMA Read channel set buffer address failed, status: 0x%X\r\n", status);
      return(status);
   }

   status = XAxiVdma_DmaStart(&vdma, XAXIVDMA_READ);
   if (status != XST_SUCCESS) {
      printf("VDMA Failed to start DMA engine (read channel), status: 0x%X\r\n", status);
      return(status);
   }
   return(XST_SUCCESS);
}

uint32_t ticks=0;
void isr_video(void *dummy)
{
   int vblank=video_formatter_read(0);
/*
   static int c=0;
   static int s=0;
   if(vblank)
   {
	   c++;
	   if(c==60)
	   {
		   c=0;
		   s++;
		   printf("[Core 0] vb %d %08lX\n",s,*((volatile uint32_t*)0xF8F0183C));
	   }
   }
*/
   if (!vblank) {
      // if this is not the vblank interrupt, set up the split buffer
      // TODO: VDMA doesn't seem to like switching buffers in the middle of a frame.
      // the first line after a switch contains an extraneous word, so we end up
      // with up to 4 pixels of the other buffer in the first line
      if (vs.split_pos != 0)
      {
         if (vs.card_feature_enabled[CARD_FEATURE_SECONDARY_PALETTE]) {
            video_formatter_write(1, MNTVF_OP_PALETTE_SEL);
         }
         init_vdma_irq(vs.vmode_hsize, vs.vmode_vsize, vs.vmode_hdiv, vs.vmode_vdiv, (uint32_t)vs.framebuffer + vs.bgbuf_offset);
      }
   } else {
      static int minitick=0;
      minitick++;
      ticks+=16; // 60Hz tick aprox.
      if(minitick>=3)
      {
         minitick=0;
         ticks+=2;
      }
      // if this is the vblank interrupt, set up the "normal" buffer in split mode
      if (vs.card_feature_enabled[CARD_FEATURE_SECONDARY_PALETTE]) {
         video_formatter_write(0, MNTVF_OP_PALETTE_SEL);
      }
      init_vdma_irq(vs.vmode_hsize, vs.vmode_vsize, vs.vmode_hdiv, vs.vmode_vdiv, ((uint32_t)vs.framebuffer) + vs.framebuffer_pan_offset);

   }

   if(vblank)
   {
      if(config.boot_mode==CPU) {
         handle_cache_flush(((uint32_t)vs.framebuffer) + vs.framebuffer_pan_offset,vs.framebuffer_size);
      }
      else
      {
#ifndef NO_L1_CACHE_FLUSH
#ifdef L1_CACHE_ENABLED
    	  Xil_L1DCacheFlush();
#endif
#endif
      }
      if (sprite_request_show) {
         vs.sprite_showing = 1;
         sprite_request_show = 0;
      }

      if (sprite_request_update_data) {
         do_clip_hw_sprite(0, 0);
         sprite_request_update_data = 0;
      }

      if (sprite_request_update_pos) {
         do_update_hw_sprite_pos(sprite_request_pos_x, sprite_request_pos_y);
         video_formatter_write((vs.sprite_y_adj << 16) | vs.sprite_x_adj, MNTVF_OP_SPRITE_XY);
         sprite_request_update_pos = 0;
      }

      if (sprite_request_hide) {
         vs.sprite_x = 2000;
         vs.sprite_y = 2000;
         video_formatter_write((vs.sprite_y << 16) | vs.sprite_x, MNTVF_OP_SPRITE_XY);
         sprite_request_hide = 0;
         vs.sprite_showing = 0;
      }

      // handle screen dragging
      if (vs.split_request_pos != vs.split_pos) {
         int scale = 1;
         if(vs.scalemode & 2) scale = 2;
         vs.split_pos = vs.split_request_pos * scale;
         video_formatter_write(vs.split_pos, MNTVF_OP_REPORT_LINE);
      }
   }
}

uint32_t dump_vdma_status(XAxiVdma *InstancePtr) {
   uint32_t status = XAxiVdma_GetStatus(InstancePtr, XAXIVDMA_READ);

   printf("Read channel dump\n\r");
   printf("\tMM2S DMA Control Register: 0x%08lx\r\n",
         XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase,
               XAXIVDMA_CR_OFFSET));
   printf("\tMM2S DMA Status Register: 0x%08lx\r\n",
         XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase,
               XAXIVDMA_SR_OFFSET));
   printf("\tMM2S HI_FRMBUF Reg: 0x%08lx\r\n",
         XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase,
               XAXIVDMA_HI_FRMBUF_OFFSET));
   printf("\tFRMSTORE Reg: %ld\r\n",
         XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase,
               XAXIVDMA_FRMSTORE_OFFSET));
   printf("\tBUFTHRES Reg: %ld\r\n",
         XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase,
               XAXIVDMA_BUFTHRES_OFFSET));
   printf("\tMM2S Vertical Size Register: %ld\r\n",
         XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase,
               XAXIVDMA_MM2S_ADDR_OFFSET + XAXIVDMA_VSIZE_OFFSET));
   printf("\tMM2S Horizontal Size Register: %ld\r\n",
         XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase,
               XAXIVDMA_MM2S_ADDR_OFFSET + XAXIVDMA_HSIZE_OFFSET));
   printf("\tMM2S Frame Delay and Stride Register: %ld\r\n",
         XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase,
               XAXIVDMA_MM2S_ADDR_OFFSET + XAXIVDMA_STRD_FRMDLY_OFFSET));
   printf("\tMM2S Start Address 1: 0x%08lx\r\n",
         XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase,
               XAXIVDMA_MM2S_ADDR_OFFSET + XAXIVDMA_START_ADDR_OFFSET));

   printf("VDMA status: ");
   if (status & XAXIVDMA_SR_HALTED_MASK)
      printf("halted\n");
   else
      printf("running\n");
   if (status & XAXIVDMA_SR_IDLE_MASK)
      printf("idle\n");
   if (status & XAXIVDMA_SR_ERR_INTERNAL_MASK)
      printf("internal err\n");
   if (status & XAXIVDMA_SR_ERR_SLAVE_MASK)
      printf("slave err\n");
   if (status & XAXIVDMA_SR_ERR_DECODE_MASK)
      printf("decode err\n");
   if (status & XAXIVDMA_SR_ERR_FSZ_LESS_MASK)
      printf("FSize Less Mismatch err\n");
   if (status & XAXIVDMA_SR_ERR_LSZ_LESS_MASK)
      printf("LSize Less Mismatch err\n");
   if (status & XAXIVDMA_SR_ERR_SG_SLV_MASK)
      printf("SG slave err\n");
   if (status & XAXIVDMA_SR_ERR_SG_DEC_MASK)
      printf("SG decode err\n");
   if (status & XAXIVDMA_SR_ERR_FSZ_MORE_MASK)
      printf("FSize More Mismatch err\n");

   return(status);
}
/*
void stubErrCallBack(void *CallBackRef, uint32_t ErrorMask)
{
   while(1);
}
uint32_t xClk_Wiz_CfgInitialize(XClk_Wiz *InstancePtr, XClk_Wiz_Config *CfgPtr,
            UINTPTR EffectiveAddr)
{
   InstancePtr->Config = *CfgPtr;

   InstancePtr->Config.BaseAddr = EffectiveAddr;

   // Set all handlers to stub values, let user configure this data later
   InstancePtr->ClkOutOfRangeCallBack  = stubErrCallBack;
   InstancePtr->ClkGlitchCallBack      = stubErrCallBack;
   InstancePtr->ClkStopCallBack        = stubErrCallBack;

   InstancePtr->ErrorCallBack = stubErrCallBack;

   InstancePtr->IsReady = (uint32_t)(XIL_COMPONENT_IS_READY);

   return(XST_SUCCESS);

}
*/
void set_pixelclock(zz_video_mode *mode) {
   XClk_Wiz_CfgInitialize(&clkwiz, &conf, XPAR_CLK_WIZ_1_BASEADDR);

   uint32_t mul = mode->mul;
   uint32_t div = mode->div<<1;
   uint32_t div2 = mode->div2;

   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x200,(0<<16) | (mul<<8) | div); // 50 * 200MHz  / 10 -> 1000MHz VCO
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x208,(0<<8) | div2)  ;          // 1000MHz / 7 = 142,9MHz

   // load configuration
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x25C,0x00000003);
}

void video_formatter_init(int scalemode, int colormode, int width, int height,
      int htotal, int vtotal, int hss, int hse, int vss, int vse,
      int polarity) {
   uint32_t height_crop=height+1;
   if(original_h<256) height_crop=(original_h+1)<<1;
   Xil_ExceptionDisable();
   video_formatter_write((vtotal << 16) | htotal, MNTVF_OP_MAX);
   video_formatter_write((height << 16) | width, MNTVF_OP_DIMENSIONS);
   video_formatter_write((hss << 16) | hse, MNTVF_OP_HS);
   video_formatter_write((vss << 16) | vse, MNTVF_OP_VS);
   video_formatter_write(polarity, MNTVF_OP_POLARITY);
   video_formatter_write((height_crop << 16) |scalemode, MNTVF_OP_SCALE);
   video_formatter_write(colormode, MNTVF_OP_COLORMODE);
   Xil_ExceptionEnable();

//   video_formatter_valign();
}
void video_system_init(zz_video_mode *mode, int hdiv, int vdiv) {

   //printf("video_system_init(%d,%d)\n",vmode->hres,vmode->vres);

   set_pixelclock(mode);
   sii9022_init(mode);
   init_vdma(mode->hres, mode->vres, hdiv, vdiv, (uint32_t)vs.framebuffer + vs.framebuffer_pan_offset);
}

void video_mode_init(int mode, int scalemode, int colormode) {
   printf("video_mode_init: %d color: %d scale: %d\n", mode, colormode, scalemode);

   // remenber mode
   vs.video_mode = mode;
   vs.scalemode = scalemode;
   vs.colormode = colormode;

   int hdiv = 1, vdiv = 1;
   stride_div = 1;
   uint32_t size;

   if (scalemode & 1) {
      hdiv = 2;
      stride_div = 2;
   }
   if (scalemode & 2)
      vdiv = 2;

   // 8 bit
   if (colormode == MNTVA_COLOR_8BIT)
   {
      size=1;
      hdiv *= 4;
   }
   else if (colormode == MNTVA_COLOR_16BIT565 || colormode == MNTVA_COLOR_15BIT)
   {
      size=2;
      hdiv *= 2;
   }
   else
   {
      size=4;
   }

   zz_video_mode *vmode = &preset_video_modes[mode];

   video_system_init(vmode, hdiv, vdiv);

   video_formatter_init(scalemode, colormode,
         vmode->hres, vmode->vres,
         vmode->hmax, vmode->vmax,
         vmode->hstart, vmode->hend,
         vmode->vstart, vmode->vend,
         vmode->polarity);

   // FIXME ???
   vs.vmode_hsize = vmode->hres;
   vs.vmode_vsize = vmode->vres;
   vs.vmode_vdiv = vdiv;
   vs.vmode_hdiv = hdiv;
   vs.framebuffer_size=vmode->hres*vmode->vres*size;
   vs.split_pos = 1; // force update slpit_pos from split_request_pos and write to videoformatter
}

void update_hw_sprite(uint8_t *data, int double_sprite)
{
   uint8_t cur_bit = 0x80;
   uint8_t cur_color = 0, out_pos = 0, iter_offset = 0;
   uint8_t cur_bytes[16]={0};
   uint32_t *colors = vs.sprite_colors;
   uint16_t w = vs.sprite_width;
   uint16_t h = vs.sprite_height;
   uint8_t line_pitch = (w / 8) * 2;

   for (uint8_t y_line = 0; y_line < h; y_line++) {
      if (w <= 16) {
         cur_bytes[0] = data[y_line * line_pitch];
         cur_bytes[1] = data[(y_line * line_pitch) + 2];
         cur_bytes[2] = data[(y_line * line_pitch) + 1];
         cur_bytes[3] = data[(y_line * line_pitch) + 3];
      }
      else {
         cur_bytes[0] = data[y_line * line_pitch];
         cur_bytes[1] = data[(y_line * line_pitch) + 4];
         cur_bytes[2] = data[(y_line * line_pitch) + 1];
         cur_bytes[3] = data[(y_line * line_pitch) + 5];
         cur_bytes[4] = data[(y_line * line_pitch) + 2];
         cur_bytes[5] = data[(y_line * line_pitch) + 6];
         cur_bytes[6] = data[(y_line * line_pitch) + 3];
         cur_bytes[7] = data[(y_line * line_pitch) + 7];
      }

      while (out_pos < 8) {
         for (uint8_t i = 0; i < line_pitch; i += 2) {
            cur_color = (cur_bytes[i] & cur_bit) ? 1 : 0;
            if (cur_bytes[i + 1] & cur_bit) cur_color += 2;

            sprite_buf[(y_line * 32) + out_pos + iter_offset] = colors[cur_color] & 0x00ffffff;
            iter_offset += 8;
         }

         out_pos++;
         cur_bit >>= 1;
         iter_offset = 0;
      }
      cur_bit = 0x80;
      out_pos = 0;
   }

   sprite_request_update_data = 1;
}

void update_hw_sprite_clut(uint8_t *data_, uint8_t *colors, uint16_t w, uint16_t h, uint8_t keycolor, int double_sprite)
{
   uint8_t *data = data_;
   uint8_t color[4];

   for (int y = 0; y < h && y < 48; y++) {
      for (int x = 0; x < w && x < 32; x++) {
         if (data[x] == keycolor) {
            *((uint32_t *)color) = 0x00ff00ff;
         }
         else {
            color[0] = colors[(data[x] * 3)+2];
            color[1] = colors[(data[x] * 3)+1];
            color[2] = colors[(data[x] * 3)];
            color[3] = 0x00;
            if (*((uint32_t *)color) == 0x00FF00FF)
               *((uint32_t *)color) = 0x00FE00FE;
         }
         sprite_buf[(y * 32) + x] = *((uint32_t *)color);
      }
      data += w;
   }

   sprite_request_update_data = 1;
}

void clear_hw_sprite()
{
   for (uint16_t i = 0; i < 32 * 48; i++) {
      sprite_buf[i] = 0x00ff00ff;
   }
   //sprite_request_update_data = 1;
}

void do_clip_hw_sprite(int16_t offset_x, int16_t offset_y)
{
   uint16_t xo = 0, yo = 0;
   if (offset_x < 0)
      xo = -offset_x;
   if (offset_y < 0)
      yo = -offset_y;

   for (int y = 0; y < 48; y++) {
      //printf("CLIP %02d: ",y);
      for (int x = 0; x < 32; x++) {
         video_formatter_write((y * 32) + x, 14);
         if (x < 32 - xo && y < 48 - yo) {
            //printf("%06lx", sprite_buf[((y + yo) * 32) + (x + xo)] & 0x00ffffff);
            video_formatter_write(sprite_buf[((y + yo) * 32) + (x + xo)] & 0x00ffffff, 15);
         } else {
            //printf("%06lx", 0x00ff00ff);
            video_formatter_write(0x00ff00ff, 15);
         }
      }
      //printf("\n");
   }
}

void do_update_hw_sprite_pos(int16_t x, int16_t y) {
   vs.sprite_x = x - vs.sprite_x_offset + 1;
   // horizontally doubled mode
   if (vs.scalemode & 1)
      vs.sprite_x_adj = (vs.sprite_x * 2) + 1;
   else
      vs.sprite_x_adj = vs.sprite_x + 2;

//   vs.sprite_y = y + vs.split_pos - vs.sprite_y_offset + 1;
   vs.sprite_y = y - vs.sprite_y_offset + 1;

   // vertically doubled mode
   if (vs.scalemode & 2)
      vs.sprite_y_adj = vs.sprite_y * 2;
   else
      vs.sprite_y_adj = vs.sprite_y;

   if (vs.sprite_x < 0 || vs.sprite_y < 0) {
      if (sprite_clip_x != vs.sprite_x || sprite_clip_y != vs.sprite_y) {
         do_clip_hw_sprite((vs.sprite_x < 0) ? vs.sprite_x : 0, (vs.sprite_y < 0) ? vs.sprite_y : 0);
      }
      sprite_clipped = 1;
      if (vs.sprite_x < 0) {
         vs.sprite_x_adj = 0;
         sprite_clip_x = vs.sprite_x;
      }
      if (vs.sprite_y < 0) {
         vs.sprite_y_adj = 0;
         sprite_clip_y = vs.sprite_y;
      }
   }
   else if (sprite_clipped && vs.sprite_x >= 0 && vs.sprite_y >= 0) {
      do_clip_hw_sprite(0, 0);
      sprite_clipped = 0;
   }
}

void update_hw_sprite_pos() {
   sprite_request_pos_x = vs.sprite_x_base;
   sprite_request_pos_y = vs.sprite_y_base;
   sprite_request_update_pos = 1;
}

void hw_sprite_show(int show) {
   if (show) {
      sprite_request_show = 1;
   } else {
      sprite_request_hide = 1;
   }
}

