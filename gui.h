#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>
#include <stdbool.h>

#include "process.h"

#define MAIN_WINDOW_UI_FILE "./ui_files/main_window.glade"
#define TREESTORE (0)
#define LISTSTORE (1)
#define ACTIVE_PROCESSES (0)
#define ALL_PROCESSES (1)
#define MY_PROCESSES (2)

extern int g_list_processes_mode;   // Gonna be set by callbacks
extern bool g_list_processes_w_dependency;  // Gonna be set by callbacks

// System tab in application
typedef struct system {
  GtkWidget *lbl_os;
  GtkWidget *lbl_kernel_version;
  GtkWidget *lbl_os_release_version;
  GtkWidget *lbl_memory;
  GtkWidget *lbl_processor;
  GtkWidget *lbl_available_disk_space;
} system_tab_t;

// Processes tab in application
typedef struct processes {
  GtkWidget *lbl_description;
  GtkTreeView *trv_processes; // Treeview
  GtkTreeStore *tree_store_processes;
  GtkWidget *btn_end_process;
} processes_tab_t;

// Resources tab in application
typedef struct resources {
  // TODO: Update this after completing the GUI
  GtkWidget *dummy_widget;
} resources_tab_t;

// File systems tab in application
typedef struct file_systems {
  GtkTreeView *trv_devices; // Treeview
  GtkListStore *lst_store_devices;
} file_systems_tab_t;

// Main application
typedef struct application {
  GtkWidget *appw_main; // Application window
  GtkWidget *mnu_bar;
  GtkBuilder *main_builder;
  GtkBuilder *memory_maps_builder;
  GtkBuilder *open_files_builder;
  GtkBuilder *process_properties_builder;

  system_tab_t *system_tab;
  processes_tab_t *processes_tab;
  resources_tab_t *resources_tab;
  file_systems_tab_t *file_systems_tab;
  
  proc_list_t *processes_list;
} application_t;

application_t *init_application(int argc, char *argv[]);
void configure_system_tab(application_t *, GtkBuilder *);
void configure_processes_tab(application_t *, GtkBuilder *);
void configure_resources_tab(application_t *, GtkBuilder *);
void configure_file_systems_tab(application_t *, GtkBuilder *);
void free_application(application_t *);
void update_processes_treeview(application_t *);
void get_selected_process(application_t *);
void update_devices_treeview(application_t *);
void clear_treeview(void *, int);
gboolean foreach_func(GtkTreeModel *, GtkTreePath *, GtkTreeIter *, GList **);

#endif // GUI_H
