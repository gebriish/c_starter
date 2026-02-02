#ifndef TOKEN_H
#define TOKEN_H

#include "base.h"

typedef u32 Token_Kind;
enum {
	Tok_Invalid,
	Tok_Eof,

	Tok_Comment,

	Tok_Ident,   // main
	Tok_Integer, // 123
	Tok_Float,   //123.12
	Tok_Rune,    // 'a'
	Tok_String,  // "hello"

	Tok_Eq,    // =
	Tok_Not,   // !
	Tok_Hash,  // #

	Tok_Add,      // +
	Tok_Sub,      // -
	Tok_Mul,      // *
	Tok_Quo,      // /
	Tok_Mod,      // %

	Tok_Add_Eq,     // +=
	Tok_Sub_Eq,     // -=
	Tok_Mul_Eq,     // *=
	Tok_Quo_Eq,     // /=
	Tok_Mod_Eq,     // %=

	Tok_Cmp_Eq, // ==
	Tok_Not_Eq, // !=
	Tok_Lt,     // <
	Tok_Gt,     // >
	Tok_Lt_Eq,  // <=
	Tok_Gt_Eq,  // >=

	Tok_Open_Paren,    // (
	Tok_Close_Paren,   // )
	Tok_Open_Bracket,  // [
	Tok_Close_Bracket, // ]
	Tok_Open_Brace,    // {
	Tok_Close_Brace,   // }
	Tok_Colon,         // :
	Tok_Semicolon,     // ;
	Tok_Period,        // .
	Tok_Comma,         // ,
	Tok_Range,         // ..

	Tok_If,          // if
	Tok_ElIf,        // elif
	Tok_Else,        // else
	Tok_Loop,        // loop
	Tok_In,          // in
	Tok_Not_In,      // not_in
	Tok_Case,        // case
	Tok_Break,       // break
	Tok_Continue,    // continue
	Tok_Fallthrough, // fallthrough
	Tok_Defer,       // defer
	Tok_Return,      // return
	Tok_fn,          // fn
	Tok_Struct,      // struct
	Tok_Cast,        // cast
	Tok_Count,
};

typedef struct {
	String8 file;
	usize offset;
	usize line;
	usize column;
} Tok_Pos;


typedef struct {
	Token_Kind kind;
	String8 text;
	Tok_Pos position;
} Token;

global const String8 token_strings[Tok_Count] = {
	[Tok_Invalid] = S("Invalid"),
	[Tok_Eof]     = S("EOF"),

	[Tok_Comment] = S("Comment"),

	[Tok_Ident]   = S("identifier"),
	[Tok_Integer] = S("integer"),
	[Tok_Float]   = S("float"),
	[Tok_Rune]    = S("rune"),
	[Tok_String]  = S("string"),

	[Tok_Eq]   = S("="),
	[Tok_Not]  = S("!"),
	[Tok_Hash] = S("#"),

	[Tok_Add] = S("+"),
	[Tok_Sub] = S("-"),
	[Tok_Mul] = S("*"),
	[Tok_Quo] = S("/"),
	[Tok_Mod] = S("%"),

	[Tok_Add_Eq] = S("+="),
	[Tok_Sub_Eq] = S("-="),
	[Tok_Mul_Eq] = S("*="),
	[Tok_Quo_Eq] = S("/="),
	[Tok_Mod_Eq] = S("%="),

	[Tok_Cmp_Eq] = S("=="),
	[Tok_Not_Eq] = S("!="),
	[Tok_Lt]     = S("<"),
	[Tok_Gt]     = S(">"),
	[Tok_Lt_Eq]  = S("<="),
	[Tok_Gt_Eq]  = S(">="),

	[Tok_Open_Paren]    = S("("),
	[Tok_Close_Paren]   = S(")"),
	[Tok_Open_Bracket]  = S("["),
	[Tok_Close_Bracket] = S("]"),
	[Tok_Open_Brace]    = S("{"),
	[Tok_Close_Brace]   = S("}"),
	[Tok_Colon]         = S(":"),
	[Tok_Semicolon]     = S(";"),
	[Tok_Period]        = S("."),
	[Tok_Comma]         = S(","),
	[Tok_Range]         = S(".."),

	[Tok_If]          = S("if"),
	[Tok_ElIf]        = S("elif"),
	[Tok_Else]        = S("else"),
	[Tok_Loop]        = S("loop"),
	[Tok_In]          = S("in"),
	[Tok_Not_In]      = S("not_in"),
	[Tok_Case]        = S("case"),
	[Tok_Break]       = S("break"),
	[Tok_Continue]    = S("continue"),
	[Tok_Fallthrough] = S("fallthrough"),
	[Tok_Defer]       = S("defer"),
	[Tok_Return]      = S("return"),
	[Tok_fn]          = S("fn"),
	[Tok_Struct]      = S("struct"),
	[Tok_Cast]        = S("cast"),
};



internal bool is_newline(Token tok);
internal String8 token_to_string(Token token);

typedef void (*Error_Handler) (Tok_Pos pos, const char *fmt, ...);
internal void tok_default_error_handler(Tok_Pos pos, const char *fmt, ...);

typedef struct {
	// immutable data
	String8 path;
	String8 src;
	Error_Handler err;

	// Tokenizing state
	rune ch;
	usize offset;
	usize read_offset;
	usize line_offset;
	usize line_count;

	// mutable data
	usize error_count;
} Tokenizer;

internal void tokenizer_init(Tokenizer *t, String8 src, String8 path, Error_Handler err);
internal void tok_advance_rune(Tokenizer *t);
internal u8   tok_peek_byte(Tokenizer *t, usize offset);
internal void tok_skip_whitspace(Tokenizer *t);


#endif

