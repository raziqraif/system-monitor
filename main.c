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
#include "usage.h"

#define DEBUG (1)

int g_seconds_passed = 0;
bool g_refresh_ready = true;
bool g_priority_refresh_pending = false;

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
  // https://www.youtube.com/watch?v=ejOFZEe7K68 
  g_timeout_add_seconds(1, (GSourceFunc) timer_handler, app);
  gtk_widget_show_all(app->appw_main);
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
  // Update global variables to be used by refresh/update functions.
  g_seconds_passed++;
  if (!g_refresh_ready) {
    g_refresh_ready = true;
  }

  // Update process treeview at the first second. Subsequent calls to it will
  // be done manually by user.
  if (g_seconds_passed == 1) {
    update_processes_treeview(app);
  }

  // Update process treeview if there is user invoked a refresh action that
  // can't be ignored.
  // Eg: When user changed which type of processes to list (active, all, etc) 
  if (g_priority_refresh_pending && g_refresh_ready) {
    update_processes_treeview(app);
    g_priority_refresh_pending = false;
  }

  // Update load averages in process tab.
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

  resources_tab_t *res_tab = app->resources_tab;

  // Update resources tab (graphs and labels).
  // Note: If the inverval is to be changed, update the calculation too
  if (g_seconds_passed % 1 == 0) {
    // Variables for cpu graph
    static long prev_time = 0;
    static long cur_time = 0;
    static int flag = 0;
    char buffer[10000];

    usage_t *usage = get_usage();
    if (usage != NULL) {
      // Update cpu graph
      cur_time = usage->cpus[0]->user + usage->cpus[0]->system;
      if (!flag) {
        flag = 1;
        prev_time = cur_time;
        return true;
      }
      res_tab->total_cpu_util = cur_time - prev_time;
      for (int i = 99; i > 0; i--) {
        res_tab->total_cpu_utils[i] = res_tab->total_cpu_utils[i - 1];
      }

      res_tab->total_cpu_utils[0] = res_tab->total_cpu_util / usage->num_cpus;
      //printf("Percentage of cpu utilization = %d%%\n", 
        res_tab->total_cpu_util);
      prev_time = cur_time;
      gtk_widget_queue_draw(app->resources_tab->drw_cpu);
    
      // Update information displayed through labels.
      sprintf(buffer, "%d%%", res_tab->total_cpu_util);
      gtk_label_set_text(GTK_LABEL(app->resources_tab->lbl_resources_total_cpu), buffer);
      gtk_label_set_text(GTK_LABEL(res_tab->lbl_resources_memory),
        usage->mem_message);
      gtk_label_set_text(GTK_LABEL(res_tab->lbl_resources_swap),
        usage->swap_message);
      int receiving = usage->net_rec_total - res_tab->prev_received_network;
      sprintf(buffer, "%d B/s", receiving);
      res_tab->prev_received_network = usage->net_rec_total;
      gtk_label_set_text(GTK_LABEL(res_tab->lbl_resources_receiving),
        buffer);
      sprintf(buffer, "%ld B", usage->net_rec_total);
      gtk_label_set_text(GTK_LABEL(res_tab->lbl_resources_total_received),
        buffer);
      int sending = usage->net_sent_total- res_tab->prev_sent_network;
      sprintf(buffer, "%d B/s", sending);
      res_tab->prev_sent_network = usage->net_sent_total;
      gtk_label_set_text(GTK_LABEL(res_tab->lbl_resources_sending),
        buffer);
      sprintf(buffer, "%ld B", usage->net_sent_total);
      gtk_label_set_text(GTK_LABEL(res_tab->lbl_resources_total_sent),
        buffer);
      free_usage(usage);
      free(usage);
      usage = NULL;
    }
  }
  return true;
} /* timer_handler() */
