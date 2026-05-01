#include "scanner.h"
#include "indexer.h"
#include <stdio.h>

static int on_file_found(const char *full_path, const char *name,
                         FILETIME last_access, void *user_data)
{
    Index *idx = (Index *)user_data;
    static int count = 0;
    
    if (index_add(idx, full_path, name, last_access) == 0) {
        count++;
        if (count % 1000 == 0) {
            printf("\rIndexed %d files...", count);
            fflush(stdout);
        }
    }
    return 1;
}

int main() {
    printf("=== Building File Index ===\n");
    printf("Scanning entire C: drive...\n");
    
    Index *idx = index_create();
    if (!idx) {
        printf("Failed to create index\n");
        return 1;
    }
    
    int total = scan_directory("C:\\", on_file_found, idx, 0, 20);
    
    printf("\n\nScanned %d files\n", total);
    printf("Indexed %d files\n", idx->count);
    
    if (index_save(idx, "data/index.db") == 0) {
        printf("Index saved to data/index.db\n");
    } else {
        printf("Failed to save index\n");
    }
    
    index_destroy(idx);
    return 0;
}
