#ifndef SOFT3D_TILING_H
#define SOFT3D_TILING_H

#include "soft3d_simd.h"

struct SOFT3D_context;
void SOFT3D_SetRenderMode(APTR sc, UBYTE mode);
static void DrawPolyPix_Tiled(struct SOFT3D_context *SC);
static int  TileOverlapsTriangle(WORD tx, WORD ty,
                                 union pixel3D *v0,
                                 union pixel3D *v1,
                                 union pixel3D *v2);

#endif
