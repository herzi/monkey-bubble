/* monkey-network.h - 
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
 
#ifndef _MONKEY_NETWORK_H
#define _MONKEY_NETWORK_H

G_BEGIN_DECLS
#include <gtk/gtk.h>

typedef struct ClientHandlerPrivate ClientHandlerPrivate;
typedef struct _clientHandler {
	GObject parent_instance;	
	ClientHandlerPrivate* private;
} ClientHandler;

typedef struct MonkeyServerPrivate MonkeyServerPrivate;
typedef struct _monkeyServer {
	GObject parent_instance;	
	MonkeyServerPrivate* private;
} MonkeyServer;

G_END_DECLS
#endif

