/* network-game.c - 
 * Copyright (C) 2002 Christophe Segui
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
#include "network-game.h"
#include "network-ui.h"
#include "monkey-client.h"
#include <unistd.h>
#include "gdk-canvas.h"
#include "gdk-view.h"
#include "clock.h"
#define FRAME_DELAY 20

#define PRIVATE(network_game) (network_game->private)
					
static void network_game_start(Game * game);
static void network_game_stop(Game * game);
static void network_game_pause(Game * game,gboolean pause);
static void propagate_network_update(NetworkGame *,  gint, gint);
void * display_foreign_client (gpointer key, gpointer value, gpointer user_data);
void *update_foreign_monkeys_client(gpointer key, gpointer value, gpointer user_data);
gint *update_display_players(gpointer);
static void do_draw_foreign_monkeys(NetworkGame *);

static GameState network_game_get_state(Game * game);

static void network_game_game_iface_init(GameClass * i);
void network_game_game_attach_observer	(Game * game,IGameObserver *observer);
void network_game_game_detach_observer	(Game * game,IGameObserver *observer);

static gint network_game_timeout (gpointer data);

static gboolean network_game_key_released(GtkWidget *widget,
					   GdkEventKey *event,
					   gpointer user_data);

static gboolean network_game_key_pressed(GtkWidget *widget,
					  GdkEventKey *event,
					  gpointer user_data);

static void time_paused(NetworkGame * game);
					  
static GObjectClass* parent_class = NULL;
struct NetworkGamePrivate {
  GdkCanvas * canvas;
  GtkWidget * window;
  GdkView * display;
  Monkey * monkey;
  guint timeout_id;
  guint update_display_players_timeout;
  GameState state;
  Clock *clock;
  GList * observers;
  MonkeyClient *monkey_client;
  gint last_game_event; 
  GSList *foreign_players_canvas_list;
  GSList *foreign_players_display_list;  
  GSList *foreign_players_window_list;
  GCond *are_foreign_monkeys_displayable;  
  unsigned long game_offset;  
};


static void network_game_instance_init(NetworkGame * game) {
  game->private =g_new0 (NetworkGamePrivate, 1);		
}

static void network_game_finalize(GObject* object) {

  NetworkGame * game = NETWORK_GAME(object);
  UiNetwork *uin = ui_network_get_instance();

  g_assert(IS_NETWORK_GAME(object));

  if (G_IS_OBJECT(PRIVATE(game)->update_display_players_timeout))
	  gtk_timeout_remove(PRIVATE(game)->update_display_players_timeout);

	game_stop(GAME(game));		
		
	ui_network_stop(uin);	

	if(PRIVATE(game)->observers != NULL) {
    	g_error("[NetworkGame] All observers has not been removed");		
  	}  	
  
  g_free(game->private);

  
  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


static void network_game_class_init (NetworkGameClass *klass) {

    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = network_game_finalize;
}


GType network_game_get_type(void) {
    static GType network_game_type = 0;
    
    if (!network_game_type) {
      static const GTypeInfo network_game_info = {
	sizeof(NetworkGameClass),
	NULL,           /* base_init */
	NULL,           /* base_finalize */
	(GClassInitFunc) network_game_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data */
	sizeof(NetworkGame),
	1,              /* n_preallocs */
	(GInstanceInitFunc) network_game_instance_init,
      };

      static const GInterfaceInfo iface_game = {
	(GInterfaceInitFunc) network_game_game_iface_init,
	NULL,
	NULL
      };

      network_game_type = g_type_register_static(G_TYPE_OBJECT,
						  "NetworkGame",
						  &network_game_info, 0);

      g_type_add_interface_static(network_game_type,
				  TYPE_GAME,
				  &iface_game);
     
    }
    
    return network_game_type;
}

static void network_game_game_iface_init(GameClass * i) {
  i->start = network_game_start;
  i->stop = network_game_stop;
  i->pause = network_game_pause;
  i->get_state = network_game_get_state;
  i->attach_observer = network_game_game_attach_observer;
  i->detach_observer = network_game_game_detach_observer;
}

NetworkGame * network_game_new(GtkWidget * window,GdkCanvas * canvas, gchar *srv, unsigned short port) {
  NetworkGame * game;
	
  game = NETWORK_GAME (g_object_new (TYPE_NETWORK_GAME, NULL));

  

#ifdef G_THREADS_ENABLED  
	if (!g_thread_supported()) 
		g_thread_init (NULL);
#endif		
	PRIVATE(game)->monkey = monkey_new();
	PRIVATE(game)->monkey_client= monkey_client_new(game);
	PRIVATE(game)->are_foreign_monkeys_displayable = g_cond_new();	
	
	if (!monkey_client_init(port, srv, PRIVATE(game)->monkey_client)) {
#ifdef _DEBUG_		
		g_print("**DEBUG** NetworkGame->newtork_game_new() :Unable to intialize client, exiting ...\n");
#endif		
		return NULL;		
	}
	monkey_client_start(PRIVATE(game)->monkey_client);	

  PRIVATE(game)->display = gdk_view_new(canvas, PRIVATE(game)->monkey,0,0,TRUE);
  PRIVATE(game)->canvas = canvas;
  PRIVATE(game)->window = window;

  g_signal_connect( G_OBJECT( window) ,"key-press-event",
		    GTK_SIGNAL_FUNC (network_game_key_pressed),game);
  
  g_signal_connect( G_OBJECT( window) ,"key-release-event",
		    GTK_SIGNAL_FUNC (network_game_key_released),game);

 
  PRIVATE(game)->timeout_id = 
    gtk_timeout_add (GAME_TIMEOUT, network_game_timeout, game);
  
  PRIVATE(game)->observers = NULL;
  PRIVATE(game)->last_game_event = 0;

  PRIVATE(game)->state = GAME_STOPPED;
  PRIVATE(game)->game_offset = 0;
  PRIVATE(game)->clock = clock_new();
  clock_start(PRIVATE(game)->clock);

  return game;
}

static gboolean network_game_key_pressed(GtkWidget *widget,
					  GdkEventKey *event,
					  gpointer user_data) {
  
  
  NetworkGame * game;
  Monkey * monkey;
  
  game = NETWORK_GAME(user_data);

  if( PRIVATE(game)->state == GAME_PLAYING) {
    monkey = PRIVATE(game)->monkey;
    
    if ( event->keyval == 115 ){
	  gint time = network_game_get_time(game);    	
      monkey_left_changed( monkey,TRUE,time);
      if (PRIVATE(game)->last_game_event != MOVE_LEFT_PRESSED) {
	      propagate_network_update(game, MOVE_LEFT_PRESSED, time);      
    	  PRIVATE(game)->last_game_event = MOVE_LEFT_PRESSED ;
      }
    }
    
    if( event->keyval == 102 ){
      gint time = network_game_get_time(game);
      monkey_right_changed( monkey,TRUE,time);
      if (PRIVATE(game)->last_game_event != MOVE_RIGHT_PRESSED) {
	      propagate_network_update(game, MOVE_RIGHT_PRESSED, time);
    	  PRIVATE(game)->last_game_event = MOVE_RIGHT_PRESSED ;      
      }
    }
    
    
    if( event->keyval == 100 ) {
    	gint time = network_game_get_time(game);
      monkey_shoot( monkey,network_game_get_time(game));
      propagate_network_update(game, SHOOT, time);
      PRIVATE(game)->last_game_event = SHOOT;
    }
  }

  return TRUE;
}

static gboolean network_game_key_released(GtkWidget *widget,
					   GdkEventKey *event,
					   gpointer user_data) {

  NetworkGame * game;
  Monkey * monkey;

  game = NETWORK_GAME(user_data);
  monkey = PRIVATE(game)->monkey;

  if( PRIVATE(game)->state == GAME_PLAYING ) {
    if( event->keyval == 115 ) {
      gint time = network_game_get_time(game);
      monkey_left_changed( monkey,FALSE,time);
      if (PRIVATE(game)->last_game_event != MOVE_LEFT_RELEASED) {
	      propagate_network_update(game, MOVE_LEFT_RELEASED, time);
	      PRIVATE(game)->last_game_event = MOVE_LEFT_RELEASED; 
      }
    }
    
    if( event->keyval == 102 ) {
    	gint time = network_game_get_time(game);
      monkey_right_changed( monkey,FALSE,time);
      if (PRIVATE(game)->last_game_event != MOVE_RIGHT_RELEASED) {
	      propagate_network_update(game, MOVE_RIGHT_RELEASED, time);          
	      PRIVATE(game)->last_game_event = MOVE_RIGHT_RELEASED ; 
      }
    }

  }
  return TRUE;

}

static gint network_game_timeout (gpointer data)
{

  NetworkGame * game;
  Monkey * monkey;
  gint time;

  game = NETWORK_GAME(data);
  monkey = PRIVATE(game)->monkey;

  if( PRIVATE(game)->state == GAME_PLAYING ) {
  
    time = network_game_get_time(game);
  
    
    monkey_update( monkey,
		   time );
    
    gdk_view_update( PRIVATE(game)->display,
		    time);
    
    gdk_canvas_paint(PRIVATE(game)->canvas);
    
  }
  
  return TRUE;
}

gint network_game_get_time(NetworkGame * game) {
  return clock_get_time(PRIVATE(game)->clock);
}

static void time_paused(NetworkGame * game) {
}

static void time_unpaused(NetworkGame * game) {
}

void network_game_time_init(NetworkGame * game, GTimeVal *time) {

}

static void network_game_start(Game * game) {
  NetworkGame * g;
  g_assert( IS_NETWORK_GAME(game));
  
  g = NETWORK_GAME(game);

  
   while (g_hash_table_size(monkey_client_get_other_monkey_players(PRIVATE(NETWORK_GAME(g))->monkey_client)) == 0) {
   		g_print("Waiting for PlayerHashTable\n");
   		sleep(2);
   }
  PRIVATE(g)->update_display_players_timeout = gtk_timeout_add(FRAME_DELAY, (GtkFunction)update_display_players,game); 	
  g_hash_table_foreach(monkey_client_get_other_monkey_players(PRIVATE(NETWORK_GAME(g))->monkey_client), (GHFunc )display_foreign_client,game);  

  PRIVATE(g)->state = GAME_PLAYING;
}
void network_game_abort(NetworkGame *game) {
	
	int i;	
	g_assert( IS_NETWORK_GAME(game));
	network_game_stop(GAME(game));
		
	for (i=0; i < (g_slist_length(PRIVATE(game)->foreign_players_canvas_list));i++) {					
		if (((g_slist_nth(PRIVATE(game)->foreign_players_display_list, i))->data) != NULL) 
				g_object_unref((GdkView *) g_slist_nth(PRIVATE(game)->foreign_players_display_list, i)->data);
		if (((g_slist_nth(PRIVATE(game)->foreign_players_canvas_list, i))->data) != NULL) 
				g_object_unref((GdkCanvas *) g_slist_nth(PRIVATE(game)->foreign_players_canvas_list, i)->data);
		if (((g_slist_nth(PRIVATE(game)->foreign_players_window_list, i))->data) != NULL) 
				gtk_widget_destroy( (GtkWidget *) g_slist_nth(PRIVATE(game)->foreign_players_window_list, i)->data);
	}
	if (g_slist_length(PRIVATE(game)->foreign_players_canvas_list) > 0 ) {	  
		g_slist_free(PRIVATE(game)->foreign_players_canvas_list);
		g_slist_free(PRIVATE(game)->foreign_players_display_list);
		g_slist_free(PRIVATE(game)->foreign_players_window_list);		
	}

	if (G_IS_OBJECT(PRIVATE(game)->monkey_client))
	  g_object_unref(PRIVATE(game)->monkey_client);
	if (G_IS_OBJECT(PRIVATE(game)->monkey))
	  g_object_unref(PRIVATE(game)->monkey);
     						
}
static void network_game_stop(Game * game) {
  
  NetworkGame *g;


  g_assert( IS_NETWORK_GAME(game));
  
  g = NETWORK_GAME(game);
  PRIVATE(g)->state = GAME_STOPPED;
  		

  g_signal_handlers_disconnect_by_func( G_OBJECT(PRIVATE(g)->window) ,
		    GTK_SIGNAL_FUNC (network_game_key_pressed),g);


  g_signal_handlers_disconnect_by_func( G_OBJECT(PRIVATE(g)->window) ,
		    GTK_SIGNAL_FUNC (network_game_key_released),g);

  gtk_timeout_remove( PRIVATE(g)->timeout_id);


}

static void network_game_pause(Game * game,gboolean pause) {
  NetworkGame * g;
  g_assert( IS_NETWORK_GAME(game));
  
  g = NETWORK_GAME(game);

  if( pause ) {
    PRIVATE(g)->state = GAME_PAUSED;
    time_paused( g);
  } else {
    PRIVATE(g)->state = GAME_PLAYING;
    time_unpaused( g);

  }
}

static GameState network_game_get_state(Game * game) {
  NetworkGame * g;
  g_assert( NETWORK_GAME(game));
  
  g = NETWORK_GAME(game);

  return PRIVATE(g)->state;
}

void network_game_game_attach_observer(Game * game,IGameObserver *observer)  {
  NetworkGame * g;
  g_assert( NETWORK_GAME(game));
  
  g = NETWORK_GAME(game);

  PRIVATE(g)->observers = g_list_append( PRIVATE(g)->observers, observer);
}

void network_game_game_detach_observer	(Game * game,IGameObserver *observer) {
  NetworkGame * g;
  g_assert( IS_NETWORK_GAME(game));
  
  g = NETWORK_GAME(game);

  PRIVATE(g)->observers = g_list_remove( PRIVATE(g)->observers, observer);
}
static void propagate_network_update(NetworkGame *game,  gint code, gint time) {
	MonkeyMessage *m;
	m = (struct _monkeyMessage *) g_malloc (sizeof(struct _monkeyMessage));	
	m->message = code;
	//local time initialized at 0 when BEGIN_GAME event received. Adding game_offset to have a network time

   g_print("**DEBUG** NetworkGame->propagate_network_update()  : Update original time_stamp %d with time stamp %d\n", time, time + (int) PRIVATE(game)->game_offset);
	
	m->time_stamp = time + PRIVATE(game)->game_offset;	
	monkey_client_send_update_to_monkey_server(m,PRIVATE(NETWORK_GAME(game))->monkey_client);				
	g_free(m);	
	
}
void network_game_send_message(NetworkGame *ng, MonkeyMessage *mm) {
	monkey_client_send_update_to_monkey_server(mm,PRIVATE(NETWORK_GAME(ng))->monkey_client);
}

void  *update_foreign_monkeys_client      (gpointer key, gpointer value, gpointer user_data) {
//	monkey_update((Monkey *) value, network_game_get_time((NetworkGame *) user_data) - PRIVATE(NETWORK_GAME(user_data))->game_offset );
monkey_update((Monkey *) value, network_game_get_time((NetworkGame *) user_data));
	return(0);
}

void  *display_foreign_client(gpointer key, gpointer value, gpointer user_data) {
 
   NetworkGame *ng = (NetworkGame *) user_data;
 	  GdkCanvas * canvas;	
   GdkView *display;
   GtkWidget *vbox;
   GtkWidget *window;
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

#ifdef _DEBUG_
	g_print("**DEBUG** NetworkGame>display_foreign_monkeys() : Displaying foreign client\n");
#endif	
   vbox = gtk_vbox_new(FALSE,0);
   canvas = gdk_canvas_new();
   gtk_container_add (GTK_CONTAINER (window), vbox);

   display = gdk_view_new(canvas, MONKEY(value),0,0,TRUE);
    gtk_box_pack_start (GTK_BOX (vbox),GTK_WIDGET(canvas), FALSE, FALSE, 0);

   PRIVATE(ng)->foreign_players_canvas_list = g_slist_append(PRIVATE(ng)->foreign_players_canvas_list, canvas );
   PRIVATE(ng)->foreign_players_display_list = g_slist_append(PRIVATE(ng)->foreign_players_display_list, display);    
   PRIVATE(ng)->foreign_players_window_list = g_slist_append(PRIVATE(ng)->foreign_players_window_list, window);       
   	do_draw_foreign_monkeys(ng);
   return(0);
}

gint *update_display_players(gpointer data) {
		NetworkGame *ng = NETWORK_GAME(data);
		int i;
 #ifdef _DEBUG_
	g_print("**DEBUG** NetworkGame->update_display_players() :Updating  display foreign client\n");
#endif	
	
		for (i=0; i < (g_slist_length(PRIVATE(ng)->foreign_players_canvas_list));i++) {					
			if (((g_slist_nth(PRIVATE(ng)->foreign_players_display_list, i))->data) != NULL)
				gdk_view_update((GdkView *) g_slist_nth(PRIVATE(ng)->foreign_players_display_list, i)->data,network_game_get_time(ng) );
			if (((g_slist_nth(PRIVATE(ng)->foreign_players_canvas_list, i))->data) != NULL)
				 gdk_canvas_paint((GdkCanvas *) g_slist_nth(PRIVATE(ng)->foreign_players_canvas_list, i)->data);				
	}
	g_hash_table_foreach(monkey_client_get_other_monkey_players(PRIVATE(NETWORK_GAME(data))->monkey_client), (GHFunc )update_foreign_monkeys_client,ng);  		 
	PRIVATE(ng)->update_display_players_timeout = gtk_timeout_add(FRAME_DELAY, (GtkFunction)update_display_players,data);  		
	return(0);
}

void network_game_draw_foreign_monkeys(NetworkGame *ng) {
	int i;
	GMutex *m= g_mutex_new();

	g_cond_wait(PRIVATE(ng)->are_foreign_monkeys_displayable, m);

	for (i=0; i < (g_slist_length(PRIVATE(ng)->foreign_players_canvas_list));i++) {			
		if (((g_slist_nth(PRIVATE(ng)->foreign_players_canvas_list, i))->data) != NULL)
			 gdk_canvas_paint((GdkCanvas *) g_slist_nth(PRIVATE(ng)->foreign_players_canvas_list, i)->data);		
		if (((g_slist_nth(PRIVATE(ng)->foreign_players_window_list, i))->data) != NULL)
				gtk_widget_show_all((GtkWidget  *) g_slist_nth(PRIVATE(ng)->foreign_players_window_list, i)->data);		
	}
	g_cond_free(PRIVATE(ng)->are_foreign_monkeys_displayable);
//	g_mutex_free(m);		
}

static void do_draw_foreign_monkeys(NetworkGame *ng) {	
	g_cond_signal(PRIVATE(ng)->are_foreign_monkeys_displayable);
}


void network_game_update_monkey(NetworkGame *ng, MonkeyMessage *m) {
	
	m->time_stamp -= PRIVATE(ng)->game_offset;
	switch (m->message) {
		
		case PLAYER_WIN:
		case PLAYER_LOST: break;
		case NEXT_BUBBLE_TO_SHOOT:
#ifdef _DEBUG_
													g_print("**DEBUG** NetworkGame->update_monkey() : next bubble color for our monkey is %d\n",m->arg.color);
#endif
														shooter_add_bubble(monkey_get_shooter(PRIVATE(ng)->monkey),bubble_new(m->arg.color,0,0));
														break;
		default: g_print("NetworkGame->UpdateMonkey() : Unknown Message %d\n", m->message);
					break;
	}
}
void network_game_set_game_offset(NetworkGame *ng, unsigned long time) { 
	PRIVATE(ng)->game_offset = time;
}
