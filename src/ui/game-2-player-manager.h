/* game_2_player_manager.h
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

#ifndef GAME_2_PLAYER_MANAGER_H
#define GAME_2_PLAYER_MANAGER_H

#include <gtk/gtk.h>
#include "game-manager.h"
#include "monkey-canvas.h"
#include "monkey.h"
G_BEGIN_DECLS

#define TYPE_GAME_2_PLAYER_MANAGER            (game_2_player_manager_get_type())

#define GAME_2_PLAYER_MANAGER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_GAME_2_PLAYER_MANAGER,Game2PlayerManager))
#define GAME_2_PLAYER_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GAME_2_PLAYER_MANAGER,Game2PlayerManagerClass))
#define IS_GAME_2_PLAYER_MANAGER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_GAME_2_PLAYER_MANAGER))
#define IS_GAME_2_PLAYER_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GAME_2_PLAYER_MANAGER))
#define GAME_2_PLAYER_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_GAME_2_PLAYER_MANAGER, Game2PlayerManagerClass))

typedef struct Game2PlayerManagerPrivate Game2PlayerManagerPrivate;



typedef struct {
  GObject parent_instance;
  Game2PlayerManagerPrivate * private;
} Game2PlayerManager;

typedef struct {
  GObjectClass parent_class;
} Game2PlayerManagerClass;


GType game_2_player_manager_get_type(void);

Game2PlayerManager * game_2_player_manager_new(GtkWidget * window,MonkeyCanvas * canvas);
void game_2_player_manager_start(GameManager * g);
void game_2_player_manager_stop(GameManager * g);





G_END_DECLS





#endif
