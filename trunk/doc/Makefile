reference.html: reference.xml jmax.dtd jmaxml_to_html.rb
	ruby jmaxml_to_html.rb < reference.xml > $@ \
		|| (rm $@ && false)

clean::
	rm reference.html

