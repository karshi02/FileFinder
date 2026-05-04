#ifndef INTERACTIVE_UI_H
#define INTERACTIVE_UI_H

#include "universal_scanner.h"

// ฟังก์ชันหลักสำหรับแสดง UI แบบโต้ตอบ
void interactive_launcher(UniversalIndex *idx);

// ฟังก์ชันสำหรับบันทึกสถิติการใช้งาน
void save_launch_statistics(const char *app_name, const char *app_path);

#endif
