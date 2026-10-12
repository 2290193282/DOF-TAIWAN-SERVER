#include "old_game/dungeon/mission/packet_dispatcher_dungeon_mission.h"

#include "old_game/dungeon/mission/dungeon_mission.h"

Dispatcher_DungeonMissionStart::Dispatcher_DungeonMissionStart()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

Dispatcher_DungeonMissionStart::~Dispatcher_DungeonMissionStart() {}

int Dispatcher_DungeonMissionStart::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    return dungeon_mission::HandleStart(user, packet);
}

int Dispatcher_DungeonMissionStart::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_DungeonMissionStart::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_DungeonMissionUpdate::Dispatcher_DungeonMissionUpdate()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

Dispatcher_DungeonMissionUpdate::~Dispatcher_DungeonMissionUpdate() {}

int Dispatcher_DungeonMissionUpdate::dispatch_sig(CUser* user,
                                                        PacketBuf& packet) {
    return dungeon_mission::HandleUpdate(user, packet);
}

int Dispatcher_DungeonMissionUpdate::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_DungeonMissionUpdate::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_DungeonMissionCheckSuccess::
    Dispatcher_DungeonMissionCheckSuccess()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

Dispatcher_DungeonMissionCheckSuccess::
    ~Dispatcher_DungeonMissionCheckSuccess() {}

int Dispatcher_DungeonMissionCheckSuccess::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    return dungeon_mission::HandleCheckSuccess(user, packet);
}

int Dispatcher_DungeonMissionCheckSuccess::read(PacketBuf& packet,
                                                     MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_DungeonMissionCheckSuccess::process(
    CUser* user,
    MSG_BASE& message,
    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
