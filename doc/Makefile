%.html: %.xml jmax.dtd jmaxml_to_html.rb
	ruby jmaxml_to_html.rb < $< > $@ \
		|| (rm $@ && false)

all:: reference.html project_policy.html


clean::
	rm reference.html

