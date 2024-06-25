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
#include "config_clk.h"
#include "mobotest.h"
#include "sleep.h"
#include "xil_misc_psreset_api.h"
#include "config_file.h"
#include "scsi/scsi.h"
#include "LTC2990/ltc2990.h"
#include "../../Z3660_emu/src/defines.h"
#include "debug_console.h"
#ifdef CPU_EMULATOR
#include "cpu_emulator.h"
#endif
#include "lwip.h"
extern SHARED *shared;
extern int boot_rom_loaded;
extern ENV_FILE_VARS env_file_vars_temp;
//extern int bm,sb,ar,cr,ks,ext_ks,*scsi_num;
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

extern uint32_t clken;
#define ENABLE_TSCONDITION(A) do{   DiscreteSet(REG0, A);\
                  }while(0)

#define ENABLE_ENCONDITION_PCLK0_CLKEN0 do{   DiscreteSet(REG0, FPGA_ENCONDITION_PCLK0_CLKEN0);\
                  }while(0)
#define ENABLE_ENCONDITION_PCLK1_CLKEN0 do{   DiscreteSet(REG0, FPGA_ENCONDITION_PCLK1_CLKEN0);\
                  }while(0)
#define ENABLE_ENCONDITION_PCLK0 do{   clken=0;DiscreteSet(REG0, FPGA_ENCONDITION_PCLK0);\
                  }while(0)
#define ENABLE_ENCONDITION_PCLK1 do{   clken=0;DiscreteSet(REG0, FPGA_ENCONDITION_PCLK1);\
                  }while(0)

#define ENABLE_CPU_RAM_FPGA do{   DiscreteSet(REG0, FPGA_RAM_EN);\
                  }while(0)
#define DISABLE_CPU_RAM_FPGA do{ DiscreteClear(REG0, FPGA_RAM_EN);\
                  }while(0)
#define ENABLE_MAPROM_FPGA do{   DiscreteSet(REG0, FPGA_MAPROM_EN);\
                  }while(0)
#define DISABLE_MAPROM_FPGA do{ DiscreteClear(REG0, FPGA_MAPROM_EN);\
                  }while(0)
#define ENABLE_MAPROMEXT_FPGA do{   DiscreteSet(REG0, FPGA_MAPROMEXT_EN);\
                  }while(0)
#define DISABLE_MAPROMEXT_FPGA do{ DiscreteClear(REG0, FPGA_MAPROMEXT_EN);\
                  }while(0)

#define ENABLE_256MB_AUTOCONFIG do{DiscreteSet(REG0, FPGA_256MB_AUTOCONFIG_EN);\
                  }while(0)
#define DISABLE_256MB_AUTOCONFIG do{DiscreteClear(REG0, FPGA_256MB_AUTOCONFIG_EN);\
                  }while(0)

#define ENABLE_RTG_AUTOCONFIG do{DiscreteSet(REG0, FPGA_RTG_AUTOCONFIG_EN);\
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
static char FileName[32] = DEFAULT_ROOT "Test.bin";
static char *SD_File;
int FfsSdPolledExample(void)
{
   FRESULT Res;
   UINT NumBytesRead;
   UINT NumBytesWritten;
   uint32_t BuffCnt;
//   BYTE work[FF_MAX_SS];
   uint32_t FileSize = (8*1024*1024);


   // To test logical drive 0, Path should be "0:/"
   // For logical drive 1, Path should be "1:/"
   //
   TCHAR *Path = DEFAULT_ROOT;

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

//   Res = f_mkfs(Path, FM_FAT32, 0, work, sizeof work);
//   if (Res != FR_OK) {
//      return XST_FAILURE;
//   }

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
/*
//char Filename[]=DEFAULT_ROOT "Test.hdf";
//char Filename[]=DEFAULT_ROOT "hd0.img";
char Filename[]=DEFAULT_ROOT "hd0.hdf";
void CreateHdf(void)
{
   TCHAR *Path = DEFAULT_ROOT;
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

//static FIL fil;      // File object
//static FATFS fatfs;
void PrepareHdf(void)
{
   TCHAR *Path = DEFAULT_PATH;

   f_mount(&fatfs, Path, 0);
   set_hard_drive_image_file_amiga(0,Filename);
}
*/
ZZ_VIDEO_STATE vs;
XAxiVdma vdma;
XClk_Wiz clkwiz;
XClk_Wiz_Config conf;
XAxiVdma_DmaSetup ReadCfg;
XAxiVdma_Config *Config=NULL;

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
   ACTIVITY_LED_OFF; // OFF

   XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_13, 1);
   XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_13, 1);
   XGpioPs_WritePin(&GpioPs, PS_MIO_13, 0);
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

//   GpioConfigPtr = XGpio_LookupConfig(XPAR_AXI_GPIO_0_DEVICE_ID);
//   XGpio_CfgInitialize(&Gpio, GpioConfigPtr, GpioConfigPtr->BaseAddress);


   DiscreteClear(REG0, FPGA_RAM_EN);
   DiscreteClear(REG0, FPGA_MAPROM_EN);
   DiscreteClear(REG0, FPGA_MAPROMEXT_EN);
   DiscreteClear(REG0, FPGA_RAM_BURST_READ_EN);
   DiscreteClear(REG0, FPGA_RAM_BURST_WRITE_EN);
   DiscreteSet(REG0, FPGA_RESET);
   usleep(500);
   DiscreteClear(REG0, FPGA_RESET);
   printf("Configured GPIO...\n\r");
}
void fpga_feature_enable(int en_ram,int en_rtg,int en_z3ram,int en_maprom, int en_mapromext)
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
   if(en_maprom)
   {
      ENABLE_MAPROM_FPGA;
      printf("FPGA MAPROM ENABLED\n\r");
   }
   else
   {
      DISABLE_MAPROM_FPGA;
      printf("FPGA MAPROM DISABLED\n\r");
   }
   if(en_mapromext)
   {
      ENABLE_MAPROMEXT_FPGA;
      printf("FPGA MAPROMEXT ENABLED\n\r");
   }
   else
   {
      DISABLE_MAPROMEXT_FPGA;
      printf("FPGA MAPROMEXT DISABLED\n\r");
   }
}
char Filename[]=DEFAULT_ROOT "Z3660.bin";
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
void reset_run(int cpu_boot_mode,int counter, int counter_max,int long_reset);
void reset_init(void);
int load_rom(void);
int load_romext(void);

#define AMR1_STARTADDR 0xFFFFFFF0
#define ARM1_BASEADDR 0x30000000
#define NOP __asm(" nop")

int enables=0;
uint32_t data=0;
void arm_write_amiga(uint32_t address, uint32_t data, uint32_t size)
{
   uint32_t bank=(address>>24)&0xFF;
   write_reg_s00(0x18,bank);
   NOP;
   if(size==LONG_)
      write_mem32(address,data);
   else if(size==WORD_)
      write_mem16(address,data);
   else
      write_mem8(address,data);
   NOP;
   while(read_reg_s00(0x14)==0) // read ack
   {}
////   write_reg_s00(0x08,address);         // address
////   write_reg_s00(0x0C,data);    // data
//   write_reg64_s00(0x08,(((uint64_t)data)<<32)|address);    // data
//   write_reg_s00(0x10,0x11|WRITE_|size); // command
//   while(read_reg_s00(0x14)==0) // read ack
//   {}
//   write_reg_s00(0x10,0x01); // confirm ack
}
void arm_write_nowait(uint32_t address, uint32_t data)
{
   uint32_t bank=(address>>24)&0xFF;
   write_reg_s00(0x18,bank);
   NOP;
   write_mem32(address,data);
   NOP;
//   while(read_reg_s00(0x14)==0) // read ack
//   {}
   write_reg(0x10,0x0); // Bus Hi-Z
}
uint32_t arm_read_amiga(uint32_t address, uint32_t size)
{
   write_reg_s00(0x08,address);        // address
   write_reg_s00(0x10,0x11|READ_|size); // command
   while(read_reg_s00(0x14)==0) // read ack
   {}
//   write_reg_s00(0x10,0x01); // confirm ack
   data=read_reg_s00(0x1C); // read data
   return(data);
}
uint32_t amiga_address=0,amiga_data=0,amiga_size=WORD_;
void flash_colors(void)
{
//#define FLASH_COLORS
#ifdef FLASH_COLORS
   NBR_ARM(0);
   CPLD_RESET_ARM(1);

   usleep(100);
   {
      NBR_ARM(0);
      int a,c;
      for(c=0;c<20;c++)
         for(a=2047;a>=1024;a--)
            arm_write_amiga(0xDFF180,a<<4,WORD_);
      arm_write_amiga(0xDFF180,0,WORD_);
      write_reg(0x10,0x0); // Bus Hi-Z
      NBR_ARM(1);
   }
#endif

}

unsigned int READ_NBG_ARM(void)
{
/*   uint32_t read=*(volatile uint32_t*)(XPAR_PS7_GPIO_0_BASEADDR+XGPIOPS_DATA_RO_OFFSET);
   mux.nbg_arm=(read>>8)&1;*/
   return(XGpioPs_ReadPin(&GpioPs, PS_MIO_15));
}
extern u32 DataAbortAddr;
extern u32 PrefetchAbortAddr;

void DataAbortHandler(void *data)
{
    uint32_t FaultStatus;
    unsigned int FaultAddress;
    FaultAddress = mfcp(XREG_CP15_DATA_FAULT_ADDRESS);
    FaultStatus = mfcp(XREG_CP15_DATA_FAULT_STATUS);
    usleep(100000); // let other printf to finish
    printf("[Core0] DataAbortHandler()!!!!\n");
    printf("Data abort with Data Fault Status Register 0x%08lx\n",FaultStatus);
    printf("Address of Instruction causing Data abort 0x%08lx\n",DataAbortAddr);
    unsigned int write_fault=FaultStatus&(1<<11);
    if(write_fault)
    	printf("Write Fault to 0x%08X\n",FaultAddress);
    else
    	printf("Read Fault from 0x%08X\n",FaultAddress);
    printf("Instruction 0x%08lX\n",*((uint32_t *)(uint32_t)DataAbortAddr));
	usleep(10000);
	hard_reboot();
	while(1);
}
void PrefetchAbortHandler(void *CallBackRef){
	(void) CallBackRef;
	u32 FaultStatus;

    FaultStatus = mfcp(XREG_CP15_INST_FAULT_STATUS);

	printf("Prefetch abort with Instruction Fault Status Register  0x%08lx\n",FaultStatus);
	printf("Address of Instruction causing Prefetch abort 0x%08lx\n",PrefetchAbortAddr);
	usleep(10000);
	hard_reboot();
	while(1);
}

extern CONFIG config;
extern int no_init;
extern uint32_t MMUTable;
void dump_mmu(void)
{
	printf("Core 0 MMUTable 0x%08lX\n",(uint32_t)(&MMUTable));
	for(uint32_t i=0;i<0x200;i++)
	{
		printf("0x%03lX ",i*8);
		for(uint32_t j=0;j<8;j++)
		{
			uint32_t data=*((uint32_t *)(&MMUTable+(i*8+j)));
			printf("%08lX ",data);
		}
		printf("\n");
	}
	if(config.boot_mode==CPU) // if core 0 is the only active core
		return;               // then return
	printf("\n");
	printf("Core 1 MMUTable 0x%08lX\n",(uint32_t)((uint32_t *)shared->mmu_core1_add));
	for(uint32_t i=0;i<0x200;i++)
	{
		printf("0x%03lX ",i*8);
		for(uint32_t j=0;j<8;j++)
		{
			uint32_t data=*((uint32_t *)(((uint32_t *)shared->mmu_core1_add)+(i*8+j)));
			printf("%08lX ",data);
		}
		printf("\n");
	}
}
void setMMU(INTPTR Addr, u32 attrib)
{
   u32 *ptr;
   u32 section;

   section = Addr / 0x100000U;
   ptr = &MMUTable;
   ptr += section;
   if(ptr != NULL) {
      *ptr = attrib;
   }
}
void finish_MMU_OP(void)
{
   Xil_DCacheFlush();

   mtcp(XREG_CP15_INVAL_UTLB_UNLOCKED, 0U);
   /* Invalidate all branch predictors */
   mtcp(XREG_CP15_INVAL_BRANCH_ARRAY, 0U);

   dsb(); /* ensure completion of the BP and TLB invalidation */
   isb(); /* synchronize context on this processor */
}

void rtg_cache_policy_core0(uint32_t policy)
{
   for(int i=0x182;i<0x1FE;i++) // RTG
      Xil_SetTlbAttributes(i*0x100000UL,policy);//NORM_WT_CACHE);// NORM_WB_CACHE);//0x14de2);
   for(int i=0x1C2;i<0x1C3;i++) // RTG Registers (1 MB for soft3d registers)
      Xil_SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);
   for(int i=0x502,j=2;i<0x580;i++,j++)
   {
      uint32_t address=(RTG_BASE+j*0x100000UL);
      setMMU(i*0x100000UL,address|policy); // mapped to RTG_BASE
   }
   finish_MMU_OP();
}

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
    //   Xil_DisableMMU();
    Xil_SetTlbAttributes(0xFFFF0000UL,NORM_NONCACHE);//0x14DE2);//STRONG_ORDERED|SHAREABLE);//NORM_WT_CACHE);//0x14de2);//NORM_NONCACHE);
//   for(int i=0x000;i<0x010;i++)
//      Xil_SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);
   for(int i=0x080;i<0x180;i++)
      Xil_SetTlbAttributes(i*0x100000UL,NORM_WB_CACHE);
   for(int i=0x180;i<0x182;i++) // RTG Registers (2 MB reserved, fb is at 0x200000)
      Xil_SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);
   for(int i=0x1FE;i<0x200;i++) // ETHERNET
      Xil_SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);//NORM_NONCACHE);// NORM_WB_CACHE);//0x14de2);
   for(int i=0x400;i<0x780;i++)
      Xil_SetTlbAttributes(i*0x100000UL,RESERVED);
   for(int i=0xE00;i<0xE03;i++) //
      Xil_SetTlbAttributes(i*0x100000UL,STRONG_ORDERED);//NORM_NONCACHE);

   for(int i=0;i<0x1200;i++)
      *(uint8_t*)(RTG_BASE+0x06000000+i)=0; // clean audio buffer

   configure_gpio();

   DiscreteSet(REG0, FPGA_RESET);
   CPLD_RESET_ARM(0);

   ENABLE_TSCONDITION(FPGA_TSCONDITION0);

   ENABLE_ENCONDITION_PCLK0_CLKEN0;
//   ENABLE_ENCONDITION_PCLK1_CLKEN0;
//   ENABLE_ENCONDITION_PCLK0;
//   ENABLE_ENCONDITION_PCLK1;

   int clk_config=10; // 50 MHz
   int verbose=0;
   int nbr=0;
   switch(clk_config)
   {
      case 0:
         configure_clk(100,verbose,nbr);
         break;
      case 1:
         configure_clk(95,verbose,nbr);
         break;
      case 2:
         configure_clk(90,verbose,nbr);
         break;
      case 3:
         configure_clk(85,verbose,nbr);
         break;
      case 4:
         configure_clk(80,verbose,nbr);
         break;
      case 5:
         configure_clk(75,verbose,nbr);
         break;
      case 6:
         configure_clk(70,verbose,nbr);
         break;
      case 7:
         configure_clk(65,verbose,nbr);
         break;
      case 8:
         configure_clk(60,verbose,nbr);
         break;
      case 9:
         configure_clk(55,verbose,nbr);
         break;
      case 10:
         configure_clk(50,verbose,nbr);
         break;
   }

   ACTIVITY_LED_OFF; // OFF
   DiscreteClear(REG0,FPGA_INT6); // set int6 to 0 (active high)

   int reset_time_counter=0;
   int reset_time_counter_max=60*4;

//   printf("\033[2J");
   printf("Z3660 starting...\n\r\n\r");
   printf(" ________   ______   ______   ______   ______ \n\r");
   printf("|___    /  |____  | |  ____| |  ____| |  __  | a.k.a. PishaStorm\n\r");
   printf("    /  /    ____| | | |____  | |____  | |  | | a.k.a. N\314\203Storm\n\r");
   printf("   /  /    |____  | |  __  | |  __  | | |  | |\n\r");
   printf("  /  /___   ____| | | |__| | | |__| | | |__| |\n\r");
   printf(" /_______| |______| |______| |______| |______|\n\r");
   printf("\n\r");
   printf("FPGA version number 0x%08lX\n",read_reg_s01(REG2));
#ifdef REVISION_BETA
   printf("BOOT.bin version number %d.%02d (BETA %d)\n",REVISION_MAJOR,REVISION_MINOR,REVISION_BETA);
#else
   printf("BOOT.bin version number %d.%02d\n",REVISION_MAJOR,REVISION_MINOR);
#endif
   int enable_ram=1;       // RAM initial state
   int enable_rtg=1;       // RTG initial state
   int enable_z3ram=1;     // Z3 RAM initial state
   int enable_maprom=1;    // MAPROM initial state
   int enable_mapromext=1; // MAPROM initial state

   fpga_feature_enable(enable_ram,enable_rtg,enable_z3ram,enable_maprom,enable_mapromext);

   init_shared();

   VolToPart[1].pd=0; // point second volume to second partition
   VolToPart[1].pt=2;

   read_config_file();

   read_env_files();

   if(config.cpu_ram)
      ENABLE_CPU_RAM_FPGA;
   else
      DISABLE_CPU_RAM_FPGA;

   if(config.autoconfig_ram)
      ENABLE_256MB_AUTOCONFIG;
   else
      DISABLE_256MB_AUTOCONFIG;

   if(config.scsiboot && config.cpu_ram)
   {
      DiscreteSet(REG0,FPGA_AUTOCONFIG_BOOT_EN);
      shared->scsiboot_rom_loaded=1;
   }
   else
   {
      if(config.scsiboot && config.cpu_ram==0)
      {
         printf("\e[30m\e[103mCPURAM disabled -> SCSI boot disabled\e[0m\n");
      }
      DiscreteClear(REG0,FPGA_AUTOCONFIG_BOOT_EN);
      shared->scsiboot_rom_loaded=0;
   }

   if(config.boot_mode==MUSASHI || config.boot_mode==UAE || config.boot_mode==UAEJIT)
   {
      //the autoconfig and CPU RAM are emulated in cpu_emulator(), so we don't need hardware autoconfig
      DISABLE_CPU_RAM_FPGA;
      DISABLE_MAPROM_FPGA;
      DISABLE_MAPROMEXT_FPGA;
      DISABLE_BURST_READ_FPGA;
      DISABLE_BURST_WRITE_FPGA;
      DISABLE_256MB_AUTOCONFIG;
      DISABLE_RTG_AUTOCONFIG;
      rtg_cache_policy_core0(RTG_CACHE_POLICY_FOR_EMU);
   }
   else
   {
      rtg_cache_policy_core0(RTG_CACHE_POLICY_FOR_060);
   }

   video_state=video_init();
   video_reset();
   mobotest();

   char *kickstart_pointer=0;
   char *ext_kickstart_pointer=0;
   switch(config.kickstart)
   {
#define CASE_KICKSTART(x) case x:\
          kickstart_pointer=&config.kickstart ## x[0];\
          break;
       CASE_KICKSTART(1)
       CASE_KICKSTART(2)
       CASE_KICKSTART(3)
       CASE_KICKSTART(4)
       CASE_KICKSTART(5)
       CASE_KICKSTART(6)
       CASE_KICKSTART(7)
       CASE_KICKSTART(8)
       CASE_KICKSTART(9)
   }
   switch(config.ext_kickstart)
   {
#define CASE_EXT_KICKSTART(x) case x:\
          ext_kickstart_pointer=&config.ext_kickstart ## x[0];\
          break;
       CASE_EXT_KICKSTART(1)
       CASE_EXT_KICKSTART(2)
       CASE_EXT_KICKSTART(3)
       CASE_EXT_KICKSTART(4)
       CASE_EXT_KICKSTART(5)
       CASE_EXT_KICKSTART(6)
       CASE_EXT_KICKSTART(7)
       CASE_EXT_KICKSTART(8)
       CASE_EXT_KICKSTART(9)
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

   // configure CPU frequency with verbose = 1, nbr = 0
   configure_clk(config.cpufreq,1,0);


//    PrepareHdf();
//    InitGayle();
//#define READ_BOOT_IMAGE
#ifdef READ_BOOT_IMAGE
   static FIL fil;      /* File object */
   static FATFS fatfs;
   TCHAR *Path = DEFAULT_ROOT;
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

   ltc2990_init();

   ACTIVITY_LED_ON; // ON
   DiscreteSet(REG0, FPGA_RESET);
   usleep(2500);

//   int ret=0;
//   int timer_counter_update_led=0;
/*
   // Zturn Patch for Ateros phy
   Xil_Out32(0xE000A000 + 0x244,0x00080000);
   Xil_Out32(0xE000A000 + 0x248,0x00080000);
   Xil_Out32(0xE000A000 +   0xC,0xFFF70008);
*/
//   sleep(1);

   debug_console_init();

   xadc_init();

   ethernet_init();

//   lwip_init();

   fpga_interrupt_connect(isr_video,isr_audio_tx,INT_IPL_ON_THIS_CORE);

   shared->z3_enabled =2; // always enable RTG system
   shared->z3_enabled|=config.autoconfig_ram?1:0;

   shared->load_rom_addr=(uint32_t)0x00F80000;
   shared->load_ext_rom_addr=(uint32_t)0x00F00000;

   if(config.kickstart!=0 && (kickstart_pointer[config.kickstart]!=0))
   {
      shared->load_rom_emu=load_rom();
      if(shared->load_rom_emu==1)
      {
	     ENABLE_MAPROM_FPGA;
      }
      else
      {
         printf("[INFO] kickstart is not loaded -> disabling MAPROM\n");
         DISABLE_MAPROM_FPGA;
      }
   }
   else
   {
      shared->load_rom_emu=2;
      printf("[INFO] kickstart is not loaded -> disabling MAPROM\n");
      DISABLE_MAPROM_FPGA;
   }
   if(config.ext_kickstart!=0 && (ext_kickstart_pointer[config.ext_kickstart]!=0))
   {
      shared->load_romext_emu=load_romext();
      if(shared->load_romext_emu==1)
      {
         ENABLE_MAPROMEXT_FPGA;
      }
      else
      {
         printf("[INFO] extended kickstart is not loaded -> disabling MAPROMEXT\n");
         DISABLE_MAPROMEXT_FPGA;
      }
   }
   else
   {
      shared->load_romext_emu=2;
      printf("[INFO] extended kickstart is not loaded -> disabling MAPROMEXT\n");
      DISABLE_MAPROMEXT_FPGA;
   }
// Force to disable maprom
//   DISABLE_MAPROM_FPGA;
//   DISABLE_MAPROMEXT_FPGA;
//   shared->load_rom_emu=2;
//   shared->load_romext_emu=2;

#ifndef DISABLE_SCSI
   piscsi_init();

#endif
   Xil_L1DCacheFlush();
   Xil_L2CacheFlush();

   Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT, DataAbortHandler,0);
   Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_PREFETCH_ABORT_INT, PrefetchAbortHandler,0);
   Xil_ExceptionEnable();

   ACTIVITY_LED_OFF; // OFF
   DiscreteClear(REG0, FPGA_RESET);

   if(config.boot_mode==MUSASHI || config.boot_mode==UAE || config.boot_mode==UAEJIT)
   {
      cpu_emulator(); // infinite loop inside
   }
/*
   if(config.cpufreq <= 66)
   {
      arm_write_nowait(0xFEA00000,0); // write when CPLD_RESET = 0 => PCLK/BCLK = 2
   }
   else
   {
      arm_write_nowait(0xFE500000,0); // write when CPLD_RESET = 0 => PCLK/BCLK = 4
   }
*/

   flash_colors();

   usleep(10000);
   NBR_ARM(1);        // relinquish bus

   usleep(1000);
   CPLD_RESET_ARM(1); // CPLD RUN -> 060 RUN

   while(1)
   {
/*
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
//                  enables=arm_read_amiga(0xDFF01C,WORD);
//                  arm_write_amiga(0xDFF09A,0x7FFF,WORD);
//                  arm_write_amiga(0xDFF09C,0x7FFF,WORD);
//                  arm_write_amiga(0xDFF096,0x7FFF,WORD);

//                  arm_write_amiga(0xDFF100,0x0200,WORD);
//                  arm_write_amiga(0xDFF110,0x0000,WORD);
//                  arm_write_amiga(0xDFF180,0x0000,WORD);
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
//                  arm_write_amiga(0xDFF09A,0x8000|enables,WORD);
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
*/
      switch(state68k)
      {
      case M68K_RUNNING:
         rtg_loop();
         debug_console_loop();
         if(XGpioPs_ReadPin(&GpioPs, n040RSTI)==0)
         {
            printf("Reset active (DOWN)...\n\r");
            state68k=M68K_RESET;
            reset_time_counter_max=60*4; // 4 seconds
            reset_time_counter=0;
            reset_init();
            piscsi_shutdown();
//            Xil_Out32(XSLCR_UNLOCK_ADDR, XSLCR_UNLOCK_CODE);
//            Xil_Out32(XSLCR_UNLOCK_ADDR, XSLCR_UNLOCK_CODE);
//            uint32_t RegVal = Xil_In32(A9_CPU_RST_CTRL);
//            XPS_SYS_CTRL_BASEADDR + A9_CPU_RST_CTRL_OFFSET
         }
         else
         {
//#define USER_SWITCH1
#ifdef USER_SWITCH1
            if(XGpioPs_ReadPin(&GpioPs, USER_SW1)==0)
            {
               clk_config++;
               clk_config&=3;
               switch(clk_config)
               {
                  case 0:
                     configure_clk(50,verbose,0);
                     XGpioPs_WritePin(&GpioPs, LED2, 1);
                     break;
                  case 1:
                     configure_clk(50,verbose,0);
                     XGpioPs_WritePin(&GpioPs, LED2, 0);
                     break;
                  case 2:
                     configure_clk(100,verbose,0);
                     XGpioPs_WritePin(&GpioPs, LED2, 1);
                     break;
                  case 3:
                     configure_clk(100,verbose,0);
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
         int long_reset=0;
         while(XGpioPs_ReadPin(&GpioPs, n040RSTI)==0)
         {
            reset_run(env_file_vars_temp.bootmode,reset_time_counter,reset_time_counter_max,long_reset);
            reset_time_counter++;
            if(reset_time_counter==60) // 60 -> 1 sec
       	    no_init=1;
            if(reset_time_counter==reset_time_counter_max) // 60 -> reset_run waits for vblank;
            {
//               no_init=1;
               if(long_reset==0)
               {
                  long_reset=1;
                  reset_time_counter=0;
                  reset_time_counter_max=60*4; // 4 seconds
                  env_file_vars_temp.bootmode++;
                  if(env_file_vars_temp.bootmode>=BOOTMODE_NUM)
                	  env_file_vars_temp.bootmode=0;
                  write_env_files(env_file_vars_temp);
                  for(int i=0;i<env_file_vars_temp.bootmode+1;i++)
                  {
                     DiscreteSet(REG0,FPGA_BP); // beep
                     usleep(10000);
                     DiscreteClear(REG0,FPGA_BP);
                     usleep(100000);
                  }
               }
               else // long_reset==1
               {
                  delete_env_files();
                  for(int i=0;i<5+1;i++)
                  {
                     DiscreteSet(REG0,FPGA_BP); // beep
                     usleep(10000);
                     DiscreteClear(REG0,FPGA_BP);
                     usleep(100000);
                  }
                  DiscreteSet(REG0,FPGA_BP); // beep
                  usleep(30000);
                  DiscreteClear(REG0,FPGA_BP);
                  hard_reboot(); //
               }
            }
         }
#ifndef NO_ARM_RESET_ON_AMIGA_RESET
//         audio_set_interrupt_enabled(0);
//         CPLD_RESET_ARM(0);
         hard_reboot(); //
#else
         if(long_reset)
        	 hard_reboot();

         CPLD_RESET_ARM(0);
         state68k=M68K_RUNNING;
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
         flash_colors();
         CPLD_RESET_ARM(0);
         usleep(100);
         CPLD_RESET_ARM(1);
         printf("Reset inactive (UP)...\n\r");
         video_reset();
         audio_reset();
         piscsi_init();
#endif
         break;
      }

   }
   cleanup_platform();
   return(0);
}
