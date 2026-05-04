#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "search.h"
#include "indexer.h"
#include "app_scanner.h"

#define WIN_W        560
#define WIN_H_BASE   58
#define ROW_H        28
#define MAX_VISIBLE  8
#define ID_EDIT      101
#define ID_LIST      102

static Index        *g_idx     = NULL;
static AppIndex     *g_app_idx = NULL;
static SearchResult  g_result;
static HWND          g_hwnd    = NULL;
static HWND          g_hEdit   = NULL;
static HWND          g_hList   = NULL;
static HFONT         g_hFont   = NULL;

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static void do_search(HWND hwnd, HWND hEdit, HWND hList);
static void resize_window(HWND hwnd, int result_count);

void ui_open_file(const char *path) {
    if (path) {
        ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWNORMAL);
    }
}

void ui_close_popup(void) {
    if (g_hwnd && IsWindow(g_hwnd)) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
        g_hEdit = NULL;
        g_hList = NULL;
    }
}

void ui_show_popup(Index *idx, AppIndex *app_idx) {
    if (g_hwnd && IsWindow(g_hwnd)) {
        ui_close_popup();
        return;
    }

    if (!idx && !app_idx) return;

    g_idx = idx;
    g_app_idx = app_idx;
    g_result.count = 0;

    static BOOL registered = FALSE;
    if (!registered) {
        WNDCLASSEXA wc = {0};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 32));
        wc.lpszClassName = "SmartFinderPopup";
        RegisterClassExA(&wc);
        registered = TRUE;
    }

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int x = (sw - WIN_W) / 2;
    int y = sh / 4;

    g_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "SmartFinderPopup", "Smart File & App Finder",
        WS_POPUP | WS_VISIBLE | WS_CAPTION,
        x, y, WIN_W, WIN_H_BASE,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (g_hwnd) {
        SetForegroundWindow(g_hwnd);
        SetFocus(g_hwnd);
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        g_hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        g_hEdit = CreateWindowExA(0, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER,
            10, 10, WIN_W - 20, 36,
            hwnd, (HMENU)ID_EDIT, GetModuleHandle(NULL), NULL);
        SendMessageA(g_hEdit, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SetFocus(g_hEdit);

        g_hList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "",
            WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_BORDER,
            10, 56, WIN_W - 20, 200,
            hwnd, (HMENU)ID_LIST, GetModuleHandle(NULL), NULL);
        SendMessageA(g_hList, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessageA(g_hList, LB_SETITEMHEIGHT, 0, ROW_H);
        break;
    }

    case WM_SIZE: {
        if (g_hEdit && g_hList) {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            SetWindowPos(g_hEdit, NULL, 10, 10, width - 20, 36, SWP_NOZORDER);
            SetWindowPos(g_hList, NULL, 10, 56, width - 20, height - 66, SWP_NOZORDER);
        }
        break;
    }

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_EDIT && HIWORD(wParam) == EN_CHANGE) {
            do_search(hwnd, g_hEdit, g_hList);
        }
        if (LOWORD(wParam) == ID_LIST && HIWORD(wParam) == LBN_DBLCLK) {
            int sel = (int)SendMessageA(g_hList, LB_GETCURSEL, 0, 0);
            if (sel >= 0) {
                // TODO: แยกว่าเป็นแอพหรือไฟล์
            }
        }
        break;

    case WM_CHAR:
        if (wParam == VK_RETURN) {
            int sel = (int)SendMessageA(g_hList, LB_GETCURSEL, 0, 0);
            if (sel >= 0) {
                // TODO: แยกว่าเป็นแอพหรือไฟล์
            }
        }
        break;

    case WM_DESTROY:
        if (g_hFont) {
            DeleteObject(g_hFont);
            g_hFont = NULL;
        }
        g_hwnd = NULL;
        g_hEdit = NULL;
        g_hList = NULL;
        break;

    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

static void do_search(HWND hwnd, HWND hEdit, HWND hList) {
    char query[256] = {0};
    GetWindowTextA(hEdit, query, sizeof(query));

    if (!g_idx && !g_app_idx) return;

    if (strlen(query) == 0) {
        SendMessageA(hList, LB_RESETCONTENT, 0, 0);
        resize_window(hwnd, 0);
        return;
    }

    SendMessageA(hList, LB_RESETCONTENT, 0, 0);
    int total_results = 0;

    // ค้นหาแอพ
    if (g_app_idx) {
        AppSearchResult app_result;
        search_apps(g_app_idx, query, &app_result);
        
        for (int i = 0; i < app_result.count && total_results < MAX_RESULTS; i++) {
            if (!app_result.entries[i]) continue;
            char display[512];
            snprintf(display, sizeof(display), "[APP] %s", app_result.entries[i]->name);
            SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)display);
            total_results++;
        }
    }

    // ค้นหาไฟล์
    if (g_idx) {
        search_query(g_idx, query, &g_result);
        
        for (int i = 0; i < g_result.count && total_results < MAX_RESULTS; i++) {
            if (!g_result.entries[i]) continue;
            char display[512];
            snprintf(display, sizeof(display), "[FILE] %s", g_result.entries[i]->name);
            SendMessageA(hList, LB_ADDSTRING, 0, (LPARAM)display);
            total_results++;
        }
    }

    if (total_results > 0) {
        SendMessageA(hList, LB_SETCURSEL, 0, 0);
    }

    resize_window(hwnd, total_results);
}

static void resize_window(HWND hwnd, int result_count) {
    int visible = (result_count < MAX_VISIBLE) ? result_count : MAX_VISIBLE;
    if (visible < 1 && result_count > 0) visible = 1;
    if (visible < 1) visible = 1;

    int new_h = WIN_H_BASE + visible * ROW_H + 10;

    RECT wr;
    GetWindowRect(hwnd, &wr);
    SetWindowPos(hwnd, NULL, wr.left, wr.top, WIN_W, new_h, SWP_NOZORDER);
}
