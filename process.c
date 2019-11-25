#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "process.h"

char CPUINFO_PATH[] = "/proc/cpuinfo";
char MEMINFO_PATH[] = "/proc/meminfo";
char VERSION_PATH[] = "/proc/version";

/*char STAT_FORMAT[] = "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu "
  "%*lu %*lu %*ld %*ld %*ld %*ld %*ld %*ld %*llu %*lu %*ld %*lu %*lu %*lu "
  "%*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*d %*d %*u %*u %*llu "
  "%*lu %*ld %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*d"*/

int g_btime = 0;

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
 * return a malloc'd username string from given UID
 */

char *get_uname(int uid){
  struct passwd *pd = getpwuid(uid);
  if (pd == NULL) {
    return strdup("NAMELESS");
  }
  return strdup(pd->pw_name);
} /* get_uname() */


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
    return NULL;
  }
  char *line = malloc(sizeof(char) * (newline_spot - key_spot + 1));
  strncpy(line, key_spot, newline_spot - key_spot);
  line[newline_spot - key_spot] = '\0';
  //printf("found: %s\n", line);
  return line;
} /* get_line_by_key() */


/*
 * extracts int from string of form found in /proc/[pid]/status,
 * where an integer is found between the last and penultimate spaces.
 */

int parse_vm_int(char *line) {
  if (line == NULL) {
    return -1;
  }
  char *last_space = strrchr(line, ' ');
  last_space[0] = '\0';
  char *penultimate_space = strrchr(line, ' ');
  if (penultimate_space == NULL) {
    printf("ERROR: penultimate_space = NULL!\n");
    printf("line = %s\n", line);
    return -1;
  }
  int val = atoi(penultimate_space);
  free(line);
  return val;
} /* parse_vm_int() */


/*
 * Mallocs a new process_t and fills its fields.
 */

process_t *get_process_info(int pid) {
  printf("getting info for: %d\n", pid);
  char path[128];
  sprintf(path, "/proc/%d/status", pid);
  char *full_status = file_to_str(path);
  if (full_status == NULL) {
    return NULL;
  }

  sprintf(path, "/proc/%d/stat", pid);
  char *full_stat = file_to_str(path);
  if (full_stat == NULL) {
    return NULL;
  }

  char *tmp_stat = strdup(full_stat);
  char *tok = strtok(tmp_stat, " ");
  for (int i = 0; i < 21; i++) {
    tok = strtok(NULL, " ");
  }

  printf("21st tok: %s\n", tok);
  long long starttime_ticks = atoll(tok);
  long long starttime_secs = starttime_ticks / sysconf(_SC_CLK_TCK);
  time_t starttime = starttime_secs + g_btime;
  char *starttime_str = strdup(ctime(&starttime));

  char *ppid_line = get_line_by_key(full_status, "PPid:");
  char *last_space = strrchr(ppid_line, '\t');
  int ppid = atoi(last_space + 1);
  free(ppid_line);

  char *uid_line = get_line_by_key(full_status, "Uid:");
  last_space = strrchr(uid_line, '\t');
  int uid = atoi(last_space + 1);
  free(uid_line);

  //sscanf(full_stat, STAT_FORMAT, "")

  /*char *vmsize_line = get_line_by_key(full_status, "VmSize:");
  last_space = strrchr(vmsize_line, ' ');
  last_space[0] = '\0';
  char *penultimate_space = strrchr(vmsize_line, ' ');
  int vmsize = atoi(penultimate_space);
  free(vmsize_line);

  char *vmrss_line = get_line_by_key(full_status, "VmRSS:");
  last_space = strrchr(vmrss_line, ' ');
  last_space[0] = '\0';
  penultimate_space = strrchr(vmrss_line, ' ');
  int vmrss = atoi(penultimate_space);
  free(vmrss_line);*/

  process_t *new_proc = malloc(sizeof(process_t));
  new_proc->name = get_line_by_key(full_status, "Name:");
  new_proc->status = get_line_by_key(full_status, "State:");
  new_proc->owner = get_uname(uid);
  new_proc->starttime = starttime_str;
  new_proc->cpu = 0;
  new_proc->pid = pid;
  new_proc->ppid = ppid;
  new_proc->uid = uid;
  new_proc->mem = 0.0;
  new_proc->vmsize = parse_vm_int(get_line_by_key(full_status, "VmSize:"));
  new_proc->vmrss = parse_vm_int(get_line_by_key(full_status, "VmRSS:"));
  new_proc->vmdata = parse_vm_int(get_line_by_key(full_status, "VmData:"));
  new_proc->vmstk = parse_vm_int(get_line_by_key(full_status, "VmStk:"));
  new_proc->vmexe = parse_vm_int(get_line_by_key(full_status, "VmExe:"));

  free(full_status);
  free(full_stat);
  return new_proc;
} /* get_process_info() */


/*
 * Free the malloc'd fields of a process_t.
 */

void free_process_t(process_t *proc) {
  free(proc->name);
  free(proc->status);
  free(proc->owner);
  free(proc->starttime);
  proc->name = NULL;
  proc->status = NULL;
  proc->owner = NULL;
  proc->starttime = NULL;
} /* free_process_t() */


/*
 * Free all processes in proc_list_t
 */

void free_proc_list_t(proc_list_t *list) {
  for (int i = 0; i < list->num_procs; i++) {
    free_process_t(list->procs[i]);
    free(list->procs[i]);
    list->procs[i] = NULL;
  }
} /* free_proc_list_t() */

/*
 * Print contents of a process_t.
 */

void print_proc(process_t *proc) {
  printf("======Name: %s\nStatus: %s\nOwner: %s\nCPU: %d\nID: %d\nMem: %f\n"
         "Starttime: %s\n",
        proc->name, proc->status, proc->owner, proc->cpu, proc->pid, proc->mem,
        proc->starttime);
} /* print_proc() */


/*
 * adds a process_t * to the given proc_list.
 */

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
  printf("added proc %d to position: %d\n", new_proc->pid, proc_list->num_procs - 1);
  //print_proc(new_proc);
} /* add_proc() */


/*
 * Populates a proc_list_t with process found in /proc.
 */

proc_list_t *get_processes() {
  char *full_stat = file_to_str("/proc/stat");
  if (full_stat == NULL) {
    printf("could not open %s\n", "/proc/stat");
    return NULL;
  }
  char *btime_line = get_line_by_key(full_stat, "btime");
  char *space_spot = strchr(btime_line, ' ');
  g_btime = atoi(space_spot + 1);
  printf("Btime: %d\n", g_btime);
  free(btime_line);
  free(full_stat);

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


/*
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
}*/
