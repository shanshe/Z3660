/*
 * writebuffer.cc
 *
 *  Created on: 30 mar. 2023
 *      Author: shanshe
 */

#include "xscutimer.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "main.h"

#ifdef WRITE_BUFFER

void wb_run(void);

extern XScuGic intc;
extern SHARED *shared;
/*
void timer_interrupt_handler(XScuTimer *timer)
{
	//if(XScuTimer_IsExpired(timer))
	{
		wb_run();
	}
	XScuTimer_ClearInterruptStatus(timer);
}
XScuTimer timer;
void timer_configure_interrupt(void)
{
	XScuTimer_Config *Timer_Config;
	Timer_Config = XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID);
	XScuTimer_CfgInitialize(&timer, Timer_Config, Timer_Config->BaseAddr);

    XScuGic_Connect(&intc,XPAR_SCUTIMER_INTR,(Xil_ExceptionHandler)timer_interrupt_handler,(void *)(&timer));
    XScuGic_Enable(&intc,XPAR_SCUTIMER_INTR);
//#define XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ 666666687
    XScuTimer_LoadTimer(&timer, 333); // 1 us
    XScuTimer_EnableInterrupt(&timer);
    XScuTimer_EnableAutoReload(&timer);
    XScuTimer_Start(&timer);
    Xil_ExceptionEnable();
}
*/
WriteBuffer wb_temp;
void wb_push(WriteBuffer *wb1)
{
	uint32_t idx=shared->wb_input_ptr;
	shared->wb[idx].address=wb1->address;
	shared->wb[idx].data   =wb1->data;
	shared->wb[idx].size   =wb1->size;
	uint32_t ptr=(idx+1)&(256-1);
	if(shared->wb_output_ptr==ptr)
		shared->wb_buffer_state=WB_FULL;
	else
		shared->wb_buffer_state=WB_DATA_AVAILABLE;
	shared->wb_input_ptr=ptr;
}
/*
WriteBuffer *wb_pop(void)
{
	uint8_t ptr=shared->wb_output_ptr;
	shared->wb_output_ptr++;
	if(shared->wb_output_ptr==shared->wb_input_ptr)
		shared->wb_buffer_state=WB_EMPTY;
	else
		shared->wb_buffer_state=WB_DATA_AVAILABLE;
	return(&shared->wb[ptr]);
}

extern "C" void arm_write_amiga_wb(uint32_t address, uint32_t data, uint32_t size)
{
	if(fase==0)
	{
		bus_busy=1;
		write_reg(0x08,address);          // address
		write_reg(0x0C,data);             // data
		write_reg(0x10,0x11|WRITE_|size); // command write
		fase=1;
		return;
	}
	else
	{
		if(read_reg(0x14)==1)        // read ack
		{
			write_reg(0x10,0x01|WRITE_);       // confirm ack (bit4=0), tristate bus (READ_)
			fase=0;
			bus_busy=0;
		}
	}
}

void wb_run(void)
{
	if((shared->wb_buffer_state!=WB_EMPTY && bus_busy==0) || (fase==1))
	{
		WriteBuffer *wb_ptr;
		if(fase==0)
		{
			wb_ptr=wb_pop();
			arm_write_amiga_wb(wb_ptr->address,wb_ptr->data,wb_ptr->size);
		}
		else
		{
			arm_write_amiga_wb(0,0,0);
		}
	}
}
*/
#ifdef WRITE_BUFFER
extern "C" void arm_write_amiga(uint32_t address, uint32_t data, uint32_t size)
{
	while(shared->wb_updating);
	shared->wb_updating=1;
	while(shared->wb_buffer_state==WB_FULL)
	{}
	wb_temp.address=address;
	wb_temp.data   =data;
	wb_temp.size   =size;
	wb_push(&wb_temp);
	shared->wb_updating=0;
}
#endif
#endif
