/* Wazp3D - HARD3D using TinyGL (software OpenGL) */
/* Based on TinyGL - Software OpenGL renderer */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xil_types.h>
#include <stdint.h>
#include "../video.h"
#include "../debug_console.h"
#include "../memorymap.h"
#include "soft3d_opengl.h"
#include "Warp3D.h"
#include "Wazp3D.h"
#include "tinygl/GL/gl.h"
#include "tinygl/zbuffer.h"
#include "tinygl/zgl.h"

extern DEBUG_CONSOLE debug_console;

#define DEBUG_GL(fmt, ...) \
    do { \
        if(debug_console.debug_soft3d) \
            printf("[GL-Tiny] " fmt "\n", ##__VA_ARGS__); \
    } while(0)

/* Warp3D primitive types constants */
#define W3D_PRIMITIVE_TRIANGLES     0
#define W3D_PRIMITIVE_TRIFAN        1
#define W3D_PRIMITIVE_TRISTRIP      2
#define W3D_PRIMITIVE_POINTS        3
#define W3D_PRIMITIVE_LINES         4
#define W3D_PRIMITIVE_LINELOOP      5
#define W3D_PRIMITIVE_LINESTRIP     6
#define W3D_PRIMITIVE_POLYGON       9999  /* True polygon: for Gallium/GL wrapping */

/* Extended HARD3D context for TinyGL */
typedef struct {
    ZBuffer *zbuffer;           /* TinyGL Z-buffer */
    PIXEL *framebuffer;         /* Framebuffer in memory */
    int fb_width;
    int fb_height;
    int fb_bpp;
    uint8_t *image8;            /* Pointer to real framebuffer */
    
    /* Textures */
    PIXEL *texture_data;        /* Converted texture data */
    int texture_width;
    int texture_height;
    int texture_bpp;
    GLuint texture_id;
    
    /* State */
    int enable_depth_test;
    int enable_texture_2d;
    
    /* Clipping */
    int clip_xmin, clip_xmax;
    int clip_ymin, clip_ymax;
    
} TinyGLContext;

/* Global context */
static TinyGLContext g_tinygl_ctx = {0};

/* Convert Warp3D format to TinyGL */
static void convert_texture_4bit_to_rgba(const uint8_t *src, PIXEL *dst, int width, int height)
{
    /* 4-bit texture: each byte contains 2 pixels (nibbles) */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width / 2; x++) {
            uint8_t byte_val = src[y * (width / 2) + x];
            uint8_t pixel0 = (byte_val >> 4) & 0x0F;  /* high nibble */
            uint8_t pixel1 = byte_val & 0x0F;          /* low nibble */
            
            /* Convert 4-bit index to color (grayscale or palette) */
            /* For simplicity, use expanded grayscale */
            dst[y * width + x * 2 + 0] = RGB_TO_PIXEL(
                (pixel0 * 17) << 3,  /* 0-15 -> 0-255 */
                (pixel0 * 17) << 3,
                (pixel0 * 17) << 3
            );
            dst[y * width + x * 2 + 1] = RGB_TO_PIXEL(
                (pixel1 * 17) << 3,
                (pixel1 * 17) << 3,
                (pixel1 * 17) << 3
            );
        }
    }
}

static void convert_texture_8bit_to_rgba(const uint8_t *src, PIXEL *dst, int width, int height)
{
    /* 8-bit texture: each byte is an index */
    for (int i = 0; i < width * height; i++) {
        uint8_t index = src[i];
        dst[i] = RGB_TO_PIXEL(
            index << 3,  /* 0-255 */
            index << 3,
            index << 3
        );
    }
}

/*===========================================================================*/
/* HARD3D functions using TinyGL                                             */
/*===========================================================================*/

void HARD3D_Start(void *hc)
{
    (void)hc;
    DEBUG_GL("HARD3D_Start");
    
    /* Initialize TinyGL context */
    memset(&g_tinygl_ctx, 0, sizeof(g_tinygl_ctx));
    
    DEBUG_GL("TinyGL initialized (waiting for SetBitmap for ZB_open)");
}

void HARD3D_End(void *hc)
{
    (void)hc;
    DEBUG_GL("HARD3D_End");
    
    /* Free Z-buffer if exists */
    if (g_tinygl_ctx.zbuffer) {
        free(g_tinygl_ctx.zbuffer);
        g_tinygl_ctx.zbuffer = NULL;
    }
    
    /* Free texture if exists */
    if (g_tinygl_ctx.texture_data) {
        free(g_tinygl_ctx.texture_data);
        g_tinygl_ctx.texture_data = NULL;
    }
}

void HARD3D_SetClipping(void *hc, unsigned short xmin, unsigned short xmax,
                        unsigned short ymin, unsigned short ymax)
{
    (void)hc;
    DEBUG_GL("HARD3D_SetClipping: %d,%d - %d,%d", xmin, ymin, xmax, ymax);
    
    g_tinygl_ctx.clip_xmin = xmin;
    g_tinygl_ctx.clip_xmax = xmax;
    g_tinygl_ctx.clip_ymin = ymin;
    g_tinygl_ctx.clip_ymax = ymax;
    
    /* TinyGL doesn't have glScissor, implement manual clipping if needed */
    /* For now just store clipping values */
}

void HARD3D_SetBitmap(void *hc, void *bm, unsigned char *Image8,
                      unsigned long format, unsigned short x, unsigned short y,
                      unsigned short large, unsigned short high)
{
    (void)hc;
    (void)x;
    (void)y;
    (void)bm;
    
    DEBUG_GL("=== HARD3D_SetBitmap ===");
    DEBUG_GL("Dimensions: %dx%d, format=%lu", large, high, format);
    DEBUG_GL("Image8 (hardware framebuffer): 0x%08lx", (unsigned long)Image8);
    
    g_tinygl_ctx.fb_width = large;
    g_tinygl_ctx.fb_height = high;
    g_tinygl_ctx.image8 = Image8;
    
    /* Determine format */
    if (format == 5) {
        /* RGB565 - 16 bit */
        g_tinygl_ctx.fb_bpp = 16;
        DEBUG_GL("RGB565 16-bit format (ZB_MODE_5R6G5B)");
    } else {
        g_tinygl_ctx.fb_bpp = 32;
        DEBUG_GL("32-bit format");
    }
    
    /* IMPORTANT: TinyGL needs ZB_open before using OpenGL */
    /* The Image8 framebuffer is already in video memory */
    /* Initialize ZBuffer with existing framebuffer */
    if (!g_tinygl_ctx.zbuffer) {
        DEBUG_GL("Initializing ZB_open: %dx%d format=%d", large, high, ZB_MODE_5R6G5B);
        
        /* Create ZBuffer pointing to Image8 framebuffer */
        g_tinygl_ctx.zbuffer = ZB_open(large, high, ZB_MODE_5R6G5B, 
                                        0, NULL, NULL, (PIXEL *)Image8);
        
        if (!g_tinygl_ctx.zbuffer) {
            DEBUG_GL("ERROR: ZB_open failed!");
            return;
        }
        
        DEBUG_GL("ZB_open completed successfully");
        DEBUG_GL("  ZBuffer structure: 0x%08lx", (unsigned long)g_tinygl_ctx.zbuffer);
        DEBUG_GL("  ZBuffer->pbuf (where TinyGL writes): 0x%08lx", (unsigned long)g_tinygl_ctx.zbuffer->pbuf);
        DEBUG_GL("  ZBuffer->zbuf: 0x%08lx", (unsigned long)g_tinygl_ctx.zbuffer->zbuf);
        DEBUG_GL("  ZBuffer->xsize=%d, ysize=%d, linesize=%d", 
                 g_tinygl_ctx.zbuffer->xsize, g_tinygl_ctx.zbuffer->ysize, g_tinygl_ctx.zbuffer->linesize);
        
        /* Verify addresses match */
        if (g_tinygl_ctx.zbuffer->pbuf == (PIXEL *)Image8) {
            DEBUG_GL("CORRECT: TinyGL writes to hardware framebuffer");
        } else {
            DEBUG_GL("ERROR: Different addresses!");
            DEBUG_GL("  Image8: 0x%08lx", (unsigned long)Image8);
            DEBUG_GL("  pbuf:   0x%08lx", (unsigned long)g_tinygl_ctx.zbuffer->pbuf);
        }
        
        DEBUG_GL("Initializing glInit with ZBuffer");
        
        /* Initialize OpenGL with ZBuffer */
        glInit(g_tinygl_ctx.zbuffer);
        
        DEBUG_GL("glInit completed");
        
        /* Configure default OpenGL state */
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        
        /* ORTHOGRAPHIC PROJECTION: map screen coordinates to clip space */
        /* Wazp3D passes coordinates in pixels (0..width, 0..height) */
        /* OpenGL needs clip coordinates (-1..1) */
        /* glOrtho(left, right, bottom, top, near, far) */
        /* Amiga: Y=0 top, X=0 left */
        /* OpenGL: Y=0 bottom, X=0 left (but uses right-handed system) */
        /* Y-flip: high 0 so Y=0 is top (like Amiga) */
        /* X remains 0 large (no mirror) */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, (double)large, (double)high, 0.0, -1.0, 1.0);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        /* NOTE: DO NOT call glMatrixMode(GL_TEXTURE) + glLoadIdentity() here
         * because gl_matrix_update() sets matrix_model_projection_updated=0
         * when matrix_mode > 1, erasing the flag that glOrtho just set.
         * The texture matrix is already identity after glInit(). */
    } else {
        DEBUG_GL("ZBuffer already exists, reusing");
        /* ZBuffer already exists, check if framebuffer changed */
        if (g_tinygl_ctx.zbuffer->pbuf != (PIXEL *)Image8) {
            DEBUG_GL("Updating framebuffer pointer: 0x%08lx -> 0x%08lx",
                     (unsigned long)g_tinygl_ctx.zbuffer->pbuf, (unsigned long)Image8);
            g_tinygl_ctx.zbuffer->pbuf = (PIXEL *)Image8;
        }
    }
    
    /* Configure viewport */
    glViewport(0, 0, large, high);
    
    DEBUG_GL("Viewport configured: 0,0,%d,%d", large, high);
    DEBUG_GL("=== End HARD3D_SetBitmap ===");
}

void HARD3D_CreateTexture(void *hc, void *ht, unsigned char *pt,
                          unsigned short large, unsigned short high,
                          unsigned short format, unsigned char TexFlags)
{
    (void)hc;
    (void)ht;
    (void)TexFlags;
    
    /* IMPORTANT: The 'format' parameter is a Warp3D code, NOT bits per pixel */
    /* W3D_CHUNKY=1 (CLUT), W3D_R8G8B8=4 (RGB 24-bit), W3D_A8R8G8B8=6 (ARGB 32-bit) */
    /* W3D_R8G8B8A8=11 (RGBA 32-bit) */
    int bits;
    switch (format) {
        case 1:  bits = 8;  break;  /* W3D_CHUNKY - palettized 8-bit */
        case 4:  bits = 24; break;  /* W3D_R8G8B8 - RGB 24-bit */
        case 6:  bits = 32; break;  /* W3D_A8R8G8B8 - ARGB 32-bit */
        case 11: bits = 32; break;  /* W3D_R8G8B8A8 - RGBA 32-bit */
        default: bits = 24; break;  /* Assume RGB 24-bit */
    }
    
    DEBUG_GL("HARD3D_CreateTexture: %dx%d format=%d (bits=%d) ptr=0x%lx", 
             large, high, format, bits, (unsigned long)pt);
    
    /* Free previous texture if exists */
    if (g_tinygl_ctx.texture_data) {
        free(g_tinygl_ctx.texture_data);
        g_tinygl_ctx.texture_data = NULL;
    }
    
    /* TinyGL requires RGB texture with 3 bytes per pixel */
    /* Allocate memory for texture converted to RGB */
    g_tinygl_ctx.texture_data = (PIXEL *)malloc(large * high * 3);
    if (!g_tinygl_ctx.texture_data) {
        DEBUG_GL("ERROR: Could not allocate memory for texture");
        return;
    }
    
    g_tinygl_ctx.texture_width = large;
    g_tinygl_ctx.texture_height = high;
    g_tinygl_ctx.texture_bpp = bits;
    
    /* Convert texture to RGB format for TinyGL */
    if (bits == 24) {
        /* RGB 24-bit texture (W3D_R8G8B8): 3 bytes per pixel, big-endian */
        /* Data comes as R,G,B consecutive bytes */
        for (int i = 0; i < large * high; i++) {
            uint8_t *src = pt + i * 3;
            uint8_t *dst = (uint8_t *)g_tinygl_ctx.texture_data + i * 3;
            dst[0] = src[0];  /* R */
            dst[1] = src[1];  /* G */
            dst[2] = src[2];  /* B */
        }
    } else if (bits == 32) {
        /* ARGB 32-bit texture (W3D_A8R8G8B8): 4 bytes per pixel, big-endian */
        /* Memory layout big-endian: A, R, G, B */
        for (int i = 0; i < large * high; i++) {
            uint32_t *src = (uint32_t *)(pt + i * 4);
            uint32_t argb = __builtin_bswap32(*src);  /* BE -> LE */
//            uint8_t a = (argb >> 24) & 0xFF;
            uint8_t r = (argb >> 16) & 0xFF;
            uint8_t g = (argb >> 8) & 0xFF;
            uint8_t b = argb & 0xFF;
            uint8_t *dst = (uint8_t *)g_tinygl_ctx.texture_data + i * 3;
            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
        }
    } else if (bits == 8) {
        /* CLUT 8-bit texture: each byte is a palette index */
        /* TODO: would need the palette to convert correctly */
        /* For now, grayscale */
        for (int i = 0; i < large * high; i++) {
            uint8_t index = pt[i];
            uint8_t *dst = (uint8_t *)g_tinygl_ctx.texture_data + i * 3;
            dst[0] = index;
            dst[1] = index;
            dst[2] = index;
        }
    } else if (bits == 16) {
        /* 16-bit RGB565 texture: convert to RGB */
        uint16_t *src = (uint16_t *)pt;
        for (int i = 0; i < large * high; i++) {
//            uint16_t rgb565 = src[i];
            /* Convert from big-endian to little-endian if needed */
            uint16_t rgb565 = __builtin_bswap16(src[i]);
            uint8_t r = (rgb565 >> 11) & 0x1F;
            uint8_t g = (rgb565 >> 5) & 0x3F;
            uint8_t b = rgb565 & 0x1F;
            /* Expand to 8-bit */
            r = (r << 3) | (r >> 2);
            g = (g << 2) | (g >> 4);
            b = (b << 3) | (b >> 2);
            uint8_t *dst = (uint8_t *)g_tinygl_ctx.texture_data + i * 3;
            dst[0] = r;
            dst[1] = g;
            dst[2] = b;
        }
    }
    
    /* Generate OpenGL texture */
    glGenTextures(1, &g_tinygl_ctx.texture_id);
    glBindTexture(GL_TEXTURE_2D, g_tinygl_ctx.texture_id);
    
    /* IMPORTANT: TinyGL only supports very limited parameters */
    /* Only GL_TEXTURE_WRAP_S/T with GL_REPEAT and GL_TEXTURE_ENV_MODE with GL_DECAL */
    /* DO NOT use glTexParameteri with GL_TEXTURE_MIN_FILTER or GL_TEXTURE_MAG_FILTER */
    
    /* IMPORTANT: TinyGL only accepts specific parameters */
    /* target=GL_TEXTURE_2D, level=0, components=3, border=0, format=GL_RGB, type=GL_UNSIGNED_BYTE */
    glTexImage2D(GL_TEXTURE_2D, 0, 3, large, high, 0, 
                 GL_RGB, GL_UNSIGNED_BYTE, g_tinygl_ctx.texture_data);
    
    /* Enable textures */
    glEnable(GL_TEXTURE_2D);
    g_tinygl_ctx.enable_texture_2d = 1;
    
    DEBUG_GL("TinyGL texture created ID=%d", g_tinygl_ctx.texture_id);
}

void HARD3D_UpdateTexture(void *hc, void *ht, unsigned char *pt,
                          unsigned short large, unsigned short high,
                          unsigned char bits)
{
    (void)hc;
    (void)ht;
    
    DEBUG_GL("HARD3D_UpdateTexture: %dx%d bits=%d", large, high, bits);
    
    if (!g_tinygl_ctx.texture_data) {
        HARD3D_CreateTexture(hc, ht, pt, large, high, bits, 0);
        return;
    }
    
    g_tinygl_ctx.texture_width = large;
    g_tinygl_ctx.texture_height = high;
    g_tinygl_ctx.texture_bpp = bits;
    
    /* Update texture data to RGB format */
    if (bits == 4) {
        for (int y = 0; y < high; y++) {
            for (int x = 0; x < large / 2; x++) {
                uint8_t byte_val = pt[y * (large / 2) + x];
                uint8_t pixel0 = (byte_val >> 4) & 0x0F;
                uint8_t pixel1 = byte_val & 0x0F;
                
                uint8_t r0 = pixel0 * 17;
                uint8_t g0 = pixel0 * 17;
                uint8_t b0 = pixel0 * 17;
                uint8_t r1 = pixel1 * 17;
                uint8_t g1 = pixel1 * 17;
                uint8_t b1 = pixel1 * 17;
                
                uint8_t *dst0 = (uint8_t *)g_tinygl_ctx.texture_data + (y * large + x * 2) * 3;
                uint8_t *dst1 = (uint8_t *)g_tinygl_ctx.texture_data + (y * large + x * 2 + 1) * 3;
                dst0[0] = r0; dst0[1] = g0; dst0[2] = b0;
                dst1[0] = r1; dst1[1] = g1; dst1[2] = b1;
            }
        }
    } else if (bits == 8) {
        for (int i = 0; i < large * high; i++) {
            uint8_t index = pt[i];
            uint8_t *dst = (uint8_t *)g_tinygl_ctx.texture_data + i * 3;
            dst[0] = index;
            dst[1] = index;
            dst[2] = index;
        }
    } else if (bits == 16) {
        uint16_t *src = (uint16_t *)pt;
        for (int i = 0; i < large * high; i++) {
            uint16_t rgb565 = src[i];
            uint8_t r = (rgb565 >> 11) & 0x1F;
            uint8_t g = (rgb565 >> 5) & 0x3F;
            uint8_t b = rgb565 & 0x1F;
            r = (r << 3) | (r >> 2);
            g = (g << 2) | (g >> 4);
            b = (b << 3) | (b >> 2);
            uint8_t *dst = (uint8_t *)g_tinygl_ctx.texture_data + i * 3;
            dst[0] = r;
            dst[1] = g;
            dst[2] = b;
        }
    }
    
    /* TinyGL doesn't have glTexSubImage2D, recreate complete texture */
    glBindTexture(GL_TEXTURE_2D, g_tinygl_ctx.texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, large, high, 0, 
                 GL_RGB, GL_UNSIGNED_BYTE, g_tinygl_ctx.texture_data);
}

void HARD3D_FreeTexture(void *hc, void *ht)
{
    (void)hc;
    (void)ht;
    
    DEBUG_GL("HARD3D_FreeTexture");
    
    if (g_tinygl_ctx.texture_id != 0) {
        glDeleteTextures(1, &g_tinygl_ctx.texture_id);
        g_tinygl_ctx.texture_id = 0;
    }
    
    if (g_tinygl_ctx.texture_data) {
        free(g_tinygl_ctx.texture_data);
        g_tinygl_ctx.texture_data = NULL;
    }
    
    g_tinygl_ctx.enable_texture_2d = 0;
    glDisable(GL_TEXTURE_2D);
}

void HARD3D_DrawPrimitive(void *hc, void *P, unsigned long Pnb, unsigned long primitive)
{
    (void)hc;
    
    DEBUG_GL("=== HARD3D_DrawPrimitive ===");
    DEBUG_GL("Vertices: %lu, Warp3D type: %lu", Pnb, primitive);
    
    if (Pnb == 0 || P == NULL) {
        DEBUG_GL("No vertices, exiting");
        return;
    }
    
    /* Data comes in big-endian format (Amiga m68k) */
    /* struct point3D in Z3660: { float x,y,z; float u,v; float w; union RGBA; uint32_t padding; } */
    /* COPYP_NEON copies 32 bytes (2 NEON vectors), confirming sizeof=32 */
    /* Layout: x(0) y(4) z(8) u(12) v(16) w(20) RGBA(24) padding(28) = 32 bytes */
    
    typedef struct {
        uint32_t x, y, z;
        uint32_t u, v;
        uint32_t w;
        uint32_t rgba;
        uint32_t padding;
    } point3D_be_t;  /* 32 bytes - matches COPYP_NEON */
    
    point3D_be_t *vertices_be = (point3D_be_t *)P;
    
    /* Helper function to convert big-endian uint32 to float */
    #define BE_TO_FLOAT(val) ({ \
        uint32_t _tmp = __builtin_bswap32(val); \
        float _result; \
        memcpy(&_result, &_tmp, sizeof(float)); \
        _result; \
    })
    
    /* Debug: show first vertices 
    if (Pnb >= 1) {
        float x0 = BE_TO_FLOAT(vertices_be[0].x);
        float y0 = BE_TO_FLOAT(vertices_be[0].y);
        float z0 = BE_TO_FLOAT(vertices_be[0].z);
        float u0 = BE_TO_FLOAT(vertices_be[0].u);
        float v0 = BE_TO_FLOAT(vertices_be[0].v);
        float w0 = BE_TO_FLOAT(vertices_be[0].w);
        // Color in ARGB big-endian format: needs byte-swap
        // In BE memory: FF 16 1F 25 = A=0xFF, R=0x16, G=0x1F, B=0x25
        // ARM reads as LE: 0x251F16FF -> needs swap
        uint32_t argb = __builtin_bswap32(vertices_be[0].rgba);
        uint8_t a = (argb >> 24) & 0xFF;
        uint8_t r = (argb >> 16) & 0xFF;
        uint8_t g = (argb >> 8) & 0xFF;
        uint8_t b = argb & 0xFF;
        DEBUG_GL("Vertex 0: x=%.1f, y=%.1f, z=%.3f", x0, y0, z0);
        DEBUG_GL("  u=%.3f, v=%.3f, w=%.3f", u0, v0, w0);
        DEBUG_GL("  ARGB=0x%08lx (A=%d, R=%d, G=%d, B=%d)", 
                 (unsigned long)argb, a, r, g, b);
    }
*/
    /* 
     * PROBLEM: TinyGL uses stride in number of floats, NOT in bytes like standard OpenGL.
     * In glopArrayElement: i = idx * (size + stride)
     * 
     * Solution: Create intermediate buffers with converted data (BE->LE + RGBA???float4)
     * and stride=0 (tightly packed).
     * 
     * Output format per vertex:
     *   vertex: 3 floats (x,y,z)
     *   texcoord: 3 floats (u,v,w)  
     *   color: 4 floats (r,g,b,a) normalized 0.0-1.0
     */
    
    /* Allocate temporary buffers */
    /* vertex: 3 floats per vertex */
    /* texcoord: 3 floats per vertex (u,v,w) */
    /* color: 4 floats per vertex (r,g,b,a) */
    size_t vert_buf_size = Pnb * 3 * sizeof(float);
    size_t tex_buf_size = Pnb * 3 * sizeof(float);
    size_t col_buf_size = Pnb * 4 * sizeof(float);
    
    float *vert_buf = (float *)malloc(vert_buf_size);
    float *tex_buf = (float *)malloc(tex_buf_size);
    float *col_buf = (float *)malloc(col_buf_size);
    
    if (!vert_buf || !tex_buf || !col_buf) {
        DEBUG_GL("ERROR: Could not allocate temporary buffers");
        free(vert_buf);
        free(tex_buf);
        free(col_buf);
        return;
    }
    
    /* Convert vertices from big-endian to little-endian float */
    for (unsigned long i = 0; i < Pnb; i++) {
        /* Vertex: x, y, z */
        vert_buf[i * 3 + 0] = BE_TO_FLOAT(vertices_be[i].x);
        vert_buf[i * 3 + 1] = BE_TO_FLOAT(vertices_be[i].y);
        vert_buf[i * 3 + 2] = BE_TO_FLOAT(vertices_be[i].z);
        
        /* TexCoord: u, v, w */
        tex_buf[i * 3 + 0] = BE_TO_FLOAT(vertices_be[i].u);
        tex_buf[i * 3 + 1] = BE_TO_FLOAT(vertices_be[i].v);
        tex_buf[i * 3 + 2] = BE_TO_FLOAT(vertices_be[i].w);
        
        /* Color: ARGB big-endian uint32 -> byte-swap -> 4 floats RGBA */
        /* In BE memory: FF 16 1F 25 -> ARM reads 0x251F16FF -> swap -> 0xFF161F25 */
        uint32_t argb = __builtin_bswap32(vertices_be[i].rgba);
        uint8_t a = (argb >> 24) & 0xFF;
        uint8_t r = (argb >> 16) & 0xFF;
        uint8_t g = (argb >>  8) & 0xFF;
        uint8_t b = (argb      ) & 0xFF;
        col_buf[i * 4 + 0] = (float)a / 255.0f;  /* R */
        col_buf[i * 4 + 1] = (float)r / 255.0f;  /* G */
        col_buf[i * 4 + 2] = (float)g / 255.0f;  /* B */
        col_buf[i * 4 + 3] = (float)b / 255.0f;  /* A */
    }
    
    #undef BE_TO_FLOAT
    
    /* Convert Warp3D primitive to OpenGL */
    GLenum gl_primitive;
    switch (primitive) {
        case W3D_PRIMITIVE_TRIANGLES:
            gl_primitive = GL_TRIANGLES;
            break;
        case W3D_PRIMITIVE_TRIFAN:
            gl_primitive = GL_TRIANGLE_FAN;
            break;
        case W3D_PRIMITIVE_TRISTRIP:
            gl_primitive = GL_TRIANGLE_STRIP;
            break;
        case W3D_PRIMITIVE_POINTS:
            gl_primitive = GL_POINTS;
            break;
        case W3D_PRIMITIVE_LINES:
            gl_primitive = GL_LINES;
            break;
        case W3D_PRIMITIVE_LINELOOP:
            gl_primitive = GL_LINE_LOOP;
            break;
        case W3D_PRIMITIVE_LINESTRIP:
            gl_primitive = GL_LINE_STRIP;
            break;
        case W3D_PRIMITIVE_POLYGON:
            gl_primitive = GL_POLYGON;
            break;
        default:
            gl_primitive = GL_TRIANGLES;
            break;
    }
    
    DEBUG_GL("GL primitive: 0x%04x", gl_primitive);
    
    /* TEST: Draw a hardcoded triangle to verify TinyGL works
    if (0)  // Change to 1 to activate test
    {
        DEBUG_GL("TEST: Drawing hardcoded red triangle...");
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisable(GL_TEXTURE_2D);
        
        // First: test direct framebuffer writing
        if (g_tinygl_ctx.zbuffer && g_tinygl_ctx.zbuffer->pbuf) {
            PIXEL *fb = g_tinygl_ctx.zbuffer->pbuf;
            int w = g_tinygl_ctx.zbuffer->xsize;
            DEBUG_GL("TEST: Writing directly to FB addr=0x%08lx linesize=%d",
                     (unsigned long)fb, g_tinygl_ctx.zbuffer->linesize);
            // Write a red pixel (RGB565) directly
            fb[100 * w + 200] = 0xF800;  // Pure red in RGB565
            fb[100 * w + 201] = 0x07E0;  // Pure green in RGB565
            fb[100 * w + 202] = 0x001F;  // Pure blue in RGB565
            fb[100 * w + 203] = 0xFFFF;  // White in RGB565
            DEBUG_GL("TEST: Direct write: pixels=%04x %04x %04x %04x",
                     fb[100*w+200], fb[100*w+201], fb[100*w+202], fb[100*w+203]);
        }
        
        // Second: test with TinyGL glBegin/glEnd
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);  // Red
        glVertex3f(100.0f, 100.0f, 0.0f);
        glColor4f(0.0f, 1.0F, 0.0f, 1.0f);  // Green
        glVertex3f(540.0f, 100.0f, 0.0f);
        glColor4f(0.0f, 0.0f, 1.0f, 1.0f);  // Blue
        glVertex3f(320.0f, 380.0f, 0.0f);
        glEnd();
        
        // Verify pixels after test
        if (g_tinygl_ctx.zbuffer && g_tinygl_ctx.zbuffer->pbuf) {
            PIXEL *fb = g_tinygl_ctx.zbuffer->pbuf;
            int w = g_tinygl_ctx.zbuffer->xsize;
            int h = g_tinygl_ctx.zbuffer->ysize;
            int nonzero = 0;
            int total = w * h;
            for (int i = 0; i < total; i++) {
                if (fb[i] != 0) nonzero++;
            }
            DEBUG_GL("TEST result: %d/%d non-zero pixels (%.2f%%)",
                     nonzero, total, (float)nonzero * 100.0f / total);
            // Verify directly written pixels
            DEBUG_GL("TEST direct: %04x %04x %04x %04x",
                     fb[100*w+200], fb[100*w+201], fb[100*w+202], fb[100*w+203]);
            // Sample screen center where triangle should be
            int cy = 200;
            DEBUG_GL("TEST pixels row %d: %04x %04x %04x %04x %04x %04x %04x %04x",
                     cy, fb[cy*w+310], fb[cy*w+311], fb[cy*w+312], fb[cy*w+313],
                     fb[cy*w+314], fb[cy*w+315], fb[cy*w+316], fb[cy*w+317]);
        }
    }
*/
    /* Now draw the real vertices using glBegin/glEnd (more reliable than arrays) */
    {
        /* Note: Texture state was already configured by HARD3D_SetDrawState */
        /* Just disable arrays to be safe */
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        
        glBegin(gl_primitive);
        for (unsigned long i = 0; i < Pnb; i++) {
            float x = vert_buf[i * 3 + 0];
            float y = vert_buf[i * 3 + 1];
            float z = vert_buf[i * 3 + 2];
            float r = col_buf[i * 4 + 0];
            float g = col_buf[i * 4 + 1];
            float b = col_buf[i * 4 + 2];
            float a = col_buf[i * 4 + 3];
            float u = tex_buf[i * 3 + 0];
            float v = tex_buf[i * 3 + 1];
            
            glColor4f(r, g, b, a);
            glTexCoord2f(u, v);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
    
    DEBUG_GL("Drawing completed (glBegin/glEnd)");
    
    /* DEBUG: Verify if TinyGL wrote pixels to framebuffer
    if (g_tinygl_ctx.zbuffer && g_tinygl_ctx.zbuffer->pbuf) {
        PIXEL *fb = g_tinygl_ctx.zbuffer->pbuf;
        int w = g_tinygl_ctx.zbuffer->xsize;
        int h = g_tinygl_ctx.zbuffer->ysize;
        int nonzero = 0;
        int total = w * h;
        
        for (int i = 0; i < total; i++) {
            if (fb[i] != 0) nonzero++;
        }
        
        DEBUG_GL("FB: %dx%d, non-zero: %d/%d (%.2f%%)",
                 w, h, nonzero, total, (float)nonzero * 100.0f / total);
    }
*/
    /* Free temporary buffers */
    free(vert_buf);
    free(tex_buf);
    free(col_buf);
    
    DEBUG_GL("=== End HARD3D_DrawPrimitive ===");
}

void HARD3D_Flush(void *hc)
{
    (void)hc;
    DEBUG_GL("HARD3D_Flush");
    glFlush();
}

void HARD3D_SetDrawFunctions(void *hc)
{
    /* IMPORTANT: This function is called from SOFT3D_SetDrawState before DrawPrimitive */
    /* We get the state from HC context, same as in original soft3d */
    struct HARD3D_context *HC = (struct HARD3D_context *)hc;
    if (HC && HC->state) {
        struct state3D *state = HC->state;
        
        DEBUG_GL("=== HARD3D_SetDrawFunctions ===");
        DEBUG_GL("UseTex=%d, TexEnvMode=%d, BlendMode=%d, UseGouraud=%d",
                 state->UseTex, state->TexEnvMode, state->BlendMode, state->UseGouraud);
        
        /* Configure textures */
        if (state->UseTex && g_tinygl_ctx.texture_id != 0) {
            glEnable(GL_TEXTURE_2D);
            g_tinygl_ctx.enable_texture_2d = 1;
            
            /* IMPORTANT: TinyGL ONLY supports GL_DECAL in glTexEnv */
            /* We ignore Warp3D's TexEnvMode and always use GL_DECAL */
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
            DEBUG_GL("Texture enabled (TinyGL only supports GL_DECAL)");
        } else {
            glDisable(GL_TEXTURE_2D);
            g_tinygl_ctx.enable_texture_2d = 0;
            DEBUG_GL("Texture disabled");
        }
        
        /* NOTE: TinyGL doesn't implement real blending (glBlendFunc is empty stub) */
        /* Blending is handled in original software rasterizer, not in TinyGL */
        /* For now, ignore BlendMode - transparencies won't work */
        if (state->BlendMode != 0) {
            int srcBlend = state->BlendMode / 16;
            int dstBlend = state->BlendMode % 16;
            DEBUG_GL("Blending requested: src=%d, dst=%d (NOT SUPPORTED in TinyGL)",
                     srcBlend, dstBlend);
        }
    } else {
        DEBUG_GL("HARD3D_SetDrawFunctions: HC or state is NULL");
    }
}

void HARD3D_WriteZSpan(void *hc, unsigned short x, unsigned short y,
                       unsigned long n, double *dz, unsigned char *mask)
{
    (void)hc;
    (void)x;
    (void)y;
    (void)n;
    (void)dz;
    (void)mask;
    /* TinyGL handles Z-buffer automatically */
}

void HARD3D_ReadZSpan(void *hc, unsigned short x, unsigned short y,
                      unsigned long n, double *dz)
{
    (void)hc;
    (void)x;
    (void)y;
    (void)n;
    (void)dz;
    /* TinyGL handles Z-buffer automatically */
}

void HARD3D_ClearZBuffer(void *hc, float fz)
{
    (void)hc;
    DEBUG_GL("HARD3D_ClearZBuffer: %f", fz);
    
    glClearDepth(fz);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void HARD3D_AllocZbuffer(void *hc, unsigned short large, unsigned short high)
{
    (void)hc;
    DEBUG_GL("HARD3D_AllocZbuffer: %dx%d", large, high);
    
    /* TinyGL handles Z-buffer internally */
    glEnable(GL_DEPTH_TEST);
    g_tinygl_ctx.enable_depth_test = 1;
}

void HARD3D_AllocImageBuffer(void *hc, unsigned short large, unsigned short high)
{
    (void)hc;
    DEBUG_GL("HARD3D_AllocImageBuffer: %dx%d", large, high);
    
    g_tinygl_ctx.fb_width = large;
    g_tinygl_ctx.fb_height = high;
}

void HARD3D_ClearImageBuffer(void *hc, unsigned short x, unsigned short y,
                             unsigned short large, unsigned short high, void *rgba)
{
    (void)hc;
    (void)x;
    (void)y;
    (void)large;
    (void)high;
    (void)rgba;
    
    DEBUG_GL("HARD3D_ClearImageBuffer");
    
    /* TinyGL doesn't define GLbitfield, use int */
    int mask = GL_COLOR_BUFFER_BIT;
    if (g_tinygl_ctx.enable_depth_test) {
        mask |= GL_DEPTH_BUFFER_BIT;
    }
    glClear(mask);
}

void HARD3D_DoUpdate(void *hc)
{
    (void)hc;
    DEBUG_GL("HARD3D_DoUpdate");
    glFlush();
    
    /* In embedded system, ensure framebuffer updates */
    /* TinyGL writes directly to pbuf, verify address */
    if (g_tinygl_ctx.zbuffer && g_tinygl_ctx.zbuffer->pbuf) {
        unsigned long fb_addr = (unsigned long)g_tinygl_ctx.zbuffer->pbuf;
        DEBUG_GL("[GL-Tiny] TinyGL framebuffer pbuf: 0x%08lx", fb_addr);
        /* Note: Video hardware must read from this address */
    }
}

void HARD3D_Current(void *hc)
{
    (void)hc;
    DEBUG_GL("HARD3D_Current");
    /* TinyGL uses global context */
}

const char *HARD3D_GetVersion(void *hc)
{
    (void)hc;
    static char version_str[64];
    sprintf(version_str, "1.0.0 (Wazp3D TinyGL v%d.%d.%d)", 
            __TINYGL_VERSION_MAJOR__, 
            __TINYGL_VERSION_MINOR__, 
            __TINYGL_VERSION_PATCH__);
    return version_str;
}

/*===========================================================================*/
/* Additional functions for compatibility                                    */
/*===========================================================================*/

void *GL_CreateContext(int width, int height)
{
    (void)width;
    (void)height;
    DEBUG_GL("GL_CreateContext: %dx%d", width, height);
    return &g_tinygl_ctx;
}

void GL_DestroyContext(void *ctx)
{
    (void)ctx;
    DEBUG_GL("GL_DestroyContext");
}

void GL_MatrixMode(unsigned int mode)
{
    glMatrixMode(mode);
}

void GL_LoadIdentity(void)
{
    glLoadIdentity();
}

void GL_LoadMatrix(const float *m)
{
    glLoadMatrixf(m);
}

void GL_Translate(float x, float y, float z)
{
    glTranslatef(x, y, z);
}

void GL_Rotate(float angle, float x, float y, float z)
{
    glRotatef(angle, x, y, z);
}

void GL_Scale(float x, float y, float z)
{
    glScalef(x, y, z);
}

void GL_Perspective(float fovy, float aspect, float zNear, float zFar)
{
    /* TinyGL doesn't have gluPerspective, use glFrustum */
    float f = 1.0f / tanf(fovy * 3.14159265358979323846f / 360.0f);
    float proj[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (zFar + zNear) / (zNear - zFar), -1,
        0, 0, (2.0f * zFar * zNear) / (zNear - zFar), 0
    };
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj);
    glMatrixMode(GL_MODELVIEW);
}

void GL_Viewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void GL_Enable(unsigned int cap)
{
    glEnable(cap);
}

void GL_Disable(unsigned int cap)
{
    glDisable(cap);
}

void GL_TexParameter(unsigned int target, unsigned int pname, int param)
{
    /* TinyGL doesn't support glTexParameteri - ignore */
    (void)target;
    (void)pname;
    (void)param;
}