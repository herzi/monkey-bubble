/* gst-audio.c
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
#include "gst-audio.h"

#define PRIVATE(gst_audio) (gst_audio->private)

static GObjectClass* parent_class = NULL;

struct GstAudioPrivate {
};



static void gst_audio_instance_init(GstAudio * gst_audio) {
    gst_audio->private =g_new0 (GstAudioPrivate, 1);			
}

static void gst_audio_finalize(GObject* object) {
    GstAudio * gst_audio = GST_AUDIO(object);

    g_free(gst_audio->private);

    if (G_OBJECT_CLASS (parent_class)->finalize) {
		  (* G_OBJECT_CLASS (parent_class)->finalize) (object);
    }
}


static void gst_audio_class_init (GstAudioClass *klass) {
    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = gst_audio_finalize;
}


GType gst_audio_get_type(void) {
    static GType gst_audio_type = 0;
    
    if (!gst_audio_type) {
		  static const GTypeInfo gst_audio_info = {
				sizeof(GstAudioClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) gst_audio_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(GstAudio),
				1,              /* n_preallocs */
				(GInstanceInitFunc) gst_audio_instance_init,
		  };


      
		  gst_audio_type = g_type_register_static(G_TYPE_OBJECT,
							  "GstAudio",
							  &gst_audio_info,
							  0);
    }
    
    return gst_audio_type;
}



GstAudio * gst_audio_new( void ) {
    GstAudio * gst_audio;
    GstAudioPrivate * dp= NULL;
  
  
    gst_audio = GST_AUDIO (g_object_new (TYPE_GST_AUDIO, NULL));

    dp = PRIVATE(gst_audio);

    return gst_audio;
}



void gst_audio_play_shoot(GstAudio * audio) {}
void gst_audio_play_rebound(GstAudio * audio) {}

void gst_audio_play_explose(GstAudio * audio) {}

void gst_audio_play_win(GstAudio * audio) {}

void gst_audio_play_lost(GstAudio * audio) {}
