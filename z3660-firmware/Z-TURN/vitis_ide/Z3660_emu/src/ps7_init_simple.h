#ifndef PS7_INIT_SIMPLE_H
#define PS7_INIT_SIMPLE_H

#include <stdio.h>
#include <stdint.h>

/**
 * @brief Simplified implementation of ps7_init
 * Configures ARM PLL for 667 MHz by default
 * @return 0 if success, -1 if error
 */
int ps7_init(void);

/**
 * @brief Configures ARM PLL for different frequencies
 * @param freq_code 0=667MHz, 1=767MHz, 2=867MHz
 * @return 0 if success, -1 if error
 */
int ps7_init_custom(int freq_code);

/**
 * @brief Simple function to get silicon version
 * @return Silicon version (3 for Zynq-7000)
 */
int ps7GetSiliconVersion(void);

#endif // PS7_INIT_SIMPLE_H