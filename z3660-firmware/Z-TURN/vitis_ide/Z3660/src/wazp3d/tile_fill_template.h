/* tile_fill_template.h — Tile fill function template (X-macro pattern).
 *
 * Included multiple times from soft3d56.c with different macros defined.
 *
 * Required macros before inclusion:
 *   TF_NAME      — function name suffix (e.g. "Flat", "Flat_SIMD")
 *   TF_SIMD      — 0 for scalar inner loop, 1 for SIMD-style (precompute + array)
 *   TF_VARS      — extra local variable declarations (semicolon-separated)
 *   TF_PRE       — precomputation statements before inner loop (SIMD only)
 *   TF_SIMD_LOAD — per-pixel SIMD array loads (sets pixbuf.L.xxx from arrays)
 *   TF_STEP      — per-pixel attribute increment statements (scalar only)
 *   TF_TEX       — texture read + Frag->Tex8 assignment (empty if non-textured)
 *   TF_COLOR     — Frag->ColorRGBA write
 *   TF_FOG       — Frag->FogRGBA write (empty if no fog)
 *
 * Optional NEON batch-optimization macros (when TF_SIMD=1 AND __ARM_NEON__):
 *   TF_NEON_VARS      — local batch-array declarations (e.g. "float z_batch[4];")
 *   TF_NEON_BATCH_PRE — precompute 4 attribute values for current batch
 *   TF_NEON_BATCH_LOAD(k) — load batch[k] into pixbuf.L.xxx
 *   TF_NEON_BATCH_ADV — advance pixbuf.L.xxx by 4 * delta
 *   If not defined, the standard SIMD array path (TF_PRE + TF_SIMD_LOAD) is used.
 *
 * After inclusion, all TF_* and internal macros are undefined by the caller.
 */
#ifndef TF_NAME
#error "tile_fill_template.h: TF_NAME not defined"
#endif

/* Token-pasting helper (needed because ## only works inside #define) */
#define TILE_CONCAT2(a, b) a ## b
#define TILE_FN(name) TILE_CONCAT2(Tile_Fill_, name)

TILE_STATIC void TILE_FN(TF_NAME)(struct SOFT3D_context *SC,
                                WORD tx, WORD ty,
                                union pixel3D *v0, union pixel3D *v1,
                                union pixel3D *v2,
                                WORD tile_size)
{
   register struct fragbuffer3D *Frag = SC->FragBufferDone;
   register struct SOFT3D_mipmap *MM = SC->MM;
   register UBYTE *Image8;
   register UBYTE *Ztest;
   union pixel3D pixbuf;
   register union pixel3D *Pix;
   WORD tile_y;
   WORD y_max = ty + tile_size;
   WORD x_max = tx + tile_size;
#if defined(__ARM_NEON__) && TF_SIMD
   const float32x4_t _neon_k0123 = {0.0f, 1.0f, 2.0f, 3.0f};
   const int32x4_t   _neon_k0123_i = {0, 1, 2, 3};
   TF_NEON_VARS
#endif
   TF_VARS
   (void)MM;

   for (tile_y = ty; tile_y < y_max; tile_y++) {
      Pix = &SC->edge1[tile_y];
      if (Pix->W.large <= 0) continue;
      WORD x_start = Pix->W.x;
      if (x_start < tx) x_start = tx;
      WORD scanline_end = Pix->W.x + Pix->W.large;
      WORD x_end = scanline_end < x_max ? scanline_end : x_max;
      if (x_end <= x_start) continue;

      WORD dx = x_start - Pix->W.x;
      TILE_INTERP_PIXEL(pixbuf, (*Pix), dx);
      pixbuf.W.large = x_end - x_start;
      pixbuf.L.bpp   = Pix->L.bpp;
      pixbuf.L.Image8Y = Pix->L.Image8Y;
      pixbuf.L.ZbufferY = Pix->L.ZbufferY;

      SC->Pix = &pixbuf;
      SC->FunctionZtest(SC);
      Ztest = SC->Ztest;

      Image8 = Pix->L.Image8Y + SC->Image8X[x_start];
      WORD n = x_end - x_start;

#if TF_SIMD
  #if defined(__ARM_NEON__)
      /* NEON batch inner loop: process 4 pixels per batch.
         Load Ztest 4 bytes → if all zero, skip attribute work entirely.
         When mask is non-zero, precompute 4 attribute values (via NEON
         vmlaq_* with _neon_k0123), then write fragments for passing pixels.
         Loop is manually unrolled so NEON vgetq_lane_* receives a compile-time
         constant lane index (required by ARM NEON intrinsics). */
      if (n >= 4) {
         WORD i = 0;
         while (i + 4 <= n) {
            uint32_t mask4 = *(const uint32_t *)(Ztest + i);
            if (mask4) {
               TF_NEON_BATCH_PRE
               if (Ztest[i + 0]) { TF_NEON_BATCH_LOAD(0) Frag->Image8 = Image8; TF_TEX TF_COLOR TF_FOG Frag++; }
               Image8 += pixbuf.L.bpp;
               if (Ztest[i + 1]) { TF_NEON_BATCH_LOAD(1) Frag->Image8 = Image8; TF_TEX TF_COLOR TF_FOG Frag++; }
               Image8 += pixbuf.L.bpp;
               if (Ztest[i + 2]) { TF_NEON_BATCH_LOAD(2) Frag->Image8 = Image8; TF_TEX TF_COLOR TF_FOG Frag++; }
               Image8 += pixbuf.L.bpp;
               if (Ztest[i + 3]) { TF_NEON_BATCH_LOAD(3) Frag->Image8 = Image8; TF_TEX TF_COLOR TF_FOG Frag++; }
               Image8 += pixbuf.L.bpp;
            } else {
               Image8 += 4u * pixbuf.L.bpp;
            }
            TF_NEON_BATCH_ADV
            i += 4;
         }
         /* Scalar remainder (0..3 pixels) */
         while (i < n) {
            if (Ztest[i]) {
               Frag->Image8 = Image8;
               TF_TEX
               TF_COLOR
               TF_FOG
               Frag++;
            }
            Image8 += pixbuf.L.bpp;
            TF_STEP
            i++;
         }
      } else
  #endif
      {
         /* Standard SIMD: precompute all arrays, then pixel loop */
         TF_PRE
         { WORD i = 0; do {
            if (Ztest[i]) {
               TF_SIMD_LOAD
               Frag->Image8 = Image8;
               TF_TEX
               TF_COLOR
               TF_FOG
               Frag++;
            }
            Image8 += pixbuf.L.bpp;
            i++;
         } while (i < n); }
      }
#else
      while (n--) {
         if (*Ztest++) {
            Frag->Image8 = Image8;
            TF_TEX
            TF_COLOR
            TF_FOG
            Frag++;
         }
         Image8 += pixbuf.L.bpp;
         TF_STEP
      }
#endif

      if (Frag > SC->FragBufferMaxi) {
         SC->FragBufferDone = Frag;
         SOFT3D_Flush(SC);
         Frag = SC->FragBuffer;
      }
   }
   SC->FragBufferDone = Frag;
}

#undef TILE_CONCAT2
#undef TILE_FN
