/* game_2_player_manager.c
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
#include <gtk/gtk.h>
#include "game-2-player-manager.h"
#include "game-2-player.h"
#include "game.h"
#include "game-manager.h"
#include "ui-main.h"

#define PRIVATE(game_2_player_manager) (game_2_player_manager->private )

static GObjectClass* parent_class = NULL;

struct Game2PlayerManagerPrivate {
	 GdkCanvas * canvas;
	 GtkWidget * window;
	 Game2Player * current_game;
	 int score_player_1;
	 int score_player_2;
};

static void game_2_player_manager_restart(Game2PlayerManager * manager);
static void game_2_player_manager_state_changed(Game * game,
						Game2PlayerManager * manager);

static void game_2_player_manager_game_manager_iface_init(GameManagerClass * i);

static void game_2_player_manager_finalize(GObject* object);
static void game_2_player_manager_instance_init(Game2PlayerManager * game_2_player_manager) {
  game_2_player_manager->private =g_new0 (Game2PlayerManagerPrivate, 1);			
}


static void game_2_player_manager_class_init (Game2PlayerManagerClass *klass) {
  GObjectClass* object_class;

  parent_class = g_type_class_peek_parent(klass);
  object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = game_2_player_manager_finalize;
}


GType game_2_player_manager_get_type(void) {
  static GType game_2_player_manager_type = 0;
    
  if (!game_2_player_manager_type) {
    static const GTypeInfo game_2_player_manager_info = {
      sizeof(Game2PlayerManagerClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) game_2_player_manager_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(Game2PlayerManager),
      1,              /* n_preallocs */
      (GInstanceInitFunc) game_2_player_manager_instance_init,
    };



      
    static const GInterfaceInfo iface_game_manager = {
      (GInterfaceInitFunc) game_2_player_manager_game_manager_iface_init,
      NULL,
      NULL
    };
      
    game_2_player_manager_type = g_type_register_static(G_TYPE_OBJECT,
					 "Game2PlayerManager",
					 &game_2_player_manager_info,
					 0);

	 
	 
    g_type_add_interface_static(game_2_player_manager_type,
				TYPE_GAME_MANAGER,
				&iface_game_manager);
      
      
  }
    
  return game_2_player_manager_type;
}


Game2PlayerManager * game_2_player_manager_new(GtkWidget * window,GdkCanvas * canvas) {
  Game2PlayerManager * game_2_player_manager;
  game_2_player_manager = GAME_2_PLAYER_MANAGER (g_object_new (TYPE_GAME_2_PLAYER_MANAGER, NULL));

  PRIVATE(game_2_player_manager)->canvas = canvas;
  PRIVATE(game_2_player_manager)->window = window;
  PRIVATE(game_2_player_manager)->current_game = NULL;
  PRIVATE(game_2_player_manager)->score_player_1 = 0;
  PRIVATE(game_2_player_manager)->score_player_2 = 0;
  
  return game_2_player_manager;
}


static void game_2_player_manager_finalize(GObject* object) {

  Game2PlayerManager * game_2_player_manager = GAME_2_PLAYER_MANAGER(object);

  g_free(game_2_player_manager->private);

  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


static void game_2_player_manager_game_manager_iface_init(GameManagerClass * i) {
  game_manager_class_virtual_init(i,
				  game_2_player_manager_start,
				  game_2_player_manager_stop);
}

static gboolean restart_function(gpointer data) {
	 Game2PlayerManager * manager;

	 manager = GAME_2_PLAYER_MANAGER(data);

	 /* TODO : CLEAN */
	 game_stop( GAME(PRIVATE(manager)->current_game));
	 
	 g_signal_handlers_disconnect_matched(  G_OBJECT( PRIVATE(manager)->current_game ),
						G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,manager);
	 
	 g_object_unref( PRIVATE(manager)->current_game);
	 
	 ui_main_set_game(ui_main_get_instance(),NULL);

	 game_2_player_manager_restart(manager);
	 return FALSE;
}

static void game_2_player_manager_state_changed(Game * game,
					       Game2PlayerManager * manager) {
	 Game2Player * g;

	 UiMain * ui_main;

	
	 ui_main =  ui_main_get_instance();

	 g = GAME_2_PLAYER(game);

	 if( game_get_state( game ) == GAME_FINISHED ) {
	   
	   if( game_2_player_get_winner( g) == PLAYER_1) {
	     PRIVATE(manager)->score_player_1++;
	   } else {
	     PRIVATE(manager)->score_player_2++;
	   }

	   ui_main_set_game(ui_main,NULL);	 
	   g_timeout_add(2000,
			 restart_function,
			 manager);
	   
	 }
	 
}

void game_2_player_manager_start(GameManager * g) {
	 
	 Game2PlayerManager * manager;

	 manager = GAME_2_PLAYER_MANAGER(g);

	 game_2_player_manager_restart(manager);
}

static void game_2_player_manager_restart(Game2PlayerManager * manager) {
	 Game2Player * game;

	 UiMain * ui_main =  ui_main_get_instance();

	 ui_main_set_game(ui_main,
			  GAME( game = game_2_player_new( PRIVATE(manager)->window,
							  PRIVATE(manager)->canvas,PRIVATE(manager)->score_player_1,
							  PRIVATE(manager)->score_player_2)));


	 game_start( GAME(game) );
	 
	 PRIVATE(manager)->current_game = game;

	 g_signal_connect( G_OBJECT(game), "state-changed",
			   G_CALLBACK(game_2_player_manager_state_changed),manager);
	 gdk_canvas_paint( PRIVATE(manager)->canvas);
	 
}

void game_2_player_manager_stop(GameManager * g) {
  Game2PlayerManager * manager;
  
  UiMain * ui_main;

  ui_main =  ui_main_get_instance();
  
  manager = GAME_2_PLAYER_MANAGER(g);
  
  game_stop( GAME(PRIVATE(manager)->current_game));
  
  g_signal_handlers_disconnect_matched(  G_OBJECT( PRIVATE(manager)->current_game ),
                                         G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,manager);
  
  g_object_unref( PRIVATE(manager)->current_game);
  
  ui_main_set_game(ui_main,NULL);
  gdk_canvas_paint( PRIVATE(manager)->canvas);
}
