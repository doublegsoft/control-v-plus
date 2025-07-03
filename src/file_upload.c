#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <string.h>

typedef struct {
  GtkWidget *dialog;
  GtkWidget *drop_area;
  GtkWidget *format_label;
  GtkWidget *done_button;
  GtkWidget *cancel_button;
  char *selected_file;
} UploadDialog;

static void on_file_selected(GtkFileChooserButton *button, gpointer user_data) {
  UploadDialog *upload_dialog = (UploadDialog *)user_data;
  gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(button));
  
  if (filename) {
    g_free(upload_dialog->selected_file);
    upload_dialog->selected_file = g_strdup(filename);
    
    // Enable the Done button when a file is selected
    gtk_widget_set_sensitive(upload_dialog->done_button, TRUE);
    
    // Update the format label to show selected file
    gchar *basename = g_path_get_basename(filename);
    gchar *label_text = g_strdup_printf("Selected: %s", basename);
    gtk_label_set_text(GTK_LABEL(upload_dialog->format_label), label_text);
    
    g_free(basename);
    g_free(label_text);
    g_free(filename);
  }
}

static void on_browse_clicked(GtkButton *button, gpointer user_data) {
  UploadDialog *upload_dialog = (UploadDialog *)user_data;
  
  GtkWidget *file_dialog = gtk_file_chooser_dialog_new(
    "Choose Image File",
    GTK_WINDOW(upload_dialog->dialog),
    GTK_FILE_CHOOSER_ACTION_OPEN,
    "_Cancel", GTK_RESPONSE_CANCEL,
    "_Open", GTK_RESPONSE_ACCEPT,
    NULL
  );
  
  // Set up file filters
  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "Image files");
  gtk_file_filter_add_pattern(filter, "*.jpg");
  gtk_file_filter_add_pattern(filter, "*.jpeg");
  gtk_file_filter_add_pattern(filter, "*.png");
  gtk_file_filter_add_mime_type(filter, "image/jpeg");
  gtk_file_filter_add_mime_type(filter, "image/png");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_dialog), filter);
  
  gint response = gtk_dialog_run(GTK_DIALOG(file_dialog));
  
  if (response == GTK_RESPONSE_ACCEPT) {
    gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_dialog));
    
    if (filename) {
      // Check file size (25MB limit)
      // GStatBuf stat_buf;
      // if (g_stat(filename, &stat_buf) == 0) {
      //   if (stat_buf.st_size > 25 * 1024 * 1024) {
      //     GtkWidget *error_dialog = gtk_message_dialog_new(
      //       GTK_WINDOW(upload_dialog->dialog),
      //       GTK_DIALOG_MODAL,
      //       GTK_MESSAGE_ERROR,
      //       GTK_BUTTONS_OK,
      //       "File size exceeds 25MB limit"
      //     );
      //     gtk_dialog_run(GTK_DIALOG(error_dialog));
      //     gtk_widget_destroy(error_dialog);
      //     g_free(filename);
      //     gtk_widget_destroy(file_dialog);
      //     return;
      //   }
      // }
      
      g_free(upload_dialog->selected_file);
      upload_dialog->selected_file = g_strdup(filename);
      
      // Enable the Done button
      gtk_widget_set_sensitive(upload_dialog->done_button, TRUE);
      
      // Update the format label
      gchar *basename = g_path_get_basename(filename);
      gchar *label_text = g_strdup_printf("Selected: %s", basename);
      gtk_label_set_text(GTK_LABEL(upload_dialog->format_label), label_text);
      
      g_free(basename);
      g_free(label_text);
      g_free(filename);
    }
  }
  
  gtk_widget_destroy(file_dialog);
}

static void on_done_clicked(GtkButton *button, gpointer user_data) {
  UploadDialog *upload_dialog = (UploadDialog *)user_data;
  
  if (upload_dialog->selected_file) {
    g_print("File selected: %s\n", upload_dialog->selected_file);
    // Here you would typically handle the file upload
  }
  
  gtk_dialog_response(GTK_DIALOG(upload_dialog->dialog), GTK_RESPONSE_OK);
}

static void on_cancel_clicked(GtkButton *button, gpointer user_data) {
  UploadDialog *upload_dialog = (UploadDialog *)user_data;
  gtk_dialog_response(GTK_DIALOG(upload_dialog->dialog), GTK_RESPONSE_CANCEL);
}

// Drag and drop handlers
static gboolean on_drag_motion(GtkWidget *widget, GdkDragContext *context,
                               gint x, gint y, guint time, gpointer user_data) {
  gdk_drag_status(context, GDK_ACTION_COPY, time);
  return TRUE;
}

static void on_drag_data_received(GtkWidget *widget, GdkDragContext *context,
                                  gint x, gint y, GtkSelectionData *data,
                                  guint info, guint time, gpointer user_data) {
  UploadDialog *upload_dialog = (UploadDialog *)user_data;
  
  gchar **uris = gtk_selection_data_get_uris(data);
  if (uris && uris[0]) {
    gchar *filename = g_filename_from_uri(uris[0], NULL, NULL);
    
    if (filename) {
      // Check if it's an image file
      gchar *lower_filename = g_ascii_strdown(filename, -1);
      if (g_str_has_suffix(lower_filename, ".jpg") ||
          g_str_has_suffix(lower_filename, ".jpeg") ||
          g_str_has_suffix(lower_filename, ".png")) {
        
        // Check file size
        // GStatBuf stat_buf;
        // if (g_stat(filename, &stat_buf) == 0) {
        //   if (stat_buf.st_size > 25 * 1024 * 1024) {
        //     GtkWidget *error_dialog = gtk_message_dialog_new(
        //       GTK_WINDOW(upload_dialog->dialog),
        //       GTK_DIALOG_MODAL,
        //       GTK_MESSAGE_ERROR,
        //       GTK_BUTTONS_OK,
        //       "File size exceeds 25MB limit"
        //     );
        //     gtk_dialog_run(GTK_DIALOG(error_dialog));
        //     gtk_widget_destroy(error_dialog);
        //     g_free(lower_filename);
        //     g_free(filename);
        //     g_strfreev(uris);
        //     gtk_drag_finish(context, FALSE, FALSE, time);
        //     return;
        //   }
        // }
        
        g_free(upload_dialog->selected_file);
        upload_dialog->selected_file = g_strdup(filename);
        
        // Enable the Done button
        gtk_widget_set_sensitive(upload_dialog->done_button, TRUE);
        
        // Update the format label
        gchar *basename = g_path_get_basename(filename);
        gchar *label_text = g_strdup_printf("Selected: %s", basename);
        gtk_label_set_text(GTK_LABEL(upload_dialog->format_label), label_text);
        
        g_free(basename);
        g_free(label_text);
        gtk_drag_finish(context, TRUE, FALSE, time);
      } else {
        gtk_drag_finish(context, FALSE, FALSE, time);
      }
      g_free(lower_filename);
      g_free(filename);
    }
    g_strfreev(uris);
  }
}

UploadDialog *create_upload_dialog(GtkWindow *parent) {
  UploadDialog *upload_dialog = g_new0(UploadDialog, 1);
  
  // Create the dialog
  upload_dialog->dialog = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(upload_dialog->dialog), "Upload Thumbnail");
  gtk_window_set_transient_for(GTK_WINDOW(upload_dialog->dialog), parent);
  gtk_window_set_modal(GTK_WINDOW(upload_dialog->dialog), TRUE);
  gtk_window_set_default_size(GTK_WINDOW(upload_dialog->dialog), 400, 300);
  gtk_window_set_resizable(GTK_WINDOW(upload_dialog->dialog), FALSE);
  
  // Get the content area
  GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(upload_dialog->dialog));
  gtk_container_set_border_width(GTK_CONTAINER(content_area), 20);
  
  // Create main vertical box
  GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_container_add(GTK_CONTAINER(content_area), main_vbox);
  
  // Title label
  GtkWidget *title_label = gtk_label_new("Upload Thumbnail");
  gtk_widget_set_name(title_label, "title-label");
  gtk_label_set_xalign(GTK_LABEL(title_label), 0.0);
  PangoAttrList *attrs = pango_attr_list_new();
  pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
  pango_attr_list_insert(attrs, pango_attr_scale_new(1.2));
  gtk_label_set_attributes(GTK_LABEL(title_label), attrs);
  pango_attr_list_unref(attrs);
  gtk_box_pack_start(GTK_BOX(main_vbox), title_label, FALSE, FALSE, 0);
  
  // Description label
  GtkWidget *desc_label = gtk_label_new("Please upload file in jpeg or png format and make sure\nthe file size is under 25 MB.");
  gtk_label_set_xalign(GTK_LABEL(desc_label), 0.0);
  gtk_box_pack_start(GTK_BOX(main_vbox), desc_label, FALSE, FALSE, 0);
  
  // Drop area frame
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
  gtk_widget_set_size_request(frame, -1, 150);
  
  // Drop area
  upload_dialog->drop_area = gtk_event_box_new();
  gtk_container_add(GTK_CONTAINER(frame), upload_dialog->drop_area);
  
  // Set up drag and drop
  gtk_drag_dest_set(upload_dialog->drop_area, GTK_DEST_DEFAULT_ALL,
                    NULL, 0, GDK_ACTION_COPY);
  gtk_drag_dest_add_uri_targets(upload_dialog->drop_area);
  
  g_signal_connect(upload_dialog->drop_area, "drag-motion",
                   G_CALLBACK(on_drag_motion), upload_dialog);
  g_signal_connect(upload_dialog->drop_area, "drag-data-received",
                   G_CALLBACK(on_drag_data_received), upload_dialog);
  
  // Drop area content
  GtkWidget *drop_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_halign(drop_vbox, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(drop_vbox, GTK_ALIGN_CENTER);
  gtk_container_add(GTK_CONTAINER(upload_dialog->drop_area), drop_vbox);
  
  // Cloud icon (using a simple label as placeholder)
  GtkWidget *icon_label = gtk_label_new("☁");
  PangoAttrList *icon_attrs = pango_attr_list_new();
  pango_attr_list_insert(icon_attrs, pango_attr_scale_new(3.0));
  pango_attr_list_insert(icon_attrs, pango_attr_foreground_new(40000, 40000, 40000));
  gtk_label_set_attributes(GTK_LABEL(icon_label), icon_attrs);
  pango_attr_list_unref(icon_attrs);
  gtk_box_pack_start(GTK_BOX(drop_vbox), icon_label, FALSE, FALSE, 0);
  
  // Drop text
  GtkWidget *drop_label = gtk_label_new("Drop file or browse");
  PangoAttrList *drop_attrs = pango_attr_list_new();
  pango_attr_list_insert(drop_attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
  gtk_label_set_attributes(GTK_LABEL(drop_label), drop_attrs);
  pango_attr_list_unref(drop_attrs);
  gtk_box_pack_start(GTK_BOX(drop_vbox), drop_label, FALSE, FALSE, 0);
  
  // Format label
  upload_dialog->format_label = gtk_label_new("Format: jpeg, png & Max file size: 25 MB");
  gtk_box_pack_start(GTK_BOX(drop_vbox), upload_dialog->format_label, FALSE, FALSE, 0);
  
  // Browse button
  GtkWidget *browse_button = gtk_button_new_with_label("Browse Files");
  gtk_widget_set_size_request(browse_button, 120, 35);
  
  // Style the browse button
  GtkCssProvider *css_provider = gtk_css_provider_new();
  gtk_css_provider_load_from_data(css_provider,
    "button { "
    "  background: #8B2893; "
    "  color: white; "
    "  border: none; "
    "  border-radius: 4px; "
    "  font-weight: bold; "
    "}"
    "button:hover { "
    "  background: #7A1F82; "
    "}"
    ".title-label { "
    "  color: #333; "
    "}", -1, NULL);
  
  GtkStyleContext *context = gtk_widget_get_style_context(browse_button);
  gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider),
                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  
  g_signal_connect(browse_button, "clicked", G_CALLBACK(on_browse_clicked), upload_dialog);
  gtk_box_pack_start(GTK_BOX(drop_vbox), browse_button, FALSE, FALSE, 0);
  
  gtk_box_pack_start(GTK_BOX(main_vbox), frame, TRUE, TRUE, 0);
  
  // Button box
  GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(button_box, GTK_ALIGN_END);
  gtk_box_pack_start(GTK_BOX(main_vbox), button_box, FALSE, FALSE, 0);
  
  // Cancel button
  upload_dialog->cancel_button = gtk_button_new_with_label("Cancel");
  gtk_widget_set_size_request(upload_dialog->cancel_button, 80, 35);
  g_signal_connect(upload_dialog->cancel_button, "clicked",
                   G_CALLBACK(on_cancel_clicked), upload_dialog);
  gtk_box_pack_start(GTK_BOX(button_box), upload_dialog->cancel_button, FALSE, FALSE, 0);
  
  // Done button
  upload_dialog->done_button = gtk_button_new_with_label("Done");
  gtk_widget_set_size_request(upload_dialog->done_button, 80, 35);
  gtk_widget_set_sensitive(upload_dialog->done_button, FALSE); // Initially disabled
  
  // Style the done button
  context = gtk_widget_get_style_context(upload_dialog->done_button);
  gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider),
                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  
  g_signal_connect(upload_dialog->done_button, "clicked",
                   G_CALLBACK(on_done_clicked), upload_dialog);
  gtk_box_pack_start(GTK_BOX(button_box), upload_dialog->done_button, FALSE, FALSE, 0);
  
  g_object_unref(css_provider);
  
  // Set up the dashed border style for the frame
  GtkCssProvider *frame_css = gtk_css_provider_new();
  gtk_css_provider_load_from_data(frame_css,
    "frame { "
    "  border: 2px dashed #8B2893; "
    "  border-radius: 8px; "
    "  background: #F8F4F9; "
    "}", -1, NULL);
  
  context = gtk_widget_get_style_context(frame);
  gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(frame_css),
                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(frame_css);
  
  return upload_dialog;
}

void free_upload_dialog(UploadDialog *upload_dialog) {
  if (upload_dialog) {
    g_free(upload_dialog->selected_file);
    g_free(upload_dialog);
  }
}

static void widget_callback(GtkWidget *widget, gpointer user_data) {
  GtkWindow *parent = GTK_WINDOW(widget);
  UploadDialog *upload_dialog = create_upload_dialog(parent);
  
  gtk_widget_show_all(upload_dialog->dialog);
  gint response = gtk_dialog_run(GTK_DIALOG(upload_dialog->dialog));
  
  if (response == GTK_RESPONSE_OK) {
    g_print("Upload completed!\n");
  } else {
    g_print("Upload cancelled.\n");
  }
  
  gtk_widget_destroy(upload_dialog->dialog);
  free_upload_dialog(upload_dialog);
}


// Example usage
int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);
  
  GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(main_window), "Main Window");
  gtk_window_set_default_size(GTK_WINDOW(main_window), 600, 400);
  
  g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  
  GtkWidget *button = gtk_button_new_with_label("Open Upload Dialog");
  gtk_container_add(GTK_CONTAINER(main_window), button);
  
  g_signal_connect(button, "clicked", G_CALLBACK(widget_callback), main_window);
  
  gtk_widget_show_all(main_window);
  gtk_main();
  
  return 0;
}