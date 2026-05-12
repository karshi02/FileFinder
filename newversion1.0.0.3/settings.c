#include "settings.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <commctrl.h>

#define REG_RUN  "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REG_NAME "SmartFinder"

/* ================================================================
   Load / Save
   ================================================================ */
void settings_load(AppSettings *s) {
    // ค่า default ก่อน
    s->mod_keys = 0;
    s->vk_code = 0;
    s->run_on_startup = 1;
    s->show_recent = 1;
    s->max_results = 12;
    s->close_on_launch = 1;

    FILE *f = fopen(SETTINGS_FILE, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        unsigned int val;
        if      (sscanf(line, "mod=%u", &val) == 1) s->mod_keys = val;
        else if (sscanf(line, "vk=%u", &val) == 1) s->vk_code = val;
        else if (sscanf(line, "startup=%u", &val) == 1) s->run_on_startup = val;
        else if (sscanf(line, "show_recent=%u", &val) == 1) s->show_recent = val;
        else if (sscanf(line, "max_results=%u", &val) == 1) s->max_results = (int)val;
        else if (sscanf(line, "close_on_launch=%u", &val) == 1) s->close_on_launch = val;
    }
    fclose(f);

    if (s->max_results < 5) s->max_results = 5;
    if (s->max_results > 50) s->max_results = 12;
}

void settings_save(const AppSettings *s) {
    CreateDirectoryA("data", NULL);
    FILE *f = fopen(SETTINGS_FILE, "w");
    if (!f) return;
    fprintf(f, "mod=%u\n", s->mod_keys);
    fprintf(f, "vk=%u\n", s->vk_code);
    fprintf(f, "startup=%u\n", (unsigned)s->run_on_startup);
    fprintf(f, "show_recent=%u\n", (unsigned)s->show_recent);
    fprintf(f, "max_results=%d\n", s->max_results);
    fprintf(f, "close_on_launch=%u\n", (unsigned)s->close_on_launch);
    fclose(f);
}

/* ================================================================
   Startup registry
   ================================================================ */
void settings_apply_startup(BOOL enable) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, REG_RUN, 0,
                      KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) return;
    if (enable) {
        char exe[MAX_PATH] = {0};
        GetModuleFileNameA(NULL, exe, sizeof(exe));
        RegSetValueExA(hKey, REG_NAME, 0, REG_SZ,
                       (BYTE*)exe, (DWORD)(strlen(exe)+1));
    } else {
        RegDeleteValueA(hKey, REG_NAME);
    }
    RegCloseKey(hKey);
}

void settings_ensure_startup_once(void) {
    HKEY hKey;
    char exe_path[MAX_PATH];
    char existing_value[MAX_PATH] = {0};
    DWORD size = sizeof(existing_value);
    
    GetModuleFileNameA(NULL, exe_path, sizeof(exe_path));
    
    if (RegOpenKeyExA(HKEY_CURRENT_USER, REG_RUN, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, REG_NAME, NULL, NULL, (BYTE*)existing_value, &size) == ERROR_SUCCESS) {
            if (strcmp(existing_value, exe_path) == 0) {
                RegCloseKey(hKey);
                return;
            }
        }
        RegCloseKey(hKey);
    }
    
    settings_apply_startup(1);
}

/* ================================================================
   Hotkey → string
   ================================================================ */
void settings_hotkey_to_string(UINT mod, UINT vk, char *out, int outlen) {
    char tmp[128] = "";
    if (mod & MOD_CONTROL) strcat(tmp, "Ctrl+");
    if (mod & MOD_ALT)     strcat(tmp, "Alt+");
    if (mod & MOD_SHIFT)   strcat(tmp, "Shift+");
    if (mod & MOD_WIN)     strcat(tmp, "Win+");

    char key[32] = "";
    if (vk == VK_SPACE) strcpy(key, "Space");
    else if (vk == VK_TAB) strcpy(key, "Tab");
    else if (vk == VK_RETURN) strcpy(key, "Enter");
    else if (vk >= 'A' && vk <= 'Z') { key[0] = (char)vk; key[1] = '\0'; }
    else if (vk >= '0' && vk <= '9') { key[0] = (char)vk; key[1] = '\0'; }
    else if (vk >= VK_F1 && vk <= VK_F12) {
        snprintf(key, sizeof(key), "F%d", vk - VK_F1 + 1);
    }
    else snprintf(key, sizeof(key), "0x%02X", vk);
    
    strncat(tmp, key, sizeof(tmp)-strlen(tmp)-1);
    strncpy(out, tmp, outlen-1);
    out[outlen-1] = '\0';
}

/* ================================================================
   Settings Dialog (ถ้าต้องการให้มี แต่ตอนนี้ไม่ใช้แล้ว)
   ================================================================ */
/* ================================================================
   Settings Dialog — ทำงานจริง
   ================================================================ */
#define ID_HK_BOX       202
#define ID_HK_BTN       203
#define ID_HK_HINT      204
#define ID_CB_STARTUP   210
#define ID_CB_RECENT    211
#define ID_CB_CLOSE     212
#define ID_SL_RESULTS   213
#define ID_LB_RESULTS   214
#define ID_BTN_OK       220
#define ID_BTN_CANCEL   221
#define ID_BTN_RESET    222

typedef struct { AppSettings orig, cur; BOOL capturing, changed; } DlgState;
static DlgState *g_ds = NULL;

static void upd_hk(HWND hwnd) {
    char buf[64];
    settings_hotkey_to_string(g_ds->cur.mod_keys, g_ds->cur.vk_code, buf, sizeof(buf));
    SetWindowTextA(GetDlgItem(hwnd, ID_HK_BOX), buf);
}
static void upd_res(HWND hwnd) {
    char buf[64];
    snprintf(buf, sizeof(buf), "Results: %d", g_ds->cur.max_results);
    SetWindowTextA(GetDlgItem(hwnd, ID_LB_RESULTS), buf);
}

static LRESULT CALLBACK SettingsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    (void)lParam;
    switch (msg) {
    case WM_CREATE: {
        HFONT hf = CreateFontA(15,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,
                               CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Segoe UI");
        int x=20,y=16,lw=420; HWND h;
        h=CreateWindowExA(0,"STATIC","Hotkey",WS_CHILD|WS_VISIBLE,x,y,lw,18,hwnd,0,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE); y+=22;
        h=CreateWindowExA(WS_EX_CLIENTEDGE,"STATIC","",WS_CHILD|WS_VISIBLE|SS_CENTER|SS_CENTERIMAGE,
            x,y,180,32,hwnd,(HMENU)ID_HK_BOX,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE);
        h=CreateWindowExA(0,"BUTTON","Change",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            x+190,y,100,32,hwnd,(HMENU)ID_HK_BTN,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE);
        h=CreateWindowExA(0,"BUTTON","Reset",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            x+300,y,80,32,hwnd,(HMENU)ID_BTN_RESET,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE); y+=38;
        h=CreateWindowExA(0,"STATIC","Press Ctrl/Alt/Shift/Win + key",
            WS_CHILD|SS_LEFT,x,y,lw,16,hwnd,(HMENU)ID_HK_HINT,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE);
        ShowWindow(h,SW_HIDE); y+=26;
        h=CreateWindowExA(0,"BUTTON","Run at startup",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,x,y,lw,22,hwnd,(HMENU)ID_CB_STARTUP,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE);
        SendMessageA(h,BM_SETCHECK,g_ds->cur.run_on_startup?BST_CHECKED:BST_UNCHECKED,0); y+=28;
        h=CreateWindowExA(0,"BUTTON","Show recent apps on open",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,x,y,lw,22,hwnd,(HMENU)ID_CB_RECENT,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE);
        SendMessageA(h,BM_SETCHECK,g_ds->cur.show_recent?BST_CHECKED:BST_UNCHECKED,0); y+=28;
        h=CreateWindowExA(0,"BUTTON","Close popup after launch",
            WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,x,y,lw,22,hwnd,(HMENU)ID_CB_CLOSE,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE);
        SendMessageA(h,BM_SETCHECK,g_ds->cur.close_on_launch?BST_CHECKED:BST_UNCHECKED,0); y+=32;
        h=CreateWindowExA(0,"STATIC","Max results:",WS_CHILD|WS_VISIBLE,x,y,lw,16,hwnd,0,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE); y+=20;
        h=CreateWindowExA(0,TRACKBAR_CLASS,"",WS_CHILD|WS_VISIBLE|TBS_HORZ|TBS_AUTOTICKS,
            x,y,240,28,hwnd,(HMENU)ID_SL_RESULTS,0,0);
        SendMessageA(h,TBM_SETRANGE,TRUE,MAKELONG(5,50));
        SendMessageA(h,TBM_SETPOS,TRUE,g_ds->cur.max_results);
        h=CreateWindowExA(0,"STATIC","",WS_CHILD|WS_VISIBLE,x+250,y+4,100,20,hwnd,(HMENU)ID_LB_RESULTS,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE); y+=40;
        h=CreateWindowExA(0,"BUTTON","Save",WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,
            lw-160,y,70,30,hwnd,(HMENU)ID_BTN_OK,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE);
        h=CreateWindowExA(0,"BUTTON","Cancel",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            lw-80,y,70,30,hwnd,(HMENU)ID_BTN_CANCEL,0,0);
        SendMessageA(h,WM_SETFONT,(WPARAM)hf,TRUE);
        upd_hk(hwnd); upd_res(hwnd);
        break;
    }
    case WM_KEYDOWN: case WM_SYSKEYDOWN: {
        if (!g_ds->capturing) break;
        UINT vk=(UINT)wParam;
        if (vk==VK_CONTROL||vk==VK_SHIFT||vk==VK_MENU||vk==VK_LWIN||vk==VK_RWIN) break;
        UINT mod=0;
        if (GetKeyState(VK_CONTROL)&0x8000) mod|=MOD_CONTROL;
        if (GetKeyState(VK_MENU)   &0x8000) mod|=MOD_ALT;
        if (GetKeyState(VK_SHIFT)  &0x8000) mod|=MOD_SHIFT;
        if ((GetKeyState(VK_LWIN)|GetKeyState(VK_RWIN))&0x8000) mod|=MOD_WIN;
        if (mod==0) { ShowWindow(GetDlgItem(hwnd,ID_HK_HINT),SW_SHOW); break; }
        g_ds->cur.mod_keys=mod; g_ds->cur.vk_code=vk;
        g_ds->capturing=FALSE; ReleaseCapture();
        SetWindowTextA(GetDlgItem(hwnd,ID_HK_BTN),"Change");
        ShowWindow(GetDlgItem(hwnd,ID_HK_HINT),SW_HIDE);
        upd_hk(hwnd); return 0;
    }
    case WM_HSCROLL: {
        HWND hs=GetDlgItem(hwnd,ID_SL_RESULTS);
        if ((HWND)lParam==hs){ g_ds->cur.max_results=(int)SendMessageA(hs,TBM_GETPOS,0,0); upd_res(hwnd); }
        break;
    }
    case WM_COMMAND: {
        int id=LOWORD(wParam);
        if (id==ID_HK_BTN) {
            g_ds->capturing=!g_ds->capturing;
            if (g_ds->capturing) { SetCapture(hwnd); SetWindowTextA(GetDlgItem(hwnd,ID_HK_BTN),"Waiting..."); SetWindowTextA(GetDlgItem(hwnd,ID_HK_BOX),"Press keys..."); ShowWindow(GetDlgItem(hwnd,ID_HK_HINT),SW_SHOW); }
            else { ReleaseCapture(); SetWindowTextA(GetDlgItem(hwnd,ID_HK_BTN),"Change"); ShowWindow(GetDlgItem(hwnd,ID_HK_HINT),SW_HIDE); upd_hk(hwnd); }
        } else if (id==ID_BTN_RESET) { g_ds->cur.mod_keys=MOD_ALT; g_ds->cur.vk_code=VK_SPACE; g_ds->capturing=FALSE; ReleaseCapture(); upd_hk(hwnd); }
        else if (id==ID_BTN_OK) {
            g_ds->cur.run_on_startup =(SendMessageA(GetDlgItem(hwnd,ID_CB_STARTUP),BM_GETCHECK,0,0)==BST_CHECKED);
            g_ds->cur.show_recent    =(SendMessageA(GetDlgItem(hwnd,ID_CB_RECENT), BM_GETCHECK,0,0)==BST_CHECKED);
            g_ds->cur.close_on_launch=(SendMessageA(GetDlgItem(hwnd,ID_CB_CLOSE),  BM_GETCHECK,0,0)==BST_CHECKED);
            g_ds->cur.max_results    =(int)SendMessageA(GetDlgItem(hwnd,ID_SL_RESULTS),TBM_GETPOS,0,0);
            g_ds->changed=TRUE; DestroyWindow(hwnd);
        } else if (id==ID_BTN_CANCEL) { g_ds->changed=FALSE; DestroyWindow(hwnd); }
        break;
    }
    case WM_DESTROY: PostQuitMessage(0); break;
    }
    return DefWindowProcA(hwnd,msg,wParam,lParam);
}

BOOL settings_show_dialog(HWND parent, AppSettings *s) {
    DlgState ds; ds.orig=*s; ds.cur=*s; ds.capturing=FALSE; ds.changed=FALSE;
    g_ds=&ds;
    INITCOMMONCONTROLSEX icc={sizeof(icc),ICC_BAR_CLASSES};
    InitCommonControlsEx(&icc);
    static BOOL reg=FALSE;
    if (!reg) {
        WNDCLASSEXA wc={0}; wc.cbSize=sizeof(wc); wc.lpfnWndProc=SettingsProc;
        wc.hInstance=GetModuleHandle(NULL); wc.hCursor=LoadCursor(NULL,IDC_ARROW);
        wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1); wc.lpszClassName="SmartFinderSettings";
        RegisterClassExA(&wc); reg=TRUE;
    }
    int sw=GetSystemMetrics(SM_CXSCREEN),sh=GetSystemMetrics(SM_CYSCREEN);
    HWND hwnd=CreateWindowExA(WS_EX_DLGMODALFRAME|WS_EX_TOPMOST,"SmartFinderSettings",
        "Smart Finder Settings",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_VISIBLE,
        (sw-470)/2,(sh-310)/2,470,310,parent,NULL,GetModuleHandle(NULL),NULL);
    if (!hwnd) return FALSE;
    MSG msg;
    while (GetMessage(&msg,NULL,0,0)) {
        if (!IsDialogMessage(hwnd,&msg)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    }
    if (ds.changed) { *s=ds.cur; settings_save(s); settings_apply_startup(s->run_on_startup); }
    g_ds=NULL;
    return ds.changed;
}
