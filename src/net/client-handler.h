/* client-handler.h - 
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
#ifndef _CLIENT_HANDLER_H_
#define _CLIENT_HANDLER_H_
#include "monkey-server.h"
#include "monkey-message.h"
#include "monkey-network.h"
#include "monkey.h"
#include <glib.h>
#include <glib/gthread.h>

G_BEGIN_DECLS

#define TYPE_CLIENT_HANDLER            (client_handler_get_type())

#define CLIENT_HANDLER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_CLIENT_HANDLER,ClientHandler))
#define CLIENT_HANDLER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CLIENT_HANDLER,ClientHandlerClass))
#define IS_CLIENT_HANDLER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_CLIENT_HANDLER))
#define IS_CLIENT_HANDLER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CLIENT_HANDLER))
#define CLIENT_HANDLER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CLIENT_HANDLER, ClientHandlerClass))

#define TYPE_ICLIENT_HANDLER_OBSERVER        (iclient_handler_observer_get_type())
#define ICLIENT_HANDLER_OBSERVER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_ICLIENT_HANDLER_OBSERVER,IClientHandlerObserver))
#define IS_ICLIENT_HANDLER_OBSERVER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_ICLIENT_HANDLER_OBSERVER))
#define ICLIENT_HANDLER_OBSERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_ICLIENT_HANDLER_OBSERVER, IClientHandlerObserverClass))
#define ICLIENT_HANDLER_OBSERVER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), TYPE_ICLIENT_HANDLER_OBSERVER, IClientHandlerObserverClass))

typedef struct IClientHandlerObserver IClientHandlerObserver;

typedef struct {
  GObjectClass parent_class;
} ClientHandlerClass;

GType client_handler_get_type(void);
ClientHandler* client_handler_new(int, MonkeyServer *);
void *client_handler_start(ClientHandler *);
void client_handler_set_thread(ClientHandler *, GThread *);
gboolean send_update_to_client(MonkeyMessage *, ClientHandler *);
Monkey *client_handler_get_monkey(ClientHandler *);
gint client_handler_get_id(ClientHandler *);
void client_handler_attach_observer(ClientHandler * ,IClientHandlerObserver *);
void client_handler_detach_observer(ClientHandler * ,IClientHandlerObserver *);
gint client_handler_get_state(ClientHandler *);
void client_handler_start_game(ClientHandler *);
void client_handler_set_game_creator(ClientHandler *);


typedef struct {
  GTypeInterface root_interface;

  void (* event) (IClientHandlerObserver *, ClientHandler *, MonkeyMessage *);

} IClientHandlerObserverClass;


GType iclient_handler_observer_get_type(void);

void iclient_handler_observer_event(IClientHandlerObserver *,ClientHandler *, MonkeyMessage *);

G_END_DECLS

#endif
