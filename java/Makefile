include ../config.make

all:
	(cd src ; $(MAKE) all)
	(cd src ; $(MAKE) jar)
.PHONY: all

clean:
	(cd src ; $(MAKE) clean)
.PHONY: clean

install:
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)/java/classes
	( \
		cd classes ; \
		$(INSTALL_DATA) $(PNAME).jar $(lib_install_dir)/packages/$(PNAME)/java/classes/$(PNAME).jar ; \
	)
.PHONY: install





