#define _POSIX_C_SOURCE 200809L

#include <gtk/gtk.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <webkit2/webkit2.h>
#include "../include/main.h"
#include "../third_party/cJSON.h"
#include <pwd.h>
#include <glib.h>

static struct Config config;

static void on_maximize_toggle(GtkButton* button, gpointer user_data) {
    (void)button;
    GtkWindow *window = GTK_WINDOW(user_data);
    if (gtk_window_is_maximized(window)) {
        gtk_window_unmaximize(window);
    } else {
        gtk_window_maximize(window);
    }
}

static char *get_config_path() {
    const char *home = getenv("HOME");
    if (!home) home = getpwuid(getuid())->pw_dir;
    static char path[512];
    snprintf(path, sizeof(path), "%s/.config/mocha/config.mconf", home);
    return path;
}

static char *get_features_path() {
    const char *home = getenv("HOME");
    if (!home) home = getpwuid(getuid())->pw_dir;
    static char path[512];
    snprintf(path, sizeof(path), "%s/.config/mocha/features.mconf", home);
    return path;
}

static char *get_theme_path() {
    const char *home = getenv("HOME");
    if (!home) home = getpwuid(getuid())->pw_dir;
    static char path[512];
    snprintf(path, sizeof(path), "%s/.config/mocha/theme.mconf", home);
    return path;
}
static char *get_keybinds_path() {
    const char *home = getenv("HOME");
    if (!home) home = getpwuid(getuid())->pw_dir;
    static char path[512];
    snprintf(path, sizeof(path), "%s/.config/mocha/keybinds.mconf", home);
    return path;
}

static void send_config_to_js(WebKitWebView *webview) {
    cJSON *cfg = cJSON_CreateObject();
    cJSON_AddNumberToObject(cfg, "tiling", config.tiling);
    cJSON_AddNumberToObject(cfg, "quotes", config.quotes);
    cJSON *colors = cJSON_CreateObject();
    cJSON_AddStringToObject(colors, "border", config.colors.border);
    cJSON_AddStringToObject(colors, "focus", config.colors.focus);
    cJSON_AddStringToObject(colors, "panel", config.colors.panel);
    cJSON_AddStringToObject(colors, "acent", config.colors.acent);
    cJSON_AddItemToObject(cfg, "colors", colors);
    cJSON_AddStringToObject(cfg, "launcher_cmd", config.launcher_cmd);
    cJSON *exec_once = cJSON_CreateArray();
    for (int i = 0; i < config.num_exec_once; ++i)
        cJSON_AddItemToArray(exec_once, cJSON_CreateString(config.exec_once[i]));
    cJSON_AddItemToObject(cfg, "exec_once", exec_once);
    cJSON *keybinds = cJSON_CreateArray();
    for (int i = 0; i < config.num_keybinds; ++i) {
        cJSON *kb = cJSON_CreateObject();
        cJSON_AddStringToObject(kb, "key", config.keybinds[i].key);
        cJSON_AddStringToObject(kb, "action", config.keybinds[i].action);
        cJSON_AddItemToArray(keybinds, kb);
    }
    cJSON_AddItemToObject(cfg, "keybinds", keybinds);
    char *json = cJSON_PrintUnformatted(cfg);
    char script[4096];
    snprintf(script, sizeof(script), "window.setConfigFromNative(%s);", json);
    webkit_web_view_evaluate_javascript(webview, script, -1, NULL, NULL, NULL, NULL, NULL);
    cJSON_Delete(cfg);
    free(json);
}

static void on_js_message(WebKitUserContentManager *manager, WebKitJavascriptResult *js_result, gpointer user_data) {
    (void)manager;
    WebKitWebView *webview = (WebKitWebView*)user_data;
    JSCValue *value = webkit_javascript_result_get_js_value(js_result);
    if (jsc_value_is_string(value)) {
        char *msg = g_strdup(jsc_value_to_string(value));
        cJSON *root = cJSON_Parse(msg);
        if (root) {
            const cJSON *action_json = cJSON_GetObjectItem(root, "action");
            if (action_json) {
                const char* action = action_json->valuestring;
                if (strcmp(action, "get_config") == 0) {
                    send_config_to_js(webview);
                } else if (strcmp(action, "read_file") == 0) {
                    const cJSON* file_json = cJSON_GetObjectItem(root, "file");
                    if (file_json && cJSON_IsString(file_json)) {
                        const char *home = getenv("HOME");
                        if (!home) home = getpwuid(getuid())->pw_dir;
                        char file_path[PATH_MAX];
                        snprintf(file_path, sizeof(file_path), "%s/.config/mocha/%s", home, file_json->valuestring);

                        gchar* content = NULL;
                        GError* error = NULL;
                        if (g_file_get_contents(file_path, &content, NULL, &error)) {
                            gchar* escaped = g_strescape(content, NULL);
                            gchar* script = g_strdup_printf("window.setEditorContent(`%s`);", escaped);
                            webkit_web_view_evaluate_javascript(webview, script, -1, NULL, NULL, NULL, NULL, NULL);
                            g_free(escaped);
                            g_free(script);
                            g_free(content);
                        } else {
                            fprintf(stderr, "Failed to read file %s: %s\n", file_path, error->message);
                            gchar* script = g_strdup_printf("window.setEditorContent(`/* Error loading file: %s */`);", error->message);
                            webkit_web_view_evaluate_javascript(webview, script, -1, NULL, NULL, NULL, NULL, NULL);
                            g_free(script);
                            g_error_free(error);
                        }
                    }
                } else if (strcmp(action, "write_file") == 0) {
                    const cJSON* file_json = cJSON_GetObjectItem(root, "file");
                    const cJSON* content_json = cJSON_GetObjectItem(root, "content");
                    if (file_json && cJSON_IsString(file_json) && content_json && cJSON_IsString(content_json)) {
                        const char *home = getenv("HOME");
                        if (!home) home = getpwuid(getuid())->pw_dir;
                        char file_path[PATH_MAX];
                        snprintf(file_path, sizeof(file_path), "%s/.config/mocha/%s", home, file_json->valuestring);

                        GError* error = NULL;
                        if (!g_file_set_contents(file_path, content_json->valuestring, -1, &error)) {
                            fprintf(stderr, "Failed to write file %s: %s\n", file_path, error->message);
                            g_error_free(error);
                        }
                    }
                }
            }
            cJSON_Delete(root);
        }
        g_free(msg);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    load_config(get_config_path(), &config);
    load_features(get_features_path(), &config);
    load_theme(get_theme_path(), &config);
    load_keybinds(get_keybinds_path(), &config);

    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if(len == -1) {
        perror("readlink");
        return 1;
    }
    exe_path[len] = '\0';
    char *prog_dir = dirname(exe_path);

    char html_path[PATH_MAX];
    snprintf(html_path, sizeof(html_path), "%s/frontend/index.html", prog_dir);

    char uri[PATH_MAX + 16];
    snprintf(uri, sizeof(uri), "file://%s", html_path);

    WebKitUserContentManager *manager = webkit_user_content_manager_new();
    webkit_user_content_manager_register_script_message_handler(manager, "mochaBridge");
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);

    GtkHeaderBar *header_bar = GTK_HEADER_BAR(gtk_header_bar_new());
    gtk_header_bar_set_show_close_button(header_bar, FALSE);
    gtk_header_bar_set_title(header_bar, "Mocha Settings");
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

    const char *css =
        "headerbar, window.titlebar, .titlebar {"
        "   background-image: none;"
        "   background-color: #1e1f23;"
        "   border-bottom: 1px solid #383838;"
        "   box-shadow: none;"
        "}"
        "headerbar .title, window.titlebar .title, .titlebar .title {"
        "   color: #bd93f9;"
        "   font-weight: bold;"
        "   text-shadow: 0 0 8px rgba(189, 147, 249, 0.5);"
        "}"
        "headerbar button, window.titlebar button, .titlebar button {"
        "   background-image: none;"
        "   background-color: transparent;"
        "   border: none;"
        "   box-shadow: none;"
        "   color: #e0e0e0;"
        "   min-height: 24px;"
        "   min-width: 24px;"
        "}"
        "headerbar button:hover, window.titlebar button:hover, .titlebar button:hover {"
        "   background-color: rgba(189, 147, 249, 0.2);"
        "   color: #bd93f9;"
        "}";

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    WebKitSettings *settings = webkit_settings_new();
    webkit_settings_set_enable_developer_extras(settings, TRUE);
    GtkWidget *webview = webkit_web_view_new_with_user_content_manager(manager);
    webkit_web_view_set_settings(WEBKIT_WEB_VIEW(webview), settings);
    
    gtk_container_add(GTK_CONTAINER(window), webview);
    g_signal_connect(manager, "script-message-received::mochaBridge", G_CALLBACK(on_js_message), webview);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), uri);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}