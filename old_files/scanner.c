#include "scanner.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>

#ifndef PATH_MAX
#define PATH_MAX 32768
#endif

static int scan_internal(const char *root_dir, ScanCallback cb,
                         void *user_data, int depth, int max_depth)
{
    WIN32_FIND_DATAA data;
    char path[PATH_MAX];
    char full[PATH_MAX];

    if (max_depth > 0 && depth > max_depth)
        return 0;

    if (snprintf(path, sizeof(path), "%s\\*", root_dir) >= (int)sizeof(path)) {
        fprintf(stderr, "[ERROR] Path too long: %s\n", root_dir);
        return -1;
    }

    HANDLE hFind = FindFirstFileA(path, &data);
    if (hFind == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err != ERROR_FILE_NOT_FOUND && err != ERROR_PATH_NOT_FOUND) {
            fprintf(stderr, "[WARN] Cannot access: %s (error %lu)\n", root_dir, err);
        }
        return -1;
    }

    int count = 0;

    do {
        if (strcmp(data.cFileName, ".") == 0 ||
            strcmp(data.cFileName, "..") == 0)
            continue;

        if (snprintf(full, sizeof(full), "%s\\%s", root_dir, data.cFileName) >= (int)sizeof(full)) {
            fprintf(stderr, "[WARN] Path too long, skipping: %s\\%s\n", root_dir, data.cFileName);
            continue;
        }

        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            int sub_count = scan_internal(full, cb, user_data, depth + 1, max_depth);
            if (sub_count > 0) {
                count += sub_count;
            }
        } else {
            if (cb) {
                BOOL is_reparse = (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
                if (!is_reparse) {
                    cb(full, data.cFileName, data.ftLastAccessTime, user_data);
                    count++;
                }
            } else {
                count++;
            }
        }
    } while (FindNextFileA(hFind, &data));

    FindClose(hFind);
    return count;
}

int scan_directory(const char *root_dir, ScanCallback cb,
                   void *user_data, int depth, int max_depth)
{
    if (!root_dir || !*root_dir) {
        fprintf(stderr, "[ERROR] scan_directory: root_dir is NULL or empty\n");
        return -1;
    }

    (void)depth;
    return scan_internal(root_dir, cb, user_data, 0, max_depth);
}
