#ifndef SETTINGS_H
#define SETTINGS_H

#include <windows.h>

/* ── ค่า default ─────────────────────────────────────── */
#define SETTINGS_FILE   "data\\settings.ini"
#define DEFAULT_MOD     MOD_CONTROL
#define DEFAULT_VK      VK_SPACE
#define HOTKEY_ID       1

typedef struct {
    UINT  mod_keys;     /* MOD_CONTROL, MOD_ALT, MOD_SHIFT, MOD_WIN */
    UINT  vk_code;      /* Virtual key code */
    BOOL  run_on_startup; /* เปิดพร้อม Windows */
} AppSettings;

/* โหลด/บันทึก settings */
void settings_load(AppSettings *s);
void settings_save(const AppSettings *s);

/* ลงทะเบียน / ลบ startup registry */
void settings_apply_startup(BOOL enable);

/* แปลง mod+vk เป็น string เช่น "Ctrl+Space" */
void settings_hotkey_to_string(UINT mod, UINT vk, char *out, int outlen);

/* เปิดหน้าต่าง Settings — คืนค่า TRUE ถ้า settings เปลี่ยน */
BOOL settings_show_dialog(HWND parent, AppSettings *s);

#endif
