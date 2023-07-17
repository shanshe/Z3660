#ifndef MHI_Z3660_H
#define MHI_Z3660_H

#include <exec/exec.h>
#include <libraries/mhi.h>
#include "compiler.h"
#include "version.h"
#include "mhilib.h"

#define MHILIB_NAME "mhiz3660.library"

#define IDSTRING NUM2STR(VERSION) "." NUM2STR(REVISION) " (" NUM2STR(DATE) ")"

APTR i_MHIAllocDecoder(REGA0(struct Task *), REGD0(ULONG), REGA6(struct MHI_LibBase *));
void i_MHIFreeDecoder(REGA3(APTR), REGA6(struct MHI_LibBase *));
BOOL i_MHIQueueBuffer(REGA3(APTR), REGA0(APTR), REGD0(ULONG), REGA6(struct MHI_LibBase *));
APTR i_MHIGetEmpty(REGA3(APTR), REGA6(struct MHI_LibBase *));
UBYTE i_MHIGetStatus(REGA3(APTR), REGA6(struct MHI_LibBase *));
void i_MHIPlay(REGA3(APTR), REGA6(struct MHI_LibBase *));
void i_MHIStop(REGA3(APTR), REGA6(struct MHI_LibBase *));
void i_MHIPause(REGA3(APTR), REGA6(struct MHI_LibBase *));
ULONG i_MHIQuery(REGD1( ULONG), REGA6(struct MHI_LibBase *));
void i_MHISetParam(REGA3(APTR), REGD0(UWORD), REGD1( ULONG), REGA6(struct MHI_LibBase *));

#endif
