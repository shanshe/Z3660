#include <stdio.h>
//#include "sleep.h"
#include <sys/_timeval.h>
#include "../memorymap.h"
#include <unistd.h>
int 	usleep (useconds_t __useconds);
void select(int a, int*b,int*c,int*d, struct timeval *tv)
{
	usleep(tv->tv_sec*1000000);
	usleep(tv->tv_usec);
}
#define MA_DR_FLAC_NO_STDIO
#define MA_DR_MP3_NO_STDIO
#define MA_DR_WAV_NO_STDIO
#define STB_VORBIS_NO_STDIO
#define MA_USE_STDINT
#define MA_NO_PTHREAD_IN_HEADER
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_NULL
#define MA_NO_ENCODING
#define MA_NO_DEVICE_IO
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE
#define MA_NO_THREADING
#define MA_NO_GENERATION
#define MA_NO_RUNTIME_LINKING
#define MA_HAS_VORBIS

#include "stb_vorbis.c"
#include "miniaudio.h"

#define SWAP16(a) a = __builtin_bswap16(a)
#define SWAP32(a) a = __builtin_bswap32(a)

ma_decoder decoder;
#define FIFOSIZE (1152*4*8)
typedef struct {
	uint8_t* FifoAddr;
	unsigned long FifoSize;
	unsigned long FifoWriteIdx; // in
	unsigned long FifoReadIdx; // out
} ma_Data;
static ma_Data ma_data={.FifoSize=FIFOSIZE,
                        .FifoReadIdx=0,
                        .FifoWriteIdx=0,
                        .FifoAddr=(uint8_t*)AUDIO_TX_BUFFER_ADDRESS};

ma_result ma_read_cb(ma_decoder* pDecoder, void* buf, size_t size, size_t* pBytesRead) {
	unsigned long BytesRead = 0;
	long BytesToRead = size;
	volatile unsigned char *src = pDecoder->pUserData;
	volatile unsigned char *dst = buf;
//	unsigned long origFifoReadIdx=FifoReadIdx;

	while(BytesToRead) {
		// If FiFo is empty then exit the loop.
		if(ma_data.FifoReadIdx == ma_data.FifoWriteIdx) break;
		dst[BytesRead++] = src[ma_data.FifoReadIdx++];
		if(ma_data.FifoReadIdx >= ma_data.FifoSize) ma_data.FifoReadIdx = 0;
		BytesToRead--;
	}
    *pBytesRead=BytesRead;
//	printf("BytesRead=%ld from %p to %p\n",BytesRead,src+origFifoReadIdx,dst);
	return(BytesRead);
}

ma_result ma_seek_cb(ma_decoder* pDecoder,  ma_int64 position, ma_seek_origin origin) {
	if(position>=ma_data.FifoSize)
		printf("position>=ma_data.FifoSize!!!!\n");
	if(origin==ma_seek_origin_start)
	{
		ma_data.FifoReadIdx = position % ma_data.FifoSize;
	}
	else if(origin==ma_seek_origin_current)
	{
//		printf("ma_seek_origin_current %ld %ld %ld\n",(int32_t)position,ma_data.FifoReadIdx,ma_data.FifoSize);
		ma_data.FifoReadIdx = (ma_data.FifoReadIdx + position) % ma_data.FifoSize;
	}
	else
	{
		printf("Not supported seek %d\n",origin);
	}
	return(MA_SUCCESS);
}

void ma_fifo_clear(void) {
	ma_data.FifoReadIdx  = 0;
	ma_data.FifoWriteIdx = 0;
}

void ma_fifo_set_write_index(unsigned long aWriteIndex) {
	if(aWriteIndex>=ma_data.FifoSize)
		printf("aWriteIndex>=ma_data.FifoSize %ld %ld!!!!\n",aWriteIndex,ma_data.FifoSize);
	ma_data.FifoWriteIdx = aWriteIndex % ma_data.FifoSize;
}

unsigned long ma_fifo_get_read_index(void) {
	return(ma_data.FifoReadIdx);
}

int decode_ma_samples(void* output_buffer, int max_samples) {
	size_t max_bytes = max_samples * 2;
	int out_offset = 0;
	int total_bytes_decoded = 0;
	ma_uint64 read_samples;

	// Clear destination buffer before trying to decode.
	memset(output_buffer, 0, max_bytes);
	while (1) {
ma_decoder_read_pcm_frames(&decoder, output_buffer+out_offset, max_samples>>1, &read_samples);
        read_samples=read_samples<<1;
		max_samples -= read_samples;

		int bytes_decoded = read_samples * sizeof(ma_int16);
		total_bytes_decoded += bytes_decoded;

		if (bytes_decoded > 0) {
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
int decode_ma_init_fifo(uint8_t* input_buffer, size_t input_buffer_size) {
	ma_data.FifoSize   = input_buffer_size;
	ma_data.FifoAddr   = input_buffer;
    ma_decoder_config config = ma_decoder_config_init(ma_format_s16, 2, 48000);
    config.encodingFormat = ma_encoding_format_unknown;
    int result;
    if((result = ma_decoder_init(ma_read_cb, ma_seek_cb, input_buffer, &config, &decoder)) != MA_SUCCESS) {
		printf("ma_init failed %d\n",result);
		return(0);
	} else {
		printf("ma_init OK\n");
	}
	return(1);
}

int decode_ma_init(uint8_t* input_buffer, size_t input_buffer_size) {
    ma_decoder_config config = ma_decoder_config_init(ma_format_s16, 2, 48000);
    config.encodingFormat = ma_encoding_format_unknown;
    int result;
    if ((result = ma_decoder_init_memory(input_buffer, input_buffer_size, &config, &decoder)) != MA_SUCCESS) {
		printf("ma_init_memory failed %d\n",result);
		return(0);
	} else {
		printf("ma_init_memory OK\n");
	}
	return(1);
}

int ma_get_hz() {
	return(decoder.outputSampleRate);
}

int ma_get_channels() {
	return(decoder.outputChannels);
}
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
