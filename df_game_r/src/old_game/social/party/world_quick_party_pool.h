#ifndef OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_QUICK_PARTY_POOL_H_
#define OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_QUICK_PARTY_POOL_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x82344CA in the companion source.


#include <stdint.h>

#include <list>

namespace QuickParty {

// IDA unit: 05_world_enter_village/QuickParty/STQuickPartyPoolMap_Key.
struct STQuickPartyPoolMap_Key {
    STQuickPartyPoolMap_Key();

    bool operator<(const STQuickPartyPoolMap_Key& rhs) const;

    uint16_t pool_id;
    uint8_t channel_type;
    uint8_t pad_03;
};

// IDA unit: 05_world_enter_village/QuickParty/STQuickPartyPoolMap_Data.
struct STQuickPartyPoolMap_Data {
    STQuickPartyPoolMap_Data();
    STQuickPartyPoolMap_Data(const STQuickPartyPoolMap_Data& rhs);
    ~STQuickPartyPoolMap_Data();

    std::list<int> first_list;
    std::list<int> second_list;
    std::list<int> third_list;
    std::list<int> fourth_list;
};

}  // namespace QuickParty

#endif  // OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_QUICK_PARTY_POOL_H_
