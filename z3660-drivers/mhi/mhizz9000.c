/*
 * MNT ZZ9000AX Amiga MHI driver (Hardware Accelerated)
 *
 * Copyright (C) 2022, Thomas Wenzel
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

#include <libraries/mhi.h>

#include <clib/debug_protos.h>
#include <clib/alib_protos.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/expansion.h>

#include <hardware/intbits.h>

#include "mhizz9000.h"
#include "ax.h"

#define OPTIMIZED_TRANSFER

// Comment out to enable debug output:
#define KPrintF(...)

#define DEVF_INT2MODE 1

#define ZZ_BYTES_PER_PERIOD 3840
#define AUDIO_BUFSZ ZZ_BYTES_PER_PERIOD*8 // TODO: query from hardware

#define REG_ZZ_CONFIG        0x04
#define REG_ZZ_AUDIO_SWAB    0x70
#define REG_ZZ_DECODER_FIFO  0x72
#define REG_ZZ_AUDIO_SCALE   0x74
#define REG_ZZ_AUDIO_PARAM   0x76
#define REG_ZZ_AUDIO_VAL     0x78
#define REG_ZZ_DECODER_PARAM 0x7A
#define REG_ZZ_DECODER_VAL   0x7C
#define REG_ZZ_DECODE        0x7E
#define REG_ZZ_AUDIO_CONFIG  0xF4

#define ID3V2_HEADER_LENGTH 10

#define FIFOSIZE (1152*4)
//#define FIFOSIZE (16*1024+1)

typedef enum {
	DECODE_CLEAR,
	DECODE_INIT,
	DECODE_RUN
} DECODE_COMMAND;

#define BSWAP_S(x) ((UWORD) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#define BSWAP_L(x) (((((ULONG)x) & 0xff000000u) >> 24) | ((((ULONG)x) & 0x00ff0000u) >> 8) | ((((ULONG)x) & 0x0000ff00u) << 8) | ((((ULONG)x) & 0x000000ffu) << 24))
#define BSWAP_P(x) (void*)(((((ULONG)x) & 0xff000000u) >> 24) | ((((ULONG)x) & 0x00ff0000u) >> 8) | ((((ULONG)x) & 0x0000ff00u) << 8) | ((((ULONG)x) & 0x000000ffu) << 24))


/* ******************************** */
/*  BEGIN ZZ9000AX parameter access */
/*  Don't worry!                    */
/*  The compiler inlines these!     */
/* ******************************** */
static void setRegister(struct MhiPlayer *mp, ULONG Register, UWORD Value) {
	*((volatile UWORD*)(mp->hw_addr+Register)) = Value;
}

static UWORD getRegister(struct MhiPlayer *mp, ULONG Register) {
	return *((volatile UWORD*)(mp->hw_addr+Register));
}

static void setAudioParam(struct MhiPlayer *mp, ULONG Param, UWORD Value) {
	*((volatile UWORD*)(mp->hw_addr+REG_ZZ_AUDIO_PARAM)) = Param;
	*((volatile UWORD*)(mp->hw_addr+REG_ZZ_AUDIO_VAL))	 = Value;
	*((volatile UWORD*)(mp->hw_addr+REG_ZZ_AUDIO_PARAM)) = 0;	
}

static void setDecoderParam(struct MhiPlayer *mp, ULONG Param, UWORD Value) {
	*((volatile UWORD*)(mp->hw_addr+REG_ZZ_DECODER_PARAM)) = Param;
	*((volatile UWORD*)(mp->hw_addr+REG_ZZ_DECODER_VAL))   = Value;
}
/* ****************************** */
/*  END ZZ9000AX parameter access */
/* ****************************** */

/* ************ */
/*  BEGIN FIFO  */
/* ************ */
// Clear FIFO on both sides.
static void clearFifo(struct MhiPlayer *mp) {
	mp->FifoMode = FIFO_PREFILL;
	mp->FifoWriteIdx = 0;
	// ZZ_DECODE (clear)
	setRegister(mp, REG_ZZ_DECODE, DECODE_CLEAR);
}

static void fillFifo(struct MhiPlayer *mp) {
	UBYTE *Buffer = (UBYTE *)mp->mp3_addr;
	LONG Space = 0;
	UWORD FifoReadIdx;
	struct ListNode *BufferNode;
	LONG i;

	// 1. Get FIFO Read Index from ZZ9k (we are the slave).
	FifoReadIdx = getRegister(mp, REG_ZZ_DECODER_FIFO);
	if(mp->FifoWriteIdx >= FifoReadIdx) {
		Space = FIFOSIZE-(mp->FifoWriteIdx-FifoReadIdx);
	}
	else {
		Space = FifoReadIdx-mp->FifoWriteIdx;
	}

	// 2. Calculate space left in FIFO.
	// In prefill mode fill the FIFO completely.
	if(mp->FifoMode == FIFO_PREFILL) {
		Space -= 1; // Note: Fill level limited for technical rasons.
	}
	// In operational mode fill it only half way to leave data for seeking back.
	else {
		Space -= FIFOSIZE/2;
	}
	if(Space <= 0) return;

	// 3. Fill the FIFO
	// Find first node in list that has not been completely played.
	for(BufferNode = (struct ListNode *)mp->BufferList->mlh_Head; BufferNode->Header.mln_Succ; BufferNode = (struct ListNode *)BufferNode->Header.mln_Succ) {
		if(BufferNode->Played == FALSE) {
			LONG BytesToCopy = BufferNode->Size - BufferNode->Index;
			if(BytesToCopy > Space) BytesToCopy = Space;

			#ifdef OPTIMIZED_TRANSFER
			// 3.1 Copy single bytes until we reach a 32-bit aligned destination address.
			if(BytesToCopy >= 3) {
				for(i=0; i<3; i++) {
					if((mp->FifoWriteIdx & 3) == 0) break;
					if(Space) {
						Buffer[mp->FifoWriteIdx++] = BufferNode->Buffer[BufferNode->Index++];
						if(mp->FifoWriteIdx >= FIFOSIZE) mp->FifoWriteIdx = 0;
						Space--;
						BytesToCopy--;
					}
				}
			}

			// 3.2 Optimized longword copy routine.
			LONG LongsToCopy = BytesToCopy/4;
			ULONG *src = (ULONG*)&BufferNode->Buffer[BufferNode->Index];
			ULONG *dst = (ULONG*)&Buffer[mp->FifoWriteIdx];
			for(i=0; i<LongsToCopy; i++) {
				*dst++ = *src++;
				mp->FifoWriteIdx  += 4;
				if(mp->FifoWriteIdx >= FIFOSIZE) {
					mp->FifoWriteIdx = 0;
					dst = (ULONG*)Buffer;
				}
			}
			Space             -= 4*LongsToCopy;
			BytesToCopy       -= 4*LongsToCopy;
			BufferNode->Index += 4*LongsToCopy;

			// 3.3 Copy remainder.
			#endif
			for(i=0; i<BytesToCopy; i++) {
				if(Space) {
					Buffer[mp->FifoWriteIdx++] = BufferNode->Buffer[BufferNode->Index++];
					if(mp->FifoWriteIdx >= FIFOSIZE) mp->FifoWriteIdx = 0;
					Space--;
				}
			}

			// If we have reached the end of the current buffer then...
			if(BufferNode->Index >= BufferNode->Size) {
				// ... mark this buffer as 'played'.
				BufferNode->Played = TRUE;	
				// ... signal the calling task that a buffer has been played.
				Signal(mp->MhiTask, mp->MhiMask);
			}
			break;
		}
	}

	mp->FifoMode = FIFO_OPERATIONAL;

	// 4. Set FIFO Write Index in ZZ9k (we are the master).
	setRegister(mp, REG_ZZ_DECODER_FIFO, mp->FifoWriteIdx);
}
/* ********** */
/*  END FIFO  */
/* ********** */

BOOL UserLibInit(struct MHI_LibBase *MhiLibBase) {
	struct ConfigDev* cd;
	ULONG hw_addr = 0;
	ULONG hw_size = 0;
	int ax_present;

	MhiLibBase->zorro_version = 0;
	if((ExpansionBase = (struct ExpansionBase*) OpenLibrary((STRPTR)"expansion.library", 0))) {
		// Find Z2 or Z3 model of MNT ZZ9000
		if((cd = (struct ConfigDev*)FindConfigDev(cd, 0x6D6E, 0x4))) {
			// ZORRO 3
			MhiLibBase->zorro_version = 3;
			KPrintF("ZZ9000 Zorro 3 Version detected.\n");
		}
		else if((cd = (struct ConfigDev*)FindConfigDev(cd, 0x6D6E, 0x3))) {
			// ZORRO 2
			MhiLibBase->zorro_version = 2;
			KPrintF("ZZ9000 Zorro 2 Version detected.\n");
		}
	} else {
		KPrintF("Error: Can't open expansion.library.\n");
		return FALSE;		
	}
	CloseLibrary((struct Library*)ExpansionBase);

	if(MhiLibBase->zorro_version == 0) {
		KPrintF("Error: ZZ9000 not detected.\n");
		return FALSE;		
	}

	hw_addr = (ULONG)cd->cd_BoardAddr;
	hw_size = (ULONG)cd->cd_BoardSize;

	ax_present = *((volatile UWORD*)(hw_addr+REG_ZZ_AUDIO_CONFIG));
	if(!ax_present) {
		KPrintF("Error: ZZ9000AX not detected.\n");
		return FALSE;
	}

	KPrintF("HwAddr=0x%08lX, HwSize=0x%08lX\n", hw_addr, hw_size);
	MhiLibBase->hw_addr = hw_addr;
	MhiLibBase->hw_size = hw_size;

	MhiLibBase->flags = 0;
	BPTR fh;
	if((fh=Open((CONST_STRPTR)"ENV:ZZ9K_INT2",MODE_OLDFILE))) {
		Close(fh);
		MhiLibBase->flags |= DEVF_INT2MODE;
	}

	MhiLibBase->NumAllocatedDecoders = 0;
	return TRUE;
}

void UserLibCleanup(struct MHI_LibBase *MhiLibBase) {
	// Nothing to clean up here because UserLibInit() didn't leave anything open.
}

extern ULONG dev_sisr(struct MhiPlayer *mp asm("a1"));
ULONG cdev_sisr(struct MhiPlayer *mp asm("a1")) {
	ULONG buf_samples = ZZ_BYTES_PER_PERIOD/4;
	fillFifo(mp);

	setRegister(mp, REG_ZZ_AUDIO_SCALE, buf_samples);

	setDecoderParam(mp, 4, (mp->decode_offset+mp->buf_offset)>>16);
	setDecoderParam(mp, 5, (mp->decode_offset+mp->buf_offset)&0xffff);
	setRegister(mp, REG_ZZ_DECODE, DECODE_RUN);
	
	// play buffer
	setRegister(mp, REG_ZZ_AUDIO_SWAB, (1<<15) | (mp->buf_offset >> 8)); // no byteswap, offset/256
	int overrun = getRegister(mp, REG_ZZ_AUDIO_SWAB);
	
	if (overrun == 1) {
	  mp->buf_offset = 0;
	} else {
	  mp->buf_offset += ZZ_BYTES_PER_PERIOD;
	}
	
	if (mp->buf_offset >= AUDIO_BUFSZ) {
	  mp->buf_offset = 0;
	}
	return 0;
}

extern ULONG dev_isr(struct MhiPlayer *mp asm("a1"));
ULONG cdev_isr(struct MhiPlayer *mp asm("a1")) {
	UWORD status = *(UWORD*)(mp->hw_addr+REG_ZZ_CONFIG);

	// audio interrupt signal set?
	if(status & 2) {
		// Ack/clear audio interrupt.
		*(USHORT*)(mp->hw_addr+REG_ZZ_CONFIG) = 8|32;
		// Cause soft interrupt to do the rest.
		Cause(&mp->sirq);
	}
	return 0;
}

void init_interrupt(struct MhiPlayer *mp) {
	// Software interrupts have only five allowable priority levels:
	// -32, -16, 0, +16, and +32
	mp->sirq.is_Node.ln_Pri  = 0;
	mp->sirq.is_Node.ln_Type = NT_INTERRUPT;
	mp->sirq.is_Node.ln_Name = "mhizz9000s";
	mp->sirq.is_Data = mp;
	mp->sirq.is_Code = (void*)dev_sisr;

	mp->irq.is_Node.ln_Type = NT_INTERRUPT;
	mp->irq.is_Node.ln_Pri  = 126; // High priority hard-interrupt server because it needs to react quickly.
	mp->irq.is_Node.ln_Name = "mhizz9000";
	mp->irq.is_Data = mp;
	mp->irq.is_Code = (void*)dev_isr;

	Forbid();
	if (mp->flags & DEVF_INT2MODE) {
		AddIntServer(INTB_PORTS, &mp->irq);
	} else {
		AddIntServer(INTB_EXTER, &mp->irq);
	}
	Permit();

	// enable HW interrupt
	setRegister(mp, REG_ZZ_AUDIO_CONFIG, 1);
}

void destroy_interrupt(struct MhiPlayer *mp) {
	// disable HW interrupt
	setRegister(mp, REG_ZZ_AUDIO_CONFIG, 0);

	Forbid();
	if (mp->flags & DEVF_INT2MODE) {
		RemIntServer(INTB_PORTS, &mp->irq);
	} else {
		RemIntServer(INTB_EXTER, &mp->irq);
	}
	Permit();
}

/*
 *
 */
APTR i_MHIAllocDecoder(REGA0(struct Task *mhi_task), REGD0(ULONG mhi_sigmask), REGA6(struct MHI_LibBase *MHI_LibBase)) {
	struct MhiPlayer *mp = NULL;

	// We only support one exclusive decoder allocation.
	if(MHI_LibBase->NumAllocatedDecoders) {
		return NULL;
	}

	mp = AllocVec(sizeof(struct MhiPlayer), MEMF_CLEAR);
	if(mp) {
		mp->hw_addr = MHI_LibBase->hw_addr;

		if (MHI_LibBase->zorro_version == 3) {
			// FIFO offset as in axmp3 (this is still within Zorro3 address range).
			mp->encoded_offset =  0x06000000;
			// Decoded audio offset right after FIFO with a little padding to be cache line aligned.
			mp->decode_offset  = (0x06000000 + FIFOSIZE + 32) & 0xFFFFFFE0;
		} else {
			// FIFO offset like offset_tx in AHI driver (this is still within Zorro2 address range).
			mp->encoded_offset = MHI_LibBase->hw_size - 0x20000;
			// Decoded audio offset at 96MB.
			mp->decode_offset  = 0x06000000;
		}
		KPrintF("encoded_offset = 0x%08lX\n", mp->encoded_offset);
		KPrintF("decode_offset  = 0x%08lX\n", mp->decode_offset);

		mp->mp3_addr = MHI_LibBase->hw_addr + 0x10000 + mp->encoded_offset;
		
		mp->decode_chunk_sz = 1920; // 16 bit sample pairs

		mp->MhiTask      = mhi_task;
		mp->MhiMask      = mhi_sigmask;
		mp->Status       = MHIF_STOPPED;

		mp->flags        = MHI_LibBase->flags;

		mp->FifoMode     = FIFO_PREFILL;
		mp->FifoWriteIdx = 0;
		mp->buf_offset   = 0;
		mp->volume	     = 100;
		mp->panning      = 50;

		mp->BufferList = AllocVec(sizeof(struct MinList), MEMF_PUBLIC|MEMF_CLEAR);
		if(mp->BufferList) {
			NewList((struct List *)mp->BufferList);
			MHI_LibBase->NumAllocatedDecoders++;
			return mp;
		}
		FreeVec(mp);
	}
	return NULL;
}


/*
 *
 */
void i_MHIFreeDecoder(REGA3(APTR mhi_handle), REGA6(struct MHI_LibBase *MHI_LibBase)) {
	struct MhiPlayer *mp = (struct MhiPlayer *)mhi_handle;
	if(mp) {
		setAudioParam(mp, AP_DSP_SET_STEREO_VOLUME, 100 | (50<<8));
		setAudioParam(mp, AP_DSP_SET_PREFACTOR,     50);
		setAudioParam(mp, AP_DSP_SET_EQ_BAND1,      50);
		setAudioParam(mp, AP_DSP_SET_EQ_BAND2,      50);
		setAudioParam(mp, AP_DSP_SET_EQ_BAND3,      50);
		setAudioParam(mp, AP_DSP_SET_EQ_BAND4,      50);
		setAudioParam(mp, AP_DSP_SET_EQ_BAND5,      50);
		setAudioParam(mp, AP_DSP_SET_EQ_BAND6,      50);
		setAudioParam(mp, AP_DSP_SET_EQ_BAND7,      50);
		setAudioParam(mp, AP_DSP_SET_EQ_BAND8,      50);
		setAudioParam(mp, AP_DSP_SET_EQ_BAND9,      50);
		setAudioParam(mp, AP_DSP_SET_EQ_BAND10,     50);

		if(mp->BufferList) {
			APTR killednode;
			while(killednode = RemHead((struct List *)mp->BufferList)) {
				FreeVec(killednode);
			}
			FreeVec(mp->BufferList);
		}
		FreeVec(mp);
	}
	if(MHI_LibBase->NumAllocatedDecoders) MHI_LibBase->NumAllocatedDecoders--;
}


/*
 *
 */
BOOL i_MHIQueueBuffer(REGA3(APTR mhi_handle), REGA0(APTR mhi_buffer), REGD0(ULONG mhi_size), REGA6(struct MHI_LibBase *MHI_LibBase)) {
	struct MhiPlayer *mp = (struct MhiPlayer *)mhi_handle;
	struct ListNode *BufferNode;
	if(mp != NULL) {
		
		BufferNode = AllocVec(sizeof(struct ListNode), MEMF_PUBLIC|MEMF_CLEAR);
		BufferNode->Buffer = mhi_buffer;
		BufferNode->Size   = mhi_size;
		BufferNode->Index  = 0;
		BufferNode->Played = FALSE;
		AddTail((struct List *)mp->BufferList, (struct Node *)BufferNode);

		KPrintF("MHIQueueBuffer: Adr=0x%08lX\n", mhi_buffer);

		return TRUE;
	}

	return FALSE;
}


/*
 *
 */
APTR i_MHIGetEmpty(REGA3(APTR mhi_handle), REGA6(struct MHI_LibBase *MHI_LibBase)) {
	struct MhiPlayer *mp = (struct MhiPlayer *)mhi_handle;
	struct ListNode *BufferNode;
	APTR mhi_buffer = NULL;

	/* Fetch first free buffer and return its memory pointer to the caller */
	if(mp) {
		for(;;) {
			BufferNode = (struct ListNode *)mp->BufferList->mlh_Head;
			if(BufferNode == NULL) break;
			if(BufferNode->Header.mln_Succ == NULL) break;
			if(BufferNode->Played == FALSE) break;

			mhi_buffer = BufferNode->Buffer;
			RemHead((struct List *)mp->BufferList);
			FreeVec(BufferNode);
		}
	}
	return mhi_buffer;
}


/*
 *
 */
UBYTE i_MHIGetStatus(REGA3(APTR mhi_handle), REGA6(struct MHI_LibBase *MHI_LibBase)) {
	struct MhiPlayer *mp = (struct MhiPlayer *)mhi_handle;

	if(mp) {
		return mp->Status;
	}
	return MHIF_STOPPED;
}


/*
 *
 */
void i_MHIPlay(REGA3(APTR mhi_handle), REGA6(struct MHI_LibBase *MHI_LibBase)) {
	struct MhiPlayer *mp = (struct MhiPlayer *)mhi_handle;

	KPrintF("MHIPlay called\n");

	if(mp) {
		switch(mp->Status) {
			case MHIF_STOPPED:
				KPrintF("MHIPlay: Clearing FIFO.\n");
				clearFifo(mp);
				KPrintF("MHIPlay: Fillng FIFO.\n");
				fillFifo(mp);
				
				// set tx buffer address to 127 MB offset
				setAudioParam(mp, AP_TX_BUF_OFFS_HI, mp->decode_offset>>16);
				setAudioParam(mp, AP_TX_BUF_OFFS_LO, mp->decode_offset&0xffff);
				
				// set LPF to 20KHz
				setAudioParam(mp, AP_DSP_SET_LOWPASS, 20000);
				
				// set decoder params
				setDecoderParam(mp, 0, mp->encoded_offset>>16);
				setDecoderParam(mp, 1, mp->encoded_offset&0xffff);
				setDecoderParam(mp, 2, FIFOSIZE>>16);
				setDecoderParam(mp, 3, FIFOSIZE&0xffff);
				setDecoderParam(mp, 4, mp->decode_offset>>16);
				setDecoderParam(mp, 5, mp->decode_offset&0xffff);
				setDecoderParam(mp, 6, mp->decode_chunk_sz>>16);
				setDecoderParam(mp, 7, mp->decode_chunk_sz&0xffff);
				
				// ZZ_DECODE (init)
				setRegister(mp, REG_ZZ_DECODE, DECODE_INIT);
	
				init_interrupt(mp);
				KPrintF("MHIPlay: interrupt inited.\n");
			
				mp->Status = MHIF_PLAYING;
			break;
			case MHIF_PAUSED:			
				// enable HW interrupt
				setRegister(mp, REG_ZZ_AUDIO_CONFIG, 1);
				mp->Status = MHIF_PLAYING;
			break;
		}
	}
}


/*
 *
 */
void i_MHIStop(REGA3(APTR mhi_handle), REGA6(struct MHI_LibBase *MHI_LibBase)) {
	struct MhiPlayer *mp = (struct MhiPlayer *)mhi_handle;
	APTR killednode;

	KPrintF("MHIStop called\n");

	if(mp) {
		switch(mp->Status) {
			case MHIF_PLAYING:
			case MHIF_PAUSED:
			case MHIF_OUT_OF_DATA:
				destroy_interrupt(mp);
				while(killednode = RemHead((struct List *)mp->BufferList)) {
					FreeVec(killednode);
				}
				mp->Status = MHIF_STOPPED;
			break;
		}
	}
}


/*
 *
 */
void i_MHIPause(REGA3(APTR mhi_handle), REGA6(struct MHI_LibBase *MHI_LibBase)) {
	struct MhiPlayer *mp = (struct MhiPlayer *)mhi_handle;

	KPrintF("MHIPause called\n");

	if(mp) {
		switch(mp->Status) {
			case MHIF_PLAYING:
				// disable HW interrupt
				setRegister(mp, REG_ZZ_AUDIO_CONFIG, 0);
				mp->Status = MHIF_PAUSED;
			break;
		}
	}
}


/*
 *
 */
ULONG i_MHIQuery(REGD1( ULONG mhi_query), REGA6(struct MHI_LibBase *MHI_LibBase)) {
	KPrintF("MHIQuery: query = %ld\n", mhi_query);

	switch(mhi_query) {
		case MHIQ_CAPABILITIES:
			return (ULONG)"audio/mpeg{audio/mp3}"; // We currently only support mp3 contained in a raw MPEG stream.

		case MHIQ_DECODER_NAME:
			return (ULONG)"ZZ9000AX";

		case MHIQ_DECODER_VERSION:
			return (ULONG)IDSTRING;

		case MHIQ_AUTHOR:
			return (ULONG)"Thomas Wenzel";

		case MHIQ_IS_HARDWARE:
			return MHIF_TRUE;

//		case MHIQ_LAYER1:
//		case MHIQ_LAYER2:
		case MHIQ_LAYER3:
			return MHIF_SUPPORTED;

		case MHIQ_MPEG1:
		case MHIQ_MPEG2:
		case MHIQ_MPEG25:
		case MHIQ_VARIABLE_BITRATE:
		case MHIQ_JOINT_STEREO:
			return MHIF_SUPPORTED;

		case MHIQ_VOLUME_CONTROL:
			return MHIF_SUPPORTED;
//		case MHIQ_PANNING_CONTROL:
//			return MHIF_SUPPORTED;

		case MHIQ_PREFACTOR_CONTROL:
		case MHIQ_BASS_CONTROL:
		case MHIQ_TREBLE_CONTROL:
		case MHIQ_MID_CONTROL:
		case MHIQ_5_BAND_EQ:
		case MHIQ_10_BAND_EQ:
			return MHIF_SUPPORTED;


		default:
			return MHIF_UNSUPPORTED;
	}
}

/*
 *
 */
void i_MHISetParam(REGA3(APTR mhi_handle), REGD0(UWORD mhi_param), REGD1(ULONG mhi_value), REGA6(struct MHI_LibBase *MHI_LibBase)) {
	struct MhiPlayer *mp = (struct MhiPlayer *)mhi_handle;

	if(mp) {
		switch(mhi_param) {
			case MHIP_PANNING: // 0..50..100
				if(mhi_value > 100) mhi_value = 100;
				mp->panning = mhi_value;
				// set volume/panning
				setAudioParam(mp, AP_DSP_SET_STEREO_VOLUME, mp->volume | (mp->panning<<8));
				break;

			case MHIP_VOLUME: // 0..100
				if(mhi_value > 100) mhi_value = 100;
				mp->volume = mhi_value;
				// set volume/panning
				setAudioParam(mp, AP_DSP_SET_STEREO_VOLUME, mp->volume | (mp->panning<<8));
				break;

			case MHIP_PREFACTOR: // 0..50..100
				if(mhi_value > 100) mhi_value = 100;
				setAudioParam(mp, AP_DSP_SET_PREFACTOR, mhi_value);
				break;

			case MHIP_BAND1:
				if(mhi_value > 100) mhi_value = 100;
				setAudioParam(mp, AP_DSP_SET_EQ_BAND1, mhi_value);
				break;
			case MHIP_BAND2:
				if(mhi_value > 100) mhi_value = 100;
				setAudioParam(mp, AP_DSP_SET_EQ_BAND2, mhi_value);
				break;
			case MHIP_BAND3:
				if(mhi_value > 100) mhi_value = 100;
				setAudioParam(mp, AP_DSP_SET_EQ_BAND3, mhi_value);
				break;
			case MHIP_BAND4:
				if(mhi_value > 100) mhi_value = 100;
				setAudioParam(mp, AP_DSP_SET_EQ_BAND4, mhi_value);
				break;
			case MHIP_BAND5:
				if(mhi_value > 100) mhi_value = 100;
				setAudioParam(mp, AP_DSP_SET_EQ_BAND5, mhi_value);
				break;
			case MHIP_BAND6:
				if(mhi_value > 100) mhi_value = 100;
				setAudioParam(mp, AP_DSP_SET_EQ_BAND6, mhi_value);
				break;
			case MHIP_BAND7:
				if(mhi_value > 100) mhi_value = 100;
				setAudioParam(mp, AP_DSP_SET_EQ_BAND7, mhi_value);
				break;
			case MHIP_BAND8:
				if(mhi_value > 100) mhi_value = 100;
				setAudioParam(mp, AP_DSP_SET_EQ_BAND8, mhi_value);
				break;
			case MHIP_BAND9:
				if(mhi_value > 100) mhi_value = 100;
				setAudioParam(mp, AP_DSP_SET_EQ_BAND9, mhi_value);
				break;
			case MHIP_BAND10:
				if(mhi_value > 100) mhi_value = 100;
				setAudioParam(mp, AP_DSP_SET_EQ_BAND10, mhi_value);
				break;

			default:
				KPrintF("MHISetParam: Unknown parameter %ld, value = %ld\n", mhi_param, mhi_value);
				break;
		}
	}
}

