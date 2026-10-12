#include "old_game/player/core/character_manage_script.h"

#include <cstdio>

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/striker_public_abi.h"


CharacManageScript* CharacManageScript::instance_ = 0;

typedef char ManagePointBonus_Size_Must_Be_0x0c[
    (sizeof(ManagePointBonus) == 0x0c) ? 1 : -1];
typedef char ContentsOptionRevision_Size_Must_Be_0x08[
    (sizeof(ContentsOptionRevision) == 0x08) ? 1 : -1];
typedef char CharacManageScript_Size_Must_Be_0x88[
    (sizeof(CharacManageScript) == 0x88) ? 1 : -1];

// IDA_EA: 0x88E0674
// IDA_NAME: _ZN18CharacManageScript10getInsanceEv
// DEMANGLED: CharacManageScript::getInsance(void)
CharacManageScript* CharacManageScript::getInsance() {
    if (!instance_) {
        instance_ = new CharacManageScript();
    }
    return instance_;
}

// IDA_EA: 0x88E06CC
// IDA_NAME: _ZN18CharacManageScriptC2Ev
// DEMANGLED: CharacManageScript::CharacManageScript(void)
CharacManageScript::CharacManageScript()
    : point_bonus_(),
      manage_level_point_(),
      exp_bonus_(),
      gold_bonus_(),
      manage_level_max_(0),
      manage_basic_explain_(),
      manage_option_(),
      contents_option_revision_() {
    clear();
}

// IDA_EA: 0x88E07F6
// IDA_NAME: _ZN18CharacManageScriptD2Ev
// DEMANGLED: CharacManageScript::~CharacManageScript()
CharacManageScript::~CharacManageScript() {}

// IDA_EA: 0x88E0904
// IDA_NAME: _ZN18CharacManageScript5clearEv
// DEMANGLED: CharacManageScript::clear(void)
int CharacManageScript::clear() {
    point_bonus_.clear();
    manage_level_point_.clear();
    exp_bonus_.clear();
    gold_bonus_.clear();
    manage_level_max_ = 0;
    manage_basic_explain_.clear();
    manage_option_.clear();
    return 0;
}

// IDA_EA: 0x88E0968
// IDA_NAME: _ZN18CharacManageScript12importScriptEPKc
// DEMANGLED: CharacManageScript::importScript(char const*)
int CharacManageScript::importScript(const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string text;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[point bonus]") {
            while (true) {
                ManagePointBonus bonus = {0, 0, 0};
                bonus.begin_level = ScanInt(&success);
                if (!success) {
                    break;
                }
                bonus.end_level = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                bonus.point = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                point_bonus_.push_back(bonus);
            }
        } else if (tag == "[manage level point]") {
            while (true) {
                const int point = ScanInt(&success);
                if (!success) {
                    break;
                }
                manage_level_point_.push_back(point);
            }
        } else if (tag == "[manage level max]") {
            manage_level_max_ = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[exp bonus]" || tag == "[gold bonus]") {
            std::map<int, int>* target =
                (tag == "[exp bonus]") ? &exp_bonus_ : &gold_bonus_;
            while (true) {
                const int key = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int value = ScanInt(&success);
                if (!success) {
                    return 0;
                }

                const std::pair<std::map<int, int>::iterator, bool> inserted =
                    target->insert(std::make_pair(key, value));
                if (!inserted.second) {
                    std::printf("ERROR [%s][%s] key duplicated\n",
                                filename,
                                tag.c_str());
                }
            }
        } else if (tag == "[manage basic explain]") {
            manage_basic_explain_.clear();
            for (int index = 0;; ++index) {
                if (ScanStr(&text) != 1) {
                    break;
                }
                manage_basic_explain_[index] = text;
            }
        } else if (tag == "[manage option]") {
            manage_option_.clear();
            while (true) {
                const int key = ScanInt(&success);
                if (!success) {
                    break;
                }
                manage_option_[key] = ScanInt(static_cast<bool*>(0));
            }
        } else {
            contents_option_revision_.clear();
            while (ScanStr(&text) == 1) {
                ContentsOptionRevision revision = {
                    static_cast<int>(ContentsType::getType(text.c_str())),
                    ScanInt(static_cast<bool*>(0))};
                contents_option_revision_.push_back(revision);
            }
        }
    }

    return 1;
}

// IDA_EA: 0x88E0E0A
// IDA_NAME: _ZN18CharacManageScript8GetPointEi
// DEMANGLED: CharacManageScript::GetPoint(int)
int CharacManageScript::GetPoint(int level) {
    int result = 0;
    for (std::vector<ManagePointBonus>::const_iterator iter =
             point_bonus_.begin();
         iter != point_bonus_.end(); ++iter) {
        for (int current = iter->begin_level; current <= iter->end_level;
             ++current) {
            if (level >= current) {
                result += iter->point;
            }
        }
    }
    return result;
}

// IDA_EA: 0x88E0ED4
// IDA_NAME: _ZN18CharacManageScript8GetLevelEi
// DEMANGLED: CharacManageScript::GetLevel(int)
int CharacManageScript::GetLevel(int point) {
    if (!point) {
        return 0;
    }

    int result = 0;
    for (int index = static_cast<int>(manage_level_point_.size()) - 1;
         index >= 0; --index) {
        if (manage_level_point_[index] <= point) {
            result = index + 1;
            break;
        }
    }

    if (manage_level_max_ >= result) {
        return result;
    }
    return manage_level_max_;
}

// IDA_EA: 0x88E0F58
// IDA_NAME: _ZN18CharacManageScript11GetExpBonusEi
// DEMANGLED: CharacManageScript::GetExpBonus(int)
long double CharacManageScript::GetExpBonus(int level) {
    std::map<int, int>::const_iterator iter = exp_bonus_.find(level);
    if (iter != exp_bonus_.end()) {
        return static_cast<long double>(iter->second) / 100.0L;
    }
    return 0.0L;
}

// IDA_EA: 0x88E0FCE
// IDA_NAME: _ZN18CharacManageScript12GetGoldBonusEi
// DEMANGLED: CharacManageScript::GetGoldBonus(int)
long double CharacManageScript::GetGoldBonus(int level) {
    std::map<int, int>::const_iterator iter = gold_bonus_.find(level);
    if (iter != gold_bonus_.end()) {
        return static_cast<long double>(iter->second) / 100.0L;
    }
    return 0.0L;
}

// IDA_EA: 0x88E1044
// IDA_NAME: _ZN18CharacManageScript11GetExpBonusEiPi
// DEMANGLED: CharacManageScript::GetExpBonus(int,int *)
char CharacManageScript::GetExpBonus(int level, int* value) {
    std::map<int, int>::const_iterator iter = exp_bonus_.find(level);
    const char found = iter != exp_bonus_.end();
    if (found) {
        *value = iter->second;
    }
    return found;
}

// IDA_EA: 0x88E10AC
// IDA_NAME: _ZN18CharacManageScript12GetGoldBonusEiPi
// DEMANGLED: CharacManageScript::GetGoldBonus(int,int *)
char CharacManageScript::GetGoldBonus(int level, int* value) {
    std::map<int, int>::const_iterator iter = gold_bonus_.find(level);
    const char found = iter != gold_bonus_.end();
    if (found) {
        *value = iter->second;
    }
    return found;
}

// IDA_EA: 0x88E1114
// IDA_NAME: _ZN18CharacManageScript22GetLevelUpRequirePointEj
// DEMANGLED: CharacManageScript::GetLevelUpRequirePoint(unsigned int)
int CharacManageScript::GetLevelUpRequirePoint(unsigned int level) {
    if (!level || manage_level_point_.size() < level) {
        return 0;
    }
    return manage_level_point_[level - 1];
}

// IDA_EA: 0x88E1166
// IDA_NAME: _ZN18CharacManageScript7GetNameEj
// DEMANGLED: CharacManageScript::GetName(unsigned int)
std::string CharacManageScript::GetName(unsigned int level) {
    std::map<int, std::string>::const_iterator iter =
        manage_basic_explain_.find(static_cast<int>(level));
    if (iter != manage_basic_explain_.end()) {
        return iter->second;
    }
    return std::string();
}

// IDA_EA: 0x88E11FC
// IDA_NAME: _ZN18CharacManageScript14GetOptionValueEi
// DEMANGLED: CharacManageScript::GetOptionValue(int)
int CharacManageScript::GetOptionValue(int key) {
    std::map<int, int>::const_iterator iter = manage_option_.find(key);
    if (iter != manage_option_.end()) {
        return iter->second;
    }
    return 0;
}

// IDA_EA: 0x88E1266
// IDA_NAME: _ZN18CharacManageScript29GetMinimumPointCharacterLevelEv
// DEMANGLED: CharacManageScript::GetMinimumPointCharacterLevel(void)
int CharacManageScript::GetMinimumPointCharacterLevel() {
    if (point_bonus_.empty()) {
        return 40;
    }
    return point_bonus_[0].begin_level;
}
