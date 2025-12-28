# Makefile for Zynq bare-metal firmware (built on macOS)
# Generates firmware.elf and firmware.bin from sources in src/

CROSS_COMPILE ?= arm-none-eabi-
CC      := $(CROSS_COMPILE)gcc
AS      := $(CROSS_COMPILE)gcc
AR      := $(CROSS_COMPILE)ar
OBJCOPY := $(CROSS_COMPILE)objcopy
SIZE    := $(CROSS_COMPILE)size

ROOT    := $(abspath .)
SRCDIR  := $(ROOT)/src
BUILDDIR:= $(ROOT)/build
LDSCRIPT:= $(SRCDIR)/lscript.ld

# Optional: Xilinx BSP includes (found in design_1_wrapper)
BSP_INC := $(ROOT)/../design_1_wrapper/zynq_fsbl/zynq_fsbl_bsp/ps7_cortexa9_0/include

# LwIP from project first, then fallback to Xilinx standalone_domain
LWIP_INC := $(ROOT)/../design_1_wrapper/ps7_cortexa9_0/standalone_domain/bsp/ps7_cortexa9_0/libsrc/lwip213_v1_1/src/lwip-2.1.3/src/include
BSP_INC := $(ROOT)/../design_1_wrapper/ps7_cortexa9_0/standalone_domain/bsp/ps7_cortexa9_0/include
# BSP library path for standalone domain (core 0)
BSP_LIB := $(ROOT)/../design_1_wrapper/ps7_cortexa9_0/standalone_domain/bsp/ps7_cortexa9_0/lib

INCLUDES:= -I$(SRCDIR) -I$(SRCDIR) -I$(BSP_INC) -I$(LWIP_INC) -I$(BSP_INC) -I$(SRCDIR)/mbedtls/include -I$(ROOT)/../design_1_wrapper/zynq_fsbl

CFLAGS  := -mcpu=cortex-a9 -marm -mfpu=neon -mfloat-abi=hard -O3 -std=gnu99 -ftree-vectorize -mvectorize-with-neon-quad -ffast-math \
           -ffunction-sections -fdata-sections -Wall -Wextra \
           -MMD -MP # Automatic dependency generation
#           -fno-builtin \

ASFLAGS := $(CFLAGS)
# Use Xilinx runtime spec and link against BSP libs; remove -nostdlib
# Ensure linker uses the same FPU/ABI as objects to avoid VFP arg mismatches
LDFLAGS := -T $(LDSCRIPT) -Wl,--gc-sections -specs=$(SRCDIR)/Xilinx.spec -L$(BSP_LIB) \
           -mcpu=cortex-a9 -marm -mfpu=neon -mfloat-abi=hard -ffast-math -flto
# CRT0 and libraries from BSP
# Link against lwIP and FatFs (xilffs) in addition to libxil and libm.
# Grouping ensures mutual dependencies are resolved correctly.
LIBS := $(BSP_LIB)/xil-crt0.o \
        -Wl,--start-group,-lxil,-llwip4,-lxilffs,-lgcc,-lc,-lm,--end-group

# Collect sources and headers
C_SRCS := $(shell find $(SRCDIR) -type f -name "*.c")
H_SRCS := $(shell find $(SRCDIR) -type f -name "*.h")
# Exclude specific files from build
C_SRCS := $(filter-out $(SRCDIR)/mp3/stb_vorbis.c,$(C_SRCS))
S_SRCS := $(shell find $(SRCDIR) -type f -name "*.S")
SRCS   := $(C_SRCS) $(S_SRCS)
OBJS   := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(SRCS:.c=.o))
OBJS   := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(OBJS:.S=.o))

# Generate dependency files
DEPS   := $(OBJS:.o=.d)

# Special handling for alfa.txt dependency
ALFA_TXT := $(SRCDIR)/alfa.txt

ELF := $(BUILDDIR)/ps7_cortexa9_0.elf
BIN := $(BUILDDIR)/ps7_cortexa9_0.bin

.PHONY: all clean size bin elf

all: $(ELF) $(BIN)
	@echo "Generados: $(ELF) y $(BIN)"

elf: $(ELF)

bin: $(BIN)

# Automatic dependency generation
$(BUILDDIR)/%.d: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Generating $(notdir $@)"
	@$(CC) $(CFLAGS) $(INCLUDES) -MM -MT '$(BUILDDIR)/$*.o $@' $< -o $@

$(BUILDDIR)/%.d: $(SRCDIR)/%.S
	@mkdir -p $(dir $@)
	@echo "Generating $(notdir $@)"
	@$(AS) $(ASFLAGS) -MM -MT '$(BUILDDIR)/$*.o $@' $< -o $@

# Special rule for alfa.c
$(BUILDDIR)/alfa.o: $(SRCDIR)/alfa.c $(ALFA_TXT)
	@mkdir -p $(dir $@)
	@echo "Compiling alfa.c"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Generic compilation rules
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $(notdir $<)"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.S
	@mkdir -p $(dir $@)
	@echo "Compiling $(notdir $<)"
	@$(AS) $(ASFLAGS) -c $< -o $@

$(ELF): $(OBJS)
	@mkdir -p $(BUILDDIR)
	@echo "Linking Z3660 project"
	@$(CC) $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group $(LIBS) -o $@
	$(SIZE) $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILDDIR)

# Include dependency files
-include $(DEPS)

