#include "config_clk.h"
#include "config_file.h"
#include "main.h"
#include <stdio.h>
#include "version.h"
#include "alfa.txt"

XClk_Wiz clkwiz0;
XClk_Wiz_Config conf0;
int timing_selected=0;

clock_data cd[]={
      {// 50 MHz
            .clk            = 50, .M            =     24, .D                =  3,
            .axi.divider    = 16, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   = 32, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 64, .clken.phase  =     60, .clken.dutycycle  = 50,
            .bclk.divider   = 32, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 64, .cpuclk.phase = 180+60, .cpuclk.dutycycle = 50,
            .clk90.divider  = 64, .clk90.phase  = 270+60, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {// 55 MHz
            .clk            = 55, .M            =     22, .D                =  5,
            .axi.divider    =  8, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   = 16, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 32, .clken.phase  =     60, .clken.dutycycle  = 50,
            .bclk.divider   = 16, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 64, .cpuclk.phase = 180+60, .cpuclk.dutycycle = 50,
            .clk90.divider  = 64, .clk90.phase  = 270+60, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {// 60 MHz
            .clk            = 60, .M            =     24, .D                =  5,
            .axi.divider    =  8, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   = 16, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 32, .clken.phase  =     60, .clken.dutycycle  = 50,
            .bclk.divider   = 16, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 64, .cpuclk.phase = 180+60, .cpuclk.dutycycle = 50,
            .clk90.divider  = 64, .clk90.phase  = 270+60, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {// 65 MHz
            .clk            = 65, .M            =     26, .D                =  5,
            .axi.divider    =  8, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   = 16, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 32, .clken.phase  =     60, .clken.dutycycle  = 50,
            .bclk.divider   = 16, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 64, .cpuclk.phase = 180+60, .cpuclk.dutycycle = 50,
            .clk90.divider  = 64, .clk90.phase  = 270+60, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {// 70 MHz
            .clk            = 70, .M            =     28, .D                =  5,
            .axi.divider    =  8, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   = 16, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 32, .clken.phase  =     60, .clken.dutycycle  = 50,
            .bclk.divider   = 16, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 64, .cpuclk.phase = 180+60, .cpuclk.dutycycle = 50,
            .clk90.divider  = 64, .clk90.phase  = 270+60, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {// 75 MHz
            .clk            = 75, .M            =     30, .D                =  5,
            .axi.divider    =  8, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   = 16, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 32, .clken.phase  =     60, .clken.dutycycle  = 50,
            .bclk.divider   = 16, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 64, .cpuclk.phase = 180+60, .cpuclk.dutycycle = 50,
            .clk90.divider  = 64, .clk90.phase  = 270+60, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {// 80 MHz
            .clk            = 80, .M            =     24, .D                =  5,
            .axi.divider    =  6, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   = 12, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 24, .clken.phase  =     60, .clken.dutycycle  = 50,
            .bclk.divider   = 12, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 48, .cpuclk.phase = 180+60, .cpuclk.dutycycle = 50,
            .clk90.divider  = 48, .clk90.phase  = 270+60, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {// 85 MHz
            .clk            = 85, .M            =     25, .D                =  5,
            .axi.divider    =  6, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   = 12, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 24, .clken.phase  =     60, .clken.dutycycle  = 50,
            .bclk.divider   = 12, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 48, .cpuclk.phase = 180+60, .cpuclk.dutycycle = 50,
            .clk90.divider  = 48, .clk90.phase  = 270+60, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {// 90 MHz
            .clk            = 90, .M            =     27, .D                =  5,
            .axi.divider    =  6, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   = 12, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 24, .clken.phase  =     60, .clken.dutycycle  = 50,
            .bclk.divider   = 12, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 48, .cpuclk.phase = 180+40, .cpuclk.dutycycle = 50,
            .clk90.divider  = 48, .clk90.phase  = 270+40, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {// 95 MHz
            .clk            = 95, .M            =     28, .D                =  5,
            .axi.divider    =  6, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   = 12, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 24, .clken.phase  =     40, .clken.dutycycle  = 50,
            .bclk.divider   = 12, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 48, .cpuclk.phase = 180+40, .cpuclk.dutycycle = 50,
            .clk90.divider  = 48, .clk90.phase  = 270+40, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {//100 MHz
            .clk            =100, .M            =     30, .D                =  5,
            .axi.divider    =  6, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   = 12, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 24, .clken.phase  =     40, .clken.dutycycle  = 50,
            .bclk.divider   = 12, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 48, .cpuclk.phase = 180+40, .cpuclk.dutycycle = 50,
            .clk90.divider  = 48, .clk90.phase  = 270+40, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {//105 MHz
            .clk            =105, .M            =     21, .D                =  5,
            .axi.divider    =  4, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   =  8, .pclk.phase   =    -20, .pclk.dutycycle   = 50,
            .clken.divider  = 16, .clken.phase  =     60, .clken.dutycycle  = 50,
            .bclk.divider   = 16, .bclk.phase   =    -20, .bclk.dutycycle   = 50,
            .cpuclk.divider = 32, .cpuclk.phase = 180+40, .cpuclk.dutycycle = 50,
            .clk90.divider  = 32, .clk90.phase  = 270+40, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {//110 MHz
            .clk            =110, .M            =     22, .D                =  5,
            .axi.divider    =  4, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   =  8, .pclk.phase   =    -25, .pclk.dutycycle   = 50,
            .clken.divider  = 16, .clken.phase  =    -60, .clken.dutycycle  = 50,
            .bclk.divider   = 16, .bclk.phase   =    -25, .bclk.dutycycle   = 50,
            .cpuclk.divider = 32, .cpuclk.phase =    240, .cpuclk.dutycycle = 50,
            .clk90.divider  = 32, .clk90.phase  =    330, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {//115 MHz
            .clk            =115, .M            =     23, .D                =  5,
            .axi.divider    =  4, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   =  8, .pclk.phase   =    -25, .pclk.dutycycle   = 50,
            .clken.divider  = 16, .clken.phase  =    -60, .clken.dutycycle  = 50,
            .bclk.divider   = 16, .bclk.phase   =    -25, .bclk.dutycycle   = 50,
            .cpuclk.divider = 32, .cpuclk.phase =    240, .cpuclk.dutycycle = 50,
            .clk90.divider  = 32, .clk90.phase  =    330, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
      {//120 MHz
            .clk            =120, .M            =     24, .D                =  5,
            .axi.divider    =  4, .axi.phase    =      0, .axi.dutycycle    = 50,
            .pclk.divider   =  8, .pclk.phase   =    -25, .pclk.dutycycle   = 50,
            .clken.divider  = 16, .clken.phase  =    -60, .clken.dutycycle  = 50,
            .bclk.divider   = 16, .bclk.phase   =    -25, .bclk.dutycycle   = 50,
            .cpuclk.divider = 32, .cpuclk.phase =    240, .cpuclk.dutycycle = 50,
            .clk90.divider  = 32, .clk90.phase  =    330, .clk90.dutycycle  = 50,
            .emu_extra_phase=  0},
};
void trim_whitespace(char *str);
void get_next_string(char *str, char *str_out, int *strpos, char separator);
int get_int_type(char *cmd);
enum TIMING_ITEMS{
   NONE,
   INDEX,
   GLOBAL_MULTIPLIER,
   GLOBAL_DIVIDER,
   PCLK_PHASE,
   CLKEN_PHASE,
   BCLK_PHASE,
   CPUCLK_PHASE,
   CLK90_PHASE,
   PCLK_DIVIDER,
   CLKEN_DIVIDER,
   BCLK_DIVIDER,
   CPUCLK_DIVIDER,
   CLK90_DIVIDER,
   EMU_EXTRA_PHASE,
   TIMING_ITEMS_NUM
};
const char *timing_item_names[TIMING_ITEMS_NUM] = {
      "NONE",
      "index",
      "global.multiplier",
      "global.divider",
      "pclk.phase",
      "clken.phase",
      "bclk.phase",
      "cpuclk.phase",
      "clk90.phase",
      "pclk.divider",
      "clken.divider",
      "bclk.divider",
      "cpuclk.divider",
      "clk90.divider",
      "emu_extra_phase",
};
int get_timing_item_type(char *cmd) {
   for (int i = 0; i < TIMING_ITEMS_NUM; i++) {
      if (strcmp(cmd, timing_item_names[i]) == 0) {
         return i;
      }
   }
   return NONE;
}
void trim_equal(char *str) {
   //trailing white spaces
  while (strlen(str) != 0 && (str[strlen(str) - 1] == '=' || str[strlen(str) - 1] == '\t' || str[strlen(str) - 1] == 0x0A || str[strlen(str) - 1] == 0x0D)) {
    str[strlen(str) - 1] = '\0';
  }
}
int read_timings(void)
{
   char Filename[]=DEFAULT_ROOT "timings/timings.txt";
   char timings_filename[100];
   static FIL fil;      /* File object */
   static FATFS fatfs;

   TCHAR *Path = DEFAULT_ROOT;

   f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
   int ret;
   ret=f_open(&fil,Filename, FA_OPEN_EXISTING | FA_READ);
   if(ret==FR_OK)
   {
      printf("Reading %s file \n",Filename);
      char temp[40];
      memset(timings_filename, 0x00, sizeof(timings_filename));
      memset(temp, 0x00, sizeof(temp));
      f_gets(temp, (s32)sizeof(temp), &fil);
      f_close(&fil);
      sprintf(timings_filename,"%stimings/%s",DEFAULT_ROOT,temp);
      trim_equal(timings_filename);
      ret=f_open(&fil,timings_filename, FA_OPEN_EXISTING | FA_READ);
      if(ret==FR_OK)
      {
//#define debug_printf printf
#define debug_printf(...)
         int cur_line = 1;
         char line[512];
         char cur_cmd[128];
         int index=0;
         printf("Reading %s file \n",timings_filename);
         while(!f_eof(&fil))
         {
            int str_pos = 0;
            memset(line, 0x00, sizeof(line));
            f_gets(line, (s32)sizeof(line), &fil);

            if (strlen(line) <= 2 || line[0] == '#')
               goto skip_line;

            trim_whitespace(line);

            get_next_string(line, cur_cmd, &str_pos, ' ');
            int item=get_timing_item_type(cur_cmd);
            switch(item){
            case INDEX:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               index=get_int_type(cur_cmd);
               debug_printf("index %d\n",index);
               break;
            case GLOBAL_MULTIPLIER:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].M=get_int_type(cur_cmd);
               debug_printf("    global.multiplier   = %d\n",cd[index].M);
               break;
            case GLOBAL_DIVIDER:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].D=get_int_type(cur_cmd);
               debug_printf("    global.divider   = %d\n",cd[index].D);
               break;
            case PCLK_PHASE:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].pclk.phase=get_int_type(cur_cmd);
               debug_printf("    pclk.phase   = %d\n",cd[index].pclk.phase);
               break;
            case CLKEN_PHASE:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].clken.phase=get_int_type(cur_cmd);
               debug_printf("    clken.phase  = %d\n",cd[index].clken.phase);
               break;
            case BCLK_PHASE:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].bclk.phase=get_int_type(cur_cmd);
               debug_printf("    bclk.phase   = %d\n",cd[index].bclk.phase);
               break;
            case CPUCLK_PHASE:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].cpuclk.phase=get_int_type(cur_cmd);
               debug_printf("    cpuclk.phase = %d\n",cd[index].cpuclk.phase);
               break;
            case CLK90_PHASE:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].clk90.phase=get_int_type(cur_cmd);
               debug_printf("    clk90.phase  = %d\n",cd[index].clk90.phase);
               break;
            case PCLK_DIVIDER:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].pclk.divider=get_int_type(cur_cmd);
               cd[index].axi.divider=cd[index].pclk.divider/2;
               debug_printf("    pclk.divider   = %d\n",cd[index].pclk.divider);
               break;
            case CLKEN_DIVIDER:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].clken.divider=get_int_type(cur_cmd);
               debug_printf("    clken.divider  = %d\n",cd[index].clken.divider);
               break;
            case BCLK_DIVIDER:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].bclk.divider=get_int_type(cur_cmd);
               debug_printf("    bclk.divider   = %d\n",cd[index].bclk.divider);
               break;
            case CPUCLK_DIVIDER:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].cpuclk.divider=get_int_type(cur_cmd);
               debug_printf("    cpuclk.divider = %d\n",cd[index].cpuclk.divider);
               break;
            case CLK90_DIVIDER:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].clk90.divider=get_int_type(cur_cmd);
               debug_printf("    clk90.divider  = %d\n",cd[index].clk90.divider);
               break;
            case EMU_EXTRA_PHASE:
               get_next_string(line, cur_cmd, &str_pos, ' ');
               get_next_string(line, cur_cmd, &str_pos, ' ');
               cd[index].emu_extra_phase=get_int_type(cur_cmd);
               debug_printf("    emu_extra_phase = %d\n",cd[index].emu_extra_phase);
               break;
            case NONE:
            default:
               debug_printf("[TIM] Unknown timing setting %s on line %d.\n", cur_cmd, cur_line);
               break;
            }
            skip_line:
            cur_line++;
         }
         f_close(&fil);
         return(1);
      }
   }
   return(0);
}

int write_timings(void)
{
   char Filename[]=DEFAULT_ROOT "timings/timings.txt";
   char timings_filename[100];
   static FIL fil;      /* File object */
   static FATFS fatfs;

   TCHAR *Path = DEFAULT_ROOT;

   f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
   int ret;
   ret=f_open(&fil,Filename, FA_OPEN_EXISTING | FA_READ);
   if(ret==FR_OK)
   {
      printf("Reading %s file \n",Filename);
      char temp[40];
      memset(timings_filename, 0x00, sizeof(timings_filename));
      memset(temp, 0x00, sizeof(temp));
      f_gets(temp, (s32)sizeof(temp), &fil);
      f_close(&fil);
      sprintf(timings_filename,"%stimings/%s",DEFAULT_ROOT,temp);
      trim_equal(timings_filename);
      ret=f_open(&fil,timings_filename, FA_CREATE_ALWAYS | FA_WRITE);
      if(ret==FR_OK)
      {
         printf("Writing %s file \n",timings_filename);
         f_printf(&fil,"# %s\n",timings_filename);
         if(REVISION_BETA > 0)
         {
            if(REVISION_ALFA > 0)
               f_printf(&fil,"# Generated by version %d.%02d BETA %d ALFA %d\n",REVISION_MAJOR,REVISION_MINOR,REVISION_BETA,REVISION_ALFA);
            else
               f_printf(&fil,"# Generated by version %d.%02d BETA %d\n",REVISION_MAJOR,REVISION_MINOR,REVISION_BETA);
         }
         else
            f_printf(&fil,"# Generated by version %d.%02d\n",REVISION_MAJOR,REVISION_MINOR);
         for(unsigned int i=0;i<sizeof(cd)/sizeof(cd[0]);i++)
         {
            f_printf(&fil,"index %d\n",i);
            f_printf(&fil,"# %d MHz\n",cd[i].clk);
            f_printf(&fil,"    global.multiplier = %3d\n", cd[i].M);
            f_printf(&fil,"    global.divider    = %3d\n", cd[i].D);
            f_printf(&fil,"    pclk.phase   = %3d\n", cd[i].pclk.phase);
            f_printf(&fil,"    clken.phase  = %3d\n", cd[i].clken.phase);
            f_printf(&fil,"    bclk.phase   = %3d\n", cd[i].bclk.phase);
            f_printf(&fil,"    cpuclk.phase = %3d\n", cd[i].cpuclk.phase);
            f_printf(&fil,"    clk90.phase  = %3d\n", cd[i].clk90.phase);
            f_printf(&fil,"    pclk.divider   = %3d\n", cd[i].pclk.divider);
            f_printf(&fil,"    clken.divider  = %3d\n", cd[i].clken.divider);
            f_printf(&fil,"    bclk.divider   = %3d\n", cd[i].bclk.divider);
            f_printf(&fil,"    cpuclk.divider = %3d\n", cd[i].cpuclk.divider);
            f_printf(&fil,"    clk90.divider  = %3d\n", cd[i].clk90.divider);
            f_printf(&fil,"    emu_extra_phase = %3d\n", cd[i].emu_extra_phase);
         }
         f_close(&fil);
         return(1);
      }
   }
   return(0);
}

void print_clkinfo(char * str,uint32_t base,uint32_t address)
{
   int32_t temp=XClk_Wiz_ReadReg(base, 0x200);
   float temp2=(float)((temp>>8)&0xff);
   if(((temp>>16)&0xFFFF)>=500)
      temp2+=0.5;
   float divider=temp2*200./(temp&0xff);
   temp=XClk_Wiz_ReadReg(base, address);
   float clk=divider/temp;
   temp=XClk_Wiz_ReadReg(base, address+4);
   float clk_phase=temp/1000.;
   temp=XClk_Wiz_ReadReg(base, address+8);
   float clk_dc=temp/1000.;
   printf("%s %06.2f MHz, PHASE %06.2f degrees, DC %06.2f%c\n",str,clk,clk_phase,clk_dc,'%');

}
uint32_t clken=1;
unsigned int get_clock_index(int clk)
{
   unsigned int ind;

   if(clk>115)      ind=14;
   else if(clk>110) ind=13;
   else if(clk>105) ind=12;
   else if(clk>100) ind=11;
   else if(clk>95)  ind=10;
   else if(clk>90)  ind=9;
   else if(clk>85)  ind=8;
   else if(clk>80)  ind=7;
   else if(clk>75)  ind=6;
   else if(clk>70)  ind=5;
   else if(clk>65)  ind=4;
   else if(clk>60)  ind=3;
   else if(clk>55)  ind=2;
   else if(clk>50)  ind=1;
   else             ind=0;
   return ind;
}
void configure_clk(int clk, int verbose, int emu)
{
#ifndef CPU_FIXED_FREQUENCY
   if(clk>CPUFREQ_MAX)
      clk=CPUFREQ_MAX;
   else if(clk<CPUFREQ_MIN)
      clk=CPUFREQ_MIN;

   // unstable frequencies: mapped to stable ones
   unsigned int ind;
   ind=get_clock_index(clk);
   timing_selected=ind;

   clk=cd[ind].clk;

   if(verbose)
      printf("Clock index %d\n",ind);

   XClk_Wiz_CfgInitialize(&clkwiz0, &conf0, XPAR_CLK_WIZ_0_BASEADDR);
   /*
   uint32_t clkbase_remainder=((cd[ind].clkbase&1)*500L)<<16;
   uint32_t clkbase=(cd[ind].clkbase<<7)&0xFF00; // clkbase is divided by 2, because the result must be under 64 !!!!
    */
   uint32_t clkbase_remainder=0;
   uint32_t M=((uint32_t)cd[ind].M)*256;
   uint32_t D=cd[ind].D;

   // force CLKEN to 0
   //   cd[ind].clken.dutycycle=0;
   int emu_div, extra_phase;

   if(emu)
   {
      emu_div=2;
      extra_phase=cd[ind].emu_extra_phase;
   }
   else
   {
      emu_div=1;
      extra_phase=0;
   }

   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200, (clkbase_remainder)+ M + D); // M=8 D=1 -> 8*200 / 1 -> 1600MHz VCO
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208,(uint32_t)( cd[ind].axi.divider                   )); //   5h= 5 -> 200 MHz AXI
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C,(uint32_t)( cd[ind].axi.phase                *1000)); //                    PHASE 0
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210,(uint32_t)( cd[ind].axi.dutycycle            *1000)); //                    DC 50%
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x214,(uint32_t)( cd[ind].pclk.divider*emu_div          )); //   Ah=10 -> 100 MHz PCLK
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x218,(uint32_t)( cd[ind].pclk.phase               *1000)); //                    PHASE 30
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x21C,(uint32_t)( cd[ind].pclk.dutycycle           *1000)); //                    DC 50%
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x220,(uint32_t)( cd[ind].clken.divider*emu_div         )); //  14h=20 ->  50 MHz CLKEN
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x224,(uint32_t)( cd[ind].clken.phase              *1000)); //                    PHASE 60
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x228,(uint32_t)( cd[ind].clken.dutycycle*clken    *1000)); //                    DC 50%

   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x22C,(uint32_t)( cd[ind].bclk.divider                  )); //  28h=40 ->  25 MHz BCLK
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x230,(uint32_t)( cd[ind].bclk.phase               *1000)); //                    PHASE 40
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x234,(uint32_t)( cd[ind].bclk.dutycycle           *1000)); //                    DC 50%
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x238,(uint32_t)( cd[ind].clk90.divider                 )); //   Ah=10 -> 100 MHz CLK90
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x23C,(uint32_t)((cd[ind].clk90.phase+extra_phase) *1000)); //                    PHASE 290
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x240,(uint32_t)( cd[ind].clk90.dutycycle          *1000)); //                    DC 50%
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x244,(uint32_t)( cd[ind].cpuclk.divider                )); //  14h=20 ->  50 MHz CPUCLK
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x248,(uint32_t)((cd[ind].cpuclk.phase+extra_phase)*1000)); //                    PHASE 200
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x24C,(uint32_t)( cd[ind].cpuclk.dutycycle         *1000)); //                    DC 50%

   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);

   while(XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x004)==0);

#else
   printf("Clk config bypassed ...\n");
   verbose=1;
#endif
   if(verbose)
   {
      print_clkinfo("AXICLK",XPAR_CLK_WIZ_0_BASEADDR,0x208);
      print_clkinfo("  PCLK",XPAR_CLK_WIZ_0_BASEADDR,0x214);
      print_clkinfo("_CLKEN",XPAR_CLK_WIZ_0_BASEADDR,0x220);
      print_clkinfo("  BCLK",XPAR_CLK_WIZ_0_BASEADDR,0x22C);
      print_clkinfo("CPUCLK",XPAR_CLK_WIZ_0_BASEADDR,0x244);
      print_clkinfo(" CLK90",XPAR_CLK_WIZ_0_BASEADDR,0x238);
   }
}
