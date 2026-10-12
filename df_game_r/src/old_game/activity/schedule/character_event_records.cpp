#include "old_game/activity/schedule/character_event_records.h"

#include <cstring>

#include "common/config.h"
#include "infra/log/format/trace.h"


typedef char CharacterInServer_Size_Must_Be_0x08[
    (sizeof(online_preliminary::CharacterInServer) == 0x08) ? 1 : -1];
typedef char CCharacterDayEvent_Size_Must_Be_0x0c[
    (sizeof(CCharacterDayEvent) == 0x0c) ? 1 : -1];
typedef char EventNewCharacterReward_Size_Must_Be_0x14[
    (sizeof(EventNewCharacterReward) == 0x14) ? 1 : -1];
typedef char SIG_GIVE_GROWTH_CREATURE_EVENT_eventCharacInfo_Size_Must_Be_0x2c[
    (sizeof(SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo) == 0x2c)
        ? 1
        : -1];

// IDA_EA: 0x810A4C8
// IDA_NAME: _ZN18CCharacterDayEventC2Ev
// DEMANGLED: CCharacterDayEvent::CCharacterDayEvent(void)
CCharacterDayEvent::CCharacterDayEvent()
    : CEventBase() {
    reserved_09_[1] = 0xffu;
    reserved_09_[2] = 0xffu;
}

// IDA_EA: 0x810A4EE
// IDA_NAME: _ZN18CCharacterDayEventD2Ev
// DEMANGLED: CCharacterDayEvent::~CCharacterDayEvent()
// IDA_EA: 0x810A51E
// IDA_NAME: _ZN18CCharacterDayEventD0Ev
// DEMANGLED: CCharacterDayEvent::~CCharacterDayEvent() deleting destructor
CCharacterDayEvent::~CCharacterDayEvent() {}

// IDA_EA: 0x810A53C
// IDA_NAME: _ZN18CCharacterDayEvent10StartEventEv
// DEMANGLED: CCharacterDayEvent::StartEvent(void)
void CCharacterDayEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 23, 0);
    trace("Character Day Event Start!");
}

// IDA_EA: 0x810A58E
// IDA_NAME: _ZN18CCharacterDayEvent10StartEventE10Word_Param
// DEMANGLED: CCharacterDayEvent::StartEvent(Word_Param)
void CCharacterDayEvent::StartEvent(Word_Param param) {
    SetEventFlag(true);
    const uint16_t event_job = static_cast<uint16_t>(param);
    reserved_09_[1] = static_cast<uint8_t>(event_job & 0xffu);
    reserved_09_[2] = static_cast<uint8_t>((event_job >> 8) & 0xffu);
    cMyTrace trace("StartEvent", 31, 0);
    trace("Character Day Event Start!");
}

// IDA_EA: 0x810A5EC
// IDA_NAME: _ZN18CCharacterDayEvent8EndEventEv
// DEMANGLED: CCharacterDayEvent::EndEvent(void)
void CCharacterDayEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 37, 0);
    trace("Character Day Event End!");
}

// IDA_EA: 0x8697886
// IDA_NAME: _ZN18CCharacterDayEvent11GetEventJobEv
// DEMANGLED: CCharacterDayEvent::GetEventJob(void)
int CCharacterDayEvent::GetEventJob() {
    return static_cast<int>(
        static_cast<uint16_t>(reserved_09_[1]) |
        (static_cast<uint16_t>(reserved_09_[2]) << 8));
}

// IDA_EA: 0x816BAC4
// IDA_NAME: _ZN23EventNewCharacterRewardC2Ev
// DEMANGLED: EventNewCharacterReward::EventNewCharacterReward(void)
EventNewCharacterReward::EventNewCharacterReward()
    : CEventBase(),
      event_character_job(-1),
      exp_ratio(0) {}

// IDA_EA: 0x816BAF4
// IDA_NAME: _ZN23EventNewCharacterRewardD2Ev
// DEMANGLED: EventNewCharacterReward::~EventNewCharacterReward()
// IDA_EA: 0x816BB24
// IDA_NAME: _ZN23EventNewCharacterRewardD0Ev
// DEMANGLED: EventNewCharacterReward::~EventNewCharacterReward()
// deleting destructor
EventNewCharacterReward::~EventNewCharacterReward() {}

// IDA_EA: 0x816BB42
// IDA_NAME: _ZN23EventNewCharacterReward10StartEventEv
// DEMANGLED: EventNewCharacterReward::StartEvent(void)
void EventNewCharacterReward::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void EventNewCharacterReward::StartEvent()", 22,
                   0);
    trace("[Taiwan, EventCharacterBonus] Event start!");
}

// IDA_EA: 0x816BB94
// IDA_NAME: _ZN23EventNewCharacterReward10StartEventE10Word_Param
// DEMANGLED: EventNewCharacterReward::StartEvent(Word_Param)
void EventNewCharacterReward::StartEvent(Word_Param param) {
    const uint32_t raw_param = static_cast<uint32_t>(param);
    event_character_job = static_cast<uint16_t>(raw_param);
    exp_ratio = static_cast<uint16_t>(raw_param >> 16);
    add_info_.word_00 = static_cast<int16_t>(event_character_job);
    add_info_.word_02 = static_cast<int16_t>(exp_ratio);

    if (event_character_job < 0 ||
        event_character_job >= oldsrv::config::kCharacterJobCount) {
        cMyTrace trace(
            "virtual void EventNewCharacterReward::StartEvent(Word_Param)",
            39, 0);
        trace("[Taiwan, EventCharacterBonus] Invalid event parameters "
              "(job:%d, exp_ratio:%d)",
              event_character_job, exp_ratio);
        return;
    }

    cMyTrace trace("virtual void EventNewCharacterReward::StartEvent(Word_Param)",
                   34, 0);
    trace("[Taiwan, EventCharacterBonus] Event parameters "
          "(job:%d, exp_ratio:%d)",
          event_character_job, exp_ratio);
    static_cast<CEventBase*>(this)->StartEvent();
}

// IDA_EA: 0x816BC98
// IDA_NAME: _ZN23EventNewCharacterReward8EndEventEv
// DEMANGLED: EventNewCharacterReward::EndEvent(void)
void EventNewCharacterReward::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void EventNewCharacterReward::EndEvent()", 47,
                   0);
    trace("[Taiwan, EventCharacterBonus] Event stop!");
}

// IDA_EA: 0x816BCEA
// IDA_NAME: _ZN23EventNewCharacterReward16isEventCharacterEi
// DEMANGLED: EventNewCharacterReward::isEventCharacter(int)
int EventNewCharacterReward::isEventCharacter(int charac_job) {
    return event_character_job == charac_job;
}

// IDA_EA: 0x816BD06
// IDA_NAME: _ZN23EventNewCharacterReward13eventExpRatioEv
// DEMANGLED: EventNewCharacterReward::eventExpRatio(void)
long double EventNewCharacterReward::eventExpRatio() {
    return static_cast<long double>(exp_ratio) / 100.0L;
}

// IDA_EA: 0x816BD22
// IDA_NAME: _ZN23EventNewCharacterReward17eventCharacterJobEv
// DEMANGLED: EventNewCharacterReward::eventCharacterJob(void)
int EventNewCharacterReward::eventCharacterJob() {
    return event_character_job;
}

// IDA_EA: 0x85868D4
// IDA_NAME: _ZNK18online_preliminary17CharacterInServerltERKS0_
// DEMANGLED: online_preliminary::CharacterInServer::operator<(online_preliminary::CharacterInServer const&)const
bool online_preliminary::CharacterInServer::operator<(
    const CharacterInServer& rhs) const {
    if (server_group < rhs.server_group) {
        return true;
    }
    return server_group == rhs.server_group && charac_no < rhs.charac_no;
}

// IDA_EA: 0x858691E
// IDA_NAME: _ZN18online_preliminary17CharacterInServerC2Eij
// DEMANGLED: online_preliminary::CharacterInServer::CharacterInServer(int,unsigned int)
online_preliminary::CharacterInServer::CharacterInServer(
    int server_group_value, uint32_t charac_no_value)
    : server_group(static_cast<uint16_t>(server_group_value)),
      charac_no(charac_no_value) {}

// IDA_EA: 0x81B7416
// IDA_NAME: _ZN30SIG_GIVE_GROWTH_CREATURE_EVENT15eventCharacInfoC2Ev
// DEMANGLED: SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo::eventCharacInfo(void)
SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo::eventCharacInfo()
    : job(0),
      pad2(0),
      charac_no(0),
      creature_id(0),
      creature_name() {}
