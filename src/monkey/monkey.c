/* monkey.c
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
#include "monkey.h"
#include <math.h>
#include <stdlib.h>
#include "monkey-marshal.h"

#define PRIVATE(monkey) (monkey->private )
#define MAX_UPDATE_TIME 1
 
static GObjectClass* parent_class = NULL;

enum {
  GAME_LOST,
  BUBBLES_EXPLODED,
  BUBBLE_SHOT,
  BUBBLE_STICKED,
  BUBBLES_WAITING_CHANGED,
  LAST_SIGNAL
};

static guint32 signals[LAST_SIGNAL];

struct MonkeyPrivate {
  Playground * playground;
  Shooter * shooter;  
  gint left_pressed;
  gint left_pressed_time;
  gint right_pressed;
  gint right_pressed_time;
  gint time;
  gint last_shoot;
  gint last_stiked;
  gint shot_count;
  GList * to_add;
};

static void monkey_finalize(GObject* object);

static void monkey_update_shooter(Monkey * monkey,gint time);
static void monkey_add_waiting_row(Monkey * monkey);


static void monkey_bubble_sticked ( Board *board,Bubble * bubble,
				   gint time,
				    Monkey * monkey);

static void monkey_bubbles_exploded ( Board *board,
				     GList * exploded,
				     GList * fallen,
				      Monkey * monkey);


static void monkey_add_new_waiting_row(Monkey * monkey);
static void monkey_playground_lost(Playground * pg,Monkey * monkey);

static void monkey_playground_shot(Playground *pg,Bubble * b,Monkey * monkey);

static void monkey_notify_game_lost(Monkey * monkey);
static void monkey_notify_bubble_shot(Monkey * monkey,Bubble * b);
static void monkey_notify_bubbles_exploded(Monkey * monkey,
					   GList * exploded,
					   GList * fallen);


static gboolean monkey_has_waiting_row(Monkey * monkey);

static void monkey_instance_init(Monkey * monkey) {
  monkey->private =g_new0 (MonkeyPrivate, 1);			
}


static void monkey_class_init (MonkeyClass *klass) {
  GObjectClass* object_class;

  parent_class = g_type_class_peek_parent(klass);
  object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = monkey_finalize;

    signals[GAME_LOST]= g_signal_new ("game-lost",
				      G_TYPE_FROM_CLASS (klass),
				      G_SIGNAL_RUN_FIRST |
				      G_SIGNAL_NO_RECURSE,
				      G_STRUCT_OFFSET (MonkeyClass, game_lost),
					     NULL, NULL,
					     g_cclosure_marshal_VOID__VOID,
					     G_TYPE_NONE,
				      0,NULL);


    signals[BUBBLES_EXPLODED]= g_signal_new ("bubbles-exploded",
					     G_TYPE_FROM_CLASS (klass),
					     G_SIGNAL_RUN_FIRST |
					     G_SIGNAL_NO_RECURSE,
					     G_STRUCT_OFFSET (MonkeyClass, bubbles_exploded),
					     NULL, NULL,
					     monkey_marshal_VOID__POINTER_POINTER,
					     G_TYPE_NONE,
					     2, G_TYPE_POINTER,G_TYPE_POINTER);

    signals[BUBBLE_SHOT]= g_signal_new ("bubble-shot",
					     G_TYPE_FROM_CLASS (klass),
					     G_SIGNAL_RUN_FIRST |
					     G_SIGNAL_NO_RECURSE,
					     G_STRUCT_OFFSET (MonkeyClass, bubble_shot),
					     NULL, NULL,
					g_cclosure_marshal_VOID__POINTER,
					G_TYPE_NONE,
					     1, G_TYPE_POINTER);

    signals[BUBBLE_STICKED]= g_signal_new ("bubble-sticked",
					     G_TYPE_FROM_CLASS (klass),
					     G_SIGNAL_RUN_FIRST |
					     G_SIGNAL_NO_RECURSE,
					     G_STRUCT_OFFSET (MonkeyClass, bubble_sticked),
					     NULL, NULL,
					     g_cclosure_marshal_VOID__POINTER,
					     G_TYPE_NONE,
					     1, G_TYPE_POINTER);


    signals[BUBBLES_WAITING_CHANGED]= g_signal_new ("bubbles-waiting-changed",
					     G_TYPE_FROM_CLASS (klass),
					     G_SIGNAL_RUN_FIRST |
					     G_SIGNAL_NO_RECURSE,
					     G_STRUCT_OFFSET (MonkeyClass, bubbles_waiting_changed),
						    NULL, NULL,
						    g_cclosure_marshal_VOID__INT,
						    G_TYPE_NONE,
						    1, G_TYPE_INT);
}


GType monkey_get_type(void) {
  static GType monkey_type = 0;
    
  if (!monkey_type) {
    static const GTypeInfo monkey_info = {
      sizeof(MonkeyClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) monkey_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(Monkey),
      1,              /* n_preallocs */
      (GInstanceInitFunc) monkey_instance_init,
    };


    monkey_type = g_type_register_static(G_TYPE_OBJECT,
					 "Monkey",
					 &monkey_info,
					 0);


      
  }
    
  return monkey_type;
}


Monkey * monkey_new_level_from_file(const gchar * filename,int level) {
  Monkey * monkey;
  monkey = MONKEY (g_object_new (TYPE_MONKEY, NULL));


  PRIVATE(monkey)->playground = playground_new( 446,190,filename,level);
  
  PRIVATE(monkey)->shooter = shooter_new( 318,400 ,
					  -M_PI/2+0.01,M_PI/2-0.01,0.5);

  PRIVATE(monkey)->shot_count = 0;

  PRIVATE(monkey)->left_pressed = 0;
  PRIVATE(monkey)->left_pressed_time = 0;
  PRIVATE(monkey)->right_pressed = 0;
  PRIVATE(monkey)->left_pressed_time = 0;
  PRIVATE(monkey)->time = 0;

  PRIVATE(monkey)->last_stiked = 0;

  

  PRIVATE(monkey)->to_add = NULL;


  
  g_signal_connect( G_OBJECT( playground_get_board(PRIVATE(monkey)->playground)),
		    "bubbles-exploded",G_CALLBACK(monkey_bubbles_exploded),monkey);


  g_signal_connect( G_OBJECT( playground_get_board(PRIVATE(monkey)->playground)),
		    "bubble-sticked",G_CALLBACK(monkey_bubble_sticked),monkey);

  g_signal_connect( G_OBJECT( PRIVATE(monkey)->playground),
		    "bubble-shot",G_CALLBACK(monkey_playground_shot),monkey);

  g_signal_connect( G_OBJECT( PRIVATE(monkey)->playground),
		    "game-lost",G_CALLBACK(monkey_playground_lost),monkey);
  return monkey;
}

Monkey * monkey_new(void) {

  return monkey_new_level_from_file(NULL,-1);
}


static void monkey_finalize(GObject* object) {
  GList * next;
  int i;
  Bubble ** bubbles;
  Monkey * monkey = MONKEY(object);
  


  g_signal_handlers_disconnect_matched(  G_OBJECT(PRIVATE(monkey)->playground) ,
					 G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,monkey);


  g_signal_handlers_disconnect_matched(  G_OBJECT( playground_get_board(PRIVATE(monkey)->playground) ),
					 G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,monkey);

  g_object_unref( G_OBJECT(PRIVATE(monkey)->playground));
  g_object_unref( G_OBJECT(PRIVATE(monkey)->shooter));

  next = PRIVATE(monkey)->to_add;

  while( next != NULL ) {
    bubbles = (Bubble **) next->data;

    for(i=0;i < 7; i++ ) {
      if( bubbles[i] != NULL ) {
	g_object_unref(bubbles[i]);
      }
    }
    g_free(next->data);
    next = g_list_next(next);
  }
  
  g_list_free( PRIVATE(monkey)->to_add);
  g_free(monkey->private);
  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


void monkey_left_changed( Monkey * monkey,gboolean pressed,
			  gint time) {

  g_assert( IS_MONKEY( monkey ) );

  if( pressed ) {

    if( PRIVATE(monkey)->left_pressed != 0 ) return;
    PRIVATE(monkey)->left_pressed = time;

  } else {
    PRIVATE(monkey)->left_pressed_time += 
      time - PRIVATE(monkey)->left_pressed;
  
    PRIVATE(monkey)->left_pressed = 0;
  }

}

void monkey_shoot(Monkey * monkey,gint time) {
  g_assert( IS_MONKEY( monkey ) );

  monkey_update_shooter( monkey, time );

  if( playground_is_ready_for_shoot( PRIVATE(monkey)->playground) &&
      (abs( time - PRIVATE(monkey)->last_shoot) > 500) ) {

    PRIVATE(monkey)->last_shoot = time;
    playground_shoot_bubble( PRIVATE(monkey)->playground,
			     shooter_shoot( PRIVATE(monkey)->shooter));
    PRIVATE(monkey)->shot_count++;
  }
}

void monkey_right_changed( Monkey * monkey,gboolean pressed,
			   gint time) {

  g_assert( IS_MONKEY( monkey ) );

  if( pressed ) {

    if( PRIVATE(monkey)->right_pressed != 0 ) return;
    PRIVATE(monkey)->right_pressed = time;

  } else {
    PRIVATE(monkey)->right_pressed_time += time -
      PRIVATE(monkey)->right_pressed;

    PRIVATE(monkey)->right_pressed = 0;
  }

}


static void monkey_update_shooter(Monkey * monkey,gint time) {

  gdouble new_angle;

  if( PRIVATE(monkey)->right_pressed != 0 ) {
    PRIVATE(monkey)->right_pressed_time += time -
      PRIVATE(monkey)->right_pressed;

    PRIVATE(monkey)->right_pressed = time;
    
  }



  if( PRIVATE(monkey)->left_pressed != 0 ) {
    PRIVATE(monkey)->left_pressed_time += time -
      PRIVATE(monkey)->left_pressed;

    PRIVATE(monkey)->left_pressed = time;
    
  }


  if( ( PRIVATE(monkey)->left_pressed_time != 0) ||
      ( PRIVATE(monkey)->right_pressed_time != 0 ) ) {

    new_angle =
      (((gdouble)
	( PRIVATE(monkey)->left_pressed_time - PRIVATE(monkey)->right_pressed_time))
       / 2000)*M_PI
      +
      shooter_get_angle(PRIVATE(monkey)->shooter);


    shooter_set_angle( PRIVATE(monkey)->shooter,
		       new_angle
		       );

    PRIVATE(monkey)->left_pressed_time = 0;
    PRIVATE(monkey)->right_pressed_time =0;
  }

}

void monkey_update( Monkey * monkey,gint time ) {

  g_assert( IS_MONKEY( monkey ) );

  monkey_update_shooter(monkey,time);


  
    
  while ( ( time - PRIVATE( monkey )->time ) != 0 ) {

    if( ( time - PRIVATE( monkey )->time ) > MAX_UPDATE_TIME ) {
      playground_update( PRIVATE( monkey )->playground,
			 MAX_UPDATE_TIME );
    
      PRIVATE( monkey )->time += MAX_UPDATE_TIME;  

    } else {

      playground_update( PRIVATE( monkey )->playground,
			 ( time - PRIVATE( monkey )->time ) );
    
      PRIVATE( monkey )->time = time;  

    }
  
  }
  PRIVATE( monkey )->time = time;

   if( (time - PRIVATE(monkey)->last_stiked) > 10000) { 
     monkey_shoot(monkey,time);
  }
}


Shooter * monkey_get_shooter(Monkey * monkey) {
  g_assert( IS_MONKEY( monkey ) );

  return PRIVATE(monkey)->shooter;
}


gint monkey_get_shot_count(Monkey * monkey) {
  g_assert( IS_MONKEY( monkey ) );

  return PRIVATE(monkey)->shot_count;
}

Playground * monkey_get_playground(Monkey * monkey) {
  g_assert( IS_MONKEY( monkey ) );

  return PRIVATE(monkey)->playground;
}
  

static void monkey_notify_bubbles_waiting_changed(Monkey * monkey) {

  GList * next;
  Bubble ** bubbles;
  int i;
  int count;

  count = 0;
  
  next = PRIVATE(monkey)->to_add;
  while( next != NULL ) {
		bubbles = (Bubble **)next->data;
    
		for(i= 0; i < 7 ;i++) {
			 if( bubbles[i] != NULL ) {
				  count++;
			 }
		}

		next = g_list_next(next);
  }



  g_signal_emit( G_OBJECT(monkey),signals[BUBBLES_WAITING_CHANGED],0,count);
  

}

																  
static void monkey_notify_bubbles_exploded(Monkey * monkey,
					   GList * exploded,
					   GList * fallen) {

  
  g_signal_emit( G_OBJECT(monkey),signals[BUBBLES_EXPLODED],0,exploded,fallen);

  
}


static void monkey_notify_bubble_shot(Monkey * monkey,Bubble * b) {
  g_signal_emit( G_OBJECT(monkey),signals[BUBBLE_SHOT],0,b);
}

static void monkey_notify_bubble_sticked(Monkey * monkey,Bubble * b) {
  g_signal_emit( G_OBJECT(monkey),signals[BUBBLE_STICKED],0,b);
}

static void monkey_notify_game_lost(Monkey * monkey) {
  g_signal_emit( G_OBJECT(monkey),signals[GAME_LOST],0);
}

static void monkey_playground_shot(Playground *pg,Bubble * b,Monkey * monkey) {
  g_assert( IS_MONKEY(monkey) );


  monkey_notify_bubble_shot(monkey,b);
  

}


static void monkey_playground_lost(Playground * pg,
				   Monkey * monkey) {

  g_assert( IS_MONKEY(monkey) );

  monkey_notify_game_lost(monkey);

}


static void monkey_bubble_sticked (Board * board,Bubble * bubble,gint time,Monkey * monkey) {

  g_assert( IS_MONKEY(monkey) );

  monkey_add_waiting_row(monkey);
  monkey_notify_bubbles_waiting_changed(monkey);
  PRIVATE(monkey)->last_stiked = time;
  monkey_notify_bubble_sticked( monkey,bubble );

}


static void monkey_bubbles_exploded (Board *board,
				     GList * exploded,
				     GList * fallen,
				     Monkey * monkey) {

  g_assert( IS_MONKEY(monkey) );
  g_assert( IS_BOARD(board));

  monkey_notify_bubbles_exploded(monkey,
				 exploded,
				 fallen);
  
}



Bubble ** monkey_get_current_free_columns(Monkey * m) {
  if( PRIVATE(m)->to_add != NULL ) {
    return (Bubble **)g_list_last(PRIVATE(m)->to_add)->data;
  } else return NULL;

}

int * monkey_add_bubbles( 
			 Monkey * monkey,
			 int bubbles_count,
			 Color * bubbles_colors ) {


  int * columns;
  int empty_column_count,c,i,index;
  Bubble ** bubbles;
  

  g_assert( IS_MONKEY( monkey ));
  
  bubbles = monkey_get_current_free_columns(monkey);

  
  columns = g_malloc( sizeof(int)*bubbles_count);
  
  /* count the empty columns */
  empty_column_count = 0;

  
  if( bubbles != NULL) {
    for( c = 0; c < 7; c++) {
      if( bubbles[c] == NULL) {
			 empty_column_count++;
      } 
    }
  }

  index = 0;
  while( index < bubbles_count  ) {
		
    if( empty_column_count == 0 ) {
      empty_column_count = 7;
      monkey_add_new_waiting_row(monkey);
      bubbles = monkey_get_current_free_columns(monkey);
    }

    c = rand()% empty_column_count;

    for( i = 0; i < 7; i++) {


      if( (c <= 0) && ( bubbles[i] == NULL) ) {
			 g_assert(bubbles[i] == NULL );
			 bubbles[ i ] = bubble_new( bubbles_colors[ index ],0,0 );
			 columns[index] = i;
			 index++;
			 empty_column_count--;
			 break;
      }

      if( bubbles[i] == NULL ) c--;


    }

	 
  }

  monkey_notify_bubbles_waiting_changed( monkey);
  return columns;
}

void monkey_add_bubbles_at (
			    Monkey * monkey,
			    int bubbles_count,
			    Color * bubbles_colors,
			    int * bubbles_column )  {

  Bubble ** bubbles;
  GList * last;
  int i;
  g_assert( IS_MONKEY( monkey ));

  last = g_list_last( PRIVATE(monkey)->to_add);
    
  bubbles = (Bubble **) last->data;

  for( i = 0; i < bubbles_count; i++ ) {
    if(bubbles[ bubbles_column[i]] != NULL)  
      monkey_add_new_waiting_row(monkey);
    bubbles[ bubbles_column[i] ] = bubble_new( bubbles_colors[i],0,0 );
  }

  monkey_notify_bubbles_waiting_changed( monkey);

}

static gboolean monkey_has_waiting_row(Monkey * monkey) {
 
  if( PRIVATE(monkey)->to_add == NULL ) return FALSE;
  else return TRUE;
}

static void monkey_add_new_waiting_row(Monkey * monkey) {
  Bubble ** row_to_add;
  int i;

  /* have to add a row */
  row_to_add = g_malloc( sizeof(Bubble *) * 7 );
  
  for(i = 0; i < 7; i++) {
    row_to_add[i] = NULL;
  }

  PRIVATE(monkey)->to_add = g_list_append( PRIVATE(monkey)->to_add,
														 row_to_add);

}

static void monkey_add_waiting_row(Monkey * monkey) {

  Bubble ** bubbles;

  if( monkey_has_waiting_row(monkey) ) {


    bubbles = (Bubble **)PRIVATE(monkey)->to_add->data;

    PRIVATE(monkey)->to_add =
      g_list_remove(PRIVATE(monkey)->to_add,
		    bubbles);
    
    board_add_bubbles( playground_get_board(PRIVATE(monkey)->playground),
		       bubbles );
    
    
    //  g_free(bubbles);
	 
  }

}

void monkey_set_board_down(Monkey * monkey) {
  g_assert( IS_MONKEY(monkey));

  board_down( playground_get_board( PRIVATE(monkey)->playground ));
}


void monkey_insert_bubbles(Monkey * monkey,Bubble ** bubbles_8) {
  g_assert( IS_MONKEY( monkey ) );

  board_insert_bubbles( playground_get_board( PRIVATE(monkey)->playground),
			bubbles_8);
}


gboolean monkey_is_empty(Monkey * monkey) {

	 g_assert( IS_MONKEY(monkey) );

	 return ( board_bubbles_count(playground_get_board(PRIVATE(monkey)->playground)) == 0 );
}
