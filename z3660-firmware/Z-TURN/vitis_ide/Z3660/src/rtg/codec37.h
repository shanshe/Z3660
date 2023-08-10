/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <stdint.h>

#define byte uint8_t
#define int32 int32_t
#define int16 int16_t
#define uint16 uint16_t
#define uint32 uint32_t

#define DECLARE_LITERAL_TEMP(v)			\
	uint32 v

#define READ_LITERAL_PIXEL(src, v)			\
	do {						\
		v = *src++;				\
		v += (v << 8) + (v << 16) + (v << 24);	\
	} while (0)

#define WRITE_4X1_LINE(dst, v)			\
	*(uint32 *)(dst) = v

#define COPY_4X1_LINE(dst, src)			\
	*(uint32 *)(dst) = *(const uint32 *)(src)

/* Fill a 4x4 pixel block with a literal pixel value */

#define LITERAL_4X4(src, dst, pitch)				\
	do {							\
		int x;						\
		DECLARE_LITERAL_TEMP(t);			\
		READ_LITERAL_PIXEL(src, t);			\
		for (x=0; x<4; x++) {				\
			WRITE_4X1_LINE(dst + pitch * x, t);	\
		}						\
		dst += 4;					\
	} while (0)

/* Fill four 4x1 pixel blocks with literal pixel values */

#define LITERAL_4X1(src, dst, pitch)				\
	do {							\
		int x;						\
		DECLARE_LITERAL_TEMP(t);			\
		for (x=0; x<4; x++) {				\
			READ_LITERAL_PIXEL(src, t);		\
			WRITE_4X1_LINE(dst + pitch * x, t);	\
		}						\
		dst += 4;					\
	} while (0)

/* Fill sixteen 1x1 pixel blocks with literal pixel values */

#define LITERAL_1X1(src, dst, pitch)				\
	do {							\
		int x;						\
		for (x=0; x<4; x++) {				\
			COPY_4X1_LINE(dst + pitch * x, src);	\
			src += 4;				\
		}						\
		dst += 4;					\
	} while (0)

/* Copy a 4x4 pixel block from a different place in the framebuffer */

#define COPY_4X4(dst2, dst, pitch)					  \
	do {								  \
		int x;							  \
		for (x=0; x<4; x++) {					  \
			COPY_4X1_LINE(dst + pitch * x, dst2 + pitch * x); \
		}							  \
		dst += 4;						  \
	} while (0)

struct Codec37Decoder {
	int32 _deltaSize;
	byte *_deltaBufs[2];
	byte *_deltaBuf;
	int16 _offsetTable[255];
	int _curtable;
	uint16 _prevSeqNb;
	int _tableLastPitch;
	int _tableLastIndex;
	int32 _frameSize;
	int _width, _height;
};
