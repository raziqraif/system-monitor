#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "usage.h"
#include "process.h"

#define SMAP_BUF_SIZE (128)
#define LINES_PER_SMAP (21)

char STAT_CPU_FORMAT[] = "%*s %d %*d %d %d %*d %*d %*d %*d %*d %*d";
char MEMINFO_FORMAT[] = "MemTotal: %lu%[^\n]\n"
    "MemFree: %*lu%[^\n]\n"
    "MemAvailable: %lu%[^\n]\n";

char STAT_PATH[] = "/proc/stat";
char MEMINFO_PATH = "/proc/meminfo";

int g_btime = 0;


usage_t *get_usage() {
  // char *full_stat = file_to_str(PROC_STAT_PATH);
  // if (full_stat == NULL) {
  //   printf("could not open %s\n", PROC_STAT_PATH);
  //   return NULL;
  // }
  FILE *stat_fp = fopen(PROC_STAT_PATH, "r");
  if (fp == NULL) {
    printf("fopen error on %s", PROC_STAT_PATH);
    return NULL;
  }

  FILE *meminfo_fp = fopen(MEMINFO_PATH, "r");
  if (fp == NULL) {
    printf("fopen error on %s", MEMINFO_PATH);
    return NULL;
  }

  usage_t *usage = malloc(sizeof(usage_t));

  fscanf(stat_fp, STAT_CPU_FORMAT, &(usage->user), &(usage->system), &(usage->idle))
  fscanf(meminfo_fp, MEMINFO_FORMAT, &(usage->memtotal), &(usage->memavailable));

  char *swap_total_line = get_line_by_key(file_to_str(MEMINFO_PATH), "SwapTotal");
  sscanf(swap_total_line, "SwapTotal: %lu%[^\n]\n", &(usage->swaptotal));
  free(swap_total_line);

  char *swap_free_line = get_line_by_key(file_to_str(MEMINFO_PATH), "SwapFree");
  sscanf(swap_free_line, "SwapFree: %lu%[^\n]\n", &(usage->swapfree));
  free(swap_free_line);

  fclose(stat_fp);
  stat_fp = NULL;

  fclose(meminfo_fp);
  meminfo_fp = NULL;
} /* get_usage() */
