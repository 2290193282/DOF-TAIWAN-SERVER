#ifndef OLDSRV_OLD_GAME_DUNGEON_ESCORT_PACKET_DISPATCHER_GOLDEN_COMMERCE_H_
#define OLDSRV_OLD_GAME_DUNGEON_ESCORT_PACKET_DISPATCHER_GOLDEN_COMMERCE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

class Dispatcher_LoadingCart
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_LoadingCart();
    virtual ~Dispatcher_LoadingCart();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_GoldenCommerceStart
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_GoldenCommerceStart();
    virtual ~Dispatcher_GoldenCommerceStart();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_GoldenCommerceReward
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_GoldenCommerceReward();
    virtual ~Dispatcher_GoldenCommerceReward();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_LoadingCart) == 0x04u,
              "CMD546 dispatcher must remain vptr-only on target i386");
static_assert(sizeof(Dispatcher_GoldenCommerceStart) == 0x04u,
              "CMD547 dispatcher must remain vptr-only on target i386");
static_assert(sizeof(Dispatcher_GoldenCommerceReward) == 0x04u,
              "CMD548 dispatcher must remain vptr-only on target i386");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_ESCORT_PACKET_DISPATCHER_GOLDEN_COMMERCE_0828_H_
