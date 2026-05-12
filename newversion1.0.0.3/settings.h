#ifndef SETTINGS_H
#define SETTINGS_H

#include <windows.h>

#define SETTINGS_FILE   "data\\settings.ini"

typedef struct {
    UINT  mod_keys;
    UINT  vk_code;
    BOOL  run_on_startup;
    BOOL  show_recent;
    int   max_results;
    BOOL  close_on_launch;
} AppSettings;

void settings_load(AppSettings *s);
void settings_save(const AppSettings *s);
void settings_apply_startup(BOOL enable);
void settings_ensure_startup_once(void);
void settings_hotkey_to_string(UINT mod, UINT vk, char *out, int outlen);
BOOL settings_show_dialog(HWND parent, AppSettings *s);

#endif
