#ifndef SEARCH_H
#define SEARCH_H

#include "indexer.h"
#include "app_scanner.h"

#define MAX_RESULTS 100

typedef struct {
    FileEntry *entries[MAX_RESULTS];
    int count;
} SearchResult;

typedef struct {
    AppEntry *entries[MAX_RESULTS];
    int count;
} AppSearchResult;

void search_query(Index *idx, const char *query, SearchResult *result);
void search_apps(AppIndex *app_idx, const char *query, AppSearchResult *result);

#endif
