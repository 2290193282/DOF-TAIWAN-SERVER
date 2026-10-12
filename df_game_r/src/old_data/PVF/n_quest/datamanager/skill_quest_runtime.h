#ifndef OLDSRV_DATAMANAGER_SKILL_QUEST_RUNTIME_H_
#define OLDSRV_DATAMANAGER_SKILL_QUEST_RUNTIME_H_

#include <stdint.h>

#include <ext/hash_map>
#include <list>
#include <map>
#include <utility>
#include <vector>

#include "common/config.h"

class CSkill;
class Quest;
enum ENUM_QUEST_GRADE : int;
struct stSelectQuestParam;

namespace WongWork {
struct CQuestClear;
}

class CSkillList {
public:
    CSkillList();
    ~CSkillList();

    bool destroy();
    int init();
    int insert_skill(int job_type, CSkill* skill);
    CSkill* find_skill(int job_type, int skill_index);
    int get_ran_groupskill_index(int job_type,
                                 int group,
                                 int exclude_skill_index);
    void set_post_learning_skill(int job_type, CSkill* skill);
    int get_post_learning_skill(
        int job_type,
        int skill_index,
        std::vector<std::pair<int, int> >& skills);
    __gnu_cxx::hash_map<int, CSkill*>* getSkillList(int job_type);

private:
    __gnu_cxx::hash_map<int, CSkill*>
        skill_by_index_[oldsrv::config::kCharacterJobCount];
    std::multimap<int, CSkill*>
        skill_by_group_[oldsrv::config::kCharacterJobCount];
    std::map<int, std::vector<std::pair<int, int> > >
        post_learning_skill_[oldsrv::config::kCharacterJobCount];
};

class QuestList {
public:
    QuestList();
    ~QuestList();

    int destroy();
    int load_list();
    char insert_NPC(int npc_id);
    Quest* find_quest(int quest_index);
    int check_ahead_quest(int quest_index,
                          const WongWork::CQuestClear& cleared_quests);
    int check_anti_quest(int quest_index,
                         const WongWork::CQuestClear& cleared_quests);
    int check_clear(int quest_index,
                    const WongWork::CQuestClear& cleared_quests);
    // IDA_EA: 0x8355304
    // IDA_NAME:
    // _ZN9QuestList17select_mail_questERSt4listIiSaIiEERKN8WongWork11CQuestClearERK18stSelectQuestParam
    // DEMANGLED: QuestList::select_mail_quest(
    //             std::list<int>&,WongWork::CQuestClear const&,
    //             stSelectQuestParam const&)
    bool select_mail_quest(
        std::list<int>& selected_quests,
        const WongWork::CQuestClear& cleared_quests,
        const stSelectQuestParam& parameter);
    int Check_Quest(Quest* quest,
                    stSelectQuestParam& parameter,
                    const WongWork::CQuestClear& cleared_quests);
    bool allowable_questlist_as_npc(
        std::list<int>& selected_quests,
        const std::multimap<int, int>& quest_npcs,
        stSelectQuestParam& parameter,
        const WongWork::CQuestClear& cleared_quests,
        bool repeatable_quest);
    // IDA_EA: 0x8355726.
    // IDA_NAME:
    // _ZN9QuestList12select_questERSt4listIiSaIiEER18stSelectQuestParamRKN8WongWork11CQuestClearE
    // The list is owned by this QuestList; no temporary or replacement quest
    // table is created here.
    bool select_quest(std::list<int>& selected_quests,
                      stSelectQuestParam& parameter,
                      const WongWork::CQuestClear& cleared_quests);
    // IDA_EA: 0x835589E
    // IDA_NAME: _ZN9QuestList17GetDailyQuestListERSt4listIiSaIiEE
    // DEMANGLED: QuestList::GetDailyQuestList(std::list<int>&)
    bool GetDailyQuestList(std::list<int>& daily_quests);
    // IDA_EA: 0x8355A10
    // IDA_NAME: _ZN9QuestList20GetTrainingQuestListERSt6vectorIiSaIiEEs
    // DEMANGLED: QuestList::GetTrainingQuestList(std::vector<int>&,short)
    std::vector<int>* GetTrainingQuestList(
        std::vector<int>& training_quests,
        int16_t charac_level);
    std::multimap<int, int>* getQuestNPCList(ENUM_QUEST_GRADE grade);
    __gnu_cxx::hash_map<int, Quest*>* getQuestList();

private:
    __gnu_cxx::hash_map<int, Quest*> quest_by_index_;
    std::multimap<int, int> selector_0_;
    std::multimap<int, int> selector_1_;
    std::multimap<int, int> selector_2_;
    std::multimap<int, int> selector_3_;
    std::multimap<int, int> selector_4_;
    std::multimap<int, int> selector_5_;
    std::multimap<int, int> selector_6_;
    std::multimap<int, int> selector_7_;
    std::vector<int> mail_quest_ids_;
    std::list<int> quest_npc_ids_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CSkillList) == 0x330u,
              "CSkillList 0828 ABI size must stay 0x330");
static_assert(sizeof(QuestList) == 0xE8u,
              "QuestList old ABI size must stay 0xE8");
#endif

#endif  // OLDSRV_DATAMANAGER_SKILL_QUEST_RUNTIME_H_
