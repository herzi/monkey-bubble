/* game_1_player_manager.c
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
#include "game-1-player-manager.h"
#include "game-1-player.h"
#include "game.h"
#include "game-manager.h"
#include "ui-main.h"

#define PRIVATE(game_1_player_manager) (game_1_player_manager->private )

static GObjectClass* parent_class = NULL;

struct Game1PlayerManagerPrivate {
  GdkCanvas * canvas;
  GtkWidget * window;
  Game1Player * current_game;
  int current_level;
};

static void game_1_player_manager_game_changed(IGameObserver * bo,
					       Game * game);

static void game_1_player_manager_start_level(Game1PlayerManager * g);

static void game_1_player_manager_igame_observer_iface_init(IGameObserverClass * i);
static void game_1_player_manager_game_manager_iface_init(GameManagerClass * i);

static void game_1_player_manager_finalize(GObject* object);
static void game_1_player_manager_instance_init(Game1PlayerManager * game_1_player_manager) {
  game_1_player_manager->private =g_new0 (Game1PlayerManagerPrivate, 1);			
}


static void game_1_player_manager_class_init (Game1PlayerManagerClass *klass) {
  GObjectClass* object_class;

  parent_class = g_type_class_peek_parent(klass);
  object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = game_1_player_manager_finalize;
}


GType game_1_player_manager_get_type(void) {
  static GType game_1_player_manager_type = 0;
    
  if (!game_1_player_manager_type) {
    static const GTypeInfo game_1_player_manager_info = {
      sizeof(Game1PlayerManagerClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) game_1_player_manager_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(Game1PlayerManager),
      1,              /* n_preallocs */
      (GInstanceInitFunc) game_1_player_manager_instance_init,
    };


    static const GInterfaceInfo iface_igame_observer = {
      (GInterfaceInitFunc) game_1_player_manager_igame_observer_iface_init,
      NULL,
      NULL
    };
      

      
    static const GInterfaceInfo iface_game_manager = {
      (GInterfaceInitFunc) game_1_player_manager_game_manager_iface_init,
      NULL,
      NULL
    };
      
    game_1_player_manager_type = g_type_register_static(G_TYPE_OBJECT,
							"Game1PlayerManager",
							&game_1_player_manager_info,
							0);


    g_type_add_interface_static(game_1_player_manager_type,
				TYPE_IGAME_OBSERVER,
				&iface_igame_observer);
	 
	 
    g_type_add_interface_static(game_1_player_manager_type,
				TYPE_GAME_MANAGER,
				&iface_game_manager);
      
      
  }
    
  return game_1_player_manager_type;
}


Game1PlayerManager * game_1_player_manager_new(GtkWidget * window,GdkCanvas * canvas) {
  Game1PlayerManager * game_1_player_manager;
  game_1_player_manager = GAME_1_PLAYER_MANAGER (g_object_new (TYPE_GAME_1_PLAYER_MANAGER, NULL));

  PRIVATE(game_1_player_manager)->canvas = canvas;
  PRIVATE(game_1_player_manager)->window = window;
  PRIVATE(game_1_player_manager)->current_game = NULL;
  
  return game_1_player_manager;
}


static gboolean startnew_function(gpointer data) {
  Game1PlayerManager * manager;

  manager = GAME_1_PLAYER_MANAGER(data);


  game_stop( GAME(PRIVATE(manager)->current_game));

  game_detach_observer( GAME(PRIVATE(manager)->current_game),IGAME_OBSERVER(manager));
  g_object_unref( PRIVATE(manager)->current_game);


  PRIVATE(manager)->current_level++;
  game_1_player_manager_start_level(manager);
  return FALSE;
}


static gboolean restart_function(gpointer data) {
  Game1PlayerManager * manager;

  manager = GAME_1_PLAYER_MANAGER(data);


  game_stop( GAME(PRIVATE(manager)->current_game));

  game_detach_observer( GAME(PRIVATE(manager)->current_game),IGAME_OBSERVER(manager));
  g_object_unref( PRIVATE(manager)->current_game);


  game_1_player_manager_start_level(manager);
  return FALSE;
}

static void game_1_player_manager_finalize(GObject* object) {

  Game1PlayerManager * game_1_player_manager = GAME_1_PLAYER_MANAGER(object);

  g_print("game_1_player_manager finalize\n");
  g_free(game_1_player_manager->private);

  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


static void game_1_player_manager_game_manager_iface_init(GameManagerClass * i) {
  game_manager_class_virtual_init(i,
				  game_1_player_manager_start,
				  game_1_player_manager_stop);
}

static void game_1_player_manager_igame_observer_iface_init(IGameObserverClass * i) {

  igame_observer_class_virtual_init(i,game_1_player_manager_game_changed);
}

static void game_1_player_manager_game_changed(IGameObserver * bo,
					       Game * game) {

  UiMain * ui_main =  ui_main_get_instance();

  Game1PlayerManager * manager;

  manager = GAME_1_PLAYER_MANAGER(bo);
  g_print("maanger state change\n");
  if( game_get_state( game ) == GAME_FINISHED ) {
		  
    g_print("game finished\n");
    if( game_1_player_is_lost( GAME_1_PLAYER(game) )) {
      ui_main_set_game(ui_main,NULL);	 
      g_timeout_add(2000,
		    restart_function,
		    manager);
    } else {
      ui_main_set_game(ui_main,NULL);	 
      g_timeout_add(2000,
		    startnew_function,
		    manager);
				
    }
  }
				
  g_print("state changed\n");
}

static void game_1_player_manager_start_level(Game1PlayerManager * g) {
  Game1Player * game;
  Game1PlayerManager * manager;
  UiMain * ui_main =  ui_main_get_instance();

  manager = GAME_1_PLAYER_MANAGER(g);

  ui_main_set_game(ui_main,
		   GAME( game = game_1_player_new( PRIVATE(manager)->window,
						   PRIVATE(manager)->canvas,
						   PRIVATE(manager)->current_level)									 
			 ));


  game_start( GAME(game) );

  game_attach_observer(GAME(game),IGAME_OBSERVER(manager));
  PRIVATE(manager)->current_game = game;
  gdk_canvas_paint( PRIVATE(manager)->canvas);

}

void game_1_player_manager_start(GameManager * g) {
	 
  Game1PlayerManager * manager;
	 
  manager = GAME_1_PLAYER_MANAGER(g);

  PRIVATE(manager)->current_level = 0;
  game_1_player_manager_start_level(manager);

}

void game_1_player_manager_stop(GameManager * g) {
  Game1PlayerManager * manager;
  UiMain * ui_main =  ui_main_get_instance();


  manager = GAME_1_PLAYER_MANAGER(g);
  game_stop( GAME(PRIVATE(manager)->current_game));


  game_attach_observer(GAME(PRIVATE(manager)->current_game),IGAME_OBSERVER(manager));
  g_object_unref( PRIVATE(manager)->current_game);

  ui_main_set_game(ui_main,NULL);
  gdk_canvas_paint( PRIVATE(manager)->canvas);
  g_print("game stopped\n");
}
