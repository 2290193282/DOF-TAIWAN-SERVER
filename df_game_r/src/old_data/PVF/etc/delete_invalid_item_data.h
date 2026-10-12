#ifndef OLDSRV_DATAMANAGER_DELETE_INVALID_ITEM_DATA_H_
#define OLDSRV_DATAMANAGER_DELETE_INVALID_ITEM_DATA_H_

#include <stdint.h>

#include <map>
#include <string>


// IDA_EA: 0x8AC1D40, 0x8AC1DCE, 0x8AC1E60, 0x8AC2066, 0x8AC2096, 0x8AC20F8, 0x8AC2180, 0x8AC220C, 0x8AC2220.
// IDA unit: 12_pvf_datamanager_scripts/DeleteInvalidItemScript_DeleteInvalidItemData.
class DeleteInvalidItemScript {
public:
    struct DeleteInvalidItemData {
        DeleteInvalidItemData();

        DeleteInvalidItemData* init();

        int32_t value_00;
        int32_t value_04;
        int32_t value_08;
    };

    DeleteInvalidItemScript();
    ~DeleteInvalidItemScript();

    int loadScript();
    int clearScript();
    int IsInvalidItem2Delete(int32_t item_id);
    int GetRewardItemInfo(int32_t item_id,
                          int32_t& reward_item_id,
                          int32_t& reward_count);
    std::string* GetMailInfo(std::string& title,
                             std::string& content) const;

    std::string mail_title;
    std::string mail_content;
    std::map<int, DeleteInvalidItemData> invalid_items;
};

#endif  // OLDSRV_DATAMANAGER_DELETE_INVALID_ITEM_DATA_H_
