#ifndef SYSINFO_H
#define SYSINFO_H

typedef struct {
  char *kernel_version;
  char *memory;
  char *process_version;
  char *disk_space;
} system_info_t;

system_info_t *get_sys_info();
void free_sys_info(system_info_t *system_info);

#endif //SYSINFI_H
