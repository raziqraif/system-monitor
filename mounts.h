#ifndef MOUNTS_H
#define MOUNTS_H

typedef struct {
  int is_valid; 
  char *device_name;
  char *mount_point;
  char *fstype;
  /*char *total_size;
  char *available;
  char *used*/
} device_t;

device_t *get_devices();

#endif //MOUNTS_H