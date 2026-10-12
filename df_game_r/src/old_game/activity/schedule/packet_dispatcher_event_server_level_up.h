#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_EVENT_SERVER_LEVEL_UP_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_EVENT_SERVER_LEVEL_UP_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// Native 0828 CMD415 reconstruction.  The 0627 server exposes the command
// enum but no registered dispatcher, so this class deliberately has no
// synthetic 0627 IDA address attribution.
class Dispatcher_RequestEventServerLevelUp
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_RequestEventServerLevelUp();
    virtual ~Dispatcher_RequestEventServerLevelUp();

    virtual int dispatch_template(CUser* user, PacketBuf& packet);
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestEventServerLevelUp) == 0x04u,
              "CMD415 dispatcher must remain vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_EVENT_SERVER_LEVEL_UP_H_
