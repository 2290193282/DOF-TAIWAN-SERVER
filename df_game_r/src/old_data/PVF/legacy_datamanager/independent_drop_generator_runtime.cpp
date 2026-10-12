#include "old_data/PVF/data/item_generator_types.h"

#include <algorithm>
#include <stdint.h>
#include <utility>
#include <vector>

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/server_utility.h"
#include "old_data/PVF/etc/independent_drop_parameter_script.h"
#include "old_data/PVF/etc/independent_drop_types.h"

namespace {

int32_t& DropRateForMemberClass(stMonsterIndependentDrop_t& drop,
                                uint8_t member_count_class) {
    // The original indexes the record's dword array at class + 1:
    // class zero selects +0x04; classes one and above select values[class-1].
    return member_count_class == 0u
               ? drop.value_04
               : drop.values[member_count_class - 1u];
}

int DropAttemptCount(const stMonsterIndependentDrop_t& drop,
                     uint8_t party_member_count) {
    // IDA indexes the record at dword (party_member_count + 5).  values[0]
    // begins at dword 2, so the typed equivalent is values[count + 3].
    return drop.values[party_member_count + 3u];
}

}  // namespace

// IDA_EA: 0x834972F
// IDA_NAME: _Z26independentItemRateControlR26stMonsterIndependentDrop_tRKN8WongWork19stGenerateRefData_tE
// DEMANGLED: independentItemRateControl(
//             stMonsterIndependentDrop_t &,
//             WongWork::stGenerateRefData_t const&)
int independentItemRateControl(
    stMonsterIndependentDrop_t& drop,
    const WongWork::stGenerateRefData_t& reference) {
    std::vector<stIndependentDropRateControl>& rate_controls =
        G_CDataManager()->get_independent_drop_parameter_script()
            ->rate_control_;
    const uint8_t member_count_class =
        reference.generator_data.boss_monster_drop.member_count_class;
    const uint8_t monster_kind =
        reference.generator_data.boss_monster_drop.monster_kind;

    for (std::vector<stIndependentDropRateControl>::iterator control =
             rate_controls.begin();
         control != rate_controls.end();
         ++control) {
        if (control->dungeon_index != drop.value_00) {
            continue;
        }

        int32_t& selected_rate =
            DropRateForMemberClass(drop, member_count_class);
        if (!selected_rate) {
            continue;
        }

        const int level_gap =
            static_cast<int>(
                reference.generator_data.boss_monster_drop
                    .reserved_0b_0f[0]) -
            static_cast<int>(monster_kind);
        if (control->rate_min <= level_gap) {
            float rate_scale = static_cast<float>(
                10000 - level_gap * control->rate_max);
            if (rate_scale < 0.0f) {
                rate_scale = 1.0f;
            }
            selected_rate = static_cast<int32_t>(
                static_cast<long double>(selected_rate) *
                (rate_scale / 10000.0f));
        }
    }
    return 0;
}

// IDA_EA: 0x834989A
// IDA_NAME: _Z23generateIndependentItemRKSt6vectorI26stMonsterIndependentDrop_tSaIS0_EERKN8WongWork19stGenerateRefData_tERS_I10Inven_ItemSaIS9_EE
// DEMANGLED: generateIndependentItem(
//             std::vector<stMonsterIndependentDrop_t> const&,
//             WongWork::stGenerateRefData_t const&,
//             std::vector<Inven_Item>&)
int generateIndependentItem(
    const std::vector<stMonsterIndependentDrop_t>& independent_drops,
    const WongWork::stGenerateRefData_t& reference,
    std::vector<Inven_Item>& generated_items) {
    const uint8_t member_count_class =
        reference.generator_data.boss_monster_drop.member_count_class;
    const uint8_t monster_kind =
        reference.generator_data.boss_monster_drop.monster_kind;
    const uint8_t party_member_count =
        reference.generator_data.boss_monster_drop.party_member_count;

    for (std::vector<stMonsterIndependentDrop_t>::const_iterator source =
             independent_drops.begin();
         source != independent_drops.end();
         ++source) {
        stMonsterIndependentDrop_t drop(*source);

        if (drop.min_count || drop.max_count) {
            if (reference.blue_marble_context) {
                if (static_cast<int>(reference.blue_marble_character_level) <
                        drop.min_count ||
                    static_cast<int>(reference.blue_marble_character_level) >
                        drop.max_count) {
                    continue;
                }
            } else if (drop.min_count > static_cast<int>(monster_kind) ||
                       drop.max_count < static_cast<int>(monster_kind)) {
                continue;
            }
        }

        int accepted_count = 0;
        independentItemRateControl(drop, reference);
        int32_t& selected_rate =
            DropRateForMemberClass(drop, member_count_class);

        // Full-reference +0x3C is generator-data +0x08.  The original applies
        // the map-context percentage only when this byte is exactly three.
        if (drop.total_rate > 0 &&
            reference.premium_independent_drop_rate > 0 &&
            reference.generator_data.boss_monster_drop
                    .reserved_03_09[5] == 3u) {
            selected_rate =
                (reference.premium_independent_drop_rate + 100) *
                selected_rate / 100;
        }

        if (reference.blue_marble_context &&
            static_cast<int>(reference.blue_marble_buff_value) > 1) {
            if (selected_rate *
                    static_cast<int>(reference.blue_marble_buff_value) <=
                999999) {
                selected_rate *=
                    static_cast<int>(reference.blue_marble_buff_value);
            } else {
                selected_rate = 1000000;
            }
        }

        const int attempt_count =
            DropAttemptCount(drop, party_member_count);
        if (drop.value_00 || drop.total_rate) {
            for (int attempt = 0; attempt < attempt_count; ++attempt) {
                if (reference.party_member_force_drop) {
                    ++accepted_count;
                } else if (
                    selected_rate >
                    static_cast<int>(get_rand_int(1000000))) {
                    ++accepted_count;
                }
            }
            accepted_count =
                std::min(accepted_count, static_cast<int>(drop.values[8]));
        } else {
            accepted_count = attempt_count;
        }

        Inven_Item item;
        const bool dungeon_filter_matches =
            !reference.blue_marble_context || drop.rate < 0 ||
            static_cast<int>(reference.blue_marble_character_job) ==
                drop.rate;
        if (drop.total_rate) {
            if (!dungeon_filter_matches) {
                continue;
            }

            for (int item_index = 0;
                 item_index < accepted_count;
                 ++item_index) {
                const int random_value =
                    static_cast<int>(get_rand_int(drop.total_rate));
                for (std::vector<std::pair<int, int> >::const_iterator entry =
                         drop.drops.begin();
                     entry != drop.drops.end();
                     ++entry) {
                    if (entry->second > random_value) {
                        item.item_id_ =
                            static_cast<uint32_t>(entry->first);
                        break;
                    }
                }
                G_CDataManager()->get_item_list()->create_item(
                    static_cast<ENUM_ITEM_CREATE_TYPE>(1), item, 1);
                generated_items.push_back(item);
            }
        } else if (dungeon_filter_matches) {
            item.item_id_ =
                static_cast<uint32_t>(drop.value_00);
            G_CDataManager()->get_item_list()->create_item(
                static_cast<ENUM_ITEM_CREATE_TYPE>(1), item, 1);
            if (drop.value_00 || drop.total_rate) {
                for (int item_index = 0;
                     item_index < accepted_count;
                     ++item_index) {
                    generated_items.push_back(item);
                }
            } else {
                item.add_info_ = accepted_count;
                generated_items.push_back(item);
            }
        }
    }
    return 0;
}
