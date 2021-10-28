#include <stdbool.h>

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

#define TRAY_APPINDICATOR_ID "runcat-applet"
#define TRAY_ICON "/home/yongsheng/repos/runcat-tray/icons/runcat-logo.png"

static void button_clicked(GtkWidget *widget, gpointer data) {
  g_print("clicked\n");
}

int main(int argc, char **argv) {
  GtkWidget *root, *item_cpu, *item_quit;

  AppIndicator *indicator;
  GError *error = NULL;

  gtk_init(&argc, &argv);

  /* Indicator */
  indicator = app_indicator_new(TRAY_APPINDICATOR_ID, TRAY_ICON,
                                APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);

  // app_indicator_set_icon(indicator, TRAY_ICON);

  root = gtk_menu_new();

  item_cpu = gtk_menu_item_new_with_label("CPU");
  g_signal_connect(item_cpu, "activate", G_CALLBACK(button_clicked), NULL);

  gtk_menu_shell_insert(GTK_MENU_SHELL(root), item_cpu, 0);

  app_indicator_set_menu(indicator, GTK_MENU(root));
  gtk_widget_show_all(root);

  gtk_main();

  return 0;
}
