#include "../include/framework.h"
#include <stdio.h>

static void on_maximize_toggle(GtkButton* button, gpointer user_data) {
    (void)button;
    GtkWindow *window = GTK_WINDOW(user_data);
    if (gtk_window_is_maximized(window)) {
        gtk_window_unmaximize(window);
    } else {
        gtk_window_maximize(window);
    }
}

GtkWidget* create_window(const char* title, const char* html_path, const char* css_path, int width, int height, void (*on_js_message_cb)(WebKitUserContentManager*, WebKitJavascriptResult*, gpointer)) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);

    GtkHeaderBar *header_bar = GTK_HEADER_BAR(gtk_header_bar_new());
    gtk_header_bar_set_show_close_button(header_bar, FALSE);
    gtk_header_bar_set_title(header_bar, title);
    gtk_widget_show(GTK_WIDGET(header_bar));
    gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(header_bar));

    GtkWidget *close_btn = gtk_button_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_BUTTON);
    g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(gtk_widget_destroy), window);
    gtk_header_bar_pack_end(header_bar, close_btn);

    GtkWidget *maximize_btn = gtk_button_new_from_icon_name("window-maximize-symbolic", GTK_ICON_SIZE_BUTTON);
    g_signal_connect(maximize_btn, "clicked", G_CALLBACK(on_maximize_toggle), window);
    gtk_header_bar_pack_end(header_bar, maximize_btn);

    GtkWidget *minimize_btn = gtk_button_new_from_icon_name("window-minimize-symbolic", GTK_ICON_SIZE_BUTTON);
    g_signal_connect_swapped(minimize_btn, "clicked", G_CALLBACK(gtk_window_iconify), window);
    gtk_header_bar_pack_end(header_bar, minimize_btn);

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
    
    g_signal_connect(manager, "script-message-received::mochaBridge", G_CALLBACK(on_js_message_cb), webview);

    char uri[1024];
    snprintf(uri, sizeof(uri), "file://%s", html_path);
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), uri);
    
    gtk_container_add(GTK_CONTAINER(window), webview);

    return window;
}
