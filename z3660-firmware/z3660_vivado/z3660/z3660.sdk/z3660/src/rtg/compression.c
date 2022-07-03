#include "gfx.h"
#include "compression/compression.h"
#include <string.h>

void decompress_rle_smush1_data(uint8_t * src, uint8_t * dst, uint32_t size, uint16_t width, uint16_t height, uint16_t pitch)
{
    int size_line, h, length;
    uint8_t code, val;

    uint8_t *dst_base = dst;

	for (h = 0; h < height; h++) {
		size_line = ((uint16_t *)src)[0];
		src += 2;
		while (size_line > 0) {
			code = *src++;
			size_line--;
			length = (code >> 1) + 1;
			if (code & 1) {
				val = *src++;
				size_line--;
				if (val)
					memset(dst, val, length);
				dst += length;
			} else {
				size_line -= length;
				while (length--) {
					val = *src++;
					if (val)
						*dst = val;
					dst++;
				}
			}
		}
        dst_base += pitch;
        dst = dst_base;
	}
}

void bompDecodeLine(uint8_t *dst, const uint8_t *src, int len) {
	int num;
	uint8_t code, color;

	while (len > 0) {
		code = *src++;
		num = (code >> 1) + 1;
		if (num > len)
			num = len;
		len -= num;
		if (code & 1) {
			color = *src++;
			memset(dst, color, num);
		} else {
			memcpy(dst, src, num);
			src += num;
		}
		dst += num;
	}
}
