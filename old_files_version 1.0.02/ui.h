#ifndef UI_H
#define UI_H

#include "indexer.h"
#include "app_scanner.h"

void ui_show_popup(Index *idx, AppIndex *app_idx);
void ui_close_popup(void);
void ui_open_file(const char *path);

#endif
