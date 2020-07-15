#ifndef __SPACEMESH_APP_BIP44_H__
#define __SPACEMESH_APP_BIP44_H__

#include "common.h"

static const uint32_t BIP44_MAX_PATH_LENGTH = 10;

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


// Checks for /44'/540'/account'
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
