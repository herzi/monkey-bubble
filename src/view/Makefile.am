NULL=


INCLUDES = \
	$(LIBGDKVIEW_LA_CFLAGS) \
	-I../monkey \
	-g -O2 -Wall -Werror -D_REENTRANT -DDATADIR=\""$(datadir)"\"

lib_LTLIBRARIES = libgdkview.la



libgdkview_la_SOURCES = \
	gdk-canvas.h gdk-canvas.c \
	gdk-view.h gdk-view.c

libgdkview_la_LDFLAGS = 

libgdkview_la_LIBADD =  \
	$(LIBGDKVIEW_LA_LIBS)

