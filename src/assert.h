#ifndef __SPACEMESH_APP_ASSERT_H__
#define __SPACEMESH_APP_ASSERT_H__

#include "common.h"

#define ASSERT_TYPE(expr, expected_type) \
    STATIC_ASSERT( \
       __builtin_types_compatible_p(__typeof__((expr)), expected_type), \
       "Wrong type" \
    )

#define STATIC_ASSERT _Static_assert

extern void assert(int cond, const char* msgStr);

#define ASSERT_WITH_MSG(cond, msg) assert(cond, msg)

// Helper function to check APDU request parameters
#define VALIDATE(cond, error) \
	do {\
		if (!(cond)) { \
			PRINTF("Validation Error in %s: %d\n", __FILE__, __LINE__); \
			THROW(error); \
		} \
	} while(0)

#define _MAX_ASSERT_LENGTH_ 25
// Shortens a string literal by skipping some prefix
#define _SHORTEN_(strLiteral, size) \
	(sizeof(strLiteral) > size \
	 ? (strLiteral) + sizeof(strLiteral) - size \
	 : strLiteral \
	)

#define _FILE_LINE_ __FILE__ ":" _TO_STR2_(__LINE__)

#define ASSERT(cond) assert((cond), _SHORTEN_( _FILE_LINE_, _MAX_ASSERT_LENGTH_))

#if DEVEL
#define TRACE(...) \
	do { \
		PRINTF("[%s:%d] ", __func__, __LINE__); \
		PRINTF("" __VA_ARGS__); \
		PRINTF("\n"); \
	} while(0)
#else
#define TRACE(...)
#endif

#endif // __SPACEMESH_APP_ASSERT_H__
