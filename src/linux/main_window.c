#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMPUTERS 10
#define MAX_ZONES 10

typedef struct {
  int id;
  char name[50];
  char status[20];
  int zone_id; // -1 if not assigned
} Computer;

typedef struct {
  int id;
  char name[20];
  Computer *assigned_computer;
  GtkWidget *drop_area;
  GtkWidget *label;
} DropZone;

typedef struct {
  Computer computers[MAX_COMPUTERS];
  DropZone zones[MAX_ZONES];
  GtkWidget *computer_list;
  GtkWidget *main_window;
  int computer_count;
  int zone_count;
} AppData;

static AppData app_data;

// Function prototypes
static void init_data(void);
static void setup_ui(GtkApplication *app);
static void on_drag_data_get(GtkWidget *widget, GdkDragContext *context,
                           GtkSelectionData *data, guint info, guint time, gpointer user_data);
static void on_drag_data_received(GtkWidget *widget, GdkDragContext *context,
                                gint x, gint y, GtkSelectionData *data,
                                guint info, guint time, gpointer user_data);
static gboolean on_drag_drop(GtkWidget *widget, GdkDragContext *context,
                            gint x, gint y, guint time, gpointer user_data);
static void on_drag_enter(GtkWidget *widget, GdkDragContext *context,
                         guint time, gpointer user_data);
static void on_drag_leave(GtkWidget *widget, GdkDragContext *context,
                         guint time, gpointer user_data);
static void create_computer_row(GtkWidget *list_box, Computer *computer);
static void create_drop_zone(GtkWidget *grid, int zone_id, int row, int col);
static void update_zone_display(DropZone *zone);
static Computer *find_computer_by_id(int id);
static DropZone *find_zone_by_id(int id);

static void init_data(void) {
  app_data.computer_count = MAX_COMPUTERS;
  app_data.zone_count = MAX_ZONES;
  
  // Initialize computers
  for (int i = 0; i < MAX_COMPUTERS; i++) {
    app_data.computers[i].id = i + 1;
    snprintf(app_data.computers[i].name, sizeof(app_data.computers[i].name), "Computer %d", i + 1);
    strcpy(app_data.computers[i].status, "Available");
    app_data.computers[i].zone_id = -1;
  }
  
  // Initialize zones
  for (int i = 0; i < MAX_ZONES; i++) {
    app_data.zones[i].id = i + 1;
    snprintf(app_data.zones[i].name, sizeof(app_data.zones[i].name), "Zone %d", i + 1);
    app_data.zones[i].assigned_computer = NULL;
  }
}

static void on_drag_data_get(GtkWidget *widget, GdkDragContext *context,
                           GtkSelectionData *data, guint info, guint time, gpointer user_data) {
  Computer *computer = (Computer *)user_data;
  char id_str[16];
  snprintf(id_str, sizeof(id_str), "%d", computer->id);
  gtk_selection_data_set_text(data, id_str, -1);
}

static void on_drag_data_received(GtkWidget *widget, GdkDragContext *context,
                                gint x, gint y, GtkSelectionData *data,
                                guint info, guint time, gpointer user_data) {
  DropZone *zone = (DropZone *)user_data;
  
  if (gtk_selection_data_get_length(data) >= 0) {
    const guchar *text = gtk_selection_data_get_text(data);
    if (text) {
      int computer_id = atoi((char *)text);
      Computer *computer = find_computer_by_id(computer_id);
      
      if (computer) {
        // Remove computer from previous zone
        if (computer->zone_id != -1) {
          DropZone *prev_zone = find_zone_by_id(computer->zone_id);
          if (prev_zone) {
            prev_zone->assigned_computer = NULL;
            update_zone_display(prev_zone);
          }
        }
        
        // Assign to new zone
        computer->zone_id = zone->id;
        zone->assigned_computer = computer;
        update_zone_display(zone);
        
        printf("Computer %s assigned to Zone %d\n", computer->name, zone->id);
      }
    }
  }
  
  gtk_drag_finish(context, TRUE, FALSE, time);
}

static gboolean on_drag_drop(GtkWidget *widget, GdkDragContext *context,
                            gint x, gint y, guint time, gpointer user_data) {
  GdkAtom target = gtk_drag_dest_find_target(widget, context, NULL);
  if (target != GDK_NONE) {
    gtk_drag_get_data(widget, context, target, time);
    return TRUE;
  }
  return FALSE;
}

static void on_drag_enter(GtkWidget *widget, GdkDragContext *context,
                         guint time, gpointer user_data) {
  // Change appearance to indicate valid drop target
  GtkStyleContext *style_context = gtk_widget_get_style_context(widget);
  gtk_style_context_add_class(style_context, "drag-hover");
}

static void on_drag_leave(GtkWidget *widget, GdkDragContext *context,
                         guint time, gpointer user_data) {
  // Restore normal appearance
  GtkStyleContext *style_context = gtk_widget_get_style_context(widget);
  gtk_style_context_remove_class(style_context, "drag-hover");
}

static void create_computer_row(GtkWidget *list_box, Computer *computer) {
  GtkWidget *row = gtk_list_box_row_new();
  GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  GtkWidget *icon = gtk_image_new_from_icon_name("computer", GTK_ICON_SIZE_LARGE_TOOLBAR);
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  GtkWidget *name_label = gtk_label_new(computer->name);
  GtkWidget *status_label = gtk_label_new(computer->status);
  GtkWidget *arrow = gtk_image_new_from_icon_name("go-next", GTK_ICON_SIZE_BUTTON);
  
  // Style the labels
  gtk_widget_set_halign(name_label, GTK_ALIGN_START);
  gtk_widget_set_halign(status_label, GTK_ALIGN_START);
  
  PangoAttrList *attr_list = pango_attr_list_new();
  PangoAttribute *attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
  pango_attr_list_insert(attr_list, attr);
  gtk_label_set_attributes(GTK_LABEL(name_label), attr_list);
  pango_attr_list_unref(attr_list);
  
  // Make status label smaller and gray
  GtkStyleContext *status_context = gtk_widget_get_style_context(status_label);
  gtk_style_context_add_class(status_context, "dim-label");
  
  // Pack widgets
  gtk_box_pack_start(GTK_BOX(vbox), name_label, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 0);
  
  gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(hbox), arrow, FALSE, FALSE, 0);
  
  gtk_container_add(GTK_CONTAINER(row), hbox);
  gtk_container_add(GTK_CONTAINER(list_box), row);
  
  // Set up drag source
  gtk_drag_source_set(row, GDK_BUTTON1_MASK, NULL, 0, GDK_ACTION_MOVE);
  gtk_drag_source_add_text_targets(row);
  
  g_signal_connect(row, "drag-data-get", G_CALLBACK(on_drag_data_get), computer);
  
  // Set row height
  gtk_widget_set_size_request(row, -1, 50);
  
  gtk_widget_show_all(row);
}

static void create_drop_zone(GtkWidget *grid, int zone_id, int row, int col) {
  DropZone *zone = &app_data.zones[zone_id - 1];
  
  GtkWidget *frame = gtk_frame_new(NULL);
  GtkWidget *event_box = gtk_event_box_new();
  GtkWidget *label = gtk_label_new(NULL);
  
  zone->drop_area = event_box;
  zone->label = label;
  
  // Set up the label
  gtk_widget_set_size_request(event_box, 140, 120);
  gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(label, GTK_ALIGN_CENTER);
  
  // Add CSS classes for styling
  GtkStyleContext *frame_context = gtk_widget_get_style_context(frame);
  gtk_style_context_add_class(frame_context, "drop-zone");
  
  GtkStyleContext *event_context = gtk_widget_get_style_context(event_box);
  gtk_style_context_add_class(event_context, "drop-zone-content");
  
  // Set up drag destination
  gtk_drag_dest_set(event_box, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_MOVE);
  gtk_drag_dest_add_text_targets(event_box);
  
  // Connect drag signals
  g_signal_connect(event_box, "drag-data-received", G_CALLBACK(on_drag_data_received), zone);
  g_signal_connect(event_box, "drag-drop", G_CALLBACK(on_drag_drop), zone);
  g_signal_connect(event_box, "drag-motion", G_CALLBACK(on_drag_enter), zone);
  g_signal_connect(event_box, "drag-leave", G_CALLBACK(on_drag_leave), zone);
  
  gtk_container_add(GTK_CONTAINER(event_box), label);
  gtk_container_add(GTK_CONTAINER(frame), event_box);
  gtk_grid_attach(GTK_GRID(grid), frame, col, row, 1, 1);
  
  update_zone_display(zone);
  
  gtk_widget_show_all(frame);
}

static void update_zone_display(DropZone *zone) {
  char text[100];
  if (zone->assigned_computer) {
    snprintf(text, sizeof(text), "%s\n%s", zone->name, zone->assigned_computer->name);
  } else {
    snprintf(text, sizeof(text), "%s\nDrag and drop computer here", zone->name);
  }
  gtk_label_set_text(GTK_LABEL(zone->label), text);
}

static Computer *find_computer_by_id(int id) {
  for (int i = 0; i < app_data.computer_count; i++) {
    if (app_data.computers[i].id == id) {
      return &app_data.computers[i];
    }
  }
  return NULL;
}

static DropZone *find_zone_by_id(int id) {
  for (int i = 0; i < app_data.zone_count; i++) {
    if (app_data.zones[i].id == id) {
      return &app_data.zones[i];
    }
  }
  return NULL;
}

static void setup_ui(GtkApplication *app) {
  GtkWidget *window;
  GtkWidget *main_hbox;
  GtkWidget *left_vbox;
  GtkWidget *right_vbox;
  GtkWidget *computers_label;
  GtkWidget *zones_label;
  GtkWidget *scrolled_window;
  GtkWidget *list_box;
  GtkWidget *zones_grid;
  
  // Create main window
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Computer Management");
  gtk_window_set_default_size(GTK_WINDOW(window), 1100, 600);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  
  app_data.main_window = window;
  
  // Create main horizontal box
  main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  gtk_container_add(GTK_CONTAINER(window), main_hbox);
  
  // Left side - Computers
  left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_size_request(left_vbox, 300, -1);
  
  computers_label = gtk_label_new("Computers");
  PangoAttrList *attr_list = pango_attr_list_new();
  PangoAttribute *attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
  pango_attr_list_insert(attr_list, attr);
  attr = pango_attr_scale_new(1.2);
  pango_attr_list_insert(attr_list, attr);
  gtk_label_set_attributes(GTK_LABEL(computers_label), attr_list);
  pango_attr_list_unref(attr_list);
  gtk_widget_set_halign(computers_label, GTK_ALIGN_START);
  
  // Create scrolled window for computer list
  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scrolled_window, 300, 500);
  
  // Create list box for computers
  list_box = gtk_list_box_new();
  app_data.computer_list = list_box;
  gtk_container_add(GTK_CONTAINER(scrolled_window), list_box);
  
  // Add computers to list
  for (int i = 0; i < app_data.computer_count; i++) {
    create_computer_row(list_box, &app_data.computers[i]);
  }
  
  gtk_box_pack_start(GTK_BOX(left_vbox), computers_label, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(left_vbox), scrolled_window, TRUE, TRUE, 0);
  
  // Right side - Drop Zones
  right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  
  zones_label = gtk_label_new("Drop Zones");
  attr_list = pango_attr_list_new();
  attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
  pango_attr_list_insert(attr_list, attr);
  attr = pango_attr_scale_new(1.2);
  pango_attr_list_insert(attr_list, attr);
  gtk_label_set_attributes(GTK_LABEL(zones_label), attr_list);
  pango_attr_list_unref(attr_list);
  gtk_widget_set_halign(zones_label, GTK_ALIGN_START);
  
  // Create grid for drop zones
  zones_grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(zones_grid), 10);
  gtk_grid_set_column_spacing(GTK_GRID(zones_grid), 10);
  
  // Create drop zones in 2x5 grid
  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < 5; col++) {
      int zone_id = (row * 5) + col + 1;
      create_drop_zone(zones_grid, zone_id, row, col);
    }
  }
  
  gtk_box_pack_start(GTK_BOX(right_vbox), zones_label, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(right_vbox), zones_grid, TRUE, TRUE, 0);
  
  // Pack left and right sides
  gtk_box_pack_start(GTK_BOX(main_hbox), left_vbox, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(main_hbox), right_vbox, TRUE, TRUE, 0);
  
  // Add CSS for styling
  GtkCssProvider *css_provider = gtk_css_provider_new();
  const char *css_data = 
    ".drop-zone { "
    "  border: 2px dashed #cccccc; "
    "  border-radius: 8px; "
    "  background-color: #f5f0e8; "
    "} "
    ".drop-zone-content { "
    "  background-color: transparent; "
    "  padding: 10px; "
    "} "
    ".drag-hover { "
    "  border-color: #4CAF50; "
    "  background-color: #e8f5e8; "
    "} "
    ".dim-label { "
    "  color: #888888; "
    "  font-size: 0.9em; "
    "}";
  
  gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                           GTK_STYLE_PROVIDER(css_provider),
                                           GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  
  gtk_widget_show_all(window);
}

static void activate(GtkApplication *app, gpointer user_data) {
  init_data();
  setup_ui(app);
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;
  
  app = gtk_application_new("com.example.computer-management", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  
  return status;
}