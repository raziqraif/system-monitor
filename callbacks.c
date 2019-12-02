#include "callbacks.h"

#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <assert.h>

#include "main.h"
#include "process.h"
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

  process_t *proc = get_selected_process(app);
  if (proc == NULL) {
    show_error_message(app->appw_main, "Select a process first."); 
    return;
  }
  smap_t **smaps = get_smaps(proc->pid);
  if (!smaps) {
    // TODO: ADD A MESSAGEBOX 
    show_error_message(app->appw_main, "Permission denied."); 
    printf("PERMISSION DENIED\n");
    return;
  } 
 
  GtkBuilder *builder = gtk_builder_new_from_file("ui_files/memory_maps_window.glade");
  // Retrieve application window and menu bar
  GtkWidget *win_main =
    GTK_WIDGET(gtk_builder_get_object(builder, "win_main"));
  GtkWidget *lbl_description =
    GTK_WIDGET(gtk_builder_get_object(builder, "lbl_description"));
  assert(lbl_description);
  GtkListStore *lst_store_mmaps =
    GTK_LIST_STORE(gtk_builder_get_object(builder, "lst_store_mmaps"));
  GtkTreeIter iter;
  gtk_builder_connect_signals(builder, app);
  gtk_window_set_modal(GTK_WINDOW(win_main), TRUE);
  gtk_window_present(GTK_WINDOW(win_main));

  GtkTreeViewColumn *col1 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_filename"));
  GtkTreeViewColumn *col2 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_vm_start"));
  GtkTreeViewColumn *col3 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_vm_end"));
  GtkTreeViewColumn *col4 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_vm_size"));
  GtkTreeViewColumn *col5 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_flags"));
  GtkTreeViewColumn *col6 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_vm_offset"));
  GtkTreeViewColumn *col7 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_private_clean"));
  GtkTreeViewColumn *col8 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_private_dirty"));
  GtkTreeViewColumn *col9 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_shared_clean"));
  GtkTreeViewColumn *col10 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_shared_dirty"));
  GtkTreeViewColumn *col11 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_inode"));
  GtkTreeViewColumn *col12 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_device"));
  GtkCellRenderer *rnd1 =
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_filename"));
  GtkCellRenderer *rnd2 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_vm_start"));
  GtkCellRenderer *rnd3 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_vm_end"));
  GtkCellRenderer *rnd4 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_vm_size"));
  GtkCellRenderer *rnd5 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_flags"));
  GtkCellRenderer *rnd6 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_vm_offset"));
  GtkCellRenderer *rnd7 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_private_clean"));
  GtkCellRenderer *rnd8 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_private_dirty"));
  GtkCellRenderer *rnd9 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_shared_clean"));
  GtkCellRenderer *rnd10 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_shared_dirty"));
  GtkCellRenderer *rnd11 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_inode"));
  GtkCellRenderer *rnd12 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_device"));
  gtk_tree_view_column_add_attribute(col1, rnd1, "text", 0);
  gtk_tree_view_column_add_attribute(col2, rnd2, "text", 1);
  gtk_tree_view_column_add_attribute(col3, rnd3, "text", 2);
  gtk_tree_view_column_add_attribute(col4, rnd4, "text", 3);
  gtk_tree_view_column_add_attribute(col5, rnd5, "text", 4);
  gtk_tree_view_column_add_attribute(col6, rnd6, "text", 5);
  gtk_tree_view_column_add_attribute(col7, rnd7, "text", 6);
  gtk_tree_view_column_add_attribute(col8, rnd8, "text", 7);
  gtk_tree_view_column_add_attribute(col9, rnd9, "text", 8);
  gtk_tree_view_column_add_attribute(col10, rnd10, "text", 9);
  gtk_tree_view_column_add_attribute(col11, rnd11, "text", 10);
  gtk_tree_view_column_add_attribute(col12, rnd12, "text", 11);
  
  g_object_unref(G_OBJECT(builder));
  
  int id = 0;
  while (smaps[id]) { 
    gtk_list_store_append(lst_store_mmaps, &iter);
    char buffer[10000];
    gtk_list_store_set(lst_store_mmaps, &iter, 0, smaps[id]->filename, -1);
    sprintf(buffer, "%ld", smaps[id]->vmstart);
    gtk_list_store_set(lst_store_mmaps, &iter, 1, buffer, -1);
    sprintf(buffer, "%ld", smaps[id]->vmend);
    gtk_list_store_set(lst_store_mmaps, &iter, 2, buffer, -1);
    sprintf(buffer, "%d", smaps[id]->size);
    gtk_list_store_set(lst_store_mmaps, &iter, 3, buffer, -1);
    gtk_list_store_set(lst_store_mmaps, &iter, 4, smaps[id]->flags, -1);
    sprintf(buffer, "%ld", smaps[id]->vmoffset);
    gtk_list_store_set(lst_store_mmaps, &iter, 5, buffer, -1);
    sprintf(buffer, "%d", smaps[id]->private_clean);
    gtk_list_store_set(lst_store_mmaps, &iter, 6, buffer, -1);
    sprintf(buffer, "%d", smaps[id]->private_dirty);
    gtk_list_store_set(lst_store_mmaps, &iter, 7, buffer, -1);
    sprintf(buffer, "%d", smaps[id]->shared_clean);
    gtk_list_store_set(lst_store_mmaps, &iter, 8, buffer, -1);
    sprintf(buffer, "%d", smaps[id]->shared_dirty);
    gtk_list_store_set(lst_store_mmaps, &iter, 9, buffer, -1);
    sprintf(buffer, "%d", smaps[id]->inode);
    gtk_list_store_set(lst_store_mmaps, &iter, 10, buffer, -1);
    gtk_list_store_set(lst_store_mmaps, &iter, 11, smaps[id]->dev, -1);
    id++;
  }
  return;
} /* on_mnu_memory_maps_activate() */

/*
 * Open Files
 */

void on_mnu_open_files_activate(GtkWidget *widget, application_t *app) {
  printf("open files\n");

  process_t *proc = get_selected_process(app);
  if (proc == NULL) {
    return;
  }
  printf("Process id = %d\n", proc->pid);

  GtkBuilder *builder = gtk_builder_new_from_file("ui_files/open_files_window.glade");
  // Retrieve application window and menu bar
  GtkWidget *win_main = GTK_WIDGET(gtk_builder_get_object(builder, "win_main"));
  GtkWidget *lbl_description = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_description"));
  GtkListStore *lbl_description = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_description"));
  assert(lbl_description);

  GtkTreeViewColumn *col1 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_filename"));
  GtkTreeViewColumn *col2 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_vm_start"));
  GtkTreeViewColumn *col3 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_vm_end"));
  GtkTreeViewColumn *col4 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_vm_size"));
  GtkTreeViewColumn *col5 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_flags"));
  GtkTreeViewColumn *col6 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_vm_offset"));
  GtkTreeViewColumn *col7 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_private_clean"));
  GtkTreeViewColumn *col8 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_private_dirty"));
  GtkTreeViewColumn *col9 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_shared_clean"));
  GtkTreeViewColumn *col10 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_shared_dirty"));
  GtkTreeViewColumn *col11 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_inode"));
  GtkTreeViewColumn *col12 = 
    GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col_device"));
  GtkCellRenderer *rnd1 =
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_filename"));
  GtkCellRenderer *rnd2 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_vm_start"));
  GtkCellRenderer *rnd3 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_vm_end"));
  GtkCellRenderer *rnd4 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_vm_size"));
  GtkCellRenderer *rnd5 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_flags"));
  GtkCellRenderer *rnd6 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_vm_offset"));
  GtkCellRenderer *rnd7 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_private_clean"));
  GtkCellRenderer *rnd8 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_private_dirty"));
  GtkCellRenderer *rnd9 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_shared_clean"));
  GtkCellRenderer *rnd10 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_shared_dirty"));
  GtkCellRenderer *rnd11 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_inode"));
  GtkCellRenderer *rnd12 = 
    GTK_CELL_RENDERER(gtk_builder_get_object(builder, "rnd_device"));
  gtk_tree_view_column_add_attribute(col1, rnd1, "text", 0);
  gtk_tree_view_column_add_attribute(col2, rnd2, "text", 1);
  gtk_tree_view_column_add_attribute(col3, rnd3, "text", 2);
  gtk_tree_view_column_add_attribute(col4, rnd4, "text", 3);
  gtk_tree_view_column_add_attribute(col5, rnd5, "text", 4);
  gtk_tree_view_column_add_attribute(col6, rnd6, "text", 5);
  gtk_tree_view_column_add_attribute(col7, rnd7, "text", 6);
  gtk_tree_view_column_add_attribute(col8, rnd8, "text", 7);
  gtk_tree_view_column_add_attribute(col9, rnd9, "text", 8);
  gtk_tree_view_column_add_attribute(col10, rnd10, "text", 9);
  gtk_tree_view_column_add_attribute(col11, rnd11, "text", 10);
  gtk_tree_view_column_add_attribute(col12, rnd12, "text", 11);
  
  g_object_unref(G_OBJECT(builder));
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
  process_t *proc = get_selected_process(app);
  if (proc == NULL) {
    return;
  }
  printf("Process id = %d\n", proc->pid);

  GtkBuilder *builder = gtk_builder_new_from_file("ui_files/process_properties_window.glade");
  // Retrieve application window and menu bar
  GtkWidget *win_main = GTK_WIDGET(gtk_builder_get_object(builder, "win_main"));
  GtkWidget *lbl_description = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_description"));
  assert(lbl_description);
  //  gtk_label_set_text(GTK_LABEL(sys_tab->lbl_description),
  //  "Properties of process");

  gtk_builder_connect_signals(builder, app);
  gtk_window_present(GTK_WINDOW(win_main));
  g_object_unref(G_OBJECT(builder));
  return;
} /* on_mnu_properties_activate() */

/*
 * Refresh
 */

void on_mnu_refresh_activate(GtkWidget *widget, application_t *app) {
  update_processes_treeview(app);
  return;
} /* on_mnu_refresh_activate() */

/*
 * All processes
 */

void on_mnu_all_processes_toggled(GtkWidget *widget, application_t *app) {
  g_list_processes_mode = ALL_PROCESSES;
  update_processes_treeview(app);
} /* on_mnu_all_processes_toggled() */

/*
 * Active processes
 */

void on_mnu_active_processes_toggled(GtkWidget *widget, application_t *app) {
  g_list_processes_mode = ACTIVE_PROCESSES;
  update_processes_treeview(app);
} /* on_mnu_active_processes_toggled() */

/*
 * My processes
 */

void on_mnu_my_processes_toggled(GtkWidget *widget, application_t *app) {
  g_list_processes_mode = MY_PROCESSES;
  update_processes_treeview(app);
} /* on_mnu_my_processes_toggled() */

/*
 * Dependencies
 */

void on_mnu_dependencies_toggled(GtkWidget *widget, application_t *app) {
  g_list_processes_w_dependency = !g_list_processes_w_dependency;
  update_processes_treeview(app);
} /* on_mnu_dependencies_toggled() */

/*
 * Show error message
 *
 * http://zetcode.com/gui/gtk2/gtkdialogs/
 */

void show_error_message(GtkWidget *window, const char *msg) {
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK, 
            "%s",
            msg);

  gtk_window_set_title(GTK_WINDOW(dialog), "Error");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
} /* show_error_message() */
