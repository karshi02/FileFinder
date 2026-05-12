/*
 * main.c — Smart Finder
 * - System tray icon
 * - ทำงานทันที ไม่ต้องตั้งค่าอะไร
 * - Hotkey อัตโนมัติ (ถ้าซ้ำก็เปลี่ยนเอง)
 * - Background scan แบบเงียบ ไม่มี splash รบกวน
 */
#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <shellapi.h>
#include <process.h>
#include <stdio.h>
#include <string.h>
#include "hotkey.h"
#include "universal_scanner.h"
#include "universal_ui.h"
#include "frequent.h"
#include "settings.h"

#define INDEX_PATH  "data\\universal.db"
#define FREQ_PATH   "data\\frequent.db"

#define WM_TRAY         (WM_USER + 1)
#define WM_SCAN_DONE    (WM_USER + 2)
#define ID_TRAY_INFO      301
#define ID_TRAY_SETTINGS  303
#define ID_TRAY_EXIT      302
#define TRAY_UID         1

/* ── globals ── */
static FrequentList   *g_freq       = NULL;
static UniversalIndex *g_idx        = NULL;
static AppSettings     g_set;
static NOTIFYICONDATAA g_nid;
static HWND            g_tray_hwnd  = NULL;
static volatile BOOL   g_scan_done  = FALSE;

/* ================================================================
   callback จาก universal_ui.c เมื่อเปิดแอพ
   ================================================================ */
void on_app_launched(const char *name, const char *path) {
    if (g_freq && name && path) {
        frequent_add_or_update(g_freq, name, path);
        frequent_save(g_freq, FREQ_PATH);
        frequent_sort(g_freq);
        if (g_idx) universal_update_freq_score(g_idx, g_freq);
    }
}

/* ================================================================
   Background scan thread (เงียบ ไม่มี splash)
   ================================================================ */
static unsigned __stdcall scan_thread(void *arg) {
    (void)arg;
    UniversalIndex *idx = universal_load(INDEX_PATH);
    if (!idx) {
        idx = universal_create();
        if (idx) {
            universal_scan_all(idx);
            universal_save(idx, INDEX_PATH);
        }
    }
    if (idx && g_freq) universal_update_freq_score(idx, g_freq);
    g_idx      = idx;
    g_scan_done = TRUE;
    if (g_tray_hwnd) PostMessageA(g_tray_hwnd, WM_SCAN_DONE, 0, 0);
    return 0;
}

/* ================================================================
   Tray
   ================================================================ */
static void tray_add(HWND hwnd) {
    memset(&g_nid, 0, sizeof(g_nid));
    g_nid.cbSize           = sizeof(g_nid);
    g_nid.hWnd             = hwnd;
    g_nid.uID              = TRAY_UID;
    g_nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAY;
    g_nid.hIcon            = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION);

    UINT mod, vk;
    char hk[80] = "Auto";
    
    if (hotkey_is_registered()) {
        hotkey_get_current(&mod, &vk);
        if (mod != 0 || vk != 0) {
            char key[32] = "";
            if (vk == VK_SPACE) strcpy(key, "Space");
            else if (vk >= 'A' && vk <= 'Z') { key[0] = (char)vk; key[1] = '\0'; }
            else if (vk >= '0' && vk <= '9') { key[0] = (char)vk; key[1] = '\0'; }
            else if (vk == VK_F1) strcpy(key, "F1");
            else if (vk == VK_F2) strcpy(key, "F2");
            else if (vk == VK_F3) strcpy(key, "F3");
            else if (vk == VK_F4) strcpy(key, "F4");
            else if (vk == VK_F5) strcpy(key, "F5");
            else if (vk == VK_F6) strcpy(key, "F6");
            else if (vk == VK_F7) strcpy(key, "F7");
            else if (vk == VK_F8) strcpy(key, "F8");
            else if (vk == VK_F9) strcpy(key, "F9");
            else if (vk == VK_F10) strcpy(key, "F10");
            else if (vk == VK_F11) strcpy(key, "F11");
            else if (vk == VK_F12) strcpy(key, "F12");
            else snprintf(key, sizeof(key), "0x%02X", vk);
            
            snprintf(hk, sizeof(hk), "%s%s%s%s+%s",
                (mod & MOD_CONTROL) ? "Ctrl" : "",
                (mod & MOD_ALT) ? "+Alt" : "",
                (mod & MOD_SHIFT) ? "+Shift" : "",
                (mod & MOD_WIN) ? "+Win" : "",
                key);
            
            if (hk[0] == '+') {
                memmove(hk, hk + 1, strlen(hk));
            }
        }
    }
    
    char tip[100];
    snprintf(tip, sizeof(tip), "Smart Finder  [%s]", hk);
    strncpy(g_nid.szTip, tip, sizeof(g_nid.szTip) - 1);
    g_nid.szTip[sizeof(g_nid.szTip) - 1] = '\0';
    Shell_NotifyIconA(NIM_ADD, &g_nid);
}

static void show_tray_menu(HWND hwnd) {
    HMENU hm = CreatePopupMenu();
    
    UINT mod, vk;
    char hk[80] = "Auto";
    if (hotkey_is_registered()) {
        hotkey_get_current(&mod, &vk);
        if (mod != 0 || vk != 0) {
            char key[32] = "";
            if (vk == VK_SPACE) strcpy(key, "Space");
            else if (vk >= 'A' && vk <= 'Z') { key[0] = (char)vk; key[1] = '\0'; }
            else if (vk >= '0' && vk <= '9') { key[0] = (char)vk; key[1] = '\0'; }
            else if (vk == VK_F1) strcpy(key, "F1");
            else if (vk == VK_F2) strcpy(key, "F2");
            else if (vk == VK_F3) strcpy(key, "F3");
            else if (vk == VK_F4) strcpy(key, "F4");
            else if (vk == VK_F5) strcpy(key, "F5");
            else if (vk == VK_F6) strcpy(key, "F6");
            else if (vk == VK_F7) strcpy(key, "F7");
            else if (vk == VK_F8) strcpy(key, "F8");
            else if (vk == VK_F9) strcpy(key, "F9");
            else if (vk == VK_F10) strcpy(key, "F10");
            else if (vk == VK_F11) strcpy(key, "F11");
            else if (vk == VK_F12) strcpy(key, "F12");
            else snprintf(key, sizeof(key), "0x%02X", vk);
            
            snprintf(hk, sizeof(hk), "%s%s%s%s+%s",
                (mod & MOD_CONTROL) ? "Ctrl" : "",
                (mod & MOD_ALT) ? "+Alt" : "",
                (mod & MOD_SHIFT) ? "+Shift" : "",
                (mod & MOD_WIN) ? "+Win" : "",
                key);
            
            if (hk[0] == '+') {
                memmove(hk, hk + 1, strlen(hk));
            }
        }
    }
    
    char label[128];
    snprintf(label, sizeof(label), "✅ Smart Finder  [%s]", hk);
    AppendMenuA(hm, MF_STRING | MF_GRAYED, 0, label);
    AppendMenuA(hm, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hm, MF_STRING, 304, "  Open Search");
    AppendMenuA(hm, MF_STRING, ID_TRAY_INFO, "  Info");
    AppendMenuA(hm, MF_STRING, ID_TRAY_SETTINGS, "  Settings...");
    AppendMenuA(hm, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hm, MF_STRING, ID_TRAY_EXIT, "  Exit");
    
    SetForegroundWindow(hwnd);
    POINT pt;
    GetCursorPos(&pt);
    TrackPopupMenu(hm, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hm);
}

static void show_balloon(const char *title, const char *message, DWORD flags) {
    g_nid.uFlags = NIF_INFO;
    strncpy(g_nid.szInfoTitle, title, sizeof(g_nid.szInfoTitle) - 1);
    g_nid.szInfoTitle[sizeof(g_nid.szInfoTitle) - 1] = '\0';
    strncpy(g_nid.szInfo, message, sizeof(g_nid.szInfo) - 1);
    g_nid.szInfo[sizeof(g_nid.szInfo) - 1] = '\0';
    g_nid.dwInfoFlags = flags;
    Shell_NotifyIconA(NIM_MODIFY, &g_nid);
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
}

/* ================================================================
   Tray WndProc
   ================================================================ */
static LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg,
                                     WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_HOTKEY:
        // Debug: เขียนลงไฟล์
        {
            FILE* debug = fopen("C:\\hotkey_debug.log", "a");
            if (debug) {
                fprintf(debug, "WM_HOTKEY received! wParam=%d, g_scan_done=%d, g_idx=%p\n", 
                        wParam, g_scan_done, g_idx);
                fclose(debug);
            }
        }
        
        if ((int)wParam == HOTKEY_ID && g_scan_done && g_idx) {
            universal_show_popup(g_idx);
        }
        break;

    case WM_TRAY:
        if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
            show_tray_menu(hwnd);
        } else if (lParam == WM_LBUTTONUP || lParam == WM_LBUTTONDBLCLK) {
            if (g_scan_done && g_idx)
                universal_show_popup(g_idx);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_INFO: {
            char msg[512];
            UINT mod, vk;
            char hk[80] = "Auto";
            
            if (hotkey_is_registered()) {
                hotkey_get_current(&mod, &vk);
                if (mod != 0 || vk != 0) {
                    char key[32] = "";
                    if (vk == VK_SPACE) strcpy(key, "Space");
                    else if (vk >= 'A' && vk <= 'Z') { key[0] = (char)vk; key[1] = '\0'; }
                    else if (vk >= '0' && vk <= '9') { key[0] = (char)vk; key[1] = '\0'; }
                    else if (vk == VK_F1) strcpy(key, "F1");
                    else if (vk == VK_F2) strcpy(key, "F2");
                    else if (vk == VK_F3) strcpy(key, "F3");
                    else if (vk == VK_F4) strcpy(key, "F4");
                    else if (vk == VK_F5) strcpy(key, "F5");
                    else if (vk == VK_F6) strcpy(key, "F6");
                    else if (vk == VK_F7) strcpy(key, "F7");
                    else if (vk == VK_F8) strcpy(key, "F8");
                    else if (vk == VK_F9) strcpy(key, "F9");
                    else if (vk == VK_F10) strcpy(key, "F10");
                    else if (vk == VK_F11) strcpy(key, "F11");
                    else if (vk == VK_F12) strcpy(key, "F12");
                    else snprintf(key, sizeof(key), "0x%02X", vk);
                    
                    snprintf(hk, sizeof(hk), "%s%s%s%s+%s",
                        (mod & MOD_CONTROL) ? "Ctrl" : "",
                        (mod & MOD_ALT) ? "+Alt" : "",
                        (mod & MOD_SHIFT) ? "+Shift" : "",
                        (mod & MOD_WIN) ? "+Win" : "",
                        key);
                    
                    if (hk[0] == '+') {
                        memmove(hk, hk + 1, strlen(hk));
                    }
                }
            }
            
            snprintf(msg, sizeof(msg),
                "Smart Finder v1.0\n\n"
                "Hotkey: %s\n\n"
                "กด hotkey เพื่อเปิดค้นหา\n"
                "หรือคลิกที่ tray icon\n"
                "คลิกขวาเพื่อออกจากโปรแกรม",
                hk);
            
            MessageBoxA(hwnd, msg, "Smart Finder", MB_OK | MB_ICONINFORMATION);
            break;
        }
        case 304:
            if (g_scan_done && g_idx)
                universal_show_popup(g_idx);
            break;
        case ID_TRAY_SETTINGS: {
            hotkey_unregister();
            settings_load(&g_set);
            BOOL changed = settings_show_dialog(hwnd, &g_set);
            if (changed) {
                if (g_set.mod_keys != 0 && g_set.vk_code != 0) {
                    if (!hotkey_register(g_set.mod_keys, g_set.vk_code)) {
                        hotkey_register_auto();
                    }
                } else {
                    hotkey_register_auto();
                }
                settings_apply_startup(g_set.run_on_startup);
                universal_apply_settings(g_set.show_recent,
                                          g_set.max_results,
                                          g_set.close_on_launch);
                UINT m, v;
                hotkey_get_current(&m, &v);
                char hk2[80], tip2[100];
                settings_hotkey_to_string(m, v, hk2, sizeof(hk2));
                snprintf(tip2, sizeof(tip2), "Smart Finder  [%s]", hk2);
                strncpy(g_nid.szTip, tip2, sizeof(g_nid.szTip)-1);
                g_nid.uFlags = NIF_TIP;
                Shell_NotifyIconA(NIM_MODIFY, &g_nid);
            } else {
                hotkey_register_auto();
            }
            break;
        }
        case ID_TRAY_EXIT:
            Shell_NotifyIconA(NIM_DELETE, &g_nid);
            PostQuitMessage(0);
            break;
        }
        break;

    case WM_SCAN_DONE:
        if (!g_idx) {
            MessageBoxA(NULL,
                "เกิดข้อผิดพลาดขณะ scan\nกรุณาลบ data\\universal.db แล้วรันใหม่",
                "Smart Finder", MB_ICONERROR);
            PostQuitMessage(1);
            break;
        }
        universal_apply_settings(TRUE, 12, TRUE);
        {
            char balloon_msg[256];
            UINT mod, vk;
            char hk[80] = "Auto";
            
            if (hotkey_is_registered()) {
                hotkey_get_current(&mod, &vk);
                if (mod != 0 || vk != 0) {
                    char key[32] = "";
                    if (vk == VK_SPACE) strcpy(key, "Space");
                    else if (vk >= 'A' && vk <= 'Z') { key[0] = (char)vk; key[1] = '\0'; }
                    else if (vk >= '0' && vk <= '9') { key[0] = (char)vk; key[1] = '\0'; }
                    else if (vk == VK_F1) strcpy(key, "F1");
                    else if (vk == VK_F2) strcpy(key, "F2");
                    else if (vk == VK_F3) strcpy(key, "F3");
                    else if (vk == VK_F4) strcpy(key, "F4");
                    else if (vk == VK_F5) strcpy(key, "F5");
                    else if (vk == VK_F6) strcpy(key, "F6");
                    else if (vk == VK_F7) strcpy(key, "F7");
                    else if (vk == VK_F8) strcpy(key, "F8");
                    else if (vk == VK_F9) strcpy(key, "F9");
                    else if (vk == VK_F10) strcpy(key, "F10");
                    else if (vk == VK_F11) strcpy(key, "F11");
                    else if (vk == VK_F12) strcpy(key, "F12");
                    else snprintf(key, sizeof(key), "0x%02X", vk);
                    
                    snprintf(hk, sizeof(hk), "%s%s%s%s+%s",
                        (mod & MOD_CONTROL) ? "Ctrl" : "",
                        (mod & MOD_ALT) ? "+Alt" : "",
                        (mod & MOD_SHIFT) ? "+Shift" : "",
                        (mod & MOD_WIN) ? "+Win" : "",
                        key);
                    
                    if (hk[0] == '+') {
                        memmove(hk, hk + 1, strlen(hk));
                    }
                }
            }
            
            snprintf(balloon_msg, sizeof(balloon_msg),
                "Smart Finder พร้อมใช้งาน!\nกด %s เพื่อเปิดค้นหา", hk);
            show_balloon("Smart Finder", balloon_msg, NIIF_INFO);
        }
        break;

    case WM_DESTROY:
        Shell_NotifyIconA(NIM_DELETE, &g_nid);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

/* ================================================================
   WinMain
   ================================================================ */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev,
                   LPSTR lpCmdLine, int nCmdShow) {
    (void)hInstance;
    (void)hPrev;
    (void)lpCmdLine;
    (void)nCmdShow;

    HANDLE hMutex = CreateMutexA(NULL, TRUE, "SmartFinder_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxA(NULL,
            "Smart Finder กำลังทำงานอยู่แล้ว\n"
            "โปรดดูที่ System Tray (ข้างๆ นาฬิกา)",
            "Smart Finder", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    CreateDirectoryA("data", NULL);

    settings_load(&g_set);
    settings_ensure_startup_once();

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "SmartFinderTray";
    RegisterClassExA(&wc);

    g_tray_hwnd = CreateWindowExA(0, "SmartFinderTray", NULL, 0, 0, 0, 0, 0,
                                   HWND_MESSAGE, NULL, GetModuleHandle(NULL), NULL);

    // สำคัญ: ตั้งค่า window handle ให้ hotkey ก่อนลงทะเบียน
    hotkey_set_window(g_tray_hwnd);
    
    g_freq = frequent_load(FREQ_PATH);
    if (!g_freq) g_freq = frequent_create();
    else frequent_sort(g_freq);

    BOOL hotkey_ok = hotkey_register_auto();
    
    tray_add(g_tray_hwnd);

    if (!hotkey_ok) {
        show_balloon("Smart Finder", 
            "ไม่สามารถลงทะเบียน Hotkey ได้\n"
            "แต่ยังใช้โปรแกรมได้โดยคลิกที่ icon นี้",
            NIIF_WARNING);
    }

    FILE *f = fopen(INDEX_PATH, "rb");
    if (f) {
        fclose(f);
        g_idx = universal_load(INDEX_PATH);
        if (g_idx) {
            universal_update_freq_score(g_idx, g_freq);
        }
        g_scan_done = TRUE;
        universal_apply_settings(TRUE, 12, TRUE);
    } else {
        _beginthreadex(NULL, 0, scan_thread, NULL, 0, NULL);
        show_balloon("Smart Finder", 
            "กำลังค้นหาแอพในคอมพิวเตอร์ของคุณ...\nสักครู่ก็ใช้ได้ครับ",
            NIIF_INFO);
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    hotkey_unregister();
    universal_destroy(g_idx);
    frequent_destroy(g_freq);
    
    if (hMutex) CloseHandle(hMutex);
    return 0;
}
