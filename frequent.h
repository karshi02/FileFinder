#ifndef FREQUENT_H
#define FREQUENT_H

#include <windows.h>

typedef struct {
    char name[256];
    char path[1024];
    int count;           // จำนวนครั้งที่เปิด
    time_t last_used;    // เวลาที่ใช้ล่าสุด
} FrequentEntry;

typedef struct {
    FrequentEntry *entries;
    int count;
    int capacity;
} FrequentList;

FrequentList* frequent_create(void);
void frequent_destroy(FrequentList *list);
int frequent_add_or_update(FrequentList *list, const char *name, const char *path);
void frequent_save(FrequentList *list, const char *filename);
FrequentList* frequent_load(const char *filename);
void frequent_sort(FrequentList *list);  // เรียงตามความถี่
int frequent_get_score(FrequentList *list, const char *name, const char *path);

#endif
