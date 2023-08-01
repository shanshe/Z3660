/*
 * Adapted by shanshe to work with Z3660
 */
/*
 * MNT ZZ9000AX Amiga MP3 Player Example (Hardware Accelerated)
 *
 * Copyright (C) 2022, Lukas F. Hartmann <lukas@mntre.com>
 *                     MNT Research GmbH, Berlin
 *                     https://mntre.com
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 */

#include <exec/exec.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <utility/hooks.h>
#include <hardware/intbits.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/expansion.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define DEVF_INT2MODE 1

static const char version[] = "$VER: axmp3 1.12\n\r";
static const char procname[] = "axmp3";

#define ZZ_BYTES_PER_PERIOD 3840
#define AUDIO_BUFSZ ZZ_BYTES_PER_PERIOD * 8 // TODO: query from hardware
#define WORKER_PRIORITY 127                 // 19 would be nicer

typedef enum
{
	DECODE_INIT,
	DECODE_RUN,
	DECODE_CLEAR_FIFO,
	DECODE_INIT_FIFO,
} DECODE_COMMAND;

#include "z3660_regs.h"

struct z9ax
{
    struct Task *t_mainproc;
    struct Process *worker_process;
    struct Interrupt irq;
    uint32_t hw_addr;
    int8_t mainproc_signal;
    int8_t enable_signal;
    int8_t worker_signal;

    uint32_t mp3_addr;
    size_t mp3_bytes;
    uint32_t encoded_offset;
    uint32_t decode_offset;
    uint32_t decode_chunk_sz;

    uint8_t flags;
} glob_ax;

size_t mp3_bytes_total = 0;
size_t mp3_bytes_read = 0;
FILE *mp3_file;

void WorkerProcess()
{
    struct Process *proc = (struct Process *)FindTask(NULL);
    struct z9ax *ax = proc->pr_Task.tc_UserData;

    ax->worker_signal = AllocSignal(-1);
    ax->enable_signal = AllocSignal(-1);

    uint32_t signals = 0;
    uint32_t buf_offset = 0;
    uint32_t buf_samples = ZZ_BYTES_PER_PERIOD / 4;

    Signal(ax->t_mainproc, 1L << ax->mainproc_signal);

    for (;;)
    {
        signals = Wait(SIGBREAKF_CTRL_C | (1L << ax->enable_signal));
        if (signals & SIGBREAKF_CTRL_C)
            break;

        *((volatile uint32_t *)(ax->hw_addr + REG_ZZ_AUDIO_SCALE)) = buf_samples;

        *((volatile uint32_t *)(ax->hw_addr + REG_ZZ_DECODER_PARAM)) = 2;
        *((volatile uint32_t *)(ax->hw_addr + REG_ZZ_DECODER_VAL)) = (ax->decode_offset + buf_offset);
        *((volatile uint32_t *)(ax->hw_addr + REG_ZZ_DECODER_PARAM)) = 0;
        // ZZ_DECODE (task)
        *((volatile uint32_t *)(ax->hw_addr + REG_ZZ_DECODE)) = DECODE_RUN;

        // play buffer
        *((volatile uint32_t *)(ax->hw_addr + REG_ZZ_AUDIO_SWAB)) = (1 << 15) | (buf_offset >> 8); // no byteswap, offset/256
        int overrun = *((volatile uint32_t *)(ax->hw_addr + REG_ZZ_AUDIO_SWAB));

        if (overrun == 1)
        {
            fprintf(stderr, "Overrun\n");
            buf_offset = 0;
        }
        else
        {
            buf_offset += ZZ_BYTES_PER_PERIOD;
            if (buf_offset >= AUDIO_BUFSZ)
            {
                buf_offset = 0;
            }
        }


        // finished playing?
        int decoded = *((volatile uint32_t *)(ax->hw_addr + REG_ZZ_DECODE));
        if (!decoded)
        {
            Signal((struct Task *)ax->t_mainproc, SIGBREAKF_CTRL_C);
            break;
        }
    }

    FreeSignal(ax->enable_signal);
    ax->enable_signal = -1;
    FreeSignal(ax->worker_signal);
    ax->worker_signal = -1;
    Signal((struct Task *)ax->t_mainproc, 1L << ax->mainproc_signal);
}

uint32_t dev_isr(struct z9ax *ax asm("a1"))
{
    uint32_t status = *(uint32_t *)(ax->hw_addr + REG_ZZ_INT_STATUS);

    // audio interrupt signal set?
    if (status & 2)
    {
        // ack/clear audio interrupt
        *(uint32_t *)(ax->hw_addr + REG_ZZ_CONFIG) = 8 | 32;

        if (ax->worker_process)
        {
            Signal((struct Task *)ax->worker_process, 1L << ax->enable_signal);
        }
    }

    if (status == 2)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void init_interrupt(struct z9ax *ax)
{
    ax->irq.is_Node.ln_Type = NT_INTERRUPT;
    ax->irq.is_Node.ln_Pri = -60;
    ax->irq.is_Node.ln_Name = "Z3660AX DIRECT";
    ax->irq.is_Data = ax;
    ax->irq.is_Code = (void *)dev_isr;

    Forbid();
    if (ax->flags & DEVF_INT2MODE)
    {
        AddIntServer(INTB_PORTS, &ax->irq);
    }
    else
    {
        AddIntServer(INTB_EXTER, &ax->irq);
    }
    Permit();

    // enable HW interrupt
    *(volatile uint32_t *)(ax->hw_addr + REG_ZZ_AUDIO_CONFIG) = 1;
}

void destroy_interrupt(struct z9ax *ax)
{
    // disable HW interrupt
    *(volatile uint32_t *)(ax->hw_addr + REG_ZZ_AUDIO_CONFIG) = 0;

    Forbid();
    if (ax->flags & DEVF_INT2MODE)
    {
        RemIntServer(INTB_PORTS, &ax->irq);
    }
    else
    {
        RemIntServer(INTB_EXTER, &ax->irq);
    }
    Permit();
}

void clean_up()
{
    fprintf(stderr, "Cleaning up.\n");
    Signal((struct Task *)glob_ax.worker_process, SIGBREAKF_CTRL_C);
    Wait(1L << glob_ax.mainproc_signal);
    destroy_interrupt(&glob_ax);
    FreeSignal(glob_ax.mainproc_signal);
}

int main(int argc, char *argv[])
{
    struct ConfigDev *cd;
    uint32_t hw_addr = 0;
    fprintf(stderr, "%s", &version[6]);
    if ((ExpansionBase = (struct ExpansionBase *)OpenLibrary((STRPTR) "expansion.library", 0)))
    {
        // Find Z3660
        if ((cd = (struct ConfigDev *)FindConfigDev(cd, 0x144B, 0x1)))
        {
            fprintf(stderr, "Z3660 detected.\n");
        }
        else
        {
            fprintf(stderr, "Error: Z3660 not detected.\n");
            exit(1);
        }
    }
    else
    {
        fprintf(stderr, "Error: Can't open expansion.library.\n");
        exit(1);
    }

    CloseLibrary((struct Library *)ExpansionBase);
    hw_addr = (uint32_t)cd->cd_BoardAddr;
    int ax_present = *((volatile uint32_t *)(hw_addr + REG_ZZ_AUDIO_CONFIG));
    if (ax_present)
    {
        fprintf(stderr, "Z3660AX detected.\n");
    }
    else
    {
        fprintf(stderr, "Error: Z3660AX not detected.\n");
        exit(1);
    }

    glob_ax.hw_addr = hw_addr;
    glob_ax.encoded_offset = 0x06000000;

    glob_ax.decode_offset = 0x07000000;
    glob_ax.decode_chunk_sz = 1920; // 16 bit sample pairs

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s soundfile.mp3\n", argv[0]);
        return RETURN_ERROR;
    }

    glob_ax.flags = 0;
    /*
      BPTR fh;
      if ((fh=Open((CONST_STRPTR)"ENV:ZZ9K_INT2",MODE_OLDFILE))) {
        printf("Using INT2 mode.\n");
        Close(fh);
        glob_ax.flags |= DEVF_INT2MODE;
      } else {
        printf("Using INT6 mode (default).\n");
      }
    */
    mp3_file = fopen(argv[1], "rb");
    if (!mp3_file)
    {
        fprintf(stderr, "Error opening input file.\n");
        return RETURN_ERROR;
    }

    fseek(mp3_file, 0L, SEEK_END);
    size_t sz = ftell(mp3_file);
    rewind(mp3_file);

    fprintf(stderr, "File size: %u bytes.\n", sz);

    glob_ax.mp3_addr = hw_addr + glob_ax.encoded_offset;
    glob_ax.mp3_bytes = sz;

    fprintf(stderr, "Loading the first 64k...\n");

    mp3_bytes_read = fread((void *)glob_ax.mp3_addr, 1, 64 * 1024*4, mp3_file);
    mp3_bytes_total = mp3_bytes_read;

    fprintf(stderr, "Playing...\n");

    // set tx buffer address to 127 MB offset
    *((volatile uint32_t *)(hw_addr + REG_ZZ_AUDIO_PARAM)) = 0;
    *((volatile uint32_t *)(hw_addr + REG_ZZ_AUDIO_VAL)) = glob_ax.decode_offset;

    // set LPF to 20KHz
    *((volatile uint32_t *)(hw_addr + REG_ZZ_AUDIO_PARAM)) = 9;
    *((volatile uint32_t *)(hw_addr + REG_ZZ_AUDIO_VAL)) = 20000;

    // set decoder params
    *((volatile uint32_t *)(hw_addr + REG_ZZ_DECODER_PARAM)) = 0;
    *((volatile uint32_t *)(hw_addr + REG_ZZ_DECODER_VAL)) = glob_ax.encoded_offset;
    *((volatile uint32_t *)(hw_addr + REG_ZZ_DECODER_PARAM)) = 1;
    *((volatile uint32_t *)(hw_addr + REG_ZZ_DECODER_VAL)) = glob_ax.mp3_bytes;
    *((volatile uint32_t *)(hw_addr + REG_ZZ_DECODER_PARAM)) = 2;
    *((volatile uint32_t *)(hw_addr + REG_ZZ_DECODER_VAL)) = glob_ax.decode_offset;
    *((volatile uint32_t *)(hw_addr + REG_ZZ_DECODER_PARAM)) = 3;
    *((volatile uint32_t *)(hw_addr + REG_ZZ_DECODER_VAL)) = glob_ax.decode_chunk_sz;

    // ZZ_DECODE (init)
    *((volatile uint32_t *)(hw_addr + REG_ZZ_DECODE)) = DECODE_INIT;

    glob_ax.t_mainproc = FindTask(NULL);
    glob_ax.mainproc_signal = AllocSignal(-1);

    if (glob_ax.mainproc_signal != -1)
    {
        Forbid();
        if (glob_ax.worker_process = CreateNewProcTags(NP_Entry, (uint32_t)&WorkerProcess,
                                                       NP_Name, (uint32_t)procname,
                                                       NP_Priority, WORKER_PRIORITY,
                                                       TAG_DONE))
        {
            glob_ax.worker_process->pr_Task.tc_UserData = &glob_ax;
        }
        Permit();

        if (glob_ax.worker_process)
        {
            Wait(1L << glob_ax.mainproc_signal);
            init_interrupt(&glob_ax);

            atexit(clean_up);

            // stream the rest of the file

            while (mp3_bytes_read > 0) {
                fprintf(stderr, "Streaming... [%d/%d]\n", mp3_bytes_total, glob_ax.mp3_bytes);
                mp3_bytes_read = fread((void*)glob_ax.mp3_addr+mp3_bytes_total, 1, 128*1024, mp3_file);
                mp3_bytes_total += mp3_bytes_read;
            }

            fprintf(stderr, "Done, waiting for Ctrl+C.\n");
            Wait(SIGBREAKF_CTRL_C);
        }
    }

    fclose(mp3_file);
    return RETURN_OK;
}
