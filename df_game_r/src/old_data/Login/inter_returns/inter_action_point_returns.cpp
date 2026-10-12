#include "old_data/Login/inter_returns/inter_action_point_returns.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_data/db/legacy_dispatch/action_point_payload.h"
#include "old_game/player/core/cuser.h"

namespace APSystem {

// IDA_EA: 0x8124D2A
// IDA_NAME: _ZN8APSystem21Inter_LoadActionPoint12dispatch_sigEP5CUserPci
// DEMANGLED: APSystem::Inter_LoadActionPoint::dispatch_sig(CUser *,char *,int)
int Inter_LoadActionPoint::dispatch_sig(CUser* user,
                                        char* payload,
                                        int /*payload_size*/) {
    if (!user || !user->get_state() || !user->getCurCharacR()) {
        return 515;
    }
    if (!payload) {
        return 520;
    }

    _SIG_LOAD_ACTION_POINT& action_point =
        *reinterpret_cast<_SIG_LOAD_ACTION_POINT*>(payload);
    user->action_point_manager_.Set(action_point);
    user->EnableCharacInfo(static_cast<CACHE_SECTION>(8));
    return 0;
}

// IDA_EA: 0x84ED410
// IDA_NAME: _ZN8APSystem21Inter_LoadActionPointC2Ev
// DEMANGLED: APSystem::Inter_LoadActionPoint::Inter_LoadActionPoint(void)
Inter_LoadActionPoint::Inter_LoadActionPoint() : ISigDispatcher() {}

// IDA_EA: 0x8125584 / 0x81255B4
// IDA_NAME: _ZN8APSystem21Inter_LoadActionPointD2Ev / D0Ev
// DEMANGLED: APSystem::Inter_LoadActionPoint::~Inter_LoadActionPoint()
Inter_LoadActionPoint::~Inter_LoadActionPoint() {}

// IDA_EA: 0x8124DB0
// IDA_NAME: _ZN8APSystem22Inter_EmptyActionPoint12dispatch_sigEP5CUserPci
// DEMANGLED: APSystem::Inter_EmptyActionPoint::dispatch_sig(CUser *,char *,int)
int Inter_EmptyActionPoint::dispatch_sig(CUser* user,
                                         char* payload,
                                         int /*payload_size*/) {
    if (!user || !user->get_state() || !user->getCurCharacR()) {
        return 532;
    }
    if (!payload) {
        return 537;
    }

    CUserProc::SetTodayActionAndCheckMedalReward(user);
    user->EnableCharacInfo(static_cast<CACHE_SECTION>(8));
    return 0;
}

// IDA_EA: 0x84ED42C
// IDA_NAME: _ZN8APSystem22Inter_EmptyActionPointC2Ev
// DEMANGLED: APSystem::Inter_EmptyActionPoint::Inter_EmptyActionPoint(void)
Inter_EmptyActionPoint::Inter_EmptyActionPoint() : ISigDispatcher() {}

// IDA_EA: 0x81255D2 / 0x8125602
// IDA_NAME: _ZN8APSystem22Inter_EmptyActionPointD2Ev / D0Ev
// DEMANGLED: APSystem::Inter_EmptyActionPoint::~Inter_EmptyActionPoint()
Inter_EmptyActionPoint::~Inter_EmptyActionPoint() {}

}  // namespace APSystem
