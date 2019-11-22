
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#define MAX_PROCS (1024)

char CPUINFO_PATH[] = "/proc/cpuinfo";
char MEMINFO_PATH[] = "/proc/meminfo";
char VERSION_PATH[] = "/proc/version";


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


/*
 * Open the file and malloc a string to hold its contents.
 * returned string must be freed by caller.
 */
char *file_to_str(char *filepath) {
  FILE *fp = fopen(filepath, "r");
  if (fp == NULL) {
    printf("fopen failed for: %s\n", filepath);
    return NULL;
  }
  int file_size = 0;
  while(fgetc(fp) != EOF) {
    file_size++;
  }
  fseek(fp, 0, SEEK_SET);
  char *file_buffer = malloc(sizeof(char) * (file_size + 1));
  fread(file_buffer, sizeof(char), file_size, fp);
  fclose(fp);
  fp = NULL;
  return file_buffer;
} /* file_to_str() */


/*
 * Searches for a line in a filebuffer, then returns a newly
 * malloc'd string of the line.
 * returns NULL if not found.
 */
char *get_line_by_key(char *filebuffer, char *key) {
  char *key_spot = strstr(filebuffer, key);
  if (key_spot == NULL) {
    printf("could not find: %s\n", key);
    return NULL;
  }
  char *newline_spot = strchr(key_spot, '\n');
  if (newline_spot == NULL) {
    printf("could not find newline after key: %s\n", key);
  }
  char *line = malloc(sizeof(char) * (newline_spot - key_spot + 1));
  strncpy(line, key_spot, newline_spot - key_spot);
  line[newline_spot - key_spot] = '\0';
  //printf("found: %s\n", line);
  return line;
} /* get_line_by_key() */


/*
 * Mallocs a new process_t and fills its fields.
 */
process_t *get_process_info(int pid) {
  char path[512];
  sprintf(path, "/proc/%d/status", pid);
  char *full_status = file_to_str(path);
  if (full_status == NULL) {
    return NULL;
  }
  process_t *new_proc = malloc(sizeof(process_t));

  new_proc->name = get_line_by_key(full_status, "Name:");
  new_proc->status = get_line_by_key(full_status, "State:");
  new_proc->owner = NULL;
  new_proc->cpu = 0;
  new_proc->id = pid;
  new_proc->mem = 0.0;

  return new_proc;
} /* get_process_info() */


/*
 * Free the malloc'd fields of a process_t.
 */
void free_process_t(process_t *proc) {
  free(proc->name);
  free(proc->status);
  free(proc->owner);
  proc->name = NULL;
  proc->status = NULL;
  proc->owner = NULL;
} /* free_process_t() */


/*
 * Print contents of a process_t.
 */
void print_proc(process_t *proc) {
  printf("======\nName: %s\nStatus: %s\nOwner: %s\nCPU: %d\nID: %d\nMem: %f\n",
        proc->name, proc->status, proc->owner, proc->cpu, proc->id, proc->mem);
} /* print_proc() */


void add_proc(proc_list_t *proc_list, process_t *new_proc) {
  if (proc_list->total_space == 0) {
    printf("proc_list is empty, mallocing space. this should not occur.\n");
    proc_list->procs = malloc(sizeof(process_t *) * MAX_PROCS);
    proc_list->procs[0] = new_proc;
    proc_list->num_procs = 1;
    proc_list->total_space = MAX_PROCS;
    return;
  }
  if (proc_list->total_space <= proc_list->num_procs) {
    proc_list->total_space += MAX_PROCS;
    proc_list->procs = realloc(proc_list->procs, sizeof(process_t *) * (proc_list->total_space));
  }
  proc_list->procs[proc_list->num_procs] = new_proc;
  proc_list->num_procs++;
  printf("added proc %d to position: %d\n", new_proc->id, proc_list->num_procs - 1);
  //print_proc(new_proc);
}

/*
 * Populates a proc_list_t with process found in /proc.
 */
proc_list_t *get_processes() {
  proc_list_t *proc_list = malloc(sizeof(proc_list_t));
  proc_list->num_procs = 0;
  proc_list->total_space = 0;

  DIR *dir = opendir("/proc/");
  if (dir == NULL) {
    printf("Could not open dir: %s\n", "/proc/");
    return 0;
  }
  struct dirent *ent = readdir(dir);
  while(ent != NULL) {
    int pid = atoi(ent->d_name);
    if (pid != 0) {
      add_proc(proc_list, get_process_info(pid));
    }
    ent = readdir(dir);
  }
  printf("proc_list: len=%d, total_space=%d\n", proc_list->num_procs, proc_list->total_space);
  closedir(dir);
  return proc_list;
} /* get_processes() */


int main(){

  get_processes();

  DIR *dir = opendir("/proc/");
  if (dir == NULL) {
    printf("Could not open dir: %s\n", "/proc/");
    return 0;
  }

  //char *cpuinfo = file_to_str(CPUINFO_PATH);
  //char *meminfo = file_to_str(MEMINFO_PATH);
  char *version = file_to_str(VERSION_PATH);

  printf("OS Release Version: %s\n", version);
  //printf("Kernel Version: %s\n", cpuinfo);
  printf("Amount of Memory: %s\n", "idk");
  printf("Processor Version: %s\n", "idk");
  printf("Disk Storage: %s\n", "idk");

  closedir(dir);

  return 0;

}
