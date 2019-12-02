#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#define MAX_PROCS (1024)

typedef struct process_t process_t;

struct process_t {
  char *name;
  char *status;
  char *owner;
  char *starttime;
  char *cputime;
  float cpu;         //cpu usage as a percent of clock ticks in a period
  unsigned long utime;
  unsigned long stime;
  int pid;
  int ppid;
  int uid;
  int vmsize;
  int vmrss;
  int vmdata;
  int vmstk;
  int vmexe;
  unsigned long vsize;
  long rss;
  float mem;
  int update_flag;
  process_t **children;
};

typedef struct {
  process_t **procs;
  int num_procs;
  int total_space;
  char *load_avg;
} proc_list_t;

typedef struct {
  long int vmstart;
  long int vmend;
  char *flags;
  long int vmoffset;
  char *dev;
  int inode;
  char *filename;
  int size;
  int kernpagesize;
  int mmupagesize;
  int rss;
  int pss;
  int shared_clean;
  int shared_dirty;
  int private_clean;
  int private_dirty;
  int referenced;
  int anonymous;
  int lazyfree;
  int anonhugepages;
  int shmempmdmapped;
  int shared_hugetlb;
  int private_hugetlb;
  int swap;
  int swappss;
  int locked;
  char *vmflags;
} smap_t;

char *file_to_str(char *filepath);
char *get_uname(int uid);
char *get_line_by_key(char *filebuffer, char *key);
char *get_val_from_line(char *line, char delim);
process_t *get_process_info(int pid);
void free_process_t(process_t *proc);
void print_proc(process_t *proc);
void add_proc(proc_list_t *proc_list, process_t *new_proc);
proc_list_t *get_processes();
void update_processes(proc_list_t *proc_list);
smap_t *get_smap_info(int pid, FILE *fp);
void free_smap_t(smap_t *smap);
smap_t **get_smaps(int pid);
void free_smaps(smap_t **smaps);
void print_smaps(smap_t **smaps);
void calc_proc_tree(proc_list_t *proc_list);
void print_children(process_t *proc, int depth);

#endif // PROCESS_H
