include ../config.make
PNAME=gridflow

install:
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)/help
	for f in *.tcl *.scm *.jmax; do \
		$(INSTALL_DATA) $$f $(lib_install_dir)/packages/$(PNAME)/help/$$f; \
	done
.PHONY: install
