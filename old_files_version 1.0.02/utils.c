#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void str_tolower(char *s) {
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

int str_contains_icase(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    char h[512], n[256];
    safe_strcpy(h, haystack, sizeof(h));
    safe_strcpy(n, needle,   sizeof(n));
    str_tolower(h);
    str_tolower(n);
    return strstr(h, n) != NULL;
}

/*
 * Fuzzy match: award points for each pattern char found in order.
 * Consecutive matches earn bonus points.
 * Returns 0-100 normalised score.
 */
int fuzzy_match(const char *text, const char *pattern) {
    if (!text || !pattern || !*pattern) return 0;

    char t[512], p[256];
    safe_strcpy(t, text,    sizeof(t));
    safe_strcpy(p, pattern, sizeof(p));
    str_tolower(t);
    str_tolower(p);

    int score = 0, consecutive = 0;
    const char *tp = t;

    for (int i = 0; p[i]; i++) {
        const char *found = strchr(tp, p[i]);
        if (!found) return 0;           /* all pattern chars must appear */
        score += 10;
        if (found == tp) { score += 5; consecutive++; }
        else              consecutive = 0;
        score += consecutive * 3;
        tp = found + 1;
    }


    /* Normalise: cap at 100 */
    int plen = (int)strlen(p);
    int max_possible = plen * (10 + 5 + plen * 3);
    if (max_possible <= 0) return 0;
    score = (score * 100) / max_possible;
    if (score > 100) score = 100;
    return score;
}

void safe_strcpy(char *dst, const char *src, size_t max_len) {
    if (!dst || !src || max_len == 0) return;
    strncpy(dst, src, max_len - 1);
    dst[max_len - 1] = '\0';
}
