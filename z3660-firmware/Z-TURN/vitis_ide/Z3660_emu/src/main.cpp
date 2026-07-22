/*
 * main.cc
 */

#include <stdio.h>
#include <xil_cache.h>
#include <xil_cache_l.h>
#include <xil_mmu.h>
#include <inttypes.h>
#include "xgpiops.h"
#include "xparameters.h"
#include "main.h"
#include "xil_exception.h"
#include "xdmaps.h"
#include "xscugic.h"
#include "xscutimer.h"
#include "memorymap.h"
#include "ps7_init_simple.h"

#define DEFAULT_NOPS_WRITE 0
#define DEFAULT_NOPS_READ 0

#define MUSASHI_EMULATOR
#define UAE_EMULATOR
#ifdef MUSASHI_EMULATOR
#include "cpu_emulator.h"
#endif
#ifdef UAE_EMULATOR
#include "uae_emulator.h"
#endif

#include "newcpu.h"
#include "custom.h"
#include "defines.h"

SHARED *shared;
extern LOCAL local;
extern "C" void z3660_printf(const TCHAR *format, ...)
{
   va_list args{};
   va_start(args, format);
   while(shared->uart_semaphore!=0);
   shared->uart_semaphore=1;
   vprintf(format, args);
   shared->uart_semaphore=0;
   va_end(args);
}
extern uint32_t MMUTable;
#pragma align 1024
uint32_t MMUL2Table[256] __attribute__ ((aligned (1024)));
void init_shared(void)
{
   shared=(SHARED *)0xFFFF0000;
   shared->mmu_core1_add=(uint32_t)(&MMUTable);
   shared->nops_write=DEFAULT_NOPS_WRITE;
   shared->nops_read=DEFAULT_NOPS_READ;
}
XGpioPs GpioPs;
XGpioPs_Config *GpioPsConfigPtr;
/*
#define TIMEOUT_MAX 20000
uint32_t timeout=TIMEOUT_MAX;
uint32_t timeoutmax=TIMEOUT_MAX;
 */
uint32_t data=0;
extern uint8_t *ROM;
extern uint8_t *EXT_ROM;

extern "C" void load_rom(void)
{
   while(shared->load_rom_emu==0){}
   ROM=(uint8_t *)shared->load_rom_addr;
   printf("load rom emu %d\n",shared->load_rom_emu);
}
extern "C" void load_romext(void)
{
   while(shared->load_romext_emu==0){}
   EXT_ROM=(uint8_t *)shared->load_ext_rom_addr;
   printf("load romext emu %d\n",shared->load_romext_emu);
}
int read_irq=0;
extern "C" int intlev(void)
{
   return(read_irq);
}
#define NOP asm(" nop")

uint16_t last_zaddr=0xFFFF;
uint16_t last_zaddr1=0xFFFF;
uint16_t last_zaddr2=0xFFFF;
uint16_t last_zaddr3=0xFFFF;
uint32_t last_zdata=0xFFFFFFFF;
uint32_t last_zdata1=0xFFFFFFFF;
int last_type1=-1;
int last_type2=-1;

extern "C" void write_rtg_register(uint16_t zaddr,uint32_t zdata)
{
   //    if(zaddr!=last_zaddr)
   shared->write_rtg_addr=zaddr;
   //    if(zdata!=last_zdata)
   shared->write_rtg_data=zdata;
   //    last_zaddr=zaddr;
   //    last_zdata=zdata;
   //    	Xil_L1DCacheFlush();
   shared->write_rtg=1;
   dsb();
   shared->shared_data=1;
   while(shared->write_rtg==1){NOP;}
}
extern "C" uint32_t read_rtg_register(uint16_t zaddr)
{
   //    if(zaddr!=last_zaddr3)
   shared->read_rtg_addr=zaddr;
   //    last_zaddr3=zaddr;
   shared->read_rtg=1;
   dsb();
   //    Xil_L1DCacheInvalidate();
   shared->shared_data=1;
   while(shared->read_rtg==1){NOP;}
   uint32_t data=shared->read_rtg_data;
   //    if(zaddr>=0x1A0 && zaddr<=0x238)
   //    	printf("[EMU] RTG read reg 0x%03X = 0x%08X\n",zaddr,data);
   return(data);
}
enum piscsi_cmds {
   PISCSI_CMD_WRITE        = 0x00,
   PISCSI_CMD_READ         = 0x04,
   PISCSI_CMD_DRVNUM       = 0x08,
   PISCSI_CMD_DRVTYPE      = 0x0C,
   PISCSI_CMD_BLOCKS       = 0x10,
   PISCSI_CMD_CYLS         = 0x14,
   PISCSI_CMD_HEADS        = 0x18,
   PISCSI_CMD_SECS         = 0x1C,
   PISCSI_CMD_ADDR1        = 0x20,
   PISCSI_CMD_ADDR2        = 0x24,
   PISCSI_CMD_ADDR3        = 0x28,
   PISCSI_CMD_ADDR4        = 0x2C,
   PISCSI_CMD_DEBUGME      = 0x30,
   PISCSI_CMD_DRIVER       = 0x50,
   PISCSI_CMD_NEXTPART     = 0x54,
   PISCSI_CMD_GETPART      = 0x58,
   PISCSI_CMD_GETPRIO      = 0x5C,
   PISCSI_CMD_WRITE64      = 0x60,
   PISCSI_CMD_READ64       = 0x64,
   PISCSI_CMD_CHECKFS      = 0x70,
   PISCSI_CMD_NEXTFS       = 0x74,
   PISCSI_CMD_COPYFS       = 0x78,
   PISCSI_CMD_FSSIZE       = 0x7C,
   PISCSI_CMD_SETFSH       = 0x80,
   PISCSI_CMD_BLOCKSIZE    = 0x84,
   PISCSI_CMD_READBYTES    = 0x88,
   PISCSI_CMD_WRITEBYTES   = 0x8C,
   PISCSI_CMD_DRVNUMX      = 0x90,
   PISCSI_CMD_LOADFS       = 0x94,
   PISCSI_CMD_GET_FS_INFO  = 0x98,
   PISCSI_CMD_USED_DMA     = 0x9C,
   PISCSI_DBG_MSG          = 0x100,
   PISCSI_DBG_VAL1         = 0x110,
   PISCSI_DBG_VAL2         = 0x114,
   PISCSI_DBG_VAL3         = 0x118,
   PISCSI_DBG_VAL4         = 0x11C,
   PISCSI_DBG_VAL5         = 0x120,
   PISCSI_DBG_VAL6         = 0x124,
   PISCSI_DBG_VAL7         = 0x128,
   PISCSI_DBG_VAL8         = 0x12C,
   PISCSI_CMD_ROM          = 0x4000,
};
// Floppy command registers (offsets from PIFLOPPY_OFFSET base)
enum floppy_cmds {
    PIFLP_CMD_READ         = 0x400,  // Read sector(s) from ADF
    PIFLP_CMD_WRITE        = 0x404,  // Write sector(s) to ADF
    PIFLP_CMD_DRVNUM       = 0x408,  // Select drive number (0-3)
    PIFLP_CMD_DRVTYPE      = 0x40C,  // Get drive type (0=none, 1=DD, 2=HD)
    PIFLP_CMD_TRACK        = 0x410,  // Set/get current track
    PIFLP_CMD_SIDE         = 0x414,  // Set/get current side (0 or 1)
    PIFLP_CMD_SECTOR       = 0x418,  // Set/get current sector
    PIFLP_CMD_MOTOR        = 0x41C,  // Motor on/off control
    PIFLP_CMD_SEEK         = 0x420,  // Seek to track
    PIFLP_CMD_STATUS       = 0x424,  // Get drive status
    PIFLP_CMD_CHANGE       = 0x428,  // Disk change status
    PIFLP_CMD_PROTECT      = 0x42C,  // Write protect status
    PIFLP_CMD_FORMAT       = 0x430,  // Format track
    PIFLP_CMD_READ_ADDR1   = 0x434,  // Read data address (offset)
    PIFLP_CMD_READ_ADDR2   = 0x438,  // Read data address (length)
    PIFLP_CMD_READ_ADDR3   = 0x43C,  // Read data address (buffer ptr)
    PIFLP_CMD_READ_ADDR4   = 0x440,  // Read data address (actual)
    PIFLP_CMD_WRITE_ADDR1  = 0x444,  // Write data address (offset)
    PIFLP_CMD_WRITE_ADDR2  = 0x448,  // Write data address (length)
    PIFLP_CMD_WRITE_ADDR3  = 0x44C,  // Write data address (buffer ptr)
    PIFLP_CMD_WRITE_ADDR4  = 0x450,  // Write data address (actual)
    PIFLP_CMD_DRVNUMX      = 0x454,  // Extended drive select (with side)
    PIFLP_CMD_READ64       = 0x458,  // 64-bit read command
    PIFLP_CMD_WRITE64      = 0x45C,  // 64-bit write command
    PIFLP_CMD_USED_DMA     = 0x460,  // DMA used flag
    PIFLP_CMD_BLOCKSIZE    = 0x464,  // Sector size (512 for DD, 1024 for HD)
    PIFLP_CMD_TOTALTRACKS  = 0x468,  // Total tracks (80 for DD, 80 for HD)
    PIFLP_CMD_TOTALSECTORS = 0x46C,  // Total sectors per track (11 DD, 22 HD)
    PIFLP_CMD_TOTALSIDES   = 0x470,  // Total sides (1 or 2)
    PIFLP_CMD_CYLINDERS    = 0x474,  // Number of cylinders
    PIFLP_CMD_HEADS        = 0x478,  // Number of heads
    PIFLP_CMD_SECPERTRACK  = 0x47C,  // Sectors per track
    PIFLP_CMD_DISKINSERTED = 0x480,  // Disk inserted flag (ADF loaded)
    PIFLP_CMD_ADFNAME      = 0x484,  // ADF filename pointer (ARM side)

    // Per-unit block size registers (like SCSI)
    PIFLP_CMD_BLOCKSIZE0   = 0x4A0,
    PIFLP_CMD_BLOCKSIZE1   = 0x4A4,
    PIFLP_CMD_BLOCKSIZE2   = 0x4A8,
    PIFLP_CMD_BLOCKSIZE3   = 0x4AC,
    PIFLP_CMD_BLOCKSIZE4   = 0x4B0,
    PIFLP_CMD_BLOCKSIZE5   = 0x4B4,
    PIFLP_CMD_BLOCKSIZE6   = 0x4B8,
    PIFLP_CMD_BLOCKSIZE7   = 0x4BC,

    // Per-unit total blocks registers
    PIFLP_CMD_BLOCKS0      = 0x4C0,
    PIFLP_CMD_BLOCKS1      = 0x4C4,
    PIFLP_CMD_BLOCKS2      = 0x4C8,
    PIFLP_CMD_BLOCKS3      = 0x4CC,
    PIFLP_CMD_BLOCKS4      = 0x4D0,
    PIFLP_CMD_BLOCKS5      = 0x4D4,
    PIFLP_CMD_BLOCKS6      = 0x4D8,
    PIFLP_CMD_BLOCKS7      = 0x4DC,

    // Debug registers
    PIFLP_DBG_MSG          = 0x500,
    PIFLP_DBG_VAL1         = 0x510,
    PIFLP_DBG_VAL2         = 0x514,
    PIFLP_DBG_VAL3         = 0x518,
    PIFLP_DBG_VAL4         = 0x51C,
    PIFLP_DBG_VAL5         = 0x520,
};

uint32_t addr2=0;
uint32_t addr3=0;
extern "C" void write_scsi_register(uint16_t zaddr,uint32_t zdata,int type)
{
   if(zaddr < PISCSI_DBG_MSG)
   {
      switch(zaddr)
      {
      case PISCSI_CMD_BLOCKSIZE:
      case PISCSI_CMD_USED_DMA:
      case PISCSI_CMD_BLOCKS:
      case PISCSI_CMD_GETPART:
      case PISCSI_CMD_CYLS:
      case PISCSI_CMD_DRVTYPE:
      case PISCSI_CMD_GETPRIO:
      case PISCSI_CMD_FSSIZE:
      case PISCSI_CMD_HEADS:
      case PISCSI_CMD_SECS:
      case PISCSI_CMD_CHECKFS:
      case PISCSI_CMD_ADDR1:
         Xil_L1DCacheFlush();
//         Xil_DCacheFlush();
         break;
      case PISCSI_CMD_ADDR2:
         break;
      case PISCSI_CMD_WRITE64:
      case PISCSI_CMD_WRITE:
      case PISCSI_CMD_WRITEBYTES:
         Xil_L1DCacheFlush();
         break;
      case PISCSI_CMD_READ64:
      case PISCSI_CMD_READ:
      case PISCSI_CMD_READBYTES:
         Xil_L1DCacheFlush();
         break;
//      default:
//         printf("SCSI write command %02x\n",zaddr);
      }
   }
   //    if(zaddr!=last_zaddr1)
   shared->write_scsi_addr=zaddr;
   //    if(zdata!=last_zdata1)
   shared->write_scsi_data=zdata;
   //    if(type!=last_type1)
   shared->write_scsi_type=type;
   //    last_zaddr1=zaddr;
   //    last_zdata1=zdata;
   //    last_type1=type;
   shared->write_scsi=1;
   dsb();
   shared->shared_data=1;


   while(shared->write_scsi==1){NOP;}
}

extern "C" uint32_t read_scsi_register(uint16_t zaddr,int type)
{
   //    if(zaddr!=last_zaddr2)
   shared->read_scsi_addr=zaddr;
   //    if(type!=last_type2)
   shared->read_scsi_type=type;
   //    last_zaddr2=zaddr;
   //    last_type2=type;
   shared->read_scsi=1;
   dsb();
   shared->shared_data=1;

   if(zaddr < PISCSI_DBG_MSG)
   {
      switch(zaddr)
      {
      case PISCSI_CMD_BLOCKSIZE:
      case PISCSI_CMD_USED_DMA:
      case PISCSI_CMD_BLOCKS:
      case PISCSI_CMD_GETPART:
      case PISCSI_CMD_CYLS:
      case PISCSI_CMD_DRVTYPE:
      case PISCSI_CMD_GETPRIO:
      case PISCSI_CMD_FSSIZE:
      case PISCSI_CMD_HEADS:
      case PISCSI_CMD_SECS:
      case PISCSI_CMD_CHECKFS:
      case PISCSI_CMD_ADDR1:
         Xil_L1DCacheFlush();
//         Xil_DCacheFlush();
         break;
      case PISCSI_CMD_ADDR2:
         break;
      case PISCSI_CMD_WRITE64:
      case PISCSI_CMD_WRITE:
      case PISCSI_CMD_WRITEBYTES:
         Xil_L1DCacheFlush();
         break;
      case PISCSI_CMD_READ64:
      case PISCSI_CMD_READ:
      case PISCSI_CMD_READBYTES:
         Xil_L1DCacheInvalidate();
         break;
      default:
         printf("SCSI read command %02x\n",zaddr);
      }
   }
   while(shared->read_scsi==1){NOP;}
   return(shared->read_scsi_data);
}
extern "C" void write_floppy_register(uint16_t zaddr,uint32_t zdata,int type)
{
   if(zaddr < PIFLP_DBG_MSG)
   {
      switch(zaddr)
      {
      case PIFLP_CMD_BLOCKSIZE:
      case PIFLP_CMD_USED_DMA:
      case PIFLP_CMD_BLOCKS0:
      case PIFLP_CMD_BLOCKS1:
      case PIFLP_CMD_BLOCKS2:
      case PIFLP_CMD_BLOCKS3:
      case PIFLP_CMD_BLOCKS4:
      case PIFLP_CMD_BLOCKS5:
      case PIFLP_CMD_BLOCKS6:
      case PIFLP_CMD_BLOCKS7:
      case PIFLP_CMD_CYLINDERS:
      case PIFLP_CMD_DRVTYPE:
      case PIFLP_CMD_HEADS:
      case PIFLP_CMD_TOTALSECTORS:
      case PIFLP_CMD_READ_ADDR1:
         Xil_L1DCacheFlush();
//         Xil_DCacheFlush();
         break;
      case PIFLP_CMD_READ_ADDR2:
         break;
      case PIFLP_CMD_WRITE64:
      case PIFLP_CMD_WRITE:
         Xil_L1DCacheFlush();
         break;
      case PIFLP_CMD_READ64:
      case PIFLP_CMD_READ:
         Xil_L1DCacheFlush();
         break;
//      default:
//         printf("FLOPPY write command %02x\n",zaddr);
      }
   }
   //    if(zaddr!=last_zaddr1)
   shared->write_floppy_addr=zaddr;
   //    if(zdata!=last_zdata1)
   shared->write_floppy_data=zdata;
   //    if(type!=last_type1)
   shared->write_floppy_type=type;
   //    last_zaddr1=zaddr;
   //    last_zdata1=zdata;
   //    last_type1=type;
   shared->write_floppy=1;
   dsb();
   shared->shared_data=1;


   while(shared->write_floppy==1){NOP;}
}

extern "C" uint32_t read_floppy_register(uint16_t zaddr,int type)
{
   //    if(zaddr!=last_zaddr2)
   shared->read_floppy_addr=zaddr;
   //    if(type!=last_type2)
   shared->read_floppy_type=type;
   //    last_zaddr2=zaddr;
   //    last_type2=type;
   shared->read_floppy=1;
   dsb();
   shared->shared_data=1;

   if(zaddr < PIFLP_DBG_MSG)
   {
      switch(zaddr)
      {
      case PIFLP_CMD_BLOCKSIZE:
      case PIFLP_CMD_USED_DMA:
      case PIFLP_CMD_BLOCKS0:
      case PIFLP_CMD_BLOCKS1:
      case PIFLP_CMD_BLOCKS2:
      case PIFLP_CMD_BLOCKS3:
      case PIFLP_CMD_BLOCKS4:
      case PIFLP_CMD_BLOCKS5:
      case PIFLP_CMD_BLOCKS6:
      case PIFLP_CMD_BLOCKS7:
      case PIFLP_CMD_CYLINDERS:
      case PIFLP_CMD_DRVTYPE:
      case PIFLP_CMD_HEADS:
      case PIFLP_CMD_WRITE_ADDR1:
         Xil_L1DCacheFlush();
//         Xil_DCacheFlush();
         break;
      case PIFLP_CMD_WRITE_ADDR2:
         break;
      case PIFLP_CMD_WRITE64:
      case PIFLP_CMD_WRITE:
         Xil_L1DCacheFlush();
         break;
      case PIFLP_CMD_READ64:
      case PIFLP_CMD_READ:
         Xil_L1DCacheInvalidate();
         break;
      case PIFLP_CMD_SECPERTRACK:
      case PIFLP_CMD_BLOCKSIZE0:
      case PIFLP_CMD_BLOCKSIZE1:
      case PIFLP_CMD_BLOCKSIZE2:
      case PIFLP_CMD_BLOCKSIZE3:
      case PIFLP_CMD_BLOCKSIZE4:
      case PIFLP_CMD_BLOCKSIZE5:
      case PIFLP_CMD_BLOCKSIZE6:
      case PIFLP_CMD_BLOCKSIZE7:
      case PIFLP_CMD_DISKINSERTED:
         break;
      default:
         printf("FLOPPY read command %04x\n",zaddr);
      }
   }
   while(shared->read_floppy==1){NOP;}
   return(shared->read_floppy_data);
}
/*
uint32_t video_formatter_read(uint16_t op)
{
    shared->read_video=1;
    shared->shared_data=1;
    while(shared->read_video==1){}
    return(shared->read_video_data);
}
 */
extern "C" void cpu_emulator_reset_core0(void)
{
   printf("Sending reset to Core0\n");
   shared->reset_emulator=1;
   shared->shared_data=1;
   while(shared->reset_emulator==1){}
}

void configure_gpio(void)
{
   GpioPsConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
   XGpioPs_CfgInitialize(&GpioPs, GpioPsConfigPtr, GpioPsConfigPtr->BaseAddr);

   XGpioPs_SetDirectionPin(&GpioPs, n040RSTI, 0);
   XGpioPs_SetOutputEnablePin(&GpioPs, n040RSTI, 0);

   XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_0, 0);
   XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_0, 0);
   XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_9, 0);
   XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_9, 0);
   XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_12, 0);
   XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_12, 0);
   XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_15, 0);
   XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_15, 0);

   XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_13, 1);
   XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_13, 1);
   XGpioPs_WritePin(&GpioPs, PS_MIO_13, 1);
   XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_8, 1);
   XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_8, 1);
//   XGpioPs_WritePin(&GpioPs, PS_MIO_8, 1); - Don't disable as 060 will take the bus

   z3660_printf("[Core1] Configured GPIO ...\n\r");
}
volatile int nbg_arm=0;

int access_failure=0;
extern u32 DataAbortAddr;
extern "C" unsigned int read_long(unsigned int address);
extern "C" unsigned int read_word(unsigned int address);
extern "C" unsigned int read_byte(unsigned int address);
void m68k_write_memory_8(unsigned int address, unsigned int value);
void m68k_write_memory_16(unsigned int address, unsigned int value);
void m68k_write_memory_32(unsigned int address, unsigned int value);
unsigned int data_;
//__attribute__ ((section (".ocm")))
/*
void ipl_interrupt_handler(XGpioPs *InstancePtr)
{
//    XGpioPs_IntrClear((XGpioPs *)InstancePtr, Bank, (IntrStatus & IntrEnabled));
 *(volatile unsigned int *)(XPAR_PS7_GPIO_0_BASEADDR+XGPIOPS_INTSTS_OFFSET)&=0x1201;// BIT12|BIT9|BIT0
//    read_irq=XGpioPs_ReadPin(gpio, PS_MIO_0)|(XGpioPs_ReadPin(gpio, PS_MIO_9)<<1)|(XGpioPs_ReadPin(gpio, PS_MIO_12)<<2);
    uint32_t read=*(volatile uint32_t*)(XPAR_PS7_GPIO_0_BASEADDR+XGPIOPS_DATA_RO_OFFSET);
    read_irq =(read>>(PS_MIO_0   ))&1;
    read_irq|=(read>>(PS_MIO_9 -1))&2;
    read_irq|=(read>>(PS_MIO_12-2))&4;
//    z3660_printf("Interrupt!\n");
}
 */
int ipl_read=0;
volatile int read_reset=1;
extern "C" void ipl_main_read(void)
{
   if(ipl_read>0)
   {
      ipl_read--;
      if(ipl_read<0) ipl_read=0;
      int read_irq1,read_irq2;
      //    read_irq=XGpioPs_ReadPin(gpio, PS_MIO_0)|(XGpioPs_ReadPin(gpio, PS_MIO_9)<<1)|(XGpioPs_ReadPin(gpio, PS_MIO_12)<<2);
      uint32_t read1=*(volatile uint32_t*)(XPAR_PS7_GPIO_0_BASEADDR+XGPIOPS_DATA_RO_OFFSET);
      read_irq1 =(read1>>(PS_MIO_0   ))&1;
      read_irq1|=(read1>>(PS_MIO_9 -1))&2;
      read_irq1|=(read1>>(PS_MIO_12-2))&4;
      nbg_arm=(read1>>(PS_MIO_15))&1;
      dsb();

      do {
//         uint32_t read2=*(volatile uint32_t*)(XPAR_PS7_GPIO_0_BASEADDR+XGPIOPS_DATA_RO_OFFSET);
         read_irq2 =(read1>>(PS_MIO_0   ))&1;
         read_irq2|=(read1>>(PS_MIO_9 -1))&2;
         read_irq2|=(read1>>(PS_MIO_12-2))&4;
         if(read_irq1!=read_irq2)
         {
            read_irq1=read_irq2;
            read_irq2=0xFFFFFFFF;
         }
      }while (read_irq1!=read_irq2);
      read_irq=read_irq2;

      read_reset=(read1>>(n040RSTI   ))&1;
   }
}
void ipl_interrupt_handler(void *CallBackRef, u32 Bank, u32 Status)
{
   (void)CallBackRef;
   (void)Bank;
   (void)Status;
   ipl_read++;
   //    z3660_printf("Interrupt!\n");
}
void SWInterruptHandler(void *data)
{
   (void)data;
   printf("[Core1] SW Interrupt!!! (Recoverable Error)\n");
   while(1);
}
void hard_reboot(void);
void UndefinedExceptionHandler(void *data)
{
   (void)data;
   printf("[Core1] Undefined Exception!!! (Unrecoverable Error)\n");
   hard_reboot();
   while(1);
}
void PrefetchAbortHandler(void* data)
{
   (void)data;
   printf("[Core1] Prefetch Abort!!! (Unrecoverable Error)\n");
   hard_reboot();
   while(1);
}
uint16_t histogram_dataabort[65536]={0};
void print_histogram_dataabort(void)
{
   for(int j=0;j<2048;j++)
   {
      printf("0x%03X",j);
      for(int i=0;i<32;i++)
      {
         printf(" %5d",histogram_dataabort[j*32+i]);
      }
      printf("\n");
   }
}
void DataAbortHandler(void *data)
{
   (void)data;
   unsigned int FaultAddress;
   FaultAddress = mfcp(XREG_CP15_DATA_FAULT_ADDRESS);
   histogram_dataabort[FaultAddress>>16]++;
   uint32_t opcode=(*((uint32_t *)DataAbortAddr))>>16;
   //    unsigned int reg=(opcode>>12)&0x0F; // ARM jit code is always compiled using R2 register as address read/write, so we don't need to ask for the register
   //        0xe780 0xe500 0xe580 // STR_rRi
   //        0xe180 0xe1C0 0xe140 // STRH
   //        0xe7C0               // STRB
   if((opcode&0x0930)==0x0100) // STR (0xe100 is the result of and of all above opcodes)
   {
      asm volatile("ldr r1,[sp,#24]");                  // read data from stack (r1)
      asm volatile("str r1, %[data_]"::[data_] "m" (data_)); // store data to data_
      if((opcode&0x0540)==0x0500)       // 0xe780 0xe500 0xe580 // STR
         m68k_write_memory_32(FaultAddress,(unsigned int)swap32(data_));
      else if((opcode&0x0700)==0x0100)  // 0xe180 0xe1C0 0xe140 // STRH
         m68k_write_memory_16(FaultAddress,(unsigned int)swap16(data_));
      else                              // STRB
         m68k_write_memory_8(FaultAddress,data_);
      asm volatile("pop {r4,r5,r6,lr}");          // pop registers
      asm volatile("ldmia    sp!,{r0-r3,r12,lr}");   // pop registers
      asm volatile("subs    pc, lr, #4");           // return
      return;
   }
   else
      //        0xe790 0xe590 // LDR_rRi
      //        0xe190 0xe1D0 // LDRH
      //        0xe7D0        // LDRB
      if((opcode&0x0190)==0x0190) // LDR (0xe190 is the AND of all opcodes)
      {
         if((opcode&0x05d0)==0x0590)                   // 0xe790 0xe590 // LDR_rRi
            data_ = swap32(read_long(FaultAddress));
         else if((opcode&0x0590)==0x0190)              // 0xe190 0xe1D0 // LDRH
            data_ = swap16(read_word(FaultAddress));
         else                                          // LDRB
            data_ = read_byte(FaultAddress);
         asm volatile("ldr r1, %[data_]"::[data_] "m" (data_)); // read data from data_
         asm volatile("str r1,[sp,#24]");          // write data to stack (r1)
         asm volatile("pop {r4,r5,r6,lr}");  // pop registers
         asm volatile("ldmia    sp!,{r0-r3,r12,lr}"); // pop registers
         asm volatile("subs    pc, lr, #4");         // return
         return;
      }
      else
      {
         uint32_t FaultStatus;
         FaultStatus = mfcp(XREG_CP15_DATA_FAULT_STATUS);
         z3660_printf("Data abort with Data Fault Status Register 0x%lx\n",FaultStatus);
         z3660_printf("Address of Instruction causing Data abort 0x%lx\n",DataAbortAddr);
         unsigned int write_fault=FaultStatus&(1<<11);
         if(write_fault)
            z3660_printf("Write Fault to 0x%08X\n",FaultAddress);
         else
            z3660_printf("Read Fault from 0x%08X\n",FaultAddress);
         z3660_printf("Instruction 0x%08X\n",*((uint32_t *)DataAbortAddr));
         z3660_printf("Opcode unknown\n");
         asm volatile("pop {r4,r5,r6,lr}");  // pop registers
         asm volatile("ldmia    sp!,{r0-r3,r12,lr}"); // pop registers
         asm volatile("subs    pc, lr, #4");         // return
         return;
      }
}

XDmaPs DmaInstance;
XScuGic GicInstance;
void SetTlbAttributes(INTPTR Addr, uint32_t attrib)
{
   uint32_t *ptr;
   uint32_t section;

   section = Addr / 0x100000U;
   ptr = &MMUTable;
   ptr += section;
   if(ptr != NULL) {
      *ptr = (Addr & 0xFFF00000U) | attrib;
   }
}
void finish_Attributes(void)
{
   Xil_DCacheFlush();

   mtcp(XREG_CP15_INVAL_UTLB_UNLOCKED, 0U);
   /* Invalidate all branch predictors */
   mtcp(XREG_CP15_INVAL_BRANCH_ARRAY, 0U);

   dsb(); /* ensure completion of the BP and TLB invalidation */
   isb(); /* synchronize context on this processor */
}
XScuGic intc;
void ipl_configure_interrupt(void)
{
   XScuGic_Config *IntcConfig;
   IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);

   XScuGic_CfgInitialize(&intc, IntcConfig, IntcConfig->CpuBaseAddress);

   int intr_target_reg;
   //#define INT_INTERRUPT_ID_0 63 // video
   //#define INT_INTERRUPT_ID_1 64 // audio
#define INT_INTERRUPT_ID_2 65 // vblank
   /*
    intr_target_reg = XScuGic_DistReadReg(&intc,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_0));
    intr_target_reg &= ~(0x000000FF << ((INT_INTERRUPT_ID_0%4)*8));
    intr_target_reg |=  (0x00000001 << ((INT_INTERRUPT_ID_0%4)*8));//CPU0
    intr_target_reg |=  (0x00000002 << ((INT_INTERRUPT_ID_0%4)*8));//CPU1
    XScuGic_DistWriteReg(&intc,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_0),intr_target_reg);

    intr_target_reg = XScuGic_DistReadReg(&intc,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_1));
    intr_target_reg &= ~(0x000000FF << ((INT_INTERRUPT_ID_1%4)*8));
    intr_target_reg |=  (0x00000001 << ((INT_INTERRUPT_ID_1%4)*8));//CPU0
    //intr_target_reg |=  (0x00000002 << ((INT_INTERRUPT_ID_1%4)*8));//CPU1
    XScuGic_DistWriteReg(&intc,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_1),intr_target_reg);
    */
   intr_target_reg = XScuGic_DistReadReg(&intc,XSCUGIC_SPI_TARGET_OFFSET_CALC(XPAR_XGPIOPS_0_INTR));
   intr_target_reg &= ~(0x000000FF << ((XPAR_XGPIOPS_0_INTR%4)*8));
   //intr_target_reg |=  (0x00000001 << ((XPAR_XGPIOPS_0_INTR%4)*8));//CPU0
   intr_target_reg |=  (0x00000002 << ((XPAR_XGPIOPS_0_INTR%4)*8));//CPU1
   XScuGic_DistWriteReg(&intc,XSCUGIC_SPI_TARGET_OFFSET_CALC(XPAR_XGPIOPS_0_INTR),intr_target_reg);

   Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,&intc);
   XScuGic_Connect(&intc,XPAR_XGPIOPS_0_INTR,(Xil_ExceptionHandler)XGpioPs_IntrHandler,(void *)(&GpioPs));
   //    XScuGic_Connect(&intc,XPAR_XGPIOPS_0_INTR,(Xil_ExceptionHandler)ipl_interrupt_handler,(void *)(&GpioPs));
   //    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)ipl_interrupt_handler,&intc);
   XGpioPs_IntrEnablePin(&GpioPs,PS_MIO_0);
   XGpioPs_IntrEnablePin(&GpioPs,PS_MIO_9);
   XGpioPs_IntrEnablePin(&GpioPs,PS_MIO_12);
   XGpioPs_IntrEnablePin(&GpioPs,PS_MIO_15);
   XGpioPs_IntrEnablePin(&GpioPs,n040RSTI);
   XGpioPs_SetIntrTypePin(&GpioPs,PS_MIO_0,XGPIOPS_IRQ_TYPE_EDGE_BOTH);
   XGpioPs_SetIntrTypePin(&GpioPs,PS_MIO_9,XGPIOPS_IRQ_TYPE_EDGE_BOTH);
   XGpioPs_SetIntrTypePin(&GpioPs,PS_MIO_12,XGPIOPS_IRQ_TYPE_EDGE_BOTH);
   XGpioPs_SetIntrTypePin(&GpioPs,PS_MIO_15,XGPIOPS_IRQ_TYPE_EDGE_BOTH);
   XGpioPs_SetIntrTypePin(&GpioPs,n040RSTI,XGPIOPS_IRQ_TYPE_EDGE_FALLING);
   XGpioPs_SetCallbackHandler(&GpioPs,(void *)&GpioPs,ipl_interrupt_handler);
   XScuGic_Enable(&intc,XPAR_XGPIOPS_0_INTR);

   // Interrupts are enabled for both cores, so we disable here what is not needed
   //    *((volatile uint32_t*)0xF8F01834)=0x03010302; // FIXME: disable interrupt for core 0 and core 1
   //    *((volatile uint32_t*)0xF8F0183C)&=~0x02000000; // FIXME: disable interrupt for core 1
   Xil_ExceptionEnable();
}
#define VIDEO_FORMATTER_BASEADDR XPAR_PROCESSING_AV_SYSTEM_AUDIO_VIDEO_ENGINE_VIDEO_VIDEO_FORMATTER_0_BASEADDR

inline uint32_t video_formatter_read(uint16_t op)
{
   return (Xil_In32(VIDEO_FORMATTER_BASEADDR+(op<<2)));
}
void isr_video(void *dummy)
{
   (void)dummy;
   int vblank=video_formatter_read(0);
   if(vblank)
   {
      //#define DEBUG_VBLANK
#ifdef DEBUG_VBLANK
      static int c=0;
      static int s=0;
      c++;
      if(c==600)
      {
         c=0;
         s++;
         printf("[Core1] vb %d %08lX\n",s,*((volatile uint32_t*)0xF8F0183C));
      }
#endif
      Xil_L1DCacheFlush();
      //      Xil_L2CacheFlush();
   }
}

int fpga_interrupt_connect(void)
{
//#define ENABLE_VBLANK_INTERRUPT
#ifdef ENABLE_VBLANK_INTERRUPT
   XScuGic_SetPriorityTriggerType(&intc,INT_INTERRUPT_ID_2, 0xA0, 0x03); // vblank priority 0xA0 (0xF8-0x00), rising edge 0x03
   //int result=
   XScuGic_Connect(&intc, INT_INTERRUPT_ID_2, (Xil_ExceptionHandler)isr_video,NULL);

   XScuGic_Enable(&intc, INT_INTERRUPT_ID_2);
#endif
   return(XST_SUCCESS);
}
uint32_t get_current_cpu_frequency();

int main()
{
   Xil_ICacheEnable();
   Xil_L1DCacheEnable();
   Xil_L2CacheEnable();

   //    Xil_L1DCacheDisable();
   //    Xil_L2CacheDisable();

   // This has happened when upgrading to Vitis...
   // This is incredible, MMU 0 position is not updated with the SetTlbAttributes subroutine!!!
   uint32_t *ptr;
   ptr = &MMUTable;
   ptr[0]=0;
   asm(" dsb");
   // end of This is incredible

   for(int i=0;i<0x00E;i++)     // RAM and Amiga regs, but 0x00F for kickstart
      SetTlbAttributes(i*0x100000UL,RESERVED);
   for(int i=0x00E;i<0x00F;i++) // RAM and Amiga regs, but 0x00F for kickstart
      SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);
   for(int i=0;i<256;i++)
      MMUL2Table[i]=0;//(0x00F00000+(i<<12))|0x5BA;
   printf("MMUL2Table = %08lx\n",((uint32_t)&MMUL2Table));

   // Configure ARM PLL frequency based on silicon version
   int silicon_ver = ps7GetSiliconVersion();
   float silicon_MHZ=666.666667;
   if (silicon_ver == 1) {
      silicon_MHZ=666.666667;
   } else if (silicon_ver == 2) {
      silicon_MHZ=766.666667;
   } else if (silicon_ver == 3) {
      silicon_MHZ=866.666667;
   }

   printf("[Core1] ARM Silicon Version %d -> %3.3f MHz Max\n", silicon_ver, silicon_MHZ);
   
   init_shared();

   if(silicon_ver > 1) {
      ps7_init_custom(shared->arm_freq_code); // 0=667, 1=767, 2=867 MHz, 3=900 MHz, 4=933 MHz, 5=967 MHz, 6=1000 MHz
   }
   printf("Current CPU Frequency: %3.3f MHz\n",get_current_cpu_frequency()/1000000.0f);

   for(int i=0x010;i<0x080;i++) // Mother Board RAM
      SetTlbAttributes(i*0x100000UL,RESERVED);
   for(int i=0x080;i<0x100;i++) // CPU RAM
      SetTlbAttributes(i*0x100000UL,RAM_CACHE_POLICY);
   for(int i=0x100;i<0x180;i++) // extended CPU RAM
      SetTlbAttributes(i*0x100000UL,RESERVED);
   for(int i=0x180;i<0x182;i++) // RTG Registers (2 MB reserved, framebuffer is at 0x200000)
      SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);
   for(int i=0x182;i<0x1FE;i++) // RTG
      SetTlbAttributes(i*0x100000UL,RTG_FB_CACHE_POLICY_FOR_EMU);
   for(int i=0x1C2;i<0x1C3;i++) // RTG Registers (1 MB for soft3d registers)
      Xil_SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);
   for(int i=0x1F0;i<0x1FE;i++) // Audio
      SetTlbAttributes(i*0x100000UL,AUDIO_CACHE_POLICY);
   for(int i=0x1FE;i<0x200;i++) // Ethernet
      SetTlbAttributes(i*0x100000UL,ETHERNET_CACHE_POLICY);
   for(int i=0x200;i<0x300;i++) // Z3 RAM
      SetTlbAttributes(i*0x100000UL,RESERVED);
   for(int i=0x400;i<0x780;i++)
      SetTlbAttributes(i*0x100000UL,RESERVED);
   for(int i=0xFF0;i<0x1000;i++)
      SetTlbAttributes(i*0x100000UL,RESERVED);
   for(int i=0x800;i<0x810;i++)
      SetTlbAttributes(i*0x100000UL,DEVICE_MEMORY);//STRONG_ORDERED);
   for(int i=0x810;i<0x880;i++)
      SetTlbAttributes(i*0x100000UL,STRONG_ORDERED);//NORM_WT_CACHE);//DEVICE_MEMORY);//STRONG_ORDERED);

   SetTlbAttributes(0xFFFF0000UL,NORM_NONCACHE);//0x14DE2);//STRONG_ORDERED|SHAREABLE);//NORM_WT_CACHE);//0x14de2);//NORM_NONCACHE);

   finish_Attributes();

   configure_gpio();
   for(int i=0;i<65536;i++)
      histogram_dataabort[i]=0;

   Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT    , DataAbortHandler,0);
   Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_PREFETCH_ABORT_INT, PrefetchAbortHandler,0);
   Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_UNDEFINED_INT     , UndefinedExceptionHandler,0);
   Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SWI_INT           , SWInterruptHandler,0);

#if INT_IPL_ON_THIS_CORE == 1
   ipl_configure_interrupt();
#endif
   fpga_interrupt_connect();
   z3660_printf("[Core1] Configured Interrupts ...\n\r");
   //    for(int i=0x400,j=0;i<0x500;i++,j++)
   //        Xil_SetTlbAttributes(i*0x100000UL,(0x10000000+j*0x100000UL)|1); // mapped to 0x10000000

   load_rom();
   load_romext();
   local.z3_enabled=shared->z3_enabled;

   local.load_rom_emu   =shared->load_rom_emu;
   local.load_romext_emu=shared->load_romext_emu;

   write_reg(REG4,1); // enable ARM bus

   while(1)
   {
      if(shared->shared_data)
      {
         //            sleep(5);
#ifdef UAE_EMULATOR
#ifdef MUSASHI_EMULATOR
         shared->shared_data=0; // ack to core0
         printf("[Core1] ACK to Core0\n");
         if(shared->cfg_emu==UAE_030)
         {
            uae_emulator(0,68030);
         }
         else if(shared->cfg_emu==UAEJIT_030)
         {
            uae_emulator(1,68030);
         }
         else if(shared->cfg_emu==UAE_040)
         {
            uae_emulator(0,68040);
         }
         else if(shared->cfg_emu==UAEJIT_040)
         {
            uae_emulator(1,68040);
         }
         else if(shared->cfg_emu==MUSASHI)
         {
            musashi_emulator();
         }
         else
         {
            z3660_printf("[Core1] No emulator selected!!!\nHALT!!!\n");
            while(1);
         }
#else
         shared->shared_data=0; // ack to core0
         if(shared->cfg_emu==UAE_030)
         {
            uae_emulator(0,68030);
         }
         else if(shared->cfg_emu==UAEJIT_030)
         {
            uae_emulator(1,68030);
         }
         else if(shared->cfg_emu==UAE_040)
         {
            uae_emulator(0,68040);
         }
         else// if(shared->cfg_emu==UAEJIT_040)
         {
            uae_emulator(1,68040);
         }
         else // decision #8: was a bare 'else' meaning UAEJIT_040 -- an unknown
         {    // mode silently booted JIT-040. Halt loudly instead.
            z3660_printf("[Core1] No emulator selected!!!\nHALT!!!\n");
            while(1);
         }
#endif
#else
#ifdef MUSASHI_EMULATOR
         shared->shared_data=0; // ack to core0
         musashi_emulator();
#else
         z3660_printf("[Core1] No emulator compiled!!!\nHALT!!!\n");
         while(1);
#endif
#endif
      }
   }
   return 0;
}

