/*
 * interrupt.c
 *
 *  Created on: 23 sept. 2022
 *      Author: shanshe
 */

#include "interrupt.h"
#include "main.h"
#include "cpu_emulator.h"
#include "xil_types.h"
#include <stdio.h>

volatile uint32_t amiga_interrupts = 0;
extern SHARED *shared;

void amiga_interrupt_set(uint32_t bit) {
	//printf("[airq] +%lu\n", bit);
	// set bit
	uint32_t old_amiga_interrupts=amiga_interrupts;
	amiga_interrupts |= bit;

	if (amiga_interrupts != 0 && old_amiga_interrupts != amiga_interrupts) {
		DiscreteSet(REG0,FPGA_INT6); // set int6 to 1 (active high)
	}
}

volatile uint32_t amiga_interrupt_get() {
	return(amiga_interrupts);
}

void amiga_interrupt_clear(uint32_t bit) {
	//printf("[airq] -%lu\n", bit);
	// unset bit
	uint32_t old_amiga_interrupts=amiga_interrupts;
	amiga_interrupts = amiga_interrupts & ~bit;

	if (amiga_interrupts == 0 && old_amiga_interrupts != 0) {
		DiscreteClear(REG0,FPGA_INT6); // set int6 to 0 (active high)
	}
}
int read_irq=0;
void ipl_interrupt_handler(void *CallBackRef, u32 Bank, u32 Status)
{
	XGpioPs *gpio=(XGpioPs *)CallBackRef;

	read_irq=XGpioPs_ReadPin(gpio, PS_MIO_0)|(XGpioPs_ReadPin(gpio, PS_MIO_9)<<1)|(XGpioPs_ReadPin(gpio, PS_MIO_12)<<2);
	shared->irq=read_irq;
	shared->int_available=1;
//	printf("Interrupt!\n");
}
volatile int read_reset=0;
void reset_interrupt_handler(void *CallBackRef, u32 Bank, u32 Status)
{
//	XGpioPs *gpio=(XGpioPs *)CallBackRef;
//	if(XGpioPs_ReadPin(gpio, n040RSTI)==0) // hehehe if you read it, surely it is high...
	{
		read_reset=1;
		printf("Reset detected!\n");
	}
}

XScuGic int_handler;
XScuGic* interrupt_get_intc()
{
	return(&int_handler);
}

int interrupt_init(void)
{
	int result;
	XScuGic_Config *int_config;
	int_config=XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	if(!int_config)
		return(XST_FAILURE);
	result=XScuGic_CfgInitialize(&int_handler,int_config,int_config->CpuBaseAddress);
	if(result!=XST_SUCCESS)
		return(result);
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,&int_handler);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,&int_handler);

	return(XST_SUCCESS);
}
int fpga_interrupt_connect(void* isr_video,void* isr_audio_tx, int ipl)
{
	if(ipl)
	{
		*((volatile uint32_t*)0xF8F01834)=0x03010302; // FIXME: disable interrupt for core 0 and core 1
//		*((volatile uint32_t*)0xF8F0183C)&=~0x02000000; // FIXME: disable interrupt for core 1
		int intr_target_reg;

		intr_target_reg = XScuGic_DistReadReg(&int_handler,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_0));
		intr_target_reg &= ~(0x000000FF << ((INT_INTERRUPT_ID_0%4)*8));
		intr_target_reg |=  (0x00000001 << ((INT_INTERRUPT_ID_0%4)*8));//CPU0
		//intr_target_reg |=  (0x00000002 << ((INT_INTERRUPT_ID_0%4)*8));//CPU1
		XScuGic_DistWriteReg(&int_handler,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_0),intr_target_reg);

		intr_target_reg = XScuGic_DistReadReg(&int_handler,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_1));
		intr_target_reg &= ~(0x000000FF << ((INT_INTERRUPT_ID_1%4)*8));
		intr_target_reg |=  (0x00000001 << ((INT_INTERRUPT_ID_1%4)*8));//CPU0
		//intr_target_reg |=  (0x00000002 << ((INT_INTERRUPT_ID_1%4)*8));//CPU1
		XScuGic_DistWriteReg(&int_handler,XSCUGIC_SPI_TARGET_OFFSET_CALC(INT_INTERRUPT_ID_1),intr_target_reg);

		intr_target_reg = XScuGic_DistReadReg(&int_handler,XSCUGIC_SPI_TARGET_OFFSET_CALC(XPAR_XGPIOPS_0_INTR));
		intr_target_reg &= ~(0x000000FF << ((XPAR_XGPIOPS_0_INTR%4)*8));
		//intr_target_reg |=  (0x00000001 << ((XPAR_XGPIOPS_0_INTR%4)*8));//CPU0
		intr_target_reg |=  (0x00000002 << ((XPAR_XGPIOPS_0_INTR%4)*8));//CPU1
		XScuGic_DistWriteReg(&int_handler,XSCUGIC_SPI_TARGET_OFFSET_CALC(XPAR_XGPIOPS_0_INTR),intr_target_reg);

	}
	XScuGic_SetPriorityTriggerType(&int_handler,INT_INTERRUPT_ID_0, 0xA0, 0x03); // vblank priority 0xA0 (0xF8-0x00), rising edge 0x03
	XScuGic_SetPriorityTriggerType(&int_handler,INT_INTERRUPT_ID_1, 0x90, 0x03); // audio priority 0x90 (0xF8-0x00), rising edge 0x03
	//int result=
	XScuGic_Connect(&int_handler, INT_INTERRUPT_ID_0, (Xil_ExceptionHandler)isr_video,NULL);
	XScuGic_Connect(&int_handler, INT_INTERRUPT_ID_1, (Xil_ExceptionHandler)isr_audio_tx,NULL);

	XScuGic_Enable(&int_handler, INT_INTERRUPT_ID_0);
	XScuGic_Enable(&int_handler, INT_INTERRUPT_ID_1);

	if(ipl)
	{
		XScuGic_Connect(&int_handler,XPAR_XGPIOPS_0_INTR,(Xil_ExceptionHandler)XGpioPs_IntrHandler,(void *)(&GpioPs));
		XGpioPs_IntrEnablePin(&GpioPs,PS_MIO_0);
		XGpioPs_IntrEnablePin(&GpioPs,PS_MIO_9);
		XGpioPs_IntrEnablePin(&GpioPs,PS_MIO_12);
		XGpioPs_SetIntrTypePin(&GpioPs,PS_MIO_0,XGPIOPS_IRQ_TYPE_EDGE_BOTH);
		XGpioPs_SetIntrTypePin(&GpioPs,PS_MIO_9,XGPIOPS_IRQ_TYPE_EDGE_BOTH);
		XGpioPs_SetIntrTypePin(&GpioPs,PS_MIO_12,XGPIOPS_IRQ_TYPE_EDGE_BOTH);
		XGpioPs_SetCallbackHandler(&GpioPs,(void *)&GpioPs,ipl_interrupt_handler);
		XScuGic_Enable(&int_handler,XPAR_XGPIOPS_0_INTR);
	}
	XScuGic_Connect(&int_handler,XPAR_XGPIOPS_0_INTR,(Xil_ExceptionHandler)XGpioPs_IntrHandler,(void *)(&GpioPs));
	XGpioPs_IntrEnablePin(&GpioPs,n040RSTI);
	XGpioPs_SetIntrTypePin(&GpioPs,n040RSTI,XGPIOPS_IRQ_TYPE_EDGE_FALLING);
	XGpioPs_SetCallbackHandler(&GpioPs,NULL,reset_interrupt_handler);
	XScuGic_Enable(&int_handler,XPAR_XGPIOPS_0_INTR);
	return(XST_SUCCESS);
}

