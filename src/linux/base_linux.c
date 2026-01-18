#include "../base.h"
#include "base_linux.h"

///////////////////////
// ~geb: helper functions

internal OS_FileProps 
os_linx_file_props_from_stats(struct stat *s)
{
	OS_FileProps props = {0};

	props.size     = (usize)s->st_size;
	props.created  = os_linx_time_from_timespec(s->st_ctim);
	props.modified = os_linx_time_from_timespec(s->st_mtim);

	MaskSet(props.flags, S_ISDIR(s->st_mode), OS_FileFlag_Directory);

	return props;
}

internal u64
os_linx_time_from_timespec(struct timespec in)
{
	return (u64)in.tv_sec * os_time_frequency() + (u64)in.tv_nsec;
}

///////////////////////

internal void *
os_reserve(usize size)
{
	void *result = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (result == MAP_FAILED)
	{
		result = 0;
	}
	return result;
}

internal int
os_commit(void *ptr, usize size)
{
	return mprotect(ptr, size, PROT_READ | PROT_WRITE);
}

internal void
os_decommit(void *ptr, usize size)
{
	madvise(ptr, size, MADV_DONTNEED);
	mprotect(ptr, size, PROT_NONE);
}

internal void
os_release(void *ptr, usize size)
{
	munmap(ptr, size);
}

internal OS_Time_Stamp
os_time_now()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (OS_Time_Stamp)ts.tv_sec * os_time_frequency() + (OS_Time_Stamp)ts.tv_nsec;
}

internal OS_Time_Stamp
os_time_frequency()
{
	return 1000000000ULL;
}

internal void
os_sleep_ns(u64 ns)
{
	struct timespec ts;
	ts.tv_sec  = ns / 1000000000ull;
	ts.tv_nsec = ns % 1000000000ull;
	nanosleep(&ts, NULL);
}

