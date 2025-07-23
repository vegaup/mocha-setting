#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

GtkWidget* create_window(const char* title, const char* html_path, const char* css_path, int width, int height, void (*on_js_message_cb)(WebKitUserContentManager*, WebKitJavascriptResult*, gpointer));

#endif // FRAMEWORK_H 