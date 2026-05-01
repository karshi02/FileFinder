#include "indexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4096
#define INDEX_MAGIC      0x53464958  /* "XIFS" */
#define INDEX_VERSION    1

Index *index_create(void) {
    Index *idx = (Index *)malloc(sizeof(Index));
    if (!idx) return NULL;
    idx->files    = (FileEntry *)malloc(INITIAL_CAPACITY * sizeof(FileEntry));
    if (!idx->files) { free(idx); return NULL; }
    idx->count    = 0;
    idx->capacity = INITIAL_CAPACITY;
    return idx;
}

void index_destroy(Index *idx) {
    if (!idx) return;
    free(idx->files);
    free(idx);
}

int index_add(Index *idx, const char *path, const char *name,
              FILETIME last_access)
{
    if (!idx || idx->count >= MAX_FILES) return -1;

    /* Grow buffer if needed */
    if (idx->count >= idx->capacity) {
        int new_cap = idx->capacity * 2;
        FileEntry *tmp = (FileEntry *)realloc(idx->files,
                             new_cap * sizeof(FileEntry));
        if (!tmp) return -1;
        idx->files    = tmp;
        idx->capacity = new_cap;
    }

    FileEntry *e = &idx->files[idx->count];
    strncpy(e->name, name, MAX_NAME - 1);
    e->name[MAX_NAME - 1] = '\0';
    strncpy(e->path, path, MAX_PATH_LEN - 1);
    e->path[MAX_PATH_LEN - 1] = '\0';

    /* Merge FILETIME hi/lo into a single 64-bit value */
    ULARGE_INTEGER ul;
    ul.LowPart  = last_access.dwLowDateTime;
    ul.HighPart = last_access.dwHighDateTime;
    e->last_access = (long long)ul.QuadPart;

    e->score = 0;
    idx->count++;
    return 0;
}

/* ---- persistence ---- */

typedef struct {
    unsigned int magic;
    int          version;
    int          count;
} IndexHeader;

int index_save(const Index *idx, const char *filepath) {
    if (!idx || !filepath) return -1;
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;

    IndexHeader hdr = { INDEX_MAGIC, INDEX_VERSION, idx->count };
    fwrite(&hdr, sizeof(hdr), 1, fp);
    fwrite(idx->files, sizeof(FileEntry), idx->count, fp);
    fclose(fp);
    return 0;
}

Index *index_load(const char *filepath) {
    if (!filepath) return NULL;
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return NULL;

    IndexHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, fp) != 1 ||
        hdr.magic != INDEX_MAGIC || hdr.version != INDEX_VERSION ||
        hdr.count < 0 || hdr.count > MAX_FILES)
    {
        fclose(fp); return NULL;
    }

    Index *idx = index_create();
    if (!idx) { fclose(fp); return NULL; }

    /* Ensure capacity */
    if (hdr.count > idx->capacity) {
        FileEntry *tmp = (FileEntry *)realloc(idx->files,
                             hdr.count * sizeof(FileEntry));
        if (!tmp) { index_destroy(idx); fclose(fp); return NULL; }
        idx->files    = tmp;
        idx->capacity = hdr.count;
    }

    idx->count = (int)fread(idx->files, sizeof(FileEntry), hdr.count, fp);
    fclose(fp);
    return idx;
}

void index_reset_scores(Index *idx) {
    if (!idx) return;
    for (int i = 0; i < idx->count; i++)
        idx->files[i].score = 0;
}