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
#include "usb/ehci.h"

volatile uint32_t amiga_interrupts = 0;
extern SHARED *shared;

void amiga_interrupt_set(uint32_t bit) {
   // set bit
//   if(bit&AMIGA_INTERRUPT_USB) {
//      printf("[set interrupt] USB\n");
//   }
   
   uint32_t old_amiga_interrupts = amiga_interrupts;
   amiga_interrupts |= bit;

   // Only activate INT6 if we're transitioning from no interrupts to having interrupts
   if (amiga_interrupts != 0 && old_amiga_interrupts == 0) {
      DiscreteSet(REG0,FPGA_INT6); // set int6 to 1 (active high)
//      printf("[ARM INT6] *** ACTIVATING INT6 *** bit=0x%08x old=0x%08x new=0x%08x\n",
//             (unsigned)bit, (unsigned)old_amiga_interrupts, (unsigned)amiga_interrupts);
   } else if (amiga_interrupts != 0) {
      // INT6 was already active, just adding more bits
//      printf("[ARM INT6] Adding interrupt bit=0x%08x (INT6 already active: old=0x%08x new=0x%08x)\n",
//             (unsigned)bit, (unsigned)old_amiga_interrupts, (unsigned)amiga_interrupts);
   }
}

uint32_t amiga_interrupt_get() {
   return(amiga_interrupts);
}

void amiga_interrupt_clear(uint32_t bit) {
   // unset bit
   uint32_t old_amiga_interrupts = amiga_interrupts;
   amiga_interrupts &= ~bit;

   // Only deactivate INT6 if we're transitioning from having interrupts to no interrupts
   if (amiga_interrupts == 0 && old_amiga_interrupts != 0) {
      DiscreteClear(REG0,FPGA_INT6); // set int6 to 0 (active high)
   }
}
int read_irq=0;
void ipl_interrupt_handler(void *CallBackRef, u32 Bank, u32 Status)
{
   (void)Bank;
   (void)Status;

   XGpioPs *gpio=(XGpioPs *)CallBackRef;

   read_irq=XGpioPs_ReadPin(gpio, PS_MIO_0)|(XGpioPs_ReadPin(gpio, PS_MIO_9)<<1)|(XGpioPs_ReadPin(gpio, PS_MIO_12)<<2);
   shared->irq=read_irq;
   shared->int_available=1;
   //	printf("Interrupt!\n");
}
volatile int read_reset=0;
extern int cached_videomode;
void reset_interrupt_handler(void *CallBackRef, u32 Bank, u32 Status)
{
   (void)CallBackRef;
   (void)Bank;
   (void)Status;
   
   //	XGpioPs *gpio=(XGpioPs *)CallBackRef;
   //	if(XGpioPs_ReadPin(gpio, n040RSTI)==0) // hehehe if you read it, surely it is high...
   {
      read_reset=1;
      printf("[Core0] Reset detected!\n");
      cached_videomode=-1;
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
int fpga_interrupt_connect(void* isr_video,void* isr_audio_tx, void* isr_usb, int ipl)
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
   XScuGic_Connect(&int_handler, INT_INTERRUPT_ID_2, (Xil_ExceptionHandler)isr_usb,NULL);

   XScuGic_Enable(&int_handler, INT_INTERRUPT_ID_0);
   XScuGic_Enable(&int_handler, INT_INTERRUPT_ID_1);
   XScuGic_Enable(&int_handler, INT_INTERRUPT_ID_2);
   
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

/*
 * Check USB interrupt status for debugging purposes
 * This function examines the current USB interrupt state
 * and logs relevant information for troubleshooting.
 */
void check_usb_interrupt_status(const char *context)
{
    printf("[USB ISR Check] %s\n", context ? context : "Unknown context");
    
    // Check if USB is initialized (check if zz_usb_init was successful)
    // We can check this by looking for a valid EHCI controller
    extern struct ehci_ctrl ehcic[];
    struct ehci_ctrl *ehci_ctrl = &ehcic[0];
    int usb_initialized = (ehci_ctrl && ehci_ctrl->hcor) ? 1 : 0;
    printf("[USB ISR Check] USB initialized: %s\n", usb_initialized ? "YES" : "NO");
    
    // Check current Amiga interrupt status
    uint32_t current_interrupts = amiga_interrupt_get();
    printf("[USB ISR Check] Amiga interrupts: 0x%08x (USB=%s)\n", 
           (unsigned)current_interrupts,
           (current_interrupts & AMIGA_INTERRUPT_USB) ? "SET" : "CLEAR");
    
    // Check if USB interrupt is enabled in GIC
    int usb_enabled = 0;
    uint32_t enable_reg = XScuGic_DistReadReg(&int_handler, XSCUGIC_ENABLE_SET_OFFSET + (INT_INTERRUPT_ID_2 / 32) * 4);
    if (enable_reg & (1U << (INT_INTERRUPT_ID_2 % 32))) {
        usb_enabled = 1;
    }
    printf("[USB ISR Check] USB interrupt enabled in GIC: %s\n", usb_enabled ? "YES" : "NO");
    
    // Check GIC pending status using available register read
    uint32_t pending_reg = XScuGic_DistReadReg(&int_handler, XSCUGIC_PENDING_SET_OFFSET + (INT_INTERRUPT_ID_2 / 32) * 4);
    int usb_pending = (pending_reg & (1U << (INT_INTERRUPT_ID_2 % 32))) ? 1 : 0;
    printf("[USB ISR Check] USB interrupt pending in GIC: %s\n", usb_pending ? "YES" : "NO");
    
    if (usb_initialized) {
        // Read EHCI USB controller registers for detailed status
        if (ehci_ctrl && ehci_ctrl->hcor) {
            uint32_t usbsts = *(volatile uint32_t *)((uintptr_t)ehci_ctrl->hcor + 0x004);  // USBSTS offset
            uint32_t usbintr = *(volatile uint32_t *)((uintptr_t)ehci_ctrl->hcor + 0x008); // USBINTR offset
            uint32_t portsc = *(volatile uint32_t *)((uintptr_t)ehci_ctrl->hcor + 0x044);  // PORTSC offset
            
            printf("[USB ISR Check] EHCI USBSTS: 0x%08x\n", (unsigned)usbsts);
            printf("[USB ISR Check] EHCI USBINTR: 0x%08x\n", (unsigned)usbintr);
            printf("[USB ISR Check] EHCI PORTSC: 0x%08x\n", (unsigned)portsc);
            
            // Check for active interrupt sources
            uint32_t active_interrupts = usbsts & usbintr;
            if (active_interrupts) {
                printf("[USB ISR Check] Active EHCI interrupts: 0x%08x\n", (unsigned)active_interrupts);
                if (active_interrupts & 0x01) printf("[USB ISR Check]   - USB Transaction Complete\n");
                if (active_interrupts & 0x02) printf("[USB ISR Check]   - USB Error\n");
                if (active_interrupts & 0x04) printf("[USB ISR Check]   - Port Change Detect\n");
                if (active_interrupts & 0x08) printf("[USB ISR Check]   - Frame List Rollover\n");
                if (active_interrupts & 0x10) printf("[USB ISR Check]   - Host System Error\n");
                if (active_interrupts & 0x20) printf("[USB ISR Check]   - Interrupt on Async Advance\n");
            } else {
                printf("[USB ISR Check] No active EHCI interrupts\n");
            }
        } else {
            printf("[USB ISR Check] EHCI controller not accessible\n");
        }
    }
    
    printf("[USB ISR Check] Status check complete for: %s\n", context ? context : "Unknown context");
}

