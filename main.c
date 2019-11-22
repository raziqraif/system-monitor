#include "main.h"

#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <math.h>
#include <ctype.h>

#include "gui.h"

#define DEBUG (1)

/*
 * Runs the application 
 */

int main(int argc, char *argv[]) {

  application_t *app = init_application(argc, argv);
  gtk_widget_show(app->appw_main);
  gtk_main(); 

  if (DEBUG) {
    printf("Application returned\n");
  }
  return 0;
} /* main() */
