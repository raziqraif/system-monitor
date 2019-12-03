#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>

#include "gui.h"

#define DEBUG (1)

extern int g_seconds_passed;
extern bool g_refresh_ready;  // For process treeview
extern bool g_priority_refresh_pending;  // For process treeview

bool timer_handler(application_t *);
#endif // MAIN_H
