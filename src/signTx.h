#ifndef __SPACEMESH_APP_SIGN_TX_H__
#define __SPACEMESH_APP_SIGN_TX_H__

#include "common.h"
#include "handlers.h"
#include "bip44.h"
#include "spacemesh.h"
#include "keyDerivation.h"

typedef enum {
    SIGN_STAGE_NONE = 0,
    SIGN_STAGE_CONFIRM = 42,
} sign_tx_stage_t;

typedef enum {
    TX_TYPE_COIN      = 0x00, // coin transaction with ed
    TX_TYPE_COIN_EXT  = 0x01, // coin transaction with ed++
    TX_TYPE_APP       = 0x02, // exec app transaction with ed
    TX_TYPE_APP_EXT   = 0x03, // exec app transaction with ed++
    TX_TYPE_SPAWN     = 0x04, // spawn app + ed
    TX_TYPE_SPAWN_EXT = 0x05, // spawn app + ed++
} tx_type_t;

typedef struct {
    uint8_t  type;
    uint64_t nonce; // Account Nonce
    uint8_t  recipient[SPACEMESH_ADDRESS_SIZE];
    uint64_t gasLimit;
    uint64_t gasPrice;
    uint64_t amount;
} tx_header_t;

#define  SPACEMESH_TX_MIN_SIZE (1 + 20 + 4*8)

typedef struct {
    sign_tx_stage_t stage;

    tx_header_t  tx;
    struct {
        uint8_t      signature[64];
        uint8_t      pubkey[PUBLIC_KEY_SIZE];
    } response;
    bip44_path_t signerPath;
    int ui_step;
} ins_sign_tx_context_t;

handler_fn_t signTx_handleAPDU;

#endif
