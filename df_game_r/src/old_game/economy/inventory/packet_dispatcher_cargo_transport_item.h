#ifndef OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_CARGO_TRANSPORT_ITEM_H_
#define OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_CARGO_TRANSPORT_ITEM_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

namespace item_growth_db {
struct Message;
}

// 0828 CMD551: consume a CargoTransportStone material stack and move one
// eligible equipment record from the ordinary inventory to account cargo.
class Dispatcher_CargoTransportItem
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_CargoTransportItem();
    virtual ~Dispatcher_CargoTransportItem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

namespace cargo_transport_item_runtime {

// Correlated SQL006 reset/compensation completion.  Account-cargo and
// inventory mutations are not allowed to begin before the reset ACK.
void HandlePersistenceResult(CUser* user,
                             const item_growth_db::Message& message);

// Prevents an asynchronous SQL006 ACK from crossing a CUser slot lifetime.
// If a reset may be durable already, compensation is appended to queue6
// before the pending state is discarded.
void ResetUserPersistence(CUser* user);

}  // namespace cargo_transport_item_runtime

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CargoTransportItem) == 0x04u,
              "CMD551 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ECONOMY_INVENTORY_PACKET_DISPATCHER_CARGO_TRANSPORT_ITEM_H_
