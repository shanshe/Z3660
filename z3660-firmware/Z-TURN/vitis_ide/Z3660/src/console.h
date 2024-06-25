#ifndef _CONSOLE_H_
#define _CONSOLE_H_
#include <stdio.h>
#define MAX_HIST 20
typedef struct {
	char cmd_buf[100];
	char subcmd1_buf[100];
	char subcmd2_buf[100];
	char cmd_hist[MAX_HIST][100];
	uint8_t hist_pointer;
	uint8_t hist_pointer_top;
	uint32_t data1;
	uint32_t data2;
	uint8_t cmd_pointer;
	uint8_t debug_rtg;
	uint8_t debug_scsi;
	uint8_t debug_audio;
	uint8_t debug_i2c;
	uint8_t cmd;
	uint8_t subcmd;
	uint8_t step;
	uint8_t stop_i2c;
} CONSOLE;
void console_loop(void);
void console_init(void);

#endif // _CONSOLE_H_
