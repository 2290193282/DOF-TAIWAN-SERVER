#ifndef OLDSRV_GLOBAL_ITEM_GENERATOR_MGR_ABI_H_
#define OLDSRV_GLOBAL_ITEM_GENERATOR_MGR_ABI_H_

#include <stdint.h>

#include "old_data/PVF/data/item_generator_types.h"

namespace WongWork {

class CItemGeneratorMgr {
public:
    // IDA names this nested enum in getGenerator/onTryGenerateRandomItem.
    // The values are the eight fixed slots initialized at 0x8533BC8.
    enum eGenerateType_t {
        kMonsterDrop = 0,
        kObjectDrop = 1,
        kClearReward = 2,
        kWorldDrop = 3,
        kDungeonDrop = 4,
        kPcRoomWorldDrop = 5,
        kPcRoom2WorldDrop = 6,
        kReservedWorldDrop = 7
    };

    // IDA_EA: 0x8374EF4 / 0x8374F0C
    CItemGeneratorMgr();
    ~CItemGeneratorMgr();

    // IDA_EA: 0x814A7D0 / 0x8533BC8
    CItemGenerator* getGenerator(eGenerateType_t type);
    int initialize();

    // IDA_EA: 0x8534278
    // IDA_NAME: _ZN8WongWork17CItemGeneratorMgr12setDropRatioEf
    bool setDropRatio(float rate);

    // IDA_EA: 0x85342C6 / 0x8534322
    int onTryGenerateRandomItem(eGenerateType_t generator_type,
                                int drop_type,
                                int rarity);
    void onTimerEvent(int timer_id);

private:
    // 0x000..0x3bf: 6 drop types * 8 generator kinds * 5 rarities.
    uint32_t generate_statistics_[240];
    // 0x3c0..0x3df: IDA slots 240..247.
    CItemGenerator* generators_[8];
    // 0x3e0: constructed by the original C2 and released by D2.
    CGeneratorCommonData common_data_;
};

#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(CItemGeneratorMgr) == 0xA28u,
              "CItemGeneratorMgr ABI");
#endif

}  // namespace WongWork

#endif  // OLDSRV_GLOBAL_ITEM_GENERATOR_MGR_ABI_H_
