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
#include <gst/gst.h>

#define N_CHANNEL 1
#define PRIVATE(gst_audio) (gst_audio->private)

static GObjectClass* parent_class = NULL;

struct GstAudioPrivate {
    GstElement * main_bin;
    GstElement * adder;
    GstElement * output;
    GQueue * free_channel;
  GList * all_channel;
    GList * used_channel;
    GstElement * t_filesrc;
    GstElement * t_bin;
    GstElement * t_decoder;
  gboolean audio_lock;
  gboolean pause;
};

typedef struct GstChannel {
    GstElement * pipe;
    GstElement * filesrc;
    GstElement * decoder;
    int id;
    gchar * location;
    
} GstChannel;

void        eos     (GstThread *gstthread,
		     gpointer user_data) {
    g_print("end \n");
}

static GstChannel * gst_audio_create_channel(GstAudio * audio, int channel_id) {
    char buffer[200];

    GstChannel * channel;

    channel = (GstChannel *) g_malloc( sizeof(GstChannel));

    channel->id = channel_id;

    sprintf(buffer,"pipeline%d",channel->id);
    channel->pipe = gst_element_factory_make("bin",buffer);


    sprintf(buffer,"mad%d",channel->id);
    channel->decoder = gst_element_factory_make("mad",buffer);
    
    sprintf(buffer,"filesrc%d",channel->id);
    channel->filesrc =  gst_element_factory_make("filesrc",buffer);
    
    gst_bin_add( GST_BIN(channel->pipe), channel->filesrc);

    gst_bin_add( GST_BIN(channel->pipe), channel->decoder);
    
    //    gst_bin_add( GST_BIN( PRIVATE(audio)->main_bin), channel->pipe);
	
    
    
    return channel;
}

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


gboolean gst_audio_idle(gpointer user_data) {
    GstAudio * audio;

    audio = GST_AUDIO(user_data);
    g_print("ilde\n");
      return gst_bin_iterate( GST_BIN( PRIVATE(audio)->main_bin));
 }

static void gst_audio_reset_channel(gpointer data,gpointer user_data) {
    GstChannel * c;

    c= (GstChannel *)data;
    gst_element_set_state( c->pipe,GST_STATE_NULL);
    //    gst_element_set_state( c->filesrc,GST_STATE_NULL);

}

static void error_handler(GstElement * e,GObject * o,gchar * string,gpointer data) {
    g_print("error : %s\n",string);
}

GstAudio * gst_audio_new( void ) {
    GstAudio * gst_audio;
    GstAudioPrivate * dp= NULL;
     GstChannel * channel;
    //   GstPad * pad;
     //GstElement * filesrc, * decoder, * test_bin;
        int i;

    gst_audio = GST_AUDIO (g_object_new (TYPE_GST_AUDIO, NULL));

    dp = PRIVATE(gst_audio);

    dp->adder = gst_element_factory_make("adder","mixer");
    dp->output = gst_element_factory_make("osssink","output");
    dp->main_bin = gst_pipeline_new("bin");
    
    g_signal_connect(G_OBJECT(dp->main_bin),"error", GTK_SIGNAL_FUNC(error_handler),NULL);
    gst_bin_add( GST_BIN( dp->main_bin),dp->adder);
    gst_bin_add( GST_BIN( dp->main_bin),dp->output);


    dp->used_channel = NULL;
    dp->free_channel = g_queue_new();
    dp->all_channel = NULL;
    
    gst_element_link(dp->adder,dp->output);


    //gst_audio_create_channel(gst_audio,i);

    for(i=0 ;i < N_CHANNEL; i++) {
      channel = gst_audio_create_channel(gst_audio,i); 
      g_queue_push_head(dp->free_channel,channel);
      dp->all_channel = g_list_append( dp->all_channel,channel);
    }
      
    
    g_list_foreach( dp->all_channel,gst_audio_reset_channel,NULL);

      
    //gst_audio_play_shoot(gst_audio);
    

    gst_element_set_state( dp->main_bin,GST_STATE_PLAYING);

    
    g_list_foreach( dp->all_channel,gst_audio_reset_channel,NULL);
      
    gtk_idle_add(gst_audio_idle,gst_audio);

    
    return gst_audio;
}



void gst_audio_play_shoot(GstAudio * audio) {
    GstChannel * channel;
    GstPad * pad;
    gchar buffer[200];

    if( PRIVATE(audio)->free_channel->length > 0 ) {
    
      gst_element_set_state( PRIVATE(audio)->main_bin, GST_STATE_PAUSED);
      channel =( GstChannel *) g_queue_pop_head(PRIVATE(audio)->free_channel);
      g_object_set( G_OBJECT( channel->filesrc), "location","/home/lolo/cvs/monkey-bubble/src/audio/reggae.mp3",NULL);
      
      gst_element_link( channel->filesrc, channel->decoder);

      gst_bin_add( GST_BIN(PRIVATE(audio)->main_bin), channel->pipe);
      pad = gst_element_get_request_pad( PRIVATE(audio)->adder,"sink%d");

      sprintf(buffer,"channel%d",channel->id);
    sprintf(buffer,"channel%d",channel->id);
    
    gst_element_add_ghost_pad( channel->pipe, gst_element_get_pad(channel->decoder,"src"),buffer);
    

      gst_pad_link( gst_element_get_pad( channel->pipe,buffer),
		    pad);

      //      gst_element_set_state( channel->pipe, GST_STATE_PLAYING);

      gst_element_set_state( PRIVATE(audio)->main_bin, GST_STATE_PLAYING);
      g_print("play sample\n");
    }

}

void gst_audio_play_rebound(GstAudio * audio) {}

void gst_audio_play_explose(GstAudio * audio) {}

void gst_audio_play_win(GstAudio * audio) {}

void gst_audio_play_lost(GstAudio * audio) {}
