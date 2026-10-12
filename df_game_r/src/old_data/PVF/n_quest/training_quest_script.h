#ifndef OLDSRV_DATAMANAGER_TRAINING_QUEST_SCRIPT_H_
#define OLDSRV_DATAMANAGER_TRAINING_QUEST_SCRIPT_H_

#include <stddef.h>

#include <set>
#include <utility>
#include <vector>

#include "common/config.h"

// IDA unit: 99_misc_global/MyRandom.  This is the exact deterministic
// functor supplied to TrainingQuestScript's random_shuffle call.
class MyRandom {
public:
    explicit MyRandom(int seed);
    int operator()(int count);

private:
    int seed_;
};

// IDA unit: 08_quest_skill_growth/ST_TrainingQuest_Select_Prob.
struct ST_TrainingQuest_Select_Prob {
    ST_TrainingQuest_Select_Prob();
    ~ST_TrainingQuest_Select_Prob();

    int select_prob_count;
    std::vector<std::pair<int, int> > select_probs;
    std::vector<int> rand_values[366];
};

// IDA unit: 08_quest_skill_growth/TrainingQuestScript.
struct TrainingQuestScript : public ST_TrainingQuest_Select_Prob {
    TrainingQuestScript();
    ~TrainingQuestScript();

    int clear();
    TrainingQuestScript* reset_same_apply_level_cnt();
    int getApplyLevel(std::multiset<int>* apply_levels);
    int getTrainingQuest(int charac_level,
                         int apply_level,
                         int prev_apply_level);
    int computeDayIndex();
    int get_random_fake_value(int index, int day_index);
    bool suffleTrainingQuests();

    std::vector<int>
        level_quests[oldsrv::config::kTrainingQuestLevelCount];
    std::vector<int> special_level_quests;
    int same_apply_level_cnt;
};

static_assert(sizeof(MyRandom) == 0x04u,
              "MyRandom IDA ABI size must stay 0x04");
static_assert(sizeof(ST_TrainingQuest_Select_Prob) == 0x1138u,
              "ST_TrainingQuest_Select_Prob IDA ABI size must stay 0x1138");
static_assert(
    sizeof(TrainingQuestScript) ==
        sizeof(ST_TrainingQuest_Select_Prob) +
            sizeof(std::vector<int>) *
                oldsrv::config::kTrainingQuestLevelCount +
            sizeof(std::vector<int>) + sizeof(int),
    "TrainingQuestScript configured ABI size drift");
static_assert(offsetof(ST_TrainingQuest_Select_Prob, select_prob_count) == 0x00u &&
                  offsetof(ST_TrainingQuest_Select_Prob, select_probs) == 0x04u &&
                  offsetof(ST_TrainingQuest_Select_Prob, rand_values) == 0x10u,
              "ST_TrainingQuest_Select_Prob IDA member offsets must stay exact");

int importTrainingQuestScript(TrainingQuestScript* script,
                              const char* filename);

#endif  // OLDSRV_DATAMANAGER_TRAINING_QUEST_SCRIPT_H_
