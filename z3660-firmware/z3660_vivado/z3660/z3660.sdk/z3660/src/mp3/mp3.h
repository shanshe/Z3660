#ifndef _MP3_H_
#define _MP3_H_
typedef enum {
	DECODE_INIT,
	DECODE_RUN,
	DECODE_CLEAR_FIFO,
	DECODE_INIT_FIFO,
	DECODE_NUM_PAPARAMS
} DECODE_COMMAND;
static const char decode_command_str[DECODE_NUM_PAPARAMS][30]={
	"DECODE_INIT",
	"DECODE_RUN",
	"DECODE_CLEAR_FIFO",
	"DECODE_INIT_FIFO",
};

int decode_mp3_init(uint8_t* input_buffer, size_t input_buffer_size);
int decode_mp3_init_fifo(uint8_t* input_buffer, size_t input_buffer_size);
void fifo_clear(void);
void fifo_set_write_index(unsigned long aWriteIndex);
unsigned long fifo_get_read_index(void);
int decode_mp3_samples(void* output_buffer, int max_samples);
int mp3_get_hz();
int mp3_get_channels();
#endif //_MP3_H_
