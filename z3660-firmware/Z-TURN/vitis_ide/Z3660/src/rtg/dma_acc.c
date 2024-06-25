#include <stdio.h>
#include "gfx.h"
#include <xil_types.h>

#include "../debug_console.h"
#include "compression/compression.h"
#include "../main.h"

#define inline

extern unsigned int cur_mem_offset;
extern uint8_t imc_tables_initialized;
int current_c37_encoder = -1;
extern DEBUG_CONSOLE debug_console;
#define Z3_OUTPUT_ADDR 0x3400000

void handle_acc_op(uint16_t zdata)
{
    struct GFXData *data = (struct GFXData*)((uint32_t)Z3_SCRATCH_ADDR);
    //int cf_bpp[MNTVA_COLOR_NUM] = { 1, 2, 4, -8, 2, };
    if(debug_console.debug_rtg)
 	   printf("acc_op 0x%X\n",zdata);

    switch (zdata) {
        // SURFACE BLIT OPS
        case ACC_OP_NONE: {
            SWAP32(data->offset[0]);
            SWAP32(data->offset[1]);

            printf ("%s: %ld - %ld\n", data->clut2, data->offset[0], data->offset[1]);
            break;
        }
        case ACC_OP_BUFFER_CLEAR: {
            SWAP16(data->x[0]);
            SWAP16(data->y[0]);

            SWAP16(data->pitch[0]);
            SWAP32(data->offset[0]);
            data->offset[0] += ADDR_ADJ;

            acc_clear_buffer(data->offset[0], data->x[0], data->y[0], data->pitch[0], data->rgb[0], data->u8_user[GFXDATA_U8_COLORMODE]);
            break;
        }
        case ACC_OP_BUFFER_FLIP:
            SWAP16(data->x[0]);
            SWAP16(data->y[0]);

            SWAP16(data->pitch[0]);
            SWAP32(data->offset[0]);
            SWAP32(data->offset[1]);
            data->offset[0] += ADDR_ADJ;
            data->offset[1] += ADDR_ADJ;

            acc_flip_to_fb(data->offset[0], data->offset[1], data->x[0], data->y[0], data->pitch[0], data->u8_user[GFXDATA_U8_COLORMODE]);
            break;
        case ACC_OP_BLIT_RECT:
            SWAP16(data->x[0]); SWAP16(data->y[0]);
            SWAP16(data->x[1]); SWAP16(data->y[1]);

            SWAP16(data->pitch[0]);
            SWAP16(data->pitch[1]);
            SWAP32(data->offset[0]);
            SWAP32(data->offset[1]);
            data->offset[0] += ADDR_ADJ;
            data->offset[1] += ADDR_ADJ;

            //printf("BLAB: %p\n", (void *)data->offset[0]);
            if (data->u8_user[0] != data->u8_user[1]) {
                if (data->u8_user[0] == 2 && data->u8_user[1] == 1) {
                    acc_blit_rect_16to8(data->offset[0], data->offset[1], data->x[0], data->y[0], data->x[1], data->y[1], data->pitch[0], data->pitch[1]);
                    break;
                }
                else
                    printf ("Unimplemented color conversion %d to %d\n", data->u8_user[0], data->u8_user[1]);
            }
            acc_blit_rect(data->offset[0], data->offset[1], data->x[0], data->y[0], data->x[1] * data->u8_user[0], data->y[1], data->pitch[0], data->pitch[1], data->u8_user[2], data->u8offset);
            break;
        // PRIMITIVE OPS
        case ACC_OP_DRAW_CIRCLE:
        case ACC_OP_FILL_CIRCLE:
            SWAP16(data->x[0]); SWAP16(data->y[0]);
            SWAP16(data->x[1]); SWAP16(data->y[1]);
            SWAP16(data->x[2]); SWAP16(data->y[2]);

            SWAP32(data->offset[0]);
            SWAP16(data->pitch[0]);
            data->offset[0] += ADDR_ADJ;

            if (zdata == ACC_OP_DRAW_CIRCLE)
                acc_draw_circle(data->offset[0], data->pitch[0], data->x[0], data->y[0], data->x[2], data->x[1], data->y[1], data->rgb[0], data->u8_user[0]);
            else
                acc_fill_circle(data->offset[0], data->pitch[0], data->x[0], data->y[0], data->x[2], data->x[1], data->y[1], data->rgb[0], data->u8_user[0]);
            break;
        case ACC_OP_DRAW_LINE:
            SWAP16(data->x[0]); SWAP16(data->y[0]);
            SWAP16(data->x[1]); SWAP16(data->y[1]);

            SWAP32(data->offset[0]);
            SWAP16(data->pitch[0]);
            data->offset[0] += ADDR_ADJ;

            //printf("Drawing line from %d,%d to %d,%d...\n", data->x[0], data->y[0], data->x[1], data->y[1]);
            acc_draw_line(data->offset[0], data->pitch[0], data->x[0], data->y[0], data->x[1], data->y[1], data->rgb[0], data->u8_user[0], data->u8_user[1], data->u8_user[2]);
            break;
        case ACC_OP_FILL_RECT:
            SWAP16(data->x[0]); SWAP16(data->y[0]);
            SWAP16(data->x[1]); SWAP16(data->y[1]);

            SWAP32(data->offset[0]);
            SWAP16(data->pitch[0]);
            data->offset[0] += ADDR_ADJ;

            //printf("Filling rect at %d,%d to %d,%d...\n", data->x[0], data->y[0], data->x[0] + data->x[1], data->y[0] + data->y[1]);
            acc_fill_rect(data->offset[0], data->pitch[0], data->x[0], data->y[0], data->x[1], data->y[1], data->rgb[0], data->u8_user[0]);
            break;
        case ACC_OP_DRAW_FLAT_TRI: {
            TriangleDef tridef;
            memset(&tridef, 0x00, sizeof(TriangleDef));
            uint32_t *pts_ptr = (uint32_t *)data->clut4;

            SWAP16(data->x[0]); SWAP16(data->y[0]);

            SWAP32(data->offset[0]);
            SWAP16(data->pitch[0]);
            data->offset[0] += ADDR_ADJ;
            SWAP32(pts_ptr[0]);
            SWAP32(pts_ptr[1]);
            SWAP32(pts_ptr[2]);
            SWAP32(pts_ptr[3]);
            SWAP32(pts_ptr[4]);
            SWAP32(pts_ptr[5]);
            SWAP32(pts_ptr[6]);

            tridef.a[0] = (pts_ptr[0] << 16);
            tridef.a[1] = pts_ptr[1];
            tridef.b[0] = (pts_ptr[2] << 16);
            tridef.b[1] = pts_ptr[3];
            tridef.c[0] = (pts_ptr[4] << 16);
            tridef.c[1] = pts_ptr[5];

            acc_fill_flat_tri(data->offset[0], &tridef, data->x[0], data->y[0], data->rgb[0], data->u8_user[0]);
            break;
        }
        // ALLOC/DATA OPS
        case ACC_OP_ALLOC_SURFACE: {
            unsigned int sfc_size = 0;
            data->offset[0] = 0;
            if (data->u8_user[1] == 1) {
                SWAP32(data->offset[1]);
                sfc_size = data->offset[1];
            }
            else {
                SWAP16(data->x[0]); SWAP16(data->y[0]);
                data->offset[0] = 0;
                sfc_size = ((data->x[0] * data->u8_user[0]) * data->y[0]);

            }

            unsigned int barf = sfc_size % 256;
            if (barf)
                sfc_size += (256 - barf);

            if (data->u8_user[1] == 1) {
                printf ("Alloc requested for %ld bytes.\n", data->offset[1]);
            }
            else {
                printf ("Alloc requested for %dx%d surface, %.2X bytes per pixel, %d bytes.\n", data->x[0], data->y[0], data->u8_user[0], sfc_size);
            }
            if (!sfc_size) {
                printf("Refusing to allocate 0 bytes for you.\n");
                break;
            }

            //uint8_t *p = malloc(sfc_size);
            //memset(p, 0x00, sfc_size);
            //allocated_surfaces++;
            //printf ("Surface allocated at offset %.8X, or %.8X on the Amiga side.\n", cur_mem_offset, cur_mem_offset - ADDR_ADJ);

            data->offset[0] = cur_mem_offset - ADDR_ADJ;
            memset((void *)cur_mem_offset, 0x00, sfc_size);
            cur_mem_offset += sfc_size;
            SWAP32(data->offset[0]);
            break;
        }
        case ACC_OP_FREE_SURFACE: {
            SWAP32(data->offset[0]);
            data->offset[0] += ADDR_ADJ;
            void *ape = (void*)data->offset[0];
            if (data->u8_user[0]) {
                printf("[%s] Freeing surface at %p... Not really.\n", data->clut2, ape);
            }
            //else
                //printf("Freeing surface at %p... Not really.\n", ape);
            data->offset[0] = 0;

            //free(ape);
            //printf(" freed!\n");
            break;
        }
        case ACC_OP_SET_BPP_CONVERSION_TABLE: {
            // TODO:
            // Add some thing to select table based on source and dest bpp.
            // Requires the destination 8bpp palette to be in R3G3B2 format to look "correct" out of the box.
            SWAP32(data->offset[0]);
            data->offset[0] += ADDR_ADJ;

            printf("Setting color conversion table...\n");
            memcpy(get_color_conversion_table(0), (void*)data->offset[0], 65536);
            break;
        }
        // COMPRESSION/DECOMPRESSION OPS
        case ACC_OP_DECOMPRESS:
            SWAP16(data->x[0]); SWAP16(data->y[0]);
            SWAP16(data->x[1]); SWAP16(data->y[1]);

            SWAP32(data->offset[0]);
            data->offset[0] += ADDR_ADJ;
            data->offset[0] &= 0x0FFFFFFF;
            SWAP16(data->pitch[0]);
            SWAP32(data->u32_user[0]);

            switch(data->u8_user[0]) {
                case ACC_CMPTYPE_SMUSH_CODEC1: {
                    uint32_t dest_offset = data->x[0] + (data->pitch[0] * data->y[0]);
                    decompress_rle_smush1_data((uint8_t *)data->clut4, (uint8_t *)data->offset[0] + dest_offset, data->u32_user[0], data->x[1], data->y[1], data->pitch[0]);
                    break;
                }
                case ACC_CMPTYPE_SMUSH_CODEC37: {
                    //XTime tim1, tim2;
                    //XTime_GetTime(&tim1);
                    uint32_t dest_offset = data->x[0] + (data->pitch[0] * data->y[0]);
                    Codec37Decoder_decode(Codec37Decoder_GetCur(), (uint8_t *)data->offset[0] + dest_offset, (uint8_t *)data->clut4);
                    //XTime_GetTime(&tim2);
                    //printf("c37 frame size: %d bytes", data->u32_user[0]);
                    //printf("c37 frame decode time: %f ms\n", ((float)(tim2 - tim1) / (float)COUNTS_PER_SECOND) * 1000.0f);
                    break;
                }
                case ACC_CMPTYPE_SMUSH_CODEC47: {
                    //XTime tim1, tim2;
                    //XTime_GetTime(&tim1);
                    uint32_t dest_offset = data->x[0] + (data->pitch[0] * data->y[0]);
                    Codec47Decoder_decode(Codec47Decoder_GetCur(), (uint8_t *)data->offset[0] + dest_offset, (uint8_t *)data->clut4);
                    //XTime_GetTime(&tim2);
                    //printf("c47 frame size: %d bytes", data->u32_user[0]);
                    //printf("c47 frame decode time: %f ms\n", ((float)(tim2 - tim1) / (float)COUNTS_PER_SECOND) * 1000.0f);
                    break;
                }
                case ACC_CMPTYPE_IMA_ADPCM_VBR: {
                    if (!imc_tables_initialized) {
                        init_imc_tables();
                        imc_tables_initialized = 1;
                    }
                    decompress_adpcm((uint8_t *)data->clut4, (uint8_t *)data->offset[0], data->u8_user[1]);
                    break;
                }
            }
            break;
        case ACC_OP_COMPRESS:
            break;
        case ACC_OP_CODEC_OP:
            switch(data->u8_user[0]) {
                case ACC_CMPTYPE_SMUSH_CODEC37:
                    if (data->u8_user[1] == 1) {
                        SWAP16(data->x[0]);
                        SWAP16(data->y[0]);
                        Codec37Decoder_Init(Codec37Decoder_GetCur(), data->x[0], data->y[0]);
                        printf("Initializing codec37 decoder %d: %dx%d\n", Codec37Decoder_GetCur(), data->x[0], data->y[0]);
                        data->u8_user[2] = Codec37Decoder_GetCur() + 1;
                    }
                    else {
                        printf("Switching to next codec37 decoder.\n");
                        Codec37Decoder_Next();
                    }
                    break;
                case ACC_CMPTYPE_SMUSH_CODEC47:
                    if (data->u8_user[1] == 1) {
                        SWAP16(data->x[0]);
                        SWAP16(data->y[0]);
                        Codec47Decoder_Init(Codec47Decoder_GetCur(), data->x[0], data->y[0]);
                        printf("Initializing codec47 decoder %d: %dx%d\n", Codec47Decoder_GetCur(), data->x[0], data->y[0]);
                        data->u8_user[2] = Codec47Decoder_GetCur() + 1;
                    }
                    else {
                        printf("Switching to next codec47 decoder.\n");
                        Codec47Decoder_Next();
                    }
                    break;
            }
            break;
        default:
            break;
    }
}
