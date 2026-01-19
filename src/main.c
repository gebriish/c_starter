#include "base.h"

#include "base.c"

#include <stdio.h>

int main(int argc, const char **argv) {
	Allocator arena = arena_allocator(Mb(512));

	String8 foo = S("hello, world");

	OS_Time_Stamp t0 = os_time_now();
	for (Str_Iterator itr = {0}; str8_iter(foo, &itr);)
	{
		printf(STR "\n", (int) itr.width, itr.ptr);
	}
	OS_Time_Stamp t1 = os_time_now();
	OS_Time_Duration diff = os_time_diff(t0, t1);

	printf("%f\n", diff.milliseconds);
}
