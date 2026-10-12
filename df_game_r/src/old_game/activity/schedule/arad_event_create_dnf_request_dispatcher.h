#ifndef OLDSRV_OLD_GAME_EVENT_ARAD_EVENT_CREATE_DNF_REQUEST_DISPATCHER_H_
#define OLDSRV_OLD_GAME_EVENT_ARAD_EVENT_CREATE_DNF_REQUEST_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

namespace ARAD {
namespace DISPATCHER {

// CMD600 / ENUM_CMDPACKET_EVENT_CREATE_DNF_REQUEST.  The old object is only
// the state-0 dispatcher base and remains unregistered until the original
// 606-slot PacketDispatcher construction transaction is restored.
class Dispatcher_EventCreateDnfRequest
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597F4E
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequestC2Ev
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::Dispatcher_EventCreateDnfRequest(void)
    Dispatcher_EventCreateDnfRequest();

    // IDA_EA: 0x81A0C1C
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequestD2Ev
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::~Dispatcher_EventCreateDnfRequest()
    // IDA_EA: 0x81A0C4C
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequestD0Ev
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::~Dispatcher_EventCreateDnfRequest()
    virtual ~Dispatcher_EventCreateDnfRequest();

    // IDA_EA: 0x81A0362
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequest12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x81A0B72
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequest4readER9PacketBufR8MSG_BASE
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81A0B7C
    // IDA_NAME: _ZN4ARAD10DISPATCHER32Dispatcher_EventCreateDnfRequest7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_EventCreateDnfRequest::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_EventCreateDnfRequest) == 0x04u,
              "CMD600 dispatcher must retain its i386 base-only ABI");
#endif

}  // namespace DISPATCHER
}  // namespace ARAD

#endif  // OLDSRV_OLD_GAME_EVENT_ARAD_EVENT_CREATE_DNF_REQUEST_DISPATCHER_H_
