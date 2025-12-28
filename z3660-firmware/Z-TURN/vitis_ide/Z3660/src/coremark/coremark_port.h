#ifndef COREMARK_PORT_H
#define COREMARK_PORT_H

#include "coremark.h"

// Declaración de la función wrapper para nuestro sistema
int coremark_run(float *return_value);

// Estas funciones ahora están definidas en coremark_port.c
void start_time(void);
void stop_time(void);
CORE_TICKS get_time(void);
secs_ret time_in_secs(CORE_TICKS ticks);

// Memory management functions for MEM_MALLOC method
void *portable_malloc(ee_size_t size);
void portable_free(void *p);

// Seed function for SEED_VOLATILE method
ee_s32 get_seed_32(int i);

// Portable functions
void portable_init(core_portable *p, int *argc, char *argv[]);
void portable_fini(core_portable *p);

// Intercept printf to capture CoreMark score
extern float captured_coremark_score;
extern int coremark_printf(const char *format, ...);

#endif // COREMARK_PORT_H