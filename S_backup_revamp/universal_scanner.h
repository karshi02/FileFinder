#ifndef UNIVERSAL_SCANNER_H
#define UNIVERSAL_SCANNER_H

#include <windows.h>
#include "frequent.h"   // เพิ่มบรรทัดนี้

typedef struct {
    char name[512];
    char path[1024];
    char type[32];
    int score;
    int freq_score;
} UniversalEntry;

typedef struct {
    UniversalEntry *entries;
    int count;
    int capacity;
} UniversalIndex;

UniversalIndex* universal_create(void);
void universal_destroy(UniversalIndex *idx);
int universal_add(UniversalIndex *idx, const char *name, const char *path, const char *type);
int universal_scan_all(UniversalIndex *idx);
int universal_save(UniversalIndex *idx, const char *filename);
UniversalIndex* universal_load(const char *filename);
void universal_search(UniversalIndex *idx, const char *query, UniversalEntry **results, int *result_count);
void universal_update_freq_score(UniversalIndex *idx, FrequentList *freq);

#endif
