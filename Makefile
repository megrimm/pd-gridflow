# $Id$
include config.make

# GridFlow Installation Directory
GFID = $(lib_install_dir)/packages/$(PNAME)
PACKAGELIB = $(LIBDIR)/lib$(PNAME).so

all:: c-src-all # doc-all

standalone:: c-src-standalone

test::
	(cd tests; $(MAKE) test)

clean:: c-src-clean c-src-clean-standalone

install::
	$(INSTALL_DIR) $(GFID)
	$(INSTALL_DIR) $(GFID)/c/lib/$(ARCH)/opt
	$(INSTALL_LIB) c/lib/$(ARCH)/opt/lib$(PNAME).so $(GFID)/c/lib/$(ARCH)/opt/lib$(PNAME).so
	$(INSTALL_DIR) $(GFID)/ruby
	$(INSTALL_DATA) $(PNAME).jpk $(GFID)/$(PNAME).jpk
	$(INSTALL_DATA) $(PNAME).scm $(GFID)/$(PNAME).scm
	$(INSTALL_DATA) base/main.rb $(GFID)/ruby/main.rb
	(cd help; $(MAKE) $@)
	(cd templates; $(MAKE) $@)

kloc::
	wc base/*.[ch] format/*.[ch] configure extra/*.rb


ifeq ($(JMAX_VERSION),25)

DISTDIR = $(JMAXROOTDIR)/fts

ifndef MODE
MODE=opt
endif

include $(JMAXDISTDIR)/Makefiles/Makefile.$(ARCH)
include $(JMAXDISTDIR)/Makefiles/Makefile.$(MODE)

ifeq ($(strip $(OBJDIR)),)
OBJDIR  = c/obj/$(ARCH)/$(MODE)
endif

ifeq ($(strip $(LIBDIR)),)
LIBDIR  = c/lib/$(ARCH)/$(MODE)
endif

ifeq ($(strip $(BINDIR)),)
BINDIR  = c/bin/$(ARCH)/$(MODE)
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

c-src-clean::
	rm -f $(OBJS1)
	rm -f $(TOCLEAN)

$(OBJDIR)/base_%.o: base/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/format_%.o: format/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(LIBDIR)/%.so: $(OBJECTS)
	$(LDSO) $(LDSOFLAGS) $(FTS_SOFLAGS) -o $@ $^ $(SOLIBS) $(MORESOLIBS)

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

LDSOFLAGS += -rdynamic $(GRIDFLOW_LDSOFLAGS)

### for hardcore malloc debugging; standalone only
# LDSOFLAGS += -lefence

OBJS1 = $(addprefix $(OBJDIR)/,$(subst /,_,$(subst .c,.o,$(SOURCES))))
OBJS1 += $(OBJDIR)/base_bridge_jmax.o

$(PACKAGELIB): $(OBJS1)
	gcc $(LDSOFLAGS) -o $(PACKAGELIB) $(OBJS1)

endif #25

ARCH_CFLAGS += -Wall # for cleanliness
ARCH_CFLAGS += -Wno-unused # it's normal to have unused parameters
ARCH_CFLAGS += -Wno-strict-prototypes # Ruby has old-skool .h files
ARCH_CFLAGS += -O6 -funroll-loops # optimisation
ARCH_CFLAGS += -g # gdb info
ARCH_CFLAGS += -fdollars-in-identifiers # $ is the 28th letter
ARCH_CFLAGS += -fpic # some OSes need that for .so files



### standalone stuff ####################################################

OBJS2 = $(addprefix c/obj2/,$(subst /,_,$(subst .c,.o,$(SOURCES))))
FIX_LD = LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH}
LIB = c/lib2/lib$(PNAME).so
# LIB2 = /home/matju/lib/$(LIB)

c-src-standalone:: $(LIB)

c-src-clean-standalone::
	rm -f c/lib2/* c/obj2/*

c/obj2/bridge_none.o: base/bridge_none.c base/bridge_none.h base/grid.h \
base/lang.h config.h Makefile
	@mkdir -p c/obj2
	gcc $(ARCH_CFLAGS) -DSTANDALONE -c $< -o $@

c/obj2/base_%.o: base/%.c base/grid.h base/lang.h config.h \
base/bridge_none.h Makefile
	@mkdir -p c/obj2
	gcc $(ARCH_CFLAGS) -DSTANDALONE -c $< -o $@

c/obj2/base_%.o: base/%.c base/grid.h base/lang.h config.h \
base/bridge_none.h Makefile
	@mkdir -p c/obj2
	gcc $(ARCH_CFLAGS) -DSTANDALONE -c $< -o $@

$(LIB): $(OBJS2) c/obj2/bridge_none.o Makefile
	@mkdir -p c/lib2
	gcc -shared $(LDSOFLAGS) $(OBJS2) c/obj2/bridge_none.o -o $@

export-config::
	@echo "#define GF_INSTALL_DIR \"$(GFID)\""
