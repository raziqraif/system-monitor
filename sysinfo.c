#include "sysinfo.h"

#include <string.h>
#include <malloc.h>

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

  char *memory = get_line_by_key(file_to_str("/proc/meminfo"),
                                 "MemTotal");
  system_info->memory = strdup(memory);

  char *process_version = get_line_by_key(file_to_str("/proc/cpuinfo"),
                                          "model name");
  system_info->process_version = strdup(process_version);

  //char disk_space[] = get_line_by_key(file_to_str("/proc/"), );
  //system_info->disk_space = strdup(disk_space);

  free(version);
  version = NULL;
  free(memory);
  memory = NULL;
  free(process_version);
  process_version = NULL;
  //free(disk_space);
  //disk_space = NULL;

  return system_info;
} /* get_sys_info() */

/*
 * Returns 0 on success, returns 1 on failure
 */

void free_sys_info(system_info_t *system_info) {
  free(system_info->kernel_version);
  free(system_info->memory);
  free(system_info->process_version);
  free(system_info->disk_space);

  free(system_info);
} /* free_sys_info() */
