#include "base.h"
#include "base.c"

int main() {
	Allocator arena = arena_allocator(Mb(512));

	printf("hello, world\n");
}
