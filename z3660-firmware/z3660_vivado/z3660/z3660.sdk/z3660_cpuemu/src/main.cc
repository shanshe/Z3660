/*
 * Empty C++ Application
 */

#include <stdio.h>
#include <xil_cache.h>
#include <xil_mmu.h>
#include <inttypes.h>
#include "xgpiops.h"
#include "xparameters.h"
#include "main.h"
#include "xil_exception.h"
#include "xdmaps.h"
#include "xscugic.h"
#include "xscutimer.h"

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

SHARED *shared;
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
void init_shared(void)
{
	shared=(SHARED *)0xFFFF0000;
}
XGpioPs GpioPs;
XGpioPs_Config *GpioPsConfigPtr;
#define TIMEOUT_MAX 20000
uint32_t timeout=TIMEOUT_MAX;
uint32_t timeoutmax=TIMEOUT_MAX;
uint32_t data=0;
extern uint8_t *ROM;

extern "C" void load_rom(int load)
{
	shared->load_rom_addr=(uint32_t)ROM;
	if(load)
		shared->load_rom_emu=1;
	else
		shared->load_rom_emu=2;
	while(shared->load_rom_emu>0){}
}
int read_irq=0;
extern "C" int intlev(void)
{
	return(read_irq);
}
#define NOP asm(" nop")

uint16_t last_zaddr=0xFFFF;
uint32_t last_zdata=0xFFFFFFFF;

extern "C" void write_rtg_register(uint16_t zaddr,uint32_t zdata)
{
	if(zaddr!=last_zaddr)
		shared->write_rtg_addr=zaddr;
	if(zdata!=last_zdata)
		shared->write_rtg_data=zdata;
	last_zaddr=zaddr;
	last_zdata=zdata;
	shared->write_rtg=1;
	shared->shared_data=1;
	while(shared->write_rtg==1){NOP;}
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
//	XGpioPs_SetDirectionPin(&GpioPs, PS_MIO_8, 1);
//	XGpioPs_SetOutputEnablePin(&GpioPs, PS_MIO_8, 1);
//	XGpioPs_WritePin(&GpioPs, PS_MIO_8, 1);

	z3660_printf("[Core1] Configured GPIO ...\n\r");
}
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
//	XGpioPs_IntrClear((XGpioPs *)InstancePtr, Bank, (IntrStatus & IntrEnabled));
	*(volatile unsigned int *)(XPAR_PS7_GPIO_0_BASEADDR+XGPIOPS_INTSTS_OFFSET)&=0x1201;// BIT12|BIT9|BIT0
//	read_irq=XGpioPs_ReadPin(gpio, PS_MIO_0)|(XGpioPs_ReadPin(gpio, PS_MIO_9)<<1)|(XGpioPs_ReadPin(gpio, PS_MIO_12)<<2);
	uint32_t read=*(volatile uint32_t*)(XPAR_PS7_GPIO_0_BASEADDR+XGPIOPS_DATA_RO_OFFSET);
	read_irq =(read>>(PS_MIO_0   ))&1;
	read_irq|=(read>>(PS_MIO_9 -1))&2;
	read_irq|=(read>>(PS_MIO_12-2))&4;
//	z3660_printf("Interrupt!\n");
}
*/
void ipl_interrupt_handler(void *CallBackRef, u32 Bank, u32 Status)
{
//	read_irq=XGpioPs_ReadPin(gpio, PS_MIO_0)|(XGpioPs_ReadPin(gpio, PS_MIO_9)<<1)|(XGpioPs_ReadPin(gpio, PS_MIO_12)<<2);
	uint32_t read=*(volatile uint32_t*)(XPAR_PS7_GPIO_0_BASEADDR+XGPIOPS_DATA_RO_OFFSET);
	read_irq =(read>>(PS_MIO_0   ))&1;
	read_irq|=(read>>(PS_MIO_9 -1))&2;
	read_irq|=(read>>(PS_MIO_12-2))&4;
//	z3660_printf("Interrupt!\n");
}

void SWInterruptHandler(void *data)
{
	z3660_printf("[Core1] SW Interrupt!!! (Recoverable Error)\n");
}
void UndefinedExceptionHandler(void *data)
{
	z3660_printf("[Core1] Undefined Exception!!! (Unrecoverable Error)\n");
}
void PrefetchAbortHandler(void* data)
{
	z3660_printf("[Core1] Prefetch Abort!!! (Unrecoverable Error)\n");
}
void DataAbortHandler(void *data)
{
	unsigned int FaultAddress;
	FaultAddress = mfcp(XREG_CP15_DATA_FAULT_ADDRESS);

	uint32_t opcode=(*((uint32_t *)DataAbortAddr))>>16;
//	unsigned int reg=(opcode>>12)&0x0F; // ARM jit code is always compiled using R2 register as address read/write, so we don't need to ask for the register
//		0xe780 0xe500 0xe580 // STR_rRi
//		0xe180 0xe1C0 0xe140 // STRH
//		0xe7C0               // STRB
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
		asm volatile("ldmia	sp!,{r0-r3,r12,lr}");   // pop registers
		asm volatile("subs	pc, lr, #4");           // return
		return;
	}
	else
//		0xe790 0xe590 // LDR_rRi
//		0xe190 0xe1D0 // LDRH
//		0xe7D0        // LDRB
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
		asm volatile("ldmia	sp!,{r0-r3,r12,lr}"); // pop registers
		asm volatile("subs	pc, lr, #4");         // return
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
	}
}

XDmaPs DmaInstance;
XScuGic GicInstance;
extern u32 MMUTable;
void SetTlbAttributes(INTPTR Addr, u32 attrib)
{
	u32 *ptr;
	u32 section;

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
XScuGic_Config *IntcConfig;
XScuGic intc;
void ipl_configure_interrupt(void)
{
    IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);

    XScuGic_CfgInitialize(&intc, IntcConfig, IntcConfig->CpuBaseAddress);

	int intr_target_reg;
#define INT_INTERRUPT_ID_0 63
#define INT_INTERRUPT_ID_1 64
	intr_target_reg = XScuGic_DistReadReg(&intc,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_0));
	intr_target_reg &= ~(0x000000FF << ((INT_INTERRUPT_ID_0%4)*8));
	intr_target_reg |=  (0x00000001 << ((INT_INTERRUPT_ID_0%4)*8));//CPU0
	//intr_target_reg |=  (0x00000002 << ((INT_INTERRUPT_ID_0%4)*8));//CPU1
	XScuGic_DistWriteReg(&intc,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_0),intr_target_reg);

	intr_target_reg = XScuGic_DistReadReg(&intc,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_1));
	intr_target_reg &= ~(0x000000FF << ((INT_INTERRUPT_ID_1%4)*8));
	intr_target_reg |=  (0x00000001 << ((INT_INTERRUPT_ID_1%4)*8));//CPU0
	//intr_target_reg |=  (0x00000002 << ((INT_INTERRUPT_ID_1%4)*8));//CPU1
	XScuGic_DistWriteReg(&intc,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_1),intr_target_reg);

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
    XGpioPs_SetIntrTypePin(&GpioPs,PS_MIO_0,XGPIOPS_IRQ_TYPE_EDGE_BOTH);
    XGpioPs_SetIntrTypePin(&GpioPs,PS_MIO_9,XGPIOPS_IRQ_TYPE_EDGE_BOTH);
    XGpioPs_SetIntrTypePin(&GpioPs,PS_MIO_12,XGPIOPS_IRQ_TYPE_EDGE_BOTH);
    XGpioPs_SetCallbackHandler(&GpioPs,(void *)&GpioPs,ipl_interrupt_handler);
    XScuGic_Enable(&intc,XPAR_XGPIOPS_0_INTR);
    // Interrupts are enabled for both cores, so we disable here what is not needed
    *((volatile uint32_t*)0xF8F01834)=0x03010302; // FIXME: disable interrupt for core 0 and core 1
//    *((volatile uint32_t*)0xF8F0183C)&=~0x02000000; // FIXME: disable interrupt for core 1
    Xil_ExceptionEnable();
}

int main()
{
    Xil_ICacheEnable();
    Xil_DCacheEnable();

    for(int i=0;i<0x00F;i++)
    	SetTlbAttributes(i*0x100000UL,RESERVED);
    for(int i=0x010;i<0x080;i++) // Mother Board RAM
    	SetTlbAttributes(i*0x100000UL,RESERVED);
//    for(int i=0x100;i<0x180;i++) // extended CPU RAM
//    	SetTlbAttributes(i*0x100000UL,RESERVED);
	for(int i=0x180;i<0x182;i++) // RTG Registers (2 MB reserved, framebuffer is at 0x200000)
		SetTlbAttributes(i*0x100000UL,NORM_WT_CACHE);//NORM_NONCACHE);
//	for(int i=0x182;i<0x200;i++) // RTG RAM
//		SetTlbAttributes(i*0x100000UL,NORM_WT_CACHE);//0x14de2);//NORM_NONCACHE);
//    for(int i=0x200;i<0x300;i++) // Z3 RAM
//    	SetTlbAttributes(i*0x100000UL,RESERVED);
    for(int i=0x400;i<0x780;i++)
    	SetTlbAttributes(i*0x100000UL,RESERVED);
    for(int i=0xFF0;i<0x1000;i++)
    	SetTlbAttributes(i*0x100000UL,RESERVED);
    SetTlbAttributes(0xFFFF0000UL,0x14DE2);//STRONG_ORDERED|SHAREABLE);//NORM_WT_CACHE);//0x14de2);//NORM_NONCACHE);
	for(int i=0x800;i<0x802;i++)
		SetTlbAttributes(i*0x100000UL,DEVICE_MEMORY);//STRONG_ORDERED);

    finish_Attributes();

    init_shared();

    configure_gpio();

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT    , DataAbortHandler,0);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_PREFETCH_ABORT_INT, PrefetchAbortHandler,0);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_UNDEFINED_INT     , UndefinedExceptionHandler,0);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SWI_INT           , SWInterruptHandler,0);

#if IPL_INT_ON_THIS_CORE == 1
    ipl_configure_interrupt();
#endif
#ifdef WRITE_BUFFER_INTERRUPT
    timer_configure_interrupt();
#endif
    //    for(int i=0x400,j=0;i<0x500;i++,j++)
    //    	Xil_SetTlbAttributes(i*0x100000UL,(0x10000000+j*0x100000UL)|1); // mapped to 0x10000000

	while(1)
	{
		if(shared->shared_data)
		{
//			sleep(5);
#ifdef UAE_EMULATOR
#ifdef MUSASHI_EMULATOR
			shared->shared_data=0; // ack to core0
			if(shared->cfg_emu==1)
			{
			    // Esto hay que repetirlo m�s adelante...
			    *((volatile uint32_t*)0xF8F01834)=0x03010302; // FIXME: disable interrupt for core 0 and core 1
				uae_emulator(1);
			}
			else if(shared->cfg_emu==2)
			{
			    // Esto hay que repetirlo m�s adelante...
			    *((volatile uint32_t*)0xF8F01834)=0x03010302; // FIXME: disable interrupt for core 0 and core 1
				uae_emulator(0);
			}
			else if(shared->cfg_emu==3)
			{
			    // Esto hay que repetirlo m�s adelante...
			    *((volatile uint32_t*)0xF8F01834)=0x03010302; // FIXME: disable interrupt for core 0 and core 1
				cpu_emulator();
			}
			else
			{
				z3660_printf("[Core1] No emulator selected!!!\nHALT!!!\n");
				while(1);
			}
#else
			shared->shared_data=0; // ack to core0
			if(shared->cfg_emu==1)
			{
			    // Esto hay que repetirlo m�s adelante...
			    *((volatile uint32_t*)0xF8F01834)=0x03010302; // FIXME: disable interrupt for core 0 and core 1
				uae_emulator(1);
			}
			else// if(shared->cfg_emu==2)
			{
			    // Esto hay que repetirlo m�s adelante...
			    *((volatile uint32_t*)0xF8F01834)=0x03010302; // FIXME: disable interrupt for core 0 and core 1
				uae_emulator(0);
			}
#endif
#else
#ifdef MUSASHI_EMULATOR
			shared->shared_data=0; // ack to core0
		    // Esto hay que repetirlo m�s adelante...
		    *((volatile uint32_t*)0xF8F01834)=0x03010302; // FIXME: disable interrupt for core 0 and core 1
			cpu_emulator();
#else
			z3660_printf("[Core1] No emulator compiled!!!\nHALT!!!\n");
			while(1);
#endif
#endif
		}
	}
	return 0;
}
