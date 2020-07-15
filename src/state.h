#ifndef __SPACEMESH_APP_STATE_H__
#define __SPACEMESH_APP_STATE_H__

#include "getVersion.h"
#include "getExtendedPublicKey.h"
#include "getAddress.h"
//#include "signTx.h"

typedef union {
    // Here should go states of all instructions
    ins_get_ext_pubkey_context_t extPubKeyContext;
    ins_get_address_context_t getAddressContext;
//    ins_sign_tx_context_t signTxContext;
} instructionState_t;

// Note(instructions are uint8_t but we have a special INS_NONE value
extern int currentInstruction;

extern instructionState_t instructionState;

#endif // __SPACEMESH_APP_STATE_H__
