/* gst-audio.h
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

#ifndef GST_AUDIO_H
#define GST_AUDIO_H

#include <gtk/gtk.h>


G_BEGIN_DECLS

#define TYPE_GST_AUDIO      (gst_audio_get_type())

#define GST_AUDIO(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_GST_AUDIO,GstAudio))
#define GST_AUDIO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GST_AUDIO,GstAudioClass))
#define IS_GST_AUDIO(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_GST_AUDIO))
#define IS_GST_AUDIO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GST_AUDIO))
#define GST_AUDIO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_GST_AUDIO, GstAudioClass))



typedef struct GstAudioPrivate GstAudioPrivate;



typedef struct {
  GObject parent_instance;
  GstAudioPrivate * private;
} GstAudio ;

typedef struct {
  GObjectClass parent_class;
} GstAudioClass;


GType gst_audio_get_type(void);

GstAudio * gst_audio_new(void);


void gst_audio_play_shoot(GstAudio * audio);
void gst_audio_play_rebound(GstAudio * audio);

void gst_audio_play_explose(GstAudio * audio);

void gst_audio_play_win(GstAudio * audio);

void gst_audio_play_lost(GstAudio * audio);

G_END_DECLS





#endif
