#include "hotkey.h"

static BOOL registered = FALSE;

BOOL hotkey_register(void) {
    if (!registered) {
        registered = RegisterHotKey(NULL, HOTKEY_ID, MOD_CONTROL, VK_SPACE);
    }
    return registered;
}

void hotkey_unregister(void) {
    if (registered) {
        UnregisterHotKey(NULL, HOTKEY_ID);
        registered = FALSE;
    }
}
