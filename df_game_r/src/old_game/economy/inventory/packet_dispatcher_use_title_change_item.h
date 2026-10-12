#ifndef OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_USE_TITLE_CHANGE_ITEM_H_
#define OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_USE_TITLE_CHANGE_ITEM_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


// 0828 CMD672: consumes the Arad title-change material and replaces a title
// record in the ordinary inventory with a freshly generated result record.
class Dispatcher_UseTitleChangeItem
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_UseTitleChangeItem();
    virtual ~Dispatcher_UseTitleChangeItem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UseTitleChangeItem) == 0x04u,
              "CMD672 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_USE_TITLE_CHANGE_ITEM_H_
