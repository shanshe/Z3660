/*
 * adc.h
 *
 *  Created on: 26 sept. 2022
 *      Author: shanshe
 */

#ifndef SRC_ADC_H_
#define SRC_ADC_H_

int xadc_init();
float xadc_get_temperature();
float xadc_get_aux_voltage();
float xadc_get_int_voltage();


#endif /* SRC_ADC_H_ */
