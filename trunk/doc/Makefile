XML = \
install.xml \
project_policy.xml \
reference.xml \
format.xml \
internals.xml \
architecture.xml

HTML = $(subst .xml,.html,$(XML))

all:: $(HTML)

%.html: %.xml moulinette.rb
	ruby moulinette.rb < $< > $@ \
		|| (rm $@ && false)

clean::
	rm $(HTML)

