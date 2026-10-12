#ifndef OLDSRV_OLD_DATA_PVF_DATA_ARAD_ONE_PLUS_CARD_EVENT_ITEM_LIST_H_
#define OLDSRV_OLD_DATA_PVF_DATA_ARAD_ONE_PLUS_CARD_EVENT_ITEM_LIST_H_

#include "old_data/PVF/common/arad_script_loader.h"

#include <stdint.h>

#include <vector>

class Arad_OnePlusCardEvent_ItemList : public Arad_ScriptLoader {
public:
    struct Data {
        Data(uint16_t web_id_value, unsigned long item_index_value);
        ~Data();

        bool operator==(const Data& rhs);

        uint16_t web_id;
        unsigned long item_index;
    };

    Arad_OnePlusCardEvent_ItemList();
    virtual ~Arad_OnePlusCardEvent_ItemList();

    virtual int parsing(char* buffer);
    int find_item(uint16_t web_id_value, Data& out);
    int find_item(uint16_t web_id_value);
    bool is_limited_map(unsigned int map_index);

private:
    std::vector<Data> item_list_;
    std::vector<unsigned int> limited_maps_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Arad_OnePlusCardEvent_ItemList::Data) == 0x08u,
              "Arad_OnePlusCardEvent_ItemList::Data must stay 0x08");
static_assert(sizeof(Arad_OnePlusCardEvent_ItemList) == 0x1Cu,
              "Arad_OnePlusCardEvent_ItemList must stay 0x1C");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_DATA_ARAD_ONE_PLUS_CARD_EVENT_ITEM_LIST_H_
