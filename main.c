#include "main.h"

#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>

#include "gui.h"

#define DEBUG (1)

int g_seconds_passed = 0;
pthread_mutex_t g_update_processes_mutex = {0};
pthread_mutex_t g_get_process_info_mutex = {0};


/*
 * Runs the application 
 */

int main(int argc, char *argv[]) {

  pthread_mutex_init(&g_update_processes_mutex, NULL);
  pthread_mutex_init(&g_get_process_info_mutex, NULL);

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
  if (g_seconds_passed == 1) {
    update_processes_treeview(app);
  }

  // Update load averages
  if (g_seconds_passed % 1 == 0) {
    char *prefix = "Load averages for the last 1, 5, 15 minutes: ";
    char *desc = malloc(strlen(prefix) + strlen(app->processes_list->load_avg) + 1);
    strcpy(desc, prefix);
    strcat(desc, app->processes_list->load_avg);
    // Set the label
    gtk_label_set_text(GTK_LABEL(app->processes_tab->lbl_description), desc);
    free(desc);
    desc = NULL;
  }

  return true;   
} /* timer_handler() */
