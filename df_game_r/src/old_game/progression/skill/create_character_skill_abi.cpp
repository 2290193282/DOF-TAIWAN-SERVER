#include "old_data/PVF/data/data_manager.h"

#include <vector>

#include "old_game/progression/skill/create_character_skill_abi.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_game/progression/skill/skill_runtime.h"

// IDA_EA: 0x8604FE2
// IDA_NAME: _Z25addSkillOnCreateCharacterP15_Mastered_skilli
// DEMANGLED: addSkillOnCreateCharacter(_Mastered_skill *,int)
void addSkillOnCreateCharacter(_Mastered_skill* mastered_skill, int job) {
    if (static_cast<unsigned int>(job) > 0x0au) return;

    typedef std::vector<std::pair<int, int> > InitialSkillVector;
    const CDataManager* const data_manager = G_CDataManager();
    // 0x8605021 constructs a local vector copy before iterating it.  Preserve
    // that ownership/lifetime even though the source vector now has a typed
    // CDataManager owner boundary.
    const InitialSkillVector initial_skills(
        data_manager->get_initial_character_skills(job));
    for (InitialSkillVector::const_iterator it = initial_skills.begin();
         it != initial_skills.end(); ++it) {
        CSkill* const skill = data_manager->find_skill(job, it->first);
        if (!skill) continue;

        int first_slot = 0;
        int end_slot = 0;
        bool active_slot_available = false;
        if (skill->is_active_skill()) {
            end_slot = 6;
            for (int slot = first_slot; slot < end_slot; ++slot) {
                if (!mastered_skill[slot].skill_index) {
                    active_slot_available = true;
                    break;
                }
            }
        }

        if (!active_slot_available) {
            const int group = skill->get_group();
            if (group == 1) {
                first_slot = 54;
                end_slot = 102;
            } else if (group == 2) {
                first_slot = 102;
                end_slot = 150;
            } else if (group == 3) {
                first_slot = 150;
                end_slot = 198;
            } else if (group == 0) {
                first_slot = 6;
                end_slot = 54;
            } else {
                cMyTrace trace(
                    "void addSkillOnCreateCharacter(_Mastered_skill*, int)",
                    1353, 5);
                trace("add skill no group %d", group);
            }
        }

        for (int slot = first_slot; slot < end_slot; ++slot) {
            if (!mastered_skill[slot].skill_index) {
                mastered_skill[slot].skill_index =
                    static_cast<char>(it->first);
                mastered_skill[slot].skill_level =
                    static_cast<char>(it->second);
                break;
            }
        }
    }
}
