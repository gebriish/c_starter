#include "base_win32.h"

///////////////////////
// ~geb: helper functions

internal u64
os_win32_u64_from_filetime(FILETIME ft)
{
    ULARGE_INTEGER u;
    u.LowPart  = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    return u.QuadPart;
}


internal OS_Time_Stamp
os_win32_time_from_filetime(FILETIME ft)
{
    // 11644473600 seconds between 1601-01-01 and 1970-01-01
    const u64 EPOCH_DIFF_100NS = 11644473600ULL * 10000000ULL;

    u64 t100ns = os_win32_u64_from_filetime(ft);

    OS_Time_Stamp out = {0};
    if (t100ns >= EPOCH_DIFF_100NS)
    {
        u64 unix_100ns = t100ns - EPOCH_DIFF_100NS;
        // convert 100ns -> ns
        out = unix_100ns * 100ULL;
    }
    return out;
}

internal OS_FileProps
os_win32_file_props_from_handle(HANDLE h)
{
    OS_FileProps props = {0};
    if (h == 0 || h == INVALID_HANDLE_VALUE)
        return props;

    FILE_BASIC_INFO basic = {0};
    FILE_STANDARD_INFO stdinfo = {0};

    if (!GetFileInformationByHandleEx(h, FileBasicInfo, &basic, sizeof(basic)))
        return props;

    if (!GetFileInformationByHandleEx(h, FileStandardInfo, &stdinfo, sizeof(stdinfo)))
        return props;

    props.size = (usize)stdinfo.EndOfFile.QuadPart;
    props.created = os_win32_time_from_filetime(*(FILETIME *)&basic.CreationTime);
    props.modified = os_win32_time_from_filetime(*(FILETIME *)&basic.LastWriteTime);

    DWORD attr = basic.FileAttributes;

    MaskSet(props.flags, (attr & FILE_ATTRIBUTE_DIRECTORY) != 0, OS_FileFlag_Directory);
    MaskSet(props.flags, (attr & FILE_ATTRIBUTE_READONLY) != 0, OS_FileFlag_ReadOnly);
    MaskSet(props.flags, (attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0, OS_FileFlag_Symlink);

    return props;
}

internal WCHAR *
os_win32_utf16_from_utf8(String8 s, Allocator a)
{
    if (s.len == 0)
        return 0;

    int needed = MultiByteToWideChar(CP_UTF8, 0, (char *)s.str, (int)s.len, 0, 0);
    if (needed <= 0)
        return 0;

    WCHAR *w = (WCHAR *)mem_alloc(a, (needed + 1) * sizeof(WCHAR), false, NULL);
    if (!w)
        return 0;

    MultiByteToWideChar(CP_UTF8, 0, (char *)s.str, (int)s.len, w, needed);
    w[needed] = 0;
    return w;
}

///////////////////////

internal void *
os_reserve(usize size)
{
    void *result = VirtualAlloc(
        0,
        size,
        MEM_RESERVE,
        PAGE_NOACCESS);

    return result;
}

internal int
os_commit(void *ptr, usize size)
{
    void *result = VirtualAlloc(
        ptr,
        size,
        MEM_COMMIT,
        PAGE_READWRITE);

    return (result == 0);
}

internal void
os_decommit(void *ptr, usize size)
{
    VirtualFree(
        ptr,
        size,
        MEM_DECOMMIT);
}

internal void
os_release(void *ptr, usize size)
{
    (void)size;
    VirtualFree(
        ptr,
        0,
        MEM_RELEASE);
}

internal OS_Time_Stamp
os_time_now()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    u64 t = (u64)counter.QuadPart;
    u64 f = (u64)freq.QuadPart;

    // ns = (t / f)*1e9 + ((t % f)*1e9)/f
    u64 sec  = t / f;
    u64 rem  = t % f;

    return (OS_Time_Stamp)(sec * 1000000000ULL + (rem * 1000000000ULL) / f);
}

internal OS_Time_Stamp
os_time_frequency()
{
    return 1000000000ULL;
}

internal void
os_sleep_ns(u64 ns)
{
    HANDLE timer = CreateWaitableTimerA(NULL, TRUE, NULL);
    if (timer)
    {
        LARGE_INTEGER due;
        due.QuadPart = -(LONGLONG)(ns / 100); // 100 ns units
        SetWaitableTimer(timer, &due, 0, NULL, NULL, FALSE);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    }
}

///////////////////////

internal OS_Handle
os_file_open(OS_AccesFlags flags, String8 path, Allocator temp_alloc)
{
    OS_Handle handle = {0};

    WCHAR *wpath = os_win32_utf16_from_utf8(path, temp_alloc);
    if (!wpath)
        return handle;

    DWORD access = 0;
    if (flags & OS_AccessFlag_Read)
        access |= GENERIC_READ;
    if (flags & OS_AccessFlag_Write)
        access |= GENERIC_WRITE;

    if (flags & OS_AccessFlag_Append)
    {
        access |= FILE_APPEND_DATA;
    }

    DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

    DWORD creation = OPEN_EXISTING;
    if (flags & (OS_AccessFlag_Write | OS_AccessFlag_Append))
    {
        creation = OPEN_ALWAYS;
    }

    DWORD attribs = FILE_ATTRIBUTE_NORMAL;

    HANDLE h = CreateFileW(
        wpath,
        access,
        share,
        NULL,
        creation,
        attribs,
        NULL);

    if (h != INVALID_HANDLE_VALUE)
    {
        handle.u64[0] = (u64)h;
    }

    return handle;
}

internal void
os_file_close(OS_Handle file)
{
    HANDLE h = cast(HANDLE) file.u64[0];
    if (h == 0 || h == INVALID_HANDLE_VALUE)
        return;
    CloseHandle(h);
}

internal usize
os_file_read(OS_Handle file, usize begin, usize end, void *out_data)
{
    HANDLE h = cast(HANDLE) file.u64[0];
    if (h == 0 || h == INVALID_HANDLE_VALUE)
        return 0;

    usize total_to_read = end - begin;
    usize total_read = 0;

    while (total_read < total_to_read)
    {
        DWORD chunk = cast(DWORD) Min((usize)0x7ffff000, total_to_read - total_read);

        OVERLAPPED ov = {0};
        u64 off = cast(u64)begin + cast(u64)total_read;
        ov.Offset = cast(DWORD)(off & 0xFFFFFFFF);
        ov.OffsetHigh = cast(DWORD)(off >> 32);

        DWORD got = 0;
        BOOL ok = ReadFile(h, (u8 *)out_data + total_read, chunk, &got, &ov);
        if (!ok)
        {
            DWORD err = GetLastError();
            break;
        }

        if (got == 0)
            break; // EOF
        total_read += got;
    }

    return total_read;
}

internal usize
os_file_write(OS_Handle file, usize begin, usize end, void *data)
{
    HANDLE h = cast(HANDLE) file.u64[0];
    if (h == 0 || h == INVALID_HANDLE_VALUE)
        return 0;

    usize total_to_write = end - begin;
    usize total_written = 0;

    while (total_written < total_to_write)
    {
        DWORD chunk = cast(DWORD)min((usize)0x7ffff000, total_to_write - total_written);

        OVERLAPPED ov = {0};
        u64 off = cast(u64)begin + cast(u64)total_written;
        ov.Offset = cast(DWORD)(off & 0xFFFFFFFF);
        ov.OffsetHigh = cast(DWORD)(off >> 32);

        DWORD wrote = 0;
        BOOL ok = WriteFile(h, (u8 *)data + total_written, chunk, &wrote, &ov);
        if (!ok)
        {
            break;
        }

        if (wrote == 0)
            break;
        total_written += wrote;
    }

    return total_written;
}

internal OS_FileProps
os_properties_from_file(OS_Handle file)
{
    HANDLE h = cast(HANDLE) file.u64[0];
    if(h == 0 || h == INVALID_HANDLE_VALUE) return (OS_FileProps){0};
    return os_win32_file_props_from_handle(h);
}
