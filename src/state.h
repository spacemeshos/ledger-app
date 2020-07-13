#ifndef __SPACEMESH_APP_STATE_H__
#define __SPACEMESH_APP_STATE_H__

#include "getVersion.h"
#include "stream.h"
//#include "getPublicKey.h"
//#include "signTx.h"

typedef struct {
	stream_t s;
} ins_tests_context_t;

typedef union {
	// Here should go states of all instructions
//	ins_get_pubkey_context_t extPubKeyContext;
//	ins_sign_tx_context_t signTxContext;
} instructionState_t;

// Note(instructions are uint8_t but we have a special INS_NONE value
extern int currentInstruction;

extern instructionState_t instructionState;

#endif // __SPACEMESH_APP_STATE_H__
