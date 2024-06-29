#
# makefile for compiling z3660 rtg card driver with vbcc
#

export PATH := /opt/vbcc/bin:/usr/bin:/bin:/usr/sbin:/sbin:/opt/amiga/bin
export VBCC := /opt/vbcc
#export VBCC := /opt/amiga/m68k-amigaos/vbcc

NDK_32_INCLUDE := /opt/vbcc/NDK3.2/Include_H

UAEPATH := ~/Documents/FS-UAE/HardDrives/Almacen/

CPU = 68060
FPU = 68060

PREFX = $(VBCC)/targets/m68k-amigaos

SASC = /usr/local/bin/vamos sc

SRC_AHI  = ahi
SRC_MHI  = mhi
SRC_RTG  = rtg
SRC_ETH  = eth
SRC_ZTP  = ZTop
SRC_W3D  = Z3660_Wazp3D
SRC_SCSI = scsi
SRC_KICK31_060 = kick31_060

ETH_COPTS  = -Os -fomit-frame-pointer -noixemul -msmall-code -Wall -Wno-pointer-sign -Wno-strict-aliasing
ETH_COPTS += -mcpu=68060 -DDEVICENAME="Z3660Net.device" -DHAVE_VERSION_H=1 -DNEWSTYLE

WARP3D_COPTS  = -m68020-40 -m68881 -nostartfiles -noixemul -Wall -Wextra -Wno-unused-parameter
WARP3D_COPTS += -DMOTOROLAORDER
WARP3D_COPTS += -D__Z3660__
WARP3D_COPTS += -DWAZP3DDEBUG
WARP3D_COPTS += -DSOFT3DLIB

GCC = /opt/amiga/bin/m68k-amigaos-gcc
RM = @rm -rf
CP = cp -f

SYSINC_I = -I$(VBCC)/NDK_3.9/Include/include_i
LINKLIBS+= -L/opt/amiga/m68k-amigaos/ndk/lib/libs
#SYSINC_I = -I$(VBCC)/targets/m68k-amigaos/include -I$(VBCC)/targets/m68k-amigaos/include2
#LINKLIBS+= -L$(VBCC)/targets/m68k-amigaos/lib
LINKLIBS+= -lamiga
#LINKLIBS+= -ldebug

RTG_CFLAGS = -v -k -c99 -O2 -cpu=$(CPU) -fpu=$(FPU)

#all: clean $(SRC_RTG)/Z3660.card $(SRC_ETH)/Z3660Net.device $(SRC_ZTP)/ZTop $(SRC_W3D)/Wazp3D.library $(SRC_AHI)/z3660ax.audio $(SRC_MHI)/mhiz3660.library $(SRC_SCSI)/z3660_scsi.device
all: clean $(SRC_RTG)/Z3660.card $(SRC_ETH)/Z3660Net.device $(SRC_ZTP)/ZTop $(SRC_W3D)/Wazp3D.library $(SRC_AHI)/z3660ax.audio $(SRC_MHI)/mhiz3660.library $(SRC_SCSI)/z3660_scsi.device $(SRC_KICK31_060)/kick060.rom

$(SRC_RTG)/Z3660.card: $(SRC_RTG)/gfx.c $(SRC_RTG)/gfx.h $(SRC_RTG)/rtg.h $(SRC_RTG)/z3660_regs.h
	$(GCC) $(SRC_RTG)/gfx.c -m68020 -mtune=68020-60 -O2 -o $(SRC_RTG)/Z3660.card -noixemul -Wall -Wextra -Wno-unused-parameter -fomit-frame-pointer -nostartfiles -lamiga -DDMARTG -ldebug
	$(RM) $(SRC_RTG)/gfx.asm
	$(RM) $(SRC_RTG)/gfx.o
	$(CP) $(SRC_RTG)/Z3660.card $(UAEPATH)

$(SRC_AHI)/z3660ax.audio: $(SRC_AHI)/z3660ax-ahi.c $(SRC_AHI)/asmfuncs.s
	$(GCC) $(SRC_AHI)/z3660ax-ahi.c $(SRC_AHI)/asmfuncs.s -m68020 -mtune=68020-60 -O3 -o $(SRC_AHI)/z3660ax.audio -Wall -Wextra -Wno-unused-parameter -nostartfiles -lamiga -ldebug
	$(RM) $(SRC_AHI)/z3660ax-ahi.asm
	$(RM) $(SRC_AHI)/z3660ax-ahi.o
	$(RM) $(SRC_AHI)/asmfuncs.o
	/opt/amiga/bin/vasmm68k_mot -phxass -Fhunk -m68020 -o $(SRC_AHI)/PREFSFILE.uncut $(SRC_AHI)/prefsfile.a -I/opt/amiga/m68k-amigaos/ndk-include -I/opt/amiga/m68k-amigaos/include
	dd bs=1 skip=40 if=$(SRC_AHI)/PREFSFILE.uncut of=$(SRC_AHI)/Z3660AX

#m68k-amigaos-gcc StartUp.c LibInit.c mhizz9000.c asmfuncs.s -O3 -o mhizz9000.library.debug -g -ggdb -Wall -Wextra -Wno-unused-parameter -Wno-pointer-to-int-cast -Wno-pointer-sign -nostartfiles -m68020 -ldebug

$(SRC_MHI)/mhiz3660.library: $(SRC_MHI)/StartUp.c $(SRC_MHI)/LibInit.c $(SRC_MHI)/mhi_z3660.c $(SRC_MHI)/asmfuncs.s
	$(GCC) -I$(SRC_MHI)/Include $(SRC_MHI)/StartUp.c $(SRC_MHI)/LibInit.c $(SRC_MHI)/mhi_z3660.c $(SRC_MHI)/asmfuncs.s -m68020 -O2 -o $(SRC_MHI)/mhiz3660.library -Wall -Wextra -Wno-unused-parameter -nostartfiles -lamiga -ldebug
	$(RM) $(SRC_MHI)/StartUp.asm
	$(RM) $(SRC_MHI)/StartUp.o
	$(RM) $(SRC_MHI)/LibInit.asm
	$(RM) $(SRC_MHI)/LibInit.o
	$(RM) $(SRC_MHI)/mhi_Z3660.asm
	$(RM) $(SRC_MHI)/mhi_Z3660.o
	$(RM) $(SRC_MHI)/asmfuncs.o
#	/opt/amiga/bin/m68k-amigaos-strip -s -o $(SRC_MHI)/mhiz3660.library $(SRC_MHI)/mhiz3660.library.debug
	$(CP) $(SRC_MHI)/mhiz3660.library $(UAEPATH)
	$(GCC) -I$(SRC_MHI)/Include $(SRC_MHI)/axmp3.c -m68030 -O2 -o $(SRC_MHI)/axmp3 -Wall -Wextra -Wno-unused-parameter -lamiga -noixemul

$(SRC_SCSI)/z3660_scsi.device: $(SRC_SCSI)/z3660_scsi.c $(SRC_SCSI)/z3660_scsi_enums.h $(SRC_SCSI)/makerom.c $(SRC_SCSI)/bootrom.s $(SRC_SCSI)/bootmenu.c $(SRC_SCSI)/printf.c $(SRC_SCSI)/port.c $(SRC_SCSI)/scsimsg.c
	$(GCC) $(SRC_SCSI)/z3660_scsi.c $(SRC_SCSI)/bootmenu.c $(SRC_SCSI)/printf.c $(SRC_SCSI)/port.c $(SRC_SCSI)/scsimsg.c -m68020 -O2 -o $(SRC_SCSI)/z3660_scsi.device -Wall -Wextra -Wno-unused-parameter -fomit-frame-pointer -nostartfiles -lc -lamiga -ldebug
	/opt/amiga/bin/vasmm68k_mot -m68020 -Fhunk $(SYSINC_I) $(SRC_SCSI)/bootrom.s -o $(SRC_SCSI)/bootrom_uncut
	dd bs=1 skip=40 if=$(SRC_SCSI)/bootrom_uncut of=$(SRC_SCSI)/bootrom
	gcc $(SRC_SCSI)/makerom.c -o $(SRC_SCSI)/makerom
	cd $(SRC_SCSI) ; ./makerom
	$(RM) $(SRC_SCSI)/bootrom
	$(RM) $(SRC_SCSI)/makerom
	$(RM) $(SRC_SCSI)/bootrom_uncut

$(SRC_KICK31_060)/kick060.rom: $(SRC_KICK31_060)/kick31_060.asm
	/opt/amiga/bin/vasmm68k_mot -m68060 -Fbin $(SYSINC_I) $(SRC_KICK31_060)/kick31_060.asm -o $(SRC_KICK31_060)/kick060.rom

$(SRC_ETH)/Z3660Net.device: $(SRC_ETH)/deviceheader.c $(SRC_ETH)/deviceinit.c $(SRC_ETH)/device.c $(SRC_ETH)/device.h $(SRC_ETH)/compiler.h $(SRC_ETH)/macros.h $(SRC_ETH)/sana2.h $(SRC_ETH)/version.h
	$(GCC) -c $(ETH_COPTS) -o $(SRC_ETH)/deviceheader.o $(SRC_ETH)/deviceheader.c
	$(GCC) -c $(ETH_COPTS) -o $(SRC_ETH)/deviceinit.o $(SRC_ETH)/deviceinit.c
	$(GCC) -c $(ETH_COPTS) -o $(SRC_ETH)/device.o $(SRC_ETH)/device.c
	$(GCC) -s -nostartfiles -nostdlib -o $(SRC_ETH)/Z3660Net.device $(SRC_ETH)/deviceheader.o $(SRC_ETH)/deviceinit.o $(SRC_ETH)/device.o -ldebug -lgcc -lc -lamiga -ramiga-dev
	$(RM) $(SRC_ETH)/deviceheader.o
	$(RM) $(SRC_ETH)/deviceinit.asm
	$(RM) $(SRC_ETH)/deviceinit.o
	$(RM) $(SRC_ETH)/device.asm
	$(RM) $(SRC_ETH)/device.o

#$(SRC_ZTP)/ZTop: $(SRC_ZTP)/newZTop.c $(SRC_ZTP)/z3660_regs.h
#	$(GCC) $(SRC_ZTP)/newZTop.c -m68030 -O2 -o $(SRC_ZTP)/ZTop -Wall -Wextra -Wno-unused-parameter -L/opt/amiga/m68k-amigaos/ndk/lib/linker_libs -lamiga
#	$(CP) $(SRC_ZTP)/ZTop $(UAEPATH)
#	$(CP) $(SRC_ZTP)/ZTop.info $(UAEPATH)
$(SRC_ZTP)/ZTop: $(SRC_ZTP)/ZTop.c $(SRC_ZTP)/z3660_regs.h
	cd $(SRC_ZTP) ; $(SASC) ZTop.c
	$(CP) $(SRC_ZTP)/ZTop $(UAEPATH)
	$(CP) $(SRC_ZTP)/ZTop.info $(UAEPATH)

$(SRC_W3D)/Wazp3D_lib.o: $(SRC_W3D)/Wazp3D_lib.c
	$(GCC) -c $(SRC_W3D)/Wazp3D_lib.c $(WARP3D_COPTS) -O0 -o $(SRC_W3D)/Wazp3D_lib.o

$(SRC_W3D)/Wazp3D56.o: $(SRC_W3D)/Wazp3D56.c $(SRC_W3D)/soft3d/soft3D56.c $(SRC_W3D)/Wazp3D.h
	$(GCC) -c $(SRC_W3D)/Wazp3D56.c $(WARP3D_COPTS) -O0 -o $(SRC_W3D)/Wazp3D56.o

$(SRC_W3D)/soft3d/soft3D56.o: $(SRC_W3D)/soft3d/soft3D56.c $(SRC_W3D)/Wazp3D.h
	$(GCC) -c $(SRC_W3D)/soft3d/soft3D56.c $(WARP3D_COPTS) -O0 -o $(SRC_W3D)/soft3d/soft3D56.o

$(SRC_W3D)/Wazp3D.library: $(SRC_W3D)/Wazp3D_lib.o $(SRC_W3D)/Wazp3D56.o $(SRC_W3D)/soft3d/soft3D56.o
	$(GCC) $(SRC_W3D)/Wazp3D56.o $(SRC_W3D)/Wazp3D_lib.o $(SRC_W3D)/soft3d/soft3D56.o -o $(SRC_W3D)/Wazp3D.library -nostartfiles -lm
	$(CP) $(SRC_W3D)/Wazp3D.library $(UAEPATH)

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
	$(RM) $(SRC_KICK31_060)/kick060.rom

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
	xdftool 0_Z3660.adf write $(SRC_ZTP)/Disk.info
	xdftool 0_Z3660.adf write $(SRC_SCSI)/z3660_scsi.device Devs
	xdftool 0_Z3660.adf write $(SRC_W3D)/Wazp3D.library Libs/Warp3D.library + list

A4000: adf
	$(CP) 0_Z3660.adf /Volumes/A4000/0_Z3660.adf
	$(RM) 0_Z3660.adf
	diskutil eject /Volumes/A4000
	@echo "Ejected"

ZTop: $(SRC_ZTP)/ZTop