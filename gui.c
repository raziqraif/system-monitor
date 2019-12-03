#include "gui.h"

#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>

#include "sysinfo.h"
#include "mounts.h"
#include "callbacks.h"
#include "usage.h"
#include "process.h"
#include "main.h"

int g_list_processes_mode = ALL_PROCESSES;
bool g_list_processes_w_dependency = false;

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
  app->processes_list = get_processes();

  GtkBuilder *builder = gtk_builder_new_from_file(MAIN_WINDOW_UI_FILE);

  // Retrieve application window and menu bar
  app->appw_main = GTK_WIDGET(gtk_builder_get_object(builder, "appw_main"));
  assert(app->appw_main);  
  app->mnu_bar = GTK_WIDGET(gtk_builder_get_object(builder, "mnu_bar"));
  assert(app->mnu_bar);  

  configure_system_tab(app, builder);
  configure_processes_tab(app, builder);
  configure_resources_tab(app, builder);
  configure_file_systems_tab(app, builder);

  g_signal_connect(app->appw_main, "destroy", G_CALLBACK(on_application_destroy), NULL);
  gtk_builder_connect_signals(builder, app);

  g_object_unref(G_OBJECT(builder));

  update_processes_treeview(app); 
  update_devices_treeview(app); 
 
  return app;
} /* init_application() */

/*
 * Load and configure widgets in system_tab. Called in init_application().
 */

void configure_system_tab(application_t *app, GtkBuilder *builder) {
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

  // Update System Info 
  system_info_t *sys_info = get_sys_info();
  
  gtk_label_set_text(GTK_LABEL(sys_tab->lbl_kernel_version),
    sys_info->kernel_version);
  gtk_label_set_text(GTK_LABEL(sys_tab->lbl_memory),
    sys_info->memory);
  gtk_label_set_text(GTK_LABEL(sys_tab->lbl_processor),
    sys_info->process_version);
  gtk_label_set_text(GTK_LABEL(sys_tab->lbl_available_disk_space),
    sys_info->disk_space);
  // TODO: Display os name too(?)

  free_sys_info(sys_info);
  sys_info = NULL;  
} /* configure_system_tab() */

/*
 * Load and configure widgets in system tab. Called in init_application().
 */

void configure_processes_tab(application_t *app, GtkBuilder *builder) {
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
  proc_tab->sel_processes = GTK_TREE_SELECTION(
    gtk_builder_get_object(builder, "sel_processes"));
  assert(proc_tab->sel_processes);
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

} /* configure_processes_tab() */

/*
 * Configure widgets in resources tab. Called in init_application().
 */

void configure_resources_tab(application_t *app, GtkBuilder *builder) {
  // Retrieve widgets for resources tab
  resources_tab_t *res_tab = app->resources_tab;
  res_tab->drw_cpu = GTK_WIDGET(
    gtk_builder_get_object(builder, "drw_cpu"));
  assert(res_tab->drw_cpu);
  res_tab->drw_cpu = 
    GTK_WIDGET( gtk_builder_get_object(builder, "drw_memory"));
  assert(res_tab->drw_cpu);
  res_tab->drw_cpu = GTK_WIDGET(
    gtk_builder_get_object(builder, "drw_network"));
  assert(res_tab->drw_cpu);
  res_tab->lbl_resources_total_cpu = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_resources_total_cpu"));
  assert(res_tab->lbl_resources_total_cpu);
  res_tab->lbl_resources_memory = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_resources_memory"));
  assert(res_tab->lbl_resources_memory);
  res_tab->lbl_resources_swap = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_resources_swap"));
  assert(res_tab->lbl_resources_swap);
  res_tab->lbl_resources_receiving = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_resources_receiving"));
  assert(res_tab->lbl_resources_receiving);
  res_tab->lbl_resources_total_received = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_resources_total_received"));
  assert(res_tab->lbl_resources_total_received);
  res_tab->lbl_resources_sending = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_resources_sending"));
  assert(res_tab->lbl_resources_sending);
  res_tab->lbl_resources_total_sent = GTK_WIDGET(
    gtk_builder_get_object(builder, "lbl_resources_total_sent"));
  assert(res_tab->lbl_resources_total_sent);

  res_tab->total_cpu_util = 0;
  for (int i = 0; i < 100; i++) {
     res_tab->total_cpu_utils[i] = 0;
  }
  res_tab->prev_received_network = 0;
  res_tab->prev_sent_network = 0;

} /*configure_resources_tab() */

/*
 * Load and configure widgets in file systems tab. Called in init_application().
 */

void configure_file_systems_tab(application_t *app, GtkBuilder *builder) {
  // Retrieve widgets for file systems tab
  app->file_systems_tab->trv_devices = GTK_TREE_VIEW(
    gtk_builder_get_object(builder, "trv_devices"));
  assert(app->file_systems_tab->trv_devices);
  app->file_systems_tab->lst_store_devices = GTK_LIST_STORE(
    gtk_builder_get_object(builder, "lst_store_devices"));
  assert(app->file_systems_tab->lst_store_devices);

  // File Systems Tab - Temporary variables to link treeview columns and
  // cell renderers
  GtkTreeViewColumn *col1_d = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_device"));
  GtkTreeViewColumn *col2_d = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_directory"));
  GtkTreeViewColumn *col3_d = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_type"));
  GtkTreeViewColumn *col4_d = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_total"));
  GtkTreeViewColumn *col5_d = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_free"));
  GtkTreeViewColumn *col6_d = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_available"));
  GtkTreeViewColumn *col7_d = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_used"));
  GtkCellRenderer *rnd1_d = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_device"));
  GtkCellRenderer *rnd2_d = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_directory"));
  GtkCellRenderer *rnd3_d = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_type"));
  GtkCellRenderer *rnd4_d = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_total"));
  GtkCellRenderer *rnd5_d = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_free"));
  GtkCellRenderer *rnd6_d = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_available"));
  GtkCellRenderer *rnd7_d = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_used"));
  gtk_tree_view_column_add_attribute(col1_d, rnd1_d, "text", 0);
  gtk_tree_view_column_add_attribute(col2_d, rnd2_d, "text", 1);
  gtk_tree_view_column_add_attribute(col3_d, rnd3_d, "text", 2);
  gtk_tree_view_column_add_attribute(col4_d, rnd4_d, "text", 3);
  gtk_tree_view_column_add_attribute(col5_d, rnd5_d, "text", 4);
  gtk_tree_view_column_add_attribute(col6_d, rnd6_d, "text", 5);
  gtk_tree_view_column_add_attribute(col7_d, rnd7_d, "text", 6);
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
  if (!g_refresh_ready) {
    return;
  }
  // TODO: Clear tree store first before appending processes 
  GtkTreeStore *treestore = app->processes_tab->tree_store_processes;
  clear_treeview(treestore, TREESTORE);
  GtkTreeIter iter;
  
  update_processes(app->processes_list); 
  proc_list_t *proc_list = app->processes_list;
  process_t **procs = proc_list->procs;
  calc_proc_tree(proc_list);

  for (int i = 0; i < proc_list->num_procs; i++) {
    
    if ((g_list_processes_mode == MY_PROCESSES) && 
        (procs[i]->uid != getuid())) {
          continue;
    }
    else if ((g_list_processes_mode == ACTIVE_PROCESSES) && 
             (procs[i]->status[0] == 'S')) {
          continue;
    }
    if (g_list_processes_w_dependency && (procs[i]->ppid != 0)) {
      continue;
    }

    // TODO: Optimize with malloc/realloc
    char cpu_str[10000];
    char id_str[10000];
    char memory_str[10000];
    sprintf(cpu_str, "%.4f", procs[i]->cpu); 
    sprintf(id_str, "%d", procs[i]->pid); 
    sprintf(memory_str, "%.4f", procs[i]->mem); 
    gtk_tree_store_append(treestore, &iter, NULL);
    gtk_tree_store_set(treestore, &iter, 0, procs[i]->name, -1);
    gtk_tree_store_set(treestore, &iter, 1, procs[i]->status, -1);
    gtk_tree_store_set(treestore, &iter, 2, cpu_str, -1);
    gtk_tree_store_set(treestore, &iter, 3, id_str, -1);
    gtk_tree_store_set(treestore, &iter, 4, memory_str, -1);
 
    if (g_list_processes_w_dependency) {
      insert_child_processes(procs[i], treestore, &iter);
    }
  }
  g_refresh_ready = false;
} /* update_processes_treeview() */

/*
 * Insert children processes into treeview.
 */

void insert_child_processes(process_t *process, GtkTreeStore *treestore,
    GtkTreeIter *p_iter_ptr) {

    GtkTreeIter iter;
    // TODO: Optimize with malloc/realloc
    int id = 0;
    while (process->children[id]) {
      process_t *child = process->children[id];
      char cpu_str[10000];
      char id_str[10000];
      char memory_str[10000];
      sprintf(cpu_str, "%.4f", child->cpu); 
      sprintf(id_str, "%d", child->pid); 
      sprintf(memory_str, "%.4f", child->mem); 
      gtk_tree_store_append(treestore, &iter, p_iter_ptr);
      gtk_tree_store_set(treestore, &iter, 0, child->name, -1);
      gtk_tree_store_set(treestore, &iter, 1, child->status, -1);
      gtk_tree_store_set(treestore, &iter, 2, cpu_str, -1);
      gtk_tree_store_set(treestore, &iter, 3, id_str, -1);
      gtk_tree_store_set(treestore, &iter, 4, memory_str, -1);
      
      insert_child_processes(child, treestore, &iter);
      id++;
    }

} /* insert_children_process() */

/*
 * Get selected process in processes treeview
 *
 * http://zetcode.com/gui/gtk2/gtktreeview/
 */

process_t *get_selected_process(application_t *app) {
  GtkTreeModel *treestore = GTK_TREE_MODEL(app->processes_tab->tree_store_processes);
  GtkTreeSelection *selection = app->processes_tab->sel_processes;
  GtkTreeIter iter;
  gchar *value = NULL;
  const int PID_COLUMN = 3;
 
  if (gtk_tree_selection_get_selected(selection, &treestore, &iter)) {
    gtk_tree_model_get(treestore, &iter, PID_COLUMN, &value, -1);
    assert(value);
    int id = atoi(value);    
    proc_list_t *proc_list = app->processes_list;
    process_t **procs = proc_list->procs;
    for (int i = 0; i < proc_list->num_procs; i++) {
      if (procs[i]->pid == id) {
        g_free(value);         
        return procs[i]; 
      }
    }
    g_free(value); 
  }
  return NULL;
} /* get_selected_processes() */

/*
 * Update the list in devices treeview
 */

void update_devices_treeview(application_t *app) {
  assert(app);
 
  GtkListStore *liststore = app->file_systems_tab->lst_store_devices;
  clear_treeview(liststore, LISTSTORE);
  GtkTreeIter iter;
  device_t *devices = get_devices();
  // TODO: Get list of devices
  for (int i = 0; devices[i].is_valid == 1; i++) {
    gtk_list_store_append(liststore, &iter);
    gtk_list_store_set(liststore, &iter, 0, devices[i].device_name, -1);
    gtk_list_store_set(liststore, &iter, 1, devices[i].mount_point, -1);
    gtk_list_store_set(liststore, &iter, 2, devices[i].fstype, -1);
    gtk_list_store_set(liststore, &iter, 3, devices[i].total_size, -1);
    gtk_list_store_set(liststore, &iter, 4, devices[i].free_size, -1);
    gtk_list_store_set(liststore, &iter, 5, devices[i].available_size, -1);
    gtk_list_store_set(liststore, &iter, 6, devices[i].used_size, -1);
  }
} /* update_devices_treeview() */

/*
 * Foreach function to be used  in clear_treeview().
 * https://en.wikibooks.org/wiki/GTK%2B_By_Example/Tree_View/Tree_Models#Removing_Multiple_Rows
 */

gboolean foreach_func (GtkTreeModel *model,
                      GtkTreePath  *path,
                      GtkTreeIter  *iter,
                      GList       **rowref_list) {
    g_assert ( rowref_list != NULL );

    GtkTreeRowReference  *rowref;
    rowref = gtk_tree_row_reference_new(model, path);
    *rowref_list = g_list_append(*rowref_list, rowref);

    return FALSE; /* do not stop walking the store, call us with next row */
} /* foreach_func() */

/*
 * Clear treeview.
 * Arg type will be 0 when treestore is passed and 1 when liststore is passed.
 *
 * https://en.wikibooks.org/wiki/GTK%2B_By_Example/Tree_View/Tree_Models#Removing_Multiple_Rows
 */

void clear_treeview(void *store, int type) {
  GList *rr_list = NULL;
  GList *node;
  gtk_tree_model_foreach(GTK_TREE_MODEL(store),
                         (GtkTreeModelForeachFunc) foreach_func,
                         &rr_list);
  
  for (node = rr_list; node != NULL; node = node->next) {
    GtkTreePath *path;
    path = gtk_tree_row_reference_get_path((GtkTreeRowReference *)node->data);

    if (path) {
      GtkTreeIter iter;
       
      if (gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path)) {
        if (type == TREESTORE) { 
          gtk_tree_store_remove(store, &iter);
        }
        else if (type == LISTSTORE) {
          gtk_list_store_remove(store, &iter);
        }
        else {
          assert(True);
        }
      }
    }
  }
  g_list_foreach(rr_list, (GFunc) gtk_tree_row_reference_free, NULL);
  g_list_free(rr_list);
} /* clear_treeview() */
