#include "universal_scanner.h"
#include "frequent.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#include <shlobj.h>

UniversalIndex* universal_create(void) {
    UniversalIndex *idx = (UniversalIndex*)malloc(sizeof(UniversalIndex));
    if (!idx) return NULL;
    
    idx->count = 0;
    idx->capacity = 100000;
    idx->entries = (UniversalEntry*)malloc(sizeof(UniversalEntry) * idx->capacity);
    
    if (!idx->entries) {
        free(idx);
        return NULL;
    }
    
    return idx;
}

void universal_destroy(UniversalIndex *idx) {
    if (idx) {
        if (idx->entries) free(idx->entries);
        free(idx);
    }
}

int universal_add(UniversalIndex *idx, const char *name, const char *path, const char *type) {
    if (!idx || !name || !path) return -1;
    
    if (idx->count >= idx->capacity) {
        idx->capacity *= 2;
        UniversalEntry *new_entries = (UniversalEntry*)realloc(idx->entries, sizeof(UniversalEntry) * idx->capacity);
        if (!new_entries) return -1;
        idx->entries = new_entries;
    }
    
    strncpy(idx->entries[idx->count].name, name, sizeof(idx->entries[idx->count].name) - 1);
    strncpy(idx->entries[idx->count].path, path, sizeof(idx->entries[idx->count].path) - 1);
    strncpy(idx->entries[idx->count].type, type, sizeof(idx->entries[idx->count].type) - 1);
    
    idx->entries[idx->count].name[sizeof(idx->entries[idx->count].name) - 1] = '\0';
    idx->entries[idx->count].path[sizeof(idx->entries[idx->count].path) - 1] = '\0';
    idx->entries[idx->count].type[sizeof(idx->entries[idx->count].type) - 1] = '\0';
    idx->entries[idx->count].score = 0;
    idx->entries[idx->count].freq_score = 0;
    
    idx->count++;
    return 0;
}


static void scan_recent_files(UniversalIndex *idx) {
    char recent_path[1024];
    if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_RECENT, NULL, 0, recent_path)))
        return;

    char search_path[2048];
    snprintf(search_path, sizeof(search_path), "%s\\*", recent_path);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        char *ext = strrchr(fd.cFileName, '.');
        if (!ext || _stricmp(ext, ".lnk") != 0) continue;

        char fullpath[2048];
        snprintf(fullpath, sizeof(fullpath), "%s\\%s", recent_path, fd.cFileName);

        char name[260];
        snprintf(name, sizeof(name), "%s", fd.cFileName);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';

        universal_add(idx, name, fullpath, "recent");
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
}

/* recurse เข้า subfolder ของ Start Menu — เร็วมากเพราะมีแค่ .lnk */
static void scan_startmenu_dir(UniversalIndex *idx, const char *dir) {
    char search_path[2048];
    snprintf(search_path, sizeof(search_path), "%s\\*", dir);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;

        char fullpath[2048];
        snprintf(fullpath, sizeof(fullpath), "%s\\%s", dir, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            scan_startmenu_dir(idx, fullpath);   /* recurse */
        } else {
            char *ext = strrchr(fd.cFileName, '.');
            if (ext && _stricmp(ext, ".lnk") == 0) {
                char name[260];
                snprintf(name, sizeof(name) - 1, "%s", fd.cFileName);
                name[sizeof(name) - 1] = '\0';
                char *dot = strrchr(name, '.');
                if (dot) *dot = '\0';
                universal_add(idx, name, fullpath, "app");
            }
        }
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
}

static void scan_apps(UniversalIndex *idx) {
    char path[1024];
    /* All Users Start Menu */
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_STARTMENU, NULL, 0, path)))
        scan_startmenu_dir(idx, path);
    /* Current User Start Menu */
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTMENU, NULL, 0, path)))
        scan_startmenu_dir(idx, path);
    /* Desktop shortcuts */
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path)))
        scan_startmenu_dir(idx, path);
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, 0, path)))
        scan_startmenu_dir(idx, path);
}


int universal_scan_all(UniversalIndex *idx) {
    if (!idx) return -1;
    /* Start Menu + Desktop — เร็วมาก ~100ms */
    scan_apps(idx);
    /* Recent files */
    scan_recent_files(idx);
    return idx->count;
}

int universal_save(UniversalIndex *idx, const char *filename) {
    if (!idx || !filename) return -1;
    
    FILE *f = fopen(filename, "wb");
    if (!f) return -1;
    
    fwrite(&idx->count, sizeof(int), 1, f);
    fwrite(idx->entries, sizeof(UniversalEntry), idx->count, f);
    
    fclose(f);
    return 0;
}

UniversalIndex* universal_load(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;
    
    UniversalIndex *idx = universal_create();
    if (!idx) {
        fclose(f);
        return NULL;
    }
    
    fread(&idx->count, sizeof(int), 1, f);
    
    if (idx->count > idx->capacity) {
        idx->capacity = idx->count + 10000;
        idx->entries = (UniversalEntry*)realloc(idx->entries, sizeof(UniversalEntry) * idx->capacity);
    }
    
    fread(idx->entries, sizeof(UniversalEntry), idx->count, f);
    
    fclose(f);
    return idx;
}

void universal_update_freq_score(UniversalIndex *idx, FrequentList *freq) {
    if (!idx || !freq) return;
    
    for (int i = 0; i < idx->count; i++) {
        idx->entries[i].freq_score = 0;
        for (int j = 0; j < freq->count; j++) {
            if (strcmp(idx->entries[i].name, freq->entries[j].name) == 0) {
                idx->entries[i].freq_score = freq->entries[j].count * 100;
                break;
            }
        }
    }
}

void universal_search(UniversalIndex *idx, const char *query, UniversalEntry **results, int *result_count) {
    *result_count = 0;
    if (!idx || !query || !*query) return;
    
    char query_lc[256];
    strncpy(query_lc, query, sizeof(query_lc)-1);
    query_lc[sizeof(query_lc)-1] = '\0';
    for (int i = 0; query_lc[i]; i++) query_lc[i] = tolower(query_lc[i]);
    
    // ใช้ 50 ให้สอดคล้องกับ MAX_RESULTS ของ caller
    UniversalEntry *temp[50];
    int temp_count = 0;
    
    for (int i = 0; i < idx->count && temp_count < 50; i++) {
        char name_lc[512];
        strncpy(name_lc, idx->entries[i].name, sizeof(name_lc)-1);
        name_lc[sizeof(name_lc)-1] = '\0';
        for (int j = 0; name_lc[j]; j++) name_lc[j] = tolower(name_lc[j]);
        
        if (strstr(name_lc, query_lc)) {
            temp[temp_count++] = &idx->entries[i];
        }
    }
    
    // เรียงตาม freq_score (มากไปน้อย)
    for (int i = 0; i < temp_count - 1; i++) {
        for (int j = i + 1; j < temp_count; j++) {
            if (temp[i]->freq_score < temp[j]->freq_score) {
                UniversalEntry *t = temp[i];
                temp[i] = temp[j];
                temp[j] = t;
            }
        }
    }
    
    int limit = temp_count < 50 ? temp_count : 50;
    for (int i = 0; i < limit; i++) {
        results[i] = temp[i];
    }
    *result_count = limit;
}
