#include "old_game/dungeon/instance/packet_dispatcher_instant_reinforce.h"

#include "old_game/dungeon/instance/instant_reinforce.h"
#include "old_game/player/core/cuser.h"

Dispatcher_RequestInstantReinforce::
    Dispatcher_RequestInstantReinforce()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(5)>() {}

Dispatcher_RequestInstantReinforce::
    ~Dispatcher_RequestInstantReinforce() {}

int Dispatcher_RequestInstantReinforce::check_state(
    CUser* user,
    PacketBuf& packet) {
    (void)packet;
    return (!user || user->get_state() != 5 || user->is_fighting() == 0)
               ? 1
               : 0;
}

int Dispatcher_RequestInstantReinforce::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    return instant_reinforce::HandleRequest(user, packet);
}

int Dispatcher_RequestInstantReinforce::read(PacketBuf& packet,
                                                  MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_RequestInstantReinforce::process(
    CUser* user,
    MSG_BASE& message,
    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
