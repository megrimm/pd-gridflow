include config.make
COMMON_DEPS = config.make Makefile Makefile.gf base/source_filter.rb
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

CC = gcc
CXX = g++-3.3
CXX = g++-3.3
LIBRUBY = $(LIBRUBY_SO)
LIBRUBY_A = lib$(RUBY_SO_NAME)-static.a
LIBRUBYARG_SHARED = -Wl,-R -Wl,$(libdir) -L$(libdir) -L. -l$(RUBY_SO_NAME)
LIBRUBYARG_STATIC = -l$(RUBY_SO_NAME)-static

CFLAGS   =  -fPIC -g -O2  -fPIC -xc++ -fno-operator-names -fno-omit-frame-pointer -I/usr/X11R6/include -fno-inline -fmessage-length=150 -pg -I/home/matju/lib/ruby/1.9/i686-linux -I../gem-cvs/Gem/src -falign-functions=16 -mcpu=$(CPU) -march=$(CPU) 
CPPFLAGS = -I. -I. -I -I.  
CXXFLAGS = $(CFLAGS) -g -O2
DLDFLAGS =   
LDSHARED = $(CC) -shared
LDSHAREDXX = $(CXX) -shared
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
DEFFILE = 

CLEANFILES = 
DISTCLEANFILES = 

extout = 
extout_prefix = 
target_prefix = 
LOCAL_LIBS = -lm -lusb -L/usr/X11R6/lib -lX11 -lXext -lglut -lGL -lGLU -lSDL -lpthread -laa -ljpeg -lpng -lz -lmpeg3 -lquicktime -ldl -lglib
LIBS = $(LIBRUBYARG_SHARED)  -ldl -lcrypt -lm   -lc
SRCS = grid.c main.c number.1.c number.2.c number.3.c bitpacking.c flow_objects.c flow_objects_for_image.c flow_objects_for_matrix.c
OBJS = base/grid.o base/main.o base/number.1.o base/number.2.o base/number.3.o base/bitpacking.o base/flow_objects.o base/flow_objects_for_image.o base/flow_objects_for_matrix.o cpu/mmx.o cpu/mmx_loader.o optional/usb.o format/x11.o format/opengl.o format/sdl.o format/aalib.o format/jpeg.o format/png.o format/videodev.o format/mpeg3.o format/quicktimehw.o optional/gem.o
TARGET = gridflow
DLLIB = $(TARGET).so
STATIC_LIB = 

RUBYCOMMONDIR = $(sitedir)$(target_prefix)
RUBYLIBDIR    = $(sitelibdir)$(target_prefix)
RUBYARCHDIR   = $(sitearchdir)$(target_prefix)

TARGET_SO     = $(DLLIB)
CLEANLIBS     = $(TARGET).so $(TARGET).il? $(TARGET).tds $(TARGET).map
CLEANOBJS     = *.o */*.o *.a *.s[ol] *.pdb *.exp *.bak

all: 		$(DLLIB) all2
static:		$(STATIC_LIB)

clean: clean2 
		@-$(RM) $(CLEANLIBS) $(CLEANOBJS) $(CLEANFILES)

distclean:	clean
		@-$(RM) Makefile extconf.h conftest.* mkmf.log
		@-$(RM) core ruby$(EXEEXT) *~ $(DISTCLEANFILES)

realclean:	distclean


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

site-install: site-install-so site-install-rb $(RUBYARCHDIR)/$(DLLIB) install2
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
include Makefile.gf
RUBY = $(RUBY_INSTALL_NAME)
ruby-all::

ruby-install::


