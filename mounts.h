#ifndef MOUNTS_H
#define MOUNTS_H

typedef struct {
  int is_valid; 
  char *device_name;
  char *mount_point;
  char *fstype;
  char *total_size;
  char *free_size;
  char *available_size;
  char *used_size;
} device_t;

device_t *get_devices();

#endif //MOUNTS_H