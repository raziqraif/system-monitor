#include "main.h"

#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

#include "gui.h"

#define DEBUG (1)

int g_seconds_passed = 0;

/*
 * Runs the application 
 */

int main(int argc, char *argv[]) {

  gtk_init(&argc, &argv);
  application_t *app = init_application(argc, argv);
  gtk_widget_show_all(app->appw_main);

  // https://www.youtube.com/watch?v=ejOFZEe7K68 
  g_timeout_add_seconds(1, (GSourceFunc) timer_handler, app);
  gtk_main(); 

  if (DEBUG) {
    printf("Application returned\n");
  }
  
  free_application(app);
  app = NULL;  
  return 0;
} /* main() */

/*
 * Handler for timer
 */

bool timer_handler(application_t *app) {
  g_seconds_passed++;
  // printf("%d seconds have passed.\n", g_seconds_passed);
  if (g_seconds_passed == 1) {
    update_processes_treeview(app);
  }
  if (g_seconds_passed % 1 == 0) {
      //update_processes_treeview(app);
  }
  return true;   
} /* timer_handler() */
