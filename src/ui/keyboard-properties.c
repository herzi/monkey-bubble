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

#include <string.h>

#include <glade/glade.h>
#include <bonobo/bonobo-i18n.h>
#include <gconf/gconf-client.h>
#include "eggaccelerators.h"
#include "eggcellrendererkeys.h"
#include "keyboard-properties.h"


#define PRIVATE(main) (main->private)

static GObjectClass* parent_class = NULL;

#define KEY_NEW_1_PLAYER CONF_KEY_PREFIX"/new_1_player_game"

#define KEY_NEW_2_PLAYERS CONF_KEY_PREFIX"/new_2_players_game"
#define KEY_PAUSE_GAME CONF_KEY_PREFIX"/pause_game"
#define KEY_STOP_GAME CONF_KEY_PREFIX"/stop_game"
#define KEY_QUIT_GAME CONF_KEY_PREFIX"/quit_game"

#define KEY_FULL_SCREEN CONF_KEY_PREFIX"/full_screen"
#define KEY_ZOOM_NORMAL CONF_KEY_PREFIX"/zoom_normal"


#define KEY_PLAYER_1_AIM_LEFT CONF_KEY_PREFIX"/player_1_left"
#define KEY_PLAYER_1_AIM_RIGHT CONF_KEY_PREFIX"/player_1_right"
#define KEY_PLAYER_1_SHOOT CONF_KEY_PREFIX"/player_1_shoot"


#define KEY_PLAYER_2_AIM_LEFT CONF_KEY_PREFIX"/player_2_left"
#define KEY_PLAYER_2_AIM_RIGHT CONF_KEY_PREFIX"/player_2_right"
#define KEY_PLAYER_2_SHOOT CONF_KEY_PREFIX"/player_2_shoot"


typedef struct
{
  const char *user_visible_name;
  const char *gconf_key;
  const char *accel_path;
  /* last values received from gconf */
  guint gconf_keyval;
  GdkModifierType gconf_mask;
  GClosure *closure;
  /* have gotten a notification from gtk */
  gboolean needs_gconf_sync;
  gboolean game_key;
} KeyEntry;

typedef struct
{
  KeyEntry *key_entry;
  gint n_elements;
  gchar *user_visible_name;
} KeyEntryList;


static KeyEntry game_entries[] =
{
  { N_("New game 1 player"),
    KEY_NEW_1_PLAYER, ACCEL_PATH_NEW_1_PLAYER, 0, 0, NULL, FALSE,FALSE },
  { N_("New game 2 player"),
    KEY_NEW_2_PLAYERS, ACCEL_PATH_NEW_2_PLAYERS, 0, 0, NULL, FALSE,FALSE },
  { N_("Pause/Resume"),
    KEY_PAUSE_GAME, ACCEL_PATH_PAUSE_GAME, 0, 0, NULL, FALSE,FALSE },
  { N_("Stop game"),
    KEY_STOP_GAME, ACCEL_PATH_STOP_GAME, 0, 0, NULL, FALSE,FALSE },
  { N_("Quit game"),
    KEY_QUIT_GAME, ACCEL_PATH_QUIT_GAME, 0, 0, NULL, FALSE,FALSE }
};

static KeyEntry view_entries[] =
{
  { N_("Full Screen"),
    KEY_FULL_SCREEN, ACCEL_PATH_FULL_SCREEN, 0, 0, NULL, FALSE ,FALSE},
  { N_("Normal Size"),
    KEY_ZOOM_NORMAL, ACCEL_PATH_ZOOM_NORMAL, 0, 0, NULL, FALSE ,FALSE}
};

static KeyEntry player_1_entries[] = {
  { N_("aim left"),
    KEY_PLAYER_1_AIM_LEFT, ACCEL_PATH_PLAYER_1_AIM_LEFT, 0, 0, NULL, FALSE,TRUE },
  { N_("aim right"),
    KEY_PLAYER_1_AIM_RIGHT, ACCEL_PATH_PLAYER_1_AIM_RIGHT, 0, 0, NULL, FALSE,TRUE },
  { N_("shoot"),
    KEY_PLAYER_1_SHOOT, ACCEL_PATH_PLAYER_1_SHOOT, 0, 0, NULL, FALSE ,TRUE}
};


static KeyEntry player_2_entries[] = {
  { N_("aim left"),
    KEY_PLAYER_2_AIM_LEFT, ACCEL_PATH_PLAYER_2_AIM_LEFT, 0, 0, NULL, FALSE,TRUE },
  { N_("aim right"),
    KEY_PLAYER_2_AIM_RIGHT, ACCEL_PATH_PLAYER_2_AIM_RIGHT, 0, 0, NULL, FALSE,TRUE },
  { N_("shoot"),
    KEY_PLAYER_2_SHOOT, ACCEL_PATH_PLAYER_2_SHOOT, 0, 0, NULL, FALSE,TRUE }
};


static KeyEntryList all_entries[] =
{
  { player_1_entries, G_N_ELEMENTS (player_1_entries), N_("Player 1") },
  { player_2_entries, G_N_ELEMENTS (player_2_entries), N_("Player 2") },
  { game_entries, G_N_ELEMENTS (game_entries), N_("Game") },
  { view_entries, G_N_ELEMENTS (view_entries), N_("View") }
  
};

enum
{
  ACTION_COLUMN,
  KEYVAL_COLUMN,
  N_COLUMNS
};


struct KeyboardPropertiesPrivate {
  GtkWidget * dialog;
  GladeXML * glade_xml;
  GConfClient * gconf_client;
  GtkAccelGroup * hack_group;
  GtkTreeModel *model;
};

static void keyboard_properties_class_init(KeyboardPropertiesClass* klass);

static void keyboard_properties_init(KeyboardProperties* main);
static void keyboard_properties_finalize(GObject* object);


void
keyboard_properties_init_accel_group (KeyboardProperties * kp);
GtkWidget * edit_keys_dialog_new (KeyboardProperties * kp,GtkWindow *transient_parent);


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



KeyboardProperties * keyboard_properties_get_instance(void) {
  if( instance == NULL) {
    instance = keyboard_properties_new();
  }

  return instance;
}

void keyboard_properties_show(KeyboardProperties * kp,
				       GtkWindow * transient_parent) {

  PRIVATE(kp)->dialog = edit_keys_dialog_new( kp,transient_parent); 


  gtk_widget_show(PRIVATE(kp)->dialog);

}


GtkAccelGroup * keyboard_properties_get_accel_group(KeyboardProperties *kp) {
  return PRIVATE(kp)->hack_group;
}

static KeyboardProperties* keyboard_properties_new(void) {
    KeyboardProperties * keyboard_properties;

    keyboard_properties = KEYBOARD_PROPERTIES(g_object_new(TYPE_KEYBOARD_PROPERTIES, NULL));

    PRIVATE(keyboard_properties)->gconf_client = gconf_client_get_default ();
                                                                                


    keyboard_properties_init_accel_group ( keyboard_properties);

    return keyboard_properties;
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



/*
 * This is kind of annoying. We have two sources of keybinding change;
 * GConf and GtkAccelMap. GtkAccelMap will change if the user uses
 * the magic in-place editing mess. If accel map changes, we propagate
 * into GConf. If GConf changes we propagate into accel map.
 * To avoid infinite loop hell, we short-circuit in both directions
 * if the value is unchanged from last known.
 * The short-circuit is also required because of:
 *  http://bugzilla.gnome.org/show_bug.cgi?id=73082
 *
 *  We have to keep our own hash of the current values in order to
 *  do this short-circuit stuff.
 */

static void keys_change_notify (GConfClient *client,
                                guint        cnxn_id,
                                GConfEntry  *entry,
                                gpointer     user_data);

static void mnemonics_change_notify (GConfClient *client,
                                    guint        cnxn_id,
                                    GConfEntry  *entry,
                                    gpointer     user_data);

static void menu_accels_change_notify (GConfClient *client,
                                       guint        cnxn_id,
                                       GConfEntry  *entry,
                                       gpointer     user_data);

static void accel_changed_callback (GtkAccelGroup  *accel_group,
                                    guint           keyval,
                                    GdkModifierType modifier,
                                    GClosure       *accel_closure,
                                    gpointer        data);

static gboolean binding_from_string (const char      *str,
                                     guint           *accelerator_key,
                                     GdkModifierType *accelerator_mods,
				     gboolean game_key);

static gboolean binding_from_value  (GConfValue       *value,
                                     guint           *accelerator_key,
                                     GdkModifierType *accelerator_mods,
				     gboolean game_key);

static char*    binding_name        (guint            keyval,
                                     GdkModifierType  mask,
                                     gboolean         translate);

static void      queue_gconf_sync (KeyboardProperties * kp);

static void      update_menu_accel_state (void);

static GSList *living_treeviews = NULL;
static GSList *living_mnemonics_checkbuttons = NULL;
static GSList *living_menu_accel_checkbuttons = NULL;
static gboolean using_mnemonics = TRUE;
static gboolean using_menu_accels = TRUE;
/* never set gconf keys in response to receiving a gconf notify. */
static int inside_gconf_notify = 0;
static char *saved_menu_accel = NULL;

void
keyboard_properties_init_accel_group (KeyboardProperties * kp)
{
  GError *err;
  int i, j;
 
  
  err = NULL;
  gconf_client_add_dir (PRIVATE(kp)->gconf_client,
			CONF_KEY_PREFIX,
                        GCONF_CLIENT_PRELOAD_ONELEVEL,
                        &err);
  if (err)
    {
      g_printerr (_("There was an error loading config from %s. (%s)\n"),
                  CONF_KEY_PREFIX, err->message);
      g_error_free (err);
    }

  err = NULL;
  gconf_client_notify_add (PRIVATE(kp)->gconf_client,
                           CONF_KEY_PREFIX,
                           keys_change_notify,
                           NULL, /* user_data */
                           NULL, &err);
  
  if (err)
    {
      g_printerr (_("There was an error subscribing to notification of terminal keybinding changes. (%s)\n"),
                  err->message);
      g_error_free (err);
    }
  
  
  PRIVATE(kp)->hack_group = gtk_accel_group_new ();
  
  i = 0;
  while (i < (int) G_N_ELEMENTS (all_entries))
    {
      j = 0;

      while (j < all_entries[i].n_elements)
	{
	  char *str;
	  guint keyval;
	  GdkModifierType mask;
	  KeyEntry *key_entry;

	  key_entry = &(all_entries[i].key_entry[j]);

	  key_entry->closure = g_closure_new_simple (sizeof (GClosure), NULL);

	  g_closure_ref (key_entry->closure);
	  g_closure_sink (key_entry->closure);
	  
	  gtk_accel_group_connect_by_path (PRIVATE(kp)->hack_group,
					   key_entry->accel_path,
					   key_entry->closure);
      
	  /* Copy from gconf to GTK */
      
	  /* FIXME handle whether the entry is writable
	   *  http://bugzilla.gnome.org/show_bug.cgi?id=73207
	   */

	  err = NULL;
	  str = gconf_client_get_string (PRIVATE(kp)->gconf_client,
					 key_entry->gconf_key, &err);

	  if (err != NULL)
	    {
	      g_printerr (_("There was an error loading a terminal keybinding. (%s)\n"),
			  err->message);
	      g_error_free (err);
	    }

	  if (binding_from_string (str, &keyval, &mask,key_entry->game_key))
	    {
	      key_entry->gconf_keyval = keyval;
	      key_entry->gconf_mask = mask;
          
	      gtk_accel_map_change_entry (key_entry->accel_path,
					  keyval, mask,
					  TRUE);
	    }
	  else
	    {
	      g_printerr (_("The value of configuration key %s is not valid; value is \"%s\"\n"),
			  key_entry->gconf_key,
			  str ? str : "(null)");
	    }

	  g_free (str);
	  
	  ++j;
	}
      ++i;
    }
  
  g_signal_connect (G_OBJECT (PRIVATE(kp)->hack_group),
                    "accel_changed",
                    G_CALLBACK (accel_changed_callback),
                    kp);

  err = NULL;
  using_mnemonics = gconf_client_get_bool (PRIVATE(kp)->gconf_client,
                                           CONF_GLOBAL_PREFIX"/use_mnemonics",
                                           &err);
  if (err)
    {
      g_printerr (_("There was an error loading config value for whether to use menubar access keys. (%s)\n"),
                  err->message);
      g_error_free (err);
    }

  err = NULL;
  gconf_client_notify_add (PRIVATE(kp)->gconf_client,
                           CONF_GLOBAL_PREFIX"/use_mnemonics",
                           mnemonics_change_notify,
                           NULL, /* user_data */
                           NULL, &err);
  
  if (err)
    {
      g_printerr (_("There was an error subscribing to notification on changes on whether to use menubar access keys (%s)\n"),
                  err->message);
      g_error_free (err);
    }

  err = NULL;
  using_menu_accels = gconf_client_get_bool (PRIVATE(kp)->gconf_client,
                                             CONF_GLOBAL_PREFIX"/use_menu_accelerators",
                                             &err);
  if (err)
    {
      g_printerr (_("There was an error loading config value for whether to use menu accelerators. (%s)\n"),
                  err->message);
      g_error_free (err);
    }

  update_menu_accel_state ();
  
  err = NULL;
  gconf_client_notify_add (PRIVATE(kp)->gconf_client,
                           CONF_GLOBAL_PREFIX"/use_menu_accelerators",
                           menu_accels_change_notify,
                           NULL, /* user_data */
                           NULL, &err);
  
  if (err)
    {
      g_printerr (_("There was an error subscribing to notification for use_menu_accelerators (%s)\n"),
                  err->message);
      g_error_free (err);
    }
}


static gboolean
update_model_foreach (GtkTreeModel *model,
		      GtkTreePath  *path,
		      GtkTreeIter  *iter,
		      gpointer      data)
{
  KeyEntry *key_entry = NULL;

  gtk_tree_model_get (model, iter,
		      KEYVAL_COLUMN, &key_entry,
		      -1);

  if (key_entry == (KeyEntry *)data)
    {
      gtk_tree_model_row_changed (model, path, iter);
      return TRUE;
    }
  return FALSE;
}

static void
keys_change_notify (GConfClient *client,
                    guint        cnxn_id,
                    GConfEntry  *entry,
                    gpointer     user_data)
{
  GConfValue *val;
  GdkModifierType mask;
  guint keyval;

      int i;

  /* FIXME handle whether the entry is writable
   *  http://bugzilla.gnome.org/show_bug.cgi?id=73207
   */

  
  val = gconf_entry_get_value (entry);

      i = 0;
      while (i < (int) G_N_ELEMENTS (all_entries))
        {
	  int j;

	  j = 0;
	  while (j < all_entries[i].n_elements)
	    {
	      KeyEntry *key_entry;

	      key_entry = &(all_entries[i].key_entry[j]);
	      if (strcmp (key_entry->gconf_key, gconf_entry_get_key (entry)) == 0)
		{
		  GSList *tmp;
    
  
		  if (binding_from_value (val, &keyval, &mask,key_entry->game_key))
		    {
		  /* found it */
		  key_entry->gconf_keyval = keyval;
		  key_entry->gconf_mask = mask;

		  /* sync over to GTK */
		  inside_gconf_notify += 1;
		  gtk_accel_map_change_entry (key_entry->accel_path,
					      keyval, mask,
					      TRUE);
		  inside_gconf_notify -= 1;

		  /* Notify tree views to repaint with new values */
		  tmp = living_treeviews;
		  while (tmp != NULL)
		    {
		      gtk_tree_model_foreach (gtk_tree_view_get_model (GTK_TREE_VIEW (tmp->data)),
					      update_model_foreach,
					      key_entry);
		      tmp = tmp->next;
		    }
              
		  break;
		    }
		}
	      ++j;
	    }
	  ++i;
        }
    
}

static void
accel_changed_callback (GtkAccelGroup  *accel_group,
                        guint           keyval,
                        GdkModifierType modifier,
                        GClosure       *accel_closure,
                        gpointer        data)
{
  /* FIXME because GTK accel API is so nonsensical, we get
   * a notify for each closure, on both the added and the removed
   * accelerator. We just use the accel closure to find our
   * accel entry, then update the value of that entry.
   * We use an idle function to avoid setting the entry
   * in gconf when the accelerator gets removed and then
   * setting it again when it gets added.
   */
  int i;
  KeyboardProperties * kp;

  kp = KEYBOARD_PROPERTIES(data);

  if (inside_gconf_notify)
    {
      return;
    }

  i = 0;
  while (i < (int) G_N_ELEMENTS (all_entries))
    {
      int j;

      j = 0;
      while (j < all_entries[i].n_elements)
	{
	  KeyEntry *key_entry;

	  key_entry = &(all_entries[i].key_entry[j]);

	  if (key_entry->closure == accel_closure)
	    {
	      key_entry->needs_gconf_sync = TRUE;
	      queue_gconf_sync (kp);
	      break;
	    }
	  j++;
	}
      ++i;
    }
}

static void
mnemonics_change_notify (GConfClient *client,
                         guint        cnxn_id,
                         GConfEntry  *entry,
                         gpointer     user_data)
{
  GConfValue *val;
  
  val = gconf_entry_get_value (entry);  
  
  if (strcmp (gconf_entry_get_key (entry),
              CONF_GLOBAL_PREFIX"/use_mnemonics") == 0)
    {
      if (val && val->type == GCONF_VALUE_BOOL)
        {
          if (using_mnemonics != gconf_value_get_bool (val))
            {
              GSList *tmp;
              
              using_mnemonics = !using_mnemonics;

              /* Reset the checkbuttons */
              tmp = living_mnemonics_checkbuttons;
              while (tmp != NULL)
                {
                  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tmp->data),
                                                !using_mnemonics);
                  tmp = tmp->next;
                }
            }
        }
    }
}

static void
menu_accels_change_notify (GConfClient *client,
                           guint        cnxn_id,
                           GConfEntry  *entry,
                           gpointer     user_data)
{
  GConfValue *val;
  
  val = gconf_entry_get_value (entry);  
  
  if (strcmp (gconf_entry_get_key (entry),
              CONF_GLOBAL_PREFIX"/use_menu_accelerators") == 0)
    {
      if (val && val->type == GCONF_VALUE_BOOL)
        {
          if (using_menu_accels != gconf_value_get_bool (val))
            {
              GSList *tmp;
              
              using_menu_accels = !using_menu_accels;

              /* Reset the checkbuttons */
              tmp = living_menu_accel_checkbuttons;
              while (tmp != NULL)
                {
                  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tmp->data),
                                                !using_menu_accels);
                  tmp = tmp->next;
                }

              /* Reset the actual feature; super broken hack alert */
              update_menu_accel_state ();
            }
        }
    }
}

static gboolean
binding_from_string (const char      *str,
                     guint           *accelerator_key,
                     GdkModifierType *accelerator_mods,
		     gboolean game_key)
{
  EggVirtualModifierType virtual;
  
  g_return_val_if_fail (accelerator_key != NULL, FALSE);
  
  if (str == NULL || (str && strcmp (str, "disabled") == 0))
    {
      *accelerator_key = 0;
      *accelerator_mods = 0;
      return TRUE;
    }

  if (!egg_accelerator_parse_virtual (str, accelerator_key, &virtual)) {
    return FALSE;
  }
  egg_keymap_resolve_virtual_modifiers (gdk_keymap_get_default (),
                                        virtual,
                                        accelerator_mods);

  /* Be sure the GTK accelerator system will be able to handle this
   * accelerator. Be sure to allow no-accelerator accels like F1.
   */


  if( game_key == FALSE &&  gtk_accelerator_valid( *accelerator_key,
			     *accelerator_mods) == FALSE)
    return FALSE;
  
  if (*accelerator_key == 0)
    return FALSE;
  else
    return TRUE;
}

static gboolean
binding_from_value (GConfValue       *value,
                    guint            *accelerator_key,
                    GdkModifierType  *accelerator_mods,
		    gboolean game_key)
{
  g_return_val_if_fail (accelerator_key != NULL, FALSE);
  
  if (value == NULL)
    {
      /* unset */
      *accelerator_key = 0;
      *accelerator_mods = 0;
      return TRUE;
    }

  if (value->type != GCONF_VALUE_STRING)
    return FALSE;

  if( gtk_accelerator_valid( *accelerator_key,
			     *accelerator_mods) == FALSE) {
    return FALSE;
  }
  return binding_from_string (gconf_value_get_string (value),
                              accelerator_key,
                              accelerator_mods,game_key);
}

static char*
binding_name (guint            keyval,
              GdkModifierType  mask,
              gboolean         translate)
{
  if (keyval != 0)
    return gtk_accelerator_name (keyval, mask);
  else
    return translate ? g_strdup (_("Disabled")) : g_strdup ("disabled");
}


static guint sync_idle = 0;

static gboolean
sync_handler (gpointer data)
{

  KeyboardProperties * kp;
  int i, j;


  kp = KEYBOARD_PROPERTIES(data);
  sync_idle = 0;

  i = 0;
  while (i < (int) G_N_ELEMENTS (all_entries))
    {
      j = 0;

      while (j < all_entries[i].n_elements)
	{
	  KeyEntry *key_entry;

	  key_entry = &(all_entries[i].key_entry[j]);

	  if (key_entry->needs_gconf_sync)
	    {
	      GtkAccelKey gtk_key;
          
	      key_entry->needs_gconf_sync = FALSE;

	      gtk_key.accel_key = 0;
	      gtk_key.accel_mods = 0;
          
	      gtk_accel_map_lookup_entry (key_entry->accel_path, &gtk_key);
          
	      if (gtk_key.accel_key != key_entry->gconf_keyval ||
		  gtk_key.accel_mods != key_entry->gconf_mask)
		{
		  GError *err;
		  char *accel_name;

		  accel_name = binding_name (gtk_key.accel_key,
					     gtk_key.accel_mods,
					     FALSE);

              
		  err = NULL;
		  gconf_client_set_string (PRIVATE(kp)->gconf_client,
					   key_entry->gconf_key,
					   accel_name,
					   &err);

		  g_free (accel_name);
              
		  if (err != NULL)
		    {
		      g_printerr (_("Error propagating accelerator change to configuration database: %s\n"),
				  err->message);

		      g_error_free (err);
		    }
		}
	    }
	  ++j;
	}
      ++i;
    }  
  
  return FALSE;
}

static void
queue_gconf_sync (KeyboardProperties * kp)
{
  if (sync_idle == 0)
    sync_idle = g_idle_add (sync_handler, kp);
}

/* We have the same KeyEntry* in both columns;
 * we only have two columns because we want to be able
 * to sort by either one of them.
 */

static void
accel_set_func (GtkTreeViewColumn *tree_column,
                GtkCellRenderer   *cell,
                GtkTreeModel      *model,
                GtkTreeIter       *iter,
                gpointer           data)
{
  KeyEntry *ke;
  
  gtk_tree_model_get (model, iter,
                      KEYVAL_COLUMN, &ke,
                      -1);

  if (ke == NULL)
    g_object_set (G_OBJECT (cell),
		  "visible", FALSE,
		  NULL);
  else
    g_object_set (G_OBJECT (cell),
		  "visible", TRUE,
		  "accel_key", ke->gconf_keyval,
		  "accel_mask", ke->gconf_mask,
		  NULL);
}

int
name_compare_func (GtkTreeModel *model,
                   GtkTreeIter  *a,
                   GtkTreeIter  *b,
                   gpointer      user_data)
{
  KeyEntry *ke_a;
  KeyEntry *ke_b;
  
  gtk_tree_model_get (model, a,
                      ACTION_COLUMN, &ke_a,
                      -1);

  gtk_tree_model_get (model, b,
                      ACTION_COLUMN, &ke_b,
                      -1);

  return g_utf8_collate (_(ke_a->user_visible_name),
                         _(ke_b->user_visible_name));
}

int
accel_compare_func (GtkTreeModel *model,
                    GtkTreeIter  *a,
                    GtkTreeIter  *b,
                    gpointer      user_data)
{
  KeyEntry *ke_a;
  KeyEntry *ke_b;
  char *name_a;
  char *name_b;
  int result;
  
  gtk_tree_model_get (model, a,
                      KEYVAL_COLUMN, &ke_a,
                      -1);
  if (ke_a == NULL)
    {
      gtk_tree_model_get (model, a,
			  ACTION_COLUMN, &name_a,
			  -1);
    }
  else
    {
      name_a = binding_name (ke_a->gconf_keyval,
			     ke_a->gconf_mask,
			     TRUE);
    }

  gtk_tree_model_get (model, b,
                      KEYVAL_COLUMN, &ke_b,
                      -1);
  if (ke_b == NULL)
    {
  gtk_tree_model_get (model, b,
                      ACTION_COLUMN, &name_b,
                      -1);
    }
  else
    {
      name_b = binding_name (ke_b->gconf_keyval,
			     ke_b->gconf_mask,
			     TRUE);
    }
  
  result = g_utf8_collate (name_a, name_b);

  g_free (name_a);
  g_free (name_b);

  return result;
}

static void
remove_from_list_callback (GtkObject *object, gpointer data)
{
  GSList **listp = data;
  
  *listp = g_slist_remove (*listp, object);
}

static void
accel_edited_callback (GtkCellRendererText *cell,
                       const char          *path_string,
                       guint                keyval,
                       GdkModifierType      mask,
                       guint                hardware_keycode,
                       gpointer             data)
{
  KeyboardProperties * kp = KEYBOARD_PROPERTIES(data);

  GtkTreeModel *model = PRIVATE(kp)->model;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;
  KeyEntry *ke;
  GError *err;
  char *str;
  
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, KEYVAL_COLUMN, &ke, -1);

  /* sanity check */
  if (ke == NULL)
    return;

  str = binding_name (keyval, mask, FALSE);

  
  err = NULL;
  gconf_client_set_string (PRIVATE(kp)->gconf_client,
                           ke->gconf_key,
                           str,
                           &err);
  g_free (str);
  
  if (err != NULL)
    {
      g_printerr (_("Error setting new accelerator in configuration database: %s\n"),
                  err->message);
      
      g_error_free (err);
    }
  
  gtk_tree_path_free (path);
}

typedef struct
{
  GtkTreeView *tree_view;
  GtkTreePath *path;
} IdleData;

static gboolean
real_start_editing_cb (IdleData *idle_data)
{
  gtk_widget_grab_focus (GTK_WIDGET (idle_data->tree_view));
  gtk_tree_view_set_cursor (idle_data->tree_view,
                            idle_data->path,
			    gtk_tree_view_get_column (idle_data->tree_view, 1),
			    TRUE);

  gtk_tree_path_free (idle_data->path);
  g_free (idle_data);

  return FALSE;
}

gboolean
start_editing_cb (GtkTreeView    *tree_view,
                  GdkEventButton *event,
		  gpointer        data)
{
  GtkTreePath *path;

  if (event->window != gtk_tree_view_get_bin_window (tree_view))
    return FALSE;

  if (gtk_tree_view_get_path_at_pos (tree_view,
                                     (gint) event->x,
				     (gint) event->y,
				     &path, NULL,
				     NULL, NULL))
    {
      IdleData *idle_data;

      if (gtk_tree_path_get_depth (path) == 1)
        {
	  gtk_tree_path_free (path);
	  return FALSE;
	}

      idle_data = g_new (IdleData, 1);
      idle_data->tree_view = tree_view;
      idle_data->path = path;
      g_signal_stop_emission_by_name (G_OBJECT (tree_view), "button_press_event");
      g_idle_add ((GSourceFunc) real_start_editing_cb, idle_data);
    }

  return TRUE;
}

GtkWidget*
edit_keys_dialog_new (KeyboardProperties *kp,
		      GtkWindow *transient_parent)
{
  GladeXML *xml;
  GtkWidget *w;
  GtkCellRenderer *cell_renderer;
  int i;
  GtkTreeModel *sort_model;
  GtkTreeStore *tree;
  GtkTreeViewColumn *column;
  GtkTreeIter parent_iter;

  xml =       glade_xml_new(DATADIR"/monkey-bubble/glade/keybinding.glade",
		    "keybindings-dialog",
		    NULL);
  if (xml == NULL)
    return NULL;
  

  
  w = glade_xml_get_widget (xml, "accelerators-treeview");

  living_treeviews = g_slist_prepend (living_treeviews, w);

  g_signal_connect (G_OBJECT (w), "button_press_event",
		    G_CALLBACK (start_editing_cb), NULL);
  g_signal_connect (G_OBJECT (w), "destroy",
                    G_CALLBACK (remove_from_list_callback),
                    &living_treeviews);

  tree = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
  
  /* Column 1 */
  cell_renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("_Action"),
						     cell_renderer,
						     "text", ACTION_COLUMN,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (w), column);
  gtk_tree_view_column_set_sort_column_id (column, ACTION_COLUMN);

  /* Column 2 */
  cell_renderer = g_object_new (EGG_TYPE_CELL_RENDERER_KEYS,
				"editable", TRUE,
				"accel_mode", EGG_CELL_RENDERER_KEYS_MODE_GTK,
				NULL);
  g_signal_connect (G_OBJECT (cell_renderer), "keys_edited",
                    G_CALLBACK (accel_edited_callback),
                    kp);
  
  PRIVATE(kp)->model = GTK_TREE_MODEL( tree );

  g_object_set (G_OBJECT (cell_renderer),
                "editable", TRUE,
                NULL);
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column, _("Shortcut _Key"));
  gtk_tree_view_column_pack_start (column, cell_renderer, TRUE);
  gtk_tree_view_column_set_cell_data_func (column, cell_renderer, accel_set_func, NULL, NULL);
  gtk_tree_view_column_set_sort_column_id (column, KEYVAL_COLUMN);  
  gtk_tree_view_append_column (GTK_TREE_VIEW (w), column);

  /* Add the data */

  i = 0;
  while (i < (gint) G_N_ELEMENTS (all_entries))
    {
      int j;
      gtk_tree_store_append (tree, &parent_iter, NULL);
      gtk_tree_store_set (tree, &parent_iter,
			  ACTION_COLUMN, _(all_entries[i].user_visible_name),
			  -1);
      j = 0;

      while (j < all_entries[i].n_elements)
	{
	  GtkTreeIter iter;
	  KeyEntry *key_entry;

	  key_entry = &(all_entries[i].key_entry[j]);
	  gtk_tree_store_append (tree, &iter, &parent_iter);
	  gtk_tree_store_set (tree, &iter,
			      ACTION_COLUMN, _(key_entry->user_visible_name),
			      KEYVAL_COLUMN, key_entry,
			      -1);
	  ++j;
	}
      ++i;
    }


  sort_model = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (tree));
  gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (sort_model),
                                   KEYVAL_COLUMN, accel_compare_func,
                                   NULL, NULL);
  gtk_tree_view_set_model (GTK_TREE_VIEW (w), sort_model);

  gtk_tree_view_expand_all (GTK_TREE_VIEW (w));
  g_object_unref (G_OBJECT (tree));
  
  w = glade_xml_get_widget (xml, "keybindings-dialog");

  g_signal_connect (G_OBJECT (w), "response",
                    G_CALLBACK (gtk_widget_destroy),
                    NULL);

  gtk_window_set_default_size (GTK_WINDOW (w),
                               -1, 350);


  g_object_unref (G_OBJECT (xml));
  
  return w;
}

static void
update_menu_accel_state (void)
{
  /* Now this is a bad hack on so many levels. */
  
  if (saved_menu_accel == NULL)
    {
      g_object_get (G_OBJECT (gtk_settings_get_default ()),
                    "gtk-menu-bar-accel",
                    &saved_menu_accel,
                    NULL);
      /* FIXME if gtkrc is reparsed we don't catch on,
       * I guess.
       */
    }
  
  if (using_menu_accels)
    {
      gtk_settings_set_string_property (gtk_settings_get_default (),
                                        "gtk-menu-bar-accel",
                                        saved_menu_accel,
                                        "gnome-terminal");
    }
  else
    {
      gtk_settings_set_string_property (gtk_settings_get_default (),
                                        "gtk-menu-bar-accel",
                                        /* no one will ever press this ;-) */
                                        "<Shift><Control><Mod1><Mod2><Mod3><Mod4><Mod5>F10",
                                        "gnome-terminal");
    }
}
