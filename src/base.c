#include "base.h"

/////////////////////////////////////////////////////////////////////////
//                            OS Layer                                 //
/////////////////////////////////////////////////////////////////////////

#if OS_LINUX
# include "linux/base_linux.c"
#elif OS_WINDOWS
# include "win32/base_win32.c"
#else
# error "OS Implementations missing"
#endif


internal OS_Time_Duration
os_time_diff(OS_Time_Stamp start, OS_Time_Stamp end)
{
	OS_Time_Duration result;
	u64 freq = os_time_frequency();
	u64 elapsed = end - start;

	result.seconds = (f64)elapsed / (f64)freq;
	result.milliseconds = result.seconds * 1000.0;
	result.microseconds = result.seconds * 1000000.0;

	return result;
}

/////////////////////////////////////////////////////////////////////////
//                        ALLOCATORS                                   //
/////////////////////////////////////////////////////////////////////////

// ~geb: general perpose (gp) allocator

#include <stdlib.h>
force_inline void*
heap_realloc(void *original_ptr, usize new_size) {
	return realloc(original_ptr, new_size);
}

force_inline void*
heap_alloc(usize size) {
	return malloc(size);
}

force_inline void
heap_free(void *ptr) {
	free(ptr);
}

internal void *
_gp_aligned_alloc(usize size, usize alignment, void *old_ptr, usize old_size, bool mem_zero, Alloc_Error *err)
{
	if (err) *err = Alloc_Err_None;
	if (size == 0) return NULL;

	alignment = Max(alignment, AlignOf(void*));

	usize space = size + alignment - 1 + sizeof(void*);
	u8 *raw = heap_alloc(space);
	if (!raw) {
		if (err) *err = Alloc_Err_OOM;
		return NULL;
	}

	u8 *aligned = (u8*)AlignPow2((usize)(raw + sizeof(void*)), alignment);
	((void**)aligned)[-1] = raw;

	if (mem_zero) {
		MemZero(aligned, size);
	}

	return aligned;
}

internal void
_gp_aligned_free(void *p)
{
	if (!p) return;
	heap_free(((void**)p)[-1]);
}

internal void *
_gp_aligned_resize(void *p, size_t old_size, size_t new_size, size_t new_alignment, bool zero_memory, Alloc_Error *err)
{
	if (err) *err = Alloc_Err_None;

	if (!p) {
		return _gp_aligned_alloc(new_size, new_alignment, NULL, 0, zero_memory, err);
	}

	if (new_size == 0) {
		_gp_aligned_free(p);
		return NULL;
	}

	new_alignment = Max(new_alignment, AlignOf(void*));

	void *old_raw = ((void**)p)[-1];

	usize space = new_size + new_alignment - 1 + sizeof(void*);
	u8 *new_raw = heap_realloc(old_raw, space);
	if (!new_raw) {
		if (err) *err = Alloc_Err_OOM;
		return NULL;
	}

	u8 *new_aligned =
		(u8*)AlignPow2((usize)(new_raw + sizeof(void*)), new_alignment);
	((void**)new_aligned)[-1] = new_raw;

	if (new_aligned != p) {
		MemMove(new_aligned, p, Min(old_size, new_size));
	}

	if (zero_memory && new_size > old_size) {
		MemZero(new_aligned + old_size, new_size - old_size);
	}

	return new_aligned;
}

internal Allocator_Proc(gp_allocator_proc)
{
	switch (type) {
		case Allocation_Alloc_Non_Zero:
		case Allocation_Alloc:
			return _gp_aligned_alloc(size, alignment, NULL, 0, type == Allocation_Alloc, err);

		case Allocation_Free:
			_gp_aligned_free(old_memory);
			break;

		case Allocation_FreeAll:
			*err = Alloc_Err_Mode_Not_Implemented;
			return NULL;

		case Allocation_Resize_Non_Zero:
		case Allocation_Resize:
			return _gp_aligned_resize(old_memory, old_size, size, alignment, type == Allocation_Resize, err);
	}

	if (err) *err = Alloc_Err_None;
	return NULL;
}

internal Allocator
heap_allocator(void)
{
	return (Allocator) {
		.proc = gp_allocator_proc,
		.data = NULL
	};
}

// ~geb: arena allocator
#define COMMIT_BLOCK_SIZE Kb(64)

internal Arena *
_arena_new(usize reserve_size)
{
	if (reserve_size > USIZE_MAX - sizeof(Arena)) {
		//log_error("reserve size overflow (%zu)", reserve_size);
		return NULL;
	}

	usize total_size = sizeof(Arena) + reserve_size;

	void *base = os_reserve(total_size);
	if (!base) {
		return NULL;
	}

	usize header_commit_size = AlignPow2(sizeof(Arena), COMMIT_BLOCK_SIZE);

	if (os_commit(base, header_commit_size)) {
		os_release(base, total_size);
		return NULL;
	}

	Arena *arena = cast(Arena *)base;

	arena->pos       = 0;
	arena->base      = cast(u8 *)base + sizeof(Arena);
	arena->reserved  = reserve_size;
	arena->committed = header_commit_size > sizeof(Arena) ?
										header_commit_size - sizeof(Arena) : 0;
	return arena;
}

internal void
_arena_free_all(Arena *arena) {
	Assert(arena);
	arena->pos = 0;
}

internal void *
_arena_alloc_aligned(Arena *arena, usize size, usize alignment, bool zero, Alloc_Error *err)
{
	if (err) *err = Alloc_Err_None;
	
	usize aligned_pos = AlignPow2(arena->pos, alignment);
	usize new_pos     = aligned_pos + size;

	if (new_pos > arena->reserved) {
		*err = Alloc_Err_OOM;
		return NULL;
	}

	if (new_pos > arena->committed) {
		usize needed      = new_pos - arena->committed;
		usize commit_size = AlignPow2(needed, COMMIT_BLOCK_SIZE);

		if (arena->committed + commit_size > arena->reserved) {
			commit_size = arena->reserved - arena->committed;
		}

		if (commit_size == 0) {
			*err = Alloc_Err_OOM;
			return NULL;
		}

		void *commit_ptr = arena->base + arena->committed;
		if (os_commit(commit_ptr, commit_size) != 0) {
			*err = Alloc_Err_OOM;
			return NULL;
		}

		arena->committed += commit_size;
	}

	arena->pos = new_pos;
	return arena->base + aligned_pos;
}

internal void *
_arena_realloc_aligned(Arena *arena, void *ptr, usize old_size, usize new_size, usize alignment, bool zero, Alloc_Error *err) 
{
	*err = Alloc_Err_None;

	if (!arena) {
		*err = Alloc_Err_Invalid_Argument;
		return NULL;
	}

	if (!ptr) {
		*err = Alloc_Err_Invalid_Pointer;
		return NULL;
	}

	if (new_size == 0) {
		*err = Alloc_Err_Invalid_Argument;
		return NULL;
	}

	u8 *expected_end = arena->base + arena->pos;
	u8 *actual_end   = (u8 *)ptr + old_size;

	bool is_last_alloc = (expected_end == actual_end);

	if (is_last_alloc) {
		usize offset     = (u8 *)ptr - (u8 *)arena->base;
		usize new_pos    = offset + new_size;

		if (new_pos > arena->reserved) {
			*err = Alloc_Err_OOM;
			return NULL;
		}

		if (new_pos > arena->committed) {
			usize needed      = new_pos - arena->committed;
			usize commit_size = AlignPow2(needed, COMMIT_BLOCK_SIZE);

			if (arena->committed + commit_size > arena->reserved) {
				commit_size = arena->reserved - arena->committed;
			}

			if (commit_size == 0) {
				*err = Alloc_Err_OOM;
				return NULL;
			}

			void *commit_ptr = arena->base + arena->committed;
			if (os_commit(commit_ptr, commit_size) != 0) {
				*err = Alloc_Err_OOM;
				return NULL;
			}

			arena->committed += commit_size;
		}

		arena->pos = new_pos;
		return ptr;
	}

	void *new_ptr = _arena_alloc_aligned(
		arena,
		new_size,
		alignment,
		zero,
		err
	);

	if (!new_ptr) {
		return NULL;
	}

	usize copy_size = old_size < new_size ? old_size : new_size;
	MemMove(new_ptr, ptr, copy_size);

	return new_ptr;
}

internal Allocator_Proc(arena_allocator_proc)
{
	Arena *arena = (Arena *) allocator_data;
	
	switch (type) {
		case Allocation_Alloc_Non_Zero: case Allocation_Alloc:
			return _arena_alloc_aligned(arena, size, alignment, type == Allocation_Alloc, err);

		case Allocation_Resize_Non_Zero: case Allocation_Resize:
			return _arena_realloc_aligned(
				arena, old_memory, old_size, size, 
				alignment, type == Allocation_Resize, err
			);
	
		case Allocation_Free:
			*err = Alloc_Err_Mode_Not_Implemented;
		break;

		case Allocation_FreeAll:
			_arena_free_all(arena);
			break;
	}

	return NULL;
}

internal Allocator
arena_allocator(usize reserve)
{
	Arena *arena = _arena_new(reserve);

	return (Allocator) {
		.proc = arena_allocator_proc,
		.data = arena
	};
}


internal Arena_Scope
arena_scope_begin(Arena *arena)
{
	Assert(arena);

	Arena_Scope scope = {
		.arena = arena,
		.pos   = arena->pos
	};
	return scope;
}

internal void
arena_scope_end(Arena_Scope scope)
{
	Assert(scope.arena && scope.pos >= scope.arena->pos);

	Arena * arena = scope.arena;
	arena->pos = scope.pos;
}

/////////////////////////////////////////////////////////////////////////
//                            STRINGS                                  //
/////////////////////////////////////////////////////////////////////////

internal String8
str8_make(const char *cstring, Allocator allocator)
{
	usize len = MemStrlen(cstring);
	String8 string = {
		.alloc = allocator,
		.len = len,
	};

	Alloc_Error err = 0;
	string.str = alloc_array(allocator, u8, len, &err);
	MemMove(string.str, cstring, len);

	if (err) {
		return S("");
	}

	return string;
}

internal Alloc_Error
str8_delete(String8 *str)
{
	Allocator a = str->alloc;
	if (!a.proc) { return Alloc_Err_Mode_Not_Implemented; }

	Alloc_Error err = 0;
	mem_free(a, str->str, &err);

	if (err) { return err; }

	return Alloc_Err_None;
}

internal String8_List
str8_make_list(const char **cstrings, usize count, Allocator allocator)
{
	String8_List list = {0};

	list.alloc = allocator;
	list.size = count;
	
	Alloc_Error err = 0;
	list.array = alloc_array(allocator, String8, count, &err);

	if (err) {
		return (String8_List) {0};
	}

	for (usize i=0; i<count; ++i) {
		list.array[i] = str8_make(cstrings[i], allocator);
	}

	return list;
}

internal Alloc_Error
str8_delete_list(String8_List *list)
{
	Allocator a = list->alloc;
	if (!a.proc) { return Alloc_Err_Mode_Not_Implemented; }

	for (usize i=0; i<list->size; ++i) {
		Alloc_Error err = str8_delete(&list->array[i]);
		if (err) { return err; }
	}

	Alloc_Error err = 0;
	mem_free(list->alloc, list->array, &err);
	if (err) { return err; }

	return Alloc_Err_None;
}

internal String8
str8_list_index(String8_List *list, usize i)
{
	Assert(list);
	Assert(i < list->size);

	return list->array[i];
}


internal String8
str8_slice(String8 string, usize begin, usize end_exclusive)
{
	Assert(begin <= end_exclusive);
	Assert(end_exclusive <= string.len);

	String8 result = {0};
	result.str = string.str + begin;
	result.len = end_exclusive - begin;

	return result;
}

internal bool
str8_equal(String8 first, String8 second) 
{
	if (first.len != second.len) {
		return false;
	}

	return MemCompare(first.str, second.str, first.len) == 0;
}
