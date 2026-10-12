#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_LOGIN_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_LOGIN_DISPATCHER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD 1 / LOGIN.  The old table stores this exact state-0 template-derived
// provider at PacketDispatcher slot 1.  Registration remains with the root
// transaction and is enabled only alongside Inter_LoginInfo, which consumes
// the SIG_LOGIN_DATA DB reply.
class DisPatcher_Login
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85953D2
    // IDA_NAME: _ZN16DisPatcher_LoginC2Ev
    DisPatcher_Login();

    // IDA_EA: 0x8232A1C / 0x8232A4C
    // IDA_NAME: _ZN16DisPatcher_LoginD2Ev / _ZN16DisPatcher_LoginD0Ev
    virtual ~DisPatcher_Login();

    // IDA_EA: 0x81E8C78
    // IDA_NAME: _ZN16DisPatcher_Login12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8232A6A / 0x8232A74
    // IDA_NAME: _ZN16DisPatcher_Login4readER9PacketBufR8MSG_BASE
    //           _ZN16DisPatcher_Login7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

private:
    uint32_t version_mismatch_count_;  // +0x04
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_Login) == 0x08u,
              "DisPatcher_Login must retain its old i386 vptr+counter ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_LOGIN_DISPATCHER_H_
