/*                  _             _                 
**   ___ ___  _ __ | |_ _ __ ___ | |    __   __ _   
**  / __/ _ \| '_ \| __| '__/ _ \| |____\ \ / /| |_ 
** | (_| (_) | | | | |_| | | (_) | |_____\ V /_   _|
**  \___\___/|_| |_|\__|_|  \___/|_|      \_/  |_|  
**
*/
#include <gtk/gtk.h>
#include <libayatana-appindicator/app-indicator.h>

static void menu_item_activated(GtkMenuItem *item, gpointer data) 
{
  g_print("Menu item '%s' activated\n", (char *)data);
}

/*
**
*/
static void create_menu(AppIndicator *indicator) {
  GtkWidget *menu = gtk_menu_new();
  
  // Add a menu item
  GtkWidget *item = gtk_menu_item_new_with_label("Show Window");
  g_signal_connect(item, "activate", G_CALLBACK(menu_item_activated), "Show Window");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  gtk_widget_show(item);
  
  // Add a separator
  item = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  gtk_widget_show(item);
  
  // Add quit item
  item = gtk_menu_item_new_with_label("Quit");
  g_signal_connect(item, "activate", G_CALLBACK(gtk_main_quit), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  gtk_widget_show(item);
  
  // Set the menu
  app_indicator_set_menu(indicator, GTK_MENU(menu));
}

int main(int argc, char **argv) 
{
  GtkWidget* window;
  GtkWidget* button;
  GtkWidget* box;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Ctrl+V Plus");
  gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    
  GtkWidget* box_left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

  // Create a vertical box container
  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add(GTK_CONTAINER(window), box);

  // Create a button
  button = gtk_button_new_with_label("Click Me!");
  // g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), window);
    
  // Add button to the box
  gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

  // Create a label
  GtkWidget* label = gtk_label_new("Drag and drop files here");
  gtk_widget_set_margin_top(label, 20);
  gtk_widget_set_margin_bottom(label, 20);
  gtk_widget_set_margin_start(label, 20);
  gtk_widget_set_margin_end(label, 20);
  gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);

  gtk_drag_dest_set(
    GTK_WIDGET(window), 
    GTK_DEST_DEFAULT_ALL, 
    NULL, 
    0, 
    GDK_ACTION_COPY | GDK_ACTION_MOVE
  );

  // Show all widgets
  gtk_widget_show_all(window);
  
  AppIndicator *indicator = app_indicator_new(
    "example-tray-app",  // Unique identifier
    "dialog-information",  // Default icon name
    APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  
  // Set the status (active/passive)
  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_icon_full(indicator, "/export/local/works/doublegsoft.dev/control-v-plus/03.Development/control-v-plus/res/logo.png", "description");
  
  // Set attention icon (when needs attention)
  app_indicator_set_attention_icon(indicator, "dialog-warning");
  
  // Create the menu
  create_menu(indicator);
  
  // You can also set a custom icon path
  // app_indicator_set_icon_full(indicator, "/path/to/icon.png", "description");
  
  // Set a tooltip
  app_indicator_set_title(indicator, "My GNOME Tray App");
  
  gtk_main();
  return 0;
}