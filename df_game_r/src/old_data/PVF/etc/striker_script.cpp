#include "old_data/PVF/etc/striker_script.h"

#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/common/script_file_list.h"
#include "common/manager/global_tools.h"

#include <new>
#include <stdint.h>
#include <string>


StrikerScript* StrikerScript::instance_ = 0;

// IDA_EA: 0x8A9D836
// IDA_NAME: _ZN13StrikerScript10getInsanceEv
// DEMANGLED: StrikerScript::getInsance(void)
StrikerScript* StrikerScript::getInsance() {
    if (!instance_) {
        instance_ = new StrikerScript();
    }
    return instance_;
}

// IDA_EA: 0x8A9D88E
// IDA_NAME: _ZN13StrikerScriptC2Ev
// DEMANGLED: StrikerScript::StrikerScript(void)
StrikerScript::StrikerScript()
    : tag_cost_time_(0),
      tag_character_setting_(0),
      striker_combo_(0),
      striker_start_time_(0),
      striker_end_time_(0),
      impossible_modules_(),
      impossible_dungeons_(),
      contents_revisions_(),
      position_x_(0),
      position_y_(0),
      skill_data_up_(),
      attack_penalties_(),
      cooltime_penalties_(),
      striker_level_penalty_min_(0),
      striker_level_penalty_max_(0),
      parent_level_penalties_(),
      striker_skills_(),
      script_file_list_(0),
      add_commands_(),
      imported_(0),
      common_cool_time_(0),
      direction_(),
      position_type_(0),
      upgrade_revision_(0) {
    clear();
    create();
}

// IDA_EA: 0x8A9DA28
// IDA_NAME: _ZN13StrikerScriptD2Ev
// DEMANGLED: StrikerScript::~StrikerScript()
StrikerScript::~StrikerScript() {
    destory();
}

// IDA_EA: 0x8A9DBB4
// IDA_NAME: _ZN13StrikerScript5clearEv
// DEMANGLED: StrikerScript::clear(void)
StrikerScript* StrikerScript::clear() {
    tag_cost_time_ = 0;
    tag_character_setting_ = 0;
    striker_combo_ = 0;
    striker_start_time_ = 0;
    striker_end_time_ = 500;
    impossible_modules_.clear();
    impossible_dungeons_.clear();
    position_x_ = 0;
    position_y_ = 0;
    script_file_list_ = 0;
    imported_ = 0;
    striker_level_penalty_min_ = 0;
    striker_level_penalty_max_ = 0;
    common_cool_time_ = 0;
    position_type_ = 0;
    upgrade_revision_ = 99;
    return this;
}

// IDA_EA: 0x8A9DC70
// IDA_NAME: _ZN13StrikerScript6createEv
// DEMANGLED: StrikerScript::create(void)
StrikerScript* StrikerScript::create() {
    script_file_list_ = new STScriptFileList();
    return this;
}

// IDA_EA: 0x8A9DCBE
// IDA_NAME: _ZN13StrikerScript7destoryEv
// DEMANGLED: StrikerScript::destory(void)
int StrikerScript::destory() {
    if (script_file_list_) {
        delete script_file_list_;
    }
    script_file_list_ = 0;

    for (std::map<int, std::vector<StrikerSkill>* >::iterator it =
             striker_skills_.begin();
         it != striker_skills_.end(); ++it) {
        if (it->second) {
            delete it->second;
            it->second = 0;
        }
    }
    striker_skills_.clear();
    return 0;
}

// IDA_EA: 0x8A9DDB2
// IDA_NAME: _ZN13StrikerScript12importScriptEPKc
// DEMANGLED: StrikerScript::importScript(char const*)
int StrikerScript::importScript(const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    std::string text;
    bool success = false;
    int scanned = 0;

    while (ScanType(tag, true) == 1) {
        if (tag == "[tag mode]") {
            scanned = ScanInt(&success);
        } else if (tag == "[tag cost time]") {
            scanned = ScanInt(&success);
            tag_cost_time_ = scanned;
        } else if (tag == "[tag character setting]") {
            tag_character_setting_ = ScanInt(&success);
        } else if (tag == "[striker combo]") {
            striker_combo_ = ScanInt(&success);
        } else if (tag == "[striker start time]") {
            scanned = ScanInt(&success);
            striker_start_time_ = scanned;
        } else if (tag == "[striker skill data up]") {
            importSkillDataUpVector();
        } else if (tag == "[striker skill attack penalty]") {
            importSkillAttackPenalty();
        } else if (tag == "[striker skill cooltime penalty]") {
            importSkillCoolTimePenalty();
        } else if (tag == "[striker level penalty]") {
            importParentLevelPenlaty();
        } else if (tag == "[striker module impossible]") {
            impossible_modules_.clear();
            while (ScanStr(&text) == 1) {
                const ContentsType::T type =
                    ContentsType::getType(text.c_str());
                impossible_modules_.push_back(type);
            }
        } else if (tag == "[striker dungeon impossible]") {
            impossible_dungeons_.clear();
            while (ScanInt(&scanned)) {
                impossible_dungeons_.push_back(scanned);
            }
        } else if (tag == "[striker end time]") {
            striker_end_time_ = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[position]") {
            position_type_ = ScanInt(static_cast<bool*>(0));
            position_x_ = ScanInt(static_cast<bool*>(0));
            position_y_ = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[striker skill]") {
            while (1) {
                StrikerSkill row;
                const int job = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int grow_type = ScanInt(static_cast<bool*>(0));
                row.skill_index = ScanInt(static_cast<bool*>(0));
                row.combo_index = ScanInt(static_cast<bool*>(0));
                row.precede_skill_index = ScanInt(static_cast<bool*>(0));
                ScanStr(&text);
                row.command = text;

                const int key = static_cast<int>(
                    static_cast<uint16_t>(job) |
                    (static_cast<uint32_t>(
                         static_cast<uint16_t>(grow_type)) << 16));
                std::map<int, std::vector<StrikerSkill>* >::iterator found =
                    striker_skills_.find(key);
                if (found == striker_skills_.end()) {
                    std::vector<StrikerSkill>* rows =
                        new std::vector<StrikerSkill>();
                    rows->push_back(row);
                    striker_skills_[key] = rows;
                } else {
                    found->second->push_back(row);
                }
            }
        } else if (tag == "[striker skill macro]") {
            add_commands_.clear();
            while (ScanInt(&scanned)) {
                StrikerSkillAddCommand row;
                row.skill_index = scanned;
                row.value_04 = ScanInt(static_cast<bool*>(0));
                row.type = 0;
                row.setType(ScanInt(static_cast<bool*>(0)));
                add_commands_.push_back(row);
            }
        } else if (tag == "[common cool time]") {
            common_cool_time_ = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[direction type]") {
            direction_.setType(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[contents revision]") {
            contents_revisions_.clear();
            while (ScanStr(&text) == 1) {
                ContentsRevision row;
                row.contents_type = ContentsType::getType(text.c_str());
                row.value_04 = ScanInt(static_cast<bool*>(0));
                row.value_08 = ScanInt(static_cast<bool*>(0));
                row.value_0C = ScanInt(static_cast<bool*>(0));
                contents_revisions_.push_back(row);
            }
        } else if (tag == "[upgrade revision]") {
            upgrade_revision_ = ScanInt(static_cast<bool*>(0));
        }
    }

    imported_ = 1;
    return 1;
}

// IDA_EA: 0x8A9E4D6
// IDA_NAME: _ZN13StrikerScript23importSkillDataUpVectorEv
// DEMANGLED: StrikerScript::importSkillDataUpVector(void)
void StrikerScript::importSkillDataUpVector() {
    std::string tag;
    skill_data_up_.clear();
    while (ScanType(tag, true) == 1 && tag == "[level]") {
        StrikerSkillDataUp row;
        row.min_level = ScanInt(static_cast<bool*>(0));
        row.max_level = ScanInt(static_cast<bool*>(0));
        importSkillDataUp(row.skill_data_up);
        skill_data_up_.push_back(row);
    }
}

// IDA_EA: 0x8A9E5D0
// IDA_NAME: _ZN13StrikerScript24importSkillAttackPenaltyEv
// DEMANGLED: StrikerScript::importSkillAttackPenalty(void)
int StrikerScript::importSkillAttackPenalty() {
    attack_penalties_.clear();
    return readSkillPenalty(attack_penalties_);
}

// IDA_EA: 0x8A9E5FC
// IDA_NAME: _ZN13StrikerScript26importSkillCoolTimePenaltyEv
// DEMANGLED: StrikerScript::importSkillCoolTimePenalty(void)
int StrikerScript::importSkillCoolTimePenalty() {
    cooltime_penalties_.clear();
    return readSkillPenalty(cooltime_penalties_);
}

// IDA_EA: 0x8A9E628
// IDA_NAME: _ZN13StrikerScript25importStrikerLevelPenaltyEv
// DEMANGLED: StrikerScript::importStrikerLevelPenalty(void)
int StrikerScript::importStrikerLevelPenalty() {
    striker_level_penalty_min_ = ScanInt(static_cast<bool*>(0));
    striker_level_penalty_max_ = ScanInt(static_cast<bool*>(0));
    return striker_level_penalty_max_;
}

// IDA_EA: 0x8A9E654
// IDA_NAME: _ZN13StrikerScript24importParentLevelPenlatyEv
// DEMANGLED: StrikerScript::importParentLevelPenlaty(void)
bool StrikerScript::importParentLevelPenlaty() {
    parent_level_penalties_.clear();
    bool success = true;
    while (1) {
        ParentLevelPenalty row;
        row.parent_level = ScanInt(&success);
        if (!success) {
            break;
        }
        row.penalty_level = ScanInt(static_cast<bool*>(0));
        row.value_08 = ScanFloat(static_cast<bool*>(0));
        row.value_0C = ScanFloat(static_cast<bool*>(0));
        parent_level_penalties_.push_back(row);
    }
    return !success;
}

// IDA_EA: 0x8A9E6CC
// IDA_NAME: _ZN13StrikerScript16readSkillPenaltyERSt6vectorI15TagSkillPenaltySaIS1_EE
// DEMANGLED: StrikerScript::readSkillPenalty(std::vector<TagSkillPenalty,std::allocator<TagSkillPenalty>> &)
bool StrikerScript::readSkillPenalty(std::vector<TagSkillPenalty>& penalties) {
    while (1) {
        bool success = true;
        TagSkillPenalty row;
        row.min_level = ScanInt(&success);
        if (!success) {
            break;
        }
        row.max_level = ScanInt(static_cast<bool*>(0));

        std::string text;
        ScanStr(&text);
        row.percent = text == "%" ? 1 : 0;
        row.value = ScanInt(static_cast<bool*>(0));
        penalties.push_back(row);
    }
    return true;
}

// IDA_EA: 0x8A9E79C
// IDA_NAME: _ZN13StrikerScript15importComboListEv
// DEMANGLED: StrikerScript::importComboList(void)
bool StrikerScript::importComboList() {
    return loadRDARScriptListFile(script_file_list_,
                                  "",
                                  "etc/LinkSystem/strikercombo.lst") == 1;
}

// IDA_EA: 0x8A9E7E0
// IDA_NAME: _ZN13StrikerScript21getStrikerSkillVectorEss
// DEMANGLED: StrikerScript::getStrikerSkillVector(short,short)
std::vector<StrikerSkill>* StrikerScript::getStrikerSkillVector(
    short job,
    short grow_type) {
    const int key = static_cast<int>(
        static_cast<uint16_t>(job) |
        (static_cast<uint32_t>(static_cast<uint16_t>(grow_type)) << 16));
    std::map<int, std::vector<StrikerSkill>* >::iterator found =
        striker_skills_.find(key);
    if (found != striker_skills_.end()) {
        return found->second;
    }
    return 0;
}

// IDA_EA: 0x8A9E868
// IDA_NAME: _ZN13StrikerScript15checkSkillIndexEssi
// DEMANGLED: StrikerScript::checkSkillIndex(short,short,int)
int StrikerScript::checkSkillIndex(short job,
                                   short grow_type,
                                   int skill_index) {
    for (int index = 0; index <= 1; ++index) {
        std::vector<StrikerSkill>* rows =
            index ? getStrikerSkillVector(job, 0)
                  : getStrikerSkillVector(job, grow_type);
        if (!rows) {
            return 0;
        }
        for (std::vector<StrikerSkill>::iterator it = rows->begin();
             it != rows->end(); ++it) {
            if (it->skill_index == skill_index) {
                return 1;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x8A9E97A
// IDA_NAME: _ZN13StrikerScript13getComboIndexEssi
// DEMANGLED: StrikerScript::getComboIndex(short,short,int)
int StrikerScript::getComboIndex(short job,
                                 short grow_type,
                                 int skill_index) {
    for (int index = 0; index <= 1; ++index) {
        std::vector<StrikerSkill>* rows =
            index ? getStrikerSkillVector(job, 0)
                  : getStrikerSkillVector(job, grow_type);
        if (!rows) {
            return 0;
        }
        for (std::vector<StrikerSkill>::iterator it = rows->begin();
             it != rows->end(); ++it) {
            if (it->skill_index == skill_index) {
                return it->combo_index;
            }
        }
    }
    return 0;
}

// IDA_EA: 0x8A9EA96
// IDA_NAME: _ZN13StrikerScript20getPrecedeSkillIndexEssi
// DEMANGLED: StrikerScript::getPrecedeSkillIndex(short,short,int)
int StrikerScript::getPrecedeSkillIndex(short job,
                                        short grow_type,
                                        int skill_index) {
    std::vector<StrikerSkill>* rows = getStrikerSkillVector(job, grow_type);
    if (!rows) {
        return 0;
    }
    for (std::vector<StrikerSkill>::size_type index = 0;
         index < rows->size(); ++index) {
        if ((*rows)[index].skill_index == skill_index) {
            return (*rows)[index].precede_skill_index;
        }
    }
    return 0;
}

// IDA_EA: 0x8A9EB3A
// IDA_NAME: _ZN13StrikerScript13getAddCommandEsi
// DEMANGLED: StrikerScript::getAddCommand(short,int)
int StrikerScript::getAddCommand(short skill_index, int command_index) {
    for (std::vector<StrikerSkillAddCommand>::size_type index = 0;
         index < add_commands_.size(); ++index) {
        if (add_commands_[index].skill_index == skill_index &&
            add_commands_[index].value_04 == command_index) {
            return add_commands_[index].type;
        }
    }
    return 0;
}

// IDA_EA: 0x8A9EBEE
// IDA_NAME: _ZN13StrikerScript13isCommonSkillEsi
// DEMANGLED: StrikerScript::isCommonSkill(short,int)
int StrikerScript::isCommonSkill(short job, int skill_index) {
    std::vector<StrikerSkill>* rows = getStrikerSkillVector(job, 0);
    if (!rows) {
        return 0;
    }
    for (std::vector<StrikerSkill>::iterator it = rows->begin();
         it != rows->end(); ++it) {
        if (it->skill_index == skill_index) {
            return 1;
        }
    }
    return 0;
}
