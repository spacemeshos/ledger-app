#include <os_io_seproxyhal.h>
#include <stdlib.h>

#include "errors.h"
#include "handlers.h"
#include "getVersion.h"
#include "getExtendedPublicKey.h"
#include "getAddress.h"
//#include "signTx.h"

// The APDU protocol uses a single-byte instruction code (INS) to specify
// which command should be executed. We'll use this code to dispatch on a
// table of function pointers.
handler_fn_t* lookupHandler(uint8_t ins)
{
    switch (ins) {
#   define  CASE(INS, HANDLER) case INS: return HANDLER;
        // 0x0* -  app status calls
        CASE(0x00, getVersion_handleAPDU);

        // 0x1* -  public-key/address related
        CASE(0x10, getExtendedPublicKey_handleAPDU);
        CASE(0x11, getAddress_handleAPDU);

        // 0x2* -  signing-transaction related
//        CASE(0x20, signTx_handleAPDU);
#   undef   CASE
    default:
        return NULL;
    }
}
