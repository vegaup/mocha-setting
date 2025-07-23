#ifndef MOCHA_MAIN_H
#define MOCHA_MAIN_H

#define MAX_COLOR_LEN 16
#define MAX_CMD_LEN 256
#define MAX_KEY_LEN 32
#define MAX_BINDS 32
#define MAX_EXEC_ONCE 32

struct ConfigColor {
    char border[MAX_COLOR_LEN];
    char focus[MAX_COLOR_LEN];
    char panel[MAX_COLOR_LEN];
    char acent[MAX_COLOR_LEN];
};

struct ConfigKeybind {
    char key[MAX_KEY_LEN];
    char action[MAX_CMD_LEN];
};

struct Config {
    int tiling;
    int quotes;
    struct ConfigColor colors;
    struct ConfigKeybind keybinds[MAX_BINDS];
    int num_keybinds;
    char launcher_cmd[MAX_CMD_LEN];
    char exec_once[MAX_EXEC_ONCE][MAX_CMD_LEN];
    int num_exec_once;
};

int load_config(const char *filename, struct Config *cfg);
int save_config(const char *filename, const struct Config *cfg);
int load_features(const char *filename, struct Config *cfg);
int save_features(const char *filename, const struct Config *cfg);
int load_theme(const char *filename, struct Config *cfg);
int save_theme(const char *filename, const struct Config *cfg);
int load_keybinds(const char *filename, struct Config *cfg);
int save_keybinds(const char *filename, const struct Config *cfg);

#endif // MOCHA_MAIN_H