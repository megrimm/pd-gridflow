# this makefile is not threadsafe, sorry.
# so don't use the -j option on this one.
# (however you can on the main makefile.)

CLASSPATH = .:/opt/lib/jmax/java/classes/jmax.jar:/opt/lib/jmax/packages/guiobj/java/classes/guiobj.jar:/opt/lib/jmax/java/lib/jacl/jacl.jar:/opt/lib/jmax/java/lib/jacl/tcljava.jar

OBJS = \
Peephole.class PeepholeControlPanel.class GridFlowExtension.class \
PathInfo.class

# libPeephole.so

JC = $(JAVA_HOME)/bin/javac -classpath $(CLASSPATH)

all:: gridflow.jar

clean::
	rm -f $(OBJS)
	
#libPeephole.so: Peephole_linux.c Peephole.h
#	gcc -g -I$(JAVA_HOME)/include \
#	-I$(JAVA_HOME)/include/genunix -I$(JAVA_HOME)/include/linux \
#	-L/usr/X11R6/lib -lX11 -shared -o $@ Peephole_linux.c
#
#Peephole.h: Peephole.class
#	-rm Peephole.h
#	$(JAVA_HOME)/bin/javah -jni -classpath $(CLASSPATH) Peephole

Peephole.class PeepholeControlPanel.class GridFlowExtension.class: Peephole.java PeepholeControlPanel.java GridFlowExtension.java PathInfo.class
	$(JC) PathInfo.java FtsPeepholeObject.java Peep*.java GridFlowExtension.java

#GridFlowExtension.class: GridFlowExtension.java FtsPeepholeObject.class
#	$(JC) GridFlowExtension.java

gridflow.jar: $(OBJS)
	/bin/rm -f gridflow.jar
	-ln -s . gridflow
	jar -cvf gridflow.jar gridflow/*.class # gridflow/*.so

PathInfo.class: PathInfo.java
	$(JC) PathInfo.java

