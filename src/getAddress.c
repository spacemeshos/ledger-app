#include "common.h"
#include "getAddress.h"
#include "keyDerivation.h"
#include "endian.h"
#include "state.h"
#include "securityPolicy.h"
#include "uiHelpers.h"
#include "deriveAddress.h"

static uint16_t RESPONSE_READY_MAGIC = 11223;

static ins_get_address_context_t* ctx = &(instructionState.getAddressContext);

enum {
    P1_RETURN  = 0x01,
    P1_DISPLAY = 0x02,
};

// forward declarations

static void getAddress_return_ui_runStep();

enum {
    RETURN_UI_STEP_WARNING = 100,
    RETURN_UI_STEP_PATH,
    RETURN_UI_STEP_CONFIRM,
    RETURN_UI_STEP_RESPOND,
    RETURN_UI_STEP_INVALID,
};

static void getAddress_handleReturn(uint8_t p2, uint8_t* data, size_t dataSize)
{
    TRACE();
    VALIDATE(p2 == 0, ERR_INVALID_REQUEST_PARAMETERS);

    // Parse data
    size_t parsedSize = bip44_parse(&ctx->pathSpec, data, dataSize);

    if (parsedSize != dataSize) {
        THROW(ERR_INVALID_DATA);
    }

    // Check security policy
//    security_policy_t policy = policyForReturnAddress(&ctx->pathSpec);
//    ENSURE_NOT_DENIED(policy);

    deriveAddress(
        &ctx->pathSpec,
        ctx->address,
        SIZEOF(ctx->address)
    );
    ctx->responseReadyMagic = RESPONSE_READY_MAGIC;
/*
    switch (policy) {
#       define  CASE(POLICY, STEP) case POLICY: {ctx->ui_step=STEP; break;}
        CASE(POLICY_PROMPT_WARN_UNUSUAL,    RETURN_UI_STEP_WARNING);
        CASE(POLICY_PROMPT_BEFORE_RESPONSE, RETURN_UI_STEP_PATH);
        CASE(POLICY_ALLOW_WITHOUT_PROMPT,   RETURN_UI_STEP_RESPOND);
#       undef   CASE
    default:
        THROW(ERR_NOT_IMPLEMENTED);
    }
*/

    ctx->ui_step = RETURN_UI_STEP_PATH;
    getAddress_return_ui_runStep();
}

static void getAddress_return_ui_runStep()
{
    TRACE("step %d\n", ctx->ui_step);
    ASSERT(ctx->responseReadyMagic == RESPONSE_READY_MAGIC);
    ui_callback_fn_t* this_fn = getAddress_return_ui_runStep;

    UI_STEP_BEGIN(ctx->ui_step);

    UI_STEP(RETURN_UI_STEP_WARNING) {
        ui_displayPaginatedText(
            "Unusual request",
            "Proceed with care",
            this_fn
        );
    }
    UI_STEP(RETURN_UI_STEP_PATH) {
        // Response
        char pathStr[100];
        {
            const char* prefix = "Path: ";
            size_t len = strlen(prefix);
            os_memcpy(pathStr, prefix, len); // Note: not null-terminated yet
            bip44_printToStr(&ctx->pathSpec, pathStr + len, SIZEOF(pathStr) - len);
        }
        ui_displayPaginatedText(
            "Export address",
            pathStr,
            this_fn
        );
    }
    UI_STEP(RETURN_UI_STEP_CONFIRM) {
        ui_displayPrompt(
            "Confirm",
            "export address?",
            this_fn,
            respond_with_user_reject
        );
    }
    UI_STEP(RETURN_UI_STEP_RESPOND) {
        ctx->responseReadyMagic = 0;
        io_send_buf(SUCCESS, ctx->address, SPACEMESH_ADDRESS_SIZE);
        ui_idle();
    }
    UI_STEP_END(RETURN_UI_STEP_INVALID);
}

static void getAddress_display_ui_runStep();

enum {
    DISPLAY_UI_STEP_WARNING = 200,
    DISPLAY_UI_STEP_INSTRUCTIONS,
    DISPLAY_UI_STEP_PATH,
    DISPLAY_UI_STEP_ADDRESS,
    DISPLAY_UI_STEP_RESPOND,
    DISPLAY_UI_STEP_INVALID
};

static void getAddress_handleDisplay(uint8_t p2, uint8_t* data, size_t dataSize)
{
    TRACE();
    VALIDATE(p2 == 0, ERR_INVALID_REQUEST_PARAMETERS);

    // Parse data
    size_t parsedSize = bip44_parse(&ctx->pathSpec, data, dataSize);

    if (parsedSize != dataSize) {
        THROW(ERR_INVALID_DATA);
    }

    // Check security policy
//    security_policy_t policy = policyForShowAddress(&ctx->pathSpec);
//    ENSURE_NOT_DENIED(policy);

    deriveAddress(
        &ctx->pathSpec,
        ctx->address,
        SIZEOF(ctx->address)
    );
    ctx->responseReadyMagic = RESPONSE_READY_MAGIC;
/*
    switch (policy) {
#       define  CASE(policy, step) case policy: {ctx->ui_step=step; break;}
        CASE(POLICY_PROMPT_WARN_UNUSUAL,  DISPLAY_UI_STEP_WARNING);
        CASE(POLICY_SHOW_BEFORE_RESPONSE, DISPLAY_UI_STEP_INSTRUCTIONS);
#       undef   CASE
    default:
        THROW(ERR_NOT_IMPLEMENTED);
    }
*/
    ctx->ui_step = DISPLAY_UI_STEP_INSTRUCTIONS;
    getAddress_display_ui_runStep();
}

static void getAddress_display_ui_runStep()
{
    ASSERT(ctx->responseReadyMagic == RESPONSE_READY_MAGIC);
    ui_callback_fn_t* this_fn = getAddress_display_ui_runStep;

    UI_STEP_BEGIN(ctx->ui_step);

    UI_STEP(DISPLAY_UI_STEP_WARNING) {
        ui_displayPaginatedText(
            "Unusual request",
            "Proceed with care",
            this_fn
        );
    }
    UI_STEP(DISPLAY_UI_STEP_INSTRUCTIONS) {
        ui_displayPaginatedText(
            "Verify address",
            "Make sure it agrees with your computer",
            this_fn
        );
    }
    UI_STEP(DISPLAY_UI_STEP_PATH) {
        // Response
        char pathStr[100];
        bip44_printToStr(&ctx->pathSpec, pathStr, SIZEOF(pathStr));
        ui_displayPaginatedText(
            "Address path",
            pathStr,
            this_fn
        );
    }
    UI_STEP(DISPLAY_UI_STEP_ADDRESS) {
        char addressStr[44];
        bin2hex(addressStr, ctx->address, SPACEMESH_ADDRESS_SIZE);
        ui_displayPaginatedText(
            "Address",
            addressStr,
            this_fn
        );
    }
    UI_STEP(DISPLAY_UI_STEP_RESPOND) {
        io_send_buf(SUCCESS, NULL, 0);
        ui_idle();
    }
    UI_STEP_END(DISPLAY_UI_STEP_INVALID);
}

void getAddress_handleAPDU(
        uint8_t p1,
        uint8_t p2,
        uint8_t *data,
        size_t dataSize,
        bool isNewCall
)
{
    // Initialize state
    if (isNewCall) {
        os_memset(ctx, 0, SIZEOF(*ctx));
    }
    ctx->responseReadyMagic = 0;
    switch (p1) {
#       define  CASE(P1, HANDLER_FN) case P1: {HANDLER_FN(p2, data, dataSize); break;}
        CASE(P1_RETURN,  getAddress_handleReturn);
        CASE(P1_DISPLAY, getAddress_handleDisplay);
#       undef  CASE
    default:
        THROW(ERR_INVALID_REQUEST_PARAMETERS);
    }
}
