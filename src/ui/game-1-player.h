/* game_1_player.h
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

#ifndef GAME_1_PLAYER_H
#define GAME_1_PLAYER_H

#include <gtk/gtk.h>
#include "game.h"
#include "gdk-canvas.h"
#include "monkey.h"
G_BEGIN_DECLS

#define TYPE_GAME_1_PLAYER            (game_1_player_get_type())

#define GAME_1_PLAYER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_GAME_1_PLAYER,Game1Player))
#define GAME_1_PLAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GAME_1_PLAYER,Game1PlayerClass))
#define IS_GAME_1_PLAYER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_GAME_1_PLAYER))
#define IS_GAME_1_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GAME_1_PLAYER))
#define GAME_1_PLAYER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_GAME_1_PLAYER, Game1PlayerClass))

typedef struct Game1PlayerPrivate Game1PlayerPrivate;



typedef struct {
  Game parent_instance;
  Game1PlayerPrivate * private;
} Game1Player;

typedef struct {
  GameClass parent_class;
} Game1PlayerClass;


GType game_1_player_get_type(void);

Game1Player * game_1_player_new(GtkWidget * window,GdkCanvas * canvas,int level,gint score);

gint game_1_player_get_score(Game1Player * g);
gboolean game_1_player_is_lost(Game1Player * g);

G_END_DECLS





#endif
