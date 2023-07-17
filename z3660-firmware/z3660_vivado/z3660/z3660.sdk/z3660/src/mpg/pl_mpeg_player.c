#include <stdlib.h>
#include <stdio.h>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

#include "pl_mpeg_player.h"
#include "ff.h"
#include "../rtg/fonts.h"

#define APP_TEXTURE_MODE_YCRCB 1
#define APP_TEXTURE_MODE_RGB 2
FIL fil;

typedef struct {
	plm_t *plm;
	double last_time;
	int wants_to_quit;
	uint8_t texture_mode;

	uint8_t *rgb_data;
} app_t;

app_t App_t;
FIL* Xopen(const char *filename, const char *mode)
{
	f_open(&fil, filename , FA_OPEN_ALWAYS | FA_READ);
	return(&fil);
}
size_t Xread(_PTR P, size_t L, size_t B, FIL *F)
{
	UINT br;
	f_read(F,P,(L)*(B),&br);
	return(br);
}
FRESULT	Xseek(FIL *fh, size_t offset, int M)
{
	if(M==SEEK_END)
		offset+=fh->obj.objsize;
	return(f_lseek(fh,offset));
}
size_t Xtell(FIL *fp)
{
	return(fp->fptr);
}


app_t * app_create(XFILE *fh,const char *filename, int texture_mode);
void app_update(app_t *self);
void app_destroy(app_t *self);

void app_on_video(plm_t *player, plm_frame_t *frame, void *user);
void app_on_audio(plm_t *player, plm_samples_t *samples, void *user);

app_t * app_create(XFILE *fh,const char *filename, int texture_mode) {
	app_t *self = &App_t;

	memset(self, 0, sizeof(app_t));

//	self->texture_mode = texture_mode;

	// Initialize plmpeg, load the video file, install decode callbacks
//	self->plm = plm_create_with_filename(filename);
	self->plm = plm_create_with_file(fh, 1);
	if (!self->plm) {
		printf("Couldn't open %s\n", filename);
		return(NULL);
	}

	int samplerate = plm_get_samplerate(self->plm);

	printf(
		"Opened %s - framerate: %f, samplerate: %d, duration: %f\n",
		filename,
		plm_get_framerate(self->plm),
		plm_get_samplerate(self->plm),
		plm_get_duration(self->plm)
	);

	plm_set_video_decode_callback(self->plm, app_on_video, self);
	plm_set_audio_decode_callback(self->plm, app_on_audio, self);

	plm_set_loop(self->plm, TRUE);
	plm_set_audio_enabled(self->plm, TRUE);
	plm_set_audio_stream(self->plm, 0);

	if (plm_get_num_audio_streams(self->plm) > 0) {
		// Initialize SDL Audio
		AudioSpec audio_spec;
//		SDL_memset(&audio_spec, 0, sizeof(audio_spec));
		audio_spec.freq = samplerate;
//		audio_spec.format = AUDIO_F32;
		audio_spec.channels = 2;
		audio_spec.samples = 4096;

//		self->audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
//		if (self->audio_device == 0) {
//			printf("Failed to open audio device: %s\n", SDL_GetError());
//		}
//		SDL_PauseAudioDevice(self->audio_device, 0);

		// Adjust the audio lead time according to the audio_spec buffer size
		plm_set_audio_lead_time(self->plm, (double)audio_spec.samples / (double)samplerate);
	}

	// Create SDL Window
//	self->window = SDL_CreateWindow(
//		"pl_mpeg",
//		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
//		plm_get_width(self->plm), plm_get_height(self->plm),
//		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
//	);
//	self->gl = SDL_GL_CreateContext(self->window);

//	SDL_GL_SetSwapInterval(1);

	self->rgb_data = (uint8_t *)vs.framebuffer;

	return(self);
}

void app_destroy(app_t *self) {
	plm_destroy(self->plm);

	free(self);
}
uint32_t ticks=0;
uint32_t getTicks(void)
{
	return(ticks);
}
void app_update(app_t *self) {
	double seek_to = -1;

//	SDL_Event ev;
//	while (SDL_PollEvent(&ev)) {
//		if (
//			ev.type == SDL_QUIT ||
//			(ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_ESCAPE)
//		) {
//			self->wants_to_quit = TRUE;
//		}
//
//		if (
//			ev.type == SDL_WINDOWEVENT &&
//			ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED
//		) {
//			glViewport(0, 0, ev.window.data1, ev.window.data2);
//		}
//
//		// Seek 3sec forward/backward using arrow keys
//		if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_RIGHT) {
//			seek_to = plm_get_time(self->plm) + 3;
//		}
//		else if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_LEFT) {
//			seek_to = plm_get_time(self->plm) - 3;
//		}
//	}

	// Compute the delta time since the last app_update(), limit max step to
	// 1/30th of a second
	double current_time = (double)getTicks() / 1000.0;
	double elapsed_time = current_time - self->last_time;
	if (elapsed_time > 1.0 / 30.0) {
		elapsed_time = 1.0 / 30.0;
	}
	self->last_time = current_time;

	// Seek using mouse position
//	int mouse_x, mouse_y;
//	if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
//		int sx, sy;
//		SDL_GetWindowSize(self->window, &sx, &sy);
//		seek_to = plm_get_duration(self->plm) * ((float)mouse_x / (float)sx);
//	}

	// Seek or advance decode
	if (seek_to != -1) {
//		SDL_ClearQueuedAudio(self->audio_device);
		plm_seek(self->plm, seek_to, FALSE);
	}
	else {
		plm_decode(self->plm, elapsed_time);
	}

	if (plm_has_ended(self->plm)) {
		self->wants_to_quit = TRUE;
	}

//	glClear(GL_COLOR_BUFFER_BIT);
//	glRectf(0.0, 0.0, 1.0, 1.0);
//	SDL_GL_SwapWindow(self->window);
}

// YCbCr conversion following the BT.601 standard:
// https://infogalactic.com/info/YCbCr#ITU-R_BT.601_conversion

inline static void plm_put_pixel(int y, int r, int g, int b, uint32_t *dest)
{
	uint32_t bgra=0;
	bgra  = plm_clamp(y + b);
	bgra |= plm_clamp(y - g)<<8;
	bgra |= plm_clamp(y + r)<<16;
	*dest=bgra;
}

void plm_frame_to_bgra_shanshe(plm_frame_t *frame, uint8_t *dest) {
	int cols = frame->width >> 1;
	int rows = frame->height >> 1;
	int yw = frame->y.width;
	int cw = frame->cb.width;
	for (int row = 0; row < rows; row++) {
		int c_index = row * cw;
		int y_index = row * 2 * yw;
		for (int col = 0; col < cols; col++) {
			int cr = frame->cr.data[c_index] - 128;
			int cb = frame->cb.data[c_index] - 128;
			int r = (cr * 104597) >> 16;
			int g = (cb * 25674 + cr * 53278) >> 16;
			int b = (cb * 132201) >> 16;
			int y1 = ((frame->y.data[y_index + 0]-16)*76309)>>16;
			int y2 = ((frame->y.data[y_index + 1]-16)*76309)>>16;
			int y3 = ((frame->y.data[y_index + yw]-16)*76309)>>16;
			int y4 = ((frame->y.data[y_index + yw+1]-16)*76309)>>16;
			uint32_t *dest_index=(uint32_t* )dest + y_index;
			plm_put_pixel(y1,r,g,b,dest_index+0);
			plm_put_pixel(y2,r,g,b,dest_index+1);
			plm_put_pixel(y3,r,g,b,dest_index+yw);
			plm_put_pixel(y4,r,g,b,dest_index+yw+1);
			c_index += 1;
			y_index += 2;
		}
	}
}

uint32_t frame_count=0;

void app_on_video(plm_t *mpeg, plm_frame_t *frame, void *user) {
	app_t *self = (app_t *)user;

	// Hand the decoded data over to OpenGL. For the RGB texture mode, the
	// YCrCb->RGB conversion is done on the CPU.
#define RENDER_YCBCR
#ifdef RENDER_YCBCR
	plm_frame_to_bgra_shanshe(frame, self->rgb_data);
#endif
	frame_count++;
	static int last_int_time=0;
	static float fps=0;
	int int_time=self->last_time;
	if(int_time!=last_int_time)
	{
		last_int_time=int_time;
		fps=frame_count/self->last_time;
		Font20.BackColor=CL_BLACK;
		Font20.TextColor=CL_GREY;
	}
	char str[20];
	sprintf(str,"fps=%2.2f",fps);
	displayStringAt(10,10,(uint8_t*)str,LEFT_MODE);
}

void app_on_audio(plm_t *mpeg, plm_samples_t *samples, void *user) {
//	app_t *self = (app_t *)user;

	// Hand the decoded samples over to SDL

//	int size = sizeof(float) * samples->count * 2;
//	SDL_QueueAudio(self->audio_device, samples->interleaved, size);
}



void player_mpeg(FIL *fh,char *filename)
{
	app_t *app = app_create(fh,filename, APP_TEXTURE_MODE_RGB);
	if(app!=NULL)
	{
		while (!app->wants_to_quit) {
			app_update(app);
		}
	}
	app_destroy(app);
}
