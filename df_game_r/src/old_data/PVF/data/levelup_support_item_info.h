#ifndef OLDSRV_OLD_DATA_PVF_DATA_LEVELUP_SUPPORT_ITEM_INFO_H_
#define OLDSRV_OLD_DATA_PVF_DATA_LEVELUP_SUPPORT_ITEM_INFO_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>


// IDA_EA: 0x8AB5A5E, 0x8AB75E4.
// IDA unit: 12_pvf_datamanager_scripts/LevelupSupport2ndScript_ItemInfo.
class LevelupSupport2ndScript {
public:
    struct ItemInfo {
        // The stored count is a 16-bit field, but the exported i386 C2 ABI
        // takes two `int` arguments (_ZN23LevelupSupport2ndScript8ItemInfoC2Eii).
        ItemInfo(int32_t item_index, int item_count);
        ~ItemInfo();

        int32_t index;
        int16_t count;
    };

    LevelupSupport2ndScript();
    ~LevelupSupport2ndScript();

    int loadScript();
    int clearScript();

    int32_t job_type;
    std::map<int, std::vector<ItemInfo>* > reward_items_by_level;
    std::string mail_title;
    std::string mail_content;
};

#endif  // OLDSRV_OLD_DATA_PVF_DATA_LEVELUP_SUPPORT_ITEM_INFO_H_
