## Process this file with automake to produce Makefile.in


SUBDIRS = \
	data	\
	pixmaps	\
        src     \
        po      \
        debian

applications_in_files = monkey-bubble.desktop.in
applications_DATA = $(applications_in_files:.desktop.in=.desktop)
applicationsdir = $(datadir)/applications

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(applications_in_files)	\
	intltool-extract.in		\
	intltool-merge.in		\
	intltool-update.in		\
	po

