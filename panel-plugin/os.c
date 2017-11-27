/*  os.c
 *  Part of xfce4-cpugraph-plugin
 *
 *  Copyright (c) Alexander Nordfelth <alex.nordfelth@telia.com>
 *  Copyright (c) gatopeich <gatoguan-os@yahoo.com>
 *  Copyright (c) 2007-2008 Angelo Arrifano <miknix@gmail.com>
 *  Copyright (c) 2007-2008 Lidiriel <lidiriel@coriolys.org>
 *  Copyright (c) 2010 Florian Rivoal <frivoal@gmail.com>
 *  Copyright (c) 2010 Peter Tribble <peter.tribble@gmail.com>
 *  Copyright (c) 2017 Tarun Prabhu <tarun.prabhu@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cpu.h"
#include "os.h"

#include <stdio.h>
#include <string.h>

#define PROC_STAT "/proc/stat"
#define PROCMAXLNLEN 256 /* should make it */

guint detect_cpu_number() {
  guint nb_lines = 0;
  FILE *fstat = NULL;
  gchar cpuStr[PROCMAXLNLEN];

  if (!(fstat = fopen(PROC_STAT, "r")))
    return 0;

  while (fgets(cpuStr, PROCMAXLNLEN, fstat)) {
    if (strncmp(cpuStr, "cpu", 3) == 0)
      nb_lines++;
    else
      break;
  }

  fclose(fstat);

  return nb_lines > 1 ? nb_lines - 1 : 0;
}

gboolean read_cpu_data(CpuData *data, guint nb_cpu) {
  FILE *fStat;
  gchar cpuStr[PROCMAXLNLEN];
  gulong user, nice, system, idle, used, total, iowait, irq, softirq;
  guint line;


  if (!(fStat = fopen(PROC_STAT, "r")))
    return FALSE;

  for (line = 0; line < nb_cpu + 1; line++) {
    if (!fgets(cpuStr, PROCMAXLNLEN, fStat) || strncmp(cpuStr, "cpu", 3) != 0) {
      fclose(fStat);
      return FALSE;
    }
    if (sscanf(cpuStr, "%*s %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system,
               &idle, &iowait, &irq, &softirq) < 7)
      iowait = irq = softirq = 0;
    used = user + nice + system + irq + softirq;
    total = used + idle + iowait;
    if ((total - data[line].previous_total) != 0) {
      data[line].load = CPU_SCALE * (used - data[line].previous_used) /
                        (total - data[line].previous_total);
    } else {
      data[line].load = 0;
    }
    data[line].previous_used = used;
    data[line].previous_total = total;
  }

  fclose(fStat);

  return TRUE;
}

guint init_temperature_data(CPUGraph* base) {
  const char* dir = "/sys/class/thermal";
  char path[PATH_MAX];
  DIR *d = NULL;
  struct dirent *ent;
  FILE* fp = NULL;
  char line[64];
  guint nr_temps;

  nr_temps = 0;
  d = opendir(dir);
  if((d = opendir(dir))) {
    while((ent = readdir(d))) {
      if(ent->d_name[0] == '.')
        continue;
      printf("ent: %s\n", ent->d_name);
      printf("%p: %p\n\n", (void*)ent->d_name, strstr(ent->d_name, "thermal"));
      if(strstr(ent->d_name, "thermal") == ent->d_name) {
        snprintf(path, PATH_MAX, "%s/%s/type", dir, ent->d_name);
        if(fp = fopen(path, "r")) {
          fgets(line, sizeof(line), fp);
          fclose(fp);
          if(strcmp(line, "x86_pkg_temp\n") == 0) {
            if(nr_temps < MAX_TEMPERATURES) {
              snprintf(path, PATH_MAX, "%s/%s/temp", dir, ent->d_name);
              g_strlcpy(base->temp_data[nr_temps].file, path, PATH_MAX);
              base->temp_data[nr_temps].temp = -1;
              nr_temps += 1;
            }
          }
        }
      }
    }
    closedir(d);
  }

  return nr_temps;
}

gboolean read_temperature_data(TemperatureData* data, guint nr_temps) {
  int i;
  FILE* fp;
  long temp;
  gboolean ret = TRUE;

  for(i = 0; i < nr_temps; i++) {
    if(fp = fopen(data[i].file, "r")) {
      fscanf(fp, "%ld", &temp);
      fclose(fp);
      temp /= 1000;
      data[i].temp = temp;
    } else {
      data[i].temp = -1;
      ret = FALSE;
    }
  }

  return ret;
}
