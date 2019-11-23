#ifndef SYSINFO_H
#define SYSINFO_H

#include "process.h"

typedef struct {
  char *kernal_version;
  char *memory;
  char *process_version;
  char *disk_space;
} system_info_t;

#endif //SYSINFI_H