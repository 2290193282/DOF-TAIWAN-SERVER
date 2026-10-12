#include "old_game/activity/schedule/hero_mission_runtime.h"

#include <string>
#include <vector>

#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"

// IDA_EA: 0x89613D7
// IDA_NAME: _ZL29ConvertString2EnumMissionTypeRSs
// DEMANGLED: ConvertString2EnumMissionType(std::string &)
static __attribute__((noinline, used))
int ConvertString2EnumMissionType(std::string& text) {
    if (text == "normal") {
        return 1;
    }
    if (text == "payment") {
        return 2;
    }
    if (text == "final") {
        return 3;
    }
    return 0;
}

enum HeroMissionTag {
    HERO_MISSION_TAG_UNKNOWN = -1,
    HERO_MISSION_TAG_BEGIN_EVENT = 1000,
    HERO_MISSION_TAG_END_EVENT = 1008,
    HERO_MISSION_TAG_MISSION = 1012,
    HERO_MISSION_TAG_TYPE = 1024,
    HERO_MISSION_TAG_REPEAT = 1036,
    HERO_MISSION_TAG_RESET = 1044,
    HERO_MISSION_TAG_ITEM = 1052,
    HERO_MISSION_TAG_CONDITION = 1073,
    HERO_MISSION_TAG_MAIL_TITLE = 1090,
    HERO_MISSION_TAG_MAIL_CONTENT = 1098,
    HERO_MISSION_TAG_END_MISSION = 1106,
};

// The original uses a function-local map<const char*, int> with a SpinLock
// during its one-time table fill.  These are the exact eleven source tokens
// and IDs observed in that map; a direct classifier retains the parser's
// token/return semantics without exporting a second helper ABI.
static HeroMissionTag GetHeroMissionTag(const std::string& tag) {
    if (tag == "[hero event]") {
        return HERO_MISSION_TAG_BEGIN_EVENT;
    }
    if (tag == "[/hero event]") {
        return HERO_MISSION_TAG_END_EVENT;
    }
    if (tag == "[mission]") {
        return HERO_MISSION_TAG_MISSION;
    }
    if (tag == "[type]") {
        return HERO_MISSION_TAG_TYPE;
    }
    if (tag == "[repeat]") {
        return HERO_MISSION_TAG_REPEAT;
    }
    if (tag == "[reset]") {
        return HERO_MISSION_TAG_RESET;
    }
    if (tag == "[item]") {
        return HERO_MISSION_TAG_ITEM;
    }
    if (tag == "[condition]") {
        return HERO_MISSION_TAG_CONDITION;
    }
    if (tag == "[mail title]") {
        return HERO_MISSION_TAG_MAIL_TITLE;
    }
    if (tag == "[mail content]") {
        return HERO_MISSION_TAG_MAIL_CONTENT;
    }
    if (tag == "[/mission]") {
        return HERO_MISSION_TAG_END_MISSION;
    }
    return HERO_MISSION_TAG_UNKNOWN;
}

// IDA_EA: 0x896171B
// IDA_NAME:
// _Z31importHeroMissionConditionsDataPSt6vectorI20HeroMissionConditionSaIS0_EEPKc
// DEMANGLED: importHeroMissionConditionsData(
//     std::vector<HeroMissionCondition> *,char const*)
int importHeroMissionConditionsData(
    std::vector<HeroMissionCondition>* conditions,
    const char* filename) {
    // `dir__18` at the original call site is the empty script directory.
    // Unlike many newer importers, this original body leaves the RDAR stream
    // lifecycle to its caller/global scanner and does not call unload here.
    if (!loadRDARScriptFile("", filename)) {
        return 992;
    }
    if (!conditions) {
        return 995;
    }

    std::string tag;
    HeroMissionCondition current;
    current.clear();
    bool saw_event = false;
    bool in_mission = false;
    unsigned int mission_fields = 0u;
    while (ScanType(tag, true) == 1) {
        switch (GetHeroMissionTag(tag)) {
            case HERO_MISSION_TAG_BEGIN_EVENT:
                if (saw_event || in_mission) {
                    return 1001;
                }
                saw_event = true;
                break;

            case HERO_MISSION_TAG_END_EVENT:
                return saw_event && !in_mission ? 0 : 1009;

            case HERO_MISSION_TAG_MISSION: {
                if (!saw_event || in_mission) {
                    return 1013;
                }
                int mission_no = 0;
                if (!ScanInt(&mission_no)) {
                    return 1015;
                }
                if (mission_no <= 0 || mission_no > 6) {
                    return 1020;
                }
                current.mission_no =
                    static_cast<HeroMissionCondition::MissionNo::T>(
                        mission_no);
                in_mission = true;
                mission_fields = 0x01u;
                break;
            }

            case HERO_MISSION_TAG_TYPE: {
                if (!in_mission || (mission_fields & 0x02u) != 0u) {
                    return 1025;
                }
                std::string mission_type;
                if (ScanStr(&mission_type) != 1) {
                    return 1027;
                }
                const int converted = ConvertString2EnumMissionType(
                    mission_type);
                if (!converted) {
                    return 1031;
                }
                current.mission_type = converted;
                mission_fields |= 0x02u;
                break;
            }

            case HERO_MISSION_TAG_REPEAT: {
                if (!in_mission || (mission_fields & 0x04u) != 0u) {
                    return 1037;
                }
                std::string repeat;
                if (ScanStr(&repeat) != 1) {
                    return 1039;
                }
                if (repeat != "true" && repeat != "false") {
                    return 1040;
                }
                current.repeat = (repeat == "true");
                mission_fields |= 0x04u;
                break;
            }

            case HERO_MISSION_TAG_RESET: {
                if (!in_mission || (mission_fields & 0x08u) != 0u) {
                    return 1045;
                }
                std::string reset;
                if (ScanStr(&reset) != 1) {
                    return 1047;
                }
                if (reset != "true" && reset != "false") {
                    return 1048;
                }
                current.reset_state = (reset == "true");
                mission_fields |= 0x08u;
                break;
            }

            case HERO_MISSION_TAG_ITEM: {
                if (!in_mission || (mission_fields & 0x10u) != 0u) {
                    return 1053;
                }
                int item_count = 0;
                if (!ScanInt(&item_count) || item_count <= 0) {
                    return 1057;
                }
                for (int i = 0; i < item_count; ++i) {
                    int item_index = 0;
                    int quantity = 0;
                    if (!ScanInt(&item_index)) {
                        return 1064;
                    }
                    if (!ScanInt(&quantity)) {
                        return 1067;
                    }
                    HeroMissionCondition::RewardData reward;
                    reward.item_index = static_cast<unsigned int>(item_index);
                    reward.count = static_cast<uint16_t>(quantity);
                    reward.reserved_06 = 0;
                    current.rewards.push_back(reward);
                }
                mission_fields |= 0x10u;
                break;
            }

            case HERO_MISSION_TAG_CONDITION: {
                if (!in_mission || (mission_fields & 0x20u) != 0u) {
                    return 1074;
                }
                int condition_count = 0;
                if (!ScanInt(&condition_count) || condition_count <= 0) {
                    return 1078;
                }
                for (int i = 0; i < condition_count; ++i) {
                    int condition_value = 0;
                    if (!ScanInt(&condition_value)) {
                        return 1084;
                    }
                    current.conditions.push_back(
                        static_cast<uint16_t>(condition_value));
                }
                mission_fields |= 0x20u;
                break;
            }

            case HERO_MISSION_TAG_MAIL_TITLE: {
                if (!in_mission || (mission_fields & 0x40u) != 0u) {
                    return 1091;
                }
                std::string title;
                if (ScanStr(&title) != 1) {
                    return 1093;
                }
                current.mail_title = title;
                mission_fields |= 0x40u;
                break;
            }

            case HERO_MISSION_TAG_MAIL_CONTENT: {
                if (!in_mission || (mission_fields & 0x80u) != 0u) {
                    return 1099;
                }
                std::string content;
                if (ScanStr(&content) != 1) {
                    return 1101;
                }
                current.mail_content = content;
                mission_fields |= 0x80u;
                break;
            }

            case HERO_MISSION_TAG_END_MISSION:
                if (!in_mission || mission_fields != 0xffu) {
                    return 1107;
                }
                for (std::vector<HeroMissionCondition>::const_iterator it =
                         conditions->begin();
                     it != conditions->end(); ++it) {
                    if (it->mission_no == current.mission_no) {
                        return 1110;
                    }
                }
                conditions->push_back(current);
                current.clear();
                in_mission = false;
                mission_fields = 0u;
                break;

            case HERO_MISSION_TAG_UNKNOWN:
            default:
                // Keep the reconstructed owner closed: accepting an unknown
                // token here would silently attach another Event schema to
                // HeroMissionCondition.
                return 1114;
        }
    }
    // Reaching EOF without the owning [/hero event] terminator is a
    // truncated script, not a successful empty tail.
    return 1115;
}

// IDA_EA: 0x8167AC8
// IDA_NAME: _ZN16HeroMissionEvent10loadScriptEv
// DEMANGLED: HeroMissionEvent::loadScript(void)
bool HeroMissionEvent::loadScript() {
    ClearDungeonMission* clear_dungeon = new ClearDungeonMission;
    if (!clear_dungeon) {
        throw 837;
    }
    conditions_[clear_dungeon->getMissionCode()] = clear_dungeon;

    UseFatigueMission* use_fatigue = new UseFatigueMission;
    if (!use_fatigue) {
        throw 841;
    }
    conditions_[use_fatigue->getMissionCode()] = use_fatigue;

    UseCubeMission* use_cube = new UseCubeMission;
    if (!use_cube) {
        throw 845;
    }
    conditions_[use_cube->getMissionCode()] = use_cube;

    LevelUpMission* level_up = new LevelUpMission;
    if (!level_up) {
        throw 849;
    }
    conditions_[level_up->getMissionCode()] = level_up;

    AttendanceMission* attendance = new AttendanceMission;
    if (!attendance) {
        throw 853;
    }
    conditions_[attendance->getMissionCode()] = attendance;

    TotalMission* total = new TotalMission;
    if (!total) {
        throw 857;
    }
    conditions_[total->getMissionCode()] = total;

    std::vector<HeroMissionCondition> parsed;
    const int error = importHeroMissionConditionsData(
        &parsed, "Event/HeroMissionEvent.evt");
    if (error) {
        LogManager::logFormat(1, "localtaiwan/Event/EventHeroMission.cpp",
                              "bool HeroMissionEvent::loadScript()", 871,
                              "[HeroMission] Load script fail. (line:%d)",
                              error);
        return false;
    }

    for (std::vector<HeroMissionCondition>::iterator condition =
             parsed.begin();
         condition != parsed.end(); ++condition) {
        if (condition->mission_no > 0 &&
            condition->mission_no <= HeroMissionCondition::MissionNo::TOTAL) {
            conditions_[condition->mission_no]->setConditionData(*condition);
        }
    }
    parsed.clear();
    return true;
}
