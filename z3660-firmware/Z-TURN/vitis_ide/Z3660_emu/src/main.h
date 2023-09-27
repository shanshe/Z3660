/*
 * main.h
 *
 *  Created on: 5 ene. 2023
 *      Author: shanshe
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include "cpu_emulator.h"
#include <inttypes.h>
extern int configured;
extern int enabled;

#define IPL_INT_ON_THIS_CORE 1
#define AUTOCONFIG_ENABLED
#define CPU_RAM

// MUX signals
#define PS_MIO_0      0 // MIO  0
#define PS_MIO_8      8 // MIO  8
#define PS_MIO_9      9 // MIO  9
#define PS_MIO_12    12 // MIO 12
#define PS_MIO_13    13 // MIO 13
#define PS_MIO_15    15 // MIO 15

// Reset Input signal
#define n040RSTI     10 // MIO 10

#define REG_BASE_ADDRESS XPAR_Z3660_0_BASEADDR

#define write_reg64(Offset,Data) (*(volatile uint64_t *)(REG_BASE_ADDRESS+(Offset)))=(Data)
#define write_reg(Offset,Data)   (*(volatile uint32_t *)(REG_BASE_ADDRESS+(Offset)))=(Data)
#define write_mem32(Offset,Data) (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x04000000+((Offset&0x00FFFFFF)<<2)))=(Data)
#define write_mem16(Offset,Data) (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x08000000+((Offset&0x00FFFFFF)<<2)))=(Data)
#define write_mem8(Offset,Data)  (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x0C000000+((Offset&0x00FFFFFF)<<2)))=(Data)
#define read_reg(Offset) (*(volatile uint32_t *)(REG_BASE_ADDRESS+(Offset)))

// Some amiga write and read definitions
#define WRITE_ (0<<1)
#define READ_  (1<<1)
#define LONG_  (0<<2)
#define BYTE_  (1<<2)
#define WORD_  (2<<2)
#define LINE_  (3<<2) // not used for now...

typedef struct {
	volatile uint32_t shared_data;     // 0xFFFF0000
	volatile uint32_t write_rtg;       // 0xFFFF0004
	volatile uint32_t write_rtg_addr;  // 0xFFFF0008
	volatile uint32_t write_rtg_data;  // 0xFFFF000C
	volatile uint32_t core0_hold;      // 0xFFFF0010
	volatile uint32_t core0_hold_ack;  // 0xFFFF0014
	volatile uint32_t irq;             // 0xFFFF0018
	volatile uint32_t uart_semaphore;  // 0xFFFF001C
	volatile uint32_t jit_enabled;     // 0xFFFF0020
	volatile uint32_t reset_emulator;  // 0xFFFF0024
	volatile uint32_t load_rom_emu;    // 0xFFFF0028
	volatile uint32_t load_rom_addr;   // 0xFFFF002C
	volatile uint32_t int_available;   // 0xFFFF0030
	volatile uint32_t cfg_emu;         // 0xFFFF0034
	volatile uint32_t write_scsi;      // 0xFFFF0038
	volatile uint32_t write_scsi_addr; // 0xFFFF003C
	volatile uint32_t write_scsi_data; // 0xFFFF0040
	volatile uint32_t write_scsi_type; // 0xFFFF0044
	volatile uint32_t read_scsi;       // 0xFFFF0048
	volatile uint32_t read_scsi_addr;  // 0xFFFF004C
	volatile uint32_t read_scsi_data;  // 0xFFFF0050
	volatile uint32_t read_scsi_type;  // 0xFFFF0054
	volatile uint32_t boot_rom_loaded; // 0xFFFF0058
	volatile uint32_t write_scsi_in_progress; // 0xFFFF005C
	volatile uint32_t read_rtg;        // 0xFFFF0060
	volatile uint32_t read_rtg_addr;   // 0xFFFF0064
	volatile uint32_t read_rtg_data;   // 0xFFFF0068
} SHARED;

enum BOOTMODE{
	CPU,
	MUSASHI,
	UAE_,
	UAEJIT,
	BOOTMODE_NUM
};

#endif /* SRC_MAIN_H_ */
