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
                snprintf(name, sizeof(name), "%s", fd.cFileName);
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

/* ================================================================
   Fuzzy search — 3 levels
   300 = exact substring | 200 = initials | 100+ = fuzzy subsequence
   ================================================================ */

static int score_exact(const char *name, const char *q) {
    return strstr(name, q) ? 300 : 0;
}

static int score_initials(const char *name, const char *q) {
    int qi = 0, qlen = (int)strlen(q), prev_space = 1;
    for (int ni = 0; name[ni] && qi < qlen; ni++) {
        if (name[ni] == ' ' || name[ni] == '-' || name[ni] == '_') {
            prev_space = 1;
        } else if (prev_space) {
            if (tolower((unsigned char)name[ni]) == (unsigned char)q[qi]) qi++;
            prev_space = 0;
        }
    }
    return (qi == qlen) ? 200 : 0;
}

static int score_fuzzy(const char *name, const char *q) {
    int qi = 0, qlen = (int)strlen(q), last = -1, bonus = 0;
    for (int ni = 0; name[ni] && qi < qlen; ni++) {
        if (tolower((unsigned char)name[ni]) == (unsigned char)q[qi]) {
            if (last == ni - 1) bonus += 10;
            last = ni; qi++;
        }
    }
    return (qi == qlen) ? 100 + bonus : 0;
}

static int compute_score(const char *name_lc, const char *q_lc, int freq) {
    int s = score_exact(name_lc, q_lc);
    if (!s) s = score_initials(name_lc, q_lc);
    if (!s) s = score_fuzzy(name_lc, q_lc);
    return s ? s + freq : 0;
}

void universal_search(UniversalIndex *idx, const char *query, UniversalEntry **results, int *result_count) {
    *result_count = 0;
    if (!idx || !query || !*query) return;

    char query_lc[256];
    strncpy(query_lc, query, sizeof(query_lc)-1);
    query_lc[sizeof(query_lc)-1] = '\0';
    for (int i = 0; query_lc[i]; i++)
        query_lc[i] = (char)tolower((unsigned char)query_lc[i]);

    typedef struct { UniversalEntry *e; int score; } Scored;
    static Scored pool[50];
    int pool_count = 0;

    for (int i = 0; i < idx->count && pool_count < 50; i++) {
        char name_lc[512];
        strncpy(name_lc, idx->entries[i].name, sizeof(name_lc)-1);
        name_lc[sizeof(name_lc)-1] = '\0';
        for (int j = 0; name_lc[j]; j++)
            name_lc[j] = (char)tolower((unsigned char)name_lc[j]);

        int s = compute_score(name_lc, query_lc, idx->entries[i].freq_score);
        if (s > 0) {
            pool[pool_count].e     = &idx->entries[i];
            pool[pool_count].score = s;
            pool_count++;
        }
    }

    for (int i = 1; i < pool_count; i++) {
        Scored key = pool[i];
        int j = i - 1;
        while (j >= 0 && pool[j].score < key.score) {
            pool[j+1] = pool[j]; j--;
        }
        pool[j+1] = key;
    }

    int limit = pool_count < 50 ? pool_count : 50;
    for (int i = 0; i < limit; i++) results[i] = pool[i].e;
    *result_count = limit;
}
