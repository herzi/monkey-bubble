/* iboard-observer.c - 
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

#include "board.h"

static guint iboard_observer_base_init_count = 0;

struct IBoardObserverClassPrivate {
  void (* bubbles_exploded )(IBoardObserver * bo,
			    Board *board,
			    GList * exploded,
			     GList * fallen);

  void (* bubbles_added) (IBoardObserver * po,
			  Board * board,
			  GList * bubbles);
  void (* bubble_sticked)(IBoardObserver * po,
			  Board * board,
			  Bubble * bubble,
			  gint time);
  
  void (* bubbles_inserted) (IBoardObserver * po,
			     Board * board,
			     Bubble ** bubbles,
			     int count);

  void (* down) (IBoardObserver * po,
		 Board * board);
};


void iboard_observer_class_virtual_init(IBoardObserverClass * class,
					void (* bubbles_exploded )(IBoardObserver * bo,
									  Board *board,
									  GList * exploded,
								   GList * fallen),
					void (* bubbles_added) (IBoardObserver * po,
								Board * board,
								GList * bubbles),
					void (* bubble_sticked)(IBoardObserver * po,
								Board * board,
								Bubble * bubble,
								gint time),
					void (* bubbles_inserted) (IBoardObserver * po,
								   Board * board,
								   Bubble ** bubbles,
								   int count),
					void (* down) (IBoardObserver *po,
						       Board * board)) {

  class->private->bubbles_exploded = bubbles_exploded;
  class->private->bubbles_added = bubbles_added;
  class->private->bubble_sticked = bubble_sticked;
  class->private->bubbles_inserted = bubbles_inserted;
  class->private->down = down;
}

static void iboard_observer_base_init(IBoardObserverClass* file) {
    iboard_observer_base_init_count++;
    file->private = g_new0(IBoardObserverClassPrivate,1);

    if (iboard_observer_base_init_count == 1) {

	/* add signals here */
    }
}

static void iboard_observer_base_finalize(IBoardObserverClass* file) {
    iboard_observer_base_init_count--;
    if (iboard_observer_base_init_count == 0) {
	/* destroy signals here */
    }
}

GType iboard_observer_get_type(void) {

  static GType iboard_observer_type = 0;
  if (!iboard_observer_type) {
    static const GTypeInfo iboard_observer_info = {
      sizeof(IBoardObserverClass),
      (GBaseInitFunc) iboard_observer_base_init,
      (GBaseFinalizeFunc) iboard_observer_base_finalize
    };
    
    iboard_observer_type = g_type_register_static(G_TYPE_INTERFACE, 
						   "IBoardObserver", 
						   &iboard_observer_info, 
						   0);

    g_type_interface_add_prerequisite(iboard_observer_type, G_TYPE_OBJECT);
  }
  return iboard_observer_type;
}



void iboard_observer_bubbles_exploded(IBoardObserver * bo,
				      Board * board,
				      GList * exploded,
				      GList * fallen) {
    
  IBoardObserverClass* iface = NULL;

  g_assert(IS_IBOARD_OBSERVER(bo));
  g_assert(G_IS_OBJECT(bo));

  iface = IBOARD_OBSERVER_GET_IFACE(bo);
  iface->private->bubbles_exploded(bo,board,exploded,fallen);
}


void iboard_observer_bubble_sticked(IBoardObserver * bo,
				    Board * board,
				    Bubble * bubble,
				    gint time) {
    
  IBoardObserverClass* iface = NULL;

  g_assert(IS_IBOARD_OBSERVER(bo));
  g_assert(G_IS_OBJECT(bo));

  iface = IBOARD_OBSERVER_GET_IFACE(bo);
  iface->private->bubble_sticked(bo,board,bubble,time);
}

void iboard_observer_bubbles_added(IBoardObserver * bo,
				    Board * board,
				    GList * bubbles) {
    
  IBoardObserverClass* iface = NULL;

  g_assert(IS_IBOARD_OBSERVER(bo));
  g_assert(G_IS_OBJECT(bo));

  iface = IBOARD_OBSERVER_GET_IFACE(bo);
  iface->private->bubbles_added(bo,board,bubbles);
}

void iboard_observer_down(IBoardObserver * bo,
			  Board * board) {

    
  IBoardObserverClass* iface = NULL;

  g_assert(IS_IBOARD_OBSERVER(bo));
  g_assert(G_IS_OBJECT(bo));

  iface = IBOARD_OBSERVER_GET_IFACE(bo);
  iface->private->down(bo,board);

}

void iboard_observer_bubbles_inserted(IBoardObserver * bo,
				   Board * board,
				   Bubble ** bubbles,
				      int count) {
    
  IBoardObserverClass* iface = NULL;

  g_assert(IS_IBOARD_OBSERVER(bo));
  g_assert(G_IS_OBJECT(bo));

  iface = IBOARD_OBSERVER_GET_IFACE(bo);
  iface->private->bubbles_inserted(bo,board,bubbles,count);
}
