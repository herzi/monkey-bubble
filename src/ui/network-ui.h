/* network-ui.h - 
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
#ifndef _NETWORK_UI_H_
#define _NETWORK_UI_H_
#include <gtk/gtk.h>

#define UI_NETWORK_TYPE                 (ui_network_get_type ())
#define UI_NETWORK(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), UI_NETWORK_TYPE, UiNetwork))
#define UI_NETWORK_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), UI_NETWORK_TYPE, UiNetworkClass))
#define UI_IS_NETWORK(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UI_NETWORK_TYPE))
#define UI_IS_NETWORK_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), UI_NETWORK_TYPE))
#define UI_NETWORK_GET_CLASS(obj)       (G_TYPE_CHECK_GET_CLASS ((obj), UI_NETWORK_TYPE, UiNetworkClass))

typedef struct UiNetworkPrivate UiNetworkPrivate;


typedef struct _UiNetwork       UiNetwork;
typedef struct _UiNetworkClass  UiNetworkClass;

struct _UiNetwork
{
  GObject parent_instance;
  UiNetworkPrivate * private;
};

struct _UiNetworkClass {
  GObjectClass        parent_class;
};


GType        ui_network_get_type          (void);
UiNetwork * ui_network_get_instance();
GtkWidget * ui_network_get_window(UiNetwork *);
void ui_network_stop(UiNetwork *);


#endif 
