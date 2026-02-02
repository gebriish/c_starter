#ifndef BASE_WIN32_H
#define BASE_WIN32_H

#include "../base.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

internal u64 os_win32_u64_from_filetime(FILETIME ft);
internal OS_FileProps os_win32_file_props_from_handle(HANDLE h);


#endif
