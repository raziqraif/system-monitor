#include "sysinfo.h"

#include <string.h>
#include <malloc.h>
#include <sys/statvfs.h>

#include "process.h"

/*
 * Return system information
 */

system_info_t *get_sys_info() {
  system_info_t *system_info = malloc(sizeof(system_info_t));

  char *version = file_to_str("/proc/version");
  int i;
  for (i = 0; i < strlen(version); i++) {
    if (version[i] == '(') {
      if ((i != 0) && (version[i - 1] == ' ')) {
        i--;
      }
      break;
    }
  }
  version[i] = '\0';
  system_info->kernel_version = strdup(version);

  FILE *meminfo_fp = fopen("/proc/meminfo", "r");
  if (meminfo_fp == NULL) {
    printf("fopen error on %s", "/proc/meminfo");
    return NULL;
  }
  unsigned long memtotal = 0;
  fscanf(meminfo_fp, "MemTotal: %lu%*[^\n]\n", &memtotal);
  fclose(meminfo_fp);

  float memtotal_GiB = memtotal / (1024*1024);

  system_info->memory = malloc(sizeof(char) * 512);
  sprintf(system_info->memory, "%.1f GB", memtotal_GiB);

  char *process_version = get_line_by_key(file_to_str("/proc/cpuinfo"),
                                          "model name");
  char *trimmed_version = strchr(process_version, ':');
  if (trimmed_version != NULL) {
    system_info->process_version = strdup(trimmed_version + 2);
  }
  else {
    system_info->process_version = strdup(process_version);
  }

  long available_space = 0;
  device_t *devices = get_devices();
  for (int i = 0; devices[i].is_valid == 1; i++) {
    available_space += devices[i].available_bytes;
  }
  system_info->disk_space = (char *) malloc(20 * sizeof(char));
  if (available_space / 1024 < 1000) {
    sprintf(system_info->disk_space, "%.1f KiB",
                                     (float) (available_space / 1024.0));
  }
  else if (available_space / 1048576 < 1000) {
    sprintf(system_info->disk_space, "%.1f MiB",
                                     (float) (available_space / 1048576.0));
  }
  else {
    sprintf(system_info->disk_space, "%.1f GiB",
                                     (float) (available_space / 1073741824.0));
  }

  free(version);
  version = NULL;
  free(process_version);
  process_version = NULL;

  return system_info;
} /* get_sys_info() */

/*
 * Frees system_info_t variables that are malloc'd in get_sys_info()
 */

void free_sys_info(system_info_t *system_info) {
  if (system_info == NULL) {
    return;
  }

  free(system_info->kernel_version);
  free(system_info->memory);
  free(system_info->process_version);
  free(system_info->disk_space);

  free(system_info);
} /* free_sys_info() */
