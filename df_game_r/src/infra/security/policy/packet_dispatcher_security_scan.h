#ifndef OLDSRV_SECURITY_PACKET_DISPATCHER_SECURITY_SCAN_H_
#define OLDSRV_SECURITY_PACKET_DISPATCHER_SECURITY_SCAN_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 283, 284, 286, 294, 295, 319 and 320.  Each object is vptr-only in
// the old i386 server and owns the standard MSG_BASE/ParamBase/state-0
// dispatch vtable surface.

class Dispatcher_Security_ProcessScan
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Security_ProcessScan();
    virtual ~Dispatcher_Security_ProcessScan();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_Security_IPScan
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Security_IPScan();
    virtual ~Dispatcher_Security_IPScan();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_Security_PacketIntegrity
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Security_PacketIntegrity();
    virtual ~Dispatcher_Security_PacketIntegrity();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_Security_NonClient_GetResponse
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Security_NonClient_GetResponse();
    virtual ~Dispatcher_Security_NonClient_GetResponse();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_Security_MemoryIntegrity
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Security_MemoryIntegrity();
    virtual ~Dispatcher_Security_MemoryIntegrity();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_Security_PI_HashModulation_S2C
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Security_PI_HashModulation_S2C();
    virtual ~Dispatcher_Security_PI_HashModulation_S2C();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_Security_PI_HashModulation_C2C
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Security_PI_HashModulation_C2C();
    virtual ~Dispatcher_Security_PI_HashModulation_C2C();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Security_ProcessScan) == 0x04u,
              "CMD283 must remain vptr-only");
static_assert(sizeof(Dispatcher_Security_IPScan) == 0x04u,
              "CMD284 must remain vptr-only");
static_assert(sizeof(Dispatcher_Security_PacketIntegrity) == 0x04u,
              "CMD286 must remain vptr-only");
static_assert(sizeof(Dispatcher_Security_NonClient_GetResponse) == 0x04u,
              "CMD294 must remain vptr-only");
static_assert(sizeof(Dispatcher_Security_MemoryIntegrity) == 0x04u,
              "CMD295 must remain vptr-only");
static_assert(sizeof(Dispatcher_Security_PI_HashModulation_S2C) == 0x04u,
              "CMD319 must remain vptr-only");
static_assert(sizeof(Dispatcher_Security_PI_HashModulation_C2C) == 0x04u,
              "CMD320 must remain vptr-only");
#endif

#endif  // OLDSRV_SECURITY_PACKET_DISPATCHER_SECURITY_SCAN_H_
