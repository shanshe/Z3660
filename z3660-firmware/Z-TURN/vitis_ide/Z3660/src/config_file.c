/*
 * config_file.c
 *
 *  Created on: 28 feb. 2023
 *      Author: shanshe
 */
#include <ff.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xil_exception.h"
#include "config_file.h"
#include "sleep.h"
#include "main.h"
CONFIG config;
extern SHARED *shared;
const char *config_item_names[CONFITEM_NUM] = {
      "NONE",
      "bootmode",
      "kickstart",
      "ext_kickstart",
      "scsiboot",
	  "hdf0",
	  "hdf1",
	  "hdf2",
	  "hdf3",
	  "hdf4",
	  "hdf5",
	  "hdf6",
	  "hdf7",
	  "hdf8",
	  "hdf9",
	  "hdf10",
	  "hdf11",
	  "hdf12",
	  "hdf13",
	  "hdf14",
	  "hdf15",
	  "hdf16",
	  "hdf17",
	  "hdf18",
	  "hdf19",
      "scsi0",
      "scsi1",
      "scsi2",
      "scsi3",
      "scsi4",
      "scsi5",
      "scsi6",
      "autoconfig_ram",
      "cpu_ram",
      "resistor",
      "temperature",
	  "cpufreq",
      "kickstart0",
      "kickstart1",
      "kickstart2",
      "kickstart3",
      "kickstart4",
      "kickstart5",
      "kickstart6",
      "kickstart7",
      "kickstart8",
      "kickstart9",
      "ext_kickstart0",
      "ext_kickstart1",
      "ext_kickstart2",
      "ext_kickstart3",
      "ext_kickstart4",
      "ext_kickstart5",
      "ext_kickstart6",
      "ext_kickstart7",
      "ext_kickstart8",
      "ext_kickstart9",
	  "enable_test",
	  "bootscreen_resolution",
};
const char *bootmode_names[BOOTMODE_NUM] = {
      "CPU",
      "MUSASHI",
      "UAE",
      "UAEJIT",
};
const char *yesno_names[YESNO_NUM] = {
      "NO",
      "YES",
};
const char *yesnomin_names[YESNOMIN_NUM] = {
      "NO",
      "YES",
	  "MIN",
};
const char *resolution_names[RES_NUM] = {
      "1920x1080",
      "1280x720",
	  "800x600",
};
void print_line(FIL *fil, char* line_in, ...)
{
   va_list args;
   UINT NumBytesWritten;
   char line[100];
   va_start(args, line_in);
   sprintf(line,line_in, args);
   f_write(fil, line, strlen(line),&NumBytesWritten);
}
void load_default_config(void)
{
   config.boot_mode=UAEJIT;
   for(int i=0;i<20;i++)
      config.hdf[i][0]=0;
   for(int i=0;i<7;i++)
      config.scsi_num[i]=-1;
   config.scsiboot=0;
   config.autoconfig_ram=0;
   config.cpu_ram=1;
   config.resistor=800.0;
   config.temperature=27.0;
   config.cpufreq=50;
   config.kickstart=0;
   config.kickstart0[0]=0;
   config.kickstart1[0]=0;
   config.kickstart2[0]=0;
   config.kickstart3[0]=0;
   config.kickstart4[0]=0;
   config.kickstart5[0]=0;
   config.kickstart6[0]=0;
   config.kickstart7[0]=0;
   config.kickstart8[0]=0;
   config.kickstart9[0]=0;
   config.ext_kickstart=0;
   config.ext_kickstart0[0]=0;
   config.ext_kickstart1[0]=0;
   config.ext_kickstart2[0]=0;
   config.ext_kickstart3[0]=0;
   config.ext_kickstart4[0]=0;
   config.ext_kickstart5[0]=0;
   config.ext_kickstart6[0]=0;
   config.ext_kickstart7[0]=0;
   config.ext_kickstart8[0]=0;
   config.ext_kickstart9[0]=0;
   config.enable_test=0;
   config.bootscreen_resolution=RES_800x600;
}
void write_config_file(char *filename)
{
   static FIL fil;      /* File object */
   f_open(&fil,filename, FA_OPEN_ALWAYS | FA_WRITE);
   load_default_config();
   print_line(&fil,"## Z3660 config file ##\n");
   print_line(&fil,"\n");
   print_line(&fil,"##General Configuration\n");
   print_line(&fil,"\n");
   print_line(&fil,"# Select boot mode: \"CPU\" for 060 CPU, \"MUSASHI\" \"UAE\" or \"UAEJIT\" for emulator\n");
   print_line(&fil,"#bootmode CPU\n");
   print_line(&fil,"#bootmode MUSASHI\n");
   print_line(&fil,"#bootmode UAE\n");
   print_line(&fil,"bootmode UAEJIT\n");
   print_line(&fil,"\n");
   print_line(&fil,"# Select 060 CPU frequency in MHz\n");
   print_line(&fil,"cpufreq 80\n");
   print_line(&fil,"\n");
   print_line(&fil,"## Emulation Configuration\n");
   print_line(&fil,"\n");
   print_line(&fil,"# Declare up to 9 kickstarts files. (1-9)\n");
   print_line(&fil,"kickstart1 kicks/A4kOS322.rom\n");
   print_line(&fil,"kickstart2 kicks/DiagROM.rom\n");
   print_line(&fil,"kickstart3 kicks/A4kOS31.rom\n");
   print_line(&fil,"kickstart4 kicks/A4kOS321.rom\n");
   print_line(&fil,"# Select the number (1-9) to map one of the above kickstarts on ARM's internal RAM, or 0 to use the installed kickstart on your Amiga\n");
   print_line(&fil,"kickstart 1\n");
   print_line(&fil,"\n");
   print_line(&fil,"# Declare up to 9 extended kickstarts files. (1-9)\n");
   print_line(&fil,"ext_kickstart1 kicks/kick060.rom\n");
   print_line(&fil,"# Select the number (1-9) to map one of the above extended kickstarts on ARM's internal RAM, or 0 to use the installed extended kickstart on your Amiga (if any)\n");
   print_line(&fil,"ext_kickstart 1\n");
   print_line(&fil,"\n");
   print_line(&fil,"# Load scsi ROM on boot (boot from SCSI hdf files on SD)\n");
   print_line(&fil,"# (YES or NO, in capitals)\n");
   print_line(&fil,"scsiboot NO\n");
   print_line(&fil,"#scsiboot YES\n");
   print_line(&fil,"\n");
   print_line(&fil,"# Declare your hdf files (from hdf0 to hdf19)\n");
   print_line(&fil,"#hdf0 hdf/A4000.hdf\n");
   print_line(&fil,"#hdf1 hdf/ZDH0.hdf\n");
   print_line(&fil,"#hdf2 hdf/Programs.hdf\n");
   print_line(&fil,"# Select the scsi number (scsi0 to scsi6) to assign one of the above hdf files\n");
   print_line(&fil,"scsi0 0\n");
   print_line(&fil,"scsi1 1\n");
   print_line(&fil,"#scsi2 2\n");
   print_line(&fil,"\n");
   print_line(&fil,"# Autoconfig RAM Enable (256 MB Zorro III RAM)\n");
   print_line(&fil,"# (YES or NO, in capitals)\n");
   print_line(&fil,"autoconfig_ram NO\n");
   print_line(&fil,"#autoconfig_ram YES\n");
   print_line(&fil,"\n");
   print_line(&fil,"# CPU RAM Enable (128 MB CPU RAM)\n");
   print_line(&fil,"# IMPORTANT NOTE: disabling CPU RAM, will also disable SCSIBOOT\n");
   print_line(&fil,"# (YES or NO, in capitals)\n");
   print_line(&fil,"#cpu_ram NO\n");
   print_line(&fil,"cpu_ram YES\n");
   print_line(&fil,"\n");
   print_line(&fil,"# Temperature sensor calibration (THERM)\n");
   print_line(&fil,"# Theoretical value 780 Ohms @ 25 Celsius, but every 060 has a random offset.\n");
   print_line(&fil,"# Resistor R30 (measured from R30 right pad to ground, with 060 cooled to room temperature) in Ohms\n");
   print_line(&fil,"resistor 800.0\n");
   print_line(&fil,"# Room temperature in Celsius\n");
   print_line(&fil,"temperature 27.0\n");
   print_line(&fil,"\n");
   print_line(&fil,"# Select Test Amiga CHIP RAM access on start (it uses the ARM, not the CPU)\n");
   print_line(&fil,"enable_test YES\n");
   print_line(&fil,"#enable_test NO\n");
   print_line(&fil,"#enable_test MIN\n");
   print_line(&fil,"\n");
   print_line(&fil,"# Select a boot screen resolution (1920x1080, 1280x720 or 800x600)\n");
   print_line(&fil,"bootscreen_resolution 1920x1080\n");
   print_line(&fil,"\n");
   f_close(&fil);

   printf("File %s written OK\n",filename);
}
void trim_whitespace(char *str) {
   //trailing white spaces
  while (strlen(str) != 0 && (str[strlen(str) - 1] == ' ' || str[strlen(str) - 1] == '\t' || str[strlen(str) - 1] == 0x0A || str[strlen(str) - 1] == 0x0D)) {
    str[strlen(str) - 1] = '\0';
  }
}
void get_next_string(char *str, char *str_out, int *strpos, char separator) {
  int str_pos = 0, out_pos = 0, startquote = 0, endstring = 0;

  if (!str_out)
    return;

  if (strpos)
    str_pos = *strpos;

  while ((str[str_pos] == ' ' || str[str_pos] == '\t') && str_pos < (int)strlen(str)) {
    str_pos++;
  }

  if (str[str_pos] == '\"') {
    str_pos++;
    startquote = 1;
  }


  for (int i = str_pos; i < (int)strlen(str); i++) {
    str_out[out_pos] = str[i];

    if (startquote) {
      if (str[i] == '\"')
        endstring = 1;
    } else {
      if ((separator == ' ' && (str[i] == ' ' || str[i] == '\t')) || str[i] == separator) {
        endstring = 1;
      }
    }

    if (endstring) {
      str_out[out_pos] = '\0';
      if (strpos) {
        *strpos = i + 1;
      }
      break;
    }

    out_pos++;
    if (i + 1 == (int)strlen(str) && strpos) {
      *strpos = i + 1;
      str_out[out_pos] = '\0';
    }
  }
}
int get_config_item_type(char *cmd) {
  for (int i = 0; i < CONFITEM_NUM; i++) {
    if (strcmp(cmd, config_item_names[i]) == 0) {
      return i;
    }
  }
  return CONFITEM_NONE;
}
int get_bootmode_type(char *cmd) {
  for (int i = 0; i < BOOTMODE_NUM; i++) {
    if (strcmp(cmd, bootmode_names[i]) == 0) {
      return i;
    }
  }
  return UAEJIT;
}
int get_yesno_type(char *cmd) {
  for (int i = 0; i < YESNO_NUM; i++) {
    if (strcmp(cmd, yesno_names[i]) == 0) {
      return i;
    }
  }
  return NO;
}
int get_yesnomin_type(char *cmd) {
  for (int i = 0; i < YESNOMIN_NUM; i++) {
    if (strcmp(cmd, yesnomin_names[i]) == 0) {
      return i;
    }
  }
  return NO;
}
int get_resolution_type(char *cmd) {
  for (int i = 0; i < RES_NUM; i++) {
    if (strcmp(cmd, resolution_names[i]) == 0) {
      return i;
    }
  }
  return RES_800x600;
}
float get_float_type(char *cmd) {
  return atof(cmd);
}
int get_int_type(char *cmd) {
  return atoi(cmd);
}

void read_config_file(void)
{
   char Filename[]=DEFAULT_ROOT "z3660cfg.txt";
   static FIL fil;      /* File object */
   static FATFS fatfs;

   TCHAR *Path = DEFAULT_ROOT;

   Xil_ExceptionDisable();

   int ret;
retry:
   ret=f_mount(&fatfs, Path, 1); // 1 mount immediately
   if(ret!=0)
   {
      printf("Error opening SD media\nRetry in 5 seconds\n");
      sleep(5);
      goto retry;
   }
   ret=f_open(&fil,Filename, FA_OPEN_EXISTING | FA_READ);
   if(ret!=0)
   {
//      printf("Error opening file \"%s\"\nCreating default file...\n",Filename);
//      write_config_file(Filename);
//      ret=f_open(&fil,Filename, FA_OPEN_EXISTING | FA_READ);
//      if(ret!=0)
      {
         printf("Error opening config file %s\nHALT!!!\n",Filename);
         while(1);
      }
   }
   printf("Reading %s file \n",Filename);
   int cur_line = 1;
   char parse_line[512];
   char cur_cmd[128];
   memset(&config, 0x00, sizeof(CONFIG));
//   sprintf(config.kickstart,""); // this produces a warning O_O
   config.kickstart0[0]=0;
   config.kickstart1[0]=0;
   config.kickstart2[0]=0;
   config.kickstart3[0]=0;
   config.kickstart4[0]=0;
   config.kickstart5[0]=0;
   config.kickstart6[0]=0;
   config.kickstart7[0]=0;
   config.kickstart8[0]=0;
   config.kickstart9[0]=0;
   config.ext_kickstart0[0]=0;
   config.ext_kickstart1[0]=0;
   config.ext_kickstart2[0]=0;
   config.ext_kickstart3[0]=0;
   config.ext_kickstart4[0]=0;
   config.ext_kickstart5[0]=0;
   config.ext_kickstart6[0]=0;
   config.ext_kickstart7[0]=0;
   config.ext_kickstart8[0]=0;
   config.ext_kickstart9[0]=0;
   for(int i=0;i<20;i++)
	   config.hdf[i][0]=0;
   for(int i=0;i<7;i++)
	   config.scsi_num[i]=-1;
   config.bootscreen_resolution=RES_800x600;

   while (!f_eof(&fil))
   {
      int str_pos = 0;
      memset(parse_line, 0x00, 512);
      f_gets(parse_line, (s32)512, &fil);

      if (strlen(parse_line) <= 2 || parse_line[0] == '#' || parse_line[0] == '/')
         goto skip_line;

      trim_whitespace(parse_line);

      get_next_string(parse_line, cur_cmd, &str_pos, ' ');
      int item=get_config_item_type(cur_cmd);
      switch (item) {
      //         case CONFITEM_CPUTYPE:
      //           cfg->cpu_type = get_m68k_cpu_type(parse_line + str_pos);
      //           break;
      case CONFITEM_BOOTMODE:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.boot_mode=get_bootmode_type(cur_cmd);
         printf("[CFG] Boot mode %s.\n", bootmode_names[config.boot_mode]);
         shared->cfg_emu=config.boot_mode;
         shared->jit_enabled=config.boot_mode==UAEJIT?1:0;
         break;

      case CONFITEM_KICKSTART:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.kickstart=get_int_type(cur_cmd);
         if(config.kickstart==0)
            printf("[CFG] Kickstart file selected: mobo kickstart.\n");
         else
            printf("[CFG] Kickstart file selected: number %d.\n", config.kickstart);
         break;

      case CONFITEM_EXT_KICKSTART:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.ext_kickstart=get_int_type(cur_cmd);
         if(config.ext_kickstart==0)
            printf("[CFG] Extended Kickstart file selected: mobo extended kickstart (if any).\n");
         else
            printf("[CFG] Extended Kickstart file selected: number %d.\n", config.ext_kickstart);
         break;

      case CONFITEM_SCSI_BOOT_ENABLE:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.scsiboot=get_yesno_type(cur_cmd);
         printf("[CFG] SCSI Boot %s.\n", yesno_names[config.scsiboot]);
         break;

      case CONFITEM_ENABLE_TEST:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.enable_test=get_yesnomin_type(cur_cmd);
         printf("[CFG] Enable Test %s.\n", yesnomin_names[config.enable_test]);
         break;

      case CONFITEM_SCSI0:
      case CONFITEM_SCSI1:
      case CONFITEM_SCSI2:
      case CONFITEM_SCSI3:
      case CONFITEM_SCSI4:
      case CONFITEM_SCSI5:
      case CONFITEM_SCSI6: {
    	  int index=item-CONFITEM_SCSI0;
    	  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
    	  config.scsi_num[index]=get_int_type(cur_cmd);
    	  printf("[CFG] SCSI%d assigned to %s\n",index,config.hdf[config.scsi_num[index]]);
    	  break;
      }
      case CONFITEM_HDF0:
      case CONFITEM_HDF1:
      case CONFITEM_HDF2:
      case CONFITEM_HDF3:
      case CONFITEM_HDF4:
      case CONFITEM_HDF5:
      case CONFITEM_HDF6:
      case CONFITEM_HDF7:
      case CONFITEM_HDF8:
      case CONFITEM_HDF9:
      case CONFITEM_HDF10:
      case CONFITEM_HDF11:
      case CONFITEM_HDF12:
      case CONFITEM_HDF13:
      case CONFITEM_HDF14:
      case CONFITEM_HDF15:
      case CONFITEM_HDF16:
      case CONFITEM_HDF17:
      case CONFITEM_HDF18:
      case CONFITEM_HDF19: {
         int index=item-CONFITEM_HDF0;
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         sprintf(config.hdf[index],"%s%s", DEFAULT_ROOT, cur_cmd);
         printf("[CFG] hdf%d file %s\n", index, config.hdf[index]);
         break;
      }

      case CONFITEM_AUTOCONFIG_RAM_ENABLE:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.autoconfig_ram=get_yesno_type(cur_cmd);
         printf("[CFG] AutoConfig RAM %s.\n", yesno_names[config.autoconfig_ram]);
         break;

      case CONFITEM_CPU_RAM_ENABLE:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.cpu_ram=get_yesno_type(cur_cmd);
         printf("[CFG] CPU RAM %s.\n", yesno_names[config.cpu_ram]);
         break;

      case CONFITEM_RESISTOR:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.resistor=get_float_type(cur_cmd);
         printf("[CFG] Calibrated Resistor %.1f.\n", config.resistor);
         break;

      case CONFITEM_TEMPERATURE:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.temperature=get_float_type(cur_cmd);
         printf("[CFG] Calibrated Temperature %.1f.\n", config.temperature);
         break;

      case CONFITEM_CPUFREQ:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.cpufreq=(get_int_type(cur_cmd)/10)*10; // 10 MHz steps for now
         printf("[CFG] 060 CPU Frequency %d MHz\n", config.cpufreq);
         break;

      case CONFITEM_KICKSTART0:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         printf("[CFG] WARNING!!! Kickstart file 0 is reserved to internal kickstart!!!.\n");
         break;

#define CASE_CONFITEM_KICKSTART(x)  case CONFITEM_KICKSTART ## x:\
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');\
         sprintf(config.kickstart ## x,"%s%s", DEFAULT_ROOT, cur_cmd);\
         printf("[CFG] kickstart%d file %s\n", x, config.kickstart ## x);\
         break;
         CASE_CONFITEM_KICKSTART(1)
         CASE_CONFITEM_KICKSTART(2)
         CASE_CONFITEM_KICKSTART(3)
         CASE_CONFITEM_KICKSTART(4)
         CASE_CONFITEM_KICKSTART(5)
         CASE_CONFITEM_KICKSTART(6)
         CASE_CONFITEM_KICKSTART(7)
         CASE_CONFITEM_KICKSTART(8)
         CASE_CONFITEM_KICKSTART(9)

      case CONFITEM_EXT_KICKSTART0:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         printf("[CFG] WARNING!!! Extended Kickstart file 0 is reserved to internal extended kickstart!!!.\n");
         break;

#define CASE_CONFITEM_EXT_KICKSTART(x)  case CONFITEM_EXT_KICKSTART ## x:\
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');\
         sprintf(config.ext_kickstart ## x,"%s%s", DEFAULT_ROOT, cur_cmd);\
         printf("[CFG] Extended kickstart%d file %s\n", x, config.ext_kickstart ## x);\
         break;
         CASE_CONFITEM_EXT_KICKSTART(1)
         CASE_CONFITEM_EXT_KICKSTART(2)
         CASE_CONFITEM_EXT_KICKSTART(3)
         CASE_CONFITEM_EXT_KICKSTART(4)
         CASE_CONFITEM_EXT_KICKSTART(5)
         CASE_CONFITEM_EXT_KICKSTART(6)
         CASE_CONFITEM_EXT_KICKSTART(7)
         CASE_CONFITEM_EXT_KICKSTART(8)
         CASE_CONFITEM_EXT_KICKSTART(9)

      case CONFITEM_BOOTSCREEN_RESOLUTION:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.bootscreen_resolution=get_resolution_type(cur_cmd);
         printf("[CFG] Boot Screen Resolution %s\n", resolution_names[config.bootscreen_resolution]);
         break;

      case CONFITEM_NONE:
      default:
         printf("[CFG] Unknown config item %s on line %d.\n", cur_cmd, cur_line);
         break;
      }

      skip_line:
      cur_line++;
   }
   goto load_successful;

//load_failed:;
   printf("Error loading config file %s\n",Filename);
   printf("Loading default config\n");
   load_default_config();
load_successful:;

   f_close(&fil);
   f_mount(NULL, Path, 1); // NULL unmount, 0 delayed
   printf("Config file read OK\n");

   Xil_ExceptionEnable();
}
void read_env_files(void)
{
   static FIL fil;      /* File object */
   static FATFS fatfs;

   TCHAR *Path = DEFAULT_ROOT;

   Xil_ExceptionDisable();

   int ret;
retry:
   ret=f_mount(&fatfs, Path, 1); // 1 mount immediately
   if(ret!=0)
   {
      printf("Error opening SD media\nRetry in 5 seconds\n");
      sleep(5);
      goto retry;
   }

   ret=f_open(&fil,DEFAULT_ROOT "env/bootmode", FA_OPEN_EXISTING | FA_READ);
   if(ret==0)
   {
	  if(f_size(&fil)>0)
	  {
		  char parse_line[512];
		  char cur_cmd[128];
		  int str_pos = 0;
		  memset(parse_line, 0x00, 512);
		  f_gets(parse_line, (s32)512, &fil);
		  get_next_string(parse_line, cur_cmd, &str_pos, '\n');
		  config.boot_mode=get_bootmode_type(cur_cmd);
		  printf("\e[30m\e[103m[ENV] Boot mode %s.\e[0m\n", bootmode_names[config.boot_mode]);
		  shared->cfg_emu=config.boot_mode;
		  shared->jit_enabled=config.boot_mode==UAEJIT?1:0;
	  }
	  else
		  printf("[ENV] Warning!!! Boot mode file is empty\n");
   }
   f_close(&fil);

   ret=f_open(&fil,DEFAULT_ROOT "env/scsiboot", FA_OPEN_EXISTING | FA_READ);
   if(ret==0)
   {
	  if(f_size(&fil)>0)
	  {
		  char parse_line[512];
		  char cur_cmd[128];
		  int str_pos = 0;
		  memset(parse_line, 0x00, 512);
		  f_gets(parse_line, (s32)512, &fil);
		  get_next_string(parse_line, cur_cmd, &str_pos, '\n');
		  config.scsiboot=get_yesno_type(cur_cmd);
		  printf("\e[30m\e[103m[ENV] SCSI Boot %s.\e[0m\n", yesno_names[config.scsiboot]);
	  }
	  else
		  printf("[ENV] Warning!!! SCSI Boot file is empty\n");
   }
   f_close(&fil);

   ret=f_open(&fil,DEFAULT_ROOT "env/autoconfig_ram", FA_OPEN_EXISTING | FA_READ);
   if(ret==0)
   {
	  if(f_size(&fil)>0)
	  {
		  char parse_line[512];
		  char cur_cmd[128];
		  int str_pos = 0;
		  memset(parse_line, 0x00, 512);
		  f_gets(parse_line, (s32)512, &fil);
		  get_next_string(parse_line, cur_cmd, &str_pos, '\n');
		  config.autoconfig_ram=get_yesno_type(cur_cmd);
		  printf("\e[30m\e[103m[ENV] AutoConfig Ram %s.\e[0m\n", yesno_names[config.autoconfig_ram]);
	  }
	  else
		  printf("[ENV] Warning!!! AutoConfig Ram file is empty\n");
   }
   f_close(&fil);

   ret=f_open(&fil,DEFAULT_ROOT "env/cpu_ram", FA_OPEN_EXISTING | FA_READ);
   if(ret==0)
   {
	  if(f_size(&fil)>0)
	  {
		  char parse_line[512];
		  char cur_cmd[128];
		  int str_pos = 0;
		  memset(parse_line, 0x00, 512);
		  f_gets(parse_line, (s32)512, &fil);
		  get_next_string(parse_line, cur_cmd, &str_pos, '\n');
		  config.cpu_ram=get_yesno_type(cur_cmd);
		  printf("\e[30m\e[103m[ENV] CPU Ram %s.\e[0m\n", yesno_names[config.cpu_ram]);
	  }
	  else
		  printf("[ENV] Warning!!! CPU Ram file is empty\n");
   }
   f_close(&fil);

   ret=f_open(&fil,DEFAULT_ROOT "env/cpufreq", FA_OPEN_EXISTING | FA_READ);
   if(ret==0)
   {
	  if(f_size(&fil)>0)
	  {
		  char parse_line[512];
		  char cur_cmd[128];
		  int str_pos = 0;
		  memset(parse_line, 0x00, 512);
		  f_gets(parse_line, (s32)512, &fil);
		  get_next_string(parse_line, cur_cmd, &str_pos, '\n');
		  config.cpufreq=(get_int_type(cur_cmd)/10)*10;
		  printf("\e[30m\e[103m[ENV] 060 CPU Frequency %d MHz\e[0m\n", config.cpufreq);
	  }
	  else
		  printf("[ENV] Warning!!! CPU Frequency file is empty\n");
   }
   f_close(&fil);

   ret=f_open(&fil,DEFAULT_ROOT "env/kickstart", FA_OPEN_EXISTING | FA_READ);
   if(ret==0)
   {
	  if(f_size(&fil)>0)
	  {
		  char parse_line[512];
		  char cur_cmd[128];
		  int str_pos = 0;
		  memset(parse_line, 0x00, 512);
		  f_gets(parse_line, (s32)512, &fil);
		  get_next_string(parse_line, cur_cmd, &str_pos, '\n');
		  config.kickstart=get_int_type(cur_cmd);
		  printf("\e[30m\e[103m[ENV] kickstart selected: number %d\e[0m\n", config.kickstart);
	  }
	  else
		  printf("[ENV] Warning!!! kickstart file is empty\n");
   }
   f_close(&fil);

   ret=f_open(&fil,DEFAULT_ROOT "env/ext_kickstart", FA_OPEN_EXISTING | FA_READ);
   if(ret==0)
   {
	  if(f_size(&fil)>0)
	  {
		  char parse_line[512];
		  char cur_cmd[128];
		  int str_pos = 0;
		  memset(parse_line, 0x00, 512);
		  f_gets(parse_line, (s32)512, &fil);
		  get_next_string(parse_line, cur_cmd, &str_pos, '\n');
		  config.ext_kickstart=get_int_type(cur_cmd);
		  printf("\e[30m\e[103m[ENV] Extended kickstart selected: number %d\e[0m\n", config.ext_kickstart);
	  }
	  else
		  printf("[ENV] Warning!!! Extended kickstart file is empty\n");
   }
   f_close(&fil);

   ret=f_open(&fil,DEFAULT_ROOT "env/scsi", FA_OPEN_EXISTING | FA_READ);
   if(ret==0)
   {
	  if(f_size(&fil)>0)
	  {
		  char parse_line[512];
		  for(int i=0;i<7;i++)
		  {
			  memset(parse_line, 0x00, 512);
			  f_gets(parse_line, (s32)512, &fil);
			  config.scsi_num[i]=get_int_type(parse_line);
			  printf("\e[30m\e[103m[ENV] SCSI%d assigned to %d.\e[0m\n", i,config.scsi_num[i]);
		  }
	  }
	  else
		  printf("[ENV] Warning!!! SCSI file is empty\n");
   }
   f_close(&fil);

   ret=f_open(&fil,DEFAULT_ROOT "env/enabletest", FA_OPEN_EXISTING | FA_READ);
   if(ret==0)
   {
	  if(f_size(&fil)>0)
	  {
		  char parse_line[512];
		  char cur_cmd[128];
		  int str_pos = 0;
		  memset(parse_line, 0x00, 512);
		  f_gets(parse_line, (s32)512, &fil);
		  get_next_string(parse_line, cur_cmd, &str_pos, '\n');
		  config.enable_test=get_yesnomin_type(cur_cmd);
		  printf("\e[30m\e[103m[ENV] Enable Test %s.\e[0m\n", yesnomin_names[config.enable_test]);
	  }
	  else
		  printf("[ENV] Warning!!! Enable Test file is empty\n");
   }
   f_close(&fil);

   f_mount(NULL, Path, 1); // NULL unmount, 0 delayed
   Xil_ExceptionEnable();
}
int write_env_files(ENV_FILE_VARS env_file)
{
   static FIL fil;      /* File object */
   static FATFS fatfs;

   TCHAR *Path = DEFAULT_ROOT;

   Xil_ExceptionDisable();

   int ret;
retry:
   ret=f_mount(&fatfs, Path, 1); // 1 mount immediately
   if(ret!=0)
   {
      printf("Error opening SD media\nRetry in 5 seconds\n");
      sleep(5);
      goto retry;
   }

   ret=f_open(&fil,DEFAULT_ROOT "env/bootmode", FA_CREATE_ALWAYS | FA_WRITE);
   if(ret==FR_OK)
   {
      printf("[Config] Write file env/bootmode with %s\n",bootmode_names[env_file.bootmode]);
      f_printf(&fil,"%s\n",bootmode_names[env_file.bootmode]);
      f_close(&fil);
   }
   else
   {
      printf("[Config] ERROR Write file env/bootmode\n");
      Xil_ExceptionEnable();
      return(0);
   }

   ret=f_open(&fil,DEFAULT_ROOT "env/scsiboot", FA_CREATE_ALWAYS | FA_WRITE);
   if(ret==FR_OK)
   {
      printf("[Config] Write file env/scsiboot with %s\n",yesno_names[env_file.scsiboot]);
      f_printf(&fil,"%s\n",yesno_names[env_file.scsiboot]);
      f_close(&fil);
   }
   else
   {
      printf("[Config] ERROR Write file env/scsiboot\n");
      Xil_ExceptionEnable();
      return(0);
   }

   ret=f_open(&fil,DEFAULT_ROOT "env/autoconfig_ram", FA_CREATE_ALWAYS | FA_WRITE);
   if(ret==FR_OK)
   {
      printf("[Config] Write file env/autoconfig_ram with %s\n",yesno_names[env_file.autoconfig_ram]);
      f_printf(&fil,"%s\n",yesno_names[env_file.autoconfig_ram]);
      f_close(&fil);
   }
   else
   {
      printf("[Config] ERROR Write file env/autoconfig_ram\n");
      Xil_ExceptionEnable();
      return(0);
   }

   ret=f_open(&fil,DEFAULT_ROOT "env/cpu_ram", FA_CREATE_ALWAYS | FA_WRITE);
   if(ret==FR_OK)
   {
      printf("[Config] Write file env/cpu_ram with %s\n",yesno_names[env_file.cpu_ram]);
      f_printf(&fil,"%s\n",yesno_names[env_file.cpu_ram]);
      f_close(&fil);
   }
   else
   {
      printf("[Config] ERROR Write file env/cpu_ram\n");
      Xil_ExceptionEnable();
      return(0);
   }

   ret=f_open(&fil,DEFAULT_ROOT "env/cpufreq", FA_CREATE_ALWAYS | FA_WRITE);
   if(ret==FR_OK)
   {
      printf("[Config] Write file env/cpufreq with %d\n",config.cpufreq);
      f_printf(&fil,"%d\n",config.cpufreq);
      f_close(&fil);
   }
   else
   {
      printf("[Config] ERROR Write file env/cpufreq\n");
      Xil_ExceptionEnable();
      return(0);
   }

   ret=f_open(&fil,DEFAULT_ROOT "env/kickstart", FA_CREATE_ALWAYS | FA_WRITE);
   if(ret==FR_OK)
   {
      printf("[Config] Write file env/kickstart with %d\n",config.kickstart);
      f_printf(&fil,"%d\n",config.kickstart);
      f_close(&fil);
   }
   else
   {
      printf("[Config] ERROR Write file env/kickstart\n");
      Xil_ExceptionEnable();
      return(0);
   }

   ret=f_open(&fil,DEFAULT_ROOT "env/ext_kickstart", FA_CREATE_ALWAYS | FA_WRITE);
   if(ret==FR_OK)
   {
      printf("[Config] Write file env/ext_kickstart with %d\n",config.kickstart);
      f_printf(&fil,"%d\n",config.ext_kickstart);
      f_close(&fil);
   }
   else
   {
      printf("[Config] ERROR Write file env/ext_kickstart\n");
      Xil_ExceptionEnable();
      return(0);
   }

   ret=f_open(&fil,DEFAULT_ROOT "env/enabletest", FA_CREATE_ALWAYS | FA_WRITE);
   if(ret==FR_OK)
   {
      printf("[Config] Write file env/enabletest with %s\n",yesno_names[env_file.enable_test]);
      f_printf(&fil,"%s\n",yesno_names[env_file.enable_test]);
      f_close(&fil);
   }
   else
   {
      printf("[Config] ERROR Write file env/enabletest\n");
      Xil_ExceptionEnable();
      return(0);
   }

   usleep(10000);
   f_mount(NULL, Path, 1); // NULL unmount, 1 immediately
   Xil_ExceptionEnable();
   usleep(10000);
   return(1);
}
int write_env_files2(int *scsi_num)
{
   static FIL fil;      /* File object */
   static FATFS fatfs;

   TCHAR *Path = DEFAULT_ROOT;

   Xil_ExceptionDisable();

   int ret;
retry:
   ret=f_mount(&fatfs, Path, 1); // 1 mount immediately
   if(ret!=0)
   {
      printf("Error opening SD media\nRetry in 5 seconds\n");
      sleep(5);
      goto retry;
   }

   ret=f_open(&fil,DEFAULT_ROOT "env/scsi", FA_CREATE_ALWAYS | FA_WRITE);
   if(ret==FR_OK)
   {
      printf("[Config] Write file env/scsi\n");
      for(int i=0;i<7;i++)
      {
    	  f_printf(&fil,"%d\n",config.scsi_num[i]-1);
      }
      f_close(&fil);
   }
   else
   {
      printf("[Config] ERROR Write file env/scsi\n");
      Xil_ExceptionEnable();
      return(0);
   }

   usleep(10000);
   f_mount(NULL, Path, 1); // NULL unmount, 1 immediately
   Xil_ExceptionEnable();
   usleep(10000);
   return(1);
}
int delete_env_files(void)
{
   static FIL fil;      /* File object */
   static FATFS fatfs;

   TCHAR *Path = DEFAULT_ROOT;

   Xil_ExceptionDisable();

   int ret;
retry:
   ret=f_mount(&fatfs, Path, 1); // 1 mount immediately
   if(ret!=0)
   {
      printf("Error opening SD media\nRetry in 5 seconds\n");
      sleep(5);
      goto retry;
   }

#define DELETE_FILE(X) ret=f_unlink(DEFAULT_ROOT X);                         \
                       if(ret==FR_OK)                                        \
                       {                                                     \
                          printf("[Config] Delete file " X "\n");      \
                          f_close(&fil);                                     \
                       }                                                     \
                       else                                                  \
                       {                                                     \
                          printf("[Config] Can't Delete file " X "\n");\
                       }

   DELETE_FILE("env/bootmode")
   DELETE_FILE("env/scsiboot");
   DELETE_FILE("env/autoconfig_ram");
   DELETE_FILE("env/cpu_ram");
   DELETE_FILE("env/cpufreq");
   DELETE_FILE("env/kickstart");
   DELETE_FILE("env/ext_kickstart");
   DELETE_FILE("env/scsi");
   DELETE_FILE("env/enabletest");

   usleep(10000);
   f_mount(NULL, Path, 1); // NULL unmount, 1 immediately
   Xil_ExceptionEnable();
   usleep(10000);
   return(1);
}
