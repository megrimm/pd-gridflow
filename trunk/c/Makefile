include ../config.make

ifeq ($(JMAX_VERSION),24)

PNAME=gridflow

all:
	(cd src; $(MAKE) -k all)
.PHONY: all

clean:
	(cd src; $(MAKE) clean)
.PHONY: clean

install:
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt
	( \
		cd lib/$(ARCH)/opt ; \
		$(INSTALL_LIB) lib$(PNAME).so $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt/lib$(PNAME).so ; \
		true \
	)
.PHONY: install

else

all:
	(cd src; $(MAKE) -k all)
.PHONY: all

clean:
	(cd src; $(MAKE) clean)
.PHONY: clean

standalone:
	(cd src; $(MAKE) standalone)
.PHONY: standalone

install:
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt
	( \
		cd lib/$(ARCH)/opt ; \
		$(INSTALL_LIB) lib$(PNAME).so $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt/lib$(PNAME).so ;\
	)
.PHONY: install

endif
