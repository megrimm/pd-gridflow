# $Id$
include ../config.make

ifeq ($(JMAX_VERSION),24)

all::
	(cd src; $(MAKE) -k all)

clean::
	(cd src; $(MAKE) clean)

install::
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt
	( \
		cd lib/$(ARCH)/opt ; \
		$(INSTALL_LIB) lib$(PNAME).so $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt/lib$(PNAME).so ; \
		true \
	)

else

all::
	(cd src; $(MAKE) -k all)

clean::
	(cd src; $(MAKE) clean)

standalone::
	(cd src; $(MAKE) standalone)

install::
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt
	( \
		cd lib/$(ARCH)/opt ; \
		$(INSTALL_LIB) lib$(PNAME).so $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt/lib$(PNAME).so ;\
	)

endif
