#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_CHARACTER_EXPERT_JOB_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_CHARACTER_EXPERT_JOB_H_

#include <stdint.h>

#include "old_game/player/core/charac_expand_cdata.h"


class CUser;
struct STExpertJobScript;

namespace expert_job {

// IDA unit: 04_character_select_create/expert_job/CCharacExpertJob.
// Slot 0 of CUser's character-expand storage.  The derived members reuse the
// CData tail padding exactly as the old 32-bit Itanium ABI does.
class CCharacExpertJob : public charac_expand::CData {
public:
    CCharacExpertJob();
    virtual ~CCharacExpertJob();

    virtual bool loadData(CUser* user, char* data);
    int set_recipe(int recipe_index);
    int get_recipe(int recipe_index) const;
    int InitScript(STExpertJobScript* script);
    int learnRecipe(unsigned long recipe_result_item_index,
                    STExpertJobScript* script,
                    int expert_job_level);
    int SendExpertJobInfo(CUser* user);

    int get_giveup_count() const;
    CCharacExpertJob* set_giveup_count(uint16_t giveup_count);
    int get_disjoint_machine_endurance() const;
    CCharacExpertJob* set_disjoint_machine_endurance(int endurance);
    int get_disjoint_machine_grade() const;
    CCharacExpertJob* set_disjoint_machine_grade(int grade);

protected:
    virtual int _saveData(CUser* user);
    virtual int _reset();

private:
    uint16_t giveup_count_;                // IDA +0x06
    int32_t disjoint_machine_endurance_;   // IDA +0x08
    int32_t disjoint_machine_grade_;       // IDA +0x0C
    uint8_t recipe_flags_[30];              // IDA +0x10..+0x2D
};

}  // namespace expert_job

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_CHARACTER_EXPERT_JOB_H_
