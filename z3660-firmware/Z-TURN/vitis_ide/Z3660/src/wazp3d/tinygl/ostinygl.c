#include "GL/ostinygl.h"
#include "zbuffer.h"
#include "zgl.h"
#include "GL/gl.h"
#include <stdlib.h>
#include <assert.h>

#define NUM_COLORS 256

#define RED(c) ((unsigned char)(((c) >> 16) & 0xFF))
#define GRN(c) ((unsigned char)(((c) >> 8) & 0xFF))
#define BLU(c) ((unsigned char)((c) & 0xFF))

static int num_contexts = 0;

/* create offscreen tinygl context with provided dimensions and depth */
ostgl_context_t *ostgl_create_context(int width, int height, int depth)
{
	/* variables */
	ostgl_context_t *context;
	int i;
	unsigned char color_indices[NUM_COLORS];
	int color_table[NUM_COLORS];

	/* check parameter validitiy */
	assert(width && height);
	assert(depth == 8 || depth == 16 || depth == 24 || depth == 32);

	/* allocate context */
	context = gl_malloc(sizeof(ostgl_context_t));
	assert(context != NULL);

	/* allocate pixels */
	context->pixels = gl_malloc(width * height * (depth / 8));
	assert(context->pixels != NULL);

	/* palette is null unless 8bpp */
	context->palette = NULL;

	/* open zbuffer */
	switch (depth)
	{
		case 8:
		{
			for (i = 0; i < NUM_COLORS; i++) color_indices[i] = i;
			context->zb = (void *)ZB_open(width, height, ZB_MODE_INDEX, NUM_COLORS, color_indices, color_table, NULL);
			/*
			context->palette = (void *)((ZBuffer *)context->zb)->ctable;
			for (i = 0; i < NUM_COLORS; i++) printf("0x%08x ", ((ZBuffer *)context->zb)->ctable[i]);
			printf("\n");
			*/
			context->palette = gl_malloc(NUM_COLORS * 3);
			for (i = 0; i < NUM_COLORS; i++)
			{
				context->palette[i * 3] = RED(((ZBuffer *)context->zb)->ctable[i]);
				context->palette[(i * 3) + 1] = GRN(((ZBuffer *)context->zb)->ctable[i]);
				context->palette[(i * 3) + 2] = BLU(((ZBuffer *)context->zb)->ctable[i]);
				/* printf("%u %u %u\n", context->palette[i * 3], context->palette[(i * 3) + 1], context->palette[(i * 3) + 2]); */
			}
			break;
		}

		case 16:
		{
			context->zb = (void *)ZB_open(width, height, ZB_MODE_5R6G5B, 0, NULL, NULL, context->pixels);
			break;
		}

		case 24:
		{
			context->zb = (void *)ZB_open(width, height, ZB_MODE_RGB24, 0, NULL, NULL, NULL);
			break;
		}

		case 32:
		{
			context->zb = (void *)ZB_open(width, height, ZB_MODE_RGBA, 0, NULL, NULL, NULL);
			break;
		}

		default:
		{
			break;
		}
	}

	/* check zb */
	assert(context->zb != NULL);

	/* init gl */
	if (++num_contexts == 1)
		glInit((ZBuffer *)context->zb);

	/* init values */
	context->width = width;
	context->height = height;
	context->depth = depth;

	/* return ptr */
	return context;
}

/* make current */
void ostgl_make_current(ostgl_context_t *context)
{
	GLContext *glcontext = gl_get_context();
	assert(glcontext != NULL);
	glcontext->zb = (ZBuffer *)context->zb;
}

/* delete zbuffers and free all memory */
void ostgl_delete_context(ostgl_context_t *context)
{
	if (context)
	{
		if (context->zb) ZB_close((ZBuffer *)context->zb);
		if (context->pixels) gl_free(context->pixels);
		if (context->palette) gl_free(context->palette);

		gl_free(context);

		if (--num_contexts == 0)
			glClose();
	}
}

/* resize all zbuffers in context */
void ostgl_resize(ostgl_context_t *context, int width, int height)
{
	assert(width && height);

	context->width = width;
	context->height = height;
	context->pixels = realloc(context->pixels, width * height * (context->depth / 8));

	if (context->depth == 16)
		ZB_resize((ZBuffer *)context->zb, context->pixels, width, height);
	else
		ZB_resize((ZBuffer *)context->zb, NULL, width, height);
}

/* prepare frame buffer (if depth != 16) */
void *ostgl_convert_framebuffer(ostgl_context_t *context)
{
	if (context->depth == 16)
		return context->pixels;

	/* convert to 8bpp, 24bpp or 32bpp and copy */
	ZB_copyFrameBuffer((ZBuffer *)context->zb, context->pixels, context->width * (context->depth / 8));

	/* return ptr */
	return context->pixels;
}
