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

static inline void CHECK_STAGE(sign_tx_stage_t expected)
{
    VALIDATE(ctx->stage == expected, ERR_INVALID_STATE);
}

enum {
    TXSIGN_STEP_STEP_WARNING = 100,
    TXSIGN_STEP_DISPLAY_AMOUNT,
    TXSIGN_STEP_DISPLAY_ADDRESS,
    TXSIGN_STEP_DISPLAY_FEE,
    TXSIGN_STEP_CONFIRM_TX,
    TXSIGN_STEP_DISPLAY_SIGNER,
    TXSIGN_STEP_CONFIRM_SIGNER,
    TXSIGN_STEP_RESPOND,
    TXSIGN_STEP_INVALID,
};

void str_formatSmeshAmount(char *buffer, size_t bufferSize, uint64_t value)
{
    buffer[0] = 0;
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
        str_formatSmeshAmount(smhAmountStr, SIZEOF(smhAmountStr), ctx->tx.fee);
        ui_displayPaginatedText(
            "Transaction Fee",
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

        io_send_buf(SUCCESS, ctx->signature, SIZEOF(ctx->signature));
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

    // Parse wire
    size_t parsedSize = bip44_parse(&ctx->signerPath, data, dataSize);

//    if (parsedSize != dataSize) {
//        THROW(ERR_INVALID_DATA);
//    }

    tx_header_t *tx = (tx_header_t *)(data + parsedSize);

//  VALIDATE(wireDataSize == 1 + SIZEOF(*wireUtxo), ERR_INVALID_DATA);

    ctx->tx.type = tx->type;
    ctx->tx.nonce = u8be_read((const uint8_t *)&tx->nonce);
    ctx->tx.gasLimit = u8be_read((const uint8_t *)&tx->gasLimit);
    ctx->tx.fee = u8be_read((const uint8_t *)&tx->fee);
    ctx->tx.amount = u8be_read((const uint8_t *)&tx->amount);

    os_memmove(ctx->tx.recipient, tx->recipient, SPACEMESH_ADDRESS_SIZE);

    TRACE("signMessage");
    signMessage(
        &ctx->signerPath,
        tx, dataSize - parsedSize,
        ctx->signature, SIZEOF(ctx->signature),
        &ctx->signer, SIZEOF(ctx->signer)
    );

    ctx->ui_step = TXSIGN_STEP_DISPLAY_AMOUNT;

    signTx_ui_runStep();
}