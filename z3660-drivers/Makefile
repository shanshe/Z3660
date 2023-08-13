#
# makefile for compiling z3660 rtg card driver with vbcc
#

export PATH := /opt/vbcc/bin:/usr/bin:/bin:/usr/sbin:/sbin:/opt/amiga/bin
export VBCC := /opt/vbcc

CPU = 68060
FPU = 68060

PREFX = $(VBCC)/targets/m68k-amigaos

SRC_AHI  = ahi
SRC_MHI  = mhi
SRC_RTG  = rtg
SRC_ETH  = eth
SRC_ZTP  = ZTop
SRC_W3D  = Wazp3D
SRC_SCSI = scsi

ETH_DEFINES  = -DDEVICENAME="Z3660Net.device"
ETH_DEFINES += -DHAVE_VERSION_H=1

WARP3D_COPTS  = -m68020-40 -m68881 -nostartfiles -noixemul -Wall -Wextra -Wno-unused-parameter
WARP3D_COPTS += -DMOTOROLAORDER
WARP3D_COPTS += -D__Z3660__
WARP3D_COPTS += -DWAZP3DDEBUG

GCC = /opt/amiga/bin/m68k-amigaos-gcc
RM = @rm -rf
CP = cp -f

SYSINC = -I$(PREFX)/include
SYSINC+= -I$(VBCC)/NDK_3.9/Include/include_h

SYSINC_I = -I$(VBCC)/NDK_3.9/Include/include_i

#LINKLIBS = -L$(PREFX)/lib
LINKLIBS+= -L$(VBCC)/NDK_3.9/Include/linker_libs
LINKLIBS+= -lamiga
#LINKLIBS+= -ldebug

CCX = $(VBCC)/bin/vc +aos68k
RTG_CFLAGS = -v -k -c99 -O2 -cpu=$(CPU) -fpu=$(FPU)
LINKEXE = vc +aos68k

ETH_CFLAGS  = -O2 -+ -sc -c99 -cpu=$(CPU)
#ETH_CFLAGS += -DDEBUG
SYSLIB = "-L$(PREFX)/lib"
ETH_OBJECTS  = $(SRC_ETH)/deviceheader.o
ETH_OBJECTS += $(SRC_ETH)/deviceinit.o
ETH_OBJECTS += $(SRC_ETH)/device.o
ETH_LINK = vlink
ETH_LDFLAGS = -bamigahunk -x -s -mrel -Cvbcc -Bstatic -nostdlib $(SYSLIB) #-Rshort

all: clean $(SRC_RTG)/Z3660.card $(SRC_ETH)/Z3660Net.device $(SRC_ZTP)/ZTop $(SRC_W3D)/Wazp3D.library $(SRC_AHI)/z3660ax.audio $(SRC_MHI)/mhiz3660.library $(SRC_SCSI)/z3660_scsi.device

$(SRC_RTG)/Z3660.card: $(SRC_RTG)/gfx.c $(SRC_RTG)/gfx.h $(SRC_RTG)/rtg.h $(SRC_RTG)/z3660_regs.h
#	$(CCX) -nostdlib $(SYSINC) $(RTG_CFLAGS) -o $(SRC_RTG)/Z3660.card $(SRC_RTG)/gfx.c $(LINKLIBS)
#	$(RM) $(SRC_RTG)/gfx.asm
#	$(RM) $(SRC_RTG)/gfx.o
	$(GCC) $(SRC_RTG)/gfx.c -m68020 -mtune=68020-60 -O2 -o $(SRC_RTG)/Z3660.card -noixemul -Wall -Wextra -Wno-unused-parameter -fomit-frame-pointer -nostartfiles -lamiga -DDMARTG -ldebug
	$(RM) $(SRC_RTG)/gfx.asm
	$(RM) $(SRC_RTG)/gfx.o
	$(CP) $(SRC_RTG)/Z3660.card ~/Documents/FS-UAE/HardDrives/Almacen/

$(SRC_AHI)/z3660ax.audio: $(SRC_AHI)/z3660ax-ahi.c $(SRC_AHI)/asmfuncs.s
	$(GCC) $(SRC_AHI)/z3660ax-ahi.c $(SRC_AHI)/asmfuncs.s -m68020 -mtune=68020-60 -O3 -o $(SRC_AHI)/z3660ax.audio -Wall -Wextra -Wno-unused-parameter -nostartfiles -lamiga -ldebug
	$(RM) $(SRC_AHI)/z3660ax-ahi.asm
	$(RM) $(SRC_AHI)/z3660ax-ahi.o
	$(RM) $(SRC_AHI)/asmfuncs.o
	/opt/amiga/bin/vasmm68k_mot -phxass -Fhunk -m68020 -o $(SRC_AHI)/PREFSFILE.uncut $(SRC_AHI)/prefsfile.a -I/opt/amiga/m68k-amigaos/ndk-include -I/opt/amiga/m68k-amigaos/include
	dd bs=1 skip=40 if=$(SRC_AHI)/PREFSFILE.uncut of=$(SRC_AHI)/Z3660AX

#m68k-amigaos-gcc StartUp.c LibInit.c mhizz9000.c asmfuncs.s -O3 -o mhizz9000.library.debug -g -ggdb -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast -Wno-pointer-sign -nostartfiles -m68020 -ldebug

$(SRC_MHI)/mhiz3660.library: $(SRC_MHI)/Startup.c $(SRC_MHI)/LibInit.c $(SRC_MHI)/mhi_Z3660.c $(SRC_MHI)/asmfuncs.s
	$(GCC) -I$(SRC_MHI)/Include $(SRC_MHI)/Startup.c $(SRC_MHI)/LibInit.c $(SRC_MHI)/mhi_Z3660.c $(SRC_MHI)/asmfuncs.s -m68020 -O2 -o $(SRC_MHI)/mhiz3660.library -Wall -Wextra -Wno-unused-parameter -nostartfiles -lamiga -ldebug
	$(RM) $(SRC_MHI)/Startup.asm
	$(RM) $(SRC_MHI)/Startup.o
	$(RM) $(SRC_MHI)/LibInit.asm
	$(RM) $(SRC_MHI)/LibInit.o
	$(RM) $(SRC_MHI)/mhi_Z3660.asm
	$(RM) $(SRC_MHI)/mhi_Z3660.o
	$(RM) $(SRC_MHI)/asmfuncs.o
#	/opt/amiga/bin/m68k-amigaos-strip -s -o $(SRC_MHI)/mhiz3660.library $(SRC_MHI)/mhiz3660.library.debug
	$(CP) $(SRC_MHI)/mhiz3660.library ~/Documents/FS-UAE/HardDrives/Almacen/mhiz3660.library
	$(GCC) -I$(SRC_MHI)/Include $(SRC_MHI)/axmp3.c -m68030 -O2 -o $(SRC_MHI)/axmp3 -Wall -Wextra -Wno-unused-parameter -lamiga -noixemul

$(SRC_SCSI)/z3660_scsi.device: $(SRC_SCSI)/z3660_scsi.c $(SRC_SCSI)/z3660_scsi_enums.h $(SRC_SCSI)/makerom.c $(SRC_SCSI)/bootrom.s $(SRC_SCSI)/bootmenu.c $(SRC_SCSI)/printf.c $(SRC_SCSI)/port.c $(SRC_SCSI)/scsimsg.c
	$(GCC) $(SRC_SCSI)/z3660_scsi.c $(SRC_SCSI)/bootmenu.c $(SRC_SCSI)/printf.c $(SRC_SCSI)/port.c $(SRC_SCSI)/scsimsg.c -m68020 -O2 -o $(SRC_SCSI)/z3660_scsi.device -Wall -Wextra -Wno-unused-parameter -fomit-frame-pointer -nostartfiles -lc -lamiga -ldebug
	/opt/amiga/bin/vasmm68k_mot -m68000 -Fhunk $(SYSINC_I) $(SRC_SCSI)/bootrom.s -o $(SRC_SCSI)/bootrom_uncut
	dd bs=1 skip=40 if=$(SRC_SCSI)/bootrom_uncut of=$(SRC_SCSI)/bootrom
	gcc $(SRC_SCSI)/makerom.c -o $(SRC_SCSI)/makerom
	cd $(SRC_SCSI) ; ./makerom
	$(RM) $(SRC_SCSI)/bootrom
	$(RM) $(SRC_SCSI)/makerom
	$(RM) $(SRC_SCSI)/bootrom_uncut

$(SRC_ETH)/Z3660Net.device: $(SRC_ETH)/deviceheader.c $(SRC_ETH)/deviceinit.c $(SRC_ETH)/device.c $(SRC_ETH)/device.h $(SRC_ETH)/compiler.h $(SRC_ETH)/macros.h $(SRC_ETH)/sana2.h $(SRC_ETH)/version.h
	$(CCX) -c $(SYSINC) $(ETH_CFLAGS) $(ETH_DEFINES) -o $(SRC_ETH)/deviceheader.o $(SRC_ETH)/deviceheader.c
	$(CCX) -c $(SYSINC) $(ETH_CFLAGS) $(ETH_DEFINES) -o $(SRC_ETH)/deviceinit.o $(SRC_ETH)/deviceinit.c
	$(CCX) -c $(SYSINC) $(ETH_CFLAGS) $(ETH_DEFINES) -o $(SRC_ETH)/device.o $(SRC_ETH)/device.c
	$(ETH_LINK) $(ETH_LDFLAGS) $(ETH_OBJECTS) -o $(SRC_ETH)/Z3660Net.device $(LINKLIBS) 

	$(RM) $(SRC_ETH)/deviceheader.asm
	$(RM) $(SRC_ETH)/deviceheader.o
	$(RM) $(SRC_ETH)/deviceinit.asm
	$(RM) $(SRC_ETH)/deviceinit.o
	$(RM) $(SRC_ETH)/device.asm
	$(RM) $(SRC_ETH)/device.o

$(SRC_ZTP)/ZTop: $(SRC_ZTP)/ZTop.c $(SRC_ZTP)/z3660_regs.h
	$(GCC) $(SRC_ZTP)/ZTop.c -m68030 -O2 -o $(SRC_ZTP)/ZTop -Wall -Wextra -Wno-unused-parameter -lamiga -noixemul -lm
	$(CP) $(SRC_ZTP)/ZTop ~/Documents/FS-UAE/HardDrives/Almacen/ZTop
	$(CP) $(SRC_ZTP)/ZTop.info ~/Documents/FS-UAE/HardDrives/Almacen/ZTop.info

$(SRC_W3D)/Wazp3D_lib.o: $(SRC_W3D)/Wazp3D_lib.c
	$(GCC) -c $(SRC_W3D)/Wazp3D_lib.c $(WARP3D_COPTS) -O0 -o $(SRC_W3D)/Wazp3D_lib.o

$(SRC_W3D)/Wazp3D56.o: $(SRC_W3D)/Wazp3D56.c $(SRC_W3D)/soft3d/soft3D56.c $(SRC_W3D)/Wazp3D.h
	$(GCC) -c $(SRC_W3D)/Wazp3D56.c $(WARP3D_COPTS) -O0 -o $(SRC_W3D)/Wazp3D56.o

$(SRC_W3D)/Wazp3D.library: $(SRC_W3D)/Wazp3D_lib.o $(SRC_W3D)/Wazp3D56.o
	$(GCC) $(SRC_W3D)/Wazp3D56.o $(SRC_W3D)/Wazp3D_lib.o -o $(SRC_W3D)/Wazp3D.library -nostartfiles -noixemul -lm
	$(CP) $(SRC_W3D)/Wazp3D.library ~/Documents/FS-UAE/HardDrives/Almacen/Warp3D.library

clean: 
	@echo cleaning...
	$(RM) $(SRC_RTG)/Z3660.card
	$(RM) $(SRC_AHI)/z3660ax.audio
	$(RM) $(SRC_AHI)/Z3660AX
	$(RM) $(SRC_MHI)/mhiz3660.library
	$(RM) $(SRC_MHI)/axmp3
	$(RM) $(SRC_ETH)/Z3660Net.device
	$(RM) $(SRC_ZTP)/ZTop
	$(RM) $(SRC_W3D)/Wazp3D56.o
	$(RM) $(SRC_W3D)/Wazp3D_lib.o
	$(RM) $(SRC_W3D)/Wazp3D.library
	$(RM) $(SRC_SCSI)/z3660_scsi.device
	$(RM) $(SRC_SCSI)/z3660_scsi.rom
	$(RM) $(SRC_SCSI)/*.o

adf: all
	@echo creating adf...
	$(RM)   0_Z3660.adf
	xdftool 0_Z3660.adf format Z3660 + makedir C + makedir Libs + makedir Libs/Picasso96 + makedir Libs/MHI + makedir Devs + makedir Devs/Monitors + makedir Devs/Networks + makedir Devs/AudioModes + makedir Devs/AHI
	xdftool 0_Z3660.adf write $(SRC_RTG)/Z3660.card Libs/Picasso96
	xdftool 0_Z3660.adf write $(SRC_RTG)/Monitors/Z3660 Devs/Monitors
	xdftool 0_Z3660.adf write $(SRC_RTG)/Monitors/Z3660.info Devs/Monitors
	xdftool 0_Z3660.adf write $(SRC_AHI)/z3660ax.audio Devs/AHI
	xdftool 0_Z3660.adf write $(SRC_AHI)/Z3660AX Devs/AudioModes
	xdftool 0_Z3660.adf write $(SRC_MHI)/mhiz3660.library Libs/MHI
	xdftool 0_Z3660.adf write $(SRC_MHI)/axmp3 C
	xdftool 0_Z3660.adf write $(SRC_ETH)/Z3660Net.device Devs/Networks
	xdftool 0_Z3660.adf write $(SRC_ZTP)/ZTop
	xdftool 0_Z3660.adf write $(SRC_ZTP)/ZTop.info
	xdftool 0_Z3660.adf write $(SRC_SCSI)/z3660_scsi.device Devs + list

A4000: adf
	$(CP) 0_Z3660.adf /Volumes/A4000/0_Z3660.adf
	$(RM) 0_Z3660.adf
	diskutil eject /Volumes/A4000
	@echo "Ejected"
