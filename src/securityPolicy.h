#ifndef __SPACEMESH_APP_SECURITY_POLICY_H__
#define __SPACEMESH_APP_SECURITY_POLICY_H__

#include "bip44.h"

typedef enum {
    POLICY_DENY = 1,
    POLICY_ALLOW_WITHOUT_PROMPT = 2,
    POLICY_PROMPT_BEFORE_RESPONSE = 3,
    POLICY_PROMPT_WARN_UNUSUAL = 4,
    POLICY_SHOW_BEFORE_RESPONSE = 5, // Show on display but do not ask for explicit confirmation
} security_policy_t;

security_policy_t policyForGetExtendedPublicKey(const bip44_path_t* pathSpec);

security_policy_t policyForShowAddress(const bip44_path_t* pathSpec);
security_policy_t policyForReturnAddress(const bip44_path_t* pathSpec);

security_policy_t policyForAttestUtxo();

security_policy_t policyForSignTxInit();
security_policy_t policyForSignTxInput();
security_policy_t policyForSignTxOutputAddress(const uint8_t* rawAddressBuffer, size_t rawAddressSize);
security_policy_t policyForSignTxOutputPath(const bip44_path_t* pathSpec);
security_policy_t policyForSignTxFee(uint64_t fee );

static inline void ENSURE_NOT_DENIED(security_policy_t policy)
{
    if (policy == POLICY_DENY) {
        THROW(ERR_REJECTED_BY_POLICY);
    }
}

#endif // __SPACEMESH_APP_SECURITY_POLICY_H__