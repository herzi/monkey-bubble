/* game.c - 
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

#include "game.h"

static guint game_base_init_count = 0;

static void game_base_init(GameClass* game) {

  game_base_init_count++;

  if (game_base_init_count == 1) {
    /* add signals here */
  }
}

static void game_base_finalize(GameClass* game) {

  game_base_init_count--;
  if (game_base_init_count == 0) {
    /* destroy signals here */
  }
}

GType game_get_type(void) {


  static GType game_type = 0;


  if (!game_type) {
    static const GTypeInfo game_info = {
      sizeof(GameClass),
      (GBaseInitFunc) game_base_init,
      (GBaseFinalizeFunc) game_base_finalize
    };
    
    game_type = g_type_register_static(G_TYPE_INTERFACE, 
				       "Game", 
				       &game_info, 
				       0);
    
    g_type_interface_add_prerequisite(game_type, G_TYPE_OBJECT);
  }
  
  return game_type;
}


/**
 * game_start:
 * @game: a #Game
 * 
 * Start the game
 **/
void game_start(Game * game) {


  GameClass* iface = NULL;

  g_assert(IS_GAME(game));
  g_assert(G_IS_OBJECT(game));

  iface = GAME_GET_IFACE(game);
  iface->start(game);
}


/**
 * game_stop:
 * @game: a #Game
 * 
 * Stop the game
 **/
void game_stop(Game * game) {


  GameClass* iface = NULL;

  g_assert(IS_GAME(game));
  g_assert(G_IS_OBJECT(game));

  iface = GAME_GET_IFACE(game);
  iface->stop(game);
}



/**
 * game_pause:
 * @game: a #Game
 * @pause: TRUE to pause, FALSE to unpause
 * 
 * Change pause game state
 **/
void game_pause(Game * game,gboolean pause) {


  GameClass* iface = NULL;

  g_assert(IS_GAME(game));
  g_assert(G_IS_OBJECT(game));

  iface = GAME_GET_IFACE(game);
  iface->pause(game,pause);
}


/**
 * game_get_state:
 * @game: a #Game
 * 
 * Return value : game state
 **/
GameState game_get_state(Game * game) {


  GameClass* iface = NULL;

  g_assert(IS_GAME(game));
  g_assert(G_IS_OBJECT(game));

  iface = GAME_GET_IFACE(game);

  return iface->get_state(game);
}

void game_attach_observer(Game * game,IGameObserver * o) {


  GameClass* iface = NULL;

  g_assert(IS_GAME(game));
  g_assert(G_IS_OBJECT(game));

  iface = GAME_GET_IFACE(game);
  iface->attach_observer(game,o);
}

void game_detach_observer(Game * game,IGameObserver * o) {


  GameClass* iface = NULL;

  g_assert(IS_GAME(game));
  g_assert(G_IS_OBJECT(game));

  iface = GAME_GET_IFACE(game);
  iface->detach_observer(game,o);
}
