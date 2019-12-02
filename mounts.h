#ifndef MOUNTS_H
#define MOUNTS_H

typedef struct {
  char *device_name;
  char *mount_point;
  char *fstype;
  /*char *total_size;
  char *available;
  char *used*/
} device_t;

#endif //MOUNTS_H