/* Created by Anjuta version 0.9.99 */
/*	This file will not be overwritten */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include "bubble.h"
#include "color.h"
#include <gtk/gtk.h>
#include <bonobo/bonobo-i18n.h>
#include <glib/gthread.h>
#include "playground.h"
#include <esd.h>
#include "monkey.h"
//#include "sound_fx.h"
#include <math.h>
#include "ui-main.h"
#include "gdk-canvas.h"
#include "gdk-view.h"
#include <gst/gst.h>


int main(int argc, char **argv)
{ 
  UiMain * ui_main;
  GtkWidget * window;

#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif
  
  gtk_init (&argc, &argv);

  HACK_load_contents();
  
  ui_main = ui_main_get_instance();

  window = ui_main_get_window(ui_main);
  //gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
  gdk_rgb_init ();

  gtk_widget_show_all (window);

  gtk_main ();


  return (0);
}

