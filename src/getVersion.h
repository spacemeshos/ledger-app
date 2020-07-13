#ifndef __SPACEMESH_APP_GET_VERSION_H__
#define __SPACEMESH_APP_GET_VERSION_H__

#include "handlers.h"
#include "common.h"

// Must be in format x.y.z
#ifndef APPVERSION
#error "Missing -DAPPVERSION=x.y.z in Makefile"
#endif
handler_fn_t getVersion_handleAPDU;

#endif // __SPACEMESH_APP_GET_VERSION_H__
