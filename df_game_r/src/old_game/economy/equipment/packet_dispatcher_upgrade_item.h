#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_UPGRADE_ITEM_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_UPGRADE_ITEM_H_

#include "old_data/PVF/equipment/item_upgrade.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/Dispatcher_UpgradeItem (CMD83).  This
// object owns only the state-0 base vptr; its transaction is performed by the
// fully restored B-owned CItemUpgrade service.
class Dispatcher_UpgradeItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595CF0
    Dispatcher_UpgradeItem();
    // IDA_EA: 0x8243294 / 0x82432C4
    virtual ~Dispatcher_UpgradeItem();
    // IDA_EA: 0x82006D0
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x82335BC / 0x82335C6
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_UpgradeItemSeparate (CMD439).
// Unlike CMD83 this request has a typed message/parameter dispatcher family.
class Dispatcher_UpgradeItemSeparate
    : public IPacketDispatcher<MSG_ITEM_UPGRADE,
                               STUdateItemParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595D0C
    Dispatcher_UpgradeItemSeparate();
    // IDA_EA: 0x8246096 / 0x82460C6
    virtual ~Dispatcher_UpgradeItemSeparate();
    // IDA_EA: 0x81CBF24 / 0x81CC106
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    // IDA_EA: 0x81CBE84
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UpgradeItem) == 0x04u,
              "CMD83 must retain the old vptr-only dispatcher ABI");
static_assert(sizeof(Dispatcher_UpgradeItemSeparate) == 0x04u,
              "CMD439 must retain the old vptr-only dispatcher ABI");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_UPGRADE_ITEM_H_
