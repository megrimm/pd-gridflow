XML = \
install.xml \
project_policy.xml \
reference.xml \
format.xml \
internals.xml \
architecture.xml

HTML = $(subst .xml,.html,$(XML))

all:: $(HTML)

%.html: %.xml jmax.dtd jmaxml_to_html.rb
	ruby jmaxml_to_html.rb < $< > $@ \
		|| (rm $@ && false)

clean::
	rm $(HTML)

