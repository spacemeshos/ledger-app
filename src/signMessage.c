#include "utils.h"
#include "assert.h"
#include "signMessage.h"
#include "endian.h"
#include "bip44.h"
#include "keyDerivation.h"
#include "spacemesh.h"

void signMessage(bip44_path_t* pathSpec,
    const uint8_t* message, size_t messageSize,
    uint8_t* signature, size_t signatureSize,
    uint8_t* signer, size_t signerSize)
{
    chain_code_t chainCode;
    privateKey_t privateKey;
    cx_ecfp_public_key_t publicKey;
    uint8_t pubKey[PUBLIC_KEY_SIZE];

    ASSERT(messageSize < BUFFER_SIZE_PARANOIA);
    ASSERT(signatureSize == 64);
    ASSERT(signerSize == PUBLIC_KEY_SIZE);

    BEGIN_TRY {
        TRY {
            TRACE("derive private key");
            derivePrivateKey(pathSpec, &chainCode, &privateKey);

            getPublicKey(&privateKey, &publicKey);

            STATIC_ASSERT(SIZEOF(pubKey) == PUBLIC_KEY_SIZE, "bad pub key size");

            extractRawPublicKey(&publicKey, pubKey, SIZEOF(pubKey));

            io_seproxyhal_io_heartbeat();
            cx_eddsa_sign(
                (const struct cx_ecfp_256_private_key_s*) &privateKey,
                0 /* mode */,
                CX_SHA512,
                message, messageSize,
                NULL /* ctx */, 0 /* ctx len */,
                signature, 64,
                0 /* info */
            );
            io_seproxyhal_io_heartbeat();

            os_memmove(signer, pubKey, signerSize);
        }
        FINALLY {
            os_memset(&privateKey, 0, SIZEOF(privateKey));
        }
    } END_TRY;
}
