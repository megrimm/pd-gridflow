# $Id$
include config.make

all:: c-src-all # doc-all java-all

standalone:: c-src-standalone

test::
	(cd tests; $(MAKE) -k test)

clean:: c-src-clean c-src-clean-standalone

install::
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt
	( \
		cd c/lib/$(ARCH)/opt ; \
		$(INSTALL_LIB) lib$(PNAME).so $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt/lib$(PNAME).so ;\
	)
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)
	$(INSTALL_DATA) $(PNAME).jpk $(lib_install_dir)/packages/$(PNAME)/$(PNAME).jpk
	$(INSTALL_DATA) $(PNAME).scm $(lib_install_dir)/packages/$(PNAME)/$(PNAME).scm
	(cd help; $(MAKE) $@)
	(cd templates; $(MAKE) $@)
	#(cd java; $(MAKE) $@)

kloc::
	wc c/src/*.[ch] configure extra/*.rb




ifeq ($(JMAX_VERSION),25)

### Makefile.package begin

PACKAGEROOT = c
PACKAGELIB=$(LIBDIR)/lib$(PNAME).so
DISTDIR = $(JMAXROOTDIR)/fts

ifndef MODE
MODE=opt
endif

include $(JMAXDISTDIR)/Makefiles/Makefile.$(ARCH)
include $(JMAXDISTDIR)/Makefiles/Makefile.$(MODE)

ifeq ($(strip $(OBJDIR)),)
OBJDIR  = $(PACKAGEROOT)/obj/$(ARCH)/$(MODE)
endif

ifeq ($(strip $(LIBDIR)),)
LIBDIR  = $(PACKAGEROOT)/lib/$(ARCH)/$(MODE)
endif

ifeq ($(strip $(BINDIR)),)
BINDIR  = $(PACKAGEROOT)/bin/$(ARCH)/$(MODE)
endif

ifeq ($(strip $(SRCDIR)),)
SRCDIR  = $(PACKAGEROOT)/src
endif

OBJECTS = $(addprefix $(OBJDIR)/, $(addsuffix .o , $(basename $(SOURCES))))
CFLAGS = -I$(JMAXDISTDIR)/include/ $(MODULE_CFLAGS) $(MODE_CFLAGS) $(ARCH_CFLAGS) 
ARFLAGS = rc $(MODE_ARFLAGS) $(ARCH_ARFLAGS)
LDFLAGS =  $(MODULE_LDFLAGS) $(MODE_LDFLAGS) $(ARCH_LDFLAGS) 
OBJCFLAGS = 

FTS_LDFLAGS = $(MODULE_LDFLAGS) $(MODE_LDFLAGS) $(ARCH_LDFLAGS) $(FTS_ARCH_LDFLAGS) 
FTS_SOFLAGS = $(MODULE_SOFLAGS) $(MODE_SOFLAGS) $(ARCH_SOFLAGS) $(FTS_ARCH_SOFLAGS) 

TOCLEAN = $(PACKAGELIB)

c-src-all:: $(LIBDIR) $(OBJDIR) $(PACKAGELIB)

c-src-cleanobjs::
	rm -f $(OBJS1)

c-src-clean:: c-src-cleanobjs
	rm -f $(TOCLEAN)

$(OBJDIR)/c_src_%.o : $(SRCDIR)/%.c
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

OBJS1 = $(addprefix $(OBJDIR)/,$(subst /,_,$(subst .c,.o,$(SOURCES))))
OBJS1 += $(OBJDIR)/c_src_bridge_jmax.o

$(PACKAGELIB): $(OBJS1)
	gcc $(LDSOFLAGS) -o $(PACKAGELIB) $(OBJS1)

ARCH_CFLAGS += -Wall # for cleanliness
ARCH_CFLAGS += -Wno-unused # it's normal to have unused parameters
ARCH_CFLAGS += -Wno-strict-prototypes # Ruby has old-skool .h files
ARCH_CFLAGS += -O6 -funroll-loops # optimisation
ARCH_CFLAGS += -g # gdb info
ARCH_CFLAGS += -fdollars-in-identifiers # $ is the 28th letter
ARCH_CFLAGS += -fpic # some OSes need that for .so files

endif #25

### standalone stuff ####################################################

OBJS2 = $(addprefix ../obj2/,$(subst .c,.o,$(SOURCES)))
FIX_LD = LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH}
LIB = ../lib2/lib$(PNAME).so
# LIB2 = /home/matju/lib/$(LIB)

c-src-standalone:: $(LIB)

c-src-clean-standalone::
	rm -f ../lib2/* ../obj2/*

c/obj2/bridge_none.o: bridge_none.c bridge_none.h grid.h lang.h config.h Makefile
	@mkdir -p ../obj2
	gcc $(ARCH_CFLAGS) -DSTANDALONE -c $< -o $@

c/obj2/%.o: %.c grid.h lang.h config.h bridge_none.h Makefile
	@mkdir -p ../obj2
	gcc $(ARCH_CFLAGS) -DSTANDALONE -c $< -o $@

$(LIB): $(OBJS2) ../obj2/bridge_none.o Makefile
	@mkdir -p ../lib2
	gcc -shared $(LDSOFLAGS) $(OBJS2) ../obj2/bridge_none.o -o $@

