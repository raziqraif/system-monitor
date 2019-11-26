#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "process.h"

char STAT_FORMAT[] = "%d %*s %*c %d %*d %*d %*d %*d %*u %*lu "
  "%*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %*ld "
  "%*ld %llu %lu %ld %*lu %*lu %*lu %*lu %*lu %*lu "
  "%*lu %*lu %*lu %*lu %*lu %*lu %*lu %*d %*d %*u "
  "%*u %*llu %*lu %*ld %*lu %*lu %*lu %*lu %*lu %*lu "
  "%*lu %*d";

int g_btime = 0;
int g_update_flag = 1;
time_t g_update_time = 0;

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
    //printf("could not find: %s\n", key);
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
 * returns malloc'd string representing the last part of line after splitting
 * on delim.
 * Frees the given line.
 */

char *get_val_from_line(char *line, char delim) {
  if (line == NULL) {
    return NULL;
  }
  char *first_occur = strchr(line, delim);
  if (first_occur == NULL) {
    return NULL;
  }
  char *val = strdup(first_occur + 1);
  free(line);
  return val;
} /* get_val_from_line() */


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
  int val = atoi(penultimate_space);
  free(line);
  return val;
} /* parse_vm_int() */


/*
 * Mallocs a new process_t and fills its fields.
 * Returns NULL if error or if pid is not a process.
 */

process_t *get_process_info(int pid) {
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

  sprintf(path, "/proc/%d/smaps", pid);
  char *full_smaps = file_to_str(path);
  if (full_smaps == NULL) {
    return NULL;
  }

  // char *ppid_line = get_line_by_key(full_status, "PPid:");
  // char *last_space = strrchr(ppid_line, '\t');
  // int ppid = atoi(last_space + 1);
  // free(ppid_line);

  char *uid_line = get_line_by_key(full_status, "Uid:");
  char *last_space = strrchr(uid_line, '\t');
  int uid = atoi(last_space + 1);
  free(uid_line);

  process_t *new_proc = malloc(sizeof(process_t));
  unsigned long long starttime_ticks = 0;

  sscanf(full_stat, STAT_FORMAT, &(new_proc->pid), &(new_proc->ppid),
          &(new_proc->utime), &(new_proc->stime),
          &starttime_ticks, &(new_proc->vsize), &(new_proc->rss));

  if (new_proc->rss == 0) {
    //probably a thread, not a process
    free(new_proc);
    free(full_status);
    free(full_stat);
    free(full_smaps);
    return NULL;
  }
  
  unsigned long long starttime_secs = starttime_ticks / sysconf(_SC_CLK_TCK);
  time_t starttime = starttime_secs + g_btime;
  char *starttime_str = strdup(ctime(&starttime));

  //int swap = parse_vm_int(get_line_by_key(full_smaps, "Swap:"));

  new_proc->name = get_val_from_line(get_line_by_key(full_status, "Name:"), '\t');
  new_proc->status = get_val_from_line(get_line_by_key(full_status, "State:"), '\t');
  new_proc->owner = get_uname(uid);
  new_proc->starttime = starttime_str;
  new_proc->cpu = 0;
  // new_proc->pid = pid;
  // new_proc->ppid = ppid;
  new_proc->uid = uid;
  // new_proc->vmsize = parse_vm_int(get_line_by_key(full_status, "VmSize:"));
  // new_proc->vmrss = parse_vm_int(get_line_by_key(full_status, "VmRSS:"));
  // new_proc->vmdata = parse_vm_int(get_line_by_key(full_status, "VmData:"));
  // new_proc->vmstk = parse_vm_int(get_line_by_key(full_status, "VmStk:"));
  // new_proc->vmexe = parse_vm_int(get_line_by_key(full_status, "VmExe:"));
  new_proc->mem = new_proc->rss;
  new_proc->update_flag = g_update_flag;

  // char *tmp_stat = strdup(full_stat);
  // char *tok = strtok(tmp_stat, " ");
  // for (int i = 0; i < 21; i++) {
  //   tok = strtok(NULL, " ");
  // }
  //long long starttime_ticks = atoll(tok);
  // unsigned long long starttime_secs = starttime_ticks / sysconf(_SC_CLK_TCK);
  // time_t starttime = starttime_secs + g_btime;
  // char *starttime_str = strdup(ctime(&starttime));

  free(full_status);
  free(full_stat);
  free(full_smaps);
  return new_proc;
} /* get_process_info() */


/*
 * Binary Search the proc_list_t for a process_t with given pid.
 * returns -1 if not found.
 */
int find_proc(int pid, proc_list_t *proc_list) {
  int check = proc_list->num_procs / 2;
  int start = 0;
  int end = proc_list->num_procs;
  while (1) {
    if (proc_list->procs[check]->pid == pid) {
      return check;
    }
    if (start == end) {
      return -1;
    }
    if (proc_list->procs[check]->pid > pid) {
      if (end == check) {
        return -1;
      }
      end = check;
    }
    else if (proc_list->procs[check]->pid < pid) {
      if (start == check) {
        return -1;
      }
      start = check;
    }
    check = (start + end) / 2;
  }
} /* find_proc() */


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
  if (proc == NULL) {
    printf("XXXXXXXXXXXXXXXXXXXXX\n");
    return;
  }
  printf("======Name: %s\nStatus: %s\nOwner: %s\nCPU: %f\nID: %d\nMem: %f\n"
         "Starttime: %s\n",
        proc->name, proc->status, proc->owner, proc->cpu, proc->pid, proc->mem,
        proc->starttime);
} /* print_proc() */


/*
 * adds a process_t * to the given proc_list.
 * inserts into position sorted by ascending PID.
 */

void add_proc(proc_list_t *proc_list, process_t *new_proc) {
  if (new_proc == NULL) {
    return;
  }
  if (proc_list->total_space == 0) {
    printf("proc_list is empty, initializing.");
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
  int pos = 0;
  while (new_proc->pid > proc_list->procs[pos]->pid) {
    pos++;
    if (pos >= proc_list->num_procs/*proc_list->procs[pos] == NULL*/) {
      break;
    }
  }
  for (int i = proc_list->num_procs; i > pos; i--) {
    proc_list->procs[i] = proc_list->procs[i - 1];
  }
  proc_list->procs[pos] = new_proc;
  proc_list->num_procs++;
  //printf("added proc %d to position: %d\n", new_proc->pid, proc_list->num_procs - 1);
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

  g_update_time = time(0);

  proc_list_t *proc_list = malloc(sizeof(proc_list_t));
  proc_list->procs = NULL;
  proc_list->num_procs = 0;
  proc_list->total_space = 0;

  DIR *dir = opendir("/proc/");
  if (dir == NULL) {
    printf("Could not open dir: %s\n", "/proc/");
    return NULL;
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
 * Update a proc_list_t. If a process no longer exists, it is freed and NULLed.
 */
void update_processes(proc_list_t *proc_list) {
  printf("--------------------------updating processes\n");
  g_update_flag++;
  DIR *dir = opendir("/proc/");
  if (dir == NULL) {
    printf("Could not open dir: %s\n", "/proc/");
    return;
  }
  time_t old_update_time = g_update_time;
  g_update_time = time(0);
  struct dirent *ent = readdir(dir);
  while(ent != NULL) {
    int pid = atoi(ent->d_name);
    if (pid != 0) {
      int idx = find_proc(pid, proc_list);
      if (idx == -1) {
        add_proc(proc_list, get_process_info(pid));
      }
      else {
        long old_utime = proc_list->procs[idx]->utime;
        long old_stime = proc_list->procs[idx]->stime;
        free_process_t(proc_list->procs[idx]);
        free(proc_list->procs[idx]);
        proc_list->procs[idx] = get_process_info(pid);
        long long used_clocks = ((proc_list->procs[idx]->utime + proc_list->procs[idx]->stime)
                            - (old_utime + old_stime));
        long long total_clocks = (g_update_time - old_update_time) * sysconf(_SC_CLK_TCK);
        proc_list->procs[idx]->cpu = 100 * (float)used_clocks / (float)total_clocks;
        //printf("cpu: %lld = %f, %lld = %f, %f\n", used_clocks, (float)used_clocks, total_clocks, (float)total_clocks, 100 * (float)used_clocks / (float)total_clocks);
      }
    }
    ent = readdir(dir);
  }
  closedir(dir);

  int count = 0;
  for (int i = 0; i < proc_list->num_procs; i++) {
    if (proc_list->procs[i]->update_flag != g_update_flag) {
      free_process_t(proc_list->procs[i]);
      free(proc_list->procs[i]);
      proc_list->procs[i] = NULL;
    }
    else {
      count++;
    }
  }

  // printf("---------holes----------------\n");
  // for (int i = 0; i < proc_list->num_procs; i++) {
  //   if (proc_list->procs[i] == NULL) { printf("_"); }
  //   else { printf("X"); }
  // }
  // printf("\n");

  //now batch remove the NULL holes
  int holes = 0;
  for (int i = 0; i + holes < proc_list->num_procs; i++) {
    while(i + holes < proc_list->num_procs) {
      if (proc_list->procs[i + holes] == NULL) {
        holes++;
      }
      else {
        break;
      }
    }
    if (holes != 0) {
      if (i + holes >= proc_list->num_procs) {
        break;
      }
      proc_list->procs[i] = proc_list->procs[i + holes];
    }
  }
  proc_list->num_procs = count;


  // printf("---------no holes----------------\n");
  // for (int i = 0; i < proc_list->num_procs; i++) {
  //   if (proc_list->procs[i] == NULL) { printf("_"); }
  //   else { printf("X"); }
  // }
  // printf("\n");

  printf("proc_list: len=%d, total_space=%d\n", proc_list->num_procs, proc_list->total_space);
} /* update_processes() */

