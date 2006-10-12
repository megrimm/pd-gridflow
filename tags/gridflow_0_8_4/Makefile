#!/usr/bin/make
# $Id$

include config.make
COMMON_DEPS = config.make Makefile base/source_filter.rb
RUBY = ruby

#--------#

SHELL = /bin/sh
LDSHARED = $(CXX) $(PDBUNDLEFLAGS)
RM = rm -f
LIBS = -L. -lruby -ldl -lcrypt -lm
CFLAGS += -Wall -Wno-unused -Wunused-variable
CFLAGS += -g -fPIC -I.
ifeq ($(HAVE_DEBUG),yes)
	CFLAGS += -O1
else
#	CFLAGS += -O3 -funroll-loops -fno-omit-frame-pointer
	CFLAGS += -O2 -funroll-loops
endif

OBJS2 = base/main.o base/grid.o base/bitpacking.o base/flow_objects.o \
base/number.1.o base/number.2.o base/number.3.o base/number.4.o

SYSTEM = $(shell uname -s | sed -e 's/^MINGW.*/NT/')
DLLIB = gridflow.$(DLEXT)
FILT = $(RUBY) -w base/source_filter.rb

all:: $(DLLIB) gridflow-for-puredata

clean::
	@-$(RM) $(DLLIB) gridflow.pd_linux *.o */*.o *.so
	rm -f $(OBJS2) $(OBJS) base/*.fcs format/*.fcs cpu/*.fcs optional/*.fcs

.SUFFIXES:

%.h.fcs: %.h $(COMMON_DEPS)
	$(FILT) $< $@
%.c.fcs: %.c $(COMMON_DEPS) base/grid.h.fcs
	$(FILT) $< $@
%.m.fcs: %.m $(COMMON_DEPS) base/grid.h.fcs
	$(FILT) $< $@
%.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -c $< -o $@
%.1.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -DPASS1 -c $< -o $@
%.2.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -DPASS2 -c $< -o $@
%.3.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -DPASS3 -c $< -o $@
%.4.o: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -DPASS4 -c $< -o $@
%.o: %.m.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -xobjective-c++ -c $< -o $@

%.s: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -S $< -o $@
%.e: %.c.fcs $(COMMON_DEPS) base/grid.h.fcs
	$(CXX) $(CFLAGS) -E $< -o $@

$(DLLIB): $(OBJS2) $(OBJS)
	@-$(RM) $@
	$(LDSHARED) $(LIBPATH) -o $@ $(OBJS2) $(OBJS) $(LDSOFLAGS) $(LIBS)

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
    PDBUNDLEFLAGS = -shared
  endif
endif

PD_LIB = gridflow$(PDSUF)

$(PD_LIB): bridge/puredata.c.fcs base/grid.h $(CONF)
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

dupabs: pd_abstractions/@color.pd

pd_abstractions/@color.pd: pd_abstractions/\#color.pd
	for z in camera_control motion_detection color mouse fade scale_to \
	apply_colormap_channelwise checkers contrast posterize ravel remap_image solarize spread \
	rgb_to_greyscale greyscale_to_rgb rgb_to_yuv yuv_to_rgb; do \
	cp -a pd_abstractions/\#$$z.pd pd_abstractions/\@$$z.pd; done

#--------#--------#--------#--------#--------#--------#--------#--------

ruby/configure: ruby/configure.in
	(cd ruby; autoconf)

ruby/Makefile: ruby/configure
	(prefix=$$(pwd); cd ruby; ./configure --enable-shared --prefix=$$prefix)

ruby/ruby: ruby/Makefile
	(cd ruby; make)

ruby:: ruby/ruby

ruby-checkout::
	cvs -d :pserver:anonymous@cvs.ruby-lang.org:/src checkout -D 2005-10-01 ruby

ruby-install:: ruby/ruby
	(cd ruby; make install)

#--------#--------#--------#--------#--------#--------#--------#--------

help::
	@echo "do one of the following:";\
	echo  "make all            compiles gridflow";\
	echo  "make beep           beeps";\
	echo  "make ruby           compiles ruby";\
	echo  "make ruby-install   installs ruby into this directory";\
	echo  "make ruby-checkout  downloads ruby";\
	echo  "make unskew         removes timestamps in the future (if you have clock issues)"
