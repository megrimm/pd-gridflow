include ../../config.make
PACKAGE_ROOT = ../..
PACKAGE_DIR = 

include $(JMAXROOTDIR)/java/Makefiles/Makefile.package

#DOC_PACKAGES =  $(PNAME)

JAR_FILE=$(PNAME).jar

ALL_CLASSES=$(shell find $(JAVA_CLASSES_ROOT)/$(PNAME) -name '*.class')

all: $(JAVA_CLASSES_ROOT)
	(cd $(PNAME); $(MAKE) all)
.PHONY: all

clean:
	(cd $(PNAME); $(MAKE) clean)
.PHONY: clean

#doc:  
#	(CLASSPATH=$(JMAX_CLASSPATH) ; export CLASSPATH ; javadoc -d $(PACKAGE_DIR)/java/doc $(ALL_PACKAGES))
#.PHONY: doc

#doc_private:  
#	(CLASSPATH=$(JMAX_CLASSPATH) ; export CLASSPATH ; javadoc -private -d $(ROOTDIR)/java/doc_private $(ALL_PACKAGES))
#.PHONY: doc_private

jar: $(JAVA_CLASSES_ROOT)/$(JAR_FILE)
.PHONY: jar

$(JAVA_CLASSES_ROOT)/$(JAR_FILE): $(ALL_CLASSES)
	(cd $(JAVA_CLASSES_ROOT) ; /bin/rm -f $(JAR_FILE) ; jar -cvf $(JAR_FILE) $(PNAME) )

include $(JMAXROOTDIR)/java/Makefiles/Makefile.package.misc

