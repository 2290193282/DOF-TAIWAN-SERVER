#ifndef OLDSRV_SECURITY_BV_HACK_INFO_DISPATCHER_H_
#define OLDSRV_SECURITY_BV_HACK_INFO_DISPATCHER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA's decoded BV report consists of six consecutive doubles.
struct stBVHackInfo_t {
    double values[6];
};

static_assert(sizeof(stBVHackInfo_t) == 0x30u,
              "BV hack report must retain its 48-byte ABI");

// IDA_EA: 0x88920F1
// IDA_NAME: _Z19decryptBVHackPacketPcR14stBVHackInfo_t
// DEMANGLED: decryptBVHackPacket(char *,stBVHackInfo_t &)
int decryptBVHackPacket(char* packet, stBVHackInfo_t& output);

class Dispatcher_BVHackInfo
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596310
    // IDA_NAME: _ZN21Dispatcher_BVHackInfoC2Ev
    // DEMANGLED: Dispatcher_BVHackInfo::Dispatcher_BVHackInfo(void)
    Dispatcher_BVHackInfo();

    // IDA_EA: 0x8242616
    // IDA_NAME: _ZN21Dispatcher_BVHackInfoD2Ev
    // DEMANGLED: Dispatcher_BVHackInfo::~Dispatcher_BVHackInfo()
    // IDA_EA: 0x8242646
    // IDA_NAME: _ZN21Dispatcher_BVHackInfoD0Ev
    // DEMANGLED: Dispatcher_BVHackInfo::~Dispatcher_BVHackInfo()
    virtual ~Dispatcher_BVHackInfo();

    // IDA_EA: 0x8207568
    // IDA_NAME: _ZN21Dispatcher_BVHackInfo12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_BVHackInfo::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233966
    // IDA_NAME: _ZN21Dispatcher_BVHackInfo4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_BVHackInfo::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233970
    // IDA_NAME: _ZN21Dispatcher_BVHackInfo7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_BVHackInfo::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_BVHackInfo) == 0x04u,
              "CMD153 dispatcher must retain its four-byte i386 ABI");
#endif

#endif  // OLDSRV_SECURITY_BV_HACK_INFO_DISPATCHER_H_
