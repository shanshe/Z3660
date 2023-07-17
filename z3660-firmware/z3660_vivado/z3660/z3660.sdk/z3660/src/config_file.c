/*
 * config_file.c
 *
 *  Created on: 28 feb. 2023
 *      Author: shanshe
 */
#include "ff.h"
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
};
const char *bootmode_names[BOOTMODE_NUM] = {
		"CPU",
		"MUSASHI",
		"UAE",
		"UAEJIT",
};
const char *yesno_names[YESNO_NUM] = {
		"no",
		"yes",
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
	print_line(&fil,"Select your kickstart file to map it on ARM's internal RAM, or comment lines to use installed Kickstart on your Amiga\n");
	print_line(&fil,"#kickstart DiagROM.rom\n");
	print_line(&fil,"#kickstart A4kOS31.rom\n");
	print_line(&fil,"#kickstart A4kOS321.rom\n");
	print_line(&fil,"kickstart A4kOS322.rom\n");
	print_line(&fil,"\n");
	print_line(&fil,"# More config lines will follow...\n");

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
	ret=f_open(&fil,Filename, FA_OPEN_ALWAYS | FA_READ);
	if(ret!=0)
	{
//		printf("Error opening file \"%s\"\nCreating default file...\n",Filename);
//		write_config_file(Filename);
//		ret=f_open(&fil,Filename, FA_OPEN_ALWAYS | FA_READ);
//		if(ret!=0)
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

		switch (get_config_item_type(cur_cmd)) {
		//	      case CONFITEM_CPUTYPE:
		//	        cfg->cpu_type = get_m68k_cpu_type(parse_line + str_pos);
		//	        break;
		case CONFITEM_BOOTMODE:
			get_next_string(parse_line, cur_cmd, &str_pos, ' ');
			config.boot_mode=get_bootmode_type(cur_cmd);
			printf("[CFG] Boot mode %s.\n", bootmode_names[config.boot_mode]);
			if(config.boot_mode==UAEJIT)
				shared->cfg_emu=1;
			else if(config.boot_mode==UAE)
				shared->cfg_emu=2;
			else if(config.boot_mode==MUSASHI)
				shared->cfg_emu=3;
			else
				shared->cfg_emu=0;
			break;

		case CONFITEM_KICKSTART:
			get_next_string(parse_line, cur_cmd, &str_pos, ' ');
			sprintf(config.kickstart,cur_cmd);
			printf("[CFG] Kickstart file %s.\n", config.kickstart);
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
