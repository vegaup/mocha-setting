#ifndef BARISTA_H
#define BARISTA_H

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include "bridge.h"

GtkWidget* create_window(const char* title, const char* html_path, const char* css_path, int width, int height);

#endif // BARISTA_H
