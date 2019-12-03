#include "mounts.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    //printf("%d", current_char);
    if (current_char == '\n') {
      //printf("\n");
      devices[line].is_valid = 1;
      word_num = 0;
      line++;
      char_num = 0;
      free(word);
      word = (char *) malloc(50 * sizeof(char));
      if (line == (num_devices - 1)) {
        // If the devices array is full, double the size and continue
        //printf("Resizing devices[] from %d to %d\n", num_devices, num_devices * 2);
        num_devices = num_devices * 2;
        devices = realloc(devices, num_devices);
      }
    }
    else if ((current_char == ' ') || (current_char == ',')) {
      // End of the word, add the null terminator
      word[char_num] = '\0';
      if (word_num == 0) {
        devices[line].device_name = strdup(word);
        //printf("%s\n", word);
      }
      else if (word_num == 1) {
        devices[line].mount_point = strdup(word);
        //printf("%s\n", word);
      }
      else if (word_num == 2) {
        devices[line].fstype = strdup(word);
        //printf("%s\n", word);
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

  printf("Finished while loop\n");

  devices[line].is_valid = 0;

  // TODO statvfs to find sizes

  return devices;

} /* get_devices() */