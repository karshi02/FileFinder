#include "interactive_ui.h"
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

#define MAX_RESULTS 50

// ฟังก์ชันสำหรับเปลี่ยนสีข้อความ
void set_color(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// ฟังก์ชันล้างหน้า console
void clear_screen() {
    system("cls");
}

// ฟังก์ชันแสดงผลลัพธ์พร้อมไฮไลท์
void display_results(UniversalEntry **results, int result_count, int selected, const char *query) {
    clear_screen();
    
    // แสดงหัวข้อ
    set_color(11); // สีฟ้า
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║           UNIVERSAL SEARCH LAUNCHER v1.0                 ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    set_color(7); // สีขาว
    
    // ช่องค้นหา
    set_color(10); // สีเขียว
    printf("\n🔍 Search: %s", query);
    printf("\n");
    set_color(7);
    
    // เส้นแบ่ง
    printf("┌──────────────────────────────────────────────────────────┐\n");
    printf("│ Results:                                                 │\n");
    printf("└──────────────────────────────────────────────────────────┘\n");
    
    // แสดงผลลัพธ์
    if (result_count == 0) {
        set_color(12); // สีแดง
        printf("\n❌ No results found for '%s'\n", query);
        set_color(7);
    } else {
        printf("\n");
        for (int i = 0; i < result_count && i < MAX_RESULTS; i++) {
            if (i == selected) {
                set_color(14); // สีเหลืองพื้นหลัง?
                printf("▶ ");
            } else {
                printf("  ");
            }
            
            // แสดงประเภทไอคอน
            if (strcmp(results[i]->type, "app") == 0) {
                set_color(10); // เขียว
                printf("[APP]  ");
            } else if (strcmp(results[i]->type, "program") == 0) {
                set_color(11); // ฟ้า
                printf("[EXE]  ");
            } else {
                set_color(13); // ม่วง
                printf("[FILE] ");
            }
            
            set_color(7);
            
            // แสดงชื่อ
            if (i == selected) {
                set_color(14); // เหลือง
                printf("%s", results[i]->name);
                set_color(8); // เทา
                printf("\n      └─ %s", results[i]->path);
                set_color(7);
            } else {
                printf("%s", results[i]->name);
            }
            printf("\n");
        }
    }
    
    // แสดงคำแนะนำ
    set_color(8); // เทา
    printf("\n────────────────────────────────────────────────────────────\n");
    printf("↑/↓ : Navigate  |  Enter : Launch  |  ESC : Exit\n");
    printf("Backspace : Delete  |  Type to search\n");
    set_color(7);
}

// ฟังก์ชันบันทึกสถิติ
void save_launch_statistics(const char *app_name, const char *app_path) {
    FILE *f = fopen("launch_stats.txt", "a");
    if (f) {
        time_t now;
        time(&now);
        fprintf(f, "%s|%s|%s", app_name, app_path, ctime(&now));
        fclose(f);
    }
}

// ฟังก์ชันรันโปรแกรม
int launch_program(const char *path) {
    // ตรวจสอบว่าเป็น .lnk หรือไม่
    if (strstr(path, ".lnk")) {
        HINSTANCE result = ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOW);
        return (intptr_t)result > 32 ? 1 : 0;
    } else {
        HINSTANCE result = ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOW);
        return (intptr_t)result > 32 ? 1 : 0;
    }
}

// ฟังก์ชันหลัก
void interactive_launcher(UniversalIndex *idx) {
    if (!idx || idx->count == 0) {
        printf("No data loaded. Please scan first.\n");
        return;
    }
    
    char query[256] = "";
    int query_len = 0;
    int selected = 0;
    int result_count = 0;
    UniversalEntry *results[MAX_RESULTS];
    int ch;
    
    while (1) {
        // ค้นหา
        universal_search(idx, query, results, &result_count);
        
        // จำกัด selected ให้อยู่ในช่วง
        if (selected >= result_count && result_count > 0) {
            selected = result_count - 1;
        }
        if (selected < 0) selected = 0;
        
        // แสดงผล
        display_results(results, result_count, selected, query);
        
        // รับอินพุต
        ch = _getch();
        
        if (ch == 27) { // ESC
            printf("\n\nExiting...\n");
            break;
        }
        else if (ch == 13) { // Enter
            if (result_count > 0 && selected < result_count) {
                clear_screen();
                set_color(10);
                printf("\n🚀 Launching: %s\n", results[selected]->name);
                printf("📂 Path: %s\n", results[selected]->path);
                set_color(7);
                
                // บันทึกสถิติ
                save_launch_statistics(results[selected]->name, results[selected]->path);
                
                // รันโปรแกรม
                if (launch_program(results[selected]->path)) {
                    printf("✅ Program launched successfully!\n");
                } else {
                    set_color(12);
                    printf("❌ Failed to launch program!\n");
                    set_color(7);
                }
                
                printf("\nPress any key to continue...");
                _getch();
            }
        }
        else if (ch == 224 || ch == 0) { // ลูกศร
            ch = _getch(); // อ่านรหัสลูกศรตัวที่สอง
            
            if (ch == 72) { // ลูกศรขึ้น
                if (selected > 0) {
                    selected--;
                }
            }
            else if (ch == 80) { // ลูกศรลง
                if (selected < result_count - 1 && selected < MAX_RESULTS - 1) {
                    selected++;
                }
            }
            else if (ch == 71) { // Home
                selected = 0;
            }
            else if (ch == 79) { // End
                selected = result_count - 1;
            }
        }
        else if (ch == 8) { // Backspace
            if (query_len > 0) {
                query_len--;
                query[query_len] = '\0';
                selected = 0; // reset selection
            }
        }
        else if (ch >= 32 && ch <= 126) { // ตัวอักษรปกติ
            if (query_len < 255) {
                query[query_len] = (char)ch;
                query_len++;
                query[query_len] = '\0';
                selected = 0; // reset selection
            }
        }
    }
}
