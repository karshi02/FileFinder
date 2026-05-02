#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "universal_scanner.h"
#include "universal_ui.h"

#define WIN_W        700
#define WIN_H_BASE   60
#define ROW_H        32
#define MAX_VISIBLE  12
#define ID_EDIT      101
#define ID_LIST      102

static UniversalIndex *g_universal = NULL;
static HWND           g_hwnd      = NULL;
static HWND           g_hEdit     = NULL;
static HWND           g_hList     = NULL;
static HFONT          g_hFont     = NULL;
static UniversalEntry *g_results[500];
static int            g_result_count = 0;
static WNDPROC        g_oldEditProc = NULL;

// Forward declarations
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK EditProc(HWND, UINT, WPARAM, LPARAM);
static void do_search(HWND hwnd, HWND hEdit, HWND hList);
static void resize_window(HWND hwnd, int result_count);
static void open_item(const char *name, const char *path);
static void open_selected(HWND hwnd);

// External function from main.c
extern void on_app_launched(const char *name, const char *path);

static void open_item(const char *name, const char *path) {
    if (path && name) {
        on_app_launched(name, path);
        ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWNORMAL);
    }
}

static void open_selected(HWND hwnd) {
    int sel = (int)SendMessageA(g_hList, LB_GETCURSEL, 0, 0);
    if (sel >= 0 && sel < g_result_count && g_results[sel]) {
        open_item(g_results[sel]->name, g_results[sel]->path);
        DestroyWindow(hwnd);
    }
}

void universal_close_popup(void) {
    if (g_hwnd && IsWindow(g_hwnd)) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
        g_hEdit = NULL;
        g_hList = NULL;
    }
}

void universal_show_popup(UniversalIndex *idx) {
    if (g_hwnd && IsWindow(g_hwnd)) {
        universal_close_popup();
        return;
    }

    if (!idx) return;

    g_universal = idx;

    static BOOL registered = FALSE;
    if (!registered) {
        WNDCLASSEXA wc = {0};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 32));
        wc.lpszClassName = "UniversalFinder";
        RegisterClassExA(&wc);
        registered = TRUE;
    }

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int x = (sw - WIN_W) / 2;
    int y = sh / 4;

    g_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "UniversalFinder", 
        "🔍 Search Everything (Type to search, ↓↑ to select, Enter to open)",
        WS_POPUP | WS_VISIBLE | WS_CAPTION,
        x, y, WIN_W, WIN_H_BASE,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (g_hwnd) {
        SetForegroundWindow(g_hwnd);
        SetFocus(g_hEdit);
    }
}

// Custom Edit control procedure to capture arrow keys
static LRESULT CALLBACK EditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_DOWN:
            if (g_hList && g_result_count > 0) {
                int curSel = (int)SendMessageA(g_hList, LB_GETCURSEL, 0, 0);
                if (curSel < g_result_count - 1) {
                    SendMessageA(g_hList, LB_SETCURSEL, curSel + 1, 0);
                } else if (curSel == -1 && g_result_count > 0) {
                    SendMessageA(g_hList, LB_SETCURSEL, 0, 0);
                }
                SetFocus(g_hList);
                return 0;
            }
            break;
            
        case VK_UP:
            if (g_hList && g_result_count > 0) {
                int curSel = (int)SendMessageA(g_hList, LB_GETCURSEL, 0, 0);
                if (curSel > 0) {
                    SendMessageA(g_hList, LB_SETCURSEL, curSel - 1, 0);
                }
                SetFocus(g_hList);
                return 0;
            }
            break;
            
        case VK_RETURN:
            if (g_hList && g_result_count > 0) {
                open_selected(g_hwnd);
                return 0;
            }
            break;
            
        case VK_ESCAPE:
            if (g_hwnd) {
                DestroyWindow(g_hwnd);
                return 0;
            }
            break;
        }
        break;
    }
    // Call original procedure
    return CallWindowProcA(g_oldEditProc, hWnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        g_hFont = CreateFontA(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        g_hEdit = CreateWindowExA(0, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER | ES_LEFT,
            10, 10, WIN_W - 20, 40,
            hwnd, (HMENU)ID_EDIT, GetModuleHandle(NULL), NULL);
        SendMessageA(g_hEdit, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        
        // Subclass edit control
        g_oldEditProc = (WNDPROC)SetWindowLongPtrA(g_hEdit, GWLP_WNDPROC, (LONG_PTR)EditProc);
        
        g_hList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "",
            WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_BORDER | WS_VSCROLL | WS_TABSTOP,
            10, 60, WIN_W - 20, 300,
            hwnd, (HMENU)ID_LIST, GetModuleHandle(NULL), NULL);
        SendMessageA(g_hList, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessageA(g_hList, LB_SETITEMHEIGHT, 0, ROW_H);
        
        SetFocus(g_hEdit);
        break;
    }

    case WM_SIZE: {
        if (g_hEdit && g_hList) {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            SetWindowPos(g_hEdit, NULL, 10, 10, width - 20, 40, SWP_NOZORDER);
            SetWindowPos(g_hList, NULL, 10, 60, width - 20, height - 70, SWP_NOZORDER);
        }
        break;
    }

    case WM_KEYDOWN: {
        switch (wParam) {
        case VK_ESCAPE:
            DestroyWindow(hwnd);
            return 0;

        case VK_RETURN:
            open_selected(hwnd);
            return 0;
            
        case VK_DOWN:
            if (g_hList && g_result_count > 0) {
                int curSel = (int)SendMessageA(g_hList, LB_GETCURSEL, 0, 0);
                if (curSel < g_result_count - 1) {
                    SendMessageA(g_hList, LB_SETCURSEL, curSel + 1, 0);
                } else if (curSel == -1 && g_result_count > 0) {
                    SendMessageA(g_hList, LB_SETCURSEL, 0, 0);
                }
                SetFocus(g_hList);
                return 0;
            }
            break;
            
        case VK_UP:
            if (g_hList && g_result_count > 0) {
                int curSel = (int)SendMessageA(g_hList, LB_GETCURSEL, 0, 0);
                if (curSel > 0) {
                    SendMessageA(g_hList, LB_SETCURSEL, curSel - 1, 0);
                }
                SetFocus(g_hList);
                return 0;
            }
            break;
        }
        break;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_EDIT && HIWORD(wParam) == EN_CHANGE) {
            do_search(hwnd, g_hEdit, g_hList);
        }
        if (LOWORD(wParam) == ID_LIST && HIWORD(wParam) == LBN_DBLCLK) {
            open_selected(hwnd);
        }
        break;

    case WM_DESTROY:
        if (g_hFont) DeleteObject(g_hFont);
        if (g_oldEditProc) {
            SetWindowLongPtrA(g_hEdit, GWLP_WNDPROC, (LONG_PTR)g_oldEditProc);
        }
        g_hwnd = NULL;
        g_hEdit = NULL;
        g_hList = NULL;
        break;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static void do_search(HWND hwnd, HWND hEdit, HWND hList) {
    char query[256] = {0};
    GetWindowTextA(hEdit, query, sizeof(query));

    SendMessageA(hList, LB_RESETCONTENT, 0, 0);
    g_result_count = 0;

    if (strlen(query) == 0) {
        resize_window(hwnd, 0);
        return;
    }

    universal_search(g_universal, query, g_results, &g_result_count);

    // Debug output
    char debug_title[512];
    snprintf(debug_title, sizeof(debug_title), 
             "🔍 Found %d results for '%s' (↓↑ to select, Enter to open)", 
             g_result_count, query);
    SetWindowTextA(hwnd, debug_title);

    for (int i = 0; i < g_result_count; i++) {
        char display[1024];
        const char *icon = "";

        // ตรวจสอบ type โดยตรง (ไม่ต้องเช็ค null เพราะเป็น array)
        if (strcmp(g_results[i]->type, "app") == 0) icon = "📱 ";
        else if (strcmp(g_results[i]->type, "program") == 0) icon = "⚙️ ";
        else if (strcmp(g_results[i]->type, "recent") == 0) icon = "📄 ";
        else icon = "📁 ";

        snprintf(display, sizeof(display), "%s%s", icon, g_results[i]->name);
        SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)display);
    }

    if (g_result_count > 0) {
        SendMessageA(hList, LB_SETCURSEL, 0, 0);
    }

    resize_window(hwnd, g_result_count);
}

static void resize_window(HWND hwnd, int result_count) {
    int visible = (result_count < MAX_VISIBLE) ? result_count : MAX_VISIBLE;
    if (visible < 1 && result_count > 0) visible = 1;
    if (visible < 1) visible = 1;

    int new_h = WIN_H_BASE + visible * ROW_H + 20;
    SetWindowPos(hwnd, NULL, 0, 0, WIN_W, new_h, SWP_NOMOVE | SWP_NOZORDER);
}
