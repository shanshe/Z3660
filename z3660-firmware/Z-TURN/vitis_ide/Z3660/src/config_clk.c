#include "config_clk.h"
#include "main.h"
#include <stdio.h>

XClk_Wiz clkwiz0;
XClk_Wiz_Config conf0;

int cpu_freq=-1;

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

clock_data cd[]={
{//100 MHz OK
		.clk            =100, .M            =  30, .D                =  5,
		.axi.divider    =  6, .axi.phase    =   0, .axi.dutycycle    = 50,
		.pclk.divider   = 12, .pclk.phase   =  35, .pclk.dutycycle   = 50,
		.clken.divider  = 24, .clken.phase  =  63, .clken.dutycycle  = 50,
		.bclk.divider   = 48, .bclk.phase   =  41, .bclk.dutycycle   = 50,
		.clk90.divider  = 48, .clk90.phase  = 270+41, .clk90.dutycycle  = 50,
		.cpuclk.divider = 48, .cpuclk.phase = 180+41, .cpuclk.dutycycle = 50},
{// 95 MHz +-
		.clk            = 95, .M            =  16, .D                =  1,
		.axi.divider    =  5, .axi.phase    =   0, .axi.dutycycle    = 50,
		.pclk.divider   = 10, .pclk.phase   =   5, .pclk.dutycycle   = 45,
		.clken.divider  = 20, .clken.phase  =  60, .clken.dutycycle  = 50,
		.bclk.divider   = 40, .bclk.phase   =   5, .bclk.dutycycle   = 50,
		.clk90.divider  = 40, .clk90.phase  = 270, .clk90.dutycycle  = 50,
		.cpuclk.divider = 40, .cpuclk.phase = 180, .cpuclk.dutycycle = 50},
{// 90 MHz OK
		.clk            = 90, .M            =  27, .D                =  5,
		.axi.divider    =  6, .axi.phase    =   0, .axi.dutycycle    = 50,
		.pclk.divider   = 12, .pclk.phase   =  30, .pclk.dutycycle   = 45,
		.clken.divider  = 24, .clken.phase  =  60, .clken.dutycycle  = 50,
		.bclk.divider   = 48, .bclk.phase   =  40, .bclk.dutycycle   = 45,
		.clk90.divider  = 48, .clk90.phase  = 270+40, .clk90.dutycycle  = 50,
		.cpuclk.divider = 48, .cpuclk.phase = 180+40, .cpuclk.dutycycle = 45},
{// 85 MHz OK
		.clk            = 85, .M            =  16, .D                =  1,
		.axi.divider    =  5, .axi.phase    =   0, .axi.dutycycle    = 50,
		.pclk.divider   = 10, .pclk.phase   =   5, .pclk.dutycycle   = 42,
		.clken.divider  = 20, .clken.phase  =  60, .clken.dutycycle  = 50,
		.bclk.divider   = 40, .bclk.phase   =   5, .bclk.dutycycle   = 50,
		.clk90.divider  = 40, .clk90.phase  = 270, .clk90.dutycycle  = 50,
		.cpuclk.divider = 40, .cpuclk.phase = 180, .cpuclk.dutycycle = 45},
{// 80 MHz OK
		.clk            = 80, .M            =  24, .D                =  5,
		.axi.divider    =  6, .axi.phase    =   0, .axi.dutycycle    = 50,
		.pclk.divider   = 12, .pclk.phase   =  30, .pclk.dutycycle   = 45,
		.clken.divider  = 24, .clken.phase  =  60+180, .clken.dutycycle  = 50,
		.bclk.divider   = 48, .bclk.phase   =  40, .bclk.dutycycle   = 50,
		.clk90.divider  = 48, .clk90.phase  = 270+40, .clk90.dutycycle  = 50,
		.cpuclk.divider = 48, .cpuclk.phase = 180+40, .cpuclk.dutycycle = 45},
{// 75 MHz OK
		.clk            = 75, .M            =  16, .D                =  1,
		.axi.divider    =  5, .axi.phase    =   0, .axi.dutycycle    = 50,
		.pclk.divider   = 10, .pclk.phase   =  30, .pclk.dutycycle   = 45,
		.clken.divider  = 20, .clken.phase  =  60, .clken.dutycycle  = 50,
		.bclk.divider   = 40, .bclk.phase   =  10, .bclk.dutycycle   = 50,
		.clk90.divider  = 40, .clk90.phase  = 270, .clk90.dutycycle  = 50,
		.cpuclk.divider = 40, .cpuclk.phase = 180, .cpuclk.dutycycle = 50},
{// 70 MHz OK
		.clk            = 70, .M            =  28, .D                =  5,
		.axi.divider    =  8, .axi.phase    =   0, .axi.dutycycle    = 50,
		.pclk.divider   = 16, .pclk.phase   =  30, .pclk.dutycycle   = 45,
		.clken.divider  = 32, .clken.phase  =  60, .clken.dutycycle  = 50,
		.bclk.divider   = 64, .bclk.phase   =  40, .bclk.dutycycle   = 50,
		.clk90.divider  = 64, .clk90.phase  = 270+40, .clk90.dutycycle  = 50,
		.cpuclk.divider = 64, .cpuclk.phase = 180+40, .cpuclk.dutycycle = 45},
{// 65 MHz OK
		.clk            = 65, .M            =  26, .D                =  5,
		.axi.divider    =  8, .axi.phase    =   0, .axi.dutycycle    = 50,
		.pclk.divider   = 16, .pclk.phase   =  35, .pclk.dutycycle   = 45,
		.clken.divider  = 32, .clken.phase  =  69, .clken.dutycycle  = 50,
		.bclk.divider   = 32, .bclk.phase   =  12, .bclk.dutycycle   = 45,
		.clk90.divider  = 32, .clk90.phase  = 311, .clk90.dutycycle  = 50,
		.cpuclk.divider = 32, .cpuclk.phase = 207, .cpuclk.dutycycle = 50},
{// 60 MHz OK
		.clk            = 60, .M            = 120, .D                = 25,
		.axi.divider    =  8, .axi.phase    =   0, .axi.dutycycle    = 50,
		.pclk.divider   = 16, .pclk.phase   =  35, .pclk.dutycycle   = 50,
		.clken.divider  = 32, .clken.phase  =  70, .clken.dutycycle  = 50,
		.bclk.divider   = 32, .bclk.phase   =  11, .bclk.dutycycle   = 45,
		.clk90.divider  = 32, .clk90.phase  = 297, .clk90.dutycycle  = 50,
		.cpuclk.divider = 32, .cpuclk.phase = 198, .cpuclk.dutycycle = 45},
{// 55 MHz --
		.clk            = 55, .M            =  21, .D                =  5,
		.axi.divider    =  8, .axi.phase    =   0, .axi.dutycycle    = 50,
		.pclk.divider   = 16, .pclk.phase   =  30, .pclk.dutycycle   = 50,
		.clken.divider  = 32, .clken.phase  =  70, .clken.dutycycle  = 50,
		.bclk.divider   = 32, .bclk.phase   =  10, .bclk.dutycycle   = 50,
		.clk90.divider  = 32, .clk90.phase  = 270, .clk90.dutycycle  = 50,
		.cpuclk.divider = 32, .cpuclk.phase = 180, .cpuclk.dutycycle = 50},
{// 50 MHz OK
		.clk            = 50, .M            =   8, .D                =  1,
		.axi.divider    = 16, .axi.phase    =   0, .axi.dutycycle    = 50,
		.pclk.divider   = 32, .pclk.phase   =  30, .pclk.dutycycle   = 50,
		.clken.divider  = 64, .clken.phase  =  60, .clken.dutycycle  = 50,
		.bclk.divider   = 64, .bclk.phase   =  10, .bclk.dutycycle   = 50,
		.clk90.divider  = 64, .clk90.phase  = 270, .clk90.dutycycle  = 50,
		.cpuclk.divider = 64, .cpuclk.phase = 180, .cpuclk.dutycycle = 50},
};

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
   printf("%s %06.2f MHz, PHASE %06.2f degrees, DC %06.2f%c\n\r",str,clk,clk_phase,clk_dc,'%');

}
uint32_t clken=1;
void configure_clk(int clk, int busclk, int verbose, int nbr)
{
#ifndef CPU_FIXED_FREQUENCY
   if(clk>100)
      clk=100;
   else if(clk<50)
      clk=50;

// unstable frequencies: mapped to stable ones
   unsigned int ind;
   if(clk>95) ind=0;
   else if(clk>90) ind=1;
   else if(clk>85) ind=2;
   else if(clk>80) ind=3;
   else if(clk>75) ind=4;
   else if(clk>70) ind=5;
   else if(clk>65) ind=6;
   else if(clk>60) ind=7;
   else if(clk>55) ind=8;
   else if(clk>50) ind=9;
   else ind=10;
   clk=cd[ind].clk;

//   if(cpu_freq==clk)
//	   return;

   cpu_freq=clk;

   printf("Clock index %d\n",ind);

   XClk_Wiz_CfgInitialize(&clkwiz0, &conf0, XPAR_CLK_WIZ_0_BASEADDR);
/*
   uint32_t clkbase_remainder=((cd[ind].clkbase&1)*500L)<<16;
   uint32_t clkbase=(cd[ind].clkbase<<7)&0xFF00; // clkbase is divided by 2, because the result must be under 64 !!!!
*/
   uint32_t clkbase_remainder=0;
   uint32_t M=((uint32_t)cd[ind].M)*256;
   uint32_t D=cd[ind].D;

   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200, (clkbase_remainder)+ M + D); // M=8 D=1 -> 8*200 / 1 -> 1600MHz VCO
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208,(uint32_t)( cd[ind].axi.divider          )); //   5h= 5 -> 200 MHz AXI
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C,(uint32_t)( cd[ind].axi.phase       *1000)); //                    PHASE 0
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210,(uint32_t)( cd[ind].axi.dutycycle   *1000)); //                    DC 50%
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x214,(uint32_t)( cd[ind].pclk.divider         )); //   Ah=10 -> 100 MHz PCLK
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x218,(uint32_t)( cd[ind].pclk.phase      *1000)); //                    PHASE 30
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x21C,(uint32_t)( cd[ind].pclk.dutycycle  *1000)); //                    DC 50%
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x220,(uint32_t)( cd[ind].clken.divider        )); //  14h=20 ->  50 MHz CLKEN
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x224,(uint32_t)( cd[ind].clken.phase     *1000)); //                    PHASE 60
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x228,(uint32_t)( cd[ind].clken.dutycycle *1000*clken)); //              DC 50%

   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x22C,(uint32_t)( cd[ind].bclk.divider         )); //  28h=40 ->  25 MHz BCLK
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x230,(uint32_t)( cd[ind].bclk.phase      *1000)); //                    PHASE 40
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x234,(uint32_t)( cd[ind].bclk.dutycycle  *1000)); //                    DC 50%
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x238,(uint32_t)( cd[ind].clk90.divider        )); //   Ah=10 -> 100 MHz CLK90
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x23C,(uint32_t)( cd[ind].clk90.phase     *1000)); //                    PHASE 290
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x240,(uint32_t)( cd[ind].clk90.dutycycle *1000)); //                    DC 50%
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x244,(uint32_t)( cd[ind].cpuclk.divider       )); //  14h=20 ->  50 MHz CPUCLK
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x248,(uint32_t)( cd[ind].cpuclk.phase    *1000)); //                    PHASE 200
   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x24C,(uint32_t)( cd[ind].cpuclk.dutycycle*1000)); //                    DC 50%
   if(nbr)
   {
      NBR_ARM(0);
      usleep(1000000);
   }

   XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);

   while(XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x004)==0);

   if(nbr)
   {
	   usleep(1000000);
	   NBR_ARM(1);
   }
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
      print_clkinfo("CPUCLK",XPAR_CLK_WIZ_0_BASEADDR,0x238);
      print_clkinfo(" CLK90",XPAR_CLK_WIZ_0_BASEADDR,0x244);
   }
}
