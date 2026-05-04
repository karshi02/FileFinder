#ifndef SCANNER_H
#define SCANNER_H

#include <windows.h>

#ifndef PATH_MAX
#define PATH_MAX 32768  /* รองรับ path ยาว */
#endif

typedef int (*ScanCallback)(const char *full_path, const char *name,
                            FILETIME last_access, void *user_data);

int scan_directory(const char *root_dir, ScanCallback cb,
                   void *user_data, int depth, int max_depth);

#endif
