#ifndef OLDSRV_DATAMANAGER_ARAD_VILLAGE_ATTACK_REWARD_H_
#define OLDSRV_DATAMANAGER_ARAD_VILLAGE_ATTACK_REWARD_H_

#include "old_data/PVF/common/arad_script_loader.h"

#include <vector>

// IDA unit: 05_world_enter_village/Arad_VillageAttackRewardItem.  It parses
// RDAR configuration and is embedded in ARAD::Arad_DataManager, so the data
// owner is B even though world code consumes its queries.
class Arad_VillageAttackRewardItem : public Arad_ScriptLoader {
public:
    struct Data {
        Data();
        Data(int win_point_value,
             unsigned long item_index_value,
             int item_count_value);
        ~Data();

        bool operator==(const Data& rhs);

        int win_point;
        unsigned long item_index;
        int item_count;
    };

    struct RankData {
        RankData();
        ~RankData();

        unsigned long item_index;
        unsigned long item_index2;
        int item_count;
        int item_count2;
    };

    Arad_VillageAttackRewardItem();
    virtual ~Arad_VillageAttackRewardItem();

    virtual int parsing(char* buffer);
    int find_reward_item(int win_point, Data& out_data);
    int find_ranking_reward_item(short rank_index, RankData& out_data);

private:
    std::vector<Data> reward_items_;
    std::vector<RankData> ranking_rewards_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Arad_VillageAttackRewardItem::Data) == 0x0Cu,
              "Arad_VillageAttackRewardItem::Data must stay 0x0C");
static_assert(sizeof(Arad_VillageAttackRewardItem::RankData) == 0x10u,
              "Arad_VillageAttackRewardItem::RankData must stay 0x10");
static_assert(sizeof(Arad_VillageAttackRewardItem) == 0x1Cu,
              "Arad_VillageAttackRewardItem must stay 0x1C");
#endif

#endif  // OLDSRV_DATAMANAGER_ARAD_VILLAGE_ATTACK_REWARD_H_
