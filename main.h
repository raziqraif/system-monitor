#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>

#include "gui.h"

#define DEBUG (1)

extern int g_seconds_passed;

bool timer_handler(application_t *);
#endif // MAIN_H
