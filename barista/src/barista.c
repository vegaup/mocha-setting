#include "../include/barista.h"
#include <stdio.h>

static void on_maximize_toggle(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    (void)widget;
    (void)event;
    GtkWindow *window = GTK_WINDOW(user_data);
    if (gtk_window_is_maximized(window)) {
        gtk_window_unmaximize(window);
    } else {
        gtk_window_maximize(window);
    }
}

static void on_minimize(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    (void)widget;
    (void)event;
    GtkWindow *window = GTK_WINDOW(user_data);
    gtk_window_iconify(window);
}

static void on_close(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    (void)widget;
    (void)event;
    GtkWindow *window = GTK_WINDOW(user_data);
    gtk_widget_destroy(GTK_WIDGET(window));
}


GtkWidget* create_window(const char* title, const char* html_path, const char* css_path, int width, int height) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);

    GtkHeaderBar *header_bar = GTK_HEADER_BAR(gtk_header_bar_new());
    gtk_header_bar_set_show_close_button(header_bar, FALSE);
    gtk_header_bar_set_title(header_bar, title);
    gtk_widget_show(GTK_WIDGET(header_bar));
    gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(header_bar));

    GtkWidget *close_box = gtk_event_box_new();
    gtk_widget_set_has_window(close_box, TRUE);
    gtk_widget_set_name(close_box, "close-btn");
    GtkWidget *close_fixed = gtk_fixed_new();
    GtkWidget *close_label = gtk_label_new("×");
    gtk_fixed_put(GTK_FIXED(close_fixed), close_label, 8, 2);
    gtk_container_add(GTK_CONTAINER(close_box), close_fixed);
    g_signal_connect(close_box, "button-press-event", G_CALLBACK(on_close), window);
    gtk_header_bar_pack_end(header_bar, close_box);

    GtkWidget *maximize_box = gtk_event_box_new();
    gtk_widget_set_has_window(maximize_box, TRUE);
    gtk_widget_set_name(maximize_box, "maximize-btn");
    GtkWidget *maximize_fixed = gtk_fixed_new();
    GtkWidget *maximize_label = gtk_label_new("□");
    gtk_fixed_put(GTK_FIXED(maximize_fixed), maximize_label, 7, 3);
    gtk_container_add(GTK_CONTAINER(maximize_box), maximize_fixed);
    g_signal_connect(maximize_box, "button-press-event", G_CALLBACK(on_maximize_toggle), window);
    gtk_header_bar_pack_end(header_bar, maximize_box);

    GtkWidget *minimize_box = gtk_event_box_new();
    gtk_widget_set_has_window(minimize_box, TRUE);
    gtk_widget_set_name(minimize_box, "minimize-btn");
    GtkWidget *minimize_fixed = gtk_fixed_new();
    GtkWidget *minimize_label = gtk_label_new("−");
    gtk_fixed_put(GTK_FIXED(minimize_fixed), minimize_label, 8, 2);
    gtk_container_add(GTK_CONTAINER(minimize_box), minimize_fixed);
    g_signal_connect(minimize_box, "button-press-event", G_CALLBACK(on_minimize), window);
    gtk_header_bar_pack_end(header_bar, minimize_box);

    GError *error = NULL;
    GtkCssProvider *provider = gtk_css_provider_new();
    if (!gtk_css_provider_load_from_path(provider, css_path, &error)) {
        fprintf(stderr, "Failed to load CSS: %s\n", error->message);
        g_error_free(error);
    } else {
        GdkScreen *screen = gdk_screen_get_default();
        gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    }

    WebKitSettings *settings = webkit_settings_new();
    webkit_settings_set_enable_developer_extras(settings, TRUE);

    WebKitUserContentManager *manager = webkit_user_content_manager_new();
    webkit_user_content_manager_register_script_message_handler(manager, "mochaBridge");

    GtkWidget *webview = webkit_web_view_new_with_user_content_manager(manager);
    webkit_web_view_set_settings(WEBKIT_WEB_VIEW(webview), settings);
    
    g_signal_connect(manager, "script-message-received::mochaBridge", G_CALLBACK(bridge_handle_message), webview);

    char uri[1024];
    snprintf(uri, sizeof(uri), "file://%s", html_path);
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), uri);
    
    gtk_container_add(GTK_CONTAINER(window), webview);

    return window;
}