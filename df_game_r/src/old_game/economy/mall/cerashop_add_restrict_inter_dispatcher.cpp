#include "old_game/economy/mall/cerashop_add_restrict_inter_dispatcher.h"

#include "infra/log/format/trace.h"
#include "old_game/economy/mall/cerashop_add_restrict_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x80E2104
// IDA_NAME: _ZN33Inter_LoadCerashopAddRestrictType12dispatch_sigEP5CUserPci
int Inter_LoadCerashopAddRestrictType::dispatch_sig(CUser* user,
                                                    char* payload,
                                                    int /*payload_size*/) {
    if (!user || !payload) {
        cMyTrace trace(
            "virtual int Inter_LoadCerashopAddRestrictType::dispatch_sig(CUser*, char*, int)",
            512, 5);
        trace("[%s] pUser or p is null.",
              "virtual int Inter_LoadCerashopAddRestrictType::dispatch_sig(CUser*, char*, int)");
        return 513;
    }

    SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE* const load =
        reinterpret_cast<SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE*>(payload);
    if (load->restrict_scope == 0) {
        CerashopAddRestrict::Manager::LoadProcess(
            user->cerashop_restrict_.value_map_0,
            user->cerashop_restrict_.parameter_map_0, load);
    } else if (load->restrict_scope == 1) {
        CerashopAddRestrict::Manager::LoadProcess(
            user->cerashop_restrict_.value_map_1,
            user->cerashop_restrict_.parameter_map_1, load);
    }
    CerashopAddRestrict::Manager::SendSyncPacket(user);
    return 0;
}

// IDA_EA: 0x84EEEF4
// IDA_NAME: _ZN33Inter_LoadCerashopAddRestrictTypeC2Ev
Inter_LoadCerashopAddRestrictType::Inter_LoadCerashopAddRestrictType()
    : ISigDispatcher() {}

// IDA_EA: 0x80E4690
// IDA_NAME: _ZN33Inter_LoadCerashopAddRestrictTypeD2Ev
// IDA_EA: 0x80E46C0
// IDA_NAME: _ZN33Inter_LoadCerashopAddRestrictTypeD0Ev
Inter_LoadCerashopAddRestrictType::~Inter_LoadCerashopAddRestrictType() {}
