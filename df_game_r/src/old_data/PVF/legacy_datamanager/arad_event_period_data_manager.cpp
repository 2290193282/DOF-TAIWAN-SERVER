#include "old_data/PVF/arad/datamanager/arad_event_period_data_manager.h"

#include "common/manager/global_data.h"
#include "common/manager/gm_accounts_abi.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/concurrent/thread/mutex.h"
#include "common/manager/system_time.h"

#include <algorithm>
#include <list>

namespace ARAD {

namespace {

int eventCodeValue(ENUM_REPEAT_EVENT_CODE event_code) {
    return static_cast<int>(event_code);
}

}  // namespace

// IDA_EA: 0x81953FA
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManagerC2Ev
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::Arad_EventPeriodDataManager(void)
Arad_EventPeriodDataManager::Arad_EventPeriodDataManager()
    : Arad_InterfaceGameScript("Arad_EventPeriodDataManager"),
      create_period_gm_days_(0),
      event_periods_(),
      reward_periods_(),
      reward_mutex_(),
      reward_items_() {}

// IDA_EA: 0x8195536
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManagerD2Ev
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::~Arad_EventPeriodDataManager()
Arad_EventPeriodDataManager::~Arad_EventPeriodDataManager() {
    clearScriptData();
}

// IDA_EA: 0x8195632
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManagerD0Ev
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::~Arad_EventPeriodDataManager()
// The virtual destructor above supplies the deleting-dtor ABI entry.

// IDA_EA: 0x8195650
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager14loadScriptDataEv
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::loadScriptData(void)
bool Arad_EventPeriodDataManager::loadScriptData() {
    return SCRIPT::importAradEventScript(event_periods_, reward_periods_) != 0;
}

// IDA_EA: 0x8195670
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager15clearScriptDataEv
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::clearScriptData(void)
void Arad_EventPeriodDataManager::clearScriptData() {
    // The original clears only the event-period vector here; reward-period
    // records remain owned by their vector until object destruction.
    event_periods_.clear();
}

// IDA_EA: 0x8195686
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager9isRunningE22ENUM_REPEAT_EVENT_CODE
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::isRunning(ENUM_REPEAT_EVENT_CODE)
int Arad_EventPeriodDataManager::isRunning(ENUM_REPEAT_EVENT_CODE event_code) {
    return isApplied(event_code, static_cast<uint32_t>(
                                    GlobalData::system_time()->getCurTickCount()));
}

// IDA_EA: 0x81956B0
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager9isAppliedE22ENUM_REPEAT_EVENT_CODEj
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::isApplied(ENUM_REPEAT_EVENT_CODE,unsigned int)
bool Arad_EventPeriodDataManager::isApplied(ENUM_REPEAT_EVENT_CODE event_code,
                                            uint32_t current_sec) {
    const int event_code_value = eventCodeValue(event_code);
    std::vector<SCRIPT::SEventPeriodData>::const_iterator it =
        std::find_if(event_periods_.begin(), event_periods_.end(),
                     [event_code_value](const SCRIPT::SEventPeriodData& data) {
                         return data.event_code == event_code_value;
                     });
    return it != event_periods_.end() && it->start_time <= current_sec &&
           it->end_time > current_sec;
}

// IDA_EA: 0x8195794
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager15checkCreateTimeE22ENUM_REPEAT_EVENT_CODEjj
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::checkCreateTime(ENUM_REPEAT_EVENT_CODE,unsigned int,unsigned int)
bool Arad_EventPeriodDataManager::checkCreateTime(
    ENUM_REPEAT_EVENT_CODE event_code,
    uint32_t character_create_time,
    uint32_t account_id) {
    const int event_code_value = eventCodeValue(event_code);
    std::vector<SCRIPT::SEventPeriodData>::const_iterator it =
        std::find_if(event_periods_.begin(), event_periods_.end(),
                     [event_code_value](const SCRIPT::SEventPeriodData& data) {
                         return data.event_code == event_code_value;
                     });
    if (it == event_periods_.end()) {
        return false;
    }

    const uint32_t first_allowed_create_time =
        GlobalData::s_pGMAccounts_->isGM(account_id)
            ? 1342558800u - 86400u *
                                  static_cast<uint32_t>(create_period_gm_days_)
            : 1342558800u;
    return first_allowed_create_time <= character_create_time &&
           character_create_time < it->end_time;
}

// IDA_EA: 0x819589E
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager18setCreatePeriod_GMEi
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::setCreatePeriod_GM(int)
Arad_EventPeriodDataManager*
Arad_EventPeriodDataManager::setCreatePeriod_GM(int days) {
    if (days >= 0) {
        create_period_gm_days_ = days;
    }
    return this;
}

// IDA_EA: 0x81958B6
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager10isRewardedENS_19EventRewardItemAradE
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::isRewarded(ARAD::EventRewardItemArad)
bool Arad_EventPeriodDataManager::isRewarded(EventRewardItemArad item) {
    MutexGuard guard(&reward_mutex_);
    return std::find(reward_items_.begin(), reward_items_.end(), item) !=
           reward_items_.end();
}

// IDA_EA: 0x8195994
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager17addUserRewardDataEP5CUserRN4arad22SigLoadRewardEventItemE
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::addUserRewardData(CUser *,arad::SigLoadRewardEventItem &)
bool Arad_EventPeriodDataManager::addUserRewardData(
    CUser* user,
    arad::SigLoadRewardEventItem& reward_items) {
    for (int index = 0; index < reward_items.reward_count; ++index) {
        EventRewardItemArad item(reward_items.event_codes[index],
                                 static_cast<uint32_t>(user->get_acc_id()),
                                 reward_items.charac_nos[index]);
        if (isRewarded(item)) {
            LogManager::logFormat(
                1,
                "localjapan/Arad_EventPeriodDataManager.cpp",
                "void ARAD::Arad_EventPeriodDataManager::addUserRewardData(CUser*, arad::SigLoadRewardEventItem&)",
                0xE3,
                "[FnReward] already reward item. (mid:%u, charac:%u, event:%d)",
                static_cast<unsigned int>(user->get_acc_id()),
                static_cast<unsigned int>(user->get_charac_no(-1)),
                reward_items.event_codes[index]);
        } else {
            addUserRewardData(user, item);
        }
    }
    // The old loop returns its terminating comparison (false), not a
    // dispatcher-success surrogate.
    return false;
}

// IDA_EA: 0x8195AA4
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager17addUserRewardDataEP5CUserRNS_19EventRewardItemAradE
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::addUserRewardData(CUser *,ARAD::EventRewardItemArad &)
int Arad_EventPeriodDataManager::addUserRewardData(CUser*,
                                                   EventRewardItemArad& item) {
    MutexGuard guard(&reward_mutex_);
    reward_items_.push_back(item);
    return 0;
}

// IDA_EA: 0x8195B06
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager17delUserRewardDataEP5CUser
// DEMANGLED: ARAD::Arad_EventPeriodDataManager::delUserRewardData(CUser *)
int Arad_EventPeriodDataManager::delUserRewardData(CUser* user) {
    MutexGuard guard(&reward_mutex_);
    const unsigned int acc_id = static_cast<unsigned int>(
        user->get_acc_id());

    for (std::list<EventRewardItemArad>::iterator it =
             reward_items_.begin();
         it != reward_items_.end();) {
        if (it->account_id == acc_id) {
            it = reward_items_.erase(it);
        } else {
            ++it;
        }
    }
    return 0;
}

// IDA_EA: 0x819ABC8
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager11eventRewardINS_13FnRewardLetheEEEb22ENUM_REPEAT_EVENT_CODEjP5CUser
// DEMANGLED: bool ARAD::Arad_EventPeriodDataManager::eventReward<ARAD::FnRewardLethe>(ENUM_REPEAT_EVENT_CODE,unsigned int,CUser *)
template <>
bool Arad_EventPeriodDataManager::eventReward<FnRewardLethe>(
    ENUM_REPEAT_EVENT_CODE event_code,
    uint32_t current_sec,
    CUser* user) {
    if (!user || !user->get_charac_no(-1)) {
        return false;
    }

    FnRewardLethe reward_functor(user);
    EventRewardItemArad reward_data =
        reward_functor.make_data(static_cast<int>(event_code));
    if (isRewarded(reward_data)) {
        return false;
    }

    std::vector<SCRIPT::SRewardPeriodData>::const_iterator reward_period =
        std::find_if(reward_periods_.begin(), reward_periods_.end(),
                     SearchEventPeriod(event_code));
    if (reward_period == reward_periods_.end() ||
        reward_period->start_time > current_sec ||
        reward_period->end_time <= current_sec) {
        return false;
    }

    if (!reward_functor(static_cast<int>(event_code),
                        reward_period->item_index,
                        reward_period->item_count,
                        reward_period->end_time)) {
        return false;
    }
    addUserRewardData(user, reward_data);
    return true;
}

// IDA_EA: 0x819ADB6
// IDA_NAME: _ZN4ARAD27Arad_EventPeriodDataManager11eventRewardINS_14FnRewardMomojiEEEb22ENUM_REPEAT_EVENT_CODEjP5CUser
// DEMANGLED: bool ARAD::Arad_EventPeriodDataManager::eventReward<ARAD::FnRewardMomoji>(ENUM_REPEAT_EVENT_CODE,unsigned int,CUser *)
template <>
bool Arad_EventPeriodDataManager::eventReward<FnRewardMomoji>(
    ENUM_REPEAT_EVENT_CODE event_code,
    uint32_t current_sec,
    CUser* user) {
    if (!user || !user->get_charac_no(-1)) {
        return false;
    }

    FnRewardMomoji reward_functor(user);
    EventRewardItemArad reward_data =
        reward_functor.make_data(static_cast<int>(event_code));
    if (isRewarded(reward_data)) {
        return false;
    }

    std::vector<SCRIPT::SRewardPeriodData>::const_iterator reward_period =
        std::find_if(reward_periods_.begin(), reward_periods_.end(),
                     SearchEventPeriod(event_code));
    if (reward_period == reward_periods_.end() ||
        reward_period->start_time > current_sec ||
        reward_period->end_time <= current_sec) {
        return false;
    }

    if (!reward_functor(static_cast<int>(event_code),
                        reward_period->item_index,
                        reward_period->item_count,
                        reward_period->end_time)) {
        return false;
    }
    addUserRewardData(user, reward_data);
    return true;
}

}  // namespace ARAD
