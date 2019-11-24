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
  char *starttime;
  float cpu;         //cpu usage as a percent of clock ticks in a period
  long utime;
  long stime;
  int pid;
  int ppid;
  int uid;
  int vmsize;
  int vmrss;
  int vmdata;
  int vmstk;
  int vmexe;
  float mem;
  int update_flag;
} process_t;

typedef struct {
  process_t **procs;
  int num_procs;
  int total_space;
} proc_list_t;

char *file_to_str(char *filepath);
char *get_uname(int uid);
char *get_line_by_key(char *filebuffer, char *key);
process_t *get_process_info(int pid);
void free_process_t(process_t *proc);
void print_proc(process_t *proc);
void add_proc(proc_list_t *proc_list, process_t *new_proc);
proc_list_t *get_processes();
void update_processes(proc_list_t *proc_list);

#endif // PROCESS_H