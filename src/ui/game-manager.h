/* game-manager.h
 * Copyright (C) 2002 Laurent Belmonte
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

#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <gtk/gtk.h>

typedef struct GameManager GameManager;
#define TYPE_GAME_MANAGER        (game_manager_get_type())

#define GAME_MANAGER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object),TYPE_GAME_MANAGER,GameManager))

#define IS_GAME_MANAGER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_GAME_MANAGER))

#define GAME_MANAGER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj),	TYPE_GAME_MANAGER, GameManagerClass))

typedef struct GameManagerPrivateClass GameManagerPrivateClass;

typedef struct {
	 GTypeInterface root_interface;
	 GameManagerPrivateClass * private;
} GameManagerClass;


GType game_manager_get_type(void);

void game_manager_start(GameManager * game);
void game_manager_stop(GameManager * game);


void game_manager_class_virtual_init(GameManagerClass * class,
												  void (*  start)(GameManager * manager),
												  void (* stop)(GameManager * manager));

G_END_DECLS


#endif
