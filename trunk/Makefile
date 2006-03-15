include config.make
COMMON_DEPS = config.make Makefile base/source_filter.rb
RUBY = ruby

#--------#

SHELL = /bin/sh
libdir = /home/matju/lib
LDSHARED = $(CXX) -shared
RM = rm -f
LIBPATH =  -L'$(libdir)' -Wl,-R'$(libdir)'
LIBS = -Wl,-R -Wl,$(libdir) -L$(libdir) -L. -lruby -ldl -lcrypt -lm
CFLAGS += -Wall -Wno-unused -Wunused-variable
CFLAGS += -fno-omit-frame-pointer -g -fPIC -I.
ifeq ($(HAVE_DEBUG),yes)
	CFLAGS += -O1
else
	CFLAGS += -O3 -funroll-loops
endif

OBJS += base/main.o base/grid.o base/bitpacking.o \
base/flow_objects.o \
base/flow_objects_for_image.o \
base/flow_objects_for_matrix.o \
base/number.1.o \
base/number.2.o \
base/number.3.o \

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

DLLIB = gridflow$(LSUF)

all:: $(DLLIB) gridflow-for-puredata

clean::
	@-$(RM) $(DLLIB) gridflow.pd_linux *.o */*.o *.so
	rm -f $(OBJS) base/*.fcs format/*.fcs cpu/*.fcs

.SUFFIXES:

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
	$(LDSHARED) $(LIBPATH) -o $@ $(OBJS) $(LDSOFLAGS) $(LIBS)

.PRECIOUS: %.h.fcs %.c.fcs %.m.fcs

cpu/mmx.asm cpu/mmx_loader.c: cpu/mmx.rb
	$(RUBY) cpu/mmx.rb cpu/mmx.asm cpu/mmx_loader.c
cpu/mmx.o: cpu/mmx.asm
	nasm -f elf cpu/mmx.asm -o cpu/mmx.o

unskew::
	find . -mtime -0 -ls -exec touch '{}' ';'

CONF = config.make config.h Makefile

ifeq ($(HAVE_PUREDATA),yes)
ifeq (${SYSTEM},Darwin)
  PDSUF = .pd_darwin
  PDBUNDLEFLAGS = -bundle -undefined suppress
else
  ifeq (${SYSTEM},NT)
    PDSUF = .dll
    PDBUNDLEFLAGS = -shared
    # huh
  else
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

