
#include <gst/gst.h>
#include <gtk/gtk.h>
#include "gst-audio.h"
#include <gdk/gdkkeysyms.h>


static gboolean key_pressed(GtkWidget *widget,
				    GdkEventKey *event,
				    gpointer user_data) {

  GstAudio * audio;

  audio = GST_AUDIO(user_data);

  if( event->keyval == GDK_f ) {

      gst_audio_play_shoot(audio);
  }

  return FALSE;
}

int main(int argc, char **argv) {
  GtkWidget * window;
  GstAudio * audio;

  gtk_init (&argc, &argv);
  gst_init(&argc,&argv);


  audio = gst_audio_new();



  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
  g_signal_connect(G_OBJECT( window ),"key-press-event",
		   GTK_SIGNAL_FUNC (key_pressed),audio);

  gdk_rgb_init ();

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
