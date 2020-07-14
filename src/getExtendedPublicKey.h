#ifndef __SPACEMESH_APP_GET_EXTENDED_PUBLIC_KEY_H__
#define __SPACEMESH_APP_GET_EXTENDED_PUBLIC_KEY_H__

#include "common.h"
#include "handlers.h"
#include "bip44.h"
#include "keyDerivation.h"

handler_fn_t getExtendedPublicKey_handleAPDU;

typedef struct {
	int16_t responseReadyMagic;
	bip44_path_t pathSpec;
	extendedPublicKey_t extPubKey;
	int ui_step;
} ins_get_ext_pubkey_context_t;

#endif // __SPACEMESH_APP_GET_EXTENDED_PUBLIC_KEY_H__
