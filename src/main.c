#include "base.h"

#include "base.c"

int main() {
	Allocator arena = arena_allocator(Mb(512));

	{
		Arena_Scope scope = arena_scope_begin(cast(Arena *)arena.data);


		arena_scope_end(scope);
	}
}
