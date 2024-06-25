#include <inttypes.h>
#include "mobotest.h"
#include <xparameters.h>
#include "main.h"
#include "lwip.h"
#include <stdio.h>
#include <stdlib.h>
#include "rtg/fonts.h"
#include "config_file.h"

extern SHARED *shared;
#define NOP __asm(" NOP")

void NOPX_WRITE(void)
{
	for(int i=shared->nops_write;i>0;i--)
	{
		NOP;
	}
}
void NOPX_READ(void)
{
	for(int i=shared->nops_read;i>0;i--)
	{
		NOP;
	}
}
#include "ff.h"

#define check_bus_error(A,B)

#define REG_BASE_ADDRESS XPAR_Z3660_0_BASEADDR

#define write_reg64(Offset,Data) (*(volatile uint64_t *)(REG_BASE_ADDRESS+(Offset)))=(Data)
#define write_reg(Offset,Data)   (*(volatile uint32_t *)(REG_BASE_ADDRESS+(Offset)))=(Data)

#define read_mem32(Offset) (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x14000000+((Offset&0x00FFFFFF)<<2)))=(0)
#define read_mem16(Offset) (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x18000000+((Offset&0x00FFFFFF)<<2)))=(0)
#define read_mem8(Offset)  (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x1C000000+((Offset&0x00FFFFFF)<<2)))=(0)

uint32_t last_bank=-1;
int timeout=0;

void arm_write_amiga_long(uint32_t address, uint32_t data)
{
	timeout=1000;
	while(READ_NBG_ARM()!=0);
	uint32_t bank=(address>>24)&0xFF;
	if(bank!=last_bank)
	{
		write_reg(0x18,bank);
		last_bank=bank;
	}
	NOP;
	write_mem32(address,data);
	NOP;
	do {
		NOPX_WRITE();
		timeout--;
	}
	while(read_reg(0x14)==0 && timeout>0);        // read ack
	check_bus_error(read_reg(0x14),address);
	write_reg(0x10,0x0); // ARM Bus Hi-Z
}
void arm_write_amiga_word(uint32_t address, uint32_t data)
{
	timeout=1000;
	while(READ_NBG_ARM()!=0);
	uint32_t bank=(address>>24)&0xFF;
	if(bank!=last_bank)
	{
		write_reg(0x18,bank);
		last_bank=bank;
	}
	NOP;
	write_mem16(address,data);
	NOP;
	do {
		NOPX_WRITE();
		timeout--;
	}
	while(read_reg(0x14)==0 && timeout>0);        // read ack
	check_bus_error(read_reg(0x14),address);
	write_reg(0x10,0x0); // ARM Bus Hi-Z
}
void arm_write_amiga_byte(uint32_t address, uint32_t data)
{
	timeout=1000;
	while(READ_NBG_ARM()!=0);
	uint32_t bank=(address>>24)&0xFF;
	if(bank!=last_bank)
	{
		write_reg(0x18,bank);
		last_bank=bank;
	}
	NOP;
	write_mem8(address,data);
	NOP;
	do {
		NOPX_WRITE();
		timeout--;
	}
	while(read_reg(0x14)==0 && timeout>0);        // read ack
	check_bus_error(read_reg(0x14),address);
	write_reg(0x10,0x0); // ARM Bus Hi-Z
}
uint32_t arm_read_amiga_long(uint32_t address)
{
	timeout=1000;
	while(READ_NBG_ARM()!=0);
	uint32_t bank=(address>>24)&0xFF;
	if(bank!=last_bank)
	{
		write_reg(0x18,bank);
		last_bank=bank;
	}
	NOP;
	read_mem32(address);
	NOP;
	do {
		NOPX_READ();
		timeout--;
	}
	while(read_reg(0x14)==0 && timeout>0);        // read ack
	check_bus_error(read_reg(0x14),address);
	uint32_t data_read=read_reg(0x1C); // read data
	write_reg(0x10,0x0); // ARM Bus Hi-Z
	return(data_read);
}
uint32_t arm_read_amiga_word(uint32_t address)
{
	timeout=1000;
	while(READ_NBG_ARM()!=0);
	uint32_t bank=(address>>24)&0xFF;
	if(bank!=last_bank)
	{
		write_reg(0x18,bank);
		last_bank=bank;
	}
	NOP;
	read_mem16(address);
	NOP;
	do {
		NOPX_READ();
		timeout--;
	}
	while(read_reg(0x14)==0 && timeout>0);        // read ack
	check_bus_error(read_reg(0x14),address);
	write_reg(0x10,0x0); // ARM Bus Hi-Z
	uint32_t data_read=read_reg(0x1C); // read data
	return(data_read);
}
uint32_t arm_read_amiga_byte(uint32_t address)
{
	timeout=1000;
	while(READ_NBG_ARM()!=0);
	uint32_t bank=(address>>24)&0xFF;
	if(bank!=last_bank)
	{
		write_reg(0x18,bank);
		last_bank=bank;
	}
	NOP;
	read_mem8(address);
	NOP;
	do {
		NOPX_READ();
		timeout--;
	}
	while(read_reg(0x14)==0 && timeout>0);        // read ack
	check_bus_error(read_reg(0x14),address);
	write_reg(0x10,0x0); // ARM Bus Hi-Z
	uint32_t data_read=read_reg(0x1C); // read data
	return(data_read);
}
char message[100]={0};
void handle_cache_flush(uint32_t address,uint32_t size);
extern ZZ_VIDEO_STATE vs;

void print_hdmi(int xpos, int line, char *message)
{
	displayStringAt(&Font12,xpos*Font12.Width,line*Font12.Height,(uint8_t*)message,LEFT_MODE);
	handle_cache_flush(((uint32_t)vs.framebuffer) + vs.framebuffer_pan_offset,vs.framebuffer_size);
/*	while(video_formatter_read(0)==1) //wait vblank
	{}
	while(video_formatter_read(0)==0)
	{}*/
}
char kb_tbl[256]=
	"`1234567890-=\\\x00" "0"
	"qwertyuiop[]" "\x00" "123"
	"asdfghjkl;'\x00\x00" "456"
	"\x00zxcvbnm,./\x00.789"
	" \x08\x09\x0d\x0d\x1b\x7f\x00\x00\x00-\x00\x1f\x1e\x1d\x1c" // 28-31 Cursor Keys
	"\xf9\xf8\xf7\xf6\xf5\xf4\xf3\xf2\xf1\xf0()/*+\xfa"          // F-Keys, Help
	"\xfe\xfe\xfb\xff\xfd\xfd\xfc\xfc"                           // Shift, Ctrl, Alt, Amiga
	"\x80\x81\x82\x83\x84\x85\x86\x87"
	"\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97"
	"~!@#$%^&*()_+|\x00" "0"
	"QWERTYUIOP{}\x00" "123"
	"ASDFGHJKL:\x22\x00\x00" "456"
	"\x00ZXCVBNM<>?\x00.789"
	" \x08\x09\x0d\x0d\x1b\x7f\x00\x00\x00-\x00\x1f\x1e\x1d\x1c" // 28-31 Cursor Keys
	"\xf9\xf8\xf7\xf6\xf5\xf4\xf3\xf2\xf1\xf0()/*+\xfa"          // F-Keys, Help
	"\xfe\xfe\xfb\xff\xfd\xfd\xfc\xfc"                           // Shift, Ctrl, Alt, Amiga
	"\x80\x81\x82\x83\x84\x85\x86\x87"
	"\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97"
	;
void keyboard_handshake(void)
{
	arm_write_amiga_byte(0xbfee01,0x50<<16);
	usleep(100);
	arm_write_amiga_byte(0xbfee01,0x10<<16);
	usleep(1000);
}
int line=0;
#define L_SHIFT   0x60
#define R_SHIFT   0x61
#define CAPS_LOCK 0x62
uint8_t shift_status=0;
uint8_t read_keyboard(uint8_t *data)
{
	static int a=0;
	uint8_t kb_int=(arm_read_amiga_byte(0xbfed01)>>16)&0xFF;
	if((kb_int&(1<<3))==0)
		return(0);
	arm_write_amiga_byte(0xbfed01,0x00<<16);
	a++;
	uint8_t kb_data=(arm_read_amiga_byte(0xbfec01)>>16)&0xFF;
	uint8_t data1=(~(kb_data>>1))&0x7F;
	keyboard_handshake();
//	sprintf(message,"kb_data %02X  data1  %02X      ",kb_data,data1);
//	print_hdmi(0,line+1,message);
	uint8_t down=kb_data&0x01;
	if(data1==L_SHIFT || data1==R_SHIFT || data1==CAPS_LOCK)
	{
		shift_status=down?1:0;
//		sprintf(message,"changed shift_status %d      ",shift_status);
//		print_hdmi(0,line+2,message);
		return(0);
	}
	if(shift_status)
		data1=data1+128;
	*data=kb_tbl[data1];
	return(down);
}
void do_update_hw_sprite_pos(int16_t x, int16_t y);
extern char version_string_export[];
uint32_t read_rtg_register(uint32_t zaddr);
extern DOWNLOAD_DATA download_data;
void hard_reboot(void);
void mobotest(void)
{
	sprintf(message,"[TEST] Simple Amiga bus test...");
	printf("%s\n",message);
	print_hdmi(0,line++,message);
    vs.sprite_x_base = 1000;
    vs.sprite_y_base = 1000;
    do_update_hw_sprite_pos(vs.sprite_x_base, vs.sprite_y_base);
    video_formatter_write((vs.sprite_y_adj << 16) | vs.sprite_x_adj, MNTVF_OP_SPRITE_XY);
	Font12.TextColor=0x00FFFFFF; // white

	NBR_ARM(0);        // bus request
	usleep(1000);
	CPLD_RESET_ARM(1); // CPLD RUN
	while(READ_NBG_ARM()==1); // make sure that we have the bus control
	usleep(1000);

	sprintf(message,"[TEST] 0xbfe201 %08lX",arm_read_amiga_byte(0xbfe201));
	printf("%s\n",message);
	print_hdmi(0,line++,message);
	sprintf(message,"[TEST] 0xbfe001 %08lX",arm_read_amiga_byte(0xbfe001));
	printf("%s\n",message);
	print_hdmi(0,line++,message);
	int failed=0;
	int ovl_failed=0;

	sprintf(message,"[TEST] Setting new values");
	printf("%s\n",message);
	print_hdmi(0,line++,message);
//#define FORCE_OVL_FAIL
#ifndef FORCE_OVL_FAIL
	arm_write_amiga_byte(0xbfe201,0x01<<16); // OVL as OUTPUT
#endif
	if(timeout==0)
	{
		failed=1;
		Font12.TextColor=0x00FF0000; // red
		sprintf(message,"[TEST] Timeout when writing 1 to 0xbfe201");
		printf("%s\n",message);
		print_hdmi(0,line++,message);
		Font12.TextColor=0x00FFFFFF; // white
	}
	arm_write_amiga_byte(0xbfe001,0x00<<16); // OVL LOW to see the CHIP RAM
	if(timeout==0)
	{
		failed=1;
		Font12.TextColor=0x00FF0000; // red
		sprintf(message,"[TEST] Timeout when writing 0 to 0xbfe001");
		printf("%s\n",message);
		print_hdmi(0,line++,message);
		Font12.TextColor=0x00FFFFFF; // white
	}
	sprintf(message,"[TEST] 0xbfe201 %08lX",arm_read_amiga_byte(0xbfe201));
	printf("%s\n",message);
	print_hdmi(0,line++,message);
	sprintf(message,"[TEST] 0xbfe001 %08lX",arm_read_amiga_byte(0xbfe001));
	printf("%s\n",message);
	print_hdmi(0,line++,message);

	arm_write_amiga_long(0x00000000,0xAA55AA55);
	if(timeout==0)
	{
		failed=1;
		Font12.TextColor=0x00FF0000; // red
		sprintf(message,"[TEST] Timeout when writing 0xAA55AA55 to 0x00000000");
		printf("%s\n",message);
		print_hdmi(0,line++,message);
		Font12.TextColor=0x00FFFFFF; // white
	}
	arm_write_amiga_long(0x00100000,0x55AA55AA);
	if(timeout==0)
	{
		failed=1;
		Font12.TextColor=0x00FF0000; // red
		sprintf(message,"[TEST] Timeout when writing 0x55AA55AA to 0x00100000");
		printf("%s\n",message);
		print_hdmi(0,line++,message);
		Font12.TextColor=0x00FFFFFF; // white
	}
	uint32_t data1=arm_read_amiga_long(0x00000000);
	if(timeout==0)
	{
		failed=1;
		Font12.TextColor=0x00FF0000; // red
		sprintf(message,"[TEST] Timeout when reading from 0x00000000");
		printf("%s\n",message);
		print_hdmi(0,line++,message);
		Font12.TextColor=0x00FFFFFF; // white
	}
	uint32_t data2=arm_read_amiga_long(0x00100000);
	if(timeout==0)
	{
		failed=1;
		Font12.TextColor=0x00FF0000; // red
		sprintf(message,"[TEST] Timeout when reading from 0x00100000");
		printf("%s\n",message);
		print_hdmi(0,line++,message);
		Font12.TextColor=0x00FFFFFF; // white
	}
	if(data1==0xAA55AA55 && data2==0x55AA55AA)
	{
		sprintf(message,"[TEST] Test basic Amiga CHIP RAM ok");
		printf("%s\n",message);
		print_hdmi(0,line++,message);
	}
	else
	{
		failed=1;
		Font12.TextColor=0x00FF0000; // red
		sprintf(message,"[TEST] Test basic Amiga CHIP RAM FAILED!!!");
		printf("%s\n",message);
		print_hdmi(0,line++,message);
		Font12.TextColor=0x00FFFFFF; // white
		if(data1==0xAA55AA55)
		{
			sprintf(message,"[TEST] (0x00000000) -> 0x%08lX == 0xAA55AA55  OK!!!",data1);
			printf("%s\n",message);
			print_hdmi(0,line++,message);
		}
		else if(data1==0x11144EF9)
		{
			ovl_failed=1;
			Font12.TextColor=0x00FF0000; // red
			sprintf(message,"[TEST] (0x00000000) -> 0x%08lX == 0x11144EF9 This is the kickstart!!!",data1);
			printf("%s\n",message);
			print_hdmi(0,line++,message);
			Font12.TextColor=0x00FFFFFF; // white
		}
		else
		{
			Font12.TextColor=0x00FF0000; // red
			sprintf(message,"[TEST] (0x00000000) -> 0x%08lX instead of 0xAA55AA55",data1);
			printf("%s\n",message);
			print_hdmi(0,line++,message);
			Font12.TextColor=0x00FFFFFF; // white
		}
		if(data2==0x55AA55AA)
		{
			sprintf(message,"[TEST] (0x00100000) -> 0x%08lX == 0x55AA55AA  OK!!!",data2);
			printf("%s\n",message);
			print_hdmi(0,line++,message);
		}
		else
		{
			Font12.TextColor=0x00FF0000; // red
			sprintf(message,"[TEST] (0x00100000) -> 0x%08lX instead of 0x55AA55AA",data2);
			printf("%s\n",message);
			print_hdmi(0,line++,message);
			Font12.TextColor=0x00FFFFFF; // white
		}
	}
	if(failed || config.enable_test)
	{
		if(failed)
			sprintf(message,"[TEST] Something failed -> exhaustive test");
		else
			sprintf(message,"[TEST] Test Enabled in z3660cfg.txt file -> exhaustive test");
		printf("%s\n",message);
		print_hdmi(0,line++,message);

		sleep(1); // wait time to allow monitor to show the screen
		failed=0;
		uint8_t * test_mem1=malloc(0x200000);
		uint8_t * test_mem2=malloc(0x200000);
		for(uint32_t i=0;i<0x200000;i++)
			test_mem1[i]=(uint8_t)(rand()&0xFF);
		sprintf(message,"[TEST] Writing 2MB of random data");
		printf("%s",message);
		print_hdmi(0,line++,message);
		int x=34;
		for(uint32_t i=0;i<0x200000;i+=4)
		{
			arm_write_amiga_long(i,((uint32_t *)test_mem1)[i>>2]);
			if(timeout==0)
			{
				if(failed==0)
					printf("\n");
				sprintf(message,"[TEST] Timeout when writing 0x%08lX to 0x%08lX",((uint32_t *)test_mem1)[i>>2],i);
				printf("%s\n",message);
				Font12.TextColor=0x00FF0000; // red
				sprintf(message,"Timeout when writing 0x%08lX to 0x%08lX      ",((uint32_t *)test_mem1)[i>>2],i);
				print_hdmi(60,0,message);
				Font12.TextColor=0x00FFFFFF; // white
				failed=1;
			}
			if(failed==0 && (i%0x10000)==0)
			{

				sprintf(message,".");
				printf("%s",message);
				print_hdmi(x++,line-1,message);
			}
		}
		if(failed==0)
		{
			sprintf(message,"done");
			printf("%s\n",message);
			print_hdmi(x,line-1,message);
		}
		failed=0;
		x=34;
		sprintf(message,"[TEST] Reading 2MB of data       ");
		printf("%s",message);
		print_hdmi(0,line++,message);
		for(uint32_t i=0;i<0x200000;i+=4)
		{
			((uint32_t *)test_mem2)[i>>2]=arm_read_amiga_long(i);
			if(timeout==0)
			{
				if(failed==0)
					printf("\n");
				sprintf(message,"[TEST] Timeout when reading from 0x%08lX",i);
				printf("%s\n",message);
				Font12.TextColor=0x00FF0000; // red
				sprintf(message,"Timeout when reading from 0x%08lX               ",i);
				print_hdmi(60,0,message);
				Font12.TextColor=0x00FFFFFF; // white
				failed=1;
			}
			if(failed==0 && (i%0x10000)==0)
			{
				sprintf(message,".");
				printf("%s",message);
				print_hdmi(x++,line-1,message);
			}
		}
		if(failed==0)
		{
			sprintf(message,"done");
			printf("%s\n",message);
			print_hdmi(x++,line-1,message);
		}
		sprintf(message,"[TEST] Comparing 2MB of data     ");
		printf("%s",message);
		print_hdmi(0,line++,message);
		failed=0;
		x=34;
		for(uint32_t i=0;i<0x200000;i+=4)
		{
			uint32_t data1=((uint32_t *)test_mem1)[i>>2];
			uint32_t data2=((uint32_t *)test_mem2)[i>>2];
			if(data1!=data2)
			{
				if(failed==0)
					printf("\n");
				sprintf(message,"[TEST] Address 0x%08lX failed: W/R %08lX/%08lX",i,data1,data2);
				printf("%s\n",message);
				Font12.TextColor=0x00FF0000; // red
				sprintf(message,"Address 0x%08lX failed: W/R %08lX/%08lX",i,data1,data2);
				print_hdmi(60,0,message);
				Font12.TextColor=0x00FFFFFF; // white
				failed=1;
			}
			if(failed==0 && (i%0x10000)==0)
			{
				sprintf(message,".");
				printf("%s",message);
				print_hdmi(x++,line-1,message);
			}
			if(ovl_failed)
			{
				if(i<0x80000)
				{
					i+=0x100-4; // if ovl is not working this space will fail completely, so speed up
				}
			}
		}
		if(failed==0)
		{
			sprintf(message,"done");
			printf("%s\n",message);
			print_hdmi(x++,line-1,message);
		}
		free(test_mem1);
		free(test_mem2);
	}

	sprintf(message,"[TEST] Restoring...");
	printf("%s\n",message);
	print_hdmi(0,line++,message);
	arm_write_amiga_byte(0xbfe001,0xFF<<16); // OVL HIGH to see the ROM
	if(timeout==0)
	{
		failed=1;
		Font12.TextColor=0x00FF0000; // red
		sprintf(message,"[TEST] Timeout when writing 0Xff to 0xbfe001");
		printf("%s\n",message);
		print_hdmi(0,line++,message);
		Font12.TextColor=0x00FFFFFF; // white
	}
	arm_write_amiga_byte(0xbfe201,0x00<<16); // OVL HIGH to see the ROM
	if(timeout==0)
	{
		failed=1;
		Font12.TextColor=0x00FF0000; // red
		sprintf(message,"[TEST] Timeout when writing 0 to 0xbfe201");
		printf("%s\n",message);
		print_hdmi(0,line++,message);
		Font12.TextColor=0x00FFFFFF; // white
	}

	sprintf(message,"[TEST] 0xbfe201 %08lX",arm_read_amiga_byte(0xbfe201));
	printf("%s\n",message);
	print_hdmi(0,line++,message);
	sprintf(message,"[TEST] 0xbfe001 %08lX",arm_read_amiga_byte(0xbfe001));
	printf("%s\n",message);
	print_hdmi(0,line++,message);

	uint8_t keybd_data=0;

	sprintf(message,"Press 'C' key to start the ARM configuration tool...");
	print_hdmi(0,line++,message);

	keybd_data=0;
	int time=2;
	if(config.enable_test)
		time=10;
	printf("Continuing in %d seconds...\n",time);
	for(int i=time*1000;i>0;i--)
	{
		if(read_keyboard(&keybd_data))
		{
			if(keybd_data=='c' || keybd_data=='C')
			{
				sprintf(message,"Starting console...                 ");
				printf("Starting console... Use the Amiga keyboard\n");
				print_hdmi(0,line++,message);
#define MSG_LINE(X) sprintf(message,X);print_hdmi(0,line++,message)
				MSG_LINE("Options:");
				MSG_LINE("'I' to connect to Internet and check if there is an update version");
				MSG_LINE("'U' to download latest BOOT.bin overwriting Z3660.bin file");
//				MSG_LINE("'F' to download latest BOOT.bin overwriting FAILSAFE.bin file");
//				MSG_LINE("'B' to copy BOOT.bin to FAILSAFE.bin");
//				MSG_LINE("'N' to copy BOOT.bin to Z3660.bin");
//				MSG_LINE("'M' to copy Z3660.bin to BOOT.bin");
//				MSG_LINE("'H' to copy Z3660.bin to FAILSAFE.bin");
//				MSG_LINE("'J' to copy FAILSAFE.bin to BOOT.bin");
//				MSG_LINE("'K' to copy FAILSAFE.bin to Z3660.bin");
				MSG_LINE("'R' to reboot");
				MSG_LINE(" ");
				MSG_LINE("Note: BOOT.bin boots  the  system  and  it  will  search  for  Z3660.bin  to  continue");
				MSG_LINE("      booting. If the Z3660.bin file is bad, then it will search for FAILSAFE.bin file");
				MSG_LINE("      to boot. If FAILSAFE.bin is bad then BOOT.bin will continue booting from itself.");

				goto start_console;
			}
		}
		if((i%1000)==0)
		{
			sprintf(message,"Continuing in %i %s   ",i/1000,i/1000==1?"second":"seconds");
			print_hdmi(0,line,message);
		}
		usleep(1000);
	}
	sprintf(message,"Rebooting                     ");
	print_hdmi(0,line,message);

	NBR_ARM(1);
	usleep(1000);
	CPLD_RESET_ARM(0); // CPLD RESET
	return;
start_console:
	int connected=0;
	while(1)
	{
		if(read_keyboard(&keybd_data))
		{
			if(keybd_data=='i' || keybd_data=='I')
			{
				if(connected==0)
					connected=lwip_connect();
				int ok=lwip_get_update_version();
				if(ok)
				{
					sprintf(message,"Latest version is %s",version_string_export);
					print_hdmi(0,line++,message);
					int data=read_rtg_register(REG_ZZ_FW_VERSION);
					int v_major=(data>>8)&0xFF;
					int v_minor=(data)&0xFF;
					int beta=read_rtg_register(REG_ZZ_FW_BETA);
					if(beta==0)
						sprintf(message,"The loaded BOOT.bin version is %d.%02d",v_major,v_minor);
					else
						sprintf(message,"The loaded BOOT.bin version is %d.%02d BETA %d",v_major,v_minor,beta);
					print_hdmi(0,line++,message);
				}
			}
			else if(keybd_data=='u' || keybd_data=='U')
			{
				if(connected==0)
					connected=lwip_connect();
				DATA=malloc(16*1024*1024);
				int ok=lwip_get_update();
				if(ok)
				{
					uint32_t checksum32=0;
					uint32_t *data_u32=(uint32_t *)DATA;
					for(int i=0;i<download_data.filesize/4;i++)
						checksum32+=__builtin_bswap32(data_u32[i]);
					sprintf(message,"Checksum-32 %lu (%08lX) %lu (%08lX)",checksum32,checksum32,download_data.checksum32,download_data.checksum32);
					print_hdmi(0,line++,message);
					if(checksum32==download_data.checksum32)
					{
						FIL fil;
						FATFS fatfs;
						TCHAR *Path = "0:/";
						f_mount(&fatfs, Path, 1); // 1 mount immediately
						f_open(&fil,"Z3660.bin", FA_CREATE_ALWAYS | FA_WRITE);
						UINT NumBytesWritten;
						f_write(&fil, DATA, download_data.filesize,&NumBytesWritten);
						f_close(&fil);

						sprintf(message,"File Z3660.bin written (%d bytes)",NumBytesWritten);
						print_hdmi(0,line++,message);
						f_mount(0,Path,1); // unmount
					}
					else
					{
						sprintf(message,"File Z3660.bin NOT written (bad checksum)");
						print_hdmi(0,line++,message);
					}
				}
				free(DATA);
			}
			else if(keybd_data=='r' || keybd_data=='R')
			{
				// reboot
				hard_reboot();
			}
//			sprintf(message,"Detected key %02X %c               ",keybd_data,keybd_data);
//			print_hdmi(0,line,message);
		}
		if(connected)
			lwip_run();
	}

}
void hdmi_tick(void)
{
	char chars[]="\\|/-";
	static int i=0;
	sprintf(message,"%c",chars[i++]);
	i=i&3;
	print_hdmi(0,line+1,message);
}