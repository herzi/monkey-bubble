/* game.c
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
#include <stdlib.h>
#include "game-1-player.h"
#include "game.h"
#include "gdk-view.h"
#include <gconf/gconf-client.h>
#include "clock.h"
#define FRAME_DELAY 10

#define PRIVATE(game_1_player) (game_1_player->private)


static GObjectClass* parent_class = NULL;

struct Game1PlayerPrivate {
    GdkCanvas * canvas;
    GtkWidget * window;
    GdkView * display;
    Monkey * monkey;
    guint timeout_id;
    GameState state;
  Clock * clock;
    GList * observers;

	 gboolean lost;


  Block * paused_block;
  Layer * paused_layer;

    GConfClient * gconf_client;

    guint left_key;
    GdkModifierType left_key_modifier;


    guint right_key;
    GdkModifierType right_key_modifier;


    guint shoot_key;
    GdkModifierType shoot_key_modifier;

    gint notify_id;
};

static void game_1_player_bubbles_waiting_changed(IMonkeyObserver * bo,
																  Monkey * monkey,
																  int bubbles_count);
static void game_1_player_bubble_sticked(IMonkeyObserver * i,Monkey * monkey,Bubble * b);

static void game_1_player_game_lost(IMonkeyObserver *bo,Monkey * monkey);

static void game_1_player_bubbles_exploded(IMonkeyObserver * bo,
					  Monkey * monkey,
					  GList * exploded,
					  GList * fallen);

static void game_1_player_bubble_shot(IMonkeyObserver * bo,
					 Monkey * monkey,
					 Bubble * bubble);

static void game_1_player_add_bubble(Game1Player * game);

static void game_1_player_start(Game * game);
static void game_1_player_stop(Game * game);
static void game_1_player_pause(Game * game,gboolean pause);

static GameState game_1_player_get_state(Game * game);

void game_1_player_fire_changed(Game1Player * game);

void game_1_player_attach_observer(Game * game,IGameObserver *observer);
void game_1_player_detach_observer(Game * game,IGameObserver *observer);


static void game_1_player_game_iface_init(GameClass * i);
static void game_1_player_imonkey_observer_iface_init(IMonkeyObserverClass * i);

static gint game_1_player_timeout (gpointer data);

static gboolean game_1_player_key_released(GtkWidget *widget,
					   GdkEventKey *event,
					   gpointer user_data);

static gboolean game_1_player_key_pressed(GtkWidget *widget,
					  GdkEventKey *event,
					  gpointer user_data);


static gint get_time(Game1Player * game);
static void time_paused(Game1Player * game);
static void time_init(Game1Player * game);

static void game_1_player_conf_keyboard(Game1Player * game);

static void game_1_player_instance_init(Game1Player * game) {
  game->private =g_new0 (Game1PlayerPrivate, 1);		
}

static void game_1_player_finalize(GObject* object) {

  Game1Player * game = GAME_1_PLAYER(object);

  gtk_timeout_remove( PRIVATE(game)->timeout_id);

  gconf_client_notify_remove(PRIVATE(game)->gconf_client, PRIVATE(game)->notify_id);
  g_signal_handlers_disconnect_by_func( G_OBJECT(PRIVATE(game)->window) ,
		    GTK_SIGNAL_FUNC (game_1_player_key_pressed),game);


  g_signal_handlers_disconnect_by_func( G_OBJECT(PRIVATE(game)->window) ,
		    GTK_SIGNAL_FUNC (game_1_player_key_released),game);


  g_object_unref( PRIVATE(game)->clock);
  g_object_unref( PRIVATE(game)->display );

  monkey_detach_observer( PRIVATE(game)->monkey,
			   IMONKEY_OBSERVER(game));

  g_object_unref( PRIVATE(game)->monkey);
  g_free(game->private);
  
  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }

}


static void game_1_player_class_init (Game1PlayerClass *klass) {

    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = game_1_player_finalize;
}


GType game_1_player_get_type(void) {
    static GType game_1_player_type = 0;
    
    if (!game_1_player_type) {
      static const GTypeInfo game_1_player_info = {
	sizeof(Game1PlayerClass),
	NULL,           /* base_init */
	NULL,           /* base_finalize */
	(GClassInitFunc) game_1_player_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data */
	sizeof(Game1Player),
	1,              /* n_preallocs */
	(GInstanceInitFunc) game_1_player_instance_init,
      };

      static const GInterfaceInfo iface_game = {
	(GInterfaceInitFunc) game_1_player_game_iface_init,
	NULL,
	NULL
      };



      static const GInterfaceInfo iface_imonkey_observer = {
	(GInterfaceInitFunc) game_1_player_imonkey_observer_iface_init,
	NULL,
	NULL
      };
      
      
      game_1_player_type = g_type_register_static(G_TYPE_OBJECT,
						  "Game1Player",
						  &game_1_player_info, 0);

      g_type_add_interface_static(game_1_player_type,
				  TYPE_GAME,
				  &iface_game);

      g_type_add_interface_static(game_1_player_type,
				  TYPE_IMONKEY_OBSERVER,
				  &iface_imonkey_observer);


    }
    
    return game_1_player_type;
}

static void game_1_player_game_iface_init(GameClass * i) {
  i->start = game_1_player_start;
  i->stop = game_1_player_stop;
  i->pause = game_1_player_pause;
  i->get_state = game_1_player_get_state;
  i->attach_observer = game_1_player_attach_observer;
  i->detach_observer = game_1_player_detach_observer;
}

static void game_1_player_imonkey_observer_iface_init(IMonkeyObserverClass * i) {
  imonkey_observer_class_virtual_init(i,
				      game_1_player_game_lost,
				      game_1_player_bubbles_exploded,
				      game_1_player_bubble_shot,
				      game_1_player_bubble_sticked,
												  game_1_player_bubbles_waiting_changed);
}


static void game_1_player_bubbles_waiting_changed(IMonkeyObserver * bo,
																  Monkey * monkey,
																  int bubbles_count) {
}
static void game_1_player_bubble_sticked(IMonkeyObserver * i,Monkey * monkey,Bubble * b) {
  Game1Player * game;

  g_assert( IS_GAME_1_PLAYER(i));

  game = GAME_1_PLAYER( i );

  if( ( monkey_get_shot_count( monkey) % 5 ) == 0 ) {

    monkey_set_board_down( monkey);
  }


}



static void
game_1_player_config_notify (GConfClient *client,
                                   guint        cnxn_id,
                                   GConfEntry  *entry,
                                   gpointer     user_data) {

  Game1Player * game;


  game = GAME_1_PLAYER (user_data);
    
  game_1_player_conf_keyboard(game);
  
}


Game1Player * game_1_player_new(GtkWidget * window,GdkCanvas * canvas, int level) {
  Game1Player * game;


  game = GAME_1_PLAYER (g_object_new (TYPE_GAME_1_PLAYER, NULL));


  PRIVATE(game)->gconf_client = gconf_client_get_default ();

  gconf_client_add_dir (PRIVATE(game)->gconf_client, "/apps/monkey-bubble-game",
                        GCONF_CLIENT_PRELOAD_NONE, NULL);

  gdk_canvas_clear(canvas);
  PRIVATE(game)->monkey = 
    monkey_new_level_from_file(DATADIR"/monkey-bubble/levels",
			       level);
  PRIVATE(game)->display = 
    gdk_view_new(canvas, 
		 PRIVATE(game)->monkey,0,0,TRUE);
  
  PRIVATE(game)->canvas = canvas;
  PRIVATE(game)->window = window;
  
  gdk_view_set_score(PRIVATE(game)->display,level+1);
  g_signal_connect( G_OBJECT( window) ,"key-press-event",
		    GTK_SIGNAL_FUNC (game_1_player_key_pressed),game);
  
  g_signal_connect( G_OBJECT( window) ,"key-release-event",
		    GTK_SIGNAL_FUNC (game_1_player_key_released),game);
  
  

  PRIVATE(game)->paused_block = 
    gdk_canvas_create_block_from_image(canvas,
				       DATADIR"/monkey-bubble/gfx/pause.svg",
				       460,240,
				       0,0);

  PRIVATE(game)->paused_layer =
    gdk_canvas_append_layer(canvas,120,140);


  PRIVATE(game)->clock = clock_new();
  PRIVATE(game)->timeout_id = 
    gtk_timeout_add (FRAME_DELAY, game_1_player_timeout, game);
  
  PRIVATE(game)->observers = NULL;
  
  PRIVATE(game)->state = GAME_STOPPED;

  PRIVATE(game)->lost = FALSE;
  
  monkey_attach_observer(PRIVATE(game)->monkey,
			 IMONKEY_OBSERVER(game));


  game_1_player_add_bubble(game);

  game_1_player_add_bubble(game);


  game_1_player_conf_keyboard(game);

  PRIVATE(game)->notify_id =
      gconf_client_notify_add( PRIVATE(game)->gconf_client,
			       "/apps/monkey-bubble-game",
			       game_1_player_config_notify,
			       game,NULL,NULL);
  return game;
}

static void game_1_player_conf_keyboard(Game1Player * game) {
    gchar * str;
    guint accel;
    GdkModifierType modifier;
    str = gconf_client_get_string(PRIVATE(game)->gconf_client,"/apps/monkey-bubble-game/player_1_left",NULL);

    if( str != NULL) {
	
	gtk_accelerator_parse (str,
			       &accel,
			       &modifier);

	PRIVATE(game)->left_key = accel;
	PRIVATE(game)->left_key_modifier =modifier;
	g_free(str);
    } else {
	PRIVATE(game)->left_key = 115;
	PRIVATE(game)->left_key_modifier = 0;
	
    }

    str = gconf_client_get_string(PRIVATE(game)->gconf_client,"/apps/monkey-bubble-game/player_1_right",NULL);

    if( str != NULL) {
	
	gtk_accelerator_parse (str,
			       &accel,
			       &modifier);

	PRIVATE(game)->right_key = accel;
	PRIVATE(game)->right_key_modifier =modifier;
	g_free(str);
    } else {
	PRIVATE(game)->right_key = 102;
	PRIVATE(game)->right_key_modifier = 0;
	
    }

 str = gconf_client_get_string(PRIVATE(game)->gconf_client,"/apps/monkey-bubble-game/player_1_shoot",NULL);

    if( str != NULL) {
	
	gtk_accelerator_parse (str,
			       &accel,
			       &modifier);

	PRIVATE(game)->shoot_key = accel;
	PRIVATE(game)->shoot_key_modifier =modifier;
	g_free(str);
    } else {
	PRIVATE(game)->shoot_key = 100;
	PRIVATE(game)->shoot_key_modifier = 0;
	
    }
}



static gboolean game_1_player_key_pressed(GtkWidget *widget,
					  GdkEventKey *event,
					  gpointer user_data) {
  
  
  Game1Player * game;
  Monkey * monkey;
  
  game = GAME_1_PLAYER(user_data);

  if( PRIVATE(game)->state == GAME_PLAYING) {
    monkey = PRIVATE(game)->monkey;
    
    if( event->keyval ==  PRIVATE(game)->left_key ) {
      monkey_left_changed( monkey,TRUE,get_time(game));
    }
    
    if( event->keyval ==  PRIVATE(game)->right_key ) {
      monkey_right_changed( monkey,TRUE,get_time(game));
    }
    
    
    if( event->keyval ==  PRIVATE(game)->shoot_key) {
      monkey_shoot( monkey,get_time(game));
    }
  }

  return FALSE;
}

static gboolean game_1_player_key_released(GtkWidget *widget,
					   GdkEventKey *event,
					   gpointer user_data) {

  Game1Player * game;
  Monkey * monkey;

  game = GAME_1_PLAYER(user_data);
  monkey = PRIVATE(game)->monkey;

  if( PRIVATE(game)->state == GAME_PLAYING ) {
      if( event->keyval == PRIVATE(game)->left_key) {
      monkey_left_changed( monkey,FALSE,get_time(game));
    }
    
    if( event->keyval ==   PRIVATE(game)->right_key) {
      monkey_right_changed( monkey,FALSE,get_time(game));
    }

  }
  return FALSE;

}

static gint game_1_player_timeout (gpointer data)
{


  Game1Player * game;
  Monkey * monkey;
  gint time;

  game = GAME_1_PLAYER(data);
  monkey = PRIVATE(game)->monkey;

  time = get_time(game);

  if( PRIVATE(game)->state == GAME_PLAYING ) {
  
  
    
    monkey_update( monkey,
		   time );
    
    
    
  } 
    gdk_view_update( PRIVATE(game)->display,
		    time);

    gdk_canvas_paint(PRIVATE(game)->canvas);
 
  return TRUE;
}

static gint get_time(Game1Player * game) {
  return clock_get_time(PRIVATE(game)->clock);
}


static void time_paused(Game1Player * game) {
  clock_pause( PRIVATE(game)->clock, TRUE);
}

static void time_unpaused(Game1Player * game) {
  clock_pause( PRIVATE(game)->clock, FALSE);
}

static void time_init(Game1Player * game) {
  clock_start( PRIVATE(game)->clock);
}

static void game_1_player_start(Game * game) {
  Game1Player * g;
 
  g_assert( IS_GAME_1_PLAYER(game));
  
  g = GAME_1_PLAYER(game);

  time_init( g );

  PRIVATE(g)->state = GAME_PLAYING;
}

static void game_1_player_stop(Game * game) {
  Game1Player * g;

  g_assert( IS_GAME_1_PLAYER(game));
  
  g = GAME_1_PLAYER(game);

  PRIVATE(g)->state = GAME_STOPPED;

  time_paused(g);
  game_1_player_fire_changed(g);

}

gboolean game_1_player_is_lost(Game1Player * g) {

	 g_assert( GAME_1_PLAYER(g));

	 return PRIVATE(g)->lost;
}

static void game_1_player_pause(Game * game,gboolean pause) {
  Game1Player * g;
  g_assert( IS_GAME_1_PLAYER(game));
  
  g = GAME_1_PLAYER(game);

  if( pause ) {
    PRIVATE(g)->state = GAME_PAUSED;
    time_paused( g);

    gdk_canvas_add_block(PRIVATE(g)->canvas,
			 PRIVATE(g)->paused_layer,
			 PRIVATE(g)->paused_block,
			 0,0);

	 game_1_player_fire_changed(g);
  } else {
    PRIVATE(g)->state = GAME_PLAYING;
    time_unpaused( g);
    gdk_canvas_remove_block(PRIVATE(g)->canvas,
			    PRIVATE(g)->paused_block);    

	 game_1_player_fire_changed(g);

  }
}

static GameState game_1_player_get_state(Game * game) {
  Game1Player * g;
  g_assert( IS_GAME_1_PLAYER(game));
  
  g = GAME_1_PLAYER(game);

  return PRIVATE(g)->state;
}

static void game_1_player_game_lost(IMonkeyObserver *bo,Monkey * monkey) {
  Game1Player * g;
  g_assert( IS_GAME_1_PLAYER(bo));

  g = GAME_1_PLAYER(bo);

  PRIVATE(g)->lost = TRUE;

  gdk_view_draw_lost( PRIVATE(g)->display );

  game_1_player_stop( GAME(bo));


  PRIVATE(g)->state = GAME_FINISHED;
  
  game_1_player_fire_changed(g);
  gdk_canvas_paint(PRIVATE(g)->canvas);
    
}

static void game_1_player_bubbles_exploded(IMonkeyObserver * bo,
					  Monkey * monkey,
					  GList * exploded,
					  GList * fallen) {

	 Game1Player * g;
	 g_assert( IS_GAME_1_PLAYER(bo));	 
	 
	 g = GAME_1_PLAYER(bo);
	 if( monkey_is_empty( monkey) ) {
		  
		  PRIVATE(g)->state = GAME_FINISHED;

		  gdk_view_draw_win( PRIVATE(g)->display );

		  game_1_player_fire_changed(g);	
		  game_1_player_stop(GAME(g));
	 }

}

static void game_1_player_add_bubble(Game1Player * game) {
  gint * colors_count;
  gint rnd,count;
  Monkey * monkey;

  g_assert( IS_GAME_1_PLAYER(game));

  monkey = PRIVATE(game)->monkey;
  colors_count = board_get_colors_count(playground_get_board(monkey_get_playground(monkey)));

  rnd = rand()%COLORS_COUNT;
  count = 0;
  while( rnd >= 0 ) {
    count++;
    count %= COLORS_COUNT;

    while(colors_count[count] == 0) {
      count++;
      count %= COLORS_COUNT;
    }
    rnd--;
  }
  
  shooter_add_bubble(monkey_get_shooter(monkey),bubble_new(count,0,0));

}
static void game_1_player_bubble_shot(IMonkeyObserver * bo,
				      Monkey * monkey,
				      Bubble * bubble) {


  g_assert( IS_GAME_1_PLAYER(bo));

  game_1_player_add_bubble(GAME_1_PLAYER(bo));
}

void game_1_player_fire_changed(Game1Player * game) {	
	 GList * next;

	 next = PRIVATE(game)->observers;

	 while( next != NULL ) {
		  
		  IGameObserver * o = IGAME_OBSERVER(next->data);
		  igame_observer_changed(o,GAME(game));
		  next = g_list_next(next);
	 }
}

void game_1_player_attach_observer(Game * game,IGameObserver *observer) {
  Game1Player * g;
  g_assert( IS_GAME_1_PLAYER(game));
  
  g = GAME_1_PLAYER(game);

  PRIVATE(g)->observers = g_list_append( PRIVATE(g)->observers, observer);
}

void game_1_player_detach_observer(Game * game,IGameObserver *observer) {
  Game1Player * g;
  g_assert( IS_GAME_1_PLAYER(game));
  
  g = GAME_1_PLAYER(game);

  PRIVATE(g)->observers = g_list_remove( PRIVATE(g)->observers, observer);
}
