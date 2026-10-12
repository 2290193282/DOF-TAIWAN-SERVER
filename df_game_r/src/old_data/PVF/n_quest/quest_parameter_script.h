#ifndef OLDSRV_OLD_DATA_PVF_N_QUEST_QUEST_PARAMETER_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_N_QUEST_QUEST_PARAMETER_SCRIPT_H_

#include <stddef.h>

#include <map>
#include <vector>

// IDA unit: 08_quest_skill_growth/pieceQuestRewardStatus.
// These values are loaded from n_Quest/questParameter.etc and consumed by
// quest gameplay; the record and its RDAR importer remain B-owned data.
struct pieceQuestRewardStatus {
    pieceQuestRewardStatus();
    ~pieceQuestRewardStatus();

    int reward_type;
    int reward_count;
    float reward_value;
};

// IDA unit: 08_quest_skill_growth/QuestParameterScript.
// IDA_EA: clear/ctor/dtor/getters/import @0x836F130..0x8A6BE66.
struct QuestParameterScript {
    QuestParameterScript();
    ~QuestParameterScript();

    QuestParameterScript* clear();
    int getRewardWeight(char difficulty);
    int getRewardExp(int charac_level);
    int getRewardGold(int charac_level);
    int getRewardLevel(int charac_level);
    std::vector<pieceQuestRewardStatus> getRewardStatus(int charac_level);

    std::map<char, int> difficulty_weights;
    int green_level_penalty;
    int grey_level_penalty;
    int bal_kun_penalty;
    std::vector<int> exp_reward_table;
    std::vector<int> gold_reward_table;
    std::map<int, int> piece_reward_levels;
    std::map<int, std::vector<pieceQuestRewardStatus> > piece_reward_status;
};

static_assert(sizeof(pieceQuestRewardStatus) == 0x0cu,
              "pieceQuestRewardStatus IDA ABI size must stay 0x0c");
static_assert(offsetof(pieceQuestRewardStatus, reward_type) == 0x00u &&
                  offsetof(pieceQuestRewardStatus, reward_count) == 0x04u &&
                  offsetof(pieceQuestRewardStatus, reward_value) == 0x08u,
              "pieceQuestRewardStatus IDA member offsets must stay exact");
static_assert(sizeof(QuestParameterScript) == 0x6cu,
              "QuestParameterScript IDA ABI size must stay 0x6c");
static_assert(offsetof(QuestParameterScript, difficulty_weights) == 0x00u &&
                  offsetof(QuestParameterScript, green_level_penalty) == 0x18u &&
                  offsetof(QuestParameterScript, grey_level_penalty) == 0x1cu &&
                  offsetof(QuestParameterScript, bal_kun_penalty) == 0x20u &&
                  offsetof(QuestParameterScript, exp_reward_table) == 0x24u &&
                  offsetof(QuestParameterScript, gold_reward_table) == 0x30u &&
                  offsetof(QuestParameterScript, piece_reward_levels) == 0x3cu &&
                  offsetof(QuestParameterScript, piece_reward_status) == 0x54u,
              "QuestParameterScript IDA member offsets must stay exact");

int importQuestParameterScript(QuestParameterScript* script,
                               const char* filename);

#endif  // OLDSRV_OLD_DATA_PVF_N_QUEST_QUEST_PARAMETER_SCRIPT_H_
