/* network-game.h - 
 * Copyright (C) 2002 Christophe Segui
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 
 #ifndef NETWORK_GAME_H
#define NETWORK_GAME_H

#include <gtk/gtk.h>

#include "gdk-canvas.h"

#include "game.h"
#include "monkey-server.h"
G_BEGIN_DECLS

#define TYPE_NETWORK_GAME           (network_game_get_type())

#define NETWORK_GAME(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_NETWORK_GAME,NetworkGame))
#define NETWORK_GAME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_NETWORK_GAME,NetworkGameClass))
#define IS_NETWORK_GAME(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_NETWORK_GAME))
#define IS_NETWORK_GAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_NETWORK_GAME))
#define NETWORK_GAME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_NETWORK_GAME, NetworkGameClass))

typedef struct NetworkGamePrivate NetworkGamePrivate;

#define TYPE_INETWORK_GAME_OBSERVER        (inetwork_game_observer_get_type())

#define INETWORK_GAME_OBSERVER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_INETWORK_GAME_OBSERVER,INetworkGameObserver))
#define IS_INETWORK_GAME_OBSERVER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_INETWORK_GAME_OBSERVER))

#define INETWORK_GAME_OBSERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_INETWORK_GAME_OBSERVER, INetworkGameObserverClass))
#define INETWORK_GAME_OBSERVER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE  ((obj), TYPE_INETWORK_GAME_OBSERVER, INetworkGameObserverClass))

typedef struct INetworkGameObserver INetworkGameObserver;


typedef struct {
  GObject parent_instance;
  NetworkGamePrivate * private;
} NetworkGame;

typedef struct {
  GObjectClass parent_class;
} NetworkGameClass;


GType network_game_get_type(void);

NetworkGame * network_game_new(GtkWidget * window,GdkCanvas * canvas, gchar *, unsigned short);
void network_game_send_message(NetworkGame *, MonkeyMessage *);
void network_game_attach_observer(NetworkGame * game,INetworkGameObserver *);
void network_game_detach_observer(NetworkGame * game,INetworkGameObserver *);
gint network_game_get_time(NetworkGame *);
void network_game_time_init(NetworkGame *, GTimeVal *);
void network_game_update_monkey(NetworkGame *, MonkeyMessage *);
void network_game_draw_foreign_monkeys(NetworkGame *);
void network_game_set_game_offset(NetworkGame *, unsigned long);
void network_game_abort(NetworkGame *);   						
	

G_END_DECLS

#endif
