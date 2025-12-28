# Top-level Makefile for Z3660 project on macOS using Wine bootgen
#
# Targets:
#   make all        -> fsbl + z3660 + z3660_emu + bootbin + version
#   make build      -> fsbl + z3660 + z3660_emu
#   make fsbl       -> build FSBL
#   make z3660      -> build main firmware
#   make z3660_emu  -> build emulator firmware (core1)
#   make bootbin    -> generate BOOT.BIN via Wine + bootgen.exe
#   make version    -> run make_version_alfa.py packaging script
#   make clean      -> clean app builds (FSBL/app dirs)
#
# Configure these if needed:
WINEPREFIX    := $(HOME)/.wine64
export WINEPREFIX
WINE          ?= MVK_CONFIG_LOG_LEVEL=0 WINEDEBUG=-all wine
# Set the path to bootgen.exe on your host system (e.g., macOS or Linux).
# Example: BOOTGEN_EXE_HOST ?= /opt/Xilinx/Vivado/2021.2/bin/bootgen.exe
# Example: BOOTGEN_EXE_HOST ?= $(HOME)/.wine/drive_c/Xilinx/bin/bootgen.exe
BOOTGEN_EXE_HOST := $(HOME)/.wine64/drive_c/Xilinx/bin/bootgen.exe

MAKE = @make

# Paths (Unix side)
ifeq ($(OS),Windows_NT)
	BIF_FILE = system.bif
else
   BIF_FILE = system_macos.bif
endif
BIF           := Z3660_system/Alfa/$(BIF_FILE)
BOOTBIN       := Z3660_system/Alfa/sd_card/BOOT.BIN

# Component directories
FSBL_DIR      := design_1_wrapper/zynq_fsbl
Z3660_DIR     := Z3660
Z3660_EMU_DIR := Z3660_emu

# Default target
.PHONY: all build fsbl z3660 z3660_em bootbin version
all: bootbin version

build: bsp fsbl z3660 z3660_emu

# DDR speed targets for FSBL
fsbl_ddr533: bsp
	$(MAKE) -C $(FSBL_DIR) DDR_SPEED=533 ddr533 -j4

fsbl_ddr667: bsp
	$(MAKE) -C $(FSBL_DIR) DDR_SPEED=667 ddr667 -j4

fsbl_ddr800: bsp
	$(MAKE) -C $(FSBL_DIR) DDR_SPEED=800 ddr800 -j4

fsbl: bsp
	$(MAKE) -C $(FSBL_DIR) DDR_SPEED=$(DDR_SPEED) clean
	$(MAKE) -C $(FSBL_DIR) DDR_SPEED=$(DDR_SPEED) ddr$(DDR_SPEED) -j4

z3660: bsp_cortexa9_0
	$(MAKE) -C $(Z3660_DIR) -j4

z3660_emu: bsp_cortexa9_1
	$(MAKE) -C $(Z3660_EMU_DIR) -j4

# Generate BOOT.BIN using Wine bootgen.exe
# Uses winepath to translate Unix paths to Windows paths for bootgen.exe
# DDR_SPEED variable can be set to 533, 667, or 800
DDR_SPEED ?= 533

# Build BSP libraries for both processors
bsp_cortexa9_0:
	$(MAKE) -C design_1_wrapper/ps7_cortexa9_0/standalone_domain/bsp

bsp_cortexa9_1:
	$(MAKE) -C design_1_wrapper/ps7_cortexa9_1/standalone_ps7_cortexa9_1/bsp

bsp: bsp_cortexa9_0 bsp_cortexa9_1

# Prepare for bootbin generation by ensuring fsbl.elf has correct DDR speed
bootbin_prepare: build
	cp $(FSBL_DIR)/fsbl_ddr$(DDR_SPEED).elf $(FSBL_DIR)/fsbl.elf

bootbin: bootbin_prepare
	@mkdir -p $(dir $(BOOTBIN))
	@echo "Generating BOOT.BIN via Wine bootgen.exe (DDR $(DDR_SPEED)MHz)"
	$(WINE) $(BOOTGEN_EXE_HOST) -arch zynq -image $(BIF) -o $(BOOTBIN) -w
	@echo "BOOT.BIN -> $(BOOTBIN)"

version:
	python3 Z3660_system/make_version_alfa.py

# Clean targets
clean:
	# Clean FSBL without building anything
	-@cd $(FSBL_DIR) && rm -rf *.o *.d fsbl.elf fsbl_ddr*.elf
	-rm -f $(FSBL_DIR)/zynq_fsbl_bsp/ps7_cortexa9_0/lib/libxil.a
	# Clean Z3660
	-@cd $(Z3660_DIR) && rm -rf build *.o *.d
	# Clean Z3660_emu  
	-@cd $(Z3660_EMU_DIR) && rm -rf build *.o *.d
	# Clean BSP libraries
	-@cd design_1_wrapper/ps7_cortexa9_0/standalone_domain/bsp && rm -rf *.o *.a
	-@cd design_1_wrapper/ps7_cortexa9_1/standalone_ps7_cortexa9_1/bsp && rm -rf *.o *.a
	# Clean additional Xilinx compiled libraries
	-@rm -rf design_1_wrapper/ps7_cortexa9_0/freertos10_xilinx_ps7_cortexa9_0/*.o
	-@rm -rf design_1_wrapper/ps7_cortexa9_0/freertos10_xilinx_ps7_cortexa9_0/*.a
	-@rm -rf design_1_wrapper/ps7_cortexa9_1/standalone_ps7_cortexa9_1/*.o
	-@rm -rf design_1_wrapper/ps7_cortexa9_1/standalone_ps7_cortexa9_1/*.a
	# Clean Xilinx BSP objects and archives recursively
	-@find design_1_wrapper -name "*.o" -delete
	-@find design_1_wrapper -name "*.d" -delete
	-@find design_1_wrapper -name "*.a" -delete
	-@find design_1_wrapper -name "build" -type d -exec rm -rf {} + 2>/dev/null || true
	@echo "Clean complete"

