include ../config.make
PNAME=video4jmax

install:
	$(INSTALL_DIR) $(lib_install_dir)/packages/$(PNAME)/templates
	for f in *.jmax; do \
		$(INSTALL_DATA) $$f $(lib_install_dir)/packages/$(PNAME)/templates/$$f; \
	done
.PHONY: install
