# $Id$
# This is an annex that covers what is not covered by the generated Makefile

INSTALL_DATA = install --mode 644
INSTALL_LIB = $(INSTALL_DATA)
INSTALL_DIR = mkdir -p

all2:: gridflow-for-jmax gridflow-for-puredata # doc-all
include config.make

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
LDSOFLAGS = -rdynamic $(GRIDFLOW_LDSOFLAGS)

CFLAGS += -Wall # for cleanliness
CFLAGS += -Wno-unused # it's normal to have unused parameters
CFLAGS += -Wno-strict-prototypes # Ruby has old-skool .h files

ifeq ($(HAVE_DEBUG),yes)
	CFLAGS += -O0 # debuggability
else
	CFLAGS += -O6 -funroll-loops # speed
endif

CFLAGS += -g    # gdb info
CFLAGS += -fdollars-in-identifiers # $ is the 28th letter
CFLAGS += -fPIC # some OSes/machines need that for .so files

OBJDIR = .

#----------------------------------------------------------------#

clean2::
	rm -f $(JMAX_LIB) \
	$(OBJDIR)/base_bridge_jmax$(OSUF) \
	$(OBJDIR)/base_bridge_puredata$(OSUF)

install2:: ruby-install jmax-install

uninstall:: ruby-uninstall
	# add uninstallation of other files here.

kloc::
	wc base/*.[ch] base/*.rb format/*.[ch] format/*.rb configure Makefile extra/*.rb

edit::
	(nedit base/*.rb base/*.[ch] format/main.rb tests/test.rb &)

CONF = config.make config.h Makefile

export-config::
	@echo "#define GF_INSTALL_DIR \"$(GFID)\""

EFENCE = /usr/lib/libefence$(LSUF)
#	if [ -f $(EFENCE) ]; then export LD_PRELOAD=$(EFENCE); fi;

test:: install
	ulimit -c unlimited; rm -f core; \
	ruby -w tests/test.rb || \
	([ -f core ] && gdb `which ruby` core)

foo::
	@echo "LDSOFLAGS = $(LDSOFLAGS)"
	@echo "GRIDFLOW_LDSOFLAGS = $(GRIDFLOW_LDSOFLAGS)"

#----------------------------------------------------------------#

ifeq ($(HAVE_JMAX_2_5),yes)
JMAX_LIB = $(OBJDIR)/libgridflow$(LSUF)
gridflow-for-jmax:: $(JMAX_LIB)

$(JMAX_LIB): base/bridge_jmax.c base/grid.h $(CONF)
	@mkdir -p $(OBJDIR)
	gcc -shared $(LDSOFLAGS) $(CFLAGS) -DLINUXPC -DOPTIMIZE $< -o $@

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

PD_LIB = $(OBJDIR)/gridflow-pd$(PDSUF)

$(PD_LIB): base/bridge_puredata.c base/grid.h $(CONF)
	@mkdir -p $(OBJDIR)
	gcc -shared $(LDSOFLAGS) $(CFLAGS) -DLINUXPC -DOPTIMIZE $< -o $@

gridflow-for-puredata:: $(PD_LIB)

puredata-install::

else

gridflow-for-puredata::
	@#nothing

puredata-install::
	@#nothing

endif # HAVE_PUREDATA
