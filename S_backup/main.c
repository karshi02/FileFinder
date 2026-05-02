#include <windows.h>
#include <stdio.h>
#include "hotkey.h"
#include "universal_scanner.h"
#include "universal_ui.h"
#include "frequent.h"

#define INDEX_PATH "data\\universal.db"
#define FREQ_PATH "data\\frequent.db"

// Global frequent list
static FrequentList *g_freq = NULL;

// ฟังก์ชัน callback เมื่อเปิดแอพ
void on_app_launched(const char *name, const char *path) {
    if (g_freq && name && path) {
        frequent_add_or_update(g_freq, name, path);
        frequent_save(g_freq, FREQ_PATH);
        frequent_sort(g_freq);  // เรียงลำดับใหม่หลังเพิ่ม
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev,
                   LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance; (void)hPrev; (void)lpCmdLine; (void)nCmdShow;

    // สร้างโฟลเดอร์ data ถ้ายังไม่มี
    CreateDirectoryA("data", NULL);

    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    printf("========================================\n");
    printf("  Universal Search with Frequent Apps\n");
    printf("========================================\n\n");

    // โหลด frequent list
    g_freq = frequent_load(FREQ_PATH);
    if (!g_freq) {
        printf("[init] Creating new frequent apps list...\n");
        g_freq = frequent_create();
    } else {
        printf("[init] Loaded %d frequent apps\n", g_freq->count);
        frequent_sort(g_freq);
        // แสดง top 5 แอพที่ใช้บ่อย
        printf("[init] Top frequent apps:\n");
        for (int i = 0; i < g_freq->count && i < 5; i++) {
            printf("       %d. %s (%d times)\n", i+1, g_freq->entries[i].name, g_freq->entries[i].count);
        }
    }

    // โหลด universal index
    UniversalIndex *idx = universal_load(INDEX_PATH);
    if (!idx) {
        printf("\n[init] First time setup. Scanning everything...\n");
        printf("[init] This may take a few moments...\n");
        idx = universal_create();
        if (!idx) return 1;

        universal_scan_all(idx);
        universal_save(idx, INDEX_PATH);
        printf("\n[init] Done! %d items indexed.\n", idx->count);
    } else {
        printf("\n[init] Loaded %d items from cache.\n", idx->count);
    }

    // อัพเดทคะแนนความถี่ให้กับ index
    universal_update_freq_score(idx, g_freq);

    if (!hotkey_register())
        fprintf(stderr, "[error] Hotkey failed (Ctrl+Space)\n");
    else
        printf("[ready] Press Ctrl+Space to search everything!\n");
    printf("[info] Use ↓↑ to select, Enter to launch, ESC to close\n\n");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY && msg.wParam == HOTKEY_ID) {
            universal_show_popup(idx);
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    hotkey_unregister();
    universal_destroy(idx);
    frequent_destroy(g_freq);
    return 0;
}
