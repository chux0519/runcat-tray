#include <dirent.h>
#include <pwd.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

static int FPS_H = 90;
static int FPS_L = 6;

static char FRAMES_DIR[1024] = {};
/* no more than 30 frames */
static char FRAMES[30][256] = {};
static int FRAMES_COUNT = 0;

/* current frame, in full path */
static char FRAME_NOW[1024] = {};

static char *MODES[256] = {"cat", "dab", "mona", "partyblobcat"};
static int MODE = 0;

static void init_frames();

/* sample rate is (always) 100HZ, check sysconf(_SC_CLK_TCK), no need to be
 * higher */
#define SAMPLE_RATE 100.0
#define FPS_DELTA                                                              \
  (((1000.0 / (double)FPS_L) - (1000.0 / (double)FPS_H)) / SAMPLE_RATE)
#define TRAY_APPINDICATOR_ID "runcat-applet"
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

/* global, avoid to pass around by now */
GtkWidget *root, *item_cpu, *item_quit, *item_mode_menu;
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
        ((double)CPU_USAGE.total_b - (double)CPU_USAGE.total_a) / 1.0 /
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
  strcpy(FRAME_NOW, FRAMES_DIR);
  strcat(FRAME_NOW, "/");
  strcat(FRAME_NOW, FRAMES[COUNTER++ % FRAMES_COUNT]);

  app_indicator_set_icon(indicator, FRAME_NOW);

  double diff = FPS_DELTA * CPU_USAGE.percent;
  double time = (1000 / (double)FPS_L - diff);
  g_timeout_add((int)time, tray_icon_update, NULL);

  return false;
}

static void on_mode_change(GtkWidget *_, const char *mode) {
  for (int i = 0; i < 4; ++i) {
    if (strcmp(MODES[i], mode) == 0) {
      MODE = i;
      break;
    }
  }

  char clear[1024] = {};
  memcpy(FRAMES_DIR, clear, 1024);
  init_frames();
}

static void init_frames() {
  struct dirent *de;
  DIR *dir;

  if (strlen(FRAMES_DIR) == 0) {
    const char *home;
    if ((home = getenv("HOME")) == NULL) {
      home = getpwuid(getuid())->pw_dir;
    }
    strcpy(FRAMES_DIR, home);
    strcat(FRAMES_DIR, "/.config/runcat/icons/");
    strcat(FRAMES_DIR, MODES[MODE]);
  }

  dir = opendir(FRAMES_DIR);
  if (dir == NULL) {
    printf("Cannot open directory '%s'\n", FRAMES_DIR);
    exit(-1);
  }

  FRAMES_COUNT = 0;
  while ((de = readdir(dir)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
      continue;
    }
    strcpy(FRAMES[FRAMES_COUNT++], de->d_name);
  }

  /* sort in aplhabetical order */
  for (int i = 0; i < FRAMES_COUNT; i++) {
    for (int j = i + 1; j < FRAMES_COUNT; j++) {
      if (strcmp(FRAMES[i], FRAMES[j]) > 0) {
        char temp[25];
        strcpy(temp, FRAMES[i]);
        strcpy(FRAMES[i], FRAMES[j]);
        strcpy(FRAMES[j], temp);
      }
    }
  }

  closedir(dir);
}

int main(int argc, char **argv) {
  /* load assets */
  init_frames();

  /* parse flags */
  int opt = 0;
  while ((opt = getopt(argc, argv, "hl:u:d:")) != -1) {
    switch (opt) {
    case 'h':
      printf("Usage: runcat -l $min_fps -u $max_fps -d "
             "/path/to/icons/root \n");
      printf("\n");
      printf("-l: lower bound of fps, default to 6.\n");
      printf("-h: upper bound of fps, default to 90.\n");
      printf("-d: root of your animated icons, by default, use `%s`.\n",
             FRAMES_DIR);
      return 0;
    case 'l':
      FPS_L = atoi(optarg);
      break;
    case 'u':
      FPS_H = atoi(optarg);
      break;
    case 'd':
      strcpy(FRAMES_DIR, optarg);
    }
  }
  if (FPS_H < FPS_L) {
    printf("invalid fps range");
    return -1;
  }

  /* init rands and ui */
  srand(time(0));

  gtk_init(&argc, &argv);

  indicator = app_indicator_new(TRAY_APPINDICATOR_ID, FRAMES[0],
                                APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);

  root = gtk_menu_new();

  /* mode submenu */
  item_mode_menu = gtk_menu_item_new_with_label("mode");
  GtkWidget *menu_mode = gtk_menu_new();
  for (int i = 0; i < 4; ++i) {
    GtkWidget *item = gtk_check_menu_item_new_with_label(MODES[i]);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), i == MODE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_mode), item);

    g_signal_connect(item, "activate", G_CALLBACK(on_mode_change), MODES[i]);
  }
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item_mode_menu), menu_mode);

  /* cpu info */
  item_cpu = gtk_menu_item_new_with_label("cpu");

  /* quit button */
  item_quit = gtk_menu_item_new_with_label("quit");
  g_signal_connect(item_quit, "activate", G_CALLBACK(gtk_main_quit), NULL);

  /* add widgets to root */
  gtk_menu_shell_append(GTK_MENU_SHELL(root), item_mode_menu);

  GtkWidget *sperator = gtk_separator_menu_item_new();

  gtk_menu_shell_append(GTK_MENU_SHELL(root), sperator);
  gtk_menu_shell_append(GTK_MENU_SHELL(root), item_cpu);
  gtk_menu_shell_append(GTK_MENU_SHELL(root), item_quit);

  app_indicator_set_menu(indicator, GTK_MENU(root));
  gtk_widget_show_all(root);

  /* timer for updating ui */
  g_timeout_add(1.0 / (double)FPS_L, tray_icon_update, NULL);

  /* timer for updating cpu usage */
  g_timeout_add_seconds(1, get_cpu_usage, NULL);

  /* loop here */
  gtk_main();

  return 0;
}
