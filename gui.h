#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

// System tab in application
typedef struct system {
  GtkWidget *lbl_hostname;
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
  GtkWidget *trv_processes; // Treeview
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
  GtkWidget *trv_devices; // Treeview
  GtkListStore *lst_store_devices;
} file_systems_tab_t;

// Main application
typedef struct application {
  GtkWidget *appw_main; // Application window
  GtkWidget *mnu_bar;
  system_tab_t *system_tab;
  processes_tab_t *processes_tab;
  resources_tab_t *resources_tab;
  file_systems_tab_t *file_systems_tab;
} application_t;

application_t *init_application(int argc, char *argv[]);
void load_application_widgets(application_t *);
void configure_system_tab(application_t *);
void configure_processes_tab(application_t *);
void configure_resources_tab(application_t *);
void configure_file_systems_tab(application_t *);
void free_application(application_t *);

#endif // GUI_H
