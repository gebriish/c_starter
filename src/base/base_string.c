#include "base_string.h"


internal String8 
str8(u8 *mem, usize len)
{
	String8 str = {
		mem,
		len
	};
	return str;
}


internal String8 
str8_slice(String8 string, usize start, usize end_exclusive)
{
	AssertAlways(start >= 0 && end_exclusive <= string.len && end_exclusive > start);
	return str8(string.str + start, Min(string.len, end_exclusive - start));
}

internal String8
str8_concat(String8 left, String8 right, Arena *arena)
{
	Assert(arena != 0);

	usize total_len = left.len + right.len;
	Assert(total_len >= left.len);

	u8 *str_data = arena_push_array(arena, u8, total_len);

	MemMove(str_data, left.str, left.len);
	MemMove(str_data + left.len, right.str, right.len);

	return str8(str_data, total_len);
}

internal String8
str8_sprintf(Arena *arena, const char *fmt, ...)
{
	va_list args, args_copy;
	va_start(args, fmt);

	va_copy(args_copy, args);
	int size = vsnprintf(NULL, 0, fmt, args_copy);
	va_end(args_copy);

	if (size < 0) {
		va_end(args);
		return S("");
	}

	u8* buf = arena_push_array(arena, u8, size + 1);
	if (!buf) {
		va_end(args);
		return S("");
	}

	vsnprintf((char *)buf, size + 1, fmt, args);
	va_end(args);

	return str8(buf, size);
}

internal bool
str8_equal(String8 s1, String8 s2)
{
	if (s1.len != s2.len) return false;
	return MemCompare(s1.str, s2.str, s1.len) == 0;
}


internal string8_list
str8_array_from_cstring_array(Arena *arena, usize count, char **cstrings)
{
	Assert(arena);
	Assert(count != 0 && cstrings);

	string8_list arr = string8_list_make(arena, count);

	for (usize i = 0; i < count; i++)
	{
		usize len = MemStrlen(cstrings[i]);
		String8 str = str8((u8 *)cstrings[i], len);

		string8_list_push(&arr, str);
	}

	return arr;
}


force_inline bool utf8_is_cont(u8 b) {
	return (b & 0xC0) == 0x80;
}

static const u8 len_table[256] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x00–0x0F
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x10–0x1F
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x20–0x2F
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x30–0x3F
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x40–0x4F
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x50–0x5F
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x60–0x6F
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 0x70–0x7F

	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x80–0x8F
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x90–0x9F
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0xA0–0xAF
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0xB0–0xBF

	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // 0xC0–0xCF
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // 0xD0–0xDF
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, // 0xE0–0xEF

	4,4,4,4,4,4,4,4,  // F0–F7
	0,0,0,0,0,0,0,0   // F8–FF (invalid in UTF-8)
};

internal usize
utf8_decode(String8 s, usize idx, rune *out, UTF8_Error *err)
{
    if (err) *err = UTF8_Err_None;
    if (idx >= s.len) {
        if (err) *err = UTF8_Err_OutOfBounds;
        return 0;
    }

    u8 *p = s.str + idx;
    u8 b0 = p[0];

    u8 len = len_table[b0];
    if (len == 0) {
        if (err) *err = UTF8_Err_InvalidLead;
        return 0;
    }

    if (idx + len > s.len) {
        if (err) *err = UTF8_Err_OutOfBounds;
        return 0;
    }

    rune r = 0;

    switch (len) {
    case 1:
        r = b0;
        break;

    case 2:
        if (!utf8_is_cont(p[1])) goto invalid_cont;
        r = ((b0 & 0x1F) << 6) |
            (p[1] & 0x3F);
        if (r < 0x80) goto overlong;
        break;

    case 3:
        if (!utf8_is_cont(p[1]) || !utf8_is_cont(p[2])) goto invalid_cont;
        r = ((b0 & 0x0F) << 12) |
            ((p[1] & 0x3F) << 6) |
            (p[2] & 0x3F);
        if (r < 0x800) goto overlong;
        if (r >= 0xD800 && r <= 0xDFFF) goto surrogate;
        break;

    case 4:
        if (!utf8_is_cont(p[1]) || !utf8_is_cont(p[2]) || !utf8_is_cont(p[3]))
            goto invalid_cont;
        r = ((b0 & 0x07) << 18) |
            ((p[1] & 0x3F) << 12) |
            ((p[2] & 0x3F) << 6) |
            (p[3] & 0x3F);
        if (r < 0x10000) goto overlong;
        if (r > 0x10FFFF) goto out_of_range;
        break;
    }

    *out = r;
    return len;

invalid_cont:
    if (err) *err = UTF8_Err_InvalidContinuation;
    return 0;

overlong:
    if (err) *err = UTF8_Err_Overlong;
    return 0;

surrogate:
    if (err) *err = UTF8_Err_Surrogate;
    return 0;

out_of_range:
    if (err) *err = UTF8_Err_OutOfRange;
    return 0;
}
