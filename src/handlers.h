#ifndef __SPACEMESH_APP_HANDLERS_H__
#define __SPACEMESH_APP_HANDLERS_H__

#include "common.h"

typedef void handler_fn_t(
        uint8_t p1,
        uint8_t p2,
        uint8_t *data,
        size_t dataSize,
        bool isNewCall
);

handler_fn_t* lookupHandler(uint8_t ins);

#endif // __SPACEMESH_APP_HANDLERS_H__
