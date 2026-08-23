# SPDX-License-Identifier: 0BSD

# ZTop -- Z3660 configuration utility (AmigaOS m68k target).
#
# The source includes ../common/z3660_regs.h, so the Docker build must mount
# the repository root, not just this directory.

# Plain assignment, not ?=: make predefines CC as "cc".
CC = m68k-amigaos-gcc
STRIP ?= m68k-amigaos-strip

AMIGA_PREFIX ?= /opt/amiga/m68k-amigaos
REACTION_LIB ?= /opt/amiga-6.5.0b/m68k-amigaos/vbcc/targets/m68k-amigaos/lib/reaction.lib
AMIGA_LIB    ?= /opt/amiga-6.5.0b/m68k-amigaos/ndk/lib/libs/amiga.lib

DOCKER_RUN = docker run --rm --user $$(id -u):$$(id -g) -v ..:/work -w /work/ZTop amiga-gcc:16.1

TARGET = ZTop
SRC    = Ztop.c
OBJ    = Ztop.o

CFLAGS  ?= -O2 -m68020 -m68881 -fomit-frame-pointer
CPPFLAGS = -DNO_INLINE_STDARG -I. -I$(AMIGA_PREFIX)/include
LDFLAGS  = -noixemul
PRINTF_FLOAT_FLAGS ?= -Wl,-u,_printf_float
LDLIBS   = $(REACTION_LIB) $(AMIGA_LIB) -lm -lgcc

all: $(TARGET)

$(TARGET): $(OBJ)
	@echo "LD  $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(PRINTF_FLOAT_FLAGS) -o $@ $(OBJ) $(LDLIBS)
	@$(STRIP) --strip-all $@

$(OBJ): $(SRC)
	@echo "CC  $<"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f $(OBJ) $(TARGET)

docker-build:
	@echo "DOCKER ZTop -> $(TARGET)"
	@$(DOCKER_RUN) make all

docker-clean:
	@$(DOCKER_RUN) make clean

.PHONY: all clean docker-build docker-clean