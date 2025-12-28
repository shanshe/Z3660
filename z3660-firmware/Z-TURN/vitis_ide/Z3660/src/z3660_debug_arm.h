/*
 * Z3660 ARM Debug System - Header file
 * 
 * This system handles debug messages sent from the Amiga via the REG_ZZ_DEBUG register.
 * It provides crash-safe logging by relaying messages through the ARM CPU.
 */

#ifndef Z3660_DEBUG_ARM_H
#define Z3660_DEBUG_ARM_H

#include <stdint.h>

/* Debug system initialization status */
extern int z3660_debug_system_initialized;

/* Function prototypes */

/*
 * Initialize ARM debug system
 */
void z3660_debug_arm_init(void);

/*
 * Process debug message from Amiga
 * Called when REG_ZZ_DEBUG is written to
 */
void z3660_debug_arm_process_message(uint32_t debug_value);

/*
 * Handle debug system initialization from Amiga
 */
void z3660_debug_arm_init_request(void);

/*
 * Get debug system status
 */
uint32_t z3660_debug_arm_get_status(void);

/*
 * Check if debug system is ready
 */
int z3660_debug_arm_is_ready(void);

/*
 * Process periodic debug tasks (if needed)
 */
void z3660_debug_arm_periodic_task(void);

/* Debug message constants */
#define Z3660_DEBUG_MAGIC_INIT      0xDB600001  /* Initialization request */
#define Z3660_DEBUG_MAGIC_STRING    0xDB600002  /* String message follows */
#define Z3660_DEBUG_MAGIC_STATUS    0xDB600003  /* Status request */

/* Debug system status flags */
#define Z3660_DEBUG_STATUS_READY    0x0001
#define Z3660_DEBUG_STATUS_BUSY     0x0002
#define Z3660_DEBUG_STATUS_ERROR    0x0004
#define Z3660_DEBUG_STATUS_INIT     0x0008

#endif /* Z3660_DEBUG_ARM_H */
