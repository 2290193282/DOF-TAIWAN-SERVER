#include "old_data/PVF/etc/action_point_etc_parameter.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace APSystem {

// IDA_EA: 0x848F5B4
// IDA_NAME: _ZN8APSystem12_ActionPointC2Ev
// DEMANGLED: APSystem::_ActionPoint::_ActionPoint(void)
_ActionPoint::_ActionPoint() {
    std::memset(this, 0, 0x0Bu);
}

// IDA_EA: 0x889566E
// IDA_NAME: _ZN8APSystem14CActionPointExC2Ev
// DEMANGLED: APSystem::CActionPointEx::CActionPointEx(void)
CActionPointEx::CActionPointEx()
    : _ActionPoint(),
      rule_value(0),
      group_option(0),
      start_value(0),
      end_value(0),
      values(),
      weekday(7),
      user_message(),
      system_message() {}

// IDA_EA: 0x83721CE
// IDA_NAME: _ZN8APSystem14CActionPointExD2Ev
// DEMANGLED: APSystem::CActionPointEx::~CActionPointEx()
CActionPointEx::~CActionPointEx() {}

// IDA_EA: 0x8895EBC
// IDA_NAME: _ZN8APSystem14CActionPointExC2ERKS0_
// DEMANGLED: APSystem::CActionPointEx::CActionPointEx(APSystem::CActionPointEx const&)
CActionPointEx::CActionPointEx(const CActionPointEx& other)
    : _ActionPoint(other),
      rule_value(other.rule_value),
      group_option(other.group_option),
      start_value(other.start_value),
      end_value(other.end_value),
      values(other.values),
      weekday(other.weekday),
      user_message(other.user_message),
      system_message(other.system_message),
      display_flag(other.display_flag) {}

// IDA_EA: 0x889603A
// IDA_NAME: _ZN8APSystem14CActionPointExaSERKS0_
// DEMANGLED: APSystem::CActionPointEx::operator=(APSystem::CActionPointEx const&)
CActionPointEx& CActionPointEx::operator=(const CActionPointEx& other) {
    if (this != &other) {
        action_index = other.action_index;
        action_group_index = other.action_group_index;
        std::memcpy(clear_state, other.clear_state, sizeof(clear_state));
        rule_value = other.rule_value;
        group_option = other.group_option;
        start_value = other.start_value;
        end_value = other.end_value;
        values = other.values;
        weekday = other.weekday;
        user_message = other.user_message;
        system_message = other.system_message;
        display_flag = other.display_flag;
    }
    return *this;
}

// IDA_EA: 0x8895712
// IDA_NAME: _ZN8APSystem16_MedalRewardItemC2Ev
// DEMANGLED: APSystem::_MedalRewardItem::_MedalRewardItem(void)
_MedalRewardItem::_MedalRewardItem() {
    std::memset(this, 0, 0x0Au);
}

// IDA_EA: 0x8897B24
// IDA_NAME: _ZN8APSystem16_MedalRewardItemD2Ev
// DEMANGLED: APSystem::_MedalRewardItem::~_MedalRewardItem()
_MedalRewardItem::~_MedalRewardItem() {}

// IDA_EA: 0x837227E
// IDA_NAME: _ZN8APSystem24CActionPointEtcParameterC2Ev
// DEMANGLED: APSystem::CActionPointEtcParameter::CActionPointEtcParameter(void)
CActionPointEtcParameter::CActionPointEtcParameter()
    : action_groups_(),
      action_index_(),
      user_messages_(),
      system_messages_(),
      medal_rewards_(),
      today_rewards_() {}

// IDA_EA: 0x837235C
// IDA_NAME: _ZN8APSystem24CActionPointEtcParameterD2Ev
// DEMANGLED: APSystem::CActionPointEtcParameter::~CActionPointEtcParameter()
CActionPointEtcParameter::~CActionPointEtcParameter() {}

// IDA_EA: 0x88948FC
// IDA_NAME: _ZN8APSystem24CActionPointEtcParameter9ClearDataEv
// DEMANGLED: APSystem::CActionPointEtcParameter::ClearData(void)
void CActionPointEtcParameter::ClearData() {
    action_groups_.clear();
    action_index_.clear();
    user_messages_.clear();
    system_messages_.clear();
    medal_rewards_.clear();
    today_rewards_.clear();
}

// IDA_EA: 0x8894956
// IDA_NAME: _ZN8APSystem24CActionPointEtcParameter23IsValidActionGroupIndexENS_17_ActionGroupIndexE
// DEMANGLED: APSystem::CActionPointEtcParameter::IsValidActionGroupIndex(APSystem::_ActionGroupIndex)
bool CActionPointEtcParameter::IsValidActionGroupIndex(
    _ActionGroupIndex index) {
    return index > 0 && index <= 45;
}

// IDA_EA: 0x8894976
// IDA_NAME: _ZN8APSystem24CActionPointEtcParameter22IsValidTodayRewardItemENS_22_SIG_LOAD_ACTION_POINT16_TodayRewardItemE
// DEMANGLED: APSystem::CActionPointEtcParameter::IsValidTodayRewardItem(APSystem::_SIG_LOAD_ACTION_POINT::_TodayRewardItem)
bool CActionPointEtcParameter::IsValidTodayRewardItem(
    _SIG_LOAD_ACTION_POINT::_TodayRewardItem item) {
    return item < 4;
}

// IDA_EA: 0x8894996
// IDA_NAME: _ZN8APSystem24CActionPointEtcParameter32importActionPointSystemEtcScriptEPKcRS0_
// DEMANGLED: APSystem::CActionPointEtcParameter::importActionPointSystemEtcScript(char const*,APSystem::CActionPointEtcParameter&)
int CActionPointEtcParameter::importActionPointSystemEtcScript(
    const char* filename,
    CActionPointEtcParameter& parameter) {
    parameter.ClearData();

    if (loadRDARScriptFile("", filename) != 1) {
        return 0;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[action point list]") {
            while (1) {
                CActionPointEx action;
                int scanned = ScanInt(&success);
                if (!success) {
                    break;
                }
                action.action_index = static_cast<uint16_t>(scanned);

                scanned = ScanInt(&success);
                if (!success) {
                    return 87;
                }
                action.display_flag = static_cast<uint8_t>(scanned);

                scanned = ScanInt(&success);
                if (!success) {
                    return 91;
                }
                action.rule_value = static_cast<uint16_t>(scanned);

                scanned = ScanInt(&success);
                if (!success) {
                    break;
                }
                if (!IsValidActionGroupIndex(
                        static_cast<_ActionGroupIndex>(scanned))) {
                    return 98;
                }
                action.action_group_index =
                    static_cast<_ActionGroupIndex>(scanned);

                scanned = ScanInt(&success);
                if (!success) {
                    return 101;
                }
                action.group_option = static_cast<uint8_t>(scanned);

                for (int index = 0; index <= 4; ++index) {
                    scanned = ScanInt(&success);
                    if (!success) {
                        return 109;
                    }
                    if (scanned != 0) {
                        action.clear_state[index] =
                            static_cast<uint8_t>(
                                -static_cast<int8_t>(scanned));
                    }
                }

                std::string values_text;
                if (ScanStr(&values_text) != 1) {
                    return 118;
                }
                std::vector<char> values_buffer(values_text.begin(),
                                                values_text.end());
                values_buffer.push_back('\0');
                char* save_ptr = 0;
                for (char* token =
                         ::strtok_r(&values_buffer[0], ",", &save_ptr);
                     token != 0;
                     token = ::strtok_r(0, ",", &save_ptr)) {
                    const int value = ::atoi(token);
                    action.values.push_back(value);
                }

                std::string weekday_text;
                if (ScanStr(&weekday_text) != 1) {
                    return 156;
                }
                unsigned int weekday = 0;
                if (ConvertWeekdayStringToInt(weekday_text, weekday) == 1) {
                    action.weekday = weekday;
                } else {
                    action.weekday = 7;
                }

                scanned = ScanInt(&success);
                if (!success) {
                    return 164;
                }
                action.start_value = static_cast<uint8_t>(scanned);

                scanned = ScanInt(&success);
                if (!success) {
                    return 167;
                }
                action.end_value = static_cast<uint8_t>(scanned);

                if (ScanStr(&action.user_message) != 1) {
                    return 170;
                }
                if (ScanStr(&action.system_message) != 1) {
                    return 173;
                }
                parameter.action_groups_[
                    static_cast<uint16_t>(action.action_group_index)]
                    .push_back(action);
            }
        } else if (tag == "[action point user message]") {
            while (1) {
                const int key = ScanInt(&success);
                if (!success) {
                    break;
                }
                std::string message;
                if (ScanStr(&message) != 1) {
                    return 199;
                }
                parameter.user_messages_.insert(
                    std::make_pair(static_cast<uint32_t>(key), message));
            }
        } else if (tag == "[action point system message]") {
            while (1) {
                const int key = ScanInt(&success);
                if (!success) {
                    break;
                }
                std::string message;
                if (ScanStr(&message) != 1) {
                    return 213;
                }
                parameter.system_messages_.insert(
                    std::make_pair(static_cast<uint32_t>(key), message));
            }
        } else if (tag == "[action point medal reward]") {
            while (1) {
                _MedalRewardItem reward;
                reward.medal_type =
                    static_cast<uint16_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                reward.grade = static_cast<uint16_t>(ScanInt(&success));
                reward.item_id =
                    static_cast<uint32_t>(ScanInt(&success));
                reward.item_count =
                    static_cast<uint16_t>(ScanInt(&success));
                parameter.medal_rewards_.push_back(reward);
            }
        } else if (tag == "[action point today reward]") {
            while (1) {
                _TodayRewardItem reward;
                unsigned int reward_slot =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                --reward_slot;
                std::memcpy(&reward.raw[0], &reward_slot, sizeof(reward_slot));
                if (!IsValidTodayRewardItem(
                        static_cast<_SIG_LOAD_ACTION_POINT::_TodayRewardItem>(
                            reward_slot))) {
                    return 250;
                }
                uint16_t word_value =
                    static_cast<uint16_t>(ScanInt(&success));
                std::memcpy(&reward.raw[4], &word_value, sizeof(word_value));
                word_value = static_cast<uint16_t>(ScanInt(&success));
                std::memcpy(&reward.raw[6], &word_value, sizeof(word_value));
                uint32_t dword_value =
                    static_cast<uint32_t>(ScanInt(&success));
                std::memcpy(&reward.raw[8], &dword_value, sizeof(dword_value));
                word_value = static_cast<uint16_t>(ScanInt(&success));
                std::memcpy(&reward.raw[12], &word_value, sizeof(word_value));
                parameter.today_rewards_.push_back(reward);
            }
        }
    }

    parameter.action_index_.clear();
    for (std::map<uint16_t, std::vector<CActionPointEx> >::iterator group =
             parameter.action_groups_.begin();
         group != parameter.action_groups_.end();
         ++group) {
        std::vector<CActionPointEx>& actions = group->second;
        for (std::vector<CActionPointEx>::iterator action = actions.begin();
             action != actions.end();
             ++action) {
            const uint16_t action_index = action->action_index;
            parameter.action_index_.insert(
                std::make_pair(action_index, &(*action)));
        }
    }
    return 0;
}

const std::map<uint16_t, std::vector<CActionPointEx> >&
CActionPointEtcParameter::action_groups() const {
    return action_groups_;
}

const std::map<uint16_t, const CActionPointEx*>&
CActionPointEtcParameter::action_index() const {
    return action_index_;
}

const std::map<uint32_t, std::string>&
CActionPointEtcParameter::user_messages() const {
    return user_messages_;
}

const std::map<uint32_t, std::string>&
CActionPointEtcParameter::system_messages() const {
    return system_messages_;
}

const std::vector<_MedalRewardItem>&
CActionPointEtcParameter::medal_rewards() const {
    return medal_rewards_;
}

const std::vector<TodayRewardItem>&
CActionPointEtcParameter::today_rewards() const {
    return today_rewards_;
}

}  // namespace APSystem
