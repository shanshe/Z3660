/*
Copyright 2018 Embedded Microprocessor Benchmark Consortium (EEMBC)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Original Author: Shay Gal-on
*/

#ifndef CORE_PORTME_H
#define CORE_PORTME_H

#include <stddef.h>
#include "coremark_types.h"

/* Configuration */
#define HAS_FLOAT 1
#define HAS_STDIO 1
#define HAS_PRINTF 0  // Prevent coremark.h from defining ee_printf
#define MEM_LOCATION "STATIC"
#define MULTITHREAD 1
#define COMPILER_VERSION "GCC ARM"
#define COMPILER_FLAGS "-O3"

/* align_mem macro */
#define align_mem(x) (void *)(4 + (((ee_ptr_int)(x)-1) & ~3))

/* Rename main to avoid conflict with system main() */
#define main core_main

/* Intercept ee_printf to capture CoreMark score */
#define ee_printf coremark_printf
extern int coremark_printf(const char *format, ...);

/* Declare core_main function */
#if MAIN_HAS_NOARGC
int core_main(void);
#else
int core_main(int argc, char *argv[]);
#endif

/* Variable : default_num_contexts */
extern ee_u32 default_num_contexts;

/* Target specific init/fini */
void portable_init(core_portable *p, int *argc, char *argv[]);
void portable_fini(core_portable *p);

#endif /* CORE_PORTME_H */