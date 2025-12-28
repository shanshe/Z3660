/*
 * pl_mpeg_handler.h - PL_MPEG ARM Handler Interface for Z3660
 * 
 * Header file for ARM-side PL_MPEG decoder handler functions
 */

#ifndef PL_MPEG_HANDLER_H
#define PL_MPEG_HANDLER_H

#include "mpg/libmpeg_arm/mpeg2dec_arm_comm_arm.h"

/* 
 * Initialize PL_MPEG decoder on ARM side
 * Returns 0 on success, -1 on failure
 */
int pl_mpeg_arm_init(void);

/* 
 * Process MPEG data using PL_MPEG decoder
 * input_data: pointer to MPEG data buffer
 * input_length: length of MPEG data in bytes  
 * sync_time: synchronization time for frame display
 * Returns 0 on success, -1 on failure
 */
int pl_mpeg_arm_process(uint8_t *input_data, uint32_t input_length, double sync_time);

/* Reset PL_MPEG decoder to initial state */
void pl_mpeg_arm_reset(void);

/* Close and clean up PL_MPEG decoder resources */
void pl_mpeg_arm_close(void);

/* Set shared memory pointer for ARM-68k communication */
void pl_mpeg_set_shared_memory(arm_decoder_shared_arm_t *shared);

/* Get PL_MPEG library version information */
const char* pl_mpeg_arm_get_version(void);

#endif // PL_MPEG_HANDLER_H