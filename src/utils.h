#ifndef __SPACEMESH_APP_UTILS_H__
#define __SPACEMESH_APP_UTILS_H__

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

#endif // __SPACEMESH_APP_UTILS_H__
