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
#define LINE_BUF (2048)

char STAT_CPU_FORMAT[] = "%*s %d %*d %d %d %*d %*d %*d %*d %*d %*d";
char MEMINFO_FORMAT[] = "MemTotal: %lu%*[^\n]\n"
    "MemFree: %*u%*[^\n]\n"
    "MemAvailable: %lu%*[^\n]\n";

char STAT_PATH[] = "/proc/stat";
char MEMINFO_PATH[] = "/proc/meminfo";
char NET_DEV_PATH[] = "/proc/net/dev";


/*
 * mallocs a new usage_t and fills its fields
 */
usage_t *get_usage() {
  FILE *stat_fp = fopen(STAT_PATH, "r");
  if (stat_fp == NULL) {
    printf("fopen error on %s", STAT_PATH);
    return NULL;
  }

  FILE *meminfo_fp = fopen(MEMINFO_PATH, "r");
  if (meminfo_fp == NULL) {
    printf("fopen error on %s", MEMINFO_PATH);
    return NULL;
  }

  FILE *net_dev_fp = fopen(NET_DEV_PATH, "r");
  if (net_dev_fp == NULL) {
    printf("fopen error on %s", NET_DEV_PATH);
    return NULL;
  }

  usage_t *usage = malloc(sizeof(usage_t));

  fscanf(stat_fp, STAT_CPU_FORMAT, &(usage->user), &(usage->system), &(usage->idle));
  fscanf(meminfo_fp, MEMINFO_FORMAT, &(usage->memtotal), &(usage->memavailable));

  char *swap_total_line = get_line_by_key(file_to_str(MEMINFO_PATH), "SwapTotal");
  sscanf(swap_total_line, "SwapTotal: %lu%*[^\n]\n", &(usage->swaptotal));
  free(swap_total_line);

  char *swap_free_line = get_line_by_key(file_to_str(MEMINFO_PATH), "SwapFree");
  sscanf(swap_free_line, "SwapFree: %lu%*[^\n]\n", &(usage->swapfree));
  free(swap_free_line);

  fscanf(net_dev_fp, "%*[^\n]\n");
  fscanf(net_dev_fp, "%*[^\n]\n");

  unsigned long net_rec_total = 0;
  unsigned long net_sent_total = 0;

  while (1) {
    char line[LINE_BUF];
    int check = fscanf(net_dev_fp, "%[^\n]\n", line);
    if (check == EOF) {
      break;
    }
    if (strstr(line, "eth") != NULL) {
      unsigned long net_rec = 0;
      unsigned long net_sent = 0;
      sscanf(line, "%*s %lu %*u %*u %*u %*u %*u %*u %*u"
                   "%lu %*u %*u %*u %*u %*u %*u %*u",
             &net_rec, &net_sent);
      net_rec_total += net_rec;
      net_sent_total += net_sent;
    }
  }

  usage->net_rec_total = net_rec_total;
  usage->net_sent_total = net_sent_total;

  fclose(stat_fp);
  stat_fp = NULL;
  fclose(meminfo_fp);
  meminfo_fp = NULL;
  fclose(net_dev_fp);
  net_dev_fp = NULL;

  return usage;
} /* get_usage() */


void print_usage(usage_t *usage) {
  printf("USAGE======\n"
      "user: %lu, system: %lu, idle: %lu\n"
      "memtotal: %lu, memavailable: %lu, swaptotal %lu, swapfree: %lu\n"
      "net_rec_total: %lu, net_sent_total: %lu\n",
        usage->user,
        usage->system,
        usage->idle,
        usage->memtotal,
        usage->memavailable,
        usage->swaptotal,
        usage->swapfree,
        usage->net_rec_total,
        usage->net_sent_total
      );
}
