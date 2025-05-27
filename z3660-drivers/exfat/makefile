CPU  := m68k
OS   := amigaos
HOST := $(CPU)-$(OS)

CC     := /opt/amiga/bin/$(HOST)-gcc
AR     := /opt/amiga/bin/$(HOST)-ar
RANLIB := /opt/amiga/bin/$(HOST)-ranlib
RM     := rm -f

#------------------------------------------------------------------------------

STRIP		= /opt/amiga/bin/$(HOST)-strip
CHMOD		= chmod
CP			= cp

CFLAGS	= -O2 -Wall -m68020 -m68881 -ffast-math -fomit-frame-pointer -fno-builtin \
          -Wno-strict-aliasing -Wno-unused-variable -Wno-pointer-sign

LFLAGS	= -lm -noixemul -nostartfiles
CDEFS		= \
	-D__AMIGADATE__=\"$(shell date "+%d.%m.%y")\" \
	-D__YEAR__=\"$(shell date "+%Y")\" \
	-D__68881__ \
#	-D__READ_ONLY__

#------------------------------------------------------------------------------

NAME = exFATFileSystem

OFILES = \
	fs_startup.o \
	fs_disk.o \
	fs_lock.o \
	fs_main.o \
	fs_notify.o \
	fs_ops.o \
	fs_os3.o \
	fs_packet.o \
	fs_scsi.o \
	fs_string.o \
	fs_support.o \
	fs_timer.o \
	ef_cluster.o \
	ef_format.o \
	ef_io.o \
	ef_log.o \
	ef_lookup.o \
	ef_mount.o \
	ef_node.o \
	ef_time.o \
	ef_utf.o \
	ef_utils.o

#------------------------------------------------------------------------------

all: $(NAME)

$(NAME): $(OFILES)
	$(CC) $(OFILES) -o $@.debug $(LFLAGS)
	$(STRIP) --strip-all --strip-unneeded --remove-section .comment $@.debug -o $@
	$(CHMOD) +x $(NAME)

.c.o:
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

#------------------------------------------------------------------------------

clean:
	$(RM) -f $(OFILES) $(NAME) $(NAME).debug

install:
	$(CP) -f $(NAME) L:$(NAME)

uninstall:
	$(RM) -f L:$(NAME)

#------------------------------------------------------------------------------

