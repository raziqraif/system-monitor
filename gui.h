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
} system_t;

// Processes tab in application
typedef struct processes {
  GtkWidget *lbl_description;
  GtkWidget *trv_processes; // Treeview
  GtkWidget *btn_end_process;
} processes_t;

// Resources tab in application
typedef struct resources {
  // TODO: Update this after completing the GUI
  GtkWidget *dummy_widget;
} resources_t;  

// File systems tab in application
typedef struct file_systems {
  GtkWidget *trv_devices; // Treeview
} file_systems_t;

// Main application
typedef struct application {
  GtkWidget *appw_main; // Application window
  GtkWidget *menu_bar;
  system_t system_tab;
  processes_t processes_tab;
  resources_t resources_tab;
  file_systems_t file_systems_tab;
} application_t;

void init_application(int argc, char *argv[]);
void clear_application_memory(application_t *);

#endif // GUI_H
