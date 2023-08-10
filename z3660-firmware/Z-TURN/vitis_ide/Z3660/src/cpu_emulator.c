/*
 * cpu_emulator.c
 *
 *  Created on: 3 dic. 2022
 *      Author: shanshe
 *
 *  Just a simple wrapper for Musashi
 */

#include <stdio.h>
#include "main.h"
#include "sleep.h"
#include "cpu_emulator.h"
#include "config_file.h"
#include "xscuwdt.h"
#include "scsi/scsi.h"

void write_rtg_register(uint16_t zaddr,uint32_t zdata);

SHARED *shared;
void init_shared(void)
{
	shared=(SHARED *)0xFFFF0000;
}

void other_tasks(void);
extern CONFIG config;
void load_rom(void)
{
//#define DUMP_ROM
#define MAP_ROM
#ifdef MAP_ROM
	static FIL fil;		/* File object */
	static FATFS fatfs;
	uint8_t *ROM=(uint8_t *)shared->load_rom_addr;
	printf("Loading Kickstart on address: 0x%08lX\n",(uint32_t)ROM);
	TCHAR *Path = DEFAULT_ROOT;
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
	ret=f_open(&fil,config.kickstart, FA_OPEN_EXISTING | FA_READ);
	if(ret!=0)
	{
		printf("Error opening file \"%s\"\nHALT!!!\n",config.kickstart);
		while(1);
	}
	unsigned int NumBytesRead;
	printf("Reading %s file:\r\n[----------------]\r\n\033[F",config.kickstart);
#define BYTES_TO_READ 128
	for(int i=0,j=0,k=2;i<0x80000;i+=BYTES_TO_READ,j+=BYTES_TO_READ)
	{
		if(j==(512*1024/16))
		{
			j=0;
			printf("%.*s\r\n\033[F",(int)++k,"[================]");
		}
		f_read(&fil, ROM+i, BYTES_TO_READ,&NumBytesRead);
		if(NumBytesRead!=BYTES_TO_READ)
		{
			printf("\nError reading at file offset 0x%08x\nHALT!!!",i);
			while(1);
		}
//		uint8_t buff[BYTES_TO_READ];
//		for(int i1=0;i1<BYTES_TO_READ;i1++)
//			ROM[i+i1]=buff[i1];
	}
	Xil_DCacheFlush();
	f_close(&fil);
	f_mount(NULL, Path, 0); // NULL unmount, 0 delayed
	printf("\r\nFile read OK\r\n");
//	while(1);
#endif
#ifdef DUMP_ROM
	char Filename[]=DEFAULT_ROOT "DiagROM11.rom";
	static FIL fil;		/* File object */
	static FATFS fatfs;
	TCHAR *Path = DEFAULT_ROOT;
	f_mount(&fatfs, Path, 1); // 1 mount immediately
//	f_open(&fil,Filename, FA_OPEN_ALWAYS | FA_READ);
	f_open(&fil,Filename, FA_OPEN_ALWAYS | FA_WRITE);
//	f_lseek(&fil, 4);
//	UINT NumBytesRead;
	UINT NumBytesWritten;
	printf("Dumping %s file:\r\n[----------]\r\n\033[F",Filename);
	for(int i=0,j=0,k=0;i<0x80000;i+=4,j+=4)
	{
		if(j==512*1024/10)
		{
			j=0;
			printf("%.*s\r\n\033[F",(int)++k,"[==========]");
		}
		uint32_t data=arm_read_amiga_map(i,LONG);
		uint8_t buff[4];
		buff[0]=(data>>24)&0xFF;
		buff[1]=(data>>16)&0xFF;
		buff[2]=(data>> 8)&0xFF;
		buff[3]=(data    )&0xFF;
		f_write(&fil, buff, 4,&NumBytesWritten);
	}
	f_close(&fil);
	f_mount(NULL, Path, 0); // NULL unmount, 0 delayed
	printf("\r\nFile written OK\r\n");
	while(1);
#endif
#ifdef SERIAL_DUMP_ROM
	for(int i=0x00F80000-0x00F80000;i<0x01000000-0x00F80000;)
	{
		printf("%08lX: ",i);
		for(int j=0;j<8;j++,i+=4)
			printf("%08lX ",arm_read_amiga_map(i,LONG));
		printf("\n");
	}
	while(1);
#endif
	Xil_ExceptionEnable();
}
int last_irq=-1;
#define IRQ_FIFO_MAX 100
int irq_fifo[IRQ_FIFO_MAX];
unsigned int irq_rd=0;
unsigned int irq_wr=0;
void cpu_emulator_reset(void)
{
//	mux.nbr_arm=0;
	CPLD_RESET_ARM(0);
	usleep(1000);
	CPLD_RESET_ARM(1);
	printf("Resetting...\n\r");
}

void hard_reboot(void)
{
	cpu_emulator_reset();
#define PS_RST_CTRL_REG			(XPS_SYS_CTRL_BASEADDR + 0x244)
#define PS_RST_MASK			0x3	/**< PS software reset (Core 1 reset)*/
	Xil_Out32(PS_RST_CTRL_REG, PS_RST_MASK);
	usleep(100);
	Xil_Out32(PS_RST_CTRL_REG, PS_RST_MASK);
	XScuWdt_Config *config=XScuWdt_LookupConfig(XPAR_SCUWDT_0_DEVICE_ID);
	XScuWdt instance;
	XScuWdt_CfgInitialize(&instance,config,config->BaseAddr);
	XScuWdt_LoadWdt(&instance,0xFF);
	XScuWdt_Start(&instance);
	XScuWdt_SetWdMode(&instance);
	while(1);
}
void cpu_emulator(void)
{
	NBR_ARM(0);

	while(READ_NBG_ARM()==1);
	shared->load_rom_emu=0;
	printf("Bus Request from ARM OK\n");

	video_reset();
	audio_reset();

	printf("Starting CPU emulator on Core1\n");
	*(volatile uint32_t *)(0xFFFFFFF0)=0x30000000;
	Xil_DCacheFlush();
	Xil_ICacheInvalidate();
	__asm__("sev");
//	m68ki_cpu_core *state= &m68ki_cpu;
	shared->shared_data=1;
	printf("Waiting ack from core1... ");
	while(shared->shared_data==1){}
	printf("OK\n");
	while(shared->load_rom_emu==0){}
	if(shared->load_rom_emu==1)
		load_rom();
	if(config.scsiboot)
		shared->boot_rom_loaded=piscsi_init();
	else
		shared->boot_rom_loaded=0;
	for(int i=0;i<7;i++)
	{
		if(config.scsi[i][0]!=0)
			piscsi_map_drive(config.scsi[i], i);
	}

	shared->load_rom_emu=0;

	while(1)
	{
		if(shared->shared_data==1)
		{
			shared->shared_data=0;
			if(shared->write_rtg==1)
			{
				uint32_t addr=shared->write_rtg_addr;
				uint32_t data=shared->write_rtg_data;
				write_rtg_register(addr,data);
				shared->write_rtg=0;
			}
			else if(shared->write_scsi==1)
			{
				int type=shared->write_scsi_type;
				uint32_t addr=shared->write_scsi_addr;
				uint32_t data=shared->write_scsi_data;
//				handle_piscsi_write(addr,data,type);
//				shared->write_scsi=0;
				shared->write_scsi_in_progress=1;
				handle_piscsi_write(addr,data,type);
				shared->write_scsi_in_progress=0;
				shared->write_scsi=0;
			}
			else if(shared->read_scsi==1)
			{
				int type=shared->read_scsi_type;
				uint32_t addr=shared->read_scsi_addr;
				shared->read_scsi_data=handle_piscsi_read(addr,type);
				shared->read_scsi=0;
			}
//			else if(shared->read_video==1)
//			{
//				shared->read_video_data=video_formatter_read(0);
//				shared->read_video=0;
//			}
//			else if(shared->set_fc==1)
//			{
//				cpu_set_fc(shared->set_fc_data);
//				shared->set_fc=0;
//			}
			else if(shared->reset_emulator==1)
			{
				cpu_emulator_reset();
				shared->reset_emulator=0;
			}
			else if(shared->core0_hold==1)
			{
				shared->core0_hold_ack=1;
				do
				{
					usleep(1000);
				}while(shared->core0_hold==1);
				shared->core0_hold_ack=0;
			}
		}
//		else
		{
			other_tasks();
			static int counter=0;
			counter++;
			if(counter==1024)
			{
				counter=0;
				if(XGpioPs_ReadPin(&GpioPs, n040RSTI)==0)
				{
					printf("[Core1] Reset active (DOWN)...\n\r");
					while(XGpioPs_ReadPin(&GpioPs, n040RSTI)==0){}
					printf("[Core1] Reset inactive (UP)...\n\r");
					hard_reboot();
				}
			}
		}
	}
}

