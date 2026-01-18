///////////////////////////////////////////////////////////////////
// ~geb: This is the base layer that is going to be used in almost
//       every other source file. It includes some usefull macros
//       for OS/Architecture recognision, easy type shorthands,
//       compiler intrinsics and A minimal Standard library.
///////////////////////////////////////////////////////////////////

#ifndef BASE_H
#define BASE_H

////////////////////////////////
// ~geb: Compiler/OS Cracking

#if defined(__clang__)

# define COMPILER_CLANG 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__gnu_linux__) || defined(__linux__)
#  define OS_LINUX 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# else
#  error This compiler/OS combo is not supported.
# endif

# if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#  define ARCH_X64 1
# elif defined(i386) || defined(__i386) || defined(__i386__)
#  define ARCH_X86 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# elif defined(__arm__)
#  define ARCH_ARM32 1
# else
#  error Architecture not supported.
# endif


#elif defined(_MSC_VER)

# define COMPILER_MSVC 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# else
#  error This compiler/OS combo is not supported.
# endif

# if defined(_M_AMD64)
#  define ARCH_X64 1
# elif defined(_M_IX86)
#  define ARCH_X86 1
# elif defined(_M_ARM64)
#  define ARCH_ARM64 1
# elif defined(_M_ARM)
#  define ARCH_ARM32 1
# else
#  error Architecture not supported.
# endif

#elif defined(__GNUC__) || defined(__GNUG__)

# define COMPILER_GCC 1

# if defined(__gnu_linux__) || defined(__linux__)
#  define OS_LINUX 1
# else
#  error This compiler/OS combo is not supported.
# endif

# if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#  define ARCH_X64 1
# elif defined(i386) || defined(__i386) || defined(__i386__)
#  define ARCH_X86 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# elif defined(__arm__)
#  define ARCH_ARM32 1
# else
#  error Architecture not supported.
# endif

#else
# error Compiler not supported.
#endif

////////////////////////////////
// ~geb: Arch Cracking

#if (ARCH_X64 || ARCH_ARM64)
# define ARCH_64BIT 1
#elif (ARCH_X86 || ARCH_ARM32)
# define ARCH_32BIT 1
#endif

////////////////////////////////
// ~geb: Zero All Undefined Options

#if !defined(ARCH_32BIT)
# define ARCH_32BIT 0
#endif
#if !defined(ARCH_64BIT)
# define ARCH_64BIT 0
#endif
#if !defined(ARCH_X64)
# define ARCH_X64 0
#endif
#if !defined(ARCH_X86)
# define ARCH_X86 0
#endif
#if !defined(ARCH_ARM64)
# define ARCH_ARM64 0
#endif
#if !defined(ARCH_ARM32)
# define ARCH_ARM32 0
#endif
#if !defined(COMPILER_MSVC)
# define COMPILER_MSVC 0
#endif
#if !defined(COMPILER_GCC)
# define COMPILER_GCC 0
#endif
#if !defined(COMPILER_CLANG)
# define COMPILER_CLANG 0
#endif
#if !defined(OS_WINDOWS)
# define OS_WINDOWS 0
#endif
#if !defined(OS_LINUX)
# define OS_LINUX 0
#endif
#if !defined(OS_MAC)
# define OS_MAC 0
#endif

#if OS_LINUX
# define _GNU_SOURCE 1
#endif

#ifdef __cplusplus
# define IS_CPP 1
# define IS_C   0
#else
# define IS_CPP 0
# define IS_C   1
#endif


#define Kb(n)   (((usize)(n)) << 10)
#define Mb(n)   (((usize)(n)) << 20)
#define Gb(n)   (((usize)(n)) << 30)

#ifndef Static_Assert
# define Static_Assert3(cond, msg) typedef char static_assertion_##msg[(!!(cond))*2-1]
# define Static_Assert2(cond, line) Static_Assert3(cond, static_assertion_at_line_##line)
# define Static_Assert1(cond, line) Static_Assert2(cond, line)
# define Static_Assert(cond)        Static_Assert1(cond, __LINE__)
#endif

#include <stdint.h>
typedef uint8_t   u8;
typedef  int8_t   i8;
typedef uint16_t u16;
typedef  int16_t i16;
typedef uint32_t u32;
typedef  int32_t i32;
typedef uint64_t u64;
typedef  int64_t i64;

#if ARCH_64BIT
typedef u64 usize;
typedef i64 isize;
#elif ARCH_32BIT
typedef u32 usize;
typedef i32 isize;
#endif

typedef float  f32;
typedef double f64;

Static_Assert(sizeof(u8)  == 1);
Static_Assert(sizeof(u16) == 2);
Static_Assert(sizeof(u32) == 4);
Static_Assert(sizeof(u64) == 8);

Static_Assert(sizeof(f32) == 4);
Static_Assert(sizeof(f64) == 8);

Static_Assert(sizeof(u8)  == sizeof(i8));
Static_Assert(sizeof(u16) == sizeof(i16));
Static_Assert(sizeof(u32) == sizeof(i32));
Static_Assert(sizeof(u64) == sizeof(i64));

#ifndef cast // ~geb: This is just for grep
#define cast(Type) (Type)
#endif

typedef u32 rune; // ~geb: Unicode code point
#define Rune_Invalid (cast(rune)0xFFFD)
#define Rune_Max     (cast(rune)0x10FFFF)
#define Rune_BOM     (cast(rune)0xFEFF)

typedef u8 bool;
#define true  (0 == 0)
#define false (1 == 0)

#ifndef U8_MIN
# define U8_MIN 0u
# define U8_MAX 0xffu
# define I8_MIN (-0x7f - 1)
# define I8_MAX 0x7f

# define U16_MIN 0u
# define U16_MAX 0xffffu
# define I16_MIN (-0x7fff - 1)
# define I16_MAX 0x7fff

# define U32_MIN 0u
# define U32_MAX 0xffffffffu
# define I32_MIN (-0x7fffffff - 1)
# define I32_MAX 0x7fffffff

# define U64_MIN 0ull
# define U64_MAX 0xffffffffffffffffull
# define I64_MIN (-0x7fffffffffffffffll - 1)
# define I64_MAX 0x7fffffffffffffffll

# define F32_MIN 1.17549435e-38f
# define F32_MAX 3.40282347e+38f

# define F64_MIN 2.2250738585072014e-308
# define F64_MAX 1.7976931348623157e+308

# if ARCH_32BIT
#  define USIZE_MAX U32_MAX
#  define ISIZE_MAX I32_MAX
# elif ARCH_64BIT
#  define USIZE_MAX U32_MAX
#  define ISIZE_MAX I64_MAX
# endif
#endif

#if COMPILER_CLANG || COMPILER_GCC
# define force_inline inline __attribute__((always_inline))
#elif COMPILER_MSVC
# define force_inline __forceinline
#else
# define force_inline inline
#endif

#if COMPILER_MSVC
# define AlignOf(T) __alignof(T)
#elif COMPILER_CLANG
# define AlignOf(T) __alignof(T)
#elif COMPILER_GCC
# define AlignOf(T) __alignof__(T)
#else
# error AlignOf not defined for this compiler.
#endif

#define AlignPow2(x,b)     (((x) + (b) - 1)&(~((b) - 1)))

#define global        static
#define internal      static 
#define local_persist static 

#define Bit(x) (1u << (x))
#define MaskCheck(flags, mask) (((flags) & (mask)) != 0)
#define MaskSet(var, set, mask) do { \
	if (set) (var) |=  (mask); \
	else     (var) &= ~(mask); \
} while (0)

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Clamp(lower, x, upper) Min(Max((x), (lower)), (upper))
#define Is_Between(lower, x, upper) (((lower) <= (x)) && ((x) <= (upper)))
#define Abs(x) ((x) < 0 ? -(x) : (x))

////////////////////////////////
// ~geb: Mem operations

#include <string.h>
#define MemMove(dst, src, size)   memmove((dst), (src), (size))
#define MemZero(dst, size)        memset((dst), 0x00, (size))
#define MemZeroStruct(dst)        memset((dst), 0x00, (sizeof(*dst)))
#define MemCompare(a, b, size)    memcmp((a), (b), (size))
#define MemStrlen(ptr)            (usize) strlen(ptr)

#if COMPILER_MSVC
# define Trap() __debugbreak()
#elif COMPILER_CLANG || COMPILER_GCC
# define Trap() __builtin_trap()
#else
# error Unknown trap intrinsic for this compiler.
#endif

#include <assert.h>
#define AssertAlways(x) assert(x)
#if !defined(NO_ASSERT)
# define Assert(x) AssertAlways(x)
#else
# define Assert(x) NoOp
#endif

///////////////////////////////////
// ~geb: Allocator

typedef enum AllocationType {
	Allocation_Alloc,
	Allocation_Alloc_Non_Zero,
	Allocation_Free,
	Allocation_FreeAll,
	Allocation_Resize,
	Allocation_Resize_Non_Zero,
} AllocationType;

typedef enum Alloc_Error {
	Alloc_Err_None,
	Alloc_Err_OOM,
	Alloc_Err_Invalid_Pointer,
	Alloc_Err_Invalid_Argument,
	Alloc_Err_Mode_Not_Implemented,
} Alloc_Error;

#define Allocator_Proc(name)                            \
void *name(void *allocator_data, AllocationType type,   \
					 usize size, usize alignment,                 \
					 void *old_memory, usize old_size,            \
					 Alloc_Error *err)
typedef Allocator_Proc(Allocator_Proc);

typedef struct Allocator {
	Allocator_Proc *proc;
	void           *data;
} Allocator;

#ifndef DEFAULT_MEMORY_ALIGNMENT
#define DEFAULT_MEMORY_ALIGNMENT cast(usize)(2 * AlignOf(void *))
#endif

force_inline void *
mem_alloc(Allocator a, usize size, bool zero, Alloc_Error *err)
{
	return a.proc(
		a.data,
		zero ? Allocation_Alloc : Allocation_Alloc_Non_Zero,
		size,
		DEFAULT_MEMORY_ALIGNMENT,
		NULL,
		0,
		err
	);
}

force_inline void *
mem_alloc_aligned(Allocator a, usize size, usize alignment, bool zero, Alloc_Error *err)
{
	return a.proc(
		a.data,
		zero ? Allocation_Alloc : Allocation_Alloc_Non_Zero,
		size,
		alignment,
		NULL,
		0,
		err
	);
}

force_inline void
mem_free(Allocator a, void *memory, Alloc_Error *err) {
	a.proc(
		a.data,
		Allocation_Free,
		0,
		0,
		memory,
		0,
		err
	);
}

force_inline void 
mem_free_all(Allocator a) {
	Alloc_Error err;
	a.proc(
		a.data,
		Allocation_FreeAll,
		0,
		0,
		0,
		0,
		&err
	);
}

#define alloc(a, T, _err) cast(T *) mem_alloc_aligned((a), sizeof(T), AlignOf(T), true, _err)
#define alloc_array(a, T, _count, _err) cast(T *) mem_alloc_aligned((a), sizeof(T) * _count, AlignOf(T), true, _err)

force_inline void *
mem_resize(Allocator a, void *old_mem, usize old_size, usize new_size, bool zero, Alloc_Error *err) 
{
	return a.proc(
		a.data,
		zero ? Allocation_Resize : Allocation_Resize_Non_Zero,
		new_size,
		DEFAULT_MEMORY_ALIGNMENT,
		old_mem,
		old_size,
		err
	);
}

force_inline void *
mem_resize_aligned(Allocator a, void *old_mem, usize old_size, usize new_size, usize alignment, bool zero, Alloc_Error *err) 
{
	return a.proc(
		a.data,
		zero ? Allocation_Resize : Allocation_Resize_Non_Zero,
		new_size,
		alignment,
		old_mem,
		old_size,
		err
	);
}

internal Allocator heap_allocator(void);
internal Allocator arena_allocator(usize reserve);

///////////////////////////////////
// ~geb: String type ( UTF8 )
// for simplicity it is best to use
// arena allocators for strings

typedef struct {
	Allocator alloc;
	usize len;
	u8 *str;
} String8;

typedef struct {
	Allocator alloc;
	String8 *array;
	usize size;
} String8_List;

#define S(x) (String8) { .len = cast(usize) sizeof(x) - 1, .str = cast(u8 *) x }
#define STR "%.*s"
#define s_fmt(s) cast(int) s.len, cast(char *)s.str

internal String8 str8_make(const char *cstring, Allocator allocator);
internal Alloc_Error str8_delete(String8 *str);

internal String8_List str8_make_list(const char **cstrings, usize count, Allocator allocator);
internal Alloc_Error str8_delete_list(String8_List *list);
internal String8     str8_list_index(String8_List *list, usize i);

internal String8 str8_slice(String8 string, usize begin, usize end_exclusive);
internal bool str8_equal(String8 first, String8 second);


///////////////////////////////////
// ~geb: OS layer

// ~geb: heap allocation procs
internal void *os_reserve(usize size);
internal int   os_commit(void *ptr, usize size);
internal void  os_decommit(void *ptr, usize size);
internal void  os_release(void *ptr, usize size);

// ~geb: file handling
typedef union OS_Handle {
	u64 u64[1];
	u32 u32[2];
	u16 u16[4];
} OS_Handle;

typedef u32 OS_AccesFlags;
enum {
  OS_AccessFlag_Read       = Bit(0),
  OS_AccessFlag_Write      = Bit(1),
  OS_AccessFlag_Execute    = Bit(2),
  OS_AccessFlag_ShareRead  = Bit(3),
  OS_AccessFlag_ShareWrite = Bit(4),
};

typedef u32 OS_FileFlags;
enum {
  OS_FileFlag_Directory  = Bit(0),
  OS_FileFlag_ReadOnly   = Bit(1),
  OS_FileFlag_Hidden     = Bit(2),
  OS_FileFlag_System     = Bit(3),
  OS_FileFlag_Archive    = Bit(4),
  OS_FileFlag_Executable = Bit(5),
  OS_FileFlag_Symlink    = Bit(6),
};


typedef u64 OS_Time_Stamp;

typedef struct OS_FileProps {
	usize        size;
	OS_FileFlags flags;
	OS_Time_Stamp created;
	OS_Time_Stamp modified;
} OS_FileProps;

typedef struct OS_Time_Duration {
	f64 seconds;
	f64 milliseconds;
	f64 microseconds;
} OS_Time_Duration;


internal OS_Time_Stamp os_time_now();
internal OS_Time_Stamp os_time_frequency();
internal void os_sleep_ns(u64 ns);

internal OS_Time_Duration os_time_diff(OS_Time_Stamp start, OS_Time_Stamp end);

#endif
