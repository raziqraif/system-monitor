#include "gui.h"

#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>

#include "sysinfo.h"

#define MAIN_WINDOW_UI_FILE "./ui_files/main_window.glade"

/*
 * Initialize application
 */

application_t *init_application(int argc, char *argv[]) {
  application_t *app = malloc(sizeof(application_t));
  assert(app);

  app->system_tab = malloc(sizeof(system_tab_t));
  assert(app->system_tab);
  app->processes_tab = malloc(sizeof(processes_tab_t));
  assert(app->processes_tab);
  app->resources_tab = malloc(sizeof(resources_tab_t));
  assert(app->resources_tab);
  app->file_systems_tab = malloc(sizeof(file_systems_tab_t));
  assert(app->file_systems_tab);

  GtkBuilder *builder = gtk_builder_new_from_file(MAIN_WINDOW_UI_FILE);
  // Retrieve application window and menu bar
  app->appw_main = GTK_WIDGET(gtk_builder_get_object(builder, "appw_main"));
  app->mnu_bar = GTK_WIDGET(gtk_builder_get_object(builder, "mnu_bar"));

  // Retrieve widgets for system tab
  system_tab_t *sys_tab = app->system_tab;
  sys_tab->lbl_hostname = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_hostname"));
  sys_tab->lbl_os = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_os"));
  sys_tab->lbl_kernel_version = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_kernel_version"));
  sys_tab->lbl_memory = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_memory"));
  sys_tab->lbl_processor = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_processor"));
  sys_tab->lbl_available_disk_space = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_available_disk_space"));

  // Retrieve widgets for processes_tab
  processes_tab_t *proc_tab = app->processes_tab;
  proc_tab->lbl_description = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_description"));
  proc_tab->trv_processes = GTK_WIDGET(
    gtk_builder_get_object(builder, "trv_processes"));
  proc_tab->btn_end_process = GTK_WIDGET(
    gtk_builder_get_object(builder, "btn_end_process"));
  
  // TODO: Create necessary GUI for resources tab and retrieve the widgets

  // Retrieve widgets for file systems tab
  app->file_systems_tab->trv_devices = GTK_WIDGET(
    gtk_builder_get_object(builder, "trv_devices"));
    
  g_object_unref(G_OBJECT(builder));

  // Update System Info 
  system_info_t *sys_info = get_sys_info();
  gtk_label_set_text(GTK_LABEL(sys_tab->lbl_kernel_version),
    sys_info->kernel_version);
  gtk_label_set_text(GTK_LABEL(sys_tab->lbl_memory),
    sys_info->memory);
  gtk_label_set_text(GTK_LABEL(sys_tab->lbl_processor),
    sys_info->process_version);
  
  // TODO: Free sys_info

  return app;
} /* init_application() */

/*
 * Deallocate all application memory
 */

void free_application(application_t *app) {

  free(app->system_tab);
  app->system_tab = NULL;
  free(app->processes_tab);
  app->processes_tab = NULL;
  free(app->resources_tab);
  app->resources_tab = NULL;
  free(app->file_systems_tab);
  app->file_systems_tab = NULL;

  free(app);
  app = NULL;
} /* free_application() */
