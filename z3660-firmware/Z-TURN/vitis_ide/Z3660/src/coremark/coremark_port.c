#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "coremark.h"
#include "core_portme.h"
#include "xtime_l.h"

// Configuration for ARM Cortex-A9
ee_u32 default_num_contexts = 1;

// Global variable to capture CoreMark score
float captured_coremark_score = 0.0f;
int capturing_score = 0;
extern uint32_t counts_per_second;

// Timer implementation using Xilinx XTime_GetTime
static XTime start_timer_value = 0;

/* Timer functions implementation for Xilinx/ARM platform */

void start_time(void) {
    XTime_GetTime(&start_timer_value);
}

void stop_time(void) {
    // Timer stops - nothing to do here
}

CORE_TICKS get_time(void) {
    XTime current_time;
    XTime_GetTime(&current_time);
    
    // Calculate delta in XTime ticks (64-bit)
    ee_u64 delta_xtime = (ee_u64)current_time - (ee_u64)start_timer_value;
    
    // Scale to approximate milliseconds to avoid overflow in 32-bit CORE_TICKS
    // Convert XTime ticks to approximate milliseconds
    ee_u32 delta_ms = (ee_u32)(delta_xtime / (counts_per_second / 1000));
    
    return (CORE_TICKS)delta_ms;
}

secs_ret time_in_secs(CORE_TICKS ticks) {
    // Convert milliseconds back to seconds
    double seconds = (double)ticks / 1000.0;
    return (secs_ret)seconds;
}

// Provide memory management functions for MEM_MALLOC method
void *portable_malloc(ee_size_t size) {
    return malloc(size);
}

void portable_free(void *p) {
    free(p);
}

// Custom printf function that intercepts CoreMark score
int coremark_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    // First, pass through to the real printf
    int result = vprintf(format, args);
    
    // Parse the output to capture CoreMark score
    if (strstr(format, "CoreMark 1.0") != NULL) {
        // CoreMark is printing the final score
        // The format is: "CoreMark 1.0 : %f / %s %s"
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        
        // Parse the score from the buffer
        // Example: "CoreMark 1.0 : 1234.56 / GCC ARM -O3"
        char *score_start = strstr(buffer, "CoreMark 1.0 : ");
        if (score_start != NULL) {
            score_start += strlen("CoreMark 1.0 : ");
            char *score_end = strchr(score_start, ' ');
            if (score_end != NULL) {
                char score_str[32];
                strncpy(score_str, score_start, score_end - score_start);
                score_str[score_end - score_start] = '\0';
                captured_coremark_score = atof(score_str);
                capturing_score = 1; // Mark that we captured the score
            }
        }
    }
    
    va_end(args);
    return result;
}

// SEED_VOLATILE method implementations (used by default)
#if PERFORMANCE_RUN
volatile ee_s32 seed1_volatile = 0x0;
volatile ee_s32 seed2_volatile = 0x0;
volatile ee_s32 seed3_volatile = 0x66;
volatile ee_s32 seed4_volatile = 0; // iterations - will be auto-determined
volatile ee_s32 seed5_volatile = ALL_ALGORITHMS_MASK;
#elif VALIDATION_RUN
volatile ee_s32 seed1_volatile = 0x3415;
volatile ee_s32 seed2_volatile = 0x3415;
volatile ee_s32 seed3_volatile = 0x66;
volatile ee_s32 seed4_volatile = 0; // iterations - will be auto-determined
volatile ee_s32 seed5_volatile = ALL_ALGORITHMS_MASK;
#else // PROFILE_RUN
volatile ee_s32 seed1_volatile = 0x8;
volatile ee_s32 seed2_volatile = 0x8;
volatile ee_s32 seed3_volatile = 0x8;
volatile ee_s32 seed4_volatile = 0; // iterations - will be auto-determined
volatile ee_s32 seed5_volatile = ALL_ALGORITHMS_MASK;
#endif

ee_s32 get_seed_32(int i) {
    switch (i) {
        case 1: return seed1_volatile;
        case 2: return seed2_volatile;
        case 3: return seed3_volatile;
        case 4: return seed4_volatile;
        case 5: return seed5_volatile;
        default: return 0;
    }
}

/* Portable layer functions */
void portable_init(core_portable *p, int *argc, char *argv[]) {
    if (p == NULL)
        return;
    p->portable_id = 0;
}

void portable_fini(core_portable *p) {
    if (p == NULL)
        return;
}

/* Wrapper function to integrate CoreMark with our system */
int coremark_run(float *return_value) {
    int argc = 1;
    char *argv[] = { "coremark" };
    
    // Reset captured score
    captured_coremark_score = 0.0f;
    capturing_score = 0;
    
    // Call CoreMark main function (renamed from main to core_main via macro)
    int result = core_main(argc, argv);
    
    // Return the captured CoreMark score
    if (return_value && capturing_score) {
        *return_value = captured_coremark_score;
    } else if (return_value) {
        *return_value = 0.0f; // Return 0 if score wasn't captured
    }
    
    return result;
}