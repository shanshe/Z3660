#ifndef CONFIG_CLK_H
#define CONFIG_CLK_H

#include "xclk_wiz.h"

typedef struct {
	int divider;
	int phase;
	int dutycycle;
} clock;
typedef struct {
	int clk;
	int M;
	int D;
	clock axi;
	clock pclk;
	clock clken;
	clock bclk;
	clock clk90;
	clock cpuclk;
} clock_data;

void configure_clk(int clk, int verbose, int nbr);
unsigned int get_clock_index(int clk);

#endif // CONFIG_CLK_H
