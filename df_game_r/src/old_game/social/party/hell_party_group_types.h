#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_HELL_PARTY_GROUP_TYPES_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_HELL_PARTY_GROUP_TYPES_H_

#include <stdint.h>

#include <list>
#include <map>

// IDA unit: 12_pvf_datamanager_scripts/RDARScriptMap.
// These values are consumed by both the PVF owner and CBattle_Field's
// map-assembly methods.  Keeping them here avoids a map_runtime/battle_field
// include cycle while retaining one complete, typed ABI definition.
struct STHellPartyGroupRate {
    int32_t group_index;
    int32_t rate;
    uint8_t group_order;
    uint8_t reserved_09[3];
};

struct STHellPartyGroupOrder {
    // IDA_EA: 0x89E8712 / 0x89E9DFC / 0x89E8744
    // IDA_NAME: _ZN21STHellPartyGroupOrderC2Ev /
    //           _ZN21STHellPartyGroupOrderC2ERKS_ /
    //           _ZN21STHellPartyGroupOrderD2Ev
    STHellPartyGroupOrder();
    STHellPartyGroupOrder(const STHellPartyGroupOrder& rhs);
    ~STHellPartyGroupOrder();

    uint16_t rates_by_group_order[6];
    std::list<STHellPartyGroupRate> group_rates;
};

typedef std::map<uint16_t, STHellPartyGroupOrder> HellPartyGroupOrder;
typedef std::map<int, HellPartyGroupOrder> HellPartyGroupOrderMapPool;

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_HELL_PARTY_GROUP_TYPES_H_
