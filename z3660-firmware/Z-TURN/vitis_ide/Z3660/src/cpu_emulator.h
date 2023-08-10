/*
 * cpu_emulator.h
 *
 *  Created on: 3 dic. 2022
 *      Author: shanshe
 *
 *  Just a simple wrapper for Musashi
 *
 */

#ifndef SRC_CPU_EMULATOR_H_
#define SRC_CPU_EMULATOR_H_

#include <inttypes.h>

void cpu_emulator(void);
#define be16toh(val) __builtin_bswap16(val)
#define htobe16(val) __builtin_bswap16(val)
#define be32toh(val) __builtin_bswap32(val)
#define htobe32(val) __builtin_bswap32(val)

void ps_write_32(unsigned int address, unsigned int value);
void ps_write_16(unsigned int address, unsigned int value);
void ps_write_8(unsigned int address, unsigned int value);
uint32_t ps_read_8(uint32_t address);
uint32_t ps_read_16(uint32_t address);
uint32_t ps_read_32(uint32_t address);

unsigned int READ_IPL(void);
unsigned int READ_NBG_ARM(void);

void M68K_StartEmu(void *addr, void *fdt);


#endif /* SRC_CPU_EMULATOR_H_ */
