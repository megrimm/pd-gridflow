# $Id$
# This is an annex that covers what is not covered by the generated Makefile

SYSTEM = $(shell uname -s)
INSTALL_DATA = install -m 644
INSTALL_LIB = $(INSTALL_DATA)
INSTALL_DIR = mkdir -p

CC = g++

all2:: gridflow-for-jmax gridflow-for-puredata # doc-all

# suffixes (not properly used)
ifeq (1,1) # Linux, MSWindows with Cygnus, etc
OSUF = .o
LSUF = .so
ESUF =
else # MSWindows without Cygnus (not supported yet)
OSUF = .OBJ
LSUF = .DLL
ESUF = .EXE
endif

#----------------------------------------------------------------#

CFLAGS += -Wall # for cleanliness
CFLAGS += -Wno-unused # it's normal to have unused parameters

ifeq ($(HAVE_DEBUG),yes)
	CFLAGS += -O0 # debuggability
else
	CFLAGS += -O3 -funroll-loops # speed
endif

CFLAGS += -fno-omit-frame-pointer
CFLAGS += -g    # gdb info
CFLAGS += -fPIC # some OSes/machines need that for .so files

#----------------------------------------------------------------#

cpu/mmx.asm cpu/mmx_loader.c: cpu/mmx.rb
	ruby cpu/mmx.rb cpu/mmx.asm cpu/mmx_loader.c

cpu/mmx.o: cpu/mmx.asm
	nasm -f elf cpu/mmx.asm -o cpu/mmx.o

clean2::
	rm -f $(JMAX_LIB) \
	$(OBJS)

install2:: ruby-install jmax-install puredata-install

uninstall:: ruby-uninstall
	# add uninstallation of other files here.

unskew::
	find . -mtime -0 -ls -exec touch '{}' ';'

kloc::
	wc base/*.[ch] base/*.rb format/*.[ch] format/*.rb \
	configure Makefile.gf extra/*.rb

edit::
	(nedit base/grid.[ch] base/number.c base/flow_objects.c base/main.c \
	*/main.rb base/test.rb &)

CONF = config.make config.h Makefile

EFENCE = /usr/lib/libefence$(LSUF)
#	if [ -f $(EFENCE) ]; then export LD_PRELOAD=$(EFENCE); fi;

BACKTRACE = ([ -f core ] && gdb `which ruby` core)
TEST = base/test.rb math
# TEST = base/test.rb formats

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

ifndef ARCH
ARCH=$(JMAX_ARCH)
endif

ifeq ($(HAVE_JMAX_4),yes)

# GridFlow/jMax4 Installation Directory
GFID = $(JMAX4_INSTALL_DIR)/gridflow

JMAX_LIB = libgridflow$(LSUF)
gridflow-for-jmax:: $(JMAX_LIB)

# the -DLINUXPC part is suspect. sorry.
$(JMAX_LIB): base/bridge_jmax4.c base/bridge.c base/grid.h $(CONF)
	$(CXX) $(LDSOFLAGS) $(BRIDGE_LDFLAGS) $(CFLAGS) \
		-DLINUXPC -DOPTIMIZE $< \
		-xnone -o $@

jmax-install::
	$(INSTALL_DIR) $(GFID)/c
	$(INSTALL_LIB) libgridflow$(LSUF) $(GFID)/c/libgridflow$(LSUF)
	$(INSTALL_DATA) gridflow.jpk $(GFID)/gridflow.jpk
	$(INSTALL_DATA) gridflow.scm $(GFID)/gridflow.scm
	$(INSTALL_DIR) $(GFID)/templates
	$(INSTALL_DIR) $(GFID)/help
	for f in templates/*.jmax help/*.tcl help/*.jmax; do \
		$(INSTALL_DATA) $$f $(GFID)/$$f; \
	done
else
ifeq ($(HAVE_JMAX_2_5),yes)

include bundled/jmax/Makefiles/Makefile.$(ARCH)

# GridFlow/jMax25 Installation Directory
GFID = $(lib_install_dir)/packages/gridflow/

JMAX_LIB = libgridflow$(LSUF)
gridflow-for-jmax:: $(JMAX_LIB)

#	echo $(LDSOFLAGS)
#	echo $(CFLAGS)

# the -DLINUXPC part is suspect. sorry.
$(JMAX_LIB): base/bridge_jmax.c base/bridge.c base/grid.h $(CONF)
	$(CXX) $(LDSOFLAGS) $(BRIDGE_LDFLAGS) $(CFLAGS) \
		-DLINUXPC -DOPTIMIZE $< \
		-xnone -o $@

jmax-install::
	$(INSTALL_DIR) $(GFID)/c/lib/$(ARCH)/opt
	$(INSTALL_LIB) libgridflow$(LSUF) \
		$(GFID)/c/lib/$(ARCH)/opt/libgridflow$(LSUF)
	$(INSTALL_DATA) gridflow.jpk $(GFID)/gridflow.jpk
	$(INSTALL_DATA) gridflow.scm $(GFID)/gridflow.scm
	$(INSTALL_DIR) $(GFID)/templates
	$(INSTALL_DIR) $(GFID)/help
	for f in templates/*.jmax help/*.tcl help/*.jmax; do \
		$(INSTALL_DATA) $$f $(GFID)/$$f; \
	done
else

$(JMAX_LIB):
	@#nothing

gridflow-for-jmax::
	@#nothing

jmax-install::

endif # HAVE_JMAX_2_5
endif # HAVE_JMAX_4

#----------------------------------------------------------------#

ifeq ($(HAVE_PUREDATA),yes)
# pd_linux pd_nt pd_irix5 pd_irix6

ifeq (${SYSTEM},Darwin)
  OS = DARWIN
  PDSUF = .pd_darwin
  PDBUNDLEFLAGS = -bundle_loader $(shell which pd)
else
  OS = LINUX
  PDSUF = .pd_linux
endif
# OS = WINNT # PDSUF = .dll
# OS = IRIX5 # PDSUF = .pd_irix5
# OS = IRIX6 # PDSUF = .pd_irix6

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


# PDCFLAGS = -o32 -DPD -DUNIX -DIRIX -O2
# PDLDFLAGS = -elf -shared -rdata_shared 

# PDCFLAGS = -n32 -DPD -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
#	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
#	-Ofast=ip32
# PDLDFLAGS = -IPA -n32 -shared -rdata_shared

#PDCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
#    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
#    -Wno-unused -Wno-parentheses -Wno-switch

PD_LIB = gridflow$(PDSUF)

$(PD_LIB): base/bridge_puredata.c base/bridge.c base/grid.h $(CONF)
	$(CXX) $(LDSOFLAGS) $(BRIDGE_LDFLAGS) $(CFLAGS) $(PDBUNDLEFLAGS) \
		$< -xnone -o $@

gridflow-for-puredata:: $(PD_LIB)

puredata-install::
	mkdir -p $(PUREDATA_PATH)/doc/5.reference/gridflow
	cp pd_help/*.pd $(PUREDATA_PATH)/doc/5.reference/gridflow
	cp $(PD_LIB) $(PUREDATA_PATH)/externs

else

gridflow-for-puredata::
	@#nothing

puredata-install::
	@#nothing

endif # HAVE_PUREDATA
