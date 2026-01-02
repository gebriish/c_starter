#include "base/base_inc.h"
#include "os/os_inc.h"

#include "base/base_inc.c"
#include "os/os_inc.c"

int main(int argc, char **argv)
{
	Arena *arena = arena = arena_new(GB(4),os_reserve,os_commit,os_decommit,os_release);

	string8_list args = str8_array_from_cstring_array(arena, (usize)argc, argv);

	arena_delete(arena);
}

