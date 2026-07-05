/**
 * cmylib - Single-header amalgamation of all cmylib modules.
 *
 * Usage:
 *   #define CMYLIB_IMPL  // Enable ALL implementations in one TU
 *   #include "cmylib.h"
 *
 *   // Or enable individual modules:
 *   #define MY_COMMONS_IMPL
 *   #define MY_ALLOCATOR_IMPL
 *   // ...
 *   #include "cmylib.h"
 *
 *   // Make ALL functions static:
 *   #define CMYLIB_DEF static
 *   #include "cmylib.h"
 *
 *   // Or make individual modules static:
 *   #define MY_COMMONS_DEF static
 *   #include "cmylib.h"
 */

#ifndef CMYLIB_DEF
#  define CMYLIB_DEF
#endif


/* ===== my_assert ===== */
#ifndef MY_ASSERT_H_
#define MY_ASSERT_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef assert
# undef assert
#endif

/** @def assert(expr)
 * @brief Assert that an expression is true.
 *
 * If the expression evaluates to false, prints a diagnostic message
 * to stderr and calls `abort()`.
 * @param expr  The expression to test.
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define assert(...) \
	do { \
		if (!(__VA_ARGS__)) { \
			fprintf(stderr, "%s:%d: Assertion Failed: %s\n", __FILE__, __LINE__, #__VA_ARGS__); \
			abort(); \
		} \
	} while (0)
#else
#  define assert(expr_) \
	do { \
		if (!(expr_)) { \
			fprintf(stderr, "%s:%d: Assertion Failed: %s\n", __FILE__, __LINE__, #expr_); \
			abort(); \
		} \
	} while (0)
#endif
#endif /* !MY_ASSERT_H_ */


/* ===== my_commons ===== */
#ifndef MY_COMMONS_H_
#define MY_COMMONS_H_

/* #define MY_COMMONS_IMPL */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#ifndef MY_COMMONS_DEF
#  define MY_COMMONS_DEF CMYLIB_DEF
#endif

/** @def unused(x)
 *  @brief Suppress unused-variable warnings. */
#define unused(__x) ((void)(__x))
/** @def panic(fmt)
 *  @brief Print file:line and formatted message, then exit with code 69. */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define panic(...) _panic(__FILE__, __LINE__, __VA_ARGS__)
#else
#  define panic(msg) _panic(__FILE__, __LINE__, msg)
#endif
/** @def unreachable()
 *  @brief Print file:line and abort — marks code paths that must never execute. */
#define unreachable() _unreachable(__FILE__, __LINE__)

/**
 * @brief Internal panic function (use the `panic` macro instead).
 * @param file  Source file name.
 * @param line  Source line number.
 * @param fmt   Printf-like format string.
 * @param ...   Format arguments.
 */
MY_COMMONS_DEF void _panic(const char *file, int line, const char *  fmt, ...);
/**
 * @brief Internal unreachable handler (use the `unreachable` macro instead).
 */
MY_COMMONS_DEF void _unreachable(const char *file, int line);

/** @def unimplemented()
 *  @brief Print file:line and exit — marks a stub that is not yet implemented. */
#define unimplemented() \
	do { \
		fprintf(stderr, "%s:%d: Error: this is not implemented yet.\n", __FILE__, __LINE__);\
		Break(); \
		exit(1); \
	} while (0)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !MY_COMMONS_H_ */

#if defined(CMYLIB_IMPL) || defined(MY_COMMONS_IMPL)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
MY_COMMONS_DEF void _panic(const char *file, int line, const char *  fmt, ...) {
	fprintf(stderr, "%s:%d: Error: Panic: %s.\n", file, line, fmt);
	exit(69);
}

MY_COMMONS_DEF void _unreachable(const char *file, int line) {
	fprintf(stderr, "%s:%d: Error: reached an unreachable.\n", file, line);
	exit(1);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !CMYLIB_IMPL && !MY_COMMONS_IMPL */


/* ===== my_allocator ===== */
#ifndef MY_ALLOCATOR_H_
#define MY_ALLOCATOR_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <string.h>

#ifndef MY_ALLOCATOR_DEF
#  define MY_ALLOCATOR_DEF CMYLIB_DEF
#endif

typedef struct allocator_interface_t allocator_interface_t;

/**
 * @brief An allocator instance (vtable pointer + opaque data).
 */
typedef struct allocator_t {
	allocator_interface_t *vtable;
	void *data;
} allocator_t;

/**
 * @brief Virtual method table for an allocator.
 */
struct allocator_interface_t {
	/** @brief Allocate memory.
	 *  @param self       Opaque state pointer.
	 *  @param alignment  Required alignment.
	 *  @param size       Number of bytes to allocate.
	 *  @return Pointer to the allocated memory, or NULL on failure. */
	void *(*allocate)(void *self, size_t alignment, size_t size);
	/** @brief Reallocate memory (may move).
	 *  @param self       Opaque state pointer.
	 *  @param old_size   Previous allocation size.
	 *  @param ptr        Previous pointer.
	 *  @param alignment  Required alignment.
	 *  @param new_size   New size in bytes.
	 *  @return Pointer to the resized memory, or NULL on failure. */
	void *(*reallocate)(void *self, size_t old_size, void *ptr, size_t alignment, size_t new_size);
	/** @brief Free memory.
	 *  @param self  Opaque state pointer.
	 *  @param size  Size of the allocation.
	 *  @param ptr   Pointer to free. */
	void (*free)(void *self, size_t size, void *ptr);
};

/** @brief Union for determining maximum alignment. */
union max_align_t_ {
	void *p;
	long l;
	double d;
	long double ld;
};
/** @def MY_DEFAULT_ALIGNMENT
 *  @brief The default alignment used when no specific alignment is requested. */
#define MY_DEFAULT_ALIGNMENT \
	(sizeof(union max_align_t_))

#ifndef CONCAT_RAW
#  define CONCAT_RAW(a, b) a ## b
#endif /* !CONCAT_RAW */
#ifndef CONCAT
#define CONCAT(a, b) CONCAT_RAW(a, b)
#endif /* !CONCAT */

/** @def GET_ALIGNMENT(T)
 *  @brief Get the alignment of a type (C89-compatible). */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define GET_ALIGNMENT(T_) ((void)(CONCAT(struct dih_master_10000_, __LINE__) { char c_; T_ x_; }){0}, offsetof(CONCAT(struct dih_master_10000_, __LINE__), x_))
#else
/* C89: sizeof is always >= alignment, safe as a conservative overestimate */
#  define GET_ALIGNMENT(T_) (sizeof(T_))
#endif

/** @def alloc(allocator, size)
 *  @brief Allocate memory with default alignment. */
#define alloc(allocator_, size_)               allocator_alloc((allocator_), MY_DEFAULT_ALIGNMENT, (size_))
/** @def align_alloc(allocator, align, size)
 *  @brief Allocate memory with a specific alignment. */
#define align_alloc(allocator_, align_, size_) allocator_alloc((allocator_), align_, (size_))

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

/* ---- C99+ variadic macros ---- */

/** @def create(allocator, T, ...)
 *  @brief Allocate and initialise a value of type T. */
#  define create(allocator_, T_, ...) \
	(allocator_alloc_with_value((allocator_), GET_ALIGNMENT(T_), sizeof((T_){ __VA_ARGS__ }), &((T_){ __VA_ARGS__ })))
/** @def destroy(allocator, ptr)
 *  @brief Free memory (size=0 variant). */
#  define destroy(allocator_, ...)               allocator_free((allocator_), 0, (__VA_ARGS__))
/** @def recreate(allocator, new_size, ptr)
 *  @brief Reallocate memory with default alignment (old_size=0). */
#  define recreate(allocator_, new_size_, ...)   allocator_realloc((allocator_), 0, (__VA_ARGS__), MY_DEFAULT_ALIGNMENT, (new_size_))
/** @def xdestroy(allocator, size, ptr)
 *  @brief Free memory with explicit size. */
#  define xdestroy(allocator_, size_, ...)      allocator_free((allocator_), size_, (__VA_ARGS__))
/** @def xrecreate(allocator, old_size, new_size, ptr)
 *  @brief Reallocate memory with explicit old and new sizes. */
#  define xrecreate(allocator_, old_size_, new_size_, ...) allocator_realloc((allocator_), old_size_, (__VA_ARGS__), MY_DEFAULT_ALIGNMENT, (new_size_))
/** @def new(T, ...)
 *  @brief Allocate and construct via the default allocator. */
#  define new(T_, ...)          create((default_allocator_), T_, __VA_ARGS__)
/** @def delete(ptr)
 *  @brief Free memory via the default allocator. */
#  define delete(...)           destroy((default_allocator_), (__VA_ARGS__))
/** @def renew(new_size, ptr)
 *  @brief Reallocate via the default allocator. */
#  define renew(new_size_, ...) recreate((default_allocator_), (new_size_), __VA_ARGS__)

#else

/* ---- C89 fixed-argument macros ---- */

#  define create(allocator_, T_, init_ptr_) \
	allocator_alloc_with_value((allocator_), GET_ALIGNMENT(T_), sizeof(T_), (init_ptr_))
#  define destroy(allocator_, ptr_)           allocator_free((allocator_), 0, (ptr_))
#  define recreate(allocator_, new_size_, ptr_) \
	allocator_realloc((allocator_), 0, (ptr_), MY_DEFAULT_ALIGNMENT, (new_size_))
#  define xdestroy(allocator_, size_, ptr_)   allocator_free((allocator_), size_, (ptr_))
#  define xrecreate(allocator_, old_size_, new_size_, ptr_) \
	allocator_realloc((allocator_), old_size_, (ptr_), MY_DEFAULT_ALIGNMENT, (new_size_))
#  define new(T_, init_ptr_)                  create((default_allocator_), T_, init_ptr_)
#  define delete(ptr_)                        destroy((default_allocator_), (ptr_))
#  define renew(new_size_, ptr_)              recreate((default_allocator_), (new_size_), ptr_)

#endif /* __STDC_VERSION__ >= 199901L */

/** @def make(size)
 *  @brief Allocate raw bytes via the default allocator. */
#define make(size_)   alloc((default_allocator_), (size_))
/** @def aligned_make(align, size)
 *  @brief Allocate aligned raw bytes via the default allocator. */
#define aligned_make(align_, size_)   alloc((default_allocator_), (align_), (size_))

/** @brief The global default allocator (initially zero-initialised). */
extern allocator_t default_allocator_;

/**
 * @brief Set the global default allocator.
 * @param allocator The allocator to use as default.
 */
MY_ALLOCATOR_DEF void set_default_allocator(allocator_t allocator);
/**
 * @brief Get the current global default allocator.
 * @return The default allocator.
 */
MY_ALLOCATOR_DEF allocator_t get_default_allocator(void);

/**
 * @brief Construct an allocator_t from a data pointer and vtable.
 * @param data    Opaque state pointer.
 * @param vtable  Method table.
 * @return A new allocator_t instance.
 */
MY_ALLOCATOR_DEF allocator_t allocator_new(
	void *data,
	allocator_interface_t *vtable);
/**
 * @brief Low-level allocate through an allocator.
 * @param allocator The allocator.
 * @param alignment Required alignment.
 * @param size      Number of bytes.
 * @return Pointer to allocated memory, or NULL.
 */
MY_ALLOCATOR_DEF void *allocator_alloc(
	allocator_t allocator,
	size_t alignment,
	size_t size);
/**
 * @brief Allocate and copy a value into the new memory.
 * @param allocator The allocator.
 * @param alignment Required alignment.
 * @param size      Size of the value.
 * @param value     Pointer to the value to copy.
 * @return Pointer to the allocated copy, or NULL.
 */
MY_ALLOCATOR_DEF void *allocator_alloc_with_value(
	allocator_t allocator,
	size_t alignment,
	size_t size,
	void *value);
/**
 * @brief Reallocate through an allocator.
 * @param allocator The allocator.
 * @param old_size  Previous allocation size.
 * @param ptr       Previous pointer.
 * @param alignment Required alignment.
 * @param new_size  New size.
 * @return Pointer to the resized memory, or NULL.
 */
MY_ALLOCATOR_DEF void *allocator_realloc(
	allocator_t allocator,
	size_t old_size,
	void *ptr,
	size_t alignment,
	size_t new_size);
/**
 * @brief Free memory through an allocator.
 * @param allocator The allocator.
 * @param size      Size of the allocation.
 * @param ptr       Pointer to free.
 */
MY_ALLOCATOR_DEF void allocator_free(allocator_t allocator, size_t size, void *ptr);

/**
 * @brief Clone a block of memory through an allocator.
 * @param allocator The allocator.
 * @param ptr       Source memory.
 * @param len       Number of bytes to copy.
 * @return A new allocated copy.
 */
MY_ALLOCATOR_DEF void *clone_memory(allocator_t allocator, const void *ptr, const size_t len);
/**
 * @brief Clone a null-terminated string through an allocator.
 * @param allocator The allocator.
 * @param str       Source string.
 * @return A new allocated copy of the string.
 */
MY_ALLOCATOR_DEF char *clone_string(allocator_t allocator, const char *str);
/**
 * @brief Clone up to `len` characters of a string through an allocator.
 * @param allocator The allocator.
 * @param str       Source string.
 * @param len       Maximum number of characters to copy.
 * @return A new allocated copy (always null-terminated).
 */
MY_ALLOCATOR_DEF char *nclone_string(allocator_t allocator, const char *str, const size_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !MY_ALLOCATOR_H_ */

#if defined(CMYLIB_IMPL) || defined(MY_ALLOCATOR_IMPL)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

allocator_t default_allocator_ = {0};

MY_ALLOCATOR_DEF void set_default_allocator(allocator_t allocator)
{
	default_allocator_ = allocator;
}

MY_ALLOCATOR_DEF allocator_t get_default_allocator(void)
{
	return default_allocator_;
}

MY_ALLOCATOR_DEF allocator_t allocator_new(
	void *data,
	allocator_interface_t *vtable)
{
	allocator_t result;
	result.data = data;
	result.vtable = vtable;
	return result;
}

MY_ALLOCATOR_DEF void *allocator_alloc_with_value(
	allocator_t allocator,
	size_t alignment,
	size_t size,
	void *value)
{
	void *result = allocator_alloc(allocator, alignment, size);
	if (result == NULL) return NULL;

	memcpy(result, value, size);

	return result;
}

MY_ALLOCATOR_DEF void *allocator_alloc(
	allocator_t allocator,
	size_t alignment,
	size_t size)
{
	assert(allocator.vtable);
	return allocator.vtable->allocate(allocator.data, alignment, size);
}

MY_ALLOCATOR_DEF void *allocator_realloc(
	allocator_t allocator,
	size_t old_size,
	void *ptr,
	size_t alignment,
	size_t new_size)
{
	assert(allocator.vtable);
	return allocator.vtable->reallocate(allocator.data, old_size, ptr, alignment, new_size);
}

MY_ALLOCATOR_DEF void allocator_free(
	allocator_t allocator,
	size_t size,
	void *ptr)
{
	assert(allocator.vtable);
	allocator.vtable->free(allocator.data, size, ptr);
}

MY_ALLOCATOR_DEF void *clone_memory(allocator_t allocator, const void *ptr, const size_t len)
{
	void *result = alloc(allocator, len);
	memcpy(result, ptr, len);
	return result;
}

MY_ALLOCATOR_DEF char *clone_string(allocator_t allocator, const char *str)
{
	const size_t len = strlen(str);
	return nclone_string(allocator, str, len);
}

MY_ALLOCATOR_DEF char *nclone_string(allocator_t allocator, const char *str, const size_t len)
{
	char *result = alloc(allocator, len + 1);
	memcpy(result, str, len);
	result[len] = '\0';
	return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !CMYLIB_IMPL && !MY_ALLOCATOR_IMPL */


/* ===== my_error ===== */
#ifndef MY_ERROR_H_
#define MY_ERROR_H_

/* C89 compat: bool is a C99 keyword */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  include <stdbool.h>
#else
#  if !defined(bool)
#    define bool int
#    define true 1
#    define false 0
#  endif
#endif

/** @def Result(T, E)
 *  @brief Create a Result type holding either an Ok value of type T or an Err value of type E.
 *  @param T  The Ok type.
 *  @param E  The Err type. */
#define Result(T_, E_) struct { bool _is_err; union { T_ _ok; E_ _err; } _u; }

/** @def Ok(result_type, value)
 *  @brief Construct an Ok result.
 *  @param result_type  The Result type (e.g. `Result(int, const char*)`).
 *  @param value        The Ok value. */
#define Ok(R, v)   ((R){ ._is_err = false, ._u._ok = (v) })

/** @def Err(result_type, value)
 *  @brief Construct an Err result.
 *  @param result_type  The Result type.
 *  @param value        The Err value. */
#define Err(R, e)  ((R){ ._is_err = true,  ._u._err = (e) })

/** @def is_ok(result)
 *  @brief Check if a Result is Ok.
 *  @return true if Ok, false if Err. */
#define is_ok(r)   (!(r)._is_err)

/** @def is_err(result)
 *  @brief Check if a Result is Err.
 *  @return true if Err, false if Ok. */
#define is_err(r)  ((r)._is_err)

/** @def unwrap(result)
 *  @brief Unwrap an Ok value. Aborts if the Result is Err.
 *  @return The contained Ok value. */
#define unwrap(r)     (is_err(r) ? unreachable() : (void)0, (r)._u._ok)

/** @def unwrap_err(result)
 *  @brief Unwrap an Err value. Aborts if the Result is Ok.
 *  @return The contained Err value. */
#define unwrap_err(r) (!is_err(r) ? unreachable() : (void)0, (r)._u._err)

/** @def expect(result, msg)
 *  @brief Unwrap an Ok value, printing a message on Err before aborting.
 *  @param r    The Result.
 *  @param msg  Message printed to stderr on Err.
 *  @return The contained Ok value. */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define expect(r, ...) \
	(is_err(r) ? panic(__VA_ARGS__) : (void)0, (r)._u._ok)
#else
#  define expect(r, msg) \
	(is_err(r) ? panic(msg) : (void)0, (r)._u._ok)
#endif

/** @def unwrap_or(result, default)
 *  @brief Unwrap an Ok value, or return a default on Err.
 *  @param r  The Result.
 *  @param d  Default value returned if Err.
 *  @return The Ok value, or the default. */
#define unwrap_or(r, d) (is_err(r) ? (d) : (r)._u._ok)

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/** @def propagate(...)
 *  @brief Return early if the expression is Err (requires `auto` / C23).
 *  @param ...  A Result expression. */
#  define propagate(...) do { auto _ = (__VA_ARGS__); if (_._is_err) return _; } while(0)

/** @def try(name, expr)
 *  @brief Bind the Ok value of a Result, returning early on Err (requires `auto` / C23).
 *  @param name  Variable name for the unwrapped Ok value.
 *  @param ...   A Result expression. */
#  define try(name_, ...) \
	for (int _ok_ = 1, _run_ = 1; _ok_; _ok_ = 0) \
	for (auto _ = (__VA_ARGS__); _run_; _run_ = 0) \
	if (_._is_err) return _; \
	else for (auto name_ = _._u._ok; _ok_; _ok_ = 0)

/** @def catch(name, expr)
 *  @brief Capture the Err value of a Result, skipping Ok (requires `auto` / C23).
 *  @param name  Variable name for the captured Err value.
 *  @param ...   A Result expression. */
#  define catch(name_, ...) \
	for (int _ok_ = 1, _run_ = 1; _ok_; _ok_ = 0) \
	for (auto _ = (__VA_ARGS__); _run_; _run_ = 0) \
	if (_._is_err) \
		for (auto name_ = _._u._err; _ok_; _ok_ = 0)
#endif
#endif /* !MY_ERROR_H_ */


/* ===== my_string ===== */
#ifndef MY_STRING_H_
#define MY_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* C89 compat: bool is a C99 keyword */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  include <stdbool.h>
#else
#  if !defined(bool)
#    define bool int
#    define true 1
#    define false 0
#  endif
#endif
#include <stddef.h>
#include <string.h>

#ifndef MY_STRING_DEF
#  define MY_STRING_DEF CMYLIB_DEF
#endif

/**
 * @brief A non-owning view over a character buffer.
 */
typedef struct string_t {
	const char *data; /**< Pointer to the character data (not null-terminated). */
	size_t len;       /**< Length of the view in bytes. */
} string_t;

/** @def string_ends_with(self, other)
 *  @brief Check if `self` ends with `other`.
 *  @param self   Pointer to a `string_t`.
 *  @param other  A `char`, `const char *`, `char *`, or `string_t *`.
 *  @return `true` if `self` ends with `other`. */
#define string_ends_with(self, other) \
	_Generic((other), \
			char: string_ends_with_char, \
			int: string_ends_with_char, \
			const char *: string_ends_with_cstr, \
			char *: string_ends_with_cstr, \
			string_t *: string_ends_with_string)(self, other)

/** @def string_starts_with(self, other)
 *  @brief Check if `self` starts with `other`.
 *  @param self   Pointer to a `string_t`.
 *  @param other  A `char`, `const char *`, `char *`, or `string_t *`.
 *  @return `true` if `self` starts with `other`. */
#define string_starts_with(self, other) \
	_Generic((other), \
			char: string_starts_with_char, \
			int: string_starts_with_char, \
			const char *: string_starts_with_cstr, \
			char *: string_starts_with_cstr, \
			string_t *: string_starts_with_string)(self, other)

/** @def string_eq(self, target)
 *  @brief Check equality between `self` and `target`.
 *  @param self    Pointer to a `string_t`.
 *  @param target  A `const char *`, `char *`, or `string_t *`.
 *  @return `true` if both strings hold the same content. */
#define string_eq(self, target) \
	_Generic((target), \
			const char *: string_eq_cstr, \
			char *: string_eq_cstr, \
			string_t *: string_eq_string)(self, target)

/** @brief Create a `string_t` view from a null-terminated C string.
 *  @param chs  The source C string (not copied).
 *  @return A `string_t` pointing into `chs` with `len == strlen(chs)`. */
MY_STRING_DEF string_t string_from_chars(const char *chs);

/** @brief Compare a `string_t` with a C string.
 *  @param self   Pointer to a `string_t`.
 *  @param other  Null-terminated C string.
 *  @return `true` if contents are identical. */
MY_STRING_DEF bool string_eq_cstr(const string_t *self, const char *other);

/** @brief Compare two `string_t` values.
 *  @param self   Pointer to the first string.
 *  @param other  Pointer to the second string.
 *  @return `true` if contents are identical. */
MY_STRING_DEF bool string_eq_string(const string_t *self, const string_t *other);

/** @brief Check if `self` starts with a single character.
 *  @param self   Pointer to a `string_t`.
 *  @param other  The character to test.
 *  @return `true` if the first character matches `other`. */
MY_STRING_DEF bool string_starts_with_char(const string_t *self, char other);

/** @brief Check if `self` starts with a C string prefix.
 *  @param self   Pointer to a `string_t`.
 *  @param other  The prefix to test.
 *  @return `true` if `self` begins with `other`. */
MY_STRING_DEF bool string_starts_with_cstr(const string_t *self, const char *other);

/** @brief Check if `self` starts with another `string_t` prefix.
 *  @param self   Pointer to a `string_t`.
 *  @param other  The prefix to test.
 *  @return `true` if `self` begins with `other`. */
MY_STRING_DEF bool string_starts_with_string(const string_t *self, const string_t *other);

/** @brief Check if `self` ends with a single character.
 *  @param self   Pointer to a `string_t`.
 *  @param other  The character to test.
 *  @return `true` if the last character matches `other`. */
MY_STRING_DEF bool string_ends_with_char(const string_t *self, char other);

/** @brief Check if `self` ends with a C string suffix.
 *  @param self   Pointer to a `string_t`.
 *  @param other  The suffix to test.
 *  @return `true` if `self` ends with `other`. */
MY_STRING_DEF bool string_ends_with_cstr(const string_t *self, const char *other);

/** @brief Check if `self` ends with another `string_t` suffix.
 *  @param self   Pointer to a `string_t`.
 *  @param other  The suffix to test.
 *  @return `true` if `self` ends with `other`. */
MY_STRING_DEF bool string_ends_with_string(const string_t *self, const string_t *other);

/** @brief Trim leading and trailing whitespace (space, tab, newline, carriage return) in place.
 *  @param self  Pointer to the string to trim. */
MY_STRING_DEF void string_trim(string_t *self);

/** @brief Trim leading whitespace in place.
 *  @param self  Pointer to the string to trim. */
MY_STRING_DEF void string_ltrim(string_t *self);

/** @brief Trim trailing whitespace in place.
 *  @param self  Pointer to the string to trim. */
MY_STRING_DEF void string_rtrim(string_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !MY_STRING_H_ */

#if defined(CMYLIB_IMPL) || defined(MY_STRING_IMPL)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

MY_STRING_DEF string_t string_from_chars(const char *chs)
{
	string_t result = {0};
	result.data = chs;
	result.len = strlen(chs);
	return result;
}

MY_STRING_DEF bool string_eq_cstr(const string_t *self, const char *other)
{
	const size_t len = strlen(other);
	if (self->len != len) {
		return false;
	}
	return strncmp(self->data, other, len) == 0;
}

MY_STRING_DEF bool string_eq_string(const string_t *self, const string_t *other)
{
	if (self->len != other->len) {
		return false;
	}
	return strncmp(self->data, other->data, self->len) == 0;
}

MY_STRING_DEF bool string_starts_with_char(const string_t *self, char other)
{
	if (self->len < 1) {
		return false;
	}
	return self->data[0] == other;
}

MY_STRING_DEF bool string_starts_with_cstr(const string_t *self, const char *other)
{
	size_t len = strlen(other);
	if (self->len < len) {
		return false;
	}
	return strncmp(self->data, other, len) == 0;
}

MY_STRING_DEF bool string_starts_with_string(const string_t *self, const string_t *other)
{
	if (self->len < other->len) {
		return false;
	}
	return strncmp(self->data, other->data, other->len) == 0;
}

MY_STRING_DEF bool string_ends_with_char(const string_t *self, char other)
{
	if (self->len < 1) {
		return false;
	}
	return self->data[self->len - 1] == other;
}

MY_STRING_DEF bool string_ends_with_cstr(const string_t *self, const char *other)
{
	size_t len = strlen(other);
	if (self->len < len) {
		return false;
	}
	return strncmp(self->data + self->len - len, other, len) == 0;
}

MY_STRING_DEF bool string_ends_with_string(const string_t *self, const string_t *other)
{
	if (self->len < other->len) {
		return false;
	}
	return strncmp(self->data + self->len - other->len, other->data, other->len) == 0;
}

MY_STRING_DEF void string_ltrim(string_t *self)
{
	size_t start = 0;
	while (start < self->len &&
	       (self->data[start] == ' ' || self->data[start] == '\t' ||
	       self->data[start] == '\n' || self->data[start] == '\r')) {
		start++;
	}
	if (start > 0) {
		self->data += start;
		self->len -= start;
	}
}

MY_STRING_DEF void string_rtrim(string_t *self)
{
	while (self->len > 0 &&
	       (self->data[self->len - 1] == ' ' || self->data[self->len - 1] == '\t' ||
	       self->data[self->len - 1] == '\n' || self->data[self->len - 1] == '\r')) {
		self->len--;
	}
}

MY_STRING_DEF void string_trim(string_t *self)
{
	string_rtrim(self);
	string_ltrim(self);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CMYLIB_IMPL && !MY_STRING_IMPL */


/* ===== my_string_view ===== */
#ifndef MY_STRING_VIEW_H_
#define MY_STRING_VIEW_H_

#ifdef __cplusplus
extern "C" {
#endif

/* C89 compat: bool is a C99 keyword */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  include <stdbool.h>
#else
#  if !defined(bool)
#    define bool int
#    define true 1
#    define false 0
#  endif
#endif
#include <stddef.h>
#include <string.h>

#ifndef MY_STRING_VIEW_DEF
#  define MY_STRING_VIEW_DEF CMYLIB_DEF
#endif

/**
 * @brief A non-owning view over an immutable character buffer.
 */
typedef struct string_view_t {
	const char *data;
	size_t      len;
} string_view_t;

/** @def sv_starts_with(self, other)
 *  @brief Check if `self` starts with `other`.
 *  @param self   A `string_view_t`.
 *  @param other  A `char`, `const char *`, or `string_view_t`.
 *  @return `true` if `self` starts with `other`. */
#define sv_starts_with(self, other) \
	_Generic((other), \
			char: sv_starts_with_char, \
			int: sv_starts_with_char, \
			const char *: sv_starts_with_cstr, \
			char *: sv_starts_with_cstr, \
			string_view_t: sv_starts_with_view)(self, other)

/** @def sv_ends_with(self, other)
 *  @brief Check if `self` ends with `other`.
 *  @param self   A `string_view_t`.
 *  @param other  A `char`, `const char *`, or `string_view_t`.
 *  @return `true` if `self` ends with `other`. */
#define sv_ends_with(self, other) \
	_Generic((other), \
			char: sv_ends_with_char, \
			int: sv_ends_with_char, \
			const char *: sv_ends_with_cstr, \
			char *: sv_ends_with_cstr, \
			string_view_t: sv_ends_with_view)(self, other)

/** @def sv_eq(self, other)
 *  @brief Check equality between `self` and `other`.
 *  @param self   A `string_view_t`.
 *  @param other  A `const char *` or `string_view_t`.
 *  @return `true` if both hold the same content. */
#define sv_eq(self, other) \
	_Generic((other), \
			const char *: sv_eq_cstr, \
			char *: sv_eq_cstr, \
			string_view_t: sv_eq_view)(self, other)

/** @brief Create a view from a null-terminated C string.
 *  @param chs  The source C string (not copied).
 *  @return A `string_view_t` pointing into `chs`. */
MY_STRING_VIEW_DEF string_view_t sv_from_chars(const char *chs);

/** @brief Create a view from a data pointer and length.
 *  @param data  Pointer to the character data.
 *  @param len   Number of bytes in the view.
 *  @return A `string_view_t` covering `[data, data + len)`. */
MY_STRING_VIEW_DEF string_view_t sv_from_parts(const char *data, size_t len);

/** @brief Compare with a null-terminated C string.
 *  @param self   A `string_view_t`.
 *  @param other  Null-terminated C string.
 *  @return `true` if contents are identical. */
MY_STRING_VIEW_DEF bool sv_eq_cstr(string_view_t self, const char *other);

/** @brief Compare two views.
 *  @param self   A `string_view_t`.
 *  @param other  Another `string_view_t`.
 *  @return `true` if contents are identical. */
MY_STRING_VIEW_DEF bool sv_eq_view(string_view_t self, string_view_t other);

/** @brief Check if `self` starts with a character.
 *  @param self  A `string_view_t`.
 *  @param c     The character.
 *  @return `true` if the first character matches `c`. */
MY_STRING_VIEW_DEF bool sv_starts_with_char(string_view_t self, char c);

/** @brief Check if `self` starts with a C string prefix.
 *  @param self    A `string_view_t`.
 *  @param prefix  The prefix.
 *  @return `true` if `self` begins with `prefix`. */
MY_STRING_VIEW_DEF bool sv_starts_with_cstr(string_view_t self, const char *prefix);

/** @brief Check if `self` starts with another view.
 *  @param self    A `string_view_t`.
 *  @param prefix  The prefix view.
 *  @return `true` if `self` begins with `prefix`. */
MY_STRING_VIEW_DEF bool sv_starts_with_view(string_view_t self, string_view_t prefix);

/** @brief Check if `self` ends with a character.
 *  @param self  A `string_view_t`.
 *  @param c     The character.
 *  @return `true` if the last character matches `c`. */
MY_STRING_VIEW_DEF bool sv_ends_with_char(string_view_t self, char c);

/** @brief Check if `self` ends with a C string suffix.
 *  @param self    A `string_view_t`.
 *  @param suffix  The suffix.
 *  @return `true` if `self` ends with `suffix`. */
MY_STRING_VIEW_DEF bool sv_ends_with_cstr(string_view_t self, const char *suffix);

/** @brief Check if `self` ends with another view.
 *  @param self    A `string_view_t`.
 *  @param suffix  The suffix view.
 *  @return `true` if `self` ends with `suffix`. */
MY_STRING_VIEW_DEF bool sv_ends_with_view(string_view_t self, string_view_t suffix);

/** @brief Extract a substring view.
 *  @param self   A `string_view_t`.
 *  @param start  Start index (inclusive).
 *  @param end    End index (exclusive).
 *  @return A view over `[start, end)`.  Clamped to bounds. */
MY_STRING_VIEW_DEF string_view_t sv_substr(string_view_t self, size_t start, size_t end);

/** @brief Trim leading and trailing whitespace.
 *  @param self  A `string_view_t`.
 *  @return A new view with whitespace removed from both ends. */
MY_STRING_VIEW_DEF string_view_t sv_trim(string_view_t self);

/** @brief Trim leading whitespace.
 *  @param self  A `string_view_t`.
 *  @return A new view with leading whitespace removed. */
MY_STRING_VIEW_DEF string_view_t sv_ltrim(string_view_t self);

/** @brief Trim trailing whitespace.
 *  @param self  A `string_view_t`.
 *  @return A new view with trailing whitespace removed. */
MY_STRING_VIEW_DEF string_view_t sv_rtrim(string_view_t self);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !MY_STRING_VIEW_H_ */

#if defined(CMYLIB_IMPL) || defined(MY_STRING_VIEW_IMPL)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

MY_STRING_VIEW_DEF string_view_t sv_from_chars(const char *chs)
{
	string_view_t result = {0};
	result.data = chs;
	result.len = strlen(chs);
	return result;
}

MY_STRING_VIEW_DEF string_view_t sv_from_parts(const char *data, size_t len)
{
	string_view_t result = {0};
	result.data = data;
	result.len = len;
	return result;
}

MY_STRING_VIEW_DEF bool sv_eq_cstr(string_view_t self, const char *other)
{
	const size_t len = strlen(other);
	if (self.len != len) {
		return false;
	}
	return strncmp(self.data, other, len) == 0;
}

MY_STRING_VIEW_DEF bool sv_eq_view(string_view_t self, string_view_t other)
{
	if (self.len != other.len) {
		return false;
	}
	return strncmp(self.data, other.data, self.len) == 0;
}

MY_STRING_VIEW_DEF bool sv_starts_with_char(string_view_t self, char c)
{
	if (self.len < 1) {
		return false;
	}
	return self.data[0] == c;
}

MY_STRING_VIEW_DEF bool sv_starts_with_cstr(string_view_t self, const char *prefix)
{
	const size_t len = strlen(prefix);
	if (self.len < len) {
		return false;
	}
	return strncmp(self.data, prefix, len) == 0;
}

MY_STRING_VIEW_DEF bool sv_starts_with_view(string_view_t self, string_view_t prefix)
{
	if (self.len < prefix.len) {
		return false;
	}
	return strncmp(self.data, prefix.data, prefix.len) == 0;
}

MY_STRING_VIEW_DEF bool sv_ends_with_char(string_view_t self, char c)
{
	if (self.len < 1) {
		return false;
	}
	return self.data[self.len - 1] == c;
}

MY_STRING_VIEW_DEF bool sv_ends_with_cstr(string_view_t self, const char *suffix)
{
	const size_t len = strlen(suffix);
	if (self.len < len) {
		return false;
	}
	return strncmp(self.data + self.len - len, suffix, len) == 0;
}

MY_STRING_VIEW_DEF bool sv_ends_with_view(string_view_t self, string_view_t suffix)
{
	if (self.len < suffix.len) {
		return false;
	}
	return strncmp(self.data + self.len - suffix.len, suffix.data, suffix.len) == 0;
}

MY_STRING_VIEW_DEF string_view_t sv_substr(string_view_t self, size_t start, size_t end)
{
	string_view_t result = {0};
	if (start > self.len) start = self.len;
	if (end > self.len) end = self.len;
	if (start >= end) {
		result.data = NULL;
		result.len = 0;
		return result;
	}

	result.data = self.data + start;
	result.len = end - start;
	return result;
}

MY_STRING_VIEW_DEF string_view_t sv_ltrim(string_view_t self)
{
	size_t start = 0;
	string_view_t result;
	while (start < self.len &&
	       (self.data[start] == ' ' || self.data[start] == '\t' ||
	        self.data[start] == '\n' || self.data[start] == '\r')) {
		start++;
	}
	result.data = self.data + start;
	result.len = self.len - start;
	return result;
}

MY_STRING_VIEW_DEF string_view_t sv_rtrim(string_view_t self)
{
	size_t len = self.len;
	string_view_t result = {0};
	while (len > 0 &&
	       (self.data[len - 1] == ' ' || self.data[len - 1] == '\t' ||
	        self.data[len - 1] == '\n' || self.data[len - 1] == '\r')) {
		len--;
	}

	result.data = self.data;
	result.len = len;
	return result;
}

MY_STRING_VIEW_DEF string_view_t sv_trim(string_view_t self)
{
	self = sv_rtrim(self);
	self = sv_ltrim(self);
	return self;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CMYLIB_IMPL && !MY_STRING_VIEW_IMPL */


/* ===== my_stream ===== */
#ifndef MY_STREAM_H_
#define MY_STREAM_H_

/* #define MY_STREAM_IMPL */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef MY_STREAM_DEF
#  define MY_STREAM_DEF CMYLIB_DEF
#endif

/**
 * @brief Virtual method table for a stream.
 */
typedef struct stream_interface_t {
	int (*close)(void *data);
	int (*read)(void *data, unsigned char *out_buf, size_t amount);
	int (*write)(void *data, const unsigned char *in, size_t count);
	int (*seek)(void *data, long int offset, int whence);
	int (*flush)(void *data);
} stream_interface_t;

/**
 * @brief An abstract stream (vtable + opaque data pointer).
 */
typedef struct stream_t {
	void *data;
	const stream_interface_t *vtable;
} stream_t;

/**
 * @brief A memory-backed stream.
 */
typedef struct mem_stream_t {
	char *buffer;
	size_t len;
	size_t pos;
} mem_stream_t;

/**
 * @brief State for parsing format modifiers.
 */
typedef struct modifier_stream_t {
	const char *current;
	size_t len;
} modifier_stream_t;

/**
 * @brief Parsed format modifier info.
 *  Syntax: `{[flags][length/base].[precision]w[width]}`
 */
typedef struct standard_format_info_t {
	int has_len;
	int has_base;
	int has_preci;
	int has_width;

	int len;
	int base;
	int preci;
	int width;

	int alternate_form;
	int zero_padded;
} standard_format_info_t;

/**
 * @brief Parse a modifier string into a `standard_format_info_t`.
 * @param mod  The modifier stream to parse.
 * @param args Varargs for `*`-width specifiers.
 * @return Parsed info struct.
 */
MY_STREAM_DEF standard_format_info_t parse_format_info(modifier_stream_t *mod, va_list args);

/** @brief Signature of a custom format-specifier callback. */
typedef int (*format_fn_t)(stream_t stream, modifier_stream_t mod, va_list args);

/**
 * @brief A single named format specifier.
 */
typedef struct format_specifier_t {
	const char *specifier;
	format_fn_t format;
} format_specifier_t;

/**
 * @brief Registry of named format specifiers.
 */
typedef struct format_specifiers_t {
	size_t len, cap;
	format_specifier_t *items;
} format_specifiers_t;

/** @brief Global format specifier registry. */
extern format_specifiers_t format_specifiers;
/** @brief Standard output stream (initialised by `setup_io_stream`). */
extern stream_t sout;
/** @brief Standard error stream. */
extern stream_t serr;
/** @brief Standard input stream. */
extern stream_t stin;

/**
 * @brief Initialise the built-in I/O streams and register default specifiers.
 *
 * Must be called before using `print`, `println`, `eprint`, `eprintln`,
 * or any `{specifier}` in format strings.
 */
MY_STREAM_DEF void setup_io_stream(void);
/**
 * @brief Look up a format specifier by name.
 * @param name  The specifier name.
 * @param out   Receives the matching specifier, if found.
 * @return 1 if found, 0 otherwise.
 */
MY_STREAM_DEF int find_format_specifier(const char *name, format_specifier_t *out);
/**
 * @brief Register a custom format specifier.
 * @param name     The specifier name (must not contain `%{} \n\r\t\v`).
 * @param callback The formatting function.
 * @return 1 on success, 0 if the name is already registered.
 */
MY_STREAM_DEF int define_format_specifier(const char *name, format_fn_t callback);

/** @brief Print to stdout (requires `setup_io_stream()` first). */
MY_STREAM_DEF int print(const char *  fmt, ...);
/** @brief Print to stdout followed by a newline. */
MY_STREAM_DEF int println(const char *  fmt, ...);
/** @brief Print to stderr. */
MY_STREAM_DEF int eprint(const char *  fmt, ...);
/** @brief Print to stderr followed by a newline. */
MY_STREAM_DEF int eprintln(const char *  fmt, ...);
/** @brief Print to an arbitrary stream. */
MY_STREAM_DEF int sprint(stream_t stream, const char *  fmt, ...);
/** @brief Print to an arbitrary stream followed by a newline. */
MY_STREAM_DEF int sprintln(stream_t stream, const char *  fmt, ...);

/**
 * @brief Print into a fixed-size buffer (safe snprintf-style).
 * @param buf  Destination buffer.
 * @param n    Buffer size.
 * @param fmt  Format string.
 * @param ...  Arguments.
 * @return Number of characters written (not including null).
 */
MY_STREAM_DEF int snsprint(char *buf, size_t n, const char *  fmt, ...);
/* int snsprintln(char *buf, size_t n, const char *  fmt, ...); */

/** @brief Write a character to a stream. */
MY_STREAM_DEF int sputc(stream_t stream, int ch);
/** @brief Write a string to a stream. */
MY_STREAM_DEF int sputs(stream_t stream, const char *  s);
/** @brief Write a string of known length to a stream. */
MY_STREAM_DEF int snputs(stream_t stream, const int len, const char *  s);

/** @brief Read from a stream. */
MY_STREAM_DEF int sread(stream_t stream, unsigned char *out, size_t size, size_t n);
/** @brief Write to a stream. */
MY_STREAM_DEF int swrite(stream_t stream, const unsigned char *in, size_t size, size_t n);
/** @brief Seek on a stream. */
MY_STREAM_DEF int sseek(stream_t stream, long int offset, int whence);
/** @brief Flush a stream. */
MY_STREAM_DEF int sflush(stream_t stream);

/**
 * @brief Open a memory-backed stream.
 * @param buffer  The buffer to read from / write to.
 * @param len     Buffer size.
 * @return A stream_t for the memory buffer.
 */
MY_STREAM_DEF stream_t smemopen(char *buffer, size_t len);
/**
 * @brief Open a file as a stream.
 * @param path  File path.
 * @param mode  fopen-style mode string.
 * @return A stream_t backed by the file, or a zeroed stream on failure.
 */
MY_STREAM_DEF stream_t sopen(const char *path, const char *mode);
/**
 * @brief Close a stream.
 * @param stream  The stream to close.
 * @return 0 on success, EOF on error.
 */
MY_STREAM_DEF int sclose(stream_t stream);

/** @brief Varargs version of `print`. */
MY_STREAM_DEF int vprint(const char *  fmt, va_list args);
/** @brief Varargs version of `sprint`. */
MY_STREAM_DEF int vsprint(stream_t stream, const char *  fmt, va_list args);
/** @brief Varargs version of `snsprint`. */
MY_STREAM_DEF int vsnsprint(char *buf, size_t n, const char *  fmt, va_list args);

/** @brief Check if a modifier stream has remaining characters. */
MY_STREAM_DEF int has_modifier(const modifier_stream_t *mod);
/** @brief Peek at the next modifier character without consuming. */
MY_STREAM_DEF char peek_modifier(const modifier_stream_t *mod);
/** @brief Consume and return the next modifier character. */
MY_STREAM_DEF char advance_modifier(modifier_stream_t *mod);
/** @brief Check if the next modifier character equals `ch` without consuming. */
MY_STREAM_DEF int check_modifier(const modifier_stream_t *mod, const char ch);
/** @brief If the next modifier character equals `ch`, consume it and return 1. */
MY_STREAM_DEF int match_modifier(modifier_stream_t *mod, const char ch);
#endif /* !MY_STREAM_H_ */


#if defined(CMYLIB_IMPL) || defined(MY_STREAM_IMPL)

/* C89 compat: snprintf is a C99 function */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
static int local_snprintf(char *buf, size_t sz, const char *fmt, ...) {
    int n;
    va_list args;
    va_start(args, fmt);
    n = vsprintf(buf, fmt, args);
    va_end(args);
    (void)sz;
    return n;
}
#  define snprintf local_snprintf
#endif

/* --- file stream implementation --- */
static int fs_close(void *data);
static int fs_read(void *data, unsigned char *out_buf, size_t amount);
static int fs_write(void *data, const unsigned char *, size_t);
static int fs_seek(void *data, long int offset, int whence);
static int fs_flush(void *data);

/* --- memory stream implementation --- */
static int mem_close(void *data);
static int mem_read(void *data, unsigned char *out_buf, size_t amount);
static int mem_write_altr(void *data, const unsigned char *in, size_t m);
static int mem_write(void *data, const unsigned char *, size_t);
static int mem_seek(void *data, long int offset, int whence);
static int mem_flush(void *data);

static const stream_interface_t file_vtable_ = {
	fs_close,
	fs_read,
	fs_write,
	fs_seek,
	fs_flush,
};

static const stream_interface_t mem_altr_vtable_ = {
	mem_close,
	mem_read,
	mem_write_altr,
	mem_seek,
	mem_flush,
};

static const stream_interface_t mem_vtable_ = {
	mem_close,
	mem_read,
	mem_write,
	mem_seek,
	mem_flush,
};

format_specifiers_t format_specifiers = {0};
stream_t sout = {0};
stream_t serr = {0};
stream_t stin = {0};

#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#    define file_into_stream(...) (stream_t){ .data = (__VA_ARGS__), .vtable = &file_vtable_ }
#  else
static stream_t file_into_stream_c89(void *data, const stream_interface_t *vtable) {
    stream_t s;
    s.data = data;
    s.vtable = vtable;
    return s;
}
#    define file_into_stream(fp) file_into_stream_c89((fp), &file_vtable_)
#  endif

/** @brief Write a string with optional width-based padding. */
static int swrite_width(
	stream_t stream,
	const unsigned char *in,
	const int len,
	const int width,
	const char *const padding_char)
{
	int i;
	int printed_amount = 0;
	for (i=width - len; 0 < i; i -= 1) {
		printed_amount += swrite(stream, (void*)padding_char, sizeof(char), 1);
	}

	printed_amount += swrite(stream, (void*)in, sizeof(char), len);

	for (i=width + len; 0 > i; i += 1) {
		printed_amount += swrite(stream, (void*)padding_char, sizeof(char), 1);
	}

	return printed_amount;
}

MY_STREAM_DEF standard_format_info_t parse_format_info(modifier_stream_t *mod, va_list args)
{
	standard_format_info_t result = {0};
	if (!has_modifier(mod)) return result;
	if (match_modifier(mod, '#')) result.alternate_form = 1;
	if (match_modifier(mod, '0')) result.zero_padded = 1;
	if (match_modifier(mod, '*')) {
		result.has_len = 1;
		result.len = va_arg(args, int);
		result.has_base = 1;
		result.base = result.len;
	} else {
		char *endptr = NULL;
		long m = strtol(mod->current, &endptr, 10);

		if (mod->current != endptr) {
			result.has_len = 1;
			result.len = (int)m;
			result.has_base = 1;
			result.base = result.len;
			mod->current = endptr;
		}
	}
	if (match_modifier(mod, '.')) {
		if (match_modifier(mod, '*')) {
			result.has_preci = 1;
			result.preci = result.width;
		} else {
			char *endptr = NULL;
			const long m = strtol(mod->current, &endptr, 10);

			if (mod->current != endptr) {
				mod->current = endptr;
				result.has_preci = 1;
				result.preci = (int)m;
			}
		}
	}
	if (match_modifier(mod, 'w')) {
		if (match_modifier(mod, '*')) {
			result.has_width = 1;
			result.width = va_arg(args, int);
		} else {
			char *endptr = NULL;
			const long m = strtol(mod->current, &endptr, 10);
			result.has_width = 1;
			result.width = (int)m;
		}
	}

	return result;
}

/** @brief Format a single character (`c`). */
static int format_char(stream_t stream, modifier_stream_t mod, va_list args)
{
	char ch;
	(void)mod;
	ch = (char)va_arg(args, int);
	return swrite(stream, (void*)&ch, sizeof(ch), 1);
}

/** @brief Format a string / C-string (`s`). */
static int format_string(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const char *str;
	standard_format_info_t fmt_info;
	int width;
	int input_len;
	const char *padding_char;

	str = va_arg(args, const char *);
	if (str == NULL) str = "(nil)";

	fmt_info = parse_format_info(&mod, args);
	width = fmt_info.width;
	input_len = fmt_info.len ? fmt_info.len : (int)strlen(str);
	padding_char = fmt_info.zero_padded ? "0" : " ";
	if (fmt_info.alternate_form) {
		int i;
		int len = 0;
		int printed_amount = 0;
		len += 1;
		for (i=0; i < input_len; i += 1) {
			const char ch = str[i];
			if (ch == '"'
				|| ch == '\a'
				|| ch == '\b'
				|| ch == '\f'
				|| ch == '\n'
				|| ch == '\r'
				|| ch == '\t'
				|| ch == '\v'
				|| ch == '\\'
				|| ch == '\0') {
				len += 1;
			}
			len += 1;
		}
		len += 1;

		for (i=width - len; 0 < i; i -= 1) {
			printed_amount += swrite(stream, (void*)padding_char, sizeof(char), 1);
		}

		printed_amount += swrite(stream, (void*)"\"", sizeof(char), 1);
		for (i=0; i<input_len; i += 1) {
			const char ch = str[i];
			switch (ch) {
			case '"':
				printed_amount += swrite(stream, (void*)"\\\"", sizeof(char), 2);
				break;
			case '\'':
				printed_amount += swrite(stream, (void*)"\\'", sizeof(char), 2);
				break;
			case '\a':
				printed_amount += swrite(stream, (void*)"\\a", sizeof(char), 2);
				break;
			case '\b':
				printed_amount += swrite(stream, (void*)"\\b", sizeof(char), 2);
				break;
			case '\f':
				printed_amount += swrite(stream, (void*)"\\f", sizeof(char), 2);
				break;
			case '\n':
				printed_amount += swrite(stream, (void*)"\\n", sizeof(char), 2);
				break;
			case '\r':
				printed_amount += swrite(stream, (void*)"\\r", sizeof(char), 2);
				break;
			case '\t':
				printed_amount += swrite(stream, (void*)"\\t", sizeof(char), 2);
				break;
			case '\v':
				printed_amount += swrite(stream, (void*)"\\v", sizeof(char), 2);
				break;
			case '\\':
				printed_amount += swrite(stream, (void*)"\\\\", sizeof(char), 2);
				break;
			case '\0':
				printed_amount += swrite(stream, (void*)"\\0", sizeof(char), 2);
				break;
			default:
				printed_amount += swrite(stream, (void*)&ch, sizeof(char), 1);
				break;
			}
		}
		printed_amount += swrite(stream, (void*)"\"", sizeof(char), 1);

		for (i=width + len; 0 > i; i += 1) {
			printed_amount += swrite(stream, (void*)padding_char, sizeof(char), 1);
		}

		return printed_amount;
	}

	return swrite_width(stream, (void*)str, input_len, width, padding_char);
}

/** @brief Convert a signed long long to a string in the given base. */
static char* ll_to_base_str(long long val, char* buf, int alternate, int base)
{
	int upper = 0;
	char *digits;
	int i = 0;
	int is_negative = 0;
	unsigned long long u_val;
	int start;
	int end;

	if (base < 0) {
		upper = 1;
		base *= -1;
	}

	if (base < 2 || base > 36) {
		buf[0] = '\0';
		return buf;
	}

	digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (upper) {
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	}
	u_val = val;

	if (val == 0) {
		buf[i++] = '0';
		buf[i] = '\0';
		return buf;
	}

	if (val < 0) {
		is_negative = 1;
		u_val = (unsigned long long)(-val);
	} else {
		u_val = (unsigned long long)val;
	}

	while (u_val > 0) {
		buf[i++] = digits[u_val % base];
		u_val /= base;
	}

	if (is_negative) {
		buf[i++] = '-';
	}

	if (alternate) {
		if (base == 2) {
			buf[i++] = upper ? 'B' : 'b';
			buf[i++] = '0';
		} else if (base == 16) {
			buf[i++] = upper ? 'X' : 'x';
			buf[i++] = '0';
		} else if (base == 8) {
			buf[i++] = '0';
		}
	}

	buf[i] = '\0';

	start = 0;
	end = i - 1;
	while (start < end) {
		char temp = buf[start];
		buf[start] = buf[end];
		buf[end] = temp;
		start++;
		end--;
	}

	return buf;
}

/** @brief Convert an unsigned long long to a string in the given base. */
static char* ull_to_base_str(unsigned long long val, char* buf, int alternate, int base)
{
	int upper = 0;
	char *digits;
	int i = 0;
	unsigned long long u_val;
	int start;
	int end;

	if (base < 0) {
		upper = 1;
		base *= -1;
	}

	if (base < 2 || base > 36) {
		buf[0] = '\0';
		return buf;
	}

	digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (upper) {
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	}
	u_val = val;

	if (val == 0) {
		buf[i++] = '0';
		buf[i] = '\0';
		return buf;
	}

	while (u_val > 0) {
		buf[i++] = digits[u_val % base];
		u_val /= base;
	}

	if (alternate) {
		if (base == 2) {
			buf[i++] = upper ? 'B' : 'b';
			buf[i++] = '0';
		} else if (base == 16) {
			buf[i++] = upper ? 'X' : 'x';
			buf[i++] = '0';
		} else if (base == 8) {
			buf[i++] = '0';
		}
	}

	buf[i] = '\0';

	start = 0;
	end = i - 1;
	while (start < end) {
		char temp = buf[start];
		buf[start] = buf[end];
		buf[end] = temp;
		start++;
		end--;
	}

	return buf;
}

/** @brief Convert a size_t to a string in the given base. */
static char* size_to_base_str(size_t val, char* buf, int alternate, int base)
{
	int upper = 0;
	char *digits;
	int i = 0;
	size_t u_val;
	int start;
	int end;

	if (base < 0) {
		upper = 1;
		base *= -1;
	}

	if (base < 2 || base > 36) {
		buf[0] = '\0';
		return buf;
	}

	digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (upper) {
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	}
	u_val = val;

	if (val == 0) {
		buf[i++] = '0';
		buf[i] = '\0';
		return buf;
	}

	while (u_val > 0) {
		buf[i++] = digits[u_val % base];
		u_val /= base;
	}

	if (alternate) {
		if (base == 2) {
			buf[i++] = upper ? 'B' : 'b';
			buf[i++] = '0';
		} else if (base == 16) {
			buf[i++] = upper ? 'X' : 'x';
			buf[i++] = '0';
		} else if (base == 8) {
			buf[i++] = '0';
		}
	}

	buf[i] = '\0';

	start = 0;
	end = i - 1;
	while (start < end) {
		char temp = buf[start];
		buf[start] = buf[end];
		buf[end] = temp;
		start++;
		end--;
	}

	return buf;
}

#if defined(CMYLIB_IMPL) || defined(MY_STREAM_IMPL)
/** @brief Convert a pointer to a hex string representation. */
#if 0
static char* ptr_to_base_str(void *val, char* buf, int alternate, int base)
{
	int upper = 0;
	char *digits;
	int i = 0;
	uintptr_t u_val;
	int start;
	int end;

	if (base < 0) {
		upper = 1;
		base *= -1;
	}

	if (base < 2 || base > 36) {
		buf[0] = '\0';
		return buf;
	}

	digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (upper) {
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	}
	u_val = (uintptr_t)val;

	if (val == 0) {
		buf[i++] = '0';
		buf[i] = '\0';
		return buf;
	}

	while (u_val > 0) {
		buf[i++] = digits[u_val % base];
		u_val /= base;
	}

	if (alternate) {
		if (base == 2) {
			buf[i++] = upper ? 'B' : 'b';
			buf[i++] = '0';
		} else if (base == 16) {
			buf[i++] = upper ? 'X' : 'x';
			buf[i++] = '0';
		} else if (base == 8) {
			buf[i++] = '0';
		}
	}

	buf[i] = '\0';

	start = 0;
	end = i - 1;
	while (start < end) {
		char temp = buf[start];
		buf[start] = buf[end];
		buf[end] = temp;
		start++;
		end--;
	}

	return buf;
}
#endif
#endif

/* --- typed format helpers for integers --- */
static int format_int8(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const int8_t integer = va_arg(args, int);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ll_to_base_str((long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_int16(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const int16_t integer = va_arg(args, int);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ll_to_base_str((long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_int32(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const int32_t integer = va_arg(args, int32_t);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ll_to_base_str((long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_int64(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const int64_t integer = va_arg(args, int64_t);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ll_to_base_str((long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_uint8(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const uint8_t integer = va_arg(args, unsigned int);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ll_to_base_str((long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_uint16(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const uint16_t integer = va_arg(args, unsigned int);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ll_to_base_str((long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_uint32(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const uint32_t integer = va_arg(args, uint32_t);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ll_to_base_str((long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_uint64(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const uint64_t integer = va_arg(args, uint64_t);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ll_to_base_str((long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_int(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const int integer = va_arg(args, int);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ll_to_base_str((long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_l_int(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const long int integer = va_arg(args, long int);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ll_to_base_str((long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_ll_int(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const long long int integer = va_arg(args, long long int);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ll_to_base_str((long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_uint(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const unsigned int integer = va_arg(args, unsigned int);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ull_to_base_str((unsigned long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_l_uint(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const unsigned long int integer = va_arg(args, unsigned long int);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ull_to_base_str((unsigned long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_ll_uint(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const long long unsigned int integer = va_arg(args, long long unsigned int);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ull_to_base_str((unsigned long long)integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_size_t(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const size_t integer = va_arg(args, size_t);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const int base = info.base ? info.base : 10;
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	size_to_base_str(integer, buffer, info.alternate_form, base);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_ptr(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const void *ptr = va_arg(args, void*);
	const standard_format_info_t info = parse_format_info(&mod, args);
	char buffer[512] = {0};
	const char *const padding_char = info.zero_padded ? "0" : " ";
	ull_to_base_str((unsigned long long)ptr, buffer, 1, 16);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_float(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const double f = va_arg(args, double);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const char *const padding_char = info.zero_padded ? "0" : " ";
	char buffer[30] = {0};
	snprintf(buffer, sizeof(buffer), "%f", f);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_double(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const double f = va_arg(args, double);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const char *const padding_char = info.zero_padded ? "0" : " ";
	char buffer[30] = {0};
	snprintf(buffer, sizeof(buffer), "%lf", f);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

static int format_long_double(
	stream_t stream,
	modifier_stream_t mod,
	va_list args)
{
	const long double f = va_arg(args, long double);
	const standard_format_info_t info = parse_format_info(&mod, args);
	const char *const padding_char = info.zero_padded ? "0" : " ";
	char buffer[30] = {0};
	snprintf(buffer, sizeof(buffer), "%Lf", f);
	return swrite_width(stream, (void*)buffer, strlen(buffer), info.width, padding_char);
}

MY_STREAM_DEF void setup_io_stream(void)
{
	sout.data = stdout;
	sout.vtable = &file_vtable_;
	serr.data = stderr;
	serr.vtable = &file_vtable_;
	stin.data = stdin;
	stin.vtable = &file_vtable_;

	define_format_specifier("c",   format_char);
	define_format_specifier("s",   format_string);

	define_format_specifier("int", format_int);

	define_format_specifier("f", format_float);
	define_format_specifier("lf", format_double);
	define_format_specifier("Lf", format_long_double);

	define_format_specifier("i8", format_int8);
	define_format_specifier("i16", format_int16);
	define_format_specifier("i32", format_int32);
	define_format_specifier("i64", format_int64);

	define_format_specifier("int8", format_int8);
	define_format_specifier("int16", format_int16);
	define_format_specifier("int32", format_int32);
	define_format_specifier("int64", format_int64);

	define_format_specifier("u8", format_uint8);
	define_format_specifier("u16", format_uint16);
	define_format_specifier("u32", format_uint32);
	define_format_specifier("u64", format_uint64);

	define_format_specifier("unt8", format_uint8);
	define_format_specifier("unt16", format_uint16);
	define_format_specifier("unt32", format_uint32);
	define_format_specifier("unt64", format_uint64);

	define_format_specifier("i",   format_int);
	define_format_specifier("li",  format_l_int);
	define_format_specifier("lli", format_ll_int);

	define_format_specifier("u",   format_uint);
	define_format_specifier("lu",  format_l_uint);
	define_format_specifier("llu", format_ll_uint);

	define_format_specifier("d",   format_int);
	define_format_specifier("ld",  format_l_int);
	define_format_specifier("lld", format_ll_int);

	define_format_specifier("z", format_size_t);
	define_format_specifier("usize", format_size_t);

	define_format_specifier("iz", format_size_t);
	define_format_specifier("isize", format_size_t);

	define_format_specifier("p", format_ptr);
	define_format_specifier("ptr", format_ptr);
}

#if 0
static int is_valid_specifier_name(const char *name)
{
	const char *current;
	const char *ch;
	const char *const banned_characters = "%{} \n\r\t\v\0";
	for (current = name; *current; current += 1) {
		for (ch = banned_characters; *ch; ch += 1) {
			if (*current == *ch) return 0;
		}
	}
	return 1;
}
#endif

static int nfind_format_specifier(const char *name, const size_t n, format_specifier_t *out)
{
	size_t i;
	if (n == 0) return 0;
	for (i=0; i < format_specifiers.len; i++) {
		if (strncmp(name, format_specifiers.items[i].specifier, n) == 0) {
			if (out != NULL) {
				*out = format_specifiers.items[i];
			}
			return 1;
		}
	}
	return 0;
}

MY_STREAM_DEF int find_format_specifier(const char *name, format_specifier_t *out)
{
	size_t i;
	for (i=0; i < format_specifiers.len; i++) {
		if (strcmp(name, format_specifiers.items[i].specifier) == 0) {
			if (out != NULL) {
				*out = format_specifiers.items[i];
			}
			return 1;
		}
	}
	return 0;
}

MY_STREAM_DEF int define_format_specifier(const char *name, format_fn_t callback)
{
	if (find_format_specifier(name, NULL)) {
		return 0;
	}
	if (format_specifiers.len >= format_specifiers.cap) {
		const size_t new_cap__ = format_specifiers.cap == 0 ? 10 : format_specifiers.cap * 2;
		format_specifiers.items = realloc(format_specifiers.items , (new_cap__ * sizeof(format_specifier_t)));
		format_specifiers.cap = new_cap__ ;
	}
	format_specifiers.items[format_specifiers.len].specifier = name;
	format_specifiers.items[format_specifiers.len].format = callback;
	format_specifiers.len += 1;
	return 1;
}

MY_STREAM_DEF int print(const char *  fmt, ...)
{
	va_list args;
	int result;
	va_start(args, fmt);
	result = vprint(fmt, args);
	va_end(args);
	return result;
}

MY_STREAM_DEF int println(const char *  fmt, ...)
{
	va_list args;
	int result;
	va_start(args, fmt);
	result = vprint(fmt, args);
	va_end(args);
	result += print("\n");
	return result;
}

MY_STREAM_DEF int eprint(const char *  fmt, ...)
{
	va_list args;
	int result;
	va_start(args, fmt);
	result = vsprint(serr, fmt, args);
	va_end(args);
	return result;
}

MY_STREAM_DEF int eprintln(const char *  fmt, ...)
{
	va_list args;
	int result;
	va_start(args, fmt);
	result = vsprint(serr, fmt, args);
	va_end(args);
	result += sprint(serr, "\n");
	return result;
}

MY_STREAM_DEF int sprint(stream_t stream, const char *  fmt, ...)
{
	va_list args;
	int result;
	va_start(args, fmt);
	result = vsprint(stream, fmt, args);
	va_end(args);
	return result;
}

MY_STREAM_DEF int sprintln(stream_t stream, const char *  fmt, ...)
{
	va_list args;
	int result;
	va_start(args, fmt);
	result = vsprint(stream, fmt, args);
	va_end(args);
	result += sprint(stream, "\n");
	return result;
}

MY_STREAM_DEF int snsprint(char *buf, size_t n, const char *  fmt, ...)
{
	va_list args;
	int amount;
	va_start(args, fmt);
	amount = vsnsprint(buf, n, fmt, args);
	va_end(args);
	return amount;
}

/* int snsprintln(char *buf, size_t n, const char *  fmt, ...) */
/* { */
/* 	va_list args; va_start(args, fmt); */
/* 	int amount = vsnsprint(buf, n, fmt, args); */
/* 	va_end(args); */
/* 	amount += println(""); */
/* 	return amount; */
/* } */

MY_STREAM_DEF int vsnsprint(char *buf, size_t n, const char *  fmt, va_list args)
{
	mem_stream_t data;
	stream_t strm;
	int amount;
	data.buffer = buf;
	data.len = n;
	data.pos = 0;
	strm.data = &data;
	strm.vtable = &mem_altr_vtable_;
	amount = vsprint(strm, fmt, args);
	amount += sflush(strm);
	return amount;
}

MY_STREAM_DEF int sputc(stream_t stream, int ch)
{
	unsigned char c;
	unsigned char chars[2];
	int count;
	c = (unsigned char)ch;
	chars[0] = c;
	chars[1] = '\0';
	count = swrite(stream, chars, 1, 1);
	if (count != 1) return EOF;

	return (int)c;
}

MY_STREAM_DEF int sputs(stream_t stream, const char *  s)
{
	const size_t len = strlen(s);
	int count = swrite(stream, (const unsigned char*)s, sizeof(char), len);
	if ((size_t)count != len) return EOF;
	return (int)len;
}

MY_STREAM_DEF int snputs(stream_t stream, const int len, const char *  s)
{
	int count = swrite(stream, (const unsigned char*)s, sizeof(char), len);
	if (count != len) return EOF;
	return (int)len;
}

MY_STREAM_DEF int sread(stream_t stream, unsigned char *out, size_t size, size_t n)
{
	return stream.vtable->read(stream.data, out, size * n);
}

MY_STREAM_DEF int swrite(stream_t stream, const unsigned char *in, size_t size, size_t n)
{
	return stream.vtable->write(stream.data, in, size * n);
}

MY_STREAM_DEF int sseek(stream_t stream, long int offset, int whence)
{
	return stream.vtable->seek(stream.data, offset, whence);
}

MY_STREAM_DEF int sflush(stream_t stream)
{
	return stream.vtable->flush(stream.data);
}

MY_STREAM_DEF stream_t smemopen(char *buffer, size_t len)
{
	stream_t result;
	mem_stream_t *data;
	data = calloc(sizeof(mem_stream_t), 1);
	result.data = data;
	result.vtable = &mem_vtable_;
	data->buffer = buffer;
	data->len = len;
	data->pos = 0;
	return result;
}

MY_STREAM_DEF stream_t sopen(const char *path, const char *mode)
{
	FILE *f = fopen(path, mode);
	return file_into_stream(f);
}

MY_STREAM_DEF int sclose(stream_t stream)
{
	sflush(stream);
	if (stream.vtable != &file_vtable_) {
		free(stream.data);
	}
	return stream.vtable->close(stream.data);
}

MY_STREAM_DEF int vprint(const char *  fmt, va_list args)
{
	return vsprint(sout, fmt, args);
}

MY_STREAM_DEF int vsprint(stream_t stream, const char *  fmt, va_list args)
{
	int printed_amount = 0;
	const char *current;

	const char *printing_span = NULL;
	size_t printing_span_len = 0;

	const char *specifier = NULL;
	size_t specifier_len = 0;

	const char *modifier = NULL;
	size_t modifier_len = 0;

	for (current = fmt; *current; current += 1) {
		if (*current == '{') {
			if (printing_span != NULL) {
				printed_amount += swrite(stream, (const unsigned char *)printing_span, sizeof(char), printing_span_len);
			}
			printing_span = NULL;
			printing_span_len = 0;

			current += 1;
			if (*current == '\0') {
				printed_amount += swrite(stream, (void*)"{", sizeof(char), 1);
				break;
			}
			if (*current == '{') {
				printed_amount += swrite(stream, (void*)"{", sizeof(char), 1);
				continue;
			}

			for (specifier = current; *current && *current != ':' && *current != '}'; current += 1) {
				specifier_len += 1;
			}

			if (*current && *current == ':' && current[1] != '}') {
				current += 1;
				for (modifier = current; *current && *current != '}'; current += 1) {
					modifier_len += 1;
				}
			}

			{
			format_specifier_t sp = {0};
			modifier_stream_t mod_st;
			if (nfind_format_specifier(specifier, specifier_len, &sp)) {
				mod_st.current = modifier;
				mod_st.len = modifier_len;
				printed_amount += sp.format(stream, mod_st, args);
			} else {
				current -= specifier_len;
				current -= modifier_len;
				current -= 1;
				printed_amount += sputc(stream, '{');
			}
		}

		specifier = NULL;
		specifier_len = 0;
		modifier = NULL;
		modifier_len = 0;
		continue;
		}

		if (printing_span_len == 0) {
			printing_span = current;
		}
		printing_span_len += 1;
	}

	if (printing_span != NULL && printing_span_len > 0) {
		printed_amount += swrite(stream, (const unsigned char *)printing_span, sizeof(char), printing_span_len);
	}

	return printed_amount;
}

MY_STREAM_DEF int has_modifier(const modifier_stream_t *mod)
{
	if (mod->current == NULL) return 0;
	return mod->len != 0;
}

MY_STREAM_DEF char peek_modifier(const modifier_stream_t *mod)
{
	if (!has_modifier(mod)) {
		return 0x0;
	}

	return mod->current[0];
}

MY_STREAM_DEF char advance_modifier(modifier_stream_t *mod)
{
	char prev;
	if (!has_modifier(mod)) return 0x0;
	prev = peek_modifier(mod);

	mod->current += 1;
	mod->len -= 1;

	return prev;
}

MY_STREAM_DEF int check_modifier(const modifier_stream_t *mod, const char ch)
{
	if (has_modifier(mod)) {
		return peek_modifier(mod) == ch;
	}
	return 0;
}

MY_STREAM_DEF int match_modifier(modifier_stream_t *mod, const char ch)
{
	if (check_modifier(mod, ch)) {
		advance_modifier(mod);
		return 1;
	}
	return 0;
}

static int fs_close(void *data)
{
	FILE *self = data;
	return fclose(self);
}

static int fs_read(void *data, unsigned char *out_buf, size_t amount)
{
	FILE *self = data;
	return fread(out_buf, 1, amount, self);
}

static int fs_write(void *data, const unsigned char *in, size_t count)
{
	FILE *self = data;
	return fwrite(in, 1, count, self);
}

static int fs_seek(void *data, long int offset, int whence)
{
	FILE *self = data;
	return fseek(self, offset, whence);
}

static int fs_flush(void *data)
{
	FILE *self = data;
	return fflush(self);
}

static int mem_close(void *data)
{
	(void)data;
	return 0;
}

static int mem_read(void *data, unsigned char *out_buf, size_t amount)
{
	(void)data;
	(void)out_buf;
	(void)amount;
	exit(1);
}

static int mem_write_altr(void *data, const unsigned char *in, size_t m)
{
	int amount = mem_write(data, in, m);
	if ((size_t)amount < m) {
		return (int)m;
	}

	return amount;
}

static int mem_write(void *data, const unsigned char *in, size_t m)
{
	mem_stream_t *self = data;
	const size_t remaining = self->len - self->pos;
	int write_amount;

	if (remaining == 0) return 0;

	write_amount = m < remaining ? (int)m : (int)(remaining - 1);
	memcpy(self->buffer + self->pos, in, sizeof(char) * write_amount);
	self->pos += write_amount;
	self->buffer[self->pos] = '\0';

	return write_amount;
}

static int mem_seek(void *data, long int offset, int whence)
{
	(void)data;
	(void)offset;
	(void)whence;
	exit(1);
}

static int mem_flush(void *data)
{
	(void)data;
	return 0;
}

# endif /* MY_STREAM_IMPL */

/* ===== my_termcolor ===== */
#ifndef MY_TERMCOLOR_H_
#define MY_TERMCOLOR_H_

/** @def ANSI_CODE_RESET
 *  @brief Reset all attributes back to default. */
#define ANSI_CODE_RESET "\033[00m"
/** @def ANSI_CODE_BOLD
 *  @brief Bold / bright foreground. */
#define ANSI_CODE_BOLD "\033[1m"
/** @def ANSI_CODE_DARK
 *  @brief Dim / dark foreground. */
#define ANSI_CODE_DARK "\033[2m"
/** @def ANSI_CODE_UNDERLINE
 *  @brief Underline text. */
#define ANSI_CODE_UNDERLINE "\033[4m"
/** @def ANSI_CODE_BLINK
 *  @brief Blinking text. */
#define ANSI_CODE_BLINK "\033[5m"
/** @def ANSI_CODE_REVERSE
 *  @brief Reverse video (swap fg/bg). */
#define ANSI_CODE_REVERSE "\033[7m"
/** @def ANSI_CODE_CONCEALED
 *  @brief Concealed / hidden text. */
#define ANSI_CODE_CONCEALED "\033[8m"
/** @def ANSI_CODE_GRAY
 *  @brief Gray foreground. */
#define ANSI_CODE_GRAY "\033[30m"
/** @def ANSI_CODE_GREY
 *  @brief Grey foreground (alias for GRAY). */
#define ANSI_CODE_GREY "\033[30m"
/** @def ANSI_CODE_RED
 *  @brief Red foreground. */
#define ANSI_CODE_RED "\033[31m"
/** @def ANSI_CODE_GREEN
 *  @brief Green foreground. */
#define ANSI_CODE_GREEN "\033[32m"
/** @def ANSI_CODE_YELLOW
 *  @brief Yellow foreground. */
#define ANSI_CODE_YELLOW "\033[33m"
/** @def ANSI_CODE_BLUE
 *  @brief Blue foreground. */
#define ANSI_CODE_BLUE "\033[34m"
/** @def ANSI_CODE_MAGENTA
 *  @brief Magenta foreground. */
#define ANSI_CODE_MAGENTA "\033[35m"
/** @def ANSI_CODE_CYAN
 *  @brief Cyan foreground. */
#define ANSI_CODE_CYAN "\033[36m"
/** @def ANSI_CODE_WHITE
 *  @brief White foreground. */
#define ANSI_CODE_WHITE "\033[37m"
/** @def ANSI_CODE_BG_GRAY
 *  @brief Gray background. */
#define ANSI_CODE_BG_GRAY "\033[40m"
/** @def ANSI_CODE_BG_GREY
 *  @brief Grey background (alias for BG_GRAY). */
#define ANSI_CODE_BG_GREY "\033[40m"
/** @def ANSI_CODE_BG_RED
 *  @brief Red background. */
#define ANSI_CODE_BG_RED "\033[41m"
/** @def ANSI_CODE_BG_GREEN
 *  @brief Green background. */
#define ANSI_CODE_BG_GREEN "\033[42m"
/** @def ANSI_CODE_BG_YELLOW
 *  @brief Yellow background. */
#define ANSI_CODE_BG_YELLOW "\033[43m"
/** @def ANSI_CODE_BG_BLUE
 *  @brief Blue background. */
#define ANSI_CODE_BG_BLUE "\033[44m"
/** @def ANSI_CODE_BG_MAGENTA
 *  @brief Magenta background. */
#define ANSI_CODE_BG_MAGENTA "\033[45m"
/** @def ANSI_CODE_BG_CYAN
 *  @brief Cyan background. */
#define ANSI_CODE_BG_CYAN "\033[46m"
/** @def ANSI_CODE_BG_WHITE
 *  @brief White background. */
#define ANSI_CODE_BG_WHITE "\033[47m"


#endif /* !MY_TERMCOLOR_H_ */


/* ===== my_macro_abuse ===== */
#ifndef MY_MACRO_ABUSE_H_
#define MY_MACRO_ABUSE_H_

/** @def defer_switch(...)
 *  @brief Execute some code after a matching switch case (use `continue` instead of `break`).
 *  @param ...  Switch expression. */
#ifndef defer_switch
#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#    define defer_switch(...) \
	switch ( __VA_ARGS__ ) while (1)
#  else
#    define defer_switch(expr) \
	switch ( expr ) while (1)
#  endif
#endif

/** @def BLOCK
 *  @brief Allows early break from a block scope. */
#ifndef BLOCK
#  define BLOCK  switch (0) default:
#endif

/** @def forange(n_, start_, ...)
 *  @brief Iterate from `start_`, incrementing by 1, until `n_` < end is false.
 *  @param n_      Loop variable name (a size_t integer).
 *  @param start_  Starting value.
 *  @param ...     Exclusive end value. */
#ifndef forange
#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#    define forange(n_, start_, ...) \
	for (size_t n_ = (start_); (n_) < (__VA_ARGS__); (n_) += 1)
#  else
/* C89 compat: user must declare `size_t n_` before using */
#    define forange(n_, start_, end_) \
	for (n_ = (start_); (n_) < (end_); (n_) += 1)
#  endif
#endif /* !forange */

/** @def leach(T_, name_, ...)
 *  @brief Iterate over a linked list by following `->next` pointers.
 *  @param T_     Node type.
 *  @param name_  Loop variable name (pointer to `T_`).
 *  @param ...    Pointer to the head node. */
#ifndef leach
#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#    define leach(T_, name_, ...) \
	for (T_ *name_ = (__VA_ARGS__); (name_) != NULL; (name_) = (name_)->next)
#  else
/* C89 compat: user must declare `T_ *name_` before using */
#    define leach(T_, name_, head_) \
	for (name_ = (head_); (name_) != NULL; (name_) = (name_)->next)
#  endif
#endif /* !leach */

/** @def iarreach(index__, array__)
 *  @brief Iterate over an array-like struct by index.
 *  The struct must have a `.len` field of type `size_t`.
 *  @param index__  Loop variable name (`size_t`).
 *  @param array__  The array struct. */
#ifndef iarreach
#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#    define iarreach(index__, array__) \
	for (size_t index__ = 0; (index__) < (array__).len; (index__) += 1)
#  else
/* C89 compat: user must declare `size_t index__` before using */
#    define iarreach(index__, array__) \
	for (index__ = 0; (index__) < (array__).len; (index__) += 1)
#  endif
#endif /* !iarreach */

/** @def arreach(type_, var_, arr_)
 *  @brief Iterate over an array-like struct by value.
 *  The struct must have `.items` and `.len` fields.
 *  At each iteration `var_` is a copy of the current element.
 *  @param type_  Element type.
 *  @param var_   Loop variable name.
 *  @param arr_   The array struct. */
#ifndef arreach
#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#    define arreach(type_, var_, arr_) \
	for (type_ *var_##_ptr_ = (arr_).items, \
	           *var_##_tmp_ = (void*)1, \
	           *var_##_end_ = (arr_).items + (arr_).len; \
	           var_##_ptr_ < var_##_end_; \
	           var_##_ptr_ += 1, var_##_tmp_ = (void*)1) \
	    for (type_ var_ = *var_##_ptr_; var_##_tmp_; var_##_tmp_ = NULL)
#  else
/* C89 compat: user must declare variables before using:
   type_ *var__ptr_, *var__end_; type_ var_; */
#    define arreach(type_, var_, arr_) \
	for (var_##_ptr_ = (arr_).items, \
	           var_##_tmp_ = (void*)1, \
	           var_##_end_ = (arr_).items + (arr_).len; \
	           var_##_ptr_ < var_##_end_; \
	           var_##_ptr_ += 1, var_##_tmp_ = (void*)1) \
	    for (var_ = *var_##_ptr_; var_##_tmp_; var_##_tmp_ = NULL)
#  endif
#endif /* !arreach */
#endif /* !MY_MACRO_ABUSE_H_ */


/* ===== my_thin_array ===== */
#ifndef MY_THIN_ARRAY_H_
#define MY_THIN_ARRAY_H_

/* #define MY_ARRAY_IMPL */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef MY_THIN_ARRAY_DEF
#  define MY_THIN_ARRAY_DEF CMYLIB_DEF
#endif

/**
 * @brief Internal header stored before every thin array allocation.
 */
typedef struct array_list_header_t {
	size_t len;
	size_t cap;
} array_list_header_t;

#ifndef INITIAL_CAP
/** @def INITIAL_CAP
 *  @brief Default initial capacity (overridable before inclusion). */
# define INITIAL_CAP 10
#endif
#ifndef GROW_FACTOR
/** @def GROW_FACTOR
 *  @brief Growth factor when reallocating (overridable before inclusion). */
# define GROW_FACTOR 2
#endif

/** @def thinarrinit(T)
 *  @brief Create a new thin array of type T.
 *  @param T  Element type.
 *  @return Pointer to the first element slot (type T*). */
#define thinarrinit(T) (T *)(create_array(sizeof(T)))
/** @def thinarrfree(arr)
 *  @brief Free a thin array.
 *  @param arr  The array pointer. */
#define thinarrfree(arr) (assert((arr) != NULL), free(thinarrheader((arr))))
/** @def thinarrheader(arr)
 *  @brief Get the array_list_header_t pointer for a thin array. */
#define thinarrheader(arr) (assert((arr) != NULL), (array_list_header_t *)(((char *)(arr)) - sizeof(array_list_header_t)))
/** @def thinarrlen(arr)
 *  @brief Get the length of a thin array (0 if NULL). */
#define thinarrlen(arr) (((arr) != NULL) ? thinarrheader((arr))->len : 0)
/** @def thinarrsetlen(arr, new_len)
 *  @brief Set the length directly (unsafe — use with care). */
#define thinarrsetlen(arr, new_len) (assert((arr) != NULL), (thinarrheader((arr))->len = new_len))
/** @def thinarrcap(arr)
 *  @brief Get the capacity of a thin array. */
#define thinarrcap(arr) (assert((arr) != NULL), thinarrheader((arr))->cap)
/** @def thinarrpush(arr, val)
 *  @brief Append a value to a thin array (auto-grows). */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define thinarrpush(arr, ...) (assert((arr) != NULL), array_push((void **)&(arr), sizeof(*(arr))), arr[thinarrheader((arr))->len++] = (__VA_ARGS__))
#else
#  define thinarrpush(arr, val) (assert((arr) != NULL), array_push((void **)&(arr), sizeof(*(arr))), arr[thinarrheader((arr))->len++] = (val))
#endif
/** @def thinarrpop(arr)
 *  @brief Pop and return the last element. */
#define thinarrpop(arr) (assert((arr) != NULL), assert(thinarrheader((arr))->len != 0), (arr)[--thinarrheader((arr))->len])
/** @def thinarrreserve(arr, new_cap)
 *  @brief Ensure at least `new_cap` capacity.
 *  @param arr      The array.
 *  @param new_cap  Minimum desired capacity. */
#define thinarrreserve(__arr, __new_cap) (assert((__arr) != NULL), array_reserve((void **)&(__arr), sizeof(*(__arr)), (__new_cap)))

MY_THIN_ARRAY_DEF void *create_array(size_t element_size);
MY_THIN_ARRAY_DEF void array_grow(void **arr, size_t element_size);
MY_THIN_ARRAY_DEF void array_reserve(void **arr, size_t element_size, size_t new_cap);
MY_THIN_ARRAY_DEF void array_push(void **arr, size_t element_size);
#endif /* !MY_THIN_ARRAY_H_ */

#if defined(CMYLIB_IMPL) || defined(MY_THIN_ARRAY_IMPL)

MY_THIN_ARRAY_DEF void *create_array(size_t element_size)
{
	array_list_header_t *res =
		malloc(sizeof(array_list_header_t) + (element_size * INITIAL_CAP));
	if (res == NULL) {
		return NULL;
	}
	memset(res, 0, sizeof(array_list_header_t) + (element_size * INITIAL_CAP));
	res->len = 0;
	res->cap = INITIAL_CAP;
	return res + 1;
}

MY_THIN_ARRAY_DEF void array_grow(void **arr, size_t element_size)
{
	array_list_header_t *header;
	array_list_header_t *new_header;
	header = thinarrheader(*arr);
	header->cap *= GROW_FACTOR;
	new_header =
		realloc(header, sizeof(array_list_header_t) + element_size * header->cap);
	*arr = new_header + 1;
}

MY_THIN_ARRAY_DEF void array_reserve(void **arr, size_t element_size, size_t new_cap)
{
	if (thinarrcap(*arr) < new_cap)
	{
		while (thinarrcap(*arr) < new_cap)
		{
			array_grow(arr, element_size);
		}
	}
}

MY_THIN_ARRAY_DEF void array_push(void **arr, size_t element_size)
{
	array_list_header_t *header = thinarrheader(*arr);
	if (header->len >= header->cap) {
		array_grow(arr, element_size);
	}
}
#endif /* !CMYLIB_IMPL && !MY_THIN_ARRAY_IMPL */


/* ===== utf8 ===== */
#ifndef UTF8_H_
#define UTF8_H_

/* C89 compat: bool is a C99 keyword */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  include <stdbool.h>
#else
#  if !defined(bool)
#    define bool int
#    define true 1
#    define false 0
#  endif
#endif
#include <stddef.h>
#include <stdint.h>

#ifndef UTF8_DEF
#  define UTF8_DEF CMYLIB_DEF
#endif

/**
 * @brief Forward iterator over UTF-8 codepoints.
 */
typedef struct utf8_iter_t {
	const unsigned char *current;
	uint32_t curr_codepoint;
	uint32_t prev_codepoint;
} utf8_iter_t;

/**
 * @brief Initialise a UTF-8 iterator.
 * @param iter  Iterator to initialise (out parameter).
 * @param str   Null-terminated UTF-8 string.
 * @return true on success, false if the string is not valid UTF-8.
 */
UTF8_DEF bool utf8_iter_init(utf8_iter_t *iter, const char *str);
/**
 * @brief Advance to the next codepoint.
 * @param iter  Iterator.
 * @return The number of bytes consumed, or -1 on error.
 */
UTF8_DEF int utf8_next(utf8_iter_t *iter);
/**
 * @brief Get the previous codepoint (before the last `utf8_next` call).
 * @param iter  Iterator (passed by value).
 * @return Unicode codepoint value.
 */
UTF8_DEF uint32_t utf8_prev(utf8_iter_t iter);
/**
 * @brief Peek at the current codepoint without advancing.
 * @param iter  Iterator (passed by value).
 * @return Current Unicode codepoint.
 */
UTF8_DEF uint32_t utf8_peek(utf8_iter_t iter);

/**
 * @brief Decode a single UTF-8 character from a byte sequence.
 * @param str  Pointer to the start of a UTF-8 character.
 * @param out  Receives the decoded codepoint (out parameter).
 * @return Number of bytes consumed (1-4), or -1 on invalid input.
 */
UTF8_DEF int decode_utf8(const unsigned char *str, uint32_t *out);
/**
 * @brief Check whether a C string is valid UTF-8.
 * @param str  The string to validate.
 * @return true if the entire string is valid UTF-8.
 */
UTF8_DEF bool is_valid_utf8_cstr(const unsigned char *str);
/**
 * @brief Determine the byte length of a UTF-8 character from its first byte.
 * @param first_byte  The leading byte.
 * @return 1, 2, 3, 4 for valid leading bytes, or 0 for invalid.
 */
UTF8_DEF size_t get_utf8_char_length(const unsigned char first_byte);
#endif /* !UTF8_H_ */

#if defined(CMYLIB_IMPL) || defined(UTF8_IMPL)
UTF8_DEF bool utf8_iter_init(utf8_iter_t *iter, const char *str) {
	if (!is_valid_utf8_cstr((const unsigned char *)str)) {
		return false;
	}
	iter->curr_codepoint = 0;
	iter->prev_codepoint = 0;
	iter->current = (const unsigned char *)str;
	return utf8_next(iter) != -1;
}

UTF8_DEF int utf8_next(utf8_iter_t *iter) {
	size_t i;
	unsigned char first_byte;
	size_t expected_len;

	iter->prev_codepoint = iter->curr_codepoint;
	assert(iter->current != NULL);
	if (iter->current[0] == '\0') {
		iter->current += 1;
		iter->curr_codepoint = 0;
		return 1;
	}

	first_byte = iter->current[0];
	expected_len = get_utf8_char_length(first_byte);
	if (expected_len > 0) {
		for (i = 0; i < expected_len; i++) {
			if (iter->current[i] == '\0') {
				return -1;
			}
		}
	}

	int len = decode_utf8(iter->current, &iter->curr_codepoint);
	if (len == -1) return len;
	iter->current += len;
	return len;
}

UTF8_DEF uint32_t utf8_prev(utf8_iter_t iter) {
	return iter.prev_codepoint;
}

UTF8_DEF uint32_t utf8_peek(utf8_iter_t iter) {
	return iter.curr_codepoint;
}

UTF8_DEF size_t get_utf8_char_length(const unsigned char first_byte) {
	if (first_byte <= 0x7F) return 1;
	if ((first_byte & 0xE0) == 0xC0) return 2;
	if ((first_byte & 0xF0) == 0xE0) return 3;
	if ((first_byte & 0xF8) == 0xF0) return 4;
	return 0;
}

UTF8_DEF int decode_utf8(const unsigned char *str, uint32_t *out) {
	if (str[0] < 0x80) {
		*out = str[0];
		return 1;
	} else if ((str[0] & 0xE0) == 0xC0) {
		if ((str[1] & 0xC0) != 0x80)
			return -1;
		*out = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
		if (*out < 0x80) return -1;
		return 2;
	} else if ((str[0] & 0xF0) == 0xE0) {
		if ((str[1] & 0xC0) != 0x80 || (str[2] & 0xC0) != 0x80)
			return -1;
		*out = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
		if (*out < 0x800) return -1;
		if (*out >= 0xD800 && *out <= 0xDFFF) return -1;
		return 3;
	} else if ((str[0] & 0xF8) == 0xF0) {
		if ((str[1] & 0xC0) != 0x80 || (str[2] & 0xC0) != 0x80 || (str[3] & 0xC0) != 0x80)
			return -1;
		*out = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) |
			  (str[3] & 0x3F);
		if (*out < 0x10000) return -1;
		if (*out > 0x10FFFF) return -1;
		return 4;
	}
	return -1;
}

UTF8_DEF bool is_valid_utf8_cstr(const unsigned char *str) {
	const unsigned char *ptr = str;

	if (str == NULL)
		return false;

	while (*ptr != '\0') {
		size_t char_len;
		int len;
		uint32_t codepoint;

		char_len = get_utf8_char_length(*ptr);
		if (char_len > 0) {
			size_t j;
			for (j = 1; j < char_len; j++) {
				if (ptr[j] == '\0') return false;
			}
		}

		len = 0;
		codepoint = 0;
		if ((len = decode_utf8(ptr, &codepoint)) == -1) return false;
		ptr += len;
	}

	return true;
}
#endif /* !CMYLIB_IMPL && !UTF8_IMPL */


/* ===== my_flags ===== */
#ifndef MY_FLAGS_H_
#define MY_FLAGS_H_

#include <stdio.h>
#include <stddef.h>

#ifdef __cpluscplus
extern "C" {
#endif /* __cpluscplus */

#ifndef MY_FLAG_DEF
#  define MY_FLAG_DEF CMYLIB_DEF
#endif

typedef enum flag_type_t {
	FLAG_NONE = 0,
	FLAG_SIZE_T,
	FLAG_INT,
	FLAG_STRING,
	FLAG_BOOL
} flag_type_t;

typedef void (*flag_call_t)(void *dest, const char *value);

/**
 * @brief define a flag
 * @param dest a pointer to a destination variable (see the example above)
 * @param type the type of the flag. this is used to determin the destination type.
 * @param long_name a name for the flag that start with `--`
 * @param short_name a name for the flag that starts with `-` usualy an abbriviation to `long_name`
 * @param description a description used for the help message
 */
MY_FLAG_DEF void def_flag(
	void *dest, flag_type_t type,
	const char *long_name,
	const char *short_name,
	const char *description);

/**
 * @brief Upon matching with the flag. it will call `callback`
 * @param callback a function pointer
 * @param long_name a name for the flag that start with `--`
 * @param short_name a name for the flag that starts with `-` usualy an abbriviation to `long_name`
 * @param description a description used for the help message
 */
MY_FLAG_DEF void def_flag_call(
	flag_call_t callback,
	void *dest,
	flag_type_t type,
	const char *long_name,
	const char *short_name,
	const char *description);

/**
 * @brief parse the arguments looking for a flag defined with `def_flag`.
 *
 * This function is special. As it parses command line arguments and at the same time
 * it shuffles them in a way that the remaining arguments are non-flag argument which
 * you can use later. e.g. suppose you defined `b` (which takes an integer as an argument) and `c` flag. and the user
 * passes `-c 123 -b 456 a b c`. the parser will take `./program-name -c 123 -b 456 a b c` and it shuffles it back to
 * `-c -b 456 ./program-name 123 a b c` where `argc` * is set to 4. and `argv` now points to `./program-name`
 * @param argc a pointer to `main`'s argc
 * @param argv a pointer to `main`'s argv
 */
MY_FLAG_DEF void parse_flag(int *argc, char ***argv);

/**
 * @brief Prints a help message to `stream`
 * @param stream a stream. e.g. `stdout`, `stderr` or even a regulare file
 */
MY_FLAG_DEF void print_help_flag(FILE *stream);

#ifdef __cplus_cplus
}
#endif /* __cpluscplus */
#endif /* !MY_FLAGS_H_ */

#if defined(CMYLIB_IMPL) || defined(MY_FLAGS_IMPL)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cpluscplus
extern "C" {
#endif /* __cpluscplus */

#ifndef FLAGS_DEFAULT_CAP
#  define FLAGS_DEFAULT_CAP 16
#endif /* !FLAGS_DEFAULT_CAP */

#define MAX_FLAG_NAME_LEN ((size_t)512)

typedef struct flag_t {
	flag_type_t type;
	const char *long_name;
	const char *short_name;
	const char *description;
	flag_call_t callback;
	void *dest;
} flag_t;

typedef struct flags_t {
	size_t len, cap;
	size_t max_name_len;
	flag_t *items;
} flags_t;

static flags_t flags = {0};

static void add_flag(flag_t flag);
static const char *flag_type_as_str(flag_type_t type);

MY_FLAG_DEF void def_flag(
	void *dest, flag_type_t type,
	const char *long_name,
	const char *short_name,
	const char *description)
{
	flag_t flag = {0};
	const size_t long_name_len = long_name == NULL ? 0 : strlen(long_name);
	const size_t short_name_len = short_name == NULL ? 0 : strlen(short_name);

	if (long_name_len == 0 && short_name_len == 0) {
		fprintf(stderr, "flag error: You can't have both of the `short_name` and `long_name` arguments as NULL. only one of them can be NULL. never both!!\n");
		fprintf(stderr, "e.g. flag(dest, FLAG_TYPE, NULL, \"short_name\", \"description\"); // good\n");
		fprintf(stderr, "e.g. flag(dest, FLAG_TYPE, \"long_name\", NULL, \"description\"); // good\n");
		fprintf(stderr, "e.g. flag(dest, FLAG_TYPE, NULL, NULL, \"description\"); // BAD\n");
		abort();
	}

	if (long_name_len >= MAX_FLAG_NAME_LEN) {
		fprintf(stderr, "`long_name` argument is too long (%zu > %zu)\n", strlen(long_name), MAX_FLAG_NAME_LEN);
		abort();
	}

	if (short_name_len >= MAX_FLAG_NAME_LEN) {
		fprintf(stderr, "`short_name` argument is too long (%zu > %zu)\n", strlen(short_name), MAX_FLAG_NAME_LEN);
		abort();
	}

	flag.type = type;
	flag.long_name = long_name && strlen(long_name) == 0 ? NULL : long_name;
	flag.short_name = short_name && strlen(short_name) == 0 ? NULL : short_name;
	flag.description = description;
	flag.dest = dest;

	add_flag(flag);
}

MY_FLAG_DEF void def_flag_call(
	flag_call_t callback,
	void *dest,
	flag_type_t type,
	const char *long_name,
	const char *short_name,
	const char *description)
{
  def_flag(dest, type, long_name, short_name, description);
	flags.items[flags.len - 1].callback = callback;
}

#define starts_with(a_, b_) (strncmp(a_, b_, strlen(b_)) == 0)
#define equals(a_, b_) (strcmp(a_, b_) == 0)
#define ended() (*i >= *argc)
#define peek() (*argv)[i]
#define consume() i += 1

static void parse_flag2(flag_t flag, char *arg);
static flag_t find_long_flag(const char *const name);
static flag_t find_short_flag(const char *const name);

MY_FLAG_DEF void parse_flag(int *argc, char ***argv)
{
	int write_pos = 1;
	int i = 1;
	if (*argc <= 0) return;

	while (i < *argc) {
		int is_flag = 0;

		if (starts_with((*argv)[i], "--")) {
			const char *flag_char = peek() + 2;
			char *sep = strchr(flag_char, '=');
			char *value = NULL;
			flag_t flag = {0};
			if (sep == NULL) {
				sep = strchr(flag_char, ':');
			}
			if (sep != NULL) {
				value = sep + 1;
				*sep = '\0';
			}

			flag = find_long_flag(flag_char);
			if (flag.type != FLAG_NONE) {
				is_flag = 1;
				i++;
				if (value == NULL && flag.type != FLAG_BOOL) {
					value = peek();
					consume();
				}
				parse_flag2(flag, value);
			}
		} else if (starts_with((*argv)[i], "-")) {
			const char *flag_char = peek() + 1;
			char *sep = strchr(flag_char, '=');
			char *value = NULL;
			flag_t flag = {0};
			if (sep == NULL) {
				sep = strchr(flag_char, ':');
			}
			if (sep != NULL) {
				value = sep + 1;
				*sep = '\0';
			}

			flag = find_short_flag(flag_char);
			if (flag.type != FLAG_NONE) {
				is_flag = 1;
				i++;
				if (value == NULL && flag.type != FLAG_BOOL) {
					value = peek();
					consume();
				}
				parse_flag2(flag, value);
			}
		}

		if (!is_flag) {
			(*argv)[write_pos] = (*argv)[i];
			write_pos++;
			i++;
		}
	}

	*argc = write_pos;
}

static void parse_flag2(flag_t flag, char *value)
{
	/* TODO: also this should work with `=` and `:` */
	if (flag.callback != NULL) {
		flag.callback(flag.dest, value);
		return;
	}

	switch (flag.type) {
	case FLAG_SIZE_T:
		/* TODO: Check for errors */
		*((size_t*)flag.dest) = strtoul(value, NULL, 10);
		break;
	case FLAG_INT:
		/* TODO: Check for errors */
		*((int*)flag.dest) = atoi(value);
		break;
	case FLAG_STRING:
		*((char **)flag.dest) = value;
		break;
	case FLAG_BOOL:
		*((int*)flag.dest) = 1;
		break;
	default:
		fprintf(stderr, "Reached unreachable case: %d\n", flag.type);
		abort();
	}
}

static flag_t find_long_flag(const char *const name)
{
	const flag_t fail = {0};
	size_t i = 0;
	for (i=0; i < flags.len; i += 1) {
		const flag_t flag = flags.items[i];
		if (equals(flag.long_name, name)) {
			return flag;
		}
	}
	return fail;
}

static flag_t find_short_flag(const char *const name)
{
	const flag_t fail = {0};
	size_t i = 0;
	for (i=0; i < flags.len; i += 1) {
		const flag_t flag = flags.items[i];
		if (equals(flag.short_name, name)) {
			return flag;
		}
	}
	return fail;
}

#undef ended
#undef peek
#undef consume
#undef starts_with

MY_FLAG_DEF void print_help_flag(FILE *stream)
{
	size_t i = 0;
	const char *indent = "  ";
	char buffer[MAX_FLAG_NAME_LEN * 2 + 67] = {0};
	size_t pos = 0;

	for (i=0; i < flags.len; i += 1) {
		flag_t flag = flags.items[i];
		fprintf(stream, "%s", indent);

		if (flag.long_name != NULL) {
			pos += sprintf(buffer + pos, "--%s ", flag.long_name);
		}
		if (flag.short_name != NULL) {
			pos += sprintf(buffer + pos, "-%s ", flag.short_name);
		}

		buffer[pos - 1] = '\0';
		fprintf(stream, "%-*s ", (int)flags.max_name_len, buffer);
		fprintf(stream, "%-10s ", flag.type == FLAG_BOOL ? "" : flag_type_as_str(flag.type));
		fprintf(stream, "%s\n", flag.description == NULL ? "" : flag.description);

		pos = 0;
		memset(buffer, 0, sizeof(char) * MAX_FLAG_NAME_LEN * 2);
	}
}

static void add_flag(flag_t flag)
{
	size_t len = 4;
	if (flags.len >= flags.cap) {
		flags.cap = flags.cap == 0 ? FLAGS_DEFAULT_CAP : flags.cap * 2;
		flags.items = realloc(flags.items, sizeof(flag_t) * flags.cap);
	}

	if (flag.long_name != NULL) {
		len += strlen(flag.long_name);
	}
	if (flag.short_name != NULL) {
		len += strlen(flag.short_name);
	}
	if (len > flags.max_name_len) {
		flags.max_name_len = len;
	}
	flags.items[flags.len] = flag;
	flags.len += 1;
}

static const char *flag_type_as_str(flag_type_t type)
{
	switch (type) {
	case FLAG_NONE:   return "<none>";
	case FLAG_SIZE_T: return "<number>";
	case FLAG_INT:    return "<number>";
	case FLAG_STRING: return "<string>";
	case FLAG_BOOL:   return "<bool>";
	}
}

#ifdef __cplus_cplus
}
#endif /* __cpluscplus */
#endif /* !CMYLIB_IMPL && !MY_FLAGS_IMPL */


/* ===== my_temporary_allocator ===== */
#ifndef MY_TEMPORARY_ALLOCATOR_H_
#define MY_TEMPORARY_ALLOCATOR_H_

/* #define MY_TEMPORARY_ALLOCATOR_IMPL */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

#ifndef MY_TEMPORARY_ALLOCATOR_DEF
#  define MY_TEMPORARY_ALLOCATOR_DEF CMYLIB_DEF
#endif

/**
 * @brief Optionally increase the internal buffer size.
 *
 * When called, the internal buffer is replaced with a heap allocation
 * of the requested size (via the default allocator). Must be matched
 * with a call to `free_temporary_allocator`.
 */
MY_TEMPORARY_ALLOCATOR_DEF void setup_temporary_allocator(size_t size);

/**
 * @brief Free the heap buffer (if any) and revert to the static default buffer.
 */
MY_TEMPORARY_ALLOCATOR_DEF void free_temporary_allocator(void);

/**
 * @brief Get an allocator_t that allocates from the temporary bump region.
 * @return An allocator_t backed by the temporary buffer.
 */
MY_TEMPORARY_ALLOCATOR_DEF allocator_t get_temporary_allocator(void);

/**
 * @brief Reset the bump pointer to the beginning (fast clear).
 *
 * Does not free any memory, just marks everything as reusable.
 */
MY_TEMPORARY_ALLOCATOR_DEF void reset_temporary_allocator(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !MY_TEMPORARY_ALLOCATOR_H_ */

#if defined(CMYLIB_IMPL) || defined(MY_TEMPORARY_ALLOCATOR_IMPL)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>

#define TEMPORARY_ALLOCATOR_DEFAULT_CAP 2048

/** @brief Check whether the temporary allocator uses a heap buffer. */
#define is_using_heap() (global_temporary.buffer != global_temporary_buffer)

/** @brief Internal state of the temporary allocator. */
typedef struct temporary_allocator_t {
	size_t used;
	size_t cap;
	unsigned char *buffer;
} temporary_allocator_t;

static unsigned char global_temporary_buffer[TEMPORARY_ALLOCATOR_DEFAULT_CAP] = {0};
static temporary_allocator_t global_temporary = {
	0,
	TEMPORARY_ALLOCATOR_DEFAULT_CAP,
	global_temporary_buffer,
};

static void *temp_allocate_virt(void *self, size_t alignment, size_t size);
static void *temp_reallocate_virt(void *self, size_t old_size, void *ptr, size_t alignment, size_t new_size);
static void temp_free_virt(void *self, size_t size, void *ptr);

static allocator_interface_t temporary_allocator_vtable = {
	temp_allocate_virt,
	temp_reallocate_virt,
	temp_free_virt,
};

MY_TEMPORARY_ALLOCATOR_DEF void setup_temporary_allocator(size_t size)
{
	if (is_using_heap())
	{
		global_temporary.buffer = renew(size, global_temporary.buffer);
		global_temporary.cap = size;
		return;
	}

	global_temporary.buffer = make(size);
	global_temporary.cap = size;
}

MY_TEMPORARY_ALLOCATOR_DEF void free_temporary_allocator(void)
{
	if (is_using_heap())
	{
		delete(global_temporary.buffer);
	}

	global_temporary.cap = TEMPORARY_ALLOCATOR_DEFAULT_CAP;
	global_temporary.used = 0;
	global_temporary.buffer = global_temporary_buffer;
}

MY_TEMPORARY_ALLOCATOR_DEF void reset_temporary_allocator(void)
{
	global_temporary.used = 0;
}

MY_TEMPORARY_ALLOCATOR_DEF allocator_t get_temporary_allocator(void)
{
	return allocator_new(&global_temporary, &temporary_allocator_vtable);
}

/** @brief Bump-allocate from the temporary buffer. */
static void *temp_allocate_virt(void *data, size_t alignment, size_t size)
{
	temporary_allocator_t *const self = data;

	uintptr_t curr = (uintptr_t)(self->buffer + self->used);
	uintptr_t aligned = (curr + alignment - 1) & ~(alignment - 1);
	size_t padding = aligned - curr;

	self->used += size + padding;
	if (self->used >= self->cap)
	{
		fprintf(stderr, "FATAL ERROR: not enough temporary space.\n");
		exit(1);
		return NULL;
	}

	return (void*)aligned;
}

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)

/** @brief Reallocate within the temporary buffer (bump + copy). */
static void *temp_reallocate_virt(void *data, size_t old_size, void *ptr, size_t alignment, size_t new_size)
{
	temporary_allocator_t *const self = data;
	size_t potential_allocation_size;
	void *result;

	(void)old_size;
	potential_allocation_size = (uintptr_t)(self->buffer + self->used) - (uintptr_t)ptr;
	result = temp_allocate_virt(self, alignment, new_size);
	memcpy(result, ptr, MIN(potential_allocation_size, new_size));

	return result;
}

static void temp_free_virt(void *data, size_t size, void *ptr)
{
	temporary_allocator_t *const self = data;
	(void)self;
	(void)size;
	(void)ptr;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CMYLIB_IMPL && !MY_TEMPORARY_ALLOCATOR_IMPL */


/* ===== my_arena_allocator ===== */
#ifndef MY_ARENA_ALLOCATOR_H_
#define MY_ARENA_ALLOCATOR_H_

/* #define MY_ARENA_ALLOCATOR_IMPL */

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#ifndef MY_ARENA_ALLOCATOR_DEF
#  define MY_ARENA_ALLOCATOR_DEF CMYLIB_DEF
#endif

/**
 * @brief A linked-list block of arena memory.
 */
typedef struct arena_block_t arena_block_t;

struct arena_block_t {
	arena_block_t *next;
	void *current;
	void *end;
};

/**
 * @brief Arena allocator state.
 */
typedef struct arena_t {
	allocator_t child_allocator;
	arena_block_t *begin;
	arena_block_t *end;
	size_t total_size;
} arena_t;

#ifndef ARENA_REGION_DEFAULT_CAPACITY
/** @def ARENA_REGION_DEFAULT_CAPACITY
 *  @brief Default block size for new arena regions (overridable). */
# define ARENA_REGION_DEFAULT_CAPACITY (8 * 1024)
#endif /* ARENA_REGION_DEFAULT_CAPACITY */

/**
 * @brief Create an arena backed by a given child allocator.
 * @param child_allocator  The allocator used to allocate arena blocks.
 * @return An initialised arena_t (no blocks allocated yet).
 */
MY_ARENA_ALLOCATOR_DEF arena_t arena_new(allocator_t child_allocator);
/**
 * @brief Create an arena backed by the default allocator.
 * @return An initialised arena_t using the global default allocator.
 */
MY_ARENA_ALLOCATOR_DEF arena_t arena_new_default(void);
/**
 * @brief Free all arena blocks and reset the arena to zero.
 * @param self  The arena to destroy.
 */
MY_ARENA_ALLOCATOR_DEF void arena_free(arena_t *self);
/**
 * @brief Print arena block usage statistics to a file.
 * @param out   Output file (e.g. stdout, stderr).
 * @param arena The arena to inspect (passed by value).
 */
MY_ARENA_ALLOCATOR_DEF void arena_print(FILE *out, const arena_t arena);

/**
 * @brief Get an allocator_t that allocates from this arena.
 * @param arena  The arena (must outlive the returned allocator).
 * @return An allocator_t backed by the arena.
 */
MY_ARENA_ALLOCATOR_DEF allocator_t arena_get_allocator(arena_t *arena);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !MY_ARENA_ALLOCATOR_H_ */

#if defined(CMYLIB_IMPL) || defined(MY_ARENA_ALLOCATOR_IMPL)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* --- arena vtable implementation --- */
static void *arena_allocate_virt(void *data, size_t alignment, size_t size);
static void *arena_reallocate_virt(void *data, size_t old_size, void *ptr, size_t alignment, size_t new_size);
static void arena_free_virt(void *data, size_t size, void *ptr);

/** @brief Vtable for arena allocations. */
static allocator_interface_t vtable = {
	arena_allocate_virt,
	arena_reallocate_virt,
	arena_free_virt,
};

MY_ARENA_ALLOCATOR_DEF arena_t arena_new(allocator_t child_allocator)
{
	arena_t result;
	memset(&result, 0, sizeof(result));
	result.child_allocator = child_allocator;
	return result;
}

MY_ARENA_ALLOCATOR_DEF arena_t arena_new_default(void)
{
	return arena_new(get_default_allocator());
}

MY_ARENA_ALLOCATOR_DEF void arena_free(arena_t *self)
{
	arena_block_t *current = self->begin;

	while (current != NULL) {
		arena_block_t *next = current->next;
		xdestroy(self->child_allocator, self->total_size, current);
		current = next;
	}

	memset(self, 0, sizeof(*self));
}

MY_ARENA_ALLOCATOR_DEF allocator_t arena_get_allocator(arena_t *arena)
{
	return allocator_new(arena, &vtable);
}

/** @brief Allocate a new arena block of at least `min_size` bytes. */
static arena_block_t *arena_append_block(arena_t *self, size_t min_size)
{
	size_t block_size;
	arena_block_t* new_block;

	block_size = (ARENA_REGION_DEFAULT_CAPACITY > min_size ? ARENA_REGION_DEFAULT_CAPACITY : min_size) + sizeof(arena_block_t);
	if (min_size + sizeof(arena_block_t) > block_size) {
		block_size = min_size + sizeof(arena_block_t);
	}

	new_block = align_alloc(
		self->child_allocator,
		GET_ALIGNMENT(arena_block_t),
		block_size);
	if (new_block == NULL) {
		return NULL;
	}

	new_block->current = new_block + 1;
	new_block->end = (void*)((uintptr_t)new_block + block_size);
	new_block->next = NULL;

	self->total_size += block_size;
	if (self->begin == NULL) {
		assert(self->end == NULL);
		self->begin = new_block;
		self->end = new_block;
	} else {
		self->end->next = new_block;
		self->end = new_block;
	}

	return new_block;
}

/** @brief Used bytes inside a block. */
#if 0
static size_t block_used_size(const arena_block_t *self)
{
	return (uintptr_t)self->current - (uintptr_t)(self + 1);
}
#endif

/** @brief Remaining usable bytes in a block. */
static size_t block_available_size(const arena_block_t *self)
{
	return (uintptr_t)self->end - (uintptr_t)self->current;
}

/** @brief Total usable capacity of a block (excluding the header). */
static size_t block_actual_size(const arena_block_t *self)
{
	return (uintptr_t)self->end - (uintptr_t)(self + 1);
}

/** @brief Find which arena block a pointer belongs to. */
static arena_block_t *find_owner(arena_t *self, void *ptr)
{
	arena_block_t *current = self->begin;

	while (current != NULL) {
		arena_block_t *next = current->next;
		void *start = current + 1;
		if (ptr < current->end && ptr >= start) {
			return current;
		}
		current = next;
	}

	return NULL;
}

/** @brief Allocate from the arena (bump + align). */
static void *arena_allocate_virt(void *data, size_t alignment, size_t size)
{
	arena_t *self = data;
	arena_block_t *blk;
	uintptr_t curr;
	uintptr_t aligned;
	size_t padding;

	if (self->end == NULL) {
		assert(self->begin == NULL);
		arena_append_block(self, size);
	}

	blk = self->end;
	curr = (uintptr_t)blk->current;
	aligned = (curr + alignment - 1) & ~(alignment - 1);
	padding = aligned - curr;

	if (block_available_size(blk) < size + padding) {
		blk = arena_append_block(self, size);

		curr = (uintptr_t)blk->current;
		aligned = (curr + alignment - 1) & ~(alignment - 1);
		padding = aligned - curr;
	}

	blk->current = (void*)(aligned + size);
	return (void*)aligned;
}

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)

static void *arena_reallocate_virt(void *data, size_t old_size, void *ptr, size_t alignment, size_t new_size)
{
	arena_t *const self = data;
	arena_block_t *const target = find_owner(self, ptr);
	size_t potential_allocation_size;
	void* result;

	(void)old_size;
	if (target == NULL) return NULL;

	potential_allocation_size = (uintptr_t)target->current - (uintptr_t)ptr;
	result = arena_allocate_virt(self, alignment, new_size);
	memcpy(result, ptr, MIN(potential_allocation_size, new_size));

	return result;
}

static void arena_free_virt(void *data, size_t size, void *ptr)
{
	(void)size;
	((void)data);
	((void)ptr);
}

MY_ARENA_ALLOCATOR_DEF void arena_print(FILE *out, const arena_t arena)
{
	const size_t width = 20;
	arena_block_t *current = arena.begin;
	size_t index = 0;
	char *const bar = calloc(sizeof(char), width + 1);
	if (bar == NULL) return;

	while (current != NULL) {
		arena_block_t *next = current->next;
		const float persentage = (float)(block_actual_size(current) - block_available_size(current)) / (float)block_actual_size(current);

		size_t i;
		memset(bar, ' ', width);
		for (i=0; i < width * persentage; i += 1) bar[i] = '=';

		fprintf(
			out,
			"Block(%04zu): %6.2f%% [%s] %zu/%zu bytes\n",
			index++, persentage * 100.0F, bar,
			block_actual_size(current) - block_available_size(current),
			block_actual_size(current));
		current = next;
	}

	{
		double size = (double)arena.total_size;
		const char *unit = "B";

		if (size >= 1024) { size /= 1024; unit = "kB"; }
		if (size >= 1024) { size /= 1024; unit = "MB"; }
		if (size >= 1024) { size /= 1024; unit = "GB"; }

		fprintf(out, "\t> Allocated: %zu bytes (%.2f %s)\n",
				arena.total_size, size, unit);
	}

	free(bar);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !CMYLIB_IMPL && !MY_ARENA_ALLOCATOR_IMPL */


/* ===== my_c_allocator ===== */
#ifndef MY_C_ALLOCATOR_H_
#define MY_C_ALLOCATOR_H_

/* #define MY_C_ALLOCATOR_IMPL */


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef MY_C_ALLOCATOR_DEF
#  define MY_C_ALLOCATOR_DEF CMYLIB_DEF
#endif

/**
 * @brief Return an allocator_t backed by libc's malloc / realloc / free.
 * @return An allocator_t that delegates to the standard C heap functions.
 */
MY_C_ALLOCATOR_DEF allocator_t get_c_allocator(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !MY_C_ALLOCATOR_H_ */

#if defined(CMYLIB_IMPL) || defined(MY_C_ALLOCATOR_IMPL)
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void *c_alloc_virt(void *self, size_t alignment, size_t size);
static void *c_realloc_virt(void *self, size_t old_size, void *ptr, size_t alignment, size_t size);
static void  c_free_virt(void *self, size_t size, void *ptr);

/** @brief Vtable for the C allocator. */
static allocator_interface_t c_allocator_vtable = {
	c_alloc_virt,
	c_realloc_virt,
	c_free_virt,
};

MY_C_ALLOCATOR_DEF allocator_t get_c_allocator(void)
{
	{ allocator_t r_; r_.vtable = &c_allocator_vtable; r_.data = NULL; return r_; }
}

/** @brief Allocate via malloc (alignment ignored). */
static void *c_alloc_virt(void *self, size_t alignment, size_t size)
{
	((void)self);
	((void)alignment);
	return malloc(size);
}

/** @brief Reallocate via realloc (old_size / alignment ignored). */
static void *c_realloc_virt(void *self, size_t old_size, void *ptr, size_t alignment, size_t size)
{
	((void)self);
	(void)old_size;
	((void)alignment);
	return realloc(ptr, size);
}

/** @brief Free via free (size / alignment ignored). */
static void c_free_virt(void *self, size_t size, void *ptr)
{
	((void)self);
	(void)size;
	free(ptr);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !CMYLIB_IMPL && !MY_C_ALLOCATOR_IMPL */


/* ===== my_array ===== */
#ifndef MY_ARRAY_H_
#define MY_ARRAY_H_

#include <stdlib.h>
#include <assert.h>

#ifndef INITIAL_CAP
/** @def INITIAL_CAP
 *  @brief Default initial capacity (overridable). */
# define INITIAL_CAP 10
#endif /* !INITIAL_CAP */
#ifndef GROW_FACTOR
/** @def GROW_FACTOR
 *  @brief Capacity multiplier when growing (overridable). */
# define GROW_FACTOR 2
#endif /* !GROW_FACTOR */

/** @def arrsize(array)
 *  @brief Total byte size of the allocated items buffer. */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define arrsize(...) (sizeof(*(__VA_ARGS__).items)) * (__VA_ARGS__).cap
#else
#  define arrsize(array__) (sizeof(*(array__).items)) * (array__).cap
#endif

/** @def arrfree(allocator, array)
 *  @brief Free the items buffer and reset len/cap to 0. */
#define arrfree(allocator_, array__) \
	do { \
		xdestroy(allocator_, arrsize(array__), (array__).items); \
		(array__).len = 0; \
		(array__).cap = 0; \
	} while (0)

/** @def arrpush(allocator, array, item)
 *  @brief Append an item to the array (auto-grows). */
#define arrpush(allocator_, array__, item__) \
	do { \
		if ((array__).len >= (array__).cap) { \
			arrgrow((allocator_), (array__)); \
		} \
		(array__).items[(array__).len++] = (item__);	\
	} while (0)

/** @def arrgrow(allocator, array)
 *  @brief Grow the array capacity by GROW_FACTOR (or initialise to INITIAL_CAP). */
#define arrgrow(allocator_, array__) \
	do { \
		const size_t new_cap__ = (array__).cap == 0 ? (INITIAL_CAP) : (array__).cap * (GROW_FACTOR); \
		(array__).items = xrecreate((allocator_), arrsize(array__), (new_cap__ * sizeof(*(array__).items)), (array__).items); \
		(array__).cap = new_cap__; \
	} while (0)

/** @def arrinsert(allocator, array, item, pos)
 *  @brief Insert an item at position `pos`, shifting elements right.
 *  @param allocator  The allocator.
 *  @param array      The array struct.
 *  @param item       Value to insert.
 *  @param pos        Target index (must be <= len). */
#define arrinsert(allocator_, array__, item__, pos__) \
	do { \
		size_t len__; \
		size_t i__; \
		len__ = (array__).len; \
		arrreserve((allocator_), (array__), len__ + 1); \
		assert((pos__) <= len__); \
		for (i__ = len__; i__ >= (pos__) ; i__ -= 1) { \
			(array__).items[i__] = (array__).items[i__ - 1]; \
		} \
		(array__).items[(pos__)] = (item__); \
		(array__).len += 1; \
	} while (0)

/** @def arrpop(array)
 *  @brief Decrement length (remove the last element, no memory free). */
#define arrpop(array__) \
	do { \
		if ((array__).len == 0) break; \
		(array__).len -= 1; \
	} while (0)

/** @def arrreserve(allocator, array, min_cap)
 *  @brief Ensure at least `min_cap` capacity. */
#define arrreserve(allocator_, array__, min_cap__) \
	do { \
		if ((array__).cap >= (min_cap__)) {	\
			break; \
		} \
		size_t new_cap__ = (array__).cap ? (array__).cap : INITIAL_CAP; \
		while (new_cap__ < (min_cap__)) { \
			new_cap__ *= (GROW_FACTOR);			  \
		} \
		(array__).items = xrecreate((allocator_), arrsize(array__), new_cap__ * sizeof(*(array__).items), (array__).items); \
		(array__).cap = new_cap__; \
	} while (0)

/** @def marrlen(array)
 *  @brief Get the current length of the array. */
#define marrlen(array__) ((array__).len)
/** @def marrcap(array)
 *  @brief Get the current capacity (NOTE: currently same as len). */
#define marrcap(array__) ((array__).len)
/** @def marrget(array, index)
 *  @brief Get the element at `index`. */
#define marrget(array__, index__) ((array__).items[index__])


#endif /* !MY_ARRAY_H_ */


/* ===== my_managed_array ===== */
#ifndef MY_MANAGED_ARRAY_H_
#define MY_MANAGED_ARRAY_H_

#include <stdlib.h>
#include <assert.h>

#ifndef INITIAL_CAP
/** @def INITIAL_CAP
 *  @brief Default initial capacity (overridable). */
# define INITIAL_CAP 256
#endif /* !INITIAL_CAP */
#ifndef GROW_FACTOR
/** @def GROW_FACTOR
 *  @brief Capacity multiplier when growing (overridable). */
# define GROW_FACTOR 2
#endif /* !GROW_FACTOR */

/** @def marrinit(T, allocator)
 *  @brief Initialise a managed array struct literal with the given allocator.
 *  @param T          The struct type (must have `.allocator` field).
 *  @param allocator  The Allocator to use. */
#define marrinit(T_, allocator_) (T_) { .allocator = (allocator_) }

/** @def marrsize(array)
 *  @brief Total byte size of the allocated items buffer. */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define marrsize(...) (sizeof(*(__VA_ARGS__).items)) * (__VA_ARGS__).cap
#else
#  define marrsize(array_) (sizeof(*(array_).items)) * (array_).cap
#endif

/** @def marrfree(array)
 *  @brief Free the items buffer and reset len/cap to 0. */
#define marrfree(array_) \
	do { \
		xdestroy((array_).allocator, marrsize(array_), (array_).items); \
		(array_).len = 0; \
		(array_).cap = 0; \
	} while (0)

/** @def marrpush(array, item)
 *  @brief Append an item (auto-grows using the embedded allocator). */
#define marrpush(array_, item_) \
	do { \
		if ((array_).len >= (array_).cap) { \
			marrgrow((array_)); \
		} \
		(array_).items[(array_).len++] = (item_);	\
	} while (0)

/** @def marrgrow(array)
 *  @brief Grow the array capacity. */
#define marrgrow(array_) \
	do { \
		const size_t old_cap_ = (array_).cap; \
		const size_t new_cap_ = old_cap_ == 0 ? (INITIAL_CAP) : old_cap_ * (GROW_FACTOR); \
		(array_).cap = new_cap_; \
		(array_).items = xrecreate((array_).allocator, old_cap_ * sizeof(*(array_).items), new_cap_ * sizeof(*(array_).items), (array_).items); \
	} while (0)

/** @def marrinsert(array, item, pos)
 *  @brief Insert an item at position `pos`, shifting elements right.
 *  @param array  The array struct.
 *  @param item   Value to insert.
 *  @param pos    Target index (must be <= len). */
#define marrinsert(array_, item_, pos_) \
	do { \
		size_t len_; \
		size_t i_; \
		len_ = (array_).len; \
		marrreserve((array_), len_ + 1); \
		assert((pos_) <= len_); \
		for (i_ = len_; i_ >= (pos_) ; i_ -= 1) { \
			(array_).items[i_] = (array_).items[i_ - 1]; \
		} \
		(array_).items[(pos_)] = (item_); \
		(array_).len += 1; \
	} while (0)

/** @def marrpop(array)
 *  @brief Decrement length (remove last element, no memory free). */
#define marrpop(array_) \
	do { \
		if ((array_).len == 0) break; \
		(array_).len -= 1; \
	} while (0)

/** @def marrreserve(array, min_cap)
 *  @brief Ensure at least `min_cap` capacity. */
#define marrreserve(array_, min_cap_) \
	do { \
		if ((array_).cap >= (min_cap_)) {	\
			break; \
		} \
		size_t new_cap_ = (array_).cap ? (array_).cap : INITIAL_CAP; \
		while (new_cap_ < (min_cap_)) { \
			new_cap_ *= (GROW_FACTOR);			  \
		} \
		(array_).items = xrecreate((array_).allocator, (array_).cap * sizeof(*(array_).items), new_cap_ * sizeof(*(array_).items), (array_).items); \
		(array_).cap = new_cap_; \
	} while (0)

/** @def arrlen(array)
 *  @brief Get the current length. */
#define arrlen(array_) ((array_).len)
/** @def arrcap(array)
 *  @brief Get the current capacity (NOTE: currently same as len). */
#define arrcap(array_) ((array_).len)
/** @def arrget(array, index)
 *  @brief Get the element at `index`. */
#define arrget(array_, index_) ((array_).items[index_])


#endif /* !MY_MANAGED_ARRAY_H_ */


/* ===== my_hashtable ===== */
#ifndef MY_HASHTABLE_H_
#define MY_HASHTABLE_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef MY_HASHTABLE_DEF
#  define MY_HASHTABLE_DEF CMYLIB_DEF
#endif

/* #define MY_HASHTABLE_IMPL */

#ifndef MY_HASHTABLE_CAPACITY
#  define MY_HASHTABLE_CAPACITY 16
#endif

#ifndef HM_LOAD_FACTOR_NUM
#  define HM_LOAD_FACTOR_NUM 7
#endif

#ifndef HM_LOAD_FACTOR_DEN
#  define HM_LOAD_FACTOR_DEN 10
#endif

/** @brief Hash function signature. Receives `const void *` pointing to a full item. Returns `size_t`. */
typedef size_t (*hash_fn_t)(const void *item);

/** @brief Equality function signature. Receives two `const void *` pointing to full items. Returns nonzero if equal. */
typedef int    (*eq_fn_t)(const void *a, const void *b);

#ifndef HM_KEY_OFFSET
/** Byte offset of the key field within the item struct (default 0 for `char *` as first member).
 *  Only used by the built-in default hash/eq functions. */
#  define HM_KEY_OFFSET 0
#endif

/* --- internal bookkeeping --- */

typedef struct hm_header {
	size_t item_size;
	size_t key_offset;
	hash_fn_t hash_fn;
	eq_fn_t   eq_fn;
} hm_header_t;

#define _HM_OCC(items_, cap_, i_) (((uint8_t *)(items_) - (cap_) * 2)[(i_)])
#define _HM_DIST(items_, cap_, i_) (((uint8_t *)(items_) - (cap_))[(i_)])
#define _HM_HDR(items_, cap_) ((hm_header_t *)((char *)(items_) - (cap_) * 2 - sizeof(hm_header_t)))

/* --- forward declarations --- */

MY_HASHTABLE_DEF size_t _hm_hash_str(const char *str);
MY_HASHTABLE_DEF void _hm_create(allocator_t allocator, void **items_ptr, size_t *cap_ptr, size_t item_size, hash_fn_t hash_fn, eq_fn_t eq_fn);
MY_HASHTABLE_DEF void _hm_free(allocator_t allocator, void *items, size_t cap, size_t item_size);
MY_HASHTABLE_DEF void _hm_grow(allocator_t allocator, void **items_ptr, size_t *len_ptr, size_t *cap_ptr, size_t item_size);
MY_HASHTABLE_DEF void _hm_put_impl(allocator_t allocator, void **items_ptr, size_t *len_ptr, size_t *cap_ptr, size_t item_size, void *item);
MY_HASHTABLE_DEF void *_hm_get_impl(void *items, size_t len, size_t cap, size_t item_size, void *key_item);
MY_HASHTABLE_DEF void _hm_del_impl(void *items, size_t *len_ptr, size_t cap, size_t item_size, void *key_item);

/* --- public macros --- */

/** Initialize the hash table with default capacity and default hash/eq functions. */
#define hmcreate(allocator_, hm_) \
	_hm_create((allocator_), (void **)&(hm_).items, &(hm_).cap, sizeof(*(hm_).items), NULL, NULL)

/** Initialize the hash table with custom hash and equality functions.
 *  @param allocator_  The allocator.
 *  @param hm_         The hash table struct (must have .items, .len, .cap).
 *  @param hash_fn_    Hash function (or NULL for default).
 *  @param eq_fn_      Equality function (or NULL for default). */
#define hminit(allocator_, hm_, hash_fn_, eq_fn_) \
	_hm_create((allocator_), (void **)&(hm_).items, &(hm_).cap, sizeof(*(hm_).items), (hash_fn_), (eq_fn_))

/** Insert or replace an item.
 *  @param allocator_  The allocator.
 *  @param hm_         The hash table struct (must have .items, .len, .cap).
 *  @param item_       Pointer to the item to insert (C89) or ... (C99).
 *
 *  In C99 you may pass a bare compound literal: `hmput(al, m, &(type){...})`.
 *  In C89 wrap it in parens or use a variable:   `hmput(al, m, (&(type){...}))`. */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define hmput(allocator_, hm_, ...) \
	_hm_put_impl((allocator_), (void **)&(hm_).items, &(hm_).len, &(hm_).cap, sizeof(*(hm_).items), (void *)(__VA_ARGS__))
#else
#  define hmput(allocator_, hm_, item_) \
	_hm_put_impl((allocator_), (void **)&(hm_).items, &(hm_).len, &(hm_).cap, sizeof(*(hm_).items), (void *)(item_))
#endif

/** Look up an item by key.
 *  @param hm_          The hash table struct.
 *  @param key_item_    Pointer to an item with the key field set (only key is read).
 *  @return Pointer to the stored item, or NULL. */
#define hmget(hm_, key_item_) \
	_hm_get_impl((hm_).items, (hm_).len, (hm_).cap, sizeof(*(hm_).items), (void *)(key_item_))

/** Delete an item by key.
 *  @param hm_          The hash table struct.
 *  @param key_item_    Pointer to an item with the key field set. */
#define hmdel(hm_, key_item_) \
	_hm_del_impl((hm_).items, &(hm_).len, (hm_).cap, sizeof(*(hm_).items), (void *)(key_item_))

/** Free the entire hash table and reset to zero. */
#define hmfree(allocator_, hm_) \
	do { _hm_free((allocator_), (hm_).items, (hm_).cap, sizeof(*(hm_).items)); (hm_).items = NULL; (hm_).len = 0; (hm_).cap = 0; } while (0)

/** Current capacity (number of slots). */
#define hmcap(hm_) ((hm_).cap)

/** Number of occupied slots. */
#define hmlen(hm_) ((hm_).len)

#ifndef hmforeach
/** Iterate over occupied slots.  `i_` is a `size_t` variable. */
#  define hmforeach(hm_, i_) \
	for ((i_) = 0; (i_) < hmcap(hm_); (i_)++) \
		if (_HM_OCC((hm_).items, (hm_).cap, (i_)))
#endif /* !hmforeach */

/** Check if slot `i_` is occupied. */
#define hmoccupied(hm_, i_) (_HM_OCC((hm_).items, (hm_).cap, (i_)) != 0)

/* ====================================================================
 *  Implementation
 * ==================================================================== */
#endif /* !MY_HASHTABLE_H_ */


#if defined(CMYLIB_IMPL) || defined(MY_HASHTABLE_IMPL)

#ifndef MY_HASHTABLE_DEF
#  error "MY_HASHTABLE_DEF must be defined before including implementation"
#endif

/* FNV-1a hash for null-terminated strings. */
MY_HASHTABLE_DEF size_t _hm_hash_str(const char *str)
{
	size_t h = 1469598103934665603ULL;
	while (*str != '\0') {
		h ^= (unsigned char)*str++;
		h *= 1099511628211ULL;
	}
	return h;
}

/** @brief Default hash: reads a `const char *` key from the item at HM_KEY_OFFSET, hashes with FNV-1a. */
static size_t _hm_default_hash(const void *item) {
	return _hm_hash_str(*(const char **)((const char *)item + HM_KEY_OFFSET));
}

/** @brief Default equality: compares two `const char *` keys at HM_KEY_OFFSET via strcmp. */
static int _hm_default_eq(const void *a, const void *b) {
	return strcmp(*(const char **)((const char *)a + HM_KEY_OFFSET),
	             *(const char **)((const char *)b + HM_KEY_OFFSET)) == 0;
}

MY_HASHTABLE_DEF void _hm_create(allocator_t allocator, void **items_ptr,
                                  size_t *cap_ptr, size_t item_size,
                                  hash_fn_t hash_fn, eq_fn_t eq_fn)
{
	size_t cap;
	size_t total;
	uint8_t *mem;
	hm_header_t *hdr;

	cap = MY_HASHTABLE_CAPACITY;
	total = sizeof(hm_header_t) + cap * 2 + cap * item_size;
	mem = (uint8_t *)alloc(allocator, total);
	memset(mem, 0, total);
	hdr = (hm_header_t *)mem;
	hdr->item_size = item_size;
	hdr->key_offset = HM_KEY_OFFSET;
	hdr->hash_fn = hash_fn ? hash_fn : _hm_default_hash;
	hdr->eq_fn   = eq_fn   ? eq_fn   : _hm_default_eq;
	*items_ptr = (void *)(mem + sizeof(hm_header_t) + cap * 2);
	*cap_ptr = cap;
}

MY_HASHTABLE_DEF void _hm_free(allocator_t allocator, void *items,
                                size_t cap, size_t item_size)
{
	hm_header_t *hdr;
	size_t total;

	if (!items) return;
	hdr = _HM_HDR(items, cap);
	total = sizeof(hm_header_t) + cap * 2 + cap * item_size;
	xdestroy(allocator, total, hdr);
}

MY_HASHTABLE_DEF void _hm_grow(allocator_t allocator, void **items_ptr,
                                size_t *len_ptr, size_t *cap_ptr,
                                size_t item_size)
{
	void *old_items;
	size_t old_cap, new_cap, total, i;
	uint8_t *mem;
	hm_header_t *hdr, *old_hdr;
	void *new_items;
	hash_fn_t old_hash;
	eq_fn_t   old_eq;

	old_items = *items_ptr;
	old_cap = *cap_ptr;
	old_hdr = _HM_HDR(old_items, old_cap);
	old_hash = old_hdr->hash_fn;
	old_eq   = old_hdr->eq_fn;

	new_cap = old_cap ? old_cap * 2 : MY_HASHTABLE_CAPACITY;
	total = sizeof(hm_header_t) + new_cap * 2 + new_cap * item_size;
	mem = (uint8_t *)alloc(allocator, total);
	memset(mem, 0, total);
	hdr = (hm_header_t *)mem;
	hdr->item_size = item_size;
	hdr->key_offset = old_hdr->key_offset;
	hdr->hash_fn = old_hash;
	hdr->eq_fn   = old_eq;
	new_items = (void *)(mem + sizeof(hm_header_t) + new_cap * 2);
	*items_ptr = new_items;
	*cap_ptr = new_cap;
	*len_ptr = 0;

	for (i = 0; i < old_cap; i++) {
		if (_HM_OCC(old_items, old_cap, i)) {
			_hm_put_impl(allocator, items_ptr, len_ptr, cap_ptr,
			             item_size,
			             (char *)old_items + i * item_size);
		}
	}

	_hm_free(allocator, old_items, old_cap, item_size);
}

MY_HASHTABLE_DEF void _hm_put_impl(allocator_t allocator, void **items_ptr,
                                    size_t *len_ptr, size_t *cap_ptr,
                                    size_t item_size, void *item)
{
	void *items;
	size_t cap, h, pos, dist, i;
	void *buf;

	if (*items_ptr == NULL) {
		_hm_create(allocator, items_ptr, cap_ptr, item_size, NULL, NULL);
	}

	if (*len_ptr * HM_LOAD_FACTOR_DEN >= *cap_ptr * HM_LOAD_FACTOR_NUM) {
		_hm_grow(allocator, items_ptr, len_ptr, cap_ptr, item_size);
	}

	items = *items_ptr;
	cap = *cap_ptr;

	{
		hm_header_t *hdr_ = _HM_HDR(items, cap);
		h = hdr_->hash_fn(item);
	}
	pos = h % cap;
	dist = 0;

	buf = alloc(allocator, item_size);
	memcpy(buf, item, item_size);

	while (1) {
		if (!_HM_OCC(items, cap, pos)) {
			memcpy((char *)items + pos * item_size, buf, item_size);
			_HM_OCC(items, cap, pos) = 1;
			_HM_DIST(items, cap, pos) = (uint8_t)dist;
			(*len_ptr)++;
			goto done;
		}

		{
			hm_header_t *hdr_ = _HM_HDR(items, cap);
			if (hdr_->eq_fn((char *)items + pos * item_size, buf)) {
				memcpy((char *)items + pos * item_size, buf, item_size);
				goto done;
			}
		}

		if (_HM_DIST(items, cap, pos) < dist) {
			uint8_t tmp_dist;
			char *slot;

			tmp_dist = _HM_DIST(items, cap, pos);
			_HM_DIST(items, cap, pos) = (uint8_t)dist;
			dist = tmp_dist;

			slot = (char *)items + pos * item_size;
			for (i = 0; i < item_size; i++) {
				char t = ((char *)buf)[i];
				((char *)buf)[i] = slot[i];
				slot[i] = t;
			}
		}

		pos = (pos + 1) % cap;
		dist++;
	}

done:
	xdestroy(allocator, item_size, buf);
}

MY_HASHTABLE_DEF void *_hm_get_impl(void *items, size_t len, size_t cap,
                                     size_t item_size, void *key_item)
{
	size_t h, pos, dist;
	hm_header_t *hdr;

	(void)len;
	if (!items || !cap) return NULL;

	hdr = _HM_HDR(items, cap);
	h = hdr->hash_fn(key_item);
	pos = h % cap;
	dist = 0;

	while (1) {
		if (!_HM_OCC(items, cap, pos)) return NULL;
		if (_HM_DIST(items, cap, pos) < dist) return NULL;

		if (hdr->eq_fn((char *)items + pos * item_size, key_item)) {
			return (char *)items + pos * item_size;
		}

		pos = (pos + 1) % cap;
		dist++;
	}
}

MY_HASHTABLE_DEF void _hm_del_impl(void *items, size_t *len_ptr,
                                    size_t cap, size_t item_size,
                                    void *key_item)
{
	size_t h, pos, dist, cur, next;
	hm_header_t *hdr;

	if (!items || !cap) return;

	hdr = _HM_HDR(items, cap);
	h = hdr->hash_fn(key_item);
	pos = h % cap;
	dist = 0;

	while (1) {
		if (!_HM_OCC(items, cap, pos)) return;
		if (_HM_DIST(items, cap, pos) < dist) return;

		if (hdr->eq_fn((char *)items + pos * item_size, key_item)) {
			break;
		}

		pos = (pos + 1) % cap;
		dist++;
	}

	_HM_OCC(items, cap, pos) = 0;
	_HM_DIST(items, cap, pos) = 0;

	cur = pos;
	next = (cur + 1) % cap;

	while (_HM_OCC(items, cap, next) && _HM_DIST(items, cap, next) > 0) {
		memcpy((char *)items + cur * item_size,
		       (char *)items + next * item_size,
		       item_size);
		_HM_OCC(items, cap, cur) = 1;
		_HM_DIST(items, cap, cur) = _HM_DIST(items, cap, next) - 1;

		_HM_OCC(items, cap, next) = 0;
		_HM_DIST(items, cap, next) = 0;

		cur = next;
		next = (next + 1) % cap;
	}

	(*len_ptr)--;
}

#endif /* !MY_HASHTABLE_IMPL */

/* ===== my_string_builder ===== */
#ifndef MY_STRING_BUILDER_H_
#define MY_STRING_BUILDER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef MY_SB_DEF
#  define MY_SB_DEF CMYLIB_DEF
#endif

#ifndef GROW_FACTOR
/** @def GROW_FACTOR
 *  @brief Capacity multiplier when growing (overridable before inclusion). */
#  define GROW_FACTOR 2
#endif /* !GROW_FACTOR */

/**
 * @brief A dynamic, heap-allocated, null-terminated string builder.
 *
 * Allocates and owns its buffer via the provided `allocator_t`.
 */
typedef struct string_builder_t {
	allocator_t allocator; /**< Allocator used for all buffer management. */
	char   *data;          /**< Null-terminated character buffer. */
	size_t  len;           /**< Current string length (excluding null terminator). */
	size_t  cap;           /**< Allocated capacity (excluding null terminator). */
} string_builder_t;

/** @def sb_push(self, target)
 *  @brief Append a value to the string builder (type-dispatched).
 *  @param self    Pointer to a `string_builder_t`.
 *  @param target  A `char`, `char *`, or `string_builder_t *` to append. */
#define sb_push(self, target) \
	_Generic((target), \
			char: sb_push_char, \
			char *: sb_push_cstr, \
			string_builder_t *: sb_push_string)(self, target)

/** @def sb(allocator_, x)
 *  @brief Convenience shorthand for `sb_from_chars_copy`. */
#define sb(allocator_, x) sb_from_chars_copy((allocator_), (x))

/** @brief Create a new empty string builder with the given initial capacity.
 *  @param allocator  The allocator to use.
 *  @param cap        Initial capacity (excluding null terminator).
 *  @return A new `string_builder_t` owning a zero-initialised buffer. */
MY_SB_DEF string_builder_t sb_new(allocator_t allocator, size_t cap);

/** @brief Create a string builder by copying a C string.
 *  @param allocator  The allocator to use.
 *  @param chs        Null-terminated source string (copied).
 *  @return A new `string_builder_t` owning a copy of `chs`. */
MY_SB_DEF string_builder_t sb_from_chars_copy(allocator_t allocator, const char *  chs);

/** @brief Shrink the allocated buffer and return `string_t`
 * @param self Pointer to the string builder
 * @returns a `string_t` of the built string */
MY_SB_DEF string_t sb_build(string_builder_t *self);

/** @brief Shrink the allocated buffer and return `string_view_t`
 * @param self Pointer to the string builder
 * @returns a `string_t` of the built string */
MY_SB_DEF string_view_t sb_build_view(string_builder_t *self);

/** @brief Create a string builder from a printf-style format string.
 *  @param allocator  The allocator to use.
 *  @param fmt        Printf-style format string.
 *  @param ...        Format arguments.
 *  @return A new `string_builder_t` containing the formatted output.
 *  @note `setup_io_stream()` must be called before using this function. */
MY_SB_DEF string_builder_t sb_format(allocator_t allocator, const char *  fmt, ...);

/** @brief Free the string builder's buffer and reset its fields.
 *  @param self  Pointer to the string builder. */
MY_SB_DEF void sb_delete(string_builder_t *  self);

/** @brief Manually set the length of the string.
 *  @param self  Pointer to the string builder.
 *  @param len   New length (must be <= `cap`).
 *  @return The new length. */
MY_SB_DEF size_t sb_set_len(string_builder_t *self, size_t len);

/** @brief Ensure the buffer can hold at least `new_cap` characters (plus null terminator).
 *  @param self     Pointer to the string builder.
 *  @param new_cap  Desired capacity. */
MY_SB_DEF void sb_reserve(string_builder_t *  self, size_t new_cap);

/** @brief Resize the string to `new_size`, zero-filling any new bytes.
 *  @param self      Pointer to the string builder.
 *  @param new_size  New size (may be larger or smaller than current length). */
MY_SB_DEF void sb_resize(string_builder_t *  self, size_t new_size);

/** @brief Append formatted text to the string builder.
 *  @param self  Pointer to the string builder.
 *  @param fmt   Printf-style format string.
 *  @param ...   Format arguments.
 *  @note `setup_io_stream()` must be called before using this function. */
MY_SB_DEF void sb_pushf(string_builder_t * self, const char *  fmt, ...);

/** @brief Append a single character.
 *  @param self  Pointer to the string builder.
 *  @param ch    The character to append. */
MY_SB_DEF void sb_push_char(string_builder_t *  self, char ch);

/** @brief Append a null-terminated C string.
 *  @param self  Pointer to the string builder.
 *  @param cstr  The C string to append (copied). */
MY_SB_DEF void sb_push_cstr(string_builder_t *  self, const char *  cstr);

/** @brief Append the contents of another string builder.
 *  @param self   Pointer to the destination.
 *  @param other  Pointer to the source string builder. */
MY_SB_DEF void sb_push_string(string_builder_t *  self, const string_builder_t *  other);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !MY_STRING_BUILDER_H_ */

#if defined(CMYLIB_IMPL) || defined(MY_STRING_BUILDER_IMPL)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief Return the capacity including the null terminator slot.
 *  @param self  Pointer to the string builder.
 *  @return `self->cap + 1`. */
MY_SB_DEF size_t sb_actual_cap(const string_builder_t *  self)
{
	return self->cap + 1;
}

/** @brief Return the length including the null terminator.
 *  @param self  Pointer to the string builder.
 *  @return `self->len + 1`. */
MY_SB_DEF size_t sb_actual_len(const string_builder_t *  self)
{
	return self->len + 1;
}

MY_SB_DEF string_builder_t sb_from_chars_copy(allocator_t allocator, const char *  chs)
{
	string_builder_t result = {0};
	result.allocator = allocator;
	result.len = strlen(chs);
	result.data = alloc(allocator, result.len + 1);
	memcpy(result.data, chs, result.len + 1);
	return result;
}

MY_SB_DEF string_builder_t sb_new(allocator_t allocator, size_t cap)
{
	string_builder_t result = {0};
	result.allocator = allocator;
	result.cap = cap;
	result.data = alloc(allocator, sizeof(char) * cap + 1);
	result.len = 0;

	memset(result.data, 0, sizeof(char) * result.cap + 1);
	return result;
}

MY_SB_DEF size_t sb_set_len(string_builder_t *self, size_t len)
{
	self->len = len;
	self->data[len] = '\0';
	return self->len;
}

MY_SB_DEF string_t sb_build(string_builder_t *self)
{
	string_t result = {0};

	self->data = xrecreate(self->allocator, self->cap + 1, self->len + 1, self->data);
	self->cap = self->len;
	result.data = self->data;
	result.len = self->len;
	return result;
}

MY_SB_DEF string_view_t sb_build_view(string_builder_t *self)
{
	string_view_t result = {0};

	self->data = xrecreate(self->allocator, self->cap + 1, self->len + 1, self->data);
	self->cap = self->len;
	result.data = self->data;
	result.len = self->len;
	return result;
}

MY_SB_DEF string_builder_t sb_format(allocator_t allocator, const char *  fmt, ...)
{
	va_list args;
	int len = 0;
	string_builder_t result = {0};

	va_start(args, fmt);
	len = vsnsprint(NULL, 0, fmt, args);
	va_end(args);

	result = sb_new(allocator, len);

	va_start(args, fmt);
	vsnsprint(result.data, len + 1, fmt, args);
	va_end(args);

	result.len = len;
	return result;
}

MY_SB_DEF void sb_reserve(string_builder_t *  self, size_t new_cap)
{
	self->data = xrecreate(self->allocator, self->cap + 1, new_cap + 1, self->data);
	self->cap = new_cap;
}

MY_SB_DEF void sb_resize(string_builder_t *  self, size_t new_size)
{
	if (self->cap < new_size) {
		self->data = xrecreate(self->allocator, self->cap + 1, new_size + 1, self->data);
	}

	if (self->len < new_size) {
		memset(self->data + self->len, 0, new_size - self->len);
	}

	self->len = new_size;
	self->data[self->len] = '\0';
}

MY_SB_DEF void sb_delete(string_builder_t *  self)
{
	xdestroy(self->allocator, self->cap + 1, self->data);
	self->len = 0;
	self->cap = 0;
	self->data = NULL;
}

MY_SB_DEF void sb_push_char(string_builder_t *  self, char ch)
{
	if (self->len + 1 >= self->cap) {
		sb_reserve(self, self->cap * GROW_FACTOR);
	}

	self->data[self->len++] = ch;
	self->data[self->len] = '\0';
}

MY_SB_DEF void sb_push_cstr(string_builder_t *  self, const char *  cstr)
{
	size_t len = strlen(cstr);
	if (self->len + len >= self->cap) {
		size_t newcap = self->cap * GROW_FACTOR;
		if (self->len + len > newcap) {
			newcap = self->len + len;
		}
		sb_reserve(self, newcap);
	}

	memcpy(self->data + self->len, cstr, len);
	self->len += len;
	self->data[self->len] = '\0';
}

MY_SB_DEF void sb_push_string(string_builder_t *  self, const string_builder_t *  other)
{
	if (self->len + other->len >= self->cap) {
		size_t newcap = self->cap * GROW_FACTOR;
		if (self->len + other->len > newcap) {
			newcap = self->len + other->len;
		}
		sb_reserve(self, newcap);
	}

	memcpy(self->data + self->len, other->data, other->len);
	self->len += other->len;
	self->data[self->len] = '\0';
}

MY_SB_DEF void sb_pushf(string_builder_t *  self, const char *  fmt, ...)
{
	va_list args;

	int len;

	va_start(args, fmt);
	len = vsnsprint(NULL, 0, fmt, args);
	va_end(args);

	if (self->len + len > self->cap) {
		size_t new_cap = self->cap * GROW_FACTOR;
		if (new_cap < self->len + len) new_cap = self->len + len;
		sb_reserve(self, new_cap);
	}

	va_start(args, fmt);
	vsnsprint(self->data + self->len, len + 1, fmt, args);
	va_end(args);

	self->len += len;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !CMYLIB_IMPL && !MY_STRING_BUILDER_IMPL */

