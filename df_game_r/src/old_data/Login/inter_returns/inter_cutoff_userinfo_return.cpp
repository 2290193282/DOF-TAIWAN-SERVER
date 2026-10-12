#include "old_data/Login/inter_returns/inter_cutoff_userinfo_return.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"

// IDA_EA: 0x84C8A70
// IDA_NAME: _ZN24Inter_CutOffCallUserInfo12dispatch_sigEP5CUserPci
int Inter_CutOffCallUserInfo::dispatch_sig(CUser*, char*, int) {
    CGameManager* const game_manager = G_CGameManager();
    if (game_manager) {
        game_manager->send_userinfos_to_cutoff_server();
        cMyTrace trace(
            "virtual int Inter_CutOffCallUserInfo::dispatch_sig(CUser*, char*, int)",
            8786, 0);
        trace("Cut Off Server Call UserInfo Arrived");
    } else {
        cMyTrace trace(
            "virtual int Inter_CutOffCallUserInfo::dispatch_sig(CUser*, char*, int)",
            8777, 5);
        trace("Inter_MonitorCallUserInfo::dispatch_sig()  G_CGameManager() is null");
    }
    return 0;
}

// IDA_EA: 0x84EA218 / 0x84EA234 / 0x84EA264
// IDA_NAME: _ZN24Inter_CutOffCallUserInfoC2Ev / D2Ev / D0Ev
Inter_CutOffCallUserInfo::Inter_CutOffCallUserInfo() : ISigDispatcher() {}
Inter_CutOffCallUserInfo::~Inter_CutOffCallUserInfo() {}
