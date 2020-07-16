#ifndef __SPACEMESH_APP_SIGN_MESSAGE_H__
#define __SPACEMESH_APP_SIGN_MESSAGE_H__

#include "bip44.h"

void signMessage(bip44_path_t* pathSpec,
    const uint8_t* message, size_t messageSize,
    uint8_t* signature, size_t signatureSize,
    uint8_t* signer, size_t signerSize);

#endif // __SPACEMESH_APP_SIGN_MESSAGE_H__
