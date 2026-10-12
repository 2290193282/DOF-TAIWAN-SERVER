#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_EVENT_ALL_USER_GIFT_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_EVENT_ALL_USER_GIFT_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;
class PacketBuf;

namespace oldsrv {
namespace policy {

// The automatic Lv50 artifact selector is not part of the retained 0828
// service surface.  Keep its wire/DB owners registered so stale clients and
// in-flight replies fail closed instead of falling into an unknown dispatcher.
static const bool kEventAllUserGiftEnabled = false;

}  // namespace policy
}  // namespace oldsrv

class Dispatcher_EventAllUserGift
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_EventAllUserGift();
    virtual ~Dispatcher_EventAllUserGift();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Inter_EventAllUserGift : public ISigDispatcher {
public:
    Inter_EventAllUserGift();
    virtual ~Inter_EventAllUserGift();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

namespace event_all_user_gift {

// Town-ready integration point.  Call once from GameWorld::reach_game_world
// immediately after its canonical set_state(3), not from Inter_LoadEtc
// (state 2) or Area::insert_user (repeats on movement).  This performs only a
// DB eligibility query; the empty NOTI368 is emitted after the account is
// proven absent from event_1306_account_reward.
void QueueEligibilityCheck(CUser* user);

}  // namespace event_all_user_gift_0828

#if defined(__i386__)
static_assert(sizeof(Dispatcher_EventAllUserGift) == 0x04u,
              "CMD420 dispatcher must remain vptr-only on target i386");
static_assert(sizeof(Inter_EventAllUserGift) == 0x04u,
              "all-user-gift inter owner must remain vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_EVENT_ALL_USER_GIFT_0828_H_
