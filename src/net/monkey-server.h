/* monkey-server.h - 
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
#ifndef _MONKEY_SERVER_H_
#define _MONKEY_SERVER_H_
#include <glib.h>
#include "monkey-message.h"
#include "monkey-network.h"
#include "game.h"
#define MONKEY_PORT 66666

G_BEGIN_DECLS

#define TYPE_MONKEY_SERVER            (monkey_server_get_type())

#define MONKEY_SERVER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_MONKEY_SERVER,MonkeyServer))
#define MONKEY_SERVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MONKEY_SERVER,MonkeyServerClass))
#define IS_MONKEY_SERVER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_MONKEY_SERVER))
#define IS_MONKEY_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MONKEY_SERVER))
#define MONKEY_SERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MONKEY_SERVER, MonkeyServerClass))

typedef struct {
  GObjectClass parent_class;
} MonkeyServerClass;


GType monkey_server_get_type(void);
MonkeyServer * monkey_server_new(void);
gboolean monkey_server_init(MonkeyServer *, unsigned short);
void *monkey_server_start(MonkeyServer *);
void monkey_server_kill_client(ClientHandler *, MonkeyServer *);
gboolean monkey_server_propagate_network_update(MonkeyMessage *,MonkeyServer *);
GSList *monkey_server_get_monkeys(MonkeyServer *);
gint monkey_server_get_game_state(MonkeyServer *);
gboolean monkey_server_start_game(MonkeyServer *,MonkeyMessage *);
void monkey_server_update_game_state(MonkeyServer *, MonkeyMessage *);
gint monkey_server_get_time(MonkeyServer *);
unsigned long monkey_server_get_game_offset(MonkeyServer *);

G_END_DECLS

#endif
