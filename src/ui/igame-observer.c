/* igame-observer.c - 
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

#include "game.h"

struct IGameObserverPrivate {

  void (*  changed)(IGameObserver *bo,
		    Game * bubble);
};

void igame_observer_class_virtual_init(IGameObserverClass * class,
				   void (*  changed)(IGameObserver *bo,
						    Game * game)) {

  class->private->changed = changed;
  
}

static guint igame_observer_base_init_count = 0;

static void igame_observer_base_init(IGameObserverClass* file) {
    igame_observer_base_init_count++;
    file->private = g_new0(IGameObserverPrivate,1);
    if (igame_observer_base_init_count == 1) {
	/* add signals here */
    }
}

static void igame_observer_base_finalize(IGameObserverClass* file) {
    igame_observer_base_init_count--;
    if (igame_observer_base_init_count == 0) {
	/* destroy signals here */
    }
}

GType igame_observer_get_type(void) {

  static GType igame_observer_type = 0;
  if (!igame_observer_type) {
    static const GTypeInfo igame_observer_info = {
      sizeof(IGameObserverClass),
      (GBaseInitFunc) igame_observer_base_init,
      (GBaseFinalizeFunc) igame_observer_base_finalize
    };
    
    igame_observer_type = g_type_register_static(G_TYPE_INTERFACE, 
						   "IGameObserver", 
						   &igame_observer_info, 
						   0);

    g_type_interface_add_prerequisite(igame_observer_type, G_TYPE_OBJECT);
  }
  return igame_observer_type;
}



void igame_observer_changed(IGameObserver * bo,
				 Game * game) {

  IGameObserverClass* iface = NULL;

  g_assert(IS_IGAME_OBSERVER(bo));
  g_assert(G_IS_OBJECT(bo));

  iface = IGAME_OBSERVER_GET_IFACE(bo);
  iface->private->changed(bo,game);
}


