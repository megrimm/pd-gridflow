# attempt to make v4j run without jmax:

ARCH=i686-linux

# include ../../../config.make
# DISTDIR = $(JMAXROOTDIR)/fts
# PACKAGEROOT = ..
# PACKAGELIB=$(LIBDIR)/lib$(PNAME).so
# include $(JMAXDISTDIR)/Makefiles/Makefile.package

LDSOFLAGS += -L/usr/X11R6/lib -lX11 -lXext -lm
# LDSOFLAGS += -lefence # for hardcore malloc debugging.

ARCH_CFLAGS += -Wall -Wno-unused
ARCH_CFLAGS += -fno-unroll-all-loops -funroll-loops -g
ARCH_CFLAGS += -DSTANDALONE
ARCH_CFLAGS += -O6 -mpentium

OBJS = \
dim.o bitpacking.o operator.o main.o io.o \
grid.o grid_basic.o grid_extra.o \
format_x11.o \
format_ppm.o \
format_targa.o \
format_grid.o \
format_videodev.o

LIB = libgridflow.so
# LIB2 = /home/matju/lib/$(LIB)
all:: test1

nofts.o: nofts.c
	gcc $(ARCH_CFLAGS) -c $<

%.o: ../%.c ../grid.h nofts.h ../config.h
	gcc $(ARCH_CFLAGS) -c $<

$(LIB): $(OBJS) nofts.o
	gcc -shared $(OBJS) nofts.o -o $@ $(LDSOFLAGS)

#$(LIB2): $(LIB)
#	install --mode=755 $(LIB) $(LIB2)

test1: test1.c $(LIB)
	gcc $(ARCH_CFLAGS) test1.c libgridflow.so -o $@

clean::
	rm libgridflow.so nofts.o $(OBJS)

test:: test1
	LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH} ./test1
