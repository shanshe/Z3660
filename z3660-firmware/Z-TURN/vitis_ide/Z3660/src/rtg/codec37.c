// Adapted from ScummVM SMUSH codec37 class
#include <string.h>
#include "codec37.h"

void bompDecodeLine(uint8_t *dst, const uint8_t *src, int len);

static int num_decoders = 0;

struct Codec37Decoder c37_decoders[4];

void Codec37Decoder_Init(int idx, int width, int height)
{
	struct Codec37Decoder *dc = &c37_decoders[idx];
	memset(dc, 0x00, sizeof(struct Codec37Decoder));
	dc->_width = width;
	dc->_height = height;
	dc->_frameSize = width * height;
	dc->_deltaSize = dc->_frameSize * 3 + 0x13600;
	dc->_deltaBuf = (byte *)0x30000000 + num_decoders * 0x1000000;
	dc->_deltaBufs[0] = dc->_deltaBuf + 0x4D80;
	dc->_deltaBufs[1] = dc->_deltaBuf + 0xE880 + dc->_frameSize;
	dc->_curtable = 0;
	dc->_prevSeqNb = 0;
	dc->_tableLastPitch = -1;
	dc->_tableLastIndex = -1;
}

void Codec37Decoder_Next()
{
	num_decoders++;
	if (num_decoders > 3) {
		num_decoders = 0;
	}
}

int Codec37Decoder_GetCur()
{
	return(num_decoders);
}

void Codec37Decoder_MakeTable(int idx, int pitch, int index)
{
	struct Codec37Decoder *dc = &c37_decoders[idx];
	static const int8_t maketable_bytes[] = {
    0,   0,   1,   0,   2,   0,   3,   0,   5,   0,
    8,   0,  13,   0,  21,   0,  -1,   0,  -2,   0,
   -3,   0,  -5,   0,  -8,   0, -13,   0, -17,   0,
  -21,   0,   0,   1,   1,   1,   2,   1,   3,   1,
    5,   1,   8,   1,  13,   1,  21,   1,  -1,   1,
   -2,   1,  -3,   1,  -5,   1,  -8,   1, -13,   1,
  -17,   1, -21,   1,   0,   2,   1,   2,   2,   2,
    3,   2,   5,   2,   8,   2,  13,   2,  21,   2,
   -1,   2,  -2,   2,  -3,   2,  -5,   2,  -8,   2,
  -13,   2, -17,   2, -21,   2,   0,   3,   1,   3,
    2,   3,   3,   3,   5,   3,   8,   3,  13,   3,
   21,   3,  -1,   3,  -2,   3,  -3,   3,  -5,   3,
   -8,   3, -13,   3, -17,   3, -21,   3,   0,   5,
    1,   5,   2,   5,   3,   5,   5,   5,   8,   5,
   13,   5,  21,   5,  -1,   5,  -2,   5,  -3,   5,
   -5,   5,  -8,   5, -13,   5, -17,   5, -21,   5,
    0,   8,   1,   8,   2,   8,   3,   8,   5,   8,
    8,   8,  13,   8,  21,   8,  -1,   8,  -2,   8,
   -3,   8,  -5,   8,  -8,   8, -13,   8, -17,   8,
  -21,   8,   0,  13,   1,  13,   2,  13,   3,  13,
    5,  13,   8,  13,  13,  13,  21,  13,  -1,  13,
   -2,  13,  -3,  13,  -5,  13,  -8,  13, -13,  13,
  -17,  13, -21,  13,   0,  21,   1,  21,   2,  21,
    3,  21,   5,  21,   8,  21,  13,  21,  21,  21,
   -1,  21,  -2,  21,  -3,  21,  -5,  21,  -8,  21,
  -13,  21, -17,  21, -21,  21,   0,  -1,   1,  -1,
    2,  -1,   3,  -1,   5,  -1,   8,  -1,  13,  -1,
   21,  -1,  -1,  -1,  -2,  -1,  -3,  -1,  -5,  -1,
   -8,  -1, -13,  -1, -17,  -1, -21,  -1,   0,  -2,
    1,  -2,   2,  -2,   3,  -2,   5,  -2,   8,  -2,
   13,  -2,  21,  -2,  -1,  -2,  -2,  -2,  -3,  -2,
   -5,  -2,  -8,  -2, -13,  -2, -17,  -2, -21,  -2,
    0,  -3,   1,  -3,   2,  -3,   3,  -3,   5,  -3,
    8,  -3,  13,  -3,  21,  -3,  -1,  -3,  -2,  -3,
   -3,  -3,  -5,  -3,  -8,  -3, -13,  -3, -17,  -3,
  -21,  -3,   0,  -5,   1,  -5,   2,  -5,   3,  -5,
    5,  -5,   8,  -5,  13,  -5,  21,  -5,  -1,  -5,
   -2,  -5,  -3,  -5,  -5,  -5,  -8,  -5, -13,  -5,
  -17,  -5, -21,  -5,   0,  -8,   1,  -8,   2,  -8,
    3,  -8,   5,  -8,   8,  -8,  13,  -8,  21,  -8,
   -1,  -8,  -2,  -8,  -3,  -8,  -5,  -8,  -8,  -8,
  -13,  -8, -17,  -8, -21,  -8,   0, -13,   1, -13,
    2, -13,   3, -13,   5, -13,   8, -13,  13, -13,
   21, -13,  -1, -13,  -2, -13,  -3, -13,  -5, -13,
   -8, -13, -13, -13, -17, -13, -21, -13,   0, -17,
    1, -17,   2, -17,   3, -17,   5, -17,   8, -17,
   13, -17,  21, -17,  -1, -17,  -2, -17,  -3, -17,
   -5, -17,  -8, -17, -13, -17, -17, -17, -21, -17,
    0, -21,   1, -21,   2, -21,   3, -21,   5, -21,
    8, -21,  13, -21,  21, -21,  -1, -21,  -2, -21,
   -3, -21,  -5, -21,  -8, -21, -13, -21, -17, -21,
    0,   0,  -8, -29,   8, -29, -18, -25,  17, -25,
    0, -23,  -6, -22,   6, -22, -13, -19,  12, -19,
    0, -18,  25, -18, -25, -17,  -5, -17,   5, -17,
  -10, -15,  10, -15,   0, -14,  -4, -13,   4, -13,
   19, -13, -19, -12,  -8, -11,  -2, -11,   0, -11,
    2, -11,   8, -11, -15, -10,  -4, -10,   4, -10,
   15, -10,  -6,  -9,  -1,  -9,   1,  -9,   6,  -9,
  -29,  -8, -11,  -8,  -8,  -8,  -3,  -8,   3,  -8,
    8,  -8,  11,  -8,  29,  -8,  -5,  -7,  -2,  -7,
    0,  -7,   2,  -7,   5,  -7, -22,  -6,  -9,  -6,
   -6,  -6,  -3,  -6,  -1,  -6,   1,  -6,   3,  -6,
    6,  -6,   9,  -6,  22,  -6, -17,  -5,  -7,  -5,
   -4,  -5,  -2,  -5,   0,  -5,   2,  -5,   4,  -5,
    7,  -5,  17,  -5, -13,  -4, -10,  -4,  -5,  -4,
   -3,  -4,  -1,  -4,   0,  -4,   1,  -4,   3,  -4,
    5,  -4,  10,  -4,  13,  -4,  -8,  -3,  -6,  -3,
   -4,  -3,  -3,  -3,  -2,  -3,  -1,  -3,   0,  -3,
    1,  -3,   2,  -3,   4,  -3,   6,  -3,   8,  -3,
  -11,  -2,  -7,  -2,  -5,  -2,  -3,  -2,  -2,  -2,
   -1,  -2,   0,  -2,   1,  -2,   2,  -2,   3,  -2,
    5,  -2,   7,  -2,  11,  -2,  -9,  -1,  -6,  -1,
   -4,  -1,  -3,  -1,  -2,  -1,  -1,  -1,   0,  -1,
    1,  -1,   2,  -1,   3,  -1,   4,  -1,   6,  -1,
    9,  -1, -31,   0, -23,   0, -18,   0, -14,   0,
  -11,   0,  -7,   0,  -5,   0,  -4,   0,  -3,   0,
   -2,   0,  -1,   0,   0, -31,   1,   0,   2,   0,
    3,   0,   4,   0,   5,   0,   7,   0,  11,   0,
   14,   0,  18,   0,  23,   0,  31,   0,  -9,   1,
   -6,   1,  -4,   1,  -3,   1,  -2,   1,  -1,   1,
    0,   1,   1,   1,   2,   1,   3,   1,   4,   1,
    6,   1,   9,   1, -11,   2,  -7,   2,  -5,   2,
   -3,   2,  -2,   2,  -1,   2,   0,   2,   1,   2,
    2,   2,   3,   2,   5,   2,   7,   2,  11,   2,
   -8,   3,  -6,   3,  -4,   3,  -2,   3,  -1,   3,
    0,   3,   1,   3,   2,   3,   3,   3,   4,   3,
    6,   3,   8,   3, -13,   4, -10,   4,  -5,   4,
   -3,   4,  -1,   4,   0,   4,   1,   4,   3,   4,
    5,   4,  10,   4,  13,   4, -17,   5,  -7,   5,
   -4,   5,  -2,   5,   0,   5,   2,   5,   4,   5,
    7,   5,  17,   5, -22,   6,  -9,   6,  -6,   6,
   -3,   6,  -1,   6,   1,   6,   3,   6,   6,   6,
    9,   6,  22,   6,  -5,   7,  -2,   7,   0,   7,
    2,   7,   5,   7, -29,   8, -11,   8,  -8,   8,
   -3,   8,   3,   8,   8,   8,  11,   8,  29,   8,
   -6,   9,  -1,   9,   1,   9,   6,   9, -15,  10,
   -4,  10,   4,  10,  15,  10,  -8,  11,  -2,  11,
    0,  11,   2,  11,   8,  11,  19,  12, -19,  13,
   -4,  13,   4,  13,   0,  14, -10,  15,  10,  15,
   -5,  17,   5,  17,  25,  17, -25,  18,   0,  18,
  -12,  19,  13,  19,  -6,  22,   6,  22,   0,  23,
  -17,  25,  18,  25,  -8,  29,   8,  29,   0,  31,
    0,   0,  -6, -22,   6, -22, -13, -19,  12, -19,
    0, -18,  -5, -17,   5, -17, -10, -15,  10, -15,
    0, -14,  -4, -13,   4, -13,  19, -13, -19, -12,
   -8, -11,  -2, -11,   0, -11,   2, -11,   8, -11,
  -15, -10,  -4, -10,   4, -10,  15, -10,  -6,  -9,
   -1,  -9,   1,  -9,   6,  -9, -11,  -8,  -8,  -8,
   -3,  -8,   0,  -8,   3,  -8,   8,  -8,  11,  -8,
   -5,  -7,  -2,  -7,   0,  -7,   2,  -7,   5,  -7,
  -22,  -6,  -9,  -6,  -6,  -6,  -3,  -6,  -1,  -6,
    1,  -6,   3,  -6,   6,  -6,   9,  -6,  22,  -6,
  -17,  -5,  -7,  -5,  -4,  -5,  -2,  -5,  -1,  -5,
    0,  -5,   1,  -5,   2,  -5,   4,  -5,   7,  -5,
   17,  -5, -13,  -4, -10,  -4,  -5,  -4,  -3,  -4,
   -2,  -4,  -1,  -4,   0,  -4,   1,  -4,   2,  -4,
    3,  -4,   5,  -4,  10,  -4,  13,  -4,  -8,  -3,
   -6,  -3,  -4,  -3,  -3,  -3,  -2,  -3,  -1,  -3,
    0,  -3,   1,  -3,   2,  -3,   3,  -3,   4,  -3,
    6,  -3,   8,  -3, -11,  -2,  -7,  -2,  -5,  -2,
   -4,  -2,  -3,  -2,  -2,  -2,  -1,  -2,   0,  -2,
    1,  -2,   2,  -2,   3,  -2,   4,  -2,   5,  -2,
    7,  -2,  11,  -2,  -9,  -1,  -6,  -1,  -5,  -1,
   -4,  -1,  -3,  -1,  -2,  -1,  -1,  -1,   0,  -1,
    1,  -1,   2,  -1,   3,  -1,   4,  -1,   5,  -1,
    6,  -1,   9,  -1, -23,   0, -18,   0, -14,   0,
  -11,   0,  -7,   0,  -5,   0,  -4,   0,  -3,   0,
   -2,   0,  -1,   0,   0, -23,   1,   0,   2,   0,
    3,   0,   4,   0,   5,   0,   7,   0,  11,   0,
   14,   0,  18,   0,  23,   0,  -9,   1,  -6,   1,
   -5,   1,  -4,   1,  -3,   1,  -2,   1,  -1,   1,
    0,   1,   1,   1,   2,   1,   3,   1,   4,   1,
    5,   1,   6,   1,   9,   1, -11,   2,  -7,   2,
   -5,   2,  -4,   2,  -3,   2,  -2,   2,  -1,   2,
    0,   2,   1,   2,   2,   2,   3,   2,   4,   2,
    5,   2,   7,   2,  11,   2,  -8,   3,  -6,   3,
   -4,   3,  -3,   3,  -2,   3,  -1,   3,   0,   3,
    1,   3,   2,   3,   3,   3,   4,   3,   6,   3,
    8,   3, -13,   4, -10,   4,  -5,   4,  -3,   4,
   -2,   4,  -1,   4,   0,   4,   1,   4,   2,   4,
    3,   4,   5,   4,  10,   4,  13,   4, -17,   5,
   -7,   5,  -4,   5,  -2,   5,  -1,   5,   0,   5,
    1,   5,   2,   5,   4,   5,   7,   5,  17,   5,
  -22,   6,  -9,   6,  -6,   6,  -3,   6,  -1,   6,
    1,   6,   3,   6,   6,   6,   9,   6,  22,   6,
   -5,   7,  -2,   7,   0,   7,   2,   7,   5,   7,
  -11,   8,  -8,   8,  -3,   8,   0,   8,   3,   8,
    8,   8,  11,   8,  -6,   9,  -1,   9,   1,   9,
    6,   9, -15,  10,  -4,  10,   4,  10,  15,  10,
   -8,  11,  -2,  11,   0,  11,   2,  11,   8,  11,
   19,  12, -19,  13,  -4,  13,   4,  13,   0,  14,
  -10,  15,  10,  15,  -5,  17,   5,  17,   0,  18,
  -12,  19,  13,  19,  -6,  22,   6,  22,   0,  23,
	};

	if (dc->_tableLastPitch == pitch && dc->_tableLastIndex == index)
		return;

	dc->_tableLastPitch = pitch;
	dc->_tableLastIndex = index;
	index *= 255;

	for (int32 i = 0; i < 255; i++) {
		int32 j = (i + index) * 2;
		dc->_offsetTable[i] = maketable_bytes[j + 1] * pitch + maketable_bytes[j];
	}

}

void Codec37Decoder_proc1(int idx, byte *dst, const byte *src, int32 next_offs, int bw, int bh, int pitch, int16 *offset_table)
{
	//struct Codec37Decoder *dc = &c37_decoders[idx];
	uint8_t code;
	uint8_t filling, skipCode;
	int32 len;
	int i, p;
	uint32_t pitches[16];

	i = bw;
	for (p = 0; p < 16; ++p) {
		pitches[p] = (p >> 2) * pitch + (p & 0x3);
	}
	code = 0;
	filling = 0;
	len = -1;
	while (1) {
		if (len < 0) {
			filling = (*src & 1) == 1;
			len = *src++ >> 1;
			skipCode = 0;
		} else {
			skipCode = 1;
		}
		if (!filling || !skipCode) {
			code = *src++;
			if (code == 0xFF) {
				--len;
				for (p = 0; p < 0x10; ++p) {
					if (len < 0) {
						filling = (*src & 1) == 1;
						len = *src++ >> 1;
						if (filling) {
							code = *src++;
						}
					}
					if (filling) {
						*(dst + pitches[p]) = code;
					} else {
						*(dst + pitches[p]) = *src++;
					}
					--len;
				}
				dst += 4;
				--i;
				if (i == 0) {
					dst += pitch * 3;
					--bh;
					if (bh == 0) return;
					i = bw;
				}
				continue;
			}
		}
		byte *dst2 = dst + offset_table[code] + next_offs;
		COPY_4X4(dst2, dst, pitch);
		--i;
		if (i == 0) {
			dst += pitch * 3;
			--bh;
			if (bh == 0) return;
			i = bw;
		}
		--len;
	}
}

void Codec37Decoder_proc3WithFDFE(int idx, byte *dst, const byte *src, int32 next_offs, int bw, int bh, int pitch, int16 *offset_table)
{
	struct Codec37Decoder *dc = &c37_decoders[idx];
	do {
		int32 i = bw;
		do {
			int32 code = *src++;
			if (code == 0xFD) {
				LITERAL_4X4(src, dst, pitch);
			} else if (code == 0xFE) {
				LITERAL_4X1(src, dst, pitch);
			} else if (code == 0xFF) {
				LITERAL_1X1(src, dst, pitch);
			} else {
				byte *dst2 = dst + dc->_offsetTable[code] + next_offs;
				COPY_4X4(dst2, dst, pitch);
			}
		} while (--i);
		dst += pitch * 3;
	} while (--bh);
}

void Codec37Decoder_proc3WithoutFDFE(int idx, byte *dst, const byte *src, int32 next_offs, int bw, int bh, int pitch, int16 *offset_table)
{
	struct Codec37Decoder *dc = &c37_decoders[idx];
	do {
		int32 i = bw;
		do {
			int32 code = *src++;
			if (code == 0xFF) {
				LITERAL_1X1(src, dst, pitch);
			} else {
				byte *dst2 = dst + dc->_offsetTable[code] + next_offs;
				COPY_4X4(dst2, dst, pitch);
			}
		} while (--i);
		dst += pitch * 3;
	} while (--bh);
}

void Codec37Decoder_proc4WithFDFE(int idx, byte *dst, const byte *src, int32 next_offs, int bw, int bh, int pitch, int16 *offset_table)
{
	struct Codec37Decoder *dc = &c37_decoders[idx];
	do {
		int32 i = bw;
		do {
			int32 code = *src++;
			if (code == 0xFD) {
				LITERAL_4X4(src, dst, pitch);
			} else if (code == 0xFE) {
				LITERAL_4X1(src, dst, pitch);
			} else if (code == 0xFF) {
				LITERAL_1X1(src, dst, pitch);
			} else if (code == 0x00) {
				int32 length = *src++ + 1;
				for (int32 l = 0; l < length; l++) {
					byte *dst2 = dst + next_offs;
					COPY_4X4(dst2, dst, pitch);
					i--;
					if (i == 0) {
						dst += pitch * 3;
						bh--;
						i = bw;
					}
				}
				if (bh == 0) {
					return;
				}
				i++;
			} else {
				byte *dst2 = dst + dc->_offsetTable[code] + next_offs;
				COPY_4X4(dst2, dst, pitch);
			}
		} while (--i);
		dst += pitch * 3;
	} while (--bh);
}

void Codec37Decoder_proc4WithoutFDFE(int idx, byte *dst, const byte *src, int32 next_offs, int bw, int bh, int pitch, int16 *offset_table)
{
	struct Codec37Decoder *dc = &c37_decoders[idx];
	do {
		int32 i = bw;
		do {
			int32 code = *src++;
			if (code == 0xFF) {
				LITERAL_1X1(src, dst, pitch);
			} else if (code == 0x00) {
				int32 length = *src++ + 1;
				for (int32 l = 0; l < length; l++) {
					byte *dst2 = dst + next_offs;
					COPY_4X4(dst2, dst, pitch);
					i--;
					if (i == 0) {
						dst += pitch * 3;
						bh--;
						i = bw;
					}
				}
				if (bh == 0) {
					return;
				}
				i++;
			} else {
				byte *dst2 = dst + dc->_offsetTable[code] + next_offs;
				COPY_4X4(dst2, dst, pitch);
			}
		} while (--i);
		dst += pitch * 3;
	} while (--bh);
}

inline uint16 READ_UINT16(const void *ptr) {
	return (*(const uint16 *)(ptr));
}

inline uint32 READ_UINT32(const void *ptr) {
	return (*(const uint32 *)(ptr));
}

void Codec37Decoder_decode(int idx, uint8_t *dst, uint8_t *src)
{
	struct Codec37Decoder *dc = &c37_decoders[idx];
	int32 bw = (dc->_width + 3) / 4, bh = (dc->_height + 3) / 4;
	int32 pitch = bw * 4;

	int16 seq_nb = READ_UINT16(src + 2);
	int32 decoded_size = READ_UINT32(src + 4);
	byte mask_flags = src[12];
	Codec37Decoder_MakeTable(idx, pitch, src[1]);
	int32 tmp;

	switch (src[0]) {
	case 0:
		if ((dc->_deltaBufs[dc->_curtable] - dc->_deltaBuf) > 0) {
			memset(dc->_deltaBuf, 0, dc->_deltaBufs[dc->_curtable] - dc->_deltaBuf);
		}
		tmp = (dc->_deltaBuf + dc->_deltaSize) - dc->_deltaBufs[dc->_curtable] - decoded_size;
		if (tmp > 0) {
			memset(dc->_deltaBufs[dc->_curtable] + decoded_size, 0, tmp);
		}
		memcpy(dc->_deltaBufs[dc->_curtable], src + 16, decoded_size);
		break;
	case 1:
		if ((seq_nb & 1) || !(mask_flags & 1)) {
			dc->_curtable ^= 1;
		}
		Codec37Decoder_proc1(idx, dc->_deltaBufs[dc->_curtable], src + 16, dc->_deltaBufs[dc->_curtable ^ 1] - dc->_deltaBufs[dc->_curtable], bw, bh, pitch, dc->_offsetTable);
		break;
	case 2:
		bompDecodeLine(dc->_deltaBufs[dc->_curtable], src + 16, decoded_size);
		if ((dc->_deltaBufs[dc->_curtable] - dc->_deltaBuf) > 0) {
			memset(dc->_deltaBuf, 0, dc->_deltaBufs[dc->_curtable] - dc->_deltaBuf);
		}
		tmp = (dc->_deltaBuf + dc->_deltaSize) - dc->_deltaBufs[dc->_curtable] - decoded_size;
		if (tmp > 0) {
			memset(dc->_deltaBufs[dc->_curtable] + decoded_size, 0, tmp);
		}
		break;
	case 3:
		if ((seq_nb & 1) || !(mask_flags & 1)) {
			dc->_curtable ^= 1;
		}

		if ((mask_flags & 4) != 0) {
			Codec37Decoder_proc3WithFDFE(idx, dc->_deltaBufs[dc->_curtable], src + 16,
										dc->_deltaBufs[dc->_curtable ^ 1] - dc->_deltaBufs[dc->_curtable], bw, bh,
										pitch, dc->_offsetTable);
		} else {
			Codec37Decoder_proc3WithoutFDFE(idx, dc->_deltaBufs[dc->_curtable], src + 16,
										dc->_deltaBufs[dc->_curtable ^ 1] - dc->_deltaBufs[dc->_curtable], bw, bh,
										pitch, dc->_offsetTable);
		}
		break;
	case 4:
		if ((seq_nb & 1) || !(mask_flags & 1)) {
			dc->_curtable ^= 1;
		}

		if ((mask_flags & 4) != 0) {
			Codec37Decoder_proc4WithFDFE(idx, dc->_deltaBufs[dc->_curtable], src + 16,
										dc->_deltaBufs[dc->_curtable ^ 1] - dc->_deltaBufs[dc->_curtable], bw, bh,
										pitch, dc->_offsetTable);
		} else {
			Codec37Decoder_proc4WithoutFDFE(idx, dc->_deltaBufs[dc->_curtable], src + 16,
										dc->_deltaBufs[dc->_curtable ^ 1] - dc->_deltaBufs[dc->_curtable], bw, bh,
										pitch, dc->_offsetTable);
		}
		break;
	default:
		break;
	}
	dc->_prevSeqNb = seq_nb;

	memcpy(dst, dc->_deltaBufs[dc->_curtable], dc->_frameSize);
}
