#include "../include/main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int load_config(const char *filename, struct Config *cfg) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    char key[64], value[256];
    cfg->num_exec_once = 0;
    while (fgets(key, sizeof(key), f)) {
        if (sscanf(key, "launcher-command = \"%255[^\"]\"", value) == 1) {
            strncpy(cfg->launcher_cmd, value, MAX_CMD_LEN);
        } else if (sscanf(key, "exec-once = \"%255[^\"]\"", value) == 1) {
            if (cfg->num_exec_once < MAX_EXEC_ONCE)
                strncpy(cfg->exec_once[cfg->num_exec_once++], value, MAX_CMD_LEN);
        }
    }
    fclose(f);
    return 0;
}

int save_config(const char *filename, const struct Config *cfg) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;
    fprintf(f, "launcher-command = \"%s\"\n", cfg->launcher_cmd);
    for (int i = 0; i < cfg->num_exec_once; ++i)
        fprintf(f, "exec-once = \"%s\"\n", cfg->exec_once[i]);
    fclose(f);
    return 0;
}

int load_features(const char *filename, struct Config *cfg) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    char line[256];
    int in_block = 0;
    while (fgets(line, sizeof(line), f)) {
        char *l = line;
        while (*l == ' ' || *l == '\t') l++;
        if (strncmp(l, "features", 8) == 0 && strchr(l, '{')) { in_block = 1; continue; }
        if (in_block && strchr(l, '}')) { in_block = 0; continue; }
        if (in_block && strchr(l, '=')) {
            char key[64], value[64];
            if (sscanf(l, "%63[^=]=%63s", key, value) == 2) {
                char *v = value; while (*v == ' ' || *v == '\t') v++;
                if (strncmp(key, "tiling", 6) == 0)
                    cfg->tiling = (strncmp(v, "true", 4) == 0 || strncmp(v, "1", 1) == 0);
                else if (strncmp(key, "quotes", 6) == 0)
                    cfg->quotes = (strncmp(v, "true", 4) == 0 || strncmp(v, "1", 1) == 0);
            }
        }
    }
    fclose(f);
    return 0;
}

int save_features(const char *filename, const struct Config *cfg) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;
    fprintf(f, "features {\n");
    fprintf(f, "  tiling = %s\n", cfg->tiling ? "true" : "false");
    fprintf(f, "  quotes = %s\n", cfg->quotes ? "true" : "false");
    fprintf(f, "}\n");
    fclose(f);
    return 0;
}

int load_theme(const char *filename, struct Config *cfg) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    char line[256];
    int in_block = 0;
    while (fgets(line, sizeof(line), f)) {
        char *l = line;
        while (*l == ' ' || *l == '\t') l++;
        if (strncmp(l, "colors", 6) == 0 && strchr(l, '{')) { in_block = 1; continue; }
        if (in_block && strchr(l, '}')) { in_block = 0; continue; }
        if (in_block && strchr(l, '=')) {
            char key[64], value[64];
            if (sscanf(l, "%63[^=]=%63s", key, value) == 2) {
                char *v = value; while (*v == ' ' || *v == '\t') v++;
                if (strncmp(key, "border", 6) == 0)
                    strncpy(cfg->colors.border, v, MAX_COLOR_LEN);
                else if (strncmp(key, "focus", 5) == 0)
                    strncpy(cfg->colors.focus, v, MAX_COLOR_LEN);
                else if (strncmp(key, "panel", 5) == 0)
                    strncpy(cfg->colors.panel, v, MAX_COLOR_LEN);
                else if (strncmp(key, "acent", 5) == 0)
                    strncpy(cfg->colors.acent, v, MAX_COLOR_LEN);
            }
        }
    }
    fclose(f);
    return 0;
}

int save_theme(const char *filename, const struct Config *cfg) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;
    fprintf(f, "colors {\n");
    fprintf(f, "  border = %s\n", cfg->colors.border);
    fprintf(f, "  focus = %s\n", cfg->colors.focus);
    fprintf(f, "  panel = %s\n", cfg->colors.panel);
    fprintf(f, "  acent = %s\n", cfg->colors.acent);
    fprintf(f, "}\n");
    fclose(f);
    return 0;
}

int load_keybinds(const char *filename, struct Config *cfg) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    char line[256];
    int in_block = 0;
    cfg->num_keybinds = 0;
    while (fgets(line, sizeof(line), f)) {
        char *l = line;
        while (*l == ' ' || *l == '\t') l++;
        if (strncmp(l, "keybinds", 8) == 0 && strchr(l, '{')) { in_block = 1; continue; }
        if (in_block && strchr(l, '}')) { in_block = 0; continue; }
        if (in_block && strchr(l, '=')) {
            char key[MAX_KEY_LEN], action[MAX_CMD_LEN];
            if (sscanf(l, " %31[^=]= \"%255[^\"]\"", key, action) == 2) {
                if (cfg->num_keybinds < MAX_BINDS) {
                    strncpy(cfg->keybinds[cfg->num_keybinds].key, key, MAX_KEY_LEN);
                    strncpy(cfg->keybinds[cfg->num_keybinds].action, action, MAX_CMD_LEN);
                    cfg->num_keybinds++;
                }
            }
        }
    }
    fclose(f);
    return 0;
}

int save_keybinds(const char *filename, const struct Config *cfg) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;
    fprintf(f, "keybinds {\n");
    for (int i = 0; i < cfg->num_keybinds; ++i)
        fprintf(f, "  %s = \"%s\"\n", cfg->keybinds[i].key, cfg->keybinds[i].action);
    fprintf(f, "}\n");
    fclose(f);
    return 0;
} 