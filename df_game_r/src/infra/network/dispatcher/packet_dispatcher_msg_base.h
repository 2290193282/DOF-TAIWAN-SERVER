#ifndef OLDSRV_PROTOCOL_PACKET_DISPATCHER_MSG_BASE_H_
#define OLDSRV_PROTOCOL_PACKET_DISPATCHER_MSG_BASE_H_

#include "old_data/db/core/i_dispatcher.h"
#include "infra/network/packet/message_base.h"


enum ch_state : int;

class CUser;
class PacketBuf;

// The old server has many concrete template instantiations.  Their common
// virtual ABI is declared here, but definitions intentionally live in the
// instantiation-only .tpp.  A concrete owner must explicitly instantiate one
// triplet after its message and parameter types are complete; including the
// declarations alone must never create an accidental COMDAT/vtable owner.
template <typename MSG_TYPE, typename PARAM_TYPE, ch_state STATE>
class IPacketDispatcher : public IDispatcher {
public:
    IPacketDispatcher();
    virtual ~IPacketDispatcher();

    // Vtable order after IDispatcher: dispatch_template, D2, D0,
    // dispatch_sig, check_state, read, process, send.
    virtual int dispatch_template(CUser* user, PacketBuf& packet);
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int check_state(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message) = 0;
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param) = 0;
    virtual void send(CUser* user, ParamBase& param);
};

template <>
class IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>
    : public IDispatcher {
public:
    // IDA_EA: 0x8236A56
    // IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EEC2Ev
    // DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::IPacketDispatcher(void)
    IPacketDispatcher();

    // IDA_EA: 0x8125696
    // IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EED2Ev
    // DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::~IPacketDispatcher()
    // IDA_EA: 0x81256C6
    // IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EED0Ev
    // DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::~IPacketDispatcher()
    virtual ~IPacketDispatcher();

    // IDA_EA: 0x81258B6
    // IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
    // DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
    virtual int dispatch_template(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8125A9E
    // IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8125A06
    // IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EE11check_stateEP5CUserR9PacketBuf
    // DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::check_state(CUser *,PacketBuf &)
    virtual int check_state(CUser* user, PacketBuf& packet);

    virtual int read(PacketBuf& packet, MSG_BASE& message) = 0;
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param) = 0;

    // IDA_EA: 0x8125A98
    // IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state0EE4sendEP5CUserRS1_
    // DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)0>::send(CUser *,ParamBase&)
    virtual void send(CUser* user, ParamBase& param);
};


#endif  // OLDSRV_PROTOCOL_PACKET_DISPATCHER_MSG_BASE_H_
