/* Wazp3D Beta 56 — SIMD abstraction layer for soft3dsimd56.c			*/
/* Supports ARM NEON (v6/v7), x86 SSE2 (x86-64 baseline), x86 MMX		*/
/* Each optimization can be disabled at compile time				*/
/*									*/
/* Recommended Cortex‑A9 build flags:						*/
/*   -mcpu=cortex-a9 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard		*/
/*   -O3 -funroll-loops -ftree-vectorize					*/
/*   -D__ARM_NEON__ -DHAVE_CONFIG_H						*/
/*									*/
/* LICENSE: GNU General Public License (GNU GPL) for this file			*/

#ifndef SOFT3D_SIMD_H
#define SOFT3D_SIMD_H

#include <string.h>

/*==========================================================================*/
/* Per-optimization compile-time switches (set to 1 to enable, 0 to disable)*/
/* All default to 1 (enabled).  Override via -DOPT_XXX_SIMD=0 on the gcc   */
/* command line.                                                             */

#ifndef OPT_COPYRGBA_SIMD
#define OPT_COPYRGBA_SIMD 0
#endif
#ifndef OPT_MUL8_SIMD
#define OPT_MUL8_SIMD 0
#endif
#ifndef OPT_FULLADDMUL8_SIMD
#define OPT_FULLADDMUL8_SIMD 0
#endif
#ifndef OPT_ADD8_SIMD
#define OPT_ADD8_SIMD 0
#endif
#ifndef OPT_SUB8_SIMD
#define OPT_SUB8_SIMD 0
#endif
#ifndef OPT_FIL8_SIMD
#define OPT_FIL8_SIMD 0
#endif
#ifndef OPT_BYTE_SHUFFLE_SIMD
#define OPT_BYTE_SHUFFLE_SIMD 0
#endif
#ifndef OPT_CONVERTER_SIMD
#define OPT_CONVERTER_SIMD 0
#endif
#ifndef OPT_CLEAR_SIMD
#define OPT_CLEAR_SIMD 0
#endif
#ifndef OPT_ZTEST_SIMD
#define OPT_ZTEST_SIMD 0
#endif
#ifndef OPT_FILL_SIMD
#define OPT_FILL_SIMD 0
#endif
#ifndef OPT_ANTIALIAS_SIMD
#define OPT_ANTIALIAS_SIMD 0
#endif
#ifndef OPT_REDUCE_SIMD
#define OPT_REDUCE_SIMD 0
#endif
#ifndef OPT_TILING
#define OPT_TILING 0
#endif

/* Render mode constants — used when OPT_TILING is enabled */
#ifndef RENDER_SCANLINE
#define RENDER_SCANLINE  0
#endif
#ifndef RENDER_TILE_8x8
#define RENDER_TILE_8x8  1
#endif
#ifndef RENDER_TILE_SIMD
#define RENDER_TILE_SIMD 2
#endif

/*==========================================================================*/
/* CPU detection   —   included headers & feature flags			*/

#if defined(__ARM_NEON__)
#  include <arm_neon.h>
#  include <stdint.h>
#  define HAVE_SIMD 1
#  define HAVE_NEON 1
#elif defined(__SSE2__)
#  include <emmintrin.h>
#  include <stdint.h>
#  define HAVE_SIMD 1
#  define HAVE_SSE2 1
#elif defined(__MMX__)
#  include <mmintrin.h>
#  include <stdint.h>
#  define HAVE_SIMD 1
#  define HAVE_MMX 1
#endif

/*==========================================================================*/
/* 4-byte RGBA load/store — memcpy for strict-aliasing safety		*/

static inline __attribute__((always_inline)) uint32_t simd_load4(const void *p) {
    uint32_t v; memcpy(&v, p, sizeof(v)); return v;
}
static inline __attribute__((always_inline)) void simd_store4(void *p, uint32_t v) {
    memcpy(p, &v, sizeof(v));
}

#define SIMD_LOAD4_PTR(ptr)     simd_load4(ptr)
#define SIMD_LOAD4_INLINE(ptr)  simd_load4(&(ptr))
#define SIMD_STORE4(ptr, val)   simd_store4(&(ptr), val)

/*==========================================================================*/
/* NEON intrinsics								*/
/* (all use unsigned saturating arithmetic matching soft3d's MUL8/ADD8/…)	*/

#ifdef HAVE_NEON

/* Byte-permutation vectors for pixel format conversion (2-pixel batches) */
static const uint8x8_t NEON_PERM_RGBA_TO_BGRA = {2,1,0,3, 6,5,4,7};
static const uint8x8_t NEON_PERM_RGBA_TO_ARGB = {3,0,1,2, 7,4,5,6};
static const uint8x8_t NEON_PERM_RGBA_TO_ABGR = {3,2,1,0, 7,6,5,4};

/* Prefetch 4 fragments ahead (next batch) for read/write */
#define PLD_NEXT_FRAG(fptr, off)  __builtin_prefetch(&(fptr)[off], 0, 3)
#define PLD_NEXT_FRAGW(fptr, off) __builtin_prefetch(&(fptr)[off], 1, 3)
#define PLD_NEXT_TEX(fptr, off)   do { if((fptr)[off].Tex8) __builtin_prefetch((fptr)[off].Tex8, 0, 3); } while(0)

/* Pack two uint32_t values into a uint8x8_t (lower = frag0, upper = frag1)*/
#define SIMD_PACK8(lower, upper) \
	vcreate_u8(((uint64_t)(upper) << 32) | (uint64_t)(lower))

/* Unpack a uint8x8_t result into two uint32_t scalars for store		*/
#define SIMD_UNPACK8(reg8, lower_out, upper_out) do {			\
	uint64_t _r64 = vget_lane_u64(vreinterpret_u64_u8(reg8), 0);	\
	*(lower_out) = (uint32_t)(_r64 & 0xFFFFFFFFULL);		\
	*(upper_out) = (uint32_t)(_r64 >> 32);				\
} while(0)

/* Exact division of 16-bit value by 255: (p * 257 + 255) >> 16		*/
#define SIMD_DIV255_16x8(prod16, result16) do {				\
	uint16x8_t _p257 = vaddq_u16(vshlq_n_u16((prod16), 8), (prod16));\
	uint16x8_t _rnd  = vaddq_u16(_p257, vdupq_n_u16(255));		\
	(result16) = vshrq_n_u16(_rnd, 16);				\
} while(0)

/* Saturating add/sub (UBYTE): clamp to 0..255				*/
#define SIMD_ADD8_8x8(a8, b8, dst8)  ((dst8) = vqadd_u8((a8), (b8)))
#define SIMD_SUB8_8x8(a8, b8, dst8)  ((dst8) = vqsub_u8((a8), (b8)))

/* 16-WIDE NEON OPERATIONS — Phase 2: Widen Pixel Pipeline */

/* 16-wide saturating add/sub (UBYTE): clamp to 0..255 */
#define SIMD_ADD8_16x16(a8, b8, dst8)  ((dst8) = vqaddq_u8((a8), (b8)))
#define SIMD_SUB8_16x16(a8, b8, dst8)  ((dst8) = vqsubq_u8((a8), (b8)))

/* Exact division of 32-bit value by 255: (p * 257 + 255) >> 16 */
#define SIMD_DIV255_32x16(prod32, result32) do {	uint32x4_t _p257 = vaddq_u32(vshlq_n_u32((prod32), 8), (prod32));	uint32x4_t _rnd  = vaddq_u32(_p257, vdupq_n_u32(255));	(result32) = vshrq_n_u32(_rnd, 16);} while(0)

/* forward declaration for 8-wide mul8 — defined below */
static inline __attribute__((always_inline)) uint8x8_t simd_mul8_u8x8(uint8x8_t a, uint8x8_t b);

/* 16-wide channel multiply with exact /255 */
static inline __attribute__((always_inline)) uint8x16_t simd_mul8_u8x16(uint8x16_t a, uint8x16_t b)
{
	uint8x8_t a_lo = vget_low_u8(a);
	uint8x8_t a_hi = vget_high_u8(a);
	uint8x8_t b_lo = vget_low_u8(b);
	uint8x8_t b_hi = vget_high_u8(b);
	uint8x8_t r_lo = simd_mul8_u8x8(a_lo, b_lo);
	uint8x8_t r_hi = simd_mul8_u8x8(a_hi, b_hi);
	return vcombine_u8(r_lo, r_hi);
}

/* 16-wide FULLADDMUL8: (a * f1 + b * f2) / 255 */
static inline __attribute__((always_inline)) uint8x16_t simd_fulladdmul8_u8x16(uint8x16_t a, uint8x16_t f1,
	                                              uint8x16_t b, uint8x16_t f2)
{
	uint8x16_t ma = simd_mul8_u8x16(a, f1);
	uint8x16_t mb = simd_mul8_u8x16(b, f2);
	return vqaddq_u8(ma, mb);
}

/* 16-wide saturating add/sub */
static inline __attribute__((always_inline)) uint8x16_t simd_add8_u8x16(uint8x16_t a, uint8x16_t b)
{
	return vqaddq_u8(a, b);
}

static inline __attribute__((always_inline)) uint8x16_t simd_sub8_u8x16(uint8x16_t a, uint8x16_t b)
{
	return vqsubq_u8(a, b);
}


/* Rounding halving add (filter): (a + b + 1) >> 1			*/
#define SIMD_FIL8_8x8(a8, b8, dst8)  ((dst8) = vrhadd_u8((a8), (b8)))

/* 8-wide channel multiply with exact /255				*/
static inline __attribute__((always_inline)) uint8x8_t simd_mul8_u8x8(uint8x8_t a, uint8x8_t b)
{
	uint16x8_t a16 = vmovl_u8(a);
	uint16x8_t b16 = vmovl_u8(b);
	uint16x8_t p16 = vmulq_u16(a16, b16);
	uint32x4_t p32_lo = vmovl_u16(vget_low_u16(p16));
	uint32x4_t p32_hi = vmovl_u16(vget_high_u16(p16));
	p32_lo = vaddq_u32(vshlq_n_u32(p32_lo, 8), p32_lo);
	p32_hi = vaddq_u32(vshlq_n_u32(p32_hi, 8), p32_hi);
	p32_lo = vaddq_u32(p32_lo, vdupq_n_u32(255));
	p32_hi = vaddq_u32(p32_hi, vdupq_n_u32(255));
	uint16x4_t r16_lo = vshrn_n_u32(p32_lo, 16);
	uint16x4_t r16_hi = vshrn_n_u32(p32_hi, 16);
	uint16x8_t r16 = vcombine_u16(r16_lo, r16_hi);
	return vqmovn_u16(r16);
}

/* 8-wide FULLADDMUL8: (a * f1 + b * f2) / 255				*/
static inline __attribute__((always_inline)) uint8x8_t simd_fulladdmul8_u8x8(uint8x8_t a, uint8x8_t f1,
					       uint8x8_t b, uint8x8_t f2)
{
	uint8x8_t ma = simd_mul8_u8x8(a, f1);
	uint8x8_t mb = simd_mul8_u8x8(b, f2);
	return vqadd_u8(ma, mb);
}

/* Batch-clear 4 ULONGs (16 bytes) with a 32-bit value (aligned OK)	*/
#define SIMD_CLEAR4_ULONG(ptr, val32) do {				\
	uint32x4_t _v = vdupq_n_u32((uint32_t)(val32));			\
	vst1q_u32((uint32_t*)(ptr), _v);				\
} while(0)

/* Batch-clear 4 floats (16 bytes) with a float value (aligned OK)	*/
#define SIMD_CLEAR4_FLOAT(ptr, valf) do {				\
	float32x4_t _v = vdupq_n_f32((float)(valf));			\
	vst1q_f32((float*)(ptr), _v);					\
} while(0)

/* Float comparison → 4-bit mask (0..15) helpers for Z-test		*/
static inline int simd_mask4_u32(uint32x4_t cmp) {
	uint32_t r = vgetq_lane_u32(cmp, 0) >> 31;
	r |= (vgetq_lane_u32(cmp, 1) >> 31) << 1;
	r |= (vgetq_lane_u32(cmp, 2) >> 31) << 2;
	r |= (vgetq_lane_u32(cmp, 3) >> 31) << 3;
	return r & 0xF;
}

#define SIMD_CMPLT_F32(zv, zb)  vcltq_f32((zv), (zb))
#define SIMD_CMPLE_F32(zv, zb)  vcleq_f32((zv), (zb))
#define SIMD_CMPGT_F32(zv, zb)  vcgtq_f32((zv), (zb))
#define SIMD_CMPGE_F32(zv, zb)  vcgeq_f32((zv), (zb))
#define SIMD_CMPEQ_F32(zv, zb)  vceqq_f32((zv), (zb))
#define SIMD_CMPNEQ_F32(zv, zb) vmvnq_u32(vceqq_f32((zv), (zb)))

#define SIMD_CMPLT_F32_MASK(zv, zb)  simd_mask4_u32(SIMD_CMPLT_F32((zv), (zb)))
#define SIMD_CMPLE_F32_MASK(zv, zb)  simd_mask4_u32(SIMD_CMPLE_F32((zv), (zb)))
#define SIMD_CMPGT_F32_MASK(zv, zb)  simd_mask4_u32(SIMD_CMPGT_F32((zv), (zb)))
#define SIMD_CMPGE_F32_MASK(zv, zb)  simd_mask4_u32(SIMD_CMPGE_F32((zv), (zb)))
#define SIMD_CMPEQ_F32_MASK(zv, zb)  simd_mask4_u32(SIMD_CMPEQ_F32((zv), (zb)))
#define SIMD_CMPNEQ_F32_MASK(zv, zb) simd_mask4_u32(SIMD_CMPNEQ_F32((zv), (zb)))

/* Conditional blend: if cmp_lane != 0 choose newval else oldval	*/
#define SIMD_BLENDV_F32(cmp, newval, oldval) vbslq_f32((cmp), (newval), (oldval))

/* Unaligned float load/store (4 floats)				*/
#define SIMD_LOADU_F32(ptr)  vld1q_f32((float*)(ptr))
#define SIMD_STOREU_F32(ptr, val) vst1q_f32((float*)(ptr), (val))

/* Z-test NEON helpers — build z-vector, store packed Ztest, conditional update */

/* Build {z, z+dz, z+2*dz, z+3*dz} in NEON registers (no stack spill)	*/
static inline __attribute__((always_inline))
float32x4_t neon_zvec(float z, float dz)
{
    static const float neon_idx4[4] = {0.0f, 1.0f, 2.0f, 3.0f};
    float32x4_t idx = vld1q_f32(neon_idx4);
    return vmlaq_f32(vdupq_n_f32(z), vdupq_n_f32(dz), idx);
}

/* Pack 4 comparison sign-bits into Ztest[0..3] as 0/1 bytes		*/
static inline __attribute__((always_inline))
void neon_ztest_store(uint8_t *ztest, uint32x4_t cmp)
{
    uint32x4_t b = vshrq_n_u32(cmp, 31);
    uint16x4_t lo = vmovn_u32(b);
    uint8x8_t  p = vmovn_u16(vcombine_u16(lo, vdup_n_u16(0)));
    vst1_lane_u32((uint32_t*)ztest, vreinterpret_u32_u8(p), 0);
}

/* Conditional Zbuffer update + packed Ztest store			*/
static inline __attribute__((always_inline))
void neon_zupdate_store(uint8_t *ztest, float *zbuf, float32x4_t zvec, uint32x4_t cmp)
{
    float32x4_t old = vld1q_f32(zbuf);
    float32x4_t new = vbslq_f32(cmp, zvec, old);
    vst1q_f32(zbuf, new);
    neon_ztest_store(ztest, cmp);
}

/* Prefetch Zbuffer ahead						*/
#define PLD_NEXT_ZBUF(zptr, off) __builtin_prefetch(&(zptr)[off], 1, 3)

/*==========================================================================*/
/* SSE2/SSSE3 intrinsics							*/

#elif defined(HAVE_SSE2)

#  if defined(__SSSE3__)
#    include <tmmintrin.h>
#    define HAVE_SSSE3 1
#  endif

/* Pack two 32-bit scalars into __m128i (lower 64 bits used)		*/
#define SIMD_PACK8(lower, upper)					\
	_mm_unpacklo_epi32(_mm_cvtsi32_si128((int)(lower)),		\
			   _mm_cvtsi32_si128((int)(upper)))

/* Unpack lower/upper 32-bit lanes from __m128i				*/
#define SIMD_UNPACK8(reg, lower_out, upper_out) do {			\
	__m128i _r = (reg);						\
	*(lower_out) = (uint32_t)_mm_cvtsi128_si32(_r);			\
	*(upper_out) = (uint32_t)_mm_cvtsi128_si32(			\
			_mm_shuffle_epi32(_r, 0x55));			\
} while(0)

/* Exact /255 on 8 lanes of uint16					*/
#define SIMD_DIV255_16x8(prod16, result16) do {				\
	__m128i _p257 = _mm_add_epi16(_mm_slli_epi16((prod16), 8),	\
				      (prod16));				\
	__m128i _rnd  = _mm_add_epi16(_p257, _mm_set1_epi16(255));	\
	(result16) = _mm_srli_epi16(_rnd, 16);				\
} while(0)

/* Saturating add/sub on 16 uint8 lanes					*/
#define SIMD_ADD8_8x8(a8, b8, dst8)  ((dst8) = _mm_adds_epu8((a8), (b8)))
#define SIMD_SUB8_8x8(a8, b8, dst8)  ((dst8) = _mm_subs_epu8((a8), (b8)))

/* Average (a+b+1)>>1 on 16 uint8 lanes					*/
#define SIMD_FIL8_8x8(a8, b8, dst8)  ((dst8) = _mm_avg_epu8((a8), (b8)))

/* 8-wide channel multiply with exact /255 (uses lower 8 of the 16 reg) */
static inline __m128i simd_mul8_u8x8(__m128i a, __m128i b)
{
	__m128i a16 = _mm_unpacklo_epi8(a, _mm_setzero_si128());
	__m128i b16 = _mm_unpacklo_epi8(b, _mm_setzero_si128());
	__m128i p16 = _mm_mullo_epi16(a16, b16);
	__m128i p32_lo = _mm_unpacklo_epi16(p16, _mm_setzero_si128());
	__m128i p32_hi = _mm_unpackhi_epi16(p16, _mm_setzero_si128());
	p32_lo = _mm_add_epi32(_mm_slli_epi32(p32_lo, 8), p32_lo);
	p32_hi = _mm_add_epi32(_mm_slli_epi32(p32_hi, 8), p32_hi);
	p32_lo = _mm_add_epi32(p32_lo, _mm_set1_epi32(255));
	p32_hi = _mm_add_epi32(p32_hi, _mm_set1_epi32(255));
	__m128i r16 = _mm_packs_epi32(_mm_srli_epi32(p32_lo, 16), _mm_srli_epi32(p32_hi, 16));
	return _mm_packus_epi16(r16, _mm_setzero_si128());
}

/* 8-wide FULLADDMUL8: (a * f1 + b * f2) / 255				*/
static inline __m128i simd_fulladdmul8_u8x8(__m128i a, __m128i f1,
					    __m128i b, __m128i f2)
{
	__m128i ma = simd_mul8_u8x8(a, f1);
	__m128i mb = simd_mul8_u8x8(b, f2);
	return _mm_adds_epu8(ma, mb);
}

/* Batch-clear 4 ULONGs (16 bytes) with a 32-bit value (unaligned OK)	*/
#define SIMD_CLEAR4_ULONG(ptr, val32) do {				\
	__m128i _v = _mm_set1_epi32((int)(val32));			\
	_mm_storeu_si128((__m128i*)(ptr), _v);				\
} while(0)

/* Batch-clear 4 floats (16 bytes) with a float value (unaligned OK)	*/
#define SIMD_CLEAR4_FLOAT(ptr, valf) do {				\
	__m128 _v = _mm_set1_ps((float)(valf));				\
	_mm_storeu_ps((float*)(ptr), _v);				\
} while(0)

/* Float comparison → vector result + 4-bit mask helpers for Z-test	*/
#define SIMD_CMPLT_F32(zv, zb)   _mm_cmplt_ps((zv), (zb))
#define SIMD_CMPLE_F32(zv, zb)   _mm_cmple_ps((zv), (zb))
#define SIMD_CMPGT_F32(zv, zb)   _mm_cmpgt_ps((zv), (zb))
#define SIMD_CMPGE_F32(zv, zb)   _mm_cmpge_ps((zv), (zb))
#define SIMD_CMPEQ_F32(zv, zb)   _mm_cmpeq_ps((zv), (zb))
#define SIMD_CMPNEQ_F32(zv, zb)  _mm_cmpneq_ps((zv), (zb))

#define SIMD_CMPLT_F32_MASK(zv, zb)  _mm_movemask_ps(SIMD_CMPLT_F32((zv), (zb)))
#define SIMD_CMPLE_F32_MASK(zv, zb)  _mm_movemask_ps(SIMD_CMPLE_F32((zv), (zb)))
#define SIMD_CMPGT_F32_MASK(zv, zb)  _mm_movemask_ps(SIMD_CMPGT_F32((zv), (zb)))
#define SIMD_CMPGE_F32_MASK(zv, zb)  _mm_movemask_ps(SIMD_CMPGE_F32((zv), (zb)))
#define SIMD_CMPEQ_F32_MASK(zv, zb)  _mm_movemask_ps(SIMD_CMPEQ_F32((zv), (zb)))
#define SIMD_CMPNEQ_F32_MASK(zv, zb) _mm_movemask_ps(SIMD_CMPNEQ_F32((zv), (zb)))

/* Conditional blend using SSE2-safe AND/OR (no SSE4.1 blendvps)	*/
#define SIMD_BLENDV_F32(cmp, newval, oldval)				\
	_mm_or_ps(_mm_and_ps((cmp), (newval)),				\
		  _mm_andnot_ps((cmp), (oldval)))

/* Unaligned float load/store (4 floats)				*/
#define SIMD_LOADU_F32(ptr)  _mm_loadu_ps((float*)(ptr))
#define SIMD_STOREU_F32(ptr, val) _mm_storeu_ps((float*)(ptr), (val))

/*==========================================================================*/
/* MMX intrinsics  (partial — only MUL8/ADD8/SUB8, no SIMD_CLEAR*)	*/

#elif defined(HAVE_MMX)

#define SIMD_PACK8(lower, upper)					\
	_mm_unpacklo_pi32(_mm_cvtsi32_si64((int)(lower)),		\
			  _mm_cvtsi32_si64((int)(upper)))

#define SIMD_UNPACK8(reg, lower_out, upper_out) do {			\
	__m64 _r = (reg);						\
	*(lower_out) = (uint32_t)_mm_cvtsi64_si32(_r);			\
	*(upper_out) = (uint32_t)_mm_cvtsi64_si32(			\
			_mm_unpackhi_pi32(_r, _r));			\
} while(0)

static inline __m64 simd_mul8_u8x8_mmx(__m64 a, __m64 b)
{
	__m64 zero = _mm_setzero_si64();
	__m64 a_lo = _mm_unpacklo_pi8(a, zero);
	__m64 a_hi = _mm_unpackhi_pi8(a, zero);
	__m64 b_lo = _mm_unpacklo_pi8(b, zero);
	__m64 b_hi = _mm_unpackhi_pi8(b, zero);
	__m64 p_lo = _mm_mullo_pi16(a_lo, b_lo);
	__m64 p_hi = _mm_mullo_pi16(a_hi, b_hi);
	/* Widen to 32-bit to avoid overflow in (p*257+255)>>16 */
	__m64 p32_lo = _mm_unpacklo_pi16(p_lo, zero);
	__m64 p32_hi = _mm_unpackhi_pi16(p_hi, zero);
	p32_lo = _mm_add_pi32(_mm_slli_pi32(p32_lo, 8), p32_lo);
	p32_hi = _mm_add_pi32(_mm_slli_pi32(p32_hi, 8), p32_hi);
	p32_lo = _mm_add_pi32(p32_lo, _mm_set1_pi32(255));
	p32_hi = _mm_add_pi32(p32_hi, _mm_set1_pi32(255));
	__m64 r16_lo = _mm_srli_pi32(p32_lo, 16);
	__m64 r16_hi = _mm_srli_pi32(p32_hi, 16);
	__m64 r16 = _mm_packs_pi32(r16_lo, r16_hi);

	/* Repeat for p_hi */
	p32_lo = _mm_unpacklo_pi16(p_hi, zero);
	p32_hi = _mm_unpackhi_pi16(p_hi, zero);
	p32_lo = _mm_add_pi32(_mm_slli_pi32(p32_lo, 8), p32_lo);
	p32_hi = _mm_add_pi32(_mm_slli_pi32(p32_hi, 8), p32_hi);
	p32_lo = _mm_add_pi32(p32_lo, _mm_set1_pi32(255));
	p32_hi = _mm_add_pi32(p32_hi, _mm_set1_pi32(255));
	r16_lo = _mm_srli_pi32(p32_lo, 16);
	r16_hi = _mm_srli_pi32(p32_hi, 16);
	__m64 r16b = _mm_packs_pi32(r16_lo, r16_hi);

	return _mm_packs_pu16(r16, r16b);
}

#define SIMD_ADD8_8x8(a8, b8, dst8)  ((dst8) = _mm_adds_pu8((a8), (b8)))
#define SIMD_SUB8_8x8(a8, b8, dst8)  ((dst8) = _mm_subs_pu8((a8), (b8)))

/* MMX2: saturating add/sub (no native support, use 32-bit hack) */
static inline __m64 simd_adds_pi16_mmx2(__m64 a, __m64 b)
{
    __m64 zero = _mm_setzero_si64();
    __m64 a16 = _mm_unpacklo_pi16(a, zero);
    __m64 b16 = _mm_unpacklo_pi16(b, zero);
    __m64 a32 = _mm_unpacklo_pi32(a16, zero);
    __m64 b32 = _mm_unpacklo_pi32(b16, zero);
    __m64 sum32 = _mm_add_pi32(a32, b32);
    __m64 sum32_hi = _mm_add_pi32(_mm_unpackhi_pi32(a16, zero), _mm_unpackhi_pi32(b16, zero));
    sum32 = _mm_and_si64(sum32, _mm_set_pi32(0xFF, 0x00));
    sum32_hi = _mm_and_si64(sum32_hi, _mm_set_pi32(0x00, 0xFF));
    return _mm_packs_pi16(sum32, sum32_hi);
}

static inline __m64 simd_subs_pi16_mmx2(__m64 a, __m64 b)
{
    __m64 zero = _mm_setzero_si64();
    __m64 a16 = _mm_unpacklo_pi16(a, zero);
    __m64 b16 = _mm_unpacklo_pi16(b, zero);
    __m64 a32 = _mm_unpacklo_pi32(a16, zero);
    __m64 b32 = _mm_unpacklo_pi32(b16, zero);
    __m64 diff32 = _mm_sub_pi32(a32, b32);
    __m64 diff32_hi = _mm_sub_pi32(_mm_unpackhi_pi32(a16, zero), _mm_unpackhi_pi32(b16, zero));
    diff32 = _mm_and_si64(diff32, _mm_set_pi32(0xFF, 0x00));
    diff32_hi = _mm_and_si64(diff32_hi, _mm_set_pi32(0x00, 0xFF));
    return _mm_packs_pi16(diff32, diff32_hi);
}

// MMX2: average with rounding
static inline __m64 simd_avg_pi16_mmx2(__m64 a, __m64 b)
{
    __m64 one = _mm_set_pi32(0x01, 0x00);
    __m64 sum = simd_adds_pi16_mmx2(a, b);
    __m64 sum_plus_one = simd_adds_pi16_mmx2(sum, one);
    return _mm_srli_pi32(sum_plus_one, 1);
}

// MMX2: full addmul8 (a * f1 + b * f2) / 255
static inline __m64 simd_fulladdmul8_mmx2(__m64 a, __m64 f1, __m64 b, __m64 f2)
{
    __m64 zero = _mm_setzero_si64();
    __m64 a_lo = _mm_unpacklo_pi8(a, zero);
    __m64 a_hi = _mm_unpackhi_pi8(a, zero);
    __m64 f1_lo = _mm_unpacklo_pi8(f1, zero);
    __m64 f1_hi = _mm_unpackhi_pi8(f1, zero);
    __m64 b_lo = _mm_unpacklo_pi8(b, zero);
    __m64 b_hi = _mm_unpackhi_pi8(b, zero);
    __m64 f2_lo = _mm_unpacklo_pi8(f2, zero);
    __m64 f2_hi = _mm_unpackhi_pi8(f2, zero);
    __m64 ma_lo = _mm_mullo_pi16(a_lo, f1_lo);
    __m64 ma_hi = _mm_mullo_pi16(a_hi, f1_hi);
    __m64 mb_lo = _mm_mullo_pi16(b_lo, f2_lo);
    __m64 mb_hi = _mm_mullo_pi16(b_hi, f2_hi);
    __m64 sum_lo = _mm_add_pi16(ma_lo, mb_lo);
    __m64 sum_hi = _mm_add_pi16(ma_hi, mb_hi);
    __m64 sum16 = _mm_packs_pi16(sum_lo, sum_hi);
    __m64 sum16u32 = _mm_unpacklo_pi16(sum16, zero);
    __m64 sum32_lo = _mm_unpacklo_pi16(sum16u32, zero);
    __m64 sum32_hi = _mm_unpackhi_pi16(sum16u32, zero);
    __m64 sum32_257_lo = _mm_add_pi32(_mm_slli_pi32(sum32_lo, 8), sum32_lo);
    __m64 sum32_257_hi = _mm_add_pi32(_mm_slli_pi32(sum32_hi, 8), sum32_hi);
    __m64 sum32_257 = _mm_add_pi32(sum32_257_lo, _mm_set_pi32(255, 0x00));
    sum32_257_hi = _mm_add_pi32(sum32_257_hi, _mm_set_pi32(255, 0x00));
    return _mm_srli_pi32(_mm_packs_pi16(sum32_257, sum32_257_hi), 16);
}

#define SIMD_ADD8_8x8_MMX2(a, b)  (simd_adds_pi16_mmx2(a, b))
#define SIMD_SUB8_8x8_MMX2(a, b)  (simd_subs_pi16_mmx2(a, b))
#define SIMD_AVG8_8x8_MMX2(a, b)  (simd_avg_pi16_mmx2(a, b))
#define SIMD_FULLADD8_8x8_MMX2(a, f1, b, f2)  (simd_fulladdmul8_mmx2(a, f1, b, f2))

/* MMX2: 64-bit aligned memory ops */
#define SIMD_LOAD64_MMX2(ptr)  (*(uint64_t*)(ptr))
#define SIMD_STORE64_MMX2(ptr, val)  (*(uint64_t*)(ptr) = (val))

/* MMX2: bitwise operations on 64-bit */
#define SIMD_AND64_MMX2(a, b)  _mm_and_si64((a), (b))
#define SIMD_OR64_MMX2(a, b)   _mm_or_si64((a), (b))
#define SIMD_XOR64_MMX2(a, b)  _mm_xor_si64((a), (b))

#endif /* HAVE_MMX */

/*==========================================================================*/
/* Tile-fill SIMD helpers – batch-compute 4 interpolated attributes	*/
/* (used by Tile_Fill_*_SIMD functions in soft3dsimd56.c)			*/

#if defined(HAVE_SSE2)
static inline void simd_lerp4_f32(float *out, float base, float delta) {
	__m128 b = _mm_set_ps(base, base + delta, base + 2*delta, base + 3*delta);
	_mm_storeu_ps(out, b);
}
static inline void simd_lerp4_f32_next(float *out, float base, float delta) {
	__m128 b = _mm_set_ps(base + 4*delta, base + 5*delta, base + 6*delta, base + 7*delta);
	_mm_storeu_ps(out, b);
}
#  ifdef __SSE4_1__
static inline void simd_lerp4_i32(int32_t *out, int32_t base, int32_t delta) {
	__m128i b = _mm_set_epi32(base, base + delta, base + 2*delta, base + 3*delta);
	_mm_storeu_si128((__m128i*)out, b);
}
#  else
/* SSE2 fallback — no 32-bit SIMD multiply */
static inline void simd_lerp4_i32(int32_t *out, int32_t base, int32_t delta) {
	out[0]=base; out[1]=base+delta; out[2]=base+2*delta; out[3]=base+3*delta;
}
#  endif
static inline void simd_lerp4_i32_next(int32_t *out, int32_t base, int32_t delta) {
	__m128i b = _mm_set_epi32(base + 4*delta, base + 5*delta, base + 6*delta, base + 7*delta);
	_mm_storeu_si128((__m128i*)out, b);
}
#elif defined(HAVE_NEON)
static inline __attribute__((always_inline)) void simd_lerp4_f32(float *out, float base, float delta) {
	float32x4_t b = vdupq_n_f32(base);
	float32x4_t d = vdupq_n_f32(delta);
	float32x4_t idx = {0.0f, 1.0f, 2.0f, 3.0f};
	vst1q_f32(out, vmlaq_f32(b, idx, d));
}
static inline __attribute__((always_inline)) void simd_lerp4_f32_next(float *out, float base, float delta) {
	float32x4_t b = vdupq_n_f32(base);
	float32x4_t d = vdupq_n_f32(delta);
	float32x4_t idx = {4.0f, 5.0f, 6.0f, 7.0f};
	vst1q_f32(out, vmlaq_f32(b, idx, d));
}
static inline __attribute__((always_inline)) void simd_lerp4_i32(int32_t *out, int32_t base, int32_t delta) {
	int32x4_t b = vdupq_n_s32(base);
	int32x4_t d = vdupq_n_s32(delta);
	int32x4_t idx = {0, 1, 2, 3};
	vst1q_s32(out, vmlaq_s32(b, idx, d));
}
static inline __attribute__((always_inline)) void simd_lerp4_i32_next(int32_t *out, int32_t base, int32_t delta) {
	int32x4_t b = vdupq_n_s32(base);
	int32x4_t d = vdupq_n_s32(delta);
	int32x4_t idx = {4, 5, 6, 7};
	vst1q_s32(out, vmlaq_s32(b, idx, d));
}
#else
/* Fallback scalar */
static inline void simd_lerp4_f32(float *out, float base, float delta) {
	out[0]=base; out[1]=base+delta; out[2]=base+2*delta; out[3]=base+3*delta;
}
static inline void simd_lerp4_f32_next(float *out, float base, float delta) {
	out[0]=base+4*delta; out[1]=base+5*delta; out[2]=base+6*delta; out[3]=base+7*delta;
}
static inline void simd_lerp4_i32(int32_t *out, int32_t base, int32_t delta) {
	out[0]=base; out[1]=base+delta; out[2]=base+2*delta; out[3]=base+3*delta;
}
static inline void simd_lerp4_i32_next(int32_t *out, int32_t base, int32_t delta) {
	out[0]=base+4*delta; out[1]=base+5*delta; out[2]=base+6*delta; out[3]=base+7*delta;
}
#endif

/* Macro to precompute all z values for a span of n pixels (n <= 8) into zbuf.
   Uses SIMD for the first 4, then the next 4 if needed.			*/
#define TILE_SIMD_PRECOMPUTE_Z(zbuf, base_z, dz, n) do {		\
	simd_lerp4_f32(zbuf, base_z, dz);				\
	if ((n) > 4) simd_lerp4_f32_next(zbuf+4, base_z, dz);		\
} while(0)

/* Same for int32 attributes (R,G,B,A,u,v,F)				*/
#define TILE_SIMD_PRECOMPUTE_I32(buf, base, delta, n) do {		\
	simd_lerp4_i32(buf, base, delta);				\
	if ((n) > 4) simd_lerp4_i32_next(buf+4, base, delta);		\
} while(0)

#endif /* SOFT3D_SIMD_H */
