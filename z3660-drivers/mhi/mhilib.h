#ifndef MHILIB_H
#define MHILIB_H

/* MHI status flags for player */
// #define MHIF_PLAYING        0
// #define MHIF_STOPPED        1
// #define MHIF_OUT_OF_DATA    2
// #define MHIF_PAUSED         3

typedef enum {
	FIFO_PREFILL,
	FIFO_OPERATIONAL
} FIFO_MODE;

struct MHI_LibBase {
	struct Library mhi_Library;
	BPTR mhi_SegList;
	struct ExecBase *mhi_SysBase;
	struct DosLibrary *mhi_DOSBase;
	ULONG NumAllocatedDecoders;

	ULONG hw_addr;
	ULONG hw_size;

	UBYTE flags;
};

struct MhiPlayer {
	struct Task *MhiTask;
	ULONG MhiMask;
	struct MinList *BufferList;
	ULONG Status;

	FIFO_MODE FifoMode;
	ULONG FifoWriteIdx;

	struct Task *t_mainproc;
	struct Process *worker_process;
	struct Interrupt irq;
	struct Interrupt sirq;
	BYTE mainproc_signal;
	BYTE enable_signal;
	BYTE worker_signal;
	
	ULONG hw_addr;
	ULONG mp3_addr;
	ULONG mp3_bytes;
	ULONG encoded_offset;
	ULONG decode_offset;
	ULONG decode_chunk_sz;
	ULONG buf_offset;
	ULONG volume;
	ULONG panning;

	UBYTE flags;

};

struct ListNode {
	struct MinNode Header;
	UBYTE*         Buffer;
	ULONG          Size;
	ULONG          Index;
	BOOL           Played;
};

#endif

