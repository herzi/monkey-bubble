/* shooter.c
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
#include "shooter.h"
#include <math.h>

static GObjectClass* parent_class = NULL;
#define PRIVATE(shooter) (shooter->private)

struct ShooterPrivate {
  gdouble x_pos;
  gdouble y_pos;
  gdouble angle;
  gdouble min_angle;
  gdouble max_angle;
  gdouble shoot_speed;
  Bubble * current_bubble;
  Bubble * waiting_bubble;
};

enum {
  ROTATED,
  BUBBLE_ADDED,
  SHOOT,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void shooter_notify_rotated(Shooter * s);
static void shooter_notify_bubble_added(Shooter * s,
					Bubble * bubble);
static void shooter_notify_shoot(Shooter * s,
				 Bubble * b);


static void shooter_instance_init(Shooter * shooter) {
  shooter->private =g_new0 (ShooterPrivate, 1);			
}

static void shooter_finalize(GObject* object) {
  Shooter * shooter = SHOOTER(object);


  if( PRIVATE(shooter)->current_bubble != NULL ) {
    g_object_unref( G_OBJECT(PRIVATE(shooter)->current_bubble) );
  }

  if( PRIVATE(shooter)->waiting_bubble != NULL ) {
    g_object_unref( G_OBJECT(PRIVATE(shooter)->waiting_bubble) );
  }

  g_free(shooter->private);

  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


static void shooter_class_init (ShooterClass *klass) {
  GObjectClass* object_class;
  
  parent_class = g_type_class_peek_parent(klass);
  object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = shooter_finalize;

  signals[ROTATED] = g_signal_new("rotated",				  
				  G_TYPE_FROM_CLASS(klass),
				  G_SIGNAL_RUN_FIRST |
				  G_SIGNAL_NO_RECURSE,
				  G_STRUCT_OFFSET (ShooterClass, rotated),
				  NULL, NULL,
				  g_cclosure_marshal_VOID__VOID,
				  G_TYPE_NONE, 
				  0,
				  NULL);


  signals[BUBBLE_ADDED] = g_signal_new("bubble-added",				  
				  G_TYPE_FROM_CLASS(klass),
				  G_SIGNAL_RUN_FIRST |
				  G_SIGNAL_NO_RECURSE,
				  G_STRUCT_OFFSET (ShooterClass, bubble_added),
				  NULL, NULL,
				  g_cclosure_marshal_VOID__POINTER,
				  G_TYPE_NONE, 
				  1,
				  G_TYPE_POINTER);

  signals[SHOOT] = g_signal_new("shoot",				  
				G_TYPE_FROM_CLASS(klass),
				G_SIGNAL_RUN_FIRST |
				G_SIGNAL_NO_RECURSE,
				G_STRUCT_OFFSET (ShooterClass, shoot),
				NULL, NULL,
				g_cclosure_marshal_VOID__POINTER,
				G_TYPE_NONE, 
				1,
				G_TYPE_POINTER);
}


GType shooter_get_type(void) {
  static GType shooter_type = 0;
  
  if (!shooter_type) {
    static const GTypeInfo shooter_info = {
      sizeof(ShooterClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) shooter_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(Shooter),
      1,              /* n_preallocs */
      (GInstanceInitFunc) shooter_instance_init,
    };
    
    
    
    shooter_type = g_type_register_static(G_TYPE_OBJECT,
					  "Shooter",
					  &shooter_info, 0);
  }
  
  return shooter_type;
}



Shooter * shooter_new(gdouble x_pos,
		      gdouble y_pos,
		      gdouble min_angle,gdouble max_angle,
		      gdouble shoot_speed) {
  Shooter *s;

  s = SHOOTER( g_object_new(TYPE_SHOOTER,NULL));
  
  PRIVATE(s)->x_pos = x_pos;
  PRIVATE(s)->y_pos = y_pos;
  PRIVATE(s)->shoot_speed = shoot_speed;
  PRIVATE(s)->min_angle = min_angle;
  PRIVATE(s)->max_angle = max_angle;
  PRIVATE(s)->angle = 0;
  PRIVATE(s)->current_bubble = NULL;
  PRIVATE(s)->waiting_bubble = NULL;

  return s;
}



void shooter_add_bubble(Shooter * s,Bubble *b) {
  gdouble x,y;

  g_assert(IS_SHOOTER(s));
  g_assert(IS_BUBBLE(b));

  g_assert( PRIVATE(s)->current_bubble == NULL);

  shooter_get_position( s, &x,&y );
  
  if( PRIVATE(s)->waiting_bubble != NULL ) {
    bubble_set_position( PRIVATE(s)->waiting_bubble, x,y);
  }
  
  bubble_set_position( b, x - 55, y + 28 );
  
  PRIVATE(s)->current_bubble = PRIVATE(s)->waiting_bubble;
  PRIVATE(s)->waiting_bubble = b;

  /* fire event */
  shooter_notify_bubble_added(s,b);
  
}


void shooter_get_position(const Shooter * shooter,gdouble * x,gdouble * y) {

  g_assert(IS_SHOOTER(shooter));
  *x = PRIVATE(shooter)->x_pos;
  *y = PRIVATE(shooter)->y_pos;
}


Bubble * shooter_get_current_bubble( Shooter *shooter) {

  g_assert( IS_SHOOTER( shooter ) );
  return PRIVATE(shooter)->current_bubble;
}

Bubble * shooter_get_waiting_bubble(Shooter *shooter) {

  g_assert( IS_SHOOTER( shooter ) );

  return PRIVATE(shooter)->waiting_bubble;
}


gdouble shooter_get_angle(Shooter * s) {
  g_assert(IS_SHOOTER(s));
  return PRIVATE(s)->angle;
}


void shooter_set_angle(Shooter *s,gdouble angle) {

  g_assert(IS_SHOOTER(s));
  
  if( angle > PRIVATE(s)->max_angle ) {
    angle  = PRIVATE(s)->max_angle;
  }

  if( angle < PRIVATE(s)->min_angle ) {
    angle  = PRIVATE(s)->min_angle;
  }
  
  PRIVATE(s)->angle = angle;
  
  shooter_notify_rotated(s);
}


Bubble * shooter_shoot(Shooter * s) {
  Bubble * b;
  gdouble vx,vy;


  g_assert(IS_SHOOTER(s));
  
  g_assert( PRIVATE(s)->current_bubble != NULL ); 
  
  b = PRIVATE(s)->current_bubble;
  
  PRIVATE(s)->current_bubble = NULL;
  
  vx = - PRIVATE(s)->shoot_speed * sin( PRIVATE(s)->angle );
  vy = - PRIVATE(s)->shoot_speed * cos( PRIVATE(s)->angle );

  bubble_set_velocity( b, vx,vy);

  shooter_notify_shoot(s,b);
  return b;
}


static void shooter_notify_bubble_added(Shooter * s,
					Bubble * bubble) {

  g_signal_emit( G_OBJECT(s), signals[BUBBLE_ADDED],0,bubble);

}

static void shooter_notify_rotated(Shooter * s) {

  g_signal_emit( G_OBJECT(s), signals[ROTATED],0);
}

static void shooter_notify_shoot(Shooter * s,
				 Bubble * b) {
  g_signal_emit( G_OBJECT(s), signals[SHOOT],0,b);  
}
