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

enum {
  STATE_CHANGED,
  LAST_SIGNAL
};

static guint32 signals[LAST_SIGNAL];

static GObjectClass* parent_class = NULL;

static void game_finalize(GObject * object) {

  //  Game * game = GAME(object);

 if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }  
}

static void game_instance_init(Game * game) {
}

static void game_class_init(GameClass* klass) {

    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = game_finalize;
    
    signals[STATE_CHANGED] = g_signal_new ("state-changed",
					   G_TYPE_FROM_CLASS (klass),
					   G_SIGNAL_RUN_FIRST |
					   G_SIGNAL_NO_RECURSE,
					   G_STRUCT_OFFSET (GameClass, state_changed),
					   NULL, NULL,
					   g_cclosure_marshal_VOID__VOID,
					   G_TYPE_NONE,
					   0,NULL);
    
}

GType game_get_type(void) {


  static GType game_type = 0;


  if (!game_type) {
    static const GTypeInfo game_info = {
      sizeof(GameClass),
	NULL,           /* base_init */
	NULL,           /* base_finalize */
	(GClassInitFunc) game_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data */
	sizeof(Game),
	1,              /* n_preallocs */
	(GInstanceInitFunc) game_instance_init,

    };

      
    game_type = g_type_register_static(G_TYPE_OBJECT,
				       "Game",
				       &game_info, 0);

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
  iface = GAME_GET_CLASS(game);

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

  iface = GAME_GET_CLASS(game);
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
  
  iface = GAME_GET_CLASS(game);
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

  iface = GAME_GET_CLASS(game);

  return iface->get_state(game);
}

void game_notify_changed(Game * game) {
  g_signal_emit( G_OBJECT(game),signals[STATE_CHANGED],0);
}
