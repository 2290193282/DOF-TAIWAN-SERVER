#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"

#include <stdint.h>

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/server_utility.h"
#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x8349DA0
// IDA_NAME: _Z22generateDropAvatarItemRK23stEnamyDropAvatarInfo_thhRSt6vectorI10Inven_ItemSaIS3_EE
// DEMANGLED: generateDropAvatarItem(stEnamyDropAvatarInfo_t const&,
//             unsigned char,unsigned char,std::vector<Inven_Item>&)
//
// The reference's two byte selectors are deliberately not range-checked:
// the original indexes grade_rates[grade_index] and
// part_rates[part_index - 1] directly.  The result is the old x87
// truncate-toward-zero probability product, followed by one independently
// weighted avatar selection per requested drop.
bool generateDropAvatarItem(const stEnamyDropAvatarInfo_t& avatar_info,
                            uint8_t grade_index,
                            uint8_t part_index,
                            std::vector<Inven_Item>& items) {
    const int drop_probability = static_cast<int>(
        static_cast<long double>(avatar_info.base_rate) *
        avatar_info.grade_rates[grade_index] *
        avatar_info.part_rates[static_cast<int>(part_index) - 1]);

    // The original uses signed `setg` for this comparison.  The random range
    // itself is small, but retaining the signed conversion matters for an
    // invalid negative script probability.
    if (static_cast<int>(get_rand_int(100000)) > drop_probability) {
        return true;
    }

    // IDA uses an unsigned loop counter and unsigned comparison here.
    for (uint32_t attempt = 0;
         static_cast<uint32_t>(avatar_info.drop_count) > attempt;
         ++attempt) {
        const unsigned int selected_rate =
            get_rand_int(avatar_info.total_rate);
        for (std::vector<stEnamyDropAvatarElement_t>::const_iterator element =
                 avatar_info.elements.begin();
             element != avatar_info.elements.end();
             ++element) {
            // `selected_rate` intentionally makes this the original unsigned
            // comparison, including its behavior for malformed negative rates.
            if (element->accumulated_rate > selected_rate) {
                Inven_Item item;
                item.item_id_ =
                    static_cast<uint32_t>(element->item_index);
                G_CDataManager()->get_item_list()->create_item(
                    static_cast<ENUM_ITEM_CREATE_TYPE>(1), item, 1);
                // The old body zero-extends only the low byte of +0x08 and
                // stores it as the 16-bit avatar selection at Inven_Item+0x0b.
                item.avatar_select_ = static_cast<uint16_t>(
                    static_cast<uint8_t>(element->avatar_part));
                items.push_back(item);
                break;
            }
        }
    }

    return false;
}

// IDA_EA: 0x8349F4B
// IDA_NAME: _Z24generateCatchMonsterItemRKSt6vectorI13CatchItemInfoSaIS0_EERS_I10Inven_ItemSaIS5_EE
// DEMANGLED: generateCatchMonsterItem(std::vector<CatchItemInfo> const&,
//             std::vector<Inven_Item>&)
int generateCatchMonsterItem(const std::vector<CatchItemInfo>& catch_items,
                             std::vector<Inven_Item>& items) {
    for (std::vector<CatchItemInfo>::const_iterator catch_item =
             catch_items.begin();
         catch_item != catch_items.end();
         ++catch_item) {
        // IDA placement-constructs this record before checking count or rate.
        Inven_Item item;
        if (catch_item->count && catch_item->probability != 0.0f) {
            const int random_value = static_cast<int>(get_rand_int(100));
            if (static_cast<long double>(catch_item->probability) >
                static_cast<long double>(random_value)) {
                item.item_id_ =
                    static_cast<uint32_t>(catch_item->item_index);
                G_CDataManager()->get_item_list()->create_item(
                    static_cast<ENUM_ITEM_CREATE_TYPE>(1), item, 1);
                for (int item_count = 0;
                     catch_item->count > item_count;
                     ++item_count) {
                    items.push_back(item);
                }
            }
        }
    }

    // The last iterator inequality is false at the original return site.
    return 0;
}

// IDA_EA: 0x834A15E
// IDA_NAME: _ZNK12CAICharacter22generateDropAvatarItemEhhRSt6vectorI10Inven_ItemSaIS1_EE
// DEMANGLED: CAICharacter::generateDropAvatarItem(
//             unsigned char,unsigned char,std::vector<Inven_Item>&)const
bool CAICharacter::generateDropAvatarItem(
    uint8_t grade_index,
    uint8_t part_index,
    std::vector<Inven_Item>& items) const {
    return ::generateDropAvatarItem(
        drop_avatar_info, grade_index, part_index, items);
}

// IDA_EA: 0x834F8A6
// IDA_NAME: _ZNK8CMonster22generateDropAvatarItemEhhRSt6vectorI10Inven_ItemSaIS1_EE
// DEMANGLED: CMonster::generateDropAvatarItem(
//             unsigned char,unsigned char,std::vector<Inven_Item>&)const
bool CMonster::generateDropAvatarItem(uint8_t grade_index,
                                      uint8_t part_index,
                                      std::vector<Inven_Item>& items) const {
    return ::generateDropAvatarItem(
        script_.enemy_drop_avatar, grade_index, part_index, items);
}

// IDA_EA: 0x834F8E8
// IDA_NAME: _ZNK8CMonster24generateCatchMonsterItemERSt6vectorI10Inven_ItemSaIS1_EE
// DEMANGLED: CMonster::generateCatchMonsterItem(std::vector<Inven_Item>&)const
int CMonster::generateCatchMonsterItem(std::vector<Inven_Item>& items) const {
    return ::generateCatchMonsterItem(script_.catch_items, items);
}
