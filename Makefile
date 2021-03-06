#!/usr/bin/make

# pexports /c/Program\ Files/pd/extra/Gem/Gem.dll > Gem.def

RUBY = ruby_not_defined
include config.make
COMMON_DEPS = config.make Makefile src/source_filter.rb
COMMON_DEPS2 = $(COMMON_DEPS) src/gridflow.hxx.fcs

SHELL = /bin/sh
RM = rm -f
GFCFLAGS += -Wall -Wno-unused -Wunused-variable -Wno-trigraphs -Wno-deprecated -g -I.

LDSOFLAGS += -lm $(LIBS)
OBJS2 = src/gridflow.o src/grid.o src/classes1.o src/classes2.o src/classes3.o src/expr.o src/classes_gui.o \
  src/numop1.o src/numop2.1.o src/numop2.2.o src/numop2.3.o src/numop2.4.o src/formats.o

# won't work with quartz.m
FCS = src/gridflow.cxx.fcs src/grid.cxx.fcs src/classes1.cxx.fcs src/classes2.cxx.fcs src/classes3.cxx.fcs \
  src/expr.cxx.fcs src/classes_gui.cxx.fcs src/numop1.cxx.fcs src/numop2.cxx.fcs src/formats.cxx.fcs \
  $(subst .o,.cxx.fcs,$(OBJS))

OS = $(shell uname -s | sed -e 's/^MINGW.*/nt/')
FILT = $(RUBY) -w src/source_filter.rb
ifeq ($(OS),Darwin)
  GFCFLAGS += -mmacosx-version-min=10.4 -fPIC
  LDSOFLAGS += -headerpad_max_install_names -bundle -flat_namespace -undefined suppress
  PDSUF = .pd_darwin
  EXECTYPE = macho
  # -undefined dynamic_lookup # is used by smlib. this might be a good idea for future use.
else
  ifeq ($(OS),nt)
    PDSUF = .dll
    LDSOFLAGS += -shared
    LDSOFLAGS += -L/c/Program\ Files/pd/bin -lpd 
    #LDSO_gem_loader += -L/c/Program\ Files/pd/extra/Gem -lGem
    GEMDEPS += libGem.a
    LDSO_gem_loader += -xnone libGem.a
    
    #GFCFLAGS += -DDES_BUGS
    # -mms-bitfields is necessary because of t_object and the way Miller compiles pd.
    GFCFLAGS += -mms-bitfields
    EXECTYPE = notdefined # not a real name
  else
    PDSUF = .pd_linux
    LDSOFLAGS += -shared -rdynamic
    EXECTYPE = elf
    ifeq ($(HAVE_GCC64),1)
      GFCFLAGS += -fPIC
      LDSOFLAGS += -fPIC
      EXECTYPE = elf64
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

fcs:: $(FCS)

.SUFFIXES:

GFCFLAGS += -DPDSUF=\"$(PDSUF)\"

#ifeq ($(PLAIN),1)
#endif

%.hxx.fcs: %.hxx $(COMMON_DEPS)
	$(FILT) $< $@
%.cxx.fcs: %.cxx $(COMMON_DEPS2)
	$(FILT) $< $@
%.m.fcs: %.m $(COMMON_DEPS2)
	$(FILT) $< $@
%.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ -DLIBGF $(GFCFLAGS) -c $< -o $@
%.1.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ -DLIBGF $(GFCFLAGS) -DPASS1 -c $< -o $@
%.2.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ -DLIBGF $(GFCFLAGS) -DPASS2 -c $< -o $@
%.3.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ -DLIBGF $(GFCFLAGS) -DPASS3 -c $< -o $@
%.4.o: %.cxx.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ -DLIBGF $(GFCFLAGS) -DPASS4 -c $< -o $@
%.o: %.m.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ -DLIBGF $(GFCFLAGS) $(SNAFU) -xobjective-c++ -c $< -o $@
.PRECIOUS: %.hxx.fcs %.cxx.fcs %.h.fcs %.c.fcs %.m.fcs

# dur de comprenure...
# found a case where 'make' would pick only the first %.o rule when the 2nd ought to apply.
src/quartz.o: src/quartz.m.fcs $(COMMON_DEPS2)
	$(CXX) -xc++ -DLIBGF $(GFCFLAGS) $(SNAFU) -xobjective-c++ -c src/quartz.m.fcs -o src/quartz.o

src/mmx.asm src/mmx_loader.cxx: src/mmx.rb
	$(RUBY) src/mmx.rb src/mmx.asm src/mmx_loader.cxx
src/mmx.o: src/mmx.asm
	nasm -f $(EXECTYPE) src/mmx.asm -o src/mmx.o

PDLIB1 = gridflow$(PDSUF)
$(PDLIB1): $(OBJS2) $(OBJS) $(COMMON_DEPS2)
	$(CXX) -DPDSUF=\"$(PDSUF)\" $(GFCFLAGS) -xnone  -o $@ $(OBJS2) $(OBJS) $(LDSOFLAGS) $(LDSO_main)

gridflow_gem_loader$(PDSUF): src/gem_loader.cxx.fcs $(COMMON_DEPS2) $(GEMDEPS)
	$(CXX) $(GFCFLAGS)                               -o $@ -xc++ src/gem_loader.cxx.fcs $(LDSOFLAGS) $(LDSO_gem_loader)
gridflow_gem9292$(PDSUF):    src/gem.cxx.fcs        $(COMMON_DEPS2)
	$(CXX) $(GFCFLAGS)                               -o $@ -xc++ src/gem.cxx.fcs        $(LDSOFLAGS) $(LDSO_gem_loader)
gridflow_gem9293$(PDSUF):    src/gem.cxx.fcs        $(COMMON_DEPS2)
	$(CXX) $(GFCFLAGS) -DGEMSTATE93                 -o $@ -xc++ src/gem.cxx.fcs         $(LDSOFLAGS) $(LDSO_gem_loader)
gridflow_gem9393$(PDSUF):    src/gem.cxx.fcs        $(COMMON_DEPS2)
	$(CXX) $(GFCFLAGS) -DGEMSTATE93 -DIMAGESTRUCT93 -o $@ -xc++ src/gem.cxx.fcs         $(LDSOFLAGS) $(LDSO_gem_loader)
gridflow_pdp$(PDSUF):        src/pdp.cxx.fcs        $(COMMON_DEPS2)
	$(CXX) $(GFCFLAGS)                              -o $@ -xc++ src/pdp.cxx.fcs         $(LDSOFLAGS) $(LDSO_pdp)
gridflow_unicorn$(PDSUF):    src/unicorn.cxx.fcs    $(COMMON_DEPS2)
	$(CXX) $(GFCFLAGS)                              -o $@ -xc++ src/unicorn.cxx.fcs     $(LDSOFLAGS) $(LDSO_unicorn)
gridflow_x11$(PDSUF):        src/x11.cxx.fcs        $(COMMON_DEPS2)
	$(CXX) $(GFCFLAGS)                              -o $@ -xc++ src/x11.cxx.fcs         $(LDSOFLAGS) $(LDSO_x11)

beep::
	@for z in 1 2 3 4 5; do echo -ne '\a'; sleep 1; done

install::
	@echo "\033[0;1;33;41mjust add the parent of this folder to the -path and leave me alone\033[0m"

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

libGem.a: src/Gem.def
	dlltool -D /c/Program\ Files/pd/extra/Gem/Gem.dll -d src/Gem.def -l libGem.a

config.make: configure
	./configure
