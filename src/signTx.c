#include "common.h"
#include "signTx.h"
#include "state.h"
#include "keyDerivation.h"
#include "ux.h"
#include "endian.h"
#include "uiHelpers.h"
//#include "textUtils.h"
#include "signMessage.h"
#include "bip44.h"

static ins_sign_tx_context_t* ctx = &(instructionState.signTxContext);

enum {
    TXSIGN_STEP_STEP_WARNING = 100,
    TXSIGN_STEP_DISPLAY_TYPE,
    TXSIGN_STEP_DISPLAY_AMOUNT,
    TXSIGN_STEP_DISPLAY_ADDRESS,
    TXSIGN_STEP_DISPLAY_FEE,
    TXSIGN_STEP_CONFIRM_TX,
    TXSIGN_STEP_DISPLAY_SIGNER,
    TXSIGN_STEP_CONFIRM_SIGNER,
    TXSIGN_STEP_RESPOND,
    TXSIGN_STEP_INVALID,
};

#define WRITE_CHAR(ptr, end, c) \
    { \
        ASSERT(ptr + 1 <= end); \
        *ptr = (c); \
        ptr++; \
    }

size_t str_formatSmeshAmount(
        char* out, size_t outSize,
        uint64_t amount
)
{
    ASSERT(outSize < BUFFER_SIZE_PARANOIA);

    char scratchBuffer[30];
    char* ptr = BEGIN(scratchBuffer);
    char* end = END(scratchBuffer);

    // We print in reverse

    // decimal digits
    int zero = 1;
    for (int dec = 0; dec < 12; dec++) {
        int value = amount % 10;
        if (0 != zero && 0 != value) {
            zero = 0;
        }
        if (0 == zero) {
            WRITE_CHAR(ptr, end, '0' + value);
        }
        amount /= 10;
    }
    if (0 != zero) {
        WRITE_CHAR(ptr, end, '0');
    }
    WRITE_CHAR(ptr, end, '.');
    // We want at least one iteration
    int place = 0;
    do {
        // thousands separator
        if (place && (place % 3 == 0)) {
            WRITE_CHAR(ptr, end, ',');
        }
        WRITE_CHAR(ptr, end, '0' + (amount % 10));
        amount /= 10;
        place++;
    } while (amount > 0);

    // Size without terminating character
    STATIC_ASSERT(sizeof(ptr - scratchBuffer) == sizeof(size_t), "bad size_t size");
    size_t rawSize = (size_t) (ptr - scratchBuffer);

    if (rawSize + 1 > outSize) {
        THROW(ERR_DATA_TOO_LARGE);
    }

    // Copy reversed & append terminator
    for (size_t i = 0; i < rawSize; i++) {
        out[i] = scratchBuffer[rawSize - 1 - i];
    }
    out[rawSize] = 0;

    return rawSize;
}

static void signTx_ui_runStep()
{
    TRACE("step %d", ctx->ui_step);
    ui_callback_fn_t* this_fn = signTx_ui_runStep;

    UI_STEP_BEGIN(ctx->ui_step);

    UI_STEP(TXSIGN_STEP_STEP_WARNING) {
        ui_displayPaginatedText(
            "Warning!",
            "Hosts asks for unusual tx",
            this_fn
        );
    }
    UI_STEP(TXSIGN_STEP_DISPLAY_TYPE) {
        const char *type = "UNKNOWN";
        switch (ctx->tx.type) {
        case TX_TYPE_COIN:
            type = "COIN ED";
            break;
        case TX_TYPE_COIN_EXT:
            type = "COIN ED++";
            break;
        case TX_TYPE_APP:
            type = "EXEC APP ED";
            break;
        case TX_TYPE_APP_EXT:
            type = "EXEC APP ED++";
            break;
        case TX_TYPE_SPAWN:
            type = "SPAWN APP ED";
            break;
        case TX_TYPE_SPAWN_EXT:
            type = "SPAWN APP ED++";
            break;
        }
        ui_displayPaginatedText(
            "Tx type:",
            type,
            this_fn
        );
    }
    UI_STEP(TXSIGN_STEP_DISPLAY_AMOUNT) {
        char smhAmountStr[50];
        str_formatSmeshAmount(smhAmountStr, SIZEOF(smhAmountStr), ctx->tx.amount);
        ui_displayPaginatedText(
            "Send SMH",
            smhAmountStr,
            this_fn
        );
    }
    UI_STEP(TXSIGN_STEP_DISPLAY_ADDRESS) {
        char addressStr[48];
        bin2hex(addressStr, ctx->tx.recipient, SPACEMESH_ADDRESS_SIZE);

        ui_displayPaginatedText(
            "To address",
            addressStr,
            this_fn
        );
    }
    UI_STEP(TXSIGN_STEP_DISPLAY_FEE) {
        char smhAmountStr[50];
        str_formatSmeshAmount(smhAmountStr, SIZEOF(smhAmountStr), ctx->tx.gasPrice * ctx->tx.gasLimit);
        ui_displayPaginatedText(
            "Max Tx Fee",
            smhAmountStr,
            this_fn
        );
    }
    UI_STEP(TXSIGN_STEP_CONFIRM_TX) {
        ui_displayPrompt(
            "Confirm",
            "transaction?",
            this_fn,
            respond_with_user_reject
        );
    }
    UI_STEP(TXSIGN_STEP_DISPLAY_SIGNER) {
        char pathStr[100];
        bip44_printToStr(&ctx->signerPath, pathStr, SIZEOF(pathStr));
        ui_displayPaginatedText(
            "Signer",
            pathStr,
            this_fn
        );
    }
    UI_STEP(TXSIGN_STEP_CONFIRM_SIGNER) {
        ui_displayPrompt(
            "Sign using",
            "this signer?",
            this_fn,
            respond_with_user_reject
        );
    }
    UI_STEP(TXSIGN_STEP_RESPOND) {
        TRACE("io_send_buf");

        io_send_buf(SUCCESS, (uint8_t*)&ctx->response, SIZEOF(ctx->response));
        ui_displayBusy(); // needs to happen after I/O
        // We are finished
        ui_idle();
    }
    UI_STEP_END(TXSIGN_STEP_INVALID);
}

void signTx_handleAPDU(
        uint8_t p1,
        uint8_t p2,
        uint8_t* data,
        size_t dataSize,
        bool isNewCall
)
{
    if (isNewCall) {
        os_memset(ctx, 0, SIZEOF(*ctx));
        ctx->stage = SIGN_STAGE_NONE;
    }

    // Validate params
    VALIDATE(p1 == 0, ERR_INVALID_REQUEST_PARAMETERS);
    VALIDATE(p2 == 0, ERR_INVALID_REQUEST_PARAMETERS);

    // Parse data
    size_t pathSize = bip44_parse(&ctx->signerPath, data, dataSize);

    size_t txSize = dataSize - pathSize;
    if (txSize < SPACEMESH_TX_MIN_SIZE) {
        THROW(ERR_INVALID_DATA);
    }

    const uint8_t *encodedTx = data + pathSize;

    tx_header_t *tx = (tx_header_t *)(data + pathSize);

    ctx->tx.type = tx->type;
    ctx->tx.nonce = u8be_read((const uint8_t *)&tx->nonce);
    ctx->tx.gasLimit = u8be_read((const uint8_t *)&tx->gasLimit);
    ctx->tx.gasPrice = u8be_read((const uint8_t *)&tx->gasPrice);
    ctx->tx.amount = u8be_read((const uint8_t *)&tx->amount);

    os_memmove(ctx->tx.recipient, tx->recipient, SPACEMESH_ADDRESS_SIZE);

    TRACE("signMessage");
    signMessage(
        &ctx->signerPath,
        encodedTx, dataSize - pathSize,
        ctx->response.signature, SIZEOF(ctx->response.signature),
        ctx->response.pubkey, SIZEOF(ctx->response.pubkey)
    );

    ctx->ui_step = TXSIGN_STEP_DISPLAY_TYPE;

    signTx_ui_runStep();
}