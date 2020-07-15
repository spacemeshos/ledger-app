#ifndef __SPACEMESH_APP_UTILS_H__
#define __SPACEMESH_APP_UTILS_H__

#include <stdint.h>
#include <stddef.h>

// Does not compile if x is pointer of some kind
// See http://zubplot.blogspot.com/2015/01/gcc-is-wonderful-better-arraysize-macro.html
#define ARRAY_NOT_A_PTR(x) \
    (sizeof(__typeof__(int[1 - 2 * \
    !!__builtin_types_compatible_p(__typeof__(x), \
    __typeof__(&x[0]))])) * 0)

// Safe array length, does not compile if you accidentally supply a pointer
#define ARRAY_LEN(arr) \
    (sizeof(arr) / sizeof((arr)[0]) + ARRAY_NOT_A_PTR(arr))

// Does not compile if x *might* be a pointer of some kind
// Might produce false positives on small structs...
// Note: ARRAY_NOT_A_PTR does not compile if arg is a struct so this is a workaround
#define SIZEOF_NOT_A_PTR(var) \
    (sizeof(__typeof(int[0 - (sizeof(var) == sizeof((void *)0))])) * 0)

// Safe version of SIZEOF, does not compile if you accidentally supply a pointer
#define SIZEOF(var) \
    (sizeof(var) + SIZEOF_NOT_A_PTR(var))

// Any buffer claiming to be longer than this is a bug
// (we anyway have only 4KB of memory)
#define BUFFER_SIZE_PARANOIA 1024

#define PTR_PIC(ptr) ((__typeof__(ptr)) PIC(ptr))

// from https://stackoverflow.com/questions/19343205/c-concatenating-file-and-line-macros
#define _TO_STR1_(x) #x
#define _TO_STR2_(x) _TO_STR1_(x)

// *INDENT-OFF*

// Warning: Following macros are *NOT* brace-balanced by design!
// The macros simplify writing resumable logic that needs to happen over
// multiple calls.

// Example usage:
// UI_STEP_BEGIN(ctx->ui_step);
// UI_STEP(1) {do something & setup callback}
// UI_STEP(2) {do something & setup callback}
// UI_STEP_END(-1); // invalid state

#define UI_STEP_BEGIN(VAR) \
	{ \
		int* __ui_step_ptr = &(VAR); \
		switch(*__ui_step_ptr) { \
			default: { \
				ASSERT(false);

#define UI_STEP(NEXT_STEP) \
				*__ui_step_ptr = NEXT_STEP; \
				break; \
			} \
			case NEXT_STEP: {

#define UI_STEP_END(INVALID_STEP) \
				*__ui_step_ptr = INVALID_STEP; \
				break; \
			} \
		} \
	}

// Early exit to another state, unused for now
// #define UI_STEP_JUMP(NEXT_STEP) *__ui_step_ptr = NEXT_STEP; break;

// *INDENT-ON*

// bin2hex converts binary to hex and appends a final NUL byte.
void bin2hex(char *dst, uint8_t *data, uint64_t inlen);

// bin2dec converts an unsigned integer to a decimal string and appends a
// final NUL byte. It returns the length of the string.
int bin2dec(char *dst, uint64_t n);

#endif // __SPACEMESH_APP_UTILS_H__
