# $Id$
include ../../config.make

ifeq ($(JMAX_VERSION),25)

### Makefile.package begin

PACKAGEROOT = ..
PACKAGELIB=$(LIBDIR)/lib$(PNAME).so
DISTDIR = $(JMAXROOTDIR)/fts

ifndef MODE
MODE=opt
endif

include $(JMAXDISTDIR)/Makefiles/Makefile.$(ARCH)
include $(JMAXDISTDIR)/Makefiles/Makefile.$(MODE)
-include Sources

ifeq ($(strip $(OBJDIR)),)
OBJDIR  = $(PACKAGEROOT)/obj/$(ARCH)/$(MODE)
endif

ifeq ($(strip $(LIBDIR)),)
LIBDIR  = $(PACKAGEROOT)/lib/$(ARCH)/$(MODE)
endif

ifeq ($(strip $(BINDIR)),)
BINDIR  = $(PACKAGEROOT)/bin/$(ARCH)/$(MODE)
endif

OBJECTS = $(addprefix $(OBJDIR)/, $(addsuffix .o , $(basename $(SOURCES))))
CFLAGS = -I$(JMAXDISTDIR)/include/ $(MODULE_CFLAGS) $(MODE_CFLAGS) $(ARCH_CFLAGS) 
ARFLAGS = rc $(MODE_ARFLAGS) $(ARCH_ARFLAGS)
LDFLAGS =  $(MODULE_LDFLAGS) $(MODE_LDFLAGS) $(ARCH_LDFLAGS) 
OBJCFLAGS = 

FTS_LDFLAGS = $(MODULE_LDFLAGS) $(MODE_LDFLAGS) $(ARCH_LDFLAGS) $(FTS_ARCH_LDFLAGS) 
FTS_SOFLAGS = $(MODULE_SOFLAGS) $(MODE_SOFLAGS) $(ARCH_SOFLAGS) $(FTS_ARCH_SOFLAGS) 

TOCLEAN = $(PACKAGELIB)

##
## Rules
##

all: $(LIBDIR) $(OBJDIR) $(PACKAGELIB)
.PHONY: all

cleanobjs::
	rm -f $(OBJECTS)

clean:: cleanobjs
	rm -f $(TOCLEAN)

$(OBJDIR)/%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
.PRECIOUS: $(OBJDIR)/%.o

$(LIBDIR)/%.so: $(OBJECTS)
	$(LDSO) $(LDSOFLAGS) $(FTS_SOFLAGS) -o $@ $^ $(SOLIBS) $(MORESOLIBS)

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

### Makefile.package end

LDSOFLAGS += -rdynamic $(GRIDFLOW_LDSOFLAGS)

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

ARCH_CFLAGS += -Wall # for cleanliness
ARCH_CFLAGS += -Wno-unused # it's normal to have unused parameters
ARCH_CFLAGS += -Wno-strict-prototypes # Ruby has old-skool .h files
ARCH_CFLAGS += -O6 -funroll-loops # optimisation
ARCH_CFLAGS += -g # gdb info
ARCH_CFLAGS += -fdollars-in-identifiers # $ is the 28th letter
ARCH_CFLAGS += -fpic # some OSes need that for .so files

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
	gcc -shared $(LDSOFLAGS) $(OBJS2) ../obj2/bridge_none.o -o $@

test::
	(cd ../..; make test)

