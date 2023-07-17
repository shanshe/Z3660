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
	printf("Loading Kickstart on address: 0x%08X\n",(uint32_t)ROM);
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
	ret=f_open(&fil,config.kickstart, FA_OPEN_ALWAYS | FA_READ);
	if(ret!=0)
	{
		printf("Error opening file \"%s\"\nHALT!!!\n",config.kickstart);
		while(1);
	}
	unsigned int NumBytesRead;
	printf("Reading %s file:\r\n[----------------]\r\n\033[F",config.kickstart);
	for(int i=0,j=0,k=2;i<0x80000;i+=4,j+=4)
	{
		if(j==(512*1024/16))
		{
			j=0;
			printf("%.*s\r\n\033[F",(int)++k,"[================]");
		}
		uint8_t buff[4];
		f_read(&fil, buff, 4,&NumBytesRead);
		if(NumBytesRead!=4)
		{
			printf("Error reading at 0x%08x\nHALT!!!",i);
			while(1);
		}
		ROM[i  ]=buff[0];
		ROM[i+1]=buff[1];
		ROM[i+2]=buff[2];
		ROM[i+3]=buff[3];
	}
	Xil_DCacheFlush();
	f_close(&fil);
	f_mount(NULL, Path, 0); // NULL unmount, 0 delayed
	printf("\r\nFile read OK\r\n");
//	while(1);
#endif
#ifdef DUMP_ROM
	char Filename[]="DiagROM11.rom";
	static FIL fil;		/* File object */
	static FATFS fatfs;
	TCHAR *Path = "0:/";
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
#ifdef WRITE_BUFFER
void arm_write_amiga_wb(uint32_t address, uint32_t data, uint32_t size)
{
	while(shared->bus_busy);
	shared->bus_busy=1;
	write_reg(0x08,address);          // address
	write_reg(0x0C,data);             // data
	write_reg(0x10,0x11|WRITE_|size); // command write
	do{
		write_reg(0x10,0x11|WRITE_|size);
		__asm( "nop");
	}while(read_reg(0x14)==0);         // read ack
	write_reg(0x10,0x01|WRITE_);      // confirm ack (bit4=0), tristate bus (READ_)
	shared->bus_busy=0;
}

WriteBuffer *wb_pop(void)
{
	uint8_t idx=shared->wb_output_ptr;
	uint32_t ptr=(idx+1)&(256-1);
	if(ptr==shared->wb_input_ptr)
		shared->wb_buffer_state=WB_EMPTY;
	else
		shared->wb_buffer_state=WB_DATA_AVAILABLE;
	shared->wb_output_ptr=ptr;
	return(&shared->wb[idx]);
}

void wb_run(void)
{
	if(shared->wb_updating==0)
	{
		shared->wb_updating=1;
		if(shared->wb_buffer_state!=WB_EMPTY && shared->bus_busy==0)
		{
			WriteBuffer *wb_ptr;
			wb_ptr=wb_pop();
			arm_write_amiga_wb(wb_ptr->address,wb_ptr->data,wb_ptr->size);
		}
		shared->wb_updating=0;
	}
}
#endif
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
	shared->load_rom_emu=0;

	while(1)
	{
#ifdef WRITE_BUFFER
		wb_run();
#endif
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
			}
		}
	}
}

