# $Id$
include config.make

all::
	(cd c; $(MAKE) -k all)
	#(cd doc; $(MAKE) -k all)
	#(cd java; $(MAKE) -k all)

standalone::
	(cd c; $(MAKE) -k standalone)

test::
	(cd tests; $(MAKE) -k test)

clean::
	(cd c; $(MAKE) clean)

install::
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)
	$(INSTALL_DATA) $(PNAME).jpk $(lib_install_dir)/packages/$(PNAME)/$(PNAME).jpk
	$(INSTALL_DATA) $(PNAME).scm $(lib_install_dir)/packages/$(PNAME)/$(PNAME).scm
	(cd help; $(MAKE) $@)
	(cd templates; $(MAKE) $@)
	#(cd java; $(MAKE) $@)
	(cd c; $(MAKE) $@)
kloc::
	wc c/src/*.[ch] configure extra/*.rb


