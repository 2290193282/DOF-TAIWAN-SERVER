#ifndef OLDSRV_DATAMANAGER_STRIKER_SCRIPT_H_
#define OLDSRV_DATAMANAGER_STRIKER_SCRIPT_H_

#include "common/manager/striker_public_abi.h"

#include <map>
#include <stddef.h>
#include <stdint.h>
#include <vector>


// IDA_EA: 0x8A9D836, 0x8A9D88E, 0x8A9DA28, 0x8A9DBB4, 0x8A9DC70, 0x8A9DCBE, 0x8A9DDB2, 0x8A9E4D6, 0x8A9E5D0, 0x8A9E5FC, 0x8A9E628, 0x8A9E654, 0x8A9E6CC, 0x8A9E79C, 0x8A9E7E0, 0x8A9E868, 0x8A9E97A, 0x8A9EA96, 0x8A9EB3A, 0x8A9EBEE.
class STScriptFileList;

// IDA unit: 12_pvf_datamanager_scripts/StrikerScript.
class StrikerScript {
public:
    static StrikerScript* getInsance();

    StrikerScript();
    ~StrikerScript();

    StrikerScript* clear();
    StrikerScript* create();
    int destory();
    int importScript(const char* filename);
    void importSkillDataUpVector();
    int importSkillAttackPenalty();
    int importSkillCoolTimePenalty();
    int importStrikerLevelPenalty();
    bool importParentLevelPenlaty();
    bool readSkillPenalty(std::vector<TagSkillPenalty>& penalties);
    bool importComboList();
    std::vector<StrikerSkill>* getStrikerSkillVector(short job,
                                                     short grow_type);
    int checkSkillIndex(short job, short grow_type, int skill_index);
    int getComboIndex(short job, short grow_type, int skill_index);
    int getPrecedeSkillIndex(short job, short grow_type, int skill_index);
    int getAddCommand(short skill_index, int command_index);
    int isCommonSkill(short job, int skill_index);

    int32_t tag_cost_time_;              // +0x00
    int32_t tag_character_setting_;      // +0x04
    int32_t striker_combo_;              // +0x08
    int32_t striker_start_time_;         // +0x0C
    int32_t striker_end_time_;           // +0x10
    std::vector<ContentsType::T> impossible_modules_;  // +0x14
    std::vector<int> impossible_dungeons_;             // +0x20
    std::vector<ContentsRevision> contents_revisions_; // +0x2C
    int32_t position_x_;                 // +0x38
    int32_t position_y_;                 // +0x3C
    std::vector<StrikerSkillDataUp> skill_data_up_;    // +0x40
    std::vector<TagSkillPenalty> attack_penalties_;    // +0x4C
    std::vector<TagSkillPenalty> cooltime_penalties_;  // +0x58
    int32_t striker_level_penalty_min_;  // +0x64
    int32_t striker_level_penalty_max_;  // +0x68
    std::vector<ParentLevelPenalty> parent_level_penalties_; // +0x6C
    std::map<int, std::vector<StrikerSkill>* > striker_skills_; // +0x78
    STScriptFileList* script_file_list_; // +0x90
    std::vector<StrikerSkillAddCommand> add_commands_; // +0x94
    uint8_t imported_;                   // +0xA0
    int32_t common_cool_time_;           // +0xA4
    StrikerDirectionType direction_;     // +0xA8
    int32_t position_type_;              // +0xAC
    int32_t upgrade_revision_;           // +0xB0

    static StrikerScript* instance_;
};

static_assert(sizeof(StrikerScript) == 0xB4u,
              "StrikerScript IDA ABI size must stay 0xB4");
static_assert(offsetof(StrikerScript, impossible_modules_) == 0x14u,
              "StrikerScript impossible_modules offset must stay 0x14");
static_assert(offsetof(StrikerScript, striker_skills_) == 0x78u,
              "StrikerScript striker_skills offset must stay 0x78");
static_assert(offsetof(StrikerScript, imported_) == 0xA0u,
              "StrikerScript imported offset must stay 0xA0");
static_assert(offsetof(StrikerScript, direction_) == 0xA8u,
              "StrikerScript direction offset must stay 0xA8");
static_assert(offsetof(StrikerScript, upgrade_revision_) == 0xB0u,
              "StrikerScript upgrade_revision offset must stay 0xB0");

#endif  // OLDSRV_DATAMANAGER_STRIKER_SCRIPT_H_
