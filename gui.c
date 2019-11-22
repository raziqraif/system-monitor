#include "gui.h"

#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <math.h>
#include <ctype.h>

#define MAIN_WINDOW_UI_FILE "./ui_files/main_window.glade"

/*
 * Initialize application
 */

void init_application(int argc, char *argv[]) {

  gtk_init(&argc, &argv); 
  GtkBuilder *builder = gtk_builder_new_from_file(MAIN_WINDOW_UI_FILE);
  GtkWidget *appw_main = GTK_WIDGET(gtk_builder_get_object(builder, "appw_main"));

  gtk_widget_show(appw_main);
  gtk_main();

} /* init_application() */
