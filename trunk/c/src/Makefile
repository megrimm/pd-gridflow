# $Id$
include ../../config.make

ifeq ($(JMAX_VERSION),25)

DISTDIR = $(JMAXROOTDIR)/fts

PACKAGEROOT = ..
PACKAGELIB=$(LIBDIR)/lib$(PNAME).so
include $(JMAXDISTDIR)/Makefiles/Makefile.package

GRIDFLOW_LDSOFLAGS += -rdynamic
LDSOFLAGS += $(GRIDFLOW_LDSOFLAGS)

### for hardcore malloc debugging; standalone only
# LDSOFLAGS += -lefence

OBJS1 = $(addprefix $(OBJDIR)/,$(subst .c,.o,$(SOURCES)))

$(PACKAGELIB): $(OBJS1) $(OBJDIR)/bridge_jmax.o
	gcc $(LDSOFLAGS) -o $(PACKAGELIB) $(OBJS1) $(OBJDIR)/bridge_jmax.o

else
ifeq ($($JMAX_VERSION),30)

# then i don't know (yet)

else # JMAX_VERSION = none
endif #30
endif #25

### standalone stuff ####################################################

OBJS2 = $(addprefix ../obj2/,$(subst .c,.o,$(SOURCES)))
FIX_LD = LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH}
LIB = ../lib2/lib$(PNAME).so
# LIB2 = /home/matju/lib/$(LIB)

ARCH_CFLAGS += -Wall -Wno-unused -O6 -funroll-loops -g
ARCH_CFLAGS += -fdollars-in-identifiers
ARCH_CFLAGS += -fpic

standalone:: $(LIB)

clean-standalone::
	rm -f ../lib2/* ../obj2/*

../obj2/bridge_none.o: bridge_none.c bridge_none.h grid.h lang.h config.h Makefile
	@mkdir -p ../obj2
	gcc $(ARCH_CFLAGS) -DSTANDALONE -c $< -o $@

../obj2/%.o: %.c grid.h lang.h config.h bridge_none.h Makefile
	@mkdir -p ../obj2
	gcc $(ARCH_CFLAGS) -DSTANDALONE -c $< -o $@

$(LIB): $(OBJS2) ../obj2/bridge_none.o Makefile
	@mkdir -p ../lib2
	gcc -shared $(OBJS2) ../obj2/bridge_none.o -o $@ $(LDSOFLAGS)

test::
	(cd ../..; make test)

