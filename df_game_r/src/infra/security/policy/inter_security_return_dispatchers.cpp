#include "infra/security/policy/inter_security_return_dispatchers.h"

#include <stddef.h>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "infra/security/policy/secu_data_control_runtime.h"

namespace {

#pragma pack(push, 1)
struct UpdateSecuServiceStatePayload {
    uint8_t routing_envelope[14];  // +0x00
    uint8_t service_kind;          // +0x0e
    uint8_t reply_flag;            // +0x0f
    char password[5];              // +0x10; byte +0x14 is forced to NUL
};
#pragma pack(pop)

static_assert(sizeof(UpdateSecuServiceStatePayload) == 0x15u,
              "security-service reply must retain its 21-byte prefix");
static_assert(offsetof(UpdateSecuServiceStatePayload, service_kind) == 0x0eu &&
                  offsetof(UpdateSecuServiceStatePayload, reply_flag) ==
                      0x0fu &&
                  offsetof(UpdateSecuServiceStatePayload, password) == 0x10u,
              "security-service reply fields moved from the 0627 ABI");

}  // namespace

// IDA_EA: 0x84E4E84
// IDA_NAME: _ZN28Inter_UpdateSecuServiceState12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_UpdateSecuServiceState::dispatch_sig(CUser *,char *,int)
int Inter_UpdateSecuServiceState::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    if (!user || !payload || payload_size <= 20) {
        return 0;
    }

    UpdateSecuServiceStatePayload& input =
        *reinterpret_cast<UpdateSecuServiceStatePayload*>(payload);
    input.password[4] = '\0';
    GlobalData::s_pSecuDataControl->UpdateSecurityServiceState(
        user, input.service_kind, input.reply_flag, input.password);
    return 0;
}

// IDA_EA: 0x84EEB90 / 0x84F619E / 0x84F61CE
Inter_UpdateSecuServiceState::Inter_UpdateSecuServiceState()
    : ISigDispatcher() {}
Inter_UpdateSecuServiceState::~Inter_UpdateSecuServiceState() {}
