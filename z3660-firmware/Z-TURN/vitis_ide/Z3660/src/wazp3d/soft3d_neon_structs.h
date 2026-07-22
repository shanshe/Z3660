/*==================================================================================*/
/* NEON-OPTIMIZED DATA STRUCTURES FOR CORTEX-A9 (Cortex-A9, NEON, 64B cache lines)
/* This header provides NEON-optimized union structures with contiguous fields
/* for optimal SIMD loading and processing
/*==================================================================================*/

#ifndef SOFT3D_NEON_STRUCTS_H
#define SOFT3D_NEON_STRUCTS_H

#include <arm_neon.h>
#include <stdint.h>

/* Architecture detection */
#ifndef __ARM_NEON__
#define __ARM_NEON__ 0
#else
#define __ARM_NEON__ 1
#endif

#if __ARM_NEON__

/*==================================================================================*/
/* NEON-OPTIMIZED UNION STRUCTURES
/* These structures use contiguous field layout for optimal NEON loading
/* Memory layout (64 bytes each):
/*
/* pixel3D_NEON:
/*   [0-15]:  z  (float32x4_t)  -> z, w, ?, ?
/*   [16-31]: coords (float32x4_t) -> u, v, x, y
/*   [32-47]:  rgb  (int32x4_t)  -> R, G, B, A
/*   [48-63]:  grad (int32x4_t)  -> du, dv, dx, dy
/*
/* point3D_NEON:
/*   [0-15]:  xyzw  (float32x4_t) -> x, y, z, w
/*   [16-31]: uvxy  (float32x4_t) -> u, v, x, y
/*   [32-47]:  rgba (int32x4_t)  -> R, G, B, A
/*   [48-63]:  grad (int32x4_t)  -> du, dv, dx, dy
/*==================================================================================*/

/* pixel3D_NEON union */
#pragma pack(push, 8)
union pixel3D_NEON {
    struct {
        float32x4_t z;      /* 4 floats = z, w, ?, ? */
        float32x4_t coords; /* u, v, x, y */
        int32x4_t rgb;      /* R, G, B, A */
        int32x4_t grad;     /* du, dv, dx, dy */
    } L;
    
    struct {
        float32x2_t z0, z1;
        float32x2_t u0, v0;
        float32x2_t x0, y0;
        int32x2_t R0, G0;
        int32x2_t B0, A0;
        int32x2_t du0, dv0;
        int32x2_t dx0, dy0;
        float32x2_t w0, pad;
        float32x2_t dw0, pad2;
    } packed;
    
    /* Scalar accessors */
    float z, w, u, v;
    int R, G, B, A, du, dv, dx, dy;
} __attribute__((aligned(16)));

/* point3D_NEON union */
#pragma pack(push, 8)
union point3D_NEON {
    struct {
        float32x4_t xyzw;   /* x, y, z, w */
        float32x4_t uvxy;   /* u, v, x, y */
        int32x4_t rgba;     /* R, G, B, A */
        int32x4_t grad;     /* du, dv, dx, dy */
    } L;
    
    struct {
        float32x2_t x0, y0;
        float32x2_t z0, w0;
        float32x2_t u0, v0;
        int32x2_t R0, G0;
        int32x2_t B0, A0;
        int32x2_t du0, dv0;
        int32x2_t pad0, pad1;
        float32x2_t dw0, pad2;
    } packed;
    
    /* Scalar accessors */
    float x, y, z, w;
    int R, G, B, A, du, dv, pad, dw;
} __attribute__((aligned(16)));

#pragma pack(pop)

/* Convenience macros for NEON operations */

/* Load 4 floats as z, w, ?, ? */
#define LOAD_PIXEL_Z(x) vld1q_f32(&x.z)
#define STORE_PIXEL_Z(x, v) vst1q_f32(&x.z, v)

/* Load coordinates u, v, x, y */
#define LOAD_PIXEL_COORDS(x) vld1q_f32(&x.u)
#define STORE_PIXEL_COORDS(x, v) vst1q_f32(&x.u, v)

/* Load RGBA */
#define LOAD_PIXEL_RGBA(x) vld1q_s32(&x.R)
#define STORE_PIXEL_RGBA(x, v) vst1q_s32(&x.R, v)

/* Load gradients */
#define LOAD_PIXEL_GRAD(x) vld1q_s32(&x.du)
#define STORE_PIXEL_GRAD(x, v) vst1q_s32(&x.du, v)

/* Load 4 floats as x, y, z, w */
#define LOAD_POINT_XYZW(x) vld1q_f32(&x.x)
#define STORE_POINT_XYZW(x, v) vst1q_f32(&x.x, v)

/* Load coordinates u, v, x, y */
#define LOAD_POINT_UVXY(x) vld1q_f32(&x.u)
#define STORE_POINT_UVXY(x, v) vst1q_f32(&x.u, v)

/* Load RGBA */
#define LOAD_POINT_RGBA(x) vld1q_s32(&x.R)
#define STORE_POINT_RGBA(x, v) vst1q_s32(&x.R, v)

/* Load gradients */
#define LOAD_POINT_GRAD(x) vld1q_s32(&x.du)
#define STORE_POINT_GRAD(x, v) vst1q_s32(&x.du, v)

/* NEON operation helpers — single-pixel RGBA (4 bytes) operations */

/* Multiply each component by factor, divide by 255 (round-to-nearest) */
static __inline__ void mul8_NEON(uint8_t *out, const uint8_t *src, uint8_t factor) {
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
static __inline__ void add8_NEON(uint8_t *out, const uint8_t *a, const uint8_t *b) {
    for(int i=0; i<4; i++) {
        uint16_t s = (uint16_t)a[i] + (uint16_t)b[i];
        out[i] = (uint8_t)(s > 255 ? 255 : s);
    }
}

/* Saturating subtract (clamp to 0) */
static __inline__ void sub8_NEON(uint8_t *out, const uint8_t *a, const uint8_t *b) {
    for(int i=0; i<4; i++) {
        out[i] = (uint8_t)(a[i] > b[i] ? a[i] - b[i] : 0);
    }
}

/* Blend RGBA with scalar alpha: out = fg*a + bg*(255-a) all /255 */
static __inline__ void blend_RGBA_NEON(uint8_t *out, const uint8_t *fg, const uint8_t *bg, uint8_t alpha) {
    uint8_t inv_a = 255 - alpha;
    for(int i=0; i<4; i++) {
        out[i] = (uint8_t)(((uint16_t)fg[i] * alpha + (uint16_t)bg[i] * inv_a + 127) / 255);
    }
}

/* Z-buffer comparison (returns true if new z is closer — smaller z = closer) */
static __inline__ uint32x4_t ztest_NEON(uint32x4_t new_z, uint32x4_t zbuf) {
    return vcltq_u32(new_z, zbuf);
}

/* Z-buffer update (write new_z where test passes; pass unused — mask recomputed) */
static __inline__ void zupdate_NEON(uint32x4_t *zbuf, uint32x4_t pass, uint32x4_t new_z) {
    (void)pass;
    uint32x4_t mask = vcltq_u32(new_z, *zbuf);
    uint32x4_t new_z_masked = vbslq_u32(mask, new_z, *zbuf);
    *zbuf = new_z_masked;
}

/* Texture fetch with alpha handling: premultiply by texel alpha */
static __inline__ void texfetch_NEON(uint8_t *color, const uint8_t *texel, uint8_t tex_alpha) {
    for(int i=0; i<4; i++) {
        color[i] = (uint8_t)(((uint16_t)texel[i] * tex_alpha + 127) / 255);
    }
}

#else

/* Fallback for non-NEON architectures */

/* pixel3D union */
#pragma pack(push, 8)
union pixel3D {
    struct pixel3DL{
        ZBUFF z;
        float w;
        LONG u;
        LONG v;
        LONG R;
        LONG G;
        LONG B;
        LONG A;
        LONG x;
        LONG y;
        LONG F;
        LONG large;
        UBYTE *Image8Y;
        UWORD bpp;
        ZBUFF *ZbufferY;
        ZBUFF dz;
        float dw;
        LONG du;
        LONG dv;
        LONG ddu;
        LONG ddv;
        LONG dR;
        LONG dG;
        LONG dB;
        LONG dA;
        LONG dx;
        LONG dF;
    }  L;
    
    struct pixel3DW{
        ZBUFF z;
        float w;
        UBYTE u4,u3,u,u1;
        UBYTE v4,v3,v,v1;
        UBYTE R4,R3,R,R1;
        UBYTE G4,G3,G,G1;
        UBYTE B4,B3,B,B1;
        UBYTE A4,A3,A,A1;
        WORD  xlow,x;
        WORD  ylow,y;
        WORD  Flow,F;
        WORD largelow,large;
        UBYTE *Image8Y;
        UWORD bpp;
        ZBUFF *ZbufferY;
        ZBUFF dz;
        float dw;
        LONG du;
        LONG dv;
        LONG ddu;
        LONG ddv;
        LONG dR;
        LONG dG;
        LONG dB;
        LONG dA;
        LONG dx;
        LONG dF;
    }  W;
} __attribute__((aligned(16)));

#pragma pack(pop)

/* point3D union */
#pragma pack(push, 8)
union point3D {
    struct {
        ULONG Index;
    } L;
    
    struct {
        UBYTE b,a,empty3,empty4;
    } B;
} __attribute__((aligned(16)));

#pragma pack(pop)

/* Fallback: scalar operations (no NEON types or intrinsics) */
#define LOAD_PIXEL_Z(x)        x.z  /* (void) — return the float itself */
#define STORE_PIXEL_Z(x, v)    do{ (x).z = vgetq_lane_f32(v,0); (x).w = vgetq_lane_f32(v,1); }while(0)
#define LOAD_PIXEL_COORDS(x)   x.u
#define STORE_PIXEL_COORDS(x, v) do{ (x).u = vgetq_lane_f32(v,0); (x).v = vgetq_lane_f32(v,1); (x).x = vgetq_lane_f32(v,2); (x).y = vgetq_lane_f32(v,3); }while(0)
#define LOAD_PIXEL_RGBA(x)     x.R
#define STORE_PIXEL_RGBA(x, v) do{ (x).R = vgetq_lane_s32(v,0); (x).G = vgetq_lane_s32(v,1); (x).B = vgetq_lane_s32(v,2); (x).A = vgetq_lane_s32(v,3); }while(0)
#define LOAD_PIXEL_GRAD(x)     x.du
#define STORE_PIXEL_GRAD(x, v) do{ (x).du = vgetq_lane_s32(v,0); (x).dv = vgetq_lane_s32(v,1); (x).dx = vgetq_lane_s32(v,2); (x).dy = vgetq_lane_s32(v,3); }while(0)
#define LOAD_POINT_XYZW(x)     x.x
#define STORE_POINT_XYZW(x, v) do{ (x).x = vgetq_lane_f32(v,0); (x).y = vgetq_lane_f32(v,1); (x).z = vgetq_lane_f32(v,2); (x).w = vgetq_lane_f32(v,3); }while(0)
#define LOAD_POINT_UVXY(x)     x.u
#define STORE_POINT_UVXY(x, v) do{ (x).u = vgetq_lane_f32(v,0); (x).v = vgetq_lane_f32(v,1); (x).x = vgetq_lane_f32(v,2); (x).y = vgetq_lane_f32(v,3); }while(0)
#define LOAD_POINT_RGBA(x)     x.R
#define STORE_POINT_RGBA(x, v) do{ (x).R = vgetq_lane_s32(v,0); (x).G = vgetq_lane_s32(v,1); (x).B = vgetq_lane_s32(v,2); (x).A = vgetq_lane_s32(v,3); }while(0)
#define LOAD_POINT_GRAD(x)     x.du
#define STORE_POINT_GRAD(x, v) do{ (x).du = vgetq_lane_s32(v,0); (x).dv = vgetq_lane_s32(v,1); }while(0)

/* Scalar pixel operations (matching the NEON function signatures above) */
#define mul8_NEON(out, src, factor) do{ \
    int i; \
    for(i=0;i<4;i++) (out)[i] = (uint8_t)(((uint16_t)(src)[i] * (uint16_t)(factor) + 127) / 255); \
}while(0)

#define add8_NEON(out, a, b) do{ \
    int i; \
    for(i=0;i<4;i++) { uint16_t _s = (uint16_t)(a)[i] + (uint16_t)(b)[i]; (out)[i] = (uint8_t)(_s > 255 ? 255 : _s); } \
}while(0)

#define sub8_NEON(out, a, b) do{ \
    int i; \
    for(i=0;i<4;i++) (out)[i] = (uint8_t)((a)[i] > (b)[i] ? (a)[i] - (b)[i] : 0); \
}while(0)

#define blend_RGBA_NEON(out, fg, bg, alpha) do{ \
    int i; uint8_t _inv = 255 - (uint8_t)(alpha); \
    for(i=0;i<4;i++) (out)[i] = (uint8_t)(((uint16_t)(fg)[i] * (uint8_t)(alpha) + (uint16_t)(bg)[i] * _inv + 127) / 255); \
}while(0)

/* Z-buffer operations use 32-bit compare — always use NEON intrinsics */
#define ztest_NEON(new_z, zbuf) vcltq_u32(new_z, zbuf)
#define zupdate_NEON(zbuf, pass, new_z) do{ \
    uint32x4_t _mask = vcltq_u32(new_z, *(zbuf)); \
    *(zbuf) = vbslq_u32(_mask, new_z, *(zbuf)); \
    (void)(pass); \
}while(0)

#define texfetch_NEON(color, texel, tex_alpha) do{ \
    int i; \
    for(i=0;i<4;i++) (color)[i] = (uint8_t)(((uint16_t)(texel)[i] * (uint8_t)(tex_alpha) + 127) / 255); \
}while(0)

#endif /* __ARM_NEON__ */

#endif /* SOFT3D_NEON_STRUCTS_H */
