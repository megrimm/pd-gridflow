# this file is no longer maintained
# copy appropriate new options from Makefile.25

include ../../config.make

ifeq ($(JMAX_VERSION),24)

PNAME=gridflow
DISTDIR = ../../../../fts
PACKAGEROOT = ..
include $(DISTDIR)/Makefiles/Makefile.lib
include Sources
PACKAGELIB=$(LIBDIR)/lib$(PNAME).so

all: $(LIBDIR) $(OBJDIR)  $(PACKAGELIB)

TOCLEAN = $(PACKAGELIB)
include $(DISTDIR)/Makefiles/Makefile.lib.end
MODULE_SOFLAGS += $(GRIDFLOW_LDSOFLAGS)
ARCH_CFLAGS += -Wall -Wno-unused -funroll-loops -g

else
ifeq ($(JMAX_VERSION),25)

DISTDIR = $(JMAXROOTDIR)/fts

PACKAGEROOT = ..
PACKAGELIB=$(LIBDIR)/lib$(PNAME).so
include $(JMAXDISTDIR)/Makefiles/Makefile.package

LDSOFLAGS += $(GRIDFLOW_LDSOFLAGS)

### for hardcore malloc debugging; standalone only
# LDSOFLAGS += -lefence

ARCH_CFLAGS += -Wall -Wno-unused
ARCH_CFLAGS += -fno-unroll-all-loops -funroll-loops -g

else
ifeq ($(JMAX_VERSION),none)

endif
endif
endif

### standalone stuff
include Sources
OBJS = $(addprefix ../obj2/,$(subst .c,.o,$(SOURCES)))
FIX_LD = LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH}
LIB = ../lib2/lib$(PNAME).so
# LIB2 = /home/matju/lib/$(LIB)

standalone:: $(LIB)

clean-standalone::
	rm -f ../lib2/* ../obj2/*

../obj2/nofts.o: nofts.c nofts.h grid.h lang.h config.h
	@mkdir -p ../obj2
	gcc $(ARCH_CFLAGS) -DSTANDALONE -c $< -o $@

../obj2/%.o: %.c grid.h lang.h config.h nofts.h
	@mkdir -p ../obj2
	gcc $(ARCH_CFLAGS) -DSTANDALONE -c $< -o $@

$(LIB): $(OBJS) ../obj2/nofts.o
	@mkdir -p ../lib2
	gcc -shared $(OBJS) ../obj2/nofts.o -o $@ $(LDSOFLAGS)

#$(LIB2): $(LIB)
#	install --mode=755 $(LIB) $(LIB2)

