#ifndef OLDSRV_OLD_DATA_DB_ACCOUNT_CARGO_PACKET_DISPATCHER_ACCOUNT_CARGO_H_
#define OLDSRV_OLD_DATA_DB_ACCOUNT_CARGO_PACKET_DISPATCHER_ACCOUNT_CARGO_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class Dispatcher_CreateAccountCargo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_CreateAccountCargo();
    virtual ~Dispatcher_CreateAccountCargo();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_UpgradeAccountCargo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_UpgradeAccountCargo();
    virtual ~Dispatcher_UpgradeAccountCargo();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CreateAccountCargo) == 0x04u,
              "CMD306 must remain vptr-only");
static_assert(sizeof(Dispatcher_UpgradeAccountCargo) == 0x04u,
              "CMD307 must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_DATA_DB_ACCOUNT_CARGO_PACKET_DISPATCHER_ACCOUNT_CARGO_H_
