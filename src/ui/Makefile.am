
NULL=

INCLUDES = \
	$(UI_CFLAGS) \
	-I../monkey \
	-I../view \
	-I../util  -DDATADIR=\""$(datadir)"\" \
	-Wall -Werror


bin_PROGRAMS = monkey-bubble

monkey_bubble_SOURCES = \
	game.c game.h \
	game-1-player.c game-1-player.h \
	game-2-player.c game-2-player.h \
	game-manager.c game-manager.h \
	game-1-player-manager.c game-1-player-manager.h \
	game-2-player-manager.c game-2-player-manager.h \
	main.c \
	keyboard-properties.c keyboard-properties.h \
	ui-main.c ui-main.h 

monkey_bubble_LDFLAGS=


monkey_bubble_LDADD =  \
	$(UI_LIBS) \
	../monkey/libmonkey.la \
	../view/libgdkview.la \
	../util/libmonkeyutil.la 
