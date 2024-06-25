/* Wazp3D Beta 56 : Alain THELLIER - Paris - FRANCE - (November 2006 to 2014)     */
/* Code clean-up and library enhancements from Gunther Nikl                 */
/* Adaptation to AROS from Matthias Rustler                            */
/* Adaptation to Morphos from Szil�rd 'BSzili' Bir�                         */
/* LICENSE: GNU General Public License (GNU GPL) for this file                */

/* This file contain the SOFT3D rasterizer that truly draw the pixels            */

#include <stdio.h>
#include "../video.h"
#include <xil_types.h>
#include "xil_printf.h"

#include "../debug_console.h"
#include "../memorymap.h"
#include "str_soft3dop.h"
#include "heap.h"

extern DEBUG_CONSOLE debug_console;

typedef int16_t WORD;

#include "Warp3D.h"
#include "Wazp3D.h"
#include "cybergraphics.h"

struct WAZP3D_parameters *Wazp3D;            /* local pointer to the struct in Warp3d.library */
struct memory3D *firstME=NULL;

void DEBUG_SOFT3D(const char *format, ...)
{
	if(debug_console.debug_soft3d==0)
		return;
	va_list args;
	va_start(args, format);
	vprintf(format,args);
	va_end(args);
}

#include "mymemory.h"
#include <stdlib.h>

void PrintP(struct point3D *P);
void PrintPoint3D(struct point3D *P);

void PrintRGBA(UBYTE *RGBA)
{}
void pf(float x)
{}
void *Libmalloc(ULONG size)
{
	return(heap_alloc(size));
}
void Libfree(void *p)
{
	heap_free(p);
}
void Libmemcpy(void *s1,void *s2,LONG n)
{
	memcpy(s1,s2,n);
}
void Libstrcpy(void *p1,void *p2)
{
	strcpy(p1,p2);
}
void Libstrcat(void *p1,void *p2)
{
	strcat(p1,p2);
}
ULONG Libstrlen(void *p)
{
	return(strlen(p));
}
void LibAlert(void *p1)
{}
void Libsavefile(void *filename,void *pt,ULONG size)
{}
void Libloadfile(void *filename,void *pt,ULONG size)
{}
#include "soft3d_protos.h"
#include "soft3d_opengl.h"

#define SWAP16(a) a = __builtin_bswap16(a)
#define SWAP32(a) a = __builtin_bswap32(a)

#define swap32(a) __builtin_bswap32(a)
#define swap16(a) __builtin_bswap16(a)

#define SWAP32_D(A) do{SWAP32(A.x.i);\
                       SWAP32(A.y.i);\
                       SWAP32(A.z.i);\
                       SWAP32(A.u.i);\
                       SWAP32(A.v.i);\
                       SWAP32(A.w.i);\
                       SWAP32(A.RGBA.L[0]);}while(0)

/* use a local pointer on Wazp3D : having a copy here allow to separate the sof3d binary & wazp3d binary*/
#ifdef SOFT3DLIB

#include "soft3d_mem_print.h"                /* memory and print/debug functions : usually in Warp3d.c */

#endif
#undef TRUE
#define TRUE 1

volatile struct Soft3DData *data3d = (volatile struct Soft3DData*)((uint32_t)Z3_SOFT3D_ADDR);
struct Soft3DData local_data;

void handle_soft3d_op(ZZ_VIDEO_STATE* vs,uint16_t zdata)
{
//    if(con.debug_rtg)
//    printf("soft3d_op 0x%X  %s\n",zdata,soft3d_op_string[zdata]);

	switch(zdata) {
    	case OP_START: {
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		DEBUG_SOFT3D("PrefsWazp3D 0x%08lx\n",local_data.offset[0]);
    		uint32_t add=(uint32_t)
    				SOFT3D_Start((uint32_t *)local_data.offset[0]);
    		*(uint32_t*)(RTG_BASE+REG_ZZ_SOFT3D_OP)=swap32(add);
    	}
    		break;
    	case OP_END:
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		SOFT3D_End((uint32_t *)local_data.offset[0]);
    		break;
    	case OP_SETBITMAP:
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.offset[1]=swap32(data3d->offset[1]);
    		local_data.offset[2]=swap32(data3d->offset[2]);
    		local_data.format[0]=swap32(data3d->format[0]);
    		local_data.x[0]=swap16(data3d->x[0]);
    		local_data.y[0]=swap16(data3d->y[0]);
    		local_data.x[1]=swap16(data3d->x[1]);
    		local_data.y[1]=swap16(data3d->y[1]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("BM 0x%08lx\n",local_data.offset[1]);
    		DEBUG_SOFT3D("BMDATA 0x%08lx\n",local_data.offset[2]);
    		DEBUG_SOFT3D("BMFORMAT 0x%08lx\n",local_data.format[0]);
    		DEBUG_SOFT3D("x %d y %d\n",local_data.x[0],local_data.y[0]);
    		DEBUG_SOFT3D("l %d h %d\n",local_data.x[1],local_data.y[1]);

    		SOFT3D_SetBitmap((uint32_t *)local_data.offset[0],
    				         (uint32_t *)local_data.offset[1],
							 (uint32_t *)local_data.offset[2],
							 local_data.format[0],
							 local_data.x[0],local_data.y[0],
							 local_data.x[1],local_data.y[1]);
    		break;
    	case OP_SETCLIPPING:
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.x[0]=swap16(data3d->x[0]);
    		local_data.x[1]=swap16(data3d->x[1]);
    		local_data.y[0]=swap16(data3d->y[0]);
    		local_data.y[1]=swap16(data3d->y[1]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("xmin %d ymin %d\n",local_data.x[0],local_data.y[0]);
    		DEBUG_SOFT3D("xmax %d ymax %d\n",local_data.x[1],local_data.y[1]);

    		SOFT3D_SetClipping((uint32_t *)local_data.offset[0],
							   local_data.x[0],local_data.x[1],
							   local_data.y[0],local_data.y[1]);
    		break;
    	case OP_SETDRAWSTATE:
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.offset[1]=swap32(data3d->offset[1]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("STA 0x%08lx\n",local_data.offset[1]);

    		SOFT3D_SetDrawState((uint32_t *)local_data.offset[0],
    				            (uint32_t *)local_data.offset[1]);
    		break;
    	case OP_DRAWPRIMITIVE:
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.offset[1]=swap32(data3d->offset[1]);
    		local_data.format[0]=swap32(data3d->format[0]);
    		local_data.format[1]=swap32(data3d->format[1]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("P 0x%08lx\n",local_data.offset[1]);
    		DEBUG_SOFT3D("PNB 0x%08lx\n",local_data.format[0]);
    		DEBUG_SOFT3D("PRIMITIVE 0x%08lx\n",local_data.format[1]);

    		SOFT3D_DrawPrimitive((uint32_t *)local_data.offset[0],
    				             (uint32_t *)local_data.offset[1],
								 local_data.format[0],
								 local_data.format[1]);
    		break;
    	case OP_DOUPDATE: {
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
//    		uint32_t dat=swap32((uint32_t)SOFT3D_DoUpdate((uint32_t *)local_data.offset[0]));
    		uint32_t dat=0; // why the amiga hangs when this returns true?
    		*(uint32_t*)(RTG_BASE+REG_ZZ_SOFT3D_OP)=dat;
    	}
    		break;
    	case OP_FLUSH:
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);

    		SOFT3D_Flush((uint32_t *)local_data.offset[0]);
    		break;
    	case OP_CREATETEXTURE: {
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.offset[1]=swap32(data3d->offset[1]);
    		local_data.x[0]=swap16(data3d->x[0]);
    		local_data.y[0]=swap16(data3d->y[0]);
    		local_data.x[1]=swap16(data3d->x[1]);
    		local_data.y[1]=swap16(data3d->y[1]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("P 0x%08lx\n",local_data.offset[1]);
    		DEBUG_SOFT3D("l %d h %d\n",local_data.x[0],local_data.y[0]);
    		DEBUG_SOFT3D("format %d textflags %d\n",local_data.x[1],local_data.y[1]);

    		uint32_t add=(uint32_t)
    		SOFT3D_CreateTexture((uint32_t *)local_data.offset[0],
    				             (uint32_t *)local_data.offset[1],
								 local_data.x[0],local_data.y[0],
								 local_data.x[1],local_data.y[1]);
            *(uint32_t*)(RTG_BASE+REG_ZZ_SOFT3D_OP)=swap32(add);
    	}
    		break;
    	case OP_FREETEXTURE:
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.offset[1]=swap32(data3d->offset[1]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("ST 0x%08lx\n",local_data.offset[1]);

    		SOFT3D_FreeTexture((uint32_t *)local_data.offset[0],
    				           (uint32_t *)local_data.offset[1]);
    		break;
    	case OP_UPDATETEXTURE:
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.offset[1]=swap32(data3d->offset[1]);
    		local_data.offset[2]=swap32(data3d->offset[2]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("ST 0x%08lx\n",local_data.offset[1]);
    		DEBUG_SOFT3D("PT 0x%08lx\n",local_data.offset[2]);

    		SOFT3D_UpdateTexture((uint32_t *)local_data.offset[0],
    				             (uint32_t *)local_data.offset[1],
								 (uint32_t *)local_data.offset[2]);
    		break;
    	case OP_ALLOCZBUFFER: {
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.x[0]=swap16(data3d->x[0]);
    		local_data.y[0]=swap16(data3d->y[0]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("l %d h %d\n",local_data.x[0],local_data.y[0]);

    		uint32_t add=(uint32_t)
    				SOFT3D_AllocZbuffer((uint32_t *)local_data.offset[0],
								        local_data.x[0],local_data.y[0]);
    		*(uint32_t*)(RTG_BASE+REG_ZZ_SOFT3D_OP)=swap32(add);
    	}
    		break;
    	case OP_ALLOCIMAGEBUFFER:
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.x[0]=swap16(data3d->x[0]);
    		local_data.y[0]=swap16(data3d->y[0]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("l %d h %d\n",local_data.x[0],local_data.y[0]);

    		SOFT3D_AllocImageBuffer((uint32_t *)local_data.offset[0],
								     local_data.x[0],local_data.y[0]);
    		break;
    	case OP_CLEARZBUFFER: {
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.format[0]=swap32(data3d->format[0]);
    		float *fz=(float *)&local_data.format[0];
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("format %f (0x%08lx)\n",*fz,local_data.format[0]);

    		SOFT3D_ClearZBuffer((uint32_t *)local_data.offset[0],
								*fz);
    		}
    		break;
    	case OP_READZSPAN:
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.x[0]=swap16(data3d->x[0]);
    		local_data.y[0]=swap16(data3d->y[0]);
    		local_data.format[0]=swap32(data3d->format[0]);
    		local_data.offset[1]=swap32(data3d->offset[1]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("x %d y %d\n",local_data.x[0],local_data.y[0]);
    		DEBUG_SOFT3D("N 0x%08lx\n",local_data.format[0]);
    		DEBUG_SOFT3D("Z 0x%08lx\n",local_data.offset[1]);

    		SOFT3D_ReadZSpan((uint32_t *)local_data.offset[0],
    				         local_data.x[0],local_data.y[0],
							 local_data.format[0],
							 (uint32_t *)local_data.offset[1]);
    		break;
    	case OP_WRITEZSPAN:
    		local_data.offset[0]=swap32(data3d->offset[0]);
    		local_data.x[0]=swap16(data3d->x[0]);
    		local_data.y[0]=swap16(data3d->y[0]);
    		local_data.format[0]=swap32(data3d->format[0]);
    		local_data.offset[1]=swap32(data3d->offset[1]);
    		local_data.offset[2]=swap32(data3d->offset[2]);
    		DEBUG_SOFT3D("SC 0x%08lx\n",local_data.offset[0]);
    		DEBUG_SOFT3D("x %d y %d\n",local_data.x[0],local_data.y[0]);
    		DEBUG_SOFT3D("N 0x%08lx\n",local_data.format[0]);
    		DEBUG_SOFT3D("Z 0x%08lx\n",local_data.offset[1]);
    		DEBUG_SOFT3D("MASK 0x%08lx\n",local_data.offset[2]);

    		SOFT3D_WriteZSpan((uint32_t *)local_data.offset[0],
    				          local_data.x[0],local_data.y[0],
							  local_data.format[0],
							  (uint32_t *)local_data.offset[1],
							  (uint32_t *)local_data.offset[2]);
    		break;
        default:
            break;
    }
}




/*==================================================================================*/
/* structures definitions only used in SOFT3D */
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

#ifdef MOTOROLAORDER
struct pixel3DW{
    ZBUFF z;
    float w;
    UBYTE u1,u,u3,u4;
    UBYTE v1,v,v3,v4;
    UBYTE R1,R,R3,R4;
    UBYTE G1,G,G3,G4;
    UBYTE B1,B,B3,B4;
    UBYTE A1,A,A3,A4;
    WORD  x,xlow;
    WORD  y,ylow;
    WORD  F,Flow;
    WORD large,largelow;
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
#else
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
#endif
};
/*=============================================================*/
#ifdef MOTOROLAORDER
union oper3D
{
    struct oper3DW
    {
        ULONG Index;
    }  L;
    struct oper3DB
    {
        UBYTE empty1,empty2,a,b;
    }  B;
};
#else
union oper3D
{
    struct oper3DW
    {
        ULONG Index;
    }  L;
    struct oper3DB
    {
        UBYTE b,a,empty3,empty4;
    }  B;
};
#endif
/*=============================================================*/
struct fragbuffer3D{
    UBYTE *Image8;
    UBYTE *Tex8;
    union
    {
        ULONG L[1];
        uint8_t b[4];
    } ColorRGBA;
    union
    {
        ULONG L[1];
        uint8_t b[4];
    } BufferRGBA;
    union
    {
        ULONG L[1];
        uint8_t b[4];
    } FogRGBA;
    union
    {
        ULONG L[1];
        uint8_t b[4];
    } TmpRGBA;
};
/*=============================================================*/
struct AvailableFunctions{
    HOOKEDFUNCTION Fill[8];            /* functions available */
    HOOKEDFUNCTION Edge[16];
    HOOKEDFUNCTION TexEnv[6*2+1];
    HOOKEDFUNCTION Ztest[16];
    HOOKEDFUNCTION BlendFast[256];
};
struct AvailableFunctions Functions;
/*=============================================================*/
struct SOFT3D_context{
#ifdef SLOWCPU
    UBYTE Mul8[256*256];            /* table for multiplying two UBYTE */
    UBYTE Add8[256*256];            /* table for adding two UBYTE with overflow */
    UBYTE Fil8[256*256];            /* table for filtering two UBYTE  */
#endif
    ULONG B0toRGBA32[256];            /* tables for converting 8/15/16 bits to RGBA */
    ULONG B1toRGBA32[256];
    UBYTE RtoB0[256];
    UBYTE GtoB0[256];
    UBYTE BtoB0[256];
    UBYTE RtoB1[256];
    UBYTE GtoB1[256];
    UBYTE BtoB1[256];
    WORD large,high,bits;
    ULONG bmformat;
    struct vertex3D ClipMin;            /* 3D clipper */
    struct vertex3D ClipMax;
    UBYTE *Image8;
    ULONG *ImageBuffer32;
    ZBUFF *Zbuffer;
    UBYTE NoopRGBA[4];
    union
    {
        ULONG L[1];
        uint8_t b[4];
    } FlatRGBA;
    struct SOFT3D_mipmap  *MM;
    WORD Pnb;
    union pixel3D *Pix;
    WORD PointLarges[MAXSCREEN];
    union pixel3D edge1[MAXSCREEN];
    union pixel3D edge2[MAXSCREEN];
    union pixel3D edgeM[MAXSCREEN];
    UWORD Image8X[MAXSCREEN];
    UBYTE Ztest[MAXSCREEN];
    HOOKEDFUNCTION FunctionPoly;                /* functions currently used in pipeline */
    HOOKEDFUNCTION FunctionEdge;
    HOOKEDFUNCTION FunctionFill;
    HOOKEDFUNCTION FunctionZtest;
    HOOKEDFUNCTION FunctionIn;
    HOOKEDFUNCTION FunctionBlend;
    HOOKEDFUNCTION FunctionBlendFast;
    HOOKEDFUNCTION FunctionTexEnv;
    HOOKEDFUNCTION FunctionFog;
    HOOKEDFUNCTION FunctionFilter;
    HOOKEDFUNCTION FunctionSepia;
    HOOKEDFUNCTION FunctionOut;
    HOOKEDFUNCTION FunctionBitmapOut;
    HOOKEDFUNCTION FunctionBitmapIn;
    HOOKEDFUNCTION FunctionWriteImageBuffer;
    struct fragbuffer3D FragBuffer[FRAGBUFFERSIZE];
    struct fragbuffer3D *FragBufferDone;        /* last pixel written in FragBuffer */
    struct fragbuffer3D *FragBufferMaxi;        /* dont fill FragBuffer beyond this limit ==> flush it*/
    ULONG FragSize2;                        /* nb pixels = FragSize*2 */
    ULONG FogRGBAs[FOGSIZE];
    WORD PolyPnb;
    union pixel3D PolyPix[MAXPOLY];
    union pixel3D *P1;        /* do FunctionEdge/FunctionPoly from P1 to P2 */
    union pixel3D *P2;
    ULONG PolyHigh,PolyLarge;
    struct point3D PolyP[MAXPOLY];
    struct point3D T1[MAXPOLY];
    struct point3D T2[MAXPOLY];
    void *firstST;
    WORD Pxmin,Pxmax,Pymin,Pymax;        /* really updated region */
    float Pzmin,Pzmax;
    WORD xUpdate,largeUpdate,yUpdate,highUpdate;    /* really updated region previous frame*/
    UWORD Tnb;
    UBYTE SrcFunc,DstFunc;
    UBYTE UseFunctionBitmapIn;
    UBYTE ColorChange;        /* V41: Soft3D can smartly desactivate the gouraud if all points got the same color */
    UBYTE ColorTransp;        /* V43: Soft3D detect if points got alpha value */
    UBYTE ColorWhite;            /* V47: Soft3D detect if points are all white ==> no coloring */
    LONG dmin;
    UWORD yoffset;
    UBYTE UseHard;
    struct state3D state;
    struct HARD3D_context HC;
#ifdef AMIGA                 /* of course the PC DLL cant manipulate an Amiga's bitmap */
    void *bm;
    void *bmHandle;            /* to directly write to bitmap */
    struct RastPort rastport;
    void *colorsbm;
#endif
};
/*=================================================================*/
struct SOFT3D_mipmap{
    UBYTE *Tex8V[256];    /* adresse of the texture at this line */
    ULONG  Tex8Vlow[256];
    UWORD  Tex8U[256];
    UWORD  Tex8Ulow[256];
};
/*=================================================================*/
#define NBMIPMAPS 12
struct SOFT3D_texture{
    struct SOFT3D_mipmap MMs[NBMIPMAPS];
    UBYTE *pt;            /* original  data from 3Dprog as RGB or RGBA */
    UBYTE *ptmm;        /* mipmaps as RGB or RGBA */
    UWORD large,high,format,bits;
    UBYTE TexFlags;
    UBYTE name[40];
    UWORD Tnum;
    void *nextST;
    void *bm;
    struct HARD3D_texture HT;
};
/*=================================================================*/
#ifdef WAZP3DDEBUG
UBYTE font8x8[14*16*8] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,8,24,24,8,0,16,8,8,16,24,0,0,0,0,8,
        0,8,24,60,28,36,40,8,16,8,24,8,0,0,0,8,
        0,8,0,24,24,8,20,0,16,8,0,28,0,24,0,24,
        0,0,0,60,12,16,40,0,16,8,0,8,0,0,0,16,
        0,8,0,24,28,36,20,0,16,8,0,0,8,0,8,16,
        0,0,0,0,8,0,0,0,8,16,0,0,8,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        28,8,28,24,4,28,28,28,28,28,0,0,0,0,0,16,
        20,24,4,4,12,16,16,4,20,20,0,0,0,0,0,8,
        20,8,28,12,20,28,28,8,28,28,8,8,8,0,16,16,
        20,8,16,4,28,4,20,8,20,4,0,0,16,24,8,0,
        28,8,28,24,4,28,28,8,28,28,8,8,8,0,16,16,
        0,0,0,0,0,0,0,0,0,0,0,8,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        24,24,56,24,56,28,28,28,36,8,4,36,16,34,36,24,
        36,24,36,36,36,16,16,32,36,8,4,40,16,54,52,36,
        90,36,56,32,36,24,24,44,60,8,4,48,16,42,44,36,
        94,60,36,36,36,16,16,36,36,8,20,40,16,34,36,36,
        32,36,56,24,56,28,16,28,36,8,24,36,28,34,36,24,
        24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        56,24,56,28,28,36,20,34,20,20,28,24,16,24,8,0,
        36,36,36,32,8,36,20,42,20,20,4,16,16,8,20,0,
        56,36,56,24,8,36,20,42,8,8,8,16,24,8,0,0,
        32,40,36,4,8,36,8,20,20,8,16,16,8,8,0,0,
        32,20,36,56,8,24,8,20,20,8,28,16,8,8,0,0,
        0,0,0,0,0,0,0,0,0,0,0,24,0,24,0,60,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        16,0,16,0,4,0,24,0,16,8,8,16,8,0,0,0,
        8,0,16,0,4,8,16,0,16,0,0,16,8,0,0,0,
        0,12,28,28,28,28,24,28,28,8,8,20,8,54,24,28,
        0,20,20,16,20,16,16,20,20,8,8,24,8,42,20,20,
        0,12,28,28,28,28,16,28,20,8,8,20,8,42,20,28,
        0,0,0,0,0,0,0,4,0,0,8,0,0,0,0,0,
        0,0,0,0,0,0,0,28,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,8,8,16,12,0,
        0,0,0,0,16,0,0,0,0,0,0,8,8,16,24,0,
        24,12,24,24,24,20,20,36,20,20,28,16,8,8,0,0,
        20,20,16,16,16,20,20,36,8,20,8,16,8,8,0,0,
        24,12,16,48,8,12,8,60,20,8,28,8,8,16,0,0,
        16,4,0,0,0,0,0,0,0,16,0,8,0,16,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,8,0,0,0,0,0,
        8,0,0,12,0,0,8,8,8,20,28,0,62,0,8,0,
        8,0,0,8,0,0,28,28,20,40,32,0,40,0,8,0,
        8,0,0,28,0,0,8,8,0,26,24,8,44,0,8,0,
        8,0,0,8,0,0,8,8,0,20,4,16,40,0,8,0,
        8,0,8,8,40,84,8,28,0,42,56,8,62,0,8,0,
        0,0,8,16,40,0,8,8,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,8,8,24,24,0,0,0,12,122,40,0,0,0,8,20,
        0,0,0,0,0,0,0,0,24,46,0,0,0,0,8,20,
        0,0,0,0,0,24,0,0,0,42,24,16,28,0,8,8,
        0,0,0,0,0,24,28,62,0,0,16,8,42,0,8,8,
        0,0,0,0,0,0,0,0,0,0,48,16,28,0,8,8,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,4,20,20,8,12,24,24,8,0,0,0,24,60,
        0,8,4,8,8,28,8,24,0,44,8,0,0,0,52,0,
        0,0,12,28,20,8,8,20,0,52,0,8,0,0,44,0,
        0,8,16,8,0,28,8,12,0,24,0,16,28,8,24,0,
        0,8,12,12,0,8,8,12,0,0,0,8,4,0,0,0,
        0,8,8,0,0,0,8,24,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,8,0,8,0,0,0,0,0,0,0,0,0,0,0,
        8,0,8,8,16,0,24,0,0,8,8,0,36,36,36,8,
        0,24,0,8,0,40,24,8,0,8,8,16,40,40,40,0,
        0,24,0,0,0,40,8,0,0,0,0,8,20,20,20,8,
        0,24,0,0,0,52,8,0,8,0,0,16,36,36,36,16,
        0,0,0,0,0,32,8,0,24,0,0,0,0,0,0,8,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        8,8,8,8,8,8,14,24,28,28,28,28,8,16,40,0,
        20,20,20,20,20,20,24,36,16,16,16,16,0,0,0,16,
        20,20,20,20,20,20,46,32,28,28,28,28,8,16,16,16,
        28,28,28,28,28,28,56,36,16,16,16,16,8,16,16,16,
        20,20,20,20,20,20,46,24,28,28,28,28,8,16,16,16,
        0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,24,16,8,24,24,36,0,0,16,8,24,24,8,0,0,
        24,36,24,24,24,24,24,0,28,36,36,36,36,20,16,8,
        20,52,36,36,36,36,36,20,44,36,36,36,36,20,28,20,
        52,44,36,36,36,36,36,8,52,36,36,36,36,8,20,28,
        20,36,36,36,36,36,36,20,52,36,36,36,36,8,28,20,
        24,36,24,24,24,24,24,0,56,24,24,24,24,8,16,24,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        8,4,12,12,20,8,0,0,16,8,8,20,16,8,16,0,
        4,8,0,0,0,0,0,0,8,16,20,0,8,16,40,40,
        12,12,12,12,12,12,28,12,24,24,24,24,0,0,0,0,
        20,20,20,20,20,20,42,16,20,20,20,20,8,16,16,16,
        12,12,12,12,12,12,30,12,12,12,12,12,8,16,16,16,
        0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,24,16,4,8,12,20,0,0,16,4,8,20,4,16,20,
        16,0,8,8,20,0,0,0,0,8,8,20,0,8,16,0,
        8,24,28,28,28,28,28,8,28,20,20,20,20,20,24,20,
        24,20,20,20,20,20,20,28,28,20,20,20,20,8,20,8,
        24,20,28,28,28,28,28,8,28,12,12,12,12,8,24,8,
        0,0,0,0,0,0,0,0,0,0,0,0,0,16,16,16,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
#endif
/*=================================================================*/
void PrintST(struct SOFT3D_texture *ST)
{
#ifdef WAZP3DDEBUG
    if(ST==NULL) return;
    if (!Wazp3D->DebugST.ON) return;
    Libprintf("SOFT3D_texture(%ld) %s  pt %ld NextST(%ld) TexFlags %ld \n",(ULONG)ST,ST->name,(ULONG)ST->pt,(ULONG)ST->nextST,(ULONG)ST->TexFlags);
#else
    return;
#endif
}
/*=================================================================*/
void PrintPix(union pixel3D *Pix)
{
#ifdef WAZP3DDEBUG
    if (!Wazp3D->DebugPoint.ON)  return;
    if (!Wazp3D->DebugSOFT3D.ON) return;
    Libprintf(" Pix XY %ld %ld \tUV %ld %ld ",(LONG)Pix->W.x,(LONG)Pix->W.y,(LONG)Pix->W.u,(LONG)Pix->W.v);
    Libprintf("RGBA %ld %ld %ld %ld large %ld",(LONG)Pix->W.R,(LONG)Pix->W.G,(LONG)Pix->W.B,(LONG)Pix->W.A,(LONG)Pix->W.large);
    Libprintf(" ZWF ");  pf((float)Pix->L.z); pf(Pix->L.w); pf(Pix->W.F);Libprintf("\n");
#else
    return;
#endif
}
/*=================================================================*/
void PrintP2(struct point3D *P)
{
#ifdef WAZP3DDEBUG
    Libprintf("ClipXYZW; %ld; %ld; %ld; %ld; UV; %ld; %ld\n",(ULONG)P->x,(ULONG)P->y,(ULONG)(1000.0*P->z),(ULONG)(1000.0*P->w),(ULONG)(P->u),(ULONG)(P->v));
#else
    return;
#endif
}
void PrintP(struct point3D *P)
{
#ifdef WAZP3DDEBUG
    Libprintf("XYZ;UVW %f %f %f; %f %f %f\n",P->x,P->y,P->z,P->u,P->v,P->w);
#else
    return;
#endif
}
void PrintPoint3D(struct point3D *P)
{
    Libprintf("XYZ;UVW %f %f %f; %f %f %f\n",P->x.f,P->y.f,P->z.f,P->u.f,P->v.f,P->w.f);
}
/*=================================================================*/
/* SOFT3D internal private functions */
void AntiAliasImage(void *image,UWORD large,UWORD high);
void ClipPoly(struct SOFT3D_context *SC);
void ClipLine(struct SOFT3D_context *SC);
void CreateMipmaps(struct SOFT3D_texture *ST);
void DrawLinePix(struct SOFT3D_context *SC);
void DrawPointPix(struct SOFT3D_context *SC);
void DrawSimplePix(struct SOFT3D_context *SC,register union pixel3D *P);
void DrawPolyP(struct SOFT3D_context *SC);
void DrawPolyPix(struct SOFT3D_context *SC);
void DrawTriP(struct SOFT3D_context *SC,register struct point3D *A,register struct point3D *B,register struct point3D *C);
void Fill_BigTexPersp2_Gouraud_Fog(struct SOFT3D_context *SC);
void Fill_BigTexPersp2(struct SOFT3D_context *SC);
void Fill_Flat(struct SOFT3D_context *SC);
void Fill_Fog(struct SOFT3D_context *SC);
void Fill_Gouraud(struct SOFT3D_context *SC);
void Fill_Gouraud_Fog(struct SOFT3D_context *SC);
void Fill_Tex(struct SOFT3D_context *SC);
void Fill_Tex_Fog(struct SOFT3D_context *SC);
void Fill_Tex_Gouraud(struct SOFT3D_context *SC);
void Fill_Tex_Gouraud_Fog(struct SOFT3D_context *SC);
BOOL LockBM(struct SOFT3D_context *SC);
void PixelsARGB(struct SOFT3D_context *SC);
void PixelsAdd24(struct SOFT3D_context *SC);
void PixelsAdd32(struct SOFT3D_context *SC);
void PixelsSub24(struct SOFT3D_context *SC);
void PixelsSub32(struct SOFT3D_context *SC);
void PixelsBlend24(struct SOFT3D_context *SC);
void PixelsBlend32(struct SOFT3D_context *SC);
void PixelsDecal24(struct SOFT3D_context *SC);
void PixelsDecal32(struct SOFT3D_context *SC);
void PixelsInOutBGRA(struct SOFT3D_context *SC);
void PixelsModulate24(struct SOFT3D_context *SC);
void PixelsModulate32(struct SOFT3D_context *SC);
void PixelsOne_One24(struct SOFT3D_context *SC);
void PixelsReplace24(struct SOFT3D_context *SC);
void PixelsReplace32(struct SOFT3D_context *SC);
void PixelsOne_Zero32(struct SOFT3D_context *SC);
void PixelsSrcAlpha_OneMinusSrcAlpha32(struct SOFT3D_context *SC);
void PixelsSrcAlpha_One32(struct SOFT3D_context *SC);
void PixelsOne_One24(struct SOFT3D_context *SC);
void PixelsZero_SrcAlpha32(struct SOFT3D_context *SC);
void PixelsZero_SrcColor24(struct SOFT3D_context *SC);
void PixelsDstColor_Zero24(struct SOFT3D_context *SC);
void PixelsZero_OneMinusSrcColor24(struct SOFT3D_context *SC);
void PixelsSrcAlpha_OneMinusSrcColor32(struct SOFT3D_context *SC);
void PixelsOne_OneMinusSrcAlpha32(struct SOFT3D_context *SC);
void PixelsSrcAlpha_OneMinusSrcAlpha32fast(struct SOFT3D_context *SC);
void PixelsSrcAlpha_OneMinusSrcAlpha32perfect(struct SOFT3D_context *SC);
void PixelsChroma32fast(struct SOFT3D_context *SC);
void PrintPix(union pixel3D *Pix);
void PrintST(struct SOFT3D_texture *ST);
void ReduceBitmap(UBYTE *pt,UBYTE *pt2,UWORD large,UWORD high, WORD bits,WORD ratio);
void UnLockBM(struct SOFT3D_context *SC);
void UVtoRGBA(struct SOFT3D_texture *ST,float u,float v,UBYTE *RGBA);
void Ztest_zalways(struct SOFT3D_context *SC);
void Ztest_zalways_update(struct SOFT3D_context *SC);
void Ztest_zequal(struct SOFT3D_context *SC);
void Ztest_zequal_update(struct SOFT3D_context *SC);
void Ztest_zgequal(struct SOFT3D_context *SC);
void Ztest_zgequal_update(struct SOFT3D_context *SC);
void Ztest_zgreater(struct SOFT3D_context *SC);
void Ztest_zgreater_update(struct SOFT3D_context *SC);
void Ztest_zlequal(struct SOFT3D_context *SC);
void Ztest_zlequal_update(struct SOFT3D_context *SC);
void Ztest_zless(struct SOFT3D_context *SC);
void Ztest_zless_update(struct SOFT3D_context *SC);
void Ztest_znequal(struct SOFT3D_context *SC);
void Ztest_znequal_update(struct SOFT3D_context *SC);
void Ztest_znever_update(struct SOFT3D_context *SC);
void Ztest_znotequal(struct SOFT3D_context *SC);
void Ztest_znotequal_update(struct SOFT3D_context *SC);
void PixelsOut8(struct SOFT3D_context *SC);
void PixelsIn8(struct SOFT3D_context *SC);
void PixelsOut16(struct SOFT3D_context *SC);
void PixelsIn16(struct SOFT3D_context *SC);
void PixelsBlendFunctionAll(struct SOFT3D_context *SC);
void Poly_Persp0_Tex(struct SOFT3D_context *SC);
void Poly_Persp0_Gouraud(struct SOFT3D_context *SC);
void Poly_Persp0_Flat(struct SOFT3D_context *SC);
void Poly_Persp0_Tex_Gouraud_Fog(struct SOFT3D_context *SC);
void Poly_Persp1_Tex(struct SOFT3D_context *SC);
void Poly_Persp1_Gouraud(struct SOFT3D_context *SC);
void Poly_Persp1_Flat(struct SOFT3D_context *SC);
void Poly_Persp1_Tex_Gouraud_Fog(struct SOFT3D_context *SC);
void Poly_Persp2x2_Tex_Gouraud_Fog(struct SOFT3D_context *SC);
void Edge_Persp_Tex_Gouraud_Fog(struct SOFT3D_context *SC);
void Edge_Tex_Gouraud_Fog(struct SOFT3D_context *SC);
void Edge_Gouraud(struct SOFT3D_context *SC);
void Edge_Flat(struct SOFT3D_context *SC);
void Edge_Persp_Tex(struct SOFT3D_context *SC);
void Edge_Tex(struct SOFT3D_context *SC);
void ChangeSoftPoint(APTR sc);
void SOFT3D_SetDrawFunctions(APTR sc);
/*==========================================================================*/
#ifdef __amigaos4__

#define OS4COMPOSITING 1
#include "soft3d_compositing.c"                /* use OS4 compositing engine as hardware renderer */

#endif
/*==========================================================================*/
void WriteImageBuffer(struct SOFT3D_context *SC)
{
#ifdef AMIGA
    if(SC->ImageBuffer32!=NULL)
    WritePixelArray(SC->ImageBuffer32,SC->xUpdate,SC->yUpdate,SC->large*(32/8),&SC->rastport,SC->xUpdate,SC->yUpdate+SC->yoffset,SC->largeUpdate,SC->highUpdate,RECTFMT_RGBA);
#endif
}
/*==========================================================================*/
void  SetImage(APTR sc,UWORD x,UWORD y,UWORD large,UWORD high,UWORD bits,UBYTE *Image8)
{
    struct SOFT3D_context *SC=sc;
    UWORD bpp;
    ULONG offset;

SFUNCTION(SetImage)
SVAR(Image8)
SVAR(x)
SVAR(y)
SVAR(large)
SVAR(high)
SVAR(bits)
    if(SC==NULL) return;
    if(Image8==NULL) return;


    offset     =y*large*bits/8 + x*bits/8;  /* Dont use x y offset in SOFT3D : use real pointer to Image */
    SC->Image8 =Image8=Image8+offset;
    SC->large  =large;
    SC->high   =high;
    SC->bits   =bits;
    bpp        =bits/8;
SVAR(offset)
SVAR(SC->Image8)

    SC->FragBufferMaxi=SC->FragBuffer + (FRAGBUFFERSIZE - SC->large - 4);

    SC->Pxmin=0;
    SC->Pymin=0;
    SC->Pxmax=large-1;
    SC->Pymax=high -1;

    YLOOP(high)
    {
        SC->edge1[y].L.Image8Y=Image8; SC->edge1[y].L.y=0; SC->edge1[y].W.y=y;SC->edge1[y].L.bpp=bpp;
        SC->edge2[y].L.Image8Y=Image8; SC->edge2[y].L.y=0; SC->edge2[y].W.y=y;SC->edge2[y].L.bpp=bpp;
        SC->edgeM[y].L.Image8Y=Image8; SC->edgeM[y].L.y=0; SC->edgeM[y].W.y=y;SC->edgeM[y].L.bpp=bpp;
        Image8+=bpp*large;
    }
    XLOOP(MAXSCREEN)
    {
        SC->Image8X[x]=x*bpp;
    }

}
/*=============================================================*/
void SOFT3D_AllocImageBuffer(APTR sc,UWORD large,UWORD high)
{
#ifdef AMIGA
    struct SOFT3D_context *SC=sc;
    ULONG size;

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
        LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */

SFUNCTION(SOFT3D_AllocImageBuffer)
SVAR(large)
SVAR(high)
    if(SC->ImageBuffer32!=NULL)
        { SREM(changing ImageBuffer...); }

    FREEPTR(SC->ImageBuffer32);

    size=large*high*32/8;
    if(size!=0)
        SC->ImageBuffer32=MMmalloc(size,"SOFT3D_ImageBuffer32");
    SetImage(SC,0,0,large,high,32,(UBYTE *)SC->ImageBuffer32);
//    chunk_list_dump(&alloced_chunks, "Alloced");
//    chunk_list_dump(&freed_chunks, "Freed");

#else
    SFUNCTION(SOFT3D_AllocImageBuffer)
    Libprintf("WAZP3D/SOFT3D: cant use Soft to bitmap!!!\n");
#endif
}
/*=============================================================*/
void SOFT3D_Debug(APTR txt)
/* to do a printf from WinUAE */
{
    if(Wazp3D->UseDLL)
        Libprintf(txt);
}
/*=============================================================*/
BOOL SOFT3D_Init(void *exec)
{
SFUNCTION(SOFT3D_Init)
#ifdef AMIGA
#ifdef SOFT3DLIB
    firstME=NULL;    /* Tracked memory-allocation    */
    if(OpenAmigaLibraries(exec)==FALSE) return(FALSE);
    OpenSoft3DDLL();
#endif
#endif
    return(TRUE);
}
/*=============================================================*/
void SOFT3D_Close(void)
{
SFUNCTION(SOFT3D_Close)
#ifdef AMIGA
#ifdef SOFT3DLIB
    CloseSoft3DDLL();
    CloseAmigaLibraries();
#endif
#endif
    return;
}
/*=============================================================*/
APTR SOFT3D_AllocZbuffer(APTR sc,UWORD large,UWORD high)
{
struct SOFT3D_context *SC=sc;
ZBUFF *Zbuffer=NULL;
ULONG y;

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(SOFT3D_AllocZbuffer)
SVAR(large)
SVAR(high)
    if(!SC->UseHard)
    {
    if(SC->Zbuffer!=NULL)    /* only happen in software mode */
        { SREM(will change an existing Zbuffer...); }

    FREEPTR(SC->Zbuffer);

    if(high!=0)
    if(large!=0)
        SC->Zbuffer=Zbuffer=MMmalloc(large*high*sizeof(ZBUFF),"Zbuffer");
//    chunk_list_dump(&alloced_chunks, "Alloced");
//    chunk_list_dump(&freed_chunks, "Freed");

    if(Zbuffer!=NULL)
    YLOOP(high)
        { SC->edge1[y].L.ZbufferY = SC->edge2[y].L.ZbufferY =SC->edgeM[y].L.ZbufferY =Zbuffer;Zbuffer+=large;}
    }
#ifdef USEOPENGL
    if(SC->UseHard)
    {
    HARD3D_AllocZbuffer(&SC->HC,large,high);
    SC->Zbuffer=NULL;            /* hard cant give the zbuffer adress */
    }
#endif
    return(SC->Zbuffer);
}
/*=============================================================*/
void *SOFT3D_Start(APTR PrefsWazp3D)
{
struct SOFT3D_context *SC;
UBYTE ZMode,FillMode;
ULONG n;
	DEBUG_SOFT3D("%s start\n",__FUNCTION__);

#ifdef USEOPENGL
struct HARD3D_context *HC;
#endif

#ifdef SLOWCPU
UWORD x,y;
union oper3D Oper;
#endif

Wazp3D=PrefsWazp3D;
if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
Wazp3D->UseDLL=FALSE;            /* then we are runnig soft3d compiled as a PC DLL */

#ifdef SOFT3DLIB
    Wazp3D=PrefsWazp3D;
    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
#ifdef AMIGA
    Wazp3D->UseDLL=FALSE;            /* will inform Wazp3D that soft3d is Amiga native */
#else
    Wazp3D->UseDLL=TRUE;            /* then we are runnig soft3d compiled as a PC DLL */
#endif
#endif

SVAR(sizeof(union pixel3D))
SVAR(sizeof(union oper3D ))
SVAR(sizeof(struct fragbuffer3D))
SVAR(sizeof(struct HARD3D_context))
SVAR(sizeof(struct SOFT3D_context))
SVAR(sizeof(struct SOFT3D_mipmap))
SVAR(sizeof(struct SOFT3D_texture))
SVAR(sizeof(struct state3D))


SFUNCTION(SOFT3D_Start)
SVAR(PrefsWazp3D)

    SC=MMmalloc(sizeof(struct SOFT3D_context),"SOFT3D_context");
    if(SC==NULL) return(NULL);
    SC->large=SC->high=0;    /* undefined now */

//    chunk_list_dump(&alloced_chunks, "Alloced");
//    chunk_list_dump(&freed_chunks, "Freed");

#ifdef AMIGA
    InitRastPort(&SC->rastport);
#endif

/* ZCompareMode is from 1 to 8 */
    ZMode=ZMODE(0,W3D_Z_NEVER);    Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_znever_update;  /* same as Ztest_znever */
    ZMode=ZMODE(0,W3D_Z_LESS);     Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zless;
    ZMode=ZMODE(0,W3D_Z_GEQUAL);   Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zgequal;
    ZMode=ZMODE(0,W3D_Z_LEQUAL);   Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zlequal;
    ZMode=ZMODE(0,W3D_Z_GREATER);  Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zgreater;
    ZMode=ZMODE(0,W3D_Z_NOTEQUAL); Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_znotequal;
    ZMode=ZMODE(0,W3D_Z_EQUAL);    Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zequal;
    ZMode=ZMODE(0,W3D_Z_ALWAYS);   Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zalways;

    ZMode=ZMODE(1,W3D_Z_NEVER);    Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_znever_update;
    ZMode=ZMODE(1,W3D_Z_LESS);     Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zless_update;
    ZMode=ZMODE(1,W3D_Z_GEQUAL);   Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zgequal_update;
    ZMode=ZMODE(1,W3D_Z_LEQUAL);   Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zlequal_update;
    ZMode=ZMODE(1,W3D_Z_GREATER);  Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zgreater_update;
    ZMode=ZMODE(1,W3D_Z_NOTEQUAL); Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_znotequal_update;
    ZMode=ZMODE(1,W3D_Z_EQUAL);    Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zequal_update;
    ZMode=ZMODE(1,W3D_Z_ALWAYS);   Functions.Ztest[ZMode]=(HOOKEDFUNCTION)Ztest_zalways_update;

    FillMode=(FALSE*4 +  TRUE*2 +  TRUE*1 );    Functions.Fill[FillMode]=(HOOKEDFUNCTION)Fill_Gouraud_Fog;
    FillMode=(FALSE*4 +  TRUE*2 + FALSE*1 );    Functions.Fill[FillMode]=(HOOKEDFUNCTION)Fill_Gouraud;
    FillMode=(FALSE*4 + FALSE*2 +  TRUE*1 );    Functions.Fill[FillMode]=(HOOKEDFUNCTION)Fill_Fog;
    FillMode=(FALSE*4 + FALSE*2 + FALSE*1 );    Functions.Fill[FillMode]=(HOOKEDFUNCTION)Fill_Flat;

    FillMode=( TRUE*4 +  TRUE*2 +  TRUE*1 );    Functions.Fill[FillMode]=(HOOKEDFUNCTION)Fill_Tex_Gouraud_Fog;
    FillMode=( TRUE*4 +  TRUE*2 + FALSE*1 );    Functions.Fill[FillMode]=(HOOKEDFUNCTION)Fill_Tex_Gouraud;
    FillMode=( TRUE*4 + FALSE*2 +  TRUE*1 );    Functions.Fill[FillMode]=(HOOKEDFUNCTION)Fill_Tex_Fog;
    FillMode=( TRUE*4 + FALSE*2 + FALSE*1 );    Functions.Fill[FillMode]=(HOOKEDFUNCTION)Fill_Tex;

    Functions.Edge[0]=(HOOKEDFUNCTION)Edge_Flat;                   /* Flat                 */
    Functions.Edge[1]=(HOOKEDFUNCTION)Edge_Tex_Gouraud_Fog;        /* Fog                  */
    Functions.Edge[2]=(HOOKEDFUNCTION)Edge_Gouraud;                /* Gouraud              */
    Functions.Edge[3]=(HOOKEDFUNCTION)Edge_Tex_Gouraud_Fog;        /* Gouraud_Fog          */
    Functions.Edge[4]=(HOOKEDFUNCTION)Edge_Tex;                    /* Tex                  */
    Functions.Edge[5]=(HOOKEDFUNCTION)Edge_Tex_Gouraud_Fog;        /* Tex_Fog              */
    Functions.Edge[6]=(HOOKEDFUNCTION)Edge_Tex_Gouraud_Fog;        /* Tex_Gouraud          */
    Functions.Edge[7]=(HOOKEDFUNCTION)Edge_Tex_Gouraud_Fog;        /* Tex_Gouraud_Fog      */

    Functions.Edge[ 8]=(HOOKEDFUNCTION)Edge_Flat;                  /* Persp_Flat           */
    Functions.Edge[ 9]=(HOOKEDFUNCTION)Edge_Tex_Gouraud_Fog;       /* Persp_Fog            */
    Functions.Edge[10]=(HOOKEDFUNCTION)Edge_Gouraud;               /* Persp_Gouraud        */
    Functions.Edge[11]=(HOOKEDFUNCTION)Edge_Tex_Gouraud_Fog;       /* Persp_Gouraud_Fog    */
    Functions.Edge[12]=(HOOKEDFUNCTION)Edge_Persp_Tex;             /* Persp_Tex            */
    Functions.Edge[13]=(HOOKEDFUNCTION)Edge_Persp_Tex_Gouraud_Fog; /* Persp_Tex_Fog        */
    Functions.Edge[14]=(HOOKEDFUNCTION)Edge_Persp_Tex_Gouraud_Fog; /* Persp_Tex_Gouraud    */
    Functions.Edge[15]=(HOOKEDFUNCTION)Edge_Persp_Tex_Gouraud_Fog; /* Persp_Tex_Gouraud_Fog*/

    Functions.TexEnv[0            *2+0]=(HOOKEDFUNCTION)NULL;
    Functions.TexEnv[W3D_REPLACE  *2+0]=(HOOKEDFUNCTION)PixelsReplace24;
    Functions.TexEnv[W3D_REPLACE  *2+1]=(HOOKEDFUNCTION)PixelsReplace32;
    Functions.TexEnv[W3D_MODULATE *2+0]=(HOOKEDFUNCTION)PixelsModulate24;
    Functions.TexEnv[W3D_MODULATE *2+1]=(HOOKEDFUNCTION)PixelsModulate32;
    Functions.TexEnv[W3D_DECAL    *2+0]=(HOOKEDFUNCTION)PixelsDecal24;
    Functions.TexEnv[W3D_DECAL    *2+1]=(HOOKEDFUNCTION)PixelsDecal32;
    Functions.TexEnv[W3D_BLEND    *2+0]=(HOOKEDFUNCTION)PixelsBlend24;
    Functions.TexEnv[W3D_BLEND    *2+1]=(HOOKEDFUNCTION)PixelsBlend32;
#ifdef WARP3DV5
    Functions.TexEnv[W3D_ADD      *2+0]=(HOOKEDFUNCTION)PixelsAdd24;
    Functions.TexEnv[W3D_ADD      *2+1]=(HOOKEDFUNCTION)PixelsAdd32;
    Functions.TexEnv[W3D_SUB      *2+0]=(HOOKEDFUNCTION)PixelsSub24;
    Functions.TexEnv[W3D_SUB      *2+1]=(HOOKEDFUNCTION)PixelsSub32;
#endif

    SC->FunctionWriteImageBuffer=(HOOKEDFUNCTION)WriteImageBuffer;

/* default set to "no fast blend function " */
    NLOOP(256)
        Functions.BlendFast[n]=(HOOKEDFUNCTION)NULL;

    Functions.BlendFast[W3D_ONE*16 + W3D_ZERO]=                      (HOOKEDFUNCTION)PixelsOne_Zero32;                  /* classic Replace */
    Functions.BlendFast[W3D_SRC_ALPHA*16 + W3D_ONE_MINUS_SRC_ALPHA]= (HOOKEDFUNCTION)PixelsSrcAlpha_OneMinusSrcAlpha32; /* classic Alpha transparency */
    Functions.BlendFast[W3D_SRC_ALPHA*16 + W3D_ONE]=                 (HOOKEDFUNCTION)PixelsSrcAlpha_One32;              /* Glxcess' transparency */
    Functions.BlendFast[W3D_ONE*16 + W3D_ONE]=                       (HOOKEDFUNCTION)PixelsOne_One24;                   /* Quake's glow effect */
    Functions.BlendFast[W3D_ZERO*16 + W3D_SRC_ALPHA]=                (HOOKEDFUNCTION)PixelsZero_SrcAlpha32;             /* Quake's lightmap effect */
    Functions.BlendFast[W3D_ZERO*16 + W3D_SRC_COLOR]=                (HOOKEDFUNCTION)PixelsZero_SrcColor24;             /* Modulate effect */
    Functions.BlendFast[W3D_DST_COLOR*16 + W3D_ZERO]=                (HOOKEDFUNCTION)PixelsDstColor_Zero24;             /* modulate effect = same as M */
    Functions.BlendFast[W3D_ZERO*16 + W3D_ONE_MINUS_SRC_COLOR]=      (HOOKEDFUNCTION)PixelsZero_OneMinusSrcColor24;     /* Coloring for demo Glxcess */
    Functions.BlendFast[W3D_SRC_ALPHA*16 + W3D_ONE_MINUS_SRC_COLOR]= (HOOKEDFUNCTION)PixelsSrcAlpha_OneMinusSrcColor32; /* coloring for demo Glxcess */
    Functions.BlendFast[W3D_ONE*16 + W3D_ONE_MINUS_SRC_ALPHA]=       (HOOKEDFUNCTION)PixelsOne_OneMinusSrcAlpha32;      /* GlTron transparency */

/* v51: fast simple chromatest : color not black */
    Functions.BlendFast[BLENDCHROMA]=                       (HOOKEDFUNCTION)PixelsChroma32fast;

    Functions.BlendFast[BLENDFASTALPHA]=                    (HOOKEDFUNCTION)PixelsSrcAlpha_OneMinusSrcAlpha32fast;


#ifdef SLOWCPU
/* compute the precalculated tables */
    Oper.L.Index=0;
    XLOOP(256)
    YLOOP(256)
    {
    Oper.B.a=x;
    Oper.B.b=y;
    SC->Mul8[Oper.L.Index]=((x*y)/255);
    if((x+y)<256)
        SC->Add8[Oper.L.Index]=x+y;
    else
        SC->Add8[Oper.L.Index]=255;
    SC->Fil8[Oper.L.Index]=(x+y)/2;
    }
#endif


/* fragments buffer sizes */
    SC->FragBufferDone=SC->FragBuffer;
    SC->FragBufferMaxi=SC->FragBuffer + (FRAGBUFFERSIZE-MAXSCREEN-4);

    SC->UseHard=FALSE;
#if defined(OS4COMPOSITING)
    SC->UseHard=(Wazp3D->Renderer.ON==2);        /* 2= Compositing so will disable soft renderering */
    if(SC->UseHard) COMP3D_Start(SC);
#endif


#ifdef USEOPENGL
    HC=&SC->HC;
    SC->UseHard=(Wazp3D->Renderer.ON>=2);         /* as 2 mean "use hard" */
    if(SC->UseHard)
    {
    HC->UseAntiAlias        = (Wazp3D->UseAntiImage.ON  );
    HC->UseOverlay        = (Wazp3D->Renderer.ON==3   );
    HC->DebugHard         = (Wazp3D->DebugSC.ON==TRUE );
    HC->awin            =  Wazp3D->window;
    HARD3D_Start(&SC->HC);
    }
#endif
    SC->Tnb=0;            /* texture number */

#if !defined(AMIGA)                     /* so we are compiling the DLL */
    Wazp3D->UseRatioAlpha.ON=FALSE;
    Wazp3D->UseAlphaMinMax.ON=FALSE;
    Wazp3D->TexMode.ON=1;            /* we assume that PC cpu is fast enough to enable nice features */
    Wazp3D->PerspMode.ON=2;
#endif

SREM(SOFT3D_Start:OK)
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
    return(SC);
}
/*=================================================================*/
void  SOFT3D_SetClipping(APTR sc,UWORD xmin,UWORD xmax,UWORD ymin,UWORD ymax)
{
struct SOFT3D_context *SC=sc;
    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(SOFT3D_SetClipping)
SVAR(xmin)
SVAR(xmax)
SVAR(ymin)
SVAR(ymax)
    SC->ClipMin.x=xmin;
    SC->ClipMax.x=xmax;
    SC->ClipMin.y=ymin;
    SC->ClipMax.y=ymax;
    SC->ClipMin.z=0.0;
    SC->ClipMax.z=1.0;
#ifdef USEOPENGL
    if(SC->UseHard) HARD3D_SetClipping(&SC->HC,xmin,xmax,ymin,ymax);
#endif
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*=============================================================*/
void SOFT3D_ClearImageBuffer(APTR sc,UWORD x,UWORD y,UWORD large,UWORD high,APTR rgba)
{
struct SOFT3D_context *SC=sc;
UBYTE *RGBA=rgba;
ULONG *ptRGBA32=(ULONG *)RGBA;
register ULONG RGBA32=ptRGBA32[0];
register ULONG *I32;
register LONG  size;
register LONG  n;
    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(SOFT3D_ClearImageBuffer)
    if(SC->ImageBuffer32==NULL) return;
SVAR(SC->Image8)
SVAR(x)
SVAR(y)
SVAR(large)
SVAR(high)
SVAR(SC->large)

#ifdef USEOPENGL
    ;    /* do nothing as the hardware cant use an existing ImageBuffer32 */
#endif
    SREM(ImageBuffer32 present)
    I32=&SC->ImageBuffer32[y*SC->large];
SVAR(I32)
    size=SC->large*high/8;
    if(RGBA32==0)
    NLOOP(size)
    {
        I32[0]=0;I32[1]=0;I32[2]=0;I32[3]=0;
        I32[4]=0;I32[5]=0;I32[6]=0;I32[7]=0;
        I32+=8;
    }
    else
    NLOOP(size)
    {
        I32[0]=RGBA32;I32[1]=RGBA32;I32[2]=RGBA32;I32[3]=RGBA32;
        I32[4]=RGBA32;I32[5]=RGBA32;I32[6]=RGBA32;I32[7]=RGBA32;
        I32+=8;
    }

    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*=============================================================*/
void SOFT3D_ClearZBuffer(APTR sc,float fz)
{
struct SOFT3D_context *SC=sc;
register ZBUFF *Zbuffer;
register ULONG size=SC->high*SC->large/8;
register ULONG n;
register ZBUFF z;
register float zresize=ZRESIZE;
    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */

SFUNCTION(SOFT3D_ClearZBuffer)

    if(SC->Zbuffer!=NULL)    /* only happen in software mode */
    {
    Zbuffer=SC->Zbuffer;
    SVARF(fz)
    z=fz*zresize;
    if (z < MINZ)    z=MINZ;
    if (MAXZ < z)    z=MAXZ;
    SVARF(z)
    NLOOP(size)
        {
        Zbuffer[0]=z;    Zbuffer[1]=z;    Zbuffer[2]=z; Zbuffer[3]=z;
        Zbuffer[4]=z;    Zbuffer[5]=z;    Zbuffer[6]=z; Zbuffer[7]=z;
        Zbuffer+=8;
        }
    }

#ifdef USEOPENGL
    if(SC->UseHard) HARD3D_ClearZBuffer(&SC->HC,fz);
#endif
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*==========================================================================*/
void SOFT3D_ReadZSpan(APTR sc, UWORD x, UWORD y,ULONG n, APTR z)
{
struct SOFT3D_context *SC=sc;
register ZBUFF *Zbuffer;
W3D_Double *dz=z;
register float fz;
register float zresize=ZRESIZE;
register ULONG i;
    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(SOFT3D_ReadZSpan)
    if(SC->Zbuffer!=NULL)    /* only happen in software mode */
    {
    Zbuffer=SC->edge1[y].L.ZbufferY + x;
    ILOOP(n)
        {
        fz=(float)Zbuffer[i];
        fz=fz/zresize;
        dz[i]=(W3D_Double)fz;
        }
    }
#ifdef USEOPENGL
    if(SC->UseHard) HARD3D_ReadZSpan(&SC->HC,x,y,n,z);
#endif
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*==========================================================================*/
void SOFT3D_WriteZSpan(APTR sc, UWORD x, UWORD y,ULONG n,APTR z,APTR mask)
{
struct SOFT3D_context *SC=sc;
register ZBUFF *Zbuffer;
register W3D_Double *dz=z;
register float fz;
register float zresize=ZRESIZE;
register UBYTE *m=mask;
register ULONG i;

    DEBUG_SOFT3D("%s start\n",__FUNCTION__);
    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(SOFT3D_WriteZSpan)
    if(SC->Zbuffer!=NULL)    /* only happen in software mode */
    {
    Zbuffer=SC->edge1[y].L.ZbufferY + x;
    ILOOP(n)
        if(m[i]!=0)
            {
            fz=dz[i];
            if (fz < MINZ)    fz=MINZ;
            if (MAXZ < fz)    fz=MAXZ;
            Zbuffer[i]=fz*zresize;
            }
    }
#ifdef USEOPENGL
    if(SC->UseHard) HARD3D_WriteZSpan(&SC->HC,x,y,n,z,mask);
#endif
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*=============================================================*/
void SOFT3D_End(APTR sc)
{
    struct SOFT3D_context *SC=sc;

    DEBUG_SOFT3D("%s start\n",__FUNCTION__);
    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
        LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(SOFT3D_End)
    if(SC==NULL) return;
    if(SC->ImageBuffer32!=NULL)
    {
        SREM(Free ImageBuffer32)
		DEBUG_SOFT3D("Free ImageBuffer32 0x%08lx\n",(uint32_t)SC->ImageBuffer32);
        FREEPTR(SC->ImageBuffer32);
    }
#ifdef USEOPENGL
    if(SC->UseHard) HARD3D_End(&SC->HC);
#endif
#if defined(OS4COMPOSITING)
    if(SC->UseHard) COMP3D_End(SC);
#endif
    DEBUG_SOFT3D("Free SC\n");
    FREEPTR(SC);
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*=============================================================*/
#ifdef SLOWCPU

#define ADD8(a1,b1,dst)                Add.B.b=b1;  Add.B.a=a1;  dst=Add8[Add.L.Index];
#define MUL8(a1,b1,dst)                Mul.B.b=b1;  Mul.B.a=a1;  dst=Mul8[Mul.L.Index];
#define NEXTADDMUL8(b1,a1,b2,a2,dst)    Mul.B.b=b1;  Mul2.B.b=b2; dst=Mul8[Mul.L.Index]+Mul8[Mul2.L.Index];
#define FULLADDMUL8(b1,a1,b2,a2,dst)    Mul.B.a=a1;  Mul2.B.a=a2; NEXTADDMUL8(b1,a1,b2,a2,dst)
#define FOG8X3(fog,dst)                Mul.B.a=fog[3]; Mul.B.b=dst[0]; dst[0]=fog[0]+Mul8[Mul.L.Index]; Mul.B.b=dst[1];dst[1]=fog[1]+Mul8[Mul.L.Index]; Mul.B.b=dst[2]; dst[2]=fog[2]+Mul8[Mul.L.Index];
#define FIL8(a1,b1,dst)                Fil.B.a=a1;  Fil.B.b=b1;  dst=Fil8[Fil.L.Index];

#else        /* On x86 dont use the precalculated tables = just compute */

#define ADD8(a1,b1,dst)                if( ((UWORD)(b1)+(UWORD)(a1)) < 255)  dst=((UWORD)(b1)+(UWORD)(a1)); else dst=255;
#define MUL8(a1,b1,dst)                dst=( ((UWORD)(b1)*(UWORD)(a1)) /255);
#define NEXTADDMUL8(b1,a1,b2,a2,dst)    dst=( ((UWORD)(b1)*(UWORD)(a1)) /255) + ( ((UWORD)(b2)*(UWORD)(a2)) /255);
#define FULLADDMUL8(b1,a1,b2,a2,dst)    dst=( ((UWORD)(b1)*(UWORD)(a1)) /255) + ( ((UWORD)(b2)*(UWORD)(a2)) /255);
#define FOG8X3(fog,dst)                dst[0]=fog[0]+( ((UWORD)(fog[3])*(UWORD)(dst[0])) /255); dst[1]=fog[1]+( ((UWORD)(fog[3])*(UWORD)(dst[1])) /255); dst[2]=fog[2]+( ((UWORD)(fog[3])*(UWORD)(dst[2])) /255);
#define FIL8(a1,b1,dst)                dst=((UWORD)(b1)+(UWORD)(a1))/2;

#endif
/*=============================================================*/
/* since v34:new pixels-functions to do multi-pass blending    */
/* see OpenGL doc Ct=texture Cf=color Cc=env*/
/*
GL_TEXTURE_ENV_MODE:
    GL_REPLACE    C = Ct            A = At
    GL_MODULATE    C = Ct * Cf            A = At * Af
    GL_BLEND    C = Cf*(1-Ct) + Cc*Ct    A = Af * At
    GL_DECAL    C = Cf*(1-At) + Ct*At    A = Af
f = fragment, t = texture, c = GL_TEXTURE_ENV_COLOR
glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,param);
glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,param);
*/
/*=============================================================*/
#define CT1R Frag[0].Tex8[0]
#define CT1G Frag[0].Tex8[1]
#define CT1B Frag[0].Tex8[2]
#define AT1  Frag[0].Tex8[3]
#define CF1R Frag[0].ColorRGBA.b[0]
#define CF1G Frag[0].ColorRGBA.b[1]
#define CF1B Frag[0].ColorRGBA.b[2]
#define AF1  Frag[0].ColorRGBA.b[3]

#define CT2R Frag[1].Tex8[0]
#define CT2G Frag[1].Tex8[1]
#define CT2B Frag[1].Tex8[2]
#define AT2  Frag[1].Tex8[3]
#define CF2R Frag[1].ColorRGBA.b[0]
#define CF2G Frag[1].ColorRGBA.b[1]
#define CF2B Frag[1].ColorRGBA.b[2]
#define AF2  Frag[1].ColorRGBA.b[3]

#define CCR EnvRGBA[0]
#define CCG EnvRGBA[1]
#define CCB EnvRGBA[2]

#define AT Ct[3]
#define AF Cf[3]
#define ONE 255
/*=============================================================*/
void PixelsTex32ToImage(struct SOFT3D_context *SC)
{
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

SREM(PixelsTex32ToImage)
    while(0<size--)
    {
    COPYRGBA(Frag[0].Image8,Frag[0].Tex8);
    COPYRGBA(Frag[1].Image8,Frag[1].Tex8);
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsTex24ToImage(struct SOFT3D_context *SC)
{
    register struct fragbuffer3D *Frag=SC->FragBuffer;
    register ULONG size=SC->FragSize2;
    register union rgba3D Color;
    register ULONG *dst32;

    SREM(PixelsTex24ToImage)
    Color.B.RGBA[3]=ONE;
    while(0<size--)
    {
        Color.B.RGBA[0]=Frag[0].Tex8[0];
        Color.B.RGBA[1]=Frag[0].Tex8[1];
        Color.B.RGBA[2]=Frag[0].Tex8[2];
        dst32=(ULONG *)Frag[0].Image8;
        *dst32=Color.L.RGBA32;

        Color.B.RGBA[0]=Frag[1].Tex8[0];
        Color.B.RGBA[1]=Frag[1].Tex8[1];
        Color.B.RGBA[2]=Frag[1].Tex8[2];
        dst32=(ULONG *)Frag[1].Image8;
        *dst32=Color.L.RGBA32;
        Frag+=2;
    }
}
/*=============================================================*/
void PixelsColorToImage(struct SOFT3D_context *SC)
{
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

SREM(PixelsColorToImage)
    while(0<size--)
    {
    COPYRGBA(Frag[0].Image8,Frag[0].ColorRGBA.L);
    COPYRGBA(Frag[1].Image8,Frag[1].ColorRGBA.L);
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsTex32ToBuffer(struct SOFT3D_context *SC)
{
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

SREM(PixelsTex32ToBuffer)
    while(0<size--)
    {
    COPYRGBA(Frag[0].BufferRGBA.L,Frag[0].Tex8);
    COPYRGBA(Frag[1].BufferRGBA.L,Frag[1].Tex8);
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsTex24ToBuffer(struct SOFT3D_context *SC)
{
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register union rgba3D Color;
register ULONG *dst32;

SREM(PixelsTex24ToBuffer)
    Color.B.RGBA[3]=ONE;
    while(0<size--)
    {
    Color.B.RGBA[0]=Frag[0].Tex8[0];
    Color.B.RGBA[1]=Frag[0].Tex8[1];
    Color.B.RGBA[2]=Frag[0].Tex8[2];
    dst32=Frag[0].BufferRGBA.L;
    *dst32=Color.L.RGBA32;

    Color.B.RGBA[0]=Frag[1].Tex8[0];
    Color.B.RGBA[1]=Frag[1].Tex8[1];
    Color.B.RGBA[2]=Frag[1].Tex8[2];
    dst32=(ULONG *)Frag[1].BufferRGBA.L;
    *dst32=Color.L.RGBA32;
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsColorToBuffer(struct SOFT3D_context *SC)
{
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

SREM(PixelsColorToBuffer)
    while(0<size--)
    {
    COPYRGBA(Frag[0].BufferRGBA.L,Frag[0].ColorRGBA.L);
    COPYRGBA(Frag[1].BufferRGBA.L,Frag[1].ColorRGBA.L);
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsReplace24(struct SOFT3D_context *SC)
{
/* replace color=tex  (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register union rgba3D Color;
register ULONG *dst32;

SREM(PixelsReplace24)
    Color.B.RGBA[3]=ONE;
    while(0<size--)
    {
    Color.B.RGBA[0]=Frag[0].Tex8[0];
    Color.B.RGBA[1]=Frag[0].Tex8[1];
    Color.B.RGBA[2]=Frag[0].Tex8[2];
    dst32=(ULONG *)Frag[0].ColorRGBA.L;
    *dst32=Color.L.RGBA32;

    Color.B.RGBA[0]=Frag[1].Tex8[0];
    Color.B.RGBA[1]=Frag[1].Tex8[1];
    Color.B.RGBA[2]=Frag[1].Tex8[2];
    dst32=(ULONG *)Frag[1].ColorRGBA.L;
    *dst32=Color.L.RGBA32;
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsReplace32(struct SOFT3D_context *SC)
{
/* replace color=tex  (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

SREM(PixelsReplace32)
    while(0<size--)
    {
    COPYRGBA(Frag[0].ColorRGBA.L,Frag[0].Tex8);
    COPYRGBA(Frag[1].ColorRGBA.L,Frag[1].Tex8);
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsModulate24(struct SOFT3D_context *SC)
{
/* v40: modulate src & dst  (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;

    Mul.L.Index=0;
#endif
SREM(PixelsModulate24)
    while(0<size--)
    {
    MUL8(CT1R,CF1R,CF1R)
    MUL8(CT1G,CF1G,CF1G)
    MUL8(CT1B,CF1B,CF1B)

    MUL8(CT2R,CF2R,CF2R)
    MUL8(CT2G,CF2G,CF2G)
    MUL8(CT2B,CF2B,CF2B)
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsModulate32(struct SOFT3D_context *SC)
{
/* v40: modulate src & dst  (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;

    Mul.L.Index=0;
#endif

SREM(PixelsModulate32)
    while(0<size--)
    {
    MUL8(CT1R,CF1R,CF1R)
    MUL8(CT1G,CF1G,CF1G)
    MUL8(CT1B,CF1B,CF1B)
    MUL8(AT1,AF1,AF1)

    MUL8(CT2R,CF2R,CF2R)
    MUL8(CT2G,CF2G,CF2G)
    MUL8(CT2B,CF2B,CF2B)
    MUL8(AT2,AF2,AF2)
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsDecal24(struct SOFT3D_context *SC)
{
/* v40: decal src & dst  (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

SREM(Pixels24DEC)
    while(0<size--)
    {
    CF1R=CT1R;
    CF1G=CT1G;
    CF1B=CT1B;
    AF1 =ONE;

    CF2R=CT2R;
    CF2G=CT2G;
    CF2B=CT2B;
    AF2 =ONE;
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsDecal32(struct SOFT3D_context *SC)
{
/* v40: decal src & dst  (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;
register union oper3D Mul2;

    Mul.L.Index=Mul2.L.Index=0;
#endif

SREM(PixelsDecal32)
    while(0<size--)
    {
    FULLADDMUL8(CT1R,AT1,CF1R,ONE-AT1,CF1R)
    FULLADDMUL8(CT1G,AT1,CF1G,ONE-AT1,CF1G)
    FULLADDMUL8(CT1B,AT1,CF1B,ONE-AT1,CF1B)
/*    AF1=AF1; */        /* alpha from color */

    FULLADDMUL8(CT2R,AT2,CF2R,ONE-AT2,CF2R)
    FULLADDMUL8(CT2G,AT2,CF2G,ONE-AT2,CF2G)
    FULLADDMUL8(CT2B,AT2,CF2B,ONE-AT2,CF2B)
/*    AF2=AF2; */        /* alpha from color */
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsBlend24(struct SOFT3D_context *SC)
{
/* v40: blend src & dst  & env (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register UBYTE *EnvRGBA=SC->state.EnvRGBA.b;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;
register union oper3D Mul2;

    Mul.L.Index=Mul2.L.Index=0;
#endif

SREM(PixelsBlend24)
    while(0<size--)
    {
    FULLADDMUL8(CF1R,ONE-CT1R,CCR,CT1R,CF1R)
    FULLADDMUL8(CF1G,ONE-CT1G,CCG,CT1G,CF1G)
    FULLADDMUL8(CF1B,ONE-CT1B,CCB,CT1B,CF1B)

    FULLADDMUL8(CF2R,ONE-CT2R,CCR,CT2R,CF2R)
    FULLADDMUL8(CF2G,ONE-CT2G,CCG,CT2G,CF2G)
    FULLADDMUL8(CF2B,ONE-CT2B,CCB,CT2B,CF2B)
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsBlend32(struct SOFT3D_context *SC)
{
/* v40: blend src & dst  & env (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register UBYTE *EnvRGBA=SC->state.EnvRGBA.b;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;
register union oper3D Mul2;

    Mul.L.Index=Mul2.L.Index=0;
#endif

SREM(PixelsBlend32)
    while(0<size--)
    {
    FULLADDMUL8(CF1R,ONE-CT1R,CCR,CT1R,CF1R)
    FULLADDMUL8(CF1G,ONE-CT1G,CCG,CT1G,CF1G)
    FULLADDMUL8(CF1B,ONE-CT1B,CCB,CT1B,CF1B)
    MUL8(AF1,AT1,AF1)

    FULLADDMUL8(CF2R,ONE-CT2R,CCR,CT2R,CF2R)
    FULLADDMUL8(CF2G,ONE-CT2G,CCG,CT2G,CF2G)
    FULLADDMUL8(CF2B,ONE-CT2B,CCB,CT2B,CF2B)
    MUL8(AF2,AT2,AF2)
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsAdd24(struct SOFT3D_context *SC)
{
/* v50: add src & dst  (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
#ifdef SLOWCPU
register UBYTE *Add8=SC->Add8;
register union oper3D Add;

    Add.L.Index=0;
#endif
SREM(PixelsAdd24)
    while(0<size--)
    {
    ADD8(CT1R,CF1R,CF1R)    /* do color=color+tex */
    ADD8(CT1G,CF1G,CF1G)
    ADD8(CT1B,CF1B,CF1B)

    ADD8(CT2R,CF2R,CF2R)
    ADD8(CT2G,CF2G,CF2G)
    ADD8(CT2B,CF2B,CF2B)
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsAdd32(struct SOFT3D_context *SC)
{
/* v50: add src & dst  (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
#ifdef SLOWCPU
register UBYTE *Add8=SC->Add8;
register union oper3D Add;

    Add.L.Index=0;
#endif

SREM(PixelsAdd32)
    while(0<size--)
    {
    ADD8(CT1R,CF1R,CF1R)    /* do color=color+tex */
    ADD8(CT1G,CF1G,CF1G)
    ADD8(CT1B,CF1B,CF1B)
    ADD8(AT1,AF1,AF1)

    ADD8(CT2R,CF2R,CF2R)
    ADD8(CT2G,CF2G,CF2G)
    ADD8(CT2B,CF2B,CF2B)
    ADD8(AT2,AF2,AF2)
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsSub24(struct SOFT3D_context *SC)
{
/* v50: sub src & dst  (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

SREM(PixelsSub24)
    while(0<size--)
    {
    if(CF1R>CT1R) CF1R=CF1R-CT1R; else CF1R=0;
    if(CF1G>CT1G) CF1G=CF1G-CT1G; else CF1G=0;
    if(CF1B>CT1B) CF1B=CF1B-CT1B; else CF1B=0;

    if(CF2R>CT2R) CF2R=CF2R-CT2R; else CF2R=0;
    if(CF2G>CT2G) CF2G=CF2G-CT2G; else CF2G=0;
    if(CF2B>CT2B) CF2B=CF2B-CT2B; else CF2B=0;
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsSub32(struct SOFT3D_context *SC)
{
/* v50: sub src & dst  (always) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

SREM(PixelsSub32)
    while(0<size--)
    {
    if(CF1R>CT1R) CF1R=CF1R-CT1R; else CF1R=0;
    if(CF1G>CT1G) CF1G=CF1G-CT1G; else CF1G=0;
    if(CF1B>CT1B) CF1B=CF1B-CT1B; else CF1B=0;
    if(AF1>AT1) AF1=AF1-AT1; else AF1=0;

    if(CF2R>CT2R) CF2R=CF2R-CT2R; else CF2R=0;
    if(CF2G>CT2G) CF2G=CF2G-CT2G; else CF2G=0;
    if(CF2B>CT2B) CF2B=CF2B-CT2B; else CF2B=0;
    if(AF2>AT2) AF2=AF2-AT2; else AF2=0;
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsFogToImage(struct SOFT3D_context *SC)
{
/* blend source & dest (always) with a fog color already weigthed to fog A    */
/* FogRGBA[3] is set to background transparency against fog    level            */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

UBYTE A;

#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;

register union oper3D Mul2;
    Mul2.L.Index=0;

    Mul.L.Index=0;
#endif

SREM(PixelsFogToBuffer)
    while(0<size--)
    {
/*    TODO: find why this dont works
    FOG8X3(Frag[0].FogRGBA,Frag[0].Image8);
    FOG8X3(Frag[1].FogRGBA,Frag[1].Image8);
*/
    A=Frag[0].FogRGBA.b[3];
    FULLADDMUL8(SC->state.FogRGBA.b[0],ONE-A,Frag[0].Image8[0],A,Frag[0].Image8[0])
    FULLADDMUL8(SC->state.FogRGBA.b[1],ONE-A,Frag[0].Image8[1],A,Frag[0].Image8[1])
    FULLADDMUL8(SC->state.FogRGBA.b[2],ONE-A,Frag[0].Image8[2],A,Frag[0].Image8[2])
    Frag[0].Image8[3]=255;

    A=Frag[1].FogRGBA.b[3];
    FULLADDMUL8(SC->state.FogRGBA.b[0],ONE-A,Frag[1].Image8[0],A,Frag[1].Image8[0])
    FULLADDMUL8(SC->state.FogRGBA.b[1],ONE-A,Frag[1].Image8[1],A,Frag[1].Image8[1])
    FULLADDMUL8(SC->state.FogRGBA.b[2],ONE-A,Frag[1].Image8[2],A,Frag[1].Image8[2])
    Frag[1].Image8[3]=255;

    Frag+=2;
    }
}
/*=============================================================*/
void PixelsFogToBuffer(struct SOFT3D_context *SC)
{
/* blend source & dest (always) with a fog color already weigthed to fog A    */
/* FogRGBA[3] is set to background transparency against fog    level            */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

UBYTE A;

#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;

register union oper3D Mul2;
    Mul2.L.Index=0;

    Mul.L.Index=0;
#endif

SREM(PixelsFogToBuffer)
    while(0<size--)
    {
/*    TODO: find why this dont works
    FOG8X3(Frag[0].FogRGBA,Frag[0].BufferRGBA);
    FOG8X3(Frag[1].FogRGBA,Frag[1].BufferRGBA);
*/
    A=Frag[0].FogRGBA.b[3];
    FULLADDMUL8(SC->state.FogRGBA.b[0],ONE-A,Frag[0].BufferRGBA.b[0],A,Frag[0].BufferRGBA.b[0])
    FULLADDMUL8(SC->state.FogRGBA.b[1],ONE-A,Frag[0].BufferRGBA.b[1],A,Frag[0].BufferRGBA.b[1])
    FULLADDMUL8(SC->state.FogRGBA.b[2],ONE-A,Frag[0].BufferRGBA.b[2],A,Frag[0].BufferRGBA.b[2])
    Frag[0].BufferRGBA.b[3]=255;

    A=Frag[1].FogRGBA.b[3];
    FULLADDMUL8(SC->state.FogRGBA.b[0],ONE-A,Frag[1].BufferRGBA.b[0],A,Frag[1].BufferRGBA.b[0])
    FULLADDMUL8(SC->state.FogRGBA.b[1],ONE-A,Frag[1].BufferRGBA.b[1],A,Frag[1].BufferRGBA.b[1])
    FULLADDMUL8(SC->state.FogRGBA.b[2],ONE-A,Frag[1].BufferRGBA.b[2],A,Frag[1].BufferRGBA.b[2])
    Frag[1].BufferRGBA.b[3]=255;

    Frag+=2;
    }
}
/*=============================================================*/
void PixelsSepiaToImage(struct SOFT3D_context *SC)
{
    /* colorize in red the pixels     */
    register struct fragbuffer3D *Frag=SC->FragBuffer;
    register ULONG size=SC->FragSize2;
    register UWORD r;

    SREM(PixelsSepiaToImage)
    while(0<size--)
    {
        r=Frag[0].Image8[0]+Frag[0].Image8[1]+Frag[0].Image8[2];
        Frag[0].Image8[0]=255;
        if(r<=255*2)
            Frag[0].Image8[0]=r/2;
        Frag[0].Image8[1]=r/4;
        Frag[0].Image8[2]=r/6;

        r=Frag[1].Image8[0]+Frag[1].Image8[1]+Frag[1].Image8[2];
        Frag[1].Image8[0]=255;
        if(r<=255*2)
            Frag[1].Image8[0]=r/2;
        Frag[1].Image8[1]=r/4;
        Frag[1].Image8[2]=r/6;
        Frag+=2;
    }
}
/*=============================================================*/
void PixelsSepiaToBuffer(struct SOFT3D_context *SC)
{
/* colorize in red the pixels     */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register UWORD r;

SREM(PixelsSepiaToBuffer)
    while(0<size--)
    {
    r=Frag[0].BufferRGBA.b[0]+Frag[0].BufferRGBA.b[1]+Frag[0].BufferRGBA.b[2];
    Frag[0].BufferRGBA.b[0]=255;
    if(r<=255*2)
        Frag[0].BufferRGBA.b[0]=r/2;
    Frag[0].BufferRGBA.b[1]=r/4;
    Frag[0].BufferRGBA.b[2]=r/6;

    r=Frag[1].BufferRGBA.b[0]+Frag[1].BufferRGBA.b[1]+Frag[1].BufferRGBA.b[2];
    Frag[1].BufferRGBA.b[0]=255;
    if(r<=255*2)
        Frag[1].BufferRGBA.b[0]=r/2;
    Frag[1].BufferRGBA.b[1]=r/4;
    Frag[1].BufferRGBA.b[2]=r/6;
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsFilterToImage(struct SOFT3D_context *SC)
{
/* colorize in red the pixels     */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register LONG size=SC->FragSize2-4;
#ifdef SLOWCPU
register UBYTE *Fil8=SC->Fil8;
register union oper3D Fil;

    Fil.L.Index=0;
#endif

SREM(PixelsFilterToImage)
    while(0<size--)
    {

    if( (Frag[1].Image8-Frag[0].Image8) <= 4)    /* if pixels are contiguous ? TODO: remove this slow test */
    {
    FIL8(Frag[0].Image8[0],Frag[1].Image8[0],Frag[0].Image8[0]);
    FIL8(Frag[0].Image8[1],Frag[1].Image8[1],Frag[0].Image8[1]);
    FIL8(Frag[0].Image8[2],Frag[1].Image8[2],Frag[0].Image8[2]);
    }

    if( (Frag[2].Image8-Frag[1].Image8) <= 4)
    {
    FIL8(Frag[1].Image8[0],Frag[2].Image8[0],Frag[1].Image8[0]);
    FIL8(Frag[1].Image8[1],Frag[2].Image8[1],Frag[1].Image8[1]);
    FIL8(Frag[1].Image8[2],Frag[2].Image8[2],Frag[1].Image8[2]);
    }

    Frag+=2;
    }
}
/*=============================================================*/
void PixelsFilterToBuffer(struct SOFT3D_context *SC)
{
/* colorize in red the pixels     */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register LONG size=SC->FragSize2-4;
#ifdef SLOWCPU
register UBYTE *Fil8=SC->Fil8;
register union oper3D Fil;

    Fil.L.Index=0;
#endif

SREM(PixelsFilterToBuffer)
    while(0<size--)
    {
    if( (Frag[1].Image8-Frag[0].Image8) <= 4)
    {
    FIL8(Frag[0].BufferRGBA.b[0],Frag[1].BufferRGBA.b[0],Frag[0].BufferRGBA.b[0]);
    FIL8(Frag[0].BufferRGBA.b[1],Frag[1].BufferRGBA.b[1],Frag[0].BufferRGBA.b[1]);
    FIL8(Frag[0].BufferRGBA.b[2],Frag[1].BufferRGBA.b[2],Frag[0].BufferRGBA.b[2]);
    }

    if( (Frag[2].Image8-Frag[1].Image8) <= 4)
    {
    FIL8(Frag[1].BufferRGBA.b[0],Frag[2].BufferRGBA.b[0],Frag[1].BufferRGBA.b[0]);
    FIL8(Frag[1].BufferRGBA.b[1],Frag[2].BufferRGBA.b[1],Frag[1].BufferRGBA.b[1]);
    FIL8(Frag[1].BufferRGBA.b[2],Frag[2].BufferRGBA.b[2],Frag[1].BufferRGBA.b[2]);
    }

    Frag+=2;
    }
}
/*=============================================================*/
/* define all that to make formulas more easy to read */
#define SRC1  Frag[0].ColorRGBA.L
#define SRC1R Frag[0].ColorRGBA.b[0]
#define SRC1G Frag[0].ColorRGBA.b[1]
#define SRC1B Frag[0].ColorRGBA.b[2]
#define SRC1A Frag[0].ColorRGBA.b[3]

#define DST1  Frag[0].BufferRGBA.L
#define DST1R Frag[0].BufferRGBA.b[0]
#define DST1G Frag[0].BufferRGBA.b[1]
#define DST1B Frag[0].BufferRGBA.b[2]
#define DST1A Frag[0].BufferRGBA.b[3]

#define TMP1  Frag[0].TmpRGBA.L
#define TMP1R Frag[0].TmpRGBA.b[0]
#define TMP1G Frag[0].TmpRGBA.b[1]
#define TMP1B Frag[0].TmpRGBA.b[2]
#define TMP1A Frag[0].TmpRGBA.b[3]

#define SRC2  Frag[1].ColorRGBA.L
#define SRC2R Frag[1].ColorRGBA.b[0]
#define SRC2G Frag[1].ColorRGBA.b[1]
#define SRC2B Frag[1].ColorRGBA.b[2]
#define SRC2A Frag[1].ColorRGBA.b[3]

#define DST2  Frag[1].BufferRGBA.L
#define DST2R Frag[1].BufferRGBA.b[0]
#define DST2G Frag[1].BufferRGBA.b[1]
#define DST2B Frag[1].BufferRGBA.b[2]
#define DST2A Frag[1].BufferRGBA.b[3]

#define TMP2  Frag[1].TmpRGBA.L
#define TMP2R Frag[1].TmpRGBA.b[0]
#define TMP2G Frag[1].TmpRGBA.b[1]
#define TMP2B Frag[1].TmpRGBA.b[2]
#define TMP2A Frag[1].TmpRGBA.b[3]

#define CONSTR SC->state.CurrentRGBA.b[0]
#define CONSTG SC->state.CurrentRGBA.b[1]
#define CONSTB SC->state.CurrentRGBA.b[2]
#define CONSTA SC->state.CurrentRGBA.b[3]

#define SRCFUNC1(rf,gf,bf,af) MUL8(SRC1R,rf,TMP1R) MUL8(SRC1G,gf,TMP1G) MUL8(SRC1B,bf,TMP1B)    MUL8(SRC1A,af,TMP1A)
#define SRCFUNC2(rf,gf,bf,af) MUL8(SRC2R,rf,TMP2R) MUL8(SRC2G,gf,TMP2G) MUL8(SRC2B,bf,TMP2B)    MUL8(SRC2A,af,TMP2A)
#define DSTFUNC1(rf,gf,bf,af) MUL8(DST1R,rf,dst) ADD8(TMP1R,dst,DST1R) MUL8(DST1G,gf,dst) ADD8(TMP1G,dst,DST1G) MUL8(DST1B,bf,dst) ADD8(TMP1B,dst,DST1B) MUL8(DST1A,af,dst) ADD8(TMP1A,dst,DST1A)
#define DSTFUNC2(rf,gf,bf,af) MUL8(DST2R,rf,dst) ADD8(TMP2R,dst,DST2R) MUL8(DST2G,gf,dst) ADD8(TMP2G,dst,DST2G) MUL8(DST2B,bf,dst) ADD8(TMP2B,dst,DST2B) MUL8(DST2A,af,dst) ADD8(TMP2A,dst,DST2A)
/*=============================================================*/
/*
srcfunc - The mode for the source pixel:
    1    W3D_ZERO
    2    W3D_ONE

    4    W3D_DST_COLOR
    6    W3D_ONE_MINUS_DST_COLOR
    9    W3D_DST_ALPHA
    10    W3D_ONE_MINUS_DST_ALPHA

    7    W3D_SRC_ALPHA
    8    W3D_ONE_MINUS_SRC_ALPHA

    12    W3D_CONSTANT_COLOR
    13    W3D_ONE_MINUS_CONSTANT_COLOR
    14    W3D_CONSTANT_ALPHA
    15    W3D_ONE_MINUS_CONSTANT_ALPHA

    11    W3D_SRC_ALPHA_SATURATE

dstfunc - Mode for the destination:
    1    W3D_ZERO
    2    W3D_ONE

    3    W3D_SRC_COLOR
    5    W3D_ONE_MINUS_SRC_COLOR
    7    W3D_SRC_ALPHA
    8    W3D_ONE_MINUS_SRC_ALPHA

    9    W3D_DST_ALPHA
    10    W3D_ONE_MINUS_DST_ALPHA

    12    W3D_CONSTANT_COLOR
    13    W3D_ONE_MINUS_CONSTANT_COLOR
    14    W3D_CONSTANT_ALPHA
    15    W3D_ONE_MINUS_CONSTANT_ALPHA

*/
/*=============================================================*/
void PixelsSrcAlpha_OneMinusSrcAlpha32perfect(struct SOFT3D_context *SC)
{
/* blend source & dest (allways)*/
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG  size=SC->FragSize2;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;
register union oper3D Mul2;

    Mul.L.Index=Mul2.L.Index=0;
#endif

SREM(PixelsSrcAlpha_OneMinusSrcAlpha32perfect)
    while(0<size--)
    {
    FULLADDMUL8(SRC1R,SRC1A,DST1R,ONE-SRC1A,DST1R)
    NEXTADDMUL8(SRC1G,SRC1A,DST1G,ONE-SRC1A,DST1G)
    NEXTADDMUL8(SRC1B,SRC1A,DST1B,ONE-SRC1A,DST1B)

    FULLADDMUL8(SRC2R,SRC2A,DST2R,ONE-SRC2A,DST2R)
    NEXTADDMUL8(SRC2G,SRC2A,DST2G,ONE-SRC2A,DST2G)
    NEXTADDMUL8(SRC2B,SRC2A,DST2B,ONE-SRC2A,DST2B)
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsSrcAlpha_OneMinusSrcAlpha32(struct SOFT3D_context *SC)
{
/* blend source & dest (if source not solid nor transparent)*/
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG  size=SC->FragSize2;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;
register union oper3D Mul2;

    Mul.L.Index=Mul2.L.Index=0;
#endif
    if(!Wazp3D->UseAlphaMinMax.ON)
        { PixelsSrcAlpha_OneMinusSrcAlpha32perfect(SC); return; }

SREM(PixelsSrcAlpha_OneMinusSrcAlpha32)
    while(0<size--)
    {
    if (SRC1A > MINALPHA)                /* if source visible ? */
    {
        if(SRC1A < MAXALPHA)                /* if source not solid ? */
        {
        FULLADDMUL8(SRC1R,SRC1A,DST1R,ONE-SRC1A,DST1R)
        NEXTADDMUL8(SRC1G,SRC1A,DST1G,ONE-SRC1A,DST1G)
        NEXTADDMUL8(SRC1B,SRC1A,DST1B,ONE-SRC1A,DST1B)
        }
        else
        {
        COPYRGBA(Frag[0].BufferRGBA.L,Frag[0].ColorRGBA.L);
        }

    }

    if (SRC2A > MINALPHA)                /* if source visible ? */
    {
        if(SRC2A < MAXALPHA)                /* if source not solid ? */
        {
        FULLADDMUL8(SRC2R,SRC2A,DST2R,ONE-SRC2A,DST2R)
        NEXTADDMUL8(SRC2G,SRC2A,DST2G,ONE-SRC2A,DST2G)
        NEXTADDMUL8(SRC2B,SRC2A,DST2B,ONE-SRC2A,DST2B)
        }
        else
        {
        COPYRGBA(Frag[1].BufferRGBA.L,Frag[1].ColorRGBA.L);
        }

    }
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsChroma32fast(struct SOFT3D_context *SC)
{
    /* copy source to dest (if not black)*/
    register struct fragbuffer3D *Frag=SC->FragBuffer;
    register ULONG  size=SC->FragSize2;
    register ULONG  black=((0<<24) + (0<<16) + (0<<8) + 255);
    register ULONG  *rgba32;

    SREM(PixelsChroma32fast)
    while(0<size--)
    {
        rgba32=(ULONG *)Frag[0].ColorRGBA.L;
        if (rgba32[0] != black)                    /* if source visible ? */
            COPYRGBA(Frag[0].BufferRGBA.L,Frag[0].ColorRGBA.L);

        rgba32=(ULONG *)Frag[1].ColorRGBA.L;
        if (rgba32[0] != black)                    /* if source visible ? */
            COPYRGBA(Frag[1].BufferRGBA.L,Frag[1].ColorRGBA.L);
        Frag+=2;
    }
}
/*=============================================================*/
void PixelsSrcAlpha_OneMinusSrcAlpha32fast(struct SOFT3D_context *SC)
{
    /* copy source to dest (if source not transparent)*/
    register struct fragbuffer3D *Frag=SC->FragBuffer;
    register ULONG  size=SC->FragSize2;

    SREM(PixelsSrcAlpha_OneMinusSrcAlpha32fast)
    while(0<size--)
    {
    if (SRC1A > MINALPHA)                    /* if source visible ? */
        COPYRGBA(Frag[0].BufferRGBA.L,Frag[0].ColorRGBA.L);

    if (SRC2A > MINALPHA)                    /* if source visible ? */
        COPYRGBA(Frag[1].BufferRGBA.L,Frag[1].ColorRGBA.L);
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsOne_Zero32(struct SOFT3D_context *SC)
{
    /* copy source to dest = replace */
    register struct fragbuffer3D *Frag=SC->FragBuffer;
    register ULONG  size=SC->FragSize2;

    SREM(PixelsOne_Zero)
    while(0<size--)
    {
        COPYRGBA(Frag[0].BufferRGBA.L,Frag[0].ColorRGBA.L);
        COPYRGBA(Frag[1].BufferRGBA.L,Frag[1].ColorRGBA.L);
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsSrcAlpha_One32(struct SOFT3D_context *SC)
{
/* v45: for glxcess :-) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG  size=SC->FragSize2;
register UBYTE tmp;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register UBYTE *Add8=SC->Add8;
register union oper3D Add;
register union oper3D Mul;

    Mul.L.Index=Add.L.Index=0;
#endif

SREM(PixelsSrcAlpha_One32)
    while(0<size--)
    {
    if (SRC1A > MINALPHA)                /* if source visible ? */
    {
        if(SRC1A < MAXALPHA)                /* if source not solid ? */
        {
        MUL8(SRC1R,SRC1A,tmp) ADD8(tmp,DST1R,DST1R)
        MUL8(SRC1G,SRC1A,tmp) ADD8(tmp,DST1G,DST1G)
        MUL8(SRC1B,SRC1A,tmp) ADD8(tmp,DST1B,DST1B)
        }
        else
        {
        ADD8(SRC1R,DST1R,DST1R)
        ADD8(SRC1G,DST1G,DST1G)
        ADD8(SRC1B,DST1B,DST1B)
        }

    }

    if (SRC2A > MINALPHA)                /* if source visible ? */
    {
        if(SRC2A < MAXALPHA)                /* if source not solid ? */
        {
        MUL8(SRC2R,SRC2A,tmp) ADD8(tmp,DST2R,DST2R)
        MUL8(SRC2G,SRC2A,tmp) ADD8(tmp,DST2G,DST2G)
        MUL8(SRC2B,SRC2A,tmp) ADD8(tmp,DST2B,DST2B)
        }
        else
        {
        ADD8(SRC2R,DST2R,DST2R)
        ADD8(SRC2G,DST2G,DST2G)
        ADD8(SRC2B,DST2B,DST2B)
        }

    }
    Frag+=2;
    }

}
/*=============================================================*/
void PixelsOne_OneMinusSrcAlpha32(struct SOFT3D_context *SC)
{
/* v45: for glxcess :-) */
/* v50: this function fixed */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG  size=SC->FragSize2;
register UBYTE tmp;
register UBYTE A;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register UBYTE *Add8=SC->Add8;
register union oper3D Add;
register union oper3D Mul;

    Mul.L.Index=Add.L.Index=0;
#endif

SREM(PixelsOne_OneMinusSrcAlpha32)
    while(0<size--)
    {
    A=ONE-SRC1A;
    if (A < MAXALPHA)                /* just keep dest unchanged */
    {
        if(A > MINALPHA)                /* if dest not cleaned */
        {
        MUL8(DST1R,A,tmp); ADD8(tmp,SRC1R,DST1R);
        MUL8(DST1G,A,tmp); ADD8(tmp,SRC1G,DST1G);
        MUL8(DST1B,A,tmp); ADD8(tmp,SRC1B,DST1B);
        }
        else
        {
        COPYRGBA(Frag[0].BufferRGBA.L,Frag[0].ColorRGBA.L);
        }

    }
    DST1A=ONE;

    A=ONE-SRC2A;
    if (A < MAXALPHA)                /* just keep dest unchanged */
    {
        if(A > MINALPHA)                /* if dest not cleaned */
        {
        MUL8(DST2R,A,tmp); ADD8(tmp,SRC2R,DST2R);
        MUL8(DST2G,A,tmp); ADD8(tmp,SRC2G,DST2G);
        MUL8(DST2B,A,tmp); ADD8(tmp,SRC2B,DST2B);
        }
        else
        {
        COPYRGBA(Frag[1].BufferRGBA.L,Frag[0].ColorRGBA.L);
        }

    }
    DST2A=ONE;

    Frag+=2;
    }

}
/*=============================================================*/
void PixelsOne_One24(struct SOFT3D_context *SC)
{
/* v41: add saturate src & dst  (allways) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG  size=SC->FragSize2;
register union rgba3D Tmp1;
register union rgba3D Tmp2;
#ifdef SLOWCPU
register UBYTE *Add8=SC->Add8;
register union oper3D Add;

    Add.L.Index=0;
#endif
SREM(PixelsOne_One24)
    while(0<size--)
    {
    Tmp1.L.RGBA32=*( (ULONG *)DST1 );
    Tmp2.L.RGBA32=*( (ULONG *)DST2 );

    ADD8(SRC1R,Tmp1.B.RGBA[0],Tmp1.B.RGBA[0])
    ADD8(SRC1G,Tmp1.B.RGBA[1],Tmp1.B.RGBA[1])
    ADD8(SRC1B,Tmp1.B.RGBA[2],Tmp1.B.RGBA[2])

    ADD8(SRC2R,Tmp2.B.RGBA[0],Tmp2.B.RGBA[0])
    ADD8(SRC2G,Tmp2.B.RGBA[1],Tmp2.B.RGBA[1])
    ADD8(SRC2B,Tmp2.B.RGBA[2],Tmp2.B.RGBA[2])

    *( (ULONG *)DST1 )=Tmp1.L.RGBA32;
    *( (ULONG *)DST2 )=Tmp2.L.RGBA32;
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsZero_SrcAlpha32(struct SOFT3D_context *SC)
{
/* v41: alpha-modulate dst  (allways) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG  size=SC->FragSize2;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;

    Mul.L.Index=0;
#endif

SREM(PixelsZero_SrcAlpha32)
    while(0<size--)
    {
    MUL8(SRC1A,DST1R,DST1R)
    MUL8(SRC1A,DST1G,DST1G)
    MUL8(SRC1A,DST1B,DST1B)

    MUL8(SRC2A,DST2R,DST2R)
    MUL8(SRC2A,DST2G,DST2G)
    MUL8(SRC2A,DST2B,DST2B)
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsZero_SrcColor24(struct SOFT3D_context *SC)
{
/* v45: color-modulate dst  (allways) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG  size=SC->FragSize2;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;

    Mul.L.Index=0;
#endif

SREM(PixelsZero_SrcColor24)
    while(0<size--)
    {
    MUL8(SRC1R,DST1R,DST1R)
    MUL8(SRC1G,DST1G,DST1G)
    MUL8(SRC1B,DST1B,DST1B)

    MUL8(SRC2R,DST2R,DST2R)
    MUL8(SRC2G,DST2G,DST2G)
    MUL8(SRC2B,DST2B,DST2B)
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsZero_OneMinusSrcColor24(struct SOFT3D_context *SC)
{
/* v45: for glxcess :-) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG  size=SC->FragSize2;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;

    Mul.L.Index=0;
#endif

SREM(PixelsZero_OneMinusSrcColor24)
    while(0<size--)
    {
    MUL8(ONE-SRC1R,DST1R,DST1R)
    MUL8(ONE-SRC1G,DST1G,DST1G)
    MUL8(ONE-SRC1B,DST1B,DST1B)

    MUL8(ONE-SRC2R,DST2R,DST2R)
    MUL8(ONE-SRC2G,DST2G,DST2G)
    MUL8(ONE-SRC2B,DST2B,DST2B)
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsSrcAlpha_OneMinusSrcColor32(struct SOFT3D_context *SC)
{
/* v45: for glxcess :-) */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG  size=SC->FragSize2;
#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;
register union oper3D Mul2;

    Mul.L.Index=Mul2.L.Index=0;
#endif

SREM(PixelsSrcAlpha_OneMinusSrcColor32)
    while(0<size--)
    {
    FULLADDMUL8(SRC1R,SRC1A,DST1R,ONE-SRC1R,DST1R)
    FULLADDMUL8(SRC1G,SRC1A,DST1G,ONE-SRC1G,DST1G)
    FULLADDMUL8(SRC1B,SRC1A,DST1B,ONE-SRC1B,DST1B)

    FULLADDMUL8(SRC2R,SRC2A,DST2R,ONE-SRC2R,DST2R)
    FULLADDMUL8(SRC2G,SRC2A,DST2G,ONE-SRC2G,DST2G)
    FULLADDMUL8(SRC2B,SRC2A,DST2B,ONE-SRC2B,DST2B)
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsDstColor_Zero24(struct SOFT3D_context *SC)
{
SREM(PixelsDstColor_Zero24)
    PixelsZero_SrcColor24(SC);
}
/*=============================================================*/
void PixelsBlendFunctionAll(struct SOFT3D_context *SC)
{
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size;
register UBYTE dst;
ULONG ZeroRGBA[1]={0x00000000};
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
UBYTE i;

#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register UBYTE *Add8=SC->Add8;
register union oper3D Add;
register union oper3D Mul;

    Mul.L.Index=Add.L.Index=0;
#endif

SREM(PixelsBlendFunctionAll)
    if(SC->FunctionBlendFast==NULL)
    if(Wazp3D->DebugBlendFunction.ON)
    {
        PrintST((struct SOFT3D_texture *)swap32((uint32_t)SC->state.ST));
        Libprintf("use slow BlendFunction(Src:%08lx,Dst%08lx)\n",(ULONG)SC->SrcFunc,(ULONG)SC->DstFunc);
    }


/* store Image to Buffer ? */
    if(SC->ImageBuffer32!=NULL)
    {
        size=SC->FragSize2;
        Frag=SC->FragBuffer;
        while(0<size--)
        {
            COPYRGBA(Frag[0].BufferRGBA.L,Frag[0].Image8);
            COPYRGBA(Frag[1].BufferRGBA.L,Frag[1].Image8);
            Frag+=2;
        }
    }

    if(SC->FunctionBlendFast!=NULL)
    {
        SC->FunctionBlendFast(SC);
        goto BlendDone;
    }

    size=SC->FragSize2;
    Frag=SC->FragBuffer;

/* Step1: do Src Function to TmpRGBA */
    switch(SC->SrcFunc)
    {

    case W3D_ZERO:
    while(0<size--)
    {
    COPYRGBA(Frag[0].TmpRGBA.L,ZeroRGBA);
    COPYRGBA(Frag[1].TmpRGBA.L,ZeroRGBA);
    Frag+=2;
    }
    break;

    case W3D_ONE:
    while(0<size--)
    {
    COPYRGBA(Frag[0].TmpRGBA.L,SRC1);    /* ONE x SRC1 = SRC1 */
    COPYRGBA(Frag[1].TmpRGBA.L,SRC2);
    Frag+=2;
    }
    break;

    case W3D_DST_COLOR:
    while(0<size--)
    {
    SRCFUNC1(DST1R,DST1G,DST1B,DST1A)
    SRCFUNC2(DST2R,DST2G,DST2B,DST2A)
    Frag+=2;
    }
    break;

    case W3D_ONE_MINUS_DST_COLOR:
    while(0<size--)
    {
    SRCFUNC1(ONE-DST1R,ONE-DST1G,ONE-DST1B,ONE-DST1A)
    SRCFUNC2(ONE-DST2R,ONE-DST2G,ONE-DST2B,ONE-DST2A)
    Frag+=2;
    }
    break;

    case W3D_SRC_ALPHA:
    while(0<size--)
    {
    SRCFUNC1(SRC1A,SRC1A,SRC1A,SRC1A)
    SRCFUNC2(SRC2A,SRC2A,SRC2A,SRC2A)
    Frag+=2;
    }
    break;

    case W3D_ONE_MINUS_SRC_ALPHA:
    while(0<size--)
    {
    SRCFUNC1(ONE-SRC1A,ONE-SRC1A,ONE-SRC1A,ONE-SRC1A)
    SRCFUNC2(ONE-SRC2A,ONE-SRC2A,ONE-SRC2A,ONE-SRC2A)
    Frag+=2;
    }
    break;

    case W3D_DST_ALPHA:
    while(0<size--)
    {
    SRCFUNC1(DST1A,DST1A,DST1A,DST1A)
    SRCFUNC2(DST2A,DST2A,DST2A,DST2A)
    Frag+=2;
    }
    break;

    case W3D_ONE_MINUS_DST_ALPHA:
    while(0<size--)
    {
    SRCFUNC1(ONE-DST1A,ONE-DST1A,ONE-DST1A,ONE-DST1A)
    SRCFUNC2(ONE-DST2A,ONE-DST2A,ONE-DST2A,ONE-DST2A)
    Frag+=2;
    }
    break;

    case W3D_CONSTANT_COLOR:
    while(0<size--)
    {
    SRCFUNC1(CONSTR,CONSTG,CONSTB,CONSTA)
    SRCFUNC2(CONSTR,CONSTG,CONSTB,CONSTA)
    Frag+=2;
    }
    break;

    case W3D_ONE_MINUS_CONSTANT_COLOR:
    while(0<size--)
    {
    SRCFUNC1(ONE-CONSTR,ONE-CONSTG,ONE-CONSTB,ONE-CONSTA)
    SRCFUNC2(ONE-CONSTR,ONE-CONSTG,ONE-CONSTB,ONE-CONSTA)
    Frag+=2;
    }
    break;

    case W3D_CONSTANT_ALPHA:
    while(0<size--)
    {
    SRCFUNC1(CONSTA,CONSTA,CONSTA,CONSTA)
    SRCFUNC2(CONSTA,CONSTA,CONSTA,CONSTA)
    Frag+=2;
    }
    break;

    case W3D_ONE_MINUS_CONSTANT_ALPHA:
    while(0<size--)
    {
    SRCFUNC1(ONE-CONSTA,ONE-CONSTA,ONE-CONSTA,ONE-CONSTA)
    SRCFUNC2(ONE-CONSTA,ONE-CONSTA,ONE-CONSTA,ONE-CONSTA)
    Frag+=2;
    }
    break;

    case W3D_SRC_ALPHA_SATURATE:
    while(0<size--)
    {
    i = MIN (SRC1A,ONE  - DST1A) ;            /* v47: OpenGL compliant */
    SRCFUNC1(i,i,i,i)
    i = MIN (SRC2A,ONE  - DST2A) ;
    SRCFUNC2(i,i,i,i)
    Frag+=2;
    }
    break;
    }

/* Step2: do Dst Function from TmpRGBA */
    size=SC->FragSize2;
    Frag=SC->FragBuffer;

    switch(SC->DstFunc)
    {
    case W3D_ZERO:
    while(0<size--)
    {
    COPYRGBA(Frag[0].BufferRGBA.L,Frag[0].TmpRGBA.L);
    COPYRGBA(Frag[1].BufferRGBA.L,Frag[1].TmpRGBA.L);
    Frag+=2;
    }
    break;

    case W3D_ONE:
    while(0<size--)
    {
    ADD8(TMP1R,DST1R,DST1R) ADD8(TMP1G,DST1G,DST1G) ADD8(TMP1B,DST1B,DST1B) ADD8(TMP1A,DST1A,DST1A)
    ADD8(TMP2R,DST2R,DST2R) ADD8(TMP2G,DST2G,DST2G) ADD8(TMP2B,DST2B,DST2B) ADD8(TMP2A,DST2A,DST2A)
    Frag+=2;
    }
    break;

    case W3D_SRC_COLOR:
    while(0<size--)
    {
    DSTFUNC1(SRC1R,SRC1G,SRC1B,SRC1A)
    DSTFUNC2(SRC2R,SRC2G,SRC2B,SRC2A)
    Frag+=2;
    }
    break;

    case W3D_ONE_MINUS_SRC_COLOR:
    while(0<size--)
    {
    DSTFUNC1(ONE-SRC1R,ONE-SRC1G,ONE-SRC1B,ONE-SRC1A)
    DSTFUNC2(ONE-SRC2R,ONE-SRC2G,ONE-SRC2B,ONE-SRC2A)
    Frag+=2;
    }
    break;

    case W3D_DST_ALPHA:
    while(0<size--)
    {
    DSTFUNC1(DST1A,DST1A,DST1A,DST1A)
    DSTFUNC2(DST2A,DST2A,DST2A,DST2A)
    Frag+=2;
    }
    break;

    case W3D_ONE_MINUS_DST_ALPHA:
    while(0<size--)
    {
    DSTFUNC1(ONE-DST1A,ONE-DST1A,ONE-DST1A,ONE-DST1A)
    DSTFUNC2(ONE-DST2A,ONE-DST2A,ONE-DST2A,ONE-DST2A)
    Frag+=2;
    }
    break;

    case W3D_SRC_ALPHA:
    while(0<size--)
    {
    DSTFUNC1(SRC1A,SRC1A,SRC1A,SRC1A)
    DSTFUNC2(SRC2A,SRC2A,SRC2A,SRC2A)
    Frag+=2;
    }
    break;

    case W3D_ONE_MINUS_SRC_ALPHA:
    while(0<size--)
    {
    DSTFUNC1(ONE-SRC1A,ONE-SRC1A,ONE-SRC1A,ONE-SRC1A)
    DSTFUNC2(ONE-SRC2A,ONE-SRC2A,ONE-SRC2A,ONE-SRC2A)
    Frag+=2;
    }
    break;

    case W3D_CONSTANT_COLOR:
    while(0<size--)
    {
    DSTFUNC1(CONSTR,CONSTG,CONSTB,CONSTA)
    DSTFUNC2(CONSTR,CONSTG,CONSTB,CONSTA)
    Frag+=2;
    }
    break;

    case W3D_ONE_MINUS_CONSTANT_COLOR:
    while(0<size--)
    {
    DSTFUNC1(ONE-CONSTR,ONE-CONSTG,ONE-CONSTB,ONE-CONSTA)
    DSTFUNC2(ONE-CONSTR,ONE-CONSTG,ONE-CONSTB,ONE-CONSTA)
    Frag+=2;
    }
    break;

    case W3D_CONSTANT_ALPHA:
    while(0<size--)
    {
    DSTFUNC1(CONSTA,CONSTA,CONSTA,CONSTA)
    DSTFUNC2(CONSTA,CONSTA,CONSTA,CONSTA)
    Frag+=2;
    }
    break;

    case W3D_ONE_MINUS_CONSTANT_ALPHA:
    while(0<size--)
    {
    DSTFUNC1(ONE-CONSTA,ONE-CONSTA,ONE-CONSTA,ONE-CONSTA)
    DSTFUNC2(ONE-CONSTA,ONE-CONSTA,ONE-CONSTA,ONE-CONSTA)
    Frag+=2;
    }
    break;

    }

BlendDone:
/* store the buffer to image8 ? */
    if(SC->ImageBuffer32!=NULL)
    {
    size=SC->FragSize2;
    Frag=SC->FragBuffer;
    while(0<size--)
    {
    COPYRGBA(Frag[0].Image8,Frag[0].BufferRGBA.L );
    COPYRGBA(Frag[1].Image8,Frag[1].BufferRGBA.L );
    Frag+=2;
    }
    }

}
/*=============================================================*/
/* v52: Optimized 32 bits PixelsIn/Out */
/*=============================================================*/
#define COLOR32(x) (*(ULONG *)x)
/*=============================================================*/
void PixelsInBGRA(struct SOFT3D_context *SC)
{
/* Convert BGRA -> buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register union rgba3D Color0;
register union rgba3D Color1;
register UBYTE temp;

    while(0<size--)
    {
    Color0.L.RGBA32=COLOR32(Frag[0].Image8);
    Color1.L.RGBA32=COLOR32(Frag[1].Image8);

    SWAP(Color0.B.RGBA[0],Color0.B.RGBA[2]);
    SWAP(Color1.B.RGBA[0],Color1.B.RGBA[2]);

//    COLOR32(Frag[0].BufferRGBA.L)=Color0.L.RGBA32;
//    COLOR32(Frag[1].BufferRGBA.L)=Color1.L.RGBA32;
    Frag[0].BufferRGBA.L[0]=Color0.L.RGBA32;
    Frag[1].BufferRGBA.L[0]=Color1.L.RGBA32;
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsInARGB(struct SOFT3D_context *SC)
{
/* Convert ARGB -> buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register union rgba3D Color0;
register union rgba3D Color1;

    while(0<size--)
    {
        Color0.L.RGBA32=COLOR32(Frag[0].Image8);
        Color1.L.RGBA32=COLOR32(Frag[1].Image8);

        Color0.L.RGBA32=(Color0.B.RGBA[0]+(Color0.L.RGBA32<<8));
        Color1.L.RGBA32=(Color1.B.RGBA[0]+(Color1.L.RGBA32<<8));

//        COLOR32(Frag[0].BufferRGBA.L)=Color0.L.RGBA32;
//        COLOR32(Frag[1].BufferRGBA.L)=Color1.L.RGBA32;
        (Frag[0].BufferRGBA.L[0])=Color0.L.RGBA32;
        (Frag[1].BufferRGBA.L[0])=Color1.L.RGBA32;
        Frag+=2;
    }
}
/*=============================================================*/
void PixelsInABGR(struct SOFT3D_context *SC)
{
/* Convert ABGR -> buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register union rgba3D Color0;
register union rgba3D Color1;
register UBYTE temp;

    while(0<size--)
    {
        Color0.L.RGBA32=COLOR32(Frag[0].Image8);
        Color1.L.RGBA32=COLOR32(Frag[1].Image8);

        SWAP(Color0.B.RGBA[0],Color0.B.RGBA[3]);
        SWAP(Color0.B.RGBA[1],Color0.B.RGBA[2]);

        SWAP(Color1.B.RGBA[0],Color1.B.RGBA[3]);
        SWAP(Color1.B.RGBA[1],Color1.B.RGBA[2]);

//        COLOR32(Frag[0].BufferRGBA.L)=Color0.L.RGBA32;
//        COLOR32(Frag[1].BufferRGBA.L)=Color1.L.RGBA32;
        (Frag[0].BufferRGBA.L[0])=Color0.L.RGBA32;
        (Frag[1].BufferRGBA.L[0])=Color1.L.RGBA32;
        Frag+=2;
    }
}
/*=============================================================*/
/*=============================================================*/
void PixelsOutBGRA_Flat(struct SOFT3D_context *SC)
{
/* Convert BGRA <- buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size4=SC->FragSize2/2;
register union rgba3D Color0;
register UBYTE temp;

//    Color0.L.RGBA32=COLOR32(Frag[0].BufferRGBA.L);
    Color0.L.RGBA32=(Frag[0].BufferRGBA.L[0]);
    SWAP(Color0.B.RGBA[0],Color0.B.RGBA[2]);

    while(0<size4--)
    {
        COLOR32(Frag[0].Image8)=Color0.L.RGBA32;
        COLOR32(Frag[1].Image8)=Color0.L.RGBA32;
        COLOR32(Frag[2].Image8)=Color0.L.RGBA32;
        COLOR32(Frag[3].Image8)=Color0.L.RGBA32;
    Frag+=4;
    }
}
/*=============================================================*/
void PixelsOutARGB_Flat(struct SOFT3D_context *SC)
{
/* Convert ARGB <- buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size4=SC->FragSize2/2;
register union rgba3D Color0;

//    Color0.L.RGBA32=COLOR32(Frag[0].BufferRGBA.L);
    Color0.L.RGBA32=(Frag[0].BufferRGBA.L[0]);
    Color0.L.RGBA32=((Color0.B.RGBA[3]<<24)+(Color0.L.RGBA32>>8));

    while(0<size4--)
    {
    COLOR32(Frag[0].Image8)=Color0.L.RGBA32;
    COLOR32(Frag[1].Image8)=Color0.L.RGBA32;
    COLOR32(Frag[2].Image8)=Color0.L.RGBA32;
    COLOR32(Frag[3].Image8)=Color0.L.RGBA32;
    Frag+=4;
    }
}
/*=============================================================*/
void PixelsOutABGR_Flat(struct SOFT3D_context *SC)
{
/* Convert ABGR <- buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size4=SC->FragSize2/2;
register union rgba3D Color0;
register UBYTE temp;

//    Color0.L.RGBA32=COLOR32(Frag[0].BufferRGBA.L);
    Color0.L.RGBA32=(Frag[0].BufferRGBA.L[0]);
    SWAP(Color0.B.RGBA[0],Color0.B.RGBA[3]);
    SWAP(Color0.B.RGBA[1],Color0.B.RGBA[2]);

    while(0<size4--)
    {
    COLOR32(Frag[0].Image8)=Color0.L.RGBA32;
    COLOR32(Frag[1].Image8)=Color0.L.RGBA32;
    COLOR32(Frag[2].Image8)=Color0.L.RGBA32;
    COLOR32(Frag[3].Image8)=Color0.L.RGBA32;
    Frag+=4;
    }
}
/*=============================================================*/
void PixelsOutBGRA(struct SOFT3D_context *SC)
{
/* Convert BGRA <- buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register union rgba3D Color0;
register union rgba3D Color1;
register UBYTE temp;

    while(0<size--)
    {
//        Color0.L.RGBA32=COLOR32(Frag[0].BufferRGBA.L);
//        Color1.L.RGBA32=COLOR32(Frag[1].BufferRGBA.L);
        Color0.L.RGBA32=(Frag[0].BufferRGBA.L[0]);
        Color1.L.RGBA32=(Frag[1].BufferRGBA.L[0]);

        SWAP(Color0.B.RGBA[0],Color0.B.RGBA[2]);
        SWAP(Color1.B.RGBA[0],Color1.B.RGBA[2]);

        COLOR32(Frag[0].Image8)=Color0.L.RGBA32;
        COLOR32(Frag[1].Image8)=Color1.L.RGBA32;
        Frag+=2;
    }
}
/*=============================================================*/
void PixelsOutARGB(struct SOFT3D_context *SC)
{
/* Convert ARGB <- buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register union rgba3D Color0;
register union rgba3D Color1;

    while(0<size--)
    {
//        Color0.L.RGBA32=COLOR32(Frag[0].BufferRGBA.L);
//        Color1.L.RGBA32=COLOR32(Frag[1].BufferRGBA.L);
        Color0.L.RGBA32=(Frag[0].BufferRGBA.L[0]);
        Color1.L.RGBA32=(Frag[1].BufferRGBA.L[0]);

        Color0.L.RGBA32=((Color0.B.RGBA[3]<<24)+(Color0.L.RGBA32>>8));
        Color1.L.RGBA32=((Color1.B.RGBA[3]<<24)+(Color1.L.RGBA32>>8));

        COLOR32(Frag[0].Image8)=Color0.L.RGBA32;
        COLOR32(Frag[1].Image8)=Color1.L.RGBA32;
        Frag+=2;
    }
}
/*=============================================================*/
void PixelsOutABGR(struct SOFT3D_context *SC)
{
/* Convert ABGR <- buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register union rgba3D Color0;
register union rgba3D Color1;
register UBYTE temp;

    while(0<size--)
    {
//        Color0.L.RGBA32=COLOR32(Frag[0].BufferRGBA.L);
//        Color1.L.RGBA32=COLOR32(Frag[1].BufferRGBA.L);
        Color0.L.RGBA32=(Frag[0].BufferRGBA.L[0]);
        Color1.L.RGBA32=(Frag[1].BufferRGBA.L[0]);

        SWAP(Color0.B.RGBA[0],Color0.B.RGBA[3]);
        SWAP(Color0.B.RGBA[1],Color0.B.RGBA[2]);

        SWAP(Color1.B.RGBA[0],Color1.B.RGBA[3]);
        SWAP(Color1.B.RGBA[1],Color1.B.RGBA[2]);

        COLOR32(Frag[0].Image8)=Color0.L.RGBA32;
        COLOR32(Frag[1].Image8)=Color1.L.RGBA32;
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsInRGBA(struct SOFT3D_context *SC)
{
/* Convert RGBA -> buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size4=SC->FragSize2/2;

    while(0<size4--)
    {
    COPYRGBA(Frag[0].BufferRGBA.L,Frag[0].Image8);
    COPYRGBA(Frag[1].BufferRGBA.L,Frag[1].Image8);
    COPYRGBA(Frag[2].BufferRGBA.L,Frag[0].Image8);
    COPYRGBA(Frag[3].BufferRGBA.L,Frag[1].Image8);
    Frag+=4;
    }

}
/*=============================================================*/
void PixelsOutRGBA_Flat(struct SOFT3D_context *SC)
{
/* Convert buffer -> RGBA */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size4=SC->FragSize2/2;
register union rgba3D Color0;

//    Color0.L.RGBA32=COLOR32(Frag[0].BufferRGBA.L);
    Color0.L.RGBA32=Frag[0].BufferRGBA.L[0];
    while(0<size4--)
    {
    COLOR32(Frag[0].Image8)=Color0.L.RGBA32;
    COLOR32(Frag[1].Image8)=Color0.L.RGBA32;
    COLOR32(Frag[2].Image8)=Color0.L.RGBA32;
    COLOR32(Frag[3].Image8)=Color0.L.RGBA32;
    Frag+=4;
    }

}
/*=============================================================*/
void PixelsOutRGBA(struct SOFT3D_context *SC)
{
/* Convert buffer -> RGBA */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

    while(0<size--)
    {
    COPYRGBA((ULONG*)Frag[0].Image8,Frag[0].BufferRGBA.L);
    COPYRGBA((ULONG*)Frag[1].Image8,Frag[1].BufferRGBA.L);
    Frag+=2;
    }

}
/*=============================================================*/
void PixelsOutBGRAfromtex24(struct SOFT3D_context *SC)
{
/* patch suggestion from "Cosmos" : */
/* For a simple prog like CoW3D dont use pipe-line but write tex directly to the BGRA bitmap  */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register union rgba3D Color0;
register union rgba3D Color1;


    Color0.B.RGBA[3]=Color1.B.RGBA[3]=ONE;
    while(0<size--)
    {
    Color0.B.RGBA[2]=Frag[0].Tex8[0];
    Color0.B.RGBA[1]=Frag[0].Tex8[1];
    Color0.B.RGBA[0]=Frag[0].Tex8[2];

    Color1.B.RGBA[2]=Frag[1].Tex8[0];
    Color1.B.RGBA[1]=Frag[1].Tex8[1];
    Color1.B.RGBA[0]=Frag[1].Tex8[2];

    COLOR32(Frag[0].Image8)=Color0.L.RGBA32;
    COLOR32(Frag[1].Image8)=Color1.L.RGBA32;
    Frag+=2;
    }
}
/*=============================================================*/
void PixelsOutARGBfromtex24(struct SOFT3D_context *SC)
{
/* patch suggestion from "Cosmos" : */
/* For a simple prog like CoW3D dont use pipe-line but write tex directly to the ARGB bitmap  */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register union rgba3D Color0;
register union rgba3D Color1;


    Color0.B.RGBA[0]=Color1.B.RGBA[0]=ONE;
    while(0<size--)
    {
    Color0.B.RGBA[1]=Frag[0].Tex8[0];
    Color0.B.RGBA[2]=Frag[0].Tex8[1];
    Color0.B.RGBA[3]=Frag[0].Tex8[2];

    Color1.B.RGBA[1]=Frag[1].Tex8[0];
    Color1.B.RGBA[2]=Frag[1].Tex8[1];
    Color1.B.RGBA[3]=Frag[1].Tex8[2];

    COLOR32(Frag[0].Image8)=Color0.L.RGBA32;
    COLOR32(Frag[1].Image8)=Color1.L.RGBA32;
    Frag+=2;
    }
}
/*=============================================================*/
/*  End of 32 bits functions */
/*=============================================================*/
void PixelsInRGB(struct SOFT3D_context *SC)
{
/* Convert RGB -> buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

    while(0<size--)
    {
    Frag[0].BufferRGBA.b[0]=Frag[0].Image8[0];
    Frag[0].BufferRGBA.b[1]=Frag[0].Image8[1];
    Frag[0].BufferRGBA.b[2]=Frag[0].Image8[2];
    Frag[0].BufferRGBA.b[3]=ONE;

    Frag[1].BufferRGBA.b[0]=Frag[1].Image8[0];
    Frag[1].BufferRGBA.b[1]=Frag[1].Image8[1];
    Frag[1].BufferRGBA.b[2]=Frag[1].Image8[2];
    Frag[1].BufferRGBA.b[3]=ONE;
    Frag+=2;
    }

}
/*=============================================================*/
void PixelsOutRGB(struct SOFT3D_context *SC)
{
/* Convert buffer -> RGB */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

    while(0<size--)
    {
    Frag[0].Image8[0]=Frag[0].BufferRGBA.b[0];
    Frag[0].Image8[1]=Frag[0].BufferRGBA.b[1];
    Frag[0].Image8[2]=Frag[0].BufferRGBA.b[2];

    Frag[1].Image8[0]=Frag[1].BufferRGBA.b[0];
    Frag[1].Image8[1]=Frag[1].BufferRGBA.b[1];
    Frag[1].Image8[2]=Frag[1].BufferRGBA.b[2];
    Frag+=2;
    }

}
/*=============================================================*/
void PixelsInBGR(struct SOFT3D_context *SC)
{
/* Convert BGR -> buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

    while(0<size--)
    {
    Frag[0].BufferRGBA.b[0]=Frag[0].Image8[2];
    Frag[0].BufferRGBA.b[1]=Frag[0].Image8[1];
    Frag[0].BufferRGBA.b[2]=Frag[0].Image8[0];
    Frag[0].BufferRGBA.b[3]=ONE;

    Frag[1].BufferRGBA.b[0]=Frag[1].Image8[2];
    Frag[1].BufferRGBA.b[1]=Frag[1].Image8[1];
    Frag[1].BufferRGBA.b[2]=Frag[1].Image8[0];
    Frag[1].BufferRGBA.b[3]=ONE;
    Frag+=2;
    }

}
/*=============================================================*/
void PixelsOutBGR(struct SOFT3D_context *SC)
{
/* Convert buffer -> BGR */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;

    while(0<size--)
    {
    Frag[0].Image8[0]=Frag[0].BufferRGBA.b[2];
    Frag[0].Image8[1]=Frag[0].BufferRGBA.b[1];
    Frag[0].Image8[2]=Frag[0].BufferRGBA.b[0];

    Frag[1].Image8[0]=Frag[1].BufferRGBA.b[2];
    Frag[1].Image8[1]=Frag[1].BufferRGBA.b[1];
    Frag[1].Image8[2]=Frag[1].BufferRGBA.b[0];
    Frag+=2;
    }

}
/*=============================================================*/
void SelectMipMap(struct SOFT3D_context *SC)
{
struct SOFT3D_texture *ST=(struct SOFT3D_texture *)swap32((uint32_t)SC->state.ST);
LONG dmin;
UBYTE mm;

    if(ST==NULL) return;
    SC->MM=&ST->MMs[0];

    if(!Wazp3D->DoMipMaps.ON) return;

SREM(SelectMipMap)
    dmin=SC->dmin>>(16-3);            /* rescale dmin to [0 to 2048] */

    mm=0;                        /* default: use biggest mipmap = more precision*/
    if(dmin>=1)        mm=1;        /* else we skip only 1 texel for each pixel ==> need the more precise(biggest) mipmap */
    if(dmin>=2)        mm=2;
    if(dmin>=8)        mm=4;
    if(dmin>=16)    mm=5;
    if(dmin>=32)    mm=6;
    if(dmin>=64)    mm=7;
    if(dmin>=128)    mm=8;
    if(dmin>=256)    mm=9;
    if(dmin>=512)    mm=10;
    if(dmin>=1024)    mm=11;    /* Example we skip 3000 texels for each pixel ==> use a 1x1 texels mipmap[11] not the 2048x2048 original */

    SVAR(dmin)
    SVAR(mm)
    SC->MM=&ST->MMs[mm];
}
/*=============================================================*/
void Fill_BigTexPersp2_Gouraud_Fog(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct SOFT3D_mipmap *MM=SC->MM;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

SREM(Fill_BigTexPersp2_Gouraud_Fog)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        Frag->Tex8 =MM->Tex8U   [Pix->W.u ]+MM->Tex8V   [Pix->W.v ];
        Frag->Tex8+=MM->Tex8Ulow[Pix->W.u3]+MM->Tex8Vlow[Pix->W.v3];
        Frag->ColorRGBA.b[0]=Pix->W.R;
        Frag->ColorRGBA.b[1]=Pix->W.G;
        Frag->ColorRGBA.b[2]=Pix->W.B;
        Frag->ColorRGBA.b[3]=Pix->W.A;
        COPYRGBA(Frag->FogRGBA.L,(&SC->FogRGBAs[Pix->W.F]));
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.u +=Pix->L.du;
    Pix->L.v +=Pix->L.dv;
    Pix->L.du+=Pix->L.ddu;
    Pix->L.dv+=Pix->L.ddv;
    Pix->L.R +=Pix->L.dR;
    Pix->L.G +=Pix->L.dG;
    Pix->L.B +=Pix->L.dB;
    Pix->L.A +=Pix->L.dA;
    Pix->L.F +=Pix->L.dF;
    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_BigTexPersp2(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct SOFT3D_mipmap *MM=SC->MM;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

SREM(Fill_BigTexPersp2)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        Frag->Tex8 =MM->Tex8U   [Pix->W.u ]+MM->Tex8V   [Pix->W.v ];
        Frag->Tex8+=MM->Tex8Ulow[Pix->W.u3]+MM->Tex8Vlow[Pix->W.v3];
        COPYRGBA(Frag->ColorRGBA.L,SC->FlatRGBA.L);
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.u +=Pix->L.du;
    Pix->L.v +=Pix->L.dv;
    Pix->L.du+=Pix->L.ddu;
    Pix->L.dv+=Pix->L.ddv;

    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_TexPersp2_Gouraud_Fog(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct SOFT3D_mipmap *MM=SC->MM;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

SREM(Fill_TexPersp2_Gouraud_Fog)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        Frag->Tex8 =MM->Tex8U   [Pix->W.u ]+MM->Tex8V   [Pix->W.v ];
        Frag->ColorRGBA.b[0]=Pix->W.R;
        Frag->ColorRGBA.b[1]=Pix->W.G;
        Frag->ColorRGBA.b[2]=Pix->W.B;
        Frag->ColorRGBA.b[3]=Pix->W.A;
        COPYRGBA(Frag->FogRGBA.L,(&SC->FogRGBAs[Pix->W.F]));
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.u +=Pix->L.du;
    Pix->L.v +=Pix->L.dv;
    Pix->L.du+=Pix->L.ddu;
    Pix->L.dv+=Pix->L.ddv;
    Pix->L.R +=Pix->L.dR;
    Pix->L.G +=Pix->L.dG;
    Pix->L.B +=Pix->L.dB;
    Pix->L.A +=Pix->L.dA;
    Pix->L.F +=Pix->L.dF;
    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_Tex_Gouraud_Fog(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct SOFT3D_mipmap *MM=SC->MM;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

    if(SC->state.PerspMode==2)
        {Fill_TexPersp2_Gouraud_Fog(SC); return;}
SREM(Fill_Tex_Gouraud_Fog)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        Frag->Tex8 =MM->Tex8U   [Pix->W.u ]+MM->Tex8V   [Pix->W.v ];
        Frag->ColorRGBA.b[0]=Pix->W.R;
        Frag->ColorRGBA.b[1]=Pix->W.G;
        Frag->ColorRGBA.b[2]=Pix->W.B;
        Frag->ColorRGBA.b[3]=Pix->W.A;
        COPYRGBA(Frag->FogRGBA.L,(&SC->FogRGBAs[Pix->W.F]));
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.u +=Pix->L.du;
    Pix->L.v +=Pix->L.dv;
    Pix->L.R +=Pix->L.dR;
    Pix->L.G +=Pix->L.dG;
    Pix->L.B +=Pix->L.dB;
    Pix->L.A +=Pix->L.dA;
    Pix->L.F +=Pix->L.dF;
    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_Gouraud_Fog(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

SREM(Fill_Gouraud_Fog)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        Frag->ColorRGBA.b[0]=Pix->W.R;
        Frag->ColorRGBA.b[1]=Pix->W.G;
        Frag->ColorRGBA.b[2]=Pix->W.B;
        Frag->ColorRGBA.b[3]=Pix->W.A;
        COPYRGBA(Frag->FogRGBA.L,(&SC->FogRGBAs[Pix->W.F]));
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.R +=Pix->L.dR;
    Pix->L.G +=Pix->L.dG;
    Pix->L.B +=Pix->L.dB;
    Pix->L.A +=Pix->L.dA;
    Pix->L.F +=Pix->L.dF;
    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_Gouraud(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

SREM(Fill_Gouraud)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        Frag->ColorRGBA.b[0]=Pix->W.R;
        Frag->ColorRGBA.b[1]=Pix->W.G;
        Frag->ColorRGBA.b[2]=Pix->W.B;
        Frag->ColorRGBA.b[3]=Pix->W.A;
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.R +=Pix->L.dR;
    Pix->L.G +=Pix->L.dG;
    Pix->L.B +=Pix->L.dB;
    Pix->L.A +=Pix->L.dA;
    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_Fog(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

SREM(Fill_Fog)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        COPYRGBA(Frag->ColorRGBA.L,SC->FlatRGBA.L);
        COPYRGBA(Frag->FogRGBA.L,(&SC->FogRGBAs[Pix->W.F]));
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.F +=Pix->L.dF;
    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_TexPersp2_Gouraud(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct SOFT3D_mipmap *MM=SC->MM;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

SREM(Fill_TexPersp2_Gouraud)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        Frag->Tex8 =MM->Tex8U   [Pix->W.u ]+MM->Tex8V   [Pix->W.v ];
        Frag->ColorRGBA.b[0]=Pix->W.R;
        Frag->ColorRGBA.b[1]=Pix->W.G;
        Frag->ColorRGBA.b[2]=Pix->W.B;
        Frag->ColorRGBA.b[3]=Pix->W.A;
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.u +=Pix->L.du;
    Pix->L.v +=Pix->L.dv;
    Pix->L.du+=Pix->L.ddu;
    Pix->L.dv+=Pix->L.ddv;
    Pix->L.R +=Pix->L.dR;
    Pix->L.G +=Pix->L.dG;
    Pix->L.B +=Pix->L.dB;
    Pix->L.A +=Pix->L.dA;
    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_Tex_Gouraud(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct SOFT3D_mipmap *MM=SC->MM;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

    if(SC->state.PerspMode==2)
        {Fill_TexPersp2_Gouraud(SC); return;}
SREM(Fill_Tex_Gouraud)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        Frag->Tex8 =MM->Tex8U   [Pix->W.u ]+MM->Tex8V   [Pix->W.v ];
        Frag->ColorRGBA.b[0]=Pix->W.R;
        Frag->ColorRGBA.b[1]=Pix->W.G;
        Frag->ColorRGBA.b[2]=Pix->W.B;
        Frag->ColorRGBA.b[3]=Pix->W.A;
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.u +=Pix->L.du;
    Pix->L.v +=Pix->L.dv;
    Pix->L.R +=Pix->L.dR;
    Pix->L.G +=Pix->L.dG;
    Pix->L.B +=Pix->L.dB;
    Pix->L.A +=Pix->L.dA;
    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_TexPersp2_Fog(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct SOFT3D_mipmap *MM=SC->MM;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

SREM(Fill_TexPersp2_Fog)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        Frag->Tex8 =MM->Tex8U   [Pix->W.u ]+MM->Tex8V   [Pix->W.v ];
        COPYRGBA(Frag->ColorRGBA.L,SC->FlatRGBA.L);
        COPYRGBA(Frag->FogRGBA.L,(&SC->FogRGBAs[Pix->W.F]));
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.u +=Pix->L.du;
    Pix->L.v +=Pix->L.dv;
    Pix->L.du+=Pix->L.ddu;
    Pix->L.dv+=Pix->L.ddv;
    Pix->L.F +=Pix->L.dF;
    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_Tex_Fog(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct SOFT3D_mipmap *MM=SC->MM;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

    if(SC->state.PerspMode==2)
        {Fill_TexPersp2_Fog(SC); return;}
SREM(Fill_Tex_Fog)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        Frag->Tex8 =MM->Tex8U   [Pix->W.u ]+MM->Tex8V   [Pix->W.v ];
        COPYRGBA(Frag->ColorRGBA.L,SC->FlatRGBA.L);
        COPYRGBA(Frag->FogRGBA.L,(&SC->FogRGBAs[Pix->W.F]));
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.u +=Pix->L.du;
    Pix->L.v +=Pix->L.dv;
    Pix->L.F +=Pix->L.dF;
    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_TexPersp2(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct SOFT3D_mipmap *MM=SC->MM;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;

SREM(Fill_TexPersp2)

//printf("Image8=0x%08lx\n",(uint32_t)Frag->Image8);
//printf("Tex8  =0x%08lx\n",(uint32_t)Frag->Tex8);
//printf("ColorRGBA=0x%08lx\n",(uint32_t)Frag->ColorRGBA.L);
    while(0<high--)
    {
        Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
        large    = Pix->W.large;

        SC->Pix=Pix;
        SC->FunctionZtest(SC);
        Ztest=SC->Ztest;
        while(0<large--)
        {
            if(*Ztest++)
            {
                Frag->Image8=Image8;
                Frag->Tex8 =MM->Tex8U   [Pix->W.u ]+MM->Tex8V   [Pix->W.v ];
                COPYRGBA(Frag->ColorRGBA.L,SC->FlatRGBA.L);
                Frag++;
            }
            Image8=Image8+Pix->L.bpp;
            Pix->L.u +=Pix->L.du;
            Pix->L.v +=Pix->L.dv;
            Pix->L.du+=Pix->L.ddu;
            Pix->L.dv+=Pix->L.ddv;
        }

        if(Frag > SC->FragBufferMaxi)
        {
            SC->FragBufferDone=Frag;
            SOFT3D_Flush(SC);
            Frag=SC->FragBuffer;
        }
        Pix++;
    }
	SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_TexMul(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;
struct SOFT3D_texture *ST=(struct SOFT3D_texture *)swap32((uint32_t)SC->state.ST);
register  UBYTE *tex8=ST->pt;
register  UWORD texU=ST->bits/8;
register  UWORD texV=ST->large*ST->bits/8;

SREM(Fill_TexMul)

    while(0<high--)
    {
    Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
    large       = Pix->W.large;

    SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
    while(0<large--)
    {
    if(*Ztest++)
        {
        Frag->Image8=Image8;
        Frag->Tex8  =tex8;
         Frag->Tex8 +=Pix->W.u * texU;
         Frag->Tex8 +=Pix->W.v * texV;
        COPYRGBA(Frag->ColorRGBA.L,SC->FlatRGBA.L);
        Frag++;
        }
    Image8=Image8+Pix->L.bpp;
    Pix->L.u +=Pix->L.du;
    Pix->L.v +=Pix->L.dv;
    }

    if(Frag > SC->FragBufferMaxi)
        {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
    Pix++;
    }
SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Fill_Tex(struct SOFT3D_context *SC)
{
register UBYTE *Image8;
register UBYTE *Ztest;
register union pixel3D *Pix=SC->Pix;
register struct SOFT3D_mipmap *MM=SC->MM;
register struct fragbuffer3D *Frag=SC->FragBufferDone;
register WORD high  =SC->PolyHigh;
register WORD large;
    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

    if(SC->state.PerspMode==2)
    {
        Fill_TexPersp2(SC);
        DEBUG_SOFT3D("%s end PerspMode=2\n",__FUNCTION__);
        return;
    }

    if(Wazp3D->DebugDriver.ON)    /* Just a way to test that function speed */
        {Fill_TexMul(SC); return;}

    SREM(Fill_Tex)
    while(0<high--)
    {
        Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
        large    = Pix->W.large;

        SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
        while(0<large--)
        {
            if(*Ztest++)
            {
                Frag->Image8=Image8;
                Frag->Tex8 =MM->Tex8U   [Pix->W.u ]+MM->Tex8V   [Pix->W.v ];
                COPYRGBA(Frag->ColorRGBA.L,SC->FlatRGBA.L);
                Frag++;
            }
            Image8=Image8+Pix->L.bpp;
            Pix->L.u +=Pix->L.du;
            Pix->L.v +=Pix->L.dv;
        }

        if(Frag > SC->FragBufferMaxi)
            {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
        Pix++;
    }
    SC->FragBufferDone=Frag;
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*=============================================================*/
void Fill_Flat(struct SOFT3D_context *SC)
{
    register UBYTE *Image8;
    register UBYTE *Ztest;
    register union pixel3D *Pix=SC->Pix;
    register struct fragbuffer3D *Frag=SC->FragBufferDone;
    register WORD high  =SC->PolyHigh;
    register WORD large;

    SREM(Fill_Flat)

    while(0<high--)
    {
        Image8   = Pix->L.Image8Y + SC->Image8X[Pix->W.x];
        large       = Pix->W.large;

        SC->Pix=Pix; SC->FunctionZtest(SC); Ztest=SC->Ztest;
        while(0<large--)
        {
            if(*Ztest++)
            {
                Frag->Image8=Image8;
                COPYRGBA(Frag->ColorRGBA.L,SC->FlatRGBA.L);
                Frag++;
            }
            Image8=Image8+Pix->L.bpp;
        }

        if(Frag > SC->FragBufferMaxi)
            {SC->FragBufferDone=Frag;  SOFT3D_Flush(SC); Frag=SC->FragBuffer;}
        Pix++;
    }
    SC->FragBufferDone=Frag;
}
/*=============================================================*/
void Ztest_znever_update(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(FALSE);  Ztest++; }
}
/*=============================================================*/
void Ztest_zless_update(struct SOFT3D_context *SC)
{
    register UBYTE *Ztest=SC->Ztest;
    register union pixel3D *Pix=SC->Pix;
    register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
    register ZBUFF dz=Pix->L.dz;
    register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE < *Zbuffer ); if(*Ztest) *Zbuffer=ZVALUE; Ztest++; Zbuffer++; Pix->L.z+=dz;}
}
/*=============================================================*/
void Ztest_zgequal_update(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE >= *Zbuffer ); if(*Ztest) *Zbuffer=ZVALUE; Ztest++; Zbuffer++; Pix->L.z+=dz;}
}
/*=============================================================*/
void Ztest_zlequal_update(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE <= *Zbuffer ); if(*Ztest) *Zbuffer=ZVALUE; Ztest++; Zbuffer++; Pix->L.z+=dz;}
}
/*=============================================================*/
void Ztest_zgreater_update(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE > *Zbuffer ); if(*Ztest) *Zbuffer=ZVALUE; Ztest++; Zbuffer++; Pix->L.z+=dz;}
}
/*=============================================================*/
void Ztest_znotequal_update(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE != *Zbuffer ); if(*Ztest) *Zbuffer=ZVALUE; Ztest++; Zbuffer++; Pix->L.z+=dz;}
}
/*=============================================================*/
void Ztest_zequal_update(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE == *Zbuffer ); if(*Ztest) *Zbuffer=ZVALUE; Ztest++; Zbuffer++; Pix->L.z+=dz;}
}
/*=============================================================*/
void Ztest_zalways_update(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(TRUE); *Zbuffer=ZVALUE; Ztest++; Zbuffer++; Pix->L.z+=dz;}
}
/*=============================================================*/
/* void Ztest_znever(struct SOFT3D_context *SC) same as Ztest_znever_update */
/*=============================================================*/
void Ztest_zless(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE < *Zbuffer ); Ztest++; Zbuffer++; Pix->L.z+=dz; }
}
/*=============================================================*/
void Ztest_zgequal(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE >= *Zbuffer ); Ztest++; Zbuffer++; Pix->L.z+=dz; }
}
/*=============================================================*/
void Ztest_zlequal(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE <= *Zbuffer ); Ztest++; Zbuffer++; Pix->L.z+=dz; }
}
/*=============================================================*/
void Ztest_zgreater(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE > *Zbuffer ); Ztest++; Zbuffer++; Pix->L.z+=dz; }
}
/*=============================================================*/
void Ztest_znotequal(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE != *Zbuffer ); Ztest++; Zbuffer++; Pix->L.z+=dz; }
}
/*=============================================================*/
void Ztest_zequal(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register ZBUFF  *Zbuffer= &(Pix->L.ZbufferY[Pix->W.x]);
register ZBUFF dz=Pix->L.dz;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(ZVALUE == *Zbuffer ); Ztest++; Zbuffer++; Pix->L.z+=dz; }
}
/*=============================================================*/
void Ztest_zalways(struct SOFT3D_context *SC)
{
register UBYTE *Ztest=SC->Ztest;
register union pixel3D *Pix=SC->Pix;
register WORD large=Pix->W.large;

    while(0<large--)
        { *Ztest=(TRUE);  Ztest++; }
}
/*=============================================================*/
void EdgeMinDeltas(struct SOFT3D_context *SC,union pixel3D *P1,union pixel3D *P2)
{
/* for selecting mipmap find the minimum uv linear delta */
register WORD size;
register LONG u,v,du,dv;

    if(SC->state.ST==NULL)        /* = Color only so dont bother with mipmapped-texturing*/
        return;

SREM(EdgeMinDeltas)
    PrintPix(P1);
    PrintPix(P2);

    size=P2->W.y - P1->W.y;
    SVAR(size)
    if(size==0)
        size=P2->W.x - P1->W.x;
    SVAR(size)
    if(size==0)
        return;

    u=(P2->L.u - P1->L.u);
    v=(P2->L.v - P1->L.v);

    SVAR(u>>16)
    SVAR(v>>16)
    du=u/size;
    dv=v/size;

    if(du<0)    du=-du;    /* absolute value */
    if(dv<0)    dv=-dv;

    if(du!=0)
    if(du<SC->dmin)            /* find minimal delta = texture step */
        SC->dmin=du;

    if(dv!=0)
    if(dv<SC->dmin)            /* find minimal delta = texture step */
        SC->dmin=dv;

SVAR(du>>16)
SVAR(dv>>16)
SVAR(SC->dmin>>16)
}
/*=============================================================*/
void Edge_Persp_Tex_Gouraud_Fog(struct SOFT3D_context *SC)
{
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register WORD high,n;
double u,v,u2,v2,du,dv,w;
union pixel3D DeltaY;

    high=P2->W.y - P1->W.y;
    if(high<2)    return;
    if(Wazp3D->DoMipMaps.ON)
        EdgeMinDeltas(SC,P1,P2);

SFUNCTION(Edge_Persp_Tex_Gouraud_Fog)
    DeltaY.L.dx=(P2->L.x - P1->L.x)/high;
    DeltaY.L.dz=(P2->L.z - P1->L.z)/high;
    DeltaY.L.dw=(P2->L.w - P1->L.w)/high;
    DeltaY.L.dR=(P2->L.R - P1->L.R)/high;
    DeltaY.L.dG=(P2->L.G - P1->L.G)/high;
    DeltaY.L.dB=(P2->L.B - P1->L.B)/high;
    DeltaY.L.dA=(P2->L.A - P1->L.A)/high;
    DeltaY.L.dF=(P2->L.F - P1->L.F)/high;

    u =(double)P1->L.u * P1->L.w;
    u2=(double)P2->L.u * P2->L.w;
    v =(double)P1->L.v * P1->L.w;
    v2=(double)P2->L.v * P2->L.w;

    du=(u2 - u)/high;
    dv=(v2 - v)/high;

    high--;                        /* ne pas recalculer les points extremites */
    NLOOP(high)
    {
        P1[1].L.x=P1->L.x+DeltaY.L.dx;
        P1[1].L.z=P1->L.z+DeltaY.L.dz;
        P1[1].L.w=P1->L.w+DeltaY.L.dw;
        P1[1].L.R=P1->L.R+DeltaY.L.dR;
        P1[1].L.G=P1->L.G+DeltaY.L.dG;
        P1[1].L.B=P1->L.B+DeltaY.L.dB;
        P1[1].L.A=P1->L.A+DeltaY.L.dA;
        P1[1].L.F=P1->L.F+DeltaY.L.dF;
        w=P1[1].L.w;
        u=u+du;
        v=v+dv;
        P1[1].L.u=u/w;
        P1[1].L.v=v/w;
        P1++;
    }
}
/*=============================================================*/
void Edge_Tex_Gouraud_Fog(struct SOFT3D_context *SC)
{
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register WORD high,n;
union pixel3D DeltaY;

    high=P2->W.y - P1->W.y;
    if(high<2)    return;
    if(Wazp3D->DoMipMaps.ON)
        EdgeMinDeltas(SC,P1,P2);

SFUNCTION(Edge_Tex_Gouraud_Fog)
    DeltaY.L.dx=(P2->L.x - P1->L.x)/high;
    DeltaY.L.dz=(P2->L.z - P1->L.z)/high;
    DeltaY.L.du=(P2->L.u - P1->L.u)/high;
    DeltaY.L.dv=(P2->L.v - P1->L.v)/high;
    DeltaY.L.dR=(P2->L.R - P1->L.R)/high;
    DeltaY.L.dG=(P2->L.G - P1->L.G)/high;
    DeltaY.L.dB=(P2->L.B - P1->L.B)/high;
    DeltaY.L.dA=(P2->L.A - P1->L.A)/high;
    DeltaY.L.dF=(P2->L.F - P1->L.F)/high;

    high--;                        /* ne pas recalculer les points extremites */
    NLOOP(high)
    {
        P1[1].L.x=P1->L.x+DeltaY.L.dx;
        P1[1].L.z=P1->L.z+DeltaY.L.dz;
        P1[1].L.u=P1->L.u+DeltaY.L.du;
        P1[1].L.v=P1->L.v+DeltaY.L.dv;
        P1[1].L.R=P1->L.R+DeltaY.L.dR;
        P1[1].L.G=P1->L.G+DeltaY.L.dG;
        P1[1].L.B=P1->L.B+DeltaY.L.dB;
        P1[1].L.A=P1->L.A+DeltaY.L.dA;
        P1[1].L.F=P1->L.F+DeltaY.L.dF;
        P1++;
    }
}
/*=============================================================*/
void Edge_Gouraud(struct SOFT3D_context *SC)
{
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register WORD high,n;
union pixel3D DeltaY;

    high=P2->W.y - P1->W.y;
    if(high<2)    return;

SFUNCTION(Edge_Gouraud)
    DeltaY.L.dx=(P2->L.x - P1->L.x)/high;
    DeltaY.L.dz=(P2->L.z - P1->L.z)/high;
    DeltaY.L.dR=(P2->L.R - P1->L.R)/high;
    DeltaY.L.dG=(P2->L.G - P1->L.G)/high;
    DeltaY.L.dB=(P2->L.B - P1->L.B)/high;
    DeltaY.L.dA=(P2->L.A - P1->L.A)/high;

    high--;                        /* ne pas recalculer les points extremites */
    NLOOP(high)
    {
        P1[1].L.x=P1->L.x+DeltaY.L.dx;
        P1[1].L.z=P1->L.z+DeltaY.L.dz;
        P1[1].L.R=P1->L.R+DeltaY.L.dR;
        P1[1].L.G=P1->L.G+DeltaY.L.dG;
        P1[1].L.B=P1->L.B+DeltaY.L.dB;
        P1[1].L.A=P1->L.A+DeltaY.L.dA;
        P1++;
    }
}
/*=============================================================*/
void Edge_Flat(struct SOFT3D_context *SC)
{
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register WORD high,n;
union pixel3D DeltaY;

    high=P2->W.y - P1->W.y;
    if(high<2)    return;

SFUNCTION(Edge_Flat)
    DeltaY.L.dx=(P2->L.x - P1->L.x)/high;
    DeltaY.L.dz=(P2->L.z - P1->L.z)/high;

    high--;                        /* ne pas recalculer les points extremites */
    NLOOP(high)
    {
        P1[1].L.x=P1->L.x+DeltaY.L.dx;
        P1[1].L.z=P1->L.z+DeltaY.L.dz;
        P1++;
    }
}
/*=============================================================*/
void Edge_Persp_Tex(struct SOFT3D_context *SC)
{
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register WORD high,n;
double u,v,u2,v2,du,dv,w;
union pixel3D DeltaY;

    high=P2->W.y - P1->W.y;
    if(high<2)    return;
    if(Wazp3D->DoMipMaps.ON)
        EdgeMinDeltas(SC,P1,P2);

SFUNCTION(Edge_Persp_Tex)
    DeltaY.L.dx=(P2->L.x - P1->L.x)/high;
    DeltaY.L.dz=(P2->L.z - P1->L.z)/high;
    DeltaY.L.dw=(P2->L.w - P1->L.w)/high;

    u =(double)P1->L.u * P1->L.w;
    u2=(double)P2->L.u * P2->L.w;
    v =(double)P1->L.v * P1->L.w;
    v2=(double)P2->L.v * P2->L.w;

    du=(u2 - u)/high;
    dv=(v2 - v)/high;

    high--;                        /* ne pas recalculer les points extremites */
    NLOOP(high)
    {
        P1[1].L.x=P1->L.x+DeltaY.L.dx;
        P1[1].L.z=P1->L.z+DeltaY.L.dz;
        P1[1].L.w=P1->L.w+DeltaY.L.dw;
        w=P1[1].L.w;
        u=u+du;
        v=v+dv;
        P1[1].L.u=u/w;
        P1[1].L.v=v/w;
        P1++;
    }
}
/*=============================================================*/
void Edge_Tex(struct SOFT3D_context *SC)
{
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register WORD high,n;
union pixel3D DeltaY;

    high=P2->W.y - P1->W.y;
    if(high<2)    return;
    if(Wazp3D->DoMipMaps.ON)
        EdgeMinDeltas(SC,P1,P2);

SFUNCTION(Edge_Tex)
    DeltaY.L.dx=(P2->L.x - P1->L.x)/high;
    DeltaY.L.dz=(P2->L.z - P1->L.z)/high;
    DeltaY.L.du=(P2->L.u - P1->L.u)/high;
    DeltaY.L.dv=(P2->L.v - P1->L.v)/high;

    high--;                        /* ne pas recalculer les points extremites */
    NLOOP(high)
    {
        P1[1].L.x=P1->L.x+DeltaY.L.dx;
        P1[1].L.z=P1->L.z+DeltaY.L.dz;
        P1[1].L.u=P1->L.u+DeltaY.L.du;
        P1[1].L.v=P1->L.v+DeltaY.L.dv;
        P1++;
    }

}
/*=============================================================*/
void SOFT3D_SetDrawState(APTR sc,APTR sta)
{
    struct state3D *state=sta;
    struct SOFT3D_context *SC=sc;
    struct SOFT3D_texture *ST=(struct SOFT3D_texture *)swap32((uint32_t)state->ST);

    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
    SFUNCTION(SOFT3D_SetDrawState)
    if(Wazp3D->DebugSOFT3D.ON)
    {
        Libprintf(" state:Changed%ld UseTex%ld ST0x%08lx ZMode%ld TexEnvMode%ld BlendMode%ld UseGouraud%ld UseFog%ld\n",(ULONG)state->Changed,(ULONG)state->UseTex,swap32((ULONG)state->ST),(ULONG)state->ZMode,(ULONG)state->TexEnvMode,(ULONG)state->BlendMode,(ULONG)state->UseGouraud,(ULONG)state->UseFog);
        PrintST((struct SOFT3D_texture *)swap32((uint32_t)state->ST));
        Libprintf(" CurrentRGBA: %ld %ld %ld %ld\n",(ULONG)state->CurrentRGBA.b[0],(ULONG)state->CurrentRGBA.b[1],(ULONG)state->CurrentRGBA.b[2],(ULONG)state->CurrentRGBA.b[3]);
    }

    if(!state->Changed)
    {
        SREM( state unchanged)        /* not changed ==> do nothing */
        DEBUG_SOFT3D("%s end\n",__FUNCTION__);
        return;
    }

    if(Wazp3D->UseStateTracker.ON)
    if(!Wazp3D->PrefsIsOpened)            /* if the user dont change states */
    if(SC->state.ST        ==state->ST)
    if(SC->state.ZMode     ==state->ZMode)
    if(SC->state.BlendMode ==state->BlendMode)
    if(SC->state.TexEnvMode==state->TexEnvMode)
    if(SC->state.PerspMode ==state->PerspMode)
    if(SC->state.CullingMode==state->CullingMode)
    if(SC->state.FogMode    ==state->FogMode)
    if(SC->state.UseGouraud ==state->UseGouraud)
    if(SC->state.UseTex     ==state->UseTex)
    if(SC->state.UseFog     ==state->UseFog)
    if(SAMERGBA(SC->state.FogRGBA.L        ,state->FogRGBA.L))
    if(SAMERGBA(SC->state.CurrentRGBA.L    ,state->CurrentRGBA.L))
    if(SAMERGBA(SC->state.EnvRGBA.L        ,state->EnvRGBA.L))
    if(SC->state.PointSize  ==state->PointSize)
    if(SC->state.LineSize   ==state->LineSize)
    if(SC->state.FogZmin    ==state->FogZmin)
    if(SC->state.FogZmax    ==state->FogZmax)
    if(SC->state.FogDensity ==state->FogDensity)
    {
        SREM( all same states)            /* if nothing truly changed ==> do nothing */
        SC->state.Changed=state->Changed=FALSE;
        DEBUG_SOFT3D("%s end\n",__FUNCTION__);
        return;
    }

    SC->state.ZMode         =state->ZMode;
    SC->state.BlendMode     =state->BlendMode;
    SC->state.TexEnvMode    =state->TexEnvMode;
    SC->state.PerspMode     =state->PerspMode;
    SC->state.CullingMode   =state->CullingMode;
    SC->state.FogMode       =state->FogMode;
    SC->state.UseGouraud    =state->UseGouraud;
    SC->state.UseTex        =state->UseTex;
    SC->state.UseFog        =state->UseFog;

    COPYRGBA(SC->state.FogRGBA.L        ,state->FogRGBA.L);
    COPYRGBA(SC->state.CurrentRGBA.L    ,state->CurrentRGBA.L);
    COPYRGBA(SC->state.EnvRGBA.L        ,state->EnvRGBA.L);
    COPYRGBA(SC->state.BackRGBA.L       ,state->BackRGBA.L);
    SC->state.PointSize     =state->PointSize;
    SC->state.LineSize      =state->LineSize;
    SC->state.FogZmin       =state->FogZmin;
    SC->state.FogZmax       =state->FogZmax;
    SC->state.FogDensity    =state->FogDensity;
    SC->state.ST            =state->ST;

/*hard stuffs */
    SC->state.gltex        =0;
    if(ST!=NULL)
        SC->state.gltex   =ST->HT.gltex;    /*direct access to gltex value */
    SC->HC.state=&SC->state;            /*direct access to state */

/*  flush remaining pixels before any changes */
    SOFT3D_Flush(SC);

/* Now change hardware/software drawing functions */
#ifdef USEOPENGL
    if(SC->UseHard)    HARD3D_SetDrawFunctions(&SC->HC);
#endif
    if(!SC->UseHard)    SOFT3D_SetDrawFunctions(sc);

    SC->state.Changed=state->Changed=FALSE;     /* change treated */
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*=============================================================*/
void SOFT3D_SetDrawFunctions(APTR sc)
{
    struct SOFT3D_context *SC=sc;
    struct SOFT3D_texture *ST=(struct SOFT3D_texture *)swap32((uint32_t)SC->state.ST);
    UBYTE FillMode,EdgeMode,TexEnvMode,BlendMode,UseFog,UseGouraud,UseBigTex,UseTex24,UseBuffer;
    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

    SFUNCTION(SOFT3D_SetDrawFunctions)

/* We may change those state values for a faster soft-rendering */
    ST            =(struct SOFT3D_texture *)swap32((uint32_t)SC->state.ST);
    TexEnvMode    =SC->state.TexEnvMode;
    if(!SC->state.UseTex)
        TexEnvMode    =0;
    BlendMode     =SC->state.BlendMode;
    UseFog        =SC->state.UseFog;
    UseGouraud    =SC->state.UseGouraud;
//    printf("ST 0x%08lx\n",(uint32_t)ST);

    if(Wazp3D->DebugSOFT3D.ON)
        Libprintf(" FlatRGBA: %ld %ld %ld %ld\n",(ULONG)SC->FlatRGBA.b[0],(ULONG)SC->FlatRGBA.b[1],(ULONG)SC->FlatRGBA.b[2],(ULONG)SC->FlatRGBA.b[3]);

    if(UseGouraud==FALSE)                    /* if truly want flat color*/
    {
        SREM(True Flat)
        if(Wazp3D->TexMode.ON!=3)             /* in this case flatcolor is already polygon's averagecolor */
        COPYRGBA(SC->FlatRGBA.L,SC->state.CurrentRGBA.L);    /* if true no-gouraud mode then flat-color come from current-color  */
    }

    if(!SC->ColorChange)                    /* then dont truly need gouraud shading */
    {
        SREM(Gouraud to Flat)
        UseGouraud=FALSE;                    /* then flat color come from point-color NOT from current-color */
    }

    UseTex24=FALSE;            /* default 32 bits color */
    UseBigTex=FALSE;
    if(SC->state.UseTex)
    {
        if(256<ST->large) UseBigTex=TRUE;
        if(256<ST->high ) UseBigTex=TRUE;
        UseTex24=(ST->bits==24);    /* but the tex can be only 24 bits */
    }

    if( (BlendMode==BLENDFASTALPHA) ou (BlendMode==BLENDNOALPHA) )
    if(TexEnvMode>W3D_REPLACE)        /* if got a coloring function */
    if(SC->ColorTransp)            /* that colorize in alpha (=make texture transparent)*/
    {
        SREM(ColorTransp so really use BLENDALPHA)
        BlendMode=BLENDALPHA;            /* then use the tex as a true alpha one */
    }

    if(Wazp3D->QuakeMatrixPatch.ON)            /* patch: for glmatrix transparency */
    if(BlendMode==W3D_ONE_MINUS_SRC_ALPHA*16 +  W3D_SRC_ALPHA)
        BlendMode=W3D_SRC_ALPHA*16 + W3D_ONE_MINUS_SRC_ALPHA;

/* if color is white dont do a stupid flat-color modulate on a texture = do nothing */
    if(TexEnvMode>W3D_REPLACE)            /* with a coloring function */
    if(!SC->ColorChange)
    if(SC->ColorWhite)
    {
        SREM(Color is white so no Coloring)
        TexEnvMode=W3D_REPLACE;
    }

    if(Wazp3D->DebugTexColor.ON)
    if(SC->state.UseTex)
    if(ST!=NULL)            /* display the parameters as the texture color */
    {
        SC->FlatRGBA.b[0]=ST->Tnum; SC->FlatRGBA.b[1]=TexEnvMode; SC->FlatRGBA.b[2]=BlendMode;
        TexEnvMode=0;    BlendMode=BLENDREPLACE; UseFog=FALSE;    UseGouraud=SC->ColorChange=FALSE;
    }

    FillMode=SC->state.UseTex*4  +  UseGouraud*2 + UseFog*1;
    SC->FunctionFill    =Functions.Fill[FillMode];

    if(UseBigTex)
    {
        SC->FunctionFill=(HOOKEDFUNCTION)Fill_BigTexPersp2_Gouraud_Fog;
        if(!UseFog)
        if(!UseGouraud)
            SC->FunctionFill=(HOOKEDFUNCTION)Fill_BigTexPersp2;
    }

    EdgeMode=(SC->state.PerspMode!=0)*8+FillMode;
    SC->FunctionEdge=Functions.Edge[EdgeMode];

    if(SC->state.PerspMode==2)
        SC->FunctionPoly=(HOOKEDFUNCTION)Poly_Persp2x2_Tex_Gouraud_Fog;

    if(SC->state.PerspMode==1)
    {
        SC->FunctionPoly=(HOOKEDFUNCTION)Poly_Persp1_Tex_Gouraud_Fog;
        if(!SC->state.UseTex)
        if(!UseFog)
            {
            if(UseGouraud)
                SC->FunctionPoly=(HOOKEDFUNCTION)Poly_Persp1_Gouraud;
            else
                SC->FunctionPoly=(HOOKEDFUNCTION)Poly_Persp1_Flat;
            }
        if(SC->state.UseTex)
        if(!UseFog)
        if(!UseGouraud)
            SC->FunctionPoly=(HOOKEDFUNCTION)Poly_Persp1_Tex;
    }

    if(SC->state.PerspMode==0)
    {
        SC->FunctionPoly=(HOOKEDFUNCTION)Poly_Persp0_Tex_Gouraud_Fog;
        if(!SC->state.UseTex)
        if(!UseFog)
        {
            if(UseGouraud)
                SC->FunctionPoly=(HOOKEDFUNCTION)Poly_Persp0_Gouraud;
            else
                SC->FunctionPoly=(HOOKEDFUNCTION)Poly_Persp0_Flat;
        }
        if(SC->state.UseTex)
        if(!UseFog)
        if(!UseGouraud)
            SC->FunctionPoly=(HOOKEDFUNCTION)Poly_Persp0_Tex;
    }

    if(SC->Zbuffer==NULL)
        SC->state.ZMode=ZMODE(0,W3D_Z_ALWAYS);

    SC->FunctionZtest    =Functions.Ztest [SC->state.ZMode];
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_znever_update)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zless)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zgequal)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zlequal)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zgreater)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_znotequal)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zequal)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zalways)

    SINFO((void *)SC->FunctionZtest,(void *)Ztest_znever_update)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zless_update)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zgequal_update)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zlequal_update)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zgreater_update)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_znotequal_update)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zequal_update)
    SINFO((void *)SC->FunctionZtest,(void *)Ztest_zalways_update)

    SC->FunctionIn        =(HOOKEDFUNCTION)SC->FunctionBitmapIn;
    SC->FunctionTexEnv    =NULL;
    SC->FunctionBlend     =NULL;
    SC->FunctionBlendFast =NULL;
    SC->FunctionFog       =NULL;
    SC->FunctionFilter    =NULL;
    SC->FunctionSepia     =NULL;
    SC->FunctionOut       =(HOOKEDFUNCTION)SC->FunctionBitmapOut;

/* FunctionTexEnv */
    SC->FunctionTexEnv    =Functions.TexEnv[TexEnvMode*2+!UseTex24];

/* FunctionBlend */
    SC->SrcFunc=BlendMode/16; SC->DstFunc=BlendMode-SC->SrcFunc*16;

    if(UseTex24)
    if(SC->FunctionTexEnv==NULL)        /* if no function to convert a 24bits tex to 32bits color with alpha*/
    {
        SREM( tex dont have alpha)
        if(SC->SrcFunc==W3D_SRC_ALPHA)            SC->SrcFunc=W3D_ONE;
        if(SC->SrcFunc==W3D_ONE_MINUS_SRC_ALPHA)        SC->SrcFunc=W3D_ZERO;
        if(SC->SrcFunc==W3D_SRC_ALPHA_SATURATE)        SC->SrcFunc=W3D_ONE;
        if(SC->DstFunc==W3D_SRC_ALPHA)            SC->DstFunc=W3D_ONE;
        if(SC->DstFunc==W3D_ONE_MINUS_SRC_ALPHA )        SC->DstFunc=W3D_ZERO;
            BlendMode=(SC->SrcFunc*16 + SC->DstFunc);
    }

    if(SC->bits==24)                /* if screen dont have  alpha*/
    {
        SREM( screen dont have alpha)
        if(SC->SrcFunc==W3D_DST_ALPHA)            SC->SrcFunc=W3D_ONE;
        if(SC->SrcFunc==W3D_ONE_MINUS_DST_ALPHA)        SC->SrcFunc=W3D_ZERO;
        if(SC->DstFunc==W3D_DST_ALPHA)            SC->DstFunc=W3D_ONE;
        if(SC->DstFunc==W3D_ONE_MINUS_DST_ALPHA)        SC->DstFunc=W3D_ZERO;
        BlendMode=(SC->SrcFunc*16 + SC->DstFunc);
    }

    if(SC->state.CurrentRGBA.b[3]==255)        /* if current-color dont have  alpha*/
    {
        SREM( CurrentRGBA dont have alpha)
        if(SC->SrcFunc==W3D_CONSTANT_ALPHA)            SC->SrcFunc=W3D_ONE;
        if(SC->SrcFunc==W3D_ONE_MINUS_CONSTANT_ALPHA)    SC->SrcFunc=W3D_ZERO;
        if(SC->DstFunc==W3D_CONSTANT_ALPHA)            SC->DstFunc=W3D_ONE;
        if(SC->DstFunc==W3D_ONE_MINUS_CONSTANT_ALPHA)    SC->DstFunc=W3D_ZERO;
        BlendMode=(SC->SrcFunc*16 + SC->DstFunc);
    }

    SC->FunctionBlend        =(HOOKEDFUNCTION)PixelsBlendFunctionAll;    /* default is an universal (slow) blend function */
    SC->FunctionBlendFast    =(HOOKEDFUNCTION)Functions.BlendFast[BlendMode];

    UseBuffer=(SC->FunctionOut!=NULL);
    if(BlendMode==BLENDREPLACE)
    {
        SC->FunctionBlend =(HOOKEDFUNCTION)PixelsColorToImage;
        if(UseBuffer)
            SC->FunctionBlend=(HOOKEDFUNCTION)PixelsColorToBuffer;

        if(TexEnvMode==W3D_REPLACE)        /* if no texenv function to convert a 24bits tex to 32bits color*/
        {
            SC->FunctionTexEnv= NULL;    /* FunctionTexEnv==FunctionBlend==REPLACE */
            if(UseTex24)
            {
                SC->FunctionBlend=(HOOKEDFUNCTION)PixelsTex24ToImage;
                if(UseBuffer)
                    SC->FunctionBlend=(HOOKEDFUNCTION)PixelsTex24ToBuffer;
            }
            else
            {
                SC->FunctionBlend=(HOOKEDFUNCTION)PixelsTex32ToImage;
                if(UseBuffer)
                    SC->FunctionBlend=(HOOKEDFUNCTION)PixelsTex32ToBuffer;
            }
            SC->FunctionIn=NULL;        /* not need to read Image's pixels = : they are all replaced */
        }
    }

    if(Wazp3D->DebugSOFT3D.ON)
        Libprintf("Functions set with TexEnvMode%ld BlendMode%ld\n",(ULONG)TexEnvMode,(ULONG)BlendMode);

/* FunctionFog */
    if(UseFog)
    {
        SC->FunctionFog=(HOOKEDFUNCTION)PixelsFogToImage;
        if(UseBuffer)
            SC->FunctionFog=(HOOKEDFUNCTION)PixelsFogToBuffer;
    }

/* FunctionFilter */
    if(Wazp3D->UseFiltering.ON)
    {
        SC->FunctionFilter=(HOOKEDFUNCTION)PixelsFilterToImage;
        if(UseBuffer)
            SC->FunctionFilter=(HOOKEDFUNCTION)PixelsFilterToBuffer;
    }

/* FunctionSepia */
    if(Wazp3D->DebugSepiaImage.ON)
    {
        SC->FunctionSepia=(HOOKEDFUNCTION)PixelsSepiaToImage;
        if(UseBuffer)
            SC->FunctionSepia=(HOOKEDFUNCTION)PixelsSepiaToBuffer;
    }


/* patch suggestion from "Cosmos" */
    if(SC->FunctionIn        ==NULL)
    if(SC->FunctionTexEnv    ==NULL)
    if(SC->FunctionBlend    ==(HOOKEDFUNCTION)PixelsTex24ToBuffer)
    if(SC->FunctionFog    ==NULL)
    if(SC->FunctionFilter    ==NULL)
    if(SC->FunctionSepia    ==NULL)
    {
        if(SC->FunctionOut     ==(HOOKEDFUNCTION)PixelsOutBGRA)
        {
            SC->FunctionBlend        =NULL;
            SC->FunctionOut         =(HOOKEDFUNCTION)PixelsOutBGRAfromtex24;
        }
        if(SC->FunctionOut     ==(HOOKEDFUNCTION)PixelsOutARGB)
        {
            SC->FunctionBlend        =NULL;
            SC->FunctionOut         =(HOOKEDFUNCTION)PixelsOutARGBfromtex24;
        }
    }

    if(FillMode==0)            /*  flat color like Blender gui */
    if(SC->FunctionBlend==NULL)    /*  not transparent */
    {
        if(SC->FunctionOut==(HOOKEDFUNCTION)PixelsOutRGBA)
            SC->FunctionOut         =(HOOKEDFUNCTION)PixelsOutRGBA_Flat;
        if(SC->FunctionOut==(HOOKEDFUNCTION)PixelsOutBGRA)
            SC->FunctionOut         =(HOOKEDFUNCTION)PixelsOutBGRA_Flat;
        if(SC->FunctionOut==(HOOKEDFUNCTION)PixelsOutARGB)
            SC->FunctionOut         =(HOOKEDFUNCTION)PixelsOutARGB_Flat;
        if(SC->FunctionOut==(HOOKEDFUNCTION)PixelsOutABGR)
            SC->FunctionOut         =(HOOKEDFUNCTION)PixelsOutABGR_Flat;
    }

    ChangeSoftPoint(SC);
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*=============================================================*/
void SOFT3D_Flush(APTR sc)
{
struct SOFT3D_context *SC=sc;

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(SOFT3D_Flush)
#ifdef USEOPENGL
    if(SC->UseHard)
     {
    HARD3D_Flush(&SC->HC);
    return;
    }
#endif

#if defined(OS4COMPOSITING)
    if(SC->UseHard)  return;
#endif

/* Will draw what is in the Fragments Buffer */
    SC->FragSize2=(SC->FragBufferDone - SC->FragBuffer + 3)/2;
    if(SC->FragSize2==0) {SREM( ==> No fragments to draw);goto DrawFragBufferEnd;}
    SVAR(SC->FragSize2)
    if(SC->Image8==NULL) return;

/* v52: add three (noop) fragments to FragBuffer so can do size2=(size + 1)/2 and so can process 2 (or even 4) Frags inside a loop   */
    SC->FragBufferDone[0].Tex8    =SC->NoopRGBA;    /* read the noop pixel as texture */
    SC->FragBufferDone[1].Tex8    =SC->NoopRGBA;
    SC->FragBufferDone[2].Tex8    =SC->NoopRGBA;
    SC->FragBufferDone[0].Image8    =SC->NoopRGBA;    /* write it in noop pixel = do nothing with Image */
    SC->FragBufferDone[1].Image8    =SC->NoopRGBA;
    SC->FragBufferDone[2].Image8    =SC->NoopRGBA;

    if(SC->FunctionIn!=NULL)
    {
SREM(Pass1: FunctionIn)
/* cant print inside lockbitmap so print the functionIn/Out name here */
    SINFO((void *)SC->FunctionIn,(void *)NULL)
    SINFO((void *)SC->FunctionIn,(void *)PixelsInBGRA)
    SINFO((void *)SC->FunctionIn,(void *)PixelsInRGBA)
    SINFO((void *)SC->FunctionIn,(void *)PixelsInARGB)
    SINFO((void *)SC->FunctionIn,(void *)PixelsInABGR)
    SINFO((void *)SC->FunctionIn,(void *)PixelsInRGB)
    SINFO((void *)SC->FunctionIn,(void *)PixelsInBGR)
    SINFO((void *)SC->FunctionIn,(void *)PixelsIn16)
    SINFO((void *)SC->FunctionIn,(void *)PixelsIn8)
		if(!LockBM(SC))
			goto DrawFragBufferEnd;        /* if cant read pixels then cancel this drawing */

		SC->FunctionIn(SC);            /* convert readed pixels to RGBA format. */
    	UnLockBM(SC);
    }

    if(SC->FunctionTexEnv!=NULL)
    {
SREM(Pass2: FunctionTexEnv (coloring/light) )
		SC->FunctionTexEnv(SC);
    }

    if(SC->FunctionBlend!=NULL)
    {
SREM(Pass3: FunctionBlend (transparency) )
		SC->FunctionBlend(SC);
    }

    if(SC->FunctionFog!=NULL)
    {
SREM(Pass4: FunctionFog)
		SC->FunctionFog(SC);
    }

    if(SC->FunctionFilter!=NULL)
    {
SREM(Pass 5: FunctionFilter )
		SC->FunctionFilter(SC);
    }

    if(SC->FunctionSepia!=NULL)
    {
SREM(Pass 6: FunctionSepia: debug effect )
		SC->FunctionSepia(SC);
    }

    if(SC->FunctionOut!=NULL)
    {
SREM(Pass 7:FunctionOut)
/* cant print inside lockbitmap so print the functionIn/Out name here */
    SINFO((void *)SC->FunctionOut,(void *)NULL)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutBGRA)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutRGBA)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutARGB)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutABGR)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutBGRA_Flat)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutRGBA_Flat)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutARGB_Flat)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutABGR_Flat)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutRGB)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutBGR)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOut16)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOut8)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutBGRAfromtex24)
    SINFO((void *)SC->FunctionOut,(void *)PixelsOutARGBfromtex24)
        if(LockBM(SC))
        {
        	SC->FunctionOut(SC);
        	UnLockBM(SC);
        }
    }

SREM(Flush done...)
DrawFragBufferEnd:
    SC->FragBufferDone =SC->FragBuffer;

}
/*=============================================================*/
void DrawSimplePix(struct SOFT3D_context *SC,register union pixel3D *P)
{
    struct SOFT3D_texture *ST=(struct SOFT3D_texture *)swap32((uint32_t)SC->state.ST);

SFUNCTION(DrawPointSimplePIX)
    if ( (P->W.x < 0) ou (P->W.y < 0) ou (SC->large <= P->W.x) ou (SC->high <= P->W.y))
        return;

    SC->Pix  =&(SC->edge1[P->W.y]);
    COPYPIX(SC->Pix,P);
    SC->Pix->W.large=1;
    SC->PolyHigh=1;

    if(ST!=NULL)
        SC->MM=&ST->MMs[0];    /* default to biggest mipmap <=> never call FunctionFill with SC->MM=NULL*/
    SC->FunctionFill(SC);
}
/*=============================================================*/
void DrawPointPix(struct SOFT3D_context *SC)
{
    register union pixel3D *P=SC->PolyPix;
    register WORD *PointLarge=SC->PointLarges;
    register WORD xmin,xmax,ymin,ymax;
    register UWORD y;
    register WORD R;
    struct SOFT3D_texture *ST=(struct SOFT3D_texture *)swap32((uint32_t)SC->state.ST);
    struct state3D flatstate;
    flatstate.EnvRGBA.L[0]=0;
    flatstate.FogRGBA.L[0]=0;
    flatstate.BackRGBA.L[0]=0;
    flatstate.CurrentRGBA.L[0]=0;
    flatstate.FogMode=0;
    flatstate.FogZmax=0;
    flatstate.FogZmin=0;
    flatstate.FogDensity=0;

    DEBUG_SOFT3D("%s start\n",__FUNCTION__);
    SFUNCTION(DrawPointPIX)
    SVAR(SC->state.PointSize)
    SC->ColorChange=FALSE;

/* point as tex nofog nogouraud just color */
    flatstate.ZMode    =SC->state.ZMode;
    flatstate.PointSize=swap32(SC->state.PointSize);
    flatstate.LineSize =swap32(SC->state.LineSize);
    flatstate.BlendMode=SC->state.BlendMode;

    flatstate.TexEnvMode=0;
    flatstate.PerspMode=0;
    flatstate.CullingMode=W3D_NOW;
    flatstate.UseGouraud=FALSE;
    flatstate.UseTex=FALSE;
    flatstate.UseFog=FALSE;
    flatstate.ST=NULL;
    flatstate.Changed=TRUE;

    SOFT3D_SetDrawState(SC,&flatstate);

    COPYRGBA(SC->FlatRGBA.L,SC->PolyP->RGBA.L);

    if(swap32(SC->state.PointSize)==1.0)
    {
        DrawSimplePix(SC,P);
        return;
    }

    R=swap32(SC->state.PointSize)/2+1;
    xmin=0+R;
    ymin=0+R;
    xmax=SC->large-R;
    ymax=SC->high -R;
    if ( (P->W.x<xmin) ou (P->W.y<ymin) ou (xmax<=P->W.x) ou (ymax<=P->W.y))
        return;

    P->W.y    =P->W.y - swap32(SC->state.PointSize)/2.0;
    SC->Pix    =&(SC->edge1[P->W.y]);
    SC->PolyHigh=swap32(SC->state.PointSize);

    YLOOP(SC->PolyHigh)
    {
        COPYPIX(&SC->Pix[y],P);
        SC->Pix[y].W.large =PointLarge[y];
        SC->Pix[y].W.x     =SC->Pix[y].W.x - PointLarge[y]/2;
    }

    if(ST!=NULL)
        SC->MM=&ST->MMs[0];    /* default to biggest mipmap <=> never call FunctionFill with SC->MM=NULL*/
    SC->FunctionFill(SC);
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*=============================================================*/
void DoDeltasUV(union pixel3D *P1,union pixel3D *P2)
{
/* V44: Compute a quadratic approximation of texture to emulate perspective */
double a,b,z0,z2,n,dt,rz4dt;
double u0,u2,du,ddu;
double v0,v2,dv,ddv;

    n=P1->W.large;
    if(P1->L.w==P2->L.w)
    {
        P1->L.du=(P2->L.u - P1->L.u)/n;
        P1->L.dv=(P2->L.v - P1->L.v)/n;
        P1->L.ddu=P1->L.ddv=0;
        return;
    }

    z0=1.0/P1->L.w;
    z2=1.0/P2->L.w;

    u0=P1->L.u;
    u2=P2->L.u;
    v0=P1->L.v;
    v2=P2->L.v;

    dt  = u2-u0;
    rz4dt = (4.0*dt*z0) / (z0 + z2) ;
    a  = ( -rz4dt +dt +dt ) / (n*n) ;
    b  = ( +rz4dt -dt )      /  (n) ;
     du = a + b ;
    ddu = a + a ;

    dt  = v2-v0;
    rz4dt = (4.0*dt*z0) / (z0 + z2) ;
    a  = ( -rz4dt +dt +dt ) / (n*n) ;
    b  = ( +rz4dt -dt )      /  (n) ;
     dv = a + b ;
    ddv = a + a ;

/* store ddu ddv du dv as LONG */
    P1->L.du    = du  ;
    P1->L.dv    = dv  ;
    P1->L.ddu    = ddu ;
    P1->L.ddv    = ddv ;

}
/*=============================================================*/
void Poly_Persp0_Tex(struct SOFT3D_context *SC)
{
    register union pixel3D *P1=SC->P1;
    register union pixel3D *P2=SC->P2;
    register LONG high=SC->PolyHigh;
    register LONG n;
    register WORD large,maxlarge,maxn;
    union pixel3D DeltaX;

    DeltaX.L.dz=0;
    DeltaX.L.du=0;
    DeltaX.L.dv=0;

    SFUNCTION(Poly_Persp0_Tex)
    maxn=maxlarge=0;
    NLOOP(high)            /* trouver le large max du polygone = meilleur delta */
    {
        large=P2->W.x - P1->W.x; P1->W.large=large;

        if(maxlarge < large)
            {maxn=n; maxlarge=large;}

        P1++;P2++;
    }

/* compute best delta */
    P1=&SC->P1[maxn];
    P2=&SC->P2[maxn];

    if(maxlarge!=0)
    {
        DeltaX.L.dz=(P2->L.z - P1->L.z)/maxlarge;
        DeltaX.L.du=(P2->L.u - P1->L.u)/maxlarge;
        DeltaX.L.dv=(P2->L.v - P1->L.v)/maxlarge;
    }


SVAR(maxlarge)
PrintPix(P1);
PrintPix(P2);
PrintPix(&DeltaX);


/* and copy same delta for all segments */
    P1=SC->P1;
    NLOOP(high)
    {
        P1->L.dz    =DeltaX.L.dz;
        P1->L.du    =DeltaX.L.du;
        P1->L.dv    =DeltaX.L.dv;
        P1->L.ddu    =0;
        P1->L.ddv    =0;
        P1++;
    }

    SC->Pix=SC->P1;
    SelectMipMap(SC);
    SC->FunctionFill(SC);
}
/*=============================================================*/
void Poly_Persp0_Gouraud(struct SOFT3D_context *SC)
{
    register union pixel3D *P1=SC->P1;
    register union pixel3D *P2=SC->P2;
    register LONG high=SC->PolyHigh;
    register LONG n;
    register WORD large,maxlarge,maxn;
    union pixel3D DeltaX;

    DeltaX.L.dz=0;
    DeltaX.L.dR=0;
    DeltaX.L.dG=0;
    DeltaX.L.dB=0;
    DeltaX.L.dA=0;

SFUNCTION(Poly_Persp0_Gouraud)
    maxn=maxlarge=0;
    NLOOP(high)            /* trouver le large max du polygone = meilleur delta */
    {
        large=P2->W.x - P1->W.x; P1->W.large=large;

        if(maxlarge < large)
            {maxn=n; maxlarge=large;}
        P1++;P2++;
    }

/* compute best delta */
    P1=&SC->P1[maxn];
    P2=&SC->P2[maxn];
    if(maxlarge!=0)
    {
        DeltaX.L.dz=(P2->L.z - P1->L.z)/maxlarge;
        DeltaX.L.dR=(P2->L.R - P1->L.R)/maxlarge;
        DeltaX.L.dG=(P2->L.G - P1->L.G)/maxlarge;
        DeltaX.L.dB=(P2->L.B - P1->L.B)/maxlarge;
        DeltaX.L.dA=(P2->L.A - P1->L.A)/maxlarge;
    }

/* and copy same delta for all segments */
    P1=SC->P1;
    NLOOP(high)
    {
        P1->L.dz    =DeltaX.L.dz;
        P1->L.dR    =DeltaX.L.dR;
        P1->L.dG    =DeltaX.L.dG;
        P1->L.dB    =DeltaX.L.dB;
        P1->L.dA    =DeltaX.L.dA;
        P1++;
    }

    SC->Pix=SC->P1;
    SC->FunctionFill(SC);
}
/*=============================================================*/
void Poly_Persp0_Flat(struct SOFT3D_context *SC)
{
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register LONG high=SC->PolyHigh;
register LONG n;
register WORD large;

SFUNCTION(Poly_Persp0_Flat)
/* compute a delta per segment */
    NLOOP(high)
    {
        large=P2->W.x - P1->W.x; P1->W.large=large;

        if(large!=0)
        {
            P1->L.dz=(P2->L.z - P1->L.z)/large;
        }
        P1++;P2++;
    }

    SC->Pix=SC->P1;
    SC->FunctionFill(SC);
}
/*=============================================================*/
void Poly_Persp0_Tex_Gouraud_Fog(struct SOFT3D_context *SC)
{
    register union pixel3D *P1=SC->P1;
    register union pixel3D *P2=SC->P2;
    register LONG high=SC->PolyHigh;
    register LONG n;
    register WORD large,maxlarge,maxn;
    union pixel3D DeltaX;

    DeltaX.L.dz=0;
    DeltaX.L.du=0;
    DeltaX.L.dv=0;
    DeltaX.L.dR=0;
    DeltaX.L.dG=0;
    DeltaX.L.dB=0;
    DeltaX.L.dA=0;
    DeltaX.L.dF=0;

    SFUNCTION(Poly_Persp0_Tex_Gouraud_Fog)
    maxn=maxlarge=0;
    NLOOP(high)            /* trouver le large max du polygone = meilleur delta */
    {
        large=P2->W.x - P1->W.x; P1->W.large=large;

        if(maxlarge < large)
            {maxn=n; maxlarge=large;}
        P1++;P2++;
    }

/* compute best delta */
    P1=&SC->P1[maxn];
    P2=&SC->P2[maxn];
    if(maxlarge!=0)
    {
        DeltaX.L.dz=(P2->L.z - P1->L.z)/maxlarge;
        DeltaX.L.du=(P2->L.u - P1->L.u)/maxlarge;
        DeltaX.L.dv=(P2->L.v - P1->L.v)/maxlarge;
        DeltaX.L.dR=(P2->L.R - P1->L.R)/maxlarge;
        DeltaX.L.dG=(P2->L.G - P1->L.G)/maxlarge;
        DeltaX.L.dB=(P2->L.B - P1->L.B)/maxlarge;
        DeltaX.L.dA=(P2->L.A - P1->L.A)/maxlarge;
        DeltaX.L.dF=(P2->L.F - P1->L.F)/maxlarge;
    }

/* and copy same delta for all segments */
    P1=SC->P1;
    NLOOP(high)
    {
        P1->L.dz    =DeltaX.L.dz;
        P1->L.du    =DeltaX.L.du;
        P1->L.dv    =DeltaX.L.dv;
        P1->L.dR    =DeltaX.L.dR;
        P1->L.dG    =DeltaX.L.dG;
        P1->L.dB    =DeltaX.L.dB;
        P1->L.dA    =DeltaX.L.dA;
        P1->L.dF    =DeltaX.L.dF;
        P1->L.ddu   =0;
        P1->L.ddv   =0;
        P1++;
    }

    SC->Pix=SC->P1;
    SelectMipMap(SC);
    SC->FunctionFill(SC);
}
/*=============================================================*/
void Poly_Persp1_Tex(struct SOFT3D_context *SC)
{
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register LONG high=SC->PolyHigh;
register LONG n;
register WORD large;

SFUNCTION(Poly_Persp1_Tex)
/* compute a delta per segment */
    NLOOP(high)
    {
    large=P2->W.x - P1->W.x; P1->W.large=large;


        if(large!=0)
        {
        	P1->L.dz=(P2->L.z - P1->L.z)/large;
        	P1->L.du=(P2->L.u - P1->L.u)/large;
        	P1->L.dv=(P2->L.v - P1->L.v)/large;
        	P1->L.ddu=0;
        	P1->L.ddv=0;
        }
        P1++;P2++;
    }

    SC->Pix=SC->P1;
    SelectMipMap(SC);
    SC->FunctionFill(SC);
}
/*=============================================================*/
void Poly_Persp1_Gouraud(struct SOFT3D_context *SC)
{
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register LONG high=SC->PolyHigh;
register LONG n;
register WORD large;

SFUNCTION(Poly_Persp1_Gouraud)
/* compute a delta per segment */
    NLOOP(high)
    {
		large=P2->W.x - P1->W.x; P1->W.large=large;

        if(large!=0)
        {
        	P1->L.dz=(P2->L.z - P1->L.z)/large;
        	P1->L.dR=(P2->L.R - P1->L.R)/large;
        	P1->L.dG=(P2->L.G - P1->L.G)/large;
        	P1->L.dB=(P2->L.B - P1->L.B)/large;
        	P1->L.dA=(P2->L.A - P1->L.A)/large;
        }
        P1++;P2++;
    }

    SC->Pix=SC->P1;
    SC->FunctionFill(SC);
}
/*=============================================================*/
void Poly_Persp1_Flat(struct SOFT3D_context *SC)
{
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register LONG high=SC->PolyHigh;
register LONG n;
register WORD large;

SFUNCTION(Poly_Persp1_Flat)
/* compute a delta per segment */
    NLOOP(high)
    {
		large=P2->W.x - P1->W.x; P1->W.large=large;

        if(large!=0)
        {
        	P1->L.dz=(P2->L.z - P1->L.z)/large;
        }
        P1++;P2++;
    }

    SC->Pix=SC->P1;
    SC->FunctionFill(SC);
}
/*=============================================================*/
void Poly_Persp1_Tex_Gouraud_Fog(struct SOFT3D_context *SC)
{
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register LONG high=SC->PolyHigh;
register LONG n;
register WORD large;

SFUNCTION(Poly_Persp1_Tex_Gouraud_Fog)
/* compute a delta per segment */
    NLOOP(high)
    {
		large=P2->W.x - P1->W.x; P1->W.large=large;

        if(large!=0)
        {
        	P1->L.dz=(P2->L.z - P1->L.z)/large;
        	P1->L.du=(P2->L.u - P1->L.u)/large;
        	P1->L.dv=(P2->L.v - P1->L.v)/large;
        	P1->L.dR=(P2->L.R - P1->L.R)/large;
        	P1->L.dG=(P2->L.G - P1->L.G)/large;
        	P1->L.dB=(P2->L.B - P1->L.B)/large;
        	P1->L.dA=(P2->L.A - P1->L.A)/large;
        	P1->L.dF=(P2->L.F - P1->L.F)/large;
        	P1->L.ddu=0;
        	P1->L.ddv=0;
        }
        P1++;P2++;
    }

    SC->Pix=SC->P1;
    SelectMipMap(SC);
    SC->FunctionFill(SC);
}
/*=============================================================*/
void Poly_Persp2_Tex_Gouraud_Fog(struct SOFT3D_context *SC,union pixel3D *P1,union pixel3D *P2)
{
register LONG high=SC->PolyHigh;
register LONG n;
register WORD large;

SFUNCTION(Poly_Persp2_Tex_Gouraud_Fog)
    SC->Pix=P1;
/* compute a delta per segment */
    NLOOP(high)
    {
    	large=P2->W.x - P1->W.x; P1->W.large=large;

        if(large!=0)
        {
        	P1->L.dz=(P2->L.z - P1->L.z)/large;
        	P1->L.dR=(P2->L.R - P1->L.R)/large;
        	P1->L.dG=(P2->L.G - P1->L.G)/large;
        	P1->L.dB=(P2->L.B - P1->L.B)/large;
        	P1->L.dA=(P2->L.A - P1->L.A)/large;
        	P1->L.dF=(P2->L.F - P1->L.F)/large;
        	DoDeltasUV(P1,P2);
        }
        P1++;P2++;
    }
    SelectMipMap(SC);
    SC->FunctionFill(SC);
}
/*=============================================================*/
void Poly_Persp2x2_Tex_Gouraud_Fog(struct SOFT3D_context *SC)
{
/* V44: convert polygon to 2 polygons for better perspective emulation */
register union pixel3D *P1=SC->P1;
register union pixel3D *P2=SC->P2;
register union pixel3D *PM=&SC->edgeM[P1->W.y];
register LONG high=SC->PolyHigh;
register LONG n;
float u,v;

    SFUNCTION(Poly_Persp2x2_Tex_Gouraud_Fog)
    if(SC->PolyLarge<10)
        {Poly_Persp1_Tex_Gouraud_Fog(SC); return;}
    if(SC->PolyLarge<40)
        {Poly_Persp2_Tex_Gouraud_Fog(SC,SC->P1,SC->P2);return;}

/* compute center point PM */
    NLOOP(high)
    {
        PM->L.x=(P1->L.x + P2->L.x)/2;
        PM->L.z=(P1->L.z + P2->L.z)/2.0;
        PM->L.w=(P1->L.w + P2->L.w)/2.0;
        PM->L.R=(P1->L.R + P2->L.R)/2;
        PM->L.G=(P1->L.G + P2->L.G)/2;
        PM->L.B=(P1->L.B + P2->L.B)/2;
        PM->L.A=(P1->L.A + P2->L.A)/2;
        PM->L.F=(P1->L.F + P2->L.F)/2;

        if(PM->L.w==0.0) PM->L.w=0.00001;
        u= ( (P1->L.u*P1->L.w) + (P2->L.u*P2->L.w) ) /2.0 ;
        PM->L.u=(u/ PM->L.w);

        v= ( (P1->L.v*P1->L.w) + (P2->L.v*P2->L.w) ) /2.0 ;
        PM->L.v=(v/ PM->L.w);

        PM->W.large=P1->W.large/2;
        P1->W.large=P1->W.large - PM->W.large;

        P1++;P2++;PM++;
    }

    P1=SC->P1;
    P2=SC->P2;
    PM=&SC->edgeM[P1->W.y];
    Poly_Persp2_Tex_Gouraud_Fog(SC,P1,PM);
    Poly_Persp2_Tex_Gouraud_Fog(SC,PM,P2);
}
/*=============================================================*/
void DrawSegmentPix(struct SOFT3D_context *SC,union pixel3D *P1,union pixel3D *P2)
{
/* draw a simple horizontal line */
register union pixel3D *temp;

    SFUNCTION(DrawSegmentPIX)
/* Find point P1 and point P2 of the two points of this line */
    if (P1->W.x > P2->W.x)
        SWAP(P1,P2);

    SC->P1=&(SC->edge1[P1->W.y]);
    COPYPIX(SC->P1,P1);
    SC->P2=&(SC->edge2[P2->W.y]);
    COPYPIX(SC->P2,P2);

    SC->PolyHigh=1;
    SC->FunctionPoly(SC);
}
/*=============================================================*/
void DrawLinePix(struct SOFT3D_context *SC)
{
    union pixel3D *P1=&SC->PolyPix[0];
    union pixel3D *P2=&SC->PolyPix[1];
    union pixel3D *temp;
    ULONG n,ordered;
    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

    SFUNCTION(DrawLinePIX)
/* Find min point and max point of the two points P1&P2 of this line */
    if (P1->W.y > P2->W.y)
    {
        P2=&SC->PolyPix[0];
        P1=&SC->PolyPix[1];
    }

    ordered=P1->W.x < P2->W.x;
    if(ordered)
        n= P2->W.x - P1->W.x;    /* normal case : fill from left to right */
    else
        n= P1->W.x - P2->W.x;    /* other case ... */
    SC->PolyHigh = P2->W.y - P1->W.y;
    n = n + SC->PolyHigh;

    if(n==0)
    {
        DrawSimplePix(SC,P1);
        return;
    }

    if(SC->PolyHigh==0)
    {
        DrawSegmentPix(SC,P1,P2);
        return;
    }

/* Store the 2 points in an edge (edge1) */
    SC->P1=&SC->edge1[P1->W.y];        /* draw edge from top(P1) to bottom (P2)*/
    SC->P2=&SC->edge1[P2->W.y];
    COPYPIX(SC->P1,P1);
    COPYPIX(SC->P2,P2);
    SC->FunctionEdge(SC);

    P1= SC->P1 ;
    P2= SC->P2 =&SC->edge2[P1->W.y];   /* get the two edges (P1 & P2) both from top*/

    if(ordered)
    {
        NLOOP(SC->PolyHigh)
        {
            COPYPIX(P2,&P1[1]);
            P2->W.x++;            /* so avoid to have a segment with large=0 */
            P1++;
            P2++;
        }
        COPYPIX(P2,P1);            /* copy last point of line identically */
    }
    else         /* not ordered */
    {
        NLOOP(SC->PolyHigh)
        {
            COPYPIX(P2,&P1[1]);
            P1->W.x++;             /* so avoid to have a segment with large=0 */
            P1++;
            P2++;
        }
        COPYPIX(P2,P1);            /* copy last point of line identically */
        SWAP(SC->P1,SC->P2);       /* swap P1 & P2 */
    }

    SC->PolyHigh++;
    SC->dmin=1024<<16;
    SC->FunctionPoly(SC);
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*=============================================================*/
void DrawPolyPix(struct SOFT3D_context *SC)
{
    register union pixel3D *P1=NULL;
    register union pixel3D *P2=NULL;
    register WORD Pnb=SC->PolyPnb;
    register WORD n;
    union pixel3D *Pxmin;
    union pixel3D *Pxmax;
    union pixel3D *Pymin;
    union pixel3D *Pymax;
    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

    SFUNCTION(DrawPolyPix)
    SVAR(SC->PolyPnb)
    Pymin=Pymax=Pxmin=Pxmax=SC->PolyPix;
    SC->dmin=1024<<16;

    /* Polygon loop : draw all edges */
    NLOOP(Pnb)
    {
        PrintPix(&SC->PolyPix[n]);
        P1 =&(SC->PolyPix[n]);
        P2 =&(SC->PolyPix[n+1]);
        if (n+1==Pnb)
            P2 =&(SC->PolyPix[0]);

/* polygon size */
        if (P1->W.x    < Pxmin->W.x)    Pxmin=P1;
        if (P2->W.x    < Pxmin->W.x)    Pxmin=P2;
        if (Pxmax->W.x < P1->W.x   )    Pxmax=P1;
        if (Pxmax->W.x < P2->W.x   )    Pxmax=P2;

/* do edge */
        if(P1->W.y != P2->W.y  )    /* skip horizontal edges */
        {
/* if P1 up and P2 down */
            if (P1->W.y < P2->W.y)
            {
                if (P1->W.y     < Pymin->W.y)    Pymin=P1;
                if (Pymax->W.y  < P2->W.y   )    Pymax=P2;
                SC->P1=&(SC->edge1[P1->W.y]);
                SC->P2=&(SC->edge1[P2->W.y]);
                COPYPIX(SC->P1,P1);         /* store the two points in this edge */
                COPYPIX(SC->P2,P2);
            }
            else
            {
/* if P2 up and P1 down */
                if (P2->W.y     < Pymin->W.y )    Pymin=P2;
                if (Pymax->W.y  < P1->W.y    )    Pymax=P1;
                SC->P1=&(SC->edge2[P2->W.y]);
                SC->P2=&(SC->edge2[P1->W.y]);
                COPYPIX(SC->P1,P2);           /* store the two points in the other edge */
                COPYPIX(SC->P2,P1);
            }
/* draw this edge */
            SC->FunctionEdge(SC);
        }
    }
/* all edges drawn */

/* compute Poly sizes */
    SC->PolyLarge= Pxmax->W.x - Pxmin->W.x ;
    SC->PolyHigh = Pymax->W.y - Pymin->W.y ;

/* if poly is only an horizontal line */
    if(SC->PolyHigh==0)
    {
        DrawSegmentPix(SC,Pxmin,Pxmax);
        DEBUG_SOFT3D("%s end\n",__FUNCTION__);
        return;
    }

/* eliminate poly if PolyLarge = 0 */
    if(SC->PolyLarge==0)
    {
        DEBUG_SOFT3D("%s end PolyLarge=0\n",__FUNCTION__);
    	return;
    }

/* patch: for TheVague if last line of screen then dont crop it */
    if(Pymax->W.y==(SC->high-1))
        SC->PolyHigh++;

/* Sort edges */
    n=(Pymin->W.y + Pymax->W.y)/2;    /* central line */

    if (SC->edge1[n].L.x < SC->edge2[n].L.x)
    {
        SC->P1=&(SC->edge1[Pymin->W.y]);
        SC->P2=&(SC->edge2[Pymin->W.y]);
    }
    else
    {
        SC->P1=&(SC->edge2[Pymin->W.y]);
        SC->P2=&(SC->edge1[Pymin->W.y]);
    }

    SC->FunctionPoly(SC);

    SFUNCTION(-----------)
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*==================================================================================*/
void ChangeSoftFog(APTR sc,UBYTE FogMode,float FogZmin,float FogZmax,float FogDensity,APTR fogrgba)
{
struct SOFT3D_context *SC=sc;
register float d,z,f;
register UWORD n;
UBYTE A;
UBYTE *FogRGBA=fogrgba;
UBYTE *RGBA;

#ifdef SLOWCPU
register UBYTE *Mul8=SC->Mul8;
register union oper3D Mul;
    Mul.L.Index=0;
#endif

#define CLAMP(X,MIN,MAX) ((X)<(MIN)?(MIN):((X)>(MAX)?(MAX):(X)))

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(ChangeSoftFog)
    if(Wazp3D->DebugSOFT3D.ON)
    {
    SVAR(FogMode)
    SVARF(FogZmin)
    SVARF(FogZmax)
    SVARF(FogDensity)
    }

    if(3<FogMode)
        return;

    if(FogZmin     <  MINZ   )    FogZmin =MINZ;
    if(MAXZ     <= FogZmax)    FogZmax =MAXZ;

    SC->state.FogMode        =FogMode;
    SC->state.FogZmin        =FogZmin;
    SC->state.FogZmax        =FogZmax;
    SC->state.FogDensity    =FogDensity;
    COPYRGBA(SC->state.FogRGBA.L,(ULONG*)FogRGBA);

    if(FogMode!=0)
    NLOOP(FOGSIZE)
    {
    z=(((float)n)/((float)FOGSIZE));
    switch (FogMode)
    {
    case 1:
        d = 1.0F / (FogZmax - FogZmin);
        f= (FogZmax - z) * d;
        break;
    case 2:
        d = -FogDensity;
        f= FEXP( d * z);
        break;

    case 3:
        d = -(FogDensity*FogDensity);
        f= FEXP( d * z*z );
        break;
    default:
        f=0.0;
        break;
    }

    f=CLAMP(f,0.0F,1.0F);
    f=1.0-f;                /* alpha for fog */
    A=(255.0*f);
    RGBA=(UBYTE *) &SC->FogRGBAs[n];
    MUL8(A,FogRGBA[0],RGBA[0])
    MUL8(A,FogRGBA[1],RGBA[1])
    MUL8(A,FogRGBA[2],RGBA[2])
    RGBA[3]=ONE-A;            /* alpha for background */

    if(Wazp3D->DebugSOFT3D.ON)    /* dont display all the fog values */
    if(n % 100 == 0)
    {
    SVARF(f*100.0)
    SVAR(A)
    }
    }



}
/*================================================================*/
void DrawPolyP(struct SOFT3D_context *SC)
{
    union  pixel3D *Pix=SC->PolyPix;
    struct point3D *P=SC->PolyP;
    struct point3D PolyMin;
    struct point3D PolyMax;
    WORD Pnb,n;
    BOOL FaceCCW;
    UBYTE ColorChange,ColorTransp,ColorWhite,FlatChange;

    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugSOFT3D.ON) Libprintf("DrawPolyP Pnb %ld  \n",(ULONG)SC->PolyPnb);
#endif
//    printf("PIX 0x%08lx\r\n",(uint32_t)Pix);
//    printf("P 0x%08lx\r\n",(uint32_t)P);


    if(SC->PolyPnb > MAXPOLY )
        {SREM(Poly has too much Points!);
        DEBUG_SOFT3D("%s end too much points\n",__FUNCTION__);
        return;
    }

    P=SC->PolyP;
    Pnb=SC->PolyPnb;

    PolyMin.x=PolyMax.x=P->x;
    PolyMin.y=PolyMax.y=P->y;
    PolyMin.z=PolyMax.z=P->z;
    PolyMin.u=PolyMax.u=P->u;
    PolyMin.v=PolyMax.v=P->v;

    NLOOP(Pnb)
    {
        P->y.f=P->y.f+0.49;  /* round to nearer pixel */
        P->x.f=P->x.f+0.49;

        if (P->x.f < PolyMin.x.f)    PolyMin.x.f=P->x.f;
        if (PolyMax.x.f < P->x.f)    PolyMax.x.f=P->x.f;
        if (P->y.f < PolyMin.y.f)    PolyMin.y.f=P->y.f;
        if (PolyMax.y.f < P->y.f)    PolyMax.y.f=P->y.f;
        if (P->z.f < PolyMin.z.f)    PolyMin.z.f=P->z.f;
        if (PolyMax.z.f < P->z.f)    PolyMax.z.f=P->z.f;

        if (P->u.f < PolyMin.u.f)    PolyMin.u.f=P->u.f;
        if (PolyMax.u.f < P->u.f)    PolyMax.u.f=P->u.f;
        if (P->v.f < PolyMin.v.f)    PolyMin.v.f=P->v.f;
        if (PolyMax.v.f < P->v.f)    PolyMax.v.f=P->v.f;

        P++;
    }

    if(Wazp3D->QuakeMatrixPatch.ON)
    {
        SREM(adjusting u v )
        if(Wazp3D->DebugSOFT3D.ON) { SVARF(PolyMin.u) SVARF(PolyMin.v) }
        PolyMin.u.f=FFLOOR(PolyMin.u.f);
        PolyMin.v.f=FFLOOR(PolyMin.v.f);
        if(Wazp3D->DebugSOFT3D.ON) { SVARF(PolyMin.u) SVARF(PolyMin.v) }

        P=SC->PolyP;
        Pnb=SC->PolyPnb;

        NLOOP(Pnb)
        {
            P->u.f=P->u.f - PolyMin.u.f;
            P->v.f=P->v.f - PolyMin.v.f;
            PrintP(P);
            P++;
        }
    }

    if(SC->PolyPnb == 2)
        ClipLine(SC);

    if(SC->PolyPnb >  2)
        ClipPoly(SC);

    if(SC->PolyPnb==0)
    {
    	SREM(  clipped away);
        DEBUG_SOFT3D("%s end clipped away\n",__FUNCTION__);
        return;
    }

    Pnb=SC->PolyPnb;
    P=SC->PolyP;

    if(Pnb>=3)
    {
        FaceCCW= ((P[1].x.f - P[0].x.f)*(P[2].y.f - P[0].y.f)-(P[2].x.f - P[0].x.f)*(P[1].y.f - P[0].y.f) <  0.0); /* formula from TinyGL */

        if(SC->state.CullingMode==W3D_CCW)
            if(!FaceCCW)
            {
            	SREM(  hided face);
                DEBUG_SOFT3D("%s end hided face\n",__FUNCTION__);
            	return;
            }

        if(SC->state.CullingMode==W3D_CW)
            if(FaceCCW)
            {
            	SREM(  hided face);
                DEBUG_SOFT3D("%s end hided face\n",__FUNCTION__);
            	return;
            }
    }

    FlatChange=FALSE;
    if(NOTSAMERGBA(SC->FlatRGBA.L,SC->PolyP->RGBA.L))
        {SREM(FlatRGBA changed); FlatChange=TRUE;}

    COPYRGBA(SC->FlatRGBA.L,SC->PolyP->RGBA.L);        /* default: flat-color come from current points */
    if(Wazp3D->TexMode.ON==3)                /* in this case use face center as flat color */
    if(Pnb>=3)
        UVtoRGBA((struct SOFT3D_texture *)swap32((uint32_t)SC->state.ST),(P[0].u.f+P[1].u.f+P[2].u.f)/3.0,(P[0].v.f+P[1].v.f+P[2].v.f)/3.0,SC->FlatRGBA.b);

    ColorChange=FALSE;
    ColorTransp=FALSE;
    ColorWhite=FALSE;

    NLOOP(Pnb)
    {


        if(Wazp3D->TexMode.ON==2)
            UVtoRGBA((struct SOFT3D_texture *)swap32((uint32_t)SC->state.ST),P->u.f,P->v.f,P->RGBA.b);

        Pix->L.x    = 0;            /* erase low parts */
        Pix->L.y    = 0;
        Pix->W.x    = P->x.f;
        Pix->W.y    = P->y.f;

        ZCONVERSION
        Pix->L.w    = P->w.f;

        Pix->L.F    = 0;
        Pix->W.F    = P->z.f * (float)FOGSIZE ;

        if(P->z.f<MINZ) Pix->W.F=0;
        if(MAXZ<P->z.f) Pix->W.F=FOGSIZE-1;


        Pix->L.u    = P->u.f * (256.0*65536.0);    /* range UV to [0..256] + shift per 16 */
        Pix->L.v    = P->v.f * (256.0*65536.0);

        Pix->W.R    = P->RGBA.b[0];
        Pix->W.G    = P->RGBA.b[1];
        Pix->W.B    = P->RGBA.b[2];
        Pix->W.A    = P->RGBA.b[3];

        if(NOTSAMERGBA(SC->PolyP->RGBA.L,P->RGBA.L))    /* if color truly change ==> do gouraud */
            {SREM(ColorChange);ColorChange=TRUE;}

        if(SC->PolyP->RGBA.b[3]!=P->RGBA.b[3])        /* if alpha change ==> do alpha */
            {SREM(ColorTransp:A changed);ColorTransp=TRUE;}

        if(Pix->W.x < SC->Pxmin)    SC->Pxmin=Pix->W.x;
        if(SC->Pxmax < Pix->W.x)    SC->Pxmax=Pix->W.x;
        if(Pix->W.y < SC->Pymin)    SC->Pymin=Pix->W.y;
        if(SC->Pymax < Pix->W.y)    SC->Pymax=Pix->W.y;
        if(ZVALUE < SC->Pzmin)      SC->Pzmin=ZVALUE;
        if(SC->Pzmax < ZVALUE)      SC->Pzmax=ZVALUE;

        Pix++;
        P++;
    }

    if(SC->PolyP->RGBA.b[3]!=255)        /* if not solid ==> do alpha */
        {SREM(ColorTransp:A not solid);ColorTransp=TRUE;}

    if(SC->FlatRGBA.b[0]==255)
    if(SC->FlatRGBA.b[1]==255)
    if(SC->FlatRGBA.b[2]==255)
    if(SC->FlatRGBA.b[3]==255)
        {SREM(ColorWhite);ColorWhite=TRUE;}

/* This face is inside a linear Fog ? */
    if(SC->state.UseFog)
    if(SC->state.FogMode==1)
    {
        if(PolyMax.z.f    < SC->state.FogZmin)     /* after start of fogging area ? */
        {
            SREM(states changed:Fog)
            SC->state.UseFog        =FALSE;
            SC->state.Changed    =TRUE;
        }
#ifdef WAZP3DDEBUG
        if(Wazp3D->DebugSOFT3D.ON)
        {
            Libprintf("LinearFog:UseFog%ld. FogZmin FogZmax PolyZmin PolyZmax:",(ULONG)SC->state.UseFog);
            pf(SC->state.FogZmin);pf(SC->state.FogZmax);pf(PolyMin.z);pf(PolyMax.z);
            Libprintf("\n");
        }
#endif
    }

    if(SC->state.TexEnvMode!=W3D_REPLACE)
        if( (FlatChange) ou (SC->ColorChange!=ColorChange) ou (SC->ColorTransp!=ColorTransp) ou (SC->ColorWhite!=ColorWhite) )
        {
            SREM(states changed:Color)
            SC->ColorChange    =ColorChange;
            SC->ColorTransp    =ColorTransp;
            SC->ColorWhite     =ColorWhite;
            SC->state.Changed  =TRUE;
        }

    if(SC->state.Changed)
    {
        SOFT3D_SetDrawFunctions(SC);        /* draw functions still can change now if ColorChange ColorTransp UseFog just changed */
        SC->state.Changed=FALSE;
    }

    switch(Pnb)
    {
        case 1:  DrawPointPix(SC); break;
        case 2:  DrawLinePix(SC);  break;
        default: DrawPolyPix(SC);  break;
    }
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);

}
/*=============================================================*/
void DrawTriP(struct SOFT3D_context *SC,register struct point3D *A,register struct point3D *B,register struct point3D *C)
{
    COPYP(&(SC->PolyP[0]),A);
    COPYP(&(SC->PolyP[1]),B);
    COPYP(&(SC->PolyP[2]),C);
    SC->PolyPnb=3;

    SWAP32_D(SC->PolyP[0]);
    SWAP32_D(SC->PolyP[1]);
    SWAP32_D(SC->PolyP[2]);
/*
    PrintP(&SC->PolyP[0]);
    PrintP(&SC->PolyP[1]);
    PrintP(&SC->PolyP[2]);
*/
    DrawPolyP(SC);
}
/*=============================================================*/
void UVtoRGBA(struct SOFT3D_texture *ST,float u,float v,UBYTE *RGBA)
{
register UBYTE *Tex8;
UBYTE u8,v8;

    SREM(UVtoRGBA)
    if(ST==NULL) return;
    PrintST(ST);

    u8=u;
    v8=v;
    Tex8=ST->MMs[0].Tex8U[u8]+ST->MMs[0].Tex8V[v8];

    RGBA[0]=Tex8[0];
    RGBA[1]=Tex8[1];
    RGBA[2]=Tex8[2];
    RGBA[3]=255;
    if (ST->bits==32)
        RGBA[3]=Tex8[3];
    if(Wazp3D->DebugSOFT3D.ON) PrintRGBA((UBYTE *)RGBA);
}
/*=============================================================*/
#ifdef WAZP3DDEBUG
/*=============================================================*/
void TexturePlot(struct SOFT3D_texture *ST,UWORD x,UWORD y,UBYTE *ColorRGBA)
{
UBYTE *RGB;
ULONG offset;

    if(x<ST->large)
    if(y<ST->high )
    {
    offset=(ST->large*y + x)  * ST->bits / 8;
    RGB=&(ST->pt[offset]);
    RGB[0]=ColorRGBA[0];
    RGB[1]=ColorRGBA[1];
    RGB[2]=ColorRGBA[2];
    if(ST->bits==32)
        RGB[3]=ColorRGBA[3];
    }
}
/*=============================================================*/
void TextureBorder(struct SOFT3D_texture *ST)
{
UBYTE GreenRGBA[4]={0,255,0,255};
UBYTE   RedRGBA[4]={255,0,0,255};
UWORD x,y;

    XLOOP(ST->large)
        {
        TexturePlot(ST,x,0,RedRGBA);
        TexturePlot(ST,x,ST->high-1,RedRGBA);
        }
    YLOOP(ST->high )
        {
        TexturePlot(ST,0,y,GreenRGBA);
        TexturePlot(ST,ST->large-1,y,GreenRGBA);
        }

}
/*=============================================================*/
void TexturePrint(struct SOFT3D_texture *ST,WORD x,WORD y,UBYTE *texte)
{
UBYTE *F;
UBYTE RGBA[4];
UWORD m,n,c;
UBYTE Bit[] = {128,64,32,16,8,4,2,1};
#define FONTSIZE 8
#define FONTLARGE (128/8)

    RGBA[3]=255;
    while(*texte!=0)
    {
    c=*texte++;
    if(32<c) c=c-32; else c=0;
    F=&(font8x8[(c AND 15) + (c/16)*FONTLARGE*FONTSIZE]);
        MLOOP(FONTSIZE)
        {
        NLOOP(FONTSIZE)
            {
            RGBA[0]=RGBA[1]=RGBA[2]=0;
            if (F[0] AND Bit[n])
                RGBA[0]=RGBA[1]=RGBA[2]=255;
            TexturePlot(ST,x+n,y+m,RGBA);
            }
        F+=FONTLARGE;
        }
    x+=FONTSIZE;
    }

}
#endif
/*================================================================*/
void SOFT3D_DrawPrimitive(APTR sc,APTR p,ULONG Pnb,ULONG primitive)
{
    struct SOFT3D_context *SC=sc;
    struct point3D *P=p;
    UWORD m,n,nb;
    BOOL DebugState;
    struct SOFT3D_texture *ST=(struct SOFT3D_texture *)swap32((uint32_t)SC->state.ST);
    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

    if(Wazp3D->Renderer.ON==0)        /* use Soft to Image */
    if(SC->ImageBuffer32==NULL)        /* but no memory for Image */
        return;                /* then panic */

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
/* If it's debug-texture then activate debugger to debug the full polygon */
    DebugState=LibDebug;
    if(ST!=NULL)
    if(Wazp3D->DebugTexNumber.ON)
    if(ST->Tnum==DEBUGTNUM)
    {
        LibDebug=TRUE;
        Libprintf("DrawPrimitive using DebugTexture %s===============\n",ST->name);
        NLOOP(Pnb)
            PrintP(&P[n]);
    }

#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugSOFT3D.ON)
    {
        Libprintf("SOFT3D_DrawPrimitive Pnb %ld (%ld)\n",(ULONG)Pnb,(ULONG)primitive);
        SVAR(ST)
        if(ST!=NULL) Libprintf(" %s\n",ST->name);
    }
#endif


    if(Wazp3D->DebugContext.ON)            /* backdoor: for debugging =  display all the scene as lines */
        primitive=W3D_PRIMITIVE_LINELOOP;

    if(!SC->UseHard)
    if(SC->Image8==NULL)
        { SREM(Image is lost or freed: cant draw !!); return;}        /* happen if SOFT3D_AllocImageBuffer() fail */

    SINFO(primitive,W3D_PRIMITIVE_TRIANGLES)
    SINFO(primitive,W3D_PRIMITIVE_TRIFAN)
    SINFO(primitive,W3D_PRIMITIVE_TRISTRIP)
    SINFO(primitive,W3D_PRIMITIVE_POINTS)
    SINFO(primitive,W3D_PRIMITIVE_LINES)
    SINFO(primitive,W3D_PRIMITIVE_LINELOOP)
    SINFO(primitive,W3D_PRIMITIVE_LINESTRIP)
    SINFO(primitive,W3D_PRIMITIVE_POLYGON)

#ifdef USEOPENGL
    if(SC->UseHard)              /* so dont use "soft" for drawing the primitive*/
    {
        HARD3D_DrawPrimitive(&SC->HC,p,Pnb,primitive);
        LibDebug    =DebugState;
        return;
    }
#endif
#if defined(OS4COMPOSITING)
    if(SC->UseHard)
    {
        COMP3D_DrawPrimitive(SC,p,Pnb,primitive);
        LibDebug    =DebugState;
        return;
    }
#endif



/* for software convert 4-5-6-7 trifans to polygons = faster */

    if(Pnb <= Wazp3D->MaxPolyHack)    /* a simple quad after a clipping in x y z can have now more than 4 points */
    {

        if(primitive==W3D_PRIMITIVE_TRIFAN)
        {
            SREM(TRIFAN to POLYGON)
            primitive=W3D_PRIMITIVE_POLYGON;
        }

        if(primitive==W3D_PRIMITIVE_TRISTRIP)
        {
            SREM(TRISTRIP to POLYGON======)
            m=n=0;
            NLOOP(Pnb)
                if(!(n AND 1))
                {
                    COPYP(&(SC->PolyP[m]),&P[n]);
                    SWAP32_D(SC->PolyP[m]);
                    m++;
                    SVAR(n)
                }
            n--;
            while(0<n)
            {
                if(n AND 1)
                {
                    COPYP(&(SC->PolyP[m]),&P[n]);
                    SWAP32_D(SC->PolyP[m]);
                    m++;
                    SVAR(n)
                }
                n--;
            }
            SC->PolyPnb=Pnb;
            DrawPolyP(SC);
            goto DrawDone;
   	    }
    }

    if(primitive==W3D_PRIMITIVE_TRIANGLES)
    {
        nb=Pnb/3;
        NLOOP(nb)
            DrawTriP(SC,&P[3*n+0],&P[3*n+1],&P[3*n+2]);
    }

    if(primitive==W3D_PRIMITIVE_POINTS)
    {
        NLOOP(Pnb)
        {
            COPYP(&(SC->PolyP[0]),&P[n]);
            SWAP32_D(SC->PolyP[0]);
            SC->PolyPnb=1;
            DrawPolyP(SC);
        }
        goto DrawDone;
    }

    if(primitive==W3D_PRIMITIVE_LINES)
    {
        nb=Pnb/2;
        NLOOP(nb)
        {
            COPYP(&(SC->PolyP[0]),&P[2*n]);
            COPYP(&(SC->PolyP[1]),&P[2*n+1]);
            SWAP32_D(SC->PolyP[0]);
            SWAP32_D(SC->PolyP[1]);
            SC->PolyPnb=2;
            DrawPolyP(SC);
        }
        goto DrawDone;
    }

    if(primitive==W3D_PRIMITIVE_POLYGON    )
    {
        NLOOP(Pnb)
        {
            COPYP(&(SC->PolyP[n]),&P[n]);
            SWAP32_D(SC->PolyP[n]);
        }
        SC->PolyPnb=Pnb;

        DrawPolyP(SC);
    }

    if(primitive==W3D_PRIMITIVE_TRIFAN)
    {
        for (n=2;n<Pnb;n++)
            DrawTriP(SC,&P[0],&P[n-1],&P[n]);
        goto DrawDone;
    }

    if(primitive==W3D_PRIMITIVE_TRISTRIP)
    {
        for (n=2;n<Pnb;n++)
//            if (n&1)      /* reverse vertex order */
                DrawTriP(SC,&P[n-1],&P[n-2],&P[n-0]);
//            else
//                DrawTriP(SC,&P[n-2],&P[n-1],&P[n-0]);
        goto DrawDone;
    }

    if(primitive==W3D_PRIMITIVE_LINELOOP)
    {
        nb=Pnb-1;
        NLOOP(nb)
        {
//        	printf("Iteracion n=%d\n",n);
//        	printf("PP0 0x%08lx P0 0x%08lx\n",(uint32_t)&(SC->PolyP[0]),(uint32_t)&P[n]);
//         	printf("PP1 0x%08lx P1 0x%08lx\n",(uint32_t)&(SC->PolyP[1]),(uint32_t)&P[n+1]);
            COPYP(&(SC->PolyP[0]),&P[n]);
            COPYP(&(SC->PolyP[1]),&P[n+1]);
            SWAP32_D(SC->PolyP[0]);
            SWAP32_D(SC->PolyP[1]);
            SC->PolyPnb=2;
            DrawPolyP(SC);
        }

        COPYP(&(SC->PolyP[0]),&P[nb]);
        COPYP(&(SC->PolyP[1]),&P[0 ]);
        SWAP32_D(SC->PolyP[0]);
        SWAP32_D(SC->PolyP[1]);
        SC->PolyPnb=2;
        DrawPolyP(SC);
        goto DrawDone;
    }

    if(primitive==W3D_PRIMITIVE_LINESTRIP)
    {
        nb=Pnb-1;
        NLOOP(nb)
        {
            COPYP(&(SC->PolyP[0]),&P[n]);
            COPYP(&(SC->PolyP[1]),&P[n+1]);
            SWAP32_D(SC->PolyP[0]);
            SWAP32_D(SC->PolyP[1]);
            SC->PolyPnb=2;
            DrawPolyP(SC);
        }
        goto DrawDone;
    }

DrawDone:
    LibDebug    =DebugState;
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*=============================================================*/
void ChangeSoftPoint(APTR sc)
{
struct SOFT3D_context *SC=sc;
register WORD *PointLarge;
register WORD x,y,D;

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */

SFUNCTION(ChangeSoftPoint)
    PointLarge=SC->PointLarges;
    D=swap32(SC->state.PointSize);
SVAR(SC->state.PointSize)
    YLOOP(D)
    XLOOP(D)
        if( ( (2*y-D)*(2*y-D) + 2*2*x*x) < D*D ) PointLarge[y]=x*2;
}
/*==================================================================*/
void *SOFT3D_CreateTexture(APTR sc,APTR pt,UWORD large,UWORD high,UWORD format,UBYTE TexFlags)
{
    struct SOFT3D_context *SC=sc;
    struct SOFT3D_texture *ST;
    UBYTE *Tex8V;
    UBYTE *ptmm;
    float TexelsPerU,TexelsPerV,nf;
    UWORD BytesPerTexel,BytesPerLine,n,m,bits;
    UBYTE BitmapName[40];
    ULONG Tsize,MMsize;
    UBYTE UseMip=TexFlags AND 1;

#ifndef W3D_R8G8B8
#define W3D_R8G8B8              4
#define W3D_R8G8B8A8           11
#endif

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(SOFT3D_CreateTexture)
    ST=MMmalloc(sizeof(struct SOFT3D_texture),"SOFT3D_texture");
    if(ST==NULL) return(NULL);
//    chunk_list_dump(&alloced_chunks, "Alloced");
//    chunk_list_dump(&freed_chunks, "Freed");

    if(large!=high)
        UseMip=FALSE;

    if(format==W3D_R8G8B8)   bits=24;
    if(format==W3D_R8G8B8A8) bits=32;

    ST->pt    =pt;
    ST->large    =large;
    ST->high    =high;
    ST->bits    =bits;
    ST->format    =format;
    ST->TexFlags=TexFlags;
/* add to linkage */
    ST->nextST =SC->firstST;
    SC->firstST=ST;
    ST->ptmm=NULL;

/* Create texture index */
    BytesPerTexel=bits/8;
    BytesPerLine =ST->large*BytesPerTexel;

    TexelsPerU=((float)ST->large)/256.0;
    TexelsPerV=((float)ST->high )/256.0;
    Tex8V=ST->pt;

    Tsize=ST->large*ST->high*ST->bits/8;

    if(UseMip)
    {
        CreateMipmaps(ST);
        if(ST->ptmm==NULL)
        {
            SOFT3D_FreeTexture(SC,ST);
            return(NULL);
        }
    }

    ptmm=ST->HT.ptmm=ST->ptmm;
    MMsize=MAXTEXTURE*MAXTEXTURE*ST->bits/8;
    MLOOP(NBMIPMAPS)
    {
        NLOOP(256)
        {
            nf=(float)n;
            ST->MMs[m].Tex8U[n]=       (ULONG)(    BytesPerTexel * (UWORD)(TexelsPerU*nf));
            ST->MMs[m].Tex8V[n]=Tex8V +(ULONG)( BytesPerLine  * (UWORD)(TexelsPerV*nf));
            ST->MMs[m].Tex8Ulow[n]=    (ULONG)(    BytesPerTexel * (UWORD)(TexelsPerU*nf/256.0));
            ST->MMs[m].Tex8Vlow[n]=    (ULONG)(    BytesPerLine  * (UWORD)(TexelsPerV*nf/256.0));
            if(TexelsPerU<=1.0)
                    ST->MMs[m].Tex8Ulow[n]=0;
            if(TexelsPerV<=1.0)
                    ST->MMs[m].Tex8Vlow[n]=0;
        }

        if(UseMip)
        {
            if(Tsize > MMsize)     /* for smaller texture-models use mipmaps */
            {
                Tex8V=ptmm;
                TexelsPerU=TexelsPerU/2.0;
                TexelsPerV=TexelsPerV/4.0;
                ptmm=ptmm+MMsize;
            }
            MMsize=MMsize/4;
        }
    }


    SC->Tnb++;    ST->Tnum=SC->Tnb;
    Libsprintf((char*)ST->name,(char*)"Texture%ld_%ldX%ldX%ld.RAW",(ULONG)ST->Tnum,(ULONG)ST->large,(ULONG)ST->high,(ULONG)ST->bits);
    Libsprintf((char*)BitmapName,(char*)"Tex%ld %ld",(ULONG)ST->Tnum,(ULONG)ST->bits);

#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugTexNumber.ON)
    {
        TextureBorder(ST);
        TexturePrint(ST,0,ST->high/2,BitmapName);
    }
#endif

#ifdef USEOPENGL
    if(ST!=NULL)
        if(SC->UseHard) HARD3D_CreateTexture(&SC->HC,&ST->HT,ST->pt,ST->large,ST->high,ST->format,ST->TexFlags);
#endif

#if defined(OS4COMPOSITING)
    if(SC->UseHard)  COMP3D_CreateTexture(SC,ST,ST->pt,ST->large,ST->high,ST->format,ST->TexFlags);
#endif

#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugSOFT3D.ON)
        Libprintf("Tex<%s> ST%ld HT%ld gltex%ld\n",ST->name,(ULONG)ST,(ULONG)&ST->HT,(ULONG)ST->HT.gltex);

    if(Wazp3D->DebugST.ON)
    if(Wazp3D->DebugSOFT3D.ON)
        LibAlert("Tex done...");
#endif

    SC->state.Changed=TRUE;        /* v54: OpenGL allways use a new texture so force to use the tex in SC->state */

    return( (void *) ST);
}
/*==================================================================*/
void SOFT3D_UpdateTexture(APTR sc,APTR st,APTR pt)
{
//struct SOFT3D_context *SC=sc;
struct SOFT3D_texture *ST=st;
UBYTE UseMip=ST->TexFlags AND 1;
LONG Tsize;

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(SOFT3D_UpdateTexture)
    if(st==NULL)
        {SREM(NULL ST!!); return;}

    if(ST->large!=ST->high)
        UseMip=FALSE;

    Tsize=ST->large*ST->high*ST->bits/8;
    Libmemcpy(ST->pt,pt,Tsize);
    if(UseMip)
        CreateMipmaps(ST);
    ST->HT.ptmm=ST->ptmm;

#ifdef USEOPENGL
    if(SC->UseHard) HARD3D_UpdateTexture(&SC->HC,&ST->HT,pt,ST->large,ST->high,ST->bits);
#endif
#if defined(OS4COMPOSITING)
    if(SC->UseHard) COMP3D_UpdateTexture(SC,ST,pt,ST->large,ST->high,ST->bits);
#endif
}
/*==================================================================*/
void SOFT3D_FreeTexture(APTR sc,APTR st)
{
struct SOFT3D_context *SC=sc;
struct SOFT3D_texture *ST=st;
struct SOFT3D_texture fakeST;
struct SOFT3D_texture *thisST=&fakeST;
WORD Ntexture=0;

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(SOFT3D_FreeTexture)
    if(st==NULL)
        {SREM(NULL ST!!); return;}

    PrintST(ST);
    thisST->nextST=SC->firstST;
    while(thisST!=NULL)
    {
    if(thisST->nextST==ST)
        {
        SREM(ST found)
        if(thisST->nextST==SC->firstST)
            SC->firstST=ST->nextST;
        else
            thisST->nextST=ST->nextST;
        FREEPTR(ST->ptmm);
        break;
        }
    thisST=thisST->nextST;
    Ntexture++;
    }

#ifdef USEOPENGL
    if(SC->UseHard) HARD3D_FreeTexture(&SC->HC,&ST->HT);
#endif
#if defined(OS4COMPOSITING)
    if(SC->UseHard) COMP3D_FreeTexture(SC,ST);
#endif
    FREEPTR(ST);
    if(SC->state.ST==ST)
        SC->FragBufferDone =SC->FragBuffer;        /* cant draw fragments from a deleted texture */
}
/*=================================================================*/
void ClipPoint(struct SOFT3D_context *SC,struct point3D *PN,struct point3D *P,float t)
{
register float c0,c1;
register float u0w,v0w,u1w,v1w;

    P->x.f= PN[0].x.f + t * (PN[1].x.f - PN[0].x.f) ;
    P->y.f= PN[0].y.f + t * (PN[1].y.f - PN[0].y.f) ;
    P->z.f= PN[0].z.f + t * (PN[1].z.f - PN[0].z.f) ;
    P->w.f= PN[0].w.f + t * (PN[1].w.f - PN[0].w.f) ;
    c0=PN[0].RGBA.b[0]; c1=PN[1].RGBA.b[0]; P->RGBA.b[0]=c0 + t * (c1 - c0);
    c0=PN[0].RGBA.b[1]; c1=PN[1].RGBA.b[1]; P->RGBA.b[1]=c0 + t * (c1 - c0);
    c0=PN[0].RGBA.b[2]; c1=PN[1].RGBA.b[2]; P->RGBA.b[2]=c0 + t * (c1 - c0);
    c0=PN[0].RGBA.b[3]; c1=PN[1].RGBA.b[3]; P->RGBA.b[3]=c0 + t * (c1 - c0);

/* if want perspective then compute the new U V with a true-perspective */
    if(SC->state.PerspMode!=0)
    if(P->w.f!=0.0)
    {
    u0w=PN[0].u.f*PN[0].w.f;
    v0w=PN[0].v.f*PN[0].w.f;
    u1w=PN[1].u.f*PN[1].w.f;
    v1w=PN[1].v.f*PN[1].w.f;
    P->u.f= (u0w + t * (u1w - u0w) )/P->w.f;
    P->v.f= (v0w + t * (v1w - v0w) )/P->w.f;
    return;
    }

/* else compute UV linearly as other coordinates */
    P->u.f= PN[0].u.f + t * (PN[1].u.f - PN[0].u.f) ;
    P->v.f= PN[0].v.f + t * (PN[1].v.f - PN[0].v.f) ;
}
/*=================================================================*/
#define CopyPoint(A)  {COPYP(P,A); P++; NewPnb++; }
#define SwapBuffers useT1=!useT1; if(useT1) {PN=T1; P=T2;} else    {PN=T2; P=T1;}
#define CLIPPOINT(val,limit)    {ClipPoint(SC,PN,P,(limit - PN[0].val) / (PN[1].val - PN[0].val)); P->val=limit; }
#define NEWCLIPPOINT(val,limit) {ClipPoint(SC,PN,P,(limit - PN[0].val) / (PN[1].val - PN[0].val)); P->val=limit; P++; NewPnb++; }
/*=================================================================*/
void ClipLine(struct SOFT3D_context *SC)
{
struct point3D *PN=SC->PolyP;
struct point3D *P;    /* point to clip */
register ULONG  IsInside0;
register ULONG  IsInside1;

    IsInside0=(SC->ClipMin.x <= PN[0].x.f);
    IsInside1=(SC->ClipMin.x <= PN[1].x.f);
    if(!IsInside0) if(!IsInside1)
        { SC->PolyPnb=0; return; }
    if(IsInside0!=IsInside1)
        { if(IsInside0) P=&PN[1]; else P=&PN[0]; CLIPPOINT(x.f,SC->ClipMin.x); }

    IsInside0=(PN[0].x.f <= SC->ClipMax.x);
    IsInside1=(PN[1].x.f <= SC->ClipMax.x);
    if(!IsInside0) if(!IsInside1)
        { SC->PolyPnb=0; return; }
    if(IsInside0!=IsInside1)
        { if(IsInside0) P=&PN[1]; else P=&PN[0]; CLIPPOINT(x.f,SC->ClipMax.x); }

    IsInside0=(SC->ClipMin.y <= PN[0].y.f);
    IsInside1=(SC->ClipMin.y <= PN[1].y.f);
    if(!IsInside0) if(!IsInside1)
         { SC->PolyPnb=0; return; }
    if(IsInside0!=IsInside1)
        { if(IsInside0) P=&PN[1]; else P=&PN[0]; CLIPPOINT(y.f,SC->ClipMin.y); }

    IsInside0=(PN[0].y.f <= SC->ClipMax.y);
    IsInside1=(PN[1].y.f <= SC->ClipMax.y);
    if(!IsInside0) if(!IsInside1)
         { SC->PolyPnb=0; return; }
    if(IsInside0!=IsInside1)
        { if(IsInside0) P=&PN[1]; else P=&PN[0]; CLIPPOINT(y.f,SC->ClipMax.y); }

    return;
}
/*=================================================================*/
void ClipPoly(struct SOFT3D_context *SC)
{
    struct point3D *P;
    struct point3D *PN;
    struct point3D *T1=(struct point3D *)&SC->T1;
    struct point3D *T2=(struct point3D *)&SC->T2;
    BOOL useT1=TRUE;
    UBYTE  IsInside[MAXPOLY];
    UBYTE InsidePnb;
    LONG    Pnb=SC->PolyPnb;
    LONG n,NewPnb;
    BOOL FaceClipped;

    if(Wazp3D->DebugClipper.ON)
    {
        SREM(ClipPoly)
    }
    if(Pnb>MAXPOLY) return;
    FaceClipped=FALSE;
    NewPnb=0;

    useT1=TRUE;
    PN=T1; P=T2;
    Libmemcpy(PN,SC->PolyP,Pnb*PSIZE);

    NewPnb=InsidePnb=0;
    NLOOP(Pnb)
    {
        IsInside[n]=FALSE;
        if(PN[n].x.f <= SC->ClipMax.x)
        { IsInside[n]=TRUE;    InsidePnb++; }
    }

    if(InsidePnb==0)
    {
        if(Wazp3D->DebugClipper.ON)
        {
            SREM(Outside Max.x)
        }
        goto HideFace;
    }

    if(InsidePnb!=Pnb)
    {
        if(Wazp3D->DebugClipper.ON)
        {
            SREM(Clip Max.x)
        }
        FaceClipped=TRUE; IsInside[Pnb]=IsInside[0]; COPYP(&(PN[Pnb]),PN);
        NLOOP(Pnb)
        {
            if(IsInside[n]==TRUE)
                CopyPoint(PN);
            if(IsInside[n]!=IsInside[n+1])
                NEWCLIPPOINT(x.f,SC->ClipMax.x);

            PN++;
        }
        Pnb=NewPnb;
        SwapBuffers
    }
/*=================================*/
    NewPnb=InsidePnb=0;
    NLOOP(Pnb)
    {

        IsInside[n]=FALSE;
        if(SC->ClipMin.x <= PN[n].x.f)
            { IsInside[n]=TRUE;    InsidePnb++; }
    }

    if(InsidePnb==0)
    {
        if(Wazp3D->DebugClipper.ON)
        {
            SREM(Outside Min.x)
        }
        goto HideFace;
    }

    if(InsidePnb!=Pnb)
    {
        if(Wazp3D->DebugClipper.ON)
        {
            SREM(Clip Min.x)
        }
        FaceClipped=TRUE; IsInside[Pnb]=IsInside[0]; COPYP(&(PN[Pnb]),PN);
        NLOOP(Pnb)
        {
            if(IsInside[n]==TRUE)
                    CopyPoint(PN);
            if(IsInside[n]!=IsInside[n+1])
                    NEWCLIPPOINT(x.f,SC->ClipMin.x);
            PN++;
        }
        Pnb=NewPnb;
        SwapBuffers
    }
/*=================================*/
    NewPnb=InsidePnb=0;
    NLOOP(Pnb)
    {

        IsInside[n]=FALSE;
        if(PN[n].y.f <= SC->ClipMax.y)
        { IsInside[n]=TRUE;    InsidePnb++; }
    }

    if(InsidePnb==0)
    {
        if(Wazp3D->DebugClipper.ON)
        {
            SREM(Outside Max.y)
        }
        goto HideFace;
    }

    if(InsidePnb!=Pnb)
    {
        if(Wazp3D->DebugClipper.ON)
        {
            SREM(Clip Max.y)
        }
        FaceClipped=TRUE; IsInside[Pnb]=IsInside[0]; COPYP(&(PN[Pnb]),PN);
        NLOOP(Pnb)
        {
            if(IsInside[n]==TRUE)
                    CopyPoint(PN);
            if(IsInside[n]!=IsInside[n+1])
                    NEWCLIPPOINT(y.f,SC->ClipMax.y);
            PN++;
        }
        Pnb=NewPnb;
        SwapBuffers
    }
/*=================================*/
    NewPnb=InsidePnb=0;
    NLOOP(Pnb)
    {

        IsInside[n]=FALSE;
        if(SC->ClipMin.y <= PN[n].y.f)
        { IsInside[n]=TRUE;    InsidePnb++; }
    }

    if(InsidePnb==0)
    {
        if(Wazp3D->DebugClipper.ON)
        {
            SREM(Outside Min.y)
        }
        goto HideFace;
    }

    if(InsidePnb!=Pnb)
    {
        if(Wazp3D->DebugClipper.ON)
        {
            SREM(Clip Min.y)
        }
        FaceClipped=TRUE; IsInside[Pnb]=IsInside[0]; COPYP(&(PN[Pnb]),PN);
        NLOOP(Pnb)
        {
            if(IsInside[n]==TRUE)
                CopyPoint(PN);
            if(IsInside[n]!=IsInside[n+1])
                NEWCLIPPOINT(y.f,SC->ClipMin.y);
            PN++;
        }
        Pnb=NewPnb;
        SwapBuffers
    }
/*=================================*/

    if(FaceClipped==FALSE) return;

/* if clipped get the new PolyPnb & PolyP */

    if(useT1) PN=T1; else PN=T2;

    if(Wazp3D->DebugClipper.ON)
    {
        SVARF(SC->ClipMin.x);
        SVARF(SC->ClipMax.x);
        SVARF(SC->ClipMin.y);
        SVARF(SC->ClipMax.y);
        SVARF(SC->ClipMin.z);
        SVARF(SC->ClipMax.z);

        if(SC->state.ST!=NULL)
        {
            PrintST((struct SOFT3D_texture *)swap32((uint32_t)SC->state.ST));
        }

        Wazp3D->DebugPoint.ON=TRUE;
        SREM(CLIPPER: original points)
        NLOOP(SC->PolyPnb)
            PrintP(&SC->PolyP[n]);
        SREM(-------: clipped points)
        NLOOP(Pnb)
            PrintP2(&PN[n]);
        SREM(=======================)
        Wazp3D->DebugPoint.ON=FALSE;
    }
    SC->PolyPnb=Pnb;
    Libmemcpy(SC->PolyP,PN,SC->PolyPnb*PSIZE);
    return;
HideFace:
    SC->PolyPnb=0;
    return;
}
/*=================================================================*/
void ReduceBitmap(UBYTE *pt,UBYTE *pt2,UWORD large,UWORD high, WORD bits,WORD ratio)
{
    UBYTE *RGB2;
    UBYTE *RGB1;
    LONG P,L;          /* bytesperpixel, bytesperline */
    LONG x,y,m,n;
    LONG r,g,b,a;
    LONG ratio2;
    LONG offset;

#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugSOFT3D.ON) Libprintf("ReduceBitmap/%ld %ldX%ld %ldbits >from %ld to %ld\n",(ULONG)ratio,(ULONG)large,(ULONG)high,(ULONG)bits,(ULONG)pt,(ULONG)pt2);
#endif
    if(pt ==NULL) return;
    if(pt2==NULL) return;
    L=large * bits /8;
    P=bits /8;
    large=large/ratio;
    high =high /ratio;
    ratio2=ratio*ratio;

    RGB1=pt;
    RGB2=pt2;

    YLOOP(high)
    {
        XLOOP(large)
        {
        r=g=b=a=0;
        MLOOP(ratio)
        NLOOP(ratio)
        {
            offset=L*m+P*n+L*ratio*y+P*ratio*x;
            RGB1=&(pt[offset]);
            r=r+(LONG)RGB1[0];
            g=g+(LONG)RGB1[1];
            b=b+(LONG)RGB1[2];
            if(bits==32)
                a=a+(LONG)RGB1[3];
        }
        r=r/ratio2;
        g=g/ratio2;
        b=b/ratio2;
        a=a/ratio2;

        RGB2[0]=r;
        RGB2[1]=g;
        RGB2[2]=b;

        if(Wazp3D->DebugST.ON)
            RGB2[1]=ratio*10;

        if(bits==32)
            RGB2[3]=a;
        RGB2=&(RGB2[P]);
        }
    }

}
/*==================================================================================*/
void CreateMipmaps(struct SOFT3D_texture *ST)
{
UBYTE *ptmm;
UWORD large,high,reduction;
ULONG size;


    large=ST->large;
    high =ST->high;
    size =ST->large * ST->high * ST->bits / 8;
    ST->ptmm=MMmalloc(size/3,"mipmaps");         /* size is mathematically false but allways fit */
    if(ST->ptmm==NULL) return;
    ptmm=ST->ptmm;
//    chunk_list_dump(&alloced_chunks, "Alloced");
//    chunk_list_dump(&freed_chunks, "Freed");

    reduction=1;
next_mipmap:
#ifdef WAZP3DDEBUG
    large=large/2;
    high =high /2;
    reduction=reduction*2;

    if(Wazp3D->DebugSOFT3D.ON) Libprintf("MipMap %ldX%ld = %ld (%ld)\n",(ULONG)large,(ULONG)high,(ULONG)size,(ULONG)ptmm);
#endif
    ReduceBitmap(ST->pt,ptmm,ST->large,ST->high,ST->bits,reduction);

    size=large * high * ST->bits / 8;
    ptmm=ptmm+size;
    if(large>1)
    if(high>1)
        goto next_mipmap;
}
/*==========================================================================*/
void AntiAliasImage(void *image,UWORD large,UWORD high)
{
UBYTE AliasedLines[4*MAXSCREEN*2];
register UWORD r,g,b,x,y;
register UBYTE *L0=(UBYTE *)image;
register UBYTE *L1;
register UBYTE *L2;
register UBYTE *line0;
register UBYTE *line1;
UBYTE *temp;
#define B32 4

    if (large>MAXSCREEN) return;
    line0=(UBYTE *) &AliasedLines[B32*MAXSCREEN*0];
    line1=(UBYTE *) &AliasedLines[B32*MAXSCREEN*1];
    L1=L0+large*B32;
    L2=L1+large*B32;
    large=large-2;
    high =high -2;

    XLOOP(large)
    {
    line0[0+B32]=L0[0+B32];
    line0[1+B32]=L0[1+B32];
    line0[2+B32]=L0[2+B32];
    line0+=B32;
    L0+=B32;
    }
    line1=(UBYTE *)&AliasedLines[B32*MAXSCREEN*1];
    L0=(UBYTE *)image;

    YLOOP(high)
    {
    line0=(UBYTE *) &AliasedLines[B32*MAXSCREEN*0];
    line1=(UBYTE *) &AliasedLines[B32*MAXSCREEN*1];
    if(y&1)
        SWAP(line0,line1)
        XLOOP(large)
        {
        r=L1[0+B32]; r=r+r; r=r+r; r=r+r;
        g=L1[1+B32]; g=g+g; g=g+g; g=g+g;
        b=L1[2+B32]; b=b+b; b=b+b; b=b+b;
        r=r+L0[0+0]+L0[0+B32]+L0[0+B32*2]+L1[0+0]+L1[0+B32*2]+L2[0+0]+L2[0+B32]+L2[0+B32*2];
        g=g+L0[1+0]+L0[1+B32]+L0[1+B32*2]+L1[1+0]+L1[1+B32*2]+L2[1+0]+L2[1+B32]+L2[1+B32*2];
        b=b+L0[2+0]+L0[2+B32]+L0[2+B32*2]+L1[2+0]+L1[2+B32*2]+L2[2+0]+L2[2+B32]+L2[2+B32*2];

        line1[0+B32]=r>>4;
        line1[1+B32]=g>>4;
        line1[2+B32]=b>>4;

        L0[0+B32]=line0[0+B32];
        L0[1+B32]=line0[1+B32];
        L0[2+B32]=line0[2+B32];

        line0+=B32;
        line1+=B32;
        L0+=B32;
        L1+=B32;
        L2+=B32;
        }

    L0+=B32*2;
    L1+=B32*2;
    L2+=B32*2;
    }
}
/*==========================================================================*/
UBYTE SOFT3D_DoUpdate(APTR sc)
{
struct SOFT3D_context *SC=sc;

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
SFUNCTION(SOFT3D_DoUpdate)

#ifdef USEOPENGL
    if(SC->UseHard)
    {
    if(Wazp3D->StepUpdate.ON)
        LibAlert("Update will occurs now !!");
    HARD3D_DoUpdate(&SC->HC);
#ifndef __MORPHOS__
    if(Wazp3D->UseClearImage.ON)
        if(SC->UseHard) HARD3D_ClearImageBuffer(&SC->HC,0,0,SC->large,SC->high,&SC->state.BackRGBA);
#endif

    return(TRUE);
    }
#endif



/* check is something new has been drawn ==> update */
    if(SC->Pxmax==0) return(FALSE);
    if(SC->Pymax==0) return(FALSE);

    if(!SC->UseHard)
    {
    if(SC->Image8==NULL) return(FALSE);
    SOFT3D_Flush(SC);
    }

    if(Wazp3D->StepUpdate.ON)
        LibAlert("Update will occurs now !!");

    if(SC->ImageBuffer32!=NULL)
    {
    SREM(Got an ImageBuffer32)
SREM(MinUpdate)
    if(Wazp3D->UseMinUpdate.ON)
        {
        SC->xUpdate        =SC->Pxmin;        /* should also be used to clear the previous drawing */
        SC->yUpdate        =SC->Pymin;
        SC->largeUpdate    =SC->Pxmax-SC->Pxmin+1;
        SC->highUpdate    =SC->Pymax-SC->Pymin+1;
        }
        else
        {

        SC->xUpdate        =0;
        SC->yUpdate        =0;
        SC->largeUpdate    =SC->large;
        SC->highUpdate    =SC->high;
        }

#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugSOFT3D.ON)
        Libprintf("Updated Region from %ld %ld (%ld X %ld pixels) \n",(ULONG)SC->xUpdate,(ULONG)SC->yUpdate,(ULONG)SC->largeUpdate,(ULONG)SC->highUpdate);
#endif
    SVARF(SC->Pzmin)
    SVARF(SC->Pzmax)

SREM(UseAntiImage)
    if(Wazp3D->UseAntiImage.ON)
        AntiAliasImage(SC->ImageBuffer32,SC->large,SC->high);

SREM(WriteImageBuffer)
    if(0<SC->largeUpdate)
    if(SC->largeUpdate<=SC->large)
    if(0< SC->highUpdate)
    if(SC->highUpdate <=SC->high)
    {
        SVAR(SC->xUpdate)
        SVAR(SC->yUpdate)
        SVAR(SC->largeUpdate)
        SVAR(SC->highUpdate)
        SC->FunctionWriteImageBuffer(SC);     /* Do in fact WritePixelArray(ImageBuffer32) to rastport */
        if(Wazp3D->UseClearImage.ON)
            SOFT3D_ClearImageBuffer(SC,0,0,SC->large,SC->high,(void *)SC->state.BackRGBA.L);
        }
    }

    SC->Pxmin=SC->large-1;
    SC->Pymin=SC->high -1;
    SC->Pzmin= 1000.0;
    SC->Pxmax=0;
    SC->Pymax=0;
    SC->Pzmax=-1000.0;

    return(TRUE);
}
/*==========================================================================*/
void SetDefaults(struct SOFT3D_context *SC)
{
    /* Set SOFT3D default values : notex nofog nozbuffer just white color */
    struct state3D defaultstate;
    ULONG WhiteRGBA[1]={0xFFFFFFFF};
	ULONG BlackRGBA[1]={0x000000FF};

    SREM(SetDefaults)
    defaultstate.ZMode=ZMODE(0,W3D_Z_ALWAYS);
    defaultstate.BlendMode=BLENDREPLACE;
    defaultstate.UseGouraud=FALSE;
    defaultstate.TexEnvMode=0;
    defaultstate.PerspMode=0;
    defaultstate.CullingMode=W3D_NOW;

    COPYRGBA(defaultstate.FogRGBA.L,WhiteRGBA);        /* default white fog           */
    COPYRGBA(defaultstate.CurrentRGBA.L,WhiteRGBA);    /* default white color         */
    COPYRGBA(defaultstate.EnvRGBA.L,WhiteRGBA);        /* default white env-color     */
    COPYRGBA(defaultstate.BackRGBA.L,BlackRGBA);       /* default black background    */

    defaultstate.PointSize=1;
    defaultstate.LineSize =1;

    defaultstate.UseFog=FALSE;
    defaultstate.FogMode=0;
    defaultstate.FogZmin=MINZ;
    defaultstate.FogZmax=MAXZ;
    defaultstate.FogDensity=0.0;

    defaultstate.UseTex=FALSE;
    defaultstate.ST=NULL;
    defaultstate.Changed=TRUE;

    SOFT3D_SetDrawState(SC,&defaultstate);
}
/*==========================================================================*/
void  SOFT3D_SetBitmap(APTR sc,void  *bm,APTR bmdata,ULONG bmformat,UWORD x,UWORD y,UWORD large,UWORD high)
{
#if !defined(PIXFMT_ABGR32)
#define PIXFMT_ABGR32    100
#define PIXFMT_0RGB32   101
#define PIXFMT_BGR032   102
#define PIXFMT_RGB032   103
#define PIXFMT_0BGR32   104
#endif

struct SOFT3D_context *SC=sc;
UBYTE *RGBA;
ULONG Rbits,Gbits,Bbits;
ULONG Rpos,Gpos,Bpos;
ULONG Rlostbits,Glostbits,Blostbits;
UWORD W,B0,B1;
ULONG n;
BOOL PcOrder,ChangeOrder;
ULONG temp;
WORD bits;
BOOL started;

    DEBUG_SOFT3D("%s start\n",__FUNCTION__);

    if(!Wazp3D->PrefsIsOpened)        /* if the user dont changing debug states */
    LibDebug=Wazp3D->DebugWazp3D.ON;    /* synchronize soft3d's LibDebug with global debug value "DebugWazp3D" setted with Wazp3-Prefs */
    if(SC==NULL)
    {
        DEBUG_SOFT3D("%s end SC NULL\n",__FUNCTION__);
    	return;
    }
SFUNCTION(SOFT3D_SetBitmap)
SVAR(bm)
SVAR(bmdata)
SVAR(bmformat)

    started=(SC->Image8!=NULL);
#ifdef AMIGA
    SC->bm=bm;
    SC->rastport.BitMap=bm;
#endif
    SC->yoffset=y;            /* only stored for WritePixelArray() */

    if(SC->ImageBuffer32!=NULL)
    {
    SREM(SetImage to ImageBuffer32)
    SC->bmformat=PIXFMT_RGBA32;
    SC->FunctionBitmapIn =NULL;        /* no need to read/write a bitmap in this case */
    SC->FunctionBitmapOut=NULL;
    SetImage(SC,0,0,large,high,32,(UBYTE *)SC->ImageBuffer32);
    DEBUG_SOFT3D("%s end ImageBuffer32 0x%08lx\n",__FUNCTION__,(uint32_t)SC->ImageBuffer32);
    return;
    }

    if(SC->bits!=0)
    if(SC->bmformat==bmformat)
        {bits=SC->bits; goto setimageonly;}

    SC->bmformat=bmformat;

/* Define the Pixels In/Out functions for this bitmap format */
    SREM(Bitmap format changed)
    Rbits=Gbits=Bbits=Rpos=Gpos=Bpos=bits=0;
    PcOrder=FALSE;

    switch (bmformat)
    {
    case PIXFMT_BGRA32:
    case PIXFMT_BGR032:
        SC->FunctionBitmapIn =(HOOKEDFUNCTION)PixelsInBGRA;
        SC->FunctionBitmapOut=(HOOKEDFUNCTION)PixelsOutBGRA;
        Rbits=8;    Gbits=8;    Bbits=8;
        Rpos=8;    Gpos= 16;    Bpos= 24;
        bits=32;
        PcOrder=FALSE;
        break;
    case PIXFMT_RGBA32:
    case PIXFMT_RGB032:
        SC->FunctionBitmapIn =(HOOKEDFUNCTION)PixelsInRGBA;
        SC->FunctionBitmapOut=(HOOKEDFUNCTION)PixelsOutRGBA;
        Rbits=8;    Gbits=8;    Bbits=8;
        Rpos=24;    Gpos= 16;    Bpos= 8;
        bits=32;
        PcOrder=FALSE;
        break;
    case PIXFMT_ARGB32:
    case PIXFMT_0RGB32:
        SC->FunctionBitmapIn =(HOOKEDFUNCTION)PixelsInARGB;
        SC->FunctionBitmapOut=(HOOKEDFUNCTION)PixelsOutARGB;
        Rbits=8;    Gbits=8;    Bbits=8;
        Rpos=16;    Gpos= 8;    Bpos= 0;
        bits=32;
        PcOrder=FALSE;
        break;
    case PIXFMT_ABGR32:
    case PIXFMT_0BGR32:
        SC->FunctionBitmapIn =(HOOKEDFUNCTION)PixelsInABGR;
        SC->FunctionBitmapOut=(HOOKEDFUNCTION)PixelsOutABGR;
        Rbits=8;    Gbits=8;    Bbits=8;
        Rpos=0;    Gpos= 8;    Bpos=16;
        bits=32;
        PcOrder=FALSE;
        break;

    case PIXFMT_RGB24:
        SC->FunctionBitmapIn =(HOOKEDFUNCTION)PixelsInRGB;
        SC->FunctionBitmapOut=(HOOKEDFUNCTION)PixelsOutRGB;
        Rbits=8;    Gbits=8;    Bbits=8;
        Rpos=16;    Gpos= 8;    Bpos= 0;
        bits=24;
        PcOrder=FALSE;
        break;
    case PIXFMT_BGR24:
        SC->FunctionBitmapIn =(HOOKEDFUNCTION)PixelsInBGR;
        SC->FunctionBitmapOut=(HOOKEDFUNCTION)PixelsOutBGR;
        Rbits=8;    Gbits=8;    Bbits=8;
        Rpos=0;    Gpos= 8;    Bpos= 16;
        bits=24;
        PcOrder=FALSE;
        break;

    case PIXFMT_RGB15:
        Rbits=5;    Gbits=5;    Bbits=5;
        Rpos=10;    Gpos= 5;    Bpos= 0;
        bits=16;
        PcOrder=FALSE;
        break;
    case PIXFMT_RGB16:
        Rbits=5;    Gbits=6;    Bbits=5;
        Rpos=11;    Gpos= 5;    Bpos= 0;
        bits=16;
        PcOrder=FALSE;
        break;
    case PIXFMT_BGR15:
        Rbits=5;    Gbits=5;    Bbits=5;
        Bpos=10;    Gpos= 5;    Rpos= 0;
        bits=16;
        PcOrder=FALSE;
        break;
    case PIXFMT_BGR16:
        Rbits=5;    Gbits=6;    Bbits=5;
        Bpos=11;    Gpos= 5;    Rpos= 0;
        bits=16;
        PcOrder=FALSE;
        break;

    case PIXFMT_RGB15PC:
        Rbits=5;    Gbits=5;    Bbits=5;
        Rpos=10;    Gpos= 5;    Bpos= 0;
        bits=16;
        PcOrder=TRUE;
        break;
    case PIXFMT_RGB16PC:
        Rbits=5;    Gbits=6;    Bbits=5;
        Rpos=11;    Gpos= 5;    Bpos= 0;
        bits=16;
        PcOrder=TRUE;
        break;
    case PIXFMT_BGR15PC:
        Rbits=5;    Gbits=5;    Bbits=5;
        Bpos=10;    Gpos= 5;    Rpos= 0;
        bits=16;
        PcOrder=TRUE;
        break;
    case PIXFMT_BGR16PC:
        Rbits=5;    Gbits=6;    Bbits=5;
        Bpos=11;    Gpos= 5;    Rpos= 0;
        bits=16;
        PcOrder=TRUE;
        break;

    case PIXFMT_LUT8:
        Rbits=3;    Gbits=3;    Bbits=2;
        Rpos=5;    Gpos= 2;    Bpos= 0;
        bits=8;
        PcOrder=FALSE;
        SC->FunctionBitmapIn =(HOOKEDFUNCTION)PixelsIn8;
        SC->FunctionBitmapOut=(HOOKEDFUNCTION)PixelsOut8;
        break;

    default:
        Libprintf("WAZP3D/SOFT3D: Unknown bitmap format %ld (%d X %d)\n",bmformat,large,high);
        DEBUG_SOFT3D("%s end Unknown bitmap format\n",__FUNCTION__);
        return;
    }

    SVAR(bmformat)
    SVAR(bits)


    if(bits<=16)
    {
    SVAR(Rbits)
    SVAR(Gbits)
    SVAR(Bbits)
    SVAR(Rpos)
    SVAR(Gpos)
    SVAR(Bpos)
    if(bits==16)
    {
    SC->FunctionBitmapIn =(HOOKEDFUNCTION)PixelsIn16;
    SC->FunctionBitmapOut=(HOOKEDFUNCTION)PixelsOut16;
    }

    ChangeOrder= PcOrder;    /* if cpu is motorola need to change pc's pixel-formats */
    SVAR(ChangeOrder)


/* do the precalculated tables to convert 8/15/16 bits <-> 32bits */
    Rlostbits=8-Rbits;
    Glostbits=8-Gbits;
    Blostbits=8-Bbits;

    NLOOP(256)
    {
    W=((n>>Rlostbits)<<Rpos);
    SC->RtoB0[n]=W>>8;
    SC->RtoB1[n]=W;
    W=((n>>Glostbits)<<Gpos);
    SC->GtoB0[n]=W>>8;
    SC->GtoB1[n]=W;
    W=((n>>Blostbits)<<Bpos);
    SC->BtoB0[n]=W>>8;
    SC->BtoB1[n]=W;

    B0=n<<8; B1=n;

    RGBA=(UBYTE *) &(SC->B0toRGBA32[n]) ;
    RGBA[0]=((B0>>Rpos)<<Rlostbits);
    RGBA[1]=((B0>>Gpos)<<Glostbits);
    RGBA[2]=((B0>>Bpos)<<Blostbits);
    RGBA[3]=255;

    RGBA=(UBYTE *) &(SC->B1toRGBA32[n]) ;
    RGBA[0]=((B1>>Rpos)<<Rlostbits);
    RGBA[1]=((B1>>Gpos)<<Glostbits);
    RGBA[2]=((B1>>Bpos)<<Blostbits);
    RGBA[3]=255;

    if(ChangeOrder)
        {
        SWAP(SC->RtoB0[n],SC->RtoB1[n])
        SWAP(SC->GtoB0[n],SC->GtoB1[n])
        SWAP(SC->BtoB0[n],SC->BtoB1[n])
        SWAP(SC->B0toRGBA32[n],SC->B1toRGBA32[n])
        }
    }

    }

setimageonly:
    SREM(SetImage to bitmap data)
    SetImage(SC,x,y,large,high,bits,bmdata);

#ifdef USEOPENGL
    SVAR(SC->Image8)
    if(SC->UseHard) HARD3D_SetBitmap(&SC->HC,bm,SC->Image8,bmformat,0,0,large,high);
#endif
    if(!started)
        SetDefaults(SC);
    DEBUG_SOFT3D("%s end\n",__FUNCTION__);
}
/*=============================================================*/
void PixelsIn16(struct SOFT3D_context *SC)
{
/* Convert 16bits -> buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register ULONG *B0toRGBA32=(ULONG *)SC->B0toRGBA32;
register ULONG *B1toRGBA32=(ULONG *)SC->B1toRGBA32;
register ULONG *RGBA32;

    while(0<size--)
    {
    RGBA32=(ULONG *)Frag[0].BufferRGBA.b;
    RGBA32[0]=B0toRGBA32[Frag[0].Image8[0]]+B1toRGBA32[Frag[0].Image8[1]];

    RGBA32=(ULONG *)Frag[1].BufferRGBA.b;
    RGBA32[0]=B0toRGBA32[Frag[1].Image8[0]]+B1toRGBA32[Frag[1].Image8[1]];
    Frag+=2;
    }

}
/*=============================================================*/
void PixelsOut16(struct SOFT3D_context *SC)
{
/* Convert buffer -> 16bits */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register UBYTE *RGB;
register UBYTE *RtoB0=SC->RtoB0;
register UBYTE *GtoB0=SC->GtoB0;
register UBYTE *BtoB0=SC->BtoB0;
register UBYTE *RtoB1=SC->RtoB1;
register UBYTE *GtoB1=SC->GtoB1;
register UBYTE *BtoB1=SC->BtoB1;

    while(0<size--)
    {
    	RGB=Frag[0].BufferRGBA.b;
    	Frag[0].Image8[0]=RtoB0[RGB[0]]+GtoB0[RGB[1]]+BtoB0[RGB[2]];
    	Frag[0].Image8[1]=RtoB1[RGB[0]]+GtoB1[RGB[1]]+BtoB1[RGB[2]];
    	RGB=Frag[1].BufferRGBA.b;
    	Frag[1].Image8[0]=RtoB0[RGB[0]]+GtoB0[RGB[1]]+BtoB0[RGB[2]];
    	Frag[1].Image8[1]=RtoB1[RGB[0]]+GtoB1[RGB[1]]+BtoB1[RGB[2]];
    	Frag+=2;
    }

}
/*=============================================================*/
void PixelsIn8(struct SOFT3D_context *SC)
{
/* Convert 8bits -> buffer */
register struct fragbuffer3D *Frag=SC->FragBuffer;
register ULONG size=SC->FragSize2;
register ULONG *B0toRGBA32=(ULONG *)SC->B0toRGBA32;
register ULONG *RGBA32;

    while(0<size--)
    {
    RGBA32=(ULONG *)Frag[0].BufferRGBA.b;
    RGBA32[0]=B0toRGBA32[Frag[0].Image8[0]];

    RGBA32=(ULONG *)Frag[1].BufferRGBA.b;
    RGBA32[0]=B0toRGBA32[Frag[1].Image8[0]];
    Frag+=2;
    }

}
/*=============================================================*/
void PixelsOut8(struct SOFT3D_context *SC)
{
    /* Convert buffer -> 8bits */
    register struct fragbuffer3D *Frag=SC->FragBuffer;
    register ULONG size=SC->FragSize2;
    register UBYTE *RtoB=SC->RtoB0;
    register UBYTE *GtoB=SC->GtoB0;
    register UBYTE *BtoB=SC->BtoB0;
    register UBYTE *RGB;

    while(0<size--)
    {
        RGB=Frag[0].BufferRGBA.b;
        Frag[0].Image8[0]=RtoB[RGB[0]]+GtoB[RGB[1]]+BtoB[RGB[2]];
        RGB=Frag[1].BufferRGBA.b;
        Frag[1].Image8[0]=RtoB[RGB[0]]+GtoB[RGB[1]]+BtoB[RGB[2]];
        Frag+=2;
    }

}
/*==========================================================================*/
BOOL LockBM(struct SOFT3D_context *SC)
{
#ifdef AMIGA
UBYTE *Image8;                    /* = bitmap memory  */

    if(SC->ImageBuffer32!=NULL)        /* So we dont write to a bitmap but to an RGBA buffer called "ImageBuffer32" */
        return(TRUE);

    SC->bmHandle=LockBitMapTags((APTR)SC->bm,LBMI_BASEADDRESS,(ULONG)&Image8, TAG_DONE);
    return(SC->bmHandle!=NULL);
#else
    return(TRUE);
#endif
}
/*==========================================================================*/
void UnLockBM(struct SOFT3D_context *SC)
{
#ifdef AMIGA
    if(SC->ImageBuffer32!=NULL)        /* So we dont write to a bitmap but to an RGBA buffer called "ImageBuffer32" */
        return;

    if(SC->bmHandle!=NULL)
        UnLockBitMap(SC->bmHandle);
#endif
}
/*=================================================================*/
