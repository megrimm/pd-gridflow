# $Id$

include ../../config.make

#!@#$ would swig -O be better? what about gcc -O2 ? -fastinit ?

LTIPREFIX = /usr/local
#LTIPREFIX = /home/matju
RUBY = ruby

LIBS = $(LTIPREFIX)/lib/ltilib/libltir.a -lpng -ljpeg -L/usr/X11R6/lib -lX11
INCS = -I$(LTIPREFIX)/include/ltilib

PINC = -I/usr/include/python
RDIR2 = `$(RUBY) -rrbconfig -e'h=Config::CONFIG;puts h["sitelibdir"]+"/"+h["arch"]'`
RDIR1 = `$(RUBY) -rrbconfig -e'h=Config::CONFIG;puts h["rubylibdir"]+"/"+h["arch"]'`
RINC = -I $(RDIR1)

SWIGFLAGS = -v
CFLAGS += -Wall -fPIC -g -O2 -fno-omit-frame-pointer -fno-inline

default:: all

all:: alt_types.o ltiPointList_extra.o basedata.so base_functors.so ltimath.so io.so statistics.so filters.so \
		mask_op.so colors.so segmentation.so edge_detectors.so feat_extr.so morph_op.so \
		transformations.so manipulation.so classifiers.so colorspaces.so drawing.so tracking.so


basedata.so: basedata_wrap.o alt_types.o
	$(CXX) -shared -fPIC -o basedata.so alt_types.o basedata_wrap.o $(LIBS) -lstdc++

basedata_wrap.o: basedata_wrap.cxx
	$(CXX) $(CFLAGS) $(RINC) -Ipatched $(INCS) -c basedata_wrap.cxx

basedata_wrap.cxx: ./swig/basedata.i ./swig/dep.i  ./generated/* alt_types.o
	echo $(CXX)
	cp $(LTIPREFIX)/include/ltilib/config.h ./patched/ltiLinConfig.h
	swig $(SWIGFLAGS) -c++ -ruby -I./generated -I./patched \
		$(INCS) -w509,-312,-362,-389,-801,-314 -o ./basedata_wrap.shoo ./swig/basedata.i
	./munge.rb < basedata_wrap.shoo > basedata_wrap.cxx
	sed -i 's/Init_rblti/Init_basedata/' basedata_wrap.cxx



base_functors.so: base_functors_wrap.o alt_types.o
	$(CXX) -shared -fPIC -o base_functors.so alt_types.o base_functors_wrap.o $(LIBS) -lstdc++

base_functors_wrap.o: base_functors_wrap.cxx
	$(CXX) $(CFLAGS) $(RINC) -Ipatched $(INCS) -c base_functors_wrap.cxx

base_functors_wrap.cxx: ./swig/dep.i ./swig/base_functors.i  ./generated/* alt_types.o
	echo $(CXX)
	cp $(LTIPREFIX)/include/ltilib/config.h ./patched/ltiLinConfig.h
	swig $(SWIGFLAGS) -c++ -ruby -I./generated -I./patched \
		$(INCS) -w509,-312,-362,-389,-801,-314 -o ./base_functors_wrap.shoo ./swig/base_functors.i
	./munge.rb < base_functors_wrap.shoo > base_functors_wrap.cxx
	sed -i 's/Init_rblti/Init_base_functors/' base_functors_wrap.cxx

alt_types.o: ./src/alt_types.cpp
	g++ -c -I./generated -I./patched $(INCS) ./src/alt_types.cpp

ltiPointList_extra.o: ./src/ltiPointList_extra.cpp
	g++ -c -I./generated -I./patched $(INCS) ./src/ltiPointList_extra.cpp


.PRECIOUS: %_wrap.cxx

%.so: %_wrap.o alt_types.o
	FOO=$$(echo $@ | sed 's/\.so$$//') ;\
	$(CXX) -shared -fPIC -o $@ alt_types.o $${FOO}_wrap.o $(LIBS) -lstdc++

%_wrap.o: %_wrap.cxx
	FOO=$$(echo $@ | sed 's/_wrap\.o$$//') ;\
	$(CXX) $(CFLAGS) $(RINC) -Ipatched $(INCS) -c $${FOO}_wrap.cxx

%_wrap.cxx: ./swig/base_functors.i ./swig/dep.i ./swig/%.i  ./generated/* alt_types.o
	cp $(LTIPREFIX)/include/ltilib/config.h ./patched/ltiLinConfig.h
	FOO=$$(echo $@ | sed 's/_wrap\.cxx$$//') ;\
	echo FOO is $$FOO ;\
	swig $(SWIGFLAGS) -c++ -ruby -I./generated -I./patched \
		$(INCS) -w509,-312,-362,-389,-801,-314 -o ./$${FOO}_wrap.shoo ./swig/$$FOO.i ;\
	./munge.rb < $${FOO}_wrap.shoo > $${FOO}_wrap.cxx ;\
	sed -i "s/Init_rblti/Init_$$FOO/" $@

tracking.so: tracking_wrap.o alt_types.o ltiPointList_extra.o blobEMTracker.o
	$(CXX) -shared -fPIC -o tracking.so ltiPointList_extra.o alt_types.o blobEMTracker.o tracking_wrap.o $(LIBS) -lstdc++

blobEMTracker.o: ./src/blobEMTracker.cpp  ./patched/blobEMTracker.h
	$(CXX) $(CFLAGS) $(RINC) -Ipatched $(INCS) -o ./blobEMTracker.o -c ./src/blobEMTracker.cpp

tracking_wrap.o: tracking_wrap.cxx
	$(CXX) $(CFLAGS) $(RINC) -Ipatched $(INCS) -c tracking_wrap.cxx

tracking_wrap.cxx: ./swig/tracking.i ./swig/dep.i  ./generated/* alt_types.o ./patched/blobEMTracker.h
	echo $(CXX)
	cp $(LTIPREFIX)/include/ltilib/config.h ./patched/ltiLinConfig.h
	swig $(SWIGFLAGS) -c++ -ruby -I./generated -I./patched \
		$(INCS) -w509,-312,-362,-389,-801,-314 -o ./tracking_wrap.shoo ./swig/tracking.i
	./munge.rb < tracking_wrap.shoo > tracking_wrap.cxx
	sed -i 's/Init_rblti/Init_tracking/' tracking_wrap.cxx



clean::
	rm -f *.so *_wrap.o *.cxx

install:: ltimath.so basedata.so base_functors.so io.so
	cp *.so $(RDIR2)

