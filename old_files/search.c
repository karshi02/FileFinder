#include "search.h"
#include "ranking.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

static int cmp_score_desc(const void *a, const void *b) {
    const FileEntry *fa = *(const FileEntry **)a;
    const FileEntry *fb = *(const FileEntry **)b;
    return fb->score - fa->score;
}

static int cmp_app_score_desc(const void *a, const void *b) {
    const AppEntry *fa = *(const AppEntry **)a;
    const AppEntry *fb = *(const AppEntry **)b;
    /* เปรียบเทียบตามชื่อ หรือ score ถ้ามี */
    return strcmp(fa->name, fb->name);
}

void search_apps(AppIndex *app_idx, const char *query, AppSearchResult *result) {
    if (!app_idx || !query || !result) return;

    result->count = 0;

    if (!*query) return;

    /* แปลง query เป็นตัวพิมพ์เล็ก */
    char query_lc[256];
    safe_strcpy(query_lc, query, sizeof(query_lc));
    str_tolower(query_lc);

    /* ค้นหาแอพที่ชื่อตรงหรือมีคำค้น */
    for (int i = 0; i < app_idx->count && result->count < MAX_RESULTS; i++) {
        char name_lc[256];
        safe_strcpy(name_lc, app_idx->apps[i].name, sizeof(name_lc));
        str_tolower(name_lc);

        /* ตรวจสอบว่า query อยู่ในชื่อแอพ */
        if (strstr(name_lc, query_lc)) {
            result->entries[result->count++] = &app_idx->apps[i];
        }
    }

    /* เรียงลำดับตามชื่อ */
    if (result->count > 0) {
        qsort(result->entries, result->count, sizeof(AppEntry *), cmp_app_score_desc);
    }
}

void search_query(Index *idx, const char *query, SearchResult *result) {
    if (!idx || !query || !result) return;

    result->count = 0;

    for (int i = 0; i < MAX_RESULTS; i++) {
        result->entries[i] = NULL;
    }

    index_reset_scores(idx);

    if (!*query) return;

    for (int i = 0; i < idx->count; i++) {
        int s = rank_score(&idx->files[i], query);
        if (s > 0) {
            idx->files[i].score = s;

            if (result->count < MAX_RESULTS) {
                result->entries[result->count++] = &idx->files[i];
            } else {
                int min_idx = -1;
                int min_score = 999999;

                for (int j = 0; j < MAX_RESULTS; j++) {
                    if (result->entries[j] != NULL) {
                        if (result->entries[j]->score < min_score) {
                            min_score = result->entries[j]->score;
                            min_idx = j;
                        }
                    }
                }

                if (min_idx != -1 && s > min_score) {
                    result->entries[min_idx] = &idx->files[i];
                }
            }
        }
    }

    if (result->count > 0) {
        qsort(result->entries, result->count, sizeof(FileEntry *), cmp_score_desc);
    }
}
