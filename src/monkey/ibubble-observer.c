/* ibubble-observer.c - 
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

#include "bubble.h"

struct IBubbleObserverPrivate {

  void (*  changed)(IBubbleObserver *bo,
		    Bubble * bubble);
};

void ibubble_observer_class_virtual_init(IBubbleObserverClass * class,
				   void (*  changed)(IBubbleObserver *bo,
						     Bubble * bubble)) {

  class->private->changed = changed;
  
}

static guint ibubble_observer_base_init_count = 0;

static void ibubble_observer_base_init(IBubbleObserverClass* file) {
    ibubble_observer_base_init_count++;
    file->private = g_new0(IBubbleObserverPrivate,1);
    if (ibubble_observer_base_init_count == 1) {
	/* add signals here */
    }
}

static void ibubble_observer_base_finalize(IBubbleObserverClass* file) {
    ibubble_observer_base_init_count--;
    if (ibubble_observer_base_init_count == 0) {
	/* destroy signals here */
    }
}

GType ibubble_observer_get_type(void) {

  static GType ibubble_observer_type = 0;
  if (!ibubble_observer_type) {
    static const GTypeInfo ibubble_observer_info = {
      sizeof(IBubbleObserverClass),
      (GBaseInitFunc) ibubble_observer_base_init,
      (GBaseFinalizeFunc) ibubble_observer_base_finalize
    };
    
    ibubble_observer_type = g_type_register_static(G_TYPE_INTERFACE, 
						   "IBubbleObserver", 
						   &ibubble_observer_info, 
						   0);

    g_type_interface_add_prerequisite(ibubble_observer_type, G_TYPE_OBJECT);
  }
  return ibubble_observer_type;
}



void ibubble_observer_changed(IBubbleObserver * bo,
				 Bubble * bubble) {

  IBubbleObserverClass* iface = NULL;

  g_assert(IS_IBUBBLE_OBSERVER(bo));
  g_assert(G_IS_OBJECT(bo));

  iface = IBUBBLE_OBSERVER_GET_IFACE(bo);
  iface->private->changed(bo,bubble);
}
