#ifndef OLDSRV_OLD_GAME_PVP_MATCH_INTEGRATED_MATCHING_RUNTIME_H_
#define OLDSRV_OLD_GAME_PVP_MATCH_INTEGRATED_MATCHING_RUNTIME_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

namespace integrated_matching {
void CancelForDisconnect(CUser* user);
}

class Dispatcher_RequestIntegratedMatching
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_RequestIntegratedMatching();
    virtual ~Dispatcher_RequestIntegratedMatching();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_CancelIntegratedMatching
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_CancelIntegratedMatching();
    virtual ~Dispatcher_CancelIntegratedMatching();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_IntegrateMatchingDirectComplete
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_IntegrateMatchingDirectComplete();
    virtual ~Dispatcher_IntegrateMatchingDirectComplete();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_IntegratedMatchingModeChange
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_IntegratedMatchingModeChange();
    virtual ~Dispatcher_IntegratedMatchingModeChange();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_IntegrateMatchingUserCount
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_IntegrateMatchingUserCount();
    virtual ~Dispatcher_IntegrateMatchingUserCount();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestIntegratedMatching) == 0x04u,
              "CMD518 dispatcher must stay vptr-only on i386");
static_assert(sizeof(Dispatcher_CancelIntegratedMatching) == 0x04u,
              "CMD519 dispatcher must stay vptr-only on i386");
static_assert(sizeof(Dispatcher_IntegrateMatchingDirectComplete) == 0x04u,
              "CMD564 dispatcher must stay vptr-only on i386");
static_assert(sizeof(Dispatcher_IntegratedMatchingModeChange) == 0x04u,
              "CMD565 dispatcher must stay vptr-only on i386");
static_assert(sizeof(Dispatcher_IntegrateMatchingUserCount) == 0x04u,
              "CMD566 dispatcher must stay vptr-only on i386");
#endif


#endif  // OLDSRV_OLD_GAME_PVP_MATCH_INTEGRATED_MATCHING_RUNTIME_H_
