#include "old_game/activity/schedule/hero_mission_runtime.h"

#include <cstring>
#include <ctime>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8165342
// IDA_NAME: _ZN19ClearDungeonMissionC2Ev
// DEMANGLED: ClearDungeonMission::ClearDungeonMission(void)
ClearDungeonMission::ClearDungeonMission()
    : BaseHeroMissionCondition(HeroMissionCondition::MissionNo::CLEAR_DUNGEON) {}

// IDA_EA: 0x8165366
// IDA_NAME: _ZN19ClearDungeonMissionD2Ev
// DEMANGLED: ClearDungeonMission::~ClearDungeonMission()
ClearDungeonMission::~ClearDungeonMission() {}

// IDA_EA: 0x81653B4
// IDA_NAME: _ZN19ClearDungeonMission14processMissionEP5CUserR16HeroMissionValuej
// DEMANGLED: ClearDungeonMission::processMission(
//     CUser *,HeroMissionValue &,unsigned int)
void ClearDungeonMission::processMission(CUser* user,
                                         HeroMissionValue& value,
                                         unsigned int param) {
    (void)user;
    const uint16_t target = condition_.conditions[0];
    if (target <= value.progress) {
        return;
    }
    if (getMissionType() == 2 && value.value != 7) {
        return;
    }
    if (target > param + value.progress) {
        value.progress += param;
    } else {
        value.progress = target;
    }
}

// IDA_EA: 0x8165484
// IDA_NAME: _ZN19ClearDungeonMission13rewardMissionEP5CUserR16HeroMissionValue
// DEMANGLED: ClearDungeonMission::rewardMission(CUser *,HeroMissionValue &)
bool ClearDungeonMission::rewardMission(CUser* user, HeroMissionValue& value) {
    if (!isRepeatMission() && value.reward_count) {
        LogManager::logFormat(
            1, "localtaiwan/Event/EventHeroMission.cpp",
            "virtual bool ClearDungeonMission::rewardMission(CUser*, "
            "HeroMissionValue&)",
            172,
            "[HeroMission] Already reward. (charac:%u,mission:%d,count:%d)",
            static_cast<unsigned int>(user->get_charac_no(-1)),
            static_cast<int>(value.mission_no),
            static_cast<int>(value.reward_count));
        return false;
    }
    if (getMissionType() == 2 && value.value != 7) {
        LogManager::logFormat(
            1, "localtaiwan/Event/EventHeroMission.cpp",
            "virtual bool ClearDungeonMission::rewardMission(CUser*, "
            "HeroMissionValue&)",
            180,
            "[HeroMission] Invalid reward condition. "
            "(charac:%u,mission:%d,value:%d)",
            static_cast<unsigned int>(user->get_charac_no(-1)),
            static_cast<int>(value.mission_no), static_cast<int>(value.value));
        return false;
    }
    if (value.progress < condition_.conditions[0]) {
        LogManager::logFormat(
            1, "localtaiwan/Event/EventHeroMission.cpp",
            "virtual bool ClearDungeonMission::rewardMission(CUser*, "
            "HeroMissionValue&)",
            187,
            "[HeroMission] Invalid reward condition. "
            "(charac:%u,mission:%d,count:%d)",
            static_cast<unsigned int>(user->get_charac_no(-1)),
            static_cast<int>(value.mission_no),
            static_cast<int>(value.reward_count));
        return false;
    }

    const bool result = rewardProcess(user);
    ++value.reward_count;
    value.state = 1;
    if (isRepeatMission()) {
        value.reset();
    }
    return result;
}

// IDA_EA: 0x8165694
// IDA_NAME: _ZN17UseFatigueMissionC2Ev
// DEMANGLED: UseFatigueMission::UseFatigueMission(void)
UseFatigueMission::UseFatigueMission()
    : BaseHeroMissionCondition(HeroMissionCondition::MissionNo::USE_FATIGUE) {}

// IDA_EA: 0x81656B8
// IDA_NAME: _ZN17UseFatigueMissionD2Ev
// DEMANGLED: UseFatigueMission::~UseFatigueMission()
UseFatigueMission::~UseFatigueMission() {}

// IDA_EA: 0x8165706
// IDA_NAME: _ZN17UseFatigueMission14processMissionEP5CUserR16HeroMissionValuej
// DEMANGLED: UseFatigueMission::processMission(
//     CUser *,HeroMissionValue &,unsigned int)
void UseFatigueMission::processMission(CUser* user,
                                       HeroMissionValue& value,
                                       unsigned int param) {
    (void)user;
    const uint16_t target = condition_.conditions[0];
    if (target <= value.progress) {
        return;
    }
    if (target > param + value.progress) {
        value.progress += param;
    } else {
        value.progress = target;
    }
}

// IDA_EA: 0x81657A2
// IDA_NAME: _ZN17UseFatigueMission13rewardMissionEP5CUserR16HeroMissionValue
// DEMANGLED: UseFatigueMission::rewardMission(CUser *,HeroMissionValue &)
bool UseFatigueMission::rewardMission(CUser* user, HeroMissionValue& value) {
    if (!isRepeatMission() && value.reward_count) {
        LogManager::logFormat(
            1, "localtaiwan/Event/EventHeroMission.cpp",
            "virtual bool UseFatigueMission::rewardMission(CUser*, "
            "HeroMissionValue&)",
            229,
            "[HeroMission] Already reward. (charac:%u,mission:%d,count:%d)",
            static_cast<unsigned int>(user->get_charac_no(-1)),
            static_cast<int>(value.mission_no),
            static_cast<int>(value.reward_count));
        return false;
    }
    if (value.progress < condition_.conditions[0]) {
        LogManager::logFormat(
            1, "localtaiwan/Event/EventHeroMission.cpp",
            "virtual bool UseFatigueMission::rewardMission(CUser*, "
            "HeroMissionValue&)",
            236,
            "[HeroMission] Invalid reward condition. "
            "(charac:%u,mission:%d,count:%d)",
            static_cast<unsigned int>(user->get_charac_no(-1)),
            static_cast<int>(value.mission_no),
            static_cast<int>(value.reward_count));
        return false;
    }

    const bool result = rewardProcess(user);
    ++value.reward_count;
    value.state = 1;
    if (isRepeatMission()) {
        value.reset();
    }
    return result;
}

// IDA_EA: 0x8165920
// IDA_NAME: _ZN14UseCubeMissionC2Ev
// DEMANGLED: UseCubeMission::UseCubeMission(void)
UseCubeMission::UseCubeMission()
    : BaseHeroMissionCondition(HeroMissionCondition::MissionNo::USE_CUBE) {}

// IDA_EA: 0x8165944
// IDA_NAME: _ZN14UseCubeMissionD2Ev
// DEMANGLED: UseCubeMission::~UseCubeMission()
UseCubeMission::~UseCubeMission() {}

// IDA_EA: 0x8165992
// IDA_NAME: _ZN14UseCubeMission14processMissionEP5CUserR16HeroMissionValuej
// DEMANGLED: UseCubeMission::processMission(
//     CUser *,HeroMissionValue &,unsigned int)
void UseCubeMission::processMission(CUser* user,
                                    HeroMissionValue& value,
                                    unsigned int param) {
    (void)user;
    const uint16_t target = condition_.conditions[0];
    if (target <= value.progress) {
        return;
    }
    if (target > param + value.progress) {
        value.progress += param;
    } else {
        value.progress = target;
    }
}

// IDA_EA: 0x8165A2E
// IDA_NAME: _ZN14UseCubeMission13rewardMissionEP5CUserR16HeroMissionValue
// DEMANGLED: UseCubeMission::rewardMission(CUser *,HeroMissionValue &)
bool UseCubeMission::rewardMission(CUser* user, HeroMissionValue& value) {
    if (!isRepeatMission() && value.reward_count) {
        LogManager::logFormat(
            1, "localtaiwan/Event/EventHeroMission.cpp",
            "virtual bool UseCubeMission::rewardMission(CUser*, "
            "HeroMissionValue&)",
            279,
            "[HeroMission] Already reward. (charac:%u,mission:%d,count:%d)",
            static_cast<unsigned int>(user->get_charac_no(-1)),
            static_cast<int>(value.mission_no),
            static_cast<int>(value.reward_count));
        return false;
    }
    if (value.progress < condition_.conditions[0]) {
        LogManager::logFormat(
            1, "localtaiwan/Event/EventHeroMission.cpp",
            "virtual bool UseCubeMission::rewardMission(CUser*, "
            "HeroMissionValue&)",
            286,
            "[HeroMission] Invalid reward condition. "
            "(charac:%u,mission:%d,count:%d)",
            static_cast<unsigned int>(user->get_charac_no(-1)),
            static_cast<int>(value.mission_no),
            static_cast<int>(value.reward_count));
        return false;
    }

    const bool result = rewardProcess(user);
    ++value.reward_count;
    value.state = 1;
    if (isRepeatMission()) {
        value.reset();
    }
    return result;
}

// IDA_EA: 0x8165BAC
// IDA_NAME: _ZN14LevelUpMissionC2Ev
// DEMANGLED: LevelUpMission::LevelUpMission(void)
LevelUpMission::LevelUpMission()
    : BaseHeroMissionCondition(HeroMissionCondition::MissionNo::LEVEL_UP) {}

// IDA_EA: 0x8165BD0
// IDA_NAME: _ZN14LevelUpMissionD2Ev
// DEMANGLED: LevelUpMission::~LevelUpMission()
LevelUpMission::~LevelUpMission() {}

// IDA_EA: 0x8165C1E
// IDA_NAME: _ZN14LevelUpMission14processMissionEP5CUserR16HeroMissionValuej
// DEMANGLED: LevelUpMission::processMission(
//     CUser *,HeroMissionValue &,unsigned int)
void LevelUpMission::processMission(CUser* user,
                                    HeroMissionValue& value,
                                    unsigned int param) {
    (void)param;
    const int level = user->get_charac_level();
    if (condition_.conditions[0] > level) {
        return;
    }
    if (!isRepeatMission() && value.reward_count) {
        return;
    }
    if (value.value >= level) {
        LogManager::logFormat(
            1, "localtaiwan/Event/EventHeroMission.cpp",
            "virtual void LevelUpMission::processMission(CUser*, "
            "HeroMissionValue&, unsigned int)",
            325,
            "[HeroMission] Invalid reward condition. "
            "(charac:%u,mission:%d,value:%d)",
            static_cast<unsigned int>(user->get_charac_no(-1)),
            static_cast<int>(value.mission_no), static_cast<int>(value.value));
        return;
    }

    for (std::vector<HeroMissionCondition::RewardData>::const_iterator
             reward = condition_.rewards.begin();
         reward != condition_.rewards.end(); ++reward) {
        CItem* const prototype = G_CDataManager()->find_item(
            reward->item_index);
        if (!prototype) {
            cMyTrace trace("virtual void LevelUpMission::processMission("
                           "CUser*, HeroMissionValue&, unsigned int)",
                           336, 5);
            trace("[HeroMission] Invalid item. (charac:%u, mission:%d, "
                  "item:%u)",
                  static_cast<unsigned int>(user->get_charac_no(-1)),
                  static_cast<int>(value.mission_no), reward->item_index);
            return;
        }

        Inven_Item item;
        item.item_id_ = static_cast<int>(reward->item_index);
        prototype->vtable_->make_item(prototype, item);
        item.set_add_info(static_cast<int>(reward->count));
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
    }

    ++value.reward_count;
    value.state = 1;
    value.value = static_cast<uint16_t>(level);
    if (isRepeatMission()) {
        value.reset();
    }
}

// IDA_EA: 0x8165F86
// IDA_NAME: _ZN14LevelUpMission13rewardMissionEP5CUserR16HeroMissionValue
// DEMANGLED: LevelUpMission::rewardMission(CUser *,HeroMissionValue &)
bool LevelUpMission::rewardMission(CUser* user, HeroMissionValue& value) {
    (void)user;
    (void)value;
    return true;
}

// IDA_EA: 0x8165F90
// IDA_NAME: _ZN17AttendanceMissionC2Ev
// DEMANGLED: AttendanceMission::AttendanceMission(void)
AttendanceMission::AttendanceMission()
    : BaseHeroMissionCondition(HeroMissionCondition::MissionNo::ATTENDANCE) {}

// IDA_EA: 0x8165FB4
// IDA_NAME: _ZN17AttendanceMissionD2Ev
// DEMANGLED: AttendanceMission::~AttendanceMission()
AttendanceMission::~AttendanceMission() {}

// IDA_EA: 0x8166002
// IDA_NAME: _ZN17AttendanceMission14processMissionEP5CUserR16HeroMissionValuej
// DEMANGLED: AttendanceMission::processMission(
//     CUser *,HeroMissionValue &,unsigned int)
void AttendanceMission::processMission(CUser* user,
                                       HeroMissionValue& value,
                                       unsigned int param) {
    (void)user;
    (void)param;
    const uint16_t target = condition_.conditions[0];
    if (target <= value.progress) {
        return;
    }

    const time_t current = static_cast<time_t>(
        GlobalData::s_systemTime_.getCurSec());
    std::tm current_tm;
    std::tm recorded_tm;
    localtime_r(&current, &current_tm);
    localtime_r(&current, &recorded_tm);

    const int reset_hour = G_CEnvironment()->get_fatigue_time();
    current_tm.tm_hour = reset_hour;
    current_tm.tm_min = 0;
    current_tm.tm_sec = 0;
    const time_t current_reset = mktime(&current_tm);

    recorded_tm.tm_mon = static_cast<int>(condition_.conditions[1]) - 1;
    recorded_tm.tm_mday = static_cast<int>(condition_.conditions[2]) +
                          static_cast<int>(value.value);
    recorded_tm.tm_hour = reset_hour;
    recorded_tm.tm_min = 0;
    recorded_tm.tm_sec = 0;
    const time_t recorded_reset = mktime(&recorded_tm);

    if (current_reset - recorded_reset > 86400) {
        value.reset();
    }
    if (target > value.progress + 1) {
        ++value.progress;
    } else {
        value.progress = target;
    }
    value.value = static_cast<uint16_t>(
        value.value + (current_reset - recorded_reset) / 86400);
}

// IDA_EA: 0x81661D6
// IDA_NAME: _ZN17AttendanceMission13rewardMissionEP5CUserR16HeroMissionValue
// DEMANGLED: AttendanceMission::rewardMission(CUser *,HeroMissionValue &)
bool AttendanceMission::rewardMission(CUser* user, HeroMissionValue& value) {
    if (!isRepeatMission() && value.reward_count) {
        return false;
    }
    if (value.progress < condition_.conditions[0]) {
        cMyTrace trace("virtual bool AttendanceMission::rewardMission("
                       "CUser*, HeroMissionValue&)",
                       420, 5);
        trace("[HeroMission] Reward condition error. (charac:%u,mission:%d,"
              "count:%d)",
              static_cast<unsigned int>(user->get_charac_no(-1)),
              static_cast<int>(value.mission_no),
              static_cast<int>(value.reward_count));
        return false;
    }

    const bool result = rewardProcess(user);
    ++value.reward_count;
    value.state = 1;
    if (isRepeatMission()) {
        value.reset();
    }
    return result;
}

// IDA_EA: 0x8166306
// IDA_NAME: _ZN12TotalMissionC2Ev
// DEMANGLED: TotalMission::TotalMission(void)
TotalMission::TotalMission()
    : BaseHeroMissionCondition(HeroMissionCondition::MissionNo::TOTAL) {}

// IDA_EA: 0x816632A
// IDA_NAME: _ZN12TotalMissionD2Ev
// DEMANGLED: TotalMission::~TotalMission()
TotalMission::~TotalMission() {}

// IDA_EA: 0x8166378
// IDA_NAME: _ZN12TotalMission14processMissionEP5CUserR16HeroMissionValuej
// DEMANGLED: TotalMission::processMission(
//     CUser *,HeroMissionValue &,unsigned int)
void TotalMission::processMission(CUser* user,
                                  HeroMissionValue& value,
                                  unsigned int param) {
    (void)user;
    if (condition_.conditions.size() <= value.progress) {
        return;
    }
    if (condition_.conditions.size() > param + value.progress) {
        value.progress += param;
    } else {
        value.progress = static_cast<uint16_t>(condition_.conditions.size());
    }
}

// IDA_EA: 0x81663F4
// IDA_NAME: _ZN12TotalMission13rewardMissionEP5CUserR16HeroMissionValue
// DEMANGLED: TotalMission::rewardMission(CUser *,HeroMissionValue &)
bool TotalMission::rewardMission(CUser* user, HeroMissionValue& value) {
    (void)user;
    (void)value;
    return true;
}

// IDA_EA: 0x81663FE
// IDA_NAME:
// _ZN12TotalMission13rewardMissionEP5CUserR16HeroMissionValueRSt6vectorIS2_SaIS2_EE
// DEMANGLED: TotalMission::rewardMission(
//     CUser *,HeroMissionValue &,std::vector<HeroMissionValue> &)
bool TotalMission::rewardMission(CUser* user,
                                 HeroMissionValue& value,
                                 std::vector<HeroMissionValue>& values) {
    if (!isRepeatMission() && value.reward_count) {
        return false;
    }

    bool all_rewarded = true;
    for (std::vector<HeroMissionValue>::const_iterator current =
             values.begin();
         current != values.end(); ++current) {
        if (current->mission_no != HeroMissionCondition::MissionNo::TOTAL &&
            !current->reward_count) {
            all_rewarded = false;
            break;
        }
    }
    if (!all_rewarded) {
        return false;
    }

    const bool result = rewardProcess(user);
    ++value.reward_count;
    value.state = 1;
    if (isRepeatMission()) {
        value.reset();
    }
    return result;
}
