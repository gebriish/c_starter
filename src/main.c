#include "base.h"
#include "base.c"

#include <stdio.h>


int main(int argc,const char **argv){
	Allocator arena = arena_allocator(Gb(4));
	String8_List args = str8_make_list(argv,(usize)argc,arena);

	String8 src = os_data_from_path(args.array[1],arena,arena);

	for(usize i=0; i<src.len; ++i) {
		u8 byte = src.str[i];
	}
}
