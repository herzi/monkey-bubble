/* network-ui.c - 
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
 
#include "network-ui.h"
#include "network-game.h"
#include "ui-main.h"
#include "game.h"
 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#define PRIVATE(UiNetwork) (UiNetwork->private)


static void on_ready_button_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget);

static void on_cancel_button_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget);

static void on_start_game_button_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget);

static void on_client_mode_radiobutton_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget);
			      
static void on_server_mode_radiobutton_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget);

static void on_start_stop_server_checkbutton_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget);

static void on_connect_server_button_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget);			      			      			      			      			      			      
			      
static void on_quit_game_button_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget);		
			      
static void build_player_list(UiNetwork *);
static void build_main_button_box(UiNetwork *);
static void build_chooser_mode_pan(UiNetwork *);
static void build_server_mode_pan(UiNetwork *);
static void build_client_mode_pan(UiNetwork *);
static GObjectClass* parent_class = NULL;
void show_dialog (gchar *);


enum GameModeCode {
	SERVER_MODE = 100,
	CLIENT_MODE = 200
};

struct UiNetworkPrivate {
  GtkAccelGroup * accel_group;
  GtkItemFactory * item_factory;
  GtkWidget * window;
  GtkWidget * main_vbox;
  GtkWidget *server_player_spinbutton;
  GtkWidget *client_mode_frame;
  GtkWidget *server_mode_frame;  
  GtkWidget *player_name_entry;   
  GtkWidget *server_adress_entry;  
  GtkWidget *start_game_button; 
  GtkWidget *start_stop_server_checkbutton;  
  GtkWidget *ready_button;
  GtkWidget *quit_game_button;    
  gint mode;
  gboolean server_started;
  gboolean connected_to_server;
  NetworkGame *ng;
  MonkeyServer * monkey_server;
};

static void ui_network_class_init(UiNetworkClass* klass);
static void ui_network_init(UiNetwork* main);
static void ui_network_finalize(GObject* object);
static UiNetwork* ui_network_new(void);


GType ui_network_get_type(void) {
  static GType ui_network_type = 0;

  if(!ui_network_type) {

    static const GTypeInfo ui_network_info = {
      sizeof(UiNetworkClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) ui_network_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(UiNetwork),
      1,              /* n_preallocs */
      (GInstanceInitFunc) ui_network_init,
    };     
      
    ui_network_type = g_type_register_static(G_TYPE_OBJECT,
					  "UiNetwork",
					  &ui_network_info, 0);
  
	}	  
  return ui_network_type;
}

UiNetwork * ui_network_get_instance(void) {
  static UiNetwork * instance = NULL;

  if( !instance) {
    instance = ui_network_new();
  }
 
  return instance;
}

static void ui_network_finalize(GObject* object) {			

    if (G_OBJECT_CLASS (parent_class)->finalize) {
	(* G_OBJECT_CLASS (parent_class)->finalize) (object);
    }
}

static void ui_network_class_init (UiNetworkClass *klass) {
    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = ui_network_finalize;
}

static void ui_network_init (UiNetwork *ui_network) {
    ui_network->private = g_new0(UiNetworkPrivate, 1);
}

static UiNetwork* ui_network_new(void) {
    UiNetwork * ui_network;    
    GtkWidget *hseparator1;    
    GtkWidget *hseparator2;        


    ui_network = UI_NETWORK(g_object_new(UI_NETWORK_TYPE, NULL));
    PRIVATE(ui_network)->ng = NULL;
    PRIVATE(ui_network)->monkey_server = NULL;    
	
	PRIVATE(ui_network)->mode = CLIENT_MODE;
	PRIVATE(ui_network)->server_started = FALSE;	
	PRIVATE(ui_network)->connected_to_server = FALSE;
    PRIVATE(ui_network)->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (PRIVATE(ui_network)->window), "Monkey Bubble Network Game");
	gtk_window_set_modal (GTK_WINDOW (PRIVATE(ui_network)->window), TRUE);
	gtk_window_set_transient_for((GtkWindow *)PRIVATE(ui_network)->window,(GtkWindow *)ui_main_get_window(ui_main_get_instance()));
	
    PRIVATE(ui_network)->main_vbox = gtk_vbox_new(FALSE,0);

    gtk_container_add (GTK_CONTAINER (PRIVATE(ui_network)->window),
		       PRIVATE(ui_network)->main_vbox);

 	build_player_list(ui_network);
 	
	hseparator1 = gtk_hseparator_new ();
	gtk_widget_show (hseparator1);
	gtk_box_pack_start (GTK_BOX (PRIVATE(ui_network)->main_vbox), hseparator1, TRUE, TRUE, 0);
	
	build_main_button_box(ui_network);

	hseparator2 = gtk_hseparator_new ();
	gtk_widget_show (hseparator2);
	gtk_box_pack_start (GTK_BOX (PRIVATE(ui_network)->main_vbox), hseparator2, TRUE, TRUE, 0);

    build_chooser_mode_pan(ui_network);
	build_server_mode_pan(ui_network);
	build_client_mode_pan(ui_network);

    
    gtk_widget_push_visual (gdk_rgb_get_visual ());
    gtk_widget_push_colormap (gdk_rgb_get_cmap ());
        
    gtk_widget_pop_visual ();
    gtk_widget_pop_colormap ();
    return ui_network;
}
GtkWidget * ui_network_get_window(UiNetwork *uin) {
	return(PRIVATE(uin)->window);
}
static void build_player_list(UiNetwork *uin) {
	GtkWidget *player_list_scrolledwindow;
    GtkWidget *player_list_treeview;

player_list_scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (PRIVATE(uin)->main_vbox), player_list_scrolledwindow, TRUE, TRUE, 0);

  player_list_treeview = gtk_tree_view_new ();
  gtk_container_add (GTK_CONTAINER (player_list_scrolledwindow), player_list_treeview);
  gtk_widget_set_size_request (player_list_treeview, 100, 289);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (player_list_treeview), FALSE);

}
static void build_main_button_box(UiNetwork *uin) {
	GtkWidget *main_hbuttonbox;
	GtkWidget *cancel_button;	
	GtkWidget *ready_button_alignment;
	GtkWidget *ready_button_hbox;
	GtkWidget *ready_button_image;
	GtkWidget *ready_button_label;
	GtkWidget *start_game_button_alignment;
	GtkWidget *start_game_button_hbox;
	GtkWidget *start_game_button_image;
	GtkWidget *start_game_button_label;
	GtkWidget *quit_game_button_alignment;
	GtkWidget *quit_game_button_hbox;
	GtkWidget *quit_game_button_image;
	GtkWidget *quit_game_button_label;
	  
	main_hbuttonbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX (PRIVATE(uin)->main_vbox), main_hbuttonbox, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (main_hbuttonbox), 8);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (main_hbuttonbox), GTK_BUTTONBOX_SPREAD);
  gtk_box_set_spacing (GTK_BOX (main_hbuttonbox), 2);

  cancel_button = gtk_button_new_from_stock ("gtk-cancel");
  gtk_container_add (GTK_CONTAINER (main_hbuttonbox), cancel_button);
  GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);

  PRIVATE(uin)->ready_button = gtk_button_new ();
  gtk_container_add (GTK_CONTAINER (main_hbuttonbox), PRIVATE(uin)->ready_button);
  GTK_WIDGET_SET_FLAGS (PRIVATE(uin)->ready_button, GTK_CAN_DEFAULT);

  ready_button_alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_container_add (GTK_CONTAINER (PRIVATE(uin)->ready_button), ready_button_alignment);

  ready_button_hbox = gtk_hbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (ready_button_alignment), ready_button_hbox);

  ready_button_image = gtk_image_new_from_stock ("gtk-yes", GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (ready_button_hbox), ready_button_image, FALSE, FALSE, 0);

  ready_button_label = gtk_label_new_with_mnemonic ("_Ready");
  gtk_box_pack_start (GTK_BOX (ready_button_hbox), ready_button_label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (ready_button_label), GTK_JUSTIFY_LEFT);

  PRIVATE(uin)->start_game_button = gtk_button_new ();
  gtk_container_add (GTK_CONTAINER (main_hbuttonbox), PRIVATE(uin)->start_game_button);
  GTK_WIDGET_SET_FLAGS (PRIVATE(uin)->start_game_button, GTK_CAN_DEFAULT);

  start_game_button_alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_container_add (GTK_CONTAINER (PRIVATE(uin)->start_game_button), start_game_button_alignment);

  start_game_button_hbox = gtk_hbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (start_game_button_alignment), start_game_button_hbox);

  start_game_button_image = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (start_game_button_hbox), start_game_button_image, FALSE, FALSE, 0);
  
  start_game_button_label = gtk_label_new_with_mnemonic ("_Start game");
  gtk_box_pack_start (GTK_BOX (start_game_button_hbox), start_game_button_label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (start_game_button_label), GTK_JUSTIFY_LEFT);  

  PRIVATE(uin)->quit_game_button = gtk_button_new ();
   gtk_container_add (GTK_CONTAINER (main_hbuttonbox), PRIVATE(uin)->quit_game_button);
  GTK_WIDGET_SET_FLAGS (PRIVATE(uin)->quit_game_button, GTK_CAN_DEFAULT); 
  
  quit_game_button_alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_container_add (GTK_CONTAINER (PRIVATE(uin)->quit_game_button), quit_game_button_alignment);

  quit_game_button_hbox = gtk_hbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (quit_game_button_alignment), quit_game_button_hbox);

  quit_game_button_image = gtk_image_new_from_stock ("gtk-quit", GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (quit_game_button_hbox), quit_game_button_image, FALSE, FALSE, 0);
  
  quit_game_button_label = gtk_label_new_with_mnemonic ("_Quit game");
  gtk_box_pack_start (GTK_BOX (quit_game_button_hbox), quit_game_button_label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (quit_game_button_label), GTK_JUSTIFY_LEFT);

    if (!PRIVATE(uin)->connected_to_server || !PRIVATE(uin)->server_started)
  		gtk_widget_set_sensitive(PRIVATE(uin)->ready_button, FALSE);   

    
    if (PRIVATE(uin)->mode == CLIENT_MODE)
  		gtk_widget_set_sensitive(PRIVATE(uin)->start_game_button, FALSE);   

   g_signal_connect ((gpointer) 	PRIVATE(uin)->ready_button, "clicked",
                    G_CALLBACK (on_ready_button_clicked),
                    NULL);
   g_signal_connect ((gpointer) cancel_button, "clicked",
                    G_CALLBACK (on_cancel_button_clicked),
                    NULL);
   g_signal_connect ((gpointer) PRIVATE(uin)->start_game_button, "clicked",
                    G_CALLBACK (on_start_game_button_clicked),
                    NULL);      
   g_signal_connect ((gpointer) PRIVATE(uin)->quit_game_button, "clicked",
                    G_CALLBACK (on_quit_game_button_clicked),
                    NULL);      
                                                      
}
static void build_chooser_mode_pan(UiNetwork *uin) {
	  GtkWidget *chooser_mode_vbox;
  GtkWidget *chooser_mode_table;
  GtkWidget *server_mode_radiobutton;
  GSList *server_mode_radiobutton_group = NULL;
  GtkWidget *client_mode_radiobutton;
	
  chooser_mode_vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (PRIVATE(uin)->main_vbox), chooser_mode_vbox, TRUE, TRUE, 0);

  chooser_mode_table = gtk_table_new (2, 1, FALSE);
  gtk_box_pack_start (GTK_BOX (chooser_mode_vbox), chooser_mode_table, TRUE, TRUE, 0);

  server_mode_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, "Server");
  gtk_table_attach (GTK_TABLE (chooser_mode_table), server_mode_radiobutton, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (server_mode_radiobutton), server_mode_radiobutton_group);
  server_mode_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (server_mode_radiobutton));



  client_mode_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, "client");
  gtk_table_attach (GTK_TABLE (chooser_mode_table), client_mode_radiobutton, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (client_mode_radiobutton), server_mode_radiobutton_group);
  if (PRIVATE(uin)->mode == CLIENT_MODE)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (client_mode_radiobutton), TRUE);
  else gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (server_mode_radiobutton), TRUE);
	
  g_signal_connect ((gpointer) server_mode_radiobutton, "clicked",
                    G_CALLBACK (on_server_mode_radiobutton_clicked),
                    NULL);
  g_signal_connect ((gpointer) client_mode_radiobutton, "clicked",
                    G_CALLBACK (on_client_mode_radiobutton_clicked),
                    NULL);


}

static void build_server_mode_pan(UiNetwork *uin) {

	GtkWidget *server_mode_vbox;
	GtkWidget *server_mode_table;
	GtkWidget *number_players_label;
	GtkObject *server_player_spinbutton_adj;
	GtkWidget *server_mode_label;

	PRIVATE(uin)->server_mode_frame = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (PRIVATE(uin)->main_vbox), PRIVATE(uin)->server_mode_frame, TRUE, TRUE, 0);

  server_mode_vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (PRIVATE(uin)->server_mode_frame), server_mode_vbox);

  server_mode_table = gtk_table_new (1, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (server_mode_vbox), server_mode_table, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (server_mode_table), 8);
  gtk_table_set_col_spacings (GTK_TABLE (server_mode_table), 21);

  number_players_label = gtk_label_new ("Number of players");
  gtk_table_attach (GTK_TABLE (server_mode_table), number_players_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (number_players_label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (number_players_label), 0, 0.5);

  server_player_spinbutton_adj = gtk_adjustment_new (2, 0, 100, 1, 10, 10);
  PRIVATE(uin)->server_player_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (server_player_spinbutton_adj), 1, 0);
  gtk_table_attach (GTK_TABLE (server_mode_table), PRIVATE(uin)->server_player_spinbutton, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  PRIVATE(uin)->start_stop_server_checkbutton = gtk_check_button_new_with_mnemonic ("_Start/_Stop server");
  gtk_box_pack_start (GTK_BOX (server_mode_vbox), PRIVATE(uin)->start_stop_server_checkbutton, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (PRIVATE(uin)->start_stop_server_checkbutton), 8);

  server_mode_label = gtk_label_new ("Server mode");
  gtk_frame_set_label_widget (GTK_FRAME (PRIVATE(uin)->server_mode_frame), server_mode_label);
  gtk_label_set_justify (GTK_LABEL (server_mode_label), GTK_JUSTIFY_LEFT);

  if (PRIVATE(uin)->mode == CLIENT_MODE)
  		gtk_widget_set_sensitive(PRIVATE(uin)->server_mode_frame, FALSE);   

  g_signal_connect ((gpointer) PRIVATE(uin)->start_stop_server_checkbutton, "clicked",
                    G_CALLBACK (on_start_stop_server_checkbutton_clicked),
                    NULL);


}
static void build_client_mode_pan(UiNetwork *uin) {
  GtkWidget *client_mode_vbox;
  GtkWidget *client_mode_table;
  GtkWidget *player_name_label;
  GtkWidget *server_adress_label;
  GtkWidget *player_name_entry_alignment;
  GtkWidget *server_adress_entry_alignment;
  GtkWidget *client_mode_hbuttonbox;
  GtkWidget *connect_server_button;
  GtkWidget *client_mode_label;
  
	  PRIVATE(uin)->client_mode_frame = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (PRIVATE(uin)->main_vbox), PRIVATE(uin)->client_mode_frame, TRUE, TRUE, 0);

  client_mode_vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (PRIVATE(uin)->client_mode_frame), client_mode_vbox);

  client_mode_table = gtk_table_new (2, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (client_mode_vbox), client_mode_table, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (client_mode_table), 8);
  gtk_table_set_row_spacings (GTK_TABLE (client_mode_table), 12);
  gtk_table_set_col_spacings (GTK_TABLE (client_mode_table), 24);

  player_name_label = gtk_label_new ("Player name");
  gtk_table_attach (GTK_TABLE (client_mode_table), player_name_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (player_name_label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (player_name_label), 0, 0.5);

  server_adress_label = gtk_label_new ("Server adress");
  gtk_table_attach (GTK_TABLE (client_mode_table), server_adress_label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (server_adress_label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (server_adress_label), 0, 0.5);

  player_name_entry_alignment = gtk_alignment_new (0, 0.5, 0, 1);
  gtk_table_attach (GTK_TABLE (client_mode_table), player_name_entry_alignment, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  PRIVATE(uin)->player_name_entry = gtk_entry_new ();
  gtk_container_add (GTK_CONTAINER (player_name_entry_alignment), PRIVATE(uin)->player_name_entry);

  server_adress_entry_alignment = gtk_alignment_new (0, 0.5, 0, 1);
  gtk_table_attach (GTK_TABLE (client_mode_table), server_adress_entry_alignment, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  PRIVATE(uin)->server_adress_entry = gtk_entry_new ();
  gtk_container_add (GTK_CONTAINER (server_adress_entry_alignment), PRIVATE(uin)->server_adress_entry);

  client_mode_hbuttonbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX (client_mode_vbox), client_mode_hbuttonbox, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (client_mode_hbuttonbox), 8);

  connect_server_button = gtk_button_new_with_mnemonic ("Connect server");
  gtk_container_add (GTK_CONTAINER (client_mode_hbuttonbox), connect_server_button);
  GTK_WIDGET_SET_FLAGS (connect_server_button, GTK_CAN_DEFAULT);

  client_mode_label = gtk_label_new ("Client mode");
  gtk_frame_set_label_widget (GTK_FRAME (PRIVATE(uin)->client_mode_frame), client_mode_label);
  gtk_label_set_justify (GTK_LABEL (client_mode_label), GTK_JUSTIFY_LEFT);

  if (PRIVATE(uin)->mode == SERVER_MODE)
  		gtk_widget_set_sensitive(PRIVATE(uin)->client_mode_frame, FALSE);   
  

  g_signal_connect ((gpointer) connect_server_button, "clicked",
                    G_CALLBACK (on_connect_server_button_clicked),
                    NULL);
  
}

static void on_ready_button_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget) {
		
		UiNetwork *uin = ui_network_get_instance();
		UiMain *ui_main = ui_main_get_instance();  		

		MonkeyMessage *mm = (MonkeyMessage *) g_malloc(sizeof(struct _monkeyMessage));		      	      	
		
		
  		gtk_widget_set_sensitive(PRIVATE(uin)->ready_button, FALSE); 		
		mm->message = PLAYER_READY;
		network_game_send_message(PRIVATE(uin)->ng , mm);  														
  		
		if (!PRIVATE(uin)->server_started) {
			gtk_widget_hide_all(PRIVATE(uin)->window);
			
			gdk_canvas_clear(ui_main_get_canvas(ui_main));															
			
			network_game_draw_foreign_monkeys(PRIVATE(uin)->ng);  						
  			gdk_canvas_paint(ui_main_get_canvas(ui_main));			
		}
}

static void on_cancel_button_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget) {
			      	
	UiNetwork *uin = ui_network_get_instance(); 
	UiMain *ui_main = ui_main_get_instance();
	ui_main_enabled_games_item(ui_main ,TRUE);
	
	if (PRIVATE(uin)->server_started == TRUE) {
		g_object_unref(PRIVATE(uin)->monkey_server);
		PRIVATE(uin)->server_started = FALSE;	
		gtk_toggle_button_set_active ((GtkToggleButton *) PRIVATE(uin)->start_stop_server_checkbutton, FALSE);
	}
	
	if (PRIVATE(uin)->connected_to_server == TRUE)
		network_game_abort(PRIVATE(uin)->ng);	
	
	gtk_widget_set_sensitive (PRIVATE(uin)->ready_button, FALSE);
	PRIVATE(uin)->connected_to_server = FALSE;


}

static void on_start_game_button_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget) {
			      	
		UiNetwork *uin = ui_network_get_instance();
		UiMain *ui_main = ui_main_get_instance();  		

		MonkeyMessage *mm = (MonkeyMessage *) g_malloc(sizeof(struct _monkeyMessage));
		     			
		gtk_widget_hide_all(PRIVATE(uin)->window);
	
		gdk_canvas_clear(ui_main_get_canvas(ui_main));															
														
  		mm->message = START_GAME;
		network_game_send_message(PRIVATE(uin)->ng, mm);													  		

		g_free(mm);
		
		network_game_draw_foreign_monkeys(PRIVATE(uin)->ng);
		gdk_canvas_paint(ui_main_get_canvas(ui_main));		
			
}

static void on_client_mode_radiobutton_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget) {
	UiNetwork *uin = ui_network_get_instance();	      	
	if (PRIVATE(uin)->mode != CLIENT_MODE) {
		PRIVATE(uin)->mode = CLIENT_MODE;			
  		gtk_widget_set_sensitive(PRIVATE(uin)->server_mode_frame, FALSE);   
  		gtk_widget_set_sensitive(PRIVATE(uin)->client_mode_frame, TRUE);     		
		gtk_widget_set_sensitive(PRIVATE(uin)->start_game_button, FALSE);  
		if (!PRIVATE(uin)->connected_to_server)
			gtk_widget_set_sensitive(PRIVATE(uin)->ready_button, FALSE); 
	}
	
}
			      
static void on_server_mode_radiobutton_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget) {
	UiNetwork *uin = ui_network_get_instance();		      	
	if (PRIVATE(uin)->mode != SERVER_MODE) {
		PRIVATE(uin)->mode = SERVER_MODE;				      	
  		gtk_widget_set_sensitive(PRIVATE(uin)->server_mode_frame, TRUE);   
  		gtk_widget_set_sensitive(PRIVATE(uin)->client_mode_frame, TRUE);     		  		
	}
}

static void on_start_stop_server_checkbutton_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget) {

	UiNetwork *uin = ui_network_get_instance();
	GError **err = NULL;
	GThread *thr = NULL;
	
	if (!PRIVATE(uin)->server_started) {
		if ((PRIVATE(uin)->monkey_server = (MonkeyServer *) monkey_server_new()) == NULL) {
    		show_dialog("Unable to create server\n");
    		gtk_toggle_button_set_active((GtkToggleButton *) PRIVATE(uin)->start_stop_server_checkbutton, FALSE);
    		return;		
    	}		   
	
	    if (!monkey_server_init(PRIVATE(uin)->monkey_server,(unsigned short) MONKEY_PORT)) {
	    	show_dialog("Unable to initialize server\n");                                                          
    		gtk_toggle_button_set_active((GtkToggleButton *) PRIVATE(uin)->start_stop_server_checkbutton, FALSE);	    	     	
	    	return;
    	}    
		thr = g_thread_create((GThreadFunc) monkey_server_start, PRIVATE(uin)->monkey_server, TRUE, err);
			      				      	
		PRIVATE(uin)->server_started = TRUE;
		show_dialog("Monkey Server started");
	}
	else {
		g_object_unref(PRIVATE(uin)->monkey_server);
		PRIVATE(uin)->server_started = FALSE;
	}
}
static void on_quit_game_button_clicked(gpointer    callback_data,
															      guint       callback_action, 
															      GtkWidget  *widget) {
	UiNetwork *uin = ui_network_get_instance();
	UiMain *ui_main = ui_main_get_instance();
	ui_main_enabled_games_item(ui_main ,TRUE);
	
	on_cancel_button_clicked(NULL,(guint) NULL,NULL);
	
	if (G_IS_OBJECT(PRIVATE(uin)->ng))
		g_object_unref(PRIVATE(uin)->ng);	
														      	
	gtk_widget_hide_all(PRIVATE(uin)->window);		
}
void show_dialog (gchar *text) {
	
	UiNetwork *uin = ui_network_get_instance();
	
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(PRIVATE(uin)->window),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  text);

 gtk_dialog_run (GTK_DIALOG (dialog));
 gtk_widget_destroy (dialog);
}

static void on_connect_server_button_clicked(gpointer    callback_data,
			      guint       callback_action,
			      GtkWidget  *widget) {
	UiNetwork *uin =  ui_network_get_instance();
	UiMain *ui_main = ui_main_get_instance();  		
	Game *game = NULL;	  				      	
	
	 if ((!PRIVATE(uin)->connected_to_server) &&
	    (gtk_entry_get_text((GtkEntry *) PRIVATE(uin)->server_adress_entry) != NULL) &&
	    (gtk_entry_get_text((GtkEntry *) PRIVATE(uin)->player_name_entry) != NULL)) {	 		 	

  		ui_main_set_game(ui_main, game = GAME(network_game_new( ui_main_get_window(ui_main), ui_main_get_canvas(ui_main),
																				  g_strdup( gtk_entry_get_text((GtkEntry *) PRIVATE(uin)->server_adress_entry)),
																				  (unsigned short) MONKEY_PORT)));
																					
		 if (game != NULL) {
			PRIVATE(uin)->ng = NETWORK_GAME(game);	  																		  		 
			PRIVATE(uin)->connected_to_server = TRUE;
			gtk_widget_set_sensitive(PRIVATE(uin)->ready_button, TRUE);
			
			if (PRIVATE(uin)->mode == SERVER_MODE) {
				gtk_widget_set_sensitive(PRIVATE(uin)->start_game_button, TRUE);		 
			}
			
		 } else {
		 	show_dialog("Unable to connect to server");
		 }
	 }
}

void ui_network_stop(UiNetwork *uin) {

   gtk_widget_set_sensitive(PRIVATE(uin)->ready_button, FALSE); 
	gtk_widget_set_sensitive(PRIVATE(uin)->start_game_button, FALSE);
	PRIVATE(uin)->connected_to_server = FALSE;
	gtk_widget_hide_all(PRIVATE(uin)->window);
}
