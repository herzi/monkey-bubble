/* iclient-handler-observer.c - 
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

#include "client-handler.h"

static guint iclient_handler_observer_base_init_count = 0;

static void iclient_handler_observer_base_init(IClientHandlerObserverClass* file) {
    iclient_handler_observer_base_init_count++;
    if (iclient_handler_observer_base_init_count == 1) {
	/* add signals here */
    }
}

static void iclient_handler_observer_base_finalize(IClientHandlerObserverClass* file) {
    iclient_handler_observer_base_init_count--;
    if (iclient_handler_observer_base_init_count == 0) {
	/* destroy signals here */
    }
}

GType iclient_handler_observer_get_type(void) {

  static GType iclient_handler_observer_type = 0;
  if (!iclient_handler_observer_type) {
    static const GTypeInfo iclient_handler_observer_info = {
      sizeof(IClientHandlerObserverClass),
      (GBaseInitFunc) iclient_handler_observer_base_init,
      (GBaseFinalizeFunc) iclient_handler_observer_base_finalize
    };
    
    iclient_handler_observer_type = g_type_register_static(G_TYPE_INTERFACE, 
						   "IClientHandlerObserver", 
						   &iclient_handler_observer_info, 
						   0);

    g_type_interface_add_prerequisite(iclient_handler_observer_type, G_TYPE_OBJECT);
  }
  return iclient_handler_observer_type;
}


void iclient_handler_observer_event(IClientHandlerObserver * cho,
			      ClientHandler * ch, MonkeyMessage *m) {
  IClientHandlerObserverClass * iface;
  g_assert(IS_ICLIENT_HANDLER_OBSERVER(cho));
  g_assert(G_IS_OBJECT(cho));

  iface = ICLIENT_HANDLER_OBSERVER_GET_IFACE(cho);
  iface->event(cho,ch, m);
}
