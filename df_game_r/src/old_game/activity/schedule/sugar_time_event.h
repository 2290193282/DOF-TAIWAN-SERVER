#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_SUGAR_TIME_EVENT_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_SUGAR_TIME_EVENT_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include <stdint.h>
#include <vector>

class CUser;
class PacketBuf;

namespace taiwan_reward_event_notifications {

struct SenseOfChoiceState {
    uint32_t day_or_result_id;
    uint8_t active;
    uint8_t first[3];
    std::vector<uint16_t> label_utf16;
    uint8_t second_marker;
    uint8_t second[3];
    uint8_t third_marker;
    uint8_t third[3];
};

// Exact client-reader serializers.  They are wire owners only: the absence
// of a native aggregate/binding producer is deliberately not registration
// authority for CMD734/CMD735.
bool SendSenseOfChoiceState(CUser* user,
                            const SenseOfChoiceState& state);
bool SendPresentReturnState(CUser* user,
                            const uint32_t* item_ids,
                            uint32_t item_count,
                            const std::vector<uint16_t>& partner_utf16);
bool SendBreakTrapSuccess(CUser* user,
                          uint8_t target_x,
                          uint8_t target_y,
                          uint8_t trap_result);

bool HasNativeSenseOfChoiceClosure();
bool HasNativePresentReturnClosure();
bool HasNativeBreakTrapClosure();

}  // namespace tw_late_event_734_741_0828

namespace sugar_time {

// Event activation is explicit and defaults to false.  OnEventStarted is only
// an owner intent: every load/mutation must additionally pass the default-off
// SQL030 campaign row (event_key, time window and fatigue reset hour) before
// any command is handled.  No guessed repeat-event ID is embedded here.
void OnEventStarted();
void OnEventEnded();
void RequestLoginState(CUser* user);
void OnWorldReady(CUser* user);
void OnTick(CUser* user);
void OnGlobalMinuteTick();
void ResetSession(CUser* user);

// Exact packed NOTI709/710 producers.  The two reserved bytes in NOTI709 and
// three reserved bytes in NOTI710 are always serialized as zero.
void SendDayState(CUser* user,
                  uint32_t remaining_seconds,
                  uint8_t next_stage,
                  uint8_t stage_index);
void SendFinalState(CUser* user,
                    uint32_t accounted_seconds,
                    uint32_t selected_goal_seconds,
                    uint8_t final_claimed);

bool HasNativeSugarTimeClosure();

}  // namespace sugar_time_0828

class Dispatcher_SugarTimeDayReward
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_SugarTimeDayReward();
    virtual ~Dispatcher_SugarTimeDayReward();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_SugarTimeFinalReward
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_SugarTimeFinalReward();
    virtual ~Dispatcher_SugarTimeFinalReward();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_SugarTimeFinalPick
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_SugarTimeFinalPick();
    virtual ~Dispatcher_SugarTimeFinalPick();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class InterSugarTime : public TaiwanInterfaceInterDispatcher {
public:
    InterSugarTime();
    virtual ~InterSugarTime();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SugarTimeDayReward) == 0x04u,
              "CMD739 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_SugarTimeFinalReward) == 0x04u,
              "CMD740 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_SugarTimeFinalPick) == 0x04u,
              "CMD741 dispatcher must remain vptr-only on i386");
static_assert(sizeof(InterSugarTime) == 0x08u,
              "Taiwan inner-30 Inter owner must preserve the base ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_TW_LATE_EVENT_734_741_0828_H_
