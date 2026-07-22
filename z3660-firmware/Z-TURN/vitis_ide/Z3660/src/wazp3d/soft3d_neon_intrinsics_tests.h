/*
 * soft3d_neon_intrinsics_tests.h
 * Test suite header for NEON intrinsics
 */

#ifndef SOFT3D_NEON_INTRINSICS_TESTS_H
#define SOFT3D_NEON_INTRINSICS_TESTS_H

#include <stdint.h>
#include <arm_neon.h>
#include <math.h>
#include <float.h>
#include <time.h>

/* Test configuration */
#define NEON_TEST_ITERATIONS 1000
#define NEON_TEST_SIZE      16
#define NEON_TEST_TOLERANCE 0.01f

/* Test result tracking */
typedef struct {
    uint32_t passed;
    uint32_t failed;
    uint32_t total;
    uint64_t total_time_ns;
} neon_test_result_t;

#endif /* SOFT3D_NEON_INTRINSICS_TESTS_H */
