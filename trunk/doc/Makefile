XML = \
install.xml \
reference.xml \
project_policy.xml

HTML = $(subst .xml,.html,$(XML))

all:: $(HTML)

%.html: %.xml jmax.dtd jmaxml_to_html.rb
	ruby jmaxml_to_html.rb < $< > $@ \
		|| (rm $@ && false)

clean::
	rm $(HTML)

