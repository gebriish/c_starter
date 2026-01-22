#include "tokenizer.h"

#include <stdio.h>
#include <stdarg.h>

//////////////////////////////////////////
// ~geb : helpers

internal Tok_Pos
_offset_to_pos(Tokenizer *t, usize offset)
{
	usize line = t->line_count;
	usize column = offset - t->line_offset + 1;

	return (Tok_Pos) {
		.file = t->path,
		.offset = offset,
		.line = line,
		.column = column,
	};
}

#define tok_error(t, offset, fmt, ...)           \
do {                                             \
    Tok_Pos pos = _offset_to_pos((t), (offset)); \
    if ((t)->err != 0) {                         \
        (t)->err(pos, (fmt), ##__VA_ARGS__);     \
    }                                            \
    (t)->error_count += 1;                       \
} while (0)

//////////////////////////////////////////

internal bool
is_newline(Token tok)
{
	return tok.kind == Tok_Semicolon && str8_equal(tok.text, S("\n"));
}

internal String8
token_to_string(Token token)
{
	if (is_newline(token)) { return S("\n"); }
	return token_strings[token.kind];
}

internal void
tokenizer_init(Tokenizer *t, String8 src, String8 path, Error_Handler err)
{
	t->src = src;
	t->err = err;
	t->ch  = cast(rune) ' ';
	t->offset = 0;
	t->line_offset = 0;
	t->line_count  = src.len > 0 ? 1 : 0;
	t->error_count = 0;

	tok_advance_rune(t);
	if (t->ch == RUNE_BOM) {
		tok_advance_rune(t);
	}
}


internal void
tok_default_error_handler(Tok_Pos pos, const char *fmt, ...)
{
	fprintf(stderr, "%s(%d:%d) ", pos.file, pos.line, pos.column);

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fprintf(stderr, "\n");
}

internal void
tok_advance_rune(Tokenizer *t)
{
	if (t->read_offset < t->src.len) {
		t->offset = t->read_offset;
		if (t->ch == '\n') {
			t->line_offset = t->offset;
			t->line_count += 1;
		}

		u8 r = t->src.str[t->read_offset];
		u8 width = 1;

		if (r == 0) {
			tok_error(t, t->offset, "illegal character NUL");
		}
		if (r >= RUNE_SELF) {
			UTF8_Error err = 0;
			r = utf8_decode(t->src.str + t->read_offset, &err);
			if (err != UTF8_Err_None) {
				tok_error(t, t->offset, "illigal UTF-8 encoding");
			}
			width = UTF8_LEN_TABLE[r];
		}

		t->read_offset += width;
		t->ch = r;

		return;
	}

	t->offset = t->src.len;
	if (t->ch == '\n') {
		t->line_offset = t->offset;
		t->line_count += 1;
	}
	t->ch = -1;
}

internal u8
tok_peek_byte(Tokenizer *t, usize offset)
{
	if (t->read_offset+offset < t->src.len) {
		return t->src.str[t->read_offset+offset];
	}
	return 0;
}

internal void
tok_skip_whitspace(Tokenizer *t)
{
	for (;;) {
		switch (t->ch) {
			case ' ': case '\t': case '\r':
				tok_advance_rune(t);
			default:
				return;
		}
	}
}
