# $Id$
include config.make

all:: gridflow-for-ruby gridflow-for-jmax # doc-all

#----------------------------------------------------------------#

# GridFlow Installation Directory
GFID = $(lib_install_dir)/packages/gridflow/

PACKAGELIB = $(LIBDIR)/libgridflow.so
OBJS = $(addprefix $(OBJDIR)/,$(subst /,_,$(subst .c,.o,$(SOURCES))))
LIB = $(LIBDIR)/libgridflow.so

CFLAGS += -Wall # for cleanliness
CFLAGS += -Wno-unused # it's normal to have unused parameters
CFLAGS += -Wno-strict-prototypes # Ruby has old-skool .h files
CFLAGS += -O1 # debuggability
# CFLAGS += -O6 -funroll-loops # optimisation
CFLAGS += -g # gdb info
CFLAGS += -fdollars-in-identifiers # $ is the 28th letter
CFLAGS += -fpic # some OSes need that for .so files

OBJDIR = c/obj/$(ARCH)
LIBDIR = c/lib/$(ARCH)
BINDIR = c/bin/$(ARCH)

gridflow-for-ruby:: $(LIB)


#----------------------------------------------------------------#

clean:: tests-clean
	rm -f $(OBJS)

install::
	$(INSTALL_DIR) $(GFID)
	$(INSTALL_DIR) $(GFID)/c/lib/$(ARCH)/opt
	$(INSTALL_LIB) c/lib/$(ARCH)/libgridflowjmax.so $(GFID)/c/lib/$(ARCH)/opt/libgridflow.so
	$(INSTALL_DIR) $(GFID)/ruby
	$(INSTALL_DATA) gridflow.jpk $(GFID)/gridflow.jpk
	$(INSTALL_DATA) gridflow.scm $(GFID)/gridflow.scm
	$(INSTALL_DATA) base/main.rb $(GFID)/ruby/main.rb
	(cd help; $(MAKE) $@)
	(cd templates; $(MAKE) $@)

kloc::
	wc base/*.[ch] format/*.[ch] configure extra/*.rb

CONF = config.make config.h Makefile

$(OBJDIR)/base_%.o: base/%.c base/grid.h $(CONF)
	@mkdir -p $(OBJDIR)
	gcc $(CFLAGS) -c $< -o $@

$(OBJDIR)/format_%.o: format/%.c base/grid.h $(CONF)
	@mkdir -p $(OBJDIR)
	gcc $(CFLAGS) -c $< -o $@

RUBY_OBJS = $(OBJS) $(OBJDIR)/base_bridge_none.o

$(LIB): $(RUBY_OBJS) $(LIBDIR) $(CONF)
	@mkdir -p $(LIBDIR)
	gcc -shared -rdynamic $(LDSOFLAGS) $(RUBY_OBJS) -o $@

export-config::
	@echo "#define GF_INSTALL_DIR \"$(GFID)\""

test:: $(LIB)
	ruby tests/test.rb

foo::
	@echo "LDSOFLAGS = $(LDSOFLAGS)"
	@echo "GRIDFLOW_LDSOFLAGS = $(GRIDFLOW_LDSOFLAGS)"

#----------------------------------------------------------------#

ifeq ($(HAVE_JMAX_2_5),yes)
DISTDIR = $(JMAXROOTDIR)/fts
JMAX_OBJS = $(OBJS) $(OBJDIR)/base_bridge_jmax.o
JMAX_LIB = $(LIBDIR)/libgridflowjmax.so
LDSOFLAGS += -rdynamic $(GRIDFLOW_LDSOFLAGS)
gridflow-for-jmax:: $(LIBDIR) $(OBJDIR) $(JMAX_LIB)

$(OBJDIR)/base_bridge_jmax.o: base/bridge_jmax.c base/grid.h $(CONF)
	@mkdir -p $(OBJDIR)
	gcc $(CFLAGS) -DLINUXPC -DOPTIMIZE -c $< -o $@

$(JMAX_LIB): $(JMAX_OBJS) 
	gcc $(LDSOFLAGS) $(FTS_SOFLAGS) $(JMAX_OBJS) -o $@ $(SOLIBS) $(MORESOLIBS)

endif # HAVE_JMAX_2_5
