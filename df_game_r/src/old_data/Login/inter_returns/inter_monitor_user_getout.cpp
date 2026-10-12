#include "old_data/Login/inter_returns/inter_monitor_user_getout.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x84C8B0C
// IDA_NAME: _ZN23Inter_MonitorUserGetout12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorUserGetout::dispatch_sig(CUser *,char *,int)
int Inter_MonitorUserGetout::dispatch_sig(CUser* /*user*/,
                                          char* payload,
                                          int /*payload_size*/) {
    const Packet_Monitor_UserGetout& request =
        *reinterpret_cast<const Packet_Monitor_UserGetout*>(payload);
    CUser* const target =
        G_CGameManager()->GetUserByAccId(request.account_id);
    if (target) {
        PacketGuard packet;
        packet.put_header(ENUM_PACKETCLASS_CMDPACKET, 1);
        packet.put_byte(0);
        packet.put_byte(65);
        packet.finalize(true);
        target->Send(packet);
        target->DisConnSig(static_cast<DISCONN_SIG>(15), true, 0);
    }
    return 0;
}

// IDA_EA: 0x84EA282
// IDA_NAME: _ZN23Inter_MonitorUserGetoutC2Ev
// DEMANGLED: Inter_MonitorUserGetout::Inter_MonitorUserGetout(void)
Inter_MonitorUserGetout::Inter_MonitorUserGetout() : ISigDispatcher() {}

// IDA_EA: 0x84EA29E / 0x84EA2CE
// IDA_NAME: _ZN23Inter_MonitorUserGetoutD2Ev / D0Ev
// DEMANGLED: Inter_MonitorUserGetout::~Inter_MonitorUserGetout()
Inter_MonitorUserGetout::~Inter_MonitorUserGetout() {}
