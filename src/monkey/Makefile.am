
NULL=


INCLUDES = \
	$(LIBMONKEY_LA_CFLAGS) \
	-Wall -Werror -g


lib_LTLIBRARIES = libmonkey.la

libmonkey_la_SOURCES = \
	bubble.c bubble.h\
	shooter.c shooter.h\
	board.c board.h\
	playground.c playground.h\
	monkey.c monkey.h\
	point.h color.h \
	monkey-marshal.c monkey-marshal.h


monkey-marshal.c: monkey-marshal.list $(GLIB_GENMARSHAL)
	echo "#include <monkey-marshal.h>" > $@ && $(GLIB_GENMARSHAL) $< --body --prefix=monkey_marshal >> $@

monkey-marshal.h: monkey-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=monkey_marshal > $@


libmonkey_la_LDFLAGS =  

libmonkey_la_LIBADD =  \
	$(LIBMONKEY_LA_LIBS)

EXTRA_DIST = monkey-marshal.list
