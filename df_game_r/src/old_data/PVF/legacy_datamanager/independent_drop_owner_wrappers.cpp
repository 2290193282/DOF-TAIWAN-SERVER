#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"

#include <vector>

#include "old_data/PVF/data/item_generator_types.h"

// IDA_EA: 0x834989A
// CALLEE_IDA_NAME:
// _Z23generateIndependentItemRKSt6vectorI26stMonsterIndependentDrop_tSaIS0_EERKN8WongWork19stGenerateRefData_tERS_I10Inven_ItemSaIS9_EE
extern int generateIndependentItem(
    const std::vector<stMonsterIndependentDrop_t>& independent_drops,
    const WongWork::stGenerateRefData_t& reference,
    std::vector<Inven_Item>& items)
    __asm__(
        "_Z23generateIndependentItemRKSt6vectorI26stMonsterIndependentDrop_tSaIS0_EERKN8WongWork19stGenerateRefData_tERS_I10Inven_ItemSaIS9_EE");

// IDA_EA: 0x834A13A
// IDA_NAME: _ZNK12CAICharacter23generateIndependentItemERKN8WongWork19stGenerateRefData_tERSt6vectorI10Inven_ItemSaIS5_EE
// DEMANGLED: CAICharacter::generateIndependentItem(
//             WongWork::stGenerateRefData_t const&,
//             std::vector<Inven_Item>&)const
int CAICharacter::generateIndependentItem(
    const WongWork::stGenerateRefData_t& reference,
    std::vector<Inven_Item>& items) const {
    return ::generateIndependentItem(
        independent_drop_items, reference, items);
}

// IDA_EA: 0x834F876
// IDA_NAME: _ZNK8CMonster23generateIndependentItemERKN8WongWork19stGenerateRefData_tERSt6vectorI10Inven_ItemSaIS5_EE
// DEMANGLED: CMonster::generateIndependentItem(
//             WongWork::stGenerateRefData_t const&,
//             std::vector<Inven_Item>&)const
int CMonster::generateIndependentItem(
    const WongWork::stGenerateRefData_t& reference,
    std::vector<Inven_Item>& items) const {
    return ::generateIndependentItem(
        getMonsterScript()->independent_drops, reference, items);
}
