#include "old_game/activity/schedule/hero_mission_runtime.h"

#include <algorithm>
#include <cstring>
#include <iterator>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x81681C8
// IDA_NAME: _ZN20HeroMissionConditionC2Ev
// DEMANGLED: HeroMissionCondition::HeroMissionCondition(void)
HeroMissionCondition::HeroMissionCondition()
    : mail_title(),
      mail_content(),
      rewards(),
      conditions() {}

// IDA_EA: 0x8168260
// IDA_NAME: _ZN20HeroMissionConditionD2Ev
// DEMANGLED: HeroMissionCondition::~HeroMissionCondition()
HeroMissionCondition::~HeroMissionCondition() {}

// IDA_EA: 0x8964F5C
// IDA_NAME: _ZN20HeroMissionConditionC2ERKS_
// DEMANGLED: HeroMissionCondition::HeroMissionCondition(
//     HeroMissionCondition const&)
HeroMissionCondition::HeroMissionCondition(
    const HeroMissionCondition& other)
    : mission_no(other.mission_no),
      mission_type(other.mission_type),
      repeat(other.repeat),
      reset_state(other.reset_state),
      mail_title(other.mail_title),
      mail_content(other.mail_content),
      rewards(other.rewards),
      conditions(other.conditions) {}

// IDA_EA: 0x8961234
// IDA_NAME: _ZN20HeroMissionConditionaSERKS_
// DEMANGLED: HeroMissionCondition::operator=(HeroMissionCondition const&)
//
// The old body deliberately appends both vectors through back-inserters.  It
// does not clear either destination vector before the copy.
HeroMissionCondition& HeroMissionCondition::operator=(
    const HeroMissionCondition& other) {
    mission_no = other.mission_no;
    mission_type = other.mission_type;
    repeat = other.repeat;
    reset_state = other.reset_state;
    mail_title = other.mail_title;
    mail_content = other.mail_content;
    std::copy(other.rewards.begin(), other.rewards.end(),
              std::back_inserter(rewards));
    std::copy(other.conditions.begin(), other.conditions.end(),
              std::back_inserter(conditions));
    return *this;
}

// IDA_EA: 0x8961376
// IDA_NAME: _ZN20HeroMissionCondition5clearEv
// DEMANGLED: HeroMissionCondition::clear(void)
void HeroMissionCondition::clear() {
    mission_no = static_cast<MissionNo::T>(0);
    mission_type = 0;
    repeat = false;
    reset_state = false;
    mail_title.clear();
    mail_content.clear();
    rewards.clear();
    conditions.clear();
}

// IDA_EA: 0x8169E18
// IDA_NAME: _ZN16HeroMissionValueD2Ev
// DEMANGLED: HeroMissionValue::~HeroMissionValue()
HeroMissionValue::~HeroMissionValue() {}

// IDA_EA: 0x8164B80
// IDA_NAME: _ZN16HeroMissionValue5clearEv
// DEMANGLED: HeroMissionValue::clear(void)
HeroMissionValue* HeroMissionValue::clear() {
    mission_no = static_cast<HeroMissionCondition::MissionNo::T>(0);
    progress = 0;
    state = 0;
    value = 0;
    reward_count = 0;
    return this;
}

// IDA_EA: 0x8164BB2
// IDA_NAME: _ZN16HeroMissionValue5resetEv
// DEMANGLED: HeroMissionValue::reset(void)
HeroMissionValue* HeroMissionValue::reset() {
    progress = 0;
    state = 0;
    return this;
}

// IDA_EA: 0x8164F12
// IDA_NAME: _ZN24BaseHeroMissionConditionC2EN20HeroMissionCondition9MissionNo1TE
// DEMANGLED: BaseHeroMissionCondition::BaseHeroMissionCondition(
//     HeroMissionCondition::MissionNo::T)
BaseHeroMissionCondition::BaseHeroMissionCondition(
    HeroMissionCondition::MissionNo::T mission_no)
    : mission_no_(mission_no),
      condition_() {}

// IDA_EA: 0x8164F3A
// IDA_NAME: _ZN24BaseHeroMissionConditionD2Ev
// DEMANGLED: BaseHeroMissionCondition::~BaseHeroMissionCondition()
// IDA_EA: 0x8164F6E is the deleting destructor.
BaseHeroMissionCondition::~BaseHeroMissionCondition() {}

// IDA_EA: 0x8164F8C
// IDA_NAME: _ZN24BaseHeroMissionCondition16setConditionDataER20HeroMissionCondition
// DEMANGLED: BaseHeroMissionCondition::setConditionData(
//     HeroMissionCondition &)
void BaseHeroMissionCondition::setConditionData(
    HeroMissionCondition& condition) {
    condition_ = condition;
}

// IDA_EA: 0x8164FAA
// IDA_NAME: _ZN24BaseHeroMissionCondition14getMissionCodeEv
// DEMANGLED: BaseHeroMissionCondition::getMissionCode(void)
HeroMissionCondition::MissionNo::T
BaseHeroMissionCondition::getMissionCode() {
    return mission_no_;
}

// IDA_EA: 0x8164FB6
// IDA_NAME: _ZN24BaseHeroMissionCondition14getMissionTypeEv
// DEMANGLED: BaseHeroMissionCondition::getMissionType(void)
int BaseHeroMissionCondition::getMissionType() {
    return condition_.mission_type;
}

// IDA_EA: 0x8164FC2
// IDA_NAME: _ZN24BaseHeroMissionCondition15isRepeatMissionEv
// DEMANGLED: BaseHeroMissionCondition::isRepeatMission(void)
bool BaseHeroMissionCondition::isRepeatMission() {
    return condition_.repeat;
}

// IDA_EA: 0x8164FCE
// IDA_NAME: _ZN24BaseHeroMissionCondition12isResetStateEv
// DEMANGLED: BaseHeroMissionCondition::isResetState(void)
bool BaseHeroMissionCondition::isResetState() {
    return condition_.reset_state;
}

// IDA_EA: 0x8164FDA
// IDA_NAME: _ZN24BaseHeroMissionCondition12getMailTitleEv
// DEMANGLED: BaseHeroMissionCondition::getMailTitle(void)
std::string BaseHeroMissionCondition::getMailTitle() {
    return condition_.mail_title;
}

// IDA_EA: 0x8165004
// IDA_NAME: _ZN24BaseHeroMissionCondition14getMailContentEv
// DEMANGLED: BaseHeroMissionCondition::getMailContent(void)
std::string BaseHeroMissionCondition::getMailContent() {
    return condition_.mail_content;
}

// IDA_EA: 0x816502E
// IDA_NAME: _ZN24BaseHeroMissionCondition13rewardProcessEP5CUser
// DEMANGLED: BaseHeroMissionCondition::rewardProcess(CUser *)
bool BaseHeroMissionCondition::rewardProcess(CUser* user) {
    for (std::vector<HeroMissionCondition::RewardData>::const_iterator
             reward = condition_.rewards.begin();
         reward != condition_.rewards.end(); ++reward) {
        CItem* const prototype = G_CDataManager()->find_item(
            reward->item_index);
        if (!prototype) {
            cMyTrace trace("bool BaseHeroMissionCondition::rewardProcess(CUser*)",
                           110, 5);
            trace("[HeroMission] Invalid item. (charac:%u, mission:%d, item:%u)",
                  static_cast<unsigned int>(user->get_charac_no(-1)),
                  static_cast<int>(mission_no_), reward->item_index);
            return false;
        }

        Inven_Item item;
        item.item_id_ = static_cast<int>(reward->item_index);
        prototype->vtable_->make_item(prototype, item);
        item.set_add_info(static_cast<int>(reward->count));

        const int inserted = user->getCurCharacInvenW()->
            insertItemIntoInventory(item, static_cast<eItemAddReason>(13),
                                    true, true);
        if (inserted < 0) {
            const char* const title = g_scriptStringManager_.findString(
                4, condition_.mail_title.c_str(), 0);
            const char* const contents = g_scriptStringManager_.findString(
                4, condition_.mail_content.c_str(), 0);
            WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                title, item, 0,
                static_cast<unsigned int>(user->get_charac_no(-1)), contents,
                static_cast<int>(std::strlen(contents)), 0,
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()), false,
                false);
        } else {
            user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0), inserted);
        }
    }
    return true;
}

// IDA_EA: 0x816858C
// IDA_NAME: _ZN4ARAD9SingletonI16HeroMissionEventEC2Ev
// DEMANGLED: ARAD::Singleton<HeroMissionEvent>::Singleton(void)
namespace ARAD {
template <>
Singleton<HeroMissionEvent>::Singleton() {}
}  // namespace ARAD

// IDA_EA: 0x8166522
// IDA_NAME: _ZN16HeroMissionEventC2Ev
// DEMANGLED: HeroMissionEvent::HeroMissionEvent(void)
HeroMissionEvent::HeroMissionEvent()
    : ARAD::Singleton<HeroMissionEvent>(),
      CEventBase(),
      conditions_(),
      values_by_charac_() {}

// IDA_EA: 0x81665A2
// IDA_NAME: _ZN16HeroMissionEventD2Ev
// DEMANGLED: HeroMissionEvent::~HeroMissionEvent()
// IDA_EA: 0x816662C is the deleting destructor.
HeroMissionEvent::~HeroMissionEvent() {}

// IDA_EA: 0x816664A
// IDA_NAME: _ZN16HeroMissionEvent10StartEventEv
// DEMANGLED: HeroMissionEvent::StartEvent(void)
void HeroMissionEvent::StartEvent() {
    if (loadScript()) {
        SetEventFlag(true);
        cMyTrace trace("virtual void HeroMissionEvent::StartEvent()", 515, 0);
        trace("[HeroMission] Start!");
        return;
    }

    cMyTrace trace("virtual void HeroMissionEvent::StartEvent()", 510, 5);
    trace("[HeroMission] Script load fail!!!");
}

// IDA_EA: 0x81666E6
// IDA_NAME: _ZN16HeroMissionEvent10StartEventE10Word_Param
// DEMANGLED: HeroMissionEvent::StartEvent(Word_Param)
void HeroMissionEvent::StartEvent(Word_Param param) {
    (void)param;
    static_cast<CEventBase*>(this)->StartEvent();
}

// IDA_EA: 0x8166700
// IDA_NAME: _ZN16HeroMissionEvent8EndEventEv
// DEMANGLED: HeroMissionEvent::EndEvent(void)
void HeroMissionEvent::EndEvent() {
    cMyTrace trace("virtual void HeroMissionEvent::EndEvent()", 525, 0);
    trace("[HeroMission] End!");
    SetEventFlag(false);
}

// IDA_EA: 0x811C9CE
// IDA_NAME: _ZN4ARAD9SingletonI16HeroMissionEventE3GetEv
// DEMANGLED: ARAD::Singleton<HeroMissionEvent>::Get(void)
//
// The original uses a function-local HeroMissionEvent with the normal
// __cxa_guard_acquire / __cxa_guard_release and __cxa_atexit sequence.  A C++
// function-local static retains that exact one-time construction/destruction
// contract without adding a second global owner.
namespace ARAD {
template <>
HeroMissionEvent* Singleton<HeroMissionEvent>::Get() {
    static HeroMissionEvent event;
    return &event;
}
}  // namespace ARAD
