#ifndef INDEXER_H
#define INDEXER_H

#include <windows.h>

#define MAX_FILES    100000
#define MAX_NAME     256
#define MAX_PATH_LEN 1024

typedef struct {
    char name[MAX_NAME];
    char path[MAX_PATH_LEN];
    long long last_access;   /* Windows FILETIME as 64-bit int  */
    int  score;              /* runtime search score            */
} FileEntry;

typedef struct {
    FileEntry *files;
    int        count;
    int        capacity;
} Index;

/* Initialise an empty index (allocates memory) */
Index *index_create(void);

/* Free all memory */
void   index_destroy(Index *idx);

/* Add one file; returns 0 on success, -1 on failure */
int    index_add(Index *idx, const char *path, const char *name,
                 FILETIME last_access);

/* Persist index to a binary file */
int    index_save(const Index *idx, const char *filepath);

/* Load index from a binary file; returns NULL on failure */
Index *index_load(const char *filepath);

/* Reset all scores to 0 */
void   index_reset_scores(Index *idx);

#endif