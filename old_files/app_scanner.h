#ifndef APP_SCANNER_H
#define APP_SCANNER_H

#include <windows.h>

typedef struct {
    char name[256];
    char path[1024];
    char icon[256];
} AppEntry;

typedef struct {
    AppEntry *apps;
    int count;
    int capacity;
} AppIndex;

AppIndex* app_index_create(void);
void app_index_destroy(AppIndex *idx);
int app_index_add(AppIndex *idx, const char *name, const char *path);
int app_scan_all(AppIndex *idx);
int app_save(AppIndex *idx, const char *filename);
AppIndex* app_load(const char *filename);

#endif
