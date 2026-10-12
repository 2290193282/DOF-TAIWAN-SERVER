#include "old_data/PVF/data/item_generator_drop_types.h"

#include <ctime>

#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/mt_rand.h"

namespace WongWork {
namespace {

bool MatchesDropType(const CItem& item, int drop_type) {
    const CItemVTable* const vtable = item.vtable_;
    if (!vtable) {
        return false;
    }
    if (item.is_stackable()) {
        if (vtable->get_item_type(&item) == 3 && drop_type == 3) {
            return true;
        }
        return drop_type == 1;
    }
    if (vtable->is_creature_item(&item) && drop_type == 4) {
        return true;
    }
    return drop_type == 2;
}

template <typename Table>
int ChooseWeightedItem(Table& table, CMTRand* random) {
    if (table.empty() || !random) {
        return -1;
    }
    const unsigned int total_weight = table.rbegin()->first;
    const unsigned int number = random->randInt(total_weight);
    typename Table::const_iterator selected = table.lower_bound(number);
    if (selected == table.end()) {
        --selected;
    }
    return selected->second;
}

}  // namespace

}  // namespace WongWork

// IDA_EA: 0x832E02A
// IDA_NAME: _ZNK5CItem12get_gen_rateEv
int CItem::get_gen_rate() const {
    return generation_rate_;
}

// IDA_EA: 0x8539E00 / 0x8539E10 / 0x8539E24
// These PVF item fields are read by the old rate-table build exactly at the
// listed offsets.  They are exposed as named CItem accessors so generators do
// not manufacture a parallel item layout.
int CItem::isSpecialMonsterDropItem() {
    return special_monster_drop_.enabled;
}

int CItem::isDropItemDungeonDifficulty(int difficulty) {
    switch (difficulty) {
        case 0: return special_monster_drop_.extra_drop_easy;
        case 1: return special_monster_drop_.extra_drop_medium;
        case 2: return special_monster_drop_.extra_drop_hard;
        case 3: return special_monster_drop_.extra_drop_ultimate;
        case 4: return special_monster_drop_.extra_drop_hero;
        default: return 0;
    }
}

int CItem::isDropItemDungeonType(int dungeon_type) {
    switch (dungeon_type) {
        case 0: return special_monster_drop_.dungeon_normal;
        case 1: return special_monster_drop_.dungeon_ancient;
        case 2: return special_monster_drop_.dungeon_dimension;
        default: return 0;
    }
}

// IDA_EA: 0x8539E38
// IDA_NAME: _ZNK9CItemList16getItemGradeListEv
const CItemGradeList* CItemList::getItemGradeList() const {
    return item_grade_list_;
}

namespace WongWork {

// IDA_EA: 0x8539E66
// IDA_NAME: _ZN8WongWork17CItemGenRateTableC2Ev
CItemGenRateTable::CItemGenRateTable()
    : item_rates_(), random_(0), random_option_handle_() {
    init();
}

// IDA_EA: 0x853A030
// IDA_NAME: _ZN8WongWork17CItemGenRateTableD2Ev
CItemGenRateTable::~CItemGenRateTable() { destroy(); }

// IDA_EA: 0x85345B6
// IDA_NAME: _ZN8WongWork17CItemGenRateTable4initEv
CItemGenRateTable* CItemGenRateTable::init() {
    delete random_;
    random_ = new CMTRand(static_cast<unsigned long>(std::time(0)));
    return this;
}

// IDA_EA: 0x8534638
// IDA_NAME: _ZN8WongWork17CItemGenRateTable7destroyEv
CItemGenRateTable* CItemGenRateTable::destroy() {
    delete random_;
    random_ = 0;
    return this;
}

// IDA_EA: 0x853466C
// IDA_NAME: _ZN8WongWork17CItemGenRateTable13generateTableEPNS_16stGenItemRange_tEb
bool CItemGenRateTable::generateTable(
    const unsigned char item_ref_range[201][2],
    bool include_random_option_items) {
    CItemList* const item_list =
        G_CDataManager()->get_arad_jumping_item_list();
    const CItemGradeList* const grade_items =
        item_list ? item_list->getItemGradeList() : 0;
    if (!grade_items) {
        return false;
    }

    for (int drop_type = 1; drop_type <= 4; ++drop_type) {
        for (int rarity = 0; rarity <= 5; ++rarity) {
            for (int level = 1; level <= 200; ++level) {
                std::map<unsigned int, int>& result =
                    item_rates_[drop_type][rarity][level];
                int total_weight = 0;
                for (int relative_level =
                         -static_cast<int>(item_ref_range[level][0]);
                     relative_level <
                         static_cast<int>(item_ref_range[level][1]) &&
                     level + relative_level <= 200;
                     ++relative_level) {
                    const CItemGradeList& items =
                        grade_items[level + relative_level];
                    for (CItemGradeList::const_iterator item_it =
                             items.begin();
                         item_it != items.end();
                         ++item_it) {
                        CItem* const item = item_it->second;
                        if (!item || !item->get_gen_rate() ||
                            item->get_rarity() != rarity ||
                            !MatchesDropType(*item, drop_type) ||
                            item->isSpecialMonsterDropItem()) {
                            continue;
                        }
                        if (!include_random_option_items &&
                            item->vtable_ &&
                            item->vtable_->is_random_option(item)) {
                            continue;
                        }
                        total_weight += item->get_gen_rate();
                        result.insert(std::make_pair(
                            static_cast<unsigned int>(total_weight),
                            item->get_index()));
                    }
                }
            }
        }
    }
    return true;
}

// IDA_EA: 0x85349A4
// IDA_NAME: _ZN8WongWork17CItemGenRateTable10chooseItemENS_15eItemDropType_tE11ENUM_RARITYi
int CItemGenRateTable::chooseItem(int drop_type,
                                  int rarity,
                                  int character_level) {
    if (drop_type < 0 || drop_type >= 5 || rarity < 0 || rarity >= 6 ||
        character_level < 0 || character_level > 200) {
        return -1;
    }

    // IDA 0x85349A4 first gives equipment drops to the old random-option
    // selector.  Its rarity argument is in/out: even when no random-option
    // item is returned, choose_item may lower the local rarity (notably the
    // level <= 9, rarity 2 path at 0x85F1560), and the normal rate table must
    // be indexed with that adjusted value.
    if (drop_type == 2) {
        ENUM_RARITY adjusted_rarity = static_cast<ENUM_RARITY>(rarity);
        const int random_option_item =
            random_option_handle_.choose_item(adjusted_rarity,
                                               character_level);
        if (random_option_item != -1) {
            return random_option_item;
        }
        rarity = static_cast<int>(adjusted_rarity);
    }

    return ChooseWeightedItem(item_rates_[drop_type][rarity][character_level],
                              random_);
}

// IDA_EA: 0x8539E4E
// IDA_NAME: _ZN8WongWork17CItemGenRateTable35set_disable_drop_random_option_itemEv
random_option::CRandomOptionItemHandle*
CItemGenRateTable::set_disable_drop_random_option_item() {
    return random_option_handle_.set_disable_drop();
}

// IDA_EA: 0x853A0B6
// IDA_NAME: _ZN8WongWork35CSecialIMonsterDropItemGenRateTableC2Ev
CSecialIMonsterDropItemGenRateTable::CSecialIMonsterDropItemGenRateTable()
    : item_rates_(), random_(0), random_option_handle_() {
    init();
}

// IDA_EA: 0x853A35C
// IDA_NAME: _ZN8WongWork35CSecialIMonsterDropItemGenRateTableD2Ev
CSecialIMonsterDropItemGenRateTable::~CSecialIMonsterDropItemGenRateTable() {
    destroy();
}

// IDA_EA: 0x8534C1A
// IDA_NAME: _ZN8WongWork35CSecialIMonsterDropItemGenRateTable4initEv
// IDA_EA: 0x8534C9C
// IDA_NAME: _ZN8WongWork35CSecialIMonsterDropItemGenRateTable7destroyEv
CSecialIMonsterDropItemGenRateTable*
CSecialIMonsterDropItemGenRateTable::init() {
    delete random_;
    random_ = new CMTRand(static_cast<unsigned long>(std::time(0)));
    return this;
}

CSecialIMonsterDropItemGenRateTable*
CSecialIMonsterDropItemGenRateTable::destroy() {
    delete random_;
    random_ = 0;
    return this;
}

// IDA_EA: 0x8534CD0
// IDA_NAME: _ZN8WongWork35CSecialIMonsterDropItemGenRateTable13generateTableEPNS_16stGenItemRange_tE
bool CSecialIMonsterDropItemGenRateTable::generateTable(
    const unsigned char item_ref_range[201][2]) {
    CItemList* const item_list =
        G_CDataManager()->get_arad_jumping_item_list();
    const CItemGradeList* const grade_items =
        item_list ? item_list->getItemGradeList() : 0;
    if (!grade_items) {
        return false;
    }

    for (int dungeon_difficulty = 0; dungeon_difficulty <= 4;
         ++dungeon_difficulty) {
        for (int dungeon_type = 0; dungeon_type <= 2; ++dungeon_type) {
            for (int drop_type = 1; drop_type <= 4; ++drop_type) {
                for (int rarity = 0; rarity <= 5; ++rarity) {
                    for (int level = 1; level <= 200; ++level) {
                        std::map<unsigned int, int>& result =
                            item_rates_[dungeon_difficulty][dungeon_type]
                                       [drop_type][rarity][level];
                        int total_weight = 0;
                        for (int relative_level =
                                 -static_cast<int>(item_ref_range[level][0]);
                             relative_level < static_cast<int>(
                                                  item_ref_range[level][1]) &&
                             level + relative_level <= 200;
                             ++relative_level) {
                            const CItemGradeList& items =
                                grade_items[level + relative_level];
                            for (CItemGradeList::const_iterator item_it =
                                     items.begin();
                                 item_it != items.end();
                                 ++item_it) {
                                CItem* const item = item_it->second;
                                if (!item || !item->get_gen_rate() ||
                                    item->get_rarity() != rarity ||
                                    !MatchesDropType(*item, drop_type) ||
                                    !item->isSpecialMonsterDropItem() ||
                                    !item->isDropItemDungeonDifficulty(
                                        dungeon_difficulty) ||
                                    !item->isDropItemDungeonType(
                                        dungeon_type)) {
                                    continue;
                                }
                                total_weight += item->get_gen_rate();
                                result.insert(std::make_pair(
                                    static_cast<unsigned int>(total_weight),
                                    item->get_index()));
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

// IDA_EA: 0x8534FCE
// IDA_NAME: _ZN8WongWork35CSecialIMonsterDropItemGenRateTable10chooseItemENS_15eItemDropType_tE11ENUM_RARITYi23ENUM_DUNGEON_DIFFICULTY27ENUM_ITEM_DROP_DUNGEON_TYPE
int CSecialIMonsterDropItemGenRateTable::chooseItem(
    int drop_type,
    int rarity,
    int character_level,
    int dungeon_difficulty,
    int dungeon_type) {
    if (drop_type < 0 || drop_type >= 5 || rarity < 0 || rarity >= 6 ||
        character_level < 0 || character_level > 200 ||
        dungeon_difficulty < 0 || dungeon_difficulty >= 5 ||
        dungeon_type < 0 || dungeon_type >= 3) {
        return -1;
    }
    return ChooseWeightedItem(item_rates_[dungeon_difficulty][dungeon_type]
                                         [drop_type][rarity][character_level],
                              random_);
}

}  // namespace WongWork
