#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <ctype.h>

/* Convert string to lowercase in-place */
void str_tolower(char *s);

/* Check if needle is a substring of haystack (case-insensitive) */
int str_contains_icase(const char *haystack, const char *needle);

/* Simple fuzzy match: returns score 0-100, 0 = no match */
int fuzzy_match(const char *text, const char *pattern);

/* Safe string copy with length limit */
void safe_strcpy(char *dst, const char *src, size_t max_len);

#endif