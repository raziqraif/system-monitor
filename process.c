#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#include "process.h"

#define SMAP_BUF_SIZE (128)
#define LINES_PER_SMAP (21)

char STAT_FORMAT[] = "%d %*s %*c %d %*d %*d %*d %*d %*u %*lu "
  "%*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %*ld "
  "%*ld %llu %lu %ld %*lu %*lu %*lu %*lu %*lu %*lu "
  "%*lu %*lu %*lu %*lu %*lu %lu %*lu %*d %*d %*u "
  "%*u %*llu %*lu %*ld %*lu %*lu %*lu %*lu %*lu %*lu "
  "%*lu %*d";

char SMAP_FORMAT[] = "%lx-%lx %s %lx %s %d ";

char SMAP_FORMAT2[] = "%*s %d kB\n"
  "KernelPageSize: %d kB\n"
  "MMUPageSize: %d kB\n"
  "Rss: %d kB\n"
  "Pss: %d kB\n"
  "Shared_Clean: %d kB\n"
  "Shared_Dirty: %d kB\n"
  "Private_Clean: %d kB\n"
  "Private_Dirty: %d kB\n"
  "Referenced: %d kB\n"
  "Anonymous: %d kB\n"
  "LazyFree: %d kB\n"
  "AnonHugePages: %d kB\n"
  "ShmemPmdMapped: %d kB\n"
  "Shared_Hugetlb: %d kB\n"
  "Private_Hugetlb: %d kB\n"
  "Swap: %d kB\n"
  "SwapPss: %d kB\n"
  "Locked: %d kB\n"
  "VmFlags: %[^\n]\n";


int g_btime = 0;
int g_update_flag = 1;
time_t g_update_time = 0;

extern pthread_mutex_t g_update_processes_mutex;
extern pthread_mutex_t g_get_process_info_mutex;

/*
 * Open the file and malloc a string to hold its contents.
 * returned string must be freed by caller.
 */

char *file_to_str(char *filepath) {
  FILE *fp = fopen(filepath, "r");
  if (fp == NULL) {
    printf("(%s)", strerror(errno));
    //printf("fopen failed for: %s\n", filepath);
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
 * Returns NULL if error or if pid is not a process.
 */

process_t *get_process_info(int pid) {
  pthread_mutex_lock(&g_get_process_info_mutex);
  char path[128];
  sprintf(path, "/proc/%d/status", pid);
  char *full_status = file_to_str(path);
  if (full_status == NULL) {
    pthread_mutex_unlock(&g_get_process_info_mutex);
    printf("<FS>");
    return NULL;
  }

  sprintf(path, "/proc/%d/stat", pid);
  char *full_stat = file_to_str(path);
  if (full_stat == NULL) {
    pthread_mutex_unlock(&g_get_process_info_mutex);
    printf("<S>");
    return NULL;
  }

  sprintf(path, "/proc/%d/statm", pid);
  FILE *statm_fp = fopen(path, "r");
  if (statm_fp == NULL) {
    pthread_mutex_unlock(&g_get_process_info_mutex);
    printf("<SM>");
    return NULL;
  }

  char *uid_line = get_line_by_key(full_status, "Uid:");
  char *last_space = strrchr(uid_line, '\t');
  int uid = atoi(last_space + 1);
  free(uid_line);

  process_t *new_proc = malloc(sizeof(process_t));
  unsigned long long starttime_ticks = 0;

  fscanf(statm_fp, "%*d %*d %d %*[^\n]\n", &(new_proc->shared));

  int nswap = 0;

  sscanf(full_stat, STAT_FORMAT, &(new_proc->pid), &(new_proc->ppid),
          &(new_proc->utime), &(new_proc->stime),
          &starttime_ticks, &(new_proc->vsize), &(new_proc->rss), &nswap);

  if (new_proc->rss == 0) {
    //probably a thread, not a process
    free(new_proc);
    free(full_status);
    free(full_stat);
    fclose(statm_fp);
    pthread_mutex_unlock(&g_get_process_info_mutex);
    return NULL;
  }
  
  unsigned long long starttime_secs = starttime_ticks / sysconf(_SC_CLK_TCK);
  time_t starttime = starttime_secs + g_btime;
  char *starttime_str = strdup(ctime(&starttime));

  unsigned long long cputime_secs = (new_proc->utime + new_proc->stime) / sysconf(_SC_CLK_TCK);
  char buf[512];
  sprintf(buf, "%d:%d", (int)(cputime_secs / 60), (int)(cputime_secs % 60));

  new_proc->name = get_val_from_line(get_line_by_key(full_status, "Name:"), '\t');
  new_proc->status = get_val_from_line(get_line_by_key(full_status, "State:"), '\t');
  new_proc->owner = get_uname(uid);
  new_proc->uid = uid;
  new_proc->starttime = starttime_str;
  new_proc->cputime = strdup(buf);
  new_proc->cpu = 0;
  new_proc->uid = uid;
  new_proc->mem = new_proc->rss + nswap;
  new_proc->update_flag = g_update_flag;

  new_proc->children = malloc(sizeof(process_t *));
  new_proc->children[0] = NULL;

  new_proc->vmsize = -1;
  new_proc->vmrss = new_proc->rss;
  new_proc->vmdata = -1;
  new_proc->vmstk = -1;
  new_proc->vmexe = -1;

  free(full_status);
  free(full_stat);
  fclose(statm_fp);
  pthread_mutex_unlock(&g_get_process_info_mutex);
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
      //printf("found %d at %d\n", pid, check);
      return check;
    }
    if (start == end) {
      //printf("start == end\n");
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
  free(proc->cputime);
  free(proc->children);
  proc->name = NULL;
  proc->status = NULL;
  proc->owner = NULL;
  proc->starttime = NULL;
  proc->cputime = NULL;
  proc->children = NULL;
} /* free_process_t() */


/*
 * Free all processes in proc_list_t
 */

void free_proc_list_t(proc_list_t *list) {
  free(list->load_avg);
  list->load_avg = NULL;
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
  /*printf("======Name: %s\nStatus: %s\nOwner: %s\nCPU: %f\nID: %d\nMem: %f\n"
         "Starttime: %s\n",
        proc->name, proc->status, proc->owner, proc->cpu, proc->pid, proc->mem,
        proc->starttime);*/

  printf("%d %s\n", proc->pid, proc->name);
} /* print_proc() */


/*
 * adds a process_t * to the given proc_list.
 * inserts into position sorted by ascending PID.
 */

void add_proc(proc_list_t *proc_list, process_t *new_proc) {
  if (new_proc == NULL) {
    //printf("add_proc() was given a NULL proc\n");
    return;
  }
  if (proc_list->total_space == 0) {
    printf("proc_list is empty, initializing.\n");
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
  proc_list->load_avg = file_to_str("/proc/loadavg");
  char *last_space = strrchr(proc_list->load_avg, ' ');
  last_space[0] = '\0';
  char *penultimate_space = strrchr(proc_list->load_avg, ' ');
  penultimate_space[0] = '\0';

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
  //printf("proc_list: len=%d, total_space=%d\n", proc_list->num_procs, proc_list->total_space);
  closedir(dir);
  return proc_list;
} /* get_processes() */


/*
 * Update a proc_list_t. If a process no longer exists, it is freed and NULLed.
 */
void update_processes(proc_list_t *proc_list) {
  //printf("-------updating processes------------");
  pthread_mutex_lock(&g_update_processes_mutex);
  g_update_flag++;

  free(proc_list->load_avg);
  proc_list->load_avg = file_to_str("/proc/loadavg");
  if (proc_list->load_avg != NULL) {
    char *last_space = strrchr(proc_list->load_avg, ' ');
    if (last_space) {
      last_space[0] = '\0';
    }
    char *penultimate_space = strrchr(proc_list->load_avg, ' ');
    if (penultimate_space) {
      penultimate_space[0] = '\0';
    }
  }
  else {
    proc_list->load_avg = strdup("unknown");
  }

  DIR *dir = opendir("/proc/");
  if (dir == NULL) {
    printf("Could not open dir: %s\n", "/proc/");
    pthread_mutex_unlock(&g_update_processes_mutex);
    return;
  }
  time_t old_update_time = g_update_time;
  g_update_time = time(0);
  struct dirent *ent = readdir(dir);
  while(ent != NULL) {
    int pid = atoi(ent->d_name);
    if (pid != 0) {
      process_t *new = get_process_info(pid);
      if (new != NULL) {
        int idx = find_proc(pid, proc_list);
        if (idx == -1) {
          add_proc(proc_list, new);
        }
        else {
          //printf("<%d == %d>", pid, proc_list->procs[idx]->pid);
          long old_utime = proc_list->procs[idx]->utime;
          long old_stime = proc_list->procs[idx]->stime;
          free_process_t(proc_list->procs[idx]);
          free(proc_list->procs[idx]);
          proc_list->procs[idx] = new;
          long long used_clocks = ((proc_list->procs[idx]->utime + proc_list->procs[idx]->stime)
                              - (old_utime + old_stime));
          long long total_clocks = (g_update_time - old_update_time) * sysconf(_SC_CLK_TCK);
          proc_list->procs[idx]->cpu = 100 * (float)used_clocks / (float)total_clocks;
          //printf("cpu: %lld = %f, %lld = %f, %f\n", used_clocks, (float)used_clocks, total_clocks, (float)total_clocks, 100 * (float)used_clocks / (float)total_clocks);
        }
      }
      else {
        //printf("[%d]", pid);
      }
    }
    ent = readdir(dir);
  }
  closedir(dir);

  int count = 0;
  for (int i = 0; i < proc_list->num_procs; i++) {
    if (proc_list->procs[i]->update_flag != g_update_flag) {
      /*printf("WRONG UPDATE_FLAG: %d != %d PID: %d\n",
          proc_list->procs[i]->update_flag, g_update_flag,
          proc_list->procs[i]->pid);*/
      free_process_t(proc_list->procs[i]);
      free(proc_list->procs[i]);
      proc_list->procs[i] = NULL;
    }
    else {
      count++;
    }
  }
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

  printf("proc_list: len=%d, total_space=%d\n", proc_list->num_procs, proc_list->total_space);

  pthread_mutex_unlock(&g_update_processes_mutex);
} /* update_processes() */


/*
 * mallocs and fills a smap_t with info from a FILE *
 * expected from /proc/[pid]/smaps
 * This should probably only be used by get_smaps()
 */
smap_t *get_smap_info(int pid, FILE *fp){
  if (fp == NULL) {
    return NULL;
  }
  smap_t *new_smap = malloc(sizeof(smap_t));
  new_smap->flags = malloc(sizeof(char) * SMAP_BUF_SIZE);
  new_smap->dev = malloc(sizeof(char) * SMAP_BUF_SIZE);
  new_smap->filename = malloc(sizeof(char) * SMAP_BUF_SIZE);
  new_smap->vmflags = malloc(sizeof(char) * SMAP_BUF_SIZE);

  int scanned = fscanf(fp, SMAP_FORMAT, 
        &(new_smap->vmstart),
        &(new_smap->vmend),
        new_smap->flags,
        &(new_smap->vmoffset),
        new_smap->dev,
        &(new_smap->inode)
      );

  fseek(fp, -1, SEEK_CUR);

  if (fgetc(fp) == '\n') {
    new_smap->filename = strdup("");
  }
  else {
    fseek(fp, -1, SEEK_CUR);
    fscanf(fp, "%[^\n]\n", new_smap->filename);
  }
  
  int scanned2 = fscanf(fp, SMAP_FORMAT2,
        &(new_smap->size),
        &(new_smap->kernpagesize),
        &(new_smap->mmupagesize),
        &(new_smap->rss),
        &(new_smap->pss),
        &(new_smap->shared_clean),
        &(new_smap->shared_dirty),
        &(new_smap->private_clean),
        &(new_smap->private_dirty),
        &(new_smap->referenced),
        &(new_smap->anonymous),
        &(new_smap->lazyfree),
        &(new_smap->anonhugepages),
        &(new_smap->shmempmdmapped),
        &(new_smap->shared_hugetlb),
        &(new_smap->private_hugetlb),
        &(new_smap->swap),
        &(new_smap->swappss),
        &(new_smap->locked),
        new_smap->vmflags
      );

  //printf("scanned: %d, pid: %d, filename: %s\n", scanned + scanned2, pid, new_smap->filename);

  if (scanned + scanned2 < 26) {
    printf("error in pid: %d\n", pid);
    free(new_smap->flags);
    free(new_smap->dev);
    free(new_smap->filename);
    free(new_smap->vmflags);
    free(new_smap);
    return NULL;
  }
  return new_smap;
} /* get_smap_info() */


/*
 * frees the malloc'd fields of an smap_t
 */
void free_smap_t(smap_t *smap) {
  free(smap->flags);
  free(smap->dev);
  free(smap->filename);
  free(smap->vmflags);
  smap->flags = NULL;
  smap->dev = NULL;
  smap->filename = NULL;
  smap->vmflags = NULL;
} /* free_smap_t() */


/*
 * Returns a pointer to an (smap_t *) that is the head of a null-terminated
 * array of (smap_t *)
 */
smap_t **get_smaps(int pid) {
  char path[128];
  sprintf(path, "/proc/%d/smaps", pid);
  FILE *fp = fopen(path, "r");
  if (fp == NULL) {
    printf("unable to open %s\n", path);
    return NULL;
  }
  int num_lines = 0;
  char c = getc(fp);
  while(c != EOF) {
    if (c == '\n') {
      num_lines++;
    }
    c = getc(fp);
  }
  fseek(fp, 0, SEEK_SET);

  int num_smaps = (num_lines / LINES_PER_SMAP) + 1;
  smap_t **smaps = malloc(sizeof(smap_t *) * (num_smaps + 1));
  smaps[num_smaps] = NULL;
  for (int i = 0; i < num_smaps; i++) {
    smaps[i] = get_smap_info(pid, fp);
    if (smaps[i] == NULL) {
      //printf("unexpected NULL\n");
    }
  }
  fclose(fp);
  fp = NULL;
  return smaps;
} /* get_smaps() */


/*
 * frees each smap_t of smaps
 */
void free_smaps(smap_t **smaps) {
  int i = 0;
  while(smaps[i] != NULL) {
    free_smap_t(smaps[i]);
    smaps[i] = NULL;
    i++;
  }
} /* free_smaps() */


/*
 * for testing
 */
void print_smaps(smap_t **smaps) {
  int i = 0;
  while(smaps[i] != NULL) {
    printf("vmstart: %lx, filename: %s\n", smaps[i]->vmstart, smaps[i]->filename);
    i++;
  }
} /* print_smaps() */


/*
 * adds a child to a process_t.
 */
void add_child(process_t *proc, process_t *child) {
  int count = 0;
  while (proc->children[count] != NULL) {
    count++;
  }
  proc->children = realloc(proc->children, sizeof(process_t *) * (count + 2));
  proc->children[count] = child;
  proc->children[count + 1] = NULL;
} /* add_child() */


/*
 * Populates the children field of each process.
 */
void calc_proc_tree(proc_list_t *proc_list) {
  for (int i = 0; i < proc_list->num_procs; i++) {
    proc_list->procs[i]->children[0] = NULL;
    for (int j = 0; j < proc_list->num_procs; j++) {
      if (proc_list->procs[j]->ppid == proc_list->procs[i]->pid) {
        add_child(proc_list->procs[i], proc_list->procs[j]);
      }
    }
  }
} /* calc_proc_tree() */



/*
 * recursively print children of a process_t
 */
void print_children(process_t *proc, int depth) {
  for (int j = 0; j < depth; j++) {
    printf("-");
  }
  print_proc(proc);
  int i = 0;
  while(proc->children[i] != NULL) {
    print_children(proc->children[i], depth + 1);
    i++;
  }
} /* print_children() */


/*
 * returns a NULL-terminated array of malloc'd fds_t
 */
fds_t **get_fds(int pid) {
  char path[512];
  sprintf(path, "/proc/%d/fd", pid);

  DIR *dir = opendir(path);
  if (dir == NULL) {
    printf("Could not open dir: %s\n", path);
    return NULL;
  }

  // fds->type = malloc(sizeof(char) * 2);
  // fds->type[1] = '\0';
  
  int count = 0;
  struct dirent *ent = readdir(dir);
  while(ent != NULL) {
    count++;
    ent = readdir(dir);
  }

  fds_t **fds_arr = malloc(sizeof(fds_t *) * (count + 1));
  fds_arr[count] = NULL;

  rewinddir(dir);

  int i = 0;
  ent = readdir(dir);
  while(ent != NULL) {
    if (i == count) {
      printf("souldn't be here!=======================\n");
    }
    if ((strcmp(ent->d_name, ".") == 0) ||
        (strcmp(ent->d_name, "..") == 0)) {
      ent = readdir(dir);
      continue;
    }
    fds_arr[i] = malloc(sizeof(fds_t));
    fds_t *fds = fds_arr[i];
    fds->fd_str = strdup(ent->d_name);

    char *readlink_path = malloc(sizeof(char *)
                            * (strlen(path) + strlen(ent->d_name) + 1));
    sprintf(readlink_path, "%s/%s", path, ent->d_name);
    char buf[1024];
    int nread = readlink(readlink_path, buf, 1023);
    buf[nread] = '\0';
    fds->object = strdup(buf);

    struct stat fstat_buf = { 0 };

    int fd_int = atoi(ent->d_name);
    fstat(fd_int, &fstat_buf);

    if (S_ISBLK(fstat_buf.st_mode)) {
      fds->type = strdup("block device");
    }
    else if (S_ISCHR(fstat_buf.st_mode)) {
      fds->type = strdup("character device");
    }
    else if (S_ISDIR(fstat_buf.st_mode)) {
      fds->type = strdup("directory");
    }
    else if (S_ISFIFO(fstat_buf.st_mode)) {
      fds->type = strdup("named pipe (Felse ifO)");
    }
    else if (S_ISLNK(fstat_buf.st_mode)) {
      fds->type = strdup("symbolic link");
    }
    else if (S_ISREG(fstat_buf.st_mode)) {
      fds->type = strdup("regular file");
    }
    else if (S_ISSOCK(fstat_buf.st_mode)) {
      fds->type = strdup("UNIX domain socket");
    }
    else {
      fds->type = strdup("error reading d_type");
    }
    i++;
    ent = readdir(dir);
  }

  fds_arr[i] = NULL;
  closedir(dir);

  return fds_arr;
} /* get_fds() */


/*
 * free malloc'd fields of NULL-terminated array fds_t
 */
void free_fds(fds_t **fds_arr) {
  int i = 0;
  while (fds_arr[i] != NULL) {
    free(fds_arr[i]);
    fds_arr[i] = NULL;
    i++;
  }
} /* free_fds() */


