#!/usr/bin/make
# $Id$

include config.make
COMMON_DEPS = config.make Makefile base/source_filter.rb
RUBY = ruby

#--------#

SHELL = /bin/sh
LDSHARED = $(CXX) $(PDBUNDLEFLAGS)
RM = rm -f
CFLAGS += -Wall -Wno-unused -Wunused-variable
CFLAGS += -g -fPIC -I.

# comment this out for normal compilation
# CFLAGS += -DSWIG

# LDFLAGS += ../gem-cvs/Gem/Gem.pd_linux
ifeq ($(HAVE_DEBUG),yes)
	CFLAGS += -O1
else
#	CFLAGS += -O3 -funroll-loops -fno-omit-frame-pointer
	CFLAGS += -O2 -funroll-loops
endif

BRIDGE_LDFLAGS += $(LIBRUBYARG) $(LIBS)


OBJS2 = base/main.o base/grid.o base/bitpacking.o base/flow_objects.o \
base/number.1.o base/number.2.o base/number.3.o base/number.4.o

SYSTEM = $(shell uname -s | sed -e 's/^MINGW.*/NT/')
DLLIB = gridflow.$(DLEXT)
FILT = $(RUBY) -w base/source_filter.rb

all:: $(DLLIB) gridflow-for-puredata

clean::
	@-$(RM) $(DLLIB) gridflow.pd_linux *.o */*.o *.so
	rm -f $(OBJS2) $(OBJS) base/*.fcs format/*.fcs optional/*.fcs

.SUFFIXES:

H = gridflow2.h base/grid.h.fcs

%.h.fcs: %.h $(COMMON_DEPS)
	$(FILT) $< $@
%.c.fcs: %.c $(COMMON_DEPS) $(H)
	$(FILT) $< $@
%.m.fcs: %.m $(COMMON_DEPS) $(H)
	$(FILT) $< $@
%.o: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -c $< -o $@
%.1.o: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -DPASS1 -c $< -o $@
%.2.o: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -DPASS2 -c $< -o $@
%.3.o: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -DPASS3 -c $< -o $@
%.4.o: %.c.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -DPASS4 -c $< -o $@
%.o: %.m.fcs $(COMMON_DEPS) $(H)
	$(CXX) $(CFLAGS) -xobjective-c++ -c $< -o $@

%.s: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -S $< -o $@
%.e: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -E $< -o $@

$(DLLIB): $(OBJS2) $(OBJS)
	@-$(RM) $@
	$(LDSHARED) $(LIBPATH) -o $@ $(OBJS2) $(OBJS) $(LDSOFLAGS)

.PRECIOUS: %.h.fcs %.c.fcs %.m.fcs

base/mmx.asm base/mmx_loader.c: base/mmx.rb
	$(RUBY) base/mmx.rb base/mmx.asm base/mmx_loader.c
base/mmx.o: base/mmx.asm
	nasm -f elf base/mmx.asm -o base/mmx.o

unskew::
	find . -mtime -0 -ls -exec touch '{}' ';'

ifeq ($(OS),darwin)
  PDSUF = .pd_darwin
  PDBUNDLEFLAGS = -bundle -flat_namespace -undefined suppress
else
  ifeq ($(OS),nt)
    PDSUF = .dll
    PDBUNDLEFLAGS = -shared
  else
    PDSUF = .pd_linux
    PDBUNDLEFLAGS = -shared -rdynamic
  endif
endif

ifeq ($(HAVE_PUREDATA),yes)
PD_LIB = gridflow$(PDSUF)

$(PD_LIB): rubyext.c.fcs base/grid.h gridflow2.h $(COMMON_DEPS)
	$(CXX) -DPDSUF=\"$(PDSUF)\" -Ibundled/pd $(LDSOFLAGS) $(BRIDGE_LDFLAGS) $(CFLAGS) $(PDBUNDLEFLAGS) \
		$< -xnone -o $@

gridflow-for-puredata:: $(PD_LIB) dupabs

else
gridflow-for-puredata::
	@#nothing
endif # HAVE_PUREDATA

beep::
	@for z in 1 2 3 4 5; do echo -ne '\a'; sleep 1; done

install::
	@echo -e "\033[0;1;33;41m"
	@echo -e "1. move this folder to lib/pd/extra or add the folder to -path"
	@echo -e "2. delete the old gridflow.pd_linux"
	@echo -e "3. and don't do \"make install\" anymore\033[0m\n"

dupabs: abstractions/@color.pd

abstractions/@color.pd: abstractions/\#color.pd
	for z in camera_control motion_detection color mouse fade scale_to \
	apply_colormap_channelwise checkers contrast posterize ravel remap_image solarize spread \
	rgb_to_greyscale greyscale_to_rgb rgb_to_yuv yuv_to_rgb rotate; do \
	cp abstractions/\#$$z.pd abstractions/\@$$z.pd; done

#--------#--------#--------#--------#--------#--------#--------#--------

help::
	@echo "do one of the following:";\
	echo  "make all            compiles gridflow";\
	echo  "make beep           beeps";\
	echo  "make unskew         removes timestamps in the future (if you have clock issues)"

#--------#--------#--------#--------#--------#--------#--------#--------

kloc::
	wc configure */*.rb
