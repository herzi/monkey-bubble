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

#include <string.h>

#include "ui-main.h"
#include "gdk-canvas.h"
#include "monkey.h"
#include "game.h"
#include "game-1-player.h"
#include "game-1-player-manager.h"
#include "game-2-player-manager.h"
#include "game-2-player.h"
#include <stdlib.h>
#include <libgnomeui/gnome-about.h>
#include <libgnome/gnome-sound.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>
#include "keyboard-properties.h"

#include <bonobo/bonobo-i18n.h>

#define PRIVATE(main) (main->private)

static GObjectClass* parent_class = NULL;

#define MENU_COUNT 13



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

static void resume_game(gpointer    callback_data,
			guint       callback_action,
			GtkWidget  *widget);


static void quit_program(gpointer    callback_data,
			 guint       callback_action,
			 GtkWidget  *widget);
			      
static void about(gpointer    callback_data,
		  guint       callback_action,
		  GtkWidget  *widget);

static void show_preferences_dialog(gpointer    callback_data,
				    guint       callback_action,
				    GtkWidget  *widget);

static gboolean ui_main_key_pressed(GtkWidget *widget,
				    GdkEventKey *event,
				    gpointer user_data);
/*
  static GtkItemFactoryEntry entries[] = {
  { "/_Game", "<META>G",NULL, 0, "<Branch>" },
  { "/Game/New _1 player game", "<CTRL>1", new_1_player_game,1, "<Item>" },
  { "/Game/New _2 player game", "<CTRL>2", new_2_player_game,    1, "<Item>" },
  { "/Game/separtor1", NULL, NULL, 0, "<Separator>" },
  { "/Game/New _network game","<CTRL>n",new_network_game,1,"<Item>"},
  { "/Game/separtor2", NULL, NULL, 0, "<Separator>" },
  { "/Game/_Resume game","<CTRL>p",resume_game,1,"<Item>"},
  { "/Game/_Pause game","<CTRL>p",pause_game,1,"<Item>"},
  { "/Game/_Stop game","<CTRL>s",stop_game,1,"<Item>"},
  { "/Game/separtor_quit", NULL, NULL, 0, "<Separator>" },
  { "/Game/_Quit",    "<CTRL>Q", quit_program, 1, "<Item>"},
  //  { "/_Edit", NULL, NULL,         0, "<Branch>" },
  //  { "/Edit/Preferences",NULL,NULL,0, "<Item>" },
  {"/_Help", "<CTRL>H", NULL,        0 , "<Branch>" },
  {"/Help/_About", "<CTRL>H", about,        0 , "<Item>" }
  };*/

struct UiMainPrivate {
  GtkAccelGroup * accel_group;
  GtkItemFactory * item_factory;
  GtkWidget * menu;
  GtkWidget * status_bar;
  GdkCanvas * canvas;
  GtkWidget * window;
  Block * main_image;
  Game * game;  
  GameManager * manager;
  gboolean fullscreen;

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

  ui_main = UI_MAIN(g_object_new(UI_TYPE_MAIN, NULL));
    

  PRIVATE(ui_main)->glade_xml = glade_xml_new(DATADIR"/monkey-bubble/glade/monkey-bubble.glade","main_window",NULL);

  PRIVATE(ui_main)->window = glade_xml_get_widget( PRIVATE(ui_main)->glade_xml, "main_window");

  vbox = glade_xml_get_widget( PRIVATE(ui_main)->glade_xml,"main_vbox");


  ui_main_enabled_games_item(ui_main ,TRUE);


  PRIVATE(ui_main)->canvas =gdk_canvas_new();
  PRIVATE(ui_main)->main_image = 
    gdk_canvas_create_block_from_image(
				       PRIVATE(ui_main)->canvas,
				       DATADIR"/monkey-bubble/gfx/splash.svg",
				       640,480,
				       0,0);
     
  gtk_box_pack_start (GTK_BOX (vbox), 
		      GTK_WIDGET(PRIVATE(ui_main)->canvas), 
		      TRUE,
		      TRUE, 0);
     
  PRIVATE(ui_main)->menu = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"main_menubar");
  g_object_ref(PRIVATE(ui_main)->menu);



  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"new_1_player");
  g_signal_connect( item,"activate",GTK_SIGNAL_FUNC(new_1_player_game),ui_main);
  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"new_2_players");
  g_signal_connect( item,"activate",GTK_SIGNAL_FUNC(new_2_player_game),ui_main);

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"resume_game");
  g_signal_connect( item,"activate",GTK_SIGNAL_FUNC(resume_game),ui_main);

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"pause_game");
  g_signal_connect( item,"activate",GTK_SIGNAL_FUNC(pause_game),ui_main);

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"stop_game");
  g_signal_connect( item,"activate",GTK_SIGNAL_FUNC(stop_game),ui_main);

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"main_quit");
  g_signal_connect( item,"activate",GTK_SIGNAL_FUNC(quit_program),ui_main);

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"game_preferences");
  g_signal_connect( item,"activate",GTK_SIGNAL_FUNC(show_preferences_dialog),ui_main);

  g_signal_connect( G_OBJECT( PRIVATE(ui_main)->window),"delete-event",GTK_SIGNAL_FUNC(quit_program),NULL);

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

  g_signal_connect( G_OBJECT( PRIVATE(ui_main)->window) ,"key-press-event",
		    GTK_SIGNAL_FUNC (ui_main_key_pressed),ui_main);
      gtk_window_set_policy (GTK_WINDOW (PRIVATE(ui_main)->window), TRUE, TRUE, FALSE);

  //   gtk_window_set_icon_from_file(GTK_WINDOW (PRIVATE(ui_main)->window),DATADIR"/monkey-bubble/gfx/monkey-bubble-icon.xpm",NULL);
  return ui_main;
}

static void ui_main_draw_main(UiMain * ui_main) {

  Layer * root_layer;
  gdk_canvas_clear( PRIVATE(ui_main)->canvas);
  root_layer = gdk_canvas_get_root_layer( PRIVATE(ui_main)->canvas);


  gdk_canvas_add_block(PRIVATE(ui_main)->canvas,
		       root_layer,
		       PRIVATE(ui_main)->main_image,
		       0,0);
    
  gdk_canvas_paint( PRIVATE(ui_main)->canvas);
}

GdkCanvas * ui_main_get_canvas(UiMain * ui_main) {
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


static void ui_main_new_1_player_game(UiMain * ui_main) {

  Layer * root_layer;
  GameManager * manager;
  root_layer = gdk_canvas_get_root_layer(PRIVATE(ui_main)->canvas);

  gdk_canvas_clear(PRIVATE(ui_main)->canvas);

  manager = GAME_MANAGER(
			 game_1_player_manager_new(PRIVATE(ui_main)->window, 
						   PRIVATE(ui_main)->canvas));

  PRIVATE(ui_main)->manager = manager;
  game_manager_start(manager);

}

static void ui_main_new_2_player_game(UiMain * ui_main) {

  Layer * root_layer;
  GameManager * manager;
  root_layer = gdk_canvas_get_root_layer(PRIVATE(ui_main)->canvas);

  gdk_canvas_clear(PRIVATE(ui_main)->canvas);


  manager = GAME_MANAGER(
			 game_2_player_manager_new(PRIVATE(ui_main)->window, 
						   PRIVATE(ui_main)->canvas));

  PRIVATE(ui_main)->manager = manager;
  game_manager_start(manager);

}

void ui_main_enabled_games_item(UiMain * ui_main ,gboolean enabled) {
  GtkWidget * item;

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"new_1_player");
  gtk_widget_set_sensitive(item,enabled);

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"new_2_players");
  gtk_widget_set_sensitive(item,enabled);


  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"pause_game");
  gtk_widget_set_sensitive(item,!enabled);

  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"stop_game");
  gtk_widget_set_sensitive(item,!enabled);


  item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"resume_game");
  gtk_widget_set_sensitive(item,FALSE);
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

static void resume_game(gpointer    callback_data,
			guint       callback_action,
			GtkWidget  *widget) {
  UiMain * ui_main;

  ui_main = ui_main_get_instance();

  game_pause(PRIVATE(ui_main)->game,FALSE);
  
}


static void pause_game(gpointer    callback_data,
		       guint       callback_action,
		       GtkWidget  *widget) {

  UiMain * ui_main;

  ui_main = ui_main_get_instance();

  game_pause(PRIVATE(ui_main)->game,TRUE);

}

static void stop_game(gpointer    callback_data,
		      guint       callback_action,
		      GtkWidget  *widget) {

  UiMain * ui_main = ui_main_get_instance();
      
  game_manager_stop(PRIVATE(ui_main)->manager);
  g_object_unref( PRIVATE(ui_main)->manager );

  PRIVATE(ui_main)->manager = NULL;
  gdk_canvas_clear(PRIVATE(ui_main)->canvas);

  ui_main_draw_main(ui_main);
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
    gtk_widget_set_sensitive(item,FALSE);

    item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"resume_game");
    gtk_widget_set_sensitive(item,TRUE);
  } else if( game_get_state(game) == GAME_PLAYING ) {
  
    item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"pause_game");
    gtk_widget_set_sensitive(item,TRUE);

    item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"resume_game");
    gtk_widget_set_sensitive(item,FALSE);
		

  } else if( game_get_state(game) == GAME_STOPPED ) {
  }
  
}

static gboolean ui_main_key_pressed(GtkWidget *widget,
				    GdkEventKey *event,
				    gpointer user_data) {

  
  UiMain * ui_main;
  GtkWidget * item;
  GtkWidget * vbox;
  ui_main = ui_main_get_instance();
  
  switch( event->keyval ) {
  case GDK_Pause :
  case GDK_Pause+1 :
    if(  PRIVATE(ui_main)->game != NULL) {
      
      game_pause(PRIVATE(ui_main)->game,
		 !(game_get_state( PRIVATE(ui_main)->game ) == GAME_PAUSED));
    }
    break;
  case GDK_f :
    if( event->state & GDK_CONTROL_MASK ) {
      
      if(! PRIVATE(ui_main)->fullscreen)  {

	item = glade_xml_get_widget(PRIVATE(ui_main)->glade_xml,"main_menubar");
	vbox = glade_xml_get_widget( PRIVATE(ui_main)->glade_xml,"main_vbox");
	gtk_container_remove( GTK_CONTAINER( vbox), item);
	gtk_window_fullscreen( GTK_WINDOW(PRIVATE(ui_main)->window));	
      } else {
	gtk_window_unfullscreen( GTK_WINDOW(PRIVATE(ui_main)->window));
	vbox = glade_xml_get_widget( PRIVATE(ui_main)->glade_xml,"main_vbox");
	gtk_box_pack_end( GTK_BOX(vbox), PRIVATE(ui_main)->menu,TRUE,TRUE,0);

      } 
      
      PRIVATE(ui_main)->fullscreen = !PRIVATE(ui_main)->fullscreen;
      return TRUE;
    }
    break;
  }

  return FALSE;
}

static void show_preferences_dialog(gpointer    callback_data,
				    guint       callback_action,
				    GtkWidget  *widget) {

  UiMain * ui_main;

  ui_main = ui_main_get_instance();

  if( PRIVATE(ui_main)->game != NULL && game_get_state(PRIVATE(ui_main)->game) != GAME_PAUSED ) {
    game_pause(PRIVATE(ui_main)->game,TRUE);      
  }
  
  keyboard_properties_show_instance();
}

static void
about (
       gpointer    callback_data,
       guint       callback_action,
       GtkWidget  *widget)
{
  const gchar* authors[] = {
    "Laurent Belmonte <lolo3d@tuxfamily.org>",
    "Sven Herzberg <herzi@gnome-de.org>",
    NULL
  };
  const gchar* documenters [] = {
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
