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

int cpuUtil = 0;
int cpu[100];

/*
 * Runs the application 
 */

int main(int argc, char *argv[]) {

  // Initialize values in cpu
  for (int i = 0; i < 100; i++) {
    cpu[i] = 0;
  }

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
 *
 * https://www.youtube.com/watch?v=ejOFZEe7K68
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

  // Update CPU Graph
  // Note: If the inverval is to be changed, update the calculation too
  if (g_seconds_passed % 1 == 0) {
    static long time1 = 0;
    static long time2 = 0;
    static int flag = 0;
    char line[128];
    char buffer[32];

    FILE *p1 = popen("head -1 /proc/stat", "r");
    if (p1 == NULL) {
      perror("popen error");
    }
    else {
      fgets(line, 128, p1);
      sscanf(line, "%s %ld", buffer, &time2);
      fclose(p1);

      if (!flag) {
        flag = 1;
        time1 = time2;
        return true;
      }
      cpuUtil = time2 - time1;
      for (int i = 99; i > 0; i--) {
        cpu[i] = cpu[i - 1];
      }
      // TODO: Update the value
      int cpu_cores = 8;
      cpu[0] = cpuUtil / cpu_cores;
      printf("Percentage of cpu utilization = %d%%\n", cpuUtil);
      time1 = time2;
      gtk_widget_queue_draw(app->resources_tab->drw_cpu);
    }
  }

  return true;
} /* timer_handler() */
