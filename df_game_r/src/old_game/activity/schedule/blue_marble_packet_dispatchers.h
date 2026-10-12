#ifndef OLDSRV_OLD_GAME_EVENT_BLUE_MARBLE_PACKET_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_EVENT_BLUE_MARBLE_PACKET_DISPATCHERS_H_

#include "old_game/activity/schedule/event_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

// CMD 496 / BLUE_MARBLE.
class Dispatcher_BlueMarble
    : public IPacketDispatcher<MSG_BLUE_MARBLE,
                               stBlueMarble,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_BlueMarble();
    virtual ~Dispatcher_BlueMarble();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_BlueMarble) == 0x04u,
              "CMD496 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_BLUE_MARBLE_PACKET_DISPATCHERS_H_
