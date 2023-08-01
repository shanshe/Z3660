#include <stdio.h>
#include <xil_cache.h>
#include "mp3.h"
#define MINIMP3_IMPLEMENTATION 1
#include "minimp3_ex.h"

#define SWAP16(a) a = __builtin_bswap16(a)
#define SWAP32(a) a = __builtin_bswap32(a)

mp3dec_ex_t mp3d;
mp3dec_io_t mp3io;
mp3dec_frame_info_t frame_info;
uint8_t* FifoAddr;
unsigned long FifoSize = 0;
//unsigned long OldFifoWriteIdx = 0;
unsigned long FifoWriteIdx = 0; // in
unsigned long FifoReadIdx  = 0; // out

static size_t read_cb(void *buf, size_t size, void *user_data) {
	unsigned long BytesRead = 0;
	long BytesToRead = size;
	volatile unsigned char *src = user_data;
	volatile unsigned char *dst = buf;
//	unsigned long origFifoReadIdx=FifoReadIdx;

	while(BytesToRead) {
		// If FiFo is empty then exit the loop.
		if(FifoReadIdx == FifoWriteIdx) break;
		dst[BytesRead++] = src[FifoReadIdx++];
		if(FifoReadIdx >= FifoSize) FifoReadIdx = 0;
		BytesToRead--;
	}
//	printf("BytesRead=%ld from %p to %p\n",BytesRead,src+origFifoReadIdx,dst);
	return(BytesRead);
}

static int seek_cb(uint64_t position, void *user_data) {
	FifoReadIdx = position % FifoSize;
	return(0);
}

void fifo_clear(void) {
	FifoReadIdx  = 0;
	FifoWriteIdx = 0;
}

void fifo_set_write_index(unsigned long aWriteIndex) {
	FifoWriteIdx = aWriteIndex;
/*	// New data has arrived from the 68k size.
	// We need to invalidate the data cache where the data came in.
	if(FifoWriteIdx > OldFifoWriteIdx) {
		// Invalidate range from old til new.
		Xil_DCacheInvalidateRange((INTPTR)&FifoAddr[OldFifoWriteIdx], FifoWriteIdx-OldFifoWriteIdx);
	}
	else {
		// 1. Invalidate range from old til end.
		Xil_DCacheInvalidateRange((INTPTR)&FifoAddr[OldFifoWriteIdx], FifoSize-OldFifoWriteIdx);
		// 2. Invalidate range from beginning til new.
		Xil_DCacheInvalidateRange((INTPTR)&FifoAddr[0], FifoWriteIdx);
	}
	OldFifoWriteIdx = FifoWriteIdx;
*/
}

unsigned long fifo_get_read_index(void) {
	return(FifoReadIdx);
}
size_t read_samples;

int decode_mp3_samples(void* output_buffer, int max_samples) {
	int max_bytes = max_samples * 2;
	int out_offset = 0;
	int total_bytes_decoded = 0;

	// Clear destination buffer before trying to decode.
	memset(output_buffer, 0, max_bytes);

	// this will point into mp3d->buffer, which is defined on the stack
	// as mp3d_sample_t buffer[MINIMP3_MAX_SAMPLES_PER_FRAME]
	mp3d_sample_t * pcm_buffer = NULL;

	while (1) {
		read_samples = mp3dec_ex_read_frame(&mp3d, &pcm_buffer, &frame_info, max_samples);
		max_samples -= read_samples;

		int bytes_decoded = read_samples * sizeof(mp3d_sample_t);
		total_bytes_decoded += bytes_decoded;
/*
		if(bytes_decoded>0)
		{
			printf("[mp3] decoded: %d bytes\n", bytes_decoded);
			printf("[mp3] output_buffer: %p max_bytes: %d\n", output_buffer, max_bytes);
		}
*/
		if (bytes_decoded > 0) {
			int bytes_to_copy = bytes_decoded;
			memcpy(output_buffer + out_offset, pcm_buffer, bytes_to_copy);
			out_offset += bytes_decoded;

			if (out_offset >= max_bytes) {
				break;
			}
		} else {
			break;
		}
	}

	return(total_bytes_decoded);
}

int decode_mp3_init_fifo(uint8_t* input_buffer, size_t input_buffer_size) {
	memset(&frame_info, 0, sizeof(frame_info));

	FifoSize   = input_buffer_size;
	FifoAddr   = input_buffer;
	mp3io.read = read_cb;
	mp3io.seek = seek_cb;
	mp3io.read_data = mp3io.seek_data = input_buffer;

	int ret = mp3dec_ex_open_cb(&mp3d, &mp3io, MP3D_DO_NOT_SCAN);
	if (ret) {
		printf("mp3dec_ex_open_cb failed: %d\n", ret);
	}

	return(ret);
}

int decode_mp3_init(uint8_t* input_buffer, size_t input_buffer_size) {
	memset(&frame_info, 0, sizeof(frame_info));

	// sets up input_buffer as mp3d->file.buffer
	int ret = mp3dec_ex_open_buf(&mp3d, input_buffer, input_buffer_size, MP3D_DO_NOT_SCAN);
	if (ret) {
		printf("mp3dec_ex_open_buf failed: %d\n", ret);
	}
	return(ret);
}

int mp3_get_hz() {
	return(mp3d.info.hz);
}

int mp3_get_channels() {
	return(mp3d.info.channels);
}
