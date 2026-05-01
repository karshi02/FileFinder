#include "frequent.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

FrequentList* frequent_create(void) {
    FrequentList *list = (FrequentList*)malloc(sizeof(FrequentList));
    if (!list) return NULL;
    
    list->count = 0;
    list->capacity = 100;
    list->entries = (FrequentEntry*)malloc(sizeof(FrequentEntry) * list->capacity);
    
    if (!list->entries) {
        free(list);
        return NULL;
    }
    
    return list;
}

void frequent_destroy(FrequentList *list) {
    if (list) {
        if (list->entries) free(list->entries);
        free(list);
    }
}

int frequent_add_or_update(FrequentList *list, const char *name, const char *path) {
    if (!list || !name || !path) return -1;
    
    // ค้นหาว่ามีอยู่แล้วหรือไม่
    for (int i = 0; i < list->count; i++) {
        if (strcmp(list->entries[i].name, name) == 0 &&
            strcmp(list->entries[i].path, path) == 0) {
            // มีแล้ว เพิ่ม count และอัพเดท time
            list->entries[i].count++;
            list->entries[i].last_used = time(NULL);
            return 0;
        }
    }
    
    // ยังไม่มี เพิ่มใหม่
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        FrequentEntry *new_entries = (FrequentEntry*)realloc(list->entries, 
                                          sizeof(FrequentEntry) * list->capacity);
        if (!new_entries) return -1;
        list->entries = new_entries;
    }
    
    strncpy(list->entries[list->count].name, name, sizeof(list->entries[list->count].name) - 1);
    strncpy(list->entries[list->count].path, path, sizeof(list->entries[list->count].path) - 1);
    list->entries[list->count].count = 1;
    list->entries[list->count].last_used = time(NULL);
    list->count++;
    
    return 0;
}

void frequent_save(FrequentList *list, const char *filename) {
    if (!list || !filename) return;
    
    FILE *f = fopen(filename, "wb");
    if (!f) return;
    
    fwrite(&list->count, sizeof(int), 1, f);
    fwrite(list->entries, sizeof(FrequentEntry), list->count, f);
    
    fclose(f);
}

FrequentList* frequent_load(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;
    
    FrequentList *list = frequent_create();
    if (!list) {
        fclose(f);
        return NULL;
    }
    
    fread(&list->count, sizeof(int), 1, f);
    
    if (list->count > list->capacity) {
        list->capacity = list->count + 50;
        list->entries = (FrequentEntry*)realloc(list->entries, 
                              sizeof(FrequentEntry) * list->capacity);
    }
    
    fread(list->entries, sizeof(FrequentEntry), list->count, f);
    
    fclose(f);
    return list;
}

void frequent_sort(FrequentList *list) {
    if (!list || list->count <= 1) return;
    
    // เรียงตาม count (มากไปน้อย) และ last_used (ใหม่ไปเก่า)
    for (int i = 0; i < list->count - 1; i++) {
        for (int j = i + 1; j < list->count; j++) {
            if (list->entries[i].count < list->entries[j].count ||
                (list->entries[i].count == list->entries[j].count && 
                 list->entries[i].last_used < list->entries[j].last_used)) {
                FrequentEntry temp = list->entries[i];
                list->entries[i] = list->entries[j];
                list->entries[j] = temp;
            }
        }
    }
}

int frequent_get_score(FrequentList *list, const char *name, const char *path) {
    if (!list) return 0;
    
    for (int i = 0; i < list->count; i++) {
        if (strcmp(list->entries[i].name, name) == 0 &&
            strcmp(list->entries[i].path, path) == 0) {
            // คะแนน = count * 100 + (เวลาที่ใช้ล่าสุด)
            return list->entries[i].count * 100;
        }
    }
    return 0;
}
