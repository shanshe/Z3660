/*
 * gfx2.c
 *
 *  Created on: 19 sept. 2022
 *      Author: shanshe
 */

#include <stdio.h>
#include <math.h>
#define kprintf(...)
//#define kprintf printf
#include "gfx2.h"
#include "../video.h"
#include "../rtg/gfx.h"

#define M_PI 3.14159265358979323846

void interpolate2(uint16_t *v,uint16_t i0, uint16_t d0,uint16_t i1, uint16_t d1)
{
	if(i0==i1)
	{
		v[0]=d0;
		return;
	}
	int16_t di=i1-i0;
	int16_t dd=d1-d0;
	kprintf("di %d\n",di);
	kprintf("dd %d\n",dd);
	int div=(1024*dd)/di;
	for(uint16_t i=0;i<=di;i++)
	{
		// interpolation with division
		v[i]=d0+((i*div)>>10);
		kprintf("v[%d]=%d\n",i,v[i]);
	}

}
/*
void interpolate(uint16_t *v,uint16_t x0, uint16_t y0,uint16_t x1, uint16_t y1)
{
	if(x0==x1)
	{
		v[0]=y0;
		return;
	}

	int16_t dx=x1-x0;
	int16_t dy=y1-y0;
	int16_t yi=1;
	if (dy<0)
	{
		yi=-1;
		dy=-dy;
	}
	int16_t avR=2*dy;
	int16_t av=avR-dx;
	int16_t avI=av-dx;
	uint16_t y=y0;
	for(uint16_t x=0;x<=dx;x++)
	{
		// Bresenham's line algorithm
		v[x]=y;
		if(av>=0)
		{
			y+=yi;
			av+=avI;
		}
		else
		{
			av+=avR;
		}
	}
}
*/
void swap(Point *p0, Point *p1)
{
	Point temp;
	temp.x=p0->x;
	temp.y=p0->y;
	p0->x=p1->x;
	p0->y=p1->y;
	p1->x=temp.x;
	p1->y=temp.y;
}
#define ABS(x) ((x)>0?(x):-(x))
extern ZZ_VIDEO_STATE vs;

uint16_t argb888_to_rgb565(uint32_t argb)
{
 // R: FF00 -> 7e0
	uint16_t c;
	c =(argb&0xF80000)>>8;
	c|=(argb&0x00FC00)>>5;
	c|=(argb&0x0000F8)>>3;
	return(swap16(c));
}
void putpixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code)
{
	if(vs.colormode==MNTVA_COLOR_8BIT)
		*(volatile uint8_t*)(((uint32_t)vs.framebuffer+(uint32_t)vs.framebuffer_pan_offset) + (1*(((Ypos*vs.vmode_hsize)>>(vs.scalemode?1:0)) + Xpos))) = (uint8_t)RGB_Code;
	else if(vs.colormode==MNTVA_COLOR_16BIT565)
		*(volatile uint16_t*)(((uint32_t)vs.framebuffer+(uint32_t)vs.framebuffer_pan_offset) + (2*(((Ypos*vs.vmode_hsize)>>(vs.scalemode?1:0)) + Xpos))) = argb888_to_rgb565(RGB_Code);
	else if(vs.colormode==MNTVA_COLOR_32BIT)
		*(volatile uint32_t*)(((uint32_t)vs.framebuffer+(uint32_t)vs.framebuffer_pan_offset) + (4*(((Ypos*vs.vmode_hsize)>>(vs.scalemode?1:0)) + Xpos))) = RGB_Code;
}

void drawline(Point P0, Point P1, Color color)
{
	if(ABS(P1.x-P0.x)>ABS(P1.y-P0.y))
	{
		// line is horizontal-ish
		if(P0.x>P1.x)
		{
			swap(&P0,&P1);
		}
		uint16_t v[P1.x-P0.x+1];
		interpolate2(v,P0.x,P0.y,P1.x,P1.y);
		for(uint16_t x=P0.x;x<=P1.x;x++)
			putpixel(x,v[x-P0.x],color.argb);
	}
	else
	{
		// line is vertical-ish
		if(P0.y>P1.y)
		{
			swap(&P0,&P1);
		}
		uint16_t v[P1.y-P0.y+1];
		interpolate2(v,P0.y,P0.x,P1.y,P1.x);
		for(uint16_t y=P0.y;y<=P1.y;y++)
			putpixel(v[y-P0.y],y,color.argb);
	}
}

void drawWireframeTriangle(TriPoint P, Color color)
{
	drawline(P.P0,P.P1,color);
	drawline(P.P1,P.P2,color);
	drawline(P.P2,P.P0,color);
}

void rotate(Point origin, Point *P, uint8_t dangle)
{
	Point temp;
	temp.x=P->x-origin.x;
	temp.y=P->y-origin.y;
	double vcos=cos(dangle*M_PI/128);
	double vsin=sin(dangle*M_PI/128);
	P->x=vcos*temp.x-vsin*temp.y+origin.x;
	P->y=vsin*temp.x+vcos*temp.y+origin.y;
}

void drawFilledTriangle(TriPoint P, Color color)
{
	if(P.P1.y<P.P0.y) swap(&P.P0,&P.P1);
	if(P.P2.y<P.P0.y) swap(&P.P0,&P.P2);
	if(P.P2.y<P.P1.y) swap(&P.P1,&P.P2);
	kprintf("x0=%d,y0=%d\n",P.P0.x,P.P0.y);
	kprintf("x1=%d,y1=%d\n",P.P1.x,P.P1.y);
	kprintf("x2=%d,y2=%d\n",P.P2.x,P.P2.y);

	uint16_t x01[P.P1.y-P.P0.y+1];
	interpolate2(x01,P.P0.y,P.P0.x,P.P1.y,P.P1.x);
	uint16_t x12[P.P2.y-P.P1.y+1];
	interpolate2(x12,P.P1.y,P.P1.x,P.P2.y,P.P2.x);
	uint16_t x02[P.P2.y-P.P0.y+1];
	interpolate2(x02,P.P0.y,P.P0.x,P.P2.y,P.P2.x);

	uint16_t x012[P.P2.y-P.P0.y+1];
	for(uint16_t x=0;x<P.P1.y-P.P0.y;x++)
		x012[x]=x01[x];
	for(uint16_t x=P.P1.y-P.P0.y;x<=P.P2.y-P.P0.y;x++)
		x012[x]=x12[x-(P.P1.y-P.P0.y)];

	uint16_t *xleft,*xright;
	int m=(P.P2.y-P.P0.y+1)>>1;
	if(x02[m]<x012[m])
	{
		xleft=x02;
		xright=x012;
	}
	else
	{
		xleft=x012;
		xright=x02;
	}

	for(int16_t y=P.P0.y;y<=P.P2.y;y++)
		for(int16_t x=xleft[y-P.P0.y];x<=xright[y-P.P0.y];x++)
			putpixel(x,y,color.argb);
}
Color shaded;

void drawShadedTriangle(Triangle T)
{
	int temp;
	if(T.P.P1.y<T.P.P0.y) {swap(&T.P.P0,&T.P.P1); temp=T.H.h0;T.H.h0=T.H.h1;T.H.h1=temp;}
	if(T.P.P2.y<T.P.P0.y) {swap(&T.P.P0,&T.P.P2); temp=T.H.h0;T.H.h0=T.H.h2;T.H.h2=temp;}
	if(T.P.P2.y<T.P.P1.y) {swap(&T.P.P1,&T.P.P2); temp=T.H.h1;T.H.h1=T.H.h2;T.H.h2=temp;}
	kprintf("x0=%d,y0=%d\n",T.P.P0.x,T.P.P0.y);
	kprintf("x1=%d,y1=%d\n",T.P.P1.x,T.P.P1.y);
	kprintf("x2=%d,y2=%d\n",T.P.P2.x,T.P.P2.y);

	uint16_t x01[T.P.P1.y-T.P.P0.y+1];
	uint16_t h01[T.P.P1.y-T.P.P0.y+1];
	interpolate2(x01,T.P.P0.y,T.P.P0.x,T.P.P1.y,T.P.P1.x);
	interpolate2(h01,T.P.P0.y,T.H.h0,T.P.P1.y,T.H.h1);
	uint16_t x12[T.P.P2.y-T.P.P1.y+1];
	uint16_t h12[T.P.P2.y-T.P.P1.y+1];
	interpolate2(x12,T.P.P1.y,T.P.P1.x,T.P.P2.y,T.P.P2.x);
	interpolate2(h12,T.P.P1.y,T.H.h1,T.P.P2.y,T.H.h2);
	uint16_t x02[T.P.P2.y-T.P.P0.y+1];
	uint16_t h02[T.P.P2.y-T.P.P0.y+1];
	interpolate2(x02,T.P.P0.y,T.P.P0.x,T.P.P2.y,T.P.P2.x);
	interpolate2(h02,T.P.P0.y,T.H.h0,T.P.P2.y,T.H.h2);

	uint16_t x012[T.P.P2.y-T.P.P0.y+1];
	uint16_t h012[T.P.P2.y-T.P.P0.y+1];
	for(uint16_t x=0;x<T.P.P1.y-T.P.P0.y;x++)
	{
		x012[x]=x01[x];
		h012[x]=h01[x];
	}
	for(uint16_t x=T.P.P1.y-T.P.P0.y;x<=T.P.P2.y-T.P.P0.y;x++)
	{
		x012[x]=x12[x-(T.P.P1.y-T.P.P0.y)];
		h012[x]=h12[x-(T.P.P1.y-T.P.P0.y)];
	}

	uint16_t *xleft,*xright;
	uint16_t *hleft,*hright;
	int m=(T.P.P2.y-T.P.P0.y+1)>>1;
	if(x02[m]<x012[m])
	{
		xleft=x02;
		hleft=h02;
		xright=x012;
		hright=h012;
	}
	else
	{
		xleft=x012;
		hleft=h012;
		xright=x02;
		hright=h02;
	}

	for(int16_t y=T.P.P0.y;y<=T.P.P2.y;y++)
	{
		uint16_t xl=xleft[y-T.P.P0.y];
		uint16_t xr=xright[y-T.P.P0.y];
		uint16_t hseg[xr-xl+1];
		interpolate2(hseg,xl,hleft[y-T.P.P0.y],xr,hright[y-T.P.P0.y]);

		for(int16_t x=xl;x<=xr;x++)
		{
			int h=1024*hseg[x-xl];
			shaded.component.r=(T.color.component.r*h)>>18;
			shaded.component.g=(T.color.component.g*h)>>18;
			shaded.component.b=(T.color.component.b*h)>>18;
			putpixel(x,y,shaded.argb);
		}
	}
}
