#ifndef __SPACEMESH_APP_COMMON_H__
#define __SPACEMESH_APP_COMMON_H__

// General libraries
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

// BOLOS
#include <os.h>
#include <os_io_seproxyhal.h>

// ours

// Note: unused removes unused warning but does not warn if you suddenly
// start using such variable. deprecated deals with that.
#define MARK_UNUSED __attribute__ ((unused, deprecated))

#include "utils.h"
#include "assert.h"
#include "io.h"
#include "ux.h"
#include "errors.h"

#endif // __SPACEMESH_APP_COMMON_H__
