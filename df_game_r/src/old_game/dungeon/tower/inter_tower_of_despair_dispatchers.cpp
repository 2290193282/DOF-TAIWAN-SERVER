#include "old_game/dungeon/tower/inter_tower_of_despair_dispatchers.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/dungeon/instance/sending_apc_info_runtime.h"
#include "old_game/dungeon/tower/tower_of_despair_mgr_runtime.h"
#include "infra/network/packet/PacketHeader.h"

namespace {

#pragma pack(push, 1)
struct Packet_TOD_DoRandomSelect {
    Packet_TOD_DoRandomSelect() : header(0x2712u, sizeof(*this)) {}

    PacketHeader header;
};
#pragma pack(pop)

static_assert(sizeof(Packet_TOD_DoRandomSelect) == 0x0Au,
              "Packet_TOD_DoRandomSelect must preserve its ten-byte wire ABI");

}  // namespace

// IDA_EA: 0x84E4D90
// IDA_NAME: _ZN44Inter_TOD_ReturnToMonitor_APCSettingComplete12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_TOD_ReturnToMonitor_APCSettingComplete::dispatch_sig(
//     CUser *,char *,int)
int Inter_TOD_ReturnToMonitor_APCSettingComplete::dispatch_sig(CUser*,
                                                                char*,
                                                                int) {
    const ENUM_SERVER_GROUP server_group =
        static_cast<ENUM_SERVER_GROUP>(G_CEnvironment()->get_server_group());
    cMyTrace trace(
        "virtual int Inter_TOD_ReturnToMonitor_APCSettingComplete::dispatch_sig(CUser*, char*, int)",
        25079, 5);
    trace("TOD Random Select Complete. send->monitor group(%d)",
          static_cast<int>(server_group));

    Packet_TOD_DoRandomSelect packet;
    GlobalData::s_monitor_proxy_mgr->GetServerProxy(server_group)->SendTcpPacket(
        reinterpret_cast<char*>(&packet), sizeof(packet));
    return 0;
}

// IDA_EA: 0x84EEB58 / 0x84F623A / 0x84F626A
Inter_TOD_ReturnToMonitor_APCSettingComplete::
    Inter_TOD_ReturnToMonitor_APCSettingComplete()
    : ISigDispatcher() {}
Inter_TOD_ReturnToMonitor_APCSettingComplete::
    ~Inter_TOD_ReturnToMonitor_APCSettingComplete() {}

// IDA_EA: 0x84E4E26
// IDA_NAME: _ZN20Inter_TOD_Reload_APC12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_TOD_Reload_APC::dispatch_sig(CUser *,char *,int)
int Inter_TOD_Reload_APC::dispatch_sig(CUser*, char* payload, int payload_size) {
    const int required_size = static_cast<int>(sizeof(_APCInfo) * 10u);
    if (!payload || payload_size < required_size ||
        !GlobalData::s_pTowerOfDespairMgr) {
        return 0;
    }

    GlobalData::s_pTowerOfDespairMgr->ResetAPCInfo();
    for (unsigned int index = 0; index != 10u; ++index) {
        const _APCInfo& info = *reinterpret_cast<const _APCInfo*>(
            payload + index * sizeof(_APCInfo));
        if (info.user_charac_no != 0u) {
            GlobalData::s_pTowerOfDespairMgr->SetAPCInfo(index, info);
        }
    }
    return 0;
}

// IDA_EA: 0x84EEB74 / 0x84F61EC / 0x84F621C
Inter_TOD_Reload_APC::Inter_TOD_Reload_APC() : ISigDispatcher() {}
Inter_TOD_Reload_APC::~Inter_TOD_Reload_APC() {}
