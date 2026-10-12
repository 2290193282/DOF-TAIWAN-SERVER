#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_FREELY_GIVE_ITEM_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_FREELY_GIVE_ITEM_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_game/activity/reward/event_base.h"

#include <stdint.h>

class CUser;
class PacketBuf;

class FreelyGiveItemEvent : public CEventBase {
public:
    FreelyGiveItemEvent();
    virtual ~FreelyGiveItemEvent();

    virtual void StartEvent();
    virtual void StartEvent(Word_Param param);
    virtual void EndEvent();
    virtual void StartAction();
};

namespace freely_give_item {

// Character/login lifecycle.  RequestLoad belongs after the selected
// character's ordinary inventory and etc-data load has completed.
void RequestLoad(CUser* user);
void FlushAndErase(CUser* user);
void ResetSession(CUser* user);
void OnGlobalMinuteTick();

// Authoritative progress sources proven by NOTI608 reason codes and the
// deployed freelygiveitemevent.evt.
void OnFatigueUsed(CUser* user, uint32_t fatigue_delta);
void OnLevelUp(CUser* user, uint32_t new_level);
// occurrence_key is a stable, monotonically increasing identifier owned by
// the authoritative timed-event producer. Replays are ignored durably.
void OnTimedLoginAward(CUser* user, uint32_t occurrence_key);

// Booster-selection result hook for gift box 690001061.  The caller must
// invoke this only after the canonical booster flow has inserted result_item.
void OnBoosterSelectionResult(CUser* user,
                              uint32_t source_item_id,
                              uint32_t result_item_id);

// Upgrade hooks run only after the native inventory mutation reports success.
void OnNormalUpgradeResult(CUser* user,
                           uint32_t item_id,
                           uint8_t upgrade,
                           bool succeeded);
// The 0828 PVF "upgrade disjoint" pair is a destruction-result threshold,
// not the independent/separate-upgrade stat stored on the item.
void OnUpgradeDisjoint(CUser* user,
                       uint32_t item_id,
                       uint8_t source_upgrade,
                       bool destroyed);

// Generic item-deletion wiring marks a live Phoenix weapon failed unless the
// feature is currently performing its own convert/reset transition.
void OnEventWeaponRemoved(CUser* user, uint32_t item_id);

// Inventory/equipment moves call this to refresh the two location flags in
// NOTI607 without changing authoritative state.
void SendEventInfo(CUser* user);

}  // namespace freely_give_item_0828

#define DECLARE_FREELY_GIVE_DISPATCHER(class_name)                         \
class class_name                                                          \
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)> { \
public:                                                                   \
    class_name();                                                         \
    virtual ~class_name();                                                \
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);             \
    virtual int read(PacketBuf& packet, MSG_BASE& message);               \
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param); \
}

DECLARE_FREELY_GIVE_DISPATCHER(Dispatcher_RequestFreelyGiveItemBox);
DECLARE_FREELY_GIVE_DISPATCHER(Dispatcher_RequestCouponChange);
DECLARE_FREELY_GIVE_DISPATCHER(Dispatcher_UseFreelyGiveItemCoupon);
DECLARE_FREELY_GIVE_DISPATCHER(Dispatcher_ChangePeriodicToUnlimited);
DECLARE_FREELY_GIVE_DISPATCHER(Dispatcher_InitFreelyGiveItemEvent);

#undef DECLARE_FREELY_GIVE_DISPATCHER

class InterFreelyGiveItem : public TaiwanInterfaceInterDispatcher {
public:
    InterFreelyGiveItem();
    virtual ~InterFreelyGiveItem();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(FreelyGiveItemEvent) == 0x0Cu,
              "Phoenix repeat event must remain base-only");
static_assert(sizeof(Dispatcher_RequestFreelyGiveItemBox) == 0x04u,
              "CMD530 dispatcher must remain vptr-only");
static_assert(sizeof(Dispatcher_RequestCouponChange) == 0x04u,
              "CMD531 dispatcher must remain vptr-only");
static_assert(sizeof(Dispatcher_UseFreelyGiveItemCoupon) == 0x04u,
              "CMD532 dispatcher must remain vptr-only");
static_assert(sizeof(Dispatcher_ChangePeriodicToUnlimited) == 0x04u,
              "CMD533 dispatcher must remain vptr-only");
static_assert(sizeof(Dispatcher_InitFreelyGiveItemEvent) == 0x04u,
              "CMD534 dispatcher must remain vptr-only");
static_assert(sizeof(InterFreelyGiveItem) == 0x08u,
              "Phoenix Taiwan Inter owner must preserve the base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_FREELY_GIVE_ITEM_0828_H_
