/* bubble.c
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
#include "bubble.h"

#define PRIVATE(bubble) (bubble->private)

static void bubble_notify(Bubble * bubble);

static GObjectClass* parent_class = NULL;

struct BubblePrivate {
  gdouble x,y;
  gdouble vx,vy;
  Color  color;
  guint life_time;
  GList * observer_list;

};



static void bubble_instance_init(Bubble * bubble) {
  bubble->private =g_new0 (BubblePrivate, 1);			
}

static void bubble_finalize(GObject* object) {
  Bubble * bubble = BUBBLE(object);

  if(PRIVATE(bubble)->observer_list != NULL) {
    g_error("[BUBBLE] All observer has not been removed");		
  }

  g_free(PRIVATE(bubble));

  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


static void bubble_class_init (BubbleClass *klass) {
    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = bubble_finalize;
}


GType bubble_get_type(void) {
    static GType bubble_type = 0;
    
    if (!bubble_type) {
      static const GTypeInfo bubble_info = {
	sizeof(BubbleClass),
	NULL,           /* base_init */
	NULL,           /* base_finalize */
	(GClassInitFunc) bubble_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data */
	sizeof(Bubble),
	1,              /* n_preallocs */
	(GInstanceInitFunc) bubble_instance_init,
      };


      
      bubble_type = g_type_register_static(G_TYPE_OBJECT,
						"Bubble",
						&bubble_info, 0);
    }
    
    return bubble_type;
}



Bubble * bubble_new( Color  color,gdouble x,gdouble y) {
  Bubble * bubble;

  bubble = BUBBLE (g_object_new (TYPE_BUBBLE, NULL));

  PRIVATE(bubble)->color = color;
  PRIVATE(bubble)->x = x;
  PRIVATE(bubble)->y = y;
  PRIVATE(bubble)->vx = 0;
  PRIVATE(bubble)->vy = 0;
  PRIVATE(bubble)->life_time = 0;
  PRIVATE(bubble)->observer_list = NULL;

  return bubble;
}

Color bubble_get_color(const Bubble * bubble) {
  return PRIVATE(bubble)->color;
}

void bubble_set_position(Bubble * bubble,gdouble x,gdouble y) {
  g_assert(IS_BUBBLE(bubble));
  PRIVATE(bubble)->x = x;
  PRIVATE(bubble)->y = y;
  bubble_notify(bubble);
}

void bubble_get_position(const Bubble * bubble,gdouble * x,gdouble *y) {
  g_assert(IS_BUBBLE(bubble));
  *x = PRIVATE(bubble)->x;
  *y = PRIVATE(bubble)->y;
}

void bubble_set_velocity(Bubble * b,gdouble vx,gdouble vy) {
  g_assert(IS_BUBBLE(b));
  PRIVATE(b)->vx = vx;
  PRIVATE(b)->vy = vy;
}

void bubble_get_velocity(const Bubble *b,gdouble *vx,gdouble *vy) {
  g_assert(IS_BUBBLE(b));
  *vx = PRIVATE(b)->vx;
  *vy = PRIVATE(b)->vy;
}

gboolean bubble_collide_bubble(const Bubble * bubble ,
			       const Bubble * bubble_to_collide ) {
  gdouble x,y,cx,cy;
  gdouble distance;
  
  g_assert( IS_BUBBLE( bubble ) );

  g_assert( IS_BUBBLE( bubble_to_collide ) );

  bubble_get_position( bubble , &x,&y);
  bubble_get_position( bubble_to_collide ,&cx,&cy );
  
  distance = (x-cx)*(x-cx ) + ( y-cy)*(y-cy);

  if(  distance < (32*0.87)*(32*0.87) ) {
    return TRUE;
  } else {
    return FALSE;
  }
}

void bubble_attach_observer(Bubble * bubble,IBubbleObserver * bo) {
  g_assert(IS_BUBBLE(bubble));
  g_assert(IS_IBUBBLE_OBSERVER(bo));

  PRIVATE(bubble)->observer_list = 
    g_list_append(PRIVATE(bubble)->observer_list, bo);
}

void bubble_detach_observer(Bubble * bubble,IBubbleObserver * bo) {
  g_assert(IS_BUBBLE(bubble));
  g_assert(IS_IBUBBLE_OBSERVER(bo));

  PRIVATE(bubble)->observer_list = 
    g_list_remove(PRIVATE(bubble)->observer_list,bo);
}


static void bubble_notify(Bubble * bubble) {
  GList * next;
  g_assert(IS_BUBBLE(bubble));
  
  next = PRIVATE(bubble)->observer_list;
  while( next != NULL ) {
    ibubble_observer_changed( ((IBubbleObserver *)next->data),
			      bubble);
    next = g_list_next(next);
  }
}


