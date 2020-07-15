#include "common.h"
#include "getAddress.h"
#include "keyDerivation.h"
#include "spacemesh.h"

void deriveAddress(
    const bip44_path_t* pathSpec,
    uint8_t* outBuffer, size_t outSize
)
{
    uint8_t rawAddressBuffer[40];
    extendedPublicKey_t extPubKey;

    ASSERT(outSize >= SPACEMESH_ADDRESS_SIZE);

    deriveExtendedPublicKey(pathSpec, &extPubKey);

    os_memmove(outBuffer, extPubKey.pubKey, SPACEMESH_ADDRESS_SIZE);
}
