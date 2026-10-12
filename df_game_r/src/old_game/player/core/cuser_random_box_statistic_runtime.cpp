#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/network/packet/PacketHeader.h"

#include <stdint.h>

#pragma pack(push, 1)
// IDA unit: 99_misc_global/Packet_Randombox_statistic.  The constructor at
// 0x86945A2 writes the 10-byte server packet header followed by these two
// one-byte fields; the complete object is exactly the 0x0c bytes sent by
// CUser::SendRandomBoxStatistic.
struct Packet_Randombox_statistic : PacketHeader {
    Packet_Randombox_statistic();

    uint8_t box_category;
    uint8_t action_kind;
};
#pragma pack(pop)

static_assert(sizeof(Packet_Randombox_statistic) == 0x0cu,
              "Packet_Randombox_statistic ABI size must stay 0x0c");

// IDA_EA: 0x86945A2
// IDA_NAME: _ZN26Packet_Randombox_statisticC2Ev
// DEMANGLED: Packet_Randombox_statistic::Packet_Randombox_statistic(void)
Packet_Randombox_statistic::Packet_Randombox_statistic()
    : PacketHeader(0x17B6u, 0x000Cu), box_category(0), action_kind(0) {}

// IDA_EA: 0x8676826
// IDA_NAME: _ZN5CUser22SendRandomBoxStatisticEi26ENUM_RANDOMBOX_ACTION_KIND
// DEMANGLED: CUser::SendRandomBoxStatistic(int,ENUM_RANDOMBOX_ACTION_KIND)
void CUser::SendRandomBoxStatistic(int box_category,
                                    ENUM_RANDOMBOX_ACTION_KIND action_kind) {
    Packet_Randombox_statistic packet;
    packet.box_category = static_cast<uint8_t>(box_category);
    packet.action_kind = static_cast<uint8_t>(action_kind);
    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(0))
        ->SendPacket(reinterpret_cast<char*>(&packet),
                     static_cast<int>(sizeof(packet)));
}
