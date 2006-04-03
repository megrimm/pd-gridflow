# $Id$

include ../../config.make

#!@#$ would swig -O be better? what about gcc -O2 ? -fastinit ?

#LTIPREFIX = /usr/local
 LTIPREFIX = /home/matju
LIBS = $(LTIPREFIX)/lib/ltilib/libltir.a -lpng -ljpeg
INCS = -I$(LTIPREFIX)/include/ltilib

PINC = -I/usr/include/python
RDIR2 = `ruby -rrbconfig -e'h=Config::CONFIG;puts h["sitelibdir"]+"/"+h["arch"]'`
RDIR1 = `ruby -rrbconfig -e'h=Config::CONFIG;puts h["rubylibdir"]+"/"+h["arch"]'`
RINC = -I $(RDIR1)

SWIGFLAGS = -v -Wall
CFLAGS += -Wall

default:: ruby

python:: _pylti.so
	python -c 'import pylti'
	python testpylti.py lemur.jpeg

_pylti.so: pylti_wrap.o Makefile
	$(CXX) -shared -o _pylti.so pylti_wrap.o $(LIBS)

pylti_wrap.o: pylti_wrap.cxx
	$(CXX) $(CFLAGS) $(PINC) -Ipatched $(INCS) -c pylti_wrap.cxx

pylti_wrap.cxx: ./swig/pylti.i
	swig $(SWIGFLAGS) -c++ -python -DHAVE_LIBJPEG -DHAVE_LIBPNG -I./generated -I./patched \
		$(INCS) -w509,-312,-362,-389  -o ./pylti_wrap.cxx ./swig/pylti.i
	#swig -c++ -python -DHAVE_LIBJPEG -DHAVE_LIBPNG -I./generated_python -I./patched \
	#	$(INCS) -w509,-312,-362,-389 pylti.i

ruby:: rblti.so
	ruby -I. testrblti.rb

rblti.so: rblti_wrap.o Makefile
	$(CXX) -shared -o rblti.so rblti_wrap.o $(LIBS) -lstdc++

rblti_wrap.o: rblti_wrap.cxx
	$(CXX) $(CFLAGS) $(RINC) -Ipatched $(INCS) -c rblti_wrap.cxx

rblti_wrap.s: rblti_wrap.cxx
	$(CXX) $(CFLAGS) $(RINC) -Ipatched $(INCS) -S rblti_wrap.cxx

rblti_wrap.cxx: ./swig/rblti.i
	swig $(SWIGFLAGS) -c++ -ruby -I./generated -I./patched \
		$(INCS) -w509,-312,-362,-389,-801,-314 -o ./rblti_wrap.cxx ./swig/rblti.i
	sed -i 's/int >>/int> >/' rblti_wrap.cxx

#rblti_wrap.cxx: rblti_wrap.cxx.bz2
#	bunzip2 -c rblti_wrap.cxx.bz2 > rblti_wrap.cxx

clean::
	rm -f \
		_pylti.so pylti_wrap.o pylti_wrap.cxx \
		rblti.so rblti_wrap.o rblti_wrap.cxx

install:: #rblti.so
	cp rblti.so $(RDIR2)

