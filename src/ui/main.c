
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "sound-manager.h"
#include "bubble.h"
#include "color.h"
#include "playground.h"
#include "monkey.h"
#include "ui-main.h"

#include <esd.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <bonobo/bonobo-i18n.h>
#include <glib/gthread.h>
#include <libgnomeui/gnome-ui-init.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

int main(int  argc, char **argv)
{ 
  UiMain * ui_main;
  GtkWidget * window;
  int i;
  gboolean active_sound;
#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif
  
  gtk_init (&argc, &argv);

  /* to get help working */
  gnome_program_init (PACKAGE, VERSION, 
		      LIBGNOMEUI_MODULE, 
		      argc, argv,
		      GNOME_PROGRAM_STANDARD_PROPERTIES, 
		      NULL);

  
  active_sound = TRUE;
  for( i= 0; i < argc; i++) {
    if( strcmp( argv[i], "--disable-sound") == 0 ) {
      active_sound = FALSE;
      break;
    }
  }
  
  if( active_sound ) {
    gst_init(&argc,&argv);  
  
  }

  sound_manager_active_sound(active_sound);
  
  ui_main = ui_main_get_instance();
  
  window = ui_main_get_window(ui_main);
  
  gdk_rgb_init ();

  gtk_widget_show_all (window);

  gtk_main ();


  return (0);
}

