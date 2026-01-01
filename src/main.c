#include "base/base_inc.h"
#include "os/os_inc.h"

#include "base/base_inc.c"
#include "os/os_inc.c"

int main(int argc, char **argv)
{
	Arena *arena = arena = arena_new(
		GB(4),
		os_reserve,
		os_commit,
		os_decommit,
		os_release
	);

	string8_list args = str8_array_from_cstring_array(arena, (usize)argc, argv);

	for(usize i=0; i<args.len; ++i) {
		String8 str = *string8_list_at(&args, i);

		log_trace(S_FMT, str8_fmt(str));
	}

	arena_delete(arena);
}

