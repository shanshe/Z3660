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
#include "main.h"
#include "sleep.h"
#include "xil_misc_psreset_api.h"
#include "config_file.h"
#include "scsi/scsi.h"

#ifdef CPU_EMULATOR
#include "cpu_emulator.h"
#endif
extern SHARED *shared;
extern int boot_rom_loaded;
void hard_reboot(void);
void z3660_printf(const TCHAR *format, __VALIST args)
{
	while(shared->uart_semaphore!=0);
	shared->uart_semaphore=1;
	vprintf(format, args);
	shared->uart_semaphore=0;
}
void init_shared(void);
#define DEBUG(...)
//#define DEBUG printf

//#define LED_HDD

XGpioPs GpioPs;
XGpioPs_Config *GpioPsConfigPtr;
//XGpio Gpio;
//XGpio_Config *GpioConfigPtr;


#define ENABLE_TSCONDITION1 do{	DiscreteSet(REG0, FPGA_TSCONDITION1);\
						}while(0)
#define ENABLE_CPU_RAM_FPGA do{	DiscreteSet(REG0, FPGA_RAM_EN);\
						}while(0)
#define DISABLE_CPU_RAM_FPGA do{ DiscreteClear(REG0, FPGA_RAM_EN);\
						}while(0)
#define ENABLE_BURST_READ_FPGA do{DiscreteSet(REG0, FPGA_RAM_BURST_READ_EN);\
						}while(0)
#define DISABLE_BURST_READ_FPGA do{DiscreteClear(REG0, FPGA_RAM_BURST_READ_EN);\
						}while(0)

#define ENABLE_BURST_WRITE_FPGA do{DiscreteSet(REG0, FPGA_RAM_BURST_WRITE_EN);\
						}while(0)
#define DISABLE_BURST_WRITE_FPGA do{DiscreteClear(REG0, FPGA_RAM_BURST_WRITE_EN);\
						}while(0)

#define ENABLE_256MB_AUTOCONFIG do{DiscreteSet(REG0, FPGA_256MB_AUTOCONFIG_EN);\
						}while(0)

#define ENABLE_RTG_AUTOCONFIG do{DiscreteSet(REG0, FPGA_RTG_AUTOCONFIG_EN);\
						}while(0)

#define DISABLE_256MB_AUTOCONFIG do{DiscreteClear(REG0, FPGA_256MB_AUTOCONFIG_EN);\
						}while(0)

#define DISABLE_RTG_AUTOCONFIG do{DiscreteClear(REG0, FPGA_RTG_AUTOCONFIG_EN);\
						}while(0)

//uint8_t DestinationAddress[10*1024*1024] __attribute__ ((aligned(32)));
//uint8_t SourceAddress[10*1024*1024] __attribute__ ((aligned(32)));

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
	uint32_t BuffCnt;
//	BYTE work[FF_MAX_SS];
	uint32_t FileSize = (8*1024*1024);


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
	uint32_t BufferSize = (10*1024*1024);

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
ZZ_VIDEO_STATE vs;
XAxiVdma vdma;
XClk_Wiz clkwiz0,clkwiz1;
XClk_Wiz clkwiz;
XClk_Wiz_Config conf0,conf1;
XClk_Wiz_Config conf;
XAxiVdma_DmaSetup ReadCfg;
XAxiVdma_Config *Config=NULL;

void print_clkinfo(char * str,uint32_t base,uint32_t address)
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
int cpu_freq=100;
void configure_clk(int clk, int busclk, int verbose, int nbr)
{
	int clken=1;
	int bclk_mul=1;
	int bclk;
	int clk_remainder;
	int bclk_remainder;
	XClk_Wiz_CfgInitialize(&clkwiz0, &conf0, XPAR_CLK_WIZ_0_BASEADDR);
	XClk_Wiz_CfgInitialize(&clkwiz1, &conf1, XPAR_CLK_WIZ_1_BASEADDR);

	if(clk>100)
		clk=100;
	else if(clk<50)
		clk=50;

	clk&=~1;
// unstable frequencies: mapped to stable ones
	if(clk==92) clk=88;
	if(clk==72) clk=76;

	cpu_freq=clk;
	*((uint32_t *)(0x18000000+REG_ZZ_CPU_FREQ))= swap32(cpu_freq);

	clk_remainder=clk%2;
	clk=clk&~1;

	if(clk<60)
		bclk_mul=2;
	bclk=(clk*bclk_mul)/4;
	bclk_remainder=(clk*bclk_mul)%4;

	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200, ((clk_remainder*500)<<16)+(clk<<7) + 10); // 64h=100 100*100 / 10 -> 1000MHz VCO
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208,(uint32_t)( 0x00000005)); //   5h= 5 -> 200 MHz AXI
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C,(uint32_t)(     0*1000)); //                    PHASE 0
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210,(uint32_t)(    50*1000)); //                    DC 50%
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x214,(uint32_t)( 0x0000000A)); //   Ah=10 -> 100 MHz PCLK
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x218,(uint32_t)(    30*1000)); //                    PHASE 30
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x21C,(uint32_t)(    50*1000)); //                    DC 50%
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x220,(uint32_t)( 0x00000014)); //  14h=20 ->  50 MHz CLKEN
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x224,(uint32_t)(    60*1000)); //                    PHASE 60
	if(clken)
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x228,(uint32_t)(50*1000)); //                    DC 50%
	else
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x228,(uint32_t)( 0*1000)); //                    DC 0%

	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x200, ((bclk_remainder*250)<<16)+((bclk*4)<<7)+10); // 64h=100 100*100 / 10 -> 1000MHz VCO
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x208,(uint32_t)( 0x00000028)); //  28h=40 ->  25 MHz BCLK
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x20C,(uint32_t)(    40*1000)); //                        PHASE 40
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x210,(uint32_t)(    50*1000)); //                    DC 50%
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x214,(uint32_t)(         80)); //  28h=40 ->  25 MHz BCLK2
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x218,(uint32_t)(     0*1000)); //                    PHASE 0
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x21C,(uint32_t)(    50*1000)); //                    DC 50%
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x220,(uint32_t)( 0x00000028/bclk_mul)); //   Ah=10 -> 100 MHz CLK90
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x224,(uint32_t)(   290*1000)); //                    PHASE 290
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x228,(uint32_t)(    50*1000)); //                    DC 50%
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x22C,(uint32_t)( 0x00000028/bclk_mul)); //  14h=20 ->  50 MHz CPUCLK
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x230,(uint32_t)(   200*1000)); //                    PHASE 200
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x234,(uint32_t)(    50*1000)); //                    DC 50%

	if(nbr)
		NBR_ARM(0);
	usleep(100000);

	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x25C, 0x00000003);

	usleep(100000);
	if(nbr)
		NBR_ARM(1);

/*
	if((clk==100)&&(busclk==50))
	{
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200, 0x0000320A); // 32h=50 50*200 / 10 -> 1000MHz VCO
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208,(uint32_t)( 0x00000005)); //   5h= 5 -> 200 MHz AXI
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C,(uint32_t)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210,(uint32_t)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x214,(uint32_t)( 0x0000000A)); //   Ah=10 -> 100 MHz PCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x218,(uint32_t)(    30*1000)); //                    PHASE 30
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x21C,(uint32_t)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x220,(uint32_t)( 0x00000014)); //  14h=20 ->  50 MHz CLKEN
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x224,(uint32_t)(    60*1000)); //                    PHASE 60
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x228,(uint32_t)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);
		int offset=10;
		int frec_bclk=25;
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x200, frec_bclk*2*256+10); // 32h=50 50*200 / 10 -> 1000MHz VCO
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x208,(uint32_t)( 0x00000028)); //  28h=40 ->  25 MHz BCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x20C,(uint32_t)(   (30+offset)*frec_bclk/25*1000)); //                        PHASE 40
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x210,(uint32_t)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x214,(uint32_t)( 80)); //  28h=40 ->  25 MHz BCLK2
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x218,(uint32_t)(     0*1000)); //                    PHASE 0
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x21C,(uint32_t)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x220,(uint32_t)( 0x00000028)); //   Ah=10 -> 100 MHz CLK90
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x224,(uint32_t)(   (270+10+offset)*frec_bclk/25*1000)); //                    PHASE 290
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x228,(uint32_t)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x22C,(uint32_t)( 0x00000028)); //  14h=20 ->  50 MHz CPUCLK
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x230,(uint32_t)(   (180+10+offset)*frec_bclk/25*1000)); //                    PHASE 200
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x234,(uint32_t)(    50*1000)); //                    DC 50%
		XClk_Wiz_WriteReg(XPAR_CLK_WIZ_1_BASEADDR, 0x25C, 0x00000003);
	}
*/
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

void configure_gpio(void)
{
	GpioPsConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
	XGpioPs_CfgInitialize(&GpioPs, GpioPsConfigPtr, GpioPsConfigPtr->BaseAddr);

	XGpioPs_SetDirectionPin(&GpioPs, n040RSTI, 0);
	XGpioPs_SetOutputEnablePin(&GpioPs, n040RSTI, 0);

	XGpioPs_SetDirectionPin(&GpioPs, USER_SW1, 0);
	XGpioPs_SetOutputEnablePin(&GpioPs, USER_SW1, 0);

	XGpioPs_SetDirectionPin(&GpioPs, LED1, 1);
	XGpioPs_SetOutputEnablePin(&GpioPs, LED1, 1);
	XGpioPs_WritePin(&GpioPs, LED1, 0);

	XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_13, 1);
	XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_13, 1);
	XGpioPs_WritePin(&GpioPs, PS_MIO_13, 1);
	XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_8, 1);
	XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_8, 1);
	XGpioPs_WritePin(&GpioPs, PS_MIO_8, 1);

	XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_0, 0);
	XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_0, 0);
	XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_9, 0);
	XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_9, 0);
	XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_12, 0);
	XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_12, 0);
	XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_15, 0);
	XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_15, 0);

	XGpioPs_SetDirectionPin(&GpioPs, PS_MIO51_501, 1);
	XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO51_501, 1);
	XGpioPs_WritePin(&GpioPs, PS_MIO51_501, 0);
	usleep(10000);
	XGpioPs_WritePin(&GpioPs, PS_MIO51_501, 1);

//	GpioConfigPtr = XGpio_LookupConfig(XPAR_AXI_GPIO_0_DEVICE_ID);
//	XGpio_CfgInitialize(&Gpio, GpioConfigPtr, GpioConfigPtr->BaseAddress);


	DiscreteClear(REG0, FPGA_RAM_EN);
	DiscreteClear(REG0, FPGA_RAM_BURST_READ_EN);
	DiscreteClear(REG0, FPGA_RAM_BURST_WRITE_EN);
	DiscreteSet(REG0, FPGA_RESET);
	usleep(500);
	DiscreteClear(REG0, FPGA_RESET);
	printf("Configured GPIO...\n\r");
}
void fpga_feature_enable(int en_ram,int en_rtg,int en_z3ram)
{
	if(en_ram)
	{
		ENABLE_CPU_RAM_FPGA;
		ENABLE_BURST_READ_FPGA;
		ENABLE_BURST_WRITE_FPGA;
		printf("FPGA CPU RAM ENABLED\n\r");
	}
	else
	{
		DISABLE_CPU_RAM_FPGA;
		DISABLE_BURST_READ_FPGA;
		DISABLE_BURST_WRITE_FPGA;
		printf("FPGA CPU RAM DISABLED\n\r");
	}
	if(en_rtg)
	{
		ENABLE_RTG_AUTOCONFIG;
		printf("FPGA RTG ENABLED\n\r");
	}
	else
	{
		DISABLE_RTG_AUTOCONFIG;
		printf("FPGA RTG DISABLED\n\r");
	}
	if(en_z3ram)
	{
		ENABLE_256MB_AUTOCONFIG;
		printf("FPGA Z3 RAM ENABLED\n\r");
	}
	else
	{
		DISABLE_256MB_AUTOCONFIG;
		printf("FPGA Z3 RAM DISABLED\n\r");
	}
}
char Filename[]="Z3660.bin";
//extern uint32_t *frameBuf;
extern ZZ_VIDEO_STATE* video_state;
int state68k=M68K_RUNNING;
#define ARM_NO_BUS_MASTER 0
#define ARM_REQUEST_BUS_MASTER 1
#define ARM_BUS_MASTER 2
#define ARM_RELINQUISH_BUS_MASTER 3
int stateARM=ARM_NO_BUS_MASTER;
int arm_command=0;
int arm_request_bus_master=0;
#include "xil_mmu.h"
void reset_run(void);
void reset_init(void);

#define AMR1_STARTADDR 0xFFFFFFF0
#define ARM1_BASEADDR 0x30000000

int enables=0;
uint32_t data=0;
void arm_write_amiga(uint32_t address, uint32_t data, uint32_t size)
{
//	write_reg(0x08,address);         // address
//	write_reg(0x0C,data);    // data
	write_reg64_s01(0x08,(((uint64_t)data)<<32)|address);    // data
	write_reg_s01(0x10,0x11|WRITE_|size); // command
	while(read_reg_s01(0x14)==0) // read ack
	{}
	write_reg_s01(0x10,0x01); // confirm ack
}
uint32_t arm_read_amiga(uint32_t address, uint32_t size)
{
	write_reg_s01(0x08,address);        // address
	write_reg_s01(0x10,0x11|READ_|size); // command
	while(read_reg_s01(0x14)==0) // read ack
	{}
	write_reg_s01(0x10,0x01); // confirm ack
	data=read_reg_s01(0x1C); // read data
	return(data);
}
uint32_t amiga_address=0,amiga_data=0,amiga_size=WORD_;
void flash_colors(void)
{
//#define FLASH_COLORS
#ifdef FLASH_COLORS
	NBR_ARM(0);
	CPLD_RESET_ARM(1);
//	XGpioPs_WritePin(&GpioPs, nBR_ARM, 0);
//	XGpioPs_WritePin(&GpioPs, CPLD_RESET, 1);

	usleep(100);
	{
		NBR_ARM(0);
//		XGpioPs_WritePin(&GpioPs, nBR_ARM, 0);
		int a,c;
		for(c=0;c<20;c++)
			for(a=2047;a>=1024;a--)
				arm_write_amiga(0xDFF180,a<<4,WORD_);
		arm_write_amiga(0xDFF180,0,WORD_);
		write_reg(0x10,0x0); // Bus Hi-Z
		NBR_ARM(1);
//		XGpioPs_WritePin(&GpioPs, nBR_ARM, 1);
	}
#endif

}

unsigned int READ_NBG_ARM(void)
{
/*	uint32_t read=*(volatile uint32_t*)(XPAR_PS7_GPIO_0_BASEADDR+XGPIOPS_DATA_RO_OFFSET);
	mux.nbg_arm=(read>>8)&1;*/
	return(XGpioPs_ReadPin(&GpioPs, PS_MIO_15));
}
void DataAbortHandler(void *data)
{
	// Do nothing??? :)
}
extern CONFIG config;

int main()
{
	init_platform();
    Xil_ICacheEnable();
#ifdef L1_CACHE_ENABLED
    Xil_L1DCacheEnable();
#else
    Xil_L1DCacheDisable();
#endif
#ifdef L2_CACHE_ENABLED
    Xil_L2CacheEnable();
#else
    Xil_L2CacheDisable();
#endif
    //	Xil_DisableMMU();
    Xil_SetTlbAttributes(0xFFFF0000UL,0x14DE2);//STRONG_ORDERED|SHAREABLE);//NORM_WT_CACHE);//0x14de2);//NORM_NONCACHE);
	for(int i=0x080;i<0x180;i++)
		Xil_SetTlbAttributes(i*0x100000UL,NORM_WB_CACHE);
	for(int i=0x180;i<0x182;i++) // RTG Registers (2 MB reserved, fb is at 0x200000)
		Xil_SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);
	for(int i=0x182;i<0x200;i++) // RTG RAM
		Xil_SetTlbAttributes(i*0x100000UL,NORM_WT_CACHE);//NORM_WT_CACHE);// NORM_WB_CACHE);//0x14de2);
	for(int i=0x400;i<0x780;i++)
		Xil_SetTlbAttributes(i*0x100000UL,RESERVED);
	for(int i=0xE00;i<0xE03;i++) //
		Xil_SetTlbAttributes(i*0x100000UL,STRONG_ORDERED);//NORM_NONCACHE);

	for(int i=0;i<0x1200;i++)
		*(uint8_t*)(0x18000000+0x06000000+i)=0; // clean audio buffer

	configure_gpio();

	configure_clk(100,50,0,0);

	XGpioPs_WritePin(&GpioPs, LED1, 1);
	DiscreteSet(REG0, FPGA_RESET);
	CPLD_RESET_ARM(0);
	NBR_ARM(0);
	while(READ_NBG_ARM()==0);
	DiscreteClear(REG0,FPGA_INT6); // set int6 to 0 (active high)

//	printf("\033[2J");
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

//	int cpu_speed=3;

	int clk_config=3;     // CPU initial state: 0 50/25, 1 50/50, 2 100/25, 3 100/50, 4 ...
	int enable_ram=1;     // RAM initial state
	int enable_rtg=1;     // RTG initial state
	int enable_z3ram=1;   // Z3 RAM initial state

	fpga_feature_enable(enable_ram,enable_rtg,enable_z3ram);

	init_shared();

	read_config_file();

	read_env_files();

	if(config.autoconfig_ram)
		ENABLE_256MB_AUTOCONFIG;
	else
		DISABLE_256MB_AUTOCONFIG;

	if(config.boot_mode==MUSASHI || config.boot_mode==UAE || config.boot_mode==UAEJIT)
	{
		//the autoconfig and CPU RAM are emulated in cpu_emulator(), so we don't need hardware autoconfig
		DISABLE_CPU_RAM_FPGA;
		DISABLE_BURST_READ_FPGA;
		DISABLE_BURST_WRITE_FPGA;
		DISABLE_256MB_AUTOCONFIG;
		DISABLE_RTG_AUTOCONFIG;
	}
	else
	{
	//remove me
//	DISABLE_CPU_RAM_FPGA;
//	DISABLE_BURST_READ_FPGA;
//	DISABLE_BURST_WRITE_FPGA;
//	DISABLE_256MB_AUTOCONFIG;
//	DISABLE_RTG_AUTOCONFIG;
	}
/*
	if((read_reg_s01(REG0)&2)==0)
		printf("Read Bursts DISABLED\n\r");
	else
		printf("Read Bursts ENABLED\n\r");
	if((read_reg_s01(REG0)&4)==0)
		printf("Write Bursts DISABLED\n\r");
	else
		printf("Write Bursts ENABLED\n\r");
*/
	int verbose=1;
	int nbr=0;
	switch(clk_config)
	{
		case 0:
//			cpu_speed=0;
			configure_clk(50,25,verbose,nbr);
			break;
		case 1:
//			cpu_speed=1;
			configure_clk(50,50,verbose,nbr);
			break;
		case 2:
//			cpu_speed=2;
			configure_clk(100,25,verbose,nbr);
			break;
		case 3:
//			cpu_speed=3;
			configure_clk(100,50,verbose,nbr);
			break;
	}

	video_state=video_init();

//    PrepareHdf();
//    InitGayle();
//#define READ_BOOT_IMAGE
#ifdef READ_BOOT_IMAGE
	static FIL fil;		/* File object */
	static FATFS fatfs;
	TCHAR *Path = "0:/";
	f_mount(&fatfs, Path, 1); // 1 mount immediately
	f_open(&fil,Filename, FA_OPEN_ALWAYS | FA_READ);
	f_lseek(&fil, 4);
	UINT NumBytesRead;
	printf("Reading %s file:\r\n[----------]\r\n\033[F",Filename);
	for(uint32_t i=0,j=0,k=1;i<1920*1080*4;i+=10*1080*4,j++)
	{
		if(j==19)
		{
			j=0;
			printf("%.*s\r\n\033[F",(int)++k,"[==========]");
		}
		f_read(&fil, (void*)((uint32_t)video_state->framebuffer+i), 10*1080*4,&NumBytesRead);
	}
	f_close(&fil);
	printf("\r\nFile read OK\r\n");
	reset_video(NO_RESET_FRAMEBUFFER);
#else
	video_reset();
#endif
	XGpioPs_WritePin(&GpioPs, LED1, 1);
	DiscreteSet(REG0, FPGA_RESET);
	CPLD_RESET_ARM(0);
	usleep(2500);

	XGpioPs_WritePin(&GpioPs, LED1, 0);
	DiscreteClear(REG0, FPGA_RESET);

	int ret=0;
//	int timer_counter_update_led=0;

	// Zturn Patch for Ateros phy
	Xil_Out32(0xE000A000 + 0x244,0x00080000);
	Xil_Out32(0xE000A000 + 0x248,0x00080000);
	Xil_Out32(0xE000A000 +   0xC,0xFFF70008);
//	sleep(1);

	xadc_init();

	ethernet_init();

#define INT_IPL_ON_THIS_CORE 0
	fpga_interrupt_connect(isr_video,isr_audio_tx,INT_IPL_ON_THIS_CORE);

	flash_colors();

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT, DataAbortHandler,0);
//    *((volatile uint32_t*)0xF8F01834)&=~0x00000001; // FIXME: disable interrupt for core 0

	NBR_ARM(0);
	CPLD_RESET_ARM(0);
	usleep(100);
	CPLD_RESET_ARM(1);

	Xil_ExceptionEnable();
	if(config.boot_mode==MUSASHI || config.boot_mode==UAE || config.boot_mode==UAEJIT)
	{
	    // Esto hay que repetirlo mas adelante...
	    *((volatile uint32_t*)0xF8F01834)=0x03010302; // FIXME: disable interrupt for core 0 and core 1
	    state68k=-1; // don't use cache flush when using emulator
	    cpu_emulator(); // infinite loop inside
	}
	if(config.scsiboot==1 && config.boot_mode!=CPU)
		shared->boot_rom_loaded=piscsi_init();

//	piscsi_map_drive("hdf/ZDH0.hdf", 0);

	NBR_ARM(1);
	CPLD_RESET_ARM(0);
	usleep(100);
	CPLD_RESET_ARM(1);

	while(1)
	{
		switch(stateARM)
		{
		case ARM_NO_BUS_MASTER:
			if(arm_request_bus_master)
			{
				NBR_ARM(0);
				stateARM=ARM_REQUEST_BUS_MASTER;
			}
			break;
		case ARM_REQUEST_BUS_MASTER:
		    if(READ_NBG_ARM()==0)
			{
				printf("Bus Request from ARM OK\n");
				stateARM=ARM_BUS_MASTER;
			}
			break;
		case ARM_BUS_MASTER:
			{
				switch(arm_command)
				{
					case 1:
						arm_command=2;
//						enables=arm_read_amiga(0xDFF01C,WORD);
//						arm_write_amiga(0xDFF09A,0x7FFF,WORD);
//						arm_write_amiga(0xDFF09C,0x7FFF,WORD);
//						arm_write_amiga(0xDFF096,0x7FFF,WORD);

//						arm_write_amiga(0xDFF100,0x0200,WORD);
//						arm_write_amiga(0xDFF110,0x0000,WORD);
//						arm_write_amiga(0xDFF180,0x0000,WORD);
						break;
					case 2:
						{
							static int a=256,b=256;
							a--;
							a&=0xFF;
							if(a==0)
							{
								b--;
								b&=0xFF;
								if(b==0)
									arm_command=3;
							}
							arm_write_amiga(0xDFF180,a,WORD_);
						}
						break;
					case 3:
//						arm_write_amiga(0xDFF09A,0x8000|enables,WORD);
						write_reg_s00(0x10,0x0); // Bus Hi-Z
						arm_command=0;
						break;
					case 4:
						arm_write_amiga(amiga_address,amiga_data,amiga_size);
						write_reg_s00(0x10,0x0); // Bus Hi-Z
						arm_command=0;
						break;
					case 5:
						amiga_data=arm_read_amiga(amiga_address,amiga_size);
						write_reg_s00(0x10,0x0); // Bus Hi-Z
						arm_command=0;
						break;
					case 0:
					default:
						break;
				}
			}
			if(arm_request_bus_master==0)
			{
				NBR_ARM(1);
				stateARM=ARM_RELINQUISH_BUS_MASTER;
			}
			break;
		case ARM_RELINQUISH_BUS_MASTER:
		    if(READ_NBG_ARM()==1)
			{
				printf("Bus Relinquish from ARM OK\n");
				stateARM=ARM_NO_BUS_MASTER;
			}
			break;
		}
		switch(state68k)
		{
		case M68K_RUNNING:
			rtg_loop();
			if(XGpioPs_ReadPin(&GpioPs, n040RSTI)==0)
			{
				printf("Reset active (DOWN)...\n\r");
				state68k=M68K_RESET;
				reset_init();
				hard_reboot();
//				Xil_Out32(XSLCR_UNLOCK_ADDR, XSLCR_UNLOCK_CODE);
//				Xil_Out32(XSLCR_UNLOCK_ADDR, XSLCR_UNLOCK_CODE);
//				uint32_t RegVal = Xil_In32(A9_CPU_RST_CTRL);
//				XPS_SYS_CTRL_BASEADDR + A9_CPU_RST_CTRL_OFFSET
			}
			else
			{
/*
				int delay;
		//		DiscreteClear(REG0, FPGA_RESET);
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
*/
/*
				static int toggle=0;
				toggle^=1;
				XGpioPs_WritePin(&GpioPs, LED1, toggle);
*/
//#define USER_SWITCH1
#ifdef USER_SWITCH1
				if(XGpioPs_ReadPin(&GpioPs, USER_SW1)==0)
				{
					clk_config++;
					clk_config&=3;
					switch(clk_config)
					{
						case 0:
//							cpu_speed=0;
							configure_clk(50,25,verbose);
							XGpioPs_WritePin(&GpioPs, LED2, 1);
							break;
						case 1:
//							cpu_speed=1;
							configure_clk(50,50,verbose);
							XGpioPs_WritePin(&GpioPs, LED2, 0);
							break;
						case 2:
//							cpu_speed=2;
							configure_clk(100,25,verbose);
							XGpioPs_WritePin(&GpioPs, LED2, 1);
							break;
						case 3:
//							cpu_speed=3;
							configure_clk(100,50,verbose);
							XGpioPs_WritePin(&GpioPs, LED2, 0);
							break;
					}
					while(XGpioPs_ReadPin(&GpioPs, USER_SW1)==0)
					{}
				}
#endif
			}
			break;
		case M68K_RESET:
			reset_run();
			ret=XGpioPs_ReadPin(&GpioPs, n040RSTI);
			if(ret!=0)
			{
				state68k=M68K_RUNNING;
//				DiscreteSet(REG0, FPGA_RESET);
				CPLD_RESET_ARM(0);
				DiscreteSet(REG0, FPGA_RESET);
				int reset_counter=10;
				while(reset_counter>0)
				{
					usleep(25000);
					reset_counter--;
				}
				DiscreteClear(REG0, FPGA_RESET);
				reset_counter=10;
				while(reset_counter>0)
				{
					usleep(25000);
					reset_counter--;
				}
				// Initialize something here... ???
				flash_colors();
				CPLD_RESET_ARM(0);
				usleep(100);
				CPLD_RESET_ARM(1);
				printf("Reset inactive (UP)...\n\r");
				video_reset();
				audio_reset();
			}
			break;
		}

	}
	cleanup_platform();
	return(0);
}
