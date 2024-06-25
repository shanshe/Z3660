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

extern "C" void musashi_emulator(void);
/*
#define be16toh(A) (A)
#define htobe16(A) (A)
#define be32toh(A) (A)
#define htobe32(A) (A)
*/
extern "C" void ps_write_32(unsigned int address, unsigned int value);
extern "C" void ps_write_16(unsigned int address, unsigned int value);
extern "C" void ps_write_8(unsigned int address, unsigned int value);
extern "C" unsigned int ps_read_8(unsigned int address);
extern "C" unsigned int ps_read_16(unsigned int address);
extern "C" unsigned int ps_read_32(unsigned int address);
extern "C" unsigned int  m68k_read_memory_8(unsigned int address);
extern "C" unsigned int  m68k_read_memory_16(unsigned int address);
extern "C" unsigned int  m68k_read_memory_32(unsigned int address);
extern "C" void m68k_write_memory_8(unsigned int address, unsigned int value);
extern "C" void m68k_write_memory_16(unsigned int address, unsigned int value);
extern "C" void m68k_write_memory_32(unsigned int address, unsigned int value);
#define swap32(a) __builtin_bswap32(a)
#define swap16(a) __builtin_bswap16(a)

void M68K_StartEmu(void *addr, void *fdt);

#endif /* SRC_CPU_EMULATOR_H_ */
