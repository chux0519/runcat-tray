#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

#define TRAY_APPINDICATOR_ID "runcat-applet"
#define TRAY_ICON                                                              \
  "/home/yongsheng/repos/runcat-tray/icons/cat/my-sleeping-symbolic.svg"

uint64_t COUNTER = 0;
int FPS_60 = 33;
int FPS_4 = 250;

static char *frames[] = {
    "/home/yongsheng/repos/runcat-tray/icons/cat/my-running-0-symbolic.svg",
    "/home/yongsheng/repos/runcat-tray/icons/cat/my-running-1-symbolic.svg",
    "/home/yongsheng/repos/runcat-tray/icons/cat/my-running-2-symbolic.svg",
    "/home/yongsheng/repos/runcat-tray/icons/cat/my-running-3-symbolic.svg",
    "/home/yongsheng/repos/runcat-tray/icons/cat/my-running-4-symbolic.svg",
};

static void button_clicked(GtkWidget *widget, gpointer data) {
  g_print("clicked\n");
}

static int get_time_per_frame() {
  // TODO: parse CPU usage and calculate
  return (rand() % (FPS_4 - FPS_60 + 1)) + FPS_60;
}

static gboolean tray_icon_update(gpointer data) {
  AppIndicator *indicator = data;

  app_indicator_set_icon(indicator, frames[COUNTER++ % 5]);

  g_timeout_add(get_time_per_frame(), tray_icon_update, indicator);

  return false;
}

int main(int argc, char **argv) {
  srand(time(0));

  GtkWidget *root, *item_cpu, *item_quit;

  AppIndicator *indicator;
  GError *error = NULL;

  gtk_init(&argc, &argv);

  /* Indicator */
  indicator = app_indicator_new(TRAY_APPINDICATOR_ID, TRAY_ICON,
                                APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);

  root = gtk_menu_new();

  // TODO: parse CPU usage and display
  item_cpu = gtk_menu_item_new_with_label("cpu");
  g_signal_connect(item_cpu, "activate", G_CALLBACK(button_clicked), NULL);

  item_quit = gtk_menu_item_new_with_label("quit");
  g_signal_connect(item_quit, "activate", G_CALLBACK(gtk_main_quit), NULL);

  gtk_menu_shell_insert(GTK_MENU_SHELL(root), item_cpu, 0);

  gtk_menu_shell_insert(GTK_MENU_SHELL(root), item_quit, 1);

  app_indicator_set_menu(indicator, GTK_MENU(root));
  gtk_widget_show_all(root);

  g_timeout_add(250, tray_icon_update, indicator);

  gtk_main();

  return 0;
}
