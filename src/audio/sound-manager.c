/* sound-manager.c
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
#include <gtk/gtk.h>
#include "sound-manager.h"
#include "playground.h"
#include <gst/gst.h>
#define PRIVATE(sound_manager) (sound_manager->private)

static GObjectClass* parent_class = NULL;

struct SoundManagerPrivate {
  GstElement * output;
  GstElement * filesrc;
  GstElement * main_bin;
  GstElement * vorbis_dec;
  gboolean is_playing;
  gint idle_id;
};


static void sound_manager_instance_init(SoundManager * sound_manager) {
    sound_manager->private =g_new0 (SoundManagerPrivate, 1);			
}

static void sound_manager_finalize(GObject* object) {
    SoundManager * sound_manager = SOUND_MANAGER(object);

    g_free(sound_manager->private);

    if (G_OBJECT_CLASS (parent_class)->finalize) {
	(* G_OBJECT_CLASS (parent_class)->finalize) (object);
    }
}


static gboolean sound_active = TRUE;

void sound_manager_active_sound(gboolean active) {
  sound_active = active;
}

static void sound_manager_class_init (SoundManagerClass *klass) {
    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = sound_manager_finalize;
}


GType sound_manager_get_type(void) {
    static GType sound_manager_type = 0;
    
    if (!sound_manager_type) {
	static const GTypeInfo sound_manager_info = {
	    sizeof(SoundManagerClass),
	    NULL,           /* base_init */
	    NULL,           /* base_finalize */
	    (GClassInitFunc) sound_manager_class_init,
	    NULL,           /* class_finalize */
	    NULL,           /* class_data */
	    sizeof(SoundManager),
	    1,              /* n_preallocs */
	    (GInstanceInitFunc) sound_manager_instance_init,
	};



	sound_manager_type = g_type_register_static(G_TYPE_OBJECT,
						"SoundManager",
						&sound_manager_info,
						0);


      
    }
    
    return sound_manager_type;
}

static void error_handler(GstElement * e,GObject * o,gchar * string,gpointer data) {
    g_print("error : %s\n",string);

}


SoundManager * sound_manager_new( void ) {
    SoundManager * sound_manager;
    SoundManagerPrivate * dp= NULL;

    sound_manager = SOUND_MANAGER (g_object_new (TYPE_SOUND_MANAGER, NULL));

    dp = PRIVATE(sound_manager);
  
    dp->is_playing = FALSE;			       
    return sound_manager;
}


void stop_play(SoundManager * m) {

  if( sound_active) {
    PRIVATE(m)->is_playing = FALSE;

    gst_element_set_state( PRIVATE(m)->main_bin,GST_STATE_NULL);
    
    g_object_unref( G_OBJECT(PRIVATE(m)->main_bin ));
    
    PRIVATE(m)->output = NULL;
    PRIVATE(m)->main_bin = NULL;
    PRIVATE(m)->vorbis_dec = NULL;
    PRIVATE(m)->filesrc = NULL;
  }
}

void start_play(SoundManager *m, gchar * path) {
  if( sound_active) {
    PRIVATE(m)->output = gst_element_factory_make("osssink","output");
    PRIVATE(m)->main_bin = gst_thread_new("bin");
    PRIVATE(m)->vorbis_dec = gst_element_factory_make("vorbisfile","ogg_dec");
    PRIVATE(m)->filesrc = gst_element_factory_make("filesrc","filesrc");
    
    g_signal_connect(G_OBJECT(PRIVATE(m)->main_bin),
		     "error", GTK_SIGNAL_FUNC(error_handler),NULL);
    
    gst_bin_add_many( GST_BIN( PRIVATE(m)->main_bin),
		      PRIVATE(m)->output,PRIVATE(m)->vorbis_dec,
		      PRIVATE(m)->filesrc, NULL);
    
    g_object_set( G_OBJECT( PRIVATE(m)->filesrc), "location",path,NULL);
    
    gst_element_link_many( PRIVATE(m)->filesrc, PRIVATE(m)->vorbis_dec,
			   PRIVATE(m)->output,NULL);
    
    
    PRIVATE(m)->is_playing = TRUE;
    
    gst_element_set_state( PRIVATE(m)->main_bin,GST_STATE_PLAYING);
    
  }
}

void sound_manager_play_music_file(SoundManager *m, gchar * path) {

  g_assert(IS_SOUND_MANAGER(m));
  
  if( PRIVATE(m)->is_playing == TRUE) {
    stop_play(m);
  }

  start_play(m,path);
}
