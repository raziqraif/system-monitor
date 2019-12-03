#ifndef USAGE_H
#define USAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#define MAX_PROCS (1024)

typedef struct {
  char *name;
  unsigned long user;
  unsigned long system;
  unsigned long idle;
} cpu_usage_t;

typedef struct {
  char *name;
  char *mem_message;
  char *swap_message;
  unsigned long user;
  unsigned long system;
  unsigned long idle;
  unsigned long memtotal;
  unsigned long memavailable;
  unsigned long swaptotal;
  unsigned long swapfree;
  unsigned long net_rec_total;
  unsigned long net_sent_total;
  cpu_usage_t **cpus;
  int num_cpus;
} usage_t;

usage_t *get_usage();
void print_usage(usage_t *usage);
void free_usage(usage_t *usage);

#endif // USAGE_H