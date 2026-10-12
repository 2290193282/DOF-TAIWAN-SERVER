#include "old_data/PVF/data/premium_item_data.h"

namespace premium_script_parsing {

typedef char STPremiumItemData_Size_Must_Be_0x08[
    (sizeof(STPremiumItemData) == 0x08) ? 1 : -1];

// IDA_EA: 0x891AE4C
// IDA_NAME: _ZN22premium_script_parsing17STPremiumItemDataC2Ev
// DEMANGLED: premium_script_parsing::STPremiumItemData::STPremiumItemData(void)
STPremiumItemData::STPremiumItemData()
    : flag_00(0), flag_01(0), padding_02(), item_id(0) {}

// IDA_EA: 0x893A00A
// IDA_NAME: _ZN22premium_script_parsing17STPremiumItemDataD2Ev
// DEMANGLED: premium_script_parsing::STPremiumItemData::~STPremiumItemData()
STPremiumItemData::~STPremiumItemData() {}

}  // namespace premium_script_parsing
