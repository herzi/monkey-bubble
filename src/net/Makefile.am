NULL=


INCLUDES = \
	$(LIBNETCLTMONKEY_LA_CFLAGS) \
$(MONEY_SRV_CFLAGS) \
	-I../monkey \
	-I../view \
	-I../ui \
	-I../util \
	-g -O2 -Wall -Werror -D_REENTRANT  -D_DEBUG_ -D_TIME_DEBUG_
# -g -O2 -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror -D_REENTRANT  -D_TIME_DEBUG_ -D_DEBUG_

lib_LTLIBRARIES = libnetcltmonkey.la

bin_PROGRAMS = monkey-srv

monkey_srv_SOURCES = \
	monkey-server.c monkey-server.h \
	client-handler.c client-handler.h \
	main.c iclient-handler-observer.c \
	monkey-message.h monkey-client.h

monkey_srv_LDFLAGS =

monkey_srv_LDADD = \
	$(MONKEY_SRV_LIBS) \
	../monkey/libmonkey.la \
	../util/libmonkeyutil.la

libnetcltmonkey_la_SOURCES = \
	monkey-client.c monkey-client.h \
	monkey-server.c monkey-server.h \
	client-handler.c client-handler.h \
	iclient-handler-observer.c monkey-network.h

libnetcltmonkey_la_LDFLAGS =  

libnetcltmonkey_la_LIBADD =  \
	$(LIBNETCLTMONKEY_LA_LIBS)

