# $Id$
# This is an annex that covers what is not covered by the generated Makefile

SYSTEM = $(shell uname -s | sed -e 's/^MINGW.*/NT/')
INSTALL_DATA = install -m 644
INSTALL_LIB = $(INSTALL_DATA)
INSTALL_DIR = mkdir -p

all2:: gridflow-for-puredata # doc-all

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
# but it's normal to have unused parameters
ifeq ($(HAVE_GCC3),yes)
CFLAGS += -Wno-unused -Wunused-variable
else
CFLAGS += -Wno-unused
endif

ifeq ($(HAVE_DEBUG),yes)
	CFLAGS += -O1 # debuggability
else
	CFLAGS += -O3 -funroll-loops # speed
endif

CFLAGS += -fno-omit-frame-pointer
CFLAGS += -g    # gdb info
CFLAGS += -fPIC # some OSes/machines need that for .so files

#----------------------------------------------------------------#

cpu/mmx.asm cpu/mmx_loader.c: cpu/mmx.rb
	$(RUBY_INSTALL_NAME) cpu/mmx.rb cpu/mmx.asm cpu/mmx_loader.c

cpu/mmx.o: cpu/mmx.asm
	nasm -f elf cpu/mmx.asm -o cpu/mmx.o

clean2::
	rm -f $(OBJS) base/*.fcs format/*.fcs cpu/*.fcs

install2:: ruby-install puredata-install
	(cd devices4ruby; make install)

uninstall:: ruby-uninstall
	# add uninstallation of other files here.

unskew::
	find . -mtime -0 -ls -exec touch '{}' ';'

CONF = config.make config.h Makefile
BACKTRACE = ([ -f core ] && gdb `which ruby` core)
TEST = base/test.rb math

test::
	rm -f core
	($(RUBY_INSTALL_NAME) -w $(TEST)) || $(BACKTRACE)

#----------------------------------------------------------------#

ifeq ($(HAVE_PUREDATA),yes)
# pd_linux pd_nt pd_irix5 pd_irix6

ifeq (${SYSTEM},Darwin)
  OS = DARWIN
  PDSUF = .pd_darwin
  PDBUNDLEFLAGS = -bundle -undefined suppress
else
  ifeq (${SYSTEM},NT)
    OS = NT
    PDSUF = .dll
    PDBUNDLEFLAGS = -shared -I$(PUREDATA_PATH)/src
    PDLIB = -L/usr/local/lib $(LIBRUBYARG_SHARED) $(PUREDATA_PATH)/bin/pd.dll
  else
    OS = LINUX
    PDSUF = .pd_linux
  endif
endif

PD_LIB = gridflow$(PDSUF)

$(PD_LIB): bridge/puredata.c.fcs base/grid.h $(CONF)
	$(CXX) -DPDSUF=\"$(PDSUF)\" -Ibundled/pd $(LDSOFLAGS) $(BRIDGE_LDFLAGS) $(CFLAGS) $(PDBUNDLEFLAGS) \
		$< -xnone -o $@

gridflow-for-puredata:: $(PD_LIB)

DOK = $(PUREDATA_PATH)/doc/5.reference/gridflow

puredata-install::
	mkdir -p $(DOK)/flow_classes
	cp pd_help/*.pd $(DOK)
	cp doc/*.html $(DOK)
	cp doc/flow_classes/*.p* $(DOK)/flow_classes
	cp -r images/ $(PUREDATA_PATH)/extra/gridflow
	cp $(PD_LIB) pd_abstractions/*.pd $(PUREDATA_PATH)/extra
	for z in camera_control motion_detection color mouse centre_of_gravity fade \
	apply_colormap_channelwise checkers contrast posterize ravel remap_image solarize spread \
	rgb_to_greyscale greyscale_to_rgb rgb_to_yuv yuv_to_rgb; do \
		cp pd_abstractions/\#$$z.pd $(PUREDATA_PATH)/extra/\@$$z.pd; done
else

gridflow-for-puredata::
	@#nothing

puredata-install::
	@#nothing

endif # HAVE_PUREDATA

beep::
	@for z in 1 2 3 4 5; do echo -ne '\a'; sleep 1; done
