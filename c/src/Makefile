# $Id$
include ../../config.make

ifeq ($(JMAX_VERSION),24)

DISTDIR = ../../../../fts
PACKAGEROOT = ..
include $(DISTDIR)/Makefiles/Makefile.lib
include Sources
PACKAGELIB=$(LIBDIR)/lib$(PNAME).so

all: $(LIBDIR) $(OBJDIR)  $(PACKAGELIB)

TOCLEAN = $(PACKAGELIB)
include $(DISTDIR)/Makefiles/Makefile.lib.end
MODULE_SOFLAGS += $(GRIDFLOW_LDSOFLAGS)

else
ifeq ($(JMAX_VERSION),25)

DISTDIR = $(JMAXROOTDIR)/fts

PACKAGEROOT = ..
PACKAGELIB=$(LIBDIR)/lib$(PNAME).so
include $(JMAXDISTDIR)/Makefiles/Makefile.package

LDSOFLAGS += $(GRIDFLOW_LDSOFLAGS)

### for hardcore malloc debugging; standalone only
# LDSOFLAGS += -lefence

$(PACKAGELIB): $(OBJS)
	gcc $(LDSOFLAGS) -o $(PACKAGELIB) $(OBJS)

else
ifeq ($(JMAX_VERSION),none)

endif #none
endif #25
endif #24

### standalone stuff
include Sources
OBJS = $(addprefix ../obj2/,$(subst .c,.o,$(SOURCES)))
FIX_LD = LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH}
LIB = ../lib2/lib$(PNAME).so
# LIB2 = /home/matju/lib/$(LIB)

ARCH_CFLAGS += -Wall -Wno-unused -O6 -funroll-loops -g
ARCH_CFLAGS += -fdollars-in-identifiers
ARCH_CFLAGS += -fpic

standalone:: $(LIB)

clean-standalone::
	rm -f ../lib2/* ../obj2/*

../obj2/nofts.o: nofts.c nofts.h grid.h lang.h config.h Makefile
	@mkdir -p ../obj2
	gcc $(ARCH_CFLAGS) -DSTANDALONE -c $< -o $@

../obj2/%.o: %.c grid.h lang.h config.h nofts.h over_fts.h Makefile
	@mkdir -p ../obj2
	gcc $(ARCH_CFLAGS) -DSTANDALONE -c $< -o $@

$(LIB): $(OBJS) ../obj2/nofts.o Makefile
	@mkdir -p ../lib2
	gcc -shared $(OBJS) ../obj2/nofts.o -o $@ $(LDSOFLAGS)

#$(LIB2): $(LIB)
#	install --mode=755 $(LIB) $(LIB2)

test::
	(cd ../..; make test)

