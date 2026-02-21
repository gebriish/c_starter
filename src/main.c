#include "base.h"
#include "base.c"

int main () {
	Allocator arena = arena_allocator(Mb(4));

	String8 data = os_data_from_path(S("src/main.c"), arena, arena);
	printf(STR "\n", s_fmt(data));
}
