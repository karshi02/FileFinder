#ifndef HOTKEY_H
#define HOTKEY_H

#include <windows.h>

#define HOTKEY_ID 1

/* ลงทะเบียน hotkey ด้วย mod+vk ที่กำหนด */
BOOL hotkey_register(UINT mod, UINT vk);
void hotkey_unregister(void);

#endif
