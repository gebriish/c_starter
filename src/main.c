#include "base.h"
#include "base.c"

int main () {
	Allocator arena = arena_allocator(Mb(4));

	String8 data = os_data_from_path(S("src/main.c"), arena, arena);
	log_info(STR, s_fmt(data));
	log_warn(STR, s_fmt(data));
	log_error(STR, s_fmt(data));
}
