#include "../base.h"
#include "base_linux.h"

///////////////////////
// ~geb: helpers

internal OS_FileProps 
os_linx_file_props_from_stats(struct stat *s)
{
	OS_FileProps props = {0};

	props.size     = (usize)s->st_size;
	props.created  = os_linx_time_from_timespec(s->st_ctim);
	props.modified = os_linx_time_from_timespec(s->st_mtim);

	MaskSet(props.flags, S_ISDIR(s->st_mode),           OS_FileFlag_Directory);
	MaskSet(props.flags, !(s->st_mode & S_IWUSR),       OS_FileFlag_ReadOnly);
	MaskSet(props.flags,  (s->st_mode & S_IXUSR),       OS_FileFlag_Executable);
	MaskSet(props.flags,  S_ISLNK(s->st_mode),          OS_FileFlag_Symlink);

	return props;
}

internal u64
os_linx_time_from_timespec(struct timespec in)
{
	return ((u64)in.tv_sec * 1000000000ULL) + (u64)in.tv_nsec;
}

///////////////////////
// ~geb: std handles

internal OS_Handle os_stdout(void)
{
	return (OS_Handle)STDOUT_FILENO;
}

internal OS_Handle os_stdin(void)
{
	return (OS_Handle)STDIN_FILENO;
}

internal OS_Handle os_stderr(void)
{
	return (OS_Handle)STDERR_FILENO;
}

///////////////////////
// ~geb: virtual memory

internal void *
os_reserve(usize size)
{
	void *p = mmap(0, size, PROT_NONE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				-1, 0);
	return (p == MAP_FAILED) ? 0 : p;
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

///////////////////////
// ~geb: time

internal OS_Time_Stamp
os_time_now(void)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
		return 0;

	return ((OS_Time_Stamp)ts.tv_sec * 1000000000ULL) +
	(OS_Time_Stamp)ts.tv_nsec;
}

internal OS_Time_Stamp
os_time_frequency(void)
{
	return 1000000000ULL;
}

internal void
os_sleep_ns(u64 ns)
{
	struct timespec ts;
	ts.tv_sec  = ns / 1000000000ULL;
	ts.tv_nsec = ns % 1000000000ULL;

	while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {
		// continue sleeping remaining duration
	}
}

///////////////////////
// ~geb: files

internal OS_Handle
os_file_open(OS_AccesFlags flags, String8 path, Allocator temp_alloc)
{
	String8 cpath = str8_copy_cstring(path, temp_alloc);
	if (!cpath.str)
		return 0;

	int lnx_flags = 0;

	if ((flags & OS_AccessFlag_Read) &&
		(flags & OS_AccessFlag_Write))
	{
		lnx_flags = O_RDWR;
	}
	else if (flags & OS_AccessFlag_Write)
	{
		lnx_flags = O_WRONLY;
	}
	else
{
		lnx_flags = O_RDONLY;
	}

	if (flags & OS_AccessFlag_Append)
		lnx_flags |= O_APPEND;

	if (flags & (OS_AccessFlag_Write | OS_AccessFlag_Append))
		lnx_flags |= O_CREAT;

	lnx_flags |= O_CLOEXEC;

	int fd = open((char *)cpath.str, lnx_flags, 0644);

	str8_delete(temp_alloc, &cpath);

	// preserve your contract: 0 is invalid
	if (fd <= 0)
		return 0;

	return (OS_Handle)fd;
}

internal void
os_file_close(OS_Handle file)
{
	if (file == 0)
		return;

	close((int)file);
}

internal usize
os_file_read(OS_Handle file, usize begin, usize end, void *out_data)
{
	if (file == 0 || end <= begin)
		return 0;

	int fd = (int)file;
	u8 *dst = (u8 *)out_data;

	usize remaining = end - begin;
	usize total = 0;

	while (remaining > 0)
	{
		ssize_t r = pread(fd,
					dst + total,
					remaining,
					begin + total);

		if (r > 0)
		{
			total += (usize)r;
			remaining -= (usize)r;
		}
		else if (r == 0)
		{
			break; // EOF
		}
		else if (errno != EINTR)
		{
			break;
		}
	}

	return total;
}

internal usize
os_file_write(OS_Handle file, usize begin, usize end, void *data)
{
	if (file == 0 || end <= begin)
		return 0;

	int fd = (int)file;
	u8 *src = (u8 *)data;

	usize remaining = end - begin;
	usize total = 0;

	while (remaining > 0)
	{
		ssize_t w = pwrite(fd,
					 src + total,
					 remaining,
					 begin + total);

		if (w > 0)
		{
			total += (usize)w;
			remaining -= (usize)w;
		}
		else if (w == 0)
		{
			break;
		}
		else if (errno != EINTR)
		{
			break;
		}
	}

	return total;
}

internal OS_FileProps
os_properties_from_file(OS_Handle file)
{
	if (file == 0)
		return (OS_FileProps){0};

	struct stat st;
	if (fstat((int)file, &st) != 0)
		return (OS_FileProps){0};

	return os_linx_file_props_from_stats(&st);
}
