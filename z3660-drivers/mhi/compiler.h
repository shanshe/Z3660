
#ifndef _COMPILER_H_
#define _COMPILER_H_

#ifdef __VBCC__
#define __aligned
#define ASM __asm
#define REGD0(f) register __reg("d0") f
#define REGD1(f) register __reg("d1") f
#define REGD2(f) register __reg("d2") f
#define REGD3(f) register __reg("d3") f
#define REGD4(f) register __reg("d4") f
#define REGD5(f) register __reg("d5") f
#define REGD6(f) register __reg("d6") f
#define REGD7(f) register __reg("d7") f
#define REGA0(f) register __reg("a0") f
#define REGA1(f) register __reg("a1") f
#define REGA2(f) register __reg("a2") f
#define REGA3(f) register __reg("a3") f
#define REGA4(f) register __reg("a4") f
#define REGA5(f) register __reg("a5") f
#define REGA6(f) register __reg("a6") f
#define REGA7(f) register __reg("a7") f

#define BITCLR  0
#define BITSET  0x8000
//#define cust ((struct Custom *) (0xDFF000))
#endif

#ifdef __GNUC__
#define ASM __asm__
#define REGD0(f) f __asm("d0")
#define REGD1(f) f __asm("d1")
#define REGD2(f) f __asm("d2")
#define REGD3(f) f __asm("d3")
#define REGD4(f) f __asm("d4")
#define REGD5(f) f __asm("d5")
#define REGD6(f) f __asm("d6")
#define REGD7(f) f __asm("d7")
#define REGA0(f) f __asm("a0")
#define REGA1(f) f __asm("a1")
#define REGA2(f) f __asm("a2")
#define REGA3(f) f __asm("a3")
#define REGA4(f) f __asm("a4")
#define REGA5(f) f __asm("a5")
#define REGA6(f) f __asm("a6")
#define REGA7(f) f __asm("a7")
//extern struct Custom custom;
//static struct Custom *cust = &custom;
#endif

#endif


