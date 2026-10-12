#ifndef OLDSRV_OLD_DATA_PVF_ITEMSHOP_QUEST_SHOP_STATUS_H_
#define OLDSRV_OLD_DATA_PVF_ITEMSHOP_QUEST_SHOP_STATUS_H_

#include <stdint.h>

#include <string>
#include <vector>

struct stStatusLevelInfo {
    stStatusLevelInfo();
    ~stStatusLevelInfo();

    uint8_t level;
    uint8_t count;
};

struct stQuestShopStatus {
    stQuestShopStatus();
    stQuestShopStatus(const stQuestShopStatus& other);
    ~stQuestShopStatus();

    int clear();
    int getMaxCount(int charac_level);
    int getNextLevel(int count);

    std::string name;
    std::string name2;
    std::string explain;
    std::string icon;
    int32_t icon_frame;
    uint8_t quest_point;
    uint8_t pad_15[3];
    int32_t appendage_type;
    int32_t value_type;
    float value;
    std::vector<stStatusLevelInfo> level_infos;
};

int loadAppendageType(const std::string& value);
int loadValueType(const std::string& value);

static_assert(sizeof(stStatusLevelInfo) == 0x02u,
              "stStatusLevelInfo IDA ABI size must stay 0x02");
static_assert(sizeof(stQuestShopStatus) == 0x30u,
              "stQuestShopStatus IDA ABI size must stay 0x30");

#endif  // OLDSRV_OLD_DATA_PVF_ITEMSHOP_QUEST_SHOP_STATUS_H_
