/*
 * pl_mpeg_player.h
 *
 *  Created on: 4 nov. 2022
 *      Author: shanshe
 */

#ifndef SRC_MPG_PL_MPEG_PLAYER_H_
#define SRC_MPG_PL_MPEG_PLAYER_H_

#include <inttypes.h>
#include "ff.h"

extern FIL fil;
#define XFILE FIL
FIL* Xopen(const char *filename, const char *mode);
FRESULT	Xseek(FIL *, size_t, int);
size_t Xtell(FIL *);

#define Xclose(F) f_close(F)

size_t Xread(_PTR P, size_t L, size_t B, FIL *F);

void player_mpeg(FIL *fh,char *filename);
typedef void AudioCallback(void* userdata, uint8_t* stream, uint32_t len);
typedef struct {
	int freq;
//	SDL_Audioformat format;
	uint8_t channels;
	uint16_t samples;
	uint32_t size;
	AudioCallback *callback;
	void *data;
} AudioSpec;

#include "../video.h"

extern ZZ_VIDEO_STATE vs;

#endif /* SRC_MPG_PL_MPEG_PLAYER_H_ */
