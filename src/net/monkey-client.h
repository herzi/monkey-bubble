/* monkey-client.h - 
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

#ifndef _MONKEY_CLIENT_H_
#define _MONKEY_CLIENT_H_
#include <stdlib.h>
#include <glib.h>
#include <glib/gthread.h>
#include <gtk/gtk.h>
#include "monkey-message.h"
#include "monkey.h"
#include "network-game.h"

G_BEGIN_DECLS

#define TYPE_MONKEY_CLIENT            (monkey_client_get_type())

#define MONKEY_CLIENT(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_MONKEY_CLIENT,MonkeyClient))
#define MONKEY_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MONKEY_CLIENT,MonkeyClientClass))
#define IS_MONKEY_CLIENT(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_MONKEY_CLIENT))
#define IS_MONKEY_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MONKEY_CLIENT))
#define MONKEY_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MONKEY_CLIENT, MonkeyClientClass))

typedef struct MonkeyClientPrivate MonkeyClientPrivate;

typedef struct _monkeyClient {
	GObject parent_instance;
	MonkeyClientPrivate *private;
} MonkeyClient;

typedef struct {
  GObjectClass parent_class;
} MonkeyClientClass;

GType monkey_client_get_type(void);
MonkeyClient *monkey_client_new(NetworkGame *);
gboolean monkey_client_init(unsigned short, char *, MonkeyClient *);
void monkey_client_start(MonkeyClient *);
GThread *monkey_client_get_thread(MonkeyClient *);
gboolean monkey_client_send_update_to_monkey_server(MonkeyMessage *, MonkeyClient *) ;
GHashTable *monkey_client_get_other_monkey_players(MonkeyClient *);

G_END_DECLS

#endif
