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
#include "../barista/include/barista.h"

static struct Config config;

static void handle_get_config(WebKitWebView *webview, const gchar *payload);
static void handle_read_file(WebKitWebView *webview, const gchar *payload);
static void handle_write_file(WebKitWebView *webview, const gchar *payload);

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

static void handle_get_config(WebKitWebView *webview, const gchar *payload) {
    (void)payload;
    send_config_to_js(webview);
}

static void handle_read_file(WebKitWebView *webview, const gchar *payload) {
    cJSON *root = cJSON_Parse(payload);
    if (!root) return;
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
    cJSON_Delete(root);
}

static void handle_write_file(WebKitWebView *webview, const gchar *payload) {
    (void)webview;
    cJSON *root = cJSON_Parse(payload);
    if (!root) return;
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
    cJSON_Delete(root);
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
    
    char css_path[PATH_MAX];
    snprintf(css_path, sizeof(css_path), "%s/barista/css/gtk_theme.css", prog_dir);

    bridge_register_action("get_config", handle_get_config);
    bridge_register_action("read_file", handle_read_file);
    bridge_register_action("write_file", handle_write_file);

    GtkWidget *window = create_window("Mocha Settings", html_path, css_path, 1000, 700);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}