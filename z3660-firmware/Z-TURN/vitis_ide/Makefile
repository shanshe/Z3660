# Top-level Makefile for Z3660 project on macOS
#
# Targets:
#   make all        -> z3660 + z3660_emu + bootbin + version (fsbl only if needed)
#   make build      -> z3660 + z3660_emu
#   make fsbl       -> build FSBL (only if not exists)
#   make z3660      -> build main firmware
#   make z3660_emu  -> build emulator firmware (core1)
#   make bootbin    -> generate BOOT.BIN
#   make version    -> run make_version_alfa.py packaging script
#   make clean      -> clean app builds (FSBL/app dirs)
#

# Set the path to bootgen.exe on your host system (e.g., macOS or Linux).
BOOTGEN_EXE_HOST := $(HOME)/git/zynq-mkbootimage/mkbootimage
MAKE = make

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
.PHONY: all build z3660 z3660_emu bootbin version bsp_cortexa9_0 bsp_cortexa9_1
# Default: build firmware only to avoid rebuilding FSBL/BOOT every `make`
all: bootbin version

build: bsp_cortexa9_0 bsp_cortexa9_1 z3660 z3660_emu

# DDR speed targets for FSBL
fsbl_ddr533: $(FSBL_DIR)/fsbl_ddr533.elf

$(FSBL_DIR)/fsbl_ddr533.elf: zynq_fsbl_bsp
	@$(MAKE) -C $(FSBL_DIR) DDR_SPEED=533 -j 4

fsbl_ddr667: $(FSBL_DIR)/fsbl_ddr667.elf

$(FSBL_DIR)/fsbl_ddr667.elf: zynq_fsbl_bsp
	@$(MAKE) -C $(FSBL_DIR) DDR_SPEED=667 ddr667 -j 4

fsbl_ddr800: $(FSBL_DIR)/fsbl_ddr800.elf

$(FSBL_DIR)/fsbl_ddr800.elf: zynq_fsbl_bsp
	@$(MAKE) -C $(FSBL_DIR) DDR_SPEED=800 ddr800 -j 4

fsbl: $(FSBL_DIR)/fsbl_ddr$(DDR_SPEED).elf

$(FSBL_DIR)/fsbl_ddr$(DDR_SPEED).elf: zynq_fsbl_bsp
	@$(MAKE) -C $(FSBL_DIR) DDR_SPEED=$(DDR_SPEED) -j 4

z3660: bsp_cortexa9_0
	@$(MAKE) -C $(Z3660_DIR) -j 4

z3660_emu: bsp_cortexa9_1
	@$(MAKE) -C $(Z3660_EMU_DIR) -j 4

# DDR_SPEED variable can be set to 533, 667, or 800
DDR_SPEED ?= 533

# Build BSP libraries for both processors
bsp_cortexa9_0:
	@if [ ! -f design_1_wrapper/ps7_cortexa9_0/standalone_domain/bsp/ps7_cortexa9_0/lib/libxil.a ]; then \
		$(MAKE) -C design_1_wrapper/ps7_cortexa9_0/standalone_domain/bsp; \
	fi

bsp_cortexa9_1:
	@if [ ! -f design_1_wrapper/ps7_cortexa9_1/standalone_ps7_cortexa9_1/bsp/ps7_cortexa9_1/lib/libxil.a ]; then \
		$(MAKE) -C design_1_wrapper/ps7_cortexa9_1/standalone_ps7_cortexa9_1/bsp; \
	fi

zynq_fsbl_bsp:
	@if [ ! -f design_1_wrapper/zynq_fsbl/fsbl.elf ]; then \
		$(MAKE) -C design_1_wrapper/zynq_fsbl; \
	fi
bsp: bsp_cortexa9_0 bsp_cortexa9_1

# Prepare for bootbin generation by ensuring fsbl.elf has correct DDR speed
bootbin_prepare: build $(FSBL_DIR)/fsbl_ddr$(DDR_SPEED).elf
	cp $(FSBL_DIR)/fsbl_ddr$(DDR_SPEED).elf $(FSBL_DIR)/fsbl.elf

bootbin: bootbin_prepare
	@mkdir -p $(dir $(BOOTBIN))
	@echo "Generating BOOT.BIN (DDR $(DDR_SPEED)MHz)"
	$(BOOTGEN_EXE_HOST) $(BIF) $(BOOTBIN)
	@echo "BOOT.BIN -> $(BOOTBIN)"

version:
	python3 Z3660_system/make_version_alfa.py

# Clean targets
clean:
	# Cleaning FSBL
	-@cd $(FSBL_DIR) && rm -rf *.o *.d fsbl.elf fsbl_ddr*.elf
	-@cd $(FSBL_DIR)/zynq_fsbl_bsp/ps7_cortexa9_0/lib && rm -f *.o *.a *.d
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
	# Clean complete

