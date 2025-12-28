# Z3660 USB Driver Makefile
# Real USB hardware device driver with ARM communication

# Compiler and tools
CC = /opt/amiga/bin/m68k-amigaos-gcc
STRIP = /opt/amiga/bin/m68k-amigaos-strip

# Compiler flags - like Ethernet driver
CFLAGS = -O2 -fomit-frame-pointer -Wall -Wno-pointer-sign -Wno-unused-function
CFLAGS += -mcpu=68040 -m68040 -D__USE_INLINE__ -D__USE_BASETYPE__
#CFLAGS += -DNEWSTYLE
CFLAGS += -noixemul
CFLAGS += -DDEVICENAME=z3660_usb.device
INCLUDES = -I. -I/opt/amiga/m68k-amigaos/include -I/opt/amiga/m68k-amigaos/ndk-include/

# Source files - main device driver
SOURCES = z3660_usb_device.c z3660_usb_arm.c z3660_debug.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Target device file
TARGET = z3660_usb.device

# Default target
all: $(TARGET)

# Build the device
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -nostartfiles -o $@ $(OBJECTS) -lamiga -ldebug -lc
#	$(STRIP) --strip-unneeded $@

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Install to datos (like the main driver)
install: $(TARGET)
	@echo "Installing $(TARGET) to /Volumes/datos/..."
	cp $(TARGET) /Volumes/datos/$(TARGET)
	@echo "Installed successfully"

# Show help
help:
	@echo "Z3660 USB Simulator Driver Build System"
	@echo "Available targets:"
	@echo "  all     - Build the device driver"
	@echo "  clean   - Clean build artifacts"
	@echo "  install - Install to Amiga Devs: directory"
	@echo "  help    - Show this help"

.PHONY: all clean install help
