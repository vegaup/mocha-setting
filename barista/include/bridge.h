#ifndef BRIDGE_H
#define BRIDGE_H

#include <webkit2/webkit2.h>
#include <glib.h>

typedef void (*ActionHandler)(WebKitWebView *webview, const gchar *payload);

void bridge_register_action(const gchar *action, ActionHandler handler);
void bridge_handle_message(WebKitUserContentManager *manager, WebKitJavascriptResult *js_result, gpointer user_data);

#endif // BRIDGE_H
