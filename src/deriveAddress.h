#ifndef __SPACEMESH_APP_DERIVE_ADDRESS_H__
#define __SPACEMESH_APP_DERIVE_ADDRESS_H__

#include "common.h"
#include "bip44.h"

void deriveAddress(
    const bip44_path_t* pathSpec,
    uint8_t* outBuffer, size_t outSize
);

#endif // __SPACEMESH_APP_DERIVE_ADDRESS_H__
