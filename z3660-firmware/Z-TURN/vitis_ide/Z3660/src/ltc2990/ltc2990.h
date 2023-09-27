/*
 * ltc2990_init.c
 *
 *  Created on: 08/13/2023
 *      Author: sHaNsHe
 */

#ifndef SRC_LTC2990_INIT_H_
#define SRC_LTC2990_INIT_H_

#include "xil_types.h"
#define LTC_I2C_ADDRESS 0x4C

int iic_write_ltc2990(uint8_t command,uint8_t data);
int iic_read_ltc2990(uint8_t command);
int ltc2990_init(void);
extern volatile uint8_t ReadBuffer_ltc2990[2];

#define LTC_CONTROL_REG 0x01
#define LTC_TRIGGER_REG 0x02
#define LTC_TINT_MSB    0x04
#define LTC_TINT_LSB    0x05
#define LTC_V1_MSB      0x06
#define LTC_V1_LSB      0x07
#define LTC_V2_MSB      0x08
#define LTC_V2_LSB      0x09
#define LTC_V3_MSB      0x0A
#define LTC_V3_LSB      0x0B
#define LTC_V4_MSB      0x0C
#define LTC_V4_LSB      0x0D
#define LTC_VCC_MSB     0x0E
#define LTC_VCC_LSB     0x0F

#endif /* SRC_LTC2990_INIT_H_ */
