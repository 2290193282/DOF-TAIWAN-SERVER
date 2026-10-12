#include "old_data/Login/inter_returns/inter_monitor_call_userinfo_return.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"

// IDA_EA: 0x84C89B2
// IDA_NAME: _ZN25Inter_MonitorCallUserInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorCallUserInfo::dispatch_sig(CUser *,char *,int)
int Inter_MonitorCallUserInfo::dispatch_sig(CUser*, char* payload, int) {
    CGameManager* const game_manager = G_CGameManager();
    if (game_manager) {
        const unsigned char target = static_cast<unsigned char>(payload[10]);
        game_manager->Send_userinfos_to_upper_server(target);
        cMyTrace trace(
            "virtual int Inter_MonitorCallUserInfo::dispatch_sig(CUser*, char*, int)",
            8752, 0);
        trace("%d Server Call UserInfo Arrived\n", target);
    } else {
        cMyTrace trace(
            "virtual int Inter_MonitorCallUserInfo::dispatch_sig(CUser*, char*, int)",
            8736, 5);
        trace("Inter_MonitorCallUserInfo::dispatch_sig()  G_CGameManager() is null");
    }
    return 0;
}

// IDA_EA: 0x84EA3C0 / 0x84EA3DC / 0x84EA40C
// IDA_NAME: _ZN25Inter_MonitorCallUserInfoC2Ev / D2Ev / D0Ev
Inter_MonitorCallUserInfo::Inter_MonitorCallUserInfo() : ISigDispatcher() {}
Inter_MonitorCallUserInfo::~Inter_MonitorCallUserInfo() {}
