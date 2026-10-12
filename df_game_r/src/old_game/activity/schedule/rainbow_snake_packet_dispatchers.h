#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_RAINBOW_SNAKE_PACKET_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_RAINBOW_SNAKE_PACKET_DISPATCHERS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

namespace RainbowSnake {

// REMOTE_DB_EVIDENCE: taiwan_game_event.event_ingame_history already owns
// the account-scoped (type, history_1, history_2) row used here.  Type 15 is
// reserved for Rainbow Snake; history_1 is the completed stage (0..8), and
// history_2 is the shared point gauge (0..200).
enum {
    kInGameEventHistoryType = 15,
};

}  // namespace RainbowSnake

class Dispatcher_AddRainbowPoint
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_AddRainbowPoint();
    virtual ~Dispatcher_AddRainbowPoint();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_RequestRainbowPoint
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_RequestRainbowPoint();
    virtual ~Dispatcher_RequestRainbowPoint();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_RequestRainbowPointReward
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_RequestRainbowPointReward();
    virtual ~Dispatcher_RequestRainbowPointReward();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_AddRainbowPoint) == 0x04u,
              "CMD579 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Dispatcher_RequestRainbowPoint) == 0x04u,
              "CMD580 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Dispatcher_RequestRainbowPointReward) == 0x04u,
              "CMD581 dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_RAINBOW_SNAKE_PACKET_DISPATCHERS_H_
