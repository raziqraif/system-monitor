#include "main.h"

#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <math.h>
#include <ctype.h>

#include "gui.h"

#define MAIN_WINDOW_UI_FILE "./ui_files/main_window.glade"

/*
 * Runs the application 
 */

int main(int argc, char *argv[]) {
  init_application(argc, argv);

} /* main() */
