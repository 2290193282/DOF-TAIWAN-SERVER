#ifndef OLDSRV_OLD_GAME_NPC_INTER_NPC_LIMIT_BUY_ITEM_INFO_RETURN_H_
#define OLDSRV_OLD_GAME_NPC_INTER_NPC_LIMIT_BUY_ITEM_INFO_RETURN_H_

#include <utility>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/npc_limit_packets.h"

class CItem;
class CUser;

// IDA unit: 07_npc_shop_dialog/Inter_NpcLimitBuyItemInfo.
//
// This DB-return receiver deliberately retains the original stateful 0x98
// object rather than borrowing the client request dispatcher.  The returned
// limit-purchase result must keep the generated item, consumed material, and
// mail fallback together until the monitor acknowledgement and CMD465 reply
// have both been emitted.
class Inter_NpcLimitBuyItemInfo : public ISigDispatcher {
public:
    // IDA_EA: 0x84E6502 / 0x84E6554 / 0x84E6584.
    Inter_NpcLimitBuyItemInfo();
    virtual ~Inter_NpcLimitBuyItemInfo();

    // IDA_EA: 0x84E65A2.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

private:
    // IDA_EA: 0x84E6ACC / 0x84E6C42 / 0x84E6E1A.
    int checkError(CUser* user, LimitNpcBuyItemInfo* response);
    void sendPacket(CUser* user);
    void reset();

    int target_item_id_;                 // +0x04
    std::pair<int, int> need_material_;  // +0x08
    int inserted_slot_;                  // +0x10
    int material_slot_;                  // +0x14
    CItem* target_prototype_;            // +0x18
    bool mailed_;                        // +0x1c
    Inven_Item target_item_;             // +0x1d
    Inven_Item material_item_;           // +0x5a
};

#if defined(__i386__)
static_assert(sizeof(Inter_NpcLimitBuyItemInfo) == 0x98u,
              "Inter_NpcLimitBuyItemInfo IDA allocation must stay 0x98");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_INTER_NPC_LIMIT_BUY_ITEM_INFO_RETURN_H_
