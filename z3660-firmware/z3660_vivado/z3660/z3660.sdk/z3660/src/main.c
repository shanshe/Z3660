/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xgpiops.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xscugic.h"

#include "sleep.h"
#include "xil_cache.h"
#include "xil_cache_l.h"
#include "xil_exception.h"
#include "xclk_wiz.h"
#include "xadcps.h"
#include "xtime_l.h"
#include "xuartps.h"

#include "ff.h"

#include "xaxivdma.h"
#include "sii9022_init/sii9022_init.h"
#include "xil_types.h"
#include "vga_modes.h"

#include "xvtc.h"
#include "sleep.h"
#include "xil_printf.h"

#include "rtg/gfx.h"
#include "main.h"

#define DEBUG(...)
//#define DEBUG printf

//#define LED_HDD

XGpioPs GpioPs;
XGpioPs_Config *GpioPsConfigPtr;
#define LED2          0 // MIO  0
#define CPLD_RAM_EN   9 // MIO  9
#define n040RSTI     10 // MIO 10
//#define CPLD_RAM_EN  14 // MIO 14 used by CAN RX !!!!
#define CPLD_RESET   15 // MIO 15
#define LED1         11 // MIO 11 (Z3660's green led)
#define USER_SW1     50 // MIO 50
XGpio Gpio;
XGpio_Config *GpioConfigPtr;
#define FPGA_RAM_EN              (1L<< 0)  // AXI GPIO  0
#define FPGA_RAM_BURST_READ_EN   (1L<< 1)  // AXI GPIO  1
#define FPGA_RAM_BURST_WRITE_EN  (1L<< 2)  // AXI GPIO  2
#define FPGA_TSCONDITION1        (1L<< 4)  // AXI GPIO  6..4
#define FPGA_ENCONDITION_BCLK    (3L<< 8)  // AXI GPIO  9..8
#define FPGA_ENABLE_256MB        (1L<<10)  // AXI GPIO 12
#define FPGA_RESET               (1L<<31)  // AXI GPIO 31


#define ENABLE_TSCONDITION1 do{	XGpio_DiscreteSet(&Gpio, 1, FPGA_TSCONDITION1);\
						}while(0)
#define ENABLE_RAM_FPGA do{	XGpioPs_WritePin(&GpioPs, CPLD_RAM_EN, 1);\
							XGpio_DiscreteSet(&Gpio, 1, FPGA_RAM_EN);\
						}while(0)
#define DISABLE_RAM_FPGA do{XGpioPs_WritePin(&GpioPs, CPLD_RAM_EN, 0);\
							XGpio_DiscreteClear(&Gpio, 1, FPGA_RAM_EN);\
						}while(0)
#define ENABLE_BURST_READ_RAM_FPGA do{XGpio_DiscreteSet(&Gpio, 1, FPGA_RAM_BURST_READ_EN);\
						}while(0)
#define DISABLE_BURST_READ_RAM_FPGA do{XGpio_DiscreteClear(&Gpio, 1, FPGA_RAM_BURST_READ_EN);\
						}while(0)

#define ENABLE_BURST_WRITE_RAM_FPGA do{XGpio_DiscreteSet(&Gpio, 1, FPGA_RAM_BURST_WRITE_EN);\
						}while(0)
#define DISABLE_BURST_WRITE_RAM_FPGA do{XGpio_DiscreteClear(&Gpio, 1, FPGA_RAM_BURST_WRITE_EN);\
						}while(0)

#define ENABLE_256MB_RAM_FPGA do{XGpio_DiscreteSet(&Gpio, 1, FPGA_ENABLE_256MB);\
						}while(0)

// kick32_A4000.rom
// kick314_A4000_R2s.rom
static FIL fil;		/* File object */
static FATFS fatfs;

//u8 DestinationAddress[10*1024*1024] __attribute__ ((aligned(32)));
//u8 SourceAddress[10*1024*1024] __attribute__ ((aligned(32)));

#define TEST 7
/*
//
// To test logical drive 0, FileName should be "0:/<File name>" or
// "<file_name>". For logical drive 1, FileName should be "1:/<file_name>"
//
static char FileName[32] = "Test.bin";
static char *SD_File;
int FfsSdPolledExample(void)
{
	FRESULT Res;
	UINT NumBytesRead;
	UINT NumBytesWritten;
	u32 BuffCnt;
//	BYTE work[FF_MAX_SS];
	u32 FileSize = (8*1024*1024);


	// To test logical drive 0, Path should be "0:/"
	// For logical drive 1, Path should be "1:/"
	//
	TCHAR *Path = "0:/";

	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		SourceAddress[BuffCnt] = TEST + BuffCnt;
	}

	//
	// Register volume work area, initialize device
	//
	Res = f_mount(&fatfs, Path, 0);

	if (Res != FR_OK) {
		return XST_FAILURE;
	}

	//
	//  Path - Path to logical driver, 0 - FDISK format.
	//  0 - Cluster size is automatically determined based on Vol size.
	//

//	Res = f_mkfs(Path, FM_FAT32, 0, work, sizeof work);
//	if (Res != FR_OK) {
//		return XST_FAILURE;
//	}

	//
	// Open file with required permissions.
	// Here - Creating new file with read/write permissions. .
	// To open file with write permissions, file system should not
	// be in Read Only mode.
	//
	SD_File = (char *)FileName;

	Res = f_open(&fil, SD_File, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if (Res) {
		return XST_FAILURE;
	}

	//
	// Pointer to beginning of file .
	//
	Res = f_lseek(&fil, 0);
	if (Res) {
		return XST_FAILURE;
	}

	//
	// Write data to file.
	//
	Res = f_write(&fil, (const void*)SourceAddress, FileSize,
			&NumBytesWritten);
	if (Res) {
		return XST_FAILURE;
	}

	//
	// Pointer to beginning of file .
	//
	Res = f_lseek(&fil, 0);
	if (Res) {
		return XST_FAILURE;
	}

	//
	// Read data from file.
	//
	Res = f_read(&fil, (void*)DestinationAddress, FileSize,
			&NumBytesRead);
	if (Res) {
		return XST_FAILURE;
	}

	//
	// Data verification
	//
	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		if(SourceAddress[BuffCnt] != DestinationAddress[BuffCnt]){
			return XST_FAILURE;
		}
	}

	//
	// Close file.
	//
	Res = f_close(&fil);
	if (Res) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
*/
//char Filename[]="Test.hdf";
//char Filename[]="hd0.hdf";
//char Filename[]="hd0.img";
/*
void CreateHdf(void)
{
	TCHAR *Path = "0:/";
	u32 BufferSize = (10*1024*1024);

	//
	// Register volume work area, initialize device
	//
	for(int32_t BuffCnt = 0; BuffCnt < BufferSize; BuffCnt++){
		SourceAddress[BuffCnt] = 0;
	}
	f_mount(&fatfs, Path, 0);
	f_open(&fil, Filename, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	UINT len;
	for(int i=0;i<10;i++)
		f_write(&fil, (const void*)SourceAddress, BufferSize, &len);
	f_close(&fil);
}

//static FIL fil;		// File object
//static FATFS fatfs;
void PrepareHdf(void)
{
	TCHAR *Path = "0:/";

	f_mount(&fatfs, Path, 0);
	set_hard_drive_image_file_amiga(0,Filename);
}
*/
static XClk_Wiz clkwiz0,clkwiz1;
static XClk_Wiz_Config conf0,conf1;
void print_clkinfo(char * str,u32 base,u32 address)
{
	int temp=XClk_Wiz_ReadReg(base, 0x200);
	float divider=((temp>>8)&0xff)*200/(temp&0xff);
	temp=XClk_Wiz_ReadReg(base, address);
	float clk=divider/temp;
	temp=XClk_Wiz_ReadReg(base, address+4);
	float clk_phase=temp/1000.;
	temp=XClk_Wiz_ReadReg(base, address+8);
	float clk_dc=temp/1000.;
	printf("%s %06.2f MHz, PHASE %06.2f degrees, DC %06.2f%c\n\r",str,clk,clk_phase,clk_dc,'%');

}
void configure_clk(int clk, int busclk, int verbose)
{
	XClk_Wiz_CfgInitialize(&clkwiz0, &conf0, XPAR_CLK_WIZ_0_BASEADDR);
	XClk_Wiz_CfgInitialize(&clkwiz1, &conf1, XPAR_CLK_WIZ_1_BASEADDR);

/*	if(busclk==0) // means, that clk frequency is variable
	{
		int divider;
		if(clk<=0)
			clk=50;
		divider=1000/clk; // (40*200)/8/clk;

		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208,(u32)( divider));    //   Ah=10 -> 100 MHz PCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C,(u32)(   355*1000)); //                    FASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210,(u32)( 0x0000C350)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x214,(u32)( 0x00000028)); //  28h=40 ->  25 MHz BCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x218,(u32)(   350*1000)); //                    FASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x21C,(u32)( 0x0000C350)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x22C,(u32)( 0x00000028)); //  28h=40 ->  25 MHz CPUCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x230,(u32)(   170*1000)); //                FASE 180
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x234,(u32)( 0x0000C350)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x238,(u32)( 0x00000028)); //  28h=40 ->  25 MHz CLK90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x23C,(u32)(   260*1000)); //                FASE 270
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x240,(u32)( 0x0000C350)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x244,(u32)(  divider*2)); //  28h=40 ->  25 MHz nCLKEN
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x248,(u32)(  42.5*1000)); //                    FASE 45
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x24C,(u32)(    75*1000)); //                    DC 75%

	}
	else*/
	if((clk!=100)&&(clk!=50))
	{
		u32 clkbout_mult=clk/2; // it should be 8 bit integer, but it seems to be less than 64
		u32 divider=10;
		u32 divider2=30;

		if(clk&1)
			clkbout_mult=clkbout_mult|(500<<8);

		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200, (clkbout_mult<<8)|(divider));
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208,(u32)(          5)); //   5h= 5 -> 200 MHz AXI
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C,(u32)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x214,(u32)(         10)); //   Ah=10 -> 100 MHz PCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x218,(u32)(    10*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x21C,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x220,(u32)(   divider2)); //  1Eh=30 ->  50 MHz CLKEN
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x224,(u32)(    33*1000)); //                    PHASE 90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x228,(u32)(    66*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);

		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x200, (clkbout_mult<<8)|(divider));
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x208,(u32)( 0x0000001E)); //  28h=40 ->  25 MHz BCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x20C,(u32)(    20*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x210,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x214,(u32)(   divider2)); //  28h=40 ->  25 MHz BCLK2
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x218,(u32)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x21C,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x220,(u32)(   divider2)); //   Ah=10 -> 100 MHz CLK90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x224,(u32)(   290*1000)); //                    PHASE 180
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x228,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x22C,(u32)(   divider2)); //  14h=20 ->  50 MHz CPUCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x230,(u32)(   200*1000)); //                    PHASE 270
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x234,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x25C, 0x00000003);
	}
	else if((clk==100)&&(busclk==50))
	{
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200, 0x0000320A); // 32h=50 50*200 / 10 -> 1000MHz VCO
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208,(u32)( 0x00000005)); //   5h= 5 -> 200 MHz AXI
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C,(u32)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x214,(u32)( 0x0000000A)); //   Ah=10 -> 100 MHz PCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x218,(u32)(    30*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x21C,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x220,(u32)( 0x00000014)); //  14h=20 ->  50 MHz CLKEN
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x224,(u32)(    60*1000)); //                    PHASE 90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x228,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);
		int offset=10;
		int frec_bclk=25;
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x200, frec_bclk*2*256+10); // 32h=50 50*200 / 10 -> 1000MHz VCO
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x208,(u32)( 0x00000028)); //  28h=40 ->  25 MHz BCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x20C,(u32)(   (30+offset)*frec_bclk/25*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x210,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x214,(u32)( 0x00000028)); //  28h=40 ->  25 MHz BCLK2
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x218,(u32)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x21C,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x220,(u32)( 0x00000028)); //   Ah=10 -> 100 MHz CLK90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x224,(u32)(   (270+10+offset)*frec_bclk/25*1000)); //                    PHASE 180
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x228,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x22C,(u32)( 0x00000028)); //  14h=20 ->  50 MHz CPUCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x230,(u32)(   (180+10+offset)*frec_bclk/25*1000)); //                    PHASE 270
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x234,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x25C, 0x00000003);
	}
	else if((clk==100)&&(busclk==25))
	{
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200, 0x0000320A); // 32h=50 50*200 / 10 -> 1000MHz VCO
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208,(u32)( 0x00000005)); //   5h= 5 -> 200 MHz AXI
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C,(u32)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x214,(u32)( 0x0000000A)); //   Ah=10 -> 100 MHz PCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x218,(u32)(    30*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x21C,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x220,(u32)( 0x00000028)); //  28h=40 ->  25 MHz CLKEN
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x224,(u32)(    40*1000)); //                    PHASE 90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x228,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);

		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x200, 0x0000320A); // 32h=50 50*200 / 10 -> 1000MHz VCO
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x208,(u32)( 0x00000028)); //  28h=40 ->  25 MHz BCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x20C,(u32)(    50*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x210,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x214,(u32)( 0x00000028)); //  28h=40 ->  25 MHz BCLK2
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x218,(u32)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x21C,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x220,(u32)( 0x00000028)); //   Ah=10 -> 100 MHz CLK90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x224,(u32)(   280*1000)); //                    PHASE 180
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x228,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x22C,(u32)( 0x00000028)); //  14h=20 ->  50 MHz CPUCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x230,(u32)(   190*1000)); //                    PHASE 270
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x234,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x25C, 0x00000003);

		XGpio_DiscreteSet(&Gpio, 1, FPGA_ENCONDITION_BCLK);
	}
	else if((clk==50)&&(busclk==50))
	{
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200, 0x0000320A); // 32h=50 50*200 / 10 -> 1000MHz VCO
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208,(u32)( 0x00000005)); //   5h= 5 -> 200 MHz AXI
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C,(u32)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x214,(u32)( 0x00000014)); //  14h=20 ->  50 MHz PCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x218,(u32)(    30*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x21C,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x220,(u32)( 0x00000028)); //  28h=40 ->  25 MHz CLKEN
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x224,(u32)(    40*1000)); //                    PHASE 90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x228,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);

		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x200, 0x0000320A); // 32h=50 50*200 / 10 -> 1000MHz VCO
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x208,(u32)( 0x00000028)); //  28h=40 ->  25 MHz BCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x20C,(u32)(    50*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x210,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x214,(u32)( 0x00000028)); //  28h=40 ->  25 MHz BCLK2
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x218,(u32)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x21C,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x220,(u32)( 0x00000028)); //   Ah=10 -> 100 MHz CLK90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x224,(u32)(   280*1000)); //                    PHASE 180
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x228,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x22C,(u32)( 0x00000028)); //  14h=20 ->  50 MHz CPUCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x230,(u32)(   190*1000)); //                    PHASE 270
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x234,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x25C, 0x00000003);

		XGpio_DiscreteSet(&Gpio, 1, FPGA_ENCONDITION_BCLK);
	}
	else //if((clk==50)&&(busclk==25))
	{
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200, 0x0000320A); // 32h=50 50*200 / 10 -> 1000MHz VCO
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208,(u32)( 0x00000005)); //   5h= 5 -> 200 MHz AXI
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C,(u32)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x214,(u32)( 0x00000014)); //  14h=20 ->  50 MHz PCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x218,(u32)(    30*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x21C,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x220,(u32)( 0x00000028)); //  28h=40 ->  25 MHz CLKEN
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x224,(u32)(    50*1000)); //                    PHASE 90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x228,(u32)(    75*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);

		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x200, 0x0000320A); // 32h=50 50*200 / 10 -> 1000MHz VCO
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x208,(u32)( 0x00000028)); //  28h=40 ->  25 MHz BCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x20C,(u32)(    50*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x210,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x214,(u32)( 0x00000028)); //  28h=40 ->  25 MHz BCLK2
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x218,(u32)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x21C,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x220,(u32)( 0x00000028)); //   Ah=10 -> 100 MHz CLK90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x224,(u32)(   280*1000)); //                    PHASE 180
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x228,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x22C,(u32)( 0x00000028)); //  14h=20 ->  50 MHz CPUCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x230,(u32)(   190*1000)); //                    PHASE 270
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x234,(u32)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x25C, 0x00000003);

		XGpio_DiscreteSet(&Gpio, 1, FPGA_ENCONDITION_BCLK);
	}
	if(verbose)
	{
		print_clkinfo("AXICLK",XPAR_CLK_WIZ_0_BASEADDR,0x208);
		print_clkinfo("  PCLK",XPAR_CLK_WIZ_0_BASEADDR,0x214);
		print_clkinfo("_CLKEN",XPAR_CLK_WIZ_0_BASEADDR,0x220);

		print_clkinfo("  BCLK",XPAR_CLK_WIZ_1_BASEADDR,0x208);
		print_clkinfo("CPUCLK",XPAR_CLK_WIZ_1_BASEADDR,0x220);
		print_clkinfo(" CLK90",XPAR_CLK_WIZ_1_BASEADDR,0x22C);
	}
	//	int32_t a,b,c;
//	a=XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200);
//	b=XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208);
//	c=XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C);
//	printf("Clk config ... %lx %lx %lx\n\r",a,b,c);
}

void configure_gpio()
{
	GpioPsConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
	XGpioPs_CfgInitialize(&GpioPs, GpioPsConfigPtr, GpioPsConfigPtr->BaseAddr);

	XGpioPs_SetDirectionPin(&GpioPs, CPLD_RESET, 1);
	XGpioPs_SetOutputEnablePin(&GpioPs, CPLD_RESET, 1);
	XGpioPs_WritePin(&GpioPs, CPLD_RESET, 0);

	XGpioPs_SetDirectionPin(&GpioPs, n040RSTI, 0);
	XGpioPs_SetOutputEnablePin(&GpioPs, n040RSTI, 0);

	XGpioPs_SetDirectionPin(&GpioPs, USER_SW1, 0);
	XGpioPs_SetOutputEnablePin(&GpioPs, USER_SW1, 0);



	XGpioPs_SetDirectionPin(&GpioPs, CPLD_RAM_EN, 1);
	XGpioPs_SetOutputEnablePin(&GpioPs, CPLD_RAM_EN, 1);
	XGpioPs_WritePin(&GpioPs, CPLD_RAM_EN, 0);

	XGpioPs_SetDirectionPin(&GpioPs, LED1, 1);
	XGpioPs_SetOutputEnablePin(&GpioPs, LED1, 1);
	XGpioPs_WritePin(&GpioPs, LED1, 0);

	XGpioPs_SetDirectionPin(&GpioPs, LED2, 1);
	XGpioPs_SetOutputEnablePin(&GpioPs, LED2, 1);
	XGpioPs_WritePin(&GpioPs, LED2, 0);

	GpioConfigPtr = XGpio_LookupConfig(XPAR_AXI_GPIO_0_DEVICE_ID);
	XGpio_CfgInitialize(&Gpio, GpioConfigPtr, GpioConfigPtr->BaseAddress);


	XGpio_DiscreteClear(&Gpio, 1, FPGA_RAM_EN);
	XGpio_DiscreteClear(&Gpio, 1, FPGA_RAM_BURST_READ_EN);
	XGpio_DiscreteClear(&Gpio, 1, FPGA_RAM_BURST_WRITE_EN);
	XGpio_DiscreteSet(&Gpio, 1, FPGA_RESET);
	usleep(500);
	XGpio_DiscreteClear(&Gpio, 1, FPGA_RESET);
	printf("Configured GPIO...\n\r");
}
void fpga_ram_enable(int enable_ram)
{
	if(enable_ram)
	{
		ENABLE_RAM_FPGA;
		ENABLE_BURST_READ_RAM_FPGA;
		ENABLE_BURST_WRITE_RAM_FPGA;
		printf("FPGA RAM ENABLED\n\r");
	}
	else
	{
		DISABLE_RAM_FPGA;
		DISABLE_BURST_READ_RAM_FPGA;
		DISABLE_BURST_WRITE_RAM_FPGA;
		printf("FPGA RAM DISABLED\n\r");
	}
}
char Filename[]="Z3660.bin";
extern u32 *frameBuf;//[1920*1080*2];
void DemoPrintTest(u8 *frame, u32 width, u32 height, u32 stride, int pattern);
void update_z3660_screen(void)
{
	/*
	int8_t r,g,b;
	for(int i=720;i<1080;i++)
	{

		for(int j=0;j<1920;j++)
		{
			b=frameBuf[1][1920*4*i+4*j+1];
			g=frameBuf[1][1920*4*i+4*j+2];
			r=frameBuf[1][1920*4*i+4*j+3];

			framebuffer[1920*4*i+4*j+1]=r;
			framebuffer[1920*4*i+4*j+2]=g;
			framebuffer[1920*4*i+4*j+3]=b;

		}
	}
	Xil_DCacheFlushRange((unsigned int) frameBuf, DEMO_MAX_FRAME);
*/
}
int main()
{
	init_platform();

	configure_gpio();

	configure_clk(100,50,0);

	XGpioPs_WritePin(&GpioPs, LED1, 1);
	XGpio_DiscreteSet(&Gpio, 1, FPGA_RESET);
	XGpioPs_WritePin(&GpioPs, CPLD_RESET, 0);
	XGpioPs_WritePin(&GpioPs, LED2, 0);

	printf("\033[2J");
	printf("Z3660 starting...\n\r\n\r");
	printf(" ________   ______   ______   ______   ______ \n\r");
	printf("|___    /  |____  | |  ____| |  ____| |  __  | a.k.a. PishaStorm\n\r");
	printf("    /  /    ____| | | |____  | |____  | |  | | a.k.a. N\314\203Storm\n\r");
	printf("   /  /    |____  | |  __  | |  __  | | |  | |\n\r");
	printf("  /  /___   ____| | | |__| | | |__| | | |__| |\n\r");
	printf(" /_______| |______| |______| |______| |______|\n\r");
	printf("\n\r");

//	configure_clk( 60,0);
//	configure_clk( 50,25);
//	configure_clk(100,25);
//	configure_clk(100,50);

	int cpu_speed=3;

	int config=3;         // CPU initial state: 0 50/25, 1 50/50, 2 100/25, 3 100/50, 4 ...
	int enable_ram=1;     // RAM initial state


	fpga_ram_enable(enable_ram);
//	ENABLE_TSCONDITION1; si intentas acelerar no arranca o se congela en algún momento...

	//remove me
//	DISABLE_BURST_READ_RAM_FPGA;
//	DISABLE_BURST_WRITE_RAM_FPGA;

	if(((*(volatile u32 *)Gpio.BaseAddress)&2)==0)
		printf("Read Bursts DISABLED\n\r");
	else
		printf("Read Bursts ENABLED\n\r");
	if(((*(volatile u32 *)Gpio.BaseAddress)&4)==0)
		printf("Write Bursts DISABLED\n\r");
	else
		printf("Write Bursts ENABLED\n\r");
	int verbose=1;
	switch(config)
	{
		case 0:
			cpu_speed=0;
			configure_clk(50,25,verbose);
			XGpioPs_WritePin(&GpioPs, LED2, 1);
			break;
		case 1:
			cpu_speed=1;
			configure_clk(50,50,verbose);
			XGpioPs_WritePin(&GpioPs, LED2, 0);
			break;
		case 2:
			cpu_speed=2;
			configure_clk(100,25,verbose);
			XGpioPs_WritePin(&GpioPs, LED2, 1);
			break;
		case 3:
			cpu_speed=3;
			configure_clk(100,50,verbose);
			XGpioPs_WritePin(&GpioPs, LED2, 0);
			break;
	}

//    PrepareHdf();
//    InitGayle();

	TCHAR *Path = "0:/";
	f_mount(&fatfs, Path, 1); // 1 mount immediately
	f_open(&fil,Filename, FA_OPEN_ALWAYS | FA_READ);
	f_lseek(&fil, 4);
	UINT NumBytesRead;
	f_read(&fil, (void*)((u32)frameBuf), 1920*1080*4,&NumBytesRead);
	f_close(&fil);
	int8_t r,g,b;
	u32* framebuffer=(u32*)((u32)frameBuf);
	for(int i=0;i<1080;i++)
	{
		for(int j=0;j<1920;j++)
		{
			SWAP32(framebuffer[1920*i+j]);
		}
	}
	Xil_DCacheFlushRange((unsigned int) framebuffer, 1920*1080*4);

	XGpioPs_WritePin(&GpioPs, LED1, 1);
	XGpio_DiscreteSet(&Gpio, 1, FPGA_RESET);
	XGpioPs_WritePin(&GpioPs, CPLD_RESET, 0);
	usleep(2500);

	XGpioPs_WritePin(&GpioPs, LED1, 0);
	XGpio_DiscreteClear(&Gpio, 1, FPGA_RESET);

//	struct zz_video_mode *vmode = &preset_video_modes[mode];
//	pixelclock_init_2(mode);
//	hdmi_ctrl_init();
//	init_vdma(1920,1080, 1, 1, (u32)framebuffer);

//	IntcInitFunction(INTC_DEVICE_ID);

	reset_video(NO_RESET_FRAMEBUFFER);

#define M68K_RUNNING 0
#define M68K_RESET   1
	int state=M68K_RUNNING;
	int ret=0;
	int timer_counter_update_screen=0;
	int timer_counter_update_led=0;
	rtg_init();
	XGpioPs_WritePin(&GpioPs, CPLD_RESET, 1);

	while(1)
	{
		switch(state)
		{
		case M68K_RUNNING:
			loop2();
			rtg_loop();
			if(timer_counter_update_screen<=10000000)
			{
				timer_counter_update_screen++;
			}
			if(timer_counter_update_screen==10000000)
			{
				update_z3660_screen();
			}
			if(XGpioPs_ReadPin(&GpioPs, n040RSTI)==0)
			{
				printf("Reset active (DOWN)...\n\r");
				state=M68K_RESET;
			}
			else
			{
				int delay;
		//		XGpio_DiscreteClear(&Gpio, 1, FPGA_RESET);
				delay=(5-cpu_speed)*(5-cpu_speed)*20000;
				if(timer_counter_update_led<=2*delay)
				{
					timer_counter_update_led++;
				}
				if(timer_counter_update_led==delay)
				{
					XGpioPs_WritePin(&GpioPs, LED1, 0);
				}
				if(timer_counter_update_led==2*delay)
				{
					timer_counter_update_led=0;
					XGpioPs_WritePin(&GpioPs, LED1, 1);
				}
				if(XGpioPs_ReadPin(&GpioPs, USER_SW1)==0)
				{
					config++;
					config&=3;
					switch(config)
					{
						case 0:
							cpu_speed=0;
							configure_clk(50,25,verbose);
							XGpioPs_WritePin(&GpioPs, LED2, 1);
							break;
						case 1:
							cpu_speed=1;
							configure_clk(50,50,verbose);
							XGpioPs_WritePin(&GpioPs, LED2, 0);
							break;
						case 2:
							cpu_speed=2;
							configure_clk(100,25,verbose);
							XGpioPs_WritePin(&GpioPs, LED2, 1);
							break;
						case 3:
							cpu_speed=3;
							configure_clk(100,50,verbose);
							XGpioPs_WritePin(&GpioPs, LED2, 0);
							break;
					}
					while(XGpioPs_ReadPin(&GpioPs, USER_SW1)==0)
					{}
				}
			}
			break;
		case M68K_RESET:
			loop2();
			if(timer_counter_update_screen<=10000000)
			{
				timer_counter_update_screen++;
			}
			if(timer_counter_update_screen==10000000)
			{
				update_z3660_screen();
			}
			ret=XGpioPs_ReadPin(&GpioPs, n040RSTI);
			if(ret!=0)
			{
				state=M68K_RUNNING;
//				XGpio_DiscreteSet(&Gpio, 1, FPGA_RESET);
				XGpioPs_WritePin(&GpioPs, CPLD_RESET, 0);
				XGpio_DiscreteSet(&Gpio, 1, FPGA_RESET);
				int reset_counter=10;
				while(reset_counter>0)
				{
					usleep(25000);
					reset_counter--;
				}
				XGpio_DiscreteClear(&Gpio, 1, FPGA_RESET);
				reset_counter=10;
				while(reset_counter>0)
				{
					usleep(25000);
					reset_counter--;
				}
				// Initialize something here... ???
				XGpioPs_WritePin(&GpioPs, CPLD_RESET, 1);
				printf("Reset inactive (UP)...\n\r");
				reset_video(RESET_FRAMEBUFFER);
			}
			break;
		}

	}
	cleanup_platform();
	return 0;
}
