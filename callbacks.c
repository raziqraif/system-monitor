#include "callbacks.h"

#include <gtk/gtk.h>

/*
 * Called when application is destroyed
 */

void on_application_destroy() {
  gtk_main_quit();
} /* on_application_destroy() */
