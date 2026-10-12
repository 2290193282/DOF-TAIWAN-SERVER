#ifndef OLDSRV_SECURITY_SENDER_SECURITY_TELEMETRY_DISPATCHERS_H_
#define OLDSRV_SECURITY_SENDER_SECURITY_TELEMETRY_DISPATCHERS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// These commands are client-to-server, sender-only security/telemetry
// reports in the 140828 client.  None of their client send paths installs a
// matching CMD response reader, so the dispatchers deliberately emit no
// synthetic success packet.

class Dispatcher_AddHackTypeCount
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_AddHackTypeCount();
    virtual ~Dispatcher_AddHackTypeCount();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_AbnormalFunctionCall
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_AbnormalFunctionCall();
    virtual ~Dispatcher_AbnormalFunctionCall();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_ReportClientHack
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ReportClientHack();
    virtual ~Dispatcher_ReportClientHack();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_ReportPvPLag
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ReportPvPLag();
    virtual ~Dispatcher_ReportPvPLag();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_VerifyPvPLagUser
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_VerifyPvPLagUser();
    virtual ~Dispatcher_VerifyPvPLagUser();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_SecurityStatus
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_SecurityStatus();
    virtual ~Dispatcher_SecurityStatus();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_AddHackTypeCount) == 0x04u,
              "CMD253 dispatcher must retain the base-only i386 ABI");
static_assert(sizeof(Dispatcher_AbnormalFunctionCall) == 0x04u,
              "CMD393 dispatcher must retain the base-only i386 ABI");
static_assert(sizeof(Dispatcher_ReportClientHack) == 0x04u,
              "CMD471 dispatcher must retain the base-only i386 ABI");
static_assert(sizeof(Dispatcher_ReportPvPLag) == 0x04u,
              "CMD478 dispatcher must retain the base-only i386 ABI");
static_assert(sizeof(Dispatcher_VerifyPvPLagUser) == 0x04u,
              "CMD479 dispatcher must retain the base-only i386 ABI");
static_assert(sizeof(Dispatcher_SecurityStatus) == 0x04u,
              "CMD587 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_SECURITY_SENDER_SECURITY_TELEMETRY_DISPATCHERS_H_
