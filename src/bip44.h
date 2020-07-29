#ifndef __SPACEMESH_APP_BIP44_H__
#define __SPACEMESH_APP_BIP44_H__

#include "common.h"

/*
Spacemesh BIP32 paths

m / 44' / coin_type / [account] / 0 / [address_index]`

    Account should be constant 0 for now. May be used in the future for additional functionality.
    Coin type is 1 for a Spacemesh Testnet and 540 for Spacemesh mainent.
    The first mainent account should be at path m / 44' / 540' / 0' / 0 / 0'.
    The second mainent account should be at path m / 44' / 540' / 0' / 0 / 1'.
    The first testnet account should be at path `m / 44' / 1' / 0' / 0 / 0'.
    The second testnet account should be at path `m / 44' / 1' / 0' / 0 / 1'.
    We have registered to have 540 for Spacemesh. see: satoshilabs/slips#943
    address_index is int32 so there can be up to 2^32 addresses per wallet.
    All path components should be hardened, indicated by an apostrophe char ' per bip32.
*/

static const uint32_t BIP44_MAX_PATH_LENGTH = 5;

typedef struct {
    uint32_t path[BIP44_MAX_PATH_LENGTH];
    uint32_t length;
} bip44_path_t;


static const uint32_t BIP_44 = 44;
static const uint32_t SMESH_COIN_TYPE = 540;

static const uint32_t HARDENED_BIP32 = ((uint32_t) 1 << 31);

size_t bip44_parse(
    bip44_path_t* pathSpec,
    const uint8_t* dataBuffer, size_t dataSize
);

// Indexes into pathSpec
enum {
    BIP44_I_PURPOSE = 0,
    BIP44_I_COIN_TYPE = 1,
    BIP44_I_ACCOUNT = 2,
    BIP44_I_CHAIN = 3,
    BIP44_I_ADDRESS = 4,
    BIP44_I_REST = 5,
};

// Checks for /44'/540'/0'/0
bool bip44_hasValidSpacemeshPrefix(const bip44_path_t* pathSpec);

bool bip44_containsAccount(const bip44_path_t* pathSpec);
bool bip44_hasReasonableAccount(const bip44_path_t* pathSpec);

bool bip44_containsChainType(const bip44_path_t* pathSpec);
bool bip44_hasValidChainType(const bip44_path_t* pathSpec);

bool bip44_containsAddress(const bip44_path_t* pathSpec);
bool bip44_hasReasonableAddress(const bip44_path_t* pathSpec);

bool bip44_containsMoreThanAddress(const bip44_path_t* pathSpec);

bool isHardened(uint32_t value);

void bip44_printToStr(const bip44_path_t*, char* out, size_t outSize);

#endif // __SPACEMESH_APP_BIP44_H__
