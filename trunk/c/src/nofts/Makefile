# attempt to make v4j run without jmax:

ifndef ARCH
ARCH=i686-linux
endif
include ../../../config.make

# include ../../../config.make
# DISTDIR = $(JMAXROOTDIR)/fts
# PACKAGEROOT = ..
# PACKAGELIB=$(LIBDIR)/lib$(PNAME).so
# include $(JMAXDISTDIR)/Makefiles/Makefile.package

LDSOFLAGS += -L/usr/X11R6/lib -lX11 -lXext -lm
LDSOFLAGS += $(GRIDFLOW_LDSOFLAGS)
# LDSOFLAGS += -lefence # for hardcore malloc debugging.

ARCH_CFLAGS += -Wall -Wno-unused
ARCH_CFLAGS += -fno-unroll-all-loops -funroll-loops -g
ARCH_CFLAGS += -DSTANDALONE
ARCH_CFLAGS += -O6 -mpentium

include ../Sources

OBJS = $(subst .c,.o,$(SOURCES))

FIX_LD = LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH}

LIB = libgridflow.so
# LIB2 = /home/matju/lib/$(LIB)
all:: test1

nofts.o: nofts.c
	gcc $(ARCH_CFLAGS) -c $<

%.o: ../%.c ../grid.h ../lang.h ../config.h nofts.h
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
	$(FIX_LD) ./test1

mpeg_test: mpeg_test.c
	gcc $(ARCH_CFLAGS) mpeg_test.c libgridflow.so -o $@ -lmpeg
