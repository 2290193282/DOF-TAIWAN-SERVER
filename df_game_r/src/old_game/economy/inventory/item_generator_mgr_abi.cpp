#include "old_game/economy/inventory/item_generator_mgr_abi.h"

namespace WongWork {

// IDA_EA: 0x8534278
// IDA_NAME: _ZN8WongWork17CItemGeneratorMgr12setDropRatioEf
// DEMANGLED: WongWork::CItemGeneratorMgr::setDropRatio(float)
bool CItemGeneratorMgr::setDropRatio(float rate) {
    bool result = false;
    for (int index = 0; ; ++index) {
        result = index <= 7;
        if (index > 7) {
            break;
        }
        getGenerator(static_cast<eGenerateType_t>(index))->setDropRatio(rate);
    }
    return result;
}

}  // namespace WongWork
