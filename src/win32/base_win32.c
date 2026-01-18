#include "base_win32.h"

internal void *
os_reserve(usize size)
{
    void *result = VirtualAlloc(
        0,
        size,
        MEM_RESERVE,
        PAGE_NOACCESS
    );

    return result;
}

internal int
os_commit(void *ptr, usize size)
{
    void *result = VirtualAlloc(
        ptr,
        size,
        MEM_COMMIT,
        PAGE_READWRITE
    );

    return (result != 0);
}

internal void
os_decommit(void *ptr, usize size)
{
    VirtualFree(
        ptr,
        size,
        MEM_DECOMMIT
    );
}

internal void 
os_release(void *ptr, usize size)
{
    (void)size;
    VirtualFree(
        ptr,
        0,
        MEM_RELEASE
    );
}
