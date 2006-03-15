include config.make
COMMON_DEPS = config.make Makefile base/source_filter.rb
RUBY = ruby

#--------#

gfbindir = /home/matju
SHELL = /bin/sh
prefix = $(RUBYDESTDIR)/home/matju
exec_prefix = $(prefix)
libdir = $(exec_prefix)/lib
LDSHARED = $(CC) -shared

arch = i686-linux
sitearch = i686-linux
RM = rm -f

#### End of system configuration section. ####

LIBPATH =  -L'$(libdir)' -Wl,-R'$(libdir)'

LOCAL_LIBS = -lm -lusb -L/usr/X11R6/lib -lX11 -lXext -lglut -lGL -lGLU -lSDL -lpthread -laa -ljpeg -lpng -lz -lmpeg3 -lquicktime -ldl -lglib
LIBS = -Wl,-R -Wl,$(libdir) -L$(libdir) -L. -lruby -ldl -lcrypt -lm   -lc
SRCS = grid.c main.c number.1.c number.2.c number.3.c bitpacking.c flow_objects.c flow_objects_for_image.c flow_objects_for_matrix.c
OBJS = base/grid.o base/main.o base/number.1.o base/number.2.o base/number.3.o base/bitpacking.o base/flow_objects.o base/flow_objects_for_image.o base/flow_objects_for_matrix.o cpu/mmx.o cpu/mmx_loader.o optional/usb.o format/x11.o format/opengl.o format/sdl.o format/aalib.o format/jpeg.o format/png.o format/videodev.o format/mpeg3.o format/quicktimehw.o optional/gem.o

DLLIB = gridflow.so
CLEANLIBS     = gridflow.so
CLEANOBJS     = *.o */*.o *.so

all:: $(DLLIB) gridflow-for-puredata

clean::
	@-$(RM) $(CLEANLIBS) $(CLEANOBJS)
	rm -f $(OBJS) base/*.fcs format/*.fcs cpu/*.fcs

.SUFFIXES:

.cc.o:
	$(CXX) $(CFLAGS) -c $<
.cxx.o:
	$(CXX) $(CFLAGS) -c $<
.cpp.o:
	$(CXX) $(CFLAGS) -c $<
.C.o:
	$(CXX) $(CFLAGS) -c $<

%.h.fcs: %.h $(COMMON_DEPS)
	$(RUBY) -w base/source_filter.rb $< $@

%.c.fcs: %.c $(COMMON_DEPS) base/grid.h.fcs
	$(RUBY) -w base/source_filter.rb $< $@

%.m.fcs: %.m $(COMMON_DEPS) base/grid.h.fcs
	$(RUBY) -w base/source_filter.rb $< $@

%.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -c $< -o $@
%.1.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -DPASS1 -c $< -o $@
%.2.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -DPASS2 -c $< -o $@
%.3.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -DPASS3 -c $< -o $@
%.o: %.m.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -xobjective-c++ -c $< -o $@

$(DLLIB): $(OBJS)
	@-$(RM) $@
	$(LDSHARED) $(LIBPATH) -o $@ $(OBJS) $(LOCAL_LIBS) $(LIBS)

$(OBJS): ruby.h defines.h
.PRECIOUS: %.h.fcs %.c.fcs %.m.fcs

SYSTEM = $(shell uname -s | sed -e 's/^MINGW.*/NT/')

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
	$(RUBY) cpu/mmx.rb cpu/mmx.asm cpu/mmx_loader.c

cpu/mmx.o: cpu/mmx.asm
	nasm -f elf cpu/mmx.asm -o cpu/mmx.o

unskew::
	find . -mtime -0 -ls -exec touch '{}' ';'

CONF = config.make config.h Makefile

#----------------------------------------------------------------#

ifeq ($(HAVE_PUREDATA),yes)
ifeq (${SYSTEM},Darwin)
  OS = DARWIN
  PDSUF = .pd_darwin
  PDBUNDLEFLAGS = -bundle -undefined suppress
else
  ifeq (${SYSTEM},NT)
    OS = NT
    PDSUF = .dll
    PDBUNDLEFLAGS = -shared
    # huh
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

else
gridflow-for-puredata::
	@#nothing
endif # HAVE_PUREDATA

beep::
	@for z in 1 2 3 4 5; do echo -ne '\a'; sleep 1; done

install:
	@echo -e "\033[0;1;33;41m"
	@echo -e "1. move this folder to lib/pd/extra or add the folder to -path"
	@echo -e "2. delete the old gridflow.pd_linux"
	@echo -e "3. and don't do \"make install\" anymore\033[0m\n"

# for z in camera_control motion_detection color mouse centre_of_gravity fade \
# apply_colormap_channelwise checkers contrast posterize ravel remap_image solarize spread \
# rgb_to_greyscale greyscale_to_rgb rgb_to_yuv yuv_to_rgb; do \
# cp pd_abstractions/\#$$z.pd $(PUREDATA_PATH)/extra/\@$$z.pd; done

