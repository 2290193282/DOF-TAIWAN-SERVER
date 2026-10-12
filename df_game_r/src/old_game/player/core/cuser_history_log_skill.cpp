#include "old_game/player/core/cuser.h"

#include <cstdio>
#include <cstring>
#include <utility>
#include <vector>

#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_changer_runtime.h"

// IDA_EA: 0x8682B2E
// IDA_NAME: _ZN15cUserHistoryLog8SkillAddEiiii15eSkillAddReason
// DEMANGLED: cUserHistoryLog::SkillAdd(
//             int,int,int,int,eSkillAddReason)
int cUserHistoryLog::SkillAdd(int charac_job,
                              int skill_tree,
                              int skill_index,
                              int skill_level,
                              eSkillAddReason reason) {
    return owner_->LogHistory("Skill+,%d,%d,%d,%d,%d",
                              charac_job,
                              skill_index,
                              skill_level,
                              static_cast<int>(reason),
                              skill_tree);
}

// IDA_EA: 0x8682B6E
// IDA_NAME: _ZN15cUserHistoryLog8SkillDelEiiii15eSkillDelReason
// DEMANGLED: cUserHistoryLog::SkillDel(
//             int,int,int,int,eSkillDelReason)
int cUserHistoryLog::SkillDel(int charac_job,
                              int skill_tree,
                              int skill_index,
                              int skill_level,
                              eSkillDelReason reason) {
    return owner_->LogHistory("Skill-,%d,%d,%d,%d,%d",
                              charac_job,
                              skill_index,
                              skill_level,
                              static_cast<int>(reason),
                              skill_tree);
}

// IDA_EA: 0x8682CFA
// IDA_NAME: _ZN15cUserHistoryLog9InitSkillEiiii16eSkillInitReason
// DEMANGLED: cUserHistoryLog::InitSkill(int,int,int,int,eSkillInitReason)
int cUserHistoryLog::InitSkill(int skill_tree,
                               int charac_level,
                               int remaining_sp,
                               int remaining_sfp,
                               eSkillInitReason reason) {
    return owner_->LogHistory("SkillInit,%d,%d,%d,%d,%d",
                              skill_tree,
                              charac_level,
                              remaining_sp,
                              remaining_sfp,
                              static_cast<int>(reason));
}

// IDA_EA: 0x8685DCE
// IDA_NAME: _ZN15cUserHistoryLog9SkillSaveEPK15_Mastered_skillji16eSkillSaveReason
// DEMANGLED: cUserHistoryLog::SkillSave(
//     _Mastered_skill const*,unsigned int,int,eSkillSaveReason)
void cUserHistoryLog::SkillSave(const _Mastered_skill* skills,
                                unsigned int charac_job,
                                int skill_tree,
                                eSkillSaveReason reason) {
    const int kSkillsPerHistoryRecord = 15;
    std::vector<std::pair<int, int> > active_skills;
    for (int slot = 0; slot <= 203; ++slot) {
        if (skills[slot].skill_index && (slot <= 137 || slot > 149)) {
            active_skills.push_back(std::make_pair(
                static_cast<unsigned char>(skills[slot].skill_index),
                static_cast<unsigned char>(skills[slot].skill_level)));
        }
    }

    const int record_count = static_cast<int>(
        static_cast<long double>(active_skills.size()) /
        static_cast<long double>(kSkillsPerHistoryRecord)) + 1;
    char reason_text[16];
    std::sprintf(reason_text, ",%d, ", static_cast<int>(reason));

    for (int record = 0; record < record_count; ++record) {
        const int first = kSkillsPerHistoryRecord * record;
        const int last = record >= record_count - 1
                             ? first + static_cast<int>(active_skills.size() %
                                                        kSkillsPerHistoryRecord) -
                                   1
                             : first + kSkillsPerHistoryRecord - 1;
        const int item_count = last - first + 1;
        if (last - first == -1) {
            break;
        }

        char history[1024];
        char header[24];
        char skill_text[512];
        std::memset(history, 0, sizeof(history));
        std::memcpy(history, "SaveLethe", 10);
        std::strcat(history, reason_text);
        std::sprintf(header, "%u/%d/%d", charac_job, item_count, skill_tree);
        std::strcat(history, header);
        std::memset(skill_text, 0, sizeof(skill_text));

        for (int index = 0; index < item_count; ++index) {
            const std::pair<int, int>& skill = active_skills[first + index];
            char entry[16];
            std::memset(entry, 0, sizeof(entry));
            if (index % 3) {
                std::sprintf(entry, "/%d/%d", skill.first, skill.second);
            } else {
                std::sprintf(entry, ",%d/%d", skill.first, skill.second);
            }
            std::strcat(skill_text, entry);
        }
        std::strcat(history, skill_text);
        owner_->LogHistory(history);
    }
}

// IDA_EA: 0x8686188
// IDA_NAME: _ZN15cUserHistoryLog15SkillSPInfoSaveEP5CUser22eSkillSPInfoSaveReason
// DEMANGLED: cUserHistoryLog::SkillSPInfoSave(CUser *,eSkillSPInfoSaveReason)
void cUserHistoryLog::SkillSPInfoSave(CUser* user,
                                      eSkillSPInfoSaveReason reason) {
    WongWork::CSkillChanger skill_changer;
    const int total_sp = skill_changer.GetCurCharacTotalSP(user, false);
    char history[128];
    char second_history[128];
    std::memset(history, 0, sizeof(history));
    std::memset(second_history, 0, sizeof(second_history));

    if (user && user->GetCurCharacSkillTreeIndex() == -1) {
        const int remain_sp = user->getCurCharacSkillR()->get_remain_sp_at_index(
            ENUM_SKILL_TREE_0);
        if (total_sp >= 0 && remain_sp >= 0) {
            std::sprintf(history,
                         "SkillSPInfo,%d,%d,%d,%d",
                         static_cast<int>(reason),
                         remain_sp,
                         total_sp,
                         -1);
            user->LogHistory(history);
        }
        return;
    }

    const int first_remain_sp =
        user->getCurCharacSkillR()->get_remain_sp_at_index(ENUM_SKILL_TREE_0);
    const int second_remain_sp =
        user->getCurCharacSkillR()->get_remain_sp_at_index(ENUM_SKILL_TREE_1);
    if (total_sp >= 0 && first_remain_sp >= 0 && !second_remain_sp) {
        std::sprintf(history,
                     "SkillSPInfo,%d,%d,%d,%d",
                     static_cast<int>(reason),
                     first_remain_sp,
                     total_sp,
                     0);
        user->LogHistory(history);
        std::sprintf(second_history,
                     "SkillSPInfo,%d,%d,%d,%d",
                     static_cast<int>(reason),
                     second_remain_sp,
                     total_sp,
                     1);
        user->LogHistory(second_history);
    }
}
