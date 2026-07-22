/*==================================================================================*/
/* NEON HOT PATH INTRINSICS FOR CORTEX-A9
/* Optimized intrinsics for the most frequently called rendering operations
/*
/* Architecture: ARMv7-A with NEON (Cortex-A9)
/* Cache: 32KB L1 data, 256KB shared L2
/*
/*==================================================================================*/

#ifndef SOFT3D_NEON_INTRINSICS_H
#define SOFT3D_NEON_INTRINSICS_H

#include <stdint.h>
#include <arm_neon.h>

/* Architecture detection */
#ifndef __ARM_NEON__
#define __ARM_NEON__ 0
#else
#define __ARM_NEON__ 1
#endif

/*==================================================================================*/
/* RGBA OPERATIONS INTRINSICS
/* These operations are called thousands of times per frame
/* All work on single 4-byte RGBA pixels via uint8_t pointers.
/*==================================================================================*/

/* Multiply each RGBA component by factor, divide by 255 (round-to-nearest) */
static __inline__ void mul_rgba_scalar_NEON(
    uint8_t *out,
    const uint8_t *src,
    uint8_t factor
) {
    uint16_t p0 = (uint16_t)src[0] * factor;
    uint16_t p1 = (uint16_t)src[1] * factor;
    uint16_t p2 = (uint16_t)src[2] * factor;
    uint16_t p3 = (uint16_t)src[3] * factor;
    out[0] = (uint8_t)((p0 + 127) / 255);
    out[1] = (uint8_t)((p1 + 127) / 255);
    out[2] = (uint8_t)((p2 + 127) / 255);
    out[3] = (uint8_t)((p3 + 127) / 255);
}

/* Saturating add (clamp to 255) */
static __inline__ void add_rgba_NEON(
    uint8_t *out,
    const uint8_t *a,
    const uint8_t *b
) {
    for(int i=0; i<4; i++) {
        uint16_t s = (uint16_t)a[i] + (uint16_t)b[i];
        out[i] = (uint8_t)(s > 255 ? 255 : s);
    }
}

/* Alpha blend: out = fg*alpha + bg*(255-alpha) all /255 */
static __inline__ void blend_rgba_NEON(
    uint8_t *out,
    const uint8_t *fg,
    const uint8_t *bg,
    uint8_t alpha
) {
    uint8_t inv_a = 255 - alpha;
    for(int i=0; i<4; i++) {
        out[i] = (uint8_t)(((uint16_t)fg[i] * alpha + (uint16_t)bg[i] * inv_a + 127) / 255);
    }
}

/* Premultiply RGBA by alpha (used for texture filtering) */
static __inline__ void premul_rgba_NEON(
    uint8_t *out,
    const uint8_t *rgba
) {
    uint8_t a = rgba[3];
    for(int i=0; i<4; i++) {
        out[i] = (uint8_t)(((uint16_t)rgba[i] * a + 127) / 255);
    }
}

/* Set RGBA to constant values */
static __inline__ void set_rgba_NEON(
    uint8_t *out,
    uint8_t r,
    uint8_t g,
    uint8_t b,
    uint8_t a
) {
#if __ARM_NEON__
    uint8x8_t val = vdup_n_u8(r);
    val = vset_lane_u8(g, val, 1);
    val = vset_lane_u8(b, val, 2);
    val = vset_lane_u8(a, val, 3);
    vst1_lane_u32((uint32_t*)out, vreinterpret_u32_u8(val), 0);
#else
    out[0] = r; out[1] = g; out[2] = b; out[3] = a;
#endif
}

/*==================================================================================*/
/* Z-BUFFER OPERATIONS INTRINSICS
/* Called every time a pixel is covered by a new fragment
/*==================================================================================*/

/* Compare new Z with Z-buffer (returns 1 if new is closer) */
static __inline__ uint32x4_t ztest_NEON(
    uint32x4_t new_z,
    uint32x4_t zbuf
)
{
#if __ARM_NEON__
    /* new_z < zbuf means new is closer (assuming smaller = closer) */
    return vcltq_u32(new_z, zbuf);
#else
    return vcltq_u32(new_z, zbuf);
#endif
}

/* Update Z-buffer if test passes */
static __inline__ void zupdate_NEON(
    uint32x4_t *zbuf,
    uint32x4_t pass,
    uint32x4_t new_z
)
{
#if __ARM_NEON__
    uint32x4_t zbuf_val = *zbuf;
    uint32x4_t new_z_masked = vbslq_u32(pass, new_z, zbuf_val);
    *zbuf = new_z_masked;
#else
    uint32x4_t zbuf_val = *zbuf;
    *zbuf = vbslq_u32(pass, new_z, zbuf_val);
#endif
}

/* Write Z-buffer with alpha test — scalar alpha, single zbuf entry */
static __inline__ void zwrite_alpha_NEON(
    uint32_t *zbuf,
    uint8_t alpha,
    uint32_t new_z
)
{
    if(alpha > 0) *zbuf = new_z;
}

/*==================================================================================*/
/* TEXTURE FETCH INTRINSICS
/* Called for every texel fetched during rendering
/*==================================================================================*/

/* Fetch texel and premultiply by texel alpha */
static __inline__ void texfetch_NEON(
    uint8_t *color,
    const uint8_t *texel,
    uint8_t tex_alpha
)
{
    for(int i=0; i<4; i++) {
        color[i] = (uint8_t)(((uint16_t)texel[i] * tex_alpha + 127) / 255);
    }
}

/* Fetch texel with alpha blending into background */
static __inline__ void texfetch_blend_NEON(
    uint8_t *out,
    const uint8_t *texel,
    uint8_t tex_alpha,
    const uint8_t *bg
)
{
    uint8_t inv_a = 255 - tex_alpha;
    for(int i=0; i<4; i++) {
        out[i] = (uint8_t)(((uint16_t)texel[i] * tex_alpha + (uint16_t)bg[i] * inv_a + 127) / 255);
    }
}

/* Fetch with alpha test (alpha > 0) */
static __inline__ void texfetch_alpha_test_NEON(
    uint8_t *color,
    const uint8_t *texel,
    uint8_t tex_alpha,
    uint32x4_t alpha_threshold
)
{
    (void)alpha_threshold;
    for(int i=0; i<4; i++) {
        color[i] = tex_alpha > 0
            ? (uint8_t)(((uint16_t)texel[i] * tex_alpha + 127) / 255)
            : 0;
    }
}

/*==================================================================================*/
/* GRADIENT OPERATIONS INTRINSICS
/* Used for Gouraud shading and perspective correction
/*==================================================================================*/

/* Calculate perspective correction factor */
static __inline__ float32x4_t persp_factor_NEON(
    float32x4_t w
)
{
#if __ARM_NEON__
    return vrecpeq_f32(w);
#else
    return vrecpeq_f32(w);
#endif
}

/* Multiply gradients by perspective factor */
static __inline__ void apply_perspective_NEON(
    float32x4_t *grads,
    float32x4_t factor,
    float32x4_t current_grads
)
{
    *grads = vmulq_f32(current_grads, factor);
}

/* Linear interpolation of gradients */
static __inline__ void lerp_gradients_NEON(
    float32x4_t *out,
    float32x4_t start_grads,
    float32x4_t end_grads,
    float32x4_t t
)
{
    float32x4_t one_minus_t = vsubq_f32(vdupq_n_f32(1.0f), t);
    float32x4_t start_scaled = vmulq_f32(start_grads, one_minus_t);
    float32x4_t end_scaled = vmulq_f32(end_grads, t);
    *out = vaddq_f32(start_scaled, end_scaled);
}

/*==================================================================================*/
/* MEMORY OPERATIONS INTRINSICS
/* Optimized for Cortex-A9's memory hierarchy
/*==================================================================================*/

/* Store 4 floats with NEON (4 cycles vs 8 cycles for scalar) */
static __inline__ void store_float4_NEON(
    float *dest,
    float32x4_t data
)
{
#if __ARM_NEON__
    vst1q_f32(dest, data);
#else
    *dest = vgetq_lane_f32(data, 0);
    dest[1] = vgetq_lane_f32(data, 1);
    dest[2] = vgetq_lane_f32(data, 2);
    dest[3] = vgetq_lane_f32(data, 3);
#endif
}

/* Load 4 floats with NEON (prefetch hint) */
static __inline__ float32x4_t load_float4_NEON(
    const float *src
)
{
#if __ARM_NEON__
    return vld1q_f32(src);
#else
    return vld1q_f32(src);
#endif
}

/* Zero 4 floats */
static __inline__ void zero_float4_NEON(
    float *dest
)
{
#if __ARM_NEON__
    float32x4_t zero = vdupq_n_f32(0.0f);
    vst1q_f32(dest, zero);
#else
    dest[0] = dest[1] = dest[2] = dest[3] = 0.0f;
#endif
}

/* Set 4 floats to same value */
static __inline__ void set_float4_NEON(
    float *dest,
    float value
)
{
#if __ARM_NEON__
    float32x4_t val = vdupq_n_f32(value);
    vst1q_f32(dest, val);
#else
    dest[0] = dest[1] = dest[2] = dest[3] = value;
#endif
}

/*==================================================================================*/
/* UTILITY INTRINSICS
/*==================================================================================*/

/* Check if NEON is available */
static __inline__ int neon_available(void)
{
#if __ARM_NEON__
    return 1;
#else
    return 0;
#endif
}

/* Get NEON version (0 if unavailable or unknown) */
static __inline__ int neon_version(void)
{
#if __ARM_NEON__ && defined(__ARM_NEON_VERSION__)
    return __ARM_NEON_VERSION__;
#else
    return 0;
#endif
}

#endif /* SOFT3D_NEON_INTRINSICS_H */
