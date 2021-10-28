#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

#define TRAY_APPINDICATOR_ID "runcat-applet"
#define TRAY_ICON                                                              \
  "/home/yongsheng/repos/runcat-tray/icons/cat/my-sleeping-symbolic.svg"
#define LEN(arr) ((int)(sizeof(arr) / sizeof(arr)[0]))

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
    "/home/yongsheng/repos/runcat-tray/icons/cat/my-running-0-symbolic.svg",
    "/home/yongsheng/repos/runcat-tray/icons/cat/my-running-1-symbolic.svg",
    "/home/yongsheng/repos/runcat-tray/icons/cat/my-running-2-symbolic.svg",
    "/home/yongsheng/repos/runcat-tray/icons/cat/my-running-3-symbolic.svg",
    "/home/yongsheng/repos/runcat-tray/icons/cat/my-running-4-symbolic.svg",
};

static int get_time_per_frame() {
  // TODO: parse CPU usage
  int percent = rand() % (100 + 1);
  double diff = FPS_DELTA * percent;
  double time = (double)FPS_30 + diff;
  return (int)time;
}

/* timer callback, will change fps by cpu usage each time */
static gboolean tray_icon_update(gpointer data) {
  AppIndicator *indicator = data;

  app_indicator_set_icon(indicator, FRAMES[COUNTER++ % LEN(FRAMES)]);

  g_timeout_add(get_time_per_frame(), tray_icon_update, indicator);

  return false;
}

int main(int argc, char **argv) {
  srand(time(0));

  GtkWidget *root, *item_cpu, *item_quit;

  AppIndicator *indicator;
  GError *error = NULL;

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

  g_timeout_add(FPS_4, tray_icon_update, indicator);

  // TODO: parse CPU usage, store and display

  gtk_main();

  return 0;
}
