#include "old_game/world/region/game_world.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/network/packet/PacketHeader.h"

#include <stdint.h>

namespace {

// IDA_EA: Packet_MiniCraneSeed::Packet_MiniCraneSeed @ 0x86D143E.
// The original constructor writes only this 10-byte server header and the
// zero seed dword, yielding the 14-byte monitor TCP frame sent at 0x86D1480.
#pragma pack(push, 1)
struct PacketMiniCraneSeed {
    PacketHeader header;
    uint32_t seed;

    PacketMiniCraneSeed() : header(0x27f8u, 0x0eu), seed(0) {}
};
#pragma pack(pop)

static_assert(sizeof(PacketMiniCraneSeed) == 0x0eu,
              "MiniCrane seed monitor frame must retain its original size");

}  // namespace

// IDA_EA: 0x86D1480
// IDA_NAME: _ZN9GameWorld19UpdateMiniCraneSeedEv
// DEMANGLED: GameWorld::UpdateMiniCraneSeed(void)
char GameWorld::UpdateMiniCraneSeed() {
    PacketMiniCraneSeed packet;
    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(
                G_CEnvironment()->get_server_group()));
    return proxy->SendTcpPacket(reinterpret_cast<char*>(&packet),
                                static_cast<int>(sizeof(packet)));
}
