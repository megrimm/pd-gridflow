include config.make

ifeq ($(JMAX_VERSION),24)

all:
	(cd c ; $(MAKE) all)
.PHONY: all

clean:
	(cd c ; $(MAKE) clean)
.PHONY: clean

all_c:
	(cd c ; $(MAKE) all)
.PHONY: all_c

clean_c:
	(cd c ; $(MAKE) clean)
.PHONY: clean_c

install:
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)
	$(INSTALL_DATA) $(PNAME).jpk $(lib_install_dir)/packages/$(PNAME)/$(PNAME).jpk
	$(INSTALL_DATA) $(PNAME).scm $(lib_install_dir)/packages/$(PNAME)/$(PNAME).scm
	( cd c ; $(MAKE) $@)
	( cd help; $(MAKE) $@)
.PHONY: install

else



all:
	(cd c; $(MAKE) -k all)
	(cd doc; $(MAKE) -k all)
	#(cd java; $(MAKE) -k all)
.PHONY: all

standalone:
	(cd c; $(MAKE) -k standalone)
.PHONY: standalone

test:
	(cd tests; $(MAKE) -k test)
.PHONY: test

clean:
	(cd c; $(MAKE) clean)
.PHONY: clean

install:
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)
	$(INSTALL_DATA) $(PNAME).jpk $(lib_install_dir)/packages/$(PNAME)/$(PNAME).jpk
	$(INSTALL_DATA) $(PNAME).scm $(lib_install_dir)/packages/$(PNAME)/$(PNAME).scm
	(cd help; $(MAKE) $@)
	(cd templates; $(MAKE) $@)
	(cd java; $(MAKE) $@)
	(cd c; $(MAKE) $@)
.PHONY: install

endif
