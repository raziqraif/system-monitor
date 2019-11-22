#include "gui.h"

#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>

#define MAIN_WINDOW_UI_FILE "./ui_files/main_window.glade"

/*
 * Initialize application
 */

application_t *init_application(int argc, char *argv[]) {
  application_t *app = malloc(sizeof(application_t));
  assert(app);

  gtk_init(&argc, &argv); 
  GtkBuilder *builder = gtk_builder_new_from_file(MAIN_WINDOW_UI_FILE);
  app->appw_main = GTK_WIDGET(gtk_builder_get_object(builder, "appw_main"));
  app->mnu_bar = GTK_WIDGET(gtk_builder_get_object(builder, "mnu_bar"));

  g_object_unref(G_OBJECT(builder));

  return app;
} /* init_application() */
