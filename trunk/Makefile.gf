# $Id$
# This is an annex that covers what is not covered by the generated Makefile

INSTALL_DATA = install --mode 644
INSTALL_LIB = $(INSTALL_DATA)
INSTALL_DIR = mkdir -p

CC = g++

all2:: gridflow-for-jmax gridflow-for-puredata # doc-all

# suffixes
ifeq (1,1) # Linux, MSWindows with Cygnus, etc
OSUF = .o
LSUF = .so
ESUF =
else # MSWindows without Cygnus
OSUF = .OBJ
LSUF = .DLL
ESUF = .EXE
endif

#----------------------------------------------------------------#

# GridFlow Installation Directory
GFID = $(lib_install_dir)/packages/gridflow/

CFLAGS += -Wall # for cleanliness
CFLAGS += -Wno-unused # it's normal to have unused parameters

ifeq ($(HAVE_DEBUG),yes)
	CFLAGS += -O0 # debuggability
else
	CFLAGS += -O3 -funroll-loops # speed
endif

CFLAGS += -g    # gdb info
CFLAGS += -fdollars-in-identifiers # $ is the 28th letter
CFLAGS += -fPIC # some OSes/machines need that for .so files

OBJDIR = .

#----------------------------------------------------------------#

ifeq ($(HAVE_STATIC_RUBY),yes)
	RUBYA1 = libruby.a/object.o
	RUBYA2 = libruby.a/*.o
endif

cpu/pentium.o: cpu/pentium.rb
	ruby cpu/pentium.rb > cpu/pentium.asm
	nasm -felf cpu/pentium.asm -o cpu/pentium.o

libruby.a/object.o:
	mkdir -p libruby.a
	(cd libruby.a; ar x $(PATH_LIBRUBY_A) || rm -f *.o)

clean2::
	rm -f $(JMAX_LIB) \
	$(OBJDIR)/base_bridge_jmax$(OSUF) \
	$(OBJDIR)/base_bridge_puredata$(OSUF) \
	$(OBJS)

install2:: ruby-install jmax-install

uninstall:: ruby-uninstall
	# add uninstallation of other files here.

kloc::
	wc base/*.[ch] base/*.rb format/*.[ch] format/*.rb \
	configure Makefile.gf extra/*.rb

edit::
	(nedit base/*.[ch] */main.rb base/test.rb configure &)

CONF = config.make config.h Makefile

export-config::
	@echo "#define GF_INSTALL_DIR \"$(GFID)\""

EFENCE = /usr/lib/libefence$(LSUF)
#	if [ -f $(EFENCE) ]; then export LD_PRELOAD=$(EFENCE); fi;

BACKTRACE = ([ -f core ] && gdb `which ruby` core)
# TEST = base/test.rb math
TEST = base/test.rb formats

test::
	rm -f core
	(ruby       -w $(TEST)) || $(BACKTRACE)

VALG = valgrind --suppressions=extra/ruby.valgrind -v
VALG += --num-callers=8
# VALG += --gdb-attach=yes

vtest::
	rm -f core
	($(VALG) ruby -w $(TEST) &> gf-valgrind) || $(BACKTRACE)
	less gf-valgrind

vvtest::
	rm -f core
	($(VALG) --leak-check=yes ruby -w $(TEST) &> gf-valgrind) || $(BACKTRACE)
	less gf-valgrind

test16:: test
	(ruby-1.6.7 -w $(TEST)) || $(BACKTRACE)

testpd::
	rm -f gridflow.pd_linux && make && \
	rm -f /opt/lib/ruby/site_ruby/1.7/i586-linux/gridflow.so && \
	make && make install && \
	(pd -path . -lib gridflow test.pd || gdb `which pd` core)

munchies::
	ruby base/test.rb munchies

foo::
	@echo "LDSOFLAGS = $(LDSOFLAGS)"

#----------------------------------------------------------------#

ifeq ($(HAVE_JMAX_2_5),yes)
JMAX_LIB = $(OBJDIR)/libgridflow$(LSUF)
gridflow-for-jmax:: $(JMAX_LIB)

#	echo $(LDSOFLAGS)
#	echo $(CFLAGS)

# the -DLINUXPC part is suspect. sorry.
$(JMAX_LIB): base/bridge_jmax.c base/bridge.c base/grid.h $(CONF) $(RUBYA1)
	@mkdir -p $(OBJDIR)
	$(CC) $(LDSOFLAGS) $(CFLAGS) -DLINUXPC -DOPTIMIZE $< \
		-xnone $(RUBYA2) $(LIBS_LIBRUBY_A) -o $@

jmax-install::
	$(INSTALL_DIR) $(GFID)/c/lib/$(ARCH)/opt
	$(INSTALL_LIB) $(OBJDIR)/libgridflow$(LSUF) \
		$(GFID)/c/lib/$(ARCH)/opt/libgridflow$(LSUF)
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

#----------------------------------------------------------------#

ifeq ($(HAVE_PUREDATA),yes)
# pd_linux pd_nt pd_irix5 pd_irix6

OS = LINUX
# OS = WINNT
# OS = IRIX5
# OS = IRIX6

#pd_nt: foo1.dll foo2.dll dspobj~.dll
#PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo
#VC="C:\Program Files\Microsoft Visual Studio\Vc98"
#PDNTINCLUDE = /I. /I\tcl\include /I\ftp\pd\src /I$(VC)\include
#PDNTLDIR = $(VC)\lib
#PDNTLIB = $(PDNTLDIR)\libc.lib \
#	$(PDNTLDIR)\oldnames.lib \
#	$(PDNTLDIR)\kernel32.lib \
#	\ftp\pd\bin\pd.lib 
#.c.dll:
#	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
#	link /dll /export:$*_setup $*.obj $(PDNTLIB)

PDSUF = .pd_linux
# PDSUF = .dll
# PDSUF = .pd_irix5
# PDSUF = .pd_irix6

# PDCFLAGS = -o32 -DPD -DUNIX -DIRIX -O2
# PDLDFLAGS = -elf -shared -rdata_shared 

# PDCFLAGS = -n32 -DPD -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
#	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
#	-Ofast=ip32
# PDLDFLAGS = -IPA -n32 -shared -rdata_shared

#PDCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
#    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
#    -Wno-unused -Wno-parentheses -Wno-switch

PD_LIB = $(OBJDIR)/gridflow$(PDSUF)

$(PD_LIB): base/bridge_puredata.c base/bridge.c base/grid.h $(CONF) $(RUBYA1)
	@mkdir -p $(OBJDIR)
	$(CC) $(LDSOFLAGS) $(CFLAGS) $< \
		-xnone $(RUBYA2) $(LIBS_LIBRUBY_A) -o $@

gridflow-for-puredata:: $(PD_LIB)

puredata-install::

else

gridflow-for-puredata::
	@#nothing

puredata-install::
	@#nothing

endif # HAVE_PUREDATA
