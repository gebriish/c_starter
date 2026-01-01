#ifndef BASE_STRING_H
#define BASE_STRING_H

#include "base_core.h"
#include "base_list.h"
#include "base_arena.h"

typedef struct String8 String8; // UTF-8 String
struct String8 {
	u8 *str;
	usize len;
};

Generate_List(String8, string8_list)

typedef u32 rune;

#define S(x) (String8) {.str = (u8 *)x, .len = (usize) sizeof(x)}
#define S_FMT "%.*s"
#define str8_fmt(s) (int)s.len, (char *)s.str

force_inline String8 str8_empty() {
	return S("");
}

internal String8 str8(u8 *mem, usize len);
internal String8 str8_slice(String8 string, usize begin, usize end_exclusive);
internal String8 str8_concat(String8 left, String8 right, Arena *arena);
internal String8 str8_sprintf(Arena *arena, const char *fmt, ...);
internal bool    str8_equal(String8 s1, String8 s2);

/* ~geb: Doesnt copy string data itself, strings are a view */  
internal string8_list str8_list_from_cstring_array(Arena *arena, usize count, char **cstrings);


typedef u32 UTF8_Error;
enum {
	UTF8_Err_None = 0,
	UTF8_Err_OutOfBounds,
	UTF8_Err_InvalidLead,
	UTF8_Err_InvalidContinuation,
	UTF8_Err_Overlong,
	UTF8_Err_Surrogate,
	UTF8_Err_OutOfRange,
};

// UTF-8
internal usize utf8_decode(String8 string, usize idx, rune *out, UTF8_Error *err);

#endif
