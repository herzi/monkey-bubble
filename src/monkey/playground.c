/* playground.c
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
#include "playground.h"
#define PRIVATE(playground) (playground->private)


static GObjectClass* parent_class = NULL;

struct PlaygroundPrivate {
  GList * observer_list;
  Board * board;
  Bubble * played_bubble;
  gdouble max_x;
  gdouble min_x;
  gint time;
};


static void playground_instance_init(Playground * playground) {
  playground->private =g_new0 (PlaygroundPrivate, 1);			
}

static void playground_finalize(GObject* object) {
  Playground * playground = PLAYGROUND(object);

  if(playground->private->observer_list != NULL) {
    g_error("[Playground] All observer has not been removed");		
  }

  g_object_unref( G_OBJECT(PRIVATE(playground)->board) );

  if( PRIVATE(playground)->played_bubble != NULL ) {
    g_object_unref( G_OBJECT(PRIVATE(playground)->played_bubble) );
  }

  g_free(playground->private);

  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


static void playground_class_init (PlaygroundClass *klass) {
  GObjectClass* object_class;
  parent_class = g_type_class_peek_parent(klass);
  object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = playground_finalize;
}


GType playground_get_type(void) {
  static GType playground_type = 0;
    
  if (!playground_type) {
    static const GTypeInfo playground_info = {
      sizeof(PlaygroundClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) playground_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(Playground),
      1,              /* n_preallocs */
      (GInstanceInitFunc) playground_instance_init,
    };


      
    playground_type = g_type_register_static(G_TYPE_OBJECT,
					     "Playground",
					     &playground_info, 0);
  }
    
  return playground_type;
}



static void playground_notify_bubble_wall_collision(Playground * p);
static void playground_notify_lost(Playground * p);

Playground * playground_new(gdouble max_x,gdouble min_x,const gchar * level_filename,
			    gint level) {
  Playground * pg;
  pg =  PLAYGROUND( g_object_new(TYPE_PLAYGROUND,NULL));

  PRIVATE(pg)->max_x = max_x;
  PRIVATE(pg)->min_x = min_x;
  PRIVATE(pg)->board =  board_new( 40,level_filename,level );
  PRIVATE(pg)->played_bubble = NULL;
  PRIVATE(pg)->observer_list = NULL;
  PRIVATE(pg)->time = 0;
  return pg;
}


Bubble * playground_get_active_bubble(Playground * pl) {
  g_assert( IS_PLAYGROUND( pl ));
  return PRIVATE(pl)->played_bubble;
}


Board * playground_get_board(Playground * pl ) {
  g_assert( IS_PLAYGROUND( pl ));
  return PRIVATE(pl)->board;
}


/*
 * The playground is ready to receive a shoot only
 * if it hasnt an active bubble
 */
gboolean playground_is_ready_for_shoot(Playground * pl) {

  g_assert( IS_PLAYGROUND( pl ));
  return ( PRIVATE(pl)->played_bubble == NULL );
}


static void playground_notify_bubble_wall_collision(Playground * p) {
  GList * next =  NULL;

  g_assert( IS_PLAYGROUND( p ));

  next =PRIVATE(p)->observer_list;

  while( next != NULL ) {
    iplayground_observer_bubble_wall_collision( IPLAYGROUND_OBSERVER(next->data),
						p);
    next = g_list_next(next);
  }

}

static void playground_notify_lost(Playground * p) {
  GList * next =  NULL;

  g_assert( IS_PLAYGROUND( p ));

  next =PRIVATE(p)->observer_list;

  while( next != NULL ) {
    iplayground_observer_game_lost( IPLAYGROUND_OBSERVER(next->data),p);
    next = g_list_next(next);
  }

}

void playground_update(Playground * p,gint time) {
  gdouble x,y,vx,vy;
  Bubble * bubble;
  GList * next =  NULL;
  g_assert( IS_PLAYGROUND( p ));
  
  PRIVATE(p)->time += time;
  bubble = PRIVATE(p)->played_bubble;
  
  if( bubble != NULL ) {
    
    bubble_get_position(bubble,&x,&y);    
    bubble_get_velocity(bubble,&vx,&vy);
    
    x += time * vx;
    y += time * vy;
    
    if( ( x - BUBBLE_RADIUS) < PRIVATE(p)->min_x ) {
      x = 
	PRIVATE(p)->min_x +
	BUBBLE_RADIUS +
	( PRIVATE(p)->min_x -
	  ( x - BUBBLE_RADIUS ) );
      vx = - vx;
      playground_notify_bubble_wall_collision(p);
    }
    
    if( (x+BUBBLE_RADIUS) > PRIVATE(p)->max_x) {
      x =
	PRIVATE(p)->max_x - 
	BUBBLE_RADIUS +
	( PRIVATE(p)->max_x - 
	  ( x + BUBBLE_RADIUS ));
      vx = - vx;
      playground_notify_bubble_wall_collision(p);
    }
    
    bubble_set_velocity(bubble,vx,vy);
    
    bubble_set_position(bubble,x,y);
    
    
    if( board_collide_bubble( PRIVATE(p)->board, bubble ) ) {
      next =PRIVATE(p)->observer_list;
      
      while( next != NULL ) {
	iplayground_observer_bubble_board_collision(IPLAYGROUND_OBSERVER(next->data),
						    p);
	next = g_list_next(next);
      }
      
      board_stick_bubble(PRIVATE(p)->board,PRIVATE(p)->played_bubble,PRIVATE(p)->time);
      PRIVATE(p)->played_bubble = NULL;
      if( board_is_lost( PRIVATE(p)->board) ) {
	playground_notify_lost(p);
      }
    }
  }
  
}

void playground_shoot_bubble(Playground *p,Bubble * bubble) {
  GList * next =  NULL;
  
  g_assert( IS_PLAYGROUND( p ));
  
  PRIVATE(p)->played_bubble = bubble;
  
  next = PRIVATE(p)->observer_list;

  while( next != NULL ) {
    iplayground_observer_bubble_shot( (IPLAYGROUND_OBSERVER(next->data)),
				      p,bubble);
    next = g_list_next(next);
  }
}


void playground_attach_observer(Playground *p , IPlaygroundObserver * po) {
  g_assert( IS_PLAYGROUND( p ));
  g_assert( IS_IPLAYGROUND_OBSERVER( po ));

  PRIVATE(p)->observer_list = 
    g_list_append(PRIVATE(p)->observer_list,
		  po); 
}

void playground_detach_observer(Playground *p,IPlaygroundObserver * po) {
  g_assert( IS_PLAYGROUND( p ));
  g_assert( IS_IPLAYGROUND_OBSERVER( po ));

  PRIVATE(p)->observer_list = g_list_remove( PRIVATE(p)->observer_list,
					     po);
}
