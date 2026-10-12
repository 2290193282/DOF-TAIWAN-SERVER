#include "old_game/economy/shop/inter_npc_limit_buy_info_returns.h"

#include <cstring>
#include <stdint.h>

#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

namespace {

uint32_t ReadU32(const char* data, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, data + offset, sizeof(value));
    return value;
}

bool IsValidCount(uint32_t count, int payload_size, int first_entry_offset) {
    return count <= 30u &&
           payload_size >= first_entry_offset + static_cast<int>(count * 12u);
}

void WriteRemainingCount(PacketGuard& packet,
                         const char* payload,
                         int first_entry_offset,
                         uint32_t count) {
    for (uint32_t index = 0; index < count; ++index) {
        const int offset = first_entry_offset + static_cast<int>(index * 12u);
        const uint32_t item_id = ReadU32(payload, offset);
        const uint32_t maximum = ReadU32(payload, offset + 4);
        const uint32_t consumed = ReadU32(payload, offset + 8);
        packet.put_int(static_cast<int>(item_id));
        packet.put_int(static_cast<int>(
            maximum && consumed < maximum ? maximum - consumed : 0u));
    }
}

}  // namespace

// IDA_EA: 0x84E6258
// IDA_NAME: _ZN28Inter_NpcLimitBuyItemInfoAll12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NpcLimitBuyItemInfoAll::dispatch_sig(CUser *,char *,int)
int Inter_NpcLimitBuyItemInfoAll::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    if (!user || user->get_state() == 0 || !payload || payload_size < 26) {
        return 0;
    }

    const uint32_t count = ReadU32(payload, 22);
    if (!IsValidCount(count, payload_size, 26)) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, 403);
    packet.put_int(static_cast<int>(count));
    WriteRemainingCount(packet, payload, 26, count);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EECFC / 0x84F5DA8 / 0x84F5DD8
Inter_NpcLimitBuyItemInfoAll::Inter_NpcLimitBuyItemInfoAll()
    : ISigDispatcher() {}
Inter_NpcLimitBuyItemInfoAll::~Inter_NpcLimitBuyItemInfoAll() {}

// IDA_EA: 0x84E6448
// IDA_NAME: _ZN31Inter_NpcLimitBuyItemChangeInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NpcLimitBuyItemChangeInfo::dispatch_sig(CUser *,char *,int)
int Inter_NpcLimitBuyItemChangeInfo::dispatch_sig(CUser*, char* payload,
                                                  int payload_size) {
    if (!payload || payload_size < 18) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, 401);
    packet.put_int(static_cast<int>(ReadU32(payload, 10)));
    packet.put_int(static_cast<int>(ReadU32(payload, 14)));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x84EED18 / 0x84F5D5A / 0x84F5D8A
Inter_NpcLimitBuyItemChangeInfo::Inter_NpcLimitBuyItemChangeInfo()
    : ISigDispatcher() {}
Inter_NpcLimitBuyItemChangeInfo::~Inter_NpcLimitBuyItemChangeInfo() {}

// IDA_EA: 0x84E6E82
// IDA_NAME: _ZN30Inter_NpcLimitBuyItemInfoRenew12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NpcLimitBuyItemInfoRenew::dispatch_sig(CUser *,char *,int)
int Inter_NpcLimitBuyItemInfoRenew::dispatch_sig(CUser*, char* payload,
                                                 int payload_size) {
    if (!payload || payload_size < 14) {
        return 0;
    }

    const uint32_t count = ReadU32(payload, 10);
    if (!IsValidCount(count, payload_size, 14)) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, 403);
    packet.put_int(static_cast<int>(count));
    WriteRemainingCount(packet, payload, 14, count);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x84EED34 / 0x84F5D0C / 0x84F5D3C
Inter_NpcLimitBuyItemInfoRenew::Inter_NpcLimitBuyItemInfoRenew()
    : ISigDispatcher() {}
Inter_NpcLimitBuyItemInfoRenew::~Inter_NpcLimitBuyItemInfoRenew() {}
