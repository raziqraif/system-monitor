#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gtk/gtk.h>
#include <gtk/gtk.h>

#include "gui.h"

void on_application_destroy();
void on_mnu_exit_activate(GtkWidget *, application_t *);
void on_mnu_memory_maps_activate(GtkWidget *, application_t *);
void on_mnu_open_files_activate(GtkWidget *, application_t *);
void on_mnu_properties_activate(GtkWidget *, application_t *);
void on_mnu_refresh_activate(GtkWidget *, application_t *);
void on_mnu_all_processes_toggled(GtkWidget *, application_t *);
void on_mnu_active_processes_toggled(GtkWidget *, application_t *);
void on_mnu_my_processes_toggled(GtkWidget *, application_t *);
void on_mnu_dependencies_toggled(GtkWidget *, application_t *);
void show_error_message(GtkWidget *, const char *);

#endif //CALLBACKS_H
