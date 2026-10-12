#ifndef OLDSRV_GLOBAL_VILLAGE_ATTACKED_ABI_H_
#define OLDSRV_GLOBAL_VILLAGE_ATTACKED_ABI_H_

#include <stdint.h>

#include <vector>

// IDA unit: 05_world_enter_village/stAttackedMapInfo_t.
struct stAttackedMapInfo_t {
    ~stAttackedMapInfo_t();

    uint32_t village;
    uint32_t area;
    uint32_t monster_group;
    uint32_t reward_group;
};

// Typed A/public declaration for the C-owned village-attacked value.
// IDA_EA: 0x837CF1E
// IDA_NAME: _ZN19stVillageAttacked_tC2Ev
// IDA_EA: 0x837CF32
// IDA_NAME: _ZN19stVillageAttacked_tD2Ev
// CDataManager embeds this object at old offset +0x61A0.
struct stVillageAttacked_t : public std::vector<stAttackedMapInfo_t> {
    stVillageAttacked_t();
    ~stVillageAttacked_t();
};

#if defined(__i386__)
static_assert(sizeof(stAttackedMapInfo_t) == 0x10u,
              "village-attacked map row ABI size drift");
static_assert(sizeof(stVillageAttacked_t) == 0x0cu,
              "village-attacked vector ABI size drift");
#endif


#endif  // OLDSRV_GLOBAL_VILLAGE_ATTACKED_ABI_H_
