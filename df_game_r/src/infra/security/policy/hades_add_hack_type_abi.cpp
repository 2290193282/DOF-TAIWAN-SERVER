#include "infra/security/policy/hades_public_abi.h"

#include "infra/security/policy/hades_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/user.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketHeader.h"

#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)
struct Packet_Hades_AddHackType : public PacketHeader {
    Packet_Hades_AddHackType();

    int32_t acc_id;
    uint16_t hack_type;
    uint16_t count;
    uint32_t value1;
    uint32_t value2;
};
#pragma pack(pop)

static_assert(sizeof(Packet_Hades_AddHackType) == 0x1au,
              "Packet_Hades_AddHackType IDA ABI size must stay 0x1a");
static_assert(offsetof(Packet_Hades_AddHackType, acc_id) == 0x0au &&
                  offsetof(Packet_Hades_AddHackType, hack_type) == 0x0eu &&
                  offsetof(Packet_Hades_AddHackType, count) == 0x10u &&
                  offsetof(Packet_Hades_AddHackType, value1) == 0x12u &&
                  offsetof(Packet_Hades_AddHackType, value2) == 0x16u,
              "Packet_Hades_AddHackType field offsets changed");

// IDA_EA: 0x84B998E
// IDA_NAME: _ZN24Packet_Hades_AddHackTypeC2Ev
// DEMANGLED: Packet_Hades_AddHackType::Packet_Hades_AddHackType(void)
Packet_Hades_AddHackType::Packet_Hades_AddHackType()
    : PacketHeader(0x0FD7u, 0x001Au) {}

// IDA_EA: 0x84B9328
// IDA_NAME: _ZN8XNuclear6CHades14AddHackTypeCntEmmmm
// DEMANGLED: XNuclear::CHades::AddHackTypeCnt(unsigned long,unsigned long,unsigned long,unsigned long)
int XNuclear::CHades::AddHackTypeCnt(unsigned long hack_type,
                                     unsigned long count,
                                     unsigned long value1,
                                     unsigned long value2) {
    needLogin();
    Packet_Hades_AddHackType packet;
    packet.acc_id = owner_->get_acc_id();
    packet.hack_type = static_cast<uint16_t>(hack_type);
    packet.count = static_cast<uint16_t>(count);
    packet.value1 = static_cast<uint32_t>(value1);
    packet.value2 = static_cast<uint32_t>(value2);
    return GlobalData::s_hades_proxy->SendHadesTcpPacket(
        reinterpret_cast<char*>(&packet));
}
