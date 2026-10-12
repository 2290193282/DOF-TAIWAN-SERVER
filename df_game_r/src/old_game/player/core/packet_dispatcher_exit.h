#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_EXIT_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_EXIT_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/DisPatcher_Exit. CMD003 performs its
// complete logout transaction in dispatch_sig; the template read/process
// hooks are intentionally empty in the original server.
class DisPatcher_Exit
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859544C
    DisPatcher_Exit();
    // IDA_EA: 0x8232BF4 / 0x8232C24
    virtual ~DisPatcher_Exit();

    // IDA_EA: 0x81ED948 / 0x8232BE0 / 0x8232BEA
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_Exit) == 0x04u,
              "CMD003 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_EXIT_H_
