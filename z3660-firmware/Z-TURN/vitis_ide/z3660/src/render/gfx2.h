/*
 * gfx2.h
 *
 *  Created on: 19 sept. 2022
 *      Author: shanshe
 */

#ifndef SRC_RENDER_GFX2_H_
#define SRC_RENDER_GFX2_H_

#include <stdint.h>

typedef struct
{
	int16_t x;
	int16_t y;
} Point;
typedef struct
{
	Point P0,P1,P2;
} TriPoint;
typedef struct
{
	union{
		uint32_t argb;
		struct{
			uint8_t b;
			uint8_t g;
			uint8_t r;
			uint8_t a;
		} component;
	};
} Color;
typedef struct
{
	int h0,h1,h2;
} TriHvalue;
typedef struct
{
	TriPoint P;
	Color color;
	TriHvalue H;
} Triangle;

void drawline(Point P0, Point P1, Color color);
void drawWireframeTriangle(TriPoint P, Color color);
void drawFilledTriangle(TriPoint P, Color color);
void drawShadedTriangle(Triangle T);

void rotate(Point origin, Point *P, uint8_t dangle);

#endif /* SRC_RENDER_GFX2_H_ */
