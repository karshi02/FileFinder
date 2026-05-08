#ifndef UNIVERSAL_UI_H
#define UNIVERSAL_UI_H
#include "universal_scanner.h"
void universal_show_popup(UniversalIndex *idx);
void universal_close_popup(void);
/* รับ settings จาก main เพื่อปรับพฤติกรรม UI */
void universal_apply_settings(BOOL show_recent, int max_results, BOOL close_on_launch);
#endif
