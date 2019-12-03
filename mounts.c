#include "mounts.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/statvfs.h>

/*
 * Finds information about each mounted fs, and returns it in an array
 */

device_t *get_devices() {
  int num_devices = 100;
  device_t *devices = (device_t *) malloc(num_devices * sizeof(device_t));
  FILE *mounts_fp = fopen("/proc/mounts", "r");

  // Variables for use while parsing
  int word_num = 0;
  int line = 0;
  char *word = (char *) malloc(50 * sizeof(char));
  unsigned char current_char = fgetc(mounts_fp);
  int char_num = 0;

  // Get the device name, mount point, and fs type
  while (current_char != 255) {
    if (current_char == '\n') {
      devices[line].is_valid = 1;
      word_num = 0;
      line++;
      char_num = 0;
      free(word);
      word = (char *) malloc(50 * sizeof(char));
      if (line == (num_devices - 1)) {
        // If the devices array is full, double the size and continue
        num_devices = num_devices * 2;
        devices = realloc(devices, num_devices);
      }
    }
    else if ((current_char == ' ') || (current_char == ',')) {
      // End of the word, add the null terminator
      word[char_num] = '\0';
      if (word_num == 0) {
        devices[line].device_name = strdup(word);
      }
      else if (word_num == 1) {
        devices[line].mount_point = strdup(word);
      }
      else if (word_num == 2) {
        devices[line].fstype = strdup(word);
      }
      free(word);
      word = (char *) malloc(50 * sizeof(char));
      char_num = 0;
      word_num++;
    }
    else {
      word[char_num] = current_char;
      char_num++;
    }
    current_char = fgetc(mounts_fp);
  }

  devices[line].is_valid = 0;

  // Use statvfs to find sizes
  for (int i = 0; devices[i].is_valid == 1; i++) {
    struct statvfs device_stats = { 0 };
    if (statvfs(devices[i].mount_point, &device_stats) != 0) {
      printf("statvfs failure\n");
    }

    devices[i].total_size = (char *) malloc(20 * sizeof(char));
    devices[i].free_size = (char *) malloc(20 * sizeof(char));
    devices[i].available_size = (char *) malloc(20 * sizeof(char));
    devices[i].used_size = (char *) malloc(20 * sizeof(char));

    long total = device_stats.f_blocks * device_stats.f_bsize;
    if (total / 1024 < 1000) {
      sprintf(devices[i].total_size, "%.1f KiB", (float) (total / 1024.0));
    }
    else if (total / 1048576 < 1000) {
      sprintf(devices[i].total_size, "%.1f MiB", (float) (total / 1048576.0));
    }
    else {
      sprintf(devices[i].total_size, "%.1f GiB", (float) (total / 1073741824.0));
    }

    long free = device_stats.f_bfree * device_stats.f_bsize;
    if (free / 1024 < 1000) {
      sprintf(devices[i].free_size, "%.1f KiB", (float) (free / 1024.0));
    }
    else if (total / 1048576 < 1000) {
      sprintf(devices[i].free_size, "%.1f MiB", (float) (free / 1048576.0));
    }
    else {
      sprintf(devices[i].free_size, "%.1f GiB", (float) (free / 1073741824.0));
    }

    long avail = device_stats.f_bavail * device_stats.f_bsize;
    if (avail / 1024 < 1000) {
      sprintf(devices[i].available_size, "%.1f KiB", (float) (avail / 1024.0));
    }
    else if (avail / 1048576 < 1000) {
      sprintf(devices[i].available_size, "%.1f MiB", (float) (avail / 1048576.0));
    }
    else {
      sprintf(devices[i].available_size, "%.1f GiB", (float) (avail / 1073741824.0));
    }
    
    long used = total - free;
    if (used / 1024 < 1000) {
      sprintf(devices[i].used_size, "%.1f KiB", (float) (used / 1024.0));
    }
    else if (total / 1048576 < 1000) {
      sprintf(devices[i].used_size, "%.1f MiB", (float) (used / 1048576.0));
    }
    else {
      sprintf(devices[i].used_size, "%.1f GiB", (float) (used / 1073741824.0));
    }
  }

  return devices;

} /* get_devices() */