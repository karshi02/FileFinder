#ifndef HOTKEY_H
#define HOTKEY_H
#include <windows.h>

#define HOTKEY_ID 1

BOOL hotkey_register_auto(void);
BOOL hotkey_register(UINT mod, UINT vk);
void hotkey_unregister(void);
void hotkey_get_current(UINT *mod, UINT *vk);
BOOL hotkey_is_registered(void);
void hotkey_set_window(HWND hwnd);  // เพิ่มฟังก์ชันนี้

#endif
