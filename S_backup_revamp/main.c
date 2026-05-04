/*
 * main.c
 * - System tray icon
 * - คลิกขวา tray → Settings / Exit
 * - Settings: ตั้ง hotkey เอง + toggle Run at Startup
 * - โหลด/บันทึก settings จาก data\settings.ini
 */
#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <process.h>
#include "hotkey.h"
#include "universal_scanner.h"
#include "universal_ui.h"
#include "frequent.h"
#include "settings.h"

#define INDEX_PATH   "data\\universal.db"
#define FREQ_PATH    "data\\frequent.db"

/* tray */
#define WM_TRAY      (WM_USER + 1)
#define ID_TRAY_SETTINGS 301
#define ID_TRAY_EXIT     302
#define TRAY_UID         1

static FrequentList  *g_freq       = NULL;
static UniversalIndex*g_idx        = NULL;
static AppSettings    g_set;
static NOTIFYICONDATAA g_nid;
static HWND            g_tray_hwnd  = NULL;
static HWND            g_splash     = NULL;
static volatile BOOL   g_scan_done  = FALSE;

/* ── Splash window ── */
static LRESULT CALLBACK SplashProc(HWND hwnd, UINT msg,
                                    WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_ERASEBKGND: {
        HBRUSH hb = CreateSolidBrush(RGB(28,28,30));
        RECT rc; GetClientRect(hwnd, &rc);
        FillRect((HDC)wParam, &rc, hb);
        DeleteObject(hb);
        return 1;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);

        /* พื้นหลัง */
        HBRUSH hb = CreateSolidBrush(RGB(28,28,30));
        FillRect(dc, &rc, hb);
        DeleteObject(hb);

        SetBkMode(dc, TRANSPARENT);

        /* ชื่อโปรแกรม */
        HFONT hf = CreateFontA(20,0,0,0,FW_SEMIBOLD,0,0,0,
                               DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,
                               DEFAULT_PITCH|FF_DONTCARE,"Segoe UI");
        HFONT old = (HFONT)SelectObject(dc, hf);
        SetTextColor(dc, RGB(220,220,220));
        RECT rcTitle = {rc.left, rc.top+12, rc.right, rc.top+40};
        DrawTextA(dc, "Smart Finder", -1, &rcTitle, DT_CENTER|DT_SINGLELINE);
        SelectObject(dc, old);
        DeleteObject(hf);

        /* subtext */
        HFONT hfs = CreateFontA(13,0,0,0,FW_NORMAL,0,0,0,
                                DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,
                                DEFAULT_PITCH|FF_DONTCARE,"Segoe UI");
        old = (HFONT)SelectObject(dc, hfs);
        SetTextColor(dc, RGB(100,100,100));
        RECT rcSub = {rc.left, rc.top+42, rc.right, rc.bottom};
        DrawTextA(dc, "Scanning apps, please wait...", -1, &rcSub, DT_CENTER|DT_SINGLELINE);
        SelectObject(dc, old);
        DeleteObject(hfs);

        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static void show_splash(void) {
    static BOOL cls_reg = FALSE;
    if (!cls_reg) {
        WNDCLASSEXA wc = {0};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = SplashProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
        wc.lpszClassName = "SmartFinderSplash";
        RegisterClassExA(&wc);
        cls_reg = TRUE;
    }
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    g_splash = CreateWindowExA(
        WS_EX_TOPMOST|WS_EX_TOOLWINDOW,
        "SmartFinderSplash", NULL,
        WS_POPUP|WS_VISIBLE,
        (sw-300)/2, (sh-80)/2, 300, 80,
        NULL, NULL, GetModuleHandle(NULL), NULL);
    UpdateWindow(g_splash);
}

static void close_splash(void) {
    if (g_splash) { DestroyWindow(g_splash); g_splash = NULL; }
}

/* ── Background scan thread ── */
static unsigned __stdcall scan_thread(void *arg) {
    (void)arg;

    /* ทำงานทั้งหมดบน local ก่อน แล้วค่อย assign g_idx เมื่อเสร็จ
       ป้องกัน main thread อ่าน g_idx ขณะยังไม่พร้อม */
    UniversalIndex *idx = universal_load(INDEX_PATH);
    if (!idx) {
        idx = universal_create();
        if (idx) {
            universal_scan_all(idx);
            universal_save(idx, INDEX_PATH);
        }
    }
    if (idx && g_freq)
        universal_update_freq_score(idx, g_freq);

    g_idx = idx;          /* assign เดี่ยวครั้งเดียว — safe บน x86/x64 */
    g_scan_done = TRUE;

    if (g_tray_hwnd) PostMessageA(g_tray_hwnd, WM_USER+2, 0, 0);
    return 0;
}

/* ── callback เมื่อเปิดแอพ (ถูกเรียกจาก universal_ui.c) ── */
void on_app_launched(const char *name, const char *path) {
    if (g_freq && name && path) {
        frequent_add_or_update(g_freq, name, path);
        frequent_save(g_freq, FREQ_PATH);
        frequent_sort(g_freq);
        universal_update_freq_score(g_idx, g_freq);
    }
}

/* ── เพิ่ม tray icon ── */
static void tray_add(HWND hwnd) {
    memset(&g_nid, 0, sizeof(g_nid));
    g_nid.cbSize           = sizeof(g_nid);
    g_nid.hWnd             = hwnd;
    g_nid.uID              = TRAY_UID;
    g_nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAY;
    g_nid.hIcon            = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION);

    char tip[100];
    settings_hotkey_to_string(g_set.mod_keys, g_set.vk_code, tip, sizeof(tip));
    snprintf(g_nid.szTip, sizeof(g_nid.szTip),
             "Smart Finder  [%s]", tip);

    Shell_NotifyIconA(NIM_ADD, &g_nid);
}

static void tray_remove(void) {
    Shell_NotifyIconA(NIM_DELETE, &g_nid);
}

static void tray_update_tip(void) {
    char tip[100];
    settings_hotkey_to_string(g_set.mod_keys, g_set.vk_code, tip, sizeof(tip));
    snprintf(g_nid.szTip, sizeof(g_nid.szTip),
             "Smart Finder  [%s]", tip);
    g_nid.uFlags = NIF_TIP;
    Shell_NotifyIconA(NIM_MODIFY, &g_nid);
}

/* ── แสดงเมนูคลิกขวา ── */
static void show_tray_menu(HWND hwnd) {
    HMENU hMenu = CreatePopupMenu();

    char hotkey_str[128];
    settings_hotkey_to_string(g_set.mod_keys, g_set.vk_code,
                              hotkey_str, sizeof(hotkey_str));

    char search_label[160];
    snprintf(search_label, sizeof(search_label),
             "Search  [%s]", hotkey_str);

    AppendMenuA(hMenu, MF_STRING | MF_GRAYED, 0, search_label);
    AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_SETTINGS, "⚙  Settings...");
    AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_EXIT, "✕  Exit");

    /* ต้อง SetForegroundWindow ก่อน TrackPopupMenu */
    SetForegroundWindow(hwnd);

    POINT pt;
    GetCursorPos(&pt);
    TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN,
                   pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}

/* ── WndProc ของ hidden tray window ── */
static LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg,
                                     WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_HOTKEY:
        if ((int)wParam == HOTKEY_ID) {
            if (g_scan_done)
                universal_show_popup(g_idx);
        }
        break;

    case WM_USER+2:   /* scan thread เสร็จ */
        close_splash();
        if (!g_idx) {
            MessageBoxA(NULL,
                "เกิดข้อผิดพลาดขณะ scan\n"
                "กรุณาลบ data\\universal.db แล้วรันใหม่",
                "Smart Finder", MB_ICONERROR);
            PostQuitMessage(1);
            break;
        }
        tray_add(hwnd);
        if (!hotkey_register(g_set.mod_keys, g_set.vk_code)) {
            MessageBoxA(NULL,
                "ไม่สามารถลงทะเบียน hotkey ได้\n"
                "คลิกขวา tray icon เพื่อเปลี่ยน hotkey",
                "Smart Finder", MB_ICONWARNING);
        }
        break;

    case WM_TRAY:
        if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU)
            show_tray_menu(hwnd);
        else if (lParam == WM_LBUTTONDBLCLK)
            universal_show_popup(g_idx);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_SETTINGS: {
            /* เปิดหน้าต่าง Settings */
            hotkey_unregister();

            BOOL changed = settings_show_dialog(hwnd, &g_set);
            if (changed) {
                /* ลงทะเบียน hotkey ใหม่ */
                if (!hotkey_register(g_set.mod_keys, g_set.vk_code)) {
                    MessageBoxA(hwnd,
                        "ไม่สามารถลงทะเบียน hotkey ได้\n"
                        "อาจถูกใช้งานโดยโปรแกรมอื่น\n"
                        "กรุณาเลือก key combo ใหม่",
                        "Smart Finder", MB_ICONWARNING);
                }
                tray_update_tip();
            } else {
                /* ถ้ายกเลิก ลงทะเบียนค่าเดิมกลับ */
                hotkey_register(g_set.mod_keys, g_set.vk_code);
            }
            break;
        }
        case ID_TRAY_EXIT:
            tray_remove();
            PostQuitMessage(0);
            break;
        }
        break;

    case WM_DESTROY:
        /* ไม่ต้องเรียก tray_remove() ซ้ำ เพราะ ID_TRAY_EXIT ทำแล้ว
           แต่ถ้ามีการปิดจากภายนอก ให้ clean up ด้วย */
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
                   LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance; (void)hPrev; (void)lpCmdLine; (void)nCmdShow;

    CreateDirectoryA("data", NULL);

    /* --- โหลด settings --- */
    settings_load(&g_set);

    /* --- apply startup ตามที่บันทึกไว้ --- */
    settings_apply_startup(g_set.run_on_startup);

    /* --- สร้าง hidden window สำหรับ tray + hotkey --- */
    WNDCLASSEXA wc = {0};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = TrayWndProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = "SmartFinderTray";
    RegisterClassExA(&wc);

    g_tray_hwnd = CreateWindowExA(0, "SmartFinderTray", NULL,
                                   0, 0, 0, 0, 0,
                                   HWND_MESSAGE, NULL,
                                   GetModuleHandle(NULL), NULL);

    /* --- โหลด frequent list --- */
    g_freq = frequent_load(FREQ_PATH);
    if (!g_freq) g_freq = frequent_create();
    else { frequent_sort(g_freq); }

    /* --- โหลด index: ถ้ามี cache โหลดเลย, ถ้าไม่มี scan ใน thread --- */
    {
        FILE *f = fopen(INDEX_PATH, "rb");
        if (f) {
            fclose(f);
            g_idx = universal_load(INDEX_PATH);
            if (g_idx) universal_update_freq_score(g_idx, g_freq);
            g_scan_done = TRUE;
            tray_add(g_tray_hwnd);
            if (!hotkey_register(g_set.mod_keys, g_set.vk_code)) {
                char buf[256], hkstr[64];
                settings_hotkey_to_string(g_set.mod_keys, g_set.vk_code,
                                          hkstr, sizeof(hkstr));
                snprintf(buf, sizeof(buf),
                         "ไม่สามารถลงทะเบียน hotkey [%s] ได้\n"
                         "คลิกขวา tray icon เพื่อเปลี่ยน hotkey", hkstr);
                MessageBoxA(NULL, buf, "Smart Finder", MB_ICONWARNING);
            }
        } else {
            /* ครั้งแรก: scan ใน background thread + splash */
            show_splash();
            _beginthreadex(NULL, 0, scan_thread, NULL, 0, NULL);
        }
    }

    /* --- message loop --- */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    hotkey_unregister();
    universal_destroy(g_idx);
    frequent_destroy(g_freq);
    return 0;
}
