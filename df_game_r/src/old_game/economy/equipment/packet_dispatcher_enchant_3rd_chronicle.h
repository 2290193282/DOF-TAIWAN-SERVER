#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_ENCHANT_3RD_CHRONICLE_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_ENCHANT_3RD_CHRONICLE_H_

#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_game/progression/quest/packet_dispatcher_sr08_quest_skill_unclaimed.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// PacketDispatcher slot 371.  The old handler owns the compact MSG_ENCHANT
// wire record and the state-0 MSG_ENCHANT/stEnchantParam template instance.
class Dispatcher_Enchant_3rdChronicleItem
    : public IPacketDispatcher<MSG_ENCHANT,
                               stEnchantParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859761E
    // IDA_NAME: _ZN35Dispatcher_Enchant_3rdChronicleItemC2Ev
    Dispatcher_Enchant_3rdChronicleItem();

    // IDA_EA: 0x82453CA / 0x82453FA
    // IDA_NAME: _ZN35Dispatcher_Enchant_3rdChronicleItemD2Ev / D0Ev
    virtual ~Dispatcher_Enchant_3rdChronicleItem();

    // IDA_EA: 0x81D343A
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81D35FE / 0x81D375E / 0x81D387E / 0x82328EC
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    virtual int check_state(CUser* user, PacketBuf& packet);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Enchant_3rdChronicleItem) == 0x04u,
              "CMD371 dispatcher must stay vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_ENCHANT_3RD_CHRONICLE_H_
