#include "old_data/PVF/data/levelup_support_item_info.h"


typedef char LevelupSupportItemInfo_Size_Must_Be_0x08[
    (sizeof(LevelupSupport2ndScript::ItemInfo) == 0x08) ? 1 : -1];

// IDA_EA: 0x8AB5A5E
// IDA_NAME: _ZN23LevelupSupport2ndScript8ItemInfoC2Eii
// DEMANGLED: LevelupSupport2ndScript::ItemInfo::ItemInfo(int,int)
LevelupSupport2ndScript::ItemInfo::ItemInfo(int32_t item_index, int item_count)
    : index(item_index), count(static_cast<int16_t>(item_count)) {}

// IDA_EA: 0x8AB75E4
// IDA_NAME: _ZN23LevelupSupport2ndScript8ItemInfoD2Ev
// DEMANGLED: LevelupSupport2ndScript::ItemInfo::~ItemInfo()
LevelupSupport2ndScript::ItemInfo::~ItemInfo() {}
