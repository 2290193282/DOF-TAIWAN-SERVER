#ifndef OLDSRV_OLD_GAME_DUNGEON_MISSION_PACKET_DISPATCHER_DUNGEON_MISSION_H_
#define OLDSRV_OLD_GAME_DUNGEON_MISSION_PACKET_DISPATCHER_DUNGEON_MISSION_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

class Dispatcher_DungeonMissionStart
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_DungeonMissionStart();
    virtual ~Dispatcher_DungeonMissionStart();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_DungeonMissionUpdate
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_DungeonMissionUpdate();
    virtual ~Dispatcher_DungeonMissionUpdate();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_DungeonMissionCheckSuccess
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_DungeonMissionCheckSuccess();
    virtual ~Dispatcher_DungeonMissionCheckSuccess();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DungeonMissionStart) == 0x04u,
              "CMD552 dispatcher must remain vptr-only on target i386");
static_assert(sizeof(Dispatcher_DungeonMissionUpdate) == 0x04u,
              "CMD553 dispatcher must remain vptr-only on target i386");
static_assert(sizeof(Dispatcher_DungeonMissionCheckSuccess) == 0x04u,
              "CMD554 dispatcher must remain vptr-only on target i386");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_MISSION_PACKET_DISPATCHER_DUNGEON_MISSION_H_
