/* ui-main.h - 
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */


#include "ui-main.h"
#include "monkey-canvas.h"
#include "monkey.h"
#include "game.h"
#include "game-1-player.h"
#include "game-1-player-manager.h"
#include "game-2-player-manager.h"
#include "game-2-player.h"
#include "keyboard-properties.h"
#include "sound-manager.h"

#include <libgnomeui/gnome-about.h>
#include <libgnome/gnome-sound.h>
#include <libgnome/gnome-help.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>
#include <bonobo/bonobo-i18n.h>

#include <string.h>
#include <stdlib.h>

#define PRIVATE(main) (main->private)

static GObjectClass* parent_class = NULL;

void ui_main_game_changed(Game * game,UiMain * ui_main);

static void new_1_player_game(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget);
static void new_2_player_game(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget);

static void pause_game(gpointer    callback_data,
		       guint       callback_action,
		       GtkWidget  *widget);
static void stop_game(gpointer    callback_data,
		      guint       callback_action,
		      GtkWidget  *widget);


static void quit_program(gpointer    callback_data,
			 guint       callback_action,
			 GtkWidget  *widget);
			      
static void about(gpointer    callback_data,
		  guint       callback_action,
		  GtkWidget  *widget);

static void show_error_dialog (GtkWindow *transient_parent,
			       const char *message_format, ...);

static void show_help_content(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget);

static void show_preferences_dialog(gpointer    callback_data,
				    guint       callback_action,
				    GtkWidget  *widget);


struct UiMainPrivate {
  GtkAccelGroup * accel_group;
  GtkItemFactory * item_factory;
  GtkWidget * menu;
  GtkWidget * status_bar;
  MonkeyCanvas * canvas;
  GtkWidget * window;
  Block * main_image;
  Game * game;  
  GameManager * manager;
  gboolean fullscreen;
  SoundManager * sm;
  GladeXML * glade_xml;
};

static void ui_main_class_init(UiMainClass* klass);
static void ui_main_init(UiMain* main);
static void ui_main_finalize(GObject* object);


static void ui_main_draw_main(UiMain * ui_main);



static UiMain* ui_main_new(void);

GType ui_main_get_type(void) {
  static GType ui_main_type = 0;

  if(!ui_main_type) {

    static const GTypeInfo ui_main_info = {
      sizeof(UiMainClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) ui_main_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(UiMain),
      1,              /* n_preallocs */
      (GInstanceInitFunc) ui_main_init,
    };
    

    ui_main_type = g_type_register_static(G_TYPE_OBJECT,
					  "UiMain",
					  &ui_main_info, 0);


  }
  return ui_main_type;
}


UiMain * ui_main_get_instance(void) {
  static UiMain * instance = NULL;

  if( !instance) {
    instance = ui_main_new();
  }

  return instance;
}


static UiMain* ui_main_new(void) {
  UiMain * ui_main;
  GtkWidget * vbox;
  GtkWidget * item;
  KeyboardProperties * kp;

  ui_main = UI_MAIN(g_object_new(UI_TYPE_MAIN, NULL));
    

  PRIVATE(ui_main)->glade_xml = glade_xml_new(DATADIR"/monkey-bubble/glade/monkey-bubble.glade","main_window",NULL);

  PRIVATE(ui_main)->window = glade_xml_get_widget( PRIVATE(ui_main)->glade_xml, "main_window");

  vbox = glade_xml_get_widget( PRIVATE(ui_main)->glade_xml,"main_vbox");


  ui_main_enabled_games_item(ui_main ,TRUE);


  PRIVATE(ui_main)->canvas =monkey_canvas_new();
  PRIVATE(ui_main)->main_image = 
    monkey_canvas_create_block_from_image(
				       PRIVATE(ui_main)->canvas,
				       DATADIR"/monkey-bubble/gfx/splash.svg",
				       640,480,
				       0,0);
     
  gtk_box_pack_end (GTK_BOX (vbox), 
		      GTK_WIDGET(PRIVATE(ui_main)->canvas), 
		     TRUE,
		     TRUE, 0);
     
  PRIVATE(ui_main)->menu = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"main_menubar");
  g_object_ref(PRIVATE(ui_main)->menu);
  kp = keyboard_properties_get_instance();

  PRIVATE(ui_main)->accel_group = gtk_accel_group_new ();
  gtk_window_add_accel_group(GTK_WINDOW(PRIVATE(ui_main)->window),
			     PRIVATE(ui_main)->accel_group);
  

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"game_menu_menu");
  gtk_menu_set_accel_group( GTK_MENU(item),
  			    PRIVATE(ui_main)->accel_group);
  
  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"new_1_player");
  g_signal_connect_swapped( item,"activate",GTK_SIGNAL_FUNC(new_1_player_game),ui_main);
  gtk_menu_item_set_accel_path( GTK_MENU_ITEM(item),
				ACCEL_PATH_NEW_1_PLAYER);

  
  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"new_2_players");
  g_signal_connect_swapped( item,"activate",GTK_SIGNAL_FUNC(new_2_player_game),ui_main);
  gtk_menu_item_set_accel_path( GTK_MENU_ITEM(item),
				ACCEL_PATH_NEW_2_PLAYERS);


  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"pause_game");
  g_signal_connect_swapped( item,"activate",GTK_SIGNAL_FUNC(pause_game),ui_main);
  gtk_menu_item_set_accel_path( GTK_MENU_ITEM(item),
				ACCEL_PATH_PAUSE_GAME);

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"stop_game");
  g_signal_connect_swapped( item,"activate",GTK_SIGNAL_FUNC(stop_game),ui_main);
  gtk_menu_item_set_accel_path( GTK_MENU_ITEM(item),
				ACCEL_PATH_STOP_GAME);

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"main_quit");
  g_signal_connect_swapped( item,"activate",GTK_SIGNAL_FUNC(quit_program),ui_main);
  gtk_menu_item_set_accel_path( GTK_MENU_ITEM(item),
				ACCEL_PATH_QUIT_GAME);

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"game_preferences");
  g_signal_connect_swapped( item,"activate",GTK_SIGNAL_FUNC(show_preferences_dialog),ui_main);

  g_signal_connect_swapped( G_OBJECT( PRIVATE(ui_main)->window),"delete-event",GTK_SIGNAL_FUNC(quit_program),NULL);

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"help_contents");
  g_signal_connect_swapped(item, "activate", 
		   GTK_SIGNAL_FUNC(show_help_content), ui_main);

  g_signal_connect_swapped (
			    glade_xml_get_widget (PRIVATE(ui_main)->glade_xml,"about"),
			    "activate",
			    G_CALLBACK (about),
			    ui_main);

  PRIVATE(ui_main)->game = NULL;

  ui_main_set_game( ui_main,NULL);
  PRIVATE(ui_main)->manager = NULL;
    
  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());
     
     
  gtk_widget_pop_visual ();
  gtk_widget_pop_colormap ();
     
     
  ui_main_draw_main(ui_main);

  PRIVATE(ui_main)->fullscreen = FALSE;

  gtk_window_set_policy (GTK_WINDOW (PRIVATE(ui_main)->window), TRUE, TRUE, FALSE);


  PRIVATE(ui_main)->sm = sound_manager_new();
  sound_manager_play_music_file(
				PRIVATE(ui_main)->sm,
				DATADIR"/monkey-bubble/sounds/splash.ogg");

  //  gtk_window_set_icon_from_file(GTK_WINDOW (PRIVATE(ui_main)->window),DATADIR"/pixmap/monkey-bubble-icon.xpm",NULL);
  return ui_main;
}

static void ui_main_draw_main(UiMain * ui_main) {

  Layer * root_layer;
  monkey_canvas_clear( PRIVATE(ui_main)->canvas);
  root_layer = monkey_canvas_get_root_layer( PRIVATE(ui_main)->canvas);


  monkey_canvas_add_block(PRIVATE(ui_main)->canvas,
		       root_layer,
		       PRIVATE(ui_main)->main_image,
		       0,0);
    
  monkey_canvas_paint( PRIVATE(ui_main)->canvas);
}

MonkeyCanvas * ui_main_get_canvas(UiMain * ui_main) {
  return PRIVATE(ui_main)->canvas;
}

GtkWidget* ui_main_get_window(UiMain * ui_main) {
  return PRIVATE(ui_main)->window;
}


static void ui_main_finalize(GObject* object) {
  //UiMain* ui_main = UI_MAIN(object);



  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}

static void ui_main_class_init (UiMainClass *klass) {
  GObjectClass* object_class;
    
  parent_class = g_type_class_peek_parent(klass);
  object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = ui_main_finalize;
}

static void ui_main_init (UiMain *ui_main) {
  ui_main->private = g_new0(UiMainPrivate, 1);
}



static void ui_main_stop_game(UiMain * ui_main);

static void ui_main_new_1_player_game(UiMain * ui_main) {

  Layer * root_layer;
  GameManager * manager;

  if( PRIVATE(ui_main)->game != NULL) {
    ui_main_stop_game(ui_main);
  }

  root_layer = monkey_canvas_get_root_layer(PRIVATE(ui_main)->canvas);

  monkey_canvas_clear(PRIVATE(ui_main)->canvas);

  manager = GAME_MANAGER(
			 game_1_player_manager_new(PRIVATE(ui_main)->window, 
						   PRIVATE(ui_main)->canvas));

  PRIVATE(ui_main)->manager = manager;
  game_manager_start(manager);


  sound_manager_play_music_file(
				PRIVATE(ui_main)->sm,
				DATADIR"/monkey-bubble/sounds/game.ogg");
}

static void ui_main_new_2_player_game(UiMain * ui_main) {

  Layer * root_layer;
  GameManager * manager;


  if( PRIVATE(ui_main)->game != NULL) {
    ui_main_stop_game(ui_main);
  }

  root_layer = monkey_canvas_get_root_layer(PRIVATE(ui_main)->canvas);

  monkey_canvas_clear(PRIVATE(ui_main)->canvas);


  manager = GAME_MANAGER(
			 game_2_player_manager_new(PRIVATE(ui_main)->window, 
						   PRIVATE(ui_main)->canvas));

  PRIVATE(ui_main)->manager = manager;
  game_manager_start(manager);


  sound_manager_play_music_file(
				PRIVATE(ui_main)->sm,
				DATADIR"/monkey-bubble/sounds/game.ogg");

}

void ui_main_enabled_games_item(UiMain * ui_main ,gboolean enabled) {
  GtkWidget * item;



  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"pause_game");
  gtk_widget_set_sensitive(item,!enabled);

  item = gtk_bin_get_child( GTK_BIN(item));
  gtk_label_set_text( GTK_LABEL(item), _("Pause game"));

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"stop_game");
  gtk_widget_set_sensitive(item,!enabled);


}

static void new_1_player_game(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget){

  UiMain * ui_main;
  ui_main = ui_main_get_instance();

  ui_main_new_1_player_game(ui_main);

}


static void new_2_player_game(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget){

  UiMain * ui_main;

  ui_main = ui_main_get_instance();

  ui_main_new_2_player_game(ui_main);



}

static void quit_program(gpointer    callback_data,
			 guint       callback_action,
			 GtkWidget  *widget){

  exit(0);
}

static void pause_game(gpointer    callback_data,
		       guint       callback_action,
		       GtkWidget  *widget) {

  UiMain * ui_main;

  ui_main = ui_main_get_instance();

  if( PRIVATE(ui_main)->game != NULL) {
    if( game_get_state( PRIVATE(ui_main)->game) == GAME_PAUSED) {
      game_pause(PRIVATE(ui_main)->game,FALSE);
    } else { 
      game_pause(PRIVATE(ui_main)->game,TRUE);
    }
  }
  
}

static void ui_main_stop_game(UiMain * ui_main) {
      
  game_manager_stop(PRIVATE(ui_main)->manager);
  g_object_unref( PRIVATE(ui_main)->manager );

  PRIVATE(ui_main)->manager = NULL;
  monkey_canvas_clear(PRIVATE(ui_main)->canvas);

  sound_manager_play_music_file(
				PRIVATE(ui_main)->sm,
				DATADIR"/monkey-bubble/sounds/splash.ogg");
  ui_main_draw_main(ui_main);
}

static void stop_game(gpointer    callback_data,
		      guint       callback_action,
		      GtkWidget  *widget) {

  UiMain * ui_main = ui_main_get_instance();
  ui_main_stop_game(ui_main);
}

Block * ui_main_get_main_image(UiMain *ui_main) {
  return(PRIVATE(ui_main)->main_image);
}

void ui_main_set_game(UiMain *ui_main, Game *game) {

  if( PRIVATE(ui_main)->game != NULL ) {

  g_signal_handlers_disconnect_matched(  G_OBJECT( PRIVATE(ui_main)->game ),
                                         G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,ui_main);
    g_object_unref(PRIVATE(ui_main)->game);

    ui_main_enabled_games_item(ui_main,TRUE);

    
  }

  PRIVATE(ui_main)->game = game;
  if( game != NULL) {
    g_object_ref(PRIVATE(ui_main)->game);

    ui_main_enabled_games_item(ui_main,FALSE);

    g_signal_connect( G_OBJECT( PRIVATE(ui_main)->game),
		      "state-changed",G_CALLBACK( ui_main_game_changed),
		      ui_main);
  }
}

void ui_main_game_changed(Game * game,
			  UiMain * ui_main) {


  GtkWidget * item;
  

  if( game_get_state(game) == GAME_PAUSED ) {


    item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"pause_game");
    
    item = gtk_bin_get_child( GTK_BIN(item));
    gtk_label_set_text( GTK_LABEL(item), _("Resume game"));

  } else if( game_get_state(game) == GAME_PLAYING ) {
  
    item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"pause_game");

    item = gtk_bin_get_child( GTK_BIN(item));
    gtk_label_set_text( GTK_LABEL(item), _("Pause game"));


		

  } else if( game_get_state(game) == GAME_STOPPED ) {
  }
  
}


static void show_preferences_dialog(gpointer    callback_data,
				    guint       callback_action,
				    GtkWidget  *widget) {

  UiMain * ui_main;

  ui_main = ui_main_get_instance();

  if( PRIVATE(ui_main)->game != NULL && game_get_state(PRIVATE(ui_main)->game) != GAME_PAUSED ) {
    game_pause(PRIVATE(ui_main)->game,TRUE);      
  }
  
  keyboard_properties_show( keyboard_properties_get_instance(),GTK_WINDOW(PRIVATE(ui_main)->window));
}

static void about (gpointer    callback_data,
		   guint       callback_action,
		   GtkWidget  *widget) {

  const gchar* authors[] = {
    "Laurent Belmonte <lolo3d@tuxfamily.org>",
    "Sven Herzberg <herzi@gnome-de.org>",
    NULL
  };
  const gchar* documenters [] = {
    "Thomas Cataldo <thomas.cataldo@aliacom.fr>",
    NULL
  };
  const gchar* translator_credits = _("translator_credits");
  GdkPixbuf	*logo = gdk_pixbuf_new_from_file(DATADIR"/monkey-bubble/gfx/monkey.png",NULL);
	
  gtk_widget_show (
		   gnome_about_new (
				    PACKAGE,
				    VERSION,
				    "Copyright (C) 2003 - Laurent Belmonte",
				    "Monkey Bubble is an Arcade Game for the GNOME Desktop Environment. Simply remove all Bubbles by the creation of unicolor triplets.",
				    authors,
				    documenters,
				    strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
				    logo)
		   );

  g_object_unref( logo);
}

static void show_help_content(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget) {
    UiMain * ui_main;
    GError *err = NULL;

    gnome_help_display ("monkey-bubble", NULL, &err);
    
    if (err) {
	ui_main = ui_main_get_instance();
	show_error_dialog (GTK_WINDOW (PRIVATE(ui_main)->window),
			   _("There was an error displaying help: %s"),
			   err->message);
	g_error_free (err);
    }
}

static void show_error_dialog (GtkWindow *transient_parent,
			       const char *message_format, ...) {
    char *message;
    va_list args;
    
    if (message_format)	{
	va_start (args, message_format);
	message = g_strdup_vprintf (message_format, args);
	va_end (args);
    } else {
	message = NULL;
    }
    
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new (transient_parent,
				     GTK_DIALOG_DESTROY_WITH_PARENT,
				     GTK_MESSAGE_ERROR,
				     GTK_BUTTONS_CLOSE,
				     message);
    
    g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (gtk_widget_destroy), NULL);
    
    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
    
    gtk_widget_show_all (dialog);
}
