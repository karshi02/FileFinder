#include "hotkey.h"
#include <stdio.h>

static BOOL registered = FALSE;
static UINT current_mod = 0;
static UINT current_vk = 0;
static HWND g_hotkey_hwnd = NULL;  // เพิ่มตัวแปรเก็บ window handle

#ifndef MOD_NOREPEAT
#define MOD_NOREPEAT 0x4000
#endif

static const struct {
    UINT mod;
    UINT vk;
} fallback_hotkeys[] = {
    {MOD_CONTROL | MOD_ALT | MOD_NOREPEAT,   VK_SPACE},  // Ctrl+Alt+Space
    {MOD_CONTROL | MOD_ALT | MOD_NOREPEAT,   'F'},        // Ctrl+Alt+F
    {MOD_CONTROL | MOD_ALT | MOD_NOREPEAT,   'Q'},        // Ctrl+Alt+Q
    {MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, VK_SPACE},   // Ctrl+Shift+Space
    {MOD_CONTROL | MOD_NOREPEAT,             VK_F6},      // Ctrl+F6
    {MOD_CONTROL | MOD_NOREPEAT,             VK_F7},      // Ctrl+F7
    {MOD_CONTROL | MOD_NOREPEAT,             VK_F8},      // Ctrl+F8
    {MOD_CONTROL | MOD_NOREPEAT,             VK_F9},      // Ctrl+F9
    {MOD_CONTROL | MOD_NOREPEAT,             VK_F10},     // Ctrl+F10
    {MOD_CONTROL | MOD_NOREPEAT,             VK_F11},     // Ctrl+F11
    {MOD_CONTROL | MOD_NOREPEAT,             VK_F12},     // Ctrl+F12
    {MOD_ALT | MOD_NOREPEAT,                 'F'},        // Alt+F
    {MOD_ALT | MOD_NOREPEAT,                 'Q'},        // Alt+Q
    {0, 0}
};

// ฟังก์ชันใหม่: ตั้งค่า window handle
void hotkey_set_window(HWND hwnd) {
    g_hotkey_hwnd = hwnd;
}

static void save_hotkey_to_registry(UINT mod, UINT vk) {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, 
        "Software\\SmartFinder", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD m = mod, v = vk;
        RegSetValueExA(hKey, "HotkeyMod", 0, REG_DWORD, (BYTE*)&m, sizeof(m));
        RegSetValueExA(hKey, "HotkeyVK", 0, REG_DWORD, (BYTE*)&v, sizeof(v));
        RegCloseKey(hKey);
    }
}

static BOOL load_hotkey_from_registry(UINT *mod, UINT *vk) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\SmartFinder", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD m = 0, v = 0, size = sizeof(DWORD);
        if (RegQueryValueExA(hKey, "HotkeyMod", NULL, NULL, (BYTE*)&m, &size) == ERROR_SUCCESS &&
            RegQueryValueExA(hKey, "HotkeyVK", NULL, NULL, (BYTE*)&v, &size) == ERROR_SUCCESS) {
            *mod = m;
            *vk = v;
            RegCloseKey(hKey);
            return TRUE;
        }
        RegCloseKey(hKey);
    }
    return FALSE;
}

// ฟังก์ชันช่วย: ลงทะเบียน hotkey กับ window
static BOOL register_hotkey_with_window(HWND hwnd, UINT mod, UINT vk) {
    HWND target = hwnd ? hwnd : g_hotkey_hwnd;
    if (!target) {
        target = GetActiveWindow();  // ลองใช้ active window
    }
    return RegisterHotKey(target, HOTKEY_ID, mod | MOD_NOREPEAT, vk);
}

BOOL hotkey_register_auto(void) {
    UINT mod, vk;
    HWND target = g_hotkey_hwnd ? g_hotkey_hwnd : GetActiveWindow();
    
    // 1. ลองโหลด hotkey ที่เคยใช้ได้
    if (load_hotkey_from_registry(&mod, &vk)) {
        if (register_hotkey_with_window(target, mod, vk)) {
            current_mod = mod;
            current_vk = vk;
            registered = TRUE;
            return TRUE;
        }
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\SmartFinder",
                          0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            RegDeleteValueA(hKey, "HotkeyMod");
            RegDeleteValueA(hKey, "HotkeyVK");
            RegCloseKey(hKey);
        }
    }
    
    // 2. ลอง fallback hotkey ทีละตัว
    for (int i = 0; fallback_hotkeys[i].mod != 0; i++) {
        if (register_hotkey_with_window(target, fallback_hotkeys[i].mod, fallback_hotkeys[i].vk)) {
            current_mod = fallback_hotkeys[i].mod;
            current_vk = fallback_hotkeys[i].vk;
            registered = TRUE;
            save_hotkey_to_registry(current_mod, current_vk);
            return TRUE;
        }
    }
    
    return FALSE;
}

BOOL hotkey_register(UINT mod, UINT vk) {
    if (registered) hotkey_unregister();
    
    HWND target = g_hotkey_hwnd ? g_hotkey_hwnd : GetActiveWindow();
    registered = RegisterHotKey(target, HOTKEY_ID, mod | MOD_NOREPEAT, vk);
    
    if (registered) {
        current_mod = mod;
        current_vk = vk;
        save_hotkey_to_registry(mod, vk);
    }
    return registered;
}

void hotkey_unregister(void) {
    if (registered) {
        HWND target = g_hotkey_hwnd ? g_hotkey_hwnd : GetActiveWindow();
        UnregisterHotKey(target, HOTKEY_ID);
        registered = FALSE;
    }
}

void hotkey_get_current(UINT *mod, UINT *vk) {
    if (mod) *mod = current_mod;
    if (vk) *vk = current_vk;
}

BOOL hotkey_is_registered(void) {
    return registered;
}
