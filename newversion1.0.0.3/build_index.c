#include "universal_scanner.h"
#include <stdio.h>
#include <windows.h>

int main(void) {
    printf("=== Building Universal Index ===\n");
    printf("Scanning Start Menu, Program Files, Recent Files...\n");

    CreateDirectoryA("data", NULL);

    UniversalIndex *idx = universal_create();
    if (!idx) {
        printf("Failed to create index\n");
        return 1;
    }

    int total = universal_scan_all(idx);
    printf("\nScanned %d items total\n", total);

    if (universal_save(idx, "data\\universal.db") == 0) {
        printf("Index saved to data\\universal.db\n");
    } else {
        printf("Failed to save index\n");
    }

    universal_destroy(idx);
    return 0;
}
