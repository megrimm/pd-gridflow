# attempt to make v4j run without jmax:

ARCH=i686-linux

# include ../../../config.make
# DISTDIR = $(JMAXROOTDIR)/fts
# PACKAGEROOT = ..
# PACKAGELIB=$(LIBDIR)/lib$(PNAME).so
# include $(JMAXDISTDIR)/Makefiles/Makefile.package

LDSOFLAGS += -L/usr/X11R6/lib -lX11 -lXext -lm

ARCH_CFLAGS += -Wall -Wno-unused
ARCH_CFLAGS += -fno-unroll-all-loops -funroll-loops -g
ARCH_CFLAGS += -DSTANDALONE

OBJS = \
dim.o bitpacking.o operator.o video4jmax.o io.o \
grid.o grid_basic.o grid_extra.o \
format_x11.o \
format_ppm.o \
format_targa.o \
format_grid.o \
format_videodev.o

LIB = libvideo4jmax.so

all:: test1

nofts.o: nofts.c
	gcc $(ARCH_CFLAGS) -c $<

%.o: ../%.c ../grid.h nofts.h ../config.h
	gcc $(ARCH_CFLAGS) -c $<

$(LIB): $(OBJS) nofts.o
	gcc -shared $(OBJS) nofts.o -o $@ $(LDSOFLAGS)

test1: test1.c $(LIB)
	gcc $(ARCH_CFLAGS) test1.c $(LIB) -o $@

clean::
	rm libvideo4jmax.so nofts.o $(OBJS)
