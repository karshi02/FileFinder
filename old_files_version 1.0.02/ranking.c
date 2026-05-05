#include "ranking.h"
#include "utils.h"
#include <string.h>
#include <time.h>
#include <windows.h>

/* Convert Windows FILETIME (100-ns ticks since 1601) to Unix timestamp */
static long long filetime_to_unix(long long ft) {
    /* 116444736000000000 = ticks between 1601-01-01 and 1970-01-01 */
    return (ft - 116444736000000000LL) / 10000000LL;
}

int rank_score(const FileEntry *entry, const char *query) {
    if (!entry || !query || !*query) return 0;

    int score = 0;

    /* --- Name match --- */
    char name_lc[MAX_NAME];
    safe_strcpy(name_lc, entry->name, sizeof(name_lc));
    str_tolower(name_lc);

    char query_lc[256];
    safe_strcpy(query_lc, query, sizeof(query_lc));
    str_tolower(query_lc);

    /* Exact name match */
    if (strcmp(name_lc, query_lc) == 0) {
        score += 50;
    }
    /* Substring match in name */
    else if (strstr(name_lc, query_lc)) {
        score += 30;
    }
    /* Fuzzy match in name */
    else {
        int fuzz = fuzzy_match(entry->name, query);
        if (fuzz == 0) return 0;   /* no match at all */
        score += (fuzz * 20) / 100;
    }

    /* --- Recency bonus --- */
    if (entry->last_access > 0) {
        long long accessed = filetime_to_unix(entry->last_access);
        long long now      = (long long)time(NULL);
        long long days_ago = (now - accessed) / 86400;

        if      (days_ago < 1)   score += 10;
        else if (days_ago < 7)   score += 7;
        else if (days_ago < 30)  score += 4;
        else if (days_ago < 90)  score += 1;
    }

    /* --- Path contains query --- */
    if (str_contains_icase(entry->path, query))
        score += 10;

    return score;
}