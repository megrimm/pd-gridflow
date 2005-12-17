#LTIPREFIX = /usr/local
LTIPREFIX = /home/matju
LIBS = $(LTIPREFIX)/lib/ltilib/libltid.a -lpng -ljpeg
INCS = -I$(LTIPREFIX)/include/ltilib -I/home/matju/share/swig/1.3.27/std
PINC = -I/usr/include/python
RINC = -I`ruby -rrbconfig -e'h=Config::CONFIG;puts h["rubylibdir"]+"/"+h["arch"]'`

ruby:: rblti.so
	ruby -e 'require "rblti"'

rblti.so: rblti_wrap.o Makefile
	g++ -shared -o rblti.so rblti_wrap.o $(LIBS)

rblti_wrap.o: rblti_wrap.cxx
	g++ $(RINC) -Ipatched $(INCS) -c rblti_wrap.cxx

rblti_wrap.s: rblti_wrap.cxx
	g++ $(RINC) -Ipatched $(INCS) -S rblti_wrap.cxx

rblti_wrap.cxx: rblti.i
	swig -c++ -ruby -DHAVE_LIBJPEG -DHAVE_LIBPNG -I./generated -I./patched \
		$(INCS) -w509,-312,-362,-389,-801,-314 rblti.i \
	|| true

#rblti_wrap.cxx: rblti_wrap.cxx.bz2
#	bzip -dc rblti_wrap.cxx.bz2 > rblti_wrap.cxx

clean::
	rm -f rblti.so rblti_wrap.o rblti_wrap.cxx


