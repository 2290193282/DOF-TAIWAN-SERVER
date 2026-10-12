#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_EVENT_SHOP_ATTENDANCE_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_EVENT_SHOP_ATTENDANCE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;
class PacketBuf;

namespace secret_event_shop {

// The dungeon-event-shop interaction owner calls OpenForUser only after it
// has validated NPC role 77 / NPC 164.  It installs the per-offer retailer
// and emits the exact NOTI 647/648 pair consumed by the 0828 client.
bool OpenForUser(CUser* user, int npc_index);
void CloseForUser(CUser* user);

}  // namespace secret_event_shop_0828

namespace event_attendance {

// Character-session and event-lifecycle hooks.  Login loads the account/day
// watermark.  WorldReady replays NOTI 651 only after the character has entered
// the world, GlobalMinuteTick drives all online users independently of packet
// traffic, and EventEnded seals every live accumulator at one shared epoch.
void RequestLoginState(CUser* user);
void OnWorldReady(CUser* user);
void OnTick(CUser* user);
void OnGlobalMinuteTick();
void OnEventStarted();
void OnEventEnded();
void ResetSession(CUser* user);

}  // namespace event_attendance_0828

class Dispatcher_SecretEventShopBuyItem
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_SecretEventShopBuyItem();
    virtual ~Dispatcher_SecretEventShopBuyItem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_EventAttendanceReward
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_EventAttendanceReward();
    virtual ~Dispatcher_EventAttendanceReward();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Inter_EventAttendance : public ISigDispatcher {
public:
    Inter_EventAttendance();
    virtual ~Inter_EventAttendance();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SecretEventShopBuyItem) == 0x04u,
              "CMD687 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Dispatcher_EventAttendanceReward) == 0x04u,
              "CMD689 dispatcher must retain its base-only i386 ABI");
static_assert(sizeof(Inter_EventAttendance) == 0x04u,
              "CMD689 Inter dispatcher must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_EVENT_SHOP_ATTENDANCE_0828_H_
