
#include <gst/gst.h>
#include <gtk/gtk.h>
  GstElement *queue,*thread,*thread2,*pipeline, *filesrc, *decoder, *audiosink;
  GstElement * adder;
  GtkWidget * window;

void        my_function      (GstThread *gstthread,
										gpointer user_data) {
	 g_print("end \n");
}


int main(int argc, char **argv) {

  gtk_init (&argc, &argv);
  gst_init(&argc,&argv);
  thread = gst_thread_new ("thread");
  thread2 = gst_thread_new ("thread2");
  g_assert (thread != NULL);
  pipeline = gst_pipeline_new ("pipeline");
  filesrc = gst_element_factory_make ("filesrc", "disk_source");
  g_object_set (G_OBJECT (filesrc), "location",
					 "/usr/share/games/frozen-bubble/snd/rebound.wav", NULL);
  /* now its time to get the decoder */
   decoder = gst_element_factory_make ("wavparse", "parser");
	queue = gst_element_factory_make("queue","queue");
	//decoder = gst_element_factory_make ("mad", "decoder");
  /* and an audio sink */
	audiosink = gst_element_factory_make ("osssink", "play_audio");
  /* add objects to the main pipeline */

	adder = gst_element_factory_make("adder","the_mixer");

  gst_element_link_many (filesrc, decoder,queue,NULL);
  //  gst_element_link_many( queue,audiosink, NULL);
  gst_bin_add_many (GST_BIN (thread),filesrc, decoder, queue,NULL);
  gst_bin_add_many( GST_BIN(thread2), thread,adder,audiosink,NULL);

  gst_element_add_ghost_pad (thread, gst_element_get_pad (queue, "src"), "src");

  gst_element_link( thread,adder);
  gst_element_link(adder,audiosink);

  //  gst_element_add_ghost_pad (thread2, gst_element_get_pad (queue, "sink"), "sink");

  g_signal_connect( filesrc,"eos",G_CALLBACK(my_function),NULL);

  //  gst_element_link_many( thread,thread2,NULL);
  /* link src to sink */
  // gst_element_set_state (thread, GST_STATE_PLAYING);  
  gst_element_set_state (thread2, GST_STATE_PLAYING);
  //	 gst_element_set_state (audiosink, GST_STATE_PLAYING);  
  
	
	//  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  
  
   //while (gst_bin_iterate (GST_BIN (pipeline)));




  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
  gdk_rgb_init ();

  gtk_widget_show_all (window);

  gtk_main ();
  return 0;
}
