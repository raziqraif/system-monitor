#include "gui.h"

#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>

#include "sysinfo.h"
#include "callbacks.h"
#include "process.h"

#define MAIN_WINDOW_UI_FILE "./ui_files/main_window.glade"

/*
 * Initialize application
 */

application_t *init_application(int argc, char *argv[]) {
  // Allocate required memories
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
  load_application_widgets(app, builder);

  configure_system_tab(app);
  configure_processes_tab(app);
  configure_resources_tab(app);
  configure_file_systems_tab(app);

  update_processes_treeview(app); 
 
  return app;
} /* init_application() */

/*
 * Load the main widgets for application from file and assign it to argument
 * app. Called in init_application().
 */

void load_application_widgets(application_t *app, GtkBuilder *builder) {
  // Retrieve application window and menu bar
  app->appw_main = GTK_WIDGET(gtk_builder_get_object(builder, "appw_main"));
  assert(app->appw_main);  
  g_signal_connect(app->appw_main, "destroy", G_CALLBACK(on_application_destroy), NULL);
  gtk_builder_connect_signals(builder, NULL);

  app->mnu_bar = GTK_WIDGET(gtk_builder_get_object(builder, "mnu_bar"));
  assert(app->mnu_bar);  

  // Retrieve widgets for system tab
  system_tab_t *sys_tab = app->system_tab;
  sys_tab->lbl_os = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_os"));
  assert(sys_tab->lbl_os);
  sys_tab->lbl_kernel_version = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_kernel_version"));
  assert(sys_tab->lbl_kernel_version);
  sys_tab->lbl_memory = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_memory"));
  assert(sys_tab->lbl_memory);
  sys_tab->lbl_processor = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_processor"));
  assert(sys_tab->lbl_processor);
  sys_tab->lbl_available_disk_space = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_available_disk_space"));
  assert(sys_tab->lbl_available_disk_space);

  // Retrieve widgets for processes_tab
  processes_tab_t *proc_tab = app->processes_tab;
  proc_tab->lbl_description = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_description"));
  assert(proc_tab->lbl_description);
  proc_tab->trv_processes = GTK_TREE_VIEW(
    gtk_builder_get_object(builder, "trv_processes"));
  assert(proc_tab->trv_processes);
  proc_tab->tree_store_processes = GTK_TREE_STORE(
    gtk_builder_get_object(builder, "tree_store_processes"));
  assert(proc_tab->tree_store_processes);
  proc_tab->btn_end_process = GTK_WIDGET(
    gtk_builder_get_object(builder, "btn_end_process"));
  assert(proc_tab->btn_end_process);

  // Processes Tab - Temporary variables to link treeview columns and cell renderer
  GtkTreeViewColumn *col1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_process_name"));
  GtkTreeViewColumn *col2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_status"));
  GtkTreeViewColumn *col3 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_cpu"));
  GtkTreeViewColumn *col4 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_id"));
  GtkTreeViewColumn *col5 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_memory"));
  GtkCellRenderer *rnd1 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_process_name"));
  GtkCellRenderer *rnd2 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_status"));
  GtkCellRenderer *rnd3 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_cpu"));
  GtkCellRenderer *rnd4 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_id"));
  GtkCellRenderer *rnd5 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_memory"));
  gtk_tree_view_column_add_attribute(col1, rnd1, "text", 0);
  gtk_tree_view_column_add_attribute(col2, rnd2, "text", 1);
  gtk_tree_view_column_add_attribute(col3, rnd3, "text", 2);
  gtk_tree_view_column_add_attribute(col4, rnd4, "text", 3);
  gtk_tree_view_column_add_attribute(col5, rnd5, "text", 4);
 
  // TODO: Create necessary GUI for resources tab and retrieve the widgets

  // Retrieve widgets for file systems tab
  app->file_systems_tab->trv_devices = GTK_TREE_VIEW(
    gtk_builder_get_object(builder, "trv_devices"));
  assert(app->file_systems_tab->trv_devices);
  app->file_systems_tab->lst_store_devices = GTK_LIST_STORE(
    gtk_builder_get_object(builder, "lst_store_devices"));
  assert(app->file_systems_tab->lst_store_devices);

  g_object_unref(G_OBJECT(builder));
} /* load_application_widgets() */

/*
 * Configure widgets in system_tab. Called in init_application().
 */

void configure_system_tab(application_t *app) {
  // Update System Info 
  system_tab_t *sys_tab = app->system_tab;
  system_info_t *sys_info = get_sys_info();
  
  gtk_label_set_text(GTK_LABEL(sys_tab->lbl_kernel_version),
    sys_info->kernel_version);
  gtk_label_set_text(GTK_LABEL(sys_tab->lbl_memory),
    sys_info->memory);
  gtk_label_set_text(GTK_LABEL(sys_tab->lbl_processor),
    sys_info->process_version);
  // TODO: Update available disk memory
  // TODO: Display os name too(?)

  free_sys_info(sys_info);
  sys_info = NULL;  
} /* configure_system_tab() */

/*
 * Configure widgets in system tab. Called in init_application().
 */

void configure_processes_tab(application_t *app) {



  //g_object_unref(G_OBJECT(builder));
} /* configure_processes_tab() */

/*
 * Configure widgets in resources tab. Called in init_application().
 */

void configure_resources_tab(application_t *app) {

} /*configure_resources_tab() */

/*
 * Configure widgets in file systems tab. Called in init_application().
 */

void configure_file_systems_tab(application_t *app) {

} /* configure_file_systems_tab() */

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

/*
 * Update the list in processes treeview
 */

void update_processes_treeview(application_t *app) {
  assert(app); 
  // TODO: Clear tree store first before appending processes 
  
  GtkTreeStore *treestore = app->processes_tab->tree_store_processes;
  GtkTreeIter iter;
  
  proc_list_t *proc_list = get_processes();
  process_t **procs = proc_list->procs;
  for (int i = 0; i < proc_list->num_procs; i++) {
    gtk_tree_store_append(treestore, &iter, NULL);
    gtk_tree_store_set(treestore, &iter, 0, procs[i]->name, -1);
    gtk_tree_store_set(treestore, &iter, 1, procs[i]->status, -1);
    gtk_tree_store_set(treestore, &iter, 2, "cpu", -1);
    gtk_tree_store_set(treestore, &iter, 3, "id", -1);
    gtk_tree_store_set(treestore, &iter, 4, "memory", -1);
  }
} /* update_processes_treeview() */

