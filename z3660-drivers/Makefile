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

SASC = /opt/homebrew/bin/vamos sc

SRC_AHI  = ahi
SRC_MHI  = mhi
SRC_RTG  = rtg
SRC_ETH  = eth
SRC_ZTP  = ZTop
SRC_ZAC  = z3660_autoconfig
SRC_W3D  = Z3660_Wazp3D
SRC_SCSI = scsi
SRC_MPG  = mpg
SRC_USB  = usb
SRC_KICK31_060 = kick31_060
SRC_FLP  = floppy
SRC_ROM  = makerom

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

RTG_CFLAGS = -v -k -c99 -O2 -cpu=$(CPU) -fpu=$(FPU)

.PHONY: all
all: clean $(SRC_RTG)/Z3660.card $(SRC_ETH)/Z3660Net.device $(SRC_ZTP)/ZTop $(SRC_ZAC)/z3660_autoconfig $(SRC_W3D)/Wazp3D.library $(SRC_AHI)/z3660ax.audio $(SRC_MHI)/mhiz3660.library $(SRC_SCSI)/z3660_scsi.device $(SRC_KICK31_060)/kick060.rom $(SRC_MPG)/mpeg_streamer $(SRC_USB)/z3660_usb.device $(SRC_FLP)/z3660_floppy.device adf

$(SRC_RTG)/Z3660.card: $(SRC_RTG)/gfx.c $(SRC_RTG)/gfx.h $(SRC_RTG)/rtg.h common/z3660_regs.h
	$(GCC) $(SRC_RTG)/gfx.c -m68020 -mtune=68020-60 -O1 -o $(SRC_RTG)/Z3660.card -noixemul -Wall -Wextra -nostartfiles -lamiga -DDMARTG -ldebug
#	$(RM) $(SRC_RTG)/gfx.asm
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

$(SRC_MPG)/mpeg_streamer: $(SRC_MPG)/mpeg_streamer.c
	make -C $(SRC_MPG)

$(SRC_SCSI)/z3660_scsi.device: $(SRC_SCSI)/z3660_scsi.c $(SRC_SCSI)/z3660_scsi_enums.h $(SRC_SCSI)/makerom.c $(SRC_SCSI)/bootrom.asm
	$(GCC) $(SRC_SCSI)/z3660_scsi.c -m68020 -O2 -o $(SRC_SCSI)/z3660_scsi.device -Wall -Wextra -Wno-unused-parameter -fomit-frame-pointer -nostartfiles -lc -lamiga -ldebug
	/opt/amiga/bin/vasmm68k_mot -m68020 -Fhunk $(SYSINC_I) $(SRC_SCSI)/bootrom.asm -o $(SRC_SCSI)/bootrom_uncut
	dd bs=1 skip=40 if=$(SRC_SCSI)/bootrom_uncut of=$(SRC_SCSI)/bootrom
	$(RM) $(SRC_SCSI)/bootrom_uncut
	gcc $(SRC_SCSI)/makerom.c -o $(SRC_SCSI)/makerom
	cd $(SRC_SCSI) ; ./makerom
	$(RM) $(SRC_SCSI)/makerom
#	cp $(SRC_SCSI)/z3660_scsi.device $(SRC_ROM)

$(SRC_FLP)/z3660_floppy.device: $(SRC_FLP)/z3660_floppy.c $(SRC_FLP)/z3660_floppy.h $(SRC_FLP)/z3660_floppy_enums.h $(SRC_FLP)/newstyle.h common/z3660_regs.h
	$(GCC) $(SRC_FLP)/z3660_floppy.c -m68020 -O2 -o $(SRC_FLP)/z3660_floppy.device -Wall -Wextra -Wno-unused-parameter -fomit-frame-pointer -nostartfiles -lc -lamiga -ldebug -I$(SRC_FLP)
#	cp $(SRC_FLP)/z3660_floppy.device $(SRC_ROM)

#$(SRC_ROM)/z3660_scsi.rom: $(SRC_ROM)/makerom.c $(SRC_ROM)/bootrom.asm $(SRC_SCSI)/z3660_scsi.device $(SRC_FLP)/z3660_floppy.device
#	gcc $(SRC_ROM)/makerom.c -o $(SRC_ROM)/makerom
#	/opt/amiga/bin/vasmm68k_mot -m68020 -Fhunk $(SYSINC_I) $(SRC_ROM)/bootrom.asm -o $(SRC_ROM)/bootrom_uncut
#	dd bs=1 skip=40 if=$(SRC_ROM)/bootrom_uncut of=$(SRC_ROM)/bootrom
#	$(RM) $(SRC_ROM)/bootrom_uncut
#	cd $(SRC_ROM) && ./makerom
#	$(RM) $(SRC_ROM)/makerom

$(SRC_USB)/z3660_usb.device: $(SRC_USB)/z3660_usb_device.c $(SRC_USB)/z3660_usb.h
	$(GCC) -I$(SRC_USB) -m68020 -mtune=68020-60 -msoft-float -s -Wall -Wextra -Wno-unused-parameter -fomit-frame-pointer -nostartfiles -O2 -I. -o $(SRC_USB)/z3660_usb.device $(SRC_USB)/z3660_usb_device.c -lgcc -lc

$(SRC_KICK31_060)/kick060.rom: $(SRC_KICK31_060)/kick31_060.asm
	/opt/amiga/bin/vasmm68k_mot -m68060 -Fbin $(SYSINC_I) $(SRC_KICK31_060)/kick31_060.asm -o $(SRC_KICK31_060)/kick060.rom
	/opt/amiga/bin/vasmm68k_mot -m68060 -Fbin $(SYSINC_I) $(SRC_KICK31_060)/kick31_060.asm -o $(SRC_KICK31_060)/kick060_z3660scsi.rom -DEXP_MODULE_ENABLED=1
	/opt/amiga/bin/vasmm68k_mot -m68060 -Fbin $(SYSINC_I) $(SRC_KICK31_060)/kick31_060.asm -o $(SRC_KICK31_060)/kick060_noscsi.rom -DNOSCSI_MODULE_ENABLED=1
	/opt/amiga/bin/vasmm68k_mot -m68060 -Fbin $(SYSINC_I) $(SRC_KICK31_060)/kick31_060.asm -o $(SRC_KICK31_060)/kick060_z3660scsi_noscsi.rom -DEXP_MODULE_ENABLED=1 -DNOSCSI_MODULE_ENABLED=1

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

$(SRC_ZTP)/ZTop: $(SRC_ZTP)/ZTop.c common/z3660_regs.h
	cd $(SRC_ZTP) ; $(SASC) ZTop.c
#	$(CP) $(SRC_ZTP)/ZTop $(UAEPATH)
#	$(CP) $(SRC_ZTP)/ZTop.info $(UAEPATH)

$(SRC_ZAC)/z3660_autoconfig: $(SRC_ZAC)/z3660_autoconfig.c common/z3660_regs.h
	cd $(SRC_ZAC) ; $(SASC) z3660_autoconfig.c
#	cd $(SRC_ZAC) && $(GCC) -O2 -DNO_INLINE_STDARG -o z3660_autoconfig z3660_autoconfig.c -L/opt/amiga/m68k-amigaos/lib -lreaction /opt/amiga/m68k-amigaos/ndk/lib/libs/amiga.lib -noixemul -m68020 -m68881 -lgcc

$(SRC_W3D)/Wazp3D_lib.o: $(SRC_W3D)/Wazp3D_lib.c
	$(GCC) -c $(SRC_W3D)/Wazp3D_lib.c $(WARP3D_COPTS) -O0 -o $(SRC_W3D)/Wazp3D_lib.o

$(SRC_W3D)/Wazp3D56.o: $(SRC_W3D)/Wazp3D56.c $(SRC_W3D)/soft3d/soft3D56.c $(SRC_W3D)/Wazp3D.h
	$(GCC) -c $(SRC_W3D)/Wazp3D56.c $(WARP3D_COPTS) -O0 -o $(SRC_W3D)/Wazp3D56.o

$(SRC_W3D)/soft3d/soft3D56.o: $(SRC_W3D)/soft3d/soft3D56.c $(SRC_W3D)/Wazp3D.h
	$(GCC) -c $(SRC_W3D)/soft3d/soft3D56.c $(WARP3D_COPTS) -O0 -o $(SRC_W3D)/soft3d/soft3D56.o

$(SRC_W3D)/Wazp3D.library: $(SRC_W3D)/Wazp3D_lib.o $(SRC_W3D)/Wazp3D56.o $(SRC_W3D)/soft3d/soft3D56.o
	$(GCC) $(SRC_W3D)/Wazp3D56.o $(SRC_W3D)/Wazp3D_lib.o $(SRC_W3D)/soft3d/soft3D56.o -o $(SRC_W3D)/Wazp3D.library -nostartfiles -lm
	$(CP) $(SRC_W3D)/Wazp3D.library $(UAEPATH)

clean: clean_scsi clean_floppy clean_kickrom clean_eth clean_ztop clean_z3660_autoconfig clean_rtg clean_soft3d clean_mhi clean_ahi clean_mpg clean_usb clean_rom
	@echo cleaning...

.PHONY: clean_rom
clean_rom:
	$(RM) $(SRC_ROM)/z3660_scsi.rom
	$(RM) $(SRC_ROM)/bootrom
	$(RM) $(SRC_ROM)/makerom
	$(RM) $(SRC_ROM)/bootrom_uncut

.PHONY: clean_usb
clean_usb:
	$(RM) $(SRC_USB)/z3660_usb.device
	$(RM) $(SRC_USB)/*.o

.PHONY: clean_rtg
clean_rtg:
	$(RM) $(SRC_RTG)/Z3660.card

.PHONY: clean_scsi
clean_scsi:
	$(RM) $(SRC_SCSI)/z3660_scsi.device
	$(RM) $(SRC_SCSI)/z3660_scsi.rom
	$(RM) $(SRC_SCSI)/*.o
	$(RM) $(SRC_SCSI)/bootrom

.PHONY: clean_floppy
clean_floppy:
	$(RM) $(SRC_FLP)/z3660_floppy.device
	$(RM) $(SRC_FLP)/*.o
	$(RM) $(SRC_FLP)/bootrom

.PHONY: clean_eth
clean_eth:
	$(RM) $(SRC_ETH)/Z3660Net.device

.PHONY: clean_ztop
clean_ztop:
	$(RM) $(SRC_ZTP)/ZTop

.PHONY: clean_z3660_autoconfig
clean_z3660_autoconfig:
	$(RM) $(SRC_ZAC)/z3660_autoconfig

.PHONY: clean_kickrom
clean_kickrom:
	$(RM) $(SRC_KICK31_060)/kick060.rom
	$(RM) $(SRC_KICK31_060)/kick060_scsi.rom
	$(RM) $(SRC_KICK31_060)/kick060_z3660scsi.rom
	$(RM) $(SRC_KICK31_060)/kick060_z3660scsi_noscsi.rom

.PHONY: clean_soft3d
clean_soft3d:
	$(RM) $(SRC_W3D)/Wazp3D56.o
	$(RM) $(SRC_W3D)/Wazp3D_lib.o
	$(RM) $(SRC_W3D)/Wazp3D.library

.PHONY: clean_mhi
clean_mhi:
	$(RM) $(SRC_MHI)/axmp3
	$(RM) $(SRC_MHI)/mhiz3660.library

.PHONY: clean_mpg
clean_mpg:
	$(RM) $(SRC_MPG)/mpeg_streamer.o
	$(RM) $(SRC_MPG)/mpeg_streamer

.PHONY: clean_ahi
clean_ahi:
	$(RM) $(SRC_AHI)/z3660ax.audio
	$(RM) $(SRC_AHI)/Z3660AX

adf: all
	@echo creating adf...
	$(RM)   0_Z3660.adf
	xdftool 0_Z3660.adf format z3660Drivers + makedir C + makedir Libs + makedir Libs/Picasso96 + makedir Libs/MHI + makedir Devs + makedir Devs/Monitors + makedir Devs/Networks + makedir Devs/AudioModes + makedir Devs/USBHardware + makedir Devs/AHI

	xdftool 0_Z3660.adf write $(SRC_RTG)/Z3660.card Libs/Picasso96
	xdftool 0_Z3660.adf write $(SRC_RTG)/Monitors/Z3660 Devs/Monitors
	xdftool 0_Z3660.adf write $(SRC_RTG)/Monitors/Z3660.info Devs/Monitors
	xdftool 0_Z3660.adf write $(SRC_RTG)/p96-z3660-hdmi Devs
	xdftool 0_Z3660.adf write $(SRC_AHI)/z3660ax.audio Devs/AHI
	xdftool 0_Z3660.adf write $(SRC_AHI)/Z3660AX Devs/AudioModes
	xdftool 0_Z3660.adf write $(SRC_MHI)/mhiz3660.library Libs/MHI
	xdftool 0_Z3660.adf write $(SRC_MHI)/axmp3 C
	xdftool 0_Z3660.adf write $(SRC_MPG)/mpeg_streamer C
	xdftool 0_Z3660.adf write $(SRC_ETH)/Z3660Net.device Devs/Networks
	xdftool 0_Z3660.adf write $(SRC_ZAC)/z3660_autoconfig C
	xdftool 0_Z3660.adf write $(SRC_ZTP)/ZTop
	xdftool 0_Z3660.adf write $(SRC_ZTP)/ZTop.info
	xdftool 0_Z3660.adf write $(SRC_ZTP)/Disk.info
	xdftool 0_Z3660.adf write $(SRC_SCSI)/z3660_scsi.device Devs
	xdftool 0_Z3660.adf write $(SRC_FLP)/z3660_floppy.device Devs
	xdftool 0_Z3660.adf write $(SRC_USB)/z3660_usb.device Devs/USBHardware
	xdftool 0_Z3660.adf write $(SRC_W3D)/Wazp3D.library Libs/Warp3D.library + list

A4000: adf
	$(CP) 0_Z3660.adf /Volumes/A4000/0_Z3660.adf
	$(RM) 0_Z3660.adf
	diskutil eject /Volumes/A4000
	@echo "Ejected"

rtg: clean_rtg $(SRC_RTG)/Z3660.card

scsi: clean_scsi $(SRC_SCSI)/z3660_scsi.device

floppy: clean_floppy $(SRC_FLP)/z3660_floppy.device

eth: clean_eth $(SRC_ETH)/Z3660Net.device

ZTop: clean_ztop $(SRC_ZTP)/ZTop

autoconfig: clean_z3660_autoconfig $(SRC_ZAC)/z3660_autoconfig

kickrom: clean_kickrom $(SRC_KICK31_060)/kick060.rom

soft3d: clean_soft3d $(SRC_W3D)/Wazp3D.library

mhi: clean_mhi $(SRC_MHI)/axmp3 $(SRC_MHI)/mhiz3660.library

mpg: clean_mpg $(SRC_MPG)/mpeg_streamer $(SRC_MPG)/mpeg_streamer.h

ahi: clean_ahi $(SRC_AHI)/z3660ax.audio $(SRC_AHI)/Z3660AX

usb: clean_usb $(SRC_USB)/z3660_usb.device

rom: clean_rom $(SRC_SCSI)/z3660_scsi.device $(SRC_FLP)/z3660_floppy.device $(SRC_ROM)/z3660_scsi.rom
