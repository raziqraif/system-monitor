#include "sysinfo.h"

#include <string.h>

#define STRING_BUF 50

system_info_t *get_sys_info() {
  system_info_t *system_info = malloc(sizeof(system_info_t));

  char *version = strdup(file_to_str("/proc/version"));
  char *kernal_version = (char *) malloc(STRING_BUF * sizeof(char));
  int i;
  for (i = 0; version[i] != '('; i++) {
    kernal_version[i] = version[i];
  }
  system_info->kernal_version = (char *) malloc((i + 1) * sizeof(char));
  system_info->kernal_version = strdup(kernal_version);

  char *memory = strdup(get_line_by_key(file_to_str("/proc/meminfo"),
                                        "MemTotal"));
  system_info->memory = (char *) malloc(sizeof(memory));
  system_info->memory = strdup(memory);

  char *process_version = strdup(get_line_by_key(file_to_str("/proc/cpuinfo"),
                                                 "model name"));
  system_info->process_version = (char *) malloc(sizeof(process_version));
  system_info->process_version = strdup(process_version);

  //char disk_space[] = get_line_by_key(file_to_str("/proc/"), );
  //system_info->disk_space = malloc(sizeof(disk_space));
  //system_info->disk_space = strdup(disk_space);

  free(version);
  free(kernal_version);
  free(memory);
  free(process_version);
  //free(disk_space);

  return system_info;
}