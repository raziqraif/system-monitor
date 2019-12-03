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

char STAT_CPU_FORMAT[] = "%s %d %*d %d %d %*d %*d %*d %*d %*d %*d";
char MEMINFO_FORMAT[] = "MemTotal: %lu%*[^\n]\n"
    "MemFree: %*u%*[^\n]\n"
    "MemAvailable: %lu%*[^\n]\n";

char STAT_PATH[] = "/proc/stat";
char MEMINFO_PATH[] = "/proc/meminfo";
char NET_DEV_PATH[] = "/proc/net/dev";

char MEM_MESSAGE[] = "%.1f GiB (%.1f%%) of %.1f GiB";
char SWAP_MESSAGE[] = "%lu bytes (%.1f%%) of %.1f GiB";

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
  usage->cpus = malloc(sizeof(cpu_usage_t *));
  usage->cpus[0] = NULL;
  usage->num_cpus = 0;

  usage->name = malloc(sizeof(char) * 128);

  int check = fscanf(stat_fp, STAT_CPU_FORMAT,
                usage->name, &(usage->user),
                &(usage->system), &(usage->idle));

  char tempname[128];
  unsigned long tempuser;
  unsigned long tempsystem;
  unsigned long tempidle;

  while (check == 4) {
    check = fscanf(stat_fp, STAT_CPU_FORMAT,
            tempname, &tempuser,
            &tempsystem, &tempidle);
    if (check == 4) {
      usage->num_cpus++;
      usage->cpus = realloc(usage->cpus, sizeof(cpu_usage_t *) * (usage->num_cpus + 1));
      usage->cpus[usage->num_cpus - 1] = malloc(sizeof(cpu_usage_t));
      usage->cpus[usage->num_cpus - 1]->name = strdup(tempname);
      usage->cpus[usage->num_cpus - 1]->user = tempuser;
      usage->cpus[usage->num_cpus - 1]->system = tempsystem;
      usage->cpus[usage->num_cpus - 1]->idle = tempidle;
      usage->cpus[usage->num_cpus] = NULL;
    }
  }

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

  //Generate messages
  char buf[512];
  float memused_GiB = (((float) (usage->memtotal - usage->memavailable) / 1024.0) / 1024.0) / 1024.0;
  float memtot_GiB = (((float) usage->memtotal / 1024.0) / 1024.0) / 1024.0;
  sprintf(buf, MEM_MESSAGE, memused_GiB, memused_GiB / memtot_GiB, memtot_GiB);
  usage->mem_message = strdup(buf);

  float swapused_GiB = (((float) (usage->swaptotal - usage->swapfree) / 1024.0) / 1024.0) / 1024.0;
  float swaptot_GiB = (((float) usage->swaptotal / 1024.0) / 1024.0) / 1024.0;
  sprintf(buf, MEM_MESSAGE, swapused_GiB, swapused_GiB / swaptot_GiB, swaptot_GiB);
  usage->swap_message = strdup(buf);

  fclose(stat_fp);
  stat_fp = NULL;
  fclose(meminfo_fp);
  meminfo_fp = NULL;
  fclose(net_dev_fp);
  net_dev_fp = NULL;

  return usage;
} /* get_usage() */


/*
 * Free the malloc'd fields of a usage_t
 */
void free_usage(usage_t *usage) {
  free(usage->name);
  free(usage->mem_message);
  free(usage->swap_message);
  usage->name = NULL;
  usage->mem_message = NULL;
  usage->swap_message = NULL;
} /* free_usage() */


/*
 *
 */
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
} /* print_usage() */
