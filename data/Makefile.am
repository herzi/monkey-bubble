##
## data/Makefile.am for money-bubble
##

leveldir = $(datadir)/monkey-bubble
level_DATA = \
	levels

gladexmldir = $(datadir)/monkey-bubble/glade
gladexml_DATA = \
	monkey-bubble.glade keybinding.glade

schemasdir   = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = monkey-bubble.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schema_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$$p; \
		done \
	fi

EXTRA_DIST = \
	$(level_DATA)	\
	$(gladexml_DATA)\
	$(schemas_in_files)
