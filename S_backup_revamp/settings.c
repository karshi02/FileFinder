/*
 * settings.c
 * - โหลด/บันทึก hotkey และ run-on-startup ลง data\settings.ini
 * - หน้าต่าง Settings: กด key combo ตรงๆ เพื่อตั้ง hotkey
 * - toggle Run at Startup ผ่าน registry
 */
#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "settings.h"

/* ── Registry path สำหรับ startup ───────────────────── */
#define REG_RUN   "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REG_NAME  "SmartFinder"

/* ================================================================
   settings_load / settings_save   (INI แบบง่าย)
   ================================================================ */

void settings_load(AppSettings *s) {
    /* ค่า default */
    s->mod_keys       = DEFAULT_MOD;
    s->vk_code        = DEFAULT_VK;
    s->run_on_startup = FALSE;

    FILE *f = fopen(SETTINGS_FILE, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        unsigned int val;
        if (sscanf(line, "mod=%u", &val) == 1)      s->mod_keys       = val;
        else if (sscanf(line, "vk=%u", &val) == 1)  s->vk_code        = val;
        else if (sscanf(line, "startup=%u",&val)==1) s->run_on_startup = (BOOL)val;
    }
    fclose(f);
}

void settings_save(const AppSettings *s) {
    /* สร้างโฟลเดอร์ data ถ้ายังไม่มี */
    CreateDirectoryA("data", NULL);

    FILE *f = fopen(SETTINGS_FILE, "w");
    if (!f) return;
    fprintf(f, "mod=%u\n", s->mod_keys);
    fprintf(f, "vk=%u\n",  s->vk_code);
    fprintf(f, "startup=%u\n", (unsigned)s->run_on_startup);
    fclose(f);
}

/* ================================================================
   startup registry
   ================================================================ */

void settings_apply_startup(BOOL enable) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, REG_RUN, 0,
                      KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
        return;

    if (enable) {
        char exe_path[MAX_PATH] = {0};
        GetModuleFileNameA(NULL, exe_path, sizeof(exe_path));
        RegSetValueExA(hKey, REG_NAME, 0, REG_SZ,
                       (BYTE *)exe_path, (DWORD)(strlen(exe_path) + 1));
    } else {
        RegDeleteValueA(hKey, REG_NAME);
    }
    RegCloseKey(hKey);
}

/* ================================================================
   settings_hotkey_to_string
   ================================================================ */

void settings_hotkey_to_string(UINT mod, UINT vk, char *out, int outlen) {
    char tmp[128] = "";

    if (mod & MOD_CONTROL) strcat(tmp, "Ctrl+");
    if (mod & MOD_ALT)     strcat(tmp, "Alt+");
    if (mod & MOD_SHIFT)   strcat(tmp, "Shift+");
    if (mod & MOD_WIN)     strcat(tmp, "Win+");

    /* ชื่อ key */
    char keyname[64] = "";
    switch (vk) {
    case VK_SPACE:  strcpy(keyname, "Space");  break;
    case VK_TAB:    strcpy(keyname, "Tab");    break;
    case VK_RETURN: strcpy(keyname, "Enter");  break;
    case VK_F1:  case VK_F2:  case VK_F3:  case VK_F4:
    case VK_F5:  case VK_F6:  case VK_F7:  case VK_F8:
    case VK_F9:  case VK_F10: case VK_F11: case VK_F12:
        snprintf(keyname, sizeof(keyname), "F%d", vk - VK_F1 + 1);
        break;
    default:
        if (vk >= 'A' && vk <= 'Z') {
            keyname[0] = (char)vk; keyname[1] = '\0';
        } else if (vk >= '0' && vk <= '9') {
            keyname[0] = (char)vk; keyname[1] = '\0';
        } else {
            snprintf(keyname, sizeof(keyname), "Key(0x%02X)", vk);
        }
    }

    strncat(tmp, keyname, sizeof(tmp) - strlen(tmp) - 1);
    strncpy(out, tmp, outlen - 1);
    out[outlen - 1] = '\0';
}

/* ================================================================
   Settings Dialog
   ================================================================ */

/* IDs ของ controls ภายใน dialog */
#define IDC_HOTKEY_LABEL    201
#define IDC_HOTKEY_BOX      202
#define IDC_HOTKEY_HINT     203
#define IDC_STARTUP_CHECK   204
#define IDC_BTN_OK          205
#define IDC_BTN_CANCEL      206
#define IDC_BTN_RESET       207

/* state ระหว่าง dialog เปิดอยู่ */
typedef struct {
    AppSettings  orig;     /* ค่าเดิมก่อนแก้ */
    AppSettings  cur;      /* ค่าที่กำลังแก้ */
    BOOL         capturing;/* กำลังรอรับ key combo */
    BOOL         changed;  /* มีการเปลี่ยนแปลง */
} DlgState;

static DlgState *g_ds = NULL;

/* อัพเดท label แสดง hotkey ปัจจุบัน */
static void update_hotkey_display(HWND hwnd) {
    char buf[128];
    settings_hotkey_to_string(g_ds->cur.mod_keys, g_ds->cur.vk_code,
                              buf, sizeof(buf));
    SetWindowTextA(GetDlgItem(hwnd, IDC_HOTKEY_BOX), buf);
}

/* ---- WndProc ของหน้าต่าง Settings ---- */
static LRESULT CALLBACK SettingsProc(HWND hwnd, UINT msg,
                                     WPARAM wParam, LPARAM lParam) {
    (void)lParam;
    switch (msg) {

    case WM_CREATE: {
        HFONT hf = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                               DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY,
                               DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        /* --- Section: Hotkey --- */
        HWND lbl = CreateWindowExA(0, "STATIC",
            "Hotkey  (กดปุ่มที่ต้องการในช่องด้านล่าง)",
            WS_CHILD | WS_VISIBLE,
            20, 20, 380, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessageA(lbl, WM_SETFONT, (WPARAM)hf, TRUE);
        (void)lbl;

        /* กล่องแสดง / รับ hotkey */
        HWND hBox = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "",
            WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
            20, 46, 260, 36, hwnd, (HMENU)IDC_HOTKEY_BOX,
            GetModuleHandle(NULL), NULL);
        SendMessageA(hBox, WM_SETFONT, (WPARAM)hf, TRUE);

        /* ปุ่ม "คลิกเพื่อเปลี่ยน" */
        HWND hCapture = CreateWindowExA(0, "BUTTON", "คลิกเพื่อตั้งค่า",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            290, 46, 110, 36, hwnd, (HMENU)IDC_BTN_RESET,
            GetModuleHandle(NULL), NULL);
        SendMessageA(hCapture, WM_SETFONT, (WPARAM)hf, TRUE);

        /* hint */
        HWND hint = CreateWindowExA(0, "STATIC",
            "กด modifier (Ctrl/Alt/Shift/Win) + ปุ่มอื่น แล้วปล่อย",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 88, 380, 18, hwnd, (HMENU)IDC_HOTKEY_HINT,
            GetModuleHandle(NULL), NULL);
        SendMessageA(hint, WM_SETFONT, (WPARAM)hf, TRUE);
        ShowWindow(hint, SW_HIDE);

        /* --- Section: Startup --- */
        HWND hStartup = CreateWindowExA(0, "BUTTON",
            "เปิดโปรแกรมอัตโนมัติเมื่อเปิด Windows",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            20, 122, 360, 24, hwnd, (HMENU)IDC_STARTUP_CHECK,
            GetModuleHandle(NULL), NULL);
        SendMessageA(hStartup, WM_SETFONT, (WPARAM)hf, TRUE);
        SendMessageA(hStartup, BM_SETCHECK,
                     g_ds->cur.run_on_startup ? BST_CHECKED : BST_UNCHECKED, 0);

        /* --- ปุ่ม OK / Cancel --- */
        HWND hOK = CreateWindowExA(0, "BUTTON", "บันทึก",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            220, 164, 90, 32, hwnd, (HMENU)IDC_BTN_OK,
            GetModuleHandle(NULL), NULL);
        SendMessageA(hOK, WM_SETFONT, (WPARAM)hf, TRUE);

        HWND hCancel = CreateWindowExA(0, "BUTTON", "ยกเลิก",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            320, 164, 80, 32, hwnd, (HMENU)IDC_BTN_CANCEL,
            GetModuleHandle(NULL), NULL);
        SendMessageA(hCancel, WM_SETFONT, (WPARAM)hf, TRUE);

        update_hotkey_display(hwnd);
        break;
    }

    /* ---- รับ key ขณะ capturing ---- */
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        if (!g_ds->capturing) break;

        UINT vk = (UINT)wParam;

        /* ถ้ากดแค่ modifier เฉยๆ ยังไม่บันทึก */
        if (vk == VK_CONTROL || vk == VK_SHIFT ||
            vk == VK_MENU    || vk == VK_LWIN  || vk == VK_RWIN)
            break;

        /* เก็บ modifier ที่กดค้างอยู่ */
        UINT mod = 0;
        if (GetKeyState(VK_CONTROL) & 0x8000) mod |= MOD_CONTROL;
        if (GetKeyState(VK_MENU)    & 0x8000) mod |= MOD_ALT;
        if (GetKeyState(VK_SHIFT)   & 0x8000) mod |= MOD_SHIFT;
        if (GetKeyState(VK_LWIN)    & 0x8000 ||
            GetKeyState(VK_RWIN)    & 0x8000)  mod |= MOD_WIN;

        /* ต้องมี modifier อย่างน้อย 1 ตัว */
        if (mod == 0) {
            SetWindowTextA(GetDlgItem(hwnd, IDC_HOTKEY_HINT),
                "ต้องกด Ctrl/Alt/Shift/Win ร่วมด้วย");
            ShowWindow(GetDlgItem(hwnd, IDC_HOTKEY_HINT), SW_SHOW);
            break;
        }

        g_ds->cur.mod_keys = mod;
        g_ds->cur.vk_code  = vk;
        g_ds->capturing    = FALSE;

        /* หยุดรับ key */
        ReleaseCapture();
        SetWindowTextA(GetDlgItem(hwnd, IDC_BTN_RESET), "คลิกเพื่อตั้งค่า");
        ShowWindow(GetDlgItem(hwnd, IDC_HOTKEY_HINT), SW_HIDE);

        update_hotkey_display(hwnd);
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);

        if (id == IDC_BTN_RESET) {
            /* toggle capture mode */
            g_ds->capturing = !g_ds->capturing;
            if (g_ds->capturing) {
                SetCapture(hwnd);
                SetWindowTextA(GetDlgItem(hwnd, IDC_BTN_RESET), "กำลังรอ...");
                SetWindowTextA(GetDlgItem(hwnd, IDC_HOTKEY_BOX),
                               "กดปุ่มที่ต้องการ...");
                ShowWindow(GetDlgItem(hwnd, IDC_HOTKEY_HINT), SW_SHOW);
            } else {
                ReleaseCapture();
                SetWindowTextA(GetDlgItem(hwnd, IDC_BTN_RESET), "คลิกเพื่อตั้งค่า");
                ShowWindow(GetDlgItem(hwnd, IDC_HOTKEY_HINT), SW_HIDE);
                update_hotkey_display(hwnd);
            }
        }
        else if (id == IDC_BTN_OK) {
            /* อ่านค่า startup checkbox */
            g_ds->cur.run_on_startup =
                (SendMessageA(GetDlgItem(hwnd, IDC_STARTUP_CHECK),
                              BM_GETCHECK, 0, 0) == BST_CHECKED);
            g_ds->changed = TRUE;
            DestroyWindow(hwnd);
        }
        else if (id == IDC_BTN_CANCEL) {
            g_ds->changed = FALSE;
            DestroyWindow(hwnd);
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

/* ================================================================
   settings_show_dialog  (public)
   ================================================================ */

BOOL settings_show_dialog(HWND parent, AppSettings *s) {
    DlgState ds;
    ds.orig      = *s;
    ds.cur       = *s;
    ds.capturing = FALSE;
    ds.changed   = FALSE;
    g_ds = &ds;

    /* register window class */
    static BOOL reg = FALSE;
    if (!reg) {
        WNDCLASSEXA wc   = {0};
        wc.cbSize        = sizeof(wc);
        wc.lpfnWndProc   = SettingsProc;
        wc.hInstance     = GetModuleHandle(NULL);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = "SmartFinderSettings";
        RegisterClassExA(&wc);
        reg = TRUE;
    }

    /* คำนวณตำแหน่งกลางจอ */
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int ww = 420, wh = 220;

    HWND hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        "SmartFinderSettings",
        "Smart Finder — ตั้งค่า",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        (sw - ww) / 2, (sh - wh) / 2,
        ww, wh,
        parent, NULL, GetModuleHandle(NULL), NULL);

    if (!hwnd) return FALSE;

    /* modal loop */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    /* ถ้า OK → บันทึก */
    if (ds.changed) {
        *s = ds.cur;
        settings_save(s);
        settings_apply_startup(s->run_on_startup);
    }

    g_ds = NULL;
    return ds.changed;
}
