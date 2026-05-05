#ifndef RANKING_H
#define RANKING_H

#include "indexer.h"

/*
 * Calculate a relevance score for a file entry given a query string.
 * Returns 0 if the file does not match at all.
 *
 * Scoring breakdown:
 *   +50  exact filename match (case-insensitive)
 *   +30  filename contains query as substring
 *   +20  fuzzy match on filename
 *   +10  bonus for recently accessed files
 *   +10  query also found anywhere in the full path
 */
int rank_score(const FileEntry *entry, const char *query);

#endif