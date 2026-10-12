#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_USE_BOOSTER_ITEM_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_USE_BOOSTER_ITEM_H_

#include <vector>

#include "old_game/economy/inventory/inventory.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/Dispatcher_UseBoosterItem (CMD158).
class Dispatcher_UseBoosterItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596428
    Dispatcher_UseBoosterItem();

    // IDA_EA: 0x824230A / 0x824233A
    virtual ~Dispatcher_UseBoosterItem();

    // IDA_EA: 0x8208D40
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233A2E / 0x8233A38
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

private:
    // IDA_EA: 0x82099BC / 0x8209A5E / 0x8209B0E
    bool _onBoosterItemUseResult(CUser* user,
                                 std::vector<Inven_Item>& output_items,
                                 int source_item_id);
    void _onBoosterItemUseResult_OnlyOne(
        CUser* user,
        std::vector<Inven_Item>& output_items,
        int source_item_id);
    void sendBroadCastItems(CUser* user,
                            const Inven_Item& item,
                            int source_item_id);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UseBoosterItem) == 0x04u,
              "CMD158 must retain the old vptr-only dispatcher ABI");
#endif

#endif
