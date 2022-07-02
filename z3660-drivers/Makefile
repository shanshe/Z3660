#
# makefile for compiling z3660 rtg card driver with vbcc
#

export PATH := /opt/vbcc/bin:/usr/bin:/bin:/usr/sbin:/sbin
export VBCC := /opt/vbcc

PREFX = $(VBCC)/targets/m68k-amigaos

SRC_RTG = rtg
CPU = 68060
FPU = 68060

SYSINC = -I$(PREFX)/include
SYSINC+= -I$(VBCC)/NDK_3.9/Include/include_h

#LINKLIBS = -L$(PREFX)/lib
#LINKLIBS+= -L$(VBCC)/NDK_3.9/Include/linker_libs
#LINKLIBS+= -ldebug
LINKLIBS+= -lamiga

CCX = $(VBCC)/bin/vc +aos68k
CFLAGS = -v -k -c99 -O2 -cpu=$(CPU) -fpu=$(FPU)

all: clean $(SRC_RTG)/Z3660.card

$(SRC_RTG)/Z3660.card: $(SRC_RTG)/gfx.c $(SRC_RTG)/gfx.h $(SRC_RTG)/rtg.h $(SRC_RTG)/z3660.h
	$(CCX) -nostdlib $(SYSINC) $(CFLAGS) -o $(SRC_RTG)/Z3660.card $(SRC_RTG)/gfx.c $(LINKLIBS)
	@rm -rf $(SRC_RTG)/gfx.asm
	@rm -rf $(SRC_RTG)/gfx.o

clean: 
	@rm -rf $(SRC_RTG)/Z3660.card