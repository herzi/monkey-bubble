/* game_2_player.h
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

#ifndef GAME_2_PLAYER_H
#define GAME_2_PLAYER_H

#include <gtk/gtk.h>
#include "game.h"
#include "monkey.h"
#include "monkey-canvas.h"
G_BEGIN_DECLS

#define PLAYER_1 1
#define PLAYER_2 2
#define TYPE_GAME_2_PLAYER            (game_2_player_get_type())

#define GAME_2_PLAYER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_GAME_2_PLAYER,Game2Player))
#define GAME_2_PLAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GAME_2_PLAYER,Game2PlayerClass))
#define IS_GAME_2_PLAYER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_GAME_2_PLAYER))
#define IS_GAME_2_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GAME_2_PLAYER))
#define GAME_2_PLAYER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_GAME_2_PLAYER, Game2PlayerClass))

typedef struct Game2PlayerPrivate Game2PlayerPrivate;



typedef struct {
  Game parent_instance;
  Game2PlayerPrivate * private;
} Game2Player;

typedef struct {
  GameClass parent_class;
} Game2PlayerClass;


GType game_2_player_get_type(void);

Game2Player * game_2_player_new(GtkWidget * window,MonkeyCanvas * canvas,int score_left,int score_right);

int game_2_player_get_winner(Game2Player * g);



G_END_DECLS





#endif
