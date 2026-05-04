#include "app_scanner.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <shlobj.h>

AppIndex* app_index_create(void) {
    AppIndex *idx = (AppIndex*)malloc(sizeof(AppIndex));
    if (!idx) return NULL;
    
    idx->count = 0;
    idx->capacity = 1000;
    idx->apps = (AppEntry*)malloc(sizeof(AppEntry) * idx->capacity);
    
    if (!idx->apps) {
        free(idx);
        return NULL;
    }
    
    return idx;
}

void app_index_destroy(AppIndex *idx) {
    if (idx) {
        if (idx->apps) free(idx->apps);
        free(idx);
    }
}

int app_index_add(AppIndex *idx, const char *name, const char *path) {
    if (!idx || !name || !path) return -1;
    
    if (idx->count >= idx->capacity) {
        idx->capacity *= 2;
        AppEntry *new_apps = (AppEntry*)realloc(idx->apps, sizeof(AppEntry) * idx->capacity);
        if (!new_apps) return -1;
        idx->apps = new_apps;
    }
    
    strncpy(idx->apps[idx->count].name, name, sizeof(idx->apps[idx->count].name) - 1);
    idx->apps[idx->count].name[sizeof(idx->apps[idx->count].name) - 1] = '\0';
    
    strncpy(idx->apps[idx->count].path, path, sizeof(idx->apps[idx->count].path) - 1);
    idx->apps[idx->count].path[sizeof(idx->apps[idx->count].path) - 1] = '\0';
    
    idx->count++;
    return 0;
}

int app_scan_all(AppIndex *idx) {
    if (!idx) return -1;
    
    char startmenu_path[1024];
    int total = 0;
    
    // All Users Start Menu
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_STARTMENU, NULL, 0, startmenu_path))) {
        char search_path[2048];
        snprintf(search_path, sizeof(search_path), "%s\\*.lnk", startmenu_path);
        
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(search_path, &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    char fullpath[2048];
                    snprintf(fullpath, sizeof(fullpath), "%s\\%s", startmenu_path, findData.cFileName);
                    
                    // ตัด .lnk
                    char name[256];
                    strncpy(name, findData.cFileName, sizeof(name)-1);
                    name[sizeof(name)-1] = '\0';
                    char *ext = strstr(name, ".lnk");
                    if (ext) *ext = '\0';
                    
                    app_index_add(idx, name, fullpath);
                    total++;
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
    }
    
    // User Start Menu
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTMENU, NULL, 0, startmenu_path))) {
        char search_path[2048];
        snprintf(search_path, sizeof(search_path), "%s\\*.lnk", startmenu_path);
        
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(search_path, &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    char fullpath[2048];
                    snprintf(fullpath, sizeof(fullpath), "%s\\%s", startmenu_path, findData.cFileName);
                    
                    char name[256];
                    strncpy(name, findData.cFileName, sizeof(name)-1);
                    name[sizeof(name)-1] = '\0';
                    char *ext = strstr(name, ".lnk");
                    if (ext) *ext = '\0';
                    
                    app_index_add(idx, name, fullpath);
                    total++;
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
    }
    
    return total;
}

int app_save(AppIndex *idx, const char *filename) {
    if (!idx || !filename) return -1;
    
    FILE *f = fopen(filename, "wb");
    if (!f) return -1;
    
    fwrite(&idx->count, sizeof(int), 1, f);
    fwrite(idx->apps, sizeof(AppEntry), idx->count, f);
    
    fclose(f);
    return 0;
}

AppIndex* app_load(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;
    
    AppIndex *idx = app_index_create();
    if (!idx) {
        fclose(f);
        return NULL;
    }
    
    fread(&idx->count, sizeof(int), 1, f);
    
    if (idx->count > idx->capacity) {
        idx->capacity = idx->count + 100;
        idx->apps = (AppEntry*)realloc(idx->apps, sizeof(AppEntry) * idx->capacity);
    }
    
    fread(idx->apps, sizeof(AppEntry), idx->count, f);
    
    fclose(f);
    return idx;
}
