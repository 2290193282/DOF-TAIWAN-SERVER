#ifndef OLDSRV_OLD_DATA_PVF_EVENT_EVENT_REWARD_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_EVENT_REWARD_SCRIPT_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "old_game/economy/inventory/slot_bound_checker.h"
#include "old_game/activity/reward/reward_key.h"


class CItem;
class CUser;


// IDA_EA: 0x80F1134, 0x80F1162, 0x80F32FA, 0x80EF0C0, 0x80EF128, 0x80EF190, 0x80EF25C, 0x80EF29C, 0x80EF3A2, 0x83738A2, 0x837DFB2.
// IDA unit: 99_misc_global/itemData, consumed by the B-owned EventReward
// DataManager/PVF table importer.
struct itemData {
    itemData();
    ~itemData();

    itemData* clear();

    uint8_t upgrade;
    uint8_t seal;
    uint8_t random_option_seal;
    int32_t item_index;
    int32_t count_or_period;
};

// IDA unit: 11_event_runtime/eventReward, restored here only for the
// DataManager EventReward table import path.
class eventReward {
public:
    typedef std::map<int, std::vector<itemData> > rewardItemMap;
    typedef std::map<rewardKey, rewardItemMap> rewardTable;

    eventReward();
    ~eventReward();

    int getSubKeyType(std::string& text);
    int getSubKeyAction(std::string& text);
    int getOptionType(std::string& text, itemData& data);
    itemData* defaultOption(char sub_key_action, itemData& data);
    int importRewardTable(const char* dir, const char* filename);
    int ImportRewardScript(const char* dir, const char* filename);
    int StartEndPos(Inven_Item::ITEM_TYPE item_type,
                    int& start,
                    int& end,
                    int capacity) const;

    bool createCharacInvenReward(Inven_Item* items,
                                 rewardKey key,
                                 int reward_value);
    bool createCharacEquipReward(Inven_Item* items,
                                 rewardKey key,
                                 int reward_value);
    int rewardWriteUserInven(Inven_Item* items,
                             rewardKey key,
                             int reward_value);
    int rewardWriteUserEquip(Inven_Item* items,
                             rewardKey key,
                             int reward_value);
    int sendPostalReward(CUser& user,
                         rewardKey key,
                         int reward_value,
                         const char* title_string_key,
                         const char* body_string_key);
    void applyItemOption(Inven_Item& inventory_item,
                         const CItem* item,
                         const itemData& data);
    bool unsealRandomOption(const CItem* item, Inven_Item& inventory_item);

    rewardTable reward_table;
    CSlotBoundChecker slot_bound_checker;

private:
    eventReward(const eventReward&);
    eventReward& operator=(const eventReward&);
};

static_assert(sizeof(itemData) == 0x0Cu,
              "itemData IDA ABI size must stay 0x0c");
static_assert(sizeof(eventReward) == 0x30u,
              "eventReward IDA ABI size must stay 0x30");

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_EVENT_REWARD_SCRIPT_H_
