#ifndef _PORT_H
#define _PORT_H

#include <stdint.h>
#include "printf.h"
#include "callout.h"
#include <proto/exec.h>
#include <exec/execbase.h>
#include <inline/exec.h>

#define BIT(x)        (1 << (x))
#define ARRAY_SIZE(x) ((sizeof (x) / sizeof ((x)[0])))

#include <sys/param.h>

#define PAGE_SIZE NBPG

#define AMIGA_MAX_TRANSFER (1 << 20)  // Maximum DMA size (scatter-gather entry)
#undef MAXPHYS
#define MAXPHYS AMIGA_MAX_TRANSFER

struct device;
typedef struct device *device_t;

void panic(const char *s, ...);
#if 0
unsigned int read_system_ticks(void);
unsigned int ticks_since_last(void);
#endif
int irq_and_timer_handler(void);

#define __USE(x) (/*LINTED*/(void)(x))

void *device_private(device_t dev);
const char *device_xname(void *dev);

int bsd_splbio();
void bsd_splx();

#define hz TICKS_PER_SECOND
#define mstohz(m) ((m) * TICKS_PER_SECOND / 1000)
#define kvtop(x) ((uint32_t)(x))

void delay(int usecs);

#define __UNVOLATILE(x) ((void *)(unsigned long)(volatile void *)(x))
#define __UNCONST(a) ((void *)(intptr_t)(a))

#define B_WRITE         0x00000000      /* Write buffer (pseudo flag). */
#define B_READ          0x00100000      /* Read buffer. */

// Can't assign printf() to KPrintF() because RawDoFmt() assumes 16-bit ints
#if 0
#undef printf
#define printf dbgprintf
int dbgprintf(const char *fmt, ...);
#endif

#define memcpy USE_CopyMem

// void *local_memcpy(void *dest, const void *src, size_t n);
// void *local_memset(void *dest, int value, size_t len);
// #define memcpy local_memcpy
// #define memset local_memset
// #define memcpy(dst, src, len) CopyMem(src, dst, len)
// #define memset(dst, value, len) SetMem(dst, value, len)

#define SDT_PROBE1(v,w,x,y,z)
#define SDT_PROBE2(u,v,w,x,y,z)
#define SDT_PROBE3(t,u,v,w,x,y,z)
#define KASSERT(x)

#define uimin(x,y) ((x <= y) ? (x) : (y))
#define mutex_init(x, y, z)
#define mutex_enter(x) do { } while (0)
#define mutex_exit(x)  do { } while (0)
#define cv_init(x, y)
#define cv_wait(x, y)
#define cv_broadcast(x)
#define cv_destroy(x)

#if (!defined(DEBUG_ATTACH)      && \
     !defined(DEBUG_DEVICE)      && \
     !defined(DEBUG_CMDHANDLER)  && \
     !defined(DEBUG_NCR53CXXX)   && \
     !defined(DEBUG_PORT)        && \
     !defined(DEBUG_SCSIPI_BASE) && \
     !defined(DEBUG_SCSICONF)    && \
     !defined(DEBUG_SCSIMSG)     && \
     !defined(DEBUG_SD)          && \
     !defined(DEBUG_SIOP)        && \
     !defined(DEBUG_BOOTMENU)    && \
     !defined(DEBUG_MOUNTER)) || defined(NO_SERIAL_OUTPUT)
#ifdef USE_SERIAL_OUTPUT
#undef USE_SERIAL_OUTPUT
#endif
#endif

#ifndef USE_SERIAL_OUTPUT
#define printf(x...)   do { } while (0)
#define vprintf(x...)  do { } while (0)
#define vfprintf(x...) do { } while (0)
#define putchar(x...)  do { } while (0)
#endif

#ifndef USING_BASEREL
#undef __saveds
#define __saveds
#endif
#endif /* _PORT_H */
