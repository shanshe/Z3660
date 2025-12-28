
#define DEBUG_CONSOLE_ENABLED 1
#ifdef DEBUG_CONSOLE_ENABLED
#include "debug_console.h"
#include "main.h"
#include <xil_printf.h>
#include <xparameters.h>
#include "xuartps_hw.h"
#include "config_clk.h"
#include "config_file.h"
#include <stdlib.h>
#include <sleep.h>

void debug_console_help(void);
void dump_mmu(void);
uint32_t read_rtg_register(uint32_t zaddr);
void write_rtg_register(uint32_t zaddr,uint32_t zdata);
int write_timings(void);

DEBUG_CONSOLE debug_console;
extern SHARED *shared;
extern int eth_backlog_nag_counter_max;
typedef enum {
   A,

   DRTG,    DEBUG_RTG,
   DUSB,    DEBUG_USB,
   DSCSI,   DEBUG_SCSI,
   DAUDIO,  DEBUG_AUDIO,
   DI2C,    DEBUG_I2C,
   DENET,   DEBUG_ENET,
   DSOFT3D, DEBUG_SOFT3D,
   H,       HELP,
   SD,      STOP_DEBUG,
   DMMU,    DUMP_MMU,
   DM,      DUMP_MEM,
   DMS,     DUMP_MEM_SWAP,
   S,       STEP,
   INW,     INC_NOPS_WRITE,
   DNW,     DEC_NOPS_WRITE,
   INR,     INC_NOPS_READ,
   DNR,     DEC_NOPS_READ,
   INE,     INC_NAG_ETH,
   DNE,     DEC_NAG_ETH,
   DIS,     DISASSEMBLE,
   DISS,    DISASSEMBLE_STEP,
   DISR,    DISASSEMBLE_RUN,
   RSTE,    RESET_EMULATOR,
   SI2C,    STOP_I2C,
   TRB,     TOGGLE_READ_BURST,
   TWB,     TOGGLE_WRITE_BURST,
   CCP,     CHANGE_CLOCK_PHASE,
   CCF,     CHANGE_CLOCK_FREQ,
   RCPLD,   RESET_CPLD,
   RFPGA,   RESET_FPGA,
   RAMIGA,  RESET_AMIGA,
   RARM,    RESET_ARM,
   JIT,     TOGGLE_JIT,
   SCT,     SAVE_CURRENT_TIMINGS,
   PHD,     PRINT_HIST_DATAABORT,
   VMR,     VIDEO_MODE_RESET,

   NUM_COMMANDS
} COMMANDS;
const char *command_names[NUM_COMMANDS] = {
      "A",

      "DRTG",    "DEBUG RTG",
      "DUSB",    "DEBUG USB",
      "DSCSI",   "DEBUG SCSI",
      "DAUDIO",  "DEBUG AUDIO",
      "DI2C",    "DEBUG I2C",
      "DENET",   "DEBUG ENET",
      "DSOFT3D", "DEBUG SOFT3D",
      "H",       "HELP",
      "SD",      "STOP DEBUG",
      "DMMU",    "DUMP MMU",
      "DM",      "DUMP MEM",
      "DMS",     "DUMP MEM SWAP",
      "S",       "STEP",
      "INW",     "INC NOPS WRITE",
      "DNW",     "DEC NOPS WRITE",
      "INR",     "INC NOPS READ",
      "DNR",     "DEC NOPS READ",
      "INE",     "INC NAG ETH",
      "DNE",     "DEC NAG ETH",
      "DIS",     "DISASEMBLE",
      "DISS",    "DISASEMBLE STEP",
      "DISR",    "DISASEMBLE RUN",
      "RSTE",    "RESET EMULATOR",
      "SI2C",    "STOP I2C",
      "TRB",     "TOGGLE READ BURST",
      "TWB",     "TOGGLE WRITE BURST",
      "CCP",     "CHANGE CLOCK PHASE",
      "CCF",     "CHANGE CLOCK FREQ",
      "RCPLD",   "RESET CPLD",
      "RFPGA",   "RESET FPGA",
      "RAMIGA",  "RESET AMIGA",
      "RARM",    "RESET ARM",
      "JIT",     "TOOGGLE_JIT",
      "SCT",     "SAVE_CURRENT_TIMINGS",
      "PHD",     "PRINT_HIST_DATAABORT",
      "VMR",     "VIDEO_MODE_RESET",
};
extern clock_data cd[];
extern CONFIG config;

void debug_console_init(void)
{
   debug_console.cmd_pointer=0;
   debug_console.debug_rtg=0;
   debug_console.debug_usb=0;
   debug_console.subcmd=0;
   debug_console.debug_scsi=0;
   debug_console.debug_audio=0;
   debug_console.debug_ethernet=0;
   debug_console.debug_soft3d=0;
   debug_console.debug_i2c=0;
   debug_console.stop_i2c=0;
   debug_console.step=0;
   debug_console.hist_pointer=0;
   debug_console.hist_pointer_top=0;
   for(int i=0;i<MAX_HIST;i++)
      debug_console.cmd_hist[i][0]=0;
   debug_console.toggle_read_burst=1;
   debug_console.toggle_write_burst=1;
   int ind=get_clock_index(config.cpufreq);
   debug_console.clocks.clk_index=ind;
   int clkbase=200*cd[ind].M/cd[ind].D;
#define CLKCONFIG(A,a)                                \
      debug_console.clocks.A.clk      =clkbase/cd[ind].a.divider; \
      debug_console.clocks.A.phase    =        cd[ind].a.phase;   \
      debug_console.clocks.A.dutycycle=        cd[ind].a.dutycycle

   CLKCONFIG(AXICLK,axi);
   CLKCONFIG(  PCLK,pclk);
   CLKCONFIG( CLKEN,clken);
   CLKCONFIG(  BCLK,bclk);
   CLKCONFIG(CPUCLK,cpuclk);
   CLKCONFIG( CLK90,clk90);
   debug_console.clocks.emu_etra_phase=cd[ind].emu_extra_phase;

   debug_console.reset_cpld=0;
   debug_console.reset_fpga=0;
}
uint32_t hextoi(char *str)
{
   int index=0;
   uint32_t data=0;
   if(str[0]=='0' && (str[1]=='x' || str[1]=='X'))
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
#include "pt/pt.h"
int debug_thread(struct pt *pt)
{
   PT_BEGIN(pt);

   while(1)
   {
      PT_WAIT_UNTIL(pt,XUartPs_IsReceiveData(STDIN_BASEADDRESS));
      char c = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
      if(debug_console.subcmd==0)
      {
         if(c>='a' && c<='z') c+='A'-'a';
         if((c>='A' && c<='Z')||
               (c==' ')||(c==8)||(c==ESC)||
               (c>='0' && c<='9'))
         {
            if(c==ESC)
            {
               while(XUartPs_IsReceiveData(STDIN_BASEADDRESS)==0);
               c=XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
               if(c=='[')
               {
                  while(XUartPs_IsReceiveData(STDIN_BASEADDRESS)==0);
                  c=XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
                  if(c=='A')
                  {
                     // UP
                     while(debug_console.cmd_pointer>0)
                     {
                        XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
                        XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
                        XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
                        debug_console.cmd_pointer--;
                     }
                     debug_console.cmd_buf[debug_console.cmd_pointer]=0;
                     int i=0;
                     if(debug_console.hist_pointer>0)
                        debug_console.hist_pointer--;
                     else
                        debug_console.hist_pointer=MAX_HIST-1;
                     strcpy(debug_console.cmd_buf,debug_console.cmd_hist[debug_console.hist_pointer]);
                     while(debug_console.cmd_hist[debug_console.hist_pointer][i]!=0)
                     {
                        XUartPs_SendByte(STDOUT_BASEADDRESS, debug_console.cmd_hist[debug_console.hist_pointer][i++]);
                        debug_console.cmd_pointer++;
                     }
                  }
                  else if(c=='B')
                  {
                     // DOWN
                     while(debug_console.cmd_pointer>0)
                     {
                        XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
                        XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
                        XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
                        debug_console.cmd_pointer--;
                     }
                     debug_console.cmd_buf[debug_console.cmd_pointer]=0;
                     int i=0;
                     debug_console.hist_pointer++;
                     if(debug_console.hist_pointer>=MAX_HIST) debug_console.hist_pointer=0;
                     if(debug_console.hist_pointer!=debug_console.hist_pointer_top)
                     {
                        strcpy(debug_console.cmd_buf,debug_console.cmd_hist[debug_console.hist_pointer]);
                        while(debug_console.cmd_hist[debug_console.hist_pointer][i]!=0)
                        {
                           XUartPs_SendByte(STDOUT_BASEADDRESS, debug_console.cmd_hist[debug_console.hist_pointer][i++]);
                           debug_console.cmd_pointer++;
                        }
                     }
                  }
                  else
                  {
                     debug_console.cmd_buf[debug_console.cmd_pointer]=0;
                  }
               }
               else
               {
                  debug_console.cmd_buf[debug_console.cmd_pointer]=0;
               }
            }
            else
            {
               XUartPs_SendByte(STDOUT_BASEADDRESS, c);
               if(c==8) // backspace
               {
                  if(debug_console.cmd_pointer>0)
                  {
                     XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
                     XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
                     debug_console.cmd_pointer--;
                  }
                  debug_console.cmd_buf[debug_console.cmd_pointer]=0;
               }
               else
               {
                  debug_console.cmd_buf[debug_console.cmd_pointer++]=c;
                  if(debug_console.cmd_pointer>90) debug_console.cmd_pointer=90;
               }
            }
         }
         else if((c=='\r' || c=='\n') && debug_console.cmd_pointer>0)
         {
            debug_console.cmd_buf[debug_console.cmd_pointer]=0;
            strcpy(debug_console.cmd_hist[debug_console.hist_pointer_top++],debug_console.cmd_buf);
            if(debug_console.hist_pointer_top>=MAX_HIST) debug_console.hist_pointer_top=0;
            debug_console.hist_pointer=debug_console.hist_pointer_top;
            XUartPs_SendByte(STDOUT_BASEADDRESS, '\r');
            XUartPs_SendByte(STDOUT_BASEADDRESS, '\n');
            uint8_t command_found=0;
            for(int i=0;i<NUM_COMMANDS;i++)
            {
               if(strcmp(debug_console.cmd_buf,command_names[i])==0)
               {
                  command_found=1;
                  switch(i)
                  {
                  case A:
                     xil_printf("test 'A' command\r\n");
                     debug_console.subcmd=0;
                     break;
                  case DRTG:
                  case DEBUG_RTG:
                     debug_console.debug_rtg=!debug_console.debug_rtg;
                     if(debug_console.debug_rtg)
                        xil_printf("DEBUG RTG ON\r\n");
                     else
                        xil_printf("DEBUG RTG OFF\r\n");
                     debug_console.subcmd=0;
                     break;
                  case DUSB:
                  case DEBUG_USB:
                     debug_console.debug_usb=!debug_console.debug_usb;
                     if(debug_console.debug_usb)
                        xil_printf("DEBUG USB ON\r\n");
                     else
                        xil_printf("DEBUG USB OFF\r\n");
                     debug_console.subcmd=0;
                     break;
                  case DSCSI:
                  case DEBUG_SCSI:
                     debug_console.debug_scsi=!debug_console.debug_scsi;
                     if(debug_console.debug_scsi)
                        xil_printf("DEBUG SCSI ON\r\n");
                     else
                        xil_printf("DEBUG SCSI OFF\r\n");
                     debug_console.subcmd=0;
                     break;
                  case DAUDIO:
                  case DEBUG_AUDIO:
                     debug_console.debug_audio=!debug_console.debug_audio;
                     if(debug_console.debug_audio)
                        xil_printf("DEBUG AUDIO ON\r\n");
                     else
                        xil_printf("DEBUG AUDIO OFF\r\n");
                     debug_console.subcmd=0;
                     break;
                  case DENET:
                  case DEBUG_ENET:
                     debug_console.debug_ethernet=!debug_console.debug_ethernet;
                     if(debug_console.debug_ethernet)
                        xil_printf("DEBUG ETHERNET ON\r\n");
                     else
                        xil_printf("DEBUG ETHERNET OFF\r\n");
                     debug_console.subcmd=0;
                     break;
                  case DSOFT3D:
                  case DEBUG_SOFT3D:
                     debug_console.debug_soft3d=!debug_console.debug_soft3d;
                     if(debug_console.debug_soft3d)
                        xil_printf("DEBUG SOFT3D ON\r\n");
                     else
                        xil_printf("DEBUG SOFT3D OFF\r\n");
                     debug_console.subcmd=0;
                     break;
                  case DI2C:
                  case DEBUG_I2C:
                     debug_console.debug_i2c=!debug_console.debug_i2c;
                     if(debug_console.debug_i2c)
                        xil_printf("DEBUG I2C ON\r\n");
                     else
                        xil_printf("DEBUG I2C OFF\r\n");
                     debug_console.subcmd=0;
                     break;
                  case H:
                  case HELP:
                     debug_console_help();
                     debug_console.subcmd=0;
                     break;
                  case SD:
                  case STOP_DEBUG:
                     debug_console.debug_rtg=0;
                     debug_console.debug_usb=0;
                     debug_console.debug_scsi=0;
                     debug_console.debug_audio=0;
                     debug_console.debug_ethernet=0;
                     debug_console.debug_soft3d=0;
                     debug_console.debug_i2c=0;
                     xil_printf("ALL DEBUG STOPPED\r\n");
                     debug_console.subcmd=0;
                     break;
                  case DMMU:
                  case DUMP_MMU:
                     dump_mmu();
                     debug_console.subcmd=0;
                     break;
                  case DM:
                  case DUMP_MEM:
                     xil_printf("Address (hex): ");
                     debug_console.subcmd=1;
                     debug_console.cmd=DM;
                     break;
                  case DMS:
                  case DUMP_MEM_SWAP:
                     xil_printf("Address (hex): ");
                     debug_console.subcmd=1;
                     debug_console.cmd=DM;
                     break;
                  case S:
                  case STEP:
                     debug_console.subcmd=0;
                     debug_console.step=1;
                     break;
                  case INW:
                  case INC_NOPS_WRITE:
                     shared->nops_write+=1;
                     xil_printf("NOPS WRITE %d\r\n",shared->nops_write);
                     debug_console.subcmd=0;
                     break;
                  case DNW:
                  case DEC_NOPS_WRITE:
                     shared->nops_write-=1;
                     xil_printf("NOPS WRITE %d\r\n",shared->nops_write);
                     debug_console.subcmd=0;
                     break;
                  case INR:
                  case INC_NOPS_READ:
                     shared->nops_read+=1;
                     xil_printf("NOPS READ %d\r\n",shared->nops_read);
                     debug_console.subcmd=0;
                     break;
                  case DNR:
                  case DEC_NOPS_READ:
                     shared->nops_read-=1;
                     xil_printf("NOPS READ %d\r\n",shared->nops_read);
                     debug_console.subcmd=0;
                     break;
                  case INE:
                  case INC_NAG_ETH:
                     eth_backlog_nag_counter_max+=100;
                     xil_printf("NAG ETH %d\r\n",eth_backlog_nag_counter_max);
                     debug_console.subcmd=0;
                     break;
                  case DNE:
                  case DEC_NAG_ETH:
                     eth_backlog_nag_counter_max-=100;
                     if(eth_backlog_nag_counter_max<0) eth_backlog_nag_counter_max=1;
                     xil_printf("NAG ETH %d\r\n",eth_backlog_nag_counter_max);
                     debug_console.subcmd=0;
                     break;
                  case DIS:
                  case DISASSEMBLE:
                     shared->disassemble=!shared->disassemble;
                     if(shared->disassemble)
                        xil_printf("DISASSEMBLE ON\r\n");
                     else
                        xil_printf("DISASSEMBLE OFF\r\n");
                     shared->musashi_step=1;
                     debug_console.subcmd=0;
                     break;
                  case DISS:
                  case DISASSEMBLE_STEP:
                     xil_printf("DISASSEMBLE STEP\r\n");
                     shared->disassemble=1;
                     shared->musashi_step=1;
                     debug_console.subcmd=0;
                     break;
                  case DISR:
                  case DISASSEMBLE_RUN:
                     xil_printf("DISASSEMBLE RUN\r\n");
                     shared->disassemble=1;
                     shared->musashi_step=0xFFFFFFFF;
                     debug_console.subcmd=0;
                     break;
                  case RSTE:
                  case RESET_EMULATOR:
                     xil_printf("RESET EMULATOR\r\n");
                     shared->reset_emulator_dis=1;
                     shared->disassemble=1;
                     shared->musashi_step=1;
                     debug_console.subcmd=0;
                     break;
                  case SI2C:
                  case STOP_I2C:
                     debug_console.stop_i2c=!debug_console.stop_i2c;
                     if(debug_console.stop_i2c)
                        xil_printf("STOP I2C ON\r\n");
                     else
                        xil_printf("STOP I2C OFF\r\n");
                     debug_console.subcmd=0;
                     break;
                  case TRB:
                  case TOGGLE_READ_BURST:
                     debug_console.toggle_read_burst=!debug_console.toggle_read_burst;
                     if(debug_console.toggle_read_burst)
                     {
                        ENABLE_BURST_READ_FPGA;
                        xil_printf("READ BURST ON\r\n");
                     }
                     else
                     {
                        DISABLE_BURST_READ_FPGA;
                        xil_printf("READ BURST OFF\r\n");
                     }
                     debug_console.subcmd=0;
                     break;
                  case TWB:
                  case TOGGLE_WRITE_BURST:
                     debug_console.toggle_write_burst=!debug_console.toggle_write_burst;
                     if(debug_console.toggle_write_burst)
                     {
                        ENABLE_BURST_WRITE_FPGA;
                        xil_printf("WRITE BURST ON\r\n");
                     }
                     else
                     {
                        DISABLE_BURST_WRITE_FPGA;
                        xil_printf("WRITE BURST OFF\r\n");
                     }
                     debug_console.subcmd=0;
                     break;
                  case CCP:
                  case CHANGE_CLOCK_PHASE:
                     xil_printf("PCLK PHASE [%d]: ",debug_console.clocks.PCLK.phase);
                     debug_console.subcmd=3;
                     debug_console.cmd=0;
                     break;
                  case CCF:
                  case CHANGE_CLOCK_FREQ:
                     xil_printf("PCLK FREQ [%d]: ",config.cpufreq);
                     debug_console.subcmd=9;
                     debug_console.cmd=0;
                     break;
                  case RCPLD:
                  case RESET_CPLD:
                     debug_console.reset_cpld=!debug_console.reset_cpld;
                     if(debug_console.reset_cpld)
                     {
                        xil_printf("RESET CPLD ON\r\n");
                        CPLD_RESET_ARM(0);
                     }
                     else
                     {
                        xil_printf("RESET CPLD OFF\r\n");
                        CPLD_RESET_ARM(1);
                     }
                     debug_console.subcmd=0;
                     break;
                  case RFPGA:
                  case RESET_FPGA:
                     debug_console.reset_fpga=!debug_console.reset_fpga;
                     if(debug_console.reset_fpga)
                     {
                        xil_printf("RESET FPGA ON\r\n");
                        DiscreteSet(REG0, FPGA_RESET);
                     }
                     else
                     {
                        xil_printf("RESET FPGA OFF\r\n");
                        DiscreteClear(REG0, FPGA_RESET);
                     }
                     debug_console.subcmd=0;
                     break;
                  case RAMIGA:
                  case RESET_AMIGA:
                     xil_printf("RESET CPLD ON\r\n");
                     CPLD_RESET_ARM(0);
                     usleep(50000);
                     xil_printf("RESET FPGA ON\r\n");
                     DiscreteSet(REG0, FPGA_RESET);
                     usleep(50000);
                     xil_printf("RESET FPGA OFF\r\n");
                     DiscreteClear(REG0, FPGA_RESET);
                     usleep(50000);
                     // it will be off later
//                     xil_printf("RESET CPLD OFF\r\n");
//                     CPLD_RESET_ARM(1);
//                     usleep(500000);
                     debug_console.subcmd=0;
                     break;
                  case RARM:
                  case RESET_ARM:
                     hard_reboot();
                     debug_console.subcmd=0;
                     break;
                  case JIT:
                  case TOGGLE_JIT:
                     write_rtg_register(REG_ZZ_JIT_ENABLE,!read_rtg_register(REG_ZZ_JIT_ENABLE));
                     printf("JIT %s\n",read_rtg_register(REG_ZZ_JIT_ENABLE)?"ON":"OFF");
                     break;
                  case SCT:
                  case SAVE_CURRENT_TIMINGS:
                     write_timings();
                     break;
                  case PHD:
                  case PRINT_HIST_DATAABORT:
                     shared->printhist_dataabort=1;
                     break;
                  case VMR:
                  case VIDEO_MODE_RESET:
                     printf("Video mode reset...\n");
                     video_mode_reset();
                     break;
                  default:
                     xil_printf("Not defined command '%s'. Type 'help' or 'h' for help.\r\n",debug_console.cmd_buf);
                     debug_console.subcmd=0;
                  }
                  break;
               }
            }
            if(command_found==0)
            {
               xil_printf("Unknown command '%s'. Type 'help' or 'h' for help.\r\n",debug_console.cmd_buf);
               debug_console.subcmd=0;
            }
            debug_console.cmd_buf[0]=0;
            debug_console.cmd_pointer=0;
         }
      }
      else if(debug_console.subcmd==1)
      {
         if(c>='a' && c<='z') c+='A'-'a';
         if((c>='0' && c<='9')||(c>='A' && c<='F')||
               (c=='H' && debug_console.cmd_pointer==0)||
               (c=='X' && debug_console.cmd_pointer==1)||(c==8))
         {
            if(c=='X') c='x';
            if(c=='H') c='h';
            XUartPs_SendByte(STDOUT_BASEADDRESS, c);
            if(c==8) // backspace
            {
               if(debug_console.cmd_pointer>0)
               {
                  XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
                  XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
                  debug_console.cmd_pointer--;
               }
               debug_console.cmd_buf[debug_console.cmd_pointer]=0;
            }
            else
            {
               debug_console.cmd_buf[debug_console.cmd_pointer++]=c;
               if(debug_console.cmd_pointer>90) debug_console.cmd_pointer=90;
            }
         }
         else if((c=='\r' || c=='\n') && debug_console.cmd_pointer>0)
         {
            debug_console.cmd_buf[debug_console.cmd_pointer]=0;
            XUartPs_SendByte(STDOUT_BASEADDRESS, '\r');
            XUartPs_SendByte(STDOUT_BASEADDRESS, '\n');
            debug_console.data1=hextoi(debug_console.cmd_buf);
            xil_printf("Length (hex): ");
            debug_console.subcmd=2;
            debug_console.cmd_buf[0]=0;
            debug_console.cmd_pointer=0;
         }
      }
      else if(debug_console.subcmd==2)
      {
         if(c>='a' && c<='z') c+='A'-'a';
         if((c>='0' && c<='9')||(c>='A' && c<='F')||
               (c=='H' && debug_console.cmd_pointer==0)||
               (c=='X' && debug_console.cmd_pointer==1)||(c==8))
         {
            if(c=='X') c='x';
            if(c=='H') c='h';
            XUartPs_SendByte(STDOUT_BASEADDRESS, c);
            if(c==8) // backspace
            {
               if(debug_console.cmd_pointer>0)
               {
                  XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
                  XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
                  debug_console.cmd_pointer--;
               }
               debug_console.cmd_buf[debug_console.cmd_pointer]=0;
            }
            else
            {
               debug_console.cmd_buf[debug_console.cmd_pointer++]=c;
               if(debug_console.cmd_pointer>90) debug_console.cmd_pointer=90;
            }
         }
         else if((c=='\r' || c=='\n') && debug_console.cmd_pointer>0)
         {
            debug_console.cmd_buf[debug_console.cmd_pointer]=0;
            XUartPs_SendByte(STDOUT_BASEADDRESS, '\r');
            XUartPs_SendByte(STDOUT_BASEADDRESS, '\n');
            debug_console.data2=hextoi(debug_console.cmd_buf);
            if(debug_console.cmd==DM)
            {
               debug_console.cmd=0;
               debug_console.subcmd=0;
               debug_console.cmd_buf[0]=0;
               debug_console.cmd_pointer=0;
               dump_mem(debug_console.data1,debug_console.data2,0);
            }
            else if(debug_console.cmd==DMS)
            {
               debug_console.cmd=0;
               debug_console.subcmd=0;
               debug_console.cmd_buf[0]=0;
               debug_console.cmd_pointer=0;
               dump_mem(debug_console.data1,debug_console.data2,1);
            }
         }
      }
      else if(debug_console.subcmd>=3 && debug_console.subcmd<=8)
      {
         if((c>='0' && c<='9')||
               (c=='-' && debug_console.cmd_pointer==0)||(c==8))
         {
            XUartPs_SendByte(STDOUT_BASEADDRESS, c);
            if(c==8) // backspace
            {
               if(debug_console.cmd_pointer>0)
               {
                  XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
                  XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
                  debug_console.cmd_pointer--;
               }
               debug_console.cmd_buf[debug_console.cmd_pointer]=0;
            }
            else
            {
               debug_console.cmd_buf[debug_console.cmd_pointer++]=c;
               if(debug_console.cmd_pointer>90) debug_console.cmd_pointer=90;
            }
         }
         else if((c=='\r' || c=='\n') && debug_console.cmd_pointer==0)
         {
            debug_console.cmd_buf[debug_console.cmd_pointer]=0;
            switch(debug_console.subcmd-3)
            {
            case 0:
               xil_printf("\r\nCLKEN PHASE [%d]: ",debug_console.clocks.CLKEN.phase);
               debug_console.subcmd++;
               break;
            case 1:
               xil_printf("\r\nBCLK PHASE [%d]: ",debug_console.clocks.BCLK.phase);
               debug_console.subcmd++;
               break;
            case 2:
               xil_printf("\r\nCPUCLK PHASE [%d]: ",debug_console.clocks.CPUCLK.phase);
               debug_console.subcmd++;
               break;
            case 3:
               xil_printf("\r\nCLK90 PHASE [%d]: ",debug_console.clocks.CLK90.phase);
               debug_console.subcmd++;
               break;
            case 4:
               xil_printf("\r\nEMU EXTRA PHASE [%d]: ",debug_console.clocks.emu_etra_phase);
               debug_console.subcmd++;
               break;
            case 5: {
               debug_console.cmd=0;
               debug_console.subcmd=0;
               int emu;
               if(   config.boot_mode==MUSASHI
                     || config.boot_mode==UAE_030
                     || config.boot_mode==UAEJIT_030
                     || config.boot_mode==UAE_040
                     || config.boot_mode==UAEJIT_040)
                  emu=1;
               else
                  emu=0;
               configure_clk(config.cpufreq,1,emu);
               }
               break;
            }
            debug_console.cmd_buf[0]=0;
            debug_console.cmd_pointer=0;
         }
         else if((c=='\r' || c=='\n') && debug_console.cmd_pointer>0)
         {
            debug_console.cmd_buf[debug_console.cmd_pointer]=0;
            switch(debug_console.subcmd-3)
            {
            case 0:
               debug_console.sdata1=atoi(debug_console.cmd_buf);
               debug_console.clocks.PCLK.phase=debug_console.sdata1;
               debug_console.clocks.clk_index=get_clock_index(config.cpufreq);
//               xil_printf("\r\nCLK INDEX %d",debug_console.clocks.clk_index);
               cd[debug_console.clocks.clk_index].pclk.phase=debug_console.sdata1;
               xil_printf("\r\nCLKEN PHASE [%d]: ",debug_console.clocks.CLKEN.phase);
               debug_console.subcmd++;
               break;
            case 1:
               debug_console.sdata1=atoi(debug_console.cmd_buf);
               debug_console.clocks.CLKEN.phase=debug_console.sdata1;
               cd[debug_console.clocks.clk_index].clken.phase=debug_console.sdata1;
               xil_printf("\r\nBCLK PHASE [%d]: ",debug_console.clocks.BCLK.phase);
               debug_console.subcmd++;
               break;
            case 2:
               debug_console.sdata1=atoi(debug_console.cmd_buf);
               debug_console.clocks.BCLK.phase=debug_console.sdata1;
               cd[debug_console.clocks.clk_index].bclk.phase=debug_console.sdata1;
               xil_printf("\r\nCPUCLK PHASE [%d]: ",debug_console.clocks.CPUCLK.phase);
               debug_console.subcmd++;
               break;
            case 3:
               debug_console.sdata1=atoi(debug_console.cmd_buf);
               debug_console.clocks.CPUCLK.phase=debug_console.sdata1;
               cd[debug_console.clocks.clk_index].cpuclk.phase=debug_console.sdata1;
               xil_printf("\r\nCLK90 PHASE [%d]: ",debug_console.clocks.CLK90.phase);
               debug_console.subcmd++;
               break;
            case 4:
               debug_console.sdata1=atoi(debug_console.cmd_buf);
               debug_console.clocks.CLK90.phase=debug_console.sdata1;
               cd[debug_console.clocks.clk_index].clk90.phase=debug_console.sdata1;
               xil_printf("\r\nEMU EXTRA PHASE [%d]: ",debug_console.clocks.emu_etra_phase);
               debug_console.subcmd++;
               break;
            case 5: {
               debug_console.sdata1=atoi(debug_console.cmd_buf);
               debug_console.clocks.emu_etra_phase=debug_console.sdata1;
               cd[debug_console.clocks.clk_index].emu_extra_phase=debug_console.sdata1;
               debug_console.cmd=0;
               debug_console.subcmd=0;
               int emu;
               if(   config.boot_mode==MUSASHI
                     || config.boot_mode==UAE_030
                     || config.boot_mode==UAEJIT_030
                     || config.boot_mode==UAE_040
                     || config.boot_mode==UAEJIT_040)
                  emu=1;
               else
                  emu=0;
               configure_clk(config.cpufreq,1,emu);
               }
               break;
            }
            debug_console.cmd_buf[0]=0;
            debug_console.cmd_pointer=0;
         }
      }
      else if(debug_console.subcmd==9)
      {
         if((c>='0' && c<='9')||
            (c==8))
         {
            XUartPs_SendByte(STDOUT_BASEADDRESS, c);
            if(c==8) // backspace
            {
               if(debug_console.cmd_pointer>0)
               {
                  XUartPs_SendByte(STDOUT_BASEADDRESS, ' ');
                  XUartPs_SendByte(STDOUT_BASEADDRESS, 8);
                  debug_console.cmd_pointer--;
               }
               debug_console.cmd_buf[debug_console.cmd_pointer]=0;
            }
            else
            {
               debug_console.cmd_buf[debug_console.cmd_pointer++]=c;
               if(debug_console.cmd_pointer>90) debug_console.cmd_pointer=90;
            }
         }
         else if((c=='\r' || c=='\n') && debug_console.cmd_pointer==0)
         {
            //Do nothing
            debug_console.cmd=0;
            debug_console.subcmd=0;
            int emu;
            if(   config.boot_mode==MUSASHI
                  || config.boot_mode==UAE_030
                  || config.boot_mode==UAEJIT_030
                  || config.boot_mode==UAE_040
                  || config.boot_mode==UAEJIT_040)
               emu=1;
            else
               emu=0;
            configure_clk(config.cpufreq,1,emu);
            debug_console.cmd_buf[0]=0;
            debug_console.cmd_pointer=0;
         }
         else if((c=='\r' || c=='\n') && debug_console.cmd_pointer>0)
         {
            debug_console.cmd_buf[debug_console.cmd_pointer]=0;
            debug_console.sdata1=atoi(debug_console.cmd_buf);
            config.cpufreq=debug_console.sdata1;
            debug_console.cmd=0;
            debug_console.subcmd=0;
            int emu;
            if(   config.boot_mode==MUSASHI
                  || config.boot_mode==UAE_030
                  || config.boot_mode==UAEJIT_030
                  || config.boot_mode==UAE_040
                  || config.boot_mode==UAEJIT_040)
               emu=1;
            else
               emu=0;
            configure_clk(config.cpufreq,1,emu);
            debug_console.cmd_buf[0]=0;
            debug_console.cmd_pointer=0;
         }
      }

   }
   PT_END(pt);
}
void debug_console_help(void)
{
   xil_printf("Commands:\r\n");
   xil_printf("'H'       or 'HELP' for this help.\r\n");
   xil_printf("'DRTG'    or 'DEBUG RTG' for toggling debug RTG reads/writes.\r\n");
   xil_printf("'DUSB'    or 'DEBUG USB' for toggling debug USB reads/writes.\r\n");
   xil_printf("'DSCSI'   or 'DEBUG SCSI' for toggling debug SCSI commands.\r\n");
   xil_printf("'DAUDIO'  or 'DEBUG AUDIO' for toggling debug AHI/MHI commands.\r\n");
   xil_printf("'DI2C'    or 'DEBUG I2C' for toggling debug i2c (LTC2990)\r\n");
   xil_printf("'DENET'   or 'DEBUG ENET' for toggling debug ETHERNET commands.\r\n");
   xil_printf("'DSOFT3D' or 'DEBUG SOFT3D' for toggling debug SOFT3D commands.\r\n");
   xil_printf("'SD'      or 'STOP DEBUG' for stop all debugging.\r\n");
   xil_printf("'DMMU'    or 'DUMP MMU' for dumping Core0 MMU content.\r\n");
   xil_printf("'DM'      or 'DUMP MEM' for dumping Core0 MEM content.\r\n");
   xil_printf("'DMS'     or 'DUMP MEM SWAP' for dumping Core0 MEM content with swap32.\r\n");
   xil_printf("'S'       or 'STEP' for debugging RTG commands step by step.\r\n"
              "                   'C' cancels step mode, and any other key makes a step\r\n");
   xil_printf("'INW'     or 'INC NOPS WRITE' increments EMU write delay\r\n");
   xil_printf("'DNW'     or 'DEC NOPS WRITE' decrements EMU write delay\r\n");
   xil_printf("'INR'     or 'INC NOPS READ' increments EMU read delay\r\n");
   xil_printf("'DNR'     or 'DEC NOPS READ' decrements EMU read delay\r\n");
   xil_printf("'INE'     or 'INC NAG ETH' increments Ethernet nag counter\r\n");
   xil_printf("'DNE'     or 'DEC NAG ETH' decrements Ethernet nag counter\r\n");
   xil_printf("'DIS'     or 'DISASSEMBLE' enable disassemble (Musashi only)\r\n");
   xil_printf("'DISS'    or 'DISASSEMBLE STEP' step disassemble (Musashi only)\r\n");
   xil_printf("'DISR'    or 'DISASSEMBLE RUN' run disassemble (Musashi only)\r\n");
   xil_printf("'RSTE'    or 'RESET EMULATOR' reset emulator (Musashi only)\r\n");
   xil_printf("'SI2C'    or 'STOP I2C' for toggling stop i2c (LTC2990)\r\n");
   xil_printf("'TRB'     or 'TOGGLE READ BURST' for toggling Read Burst\r\n");
   xil_printf("'TWB'     or 'TOGGLE WRITE BURST' for toggling Write Burst\r\n");
   xil_printf("'CCP'     or 'CHANGE CLOCK PHASE' for changing clock phases\r\n");
   xil_printf("'CCF'     or 'CHANGE CLOCK FREQ' for changing clock fequency\r\n");
   xil_printf("'RCPLD'   or 'RESET CPLD' for toggling reset CPLD\r\n");
   xil_printf("'RFPGA'   or 'RESET FPGA' for toggling reset FPGA\r\n");
   xil_printf("'RAMIGA'  or 'RESET AMIGA' for resetting the AMIGA (sequence of the two above)\r\n");
   xil_printf("'RARM'    or 'RESET ARM' for resetting the ARM (reboot the entire system)\r\n");
   xil_printf("'JIT'     or 'TOGGLE_JIT' for toggling ON/OFF JIT CPU emulation\r\n");
   xil_printf("'SCT'     or 'SAVE_CURRENT_TIMINGS' for saving current timings on the timings file\r\n");
   xil_printf("'PHD'     or 'PRINT_HIST_DATAABORT' for printing the data abort histogram (EMU only)\r\n");
   xil_printf("'VMR'     or 'VIDEO_MODE_RESET' for reseting vdma\r\n");
}
#endif
