#include "bridge.h"
#include "../lib/cJSON.h"

static GHashTable *action_handlers = NULL;

void bridge_register_action(const gchar *action, ActionHandler handler) {
    if (!action_handlers) {
        action_handlers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    }
    g_hash_table_insert(action_handlers, g_strdup(action), handler);
}

void bridge_handle_message(WebKitUserContentManager *manager, WebKitJavascriptResult *js_result, gpointer user_data) {
    (void)manager;
    WebKitWebView *webview = (WebKitWebView*)user_data;
    JSCValue *value = webkit_javascript_result_get_js_value(js_result);

    if (!jsc_value_is_string(value)) return;
    
    char *msg = jsc_value_to_string(value);
    cJSON *root = cJSON_Parse(msg);
    if (!root) {
        g_free(msg);
        return;
    }

    const cJSON *action_json = cJSON_GetObjectItem(root, "action");
    if (cJSON_IsString(action_json)) {
        const char *action = action_json->valuestring;
        ActionHandler handler = g_hash_table_lookup(action_handlers, action);
        if (handler) {
            cJSON *data = cJSON_GetObjectItem(root, "data");
            char *payload = data ? cJSON_PrintUnformatted(data) : NULL;
            handler(webview, payload);
            free(payload);
        }
    }
    cJSON_Delete(root);
    g_free(msg);
}
