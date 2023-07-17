#ifndef CLIB_MHI_PROTOS_H
#define CLIB_MHI_PROTOS_H

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

APTR    MHIAllocDecoder(struct Task *task, ULONG mhisignal);
VOID    MHIFreeDecoder(APTR handle);
ULONG   MHIQuery(ULONG query);
BOOL    MHIQueueBuffer(APTR handle, APTR buffer, ULONG size);
APTR    MHIGetEmpty(APTR handle);
UBYTE   MHIGetStatus(APTR handle);
VOID    MHIPlay(APTR handle);
VOID    MHIStop(APTR handle);
VOID    MHIPause(APTR handle);
ULONG   MHIQuery(ULONG query);
VOID    MHISetParam(APTR handle, UWORD param, ULONG value);

#ifdef __cplusplus
};
#endif

#endif
