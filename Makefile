# $Id$

all:: ruby-all gridflow-for-ruby gridflow-for-jmax # doc-all
include config.make

#----------------------------------------------------------------#

# GridFlow Installation Directory
GFID = $(lib_install_dir)/packages/gridflow/
LDSOFLAGS = -rdynamic $(GRIDFLOW_LDSOFLAGS)

OBJS = $(addprefix $(OBJDIR)/,$(subst /,_,$(subst .c,.o,$(SOURCES))))
LIB = $(OBJDIR)/gridflow.so

CFLAGS += -Wall # for cleanliness
CFLAGS += -Wno-unused # it's normal to have unused parameters
CFLAGS += -Wno-strict-prototypes # Ruby has old-skool .h files

ifeq ($(HAVE_SPEED),yes)
	CFLAGS += -O6 -funroll-loops # optimisation
else
	CFLAGS += -O0 # debuggability
endif

CFLAGS += -g # gdb info
CFLAGS += -fdollars-in-identifiers # $ is the 28th letter
CFLAGS += -fPIC # some OSes/machines need that for .so files

OBJDIR = $(ARCH)

gridflow-for-ruby:: $(LIB)


#----------------------------------------------------------------#

clean::
	rm -f $(OBJS) $(LIB) $(JMAX_LIB) \
	$(OBJDIR)/base_bridge_jmax.o

install:: all ruby-install jmax-install
	$(INSTALL_DATA) $(OBJDIR)/gridflow.so $(RUBYDESTDIR)/$(RUBYARCH)/gridflow.so

uninstall:: ruby-uninstall
	# add uninstallation of other files here.

kloc::
	wc base/*.[ch] base/*.rb format/*.[ch] configure Makefile extra/*.rb

#edit::
#	(nedit base/*.rb base/*.[ch] Makefile configure tests/test.rb &)

edit::
	(nedit format/*.c base/flow_objects.c base/grid.[ch] &)

CONF = config.make config.h Makefile

$(OBJDIR)/base_%.o: base/%.c base/grid.h $(CONF)
	@mkdir -p $(OBJDIR)
	gcc $(CFLAGS) -c $< -o $@

$(OBJDIR)/format_%.o: format/%.c base/grid.h $(CONF)
	@mkdir -p $(OBJDIR)
	gcc $(CFLAGS) -c $< -o $@

RUBY_OBJS = $(OBJS)

$(LIB): $(RUBY_OBJS) $(CONF)
	@mkdir -p $(OBJDIR)
	gcc -shared -rdynamic $(LDSOFLAGS) $(RUBY_OBJS) -o $@

export-config::
	@echo "#define GF_INSTALL_DIR \"$(GFID)\""

EFENCE = /usr/lib/libefence.so
#	if [ -f $(EFENCE) ]; then export LD_PRELOAD=$(EFENCE); fi;

test:: install
	ulimit -c unlimited; rm -f core; \
	ruby tests/test.rb || \
	([ -f core ] && gdb `which ruby` core)

foo::
	@echo "LDSOFLAGS = $(LDSOFLAGS)"
	@echo "GRIDFLOW_LDSOFLAGS = $(GRIDFLOW_LDSOFLAGS)"

#----------------------------------------------------------------#

ifeq ($(HAVE_JMAX_2_5),yes)
JMAX_OBJS = $(OBJDIR)/base_bridge_jmax.o
JMAX_LIB = $(OBJDIR)/libgridflow.so
gridflow-for-jmax:: $(JMAX_LIB)

$(OBJDIR)/base_bridge_jmax.o: base/bridge_jmax.c base/grid.h $(CONF)
	@mkdir -p $(OBJDIR)
	gcc $(CFLAGS) -DLINUXPC -DOPTIMIZE -c $< -o $@

$(JMAX_LIB): $(JMAX_OBJS) 
	@mkdir -p $(OBJDIR)
	gcc -shared -rdynamic $(LDSOFLAGS) $(JMAX_OBJS) -o $@

jmax-install::
	$(INSTALL_DIR) $(GFID)/c/lib/$(ARCH)/opt
	$(INSTALL_LIB) $(OBJDIR)/libgridflow.so $(GFID)/c/lib/$(ARCH)/opt/libgridflow.so
	$(INSTALL_DATA) gridflow.jpk $(GFID)/gridflow.jpk
	$(INSTALL_DATA) gridflow.scm $(GFID)/gridflow.scm
	$(INSTALL_DIR) $(lib_install_dir)/packages/gridflow/templates
	$(INSTALL_DIR) $(lib_install_dir)/packages/gridflow/help
	for f in templates/*.jmax help/*.tcl help/*.jmax; do \
		$(INSTALL_DATA) $$f $(lib_install_dir)/packages/gridflow/$$f; \
	done

else

$(JMAX_LIB):
	@#nothing

gridflow-for-jmax::
	@#nothing

jmax-install::

endif # HAVE_JMAX_2_5
