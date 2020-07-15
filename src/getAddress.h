#ifndef __SPACEMESH_APP_GET_ADDRESS_H__
#define __SPACEMESH_APP_GET_ADDRESS_H__

#include "common.h"
#include "bip44.h"
#include "handlers.h"
#include "spacemesh.h"

handler_fn_t getAddress_handleAPDU;

typedef struct {
    uint16_t responseReadyMagic;
    bip44_path_t pathSpec;
    uint8_t address[SPACEMESH_ADDRESS_SIZE];
    int ui_step;
} ins_get_address_context_t;

#endif // __SPACEMESH_APP_GET_ADDRESS_H__
