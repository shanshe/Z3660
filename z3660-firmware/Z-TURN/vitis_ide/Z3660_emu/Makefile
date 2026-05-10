# Makefile for Zynq Cortex-A9 (Core 1) bare-metal app in Z3660_emu

CROSS_COMPILE ?= arm-none-eabi-
CC      := $(CROSS_COMPILE)gcc
CXX     := $(CROSS_COMPILE)g++
AS      := $(CROSS_COMPILE)gcc
AR      := $(CROSS_COMPILE)ar
OBJCOPY := $(CROSS_COMPILE)objcopy
SIZE    := $(CROSS_COMPILE)size

ROOT    := $(abspath .)
SRCDIR  := $(ROOT)/src
BUILDDIR:= $(ROOT)/build
LDSCRIPT:= $(SRCDIR)/lscript.ld

# Xilinx BSP include paths for Core 1 (ps7_cortexa9_1)
BSP_CORE1_INC := $(ROOT)/../design_1_wrapper/ps7_cortexa9_1/standalone_ps7_cortexa9_1/bsp/ps7_cortexa9_1/include
BSP_CORE1_LIB := $(dir $(BSP_CORE1_INC))lib
# Exported BSP include/lib provided by Vitis (flattened)
BSP_CORE1_EXPORT_INC := $(ROOT)/../design_1_wrapper/export/design_1_wrapper/sw/design_1_wrapper/standalone_ps7_cortexa9_1/bspinclude/include
BSP_CORE1_EXPORT_LIB := $(ROOT)/../design_1_wrapper/export/design_1_wrapper/sw/design_1_wrapper/standalone_ps7_cortexa9_1/bsplib/lib

INCLUDES:= -I$(SRCDIR) \
           -I$(SRCDIR)/uae/include \
           -I$(SRCDIR)/musashi \
           -I$(SRCDIR)/uae/machdep \
           -I$(SRCDIR)/uae \
           -I$(SRCDIR)/uae/jit \
           -I$(SRCDIR)/uae/include/uae \
           -I$(BSP_CORE1_INC) \
           -I$(BSP_CORE1_EXPORT_INC)

CFLAGS  := -mcpu=cortex-a9 -mfpu=neon-vfpv3 -mfloat-abi=hard -O3 -std=c++11 \
           -ffunction-sections -fdata-sections \
           -ffast-math -ftree-vectorize -mvectorize-with-neon-quad \
           -D__ARM_ARCH=7 -DARM_ASSEMBLY -DARMV6T2 -DARMV6_ASSEMBLY
#-Wall -Wextra -marm

# Additional C++ flags
CXXFLAGS := $(CFLAGS)
ASFLAGS := $(CFLAGS)
LDFLAGS := $(CFLAGS) -T $(LDSCRIPT) -specs=$(SRCDIR)/Xilinx.spec \
           -Wl,-build-id=none,-Map=$(BUILDDIR)/mapfile.txt \
           -flto -L$(BSP_CORE1_EXPORT_LIB) -L$(BSP_CORE1_LIB) -L$(BSP_CORE1_LIB)/cpu/cortexa9 -L$(dir $(BSP_CORE1_LIB))
#-Wl,--gc-sections
#-nostdlib -specs=nosys.specs 

# Sources (musashi/uae emulators)
C_SRCS   := $(shell find $(SRCDIR) -type f -name "*.c")
CXX_SRCS := $(shell find $(SRCDIR) -type f \( -name "*.cc" -o -name "*.cpp" -o -name "*.cxx" \))
# Exclude specific files
C_SRCS   := $(filter-out $(SRCDIR)/musashi/m68kfpu.c,$(C_SRCS))
CXX_SRCS := $(filter-out \
	$(SRCDIR)/uae/fpp_native.cpp \
	$(SRCDIR)/uae/jit/codegen_arm.cpp \
	$(SRCDIR)/uae/jit/compemu_midfunc_arm.cpp \
	$(SRCDIR)/uae/jit/compemu_midfunc_arm2.cpp,$(CXX_SRCS))
S_SRCS := $(shell find $(SRCDIR) -type f -name "*.S")
SRCS   := $(C_SRCS) $(CXX_SRCS) $(S_SRCS)
OBJS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(C_SRCS))
OBJS += $(patsubst $(SRCDIR)/%.cc,$(BUILDDIR)/%.o,$(filter %.cc,$(CXX_SRCS)))
OBJS += $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(filter %.cpp,$(CXX_SRCS)))
OBJS += $(patsubst $(SRCDIR)/%.cxx,$(BUILDDIR)/%.o,$(filter %.cxx,$(CXX_SRCS)))
OBJS += $(patsubst $(SRCDIR)/%.S,$(BUILDDIR)/%.o,$(S_SRCS))

ELF := $(BUILDDIR)/Z3660_emu.elf
BIN := $(BUILDDIR)/Z3660_emu.bin

.PHONY: all clean size bin elf

all: $(ELF) $(BIN)
	@echo "Generated: $(ELF) and $(BIN)"

elf: $(ELF)

bin: $(BIN)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $(notdir $<)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cc
	@mkdir -p $(dir $@)
	@echo "Compiling $(notdir $<)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $(notdir $<)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cxx
	@mkdir -p $(dir $@)
	@echo "Compiling $(notdir $<)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.S
	@mkdir -p $(dir $@)
	@echo "Compiling $(notdir $<)"
	@$(AS) $(ASFLAGS) -c $< -o $@

# Link libraries: Xilinx BSP, C/C++ stdlibs, math. Include BSP's crt0.
# Xilinx standalone BSP typically produces libxil.a and xil-crt0.o
LIBS := $(BSP_CORE1_LIB)/xil-crt0.o \
        -Wl,--start-group,-lxil,-lgcc,-lstdc++,-lm,-lc,--end-group

$(ELF): $(OBJS)
	@mkdir -p $(BUILDDIR)
	@echo "Linking Z3660_emu project"
	@$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@
	$(SIZE) $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILDDIR)

