/* imonkey-observer.c - 
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

#include "monkey.h"

static guint imonkey_observer_base_init_count = 0;

struct IMonkeyObserverClassPrivate {
  void (*  game_lost)(IMonkeyObserver *bo,
			Monkey * monkey);

  void (* bubbles_exploded)(IMonkeyObserver * bo,
			   Monkey * monkey,
			   GList * exploded,
			   GList * fallen);

  void (*bubble_shot)(IMonkeyObserver * bo,
			 Monkey * monkey,
			 Bubble * bubble);

  void (*bubble_sticked)(IMonkeyObserver * bo,
			 Monkey * monkey,
			 Bubble * bubble);

  void (*bubbles_waiting_changed)(IMonkeyObserver * bo,
											 Monkey * monkey,
											 int bubbles_count);


};

void imonkey_observer_class_virtual_init(IMonkeyObserverClass * i,
													  void (*  game_lost)(IMonkeyObserver *bo,
																				 Monkey * monkey),
													  void (* bubbles_exploded)(IMonkeyObserver * bo,
																						 Monkey * monkey,
																						 GList * exploded,
																						 GList * fallen),
													  void (*bubble_shot)(IMonkeyObserver * bo,
																				 Monkey * monkey,
																				 Bubble * bubble),
													  void (*bubble_sticked)(IMonkeyObserver * bo,
																					 Monkey * monkey,
																					 Bubble * bubble),
													  void (*bubbles_waiting_changed)(IMonkeyObserver * bo,
																								Monkey * monkey,
																								int bubbles_count)) {

  i->private->game_lost = game_lost;
  i->private->bubbles_exploded = bubbles_exploded;
  i->private->bubble_shot = bubble_shot;
  i->private->bubble_sticked = bubble_sticked;
  i->private->bubbles_waiting_changed = bubbles_waiting_changed;
}


static void imonkey_observer_base_init(IMonkeyObserverClass* file) {
    imonkey_observer_base_init_count++;
    file->private = g_new0(IMonkeyObserverClassPrivate,1);
    if (imonkey_observer_base_init_count == 1) {
	/* add signals here */
    }
}

static void imonkey_observer_base_finalize(IMonkeyObserverClass* file) {
    imonkey_observer_base_init_count--;
    if (imonkey_observer_base_init_count == 0) {
	/* destroy signals here */
    }
}

GType imonkey_observer_get_type(void) {

  static GType imonkey_observer_type = 0;
  if (!imonkey_observer_type) {
    static const GTypeInfo imonkey_observer_info = {
      sizeof(IMonkeyObserverClass),
      (GBaseInitFunc) imonkey_observer_base_init,
      (GBaseFinalizeFunc) imonkey_observer_base_finalize
    };
    
    imonkey_observer_type = g_type_register_static(G_TYPE_INTERFACE, 
						   "IMonkeyObserver", 
						   &imonkey_observer_info, 
						   0);

    g_type_interface_add_prerequisite(imonkey_observer_type, G_TYPE_OBJECT);
  }
  return imonkey_observer_type;
}




void imonkey_observer_game_lost(IMonkeyObserver * mo,
				  Monkey * monkey) {
  
  IMonkeyObserverClass* iface = NULL;

  g_assert(IS_IMONKEY_OBSERVER(mo));
  g_assert(G_IS_OBJECT(mo));

  iface = IMONKEY_OBSERVER_GET_IFACE(mo);
  iface->private->game_lost(mo,monkey);


}

void imonkey_observer_bubble_shot(IMonkeyObserver * mo,
				     Monkey * monkey,
				     Bubble * bubble) {
  
  IMonkeyObserverClass* iface = NULL;

  g_assert(IS_IMONKEY_OBSERVER(mo));
  g_assert(G_IS_OBJECT(mo));

  iface = IMONKEY_OBSERVER_GET_IFACE(mo);
  iface->private->bubble_shot(mo,monkey,bubble);
}


void imonkey_observer_bubble_sticked(IMonkeyObserver * mo,
				     Monkey * monkey,
				     Bubble * bubble) {
  
  IMonkeyObserverClass* iface = NULL;

  g_assert(IS_IMONKEY_OBSERVER(mo));
  g_assert(G_IS_OBJECT(mo));

  iface = IMONKEY_OBSERVER_GET_IFACE(mo);
  iface->private->bubble_sticked(mo,monkey,bubble);
}



void imonkey_observer_bubbles_exploded(IMonkeyObserver * mo,
				      Monkey * monkey,
				     GList * exploded,
				      GList * fallen) {
  
  IMonkeyObserverClass* iface = NULL;

  g_assert(IS_IMONKEY_OBSERVER(mo));
  g_assert(G_IS_OBJECT(mo));

  iface = IMONKEY_OBSERVER_GET_IFACE(mo);
  iface->private->bubbles_exploded(mo,monkey,exploded,fallen);
}


void imonkey_observer_bubbles_waiting_changed(IMonkeyObserver * mo,
															 Monkey * monkey,
															 int bubbles_count) {
  IMonkeyObserverClass* iface = NULL;

  g_assert(IS_IMONKEY_OBSERVER(mo));
  g_assert(G_IS_OBJECT(mo));

  iface = IMONKEY_OBSERVER_GET_IFACE(mo);
  iface->private->bubbles_waiting_changed(mo,monkey,bubbles_count);


}
