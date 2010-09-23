#!/usr/bin/make

include config.make
COMMON_DEPS = config.make Makefile src/source_filter.rb
COMMON_DEPS2 = $(COMMON_DEPS) src/gridflow.hxx.fcs
RUBY = ruby

SHELL = /bin/sh
LDSHARED = $(CXX) $(PDBUNDLEFLAGS)
RM = rm -f
CFLAGS += -Wall -Wno-unused -Wunused-variable -Wno-trigraphs -g -I.

LDSOFLAGS += -lm $(LIBS)
OBJS2 = src/gridflow.o src/grid.o src/classes1.o src/classes2.o src/classes3.o src/expr.o src/classes_gui.o \
src/number.1.o src/number.2.o src/number.3.o src/number.4.o src/formats.o
PDLIB += gridflow_gem_loader$(PDSUF) gridflow_pdp$(PDSUF) gridflow_unicorn$(PDSUF)
PDLIB += gridflow_gem9292$(PDSUF) gridflow_gem9293$(PDSUF) gridflow_gem9393$(PDSUF)

OS = $(shell uname -s | sed -e 's/^MINGW.*/nt/')
FILT = $(RUBY) -w src/source_filter.rb
ifeq ($(OS),Darwin)
  CFLAGS += -mmacosx-version-min=10.4 -fPIC
  LDSOFLAGS += -headerpad_max_install_names
  PDSUF = .pd_darwin
  PDBUNDLEFLAGS = -bundle -flat_namespace -undefined suppress
  # -undefined dynamic_lookup # is used by smlib. this might be a good idea for future use.
else
  ifeq ($(OS),nt)
    PDSUF = .dll
    PDBUNDLEFLAGS = -shared
    LDSOFLAGS += -L/c/Program\ Files/pd/bin -lpd
    #CFLAGS += -DDES_BUGS
    CFLAGS += -mms-bitfields
  else
    PDSUF = .pd_linux
    PDBUNDLEFLAGS = -shared -rdynamic
    ifeq ($(HAVE_GCC64),yes)
      CFLAGS += -fPIC
      PDBUNDLEFLAGS += -fPIC
    endif
  endif
endif

# this fixes a bug with g++ and objective-c include paths on OSX
ifeq ($(CPLUS_INCLUDE_PATH),)
  SNAFU =
else
  SNAFU = $(subst :, -I,:$(CPLUS_INCLUDE_PATH))
endif

all:: $(PDLIB) aliases

.SUFFIXES:

CFLAGS += -DPDSUF=\"$(PDSUF)\"

#ifeq ($(PLAIN),1)
#endif

%.hxx.fcs: %.hxx $(COMMON_DEPS)
	$(FILT) $< $@
%.cxx.fcs: %.cxx $(COMMON_DEPS2)
	$(FILT) $< $@
%.m.fcs: %.m $(COMMON_DEPS2)
	$(FILT) $< $@
%.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) -c $< -o $@
%.1.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) -DPASS1 -c $< -o $@
%.2.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) -DPASS2 -c $< -o $@
%.3.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) -DPASS3 -c $< -o $@
%.4.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) -DPASS4 -c $< -o $@
%.o: %.m.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ $(CFLAGS) $(SNAFU) -xobjective-c++ -c $< -o $@
%.s: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) $(CFLAGS) -S $< -o $@
%.e: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) $(CFLAGS) -E $< -o $@

.PRECIOUS: %.hxx.fcs %.cxx.fcs %.h.fcs %.c.fcs %.m.fcs

src/mmx.asm src/mmx_loader.cxx: src/mmx.rb
	$(RUBY) src/mmx.rb src/mmx.asm src/mmx_loader.cxx
src/mmx.o: src/mmx.asm
	nasm -f elf src/mmx.asm -o src/mmx.o

PDLIB1 = gridflow$(PDSUF)
$(PDLIB1): $(OBJS2) $(OBJS) $(H) $(COMMON_DEPS)
	$(CXX) -DPDSUF=\"$(PDSUF)\" $(CFLAGS) $(PDBUNDLEFLAGS) $(LIBPATH) -xnone $(OBJS2) $(OBJS) $(LDSOFLAGS) -o $@

gridflow_gem_loader$(PDSUF): src/gem_loader.cxx.fcs $(H) $(COMMON_DEPS)
	$(CXX) $(CFLAGS) $(PDBUNDLEFLAGS) $(LIBPATH)                              -o $@ -xc++ src/gem_loader.cxx.fcs
gridflow_gem9292$(PDSUF):    src/gem.cxx.fcs        $(H) $(COMMON_DEPS)
	$(CXX) $(CFLAGS) $(PDBUNDLEFLAGS) $(LIBPATH)                              -o $@ -xc++ src/gem.cxx.fcs
gridflow_gem9293$(PDSUF):    src/gem.cxx.fcs        $(H) $(COMMON_DEPS)
	$(CXX) $(CFLAGS) $(PDBUNDLEFLAGS) $(LIBPATH) -DGEMSTATE93                 -o $@ -xc++ src/gem.cxx.fcs
gridflow_gem9393$(PDSUF):    src/gem.cxx.fcs        $(H) $(COMMON_DEPS)
	$(CXX) $(CFLAGS) $(PDBUNDLEFLAGS) $(LIBPATH) -DGEMSTATE93 -DIMAGESTRUCT93 -o $@ -xc++ src/gem.cxx.fcs
gridflow_pdp$(PDSUF):        src/pdp.cxx.fcs        $(H) $(COMMON_DEPS)
	$(CXX) $(CFLAGS) $(PDBUNDLEFLAGS) $(LIBPATH)                              -o $@ -xc++ src/pdp.cxx.fcs
gridflow_unicorn$(PDSUF):    src/unicorn.cxx.fcs    $(H) $(COMMON_DEPS)
	$(CXX) $(CFLAGS) $(PDBUNDLEFLAGS) $(LIBPATH)                              -o $@ -xc++ src/unicorn.cxx.fcs

beep::
	@for z in 1 2 3 4 5; do echo -ne '\a'; sleep 1; done

install::
	@echo -e "\033[0;1;33;41m"
	@echo -e "1. move this folder to lib/pd/extra or add the folder to -path"
	@echo -e "2. delete the old gridflow.pd_linux if you have one (from years ago)"
	@echo -e "3. and don't do \"make install\" anymore\033[0m\n"

DEPRECATED = motion_detection color mouse fade scale_to \
	apply_colormap_channelwise checkers contrast posterize ravel remap_image solarize spread \
	rgb_to_greyscale greyscale_to_rgb rgb_to_yuv yuv_to_rgb rotate in out

ALIASES += deprecated/@fade.pd deprecated/@!.pd doc/flow_classes/@complex_sq-help.pd

aliases:: $(ALIASES)

deprecated/@fade.pd: abstractions/\#fade.pd
	for z in $(DEPRECATED); do cp abstractions/\#$$z.pd deprecated/\@$$z.pd; done

deprecated/@!.pd: deprecated/0x40!.pd
	for z in complex_sq convolve fold inner \! scan; do cp deprecated/0x40$$z.pd deprecated/@$$z.pd; done

doc/flow_classes/@complex_sq-help.pd: doc/flow_classes/0x40complex_sq-help.pd
	cp doc/flow_classes/0x40complex_sq-help.pd doc/flow_classes/@complex_sq-help.pd

clean::
	@-$(RM) gridflow.pd_linux *.o */*.o *.so
	rm -f $(OBJS2) $(OBJS) src/*.fcs format/*.fcs $(patsubst %,deprecated/@%.pd,$(DEPRECATED))

distclean:: clean
	rm -f config.make config.log config.h
	rm -rf tmp
