#ifndef OLDSRV_OLD_GAME_WORLD_SOLO_TELEPORT_DISPATCHER_H_
#define OLDSRV_OLD_GAME_WORLD_SOLO_TELEPORT_DISPATCHER_H_

#include "old_data/Login/packet_dispatch/precheck_solo_teleport_dispatcher.h"

// IDA unit: 13_database_dispatch/Dispatcher_SoloTeleport.
// CMD470 and CMD471 intentionally share the existing state-0 typed template;
// this class contributes only its distinct C2/D2/D0 and virtual business
// methods.  It remains detached from the frozen 606-slot registration table.
class Dispatcher_SoloTeleport
    : public IPacketDispatcher<MSG_SOLO_TELEPORT,
                               stSoloTeleportParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597B96
    Dispatcher_SoloTeleport();
    virtual ~Dispatcher_SoloTeleport();

    // IDA_EA: 0x81E1B9C / 0x81E1D9C / 0x81E1FF6
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E1E98
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SoloTeleport) == 0x04u,
              "CMD471 dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_WORLD_SOLO_TELEPORT_DISPATCHER_H_
