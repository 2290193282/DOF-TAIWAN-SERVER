#ifndef OLDSRV_GLOBAL_STRIKER_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_STRIKER_PUBLIC_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>


// IDA_EA: ContentsType @0x8AC3434/0x8AC34A4; TagSkillPenalty @0x8A9D768;
// importSkillDataUp @0x88C049C.
// IDA unit: 99_misc_global/ContentsType.
struct ContentsType {
    enum T {
        CONTENTS_NONE = 0,
        CONTENTS_NORMAL = 1,
        CONTENTS_ANCIENT = 2,
        CONTENTS_DIMENSION = 3,
        CONTENTS_TOWER = 4,
        CONTENTS_INFINITE = 5,
        CONTENTS_ULTIMATE = 6,
        CONTENTS_DESPAIR = 7,
        CONTENTS_TOURNAMENT = 8,
        CONTENTS_PVP = 9,
        CONTENTS_FAIR_PVP = 10,
        CONTENTS_WARROOM = 11,
        CONTENTS_ASSAULT = 12
    };

    static const char* getString(T type);
    static T getType(const char* text);
};

// IDA unit: 99_misc_global/ContentsRevision.
struct ContentsRevision {
    ~ContentsRevision();

    int32_t contents_type;  // +0x00, ContentsType::T
    int32_t value_04;       // +0x04
    int32_t value_08;       // +0x08
    int32_t value_0C;       // +0x0C
};

static_assert(sizeof(ContentsRevision) == 0x10u,
              "ContentsRevision IDA ABI size must stay 0x10");

// IDA unit: 99_misc_global/ParentLevelPenalty.
struct ParentLevelPenalty {
    ~ParentLevelPenalty();

    int32_t parent_level;   // +0x00
    int32_t penalty_level;  // +0x04
    float value_08;         // +0x08
    float value_0C;         // +0x0C
};

static_assert(sizeof(ParentLevelPenalty) == 0x10u,
              "ParentLevelPenalty IDA ABI size must stay 0x10");

// IDA unit: 99_misc_global/StrikerDirectionType.
struct StrikerDirectionType {
    enum T {
        DIRECTION_LEFT = 0,
        DIRECTION_RIGHT = 1
    };

    StrikerDirectionType();

    StrikerDirectionType* setType(int type);
    bool isType(T type) const;

    int32_t type;  // +0x00
};

static_assert(sizeof(StrikerDirectionType) == 0x04u,
              "StrikerDirectionType IDA ABI size must stay 0x04");

// IDA unit: 08_quest_skill_growth/TagSkillPenalty.
struct TagSkillPenalty {
    TagSkillPenalty();
    ~TagSkillPenalty();

    TagSkillPenalty* clear();
    bool isPenaltyLevel(int level);

    int32_t min_level;      // +0x00
    int32_t max_level;      // +0x04
    uint8_t percent;        // +0x08
    uint8_t pad_09[3];
    int32_t value;          // +0x0C
};

static_assert(sizeof(TagSkillPenalty) == 0x10u,
              "TagSkillPenalty IDA ABI size must stay 0x10");
static_assert(offsetof(TagSkillPenalty, value) == 0x0Cu,
              "TagSkillPenalty value offset must stay 0x0C");

// IDA unit: 06_userinfo_stat_inventory/EquipmentSkillDataUp.
struct EquipmentSkillDataUp {
    EquipmentSkillDataUp();
    ~EquipmentSkillDataUp();

    int32_t character_job;      // +0x00, invalid-job sentinel
    int32_t skill_index;        // +0x04
    int32_t value_08;           // +0x08
    int32_t sign_is_not_plus;   // +0x0C
    int32_t value_10;           // +0x10
    int32_t data_up_type;       // +0x14
    int32_t module_type;        // +0x18, default 2
};

static_assert(sizeof(EquipmentSkillDataUp) == 0x1Cu,
              "EquipmentSkillDataUp IDA ABI size must stay 0x1C");
static_assert(offsetof(EquipmentSkillDataUp, data_up_type) == 0x14u,
              "EquipmentSkillDataUp data-up type offset must stay 0x14");
static_assert(offsetof(EquipmentSkillDataUp, module_type) == 0x18u,
              "EquipmentSkillDataUp module type offset must stay 0x18");

// IDA unit: 08_quest_skill_growth/StrikerSkill.
struct StrikerSkill {
    StrikerSkill();
    ~StrikerSkill();
    StrikerSkill(const StrikerSkill& other);

    StrikerSkill& operator=(const StrikerSkill& other);

    int32_t skill_index;          // +0x00
    int32_t combo_index;          // +0x04
    int32_t precede_skill_index;  // +0x08
    std::string command;          // +0x0C
};

static_assert(sizeof(StrikerSkill) == 0x10u,
              "StrikerSkill IDA ABI size must stay 0x10");
static_assert(offsetof(StrikerSkill, command) == 0x0Cu,
              "StrikerSkill command offset must stay 0x0C");

// IDA unit: 08_quest_skill_growth/StrikerSkillDataUp.
struct StrikerSkillDataUp {
    StrikerSkillDataUp();
    ~StrikerSkillDataUp();
    StrikerSkillDataUp(const StrikerSkillDataUp& other);

    StrikerSkillDataUp& operator=(const StrikerSkillDataUp& other);

    int32_t min_level;  // +0x00
    int32_t max_level;  // +0x04
    std::vector<EquipmentSkillDataUp> skill_data_up;  // +0x08
};

static_assert(sizeof(StrikerSkillDataUp) == 0x14u,
              "StrikerSkillDataUp IDA ABI size must stay 0x14");
static_assert(offsetof(StrikerSkillDataUp, skill_data_up) == 0x08u,
              "StrikerSkillDataUp vector offset must stay 0x08");

// IDA unit: 08_quest_skill_growth/StrikerSkillAddCommand.
struct StrikerSkillAddCommand {
    ~StrikerSkillAddCommand();

    StrikerSkillAddCommand* setType(int type);

    int32_t skill_index;  // +0x00
    int32_t value_04;     // +0x04
    int32_t type;         // +0x08
};

static_assert(sizeof(StrikerSkillAddCommand) == 0x0Cu,
              "StrikerSkillAddCommand IDA ABI size must stay 0x0C");

// IDA unit: 99_misc_global/Global::importSkillDataUp.
void importSkillDataUp(std::vector<EquipmentSkillDataUp>& data_up);


#endif  // OLDSRV_GLOBAL_STRIKER_PUBLIC_ABI_H_
