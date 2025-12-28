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
 * @brief Reads current DDR PLL frequency configuration
 * @param debug Set to 1 to print debug information, 0 for silent
 * @return DDR frequency in MHz, or 0 if unable to read
 */
int ps7_ddr_get_frequency(int debug);

/**
 * @brief Simple wrapper for backward compatibility
 * @return DDR frequency in MHz, or 0 if unable to read
 */
int ps7_ddr_get_frequency_simple(void);

/**
 * @brief Simple function to get silicon version
 * @return Silicon version
 */
int ps7GetSiliconVersion(void);

#endif // PS7_INIT_SIMPLE_H