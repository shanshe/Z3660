#define CONSOLE_ENABLED 1
#ifdef CONSOLE_ENABLED
#include "console.h"
#include "main.h"
#include <xil_printf.h>
#include <xparameters.h>
#include "xuartps_hw.h"

void console_help(void);
void dump_mmu(void);

CONSOLE con;
extern SHARED *shared;
extern int eth_backlog_nag_counter_max;
typedef enum {
	A,

	DRTG,   DEBUG_RTG,
	DSCSI,  DEBUG_SCSI,
	DAUDIO, DEBUG_AUDIO,
	DIIC,   DEBUG_IIC,
	H,      HELP,
	SD,     STOP_DEBUG,
	DMMU,   DUMP_MMU,
	DM,     DUMP_MEM,
	DMS,    DUMP_MEM_SWAP,
	S,      STEP,
	INW,    INC_NOPS_WRITE,
	DNW,    DEC_NOPS_WRITE,
	INR,    INC_NOPS_READ,
	DNR,    DEC_NOPS_READ,
	INE,    INC_NAG_ETH,
	DNE,    DEC_NAG_ETH,
	DIS,    DISASSEMBLE,
	DISS,   DISASSEMBLE_STEP,
	DISR,   DISASSEMBLE_RUN,
	RSTE,   RESET_EMULATOR,
	SIIC,   STOP_IIC,

	NUM_COMMANDS
} COMMANDS;
const char *command_names[NUM_COMMANDS] = {
    "A",

	"DRTG",   "DEBUG RTG",
	"DSCSI",  "DEBUG SCSI",
	"DAUDIO", "DEBUG AUDIO",
	"DIIC",   "DEBUG IIC",
	"H",      "HELP",
	"SD",     "STOP DEBUG",
	"DMMU",   "DUMP MMU",
	"DM",     "DUMP MEM",
	"DMS",    "DUMP MEM SWAP",
	"S",      "STEP",
	"INW",    "INC NOPS WRITE",
	"DNW",    "DEC NOPS WRITE",
	"INR",    "INC NOPS READ",
	"DNR",    "DEC NOPS READ",
	"INE",    "INC NAG ETH",
	"DNE",    "DEC NAG ETH",
	"DIS",    "DISASEMBLE",
	"DISS",   "DISASEMBLE STEP",
	"DISR",   "DISASEMBLE RUN",
	"RSTE",   "RESET EMULATOR",
	"SIIC",   "STOP IIC",
};

void console_init(void)
{
	con.cmd_pointer=0;
	con.debug_rtg=0;
	con.subcmd=0;
	con.debug_scsi=0;
	con.debug_audio=0;
	con.debug_i2c=0;
	con.stop_i2c=0;
	con.step=0;
	con.hist_pointer=0;
	con.hist_pointer_top=0;
	for(int i=0;i<MAX_HIST;i++)
		con.cmd_hist[i][0]=0;
}
uint32_t hextoi(char *str)
{
	int index=0;
	uint32_t data=0;
	if(str[0]==0 && str[1]=='x')
		index=2;
	if(str[0]=='h')
		index=1;
	while(str[index]!=0)
	{
		char c=str[index++];
		if(c>='A' && c<='F') c=c-'A'+10;
		else if(c>='0' && c<='9') c=c-'0';
		data=(data<<4)+c;
	}
	return(data);
}
void dump_mem(uint32_t address, uint32_t length, int swap)
{
	if(swap==0)
	{
		for(uint32_t i=0;i<length;i+=4*8)
		{
			printf("0x%08lX ",address+i);
			for(uint32_t j=0;j<4*8;j+=4)
			{
				printf("%08lX ",*((uint32_t *)(address+i+j)));
			}
			printf("\n");
		}
	}
	else
	{
		for(uint32_t i=0;i<length;i+=4*8)
		{
			printf("0x%08lX ",address+i);
			for(uint32_t j=0;j<4*8;j+=4)
			{
				printf("%08lX ",__builtin_bswap32(*((uint32_t *)(address+i+j))));
			}
			printf("\n");
		}
	}
}
#define ESC 27
void console_loop(void)
{
	if(XUartPs_IsReceiveData(STDIN_BASEADDRESS))
	{
		char c = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
		if(con.subcmd==0)
		{
			if(c>='a' && c<='z') c+='A'-'a';
			if((c>='A' && c<='Z')||(c==' ')||(c==8)||(c==ESC))
			{
				if(c==ESC)
				{
//					XUartPs_SendByte(STDOUT_BASEADDRESS, '\\');
//					XUartPs_SendByte(STDOUT_BASEADDRESS, '2');
//					XUartPs_SendByte(STDOUT_BASEADDRESS, '7');
					while(XUartPs_IsReceiveData(STDIN_BASEADDRESS)==0);
					c=XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
//					XUartPs_SendByte(STDOUT_BASEADDRESS, c);
					if(c=='[')
					{
						while(XUartPs_IsReceiveData(STDIN_BASEADDRESS)==0);
						c=XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
//						XUartPs_SendByte(STDOUT_BASEADDRESS, c);
//						XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
//						XUartPs_SendByte(STDOUT_BASEADDRESS, '<');
//						XUartPs_SendByte(STDOUT_BASEADDRESS, '-');
						if(c=='A')
						{
							// UP
							while(con.cmd_pointer>0)
							{
								XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
								XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
								XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
								con.cmd_pointer--;
							}
							con.cmd_buf[con.cmd_pointer]=0;
							int i=0;
							if(con.hist_pointer>0)
								con.hist_pointer--;
							else
								con.hist_pointer=MAX_HIST-1;
							strcpy(con.cmd_buf,con.cmd_hist[con.hist_pointer]);
							while(con.cmd_hist[con.hist_pointer][i]!=0)
							{
								XUartPs_SendByte(STDOUT_BASEADDRESS, con.cmd_hist[con.hist_pointer][i++]);
								con.cmd_pointer++;
							}
						}
						else if(c=='B')
						{
							// DOWN
							while(con.cmd_pointer>0)
							{
								XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
								XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
								XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
								con.cmd_pointer--;
							}
							con.cmd_buf[con.cmd_pointer]=0;
							int i=0;
							con.hist_pointer++;
							if(con.hist_pointer>=MAX_HIST) con.hist_pointer=0;
							if(con.hist_pointer!=con.hist_pointer_top)
							{
								strcpy(con.cmd_buf,con.cmd_hist[con.hist_pointer]);
								while(con.cmd_hist[con.hist_pointer][i]!=0)
								{
									XUartPs_SendByte(STDOUT_BASEADDRESS, con.cmd_hist[con.hist_pointer][i++]);
									con.cmd_pointer++;
								}
							}
						}
						else
						{
							con.cmd_buf[con.cmd_pointer]=0;
						}
					}
					else
					{
						con.cmd_buf[con.cmd_pointer]=0;
					}
				}
				else
				{
					XUartPs_SendByte(STDOUT_BASEADDRESS, c);
					if(c==8) // backspace
					{
						if(con.cmd_pointer>0)
						{
							XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
							XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
							con.cmd_pointer--;
						}
						con.cmd_buf[con.cmd_pointer]=0;
					}
					else
					{
						con.cmd_buf[con.cmd_pointer++]=c;
						if(con.cmd_pointer>90) con.cmd_pointer=90;
					}
				}
			}
			else if((c=='\r' || c=='\n') && con.cmd_pointer>0)
			{
				con.cmd_buf[con.cmd_pointer]=0;
				strcpy(con.cmd_hist[con.hist_pointer_top++],con.cmd_buf);
				if(con.hist_pointer_top>=MAX_HIST) con.hist_pointer_top=0;
				con.hist_pointer=con.hist_pointer_top;
				XUartPs_SendByte(STDOUT_BASEADDRESS, '\r');
				XUartPs_SendByte(STDOUT_BASEADDRESS, '\n');
				uint8_t command_found=0;
				for(int i=0;i<NUM_COMMANDS;i++)
				{
					if(strcmp(con.cmd_buf,command_names[i])==0)
					{
						command_found=1;
						switch(i)
						{
						case A:
							xil_printf("test 'A' command\r\n");
							con.subcmd=0;
							break;
						case DRTG:
						case DEBUG_RTG:
							con.debug_rtg=!con.debug_rtg;
							if(con.debug_rtg)
								xil_printf("DEBUG RTG ON\r\n");
							else
								xil_printf("DEBUG RTG OFF\r\n");
							con.subcmd=0;
							break;
						case DSCSI:
						case DEBUG_SCSI:
							con.debug_scsi=!con.debug_scsi;
							if(con.debug_scsi)
								xil_printf("DEBUG SCSI ON\r\n");
							else
								xil_printf("DEBUG SCSI OFF\r\n");
							con.subcmd=0;
							break;
						case DAUDIO:
						case DEBUG_AUDIO:
							con.debug_audio=!con.debug_audio;
							if(con.debug_audio)
								xil_printf("DEBUG AUDIO ON\r\n");
							else
								xil_printf("DEBUG AUDIO OFF\r\n");
							con.subcmd=0;
							break;
						case DIIC:
						case DEBUG_IIC:
							con.debug_i2c=!con.debug_i2c;
							if(con.debug_i2c)
								xil_printf("DEBUG I2C ON\r\n");
							else
								xil_printf("DEBUG I2C OFF\r\n");
							con.subcmd=0;
							break;
						case H:
						case HELP:
							console_help();
							con.subcmd=0;
							break;
						case SD:
						case STOP_DEBUG:
							con.debug_rtg=0;
							con.debug_scsi=0;
							con.debug_audio=0;
							con.debug_i2c=0;
							xil_printf("ALL DEBUG STOPPED\r\n");
							con.subcmd=0;
							break;
						case DMMU:
						case DUMP_MMU:
							dump_mmu();
							con.subcmd=0;
							break;
						case DM:
						case DUMP_MEM:
							xil_printf("Address (hex): ");
							con.subcmd=1;
							con.cmd=DM;
							break;
						case DMS:
						case DUMP_MEM_SWAP:
							xil_printf("Address (hex): ");
							con.subcmd=1;
							con.cmd=DM;
							break;
						case S:
						case STEP:
							con.subcmd=0;
							con.step=1;
							break;
						case INW:
						case INC_NOPS_WRITE:
                            shared->nops_write+=1;
							xil_printf("NOPS WRITE %d\r\n",shared->nops_write);
							con.subcmd=0;
							break;
						case DNW:
						case DEC_NOPS_WRITE:
                            shared->nops_write-=1;
							xil_printf("NOPS WRITE %d\r\n",shared->nops_write);
							con.subcmd=0;
							break;
						case INR:
						case INC_NOPS_READ:
                            shared->nops_read+=1;
							xil_printf("NOPS READ %d\r\n",shared->nops_read);
							con.subcmd=0;
							break;
						case DNR:
						case DEC_NOPS_READ:
                            shared->nops_read-=1;
							xil_printf("NOPS READ %d\r\n",shared->nops_read);
							con.subcmd=0;
							break;
						case INE:
						case INC_NAG_ETH:
							eth_backlog_nag_counter_max+=100;
							xil_printf("NAG ETH %d\r\n",eth_backlog_nag_counter_max);
							con.subcmd=0;
							break;
						case DNE:
						case DEC_NAG_ETH:
							eth_backlog_nag_counter_max-=100;
							if(eth_backlog_nag_counter_max<0) eth_backlog_nag_counter_max=1;
							xil_printf("NAG ETH %d\r\n",eth_backlog_nag_counter_max);
							con.subcmd=0;
							break;
						case DIS:
						case DISASSEMBLE:
							shared->disassemble=!shared->disassemble;
							if(shared->disassemble)
								xil_printf("DISASSEMBLE ON\r\n");
							else
								xil_printf("DISASSEMBLE OFF\r\n");
							shared->musashi_step=1;
							con.subcmd=0;
							break;
						case DISS:
						case DISASSEMBLE_STEP:
							xil_printf("DISASSEMBLE STEP\r\n");
							shared->disassemble=1;
							shared->musashi_step=1;
							con.subcmd=0;
							break;
						case DISR:
						case DISASSEMBLE_RUN:
							xil_printf("DISASSEMBLE RUN\r\n");
							shared->disassemble=1;
							shared->musashi_step=0xFFFFFFFF;
							con.subcmd=0;
							break;
						case RSTE:
						case RESET_EMULATOR:
							xil_printf("RESET EMULATOR\r\n");
							shared->reset_emulator_dis=1;
							shared->disassemble=1;
							shared->musashi_step=1;
							con.subcmd=0;
							break;
						case SIIC:
						case STOP_IIC:
							con.stop_i2c=!con.stop_i2c;
							if(con.stop_i2c)
								xil_printf("STOP I2C ON\r\n");
							else
								xil_printf("STOP I2C OFF\r\n");
							con.subcmd=0;
							break;
						default:
							xil_printf("Not defined command '%s'. Type 'help' or 'h' for help.\r\n",con.cmd_buf);
							con.subcmd=0;
						}
						break;
					}
				}
				if(command_found==0)
				{
					xil_printf("Unknown command '%s'. Type 'help' or 'h' for help.\r\n",con.cmd_buf);
					con.subcmd=0;
				}
				con.cmd_buf[0]=0;
				con.cmd_pointer=0;
			}
		}
		else if(con.subcmd==1)
		{
			if(c>='a' && c<='z') c+='A'-'a';
			if((c>='0' && c<='9')||(c>='A' && c<='F')||
			   (c=='H' && con.cmd_pointer==0)||
			   (c=='X' && con.cmd_pointer==1)||(c==8))
			{
				if(c=='X') c='x';
				if(c=='H') c='h';
				XUartPs_SendByte(STDOUT_BASEADDRESS, c);
				if(c==8) // backspace
				{
					if(con.cmd_pointer>0)
					{
						XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
						XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
						con.cmd_pointer--;
					}
					con.cmd_buf[con.cmd_pointer]=0;
				}
				else
				{
					con.cmd_buf[con.cmd_pointer++]=c;
					if(con.cmd_pointer>90) con.cmd_pointer=90;
				}
			}
			else if((c=='\r' || c=='\n') && con.cmd_pointer>0)
			{
				con.cmd_buf[con.cmd_pointer]=0;
				XUartPs_SendByte(STDOUT_BASEADDRESS, '\r');
				XUartPs_SendByte(STDOUT_BASEADDRESS, '\n');
				con.data1=hextoi(con.cmd_buf);
				xil_printf("Length (hex): ");
				con.subcmd=2;
				con.cmd_buf[0]=0;
				con.cmd_pointer=0;
			}
		}
		else if(con.subcmd==2)
		{
			if(c>='a' && c<='z') c+='A'-'a';
			if((c>='0' && c<='9')||(c>='A' && c<='F')||
			   (c=='H' && con.cmd_pointer==0)||
			   (c=='X' && con.cmd_pointer==1)||(c==8))
			{
				if(c=='X') c='x';
				if(c=='H') c='h';
				XUartPs_SendByte(STDOUT_BASEADDRESS, c);
				if(c==8) // backspace
				{
					if(con.cmd_pointer>0)
					{
						XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
						XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
						con.cmd_pointer--;
					}
					con.cmd_buf[con.cmd_pointer]=0;
				}
				else
				{
					con.cmd_buf[con.cmd_pointer++]=c;
					if(con.cmd_pointer>90) con.cmd_pointer=90;
				}
			}
			else if((c=='\r' || c=='\n') && con.cmd_pointer>0)
			{
				con.cmd_buf[con.cmd_pointer]=0;
				XUartPs_SendByte(STDOUT_BASEADDRESS, '\r');
				XUartPs_SendByte(STDOUT_BASEADDRESS, '\n');
				con.data2=hextoi(con.cmd_buf);
				if(con.cmd==DM)
				{
					con.cmd=0;
					con.subcmd=0;
					con.cmd_buf[0]=0;
					con.cmd_pointer=0;
					dump_mem(con.data1,con.data2,0);
				}
				else if(con.cmd==DMS)
				{
					con.cmd=0;
					con.subcmd=0;
					con.cmd_buf[0]=0;
					con.cmd_pointer=0;
					dump_mem(con.data1,con.data2,1);
				}
			}
		}
	}
}
void console_help(void)
{
	xil_printf("Commands:\r\n");
	xil_printf("'H'      or 'HELP' for this help.\r\n");
	xil_printf("'DRTG'   or 'DEBUG RTG' for toggling debug RTG reads/writes.\r\n");
	xil_printf("'DSCSI'  or 'DEBUG SCSI' for toggling debug SCSI commands.\r\n");
	xil_printf("'DAUDIO' or 'DEBUG AUDIO' for toggling debug AHI/MHI commands.\r\n");
	xil_printf("'SD'     or 'STOP DEBUG' for stop all debugging.\r\n");
	xil_printf("'DMMU'   or 'DUMP MMU' for dumping Core0 MMU content.\r\n");
	xil_printf("'DM'     or 'DUMP MEM' for dumping Core0 MEM content.\r\n");
	xil_printf("'DMS'    or 'DUMP MEM SWAP' for dumping Core0 MEM content with swap32.\r\n");
	xil_printf("'S'      or 'STEP' for debugging RTG commands step by step.\r\n"
			   "                  'C' cancels step mode, and any other key makes a step\r\n");
	xil_printf("'INW'    or 'INC NOPS WRITE' increments EMU write delay\r\n");
	xil_printf("'DNW'    or 'DEC NOPS WRITE' decrements EMU write delay\r\n");
	xil_printf("'INR'    or 'INC NOPS READ' increments EMU read delay\r\n");
	xil_printf("'DNR'    or 'DEC NOPS READ' decrements EMU read delay\r\n");
	xil_printf("'INE'    or 'INC NAG ETH' increments Ethernet nag counter\r\n");
	xil_printf("'DNE'    or 'DEC NAG ETH' decrements Ethernet nag counter\r\n");
	xil_printf("'DIS'    or 'DISASSEMBLE' enable disassemble (Musashi only)\r\n");
	xil_printf("'DISS'   or 'DISASSEMBLE STEP' step disassemble (Musashi only)\r\n");
	xil_printf("'DISR'   or 'DISASSEMBLE RUN' run disassemble (Musashi only)\r\n");
	xil_printf("'RSTE'   or 'RESET EMULATOR' reset emulator (Musashi only)\r\n");
	xil_printf("'DIIC'   or 'DEBUG IIC' for toggling debug i2c (LTC2990)\r\n");
	xil_printf("'SIIC'   or 'STOP IIC' for toggling stop i2c (LTC2990)\r\n");
}
#endif
