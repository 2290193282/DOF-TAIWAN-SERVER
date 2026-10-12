#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_HUMAN_CERTIFY_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_HUMAN_CERTIFY_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/Dispatcher_HumanCertify.  The wire work is
// a direct dispatch_sig transaction; read/process are the original five-byte
// return-zero members retained for the PacketDispatcher vtable ABI.
class Dispatcher_HumanCertify
    : public IPacketDispatcher<MSG_BASE, ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596230
    // IDA_NAME: _ZN23Dispatcher_HumanCertifyC2Ev
    // DEMANGLED: Dispatcher_HumanCertify::Dispatcher_HumanCertify(void)
    Dispatcher_HumanCertify();

    // IDA_EA: 0x8242886
    // IDA_NAME: _ZN23Dispatcher_HumanCertifyD2Ev
    // DEMANGLED: Dispatcher_HumanCertify::~Dispatcher_HumanCertify()
    // IDA_EA: 0x82428B6
    // IDA_NAME: _ZN23Dispatcher_HumanCertifyD0Ev
    // DEMANGLED: Dispatcher_HumanCertify::~Dispatcher_HumanCertify()
    virtual ~Dispatcher_HumanCertify();

    // IDA_EA: 0x82064D6
    // IDA_NAME: _ZN23Dispatcher_HumanCertify12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_HumanCertify) == 0x04u,
              "CMD145 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_HUMAN_CERTIFY_H_
