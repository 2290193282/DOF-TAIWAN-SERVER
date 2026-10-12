#include "old_game/world/region/inter_world_return_dispatchers.h"

#include <cstring>
#include <stddef.h>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/concurrent/timer/time_gate.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/cuser_reset_time_gate_quest.h"
#include "old_game/world/region/game_world.h"
#include "old_game/world/region/world_area_allocators.h"
#include "infra/network/packet/packet_guard.h"

namespace {

#pragma pack(push, 1)

struct NoCachePayload {
    uint8_t routing_00[2];       // +0x00
    uint16_t forward_size;       // +0x02
    uint8_t routing_04[6];       // +0x04
    uint32_t account_id;         // +0x0a
    uint32_t server_group;       // +0x0e
    uint32_t mode;               // +0x12
};

struct BroadcastMessageHeader {
    uint8_t routing_envelope[10]; // +0x00
    uint32_t recipient_account_id;// +0x0a; zero means the teen group
    uint8_t text_length;          // +0x0e
};

struct CollectItemsResultPayload {
    uint8_t routing_envelope[10]; // +0x00
    uint32_t total_count;         // +0x0a
    uint32_t current_count;       // +0x0e
};

struct CollectItemsRewardBroadcastHeader {
    uint8_t routing_envelope[18]; // +0x00
    uint8_t reward_type;          // +0x12
    uint8_t reserved_13;          // +0x13
};

#pragma pack(pop)

static_assert(sizeof(NoCachePayload) == 0x16u &&
                  offsetof(NoCachePayload, account_id) == 0x0au &&
                  offsetof(NoCachePayload, server_group) == 0x0eu &&
                  offsetof(NoCachePayload, mode) == 0x12u,
              "no-cache internal packet fields moved from the 0627 ABI");
static_assert(sizeof(BroadcastMessageHeader) == 0x0fu &&
                  offsetof(BroadcastMessageHeader, text_length) == 0x0eu,
              "broadcast internal packet fields moved from the 0627 ABI");
static_assert(sizeof(CollectItemsResultPayload) == 0x12u &&
                  offsetof(CollectItemsResultPayload, total_count) == 0x0au &&
                  offsetof(CollectItemsResultPayload, current_count) == 0x0eu,
              "collect-items result fields moved from the 0627 ABI");
static_assert(sizeof(CollectItemsRewardBroadcastHeader) == 0x14u &&
                  offsetof(CollectItemsRewardBroadcastHeader, reward_type) ==
                      0x12u,
              "collect-items reward fields moved from the 0627 ABI");

int BoundedCStringLength(const char* text, int maximum_length) {
    int length = 0;
    while (length < maximum_length && text[length]) {
        ++length;
    }
    return length;
}

}  // namespace

// IDA_EA: 0x84E3082
// IDA_NAME: _ZN13Inter_NoCache12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoCache::dispatch_sig(CUser *,char *,int)
int Inter_NoCache::dispatch_sig(CUser*, char* payload, int payload_size) {
    if (!payload || payload_size < 22) {
        return 0;
    }

    const NoCachePayload& input =
        *reinterpret_cast<const NoCachePayload*>(payload);
    if (input.mode <= 1u) {
        if (input.account_id) {
            G_GameWorld()->ResetCacheCharactorMemory(input.account_id);
        } else {
            G_GameWorld()->ResetCacheCharactorMemory();
        }
    }

    if (input.mode == 0u || input.mode == 2u) {
        const int forward_size = static_cast<int>(input.forward_size);
        if (forward_size >= 0 && forward_size <= payload_size) {
            CMonitorServerProxy* const proxy =
                GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(input.server_group));
            proxy->SendPacket(payload, forward_size);
        }
    }
    return 0;
}

// IDA_EA: 0x84EE928 / 0x84F698A / 0x84F69BA
Inter_NoCache::Inter_NoCache() : ISigDispatcher() {}
Inter_NoCache::~Inter_NoCache() {}

// IDA_EA: 0x84E4C2A
// IDA_NAME: _ZN18Inter_BroadcastMsg12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_BroadcastMsg::dispatch_sig(CUser *,char *,int)
int Inter_BroadcastMsg::dispatch_sig(CUser*, char* payload, int payload_size) {
    if (!payload || payload_size < 15) {
        return 0;
    }

    const BroadcastMessageHeader& input =
        *reinterpret_cast<const BroadcastMessageHeader*>(payload);
    if (input.recipient_account_id) {
        return 0;
    }
    const int text_length = input.text_length;
    if (text_length > payload_size - 15) {
        return 0;
    }
    const char* const text = payload + sizeof(input);

    PacketGuard packet;
    packet.put_header(0, 12);
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_byte(0);
    packet.put_int(text_length);
    packet.put_str(text, text_length);
    packet.finalize(true);

    CUserTeen recipients;
    G_GameWorld()->send_group(packet, &recipients);
    return 0;
}

// IDA_EA: 0x84EEB3C / 0x84F62D6 / 0x84F6306
Inter_BroadcastMsg::Inter_BroadcastMsg() : ISigDispatcher() {}
Inter_BroadcastMsg::~Inter_BroadcastMsg() {}

// IDA_EA: 0x84E770C
// IDA_NAME: _ZN24Inter_CollectItemsResult12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_CollectItemsResult::dispatch_sig(CUser *,char *,int)
int Inter_CollectItemsResult::dispatch_sig(CUser*, char* payload,
                                           int payload_size) {
    if (!payload || payload_size < 18 || !GlobalData::s_timeGate_) {
        return 0;
    }

    const CollectItemsResultPayload& input =
        *reinterpret_cast<const CollectItemsResultPayload*>(payload);
    if (!input.total_count) {
        return 0;
    }

    uint32_t level = (100u * input.current_count) / input.total_count;
    if (level > 100u) {
        level = 100u;
    }
    GlobalData::s_timeGate_->SetLevel(static_cast<int>(level));
    const unsigned int effective_level =
        GlobalData::s_timeGate_->isOpen() ? 100u : level;

    PacketGuard packet;
    packet.put_header(0, 422);
    packet.put_byte(static_cast<unsigned char>(effective_level));
    packet.finalize(true);
    G_GameWorld()->send_to_area(GlobalData::s_timeGate_->getVillageIndex(),
                                GlobalData::s_timeGate_->getAreaIndex(),
                                packet);
    return 0;
}

// IDA_EA: 0x84EEDA4 / 0x84F5BD4 / 0x84F5C04
Inter_CollectItemsResult::Inter_CollectItemsResult() : ISigDispatcher() {}
Inter_CollectItemsResult::~Inter_CollectItemsResult() {}

// IDA_EA: 0x84E7598
// IDA_NAME: _ZN33Inter_CollectItemsRewardBroadcast12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_CollectItemsRewardBroadcast::dispatch_sig(CUser *,char *,int)
int Inter_CollectItemsRewardBroadcast::dispatch_sig(CUser*, char* payload,
                                                    int payload_size) {
    if (!payload || payload_size < 21) {
        return 0;
    }

    const CollectItemsRewardBroadcastHeader& input =
        *reinterpret_cast<const CollectItemsRewardBroadcastHeader*>(payload);
    const unsigned char reward_type = input.reward_type;
    int npc_index = 0;
    if (reward_type == 2 && GlobalData::s_timeGate_) {
        GlobalData::s_timeGate_->SetLevel(100);
        CUserResetTimeGateQuest reset_time_gate_quest;
        G_GameWorld()->modify_user(&reset_time_gate_quest);
        npc_index = GlobalData::s_timeGate_->getNpcIndex();
    }

    const char* const text = payload + sizeof(input);
    const int text_length = BoundedCStringLength(
        text, payload_size - static_cast<int>(sizeof(input)));
    PacketGuard packet;
    packet.put_header(0, 86);
    packet.put_byte(11);
    packet.put_byte(reward_type);
    packet.put_short(npc_index);
    packet.put_int(text_length);
    packet.put_str(text, text_length);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x84EEDDC / 0x84F5C22 / 0x84F5C52
Inter_CollectItemsRewardBroadcast::Inter_CollectItemsRewardBroadcast()
    : ISigDispatcher() {}
Inter_CollectItemsRewardBroadcast::~Inter_CollectItemsRewardBroadcast() {}
