/* iplayground-observer.c - 
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

static guint iplayground_observer_base_init_count = 0;

struct IPlaygroundObserverClassPrivate {
  void (* bubble_shot )(IPlaygroundObserver *po,
			Playground *pg,
			Bubble * b);
  
  void (* bubble_wall_collision )(IPlaygroundObserver * po,
				  Playground *pg);

  void (* bubble_board_collision )(IPlaygroundObserver * po,
				   Playground *pg);

  void (* game_lost)(IPlaygroundObserver * po,
		     Playground * pg);
};

void iplayground_observer_class_virtual_init(IPlaygroundObserverClass * i,
					     void (* bubble_shot )(IPlaygroundObserver *po,
								   Playground *pg,
								   Bubble * b),					     
					     void (* bubble_wall_collision )(IPlaygroundObserver * po,
									     Playground *pg),
					     void (* bubble_board_collision )(IPlaygroundObserver * po,
									      Playground *pg),
					     void (* game_lost)(IPlaygroundObserver * po,
								Playground * pg)) {
  i->private->bubble_shot = bubble_shot;
  i->private->bubble_wall_collision = bubble_wall_collision;
  i->private->bubble_board_collision = bubble_board_collision;
  i->private->game_lost = game_lost;
}
  
static void iplayground_observer_base_init(IPlaygroundObserverClass* file) {
    iplayground_observer_base_init_count++;
    file->private = g_new0(IPlaygroundObserverClassPrivate,1);
    if (iplayground_observer_base_init_count == 1) {
	/* add signals here */
    }
}

static void iplayground_observer_base_finalize(IPlaygroundObserverClass* file) {
    iplayground_observer_base_init_count--;
    if (iplayground_observer_base_init_count == 0) {
	/* destroy signals here */
    }
}

GType iplayground_observer_get_type(void) {

  static GType iplayground_observer_type = 0;

  if (!iplayground_observer_type) {
    static const GTypeInfo iplayground_observer_info = {
      sizeof(IPlaygroundObserverClass),
      (GBaseInitFunc) iplayground_observer_base_init,
      (GBaseFinalizeFunc) iplayground_observer_base_finalize
    };
    
    iplayground_observer_type = g_type_register_static(G_TYPE_INTERFACE, 
						   "IPlaygroundObserver", 
						   &iplayground_observer_info, 
						   0);

    g_type_interface_add_prerequisite(iplayground_observer_type, G_TYPE_OBJECT);

  }
  return iplayground_observer_type;
}


void iplayground_observer_bubble_shot(IPlaygroundObserver *po,
					 Playground *pg,
					 Bubble * b) {
  IPlaygroundObserverClass * iface;

  g_assert(IS_IPLAYGROUND_OBSERVER(po));

  g_assert(G_IS_OBJECT(po));


  iface = IPLAYGROUND_OBSERVER_GET_IFACE(po);

  iface->private->bubble_shot(po,pg,b);
  
}


void iplayground_observer_bubble_wall_collision(IPlaygroundObserver * po,
						Playground *pg) {
  IPlaygroundObserverClass * iface;
  g_assert(IS_IPLAYGROUND_OBSERVER(po));
  g_assert(G_IS_OBJECT(po));

  iface = IPLAYGROUND_OBSERVER_GET_IFACE(po);
  
  iface->private->bubble_wall_collision(po,pg);
}

void iplayground_observer_bubble_board_collision(IPlaygroundObserver *po,
						 Playground * pg) {
  IPlaygroundObserverClass * iface;
  g_assert(IS_IPLAYGROUND_OBSERVER(po));
  g_assert(G_IS_OBJECT(po));

  iface = IPLAYGROUND_OBSERVER_GET_IFACE(po);
  iface->private->bubble_board_collision(po,pg);

}

void iplayground_observer_game_lost(IPlaygroundObserver * po,
			       Playground * pg) {

  IPlaygroundObserverClass * iface;
  g_assert(IS_IPLAYGROUND_OBSERVER(po));
  g_assert(G_IS_OBJECT(po));

  iface = IPLAYGROUND_OBSERVER_GET_IFACE(po);
  iface->private->game_lost(po,pg);

}
