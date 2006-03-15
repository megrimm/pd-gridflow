include config.make
COMMON_DEPS = config.make Makefile base/source_filter.rb
gfbindir = /home/matju

SHELL = /bin/sh

#### Start of system configuration section. ####

srcdir = .
topdir = /home/matju/lib/ruby/1.9/i686-linux
hdrdir = $(topdir)
VPATH = $(srcdir):$(topdir):$(hdrdir)
prefix = $(RUBYDESTDIR)/home/matju
exec_prefix = $(prefix)
sitedir = $(prefix)/lib/ruby/site_ruby
rubylibdir = $(libdir)/ruby/$(ruby_version)
archdir = $(rubylibdir)/$(arch)
sbindir = $(exec_prefix)/sbin
datadir = $(prefix)/share
includedir = $(prefix)/include
infodir = $(prefix)/info
sysconfdir = $(prefix)/etc
mandir = $(prefix)/man
libdir = $(exec_prefix)/lib
sharedstatedir = $(prefix)/com
oldincludedir = $(RUBYDESTDIR)/usr/include
sitearchdir = $(sitelibdir)/$(sitearch)
bindir = $(exec_prefix)/bin
localstatedir = $(prefix)/var
sitelibdir = /home/matju/lib/pd/extra
libexecdir = $(exec_prefix)/libexec

LIBRUBY = $(LIBRUBY_SO)
LIBRUBY_A = lib$(RUBY_SO_NAME)-static.a
LIBRUBYARG_SHARED = -Wl,-R -Wl,$(libdir) -L$(libdir) -L. -l$(RUBY_SO_NAME)
LIBRUBYARG_STATIC = -l$(RUBY_SO_NAME)-static

CFLAGS   =  -fPIC -g -O2  -fPIC -xc++ -fno-operator-names -fno-omit-frame-pointer -I/usr/X11R6/include -fno-inline -fmessage-length=150 -pg -I/home/matju/lib/ruby/1.9/i686-linux -I../gem-cvs/Gem/src -falign-functions=16 -mcpu=$(CPU) -march=$(CPU) 
CPPFLAGS = -I. -I. -I -I.  
CXXFLAGS = $(CFLAGS) -g -O2
DLDFLAGS =   
LDSHARED = $(CC) -shared
AR = ar
EXEEXT = 

RUBY_INSTALL_NAME = ruby
RUBY_SO_NAME = ruby
arch = i686-linux
sitearch = i686-linux
ruby_version = 1.9
ruby = /home/matju/bin/ruby
RUBY = $(ruby)
RM = rm -f
MAKEDIRS = mkdir -p
INSTALL = /usr/bin/install -c
INSTALL_PROG = $(INSTALL) -m 0755
INSTALL_DATA = $(INSTALL) -m 644
COPY = cp

#### End of system configuration section. ####

preload = 

libpath = $(libdir)
LIBPATH =  -L'$(libdir)' -Wl,-R'$(libdir)'

LOCAL_LIBS = -lm -lusb -L/usr/X11R6/lib -lX11 -lXext -lglut -lGL -lGLU -lSDL -lpthread -laa -ljpeg -lpng -lz -lmpeg3 -lquicktime -ldl -lglib
LIBS = $(LIBRUBYARG_SHARED)  -ldl -lcrypt -lm   -lc
SRCS = grid.c main.c number.1.c number.2.c number.3.c bitpacking.c flow_objects.c flow_objects_for_image.c flow_objects_for_matrix.c
OBJS = base/grid.o base/main.o base/number.1.o base/number.2.o base/number.3.o base/bitpacking.o base/flow_objects.o base/flow_objects_for_image.o base/flow_objects_for_matrix.o cpu/mmx.o cpu/mmx_loader.o optional/usb.o format/x11.o format/opengl.o format/sdl.o format/aalib.o format/jpeg.o format/png.o format/videodev.o format/mpeg3.o format/quicktimehw.o optional/gem.o

DLLIB = gridflow.so
RUBYCOMMONDIR = $(sitedir)
RUBYLIBDIR    = $(sitelibdir)
RUBYARCHDIR   = $(sitearchdir)
CLEANLIBS     = gridflow.so
CLEANOBJS     = *.o */*.o *.so

all:: $(DLLIB) all2

clean:: clean2 
	@-$(RM) $(CLEANLIBS) $(CLEANOBJS)

install-so: $(RUBYARCHDIR)
install-so: $(RUBYARCHDIR)/$(DLLIB)
$(RUBYARCHDIR)/$(DLLIB): $(DLLIB)
	$(INSTALL_PROG) $(DLLIB) $(RUBYARCHDIR)
install-rb: pre-install-rb install-rb-default
install-rb-default: pre-install-rb-default
pre-install-rb: Makefile
pre-install-rb-default: Makefile
$(RUBYARCHDIR):
	$(MAKEDIRS) $@

site-install: site-install-so site-install-rb $(RUBYARCHDIR)/$(DLLIB) ruby-install puredata-install
	(cd devices4ruby; make install)

site-install-so: install-so
site-install-rb: install-rb

.SUFFIXES:



.cc.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

.C.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

%.h.fcs: %.h $(COMMON_DEPS)
	ruby -w base/source_filter.rb $< $@

%.c.fcs: %.c $(COMMON_DEPS) base/grid.h.fcs
	ruby -w base/source_filter.rb $< $@

%.m.fcs: %.m $(COMMON_DEPS) base/grid.h.fcs
	ruby -w base/source_filter.rb $< $@

%.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
%.1.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) $(CPPFLAGS) -DPASS1 -c $< -o $@
%.2.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) $(CPPFLAGS) -DPASS2 -c $< -o $@
%.3.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) $(CPPFLAGS) -DPASS3 -c $< -o $@
%.o: %.m.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) $(CPPFLAGS) -xobjective-c++ -c $< -o $@

$(DLLIB): $(OBJS)
	@-$(RM) $@
	$(LDSHARED) $(DLDFLAGS) $(LIBPATH) -o $@ $(OBJS) $(LOCAL_LIBS) $(LIBS)



$(OBJS): ruby.h defines.h
.PRECIOUS: %.h.fcs %.c.fcs %.m.fcs

RUBY = $(RUBY_INSTALL_NAME)
ruby-all::

ruby-install::




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

install:
	@echo -e "\033[0;1;33;41m"
	@echo -e "1. move this folder to lib/pd/extra or add the folder to -path"
	@echo -e "2. delete the old gridflow.pd_linux"
	@echo -e "3. and don't do \"make install\" anymore\033[0m\n"
