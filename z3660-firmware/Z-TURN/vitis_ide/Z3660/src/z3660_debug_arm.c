/*
 * Z3660 ARM Debug System - Simple Implementation
 * 
 * This system handles debug messages sent from the Amiga via the REG_ZZ_DEBUG register.
 * The Amiga sends a pointer to a string, and the ARM reads and prints it.
 */

#include <stdio.h>
#include <string.h>
#include "z3660_debug_arm.h"
#include "xil_cache.h"

/* Global debug system state */
int z3660_debug_system_initialized = 0;
static uint32_t debug_sequence_counter = 0;

/*
 * Initialize ARM debug system
 */
void z3660_debug_arm_init(void)
{
    printf("[ARM Debug] Z3660 ARM debug system initialized\n");
    printf("[ARM Debug] Ready to receive debug string pointers from Amiga\n");
    z3660_debug_system_initialized = 1;
    debug_sequence_counter = 0;
}

/*
 * Process debug message from Amiga
 * Called when REG_ZZ_DEBUG is written to
 */
void z3660_debug_arm_process_message(uint32_t debug_value)
{
    // Special initialization value
    if (debug_value == 0xDB600001) {
        z3660_debug_arm_init();
        return;
    }
    
    if (!z3660_debug_system_initialized) {
        // Auto-initialize on first use
        z3660_debug_arm_init();
    }
    
    // debug_value is a pointer to a string in Amiga memory
    if (debug_value == 0 || debug_value < 0x08000000 || debug_value > 0x20000000) {
        printf("[ARM Debug] Invalid pointer: 0x%08lx\n", debug_value);
        return;
    }
//    printf("Debug pointer 0x%08lx\n", debug_value);
    
    // Read the string from Amiga memory
    // The ARM can directly access Amiga memory through the shared memory space
    char *amiga_string = (char *)debug_value;
    
    // CACHE COHERENCY: Invalidate cache lines for the memory region we're about to read
    // This ensures we read the latest data written by the Amiga
    // Invalidate up to 512 bytes (our max buffer size) to be safe
    //Xil_DCacheInvalidateRange((INTPTR)amiga_string, 512);
    
    // Safety check - limit string length to prevent crashes
    char safe_buffer[512];
    unsigned int i;
    for (i = 0; i < sizeof(safe_buffer) - 1; i++) {
        char c = amiga_string[i];
        if (c == '\0') break;
        // Only copy printable characters and newlines
        if ((c >= 0x20 && c <= 0x7E) || c == '\n' || c == '\r' || c == '\t') {
            safe_buffer[i] = c;
        } else {
            safe_buffer[i] = '?';  // Replace non-printable with '?'
        }
    }
    safe_buffer[i] = '\0';
    
    // Output with sequence number and ARM prefix
    debug_sequence_counter++;
    printf("[ARM Debug #%lu] %s", debug_sequence_counter, safe_buffer);
    
    // Add newline if the message doesn't end with one
    if (i > 0 && safe_buffer[i-1] != '\n') {
        printf("\n");
    }
}

/*
 * Handle debug system initialization from Amiga
 */
void z3660_debug_arm_init_request(void)
{
    z3660_debug_arm_init();
}

/*
 * Get debug system status
 */
uint32_t z3660_debug_arm_get_status(void)
{
    return z3660_debug_system_initialized ? 0x0001 : 0x0000;
}

/*
 * Check if debug system is ready
 */
int z3660_debug_arm_is_ready(void)
{
    return z3660_debug_system_initialized;
}

/*
 * Process periodic debug tasks (if needed)
 */
void z3660_debug_arm_periodic_task(void)
{
    // Nothing needed for simple string pointer system
}
