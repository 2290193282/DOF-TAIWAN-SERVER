#ifndef OLDSRV_OLD_DATA_PVF_EVENT_LEVEL_REWARD_SCRIPTS_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_LEVEL_REWARD_SCRIPTS_H_

#include <stdint.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "common/types/stackable_value_types.h"

// IDA unit: 12_pvf_datamanager_scripts/LevelUpRewardItemScript.
struct LevelUpRewardItemData {
    // IDA_EA: 0x81B88BC
    LevelUpRewardItemData();
    // IDA_EA: 0x81B967E
    ~LevelUpRewardItemData();
    // IDA_EA: 0x81B88D0
    LevelUpRewardItemData* clear();

    int32_t item_id;
    int32_t count;
};

struct LevelUpRewardItemTableKey {
    // IDA_EA: 0x891BAC0
    LevelUpRewardItemTableKey();
    // IDA_EA: 0x81B8896
    LevelUpRewardItemTableKey(int charac_level, int charac_job,
                              int first_grow_type);
    // IDA_EA: 0x81B8830
    bool operator<(const LevelUpRewardItemTableKey& rhs) const;
    // IDA_EA: 0x891BAD4
    LevelUpRewardItemTableKey* clear();

    uint16_t charac_level;
    uint8_t charac_job;
    uint8_t first_grow_type;
};

struct stLevelUpRewardItemScript {
    struct GetGrowUpRewardItemFunctor {
        // IDA_EA: 0x81B88E8
        GetGrowUpRewardItemFunctor(
            const LevelUpRewardItemTableKey& key_value,
            std::vector<LevelUpRewardItemData>& out_value);
        // IDA_EA: 0x81B88FE
        void operator()(
            const std::pair<const LevelUpRewardItemTableKey,
                            LevelUpRewardItemData>& entry) const;

        const LevelUpRewardItemTableKey* key;
        std::vector<LevelUpRewardItemData>* out;
    };

    // IDA_EA: 0x8371194
    stLevelUpRewardItemScript();
    // IDA_EA: 0x8371530
    ~stLevelUpRewardItemScript();
    // IDA_EA: 0x81B895A
    int get_data_when_growup(const LevelUpRewardItemTableKey& key,
                             std::vector<LevelUpRewardItemData>& out);
    // IDA_EA: 0x81B89DE
    int get_data(const LevelUpRewardItemTableKey& key,
                 LevelUpRewardItemData& out);
    // IDA_EA: 0x83711D6
    void clear();

    std::map<LevelUpRewardItemTableKey, LevelUpRewardItemData> reward_items;
};

// IDA unit: 12_pvf_datamanager_scripts/NewAccountLevelUpToJobScript.
struct stStepRewardStackableItemScript {
    // IDA_EA: 0x83714C6
    stStepRewardStackableItemScript();
    // IDA_EA: 0x837151C
    ~stStepRewardStackableItemScript();
    // IDA_EA: 0x8371508
    void clear();
    // IDA_EA: 0x891BD0E
    void set_data(int step_index, StackableItemData& item_data);

    std::map<int, std::vector<StackableItemData> > reward_items_by_step;
};

struct stGrowthCapsuleScript {
    // IDA_EA: 0x837114A
    stGrowthCapsuleScript();

    int32_t max_gage_exp;
    int32_t gage_percent;
    int32_t capsule_id;
    int32_t capsule_exp;
    int32_t capsule_limit_lv;
};

struct stReturnUserRewardScript {
    // IDA_EA: 0x8371544
    stReturnUserRewardScript();
    // IDA_EA: 0x837DDE6
    ~stReturnUserRewardScript();

    uint32_t first_dword;
    uint32_t second_dword;
    stLevelUpRewardItemScript level_up_rewards;
};

struct stNewAccountLevelUpToJobScript {
    // IDA_EA: 0x837DED8
    stNewAccountLevelUpToJobScript();
    // IDA_EA: 0x837DF44
    ~stNewAccountLevelUpToJobScript();

    stStepRewardStackableItemScript stackable_rewards;
    stLevelUpRewardItemScript equip_rewards;
    stLevelUpRewardItemScript avatar_rewards;
};

// IDA_EA: 0x89107E9
// IDA_NAME: _Z29ImportLevelUpRewardItemScriptPKcR25stLevelUpRewardItemScript
int ImportLevelUpRewardItemScript(const char* filename,
                                  stLevelUpRewardItemScript& script);
// IDA_EA: 0x891120C
// IDA_NAME: _Z25ImportGrowthCapsuleScriptPKcR21stGrowthCapsuleScript
int ImportGrowthCapsuleScript(const char* filename,
                              stGrowthCapsuleScript& script);
// IDA_EA: 0x89179EB
// IDA_NAME: _Z35ImportNewAccountLeventUpToJobScriptPKcR30stNewAccountLevelUpToJobScript
int ImportNewAccountLeventUpToJobScript(
    const char* filename,
    stNewAccountLevelUpToJobScript& script);
// IDA_EA: 0x8917C99
// IDA_NAME: _Z22LoadRewardDataByNoTypeR31stStepRewardStackableItemScriptSs
int LoadRewardDataByNoType(stStepRewardStackableItemScript& script,
                           std::string end_tag);
// IDA_EA: 0x8917DB3
// IDA_NAME: _Z22deleteTagBracketStringRSs
void deleteTagBracketString(std::string& text);
// IDA_EA: 0x8917F44
// IDA_NAME: _Z24LoadRewardDataByGrowTypeR25stLevelUpRewardItemScriptSs
int LoadRewardDataByGrowType(stLevelUpRewardItemScript& script,
                             std::string end_tag);
// IDA_EA: 0x89180EA
// IDA_NAME: _Z28ImportReturnUserRewardScriptPKcR24stReturnUserRewardScript
int ImportReturnUserRewardScript(const char* filename,
                                 stReturnUserRewardScript& script);

static_assert(sizeof(LevelUpRewardItemData) == 0x08u,
              "LevelUpRewardItemData IDA ABI size must stay 0x08");
static_assert(sizeof(LevelUpRewardItemTableKey) == 0x04u,
              "LevelUpRewardItemTableKey IDA ABI size must stay 0x04");
static_assert(sizeof(stLevelUpRewardItemScript::GetGrowUpRewardItemFunctor) ==
                  0x08u,
              "GetGrowUpRewardItemFunctor IDA ABI size must stay 0x08");
static_assert(sizeof(stLevelUpRewardItemScript) == 0x18u,
              "stLevelUpRewardItemScript IDA ABI size must stay 0x18");
static_assert(sizeof(stStepRewardStackableItemScript) == 0x18u,
              "stStepRewardStackableItemScript IDA ABI size must stay 0x18");
static_assert(sizeof(stGrowthCapsuleScript) == 0x14u,
              "stGrowthCapsuleScript IDA ABI size must stay 0x14");
static_assert(sizeof(stReturnUserRewardScript) == 0x20u,
              "stReturnUserRewardScript IDA ABI size must stay 0x20");
static_assert(sizeof(stNewAccountLevelUpToJobScript) == 0x48u,
              "stNewAccountLevelUpToJobScript IDA ABI size must stay 0x48");

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_LEVEL_REWARD_SCRIPTS_H_
