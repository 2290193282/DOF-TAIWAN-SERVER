#include "infra/network/packet/stream.h"

#include "old_data/Login/inter_returns/inter_select_mobile_auth_reward.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"
#include "old_game/activity/schedule/event_value_types.h"

static_assert(sizeof(InterSelectCreateDnfEventInfo::Msg) == 0x0Cu,
              "InterSelectCreateDnfEventInfo::Msg IDA stream size must stay 12");
static_assert(sizeof(InterSelectPcroomDailyReward::Msg) == 0x08u,
              "InterSelectPcroomDailyReward::Msg IDA stream size must stay 8");
static_assert(sizeof(InterSelectMobileAuthReward::Msg) == 0x08u,
              "InterSelectMobileAuthReward::Msg IDA stream size must stay 8");
static_assert(sizeof(InformNoticeFlagData) == 0x14u,
              "InformNoticeFlagData IDA stream size must stay 20");

// IDA_EA: 0x8162C06
// IDA_NAME: _ZN6Stream11GetInBufferIN29InterSelectCreateDnfEventInfo3MsgEEEPT_v
// DEMANGLED: InterSelectCreateDnfEventInfo::Msg * Stream::GetInBuffer<InterSelectCreateDnfEventInfo::Msg>(void)
template <>
InterSelectCreateDnfEventInfo::Msg*
Stream::GetInBuffer<InterSelectCreateDnfEventInfo::Msg>() {
    const int bytes = sizeof(InterSelectCreateDnfEventInfo::Msg);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    InterSelectCreateDnfEventInfo::Msg* const result =
        reinterpret_cast<InterSelectCreateDnfEventInfo::Msg*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8162CDA
// IDA_NAME: _ZN6Stream11GetInBufferIN27InterSelectMobileAuthReward3MsgEEEPT_v
// DEMANGLED: InterSelectMobileAuthReward::Msg * Stream::GetInBuffer<InterSelectMobileAuthReward::Msg>(void)
template <>
InterSelectMobileAuthReward::Msg*
Stream::GetInBuffer<InterSelectMobileAuthReward::Msg>() {
    const int bytes = sizeof(InterSelectMobileAuthReward::Msg);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    InterSelectMobileAuthReward::Msg* const result =
        reinterpret_cast<InterSelectMobileAuthReward::Msg*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8162D44
// IDA_NAME: _ZN6Stream11GetInBufferIN28InterSelectPcroomDailyReward3MsgEEEPT_v
// DEMANGLED: InterSelectPcroomDailyReward::Msg * Stream::GetInBuffer<InterSelectPcroomDailyReward::Msg>(void)
template <>
InterSelectPcroomDailyReward::Msg*
Stream::GetInBuffer<InterSelectPcroomDailyReward::Msg>() {
    const int bytes = sizeof(InterSelectPcroomDailyReward::Msg);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    InterSelectPcroomDailyReward::Msg* const result =
        reinterpret_cast<InterSelectPcroomDailyReward::Msg*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x869FCF2
// IDA_NAME: _ZN6Stream11GetInBufferI20InformNoticeFlagDataEEPT_v
// DEMANGLED: InformNoticeFlagData * Stream::GetInBuffer<InformNoticeFlagData>(void)
template <>
InformNoticeFlagData* Stream::GetInBuffer<InformNoticeFlagData>() {
    const int bytes = sizeof(InformNoticeFlagData);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    InformNoticeFlagData* const result =
        reinterpret_cast<InformNoticeFlagData*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}
