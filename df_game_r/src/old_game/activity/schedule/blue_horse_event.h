#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_BLUE_HORSE_EVENT_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_BLUE_HORSE_EVENT_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_game/activity/reward/event_base.h"

#include <stdint.h>

class CUser;
class PacketBuf;

namespace blue_horse_event {

struct OnTimeImdReward {
    uint32_t item_id;
    uint8_t count;
};

struct FortuneData {
    uint16_t field_0;
    uint16_t field_1;
    uint16_t field_2;
    uint8_t field_3;
    uint8_t field_4;
    uint8_t field_5;
    uint8_t field_6;
};

// Town-ready lifecycle plus the exact committed-fatigue hook.  The latter is
// placed after CUser::FatigueUp has accepted a positive fatigue delta.
void BeginSession(CUser* user);
void ResetSession(CUser* user);
void FlushAndErase(CUser* user);
void OnFatigueUsed(CUser* user, uint32_t amount);

// Exact client-reader serializers.  Only SendBlueHorseData has an enabled
// business producer in this feature; the others remain useful, gated wire
// owners and are not registration evidence by themselves.
void SendOnTimeImdState(CUser* user, uint8_t state);
void SendOnTimeImdRewardState(CUser* user, uint8_t available);
void SendSeaBottomInfo(CUser* user,
                       uint32_t field_0,
                       const uint32_t* item_ids,
                       uint32_t count);
void SendTripleExpKeepingInfo(CUser* user,
                              uint8_t field_0,
                              uint8_t field_1,
                              uint8_t field_2,
                              uint32_t field_3,
                              uint32_t field_4,
                              uint8_t field_5);
void SendFortuneData(CUser* user, const FortuneData& data);
void SendFortuneBroadcast(CUser* user,
                          const char* character_name,
                          uint32_t character_name_bytes,
                          uint16_t field_0,
                          uint16_t field_1,
                          uint16_t field_2,
                          uint8_t field_3);
void SendBlueHorseData(CUser* user,
                       uint8_t show_ui,
                       uint8_t ticket_count,
                       const uint8_t slots[3],
                       uint8_t distance,
                       uint8_t completed);

bool HasNativeOnTimeImdClosure();
bool HasNativeSeaBottomClosure();
bool HasNativeTripleExpKeepingClosure();
bool HasNativeFortuneClosure();
bool HasNativeBlueHorseClosure();

}  // namespace tw_late_event_0828

// Official 0828 eventnotice.evt event id 260 and client RTTI both name this
// owner BlueHorseEvent.  It deliberately carries no fields beyond CEventBase;
// all character state is feature-owned and durable in DB_HANDLE_2.
class BlueHorseEvent : public CEventBase {
public:
    BlueHorseEvent();
    virtual ~BlueHorseEvent();

    virtual void StartEvent();
    virtual void StartEvent(Word_Param param);
    virtual void EndEvent();
};

namespace blue_horse_event {

// Event 260 is newer than the fixed 0627 CEventManager::repeat_events_[166]
// storage.  These helpers preserve that public i386 object layout while
// giving the normal event-table loader a single, explicitly owned sidecar.
bool InstallBlueHorseEventSidecar(CEventBase* event);
CEventBase* GetBlueHorseEventSidecar();
void DestroyBlueHorseEventSidecar();

}  // namespace tw_late_event_0828

class Dispatcher_PlayBlueHorseSlot
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_PlayBlueHorseSlot();
    virtual ~Dispatcher_PlayBlueHorseSlot();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_RewardBlueHorseSlot
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_RewardBlueHorseSlot();
    virtual ~Dispatcher_RewardBlueHorseSlot();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class InterBlueHorseEvent : public TaiwanInterfaceInterDispatcher {
public:
    InterBlueHorseEvent();
    virtual ~InterBlueHorseEvent();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PlayBlueHorseSlot) == 0x04u,
              "CMD730 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_RewardBlueHorseSlot) == 0x04u,
              "CMD731 dispatcher must remain vptr-only on i386");
static_assert(sizeof(InterBlueHorseEvent) == 0x08u,
              "Taiwan inner-29 Inter owner must preserve the base ABI");
static_assert(sizeof(BlueHorseEvent) == 0x0cu,
              "BlueHorseEvent must remain a base-only event owner on i386");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_TW_LATE_EVENT_726_731_0828_H_
