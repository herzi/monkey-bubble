/* game-2-player.c
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
#include "game-2-player.h"
#include "game.h"
#include "gdk-canvas.h"
#include "gdk-view.h"
#include <stdlib.h>
#include "clock.h"
#include <gconf/gconf-client.h>

#define FRAME_DELAY 10
#define INIT_BUBBLES_COUNT 8+7+8+7
#define PRIVATE(game_2_player) (game_2_player->private)


static GObjectClass* parent_class = NULL;

struct Game2PlayerPrivate {
  GdkCanvas * canvas;
  GtkWidget * window;
  GdkView * display_left;
  GdkView * display_right;
  Block * background;
  Block * paused_block;
  Layer * paused_layer;
  Monkey * monkey_left;
  Monkey * monkey_right;
  guint timeout_id;
  GameState state;
  Clock * clock;
  GList * observers;
	 int winner;


    GConfClient * gconf_client;

    guint p1_left_key;
    GdkModifierType p1_left_key_modifier;


    guint p1_right_key;
    GdkModifierType p1_right_key_modifier;


    guint p1_shoot_key;
    GdkModifierType p1_shoot_key_modifier;


    guint p2_left_key;
    GdkModifierType p2_left_key_modifier;


    guint p2_right_key;
    GdkModifierType p2_right_key_modifier;


    guint p2_shoot_key;
    GdkModifierType p2_shoot_key_modifier;

    gint notify_id;

};

static void game_2_player_conf_keyboard(Game2Player * game);
static void game_2_player_fire_changed(Game2Player * game);
static void game_2_player_disconnect_input(Game2Player * game);
static void game_2_player_connect_input(Game2Player * game);

static void game_2_player_game_lost(IMonkeyObserver *bo,Monkey * monkey);

static void game_2_player_bubbles_exploded(IMonkeyObserver * bo,
					   Monkey * monkey,
					   GList * exploded,
					   GList * fallen);

static void game_2_player_bubbles_waiting_changed(IMonkeyObserver * bo,
																  Monkey * monkey,
																  int bubbles_count);

static void game_2_player_bubble_shot(IMonkeyObserver * bo,
				      Monkey * monkey,
				      Bubble * bubble);

static gint game_2_player_add_bubble(Game2Player * game,Monkey * m);

static void game_2_player_bubble_sticked(IMonkeyObserver * bo,
					 Monkey * monkey,
					 Bubble * bubble);

static void game_2_player_start(Game * game);
static void game_2_player_stop(Game * game);
static void game_2_player_pause(Game * game,gboolean pause);

static GameState game_2_player_get_state(Game * game);

void game_2_player_attach_observer(Game * game,IGameObserver *observer);
void game_2_player_detach_observer(Game * game,IGameObserver *observer);

static void game_2_player_imonkey_observer_iface_init(IMonkeyObserverClass * i);
static void game_2_player_game_iface_init(GameClass * i);

static gint game_2_player_timeout (gpointer data);

static gboolean game_2_player_key_released(GtkWidget *widget,
					   GdkEventKey *event,
					   gpointer user_data);

static gboolean game_2_player_key_pressed(GtkWidget *widget,
					  GdkEventKey *event,
					  gpointer user_data);


static gint get_time(Game2Player * game);
static void time_paused(Game2Player * game);
static void time_init(Game2Player * game);

static void game_2_player_instance_init(Game2Player * game) {
  game->private =g_new0 (Game2PlayerPrivate, 2);		
}

static void game_2_player_finalize(GObject* object) {

  Game2Player * game = GAME_2_PLAYER(object);


  gconf_client_notify_remove(PRIVATE(game)->gconf_client, PRIVATE(game)->notify_id);


  monkey_detach_observer( PRIVATE(game)->monkey_left,
			  IMONKEY_OBSERVER(game));
  monkey_detach_observer( PRIVATE(game)->monkey_right,
			  IMONKEY_OBSERVER(game));

  g_object_unref( PRIVATE(game)->clock);
  gtk_timeout_remove( PRIVATE(game)->timeout_id);

  g_object_unref( PRIVATE(game)->display_left );


  g_object_unref( PRIVATE(game)->display_right );

  
  g_object_unref( PRIVATE(game)->monkey_left);


  g_object_unref( PRIVATE(game)->monkey_right);


  g_free(game->private);

  
  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


static void game_2_player_class_init (Game2PlayerClass *klass) {

  GObjectClass* object_class;
    
  parent_class = g_type_class_peek_parent(klass);
  object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = game_2_player_finalize;
}


GType game_2_player_get_type(void) {
  static GType game_2_player_type = 0;
    
  if (!game_2_player_type) {
    static const GTypeInfo game_2_player_info = {
      sizeof(Game2PlayerClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) game_2_player_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(Game2Player),
      2,              /* n_preallocs */
      (GInstanceInitFunc) game_2_player_instance_init,
    };

    static const GInterfaceInfo iface_game = {
      (GInterfaceInitFunc) game_2_player_game_iface_init,
      NULL,
      NULL
    };


      
    static const GInterfaceInfo iface_imonkey_observer = {
      (GInterfaceInitFunc) game_2_player_imonkey_observer_iface_init,
      NULL,
      NULL
    };
      
    game_2_player_type = g_type_register_static(G_TYPE_OBJECT,
						"Game2Player",
						&game_2_player_info, 0);

    g_type_add_interface_static(game_2_player_type,
				TYPE_GAME,
				&iface_game);

    g_type_add_interface_static(game_2_player_type,
				TYPE_IMONKEY_OBSERVER,
				&iface_imonkey_observer);

  }
    
  return game_2_player_type;
}

static void game_2_player_game_iface_init(GameClass * i) {
  i->start = game_2_player_start;
  i->stop = game_2_player_stop;
  i->pause = game_2_player_pause;
  i->get_state = game_2_player_get_state;
  i->attach_observer = game_2_player_attach_observer;
  i->detach_observer = game_2_player_detach_observer;
}

static void game_2_player_imonkey_observer_iface_init(IMonkeyObserverClass * i) {
  imonkey_observer_class_virtual_init(i,
												  game_2_player_game_lost,
												  game_2_player_bubbles_exploded,
												  game_2_player_bubble_shot,
												  game_2_player_bubble_sticked,
												  game_2_player_bubbles_waiting_changed);
}


static void game_2_player_bubbles_waiting_changed(IMonkeyObserver * bo,
																  Monkey * monkey,
																  int bubbles_count) {
}



static void
game_2_player_config_notify (GConfClient *client,
                                   guint        cnxn_id,
                                   GConfEntry  *entry,
                                   gpointer     user_data) {

  Game2Player * game;


  game = GAME_2_PLAYER (user_data);
    
  game_2_player_conf_keyboard(game);
  
}

Game2Player * game_2_player_new(GtkWidget * window,GdkCanvas * canvas,int score_left,int score_right){
  Game2Player * game;
  Bubble * bubbles_left[INIT_BUBBLES_COUNT];
  Bubble * bubbles_right[INIT_BUBBLES_COUNT];
  Bubble * b_left;
  Bubble * b_right;
  gint i;
  game = GAME_2_PLAYER (g_object_new (TYPE_GAME_2_PLAYER, NULL));


  PRIVATE(game)->gconf_client = gconf_client_get_default ();

  gconf_client_add_dir (PRIVATE(game)->gconf_client, "/apps/monkey-bubble-game",
                        GCONF_CLIENT_PRELOAD_NONE, NULL);


    gdk_canvas_clear(canvas);
  PRIVATE(game)->clock = clock_new();

  PRIVATE(game)->monkey_left = monkey_new();  
  PRIVATE(game)->monkey_right = monkey_new();

  for(i = 0; i < INIT_BUBBLES_COUNT; i++ ) {
    b_left = bubble_new(rand()%COLORS_COUNT,0,0);
    b_right = bubble_new( bubble_get_color(b_left),0,0);
    bubbles_left[i]=b_left;
    bubbles_right[i]= b_right;
  }


  board_init( playground_get_board( monkey_get_playground( PRIVATE(game)->monkey_left )),bubbles_left,INIT_BUBBLES_COUNT);
  board_init( playground_get_board( monkey_get_playground( PRIVATE(game)->monkey_right )),bubbles_right,INIT_BUBBLES_COUNT);


  PRIVATE(game)->display_left = 
    gdk_view_new(canvas, 
		 PRIVATE(game)->monkey_left,
		 -165,0,FALSE);

  PRIVATE(game)->display_right = 
    gdk_view_new(canvas, 
		 PRIVATE(game)->monkey_right,
		 165,0,FALSE);

  PRIVATE(game)->canvas = canvas;

  PRIVATE(game)->window = window;
    

  PRIVATE(game)->paused_block = 
    gdk_canvas_create_block_from_image(canvas,
				       DATADIR"/monkey-bubble/gfx/pause.svg",
				       460,240,
				       0,0);

  PRIVATE(game)->paused_layer =
    gdk_canvas_append_layer(canvas,120,140);

  
  PRIVATE(game)->background = 
    gdk_canvas_create_block_from_image(
				       canvas,
				       DATADIR"/monkey-bubble/gfx/layout_2_players.svg",
				       640,480,
				       0,0);
  

  gdk_canvas_add_block(canvas,
		       gdk_canvas_get_root_layer( PRIVATE(game)->canvas),
		       PRIVATE(game)->background,
		       0,0);
    
  PRIVATE(game)->timeout_id = 
    gtk_timeout_add (FRAME_DELAY, game_2_player_timeout, game);
  
  PRIVATE(game)->observers = NULL;
  
  PRIVATE(game)->state = GAME_STOPPED;
  

  
  gdk_view_set_score( PRIVATE(game)->display_left,score_left);

  gdk_view_set_score( PRIVATE(game)->display_right,score_right);
  
  monkey_attach_observer(PRIVATE(game)->monkey_left,
			 IMONKEY_OBSERVER(game));


  monkey_attach_observer(PRIVATE(game)->monkey_right,
			 IMONKEY_OBSERVER(game));


  shooter_add_bubble(monkey_get_shooter(PRIVATE(game)->monkey_right),bubble_new( game_2_player_add_bubble(game,PRIVATE(game)->monkey_left),0,0));


  shooter_add_bubble(monkey_get_shooter(PRIVATE(game)->monkey_right),bubble_new( game_2_player_add_bubble(game,PRIVATE(game)->monkey_left),0,0));

  game_2_player_conf_keyboard(game);

  PRIVATE(game)->notify_id =
      gconf_client_notify_add( PRIVATE(game)->gconf_client,
			       "/apps/monkey-bubble-game",
			       game_2_player_config_notify,
			       game,NULL,NULL);

  return game;
}


static void game_2_player_conf_keyboard(Game2Player * game) {
    gchar * str;
    guint accel;
    GdkModifierType modifier;
    str = gconf_client_get_string(PRIVATE(game)->gconf_client,"/apps/monkey-bubble-game/player_1_left",NULL);

    if( str != NULL) {
	
	gtk_accelerator_parse (str,
			       &accel,
			       &modifier);

	PRIVATE(game)->p1_left_key = accel;
	PRIVATE(game)->p1_left_key_modifier =modifier;
	g_free(str);
    } else {
	PRIVATE(game)->p1_left_key = 115;
	PRIVATE(game)->p1_left_key_modifier = 0;
	
    }

    str = gconf_client_get_string(PRIVATE(game)->gconf_client,"/apps/monkey-bubble-game/player_1_right",NULL);

    if( str != NULL) {
	
	gtk_accelerator_parse (str,
			       &accel,
			       &modifier);

	PRIVATE(game)->p1_right_key = accel;
	PRIVATE(game)->p1_right_key_modifier =modifier;
	g_free(str);
    } else {
	PRIVATE(game)->p1_right_key = 102;
	PRIVATE(game)->p1_right_key_modifier = 0;
	
    }

    str = gconf_client_get_string(PRIVATE(game)->gconf_client,"/apps/monkey-bubble-game/player_1_shoot",NULL);

    if( str != NULL) {
	
	gtk_accelerator_parse (str,
			       &accel,
			       &modifier);

	PRIVATE(game)->p1_shoot_key = accel;
	PRIVATE(game)->p1_shoot_key_modifier =modifier;
	g_free(str);
    } else {
	PRIVATE(game)->p1_shoot_key = 100;
	PRIVATE(game)->p1_shoot_key_modifier = 0;
	
    }


    str = gconf_client_get_string(PRIVATE(game)->gconf_client,"/apps/monkey-bubble-game/player_2_left",NULL);

    if( str != NULL) {
	
	gtk_accelerator_parse (str,
			       &accel,
			       &modifier);

	PRIVATE(game)->p2_left_key = accel;
	PRIVATE(game)->p2_left_key_modifier =modifier;
	g_free(str);
    } else {
	PRIVATE(game)->p2_left_key = 106;
	PRIVATE(game)->p2_left_key_modifier = 0;
	
    }

    str = gconf_client_get_string(PRIVATE(game)->gconf_client,"/apps/monkey-bubble-game/player_2_right",NULL);

    if( str != NULL) {
	
	gtk_accelerator_parse (str,
			       &accel,
			       &modifier);

	PRIVATE(game)->p2_right_key = accel;
	PRIVATE(game)->p2_right_key_modifier =modifier;
	g_free(str);
    } else {
	PRIVATE(game)->p2_right_key = 108;
	PRIVATE(game)->p2_right_key_modifier = 0;
	
    }

    str = gconf_client_get_string(PRIVATE(game)->gconf_client,"/apps/monkey-bubble-game/player_2_shoot",NULL);

    if( str != NULL) {
	
	gtk_accelerator_parse (str,
			       &accel,
			       &modifier);

	PRIVATE(game)->p2_shoot_key = accel;
	PRIVATE(game)->p2_shoot_key_modifier =modifier;
	g_free(str);
    } else {
	PRIVATE(game)->p2_shoot_key = 108;
	PRIVATE(game)->p2_shoot_key_modifier = 0;
	
    }


}


static gboolean game_2_player_key_pressed(GtkWidget *widget,
					  GdkEventKey *event,
					  gpointer user_data) {
  
  
  Game2Player * game;
  Monkey * monkey;
  
  game = GAME_2_PLAYER(user_data);

  if( PRIVATE(game)->state == GAME_PLAYING) {

    monkey = PRIVATE(game)->monkey_left;
    if( event->keyval == PRIVATE(game)->p1_left_key ) {
      monkey_left_changed( monkey,TRUE,get_time(game));
    }
    
    if( event->keyval == PRIVATE(game)->p1_right_key ) {
      monkey_right_changed( monkey,TRUE,get_time(game));
    }
    
    
    if( event->keyval == PRIVATE(game)->p1_shoot_key ) {
      monkey_shoot( monkey,get_time(game));
    }

    monkey = PRIVATE(game)->monkey_right;

    if( event->keyval == PRIVATE(game)->p2_left_key ) {
      monkey_left_changed( monkey,TRUE,get_time(game));
    }
    
    if( event->keyval == PRIVATE(game)->p2_right_key ) {
      monkey_right_changed( monkey,TRUE,get_time(game));
    }
    
    
    if( event->keyval == PRIVATE(game)->p2_shoot_key) {
      monkey_shoot( monkey,get_time(game));
    }

  }

  return FALSE;
}

static gboolean game_2_player_key_released(GtkWidget *widget,
					   GdkEventKey *event,
					   gpointer user_data) {

  Game2Player * game;
  Monkey * monkey;

  game = GAME_2_PLAYER(user_data);


  if( PRIVATE(game)->state == GAME_PLAYING ) {

    monkey = PRIVATE(game)->monkey_left;

    if( event->keyval == PRIVATE(game)->p1_left_key ) {
      monkey_left_changed( monkey,FALSE,get_time(game));
    }
    
    if( event->keyval == PRIVATE(game)->p1_right_key) {
      monkey_right_changed( monkey,FALSE,get_time(game));
    }

    monkey = PRIVATE(game)->monkey_right;

    if( event->keyval == PRIVATE(game)->p2_left_key ) {
      monkey_left_changed( monkey,FALSE,get_time(game));
    }
  
    if( event->keyval == PRIVATE(game)->p2_right_key ) {
      monkey_right_changed( monkey,FALSE,get_time(game));
    }

  }
  return FALSE;

}

static gint game_2_player_timeout (gpointer data)
{


  Game2Player * game;
  Monkey * monkey;
  gint time;

  g_assert( IS_GAME_2_PLAYER(data) );

  game = GAME_2_PLAYER(data);

    time = get_time(game);
  
  if( PRIVATE(game)->state == GAME_PLAYING ) {
  
    monkey = PRIVATE(game)->monkey_left;
    
    monkey_update( monkey,
		   time );
    

    monkey = PRIVATE(game)->monkey_right;
    
    monkey_update( monkey,
		   time );
    
    
  }

    gdk_view_update( PRIVATE(game)->display_left,
		     time);
    


    gdk_view_update( PRIVATE(game)->display_right,
		     time);
    

  gdk_canvas_paint(PRIVATE(game)->canvas);

  return TRUE;
}

static gint get_time(Game2Player * game) {
  return clock_get_time(PRIVATE(game)->clock);
}


static void time_paused(Game2Player * game) {
  clock_pause( PRIVATE(game)->clock,TRUE);
  
}

static void time_unpaused(Game2Player * game) {
  clock_pause( PRIVATE(game)->clock,FALSE);
	 
  
}

static void time_init(Game2Player * game) {
  clock_start( PRIVATE(game)->clock);
}

static void game_2_player_start(Game * game) {
  Game2Player * g;
 
  g_assert( IS_GAME_2_PLAYER(game));
  
  g = GAME_2_PLAYER(game);

  time_init( g );

  PRIVATE(g)->state = GAME_PLAYING;
  game_2_player_connect_input(g);
}

static void game_2_player_stop(Game * game) {
  Game2Player * g;

  g_assert( IS_GAME_2_PLAYER(game));
  
  g = GAME_2_PLAYER(game);

  PRIVATE(g)->state = GAME_STOPPED;
  game_2_player_disconnect_input(g);

}

static void game_2_player_connect_input(Game2Player * game) {
  g_signal_connect( G_OBJECT( PRIVATE(game)->window) ,"key-press-event",
		    GTK_SIGNAL_FUNC (game_2_player_key_pressed),game);
  
  g_signal_connect( G_OBJECT( PRIVATE(game)->window) ,"key-release-event",
		    GTK_SIGNAL_FUNC (game_2_player_key_released),game);
}

static void game_2_player_disconnect_input(Game2Player * game) {
  g_signal_handlers_disconnect_by_func( G_OBJECT(PRIVATE(game)->window) ,
					GTK_SIGNAL_FUNC (game_2_player_key_pressed),game);


  g_signal_handlers_disconnect_by_func( G_OBJECT(PRIVATE(game)->window) ,
					GTK_SIGNAL_FUNC (game_2_player_key_released),game);
}

static void game_2_player_pause(Game * game,gboolean pause) {
  Game2Player * g;
  g_assert( IS_GAME_2_PLAYER(game));
  
  g = GAME_2_PLAYER(game);

  if( pause ) {
    PRIVATE(g)->state = GAME_PAUSED;
    time_paused( g);
    game_2_player_disconnect_input(g);
    gdk_canvas_add_block(PRIVATE(g)->canvas,
								 PRIVATE(g)->paused_layer,
								 PRIVATE(g)->paused_block,
								 0,0);
	 game_2_player_fire_changed(g);


  } else {
    PRIVATE(g)->state = GAME_PLAYING;
    time_unpaused( g);
    game_2_player_connect_input(g);
    gdk_canvas_remove_block(PRIVATE(g)->canvas,
									 PRIVATE(g)->paused_block);    
	 game_2_player_fire_changed(g);

  }
}

static GameState game_2_player_get_state(Game * game) {
  Game2Player * g;
  g_assert( IS_GAME_2_PLAYER(game));
  
  g = GAME_2_PLAYER(game);

  return PRIVATE(g)->state;
}


static void game_2_player_game_lost(IMonkeyObserver *bo,Monkey * monkey) {
  Game2Player * g;
  g_assert( IS_GAME_2_PLAYER(bo));

  g = GAME_2_PLAYER(bo);

  if( PRIVATE(g)->monkey_left == monkey ) {
  gdk_view_draw_lost(PRIVATE(g)->display_left);
  gdk_view_draw_win(PRIVATE(g)->display_right);

		PRIVATE(g)->winner = PLAYER_2;
  } else {
		PRIVATE(g)->winner = PLAYER_1;

  gdk_view_draw_win(PRIVATE(g)->display_left);
  gdk_view_draw_lost(PRIVATE(g)->display_right);

  }

  gdk_canvas_paint(PRIVATE(g)->canvas);

  time_paused(g);
  
  game_2_player_stop(GAME(g));
  PRIVATE(g)->state = GAME_FINISHED;

  game_2_player_fire_changed(g);

}

static void game_2_player_bubbles_exploded(IMonkeyObserver * bo,
					   Monkey * monkey,
					   GList * exploded,
					   GList * fallen) {
  int i;
  int * columns;
  int to_go;
  GdkView * view;
  Game2Player * p;
  Monkey * other;
  Color * colors;

  g_assert( IS_GAME_2_PLAYER(bo));
  
  p = GAME_2_PLAYER(bo);
  to_go = MAX(0,-3 + g_list_length(exploded)  + g_list_length(fallen)*1.5);
  g_print("exploded : %d , fallen : %d, other player %d\n",g_list_length(exploded),
			 g_list_length(fallen),
			 to_go);

  if( to_go != 0) {

    colors = g_malloc( sizeof(Color)*to_go );

    for( i = 0 ; i < to_go ; i++ ) {
		  colors[i] = rand()%COLORS_COUNT;
    }


    other = PRIVATE(p)->monkey_left;
	 view = PRIVATE(p)->display_left;
    if( monkey == other ) {
		  view = PRIVATE(p)->display_right;

      other = PRIVATE(p)->monkey_right;
    }

    columns = monkey_add_bubbles( other, 
				  to_go,
				  colors); 
  

    g_free(columns); 
    g_free(colors); 
  }
}

static gint game_2_player_add_bubble(Game2Player * game,Monkey * monkey) {
  gint * colors_count;
  gint rnd,count;

  g_assert( IS_GAME_2_PLAYER(game));

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

  return count;

}

static void game_2_player_bubble_shot(IMonkeyObserver * bo,
				      Monkey * monkey,
				      Bubble * bubble) {


  game_2_player_add_bubble(GAME_2_PLAYER(bo),monkey);
}


static void game_2_player_bubble_sticked(IMonkeyObserver * bo,
					 Monkey * monkey,
					 Bubble * bubble) {

  Game2Player * g;
  Bubble ** bubbles;
  int i;

  g_assert( IS_GAME_2_PLAYER(bo));

  g = GAME_2_PLAYER(bo);
  
  
  if( (monkey_get_shot_count( monkey ) % 12) == 0) {
    
    bubbles = g_malloc( sizeof(Bubble *) * 8 );
  
    for(i = 0;i < 8; i++) {
      bubbles[i] =bubble_new(rand()%COLORS_COUNT,0,0);
    }
    monkey_insert_bubbles(monkey,bubbles);

    g_free(bubbles);
  }

}


int game_2_player_get_winner(Game2Player * g) {
	 g_assert( IS_GAME_2_PLAYER(g));

	 return PRIVATE(g)->winner;
}


void game_2_player_fire_changed(Game2Player * game ) {
	 GList * next;

	 next = PRIVATE(game)->observers;

	 while( next != NULL ) {
		  
		  IGameObserver * o = IGAME_OBSERVER(next->data);
		  igame_observer_changed(o,GAME(game));
		  next = g_list_next(next);
	 }

}

void game_2_player_attach_observer(Game * game,IGameObserver *observer) {
  Game2Player * g;
  g_assert( IS_GAME_2_PLAYER(game));
  
  g = GAME_2_PLAYER(game);

  PRIVATE(g)->observers = g_list_append( PRIVATE(g)->observers, observer);
}

void game_2_player_detach_observer(Game * game,IGameObserver *observer) {
  Game2Player * g;
  g_assert( IS_GAME_2_PLAYER(game));
  
  g = GAME_2_PLAYER(game);

  PRIVATE(g)->observers = g_list_remove( PRIVATE(g)->observers, observer);
}
