# $Id$
include config.make

all:: gridflow-for-ruby gridflow-for-jmax # doc-all

#----------------------------------------------------------------#

# GridFlow Installation Directory
GFID = $(lib_install_dir)/packages/gridflow/

OBJS = $(addprefix $(OBJDIR)/,$(subst /,_,$(subst .c,.o,$(SOURCES))))
LIB = $(LIBDIR)/gridflow.so

CFLAGS += -Wall # for cleanliness
CFLAGS += -Wno-unused # it's normal to have unused parameters
CFLAGS += -Wno-strict-prototypes # Ruby has old-skool .h files

ifeq ($(HAVE_SPEED),yes)
	CFLAGS += -O6 -funroll-loops # optimisation
else
	CFLAGS += -O1 # debuggability
endif

CFLAGS += -g # gdb info
CFLAGS += -fdollars-in-identifiers # $ is the 28th letter
CFLAGS += -fPIC # some OSes/machines need that for .so files

OBJDIR = c/obj/$(ARCH)
LIBDIR = c/lib/$(ARCH)
BINDIR = c/bin/$(ARCH)

gridflow-for-ruby:: $(LIB)

#----------------------------------------------------------------#

clean::
	rm -f $(OBJS) $(LIB) $(JMAX_LIB) \
	$(OBJDIR)/base_bridge_jmax.o $(OBJDIR)/base_bridge_none.o

install::
	$(INSTALL_DIR) $(GFID)
	$(INSTALL_DIR) $(GFID)/c/lib/$(ARCH)/opt
	[ -f c/lib/$(ARCH)/libgridflow.so ] && \
		$(INSTALL_LIB) c/lib/$(ARCH)/libgridflow.so \
		$(GFID)/c/lib/$(ARCH)/opt/libgridflow.so
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

EFENCE = /usr/lib/libefence.so
#	if [ -f $(EFENCE) ]; then export LD_PRELOAD=$(EFENCE); fi

test:: $(LIB)
	ulimit -c unlimited; rm -f core; \
	ruby tests/test.rb || \
	([ -f core ] && gdb `which ruby` core)

foo::
	@echo "LDSOFLAGS = $(LDSOFLAGS)"
	@echo "GRIDFLOW_LDSOFLAGS = $(GRIDFLOW_LDSOFLAGS)"

#----------------------------------------------------------------#

ifeq ($(HAVE_JMAX_2_5),yes)
DISTDIR = $(JMAXROOTDIR)/fts
JMAX_OBJS = $(OBJS) $(OBJDIR)/base_bridge_jmax.o
JMAX_LIB = $(LIBDIR)/libgridflow.so
LDSOFLAGS += -rdynamic $(GRIDFLOW_LDSOFLAGS)
gridflow-for-jmax:: $(LIBDIR) $(OBJDIR) $(JMAX_LIB)

$(OBJDIR)/base_bridge_jmax.o: base/bridge_jmax.c base/grid.h $(CONF)
	@mkdir -p $(OBJDIR)
	gcc $(CFLAGS) -DLINUXPC -DOPTIMIZE -c $< -o $@

$(JMAX_LIB): $(JMAX_OBJS) 
	gcc $(LDSOFLAGS) $(FTS_SOFLAGS) $(JMAX_OBJS) -o $@ $(SOLIBS) $(MORESOLIBS)

else

$(JMAX_LIB):
	@#nothing

gridflow-for-jmax::
	@#nothing

endif # HAVE_JMAX_2_5
