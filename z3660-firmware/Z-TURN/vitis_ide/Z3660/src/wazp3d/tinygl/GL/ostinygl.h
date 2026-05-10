
#ifndef _tgl_osbuffer_h_
#define _tgl_osbuffer_h_
#ifdef __cplusplus
extern "C" {
#endif

/* struct type */
typedef struct ostgl_context {
	void *zb;					/* private ZBuffer struct */
	void *pixels;				/* read-write pixel buffer */
	unsigned char *palette;		/* 768 bytes of RGB color data if depth == 8 */
	int width, height;			/* width, height */
	int depth;					/* bpp */
} ostgl_context_t;

/* create context. valid depth values are 8, 16, 24, and 32 */
ostgl_context_t *ostgl_create_context(int width, int height, int depth);

/* if you have multiple contexts, call this to make one "current" */
void ostgl_make_current(ostgl_context_t *context);

/* delete context and free all associated memory */
void ostgl_delete_context(ostgl_context_t *context);

/* resize framebuffer to specified with and height */
void ostgl_resize(ostgl_context_t *context, int width, int height);

/* convert internal framebuffer to depth specified by context->depth */
/* you only need to call this function if your target bits-per-pixel != 16 */
/* NOTE: this pointer does not need to be freed, as it just returns context->pixels */
void *ostgl_convert_framebuffer(ostgl_context_t *context);

#ifdef __cplusplus
}
#endif
#endif /* _tgl_osbuffer_h_ */
