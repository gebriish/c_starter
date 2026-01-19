#include "base_win32.h"


///////////////////////
// ~geb: helper functions



///////////////////////

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


internal OS_Time_Stamp
os_time_now()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (OS_Time_Stamp)counter.QuadPart;
}

internal OS_Time_Stamp
os_time_frequency()
{
    static OS_Time_Stamp freq = 0;
    if (freq == 0)
    {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        freq = (OS_Time_Stamp)f.QuadPart;
    }
    return freq;
}

internal u64
os_time_ns(OS_Time_Stamp t)
{
    return (u64)((t * 1000000000ull) / os_time_frequency());
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
