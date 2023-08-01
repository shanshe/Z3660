/*
 * config_file.c
 *
 *  Created on: 28 feb. 2023
 *      Author: shanshe
 */
#include "mpg/ff.h"
#include <stdarg.h>
#include <stdio.h>
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
		"scsiboot",
		"scsi0",
		"scsi1",
		"scsi2",
		"scsi3",
		"scsi4",
		"scsi5",
		"scsi6",
		"autoconfig_ram",
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
	sprintf(config.kickstart,"A4kOS321.rom");
	for(int i=0;i<7;i++)
		sprintf(config.scsi[i],"");
}
void write_config_file(char *filename)
{
	static FIL fil;		/* File object */
	f_open(&fil,filename, FA_OPEN_ALWAYS | FA_WRITE);
	load_default_config();
	UINT NumBytesWritten;
	print_line(&fil,"## Z3660 config file ##\n");
	print_line(&fil,"# Warning: File names have a limit of 8 characters (+ 3 characters for extension)\n");
	print_line(&fil,"\n");
	print_line(&fil,"##General Configuration\n");
	print_line(&fil,"\n");
	print_line(&fil,"# Select boot mode: \"CPU\" for 060 CPU, \"MUSASHI\" \"UAE\" or \"UAEJIT\" for emulator\n");
	print_line(&fil,"#bootmode CPU\n");
	print_line(&fil,"#bootmode MUSASHI\n");
	print_line(&fil,"#bootmode UAE\n");
	print_line(&fil,"bootmode UAEJIT\n");
	print_line(&fil,"\n");
	print_line(&fil,"## Emulation Configuration\n");
	print_line(&fil,"\n");
	print_line(&fil,"# Select your kickstart file to map it on ARM's internal RAM, or comment lines to use installed Kickstart on your Amiga\n");
	print_line(&fil,"#kickstart DiagROM.rom\n");
	print_line(&fil,"#kickstart A4kOS31.rom\n");
	print_line(&fil,"#kickstart A4kOS321.rom\n");
	print_line(&fil,"kickstart A4kOS322.rom\n");
	print_line(&fil,"\n");
	print_line(&fil,"# Load scsi ROM on boot (boot from SCSI hdf files on SD)\n");
	print_line(&fil,"# (YES or NO, in capitals)\n");
	print_line(&fil,"scsiboot NO\n");
	print_line(&fil,"#scsiboot YES\n");
	print_line(&fil,"\n");
	print_line(&fil,"# Select your hdf files (from scsi0 to scsi6)\n");
	print_line(&fil,"#scsi0 hdf/A4000.hdf\n");
	print_line(&fil,"#scsi1 hdf/ZDH0.hdf\n");
	print_line(&fil,"#scsi2 hdf/Programs.hdf\n");
	print_line(&fil,"\n");
	print_line(&fil,"# Autoconfig RAM Enable (256 MB Zorro III RAM)\n");
	print_line(&fil,"# (YES or NO, in capitals)\n");
	print_line(&fil,"autoconfig_ram NO\n");
	print_line(&fil,"#autoconfig_ram YES\n");
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

void read_config_file(void)
{
	char Filename[]="z3660cfg.txt";
	static FIL fil;		/* File object */
	static FATFS fatfs;

	TCHAR *Path = "0:/";

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
		printf("Error opening file \"%s\"\nCreating default file...\n",Filename);
		write_config_file(Filename);
		ret=f_open(&fil,Filename, FA_OPEN_EXISTING | FA_READ);
		if(ret!=0)
		{
			printf("Error opening config file %s\nHALT!!!\n",Filename);
			while(1);
		}
	}
	unsigned int NumBytesRead;
	printf("Reading %s file \n",Filename);
	int cur_line = 1;
	char parse_line[512];
	char cur_cmd[128];
	memset(&config, 0x00, sizeof(CONFIG));
	sprintf(config.kickstart,"");

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
		//	      case CONFITEM_CPUTYPE:
		//	        cfg->cpu_type = get_m68k_cpu_type(parse_line + str_pos);
		//	        break;
		case CONFITEM_BOOTMODE:
			get_next_string(parse_line, cur_cmd, &str_pos, ' ');
			config.boot_mode=get_bootmode_type(cur_cmd);
			printf("[CFG] Boot mode %s.\n", bootmode_names[config.boot_mode]);
			shared->cfg_emu=config.boot_mode;
			shared->jit_enabled=config.boot_mode==UAEJIT?1:0;
			break;

		case CONFITEM_KICKSTART:
			get_next_string(parse_line, cur_cmd, &str_pos, ' ');
			sprintf(config.kickstart, cur_cmd);
			printf("[CFG] Kickstart file %s.\n", config.kickstart);
			break;

		case CONFITEM_SCSI_BOOT_ENABLE:
			get_next_string(parse_line, cur_cmd, &str_pos, ' ');
			config.scsiboot=get_yesno_type(cur_cmd);
			printf("[CFG] SCSI Boot %s.\n", yesno_names[config.scsiboot]);
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
			sprintf(config.scsi[index], cur_cmd);
			printf("[CFG] scsi%d file %s\n", index, config.scsi[index]);
			break;
		}

		case CONFITEM_AUTOCONFIG_RAM_ENABLE:
			get_next_string(parse_line, cur_cmd, &str_pos, ' ');
			config.autoconfig_ram=get_yesno_type(cur_cmd);
			printf("[CFG] AutoConfig RAM %s.\n", yesno_names[config.autoconfig_ram]);
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

load_failed:;
	printf("Error loading config file %s\n",Filename);
	printf("Loading default config\n",Filename);
	load_default_config();
load_successful:;

	f_close(&fil);
	f_mount(NULL, Path, 0); // NULL unmount, 0 delayed
	printf("Config file read OK\n");

	Xil_ExceptionEnable();
}
void read_env_files(void)
{
	static FIL fil;		/* File object */
	static FATFS fatfs;

	TCHAR *Path = "0:/";

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

	ret=f_open(&fil,"env/bootmode", FA_OPEN_EXISTING | FA_READ);
	if(ret==0)
	{
		unsigned int NumBytesRead;
//		int cur_line = 1;
		char parse_line[512];
		char cur_cmd[128];
		int str_pos = 0;
		memset(parse_line, 0x00, 512);
		f_gets(parse_line, (s32)512, &fil);
		get_next_string(parse_line, cur_cmd, &str_pos, '\n');
		config.boot_mode=get_bootmode_type(cur_cmd);
		printf("[ENV] Boot mode %s.\n", bootmode_names[config.boot_mode]);
		shared->cfg_emu=config.boot_mode;
		shared->jit_enabled=config.boot_mode==UAEJIT?1:0;
	}
	f_close(&fil);

	ret=f_open(&fil,"env/scsiboot", FA_OPEN_EXISTING | FA_READ);
	if(ret==0)
	{
		unsigned int NumBytesRead;
//		int cur_line = 1;
		char parse_line[512];
		char cur_cmd[128];
		int str_pos = 0;
		memset(parse_line, 0x00, 512);
		f_gets(parse_line, (s32)512, &fil);
		get_next_string(parse_line, cur_cmd, &str_pos, '\n');
		config.scsiboot=get_yesno_type(cur_cmd);
		printf("[ENV] SCSI Boot %s.\n", yesno_names[config.scsiboot]);
	}

	ret=f_open(&fil,"env/autoconfig_ram", FA_OPEN_EXISTING | FA_READ);
	if(ret==0)
	{
		unsigned int NumBytesRead;
//		int cur_line = 1;
		char parse_line[512];
		char cur_cmd[128];
		int str_pos = 0;
		memset(parse_line, 0x00, 512);
		f_gets(parse_line, (s32)512, &fil);
		get_next_string(parse_line, cur_cmd, &str_pos, '\n');
		config.autoconfig_ram=get_yesno_type(cur_cmd);
		printf("[ENV] AutoConfig Ram %s.\n", yesno_names[config.autoconfig_ram]);
	}
	f_close(&fil);


	f_mount(NULL, Path, 0); // NULL unmount, 0 delayed
	Xil_ExceptionEnable();
}
void write_env_files(int bootmode, int scsiboot, int autoconfig_ram)
{
	static FIL fil;		/* File object */
	static FATFS fatfs;

	TCHAR *Path = "0:/";

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

	ret=f_open(&fil,"env/bootmode", FA_CREATE_ALWAYS | FA_WRITE);
	if(ret==0)
	{
		f_printf(&fil,"%s\n",bootmode_names[bootmode]);
	}
	f_close(&fil);

	ret=f_open(&fil,"env/scsiboot", FA_CREATE_ALWAYS | FA_WRITE);
	if(ret==0)
	{
		f_printf(&fil,"%s\n",yesno_names[scsiboot]);
	}
	f_close(&fil);

	ret=f_open(&fil,"env/autoconfig_ram", FA_CREATE_ALWAYS | FA_WRITE);
	if(ret==0)
	{
		f_printf(&fil,"%s\n",yesno_names[autoconfig_ram]);
	}
	f_close(&fil);

	usleep(10000);
	f_mount(NULL, Path, 1); // NULL unmount, 1 immediately
	Xil_ExceptionEnable();
	usleep(10000);
}
