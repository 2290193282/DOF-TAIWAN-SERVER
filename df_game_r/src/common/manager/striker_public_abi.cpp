#include "common/manager/striker_public_abi.h"

#include "common/config.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/global_tools.h"

#include <cstring>
#include <string>


namespace {

struct SkillDataUpTag {
    const char* tag;
    int32_t type;
    bool keep_scanned_value;
};

const SkillDataUpTag kSkillDataUpTags[] = {
    {"[static]", 0, true},
    {"[level]", 1, true},
    {"[cooltime]", 2, false},
    {"[mp]", 3, false},
    {"[maintain mp]", 4, false},
    {"[skill consume item]", 5, true},
    {"[casting time]", 6, false},
};

bool GetSkillDataUpTag(const std::string& tag, SkillDataUpTag* result) {
    for (std::size_t index = 0;
         index < sizeof(kSkillDataUpTags) / sizeof(kSkillDataUpTags[0]);
         ++index) {
        if (tag == kSkillDataUpTags[index].tag) {
            *result = kSkillDataUpTags[index];
            return true;
        }
    }
    return false;
}

}  // namespace

// IDA_EA: 0x8AC3434
// IDA_NAME: _ZN12ContentsType9getStringENS_1TE
// DEMANGLED: ContentsType::getString(ContentsType::T)
const char* ContentsType::getString(ContentsType::T type) {
    switch (type) {
        case CONTENTS_NORMAL:
            return "[normal]";
        case CONTENTS_ANCIENT:
            return "[ancient]";
        case CONTENTS_DIMENSION:
            return "[dimension]";
        case CONTENTS_TOWER:
            return "[tower]";
        case CONTENTS_INFINITE:
            return "[infinite]";
        case CONTENTS_ULTIMATE:
            return "[ultimate]";
        case CONTENTS_DESPAIR:
            return "[despair]";
        case CONTENTS_TOURNAMENT:
            return "[tournament]";
        case CONTENTS_PVP:
            return "[pvp]";
        case CONTENTS_FAIR_PVP:
            return "[fair pvp]";
        case CONTENTS_WARROOM:
            return "[warroom]";
        case CONTENTS_ASSAULT:
            return "[assault]";
        default:
            return "";
    }
}

// IDA_EA: 0x8AC34A4
// IDA_NAME: _ZN12ContentsType7getTypeEPKc
// DEMANGLED: ContentsType::getType(char const*)
ContentsType::T ContentsType::getType(const char* text) {
    for (int value = CONTENTS_NONE; value <= CONTENTS_ASSAULT; ++value) {
        if (std::strcmp(text, getString(static_cast<ContentsType::T>(value))) ==
            0) {
            return static_cast<ContentsType::T>(value);
        }
    }
    return CONTENTS_NONE;
}

// IDA_EA: 0x8AA2910
// IDA_NAME: _ZN16ContentsRevisionD2Ev
// DEMANGLED: ContentsRevision::~ContentsRevision()
ContentsRevision::~ContentsRevision() {}

// IDA_EA: 0x8AA2DD6
// IDA_NAME: _ZN18ParentLevelPenaltyD2Ev
// DEMANGLED: ParentLevelPenalty::~ParentLevelPenalty()
ParentLevelPenalty::~ParentLevelPenalty() {}

// IDA_EA: 0x8A9D7EC
// IDA_NAME: _ZN20StrikerDirectionTypeC2Ev
// DEMANGLED: StrikerDirectionType::StrikerDirectionType(void)
StrikerDirectionType::StrikerDirectionType()
    : type(0) {}

// IDA_EA: 0x8A9D7FA
// IDA_NAME: _ZN20StrikerDirectionType7setTypeEi
// DEMANGLED: StrikerDirectionType::setType(int)
StrikerDirectionType* StrikerDirectionType::setType(int new_type) {
    if (new_type >= DIRECTION_LEFT && new_type <= DIRECTION_RIGHT) {
        type = new_type;
    }
    return this;
}

// IDA_EA: 0x8A9D81A
// IDA_NAME: _ZN20StrikerDirectionType6isTypeENS_1TE
// DEMANGLED: StrikerDirectionType::isType(StrikerDirectionType::T)
bool StrikerDirectionType::isType(StrikerDirectionType::T query) const {
    return type == query;
}

// IDA_EA: 0x8A9D77C
// IDA_NAME: _ZN15TagSkillPenalty5clearEv
// DEMANGLED: TagSkillPenalty::clear(void)
TagSkillPenalty* TagSkillPenalty::clear() {
    min_level = 0;
    max_level = 0;
    percent = 0;
    value = 0;
    return this;
}

// IDA_EA: 0x8A9D768
// IDA_NAME: _ZN15TagSkillPenaltyC2Ev
// DEMANGLED: TagSkillPenalty::TagSkillPenalty(void)
TagSkillPenalty::TagSkillPenalty() {
    clear();
}

// IDA_EA: 0x8AA3022
// IDA_NAME: _ZN15TagSkillPenaltyD2Ev
// DEMANGLED: TagSkillPenalty::~TagSkillPenalty()
TagSkillPenalty::~TagSkillPenalty() {}

// IDA_EA: 0x8A9D7A6
// IDA_NAME: _ZN15TagSkillPenalty14isPenaltyLevelEi
// DEMANGLED: TagSkillPenalty::isPenaltyLevel(int)
bool TagSkillPenalty::isPenaltyLevel(int level) {
    return min_level <= level && max_level >= level;
}

// IDA_EA: 0x88C41B6
// IDA_NAME: _ZN20EquipmentSkillDataUpC2Ev
// DEMANGLED: EquipmentSkillDataUp::EquipmentSkillDataUp(void)
EquipmentSkillDataUp::EquipmentSkillDataUp()
    : character_job(oldsrv::config::kCharacterJobInvalid),
      skill_index(0),
      value_08(0),
      sign_is_not_plus(0),
      value_10(0),
      data_up_type(0),
      module_type(2) {}

// IDA_EA: 0x88C6B9C
// IDA_NAME: _ZN20EquipmentSkillDataUpD2Ev
// DEMANGLED: EquipmentSkillDataUp::~EquipmentSkillDataUp()
EquipmentSkillDataUp::~EquipmentSkillDataUp() {}

// IDA_EA: 0x8A9ED1E
// IDA_NAME: _ZN12StrikerSkillC2Ev
// DEMANGLED: StrikerSkill::StrikerSkill(void)
StrikerSkill::StrikerSkill()
    : skill_index(0),
      combo_index(0),
      precede_skill_index(0),
      command() {}

// IDA_EA: 0x8A9ED34
// IDA_NAME: _ZN12StrikerSkillD2Ev
// DEMANGLED: StrikerSkill::~StrikerSkill()
StrikerSkill::~StrikerSkill() {}

// IDA_EA: 0x8A9FF52
// IDA_NAME: _ZN12StrikerSkillC2ERKS_
// DEMANGLED: StrikerSkill::StrikerSkill(StrikerSkill const&)
StrikerSkill::StrikerSkill(const StrikerSkill& other)
    : skill_index(other.skill_index),
      combo_index(other.combo_index),
      precede_skill_index(other.precede_skill_index),
      command(other.command) {}

// IDA_EA: 0x8A9FFF2
// IDA_NAME: _ZN12StrikerSkillaSERKS_
// DEMANGLED: StrikerSkill::operator=(StrikerSkill const&)
StrikerSkill& StrikerSkill::operator=(const StrikerSkill& other) {
    skill_index = other.skill_index;
    combo_index = other.combo_index;
    precede_skill_index = other.precede_skill_index;
    command = other.command;
    return *this;
}

// IDA_EA: 0x8A9ED4A
// IDA_NAME: _ZN18StrikerSkillDataUpC2Ev
// DEMANGLED: StrikerSkillDataUp::StrikerSkillDataUp(void)
StrikerSkillDataUp::StrikerSkillDataUp()
    : min_level(0),
      max_level(0),
      skill_data_up() {}

// IDA_EA: 0x8A9ED60
// IDA_NAME: _ZN18StrikerSkillDataUpD2Ev
// DEMANGLED: StrikerSkillDataUp::~StrikerSkillDataUp()
StrikerSkillDataUp::~StrikerSkillDataUp() {}

// IDA_EA: 0x8AA0AF8
// IDA_NAME: _ZN18StrikerSkillDataUpC2ERKS_
// DEMANGLED: StrikerSkillDataUp::StrikerSkillDataUp(StrikerSkillDataUp const&)
StrikerSkillDataUp::StrikerSkillDataUp(const StrikerSkillDataUp& other)
    : min_level(other.min_level),
      max_level(other.max_level),
      skill_data_up(other.skill_data_up) {}

// IDA_EA: 0x8AA0BB2
// IDA_NAME: _ZN18StrikerSkillDataUpaSERKS_
// DEMANGLED: StrikerSkillDataUp::operator=(StrikerSkillDataUp const&)
StrikerSkillDataUp& StrikerSkillDataUp::operator=(
    const StrikerSkillDataUp& other) {
    min_level = other.min_level;
    max_level = other.max_level;
    skill_data_up = other.skill_data_up;
    return *this;
}

// IDA_EA: 0x8AA26B4
// IDA_NAME: _ZN22StrikerSkillAddCommandD2Ev
// DEMANGLED: StrikerSkillAddCommand::~StrikerSkillAddCommand()
StrikerSkillAddCommand::~StrikerSkillAddCommand() {}

// IDA_EA: 0x8A9D7CC
// IDA_NAME: _ZN22StrikerSkillAddCommand7setTypeEi
// DEMANGLED: StrikerSkillAddCommand::setType(int)
StrikerSkillAddCommand* StrikerSkillAddCommand::setType(int new_type) {
    if (new_type >= 0 && new_type <= 8) {
        type = new_type;
    }
    return this;
}

// IDA_EA: 0x88C049C
// IDA_NAME: _Z17importSkillDataUpRSt6vectorI20EquipmentSkillDataUpSaIS0_EE
// DEMANGLED: importSkillDataUp(std::vector<EquipmentSkillDataUp> &)
void importSkillDataUp(std::vector<EquipmentSkillDataUp>& data_up) {
    EquipmentSkillDataUp row;
    std::string text;

    data_up.reserve(20u);
    while (ScanStr(&text) == 1) {
        row.character_job = ::getCharacterJob(text.c_str());
        row.skill_index = ScanInt(static_cast<bool*>(0));
        row.module_type = scanModuleType();

        if (ScanStr(&text) != 1) {
            break;
        }

        SkillDataUpTag tag;
        if (!GetSkillDataUpTag(text, &tag)) {
            break;
        }

        row.data_up_type = tag.type;
        if (tag.keep_scanned_value) {
            row.value_08 = ScanInt(static_cast<bool*>(0));
        } else {
            ScanInt(static_cast<bool*>(0));
            row.value_08 = 0;
        }

        if (ScanStr(&text) != 1) {
            break;
        }

        row.sign_is_not_plus = text != "+";
        row.value_10 = ScanInt(static_cast<bool*>(0));
        data_up.push_back(row);
    }
}
