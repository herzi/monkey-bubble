/* keyboard-properties.c 
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


#include "keyboard-properties.h"
#include <glade/glade.h>
#include <bonobo/bonobo-i18n.h>

#include <gconf/gconf-client.h>

#define PRIVATE(main) (main->private)

static GObjectClass* parent_class = NULL;


struct KeyboardPropertiesPrivate {
    GtkWidget * dialog;
    GladeXML * glade_xml;
    GConfClient * gconf_client;
};

static void keyboard_properties_class_init(KeyboardPropertiesClass* klass);

static void keyboard_properties_init(KeyboardProperties* main);
static void keyboard_properties_finalize(GObject* object);



static gint define_key (GtkWidget *widget, GdkEventKey *event, gpointer data);



static void on_close(GtkButton * button,
		     gpointer    callback_data);

static void on_close_dialog(GtkDialog * dialog,
			    gpointer    callback_data);

static KeyboardProperties* keyboard_properties_new(void);


GType keyboard_properties_get_type(void) {
  static GType keyboard_properties_type = 0;

  if(!keyboard_properties_type) {

    static const GTypeInfo keyboard_properties_info = {
      sizeof(KeyboardPropertiesClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) keyboard_properties_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(KeyboardProperties),
      1,              /* n_preallocs */
      (GInstanceInitFunc) keyboard_properties_init,
    };
    

    keyboard_properties_type = g_type_register_static(G_TYPE_OBJECT,
					  "KeyboardProperties",
					  &keyboard_properties_info, 0);


  }
  return keyboard_properties_type;
}


static KeyboardProperties * instance = NULL;

void keyboard_properties_show_instance(void) {

  if( !instance) {
    instance = keyboard_properties_new();
  }

  gtk_widget_show(PRIVATE(instance)->dialog);

}


static KeyboardProperties* keyboard_properties_new(void) {
    KeyboardProperties * keyboard_properties;
    GtkWidget * entry;
    gchar * str;

    keyboard_properties = KEYBOARD_PROPERTIES(g_object_new(TYPE_KEYBOARD_PROPERTIES, NULL));

    PRIVATE(keyboard_properties)->gconf_client = gconf_client_get_default ();

    gconf_client_add_dir (PRIVATE(keyboard_properties)->gconf_client, "/apps/monkey-bubble-game",
			  GCONF_CLIENT_PRELOAD_NONE, NULL);
    

    PRIVATE(keyboard_properties)->glade_xml = glade_xml_new(DATADIR"/monkey-bubble/glade/keybinding.glade",
							    "keyboard_dialog",NULL);

    PRIVATE(keyboard_properties)->dialog = glade_xml_get_widget( PRIVATE(keyboard_properties)->glade_xml,
								 "keyboard_dialog");

    entry = glade_xml_get_widget(PRIVATE(keyboard_properties)->glade_xml,"entry_1_left");

    g_signal_connect (GTK_OBJECT (entry), "key_press_event",
		      GTK_SIGNAL_FUNC (define_key), g_strdup( "/apps/monkey-bubble-game/player_1_left"));

    str = gconf_client_get_string(PRIVATE(keyboard_properties)->gconf_client,"/apps/monkey-bubble-game/player_1_left",NULL);
    if( str != NULL) {
	gtk_entry_set_text (GTK_ENTRY (entry), str);
	g_free(str);
    }


    entry = glade_xml_get_widget(PRIVATE(keyboard_properties)->glade_xml,"entry_2_left");

    g_signal_connect (GTK_OBJECT (entry), "key_press_event",
		      GTK_SIGNAL_FUNC (define_key), g_strdup("/apps/monkey-bubble-game/player_2_left"));

    str = gconf_client_get_string(PRIVATE(keyboard_properties)->gconf_client,"/apps/monkey-bubble-game/player_2_left",NULL);
    if( str != NULL) {
	gtk_entry_set_text (GTK_ENTRY (entry), str);
	g_free(str);
    }

    entry = glade_xml_get_widget(PRIVATE(keyboard_properties)->glade_xml,"entry_1_right");

    g_signal_connect (GTK_OBJECT (entry), "key_press_event",
		      GTK_SIGNAL_FUNC (define_key),g_strdup( "/apps/monkey-bubble-game/player_1_right"));

    str = gconf_client_get_string(PRIVATE(keyboard_properties)->gconf_client,"/apps/monkey-bubble-game/player_1_right",NULL);
    if( str != NULL) {
	gtk_entry_set_text (GTK_ENTRY (entry), str);
	g_free(str);
    }

    entry = glade_xml_get_widget(PRIVATE(keyboard_properties)->glade_xml,"entry_2_right");

    g_signal_connect (GTK_OBJECT (entry), "key_press_event",
		      GTK_SIGNAL_FUNC (define_key), g_strdup("/apps/monkey-bubble-game/player_2_right"));

    str = gconf_client_get_string(PRIVATE(keyboard_properties)->gconf_client,"/apps/monkey-bubble-game/player_2_right",NULL);
    if( str != NULL) {
	gtk_entry_set_text (GTK_ENTRY (entry), str);
	g_free(str);
    }


    entry = glade_xml_get_widget(PRIVATE(keyboard_properties)->glade_xml,"entry_1_shoot");

    g_signal_connect (GTK_OBJECT (entry), "key_press_event",
		      GTK_SIGNAL_FUNC (define_key),g_strdup( "/apps/monkey-bubble-game/player_1_shoot"));

    str = gconf_client_get_string(PRIVATE(keyboard_properties)->gconf_client,"/apps/monkey-bubble-game/player_1_shoot",NULL);
    if( str != NULL) {
	gtk_entry_set_text (GTK_ENTRY (entry), str);
	g_free(str);
    }


    entry = glade_xml_get_widget(PRIVATE(keyboard_properties)->glade_xml,"entry_2_shoot");

    g_signal_connect (GTK_OBJECT (entry), "key_press_event",
		      GTK_SIGNAL_FUNC (define_key), g_strdup("/apps/monkey-bubble-game/player_2_shoot"));

    str = gconf_client_get_string(PRIVATE(keyboard_properties)->gconf_client,"/apps/monkey-bubble-game/player_2_shoot",NULL);
    if( str != NULL) {
	gtk_entry_set_text (GTK_ENTRY (entry), str);
	g_free(str);
    }

    g_signal_connect(GTK_OBJECT( glade_xml_get_widget(PRIVATE(keyboard_properties)->glade_xml,"ok_button")),
				 "clicked",GTK_SIGNAL_FUNC(on_close),NULL);

    g_signal_connect(GTK_OBJECT( PRIVATE(keyboard_properties)->dialog),
		     "delete-event",
		     GTK_SIGNAL_FUNC(on_close_dialog),NULL);

    return keyboard_properties;
}




static gint define_key (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	gchar *key_name;

	
	key_name = gdk_keyval_name( event->keyval);
	gtk_entry_set_text (GTK_ENTRY (widget), key_name);
	//	gtk_widget_set_sensitive (widget, FALSE);
	
	//gtk_widget_grab_focus (control_button[(gint) data][0]);
	
	//	t_properties->wormprops[(gint) data]->up = key_name;

	//	gnibbles_properties_set_worm_up ((gint)data, key_name);

	gconf_client_set_string(PRIVATE(instance)->gconf_client,(gchar *)data,key_name,NULL);

	return TRUE;
}



static void on_close(GtkButton * button,
		     gpointer    callback_data) {

     g_object_unref( instance );

     instance = NULL;
 }

static void on_close_dialog(GtkDialog * dialog,
		     gpointer    callback_data) {

     g_object_unref( instance );

     instance = NULL;
 }

static void keyboard_properties_finalize(GObject* object) {
    KeyboardProperties* kp = KEYBOARD_PROPERTIES(object);

    gtk_widget_destroy( PRIVATE(kp)->dialog);

    g_object_unref( PRIVATE(kp)->glade_xml);
    g_object_unref( PRIVATE(kp)->gconf_client);

    g_free( kp->private);
    if (G_OBJECT_CLASS (parent_class)->finalize) {
	(* G_OBJECT_CLASS (parent_class)->finalize) (object);
    }

}

static void keyboard_properties_class_init (KeyboardPropertiesClass *klass) {
    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = keyboard_properties_finalize;
}

static void keyboard_properties_init (KeyboardProperties *keyboard_properties) {
    keyboard_properties->private = g_new0(KeyboardPropertiesPrivate, 1);
}

