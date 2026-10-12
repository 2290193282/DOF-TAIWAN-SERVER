#ifndef OLDSRV_OLD_GAME_GAME_MASTER_DISPATCHER_GM_COMMAND_H_
#define OLDSRV_OLD_GAME_GAME_MASTER_DISPATCHER_GM_COMMAND_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 13_database_dispatch/Dispatcher_GM_Command.  CMD110 uses its
// dispatch_sig body directly; the inherited template hooks are empty in the
// original server.
class Dispatcher_GM_Command
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595698 / 0x823379C / 0x82337CC.
    Dispatcher_GM_Command();
    virtual ~Dispatcher_GM_Command();

    // IDA_EA: 0x81FA19E / 0x8233788 / 0x8233792.
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GM_Command) == 0x04u,
              "CMD110 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GAME_MASTER_DISPATCHER_GM_COMMAND_H_
