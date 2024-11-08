/* Wazp3D Beta 56 : Alain THELLIER - Paris - FRANCE - (November 2006 to 2014)     */
/* Code clean-up and library enhancements from Gunther Nikl                 */
/* Adaptation to AROS from Matthias Rustler                            */
/* Adaptation to Morphos from Szil�rd 'BSzili' Bir�                         */
/* LICENSE: GNU General Public License (GNU GPL) for this file                */

/* This file contain the SOFT3D rasterizer that truly draw the pixels            */

//#include "../Wazp3D.h"

#include <inttypes.h>
typedef uint8_t UBYTE;
typedef uint16_t UWORD;
typedef unsigned long ULONG;
typedef int8_t BYTE;
typedef int16_t WORD;
//typedef int32_t LONG;
typedef void * APTR;
#define NULL ((void *)0)
#include "soft3d_protos.h"
#include "soft3d_opengl.h"
#include <proto/expansion.h>
#include <proto/exec.h>

/* use a local pointer on Wazp3D : having a copy here allow to separate the sof3d binary & wazp3d binary*/
#ifdef SOFT3DLIB

struct WAZP3D_parameters *Wazp3D;            /* local pointer to the struct in Warp3d.library */
//#include "soft3d_mem_print.h"                /* memory and print/debug functions : usually in Warp3d.c */

#endif
#include "z3660_regs.h"

volatile uint32_t *registers;
#define ZZ_REGS_WRITE(b, c) do{registers[(b)>>2]=c;}while(0)
#define ZZ_REGS_READ(b) registers[(b)>>2]
#define Z3660_MEMBASE_ADDR  0x00200000
#define Z3_SOFT3DDATA_ADDR  0x04200000
static volatile struct Soft3dData *soft3ddata;
#define KPrintF(...)
//#include <stdio.h>
//#define KPrintF printf
#include <string.h>

void open_soft3d56(void)
{
    struct ConfigDev* cd=NULL;
	if ((cd = (struct ConfigDev*)FindConfigDev(cd,0x144B,0x1)))
		registers = (uint32_t *)(cd->cd_BoardAddr);
    soft3ddata = (volatile struct Soft3dData*)(((uint32_t)cd->cd_BoardAddr) + (uint32_t)Z3_SOFT3DDATA_ADDR);
    KPrintF((const char *)"Soft3dData   0x%lx\n",(uint32_t)soft3ddata);
    memset((void *)soft3ddata, 0x00, sizeof(struct Soft3dData));
}

void *SOFT3D_Start(APTR PrefsWazp3D, ULONG len)
{
    soft3ddata->offset[0]=(uint32_t)PrefsWazp3D;
    uint32_t len2=len;
    CachePreDMA((APTR)(PrefsWazp3D),&len2,0);
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_START);
    CachePostDMA((APTR)PrefsWazp3D,&len2,0);
    uint32_t *a=(uint32_t *)ZZ_REGS_READ(REG_ZZ_SOFT3D_OP);
    KPrintF((const char *)"%s  start\n",__FUNCTION__);
    return(a);
}
void SOFT3D_End(APTR sc)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_END);
}

void  SOFT3D_SetBitmap(APTR sc,void  *bm,APTR bmdata,ULONG bmformat,UWORD x,UWORD y,UWORD large,UWORD high)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->offset[1]=(uint32_t)bm;
    soft3ddata->offset[2]=(uint32_t)bmdata;
    soft3ddata->format[0]=bmformat;
    soft3ddata->x[0]= x;
    soft3ddata->y[0]= y;
    soft3ddata->x[1]= large;
    soft3ddata->y[1]= high;
    uint32_t len2=sizeof(struct  BitMap);
    CachePreDMA((APTR)(bm),&len2,0);
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_SETBITMAP);
    CachePostDMA((APTR)bm,&len2,0);
}
void  SOFT3D_SetClipping(APTR sc,UWORD xmin,UWORD xmax,UWORD ymin,UWORD ymax)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->x[0]= xmin;
    soft3ddata->x[1]= xmax;
    soft3ddata->y[0]= ymin;
    soft3ddata->y[1]= ymax;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_SETCLIPPING);
}
void SOFT3D_SetDrawState(APTR sc,APTR sta)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->offset[1]=(uint32_t)sta;
    uint32_t len2=15*4; //struct state3D
    CachePreDMA((APTR)(sta),&len2,0);
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_SETDRAWSTATE);
    CachePostDMA((APTR)sta,&len2,0);
}
void SOFT3D_DrawPrimitive(APTR sc,APTR p,ULONG Pnb,ULONG primitive)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->offset[1]=(uint32_t)p;
    soft3ddata->format[0]=Pnb;
    soft3ddata->format[1]=primitive;
    uint32_t len2=7*4*MAXPRIM; // struct Point3D 6 floats + 1 long
    CachePreDMA((APTR)(p),&len2,0);
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_DRAWPRIMITIVE);
    CachePostDMA((APTR)p,&len2,0);
}
UBYTE SOFT3D_DoUpdate(APTR sc)
{
    uint32_t a;
    soft3ddata->offset[0]=(uint32_t)sc;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_DOUPDATE);
    a=ZZ_REGS_READ(REG_ZZ_SOFT3D_OP);
    return(a);
}
void SOFT3D_Flush(APTR sc)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_FLUSH);
}
void *SOFT3D_CreateTexture(APTR sc,APTR pt,UWORD large,UWORD high,UWORD format,UBYTE TexFlags)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->offset[1]=(uint32_t)pt;
    soft3ddata->x[0]=large;
    soft3ddata->y[0]=high;
    soft3ddata->x[1]=format;
    soft3ddata->y[1]=TexFlags;
    uint32_t len2=256*256*4; //???
    CachePreDMA((APTR)(pt),&len2,0);
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_CREATETEXTURE);
    CachePostDMA((APTR)pt,&len2,0);
    uint32_t *a=(uint32_t *)ZZ_REGS_READ(REG_ZZ_SOFT3D_OP);
    return(a);
}
void SOFT3D_Debug(APTR txt)
{
    soft3ddata->offset[0]=(uint32_t)txt;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_DEBUG);
}
void SOFT3D_FreeTexture(APTR sc,APTR st)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->offset[1]=(uint32_t)st;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_FREETEXTURE);
}
void SOFT3D_UpdateTexture(APTR sc,APTR st,APTR pt)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->offset[1]=(uint32_t)st;
    soft3ddata->offset[2]=(uint32_t)pt;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_UPDATETEXTURE);
}
APTR SOFT3D_AllocZbuffer(APTR sc,UWORD large,UWORD high)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->x[0]=large;
    soft3ddata->y[0]=high;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_ALLOCZBUFFER);
    uint32_t *a=(uint32_t *)ZZ_REGS_READ(REG_ZZ_SOFT3D_OP);
    return(a);
}
void SOFT3D_AllocImageBuffer(APTR sc,UWORD large,UWORD high)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->x[0]=large;
    soft3ddata->y[0]=high;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_ALLOCIMAGEBUFFER);
}
void SOFT3D_ClearZBuffer(APTR sc,float fz)
{
    uint32_t *fz_uint32=(uint32_t *)&fz;
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->format[0]=*fz_uint32;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_CLEARZBUFFER);
}
void SOFT3D_ReadZSpan(APTR sc, UWORD x, UWORD y,ULONG n, APTR z)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->x[0]=x;
    soft3ddata->y[0]=y;
    soft3ddata->format[0]=n;
    soft3ddata->offset[1]=(uint32_t)z;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_READZSPAN);
}
void SOFT3D_WriteZSpan(APTR sc, UWORD x, UWORD y,ULONG n,APTR z,APTR mask)
{
    soft3ddata->offset[0]=(uint32_t)sc;
    soft3ddata->x[0]=x;
    soft3ddata->y[0]=y;
    soft3ddata->format[0]=n;
    soft3ddata->offset[1]=(uint32_t)z;
    soft3ddata->offset[2]=(uint32_t)mask;
    ZZ_REGS_WRITE(REG_ZZ_SOFT3D_OP, OP_WRITEZSPAN);
}
