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
#include "ARM_ztop/checkbox.h"
CONFIG config,default_config,temp_config;
extern SHARED *shared;
extern uint8_t EmacPsMAC[6];

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
      "autoconfig_rtg",
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
      "mac_address",
      "bp_ton",
      "bp_toff",
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
      "800x600",
      "1280x720",
      "1920x1080",
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
   config.autoconfig_rtg=0;
   config.cpu_ram=1;
   config.resistor=800.0;
   config.temperature=27.0;
   config.cpufreq=CPUFREQ_MIN;
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
   for(int i=0;i<6;i++)
      config.mac_address[i]=EmacPsMAC[i];
   config.bp_ton=0;
   config.bp_toff=0;
   memcpy(&default_config,&config,sizeof(CONFIG));
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
   print_line(&fil,"# Autoconfig RTG Enable (128 MB Zorro III space)\n");
   print_line(&fil,"# (YES or NO, in capitals)\n");
   print_line(&fil,"autoconfig_rtg NO\n");
   print_line(&fil,"#autoconfig_rtg YES\n");
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
   print_line(&fil,"# Select a mac address for Ethernet\n");
   print_line(&fil,"mac_address 00:80:10:00:01:00\n");
   print_line(&fil,"\n");
   print_line(&fil,"# Set the BEEPER time on/off to simulate SCSI activity noise\n");
   print_line(&fil,"bp_ton 0.001\n");
   print_line(&fil,"bp_toff 0.010\n");
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
int get_mac_address_type(char *cmd) {
   if(cmd[2]!=':')
      return(0);
   if(cmd[5]!=':')
      return(0);
   if(cmd[8]!=':')
      return(0);
   if(cmd[11]!=':')
      return(0);
   if(cmd[14]!=':')
      return(0);
   for (int i = 0; i < 6; i++) {
      uint8_t hi=cmd[0+i*3];
      if(hi>='0' && hi<='9') hi-='0';
      if(hi>='a' && hi<='f') hi=-'a'+10;
      if(hi>='F' && hi<='F') hi=-'A'+10;
      uint8_t lo=cmd[1+i*3];
      if(lo>='0' && lo<='9') lo-='0';
      if(lo>='a' && lo<='f') lo=-'a'+10;
      if(lo>='F' && lo<='F') lo=-'A'+10;
      config.mac_address[i]=hi*16+lo;
   }
   return(1);
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
   config.boot_mode=UAEJIT;
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
   for(int i=0;i<6;i++)
      config.mac_address[i]=EmacPsMAC[i];

   config.resistor=800.0;
   config.temperature=27.0;

   config.scsiboot=0;
   config.autoconfig_ram=0;
   config.autoconfig_rtg=0;
   config.cpu_ram=1;
   config.resistor=800.0;
   config.temperature=27.0;
   config.cpufreq=CPUFREQ_MIN;
   config.kickstart=0;
   config.ext_kickstart=0;
   config.enable_test=0;
   config.bp_ton=0;
   config.bp_toff=0;

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
         if(config.scsi_num[index]>=0)
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

      case CONFITEM_AUTOCONFIG_RTG_ENABLE:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.autoconfig_rtg=get_yesno_type(cur_cmd);
         printf("[CFG] AutoConfig RTG %s.\n", yesno_names[config.autoconfig_rtg]);
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
         config.cpufreq=(get_int_type(cur_cmd)/5)*5; // 5 MHz steps
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

      case CONFITEM_MAC_ADDRESS:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         if(get_mac_address_type(cur_cmd)) {
            printf("[CFG] Mac Address %02X:%02X:%02X:%02X:%02X:%02X\n", config.mac_address[0],
                  config.mac_address[1],config.mac_address[2],
                  config.mac_address[3],config.mac_address[4],config.mac_address[5]);
            for(int i=0;i<6;i++)
               EmacPsMAC[i]=config.mac_address[i];
         }
         else
            printf("[CFG] BAD Mac Address %s\n",cur_cmd);
         break;
      case CONFITEM_BP_TON:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.bp_ton=get_float_type(cur_cmd);
         printf("[CFG] Beeper Ton %f\n", config.bp_ton);
         break;
      case CONFITEM_BP_TOFF:
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         config.bp_toff=get_float_type(cur_cmd);
         printf("[CFG] Beeper Toff %f\n", config.bp_toff);
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
// this will never be executed
//load_failed:;
   printf("Error loading config file %s\n",Filename);
   printf("Loading default config\n");
   load_default_config();
load_successful:;

   f_close(&fil);
   f_mount(NULL, Path, 1); // NULL unmount, 0 delayed
   printf("Config file read OK\n");

   memcpy(&default_config,&config,sizeof(CONFIG));
   memcpy(&temp_config,&config,sizeof(CONFIG));

   for(int i=0;i<8;i++)
   {
#define COPY_DEFAULT(A) env_file_vars_temp[i].A=default_config.A
      COPY_DEFAULT(boot_mode);
      COPY_DEFAULT(scsiboot);
      COPY_DEFAULT(autoconfig_ram);
      COPY_DEFAULT(autoconfig_rtg);
      COPY_DEFAULT(cpu_ram);
      COPY_DEFAULT(kickstart);
      COPY_DEFAULT(ext_kickstart);
      COPY_DEFAULT(enable_test);
      COPY_DEFAULT(cpufreq);
      for(int j=0;j<7;j++)
         COPY_DEFAULT(scsi_num[j]);
      for(int j=0;j<6;j++)
         COPY_DEFAULT(mac_address[j]);
      COPY_DEFAULT(bp_ton);
      COPY_DEFAULT(bp_toff);
      env_file_vars_temp[i].preset_name[0]=0;
   }

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
   int cur_line = 1;
   char parse_line[512];
   char cur_cmd[128];

//   for(int preset=0;preset<8;preset++)
//      memset(&env_file_vars_temp[preset], 0x00, sizeof(ENV_FILE_VARS));

   ret=f_open(&fil,DEFAULT_ROOT "presets/preset.txt", FA_OPEN_EXISTING | FA_READ);
   if(ret==0)
   {
      preset_selected=-1;
      while (!f_eof(&fil))
      {
         int str_pos = 0;
         memset(parse_line, 0x00, 512);
         f_gets(parse_line, (s32)512, &fil);
         if (strlen(parse_line) <= 2 || parse_line[0] == '#' || parse_line[0] == '/')
            goto skip_line_preset;
         trim_whitespace(parse_line);
         get_next_string(parse_line, cur_cmd, &str_pos, ' ');
         if (strcmp(cur_cmd, "preset") == 0) {
            get_next_string(parse_line, cur_cmd, &str_pos, ' ');
            preset_selected=get_int_type(cur_cmd);
            if(preset_selected>=0 && preset_selected<PRESET_CB_MAX-1)
               printf("[ENV] Preset %d selected\n",preset_selected);
            else
            {
               preset_selected=PRESET_CB_MAX-1;
               printf("[ENV] Default preset selected (z3660cfg.txt file)\n");
            }
            break;
         }
         skip_line_preset:
         cur_line++;
         if(preset_selected==-1)
         {
            preset_selected=PRESET_CB_MAX-1;
            printf("[ENV] WARNING!!! preset NOT selected/loaded\n");
            f_close(&fil);
            return;
         }
      }
   }
   else
   {
      printf("[ENV] Warning!!! Can't open presets/preset.txt file\n");
       return;
   }
   char text[50];
   // Try to read all files
   for(int preset=0;preset<PRESET_CB_MAX-1;preset++)
   {
      sprintf(text,DEFAULT_ROOT "presets/preset%d.txt",preset);

      ret=f_open(&fil,text, FA_OPEN_EXISTING | FA_READ);
      if(ret==0)
      {
         memset(parse_line, 0x00, 512);
         f_gets(parse_line, (s32)512, &fil);
         if(parse_line[0] == '#')
         {
            int str_pos = 1;
            trim_whitespace(parse_line);
            get_next_string(parse_line, cur_cmd, &str_pos, ' ');
            char name[50];
            if (strcmp(cur_cmd,"preset_name") == 0)
            {
               get_next_string(parse_line, cur_cmd, &str_pos, ' ');
               strcpy(name,cur_cmd);
               if(strcmp(name,"")==0)
                  sprintf(name,"preset%d default name",preset);
            }
            else
            {
               sprintf(name,"preset%d default name",preset);
            }
            strcpy(env_file_vars_temp[preset].preset_name,name);
         }
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
               case CONFITEM_BOOTMODE:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].boot_mode=get_bootmode_type(cur_cmd);
                  if(preset==preset_selected)
                     printf("\e[30m\e[103m[ENV] Boot mode %s.\e[0m\n", bootmode_names[env_file_vars_temp[preset].boot_mode]);
                  break;
               case CONFITEM_KICKSTART:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].kickstart=get_int_type(cur_cmd);
                  if(preset==preset_selected)
                  {
                     if(env_file_vars_temp[preset].kickstart==0)
                        printf("\e[30m\e[103m[ENV] Kickstart file selected: mobo kickstart.\e[0m\n");
                     else
                        printf("\e[30m\e[103m[ENV] kickstart selected: number %d\e[0m\n", env_file_vars_temp[preset].kickstart);
                  }
                  break;
               case CONFITEM_EXT_KICKSTART:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].ext_kickstart=get_int_type(cur_cmd);
                  if(preset==preset_selected)
                  {
                     if(env_file_vars_temp[preset].ext_kickstart==0)
                        printf("\e[30m\e[103m[ENV] Extended kickstart selected: mobo extended kickstart (if any).\e[0m\n");
                     else
                        printf("\e[30m\e[103m[ENV] Extended kickstart selected: number %d\e[0m\n", env_file_vars_temp[preset].ext_kickstart);
                  }
                  break;
               case CONFITEM_SCSI_BOOT_ENABLE:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].scsiboot=get_yesno_type(cur_cmd);
                  if(preset==preset_selected)
                     printf("\e[30m\e[103m[ENV] SCSI Boot %s.\e[0m\n", yesno_names[env_file_vars_temp[preset].scsiboot]);
                  break;
               case CONFITEM_ENABLE_TEST:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].enable_test=get_yesnomin_type(cur_cmd);
                  if(preset==preset_selected)
                     printf("\e[30m\e[103m[ENV] Enable Test %s.\e[0m\n", yesnomin_names[env_file_vars_temp[preset].enable_test]);
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
                  env_file_vars_temp[preset].scsi_num[index]=get_int_type(cur_cmd);
                  if(preset==preset_selected)
                  {
                     if(env_file_vars_temp[preset].scsi_num[index]>=0)
                        printf("\e[30m\e[103m[ENV] SCSI%d assigned to %s.\e[0m\n", index,config.hdf[env_file_vars_temp[preset].scsi_num[index]]);
                  }
                  break;
               }
               case CONFITEM_AUTOCONFIG_RAM_ENABLE:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].autoconfig_ram=get_yesno_type(cur_cmd);
                  if(preset==preset_selected)
                  {
                     printf("\e[30m\e[103m[ENV] AutoConfig Ram %s.\e[0m\n", yesno_names[env_file_vars_temp[preset].autoconfig_ram]);
                  }
                  break;
               case CONFITEM_AUTOCONFIG_RTG_ENABLE:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].autoconfig_rtg=get_yesno_type(cur_cmd);
                  if(preset==preset_selected)
                  {
                     printf("\e[30m\e[103m[ENV] AutoConfig RTG %s.\e[0m\n", yesno_names[env_file_vars_temp[preset].autoconfig_rtg]);
                  }
                  break;
               case CONFITEM_CPU_RAM_ENABLE:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].cpu_ram=get_yesno_type(cur_cmd);
                  if(preset==preset_selected)
                     printf("\e[30m\e[103m[ENV] CPU Ram %s.\e[0m\n", yesno_names[env_file_vars_temp[preset].cpu_ram]);
                  break;
               case CONFITEM_CPUFREQ:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].cpufreq=(get_int_type(cur_cmd)/5)*5; // 5 MHz steps
                  if(preset==preset_selected)
                     printf("\e[30m\e[103m[ENV] 060 CPU Frequency %d MHz\e[0m\n", env_file_vars_temp[preset].cpufreq);
                  break;
               case CONFITEM_BOOTSCREEN_RESOLUTION:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].bootscreen_resolution=get_resolution_type(cur_cmd);
                  if(preset==preset_selected)
                     printf("\e[30m\e[103m[ENV] Boot Screen Resolution %s\e[0m\n", resolution_names[env_file_vars_temp[preset].bootscreen_resolution]);
                  break;
               case CONFITEM_MAC_ADDRESS:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  if(get_mac_address_type(cur_cmd)) {
                     for(int i=0;i<6;i++)
                        EmacPsMAC[i]=env_file_vars_temp[preset].mac_address[i]=config.mac_address[i];
                     if(preset==preset_selected)
                        printf("\e[30m\e[103m[ENV] Mac Address %02X:%02X:%02X:%02X:%02X:%02X\e[0m\n", config.mac_address[0],
                        config.mac_address[1],config.mac_address[2],
                        config.mac_address[3],config.mac_address[4],config.mac_address[5]);
                  }
                  else
                  {
                     if(preset==preset_selected)
                        printf("\e[30m\e[103m[ENV] BAD Mac Address %s\e[0m\n",cur_cmd);
                  }
                  break;
               case CONFITEM_BP_TON:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].bp_ton=get_float_type(cur_cmd);
                  if(preset==preset_selected)
                     printf("\e[30m\e[103m[ENV] Beeper Ton %f\e[0m\n", env_file_vars_temp[preset].bp_ton);
                  break;
               case CONFITEM_BP_TOFF:
                  get_next_string(parse_line, cur_cmd, &str_pos, ' ');
                  env_file_vars_temp[preset].bp_toff=get_float_type(cur_cmd);
                  if(preset==preset_selected)
                     printf("\e[30m\e[103m[ENV] Beeper Toff %f\e[0m\n", env_file_vars_temp[preset].bp_toff);
                  break;
               case CONFITEM_NONE:
               default:
                  printf("\e[30m\e[103m[ENV] Unknown config item %s on line %d. (file %s)\e[0m\n", cur_cmd, cur_line,text);
                  break;
            }

            skip_line:
            cur_line++;
         }
      }
      f_close(&fil);
   }
   // Finally, set the presets we are going to use... (preset_selected)
   if(preset_selected<PRESET_CB_MAX-1)
   {
      config.boot_mode=env_file_vars_temp[preset_selected].boot_mode;
      shared->cfg_emu=config.boot_mode;
      shared->jit_enabled=config.boot_mode==UAEJIT?1:0;
      config.kickstart=env_file_vars_temp[preset_selected].kickstart;
      config.ext_kickstart=env_file_vars_temp[preset_selected].ext_kickstart;
      config.scsiboot=env_file_vars_temp[preset_selected].scsiboot;
      config.enable_test=env_file_vars_temp[preset_selected].enable_test;
      for(int i=0;i<7;i++)
         config.scsi_num[i]=env_file_vars_temp[preset_selected].scsi_num[i];
      config.autoconfig_ram=env_file_vars_temp[preset_selected].autoconfig_ram;
      config.autoconfig_rtg=env_file_vars_temp[preset_selected].autoconfig_rtg;
      config.cpu_ram=env_file_vars_temp[preset_selected].cpu_ram;
      config.cpufreq=env_file_vars_temp[preset_selected].cpufreq;
      config.bootscreen_resolution=env_file_vars_temp[preset_selected].bootscreen_resolution;
      for(int i=0;i<6;i++)
         EmacPsMAC[i]=config.mac_address[i]=env_file_vars_temp[preset_selected].mac_address[i];
      config.bp_ton=env_file_vars_temp[preset_selected].bp_ton;
      config.bp_toff=env_file_vars_temp[preset_selected].bp_toff;
      memcpy(&temp_config,&config,sizeof(CONFIG));
   }

   f_mount(NULL, Path, 1); // NULL unmount, 0 delayed
   Xil_ExceptionEnable();
}
int write_env_files(ENV_FILE_VARS *env_file)
{
   static FIL fil;      /* File object */
   static FATFS fatfs;
   TCHAR text[50];
   TCHAR *Path = DEFAULT_ROOT;
   delete_env_files();
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
   sprintf(text,DEFAULT_ROOT "presets");
   ret=f_stat(text,NULL);
   if(ret==FR_NO_FILE)
   {
      if(f_mkdir(text)==FR_OK)
         printf("Created directory %s\n",text);
      else
      {
         printf("Can't create directory %s\n",text);
         Xil_ExceptionEnable();
         return(0);
      }
   }
   sprintf(text,DEFAULT_ROOT "presets/preset.txt");
   ret=f_open(&fil,text, FA_CREATE_ALWAYS | FA_WRITE);
   if(ret==FR_OK)
   {
      f_printf(&fil,"preset %d\n",preset_selected);
      f_close(&fil);
   }
   else
   {
      printf("[Config] ERROR Write file presets/preset.txt\n");
      Xil_ExceptionEnable();
      return(0);
   }
   if(preset_selected<PRESET_CB_MAX-1) // if default preset selected, don't write the preset
   {
      sprintf(text,DEFAULT_ROOT "presets/preset%d.txt",preset_selected);
      printf("writing to file %s name %s\n",text,env_file->preset_name);
      ret=f_open(&fil,text, FA_CREATE_ALWAYS | FA_WRITE);
      if(ret==FR_OK)
      {
         env_file->preset_name[49]=0; // 50 max length
         f_printf(&fil,"#preset_name \"%s\"\n",env_file->preset_name);

         f_printf(&fil,"#############\n");
         f_printf(&fil,"#BOOT options\n");
         f_printf(&fil,"#############\n");
         f_printf(&fil,"bootmode %s\n",bootmode_names[env_file->boot_mode]);
         f_printf(&fil,"scsiboot %s\n",yesno_names[env_file->scsiboot]);
         f_printf(&fil,"autoconfig_ram %s\n",yesno_names[env_file->autoconfig_ram]);
         f_printf(&fil,"autoconfig_rtg %s\n",yesno_names[env_file->autoconfig_rtg]);
         f_printf(&fil,"cpu_ram %s\n",yesno_names[env_file->cpu_ram]);
         f_printf(&fil,"cpufreq %d\n",env_file->cpufreq);
         f_printf(&fil,"kickstart %d\n",env_file->kickstart);
         f_printf(&fil,"ext_kickstart %d\n",env_file->ext_kickstart);
         f_printf(&fil,"enable_test %s\n",yesno_names[env_file->enable_test]);
         f_printf(&fil,"bootscreen_resolution %s\n",resolution_names[env_file->bootscreen_resolution]);

         f_printf(&fil,"#############\n");
         f_printf(&fil,"#SCSI options\n");
         f_printf(&fil,"#############\n");
         for(int i=0;i<7;i++)
            f_printf(&fil,"scsi%d %d\n",i,env_file->scsi_num[i]);

         f_printf(&fil,"#############\n");
         f_printf(&fil,"#MISC options\n");
         f_printf(&fil,"#############\n");
         f_printf(&fil,"mac_address %02X:%02X:%02X:%02X:%02X:%02X\n",env_file->mac_address[0],
               env_file->mac_address[1],env_file->mac_address[2],
               env_file->mac_address[3],env_file->mac_address[4],env_file->mac_address[5]);
         f_printf(&fil,"bp_ton %f\n",env_file->bp_ton);
         f_printf(&fil,"bp_toff %f\n",env_file->bp_toff);

         f_printf(&fil,"\n"); // we really need this at the end of file...
         f_close(&fil);
      }
      else
      {
         printf("[Config] ERROR Write file presets/preset%d.txt\n",preset_selected);
         Xil_ExceptionEnable();
         return(0);
      }
   }
   usleep(10000);
   f_mount(NULL, Path, 1); // NULL unmount, 1 immediately
   Xil_ExceptionEnable();
   usleep(10000);
   return(1);
}
int write_env_files_boot(ENV_FILE_VARS *env_file)
{
//   env_file->preset_name;
//   env_file->boot_mode=config.boot_mode;
//   env_file->scsiboot=config.scsiboot;
//   env_file->autoconfig_ram=config.autoconfig_ram;
//   env_file->autoconfig_rtg=config.autoconfig_rtg;
   //   env_file->enable_test=config.enable_test;
//   env_file->cpu_ram=config.cpu_ram;
//   env_file->cpufreq=config.cpufreq;
//   env_file->kickstart=config.kickstart;
//   env_file->ext_kickstart=config.ext_kickstart;
   env_file->bootscreen_resolution=config.bootscreen_resolution;
   for(int i=0;i<7;i++)
      env_file->scsi_num[i]=config.scsi_num[i];
   for(int i=0;i<6;i++)
      env_file->mac_address[i]=config.mac_address[i];
   env_file->bp_ton=config.bp_ton;
   env_file->bp_toff=config.bp_toff;

   return(write_env_files(env_file));
}
int write_env_files_scsi(ENV_FILE_VARS *env_file)
{
//   env_file->preset_name;
   env_file->boot_mode=config.boot_mode;
   env_file->scsiboot=config.scsiboot;
   env_file->autoconfig_ram=config.autoconfig_ram;
   env_file->autoconfig_rtg=config.autoconfig_rtg;
   env_file->enable_test=config.enable_test;
   env_file->cpu_ram=config.cpu_ram;
   env_file->cpufreq=config.cpufreq;
   env_file->kickstart=config.kickstart;
   env_file->ext_kickstart=config.ext_kickstart;
   env_file->bootscreen_resolution=config.bootscreen_resolution;
//   for(int i=0;i<7;i++)
//      env_file->scsi_num[i]=config.scsi_num[i];
   for(int i=0;i<6;i++)
      env_file->mac_address[i]=config.mac_address[i];
   env_file->bp_ton=config.bp_ton;
   env_file->bp_toff=config.bp_toff;

   return(write_env_files(env_file));
}
int write_env_files_bootscres(ENV_FILE_VARS *env_file)
{
//   env_file->preset_name;
   env_file->boot_mode=config.boot_mode;
   env_file->scsiboot=config.scsiboot;
   env_file->autoconfig_ram=config.autoconfig_ram;
   env_file->autoconfig_rtg=config.autoconfig_rtg;
   env_file->enable_test=config.enable_test;
   env_file->cpu_ram=config.cpu_ram;
   env_file->cpufreq=config.cpufreq;
   env_file->kickstart=config.kickstart;
   env_file->ext_kickstart=config.ext_kickstart;
//   env_file->bootscreen_resolution=config.bootscreen_resolution;
   for(int i=0;i<7;i++)
      env_file->scsi_num[i]=config.scsi_num[i];
   for(int i=0;i<6;i++)
      env_file->mac_address[i]=config.mac_address[i];
   env_file->bp_ton=config.bp_ton;
   env_file->bp_toff=config.bp_toff;

   return(write_env_files(env_file));
}
int write_env_files_misc(ENV_FILE_VARS *env_file)
{
//   env_file->preset_name;
   env_file->boot_mode=config.boot_mode;
   env_file->scsiboot=config.scsiboot;
   env_file->autoconfig_ram=config.autoconfig_ram;
   env_file->autoconfig_rtg=config.autoconfig_rtg;
   env_file->enable_test=config.enable_test;
   env_file->cpu_ram=config.cpu_ram;
   env_file->cpufreq=config.cpufreq;
   env_file->kickstart=config.kickstart;
   env_file->ext_kickstart=config.ext_kickstart;
   env_file->bootscreen_resolution=config.bootscreen_resolution;
   for(int i=0;i<7;i++)
      env_file->scsi_num[i]=config.scsi_num[i];
//   for(int i=0;i<6;i++)
//      env_file->mac_address[i]=config.mac_address[i];
//   env_file->bp_ton=config.bp_ton;
//   env_file->bp_toff=config.bp_toff;

   return(write_env_files(env_file));
}
int write_env_files_preset(ENV_FILE_VARS *env_file)
{
//   env_file->preset_name;
   env_file->boot_mode=config.boot_mode;
   env_file->scsiboot=config.scsiboot;
   env_file->autoconfig_ram=config.autoconfig_ram;
   env_file->autoconfig_rtg=config.autoconfig_rtg;
   env_file->enable_test=config.enable_test;
   env_file->cpu_ram=config.cpu_ram;
   env_file->cpufreq=config.cpufreq;
   env_file->kickstart=config.kickstart;
   env_file->ext_kickstart=config.ext_kickstart;
   env_file->bootscreen_resolution=config.bootscreen_resolution;
   for(int i=0;i<7;i++)
      env_file->scsi_num[i]=config.scsi_num[i];
   for(int i=0;i<6;i++)
      env_file->mac_address[i]=config.mac_address[i];
   env_file->bp_ton=config.bp_ton;
   env_file->bp_toff=config.bp_toff;

   return(write_env_files(env_file));
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
                          /*printf("[Config] Can't Delete file " X "\n");*/\
                       }

   DELETE_FILE("env/bootmode")
   DELETE_FILE("env/scsiboot");
   DELETE_FILE("env/autoconfig_ram");
   DELETE_FILE("env/autoconfig_rtg");
   DELETE_FILE("env/cpu_ram");
   DELETE_FILE("env/cpufreq");
   DELETE_FILE("env/kickstart");
   DELETE_FILE("env/ext_kickstart");
   DELETE_FILE("env/scsi");
   DELETE_FILE("env/enabletest");
   DELETE_FILE("env/bootscres");
   DELETE_FILE("env/macaddress");

   usleep(10000);
   f_mount(NULL, Path, 1); // NULL unmount, 1 immediately
   Xil_ExceptionEnable();
   usleep(10000);
   return(1);
}
int delete_selected_preset(void)
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

   char file[50];
   sprintf(file,DEFAULT_ROOT "presets/preset%d.txt",preset_selected);

   ret=f_unlink(file);
   if(ret==FR_OK)
   {
      printf("[Config] Delete file %s\n",file);
      f_close(&fil);
   }
   else
   {
      printf("[Config] Can't Delete file %s\n",file);
      return(0);
   }
   usleep(10000);
   f_mount(NULL, Path, 1); // NULL unmount, 1 immediately
   Xil_ExceptionEnable();
   usleep(10000);
   return(1);
}
