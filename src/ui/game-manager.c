/* game-manager.c - 
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

#include "game-manager.h"

#define PRIVATE(class) class->private

struct GameManagerPrivateClass {

  void (*  start)(GameManager * manager);
  void (*  stop)(GameManager * manager);
};

static guint game_manager_base_init_count = 0;

static void game_manager_base_init(GameManagerClass* game) {

  game_manager_base_init_count++;

  game->private = g_new0(GameManagerPrivateClass,1);
  if (game_manager_base_init_count == 1) {
    /* add signals here */
  }
}

static void game_manager_base_finalize(GameManagerClass* game) {

  game_manager_base_init_count--;
  if (game_manager_base_init_count == 0) {
    /* destroy signals here */
  }
}

GType game_manager_get_type(void) {


  static GType game_manager_type = 0;


  if (!game_manager_type) {
    static const GTypeInfo game_manager_info = {
      sizeof(GameManagerClass),
      (GBaseInitFunc) game_manager_base_init,
      (GBaseFinalizeFunc) game_manager_base_finalize
    };
    
    game_manager_type = g_type_register_static(G_TYPE_INTERFACE, 
															  "GameManager", 
															  &game_manager_info, 
															  0);
    
    g_type_interface_add_prerequisite(game_manager_type, G_TYPE_OBJECT);
  }
  
  return game_manager_type;
}


void game_manager_start(GameManager * game) {


  GameManagerClass* iface = NULL;

  g_assert(IS_GAME_MANAGER(game));
  g_assert(G_IS_OBJECT(game));

  iface = GAME_MANAGER_GET_IFACE(game);
  PRIVATE(iface)->start(game);
}


/**
 * game_stop:
 * @game: a #Game
 * 
 * Stop the game
 **/
void game_manager_stop(GameManager * game) {


  GameManagerClass* iface = NULL;

  g_assert(IS_GAME_MANAGER(game));
  g_assert(G_IS_OBJECT(game));

  iface = GAME_MANAGER_GET_IFACE(game);
  PRIVATE(iface)->stop(game);
}

void game_manager_class_virtual_init(GameManagerClass * class,
												  void (*  start)(GameManager * manager),
												 void (* stop)(GameManager * manager)) {
	 PRIVATE(class)->start = start;
	 PRIVATE(class)->stop = stop;
}
