
NULL=


INCLUDES = \
	$(LIBMONKEY_A_CFLAGS) \
	-Wall -Werror -g


noinst_LIBRARIES = libmonkey.a

marshal_sources = 			\
	monkey-marshal.h		\
	monkey-marshal.c		\
	$(NULL)

libmonkey_a_SOURCES = \
	bubble.c bubble.h\
	shooter.c shooter.h\
	board.c board.h\
	playground.c playground.h\
	monkey.c monkey.h\
	point.h color.h \
	$(marshal_sources)

$(libmonkey_a_OBJECTS): $(marshal_sources)

monkey-marshal.c monkey-marshal.h: monkey-marshal.list
	glib-genmarshal --prefix=monkey_marshal --body $^ > monkey-marshal.c
	glib-genmarshal --prefix=monkey_marshal --header $^ > monkey-marshal.h


CLEANFILES = monkey-marshal.c monkey-marshal.h

EXTRA_DIST = monkey-marshal.list

