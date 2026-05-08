/*
 * universal_ui.c
 * Owner-draw listbox — แสดงไอคอนจริงของแอพแต่ละตัว
 * Icons ถูก cache ไว้หลังดึงครั้งแรก เพื่อไม่ให้ช้า
 */
#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <shellapi.h>   /* ExtractIconExA, SHGetFileInfoA */
#include <commctrl.h>   /* ImageList_* for HQ icon */
#include <shlobj.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "universal_scanner.h"
#include "universal_ui.h"
#include "frequent.h"

/* ── layout ─────────────────────────────────────────────── */
#define WIN_W        700
#define WIN_H_BASE    60
#define ROW_H         44      /* สูงพอให้ไอคอน 32px + padding */
#define ICON_SIZE     32
#define ICON_PAD_X     6
#define TEXT_OFF_X    (ICON_PAD_X*2 + ICON_SIZE)   /* x เริ่มต้นของ text */
#define MAX_VISIBLE   12
#define MAX_RESULTS   50

/* ── control IDs ─────────────────────────────────────────── */
#define ID_EDIT  101
#define ID_LIST  102

/* ── icon cache ──────────────────────────────────────────── */
#define ICON_CACHE_SIZE 256

typedef struct {
    char  path[1024];
    HICON hIcon;          /* NULL = ยังไม่ได้ดึง / ไม่มีไอคอน */
    BOOL  fetched;        /* TRUE = เคยพยายามดึงแล้ว */
} IconCacheEntry;

static IconCacheEntry  g_icon_cache[ICON_CACHE_SIZE];
static int             g_icon_cache_count = 0;

/* ── globals ─────────────────────────────────────────────── */
static UniversalIndex *g_universal    = NULL;
static HWND            g_hwnd         = NULL;
static HWND            g_hEdit        = NULL;
static HWND            g_hList        = NULL;
static HFONT           g_hFont        = NULL;
static HFONT           g_hFontSmall   = NULL;
static HFONT           g_hFontEdit    = NULL;
static HICON           g_hDefaultIcon = NULL;   /* fallback icon */
static UniversalEntry *g_results[MAX_RESULTS];
static int             g_result_count = 0;
static WNDPROC         g_oldEditProc  = NULL;
static WNDPROC         g_oldListProc  = NULL;

/* settings จาก main */
static BOOL  g_cfg_show_recent     = TRUE;
static int   g_cfg_max_results     = 12;
static BOOL  g_cfg_close_on_launch = TRUE;

void universal_apply_settings(BOOL show_recent, int max_results, BOOL close_on_launch) {
    g_cfg_show_recent     = show_recent;
    g_cfg_max_results     = max_results < 5 ? 5 : (max_results > 50 ? 50 : max_results);
    g_cfg_close_on_launch = close_on_launch;
}

/* ── forward declarations ────────────────────────────────── */
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK EditProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK ListProc(HWND, UINT, WPARAM, LPARAM);
static void do_search(HWND, HWND, HWND);
static void show_recent(HWND, HWND);
static void resize_window(HWND, int);
static void open_item(const char *, const char *);
static void open_selected(HWND);
static void move_selection(int);
static HICON get_icon(const char *path);
static void  free_icon_cache(void);

/* ── callback ใน main.c ──────────────────────────────────── */
extern void on_app_launched(const char *name, const char *path);

/* ================================================================
   icon cache
   ================================================================ */

/*
 * get_icon — ดึง HICON จาก path ของ .lnk / .exe / ไฟล์อื่น
 * ใช้ SHGetFileInfoA ซึ่งรู้จัก .lnk และ resolve ได้อัตโนมัติ
 * ผลลัพธ์ถูก cache ด้วย path เป็น key
 */
static HICON get_icon(const char *path) {
    if (!path || !*path) return g_hDefaultIcon;

    /* ค้นหาใน cache ก่อน */
    for (int i = 0; i < g_icon_cache_count; i++) {
        if (strcmp(g_icon_cache[i].path, path) == 0) {
            return g_icon_cache[i].hIcon
                       ? g_icon_cache[i].hIcon
                       : g_hDefaultIcon;
        }
    }

    /* cache เต็ม → ใช้ default icon (ไม่ขยาย cache เพื่อความปลอดภัย) */
    if (g_icon_cache_count >= ICON_CACHE_SIZE)
        return g_hDefaultIcon;

    /*
     * ดึง large icon (32x32) โดยตรง — ชัดกว่า small (16x16) มาก
     * ลำดับ: ExtractIconExA (large) → SHGetFileInfoA (large) → fallback
     */
    HICON hIcon = NULL;

    /* วิธีที่ 1: ExtractIconExA — ได้ HICON ขนาด system large (32x32) */
    {
        HICON hLarge = NULL, hSmall = NULL;
        UINT n = ExtractIconExA(path, 0, &hLarge, &hSmall, 1);
        if (n > 0 && hLarge) {
            hIcon = hLarge;                     /* ใช้ large icon */
            if (hSmall) DestroyIcon(hSmall);    /* ทิ้ง small */
        } else if (n > 0 && hSmall) {
            hIcon = hSmall;
        }
    }

    /* วิธีที่ 2: SHGetFileInfoA — fallback และรองรับ .lnk resolve */
    if (!hIcon) {
        SHFILEINFOA sfi = {0};
        UINT flags = SHGFI_ICON | SHGFI_LARGEICON;   /* LARGEICON = 32x32 */
        if ((BOOL)SHGetFileInfoA(path, 0, &sfi, sizeof(sfi), flags) && sfi.hIcon)
            hIcon = sfi.hIcon;
    }

    /* วิธีที่ 3: ถ้าเป็น .lnk ให้ resolve แล้วลองอีกรอบ */
    if (!hIcon) {
        SHFILEINFOA sfi = {0};
        UINT flags = SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES;
        if ((BOOL)SHGetFileInfoA(path, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), flags)
            && sfi.hIcon)
            hIcon = sfi.hIcon;
    }

    /* บันทึก cache */
    snprintf(g_icon_cache[g_icon_cache_count].path,
             sizeof(g_icon_cache[0].path), "%s", path);
    g_icon_cache[g_icon_cache_count].hIcon   = hIcon;
    g_icon_cache[g_icon_cache_count].fetched = TRUE;
    g_icon_cache_count++;

    return hIcon ? hIcon : g_hDefaultIcon;
}

static void free_icon_cache(void) {
    for (int i = 0; i < g_icon_cache_count; i++) {
        if (g_icon_cache[i].hIcon &&
            g_icon_cache[i].hIcon != g_hDefaultIcon) {
            DestroyIcon(g_icon_cache[i].hIcon);
        }
        g_icon_cache[i].hIcon   = NULL;
        g_icon_cache[i].fetched = FALSE;
        g_icon_cache[i].path[0] = '\0';
    }
    g_icon_cache_count = 0;
}

/* ================================================================
   helper functions
   ================================================================ */

static void open_item(const char *name, const char *path) {
    if (name && path) {
        on_app_launched(name, path);
        ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWNORMAL);
    }
}

static void open_selected(HWND hwnd) {
    int sel = (int)SendMessageA(g_hList, LB_GETCURSEL, 0, 0);
    if (sel >= 0 && sel < g_result_count && g_results[sel]) {
        open_item(g_results[sel]->name, g_results[sel]->path);
        if (g_cfg_close_on_launch) DestroyWindow(hwnd);
    }
}

/* เลื่อน selection โดย focus ไม่ออกจาก edit */
static void move_selection(int delta) {
    if (!g_hList || g_result_count == 0) return;
    int cur = (int)SendMessageA(g_hList, LB_GETCURSEL, 0, 0);
    if (cur == LB_ERR) cur = -1;
    int next = cur + delta;
    if (next < 0) next = 0;
    if (next >= g_result_count) next = g_result_count - 1;
    SendMessageA(g_hList, LB_SETCURSEL, next, 0);
    /* ไม่ SetFocus(g_hList) — focus อยู่ edit ตลอด */
}

/* populate list จาก g_results */
static void populate_list(HWND hwnd, HWND hList) {
    SendMessageA(hList, LB_RESETCONTENT, 0, 0);

    for (int i = 0; i < g_result_count; i++) {
        /*
         * Owner-draw listbox: LB_ADDSTRING เก็บ string เป็น item data
         * แต่เราจะวาดเองใน WM_DRAWITEM จึงเก็บ index เป็น item data
         * และยังต้องส่ง string เพื่อให้ LB_GETTEXT ทำงานได้
         */
        SendMessageA(hList, LB_ADDSTRING, 0,
                     (LPARAM)g_results[i]->name);
    }

    if (g_result_count > 0)
        SendMessageA(hList, LB_SETCURSEL, 0, 0);

    resize_window(hwnd, g_result_count);
}

/* ----------------------------------------------------------------
   show_recent — แสดงแอพที่ใช้บ่อยตอนช่องพิมพ์ว่าง
   ---------------------------------------------------------------- */
static void show_recent(HWND hwnd, HWND hList) {
    g_result_count = 0;

    if (!g_cfg_show_recent) {
        SendMessageA(hList, LB_RESETCONTENT, 0, 0);
        resize_window(hwnd, 0);
        return;
    }

    if (!g_universal) {
        SendMessageA(hList, LB_RESETCONTENT, 0, 0);
        resize_window(hwnd, 0);
        return;
    }

    int added = 0;

    /* รอบ 1: แอพที่มี freq_score > 0 เรียงมากไปน้อย */
    for (int i = 0; i < g_universal->count && added < MAX_RESULTS; i++) {
        if (g_universal->entries[i].freq_score > 0)
            g_results[added++] = &g_universal->entries[i];
    }
    for (int i = 0; i < added - 1; i++)
        for (int j = i + 1; j < added; j++)
            if (g_results[i]->freq_score < g_results[j]->freq_score) {
                UniversalEntry *t = g_results[i];
                g_results[i] = g_results[j];
                g_results[j] = t;
            }

    /* รอบ 2: เติม app ทั่วไปจนครบ 12 */
    for (int i = 0; i < g_universal->count && added < g_cfg_max_results; i++) {
        if (g_universal->entries[i].freq_score == 0 &&
            strcmp(g_universal->entries[i].type, "app") == 0)
            g_results[added++] = &g_universal->entries[i];
    }

    g_result_count = added;

    SetWindowTextA(hwnd,
        "Smart Finder  —  type to search  |  up/down  |  Enter to open  |  ESC to close");

    populate_list(hwnd, hList);
    SetFocus(g_hEdit);
}

/* ================================================================
   popup API
   ================================================================ */

void universal_close_popup(void) {
    if (g_hwnd && IsWindow(g_hwnd)) {
        DestroyWindow(g_hwnd);
        g_hwnd  = NULL;
        g_hEdit = NULL;
        g_hList = NULL;
    }
}

void universal_show_popup(UniversalIndex *idx) {
    /* toggle: ถ้าเปิดอยู่แล้ว ปิด */
    if (g_hwnd && IsWindow(g_hwnd)) { universal_close_popup(); return; }
    if (!idx) return;
    g_universal = idx;

    static BOOL registered = FALSE;
    if (!registered) {
        WNDCLASSEXA wc   = {0};
        wc.cbSize        = sizeof(wc);
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = GetModuleHandle(NULL);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(RGB(28, 28, 30));
        wc.lpszClassName = "UniversalFinder";
        RegisterClassExA(&wc);
        registered = TRUE;
    }

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    /* ── Step 1: attach thread input ก่อน create window ──
       ทำก่อน CreateWindowExA เพราะ WM_CREATE จะเรียก SetFocus ภายใน */
    HWND fg_win   = GetForegroundWindow();
    DWORD fg_tid  = GetWindowThreadProcessId(fg_win, NULL);
    DWORD our_tid = GetCurrentThreadId();
    BOOL  attached = FALSE;

    if (fg_tid && fg_tid != our_tid) {
        attached = AttachThreadInput(our_tid, fg_tid, TRUE);
    }

    /* ── Step 2: สร้าง window ไม่ใส่ WS_VISIBLE ก่อน ──
       เพื่อให้ attach thread input ทำงานก่อน window ปรากฏ */
    g_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "UniversalFinder",
        "Smart Finder",
        WS_POPUP | WS_CAPTION,   /* ไม่มี WS_VISIBLE */
        (sw - WIN_W) / 2, sh / 4,
        WIN_W, WIN_H_BASE,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    if (g_hwnd) {
        /* ── Step 3: บังคับ foreground ── */
        SetWindowPos(g_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE);
        ShowWindow(g_hwnd, SW_SHOW);
        SetForegroundWindow(g_hwnd);

        /* ── Step 4: focus ไป edit ── */
        if (g_hEdit && IsWindow(g_hEdit)) {
            SetFocus(g_hEdit);
            /* clear text เก่าออก */
            SetWindowTextA(g_hEdit, "");
        }

        /* ── Step 5: detach ── */
        if (attached) AttachThreadInput(our_tid, fg_tid, FALSE);

        /* ── Step 6: reload recent ── */
        if (g_hList && IsWindow(g_hList))
            show_recent(g_hwnd, g_hList);

        UpdateWindow(g_hwnd);
    } else {
        if (attached) AttachThreadInput(our_tid, fg_tid, FALSE);
    }
}

/* ================================================================
   Edit subclass
   ================================================================ */
static LRESULT CALLBACK EditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN) {
        switch (wParam) {
        case VK_DOWN:   move_selection(+1); return 0;
        case VK_UP:     move_selection(-1); return 0;
        case VK_RETURN: if (g_result_count > 0) open_selected(g_hwnd); return 0;
        case VK_ESCAPE: if (g_hwnd) DestroyWindow(g_hwnd); return 0;
        }
    }
    return CallWindowProcA(g_oldEditProc, hWnd, uMsg, wParam, lParam);
}

/* ================================================================
   List subclass — ตัวอักษร / backspace → โยนกลับ edit
   ================================================================ */
static LRESULT CALLBACK ListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN) {
        switch (wParam) {
        case VK_DOWN:   move_selection(+1); SetFocus(g_hEdit); return 0;
        case VK_UP:     move_selection(-1); SetFocus(g_hEdit); return 0;
        case VK_RETURN: if (g_result_count > 0) open_selected(g_hwnd); return 0;
        case VK_ESCAPE: if (g_hwnd) DestroyWindow(g_hwnd); return 0;
        case VK_BACK:
            SetFocus(g_hEdit);
            SendMessageA(g_hEdit, WM_KEYDOWN, VK_BACK, lParam);
            return 0;
        default:
            if (wParam >= 0x20 && wParam <= 0x7E) {
                SetFocus(g_hEdit);
                SendMessageA(g_hEdit, WM_CHAR, wParam, lParam);
                return 0;
            }
        }
    }
    return CallWindowProcA(g_oldListProc, hWnd, uMsg, wParam, lParam);
}

/* ================================================================
   WM_DRAWITEM — วาด row ด้วยมือ: icon + name + path (เล็ก)
   ================================================================ */
static void draw_item(DRAWITEMSTRUCT *di) {
    if (!di || di->itemID == (UINT)-1) return;
    if (di->itemID >= (UINT)g_result_count) return;

    UniversalEntry *entry = g_results[di->itemID];
    HDC dc    = di->hDC;
    RECT rc   = di->rcItem;

    /* ── สีพื้นหลัง ── */
    BOOL selected = (di->itemState & ODS_SELECTED) != 0;
    COLORREF bg   = selected ? RGB(0, 120, 215) : RGB(28, 28, 30);
    COLORREF fg   = selected ? RGB(255, 255, 255) : RGB(220, 220, 220);
    COLORREF fgSub= selected ? RGB(200, 230, 255) : RGB(130, 130, 130);

    HBRUSH hBrush = CreateSolidBrush(bg);
    FillRect(dc, &rc, hBrush);
    DeleteObject(hBrush);

    /* ── icon ── */
    HICON hIcon = get_icon(entry->path);
    if (hIcon) {
        int iy = rc.top + (ROW_H - ICON_SIZE) / 2;
        DrawIconEx(dc, rc.left + ICON_PAD_X, iy,
                   hIcon, ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL | DI_COMPAT);
    }

    /* ── ชื่อแอพ (main text) ── */
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, fg);

    HFONT oldFont = (HFONT)SelectObject(dc, g_hFont);

    RECT rcName = {
        rc.left + TEXT_OFF_X,
        rc.top + 3,
        rc.right - 8,
        rc.top + ROW_H / 2 + 2
    };
    DrawTextA(dc, entry->name, -1, &rcName,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    /* ── path (subtext เล็กๆ) ── */
    SelectObject(dc, g_hFontSmall);
    SetTextColor(dc, fgSub);

    RECT rcPath = {
        rc.left + TEXT_OFF_X,
        rc.top + ROW_H / 2 + 2,
        rc.right - 8,
        rc.bottom - 2
    };
    DrawTextA(dc, entry->path, -1, &rcPath,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    SelectObject(dc, oldFont);

    /* ── เส้นคั่น ── */
    if (!selected) {
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 52));
        HPEN old  = (HPEN)SelectObject(dc, hPen);
        MoveToEx(dc, rc.left, rc.bottom - 1, NULL);
        LineTo  (dc, rc.right, rc.bottom - 1);
        SelectObject(dc, old);
        DeleteObject(hPen);
    }
}

/* ================================================================
   WndProc
   ================================================================ */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    /* ── ป้องกัน flicker ── */
    case WM_ERASEBKGND: {
        HDC dc = (HDC)wParam;
        RECT rc; GetClientRect(hwnd, &rc);
        HBRUSH hb = CreateSolidBrush(RGB(28, 28, 30));
        FillRect(dc, &rc, hb);
        DeleteObject(hb);
        return 1;
    }

    /* ── สร้าง controls ── */
    case WM_CREATE: {
        /* font หลัก */
        g_hFont = CreateFontA(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        /* font เล็กสำหรับ path */
        g_hFontSmall = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        /* default icon (generic application icon) */
        g_hDefaultIcon = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION);

        /* edit box */
        g_hEdit = CreateWindowExA(
            WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT,
            10, 10, WIN_W - 20, 40,
            hwnd, (HMENU)ID_EDIT, GetModuleHandle(NULL), NULL);

        /* ตั้งค่า font ใหญ่สำหรับ edit */
        g_hFontEdit = CreateFontA(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                      CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                      DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        SendMessageA(g_hEdit, WM_SETFONT, (WPARAM)g_hFontEdit, TRUE);

        g_oldEditProc = (WNDPROC)SetWindowLongPtrA(
            g_hEdit, GWLP_WNDPROC, (LONG_PTR)EditProc);

        /*
         * listbox — ใช้ LBS_OWNERDRAWFIXED เพื่อวาด icon + text เอง
         * LBS_HASSTRINGS ยังต้องใส่ เพื่อให้ LB_ADDSTRING / LB_GETTEXT ทำงาน
         */
        g_hList = CreateWindowExA(
            0, "LISTBOX", "",
            WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_OWNERDRAWFIXED |
            LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP,
            10, 60, WIN_W - 20, 300,
            hwnd, (HMENU)ID_LIST, GetModuleHandle(NULL), NULL);
        SendMessageA(g_hList, LB_SETITEMHEIGHT, 0, ROW_H);

        g_oldListProc = (WNDPROC)SetWindowLongPtrA(
            g_hList, GWLP_WNDPROC, (LONG_PTR)ListProc);

        /* placeholder hint ใน edit */
        SendMessageA(g_hEdit, EM_SETCUEBANNER, FALSE,
                     (LPARAM)L"Type to search apps...");
        /* show_recent และ SetFocus ทำใน show_popup หลัง attach thread */
        break;
    }

    /* ── resize controls ── */
    case WM_SIZE: {
        int w = LOWORD(lParam), h = HIWORD(lParam);
        if (g_hEdit) SetWindowPos(g_hEdit, NULL, 10, 10, w-20, 40, SWP_NOZORDER);
        if (g_hList) SetWindowPos(g_hList, NULL, 10, 60, w-20, h-70, SWP_NOZORDER);
        break;
    }

    /* ── owner draw: วาด row ── */
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
        if (di->CtlID == ID_LIST) draw_item(di);
        return TRUE;
    }

    /* ── owner draw: บอกความสูง row ── */
    case WM_MEASUREITEM: {
        MEASUREITEMSTRUCT *mi = (MEASUREITEMSTRUCT *)lParam;
        if (mi->CtlID == ID_LIST) mi->itemHeight = ROW_H;
        return TRUE;
    }

    /* ── commands ── */
    case WM_COMMAND:
        /* edit เปลี่ยน → search */
        if (LOWORD(wParam) == ID_EDIT && HIWORD(wParam) == EN_CHANGE) {
            char query[256] = {0};
            GetWindowTextA(g_hEdit, query, sizeof(query));
            if (strlen(query) == 0)
                show_recent(hwnd, g_hList);
            else
                do_search(hwnd, g_hEdit, g_hList);
        }
        /* double-click */
        if (LOWORD(wParam) == ID_LIST && HIWORD(wParam) == LBN_DBLCLK)
            open_selected(hwnd);
        /* single-click → คืน focus edit */
        if (LOWORD(wParam) == ID_LIST && HIWORD(wParam) == LBN_SELCHANGE)
            SetFocus(g_hEdit);
        break;

    /* ── cleanup ── */
    case WM_DESTROY:
        free_icon_cache();
        if (g_hFont)      { DeleteObject(g_hFont);      g_hFont      = NULL; }
        if (g_hFontSmall) { DeleteObject(g_hFontSmall); g_hFontSmall = NULL; }
        if (g_hFontEdit)  { DeleteObject(g_hFontEdit);  g_hFontEdit  = NULL; }
        if (g_oldEditProc && g_hEdit)
            SetWindowLongPtrA(g_hEdit, GWLP_WNDPROC, (LONG_PTR)g_oldEditProc);
        if (g_oldListProc && g_hList)
            SetWindowLongPtrA(g_hList, GWLP_WNDPROC, (LONG_PTR)g_oldListProc);
        g_hwnd = g_hEdit = g_hList = NULL;
        g_oldEditProc = g_oldListProc = NULL;
        break;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

/* ================================================================
   do_search
   ================================================================ */
static void do_search(HWND hwnd, HWND hEdit, HWND hList) {
    char query[256] = {0};
    GetWindowTextA(hEdit, query, sizeof(query));

    g_result_count = 0;
    UniversalEntry *temp[MAX_RESULTS];
    int temp_count = 0;
    universal_search(g_universal, query, temp, &temp_count);

    int limit = temp_count < g_cfg_max_results ? temp_count : g_cfg_max_results;
    for (int i = 0; i < limit; i++) g_results[i] = temp[i];
    g_result_count = limit;

    char title[512];
    snprintf(title, sizeof(title),
             "Smart Finder  —  %d result%s for \"%s\"",
             g_result_count, g_result_count == 1 ? "" : "s", query);
    SetWindowTextA(hwnd, title);

    populate_list(hwnd, hList);
}

/* ================================================================
   resize_window
   ================================================================ */
static void resize_window(HWND hwnd, int result_count) {
    int vis = result_count < MAX_VISIBLE ? result_count : MAX_VISIBLE;
    if (vis < 1) vis = 1;
    SetWindowPos(hwnd, NULL, 0, 0, WIN_W,
                 WIN_H_BASE + vis * ROW_H + 20,
                 SWP_NOMOVE | SWP_NOZORDER);
}
