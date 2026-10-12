#ifndef OLDSRV_OLD_GAME_ECONOMY_SHOP_PACKET_DISPATCHER_GIVE_GIFT_TO_NPC_H_
#define OLDSRV_OLD_GAME_ECONOMY_SHOP_PACKET_DISPATCHER_GIVE_GIFT_TO_NPC_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

struct SIG_LOAD_ETC;

// 140828 CMD222.  CLIENT_IDA_EVIDENCE: sub_1177A50 begins the command at
// 0x1177B50 and writes u16/u8/u16/u32/u32 at 0x1177B60..0x1177B9A.
#pragma pack(push, 1)
struct MSG_GIVE_GIFT_TO_NPC : public MSG_BASE {
    MSG_GIVE_GIFT_TO_NPC();

    uint16_t npc_id;       // +0x0d, current NPC selected by the client
    uint8_t item_space;    // +0x0f, ENUM_ITEMSPACE
    uint16_t item_slot;    // +0x10
    uint32_t item_id;      // +0x12, template id (not an instance UID)
    uint32_t item_count;   // +0x16
};
#pragma pack(pop)

static_assert(sizeof(MSG_GIVE_GIFT_TO_NPC) == 0x1au,
              "CMD222 request must be the exact 13-byte payload");
static_assert(offsetof(MSG_GIVE_GIFT_TO_NPC, npc_id) == 0x0du &&
                  offsetof(MSG_GIVE_GIFT_TO_NPC, item_space) == 0x0fu &&
                  offsetof(MSG_GIVE_GIFT_TO_NPC, item_slot) == 0x10u &&
                  offsetof(MSG_GIVE_GIFT_TO_NPC, item_id) == 0x12u &&
                  offsetof(MSG_GIVE_GIFT_TO_NPC, item_count) == 0x16u,
              "CMD222 request field offsets drifted");

class stGiveGiftToNPC : public ParamBase {
public:
    stGiveGiftToNPC();
    virtual ~stGiveGiftToNPC();

    uint8_t error_code;
    uint8_t special_event;
    uint16_t npc_id;
    uint8_t daily_gift_count;
    uint8_t favor_level;
    uint8_t item_space;
    uint16_t item_slot;
    uint32_t item_id;
    uint32_t remaining_count;
    uint8_t reward_gate;
    uint8_t level_change;
    uint8_t favor_change;
    uint8_t gift_result;
    uint16_t favor_value;
    uint32_t reward_item_id;
    uint32_t reward_item_count;
    uint8_t reaction;
    int reward_slot;  // server-only; update is sent after the CMD reply
};

// Exact legacy class identity retained for the registry owner.  The request
// and response ABIs are the 140828 client ABIs, not the removed 130627
// padded/no-op placeholder.
class Dispatcher_GiveGiftToNPC
    : public IPacketDispatcher<MSG_GIVE_GIFT_TO_NPC,
                               stGiveGiftToNPC,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_GiveGiftToNPC();
    virtual ~Dispatcher_GiveGiftToNPC();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

namespace npc_gift {

// Installs the already DB-loaded relationship snapshot at character login.
// A CMD222 request fails closed until this hook has succeeded.
bool Load(CUser* user, const SIG_LOAD_ETC* payload);

// Copies the live relationship sidecar into the exchange-server character
// cache.  The legacy CUser::GetEtcInfo serializer does not know about the
// 0828 fields, so cache hits would otherwise lose both the identity prefix
// and every NPC relationship record.
bool ExportToCache(CUser* user, SIG_LOAD_ETC* payload);

// Signal 367 writes an absolute 608-byte snapshot, so repeated calls and the
// forced disconnect write are idempotent.  Queue acceptance is not treated as
// a durable ACK; Erase deliberately submits the latest snapshot once more.
bool Flush(CUser* user);
void ResetDaily(CUser* user);
void Erase(CUser* user);

}  // namespace npc_gift_0828

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GiveGiftToNPC) == 0x04u,
              "CMD222 dispatcher must remain a base-only i386 object");
#endif

#endif  // OLDSRV_OLD_GAME_ECONOMY_SHOP_PACKET_DISPATCHER_GIVE_GIFT_TO_NPC_0828_H_
