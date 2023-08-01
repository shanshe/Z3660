#include <stdint.h>
#include <xil_types.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../../../main.h"
#define inline

#define int32 int32_t
#define byte uint8_t
#define uint8 uint8_t
#define int16 int16_t
#define int8 int8_t
#define uint16 uint16_t
#define uint32 uint32_t

#define MAX_CHANNELS 2
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

uint8_t imc_tables_initialized = 0;
byte dest_imc_table[89];
uint32 dest_imc_table_2[89 * 64];

const int16 _stepAdjustTable[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

const int16 _imaTable[89] = {
		7,    8,    9,   10,   11,   12,   13,   14,
	   16,   17,   19,   21,   23,   25,   28,   31,
	   34,   37,   41,   45,   50,   55,   60,   66,
	   73,   80,   88,   97,  107,  118,  130,  143,
	  157,  173,  190,  209,  230,  253,  279,  307,
	  337,  371,  408,  449,  494,  544,  598,  658,
	  724,  796,  876,  963, 1060, 1166, 1282, 1411,
	 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
	 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
	 7132, 7845, 8630, 9493,10442,11487,12635,13899,
	15289,16818,18500,20350,22385,24623,27086,29794,
	32767
};

const byte imxOtherTable[6][64] = {
	{
		0xFF,
		4
	},

	{
		0xFF, 0xFF,
		   2,    8
	},

	{
		0xFF, 0xFF, 0xFF, 0xFF,
		   1,    2,    4,    6
	},

	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		   1,    2,    4,    6,    8,   12,   16,   32
	},

	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		   1,    2,    4,    6,    8,   10,   12,   14,
		  16,   18,   20,   22,   24,   26,   28,   32
	},

	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		   1,    2,    3,    4,    5,    6,    7,    8,
		   9,   10,   11,   12,   13,   14,   15,   16,
		  17,   18,   19,   20,   21,   22,   23,   24,
		  25,   26,   27,   28,   29,   30,   31,   32
	}
};

int32 clip_int32(int32 v, int32 amin, int32 amax) {
	if (v < amin) return (amin);
	else if (v > amax) return (amax);
	return (v);
}

inline uint16 READ_BE_UINT16(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return ((b[0] << 8) | b[1]);
}

inline uint32 READ_BE_UINT32(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3]));
}

inline void WRITE_BE_UINT16(void *ptr, uint16 value) {
	uint8 *b = (uint8 *)ptr;
	b[0] = (uint8)(value >> 8);
	b[1] = (uint8)(value >> 0);
}
inline void WRITE_BE_UINT32(void *ptr, uint32 value) {
	uint8 *b = (uint8 *)ptr;
	b[0] = (uint8)(value >> 24);
	b[1] = (uint8)(value >> 16);
	b[2] = (uint8)(value >>  8);
	b[3] = (uint8)(value >>  0);
}


void init_imc_tables() {
	int pos;

	for (pos = 0; pos <= 88; ++pos) {
		byte put = 1;
		int32 tableValue = ((_imaTable[pos] * 4) / 7) / 2;
		while (tableValue != 0) {
			tableValue /= 2;
			put++;
		}
		if (put < 3) {
			put = 3;
		}
		if (put > 8) {
			put = 8;
		}
		dest_imc_table[pos] = put - 1;
	}

	for (int n = 0; n < 64; n++) {
		for (pos = 0; pos <= 88; ++pos) {
			int32 count = 32;
			int32 put = 0;
			int32 tableValue = _imaTable[pos];
			do {
				if ((count & n) != 0) {
					put += tableValue;
				}
				count /= 2;
				tableValue /= 2;
			} while (count != 0);
			dest_imc_table_2[n + pos * 64] = put;
		}
	}
}

uint32_t decompress_adpcm(uint8_t *compInput, uint8_t *compOutput, int channels) {
	byte *src;

	// Decoder for the the IMA ADPCM variants used in COMI.
	// Contrary to regular IMA ADPCM, this codec uses a variable
	// bitsize for the encoded data.

	int32 outputSamplesLeft;
	int32 destPos;
	int16 firstWord;
	byte initialTablePos[MAX_CHANNELS] = {0, 0};
	int32 initialOutputWord[MAX_CHANNELS] = {0, 0};
	int32 totalBitOffset, curTablePos, outputWord;
	byte *dst;
	int i;

	src = compInput;
	dst = compOutput;
	outputSamplesLeft = 0x1000;

	// Every data packet contains 0x2000 bytes of audio data
	// when extracted. In order to encode bigger data sets,
	// one has to split the data into multiple blocks.
	//
	// Every block starts with a 2 byte word. If that word is
	// non-zero, it indicates the size of a block of raw audio
	// data (not encoded) following it. That data we simply copy
	// to the output buffer and then proceed by decoding the
	// remaining data.
	//
	// If on the other hand the word is zero, then what follows
	// are 7*channels bytes containing seed data for the decoder.
	firstWord = READ_BE_UINT16(src);
	src += 2;
	if (firstWord != 0) {
		// Copy raw data
		memcpy(dst, src, firstWord);
		dst += firstWord;
		src += firstWord;
		outputSamplesLeft -= firstWord / 2;
	} else {
		// Read the seed values for the decoder.
		for (i = 0; i < channels; i++) {
			initialTablePos[i] = *src;
			src += 1;
			//initialimcTableEntry[i] = READ_BE_UINT32(src);
			src += 4;
			initialOutputWord[i] = READ_BE_UINT32(src);
			src += 4;
		}
	}

	totalBitOffset = 0;
	// The channels are encoded separately.
	for (int chan = 0; chan < channels; chan++) {
		// Read initial state (this makes it possible for the data stream
		// to be split & spread across multiple data chunks.
		curTablePos = initialTablePos[chan];
		//imcTableEntry = initialimcTableEntry[chan];
		outputWord = initialOutputWord[chan];

		// We need to interleave the channels in the output; we achieve
		// that by using a variables dest offset:
		destPos = chan * 2;

		const int bound = (channels == 1)
							? outputSamplesLeft
							: ((chan == 0)
								? (outputSamplesLeft+1) / 2
								: outputSamplesLeft / 2);
		for (i = 0; i < bound; ++i) {
			// Determine the size (in bits) of the next data packet
			const int32 curTableEntryBitCount = dest_imc_table[curTablePos];

			// Read the next data packet
			const byte *readPos = src + (totalBitOffset >> 3);
			const uint16 readWord = (uint16)(READ_BE_UINT16(readPos) << (totalBitOffset & 7));
			const byte packet = (byte)(readWord >> (16 - curTableEntryBitCount));

			// Advance read position to the next data packet
			totalBitOffset += curTableEntryBitCount;

			// Decode the data packet into a delta value for the output signal.
			const byte signBitMask = (1 << (curTableEntryBitCount - 1));
			const byte dataBitMask = (signBitMask - 1);
			const byte data = (packet & dataBitMask);

			const int32 tmpA = (data << (7 - curTableEntryBitCount));
			const int32 imcTableEntry = _imaTable[curTablePos] >> (curTableEntryBitCount - 1);
			int32 delta = imcTableEntry + dest_imc_table_2[tmpA + (curTablePos * 64)];

			// The topmost bit in the data packet tells is a sign bit
			if ((packet & signBitMask) != 0) {
				delta = -delta;
			}

			// Accumulate the delta onto the output data
			outputWord += delta;

			// Clip outputWord to 16 bit signed, and write it into the destination stream
			outputWord = clip_int32(outputWord, -0x8000, 0x7fff);
			WRITE_BE_UINT16(dst + destPos, outputWord);
			destPos += channels << 1;

			// Adjust the curTablePos
			curTablePos += (int8)imxOtherTable[curTableEntryBitCount - 2][data];
			curTablePos = clip_int32(curTablePos, 0, ARRAYSIZE(_imaTable) - 1);
		}
	}

	return (0x2000);
}
