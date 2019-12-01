#ifndef USAGE_H
#define USAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#define MAX_PROCS (1024)

typedef struct {
  unsigned long user;
  unsigned long system;
  unsigned long idle;
  unsigned long memtotal;
  unsigned long memavailable;
  unsigned long swaptotal;
  unsigned long swapfree;
} usage_t;

#endif // USAGE_H