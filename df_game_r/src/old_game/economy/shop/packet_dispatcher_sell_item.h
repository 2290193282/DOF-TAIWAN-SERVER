#ifndef OLDSRV_OLD_GAME_NPC_PACKET_DISPATCHER_SELL_ITEM_H_
#define OLDSRV_OLD_GAME_NPC_PACKET_DISPATCHER_SELL_ITEM_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/DisPatcher_SellItem (CMD24).  As in the
// original, the dispatcher parses its six-byte payload itself instead of
// materialising a template message object: item-space byte, slot/count shorts,
// then a four-byte client checksum after the private-store gate.
class DisPatcher_SellItem
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859552C
    // IDA_NAME: _ZN19DisPatcher_SellItemC2Ev
    DisPatcher_SellItem();

    // IDA_EA: 0x8232F18 / 0x8232F48
    // IDA_NAME: _ZN19DisPatcher_SellItemD2Ev / D0Ev
    virtual ~DisPatcher_SellItem();

    virtual int dispatch_template(CUser* user, PacketBuf& packet);
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SellItem) == 0x04u,
              "CMD24 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_PACKET_DISPATCHER_SELL_ITEM_H_
