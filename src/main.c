#include "base.h"

#include "base.c"

int main(int argc, const char **argv) {
	Allocator arena = arena_allocator(Mb(512));

	String8_List args = str8_make_list(
		argv, (usize) argc, arena 
	);
}
