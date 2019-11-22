#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#define MAX_PROCS (1024)

typedef struct {
  char *name;
  char *status;
  char *owner;
  int cpu;
  int id;
  float mem;
} process_t;

typedef struct {
  process_t **procs;
  int num_procs;
  int total_space;
} proc_list_t;

char *file_to_str(char *filepath);
char *get_line_by_key(char *filebuffer, char *key);
process_t *get_process_info(int pid);
void free_process_t(process_t *proc);
void print_proc(process_t *proc);
void add_proc(proc_list_t *proc_list, process_t *new_proc);
proc_list_t *get_processes();

#endif // PROCESS_H