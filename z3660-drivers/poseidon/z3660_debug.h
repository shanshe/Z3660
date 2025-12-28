#ifndef Z3660_DEBUG_H
#define Z3660_DEBUG_H

#include <exec/types.h>
#include <stdarg.h>

void z3660_usb_debug_init_regs(volatile ULONG *z3660_regs);
void kprintf_good(const char *fmt, ...);
/* Kernel printf for debugging */
extern void kprintf(const char *fmt, ...);

#endif /* Z3660_DEBUG_H */