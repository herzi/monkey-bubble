/* game.h
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

#ifndef GAME_H
#define GAME_H

#include <gtk/gtk.h>
#include "monkey.h"

#define TYPE_GAME        (game_get_type())

#define GAME(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object),TYPE_GAME,Game))

#define IS_GAME(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_GAME))

#define GAME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),	TYPE_GAME, GameClass))


#define GameState int

// 0-9 reserved
enum GameStateCode {
	GAME_NOT_READY =0,
	GAME_READY  		   =1,
	GAME_STARTED 		=2,
	GAME_PLAYING 		=3,
	GAME_FINISHED 		=4,
	GAME_PAUSED 		=5,
	GAME_STOPPED		=6,
};


typedef struct Game {
  GObject parent_instance;
} Game;

typedef struct {
  GObjectClass parent_class;

  void (*start)(Game * game);
  void (*stop)(Game * game);
  void (*pause)(Game * game,gboolean pause);
  GameState (*get_state)(Game * game);
  void (*state_changed)(Game * game);

} GameClass;


GType game_get_type(void);

void game_start(Game * game);
void game_stop(Game * game);
void game_pause(Game * game,gboolean pause);

GameState game_get_state(Game * game);

// protected ?!
void game_notify_changed(Game * game);

G_END_DECLS


#endif
