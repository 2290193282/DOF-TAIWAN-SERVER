#include "old_game/economy/inventory/item.h"

// IDA_EA: 0x822C97A
// IDA_NAME: _ZNK5CItem11hasCategoryEN12STItemScript18ENUM_ITEM_CATEGORYE
// DEMANGLED: CItem::hasCategory(STItemScript::ENUM_ITEM_CATEGORY)const
int CItem::hasCategory(STItemScript::ENUM_ITEM_CATEGORY category) const {
    return categories_.find(category) != categories_.end();
}
