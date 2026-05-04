#include "hotkey.h"

static BOOL registered = FALSE;

BOOL hotkey_register(UINT mod, UINT vk) {
    if (registered) hotkey_unregister();
    registered = RegisterHotKey(NULL, HOTKEY_ID, mod, vk);
    return registered;
}

void hotkey_unregister(void) {
    if (registered) {
        UnregisterHotKey(NULL, HOTKEY_ID);
        registered = FALSE;
    }
}
