NULL=


INCLUDES = \
	$(LIBGSTAUDIO_LA_CFLAGS) \
	-I../monkey \
	-g -O2 -Wall -Werror -D_REENTRANT -DDATADIR=\""$(datadir)"\"

lib_LTLIBRARIES = libgstaudio.la

bin_PROGRAMS = gsttest

gsttest_SOURCES = \
	gst-audio.h gst-audio.c \
	test.c

gsttest_LDFLAGS = 
gsttest_LDADD =   \
	$(LIBGSTAUDIO_LA_LIBS)


libgstaudio_la_SOURCES = \
	gst-audio.h gst-audio.c

libgstaudio_la_LDFLAGS = 

libgstaudio_la_LIBADD =  \
	$(LIBGSTAUDIO_LA_LIBS)

