#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_REQUEST_SERIA_BUFF_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_REQUEST_SERIA_BUFF_DISPATCHER_H_

#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD433 has no body.  It reuses MSG_BASE and the existing growth-power
// response object, while preserving the old state-0 template specialization.
class Dispatcher_Request_Seria_Buff
    : public IPacketDispatcher<MSG_BASE,
                               stGrowthPowerParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597912
    // IDA_NAME: _ZN29Dispatcher_Request_Seria_BuffC2Ev
    // DEMANGLED: Dispatcher_Request_Seria_Buff::Dispatcher_Request_Seria_Buff(void)
    Dispatcher_Request_Seria_Buff();

    // IDA_EA: 0x8244920 / 0x8244950
    // IDA_NAME: _ZN29Dispatcher_Request_Seria_BuffD2Ev /
    //           _ZN29Dispatcher_Request_Seria_BuffD0Ev
    // DEMANGLED: Dispatcher_Request_Seria_Buff::~Dispatcher_Request_Seria_Buff()
    virtual ~Dispatcher_Request_Seria_Buff();

    // IDA_EA: 0x81DBCCE / 0x81DBDA0
    // IDA_NAME: _ZN29Dispatcher_Request_Seria_Buff4readER9PacketBufR8MSG_BASE /
    //           _ZN29Dispatcher_Request_Seria_Buff7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Request_Seria_Buff::read(PacketBuf &,MSG_BASE &) /
    //            Dispatcher_Request_Seria_Buff::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81DBE08
    // IDA_NAME: _ZN29Dispatcher_Request_Seria_Buff4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_Request_Seria_Buff::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81DBCD8
    // IDA_NAME: _ZN29Dispatcher_Request_Seria_Buff11check_errorEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Request_Seria_Buff::check_error(
    //             CUser *,MSG_BASE &,ParamBase &)
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Request_Seria_Buff) == 0x04u,
              "CMD433 dispatcher must retain the old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_REQUEST_SERIA_BUFF_DISPATCHER_H_
