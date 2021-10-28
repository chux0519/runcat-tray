#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

#define TRAY_APPINDICATOR_ID "runcat-applet"
#define TRAY_ICON "/usr/local/share/runcat/icons/cat/my-sleeping-symbolic.svg"
#define LEN(arr) ((int)(sizeof(arr) / sizeof(arr)[0]))
#define PROC_STAT "/proc/stat"

typedef struct {
  /* see:
   * https://unix.stackexchange.com/questions/123908/how-to-get-cpu-percentage-as-a-counter
   */
  uint64_t total_a;
  uint64_t total_b;
  int num_cores;
  double percent;
} cpu_usage_t;

cpu_usage_t CPU_USAGE = {0};

/* timer counter, to choose which frame to show */
uint64_t COUNTER = 0;

/* fps range is [3, 30] */
int FPS_30 = 33;
int FPS_4 = 250;
/* ((double)FPS_4 - (double)FPS_30) / 100.0 */
double FPS_DELTA = 2.17;

/* sample rate is (always) 100HZ, check sysconf(_SC_CLK_TCK), no need to be
 * higher */
int SAMPLE_RATE = 100;

/* icons from: https://github.com/win0err/gnome-runcat/tree/master/src/icons/cat
 */
static char *FRAMES[] = {
    "/usr/local/share/runcat/icons/cat/my-running-0-symbolic.svg",
    "/usr/local/share/runcat/icons/cat/my-running-1-symbolic.svg",
    "/usr/local/share/runcat/icons/cat/my-running-2-symbolic.svg",
    "/usr/local/share/runcat/icons/cat/my-running-3-symbolic.svg",
    "/usr/local/share/runcat/icons/cat/my-running-4-symbolic.svg",
};

/* global, avoid to pass around by now */
GtkWidget *root, *item_cpu, *item_quit;
AppIndicator *indicator;
GError *error = NULL;

static gboolean get_cpu_usage() {
  FILE *fd = fopen(PROC_STAT, "r");
  if (fd == NULL) {
    g_print("failed to open %s\n", PROC_STAT);
    return false;
  }

  char *line = NULL;
  size_t len = 0;

  int row = 0;
  uint64_t total = 0;
  while (getline(&line, &len, fd) != -1) {
    if (strncmp(line, "cpu", 3) == 0) {
      if (row == 0) {
        /* example: cpu  2350865 48 1271510 154132720 73802 20 68979 0 0 0 */
        /* sum the col1 col2 col3 */
        char *token = strtok(line, " ");
        int pos = 0;
        while (token) {
          if (pos >= 1 && pos <= 3) {
            total += atoi(token);
          }
          token = strtok(NULL, " ");
          pos++;
        }
      }
      row++;
    }
  }

  /* update cpu usage */
  CPU_USAGE.num_cores = row;
  CPU_USAGE.total_a = CPU_USAGE.total_b;
  CPU_USAGE.total_b = total;
  if (CPU_USAGE.total_b != 0 && CPU_USAGE.total_a != 0) {
    CPU_USAGE.percent =
        ((double)CPU_USAGE.total_b - (double)CPU_USAGE.total_a) / 5.0 /
        (double)SAMPLE_RATE / (double)CPU_USAGE.num_cores * 100;

    /* update label */
    char label[16] = {0};
    sprintf(label, "cpu: %.2lf%%", CPU_USAGE.percent);
    gtk_menu_item_set_label((GtkMenuItem *)item_cpu, label);
  }

  fclose(fd);
  return true;
}

/* timer callback, will change fps by cpu usage each time */
static gboolean tray_icon_update(gpointer data) {
  app_indicator_set_icon(indicator, FRAMES[COUNTER++ % LEN(FRAMES)]);

  double diff = FPS_DELTA * CPU_USAGE.percent;
  double time = (double)FPS_4 - diff;
  g_timeout_add((int)time, tray_icon_update, NULL);

  return false;
}

int main(int argc, char **argv) {
  srand(time(0));

  gtk_init(&argc, &argv);

  indicator = app_indicator_new(TRAY_APPINDICATOR_ID, TRAY_ICON,
                                APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);

  root = gtk_menu_new();

  item_cpu = gtk_menu_item_new_with_label("cpu");

  item_quit = gtk_menu_item_new_with_label("quit");
  g_signal_connect(item_quit, "activate", G_CALLBACK(gtk_main_quit), NULL);

  gtk_menu_shell_insert(GTK_MENU_SHELL(root), item_cpu, 0);

  gtk_menu_shell_insert(GTK_MENU_SHELL(root), item_quit, 1);

  app_indicator_set_menu(indicator, GTK_MENU(root));
  gtk_widget_show_all(root);

  g_timeout_add(FPS_4, tray_icon_update, NULL);

  g_timeout_add_seconds(5, get_cpu_usage, NULL);

  gtk_main();

  return 0;
}
