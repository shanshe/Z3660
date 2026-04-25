.PHONY: all build bootbin images fsbl z3660 z3660_emu bsp clean

FIRMWARE_DIR := z3660-firmware/Z-TURN/vitis_ide

all build bootbin images fsbl z3660 z3660_emu bsp clean:
	$(MAKE) -C $(FIRMWARE_DIR) $@
