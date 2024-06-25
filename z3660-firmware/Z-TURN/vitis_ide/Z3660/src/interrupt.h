/*
 * interrupt.h
 *
 *  Created on: 23 sept. 2022
 *      Author: shanshe
 */

#ifndef SRC_INTERRUPT_H_
#define SRC_INTERRUPT_H_

#define INT_INTERRUPT_ID_0 63
#define INT_INTERRUPT_ID_1 64

#include "xscugic.h"
#include "xgpiops.h"

// IRQ mask bits
#define AMIGA_INTERRUPT_ETH   1
#define AMIGA_INTERRUPT_AUDIO 2

extern XScuGic int_handler;
extern XGpioPs GpioPs;
int interrupt_init(void);
void amiga_interrupt_clear(uint32_t bit);
void amiga_interrupt_set(uint32_t bit);
volatile uint32_t amiga_interrupt_get();
XScuGic* interrupt_get_intc();
int fpga_interrupt_connect(void* isr_video,void* isr_audio_tx, int ipl);

#endif /* SRC_INTERRUPT_H_ */
