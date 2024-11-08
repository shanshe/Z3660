#ifndef CONFIG_CLK_H
#define CONFIG_CLK_H

#include "xclk_wiz.h"

typedef struct {
	int divider;
	int phase;
	int dutycycle;
} clock_st;
typedef struct {
	int clk;
	int M;
	int D;
	clock_st axi;
	clock_st pclk;
	clock_st clken;
	clock_st bclk;
	clock_st cpuclk;
	clock_st clk90;
} clock_data;

void configure_clk(int clk, int verbose, int nbr);
unsigned int get_clock_index(int clk);

#endif // CONFIG_CLK_H
