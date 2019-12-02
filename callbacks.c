#include "callbacks.h"

#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <assert.h>

#include "main.h"
#include "gui.h"

/*
 * Called when application is destroyed
 */

void on_application_destroy() {
  gtk_main_quit();
} /* on_application_destroy() */

/*
 * Called when a toplevel application's exit button is destroyed.
 */

void on_general_btn_exit_clicked(GtkWidget *widget, application_t *app) {
  GtkWidget *toplevel = gtk_widget_get_toplevel (widget);
  gtk_widget_destroy(toplevel);
}

/*
 * Called when exit menu is clicked
 */

void on_mnu_exit_activate(GtkWidget *widget, application_t *app) {
  gtk_widget_destroy(GTK_WIDGET(app->appw_main));
} /* on_mnu_exit_activate() */

/*
 * Memory maps
 */

void on_mnu_memory_maps_activate(GtkWidget *widget, application_t *app) {
  printf("memory maps\n");
  GtkBuilder *builder = gtk_builder_new_from_file("ui_files/memory_maps_window.glade");

  // Retrieve application window and menu bar
  GtkWidget *win_main = GTK_WIDGET(gtk_builder_get_object(builder, "win_main"));
  GtkWidget *lbl_description = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_description"));
  assert(lbl_description);

  gtk_builder_connect_signals(builder, app);
  gtk_window_set_modal(GTK_WINDOW(win_main), TRUE);
  gtk_window_present(GTK_WINDOW(win_main));
  g_object_unref(G_OBJECT(builder));
  return;
} /* on_mnu_memory_maps_activate() */

/*
 * Open Files
 */

void on_mnu_open_files_activate(GtkWidget *widget, application_t *app) {
  printf("open files\n");
  GtkBuilder *builder = gtk_builder_new_from_file("ui_files/open_files_window.glade");
  // Retrieve application window and menu bar
  GtkWidget *win_main = GTK_WIDGET(gtk_builder_get_object(builder, "win_main"));
  GtkWidget *lbl_description = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_description"));
  assert(lbl_description);

  gtk_builder_connect_signals(builder, app);
  gtk_window_set_modal(GTK_WINDOW(win_main), TRUE);
  gtk_window_present(GTK_WINDOW(win_main));
  g_object_unref(G_OBJECT(builder));
  return;
} /* on_mnu_open_files_activate() */

/*
 * Process Properties
 */

void on_mnu_properties_activate(GtkWidget *widget, application_t *app) {
  printf("properties\n");
  GtkBuilder *builder = gtk_builder_new_from_file("ui_files/process_properties_window.glade");

  // Retrieve application window and menu bar
  GtkWidget *win_main = GTK_WIDGET(gtk_builder_get_object(builder, "win_main"));
  GtkWidget *lbl_description = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_description"));
  assert(lbl_description);

  gtk_builder_connect_signals(builder, app);
  gtk_window_present(GTK_WINDOW(win_main));
  g_object_unref(G_OBJECT(builder));
  return;
} /* on_mnu_properties_activate() */

/*
 * Refresh
 */

void on_mnu_refresh_activate(GtkWidget *widget, application_t *app) {
  printf("REFRESH!!\n");
  update_processes_treeview(app);
  return;
} /* on_mnu_refresh_activate() */

/*
 * All processes
 */

void on_mnu_all_processes_toggled(GtkWidget *widget, application_t *app) {
  g_list_processes_mode = ALL_PROCESSES;
} /* on_mnu_all_processes_toggled() */

/*
 * Active processes
 */

void on_mnu_active_processes_toggled(GtkWidget *widget, application_t *app) {
  g_list_processes_mode = ACTIVE_PROCESSES;
} /* on_mnu_active_processes_toggled() */

/*
 * My processes
 */

void on_mnu_my_processes_toggled(GtkWidget *widget, application_t *app) {
  g_list_processes_mode = MY_PROCESSES;
} /* on_mnu_my_processes_toggled() */

/*
 * Dependencies
 */

void on_mnu_dependencies_toggled(GtkWidget *widget, application_t *app) {
  g_list_processes_w_dependency = !g_list_processes_w_dependency;
} /* on_mnu_dependencies_toggled() */
