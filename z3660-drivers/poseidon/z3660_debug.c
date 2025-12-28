#include <exec/types.h>
#include <stdlib.h>
#include <string.h>

#include "z3660_debug.h"
#include "z3660_regs.h"

/* Cache coherency functions */
#include <proto/exec.h>
#include <clib/exec_protos.h>

/* Global flag to silence logs after fatal stop */
static volatile UBYTE g_log_silenced = 0;
static volatile ULONG *g_arm_debug_regs = NULL;  /* Pointer to Z3660 registers for ARM debug */
static volatile ULONG s_log_seq = 0;             /* Monotonic sequence counter */

/* Initialize ARM debug system */
void z3660_usb_debug_init_regs(volatile ULONG *z3660_regs)
{
    g_arm_debug_regs = z3660_regs;
    if (g_arm_debug_regs) {
        /* Send initialization marker to ARM */
        g_arm_debug_regs[REG_ZZ_DEBUG >> 2] = 0xDB600001;  /* Magic marker for ARM */
        kprintf("[ARM Debug] System initialized with crash-safe logging\n");
    }
}

/* Enhanced kprintf_good that sends debug to ARM for crash-safe logging */
void kprintf_good(const char *fmt, ...)
{
    if (g_log_silenced) {
        return;
    }
    
    static char message_buffer[256];  /* Static to avoid stack issues during crashes */
    va_list args;

    va_start(args, fmt);
    vsprintf(message_buffer, fmt, args);
    va_end(args);

    /* Add sequence counter for timing correlation */
    s_log_seq++;
    
    /* CRITICAL: Send debug message to ARM first (crash-safe) */
    if (g_arm_debug_regs) {
        /* Create a formatted message with sequence number */
        static char arm_message[280];  /* Static to avoid stack issues */
        sprintf(arm_message, "[AMG:%06lu] %s", s_log_seq, message_buffer);
        
        /* CACHE COHERENCY: Ensure data is visible to ARM before sending pointer */
        /* This cleans the CPU cache line containing our message to main memory */
        /* so the ARM can read the correct data from its cache-coherent view */
        {
            ULONG msg_len = strlen(arm_message) + 1;
            CachePreDMA((APTR)arm_message, &msg_len, 0);
        }
        
        /* Send string pointer to ARM via debug register */
        /* ARM will read the string from this address and output it */
        g_arm_debug_regs[REG_ZZ_DEBUG >> 2] = (ULONG)arm_message;
        
        /* Small delay to ensure ARM has time to read the string before we continue */
        volatile int arm_delay = 2000;  /* Adjusted for crash scenarios */
        while (arm_delay--) {
            __asm__ __volatile__("" ::: "memory");  /* Memory barrier */
        }
        
        /* CACHE COHERENCY: Restore cache state after ARM has read the data */
        /* This ensures the CPU cache is synchronized after the DMA-like operation */
        {
            ULONG msg_len2 = strlen(arm_message) + 1;
            CachePostDMA((APTR)arm_message, &msg_len2, 0);
        }
    }
    
    /* SECONDARY: Also send to Amiga console (may be lost in crash) */
    if(0)
    {
        static char amiga_message[320];  /* Static to avoid stack issues */
        sprintf(amiga_message, "[M:%06lu] %s", s_log_seq, message_buffer);
        kprintf(amiga_message);  /* Traditional Amiga output */
    }
}

void Z3660USB_SetLogSilence(BOOL on)
{
    g_log_silenced = on ? 1 : 0;
}
