# $Id$
include ../config.make

all::
	(cd src; $(MAKE) -k all)

clean::
	(cd src; $(MAKE) clean)
	(cd src; $(MAKE) clean-standalone)

standalone::
	(cd src; $(MAKE) standalone)

install::
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt
	( \
		cd lib/$(ARCH)/opt ; \
		$(INSTALL_LIB) lib$(PNAME).so $(lib_install_dir)/packages/$(PNAME)/c/lib/$(ARCH)/opt/lib$(PNAME).so ;\
	)

