#ifndef _INCLUDE_PRAGMA_MHI_LIB_H
#define _INCLUDE_PRAGMA_MHI_LIB_H

#ifndef CLIB_MHI_PROTOS_H
#include <clib/mhi_protos.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma amicall(MHIBase,0x01E,MHIAllocDecoder(a0,d0))
#pragma amicall(MHIBase,0x024,MHIFreeDecoder(a3))
#pragma amicall(MHIBase,0x02A,MHIQueueBuffer(a3,a0,d0))
#pragma amicall(MHIBase,0x030,MHIGetEmpty(a3))
#pragma amicall(MHIBase,0x036,MHIGetStatus(a3))
#pragma amicall(MHIBase,0x03C,MHIPlay(a3))
#pragma amicall(MHIBase,0x042,MHIStop(a3))
#pragma amicall(MHIBase,0x048,MHIPause(a3))
#pragma amicall(MHIBase,0x04E,MHIQuery(d1))
#pragma amicall(MHIBase,0x054,MHISetParam(a3,d0,d1))

#ifdef __cplusplus
}
#endif

#endif	/*  _INCLUDE_PRAGMA_MHI_LIB_H  */
