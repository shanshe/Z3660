#ifndef _CONSOLE_H_
#define _CONSOLE_H_
#include <stdio.h>
#define MAX_HIST 20
typedef struct {
	uint8_t clk;
	int phase;
	uint8_t dutycycle;
} CLOCK;
typedef struct {
	uint8_t clk_index;
	CLOCK AXICLK;
	CLOCK PCLK;
	CLOCK CLKEN;
	CLOCK BCLK;
	CLOCK CPUCLK;
	CLOCK CLK90;
} CLOCKS;
typedef struct {
	char cmd_buf[100];
	char subcmd1_buf[100];
	char subcmd2_buf[100];
	char cmd_hist[MAX_HIST][100];
	uint8_t hist_pointer;
	uint8_t hist_pointer_top;
	uint32_t data1;
	uint32_t data2;
	int32_t sdata1;
	uint8_t cmd_pointer;
	uint8_t debug_rtg;
	uint8_t debug_scsi;
	uint8_t debug_audio;
	uint8_t debug_soft3d;
	uint8_t debug_i2c;
	uint8_t cmd;
	uint8_t subcmd;
	uint8_t step;
	uint8_t stop_i2c;
	uint8_t toggle_read_burst;
	uint8_t toggle_write_burst;
	uint8_t reset_cpld;
	uint8_t reset_fpga;
	CLOCKS clocks;
} DEBUG_CONSOLE;
void debug_console_loop(void);
void debug_console_init(void);

#endif // _CONSOLE_H_
