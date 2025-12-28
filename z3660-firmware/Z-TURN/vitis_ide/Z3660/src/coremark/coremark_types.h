#ifndef COREMARK_TYPES_H
#define COREMARK_TYPES_H

#include <stdint.h>

// Type definitions for CoreMark (EEMBC)
typedef uint8_t   ee_u8;
typedef uint16_t  ee_u16;
typedef uint32_t  ee_u32;
typedef uint64_t  ee_u64;
typedef int8_t    ee_s8;
typedef int16_t   ee_s16;
typedef int32_t   ee_s32;
typedef int64_t   ee_s64;

// Floating point types
typedef float     ee_f32;
typedef double    ee_f64;

// Pointer types  
typedef ee_u32    ee_ptr_int;
typedef size_t    ee_size_t;

// Return types for main
typedef int MAIN_RETURN_TYPE;

// Performance metric types - use 64-bit for timing to avoid overflow
typedef ee_u64 CORE_TICKS;
typedef double secs_ret;

// Portable layer structure
typedef struct {
    ee_u8 portable_id;
} core_portable;

#endif // COREMARK_TYPES_H