#include "old_game/economy/inventory/item_list.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/avatar_type_select_runtime.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/data/cash_shop_gift_only_0828.h"
#include "old_data/PVF/data/sync_script.h"
#include "old_data/PVF/equipment/random_option_script.h"
#include "old_game/dungeon/instance/compound_extream_dun_item_runtime.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"

#include <algorithm>
#include <cstdio>
#include <map>
#include <new>
#include <set>
#include <string>
#include <vector>


namespace {

typedef std::map<int, std::map<int, BaseItemValue> > RandomBaseItemMap;

typedef void (*VirtualDeleteFn)(CItem*);

std::set<unsigned int>& GiftOnlyGoods0828() {
    static std::set<unsigned int> goods;
    return goods;
}

void DeleteItemThroughOldVtable(CItem* item) {
    if (item != 0) {
        void** vtable = *reinterpret_cast<void***>(item);
        reinterpret_cast<VirtualDeleteFn>(vtable[1])(item);
    }
}

void LogItemInitFailure(int line, const char* text) {
    LogManager::logFormat(1, "item.cpp", "bool CItemList::init()", line,
                          "%s", text);
}

RandomBaseItemMap* RandomBaseItems() {
    return &G_CDataManager()->get_random_option_script()->base_items;
}

typedef std::map<int, std::vector<std::vector<int> > >
    RecommendedAvatarMap;

int ScanCashShopInt() {
    return ScanInt(static_cast<bool*>(0));
}

void ReadCashShopText() {
    std::string ignored;
    ScanStr(&ignored);
}

void ReadAvatarItems(std::vector<STCeraShopItem>& output,
                     int& sequence,
                     bool store) {
    bool success = false;
    for (;;) {
        const int goods_no = ScanInt(&success);
        if (!success) {
            break;
        }

        STCeraShopItem item;
        item.raw_04 = sequence++;
        item.compare_key = goods_no;
        item.item_idx = ScanCashShopInt();
        item.expiry_date = static_cast<int16_t>(ScanCashShopInt());
        item.sticker = static_cast<uint8_t>(ScanCashShopInt());
        item.featured_idx = ScanCashShopInt();
        item.raw_3c = static_cast<uint8_t>(ScanCashShopInt());
        item.group_idx = 0;
        item.stack_count = 1;
        item.item_type = 0;
        if (!store) {
            item.raw_7c = ScanCashShopInt();
        } else {
            output.push_back(item);
        }
    }
}

void ReadGenericCashShopItems(std::vector<STCeraShopItem>& output,
                              int group_index,
                              uint8_t item_type) {
    bool success = false;
    for (;;) {
        const int goods_no = ScanInt(&success);
        if (!success) {
            break;
        }

        STCeraShopItem item;
        item.item_type = item_type;
        item.compare_key = goods_no;
        item.item_idx = ScanCashShopInt();
        item.stack_count = ScanCashShopInt();
        item.gold_price = ScanCashShopInt();
        item.win_point = ScanCashShopInt();
        item.cera_price = ScanCashShopInt();
        ReadCashShopText();
        item.sticker = static_cast<uint8_t>(ScanCashShopInt());
        item.featured_idx = ScanCashShopInt();
        item.raw_40 = "";
        item.group_idx = group_index;
        output.push_back(item);
    }
}

void ReadMileageCashShopItems(std::vector<STCeraShopItem>& output) {
    bool success = false;
    for (;;) {
        const int goods_no = ScanInt(&success);
        if (!success) {
            break;
        }

        STCeraShopItem item;
        item.group_idx = 11;
        item.compare_key = goods_no;
        item.item_type = static_cast<uint8_t>(ScanCashShopInt());
        item.item_idx = ScanCashShopInt();
        item.stack_count = ScanCashShopInt();
        item.mileage_price = ScanCashShopInt();
        ReadCashShopText();
        item.sticker = static_cast<uint8_t>(ScanCashShopInt());
        item.raw_40 = "";
        output.push_back(item);
    }
}

void ReadPackageCashShopItems(std::vector<STCeraShopItem>& output) {
    bool success = false;
    for (;;) {
        const int goods_no = ScanInt(&success);
        if (!success) {
            break;
        }

        STCeraShopItem item;
        item.compare_key = goods_no;
        item.item_idx = ScanCashShopInt();
        item.gold_price = ScanCashShopInt();
        item.win_point = ScanCashShopInt();
        item.cera_price = ScanCashShopInt();
        ReadCashShopText();
        item.sticker = static_cast<uint8_t>(ScanCashShopInt());
        item.featured_idx = ScanCashShopInt();
        item.expiry_date = static_cast<int16_t>(ScanCashShopInt());
        item.raw_3c = static_cast<uint8_t>(ScanCashShopInt());
        item.group_idx = 7;
        item.stack_count = 1;
        item.item_type = 7;
        item.raw_40 = "";
        output.push_back(item);
    }
}

void SetCashShopItemFlag(std::vector<STCeraShopItem>* groups,
                         int goods_no,
                         int flag_offset) {
    for (int group = 0; group <= 15; ++group) {
        for (std::vector<STCeraShopItem>::iterator item =
                 groups[group].begin();
             item != groups[group].end(); ++item) {
            if (item->compare_key != goods_no) {
                continue;
            }
            if (flag_offset == 0x50) {
                item->gift_disallowance = 1;
            } else if (flag_offset == 0x51) {
                item->cera_point_disallowance = 1;
            } else {
                item->raw_52 = 1;
            }
            break;
        }
    }
}

void ReadCashShopDisallowance(std::vector<STCeraShopItem>* groups,
                              int flag_offset) {
    bool success = false;
    for (;;) {
        const int goods_no = ScanInt(&success);
        if (!success) {
            break;
        }
        SetCashShopItemFlag(groups, goods_no, flag_offset);
    }
}

bool ReadCashShopBonusItems(
    std::vector<stCeraShopBonusInfo_t>& bonus_items) {
    bool success = false;
    for (;;) {
        stCeraShopBonusInfo_t bonus;
        bonus.raw_00 = ScanInt(&success);
        if (!success) {
            return true;
        }
        bonus.raw_04 = ScanInt(&success);
        if (!success) {
            return false;
        }
        const int item_count = ScanInt(&success);
        if (!success) {
            return false;
        }

        int rate_sum = 0;
        bonus.bonus_items.reserve(item_count);
        for (int index = 0; index < item_count; ++index) {
            stCeraShopBonusItemInfo_t item;
            item.item_index = ScanInt(&success);
            if (!success) {
                return false;
            }
            item.count = static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return false;
            }
            item.reserved_06 = 0;
            item.rate = ScanInt(&success);
            if (!success) {
                return false;
            }
            rate_sum += item.rate;
            bonus.bonus_items.push_back(item);
        }

        if (rate_sum != 1000) {
            std::printf("bonus rate error %d", rate_sum);
            return false;
        }
        bonus_items.push_back(bonus);
    }
}

void ApplyNotStackableBuy(std::vector<STCeraShopItem>* groups) {
    std::vector<int> goods_numbers;
    bool success = false;
    for (;;) {
        const int goods_no = ScanInt(&success);
        if (!success) {
            break;
        }
        goods_numbers.push_back(goods_no);
    }

    for (int group = 0; group <= 15; ++group) {
        for (std::vector<STCeraShopItem>::iterator item =
                 groups[group].begin();
             item != groups[group].end(); ++item) {
            for (std::vector<int>::const_iterator goods_no =
                     goods_numbers.begin();
                 goods_no != goods_numbers.end(); ++goods_no) {
                if (item->compare_key == *goods_no) {
                    item->stack_buyable = 0;
                }
            }
        }
    }
}

void ReadRecommendedAvatarList(RecommendedAvatarMap* recommendations) {
    bool success = false;
    for (;;) {
        const unsigned int key =
            static_cast<unsigned int>(ScanInt(&success));
        if (!success) {
            break;
        }
        std::vector<int> selection;
        for (int index = 0; index <= 9; ++index) {
            int value = ScanCashShopInt();
            if (!value) {
                value = -1;
            }
            selection.push_back(value);
        }
        if (key <= 10u && recommendations) {
            (*recommendations)[static_cast<int>(key)].push_back(selection);
        }
    }
}

void ConsumeCashShopIntPairs() {
    bool success = false;
    for (;;) {
        ScanInt(&success);
        if (!success) {
            break;
        }
        ScanInt(&success);
        if (!success) {
            break;
        }
    }
}

void MarkFeaturedGoodsGiftDisallowed(
    std::vector<STCeraShopItem>* groups) {
    for (int group = 0; group <= 15; ++group) {
        for (std::vector<STCeraShopItem>::iterator item =
                 groups[group].begin();
             item != groups[group].end(); ++item) {
            if (item->featured_idx) {
                item->gift_disallowance = 1;
            }
        }
    }
}

}  // namespace

namespace cash_shop_0828 {

void clear_gift_only_goods() {
    GiftOnlyGoods0828().clear();
}

void register_gift_only_goods(unsigned int goods_no) {
    GiftOnlyGoods0828().insert(goods_no);
}

bool is_gift_only_goods(unsigned int goods_no) {
    return GiftOnlyGoods0828().find(goods_no) != GiftOnlyGoods0828().end();
}

}  // namespace cash_shop_0828

// IDA_EA: 0x88FDCB5
// IDA_NAME: _Z22importCashShopItemListPcPSt6vectorI14STCeraShopItemSaIS1_EERS0_I21stCeraShopBonusInfo_tSaIS5_EEPvPiPSt3mapIiS0_IS0_IiSaIiEESaISD_EESt4lessIiESaISt4pairIKiSF_EEEPSD_
// DEMANGLED: importCashShopItemList(char *, vector<STCeraShopItem> *,
//             vector<stCeraShopBonusInfo_t> &, void *, int *,
//             map<int,vector<vector<int> > > *, vector<int> *)
bool importCashShopItemList(
    char* filename,
    std::vector<STCeraShopItem>* groups,
    std::vector<stCeraShopBonusInfo_t>& bonus_items,
    void* unused,
    int* max_gift_count,
    RecommendedAvatarMap* recommendations,
    std::vector<int>* category_event_tags) {
    (void)unused;
    if (!loadRDARScriptFile("", filename)) {
        return false;
    }

    groups[11].clear();
    std::string tag;
    std::string ignored;
    bool success = false;
    int avatar_sequence = 0;
    std::vector<unsigned int> gift_only_goods;
    // Official NewArad_CashShop.shp stores these four coupon/service
    // sections with the exact generic item record ABI.  Buffer them until
    // the main [item] section has performed its one authoritative clear,
    // then feed the normal group-2 materialize/register path.
    std::vector<STCeraShopItem> service_items;

    while (ScanType(tag, true) == 1) {
        if (tag == "[cerashop banner]") {
            ScanInt(&success);
        }
        if (tag == "[npc info]") {
            for (int index = 0; index <= 15; ++index) {
                ScanStr(&ignored);
                ScanCashShopInt();
            }
        }

        if (tag == "[category event tag]") {
            for (;;) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                if (category_event_tags) {
                    category_event_tags->push_back(value);
                }
            }
        } else if (tag == "[avatar]" || tag == "[national avatar]") {
            if (tag == "[avatar]") {
                groups[0].clear();
            }
            ReadAvatarItems(groups[0], avatar_sequence, true);
        } else if (tag == "[dont trade avatar]") {
            ReadAvatarItems(groups[0], avatar_sequence, false);
        } else if (tag == "[coin]" || tag == "[national coin]") {
            if (tag == "[coin]") {
                groups[1].clear();
            }
            ReadGenericCashShopItems(groups[1], 1, 1);
        } else if (tag == "[avatar dye]") {
            groups[15].clear();
            ReadGenericCashShopItems(groups[15], 2, 2);
        } else if (tag == "[item]" || tag == "[national item]") {
            if (tag == "[item]") {
                groups[2].clear();
            }
            ReadGenericCashShopItems(groups[2], 2, 2);
        } else if (tag == "[avagacha coupon]" ||
                   tag == "[cargo upgrade kit]" ||
                   tag == "[cho service]" ||
                   tag == "[creature coupon]") {
            ReadGenericCashShopItems(service_items, 2, 2);
        } else if (tag == "[premium]" || tag == "[national premium]") {
            if (tag == "[premium]") {
                groups[9].clear();
            }
            ReadGenericCashShopItems(groups[9], 9, 9);
        } else if (tag == "[creature]" || tag == "[national creature]") {
            if (tag == "[creature]") {
                groups[3].clear();
            }
            ReadGenericCashShopItems(groups[3], 3, 3);
        } else if (tag == "[NPC]") {
            ScanInt(&success);
        } else if (tag == "[type]") {
            ScanStr(&ignored);
        } else if (tag == "[item group index]") {
            ScanCashShopInt();
        } else if (tag == "[mileage item]") {
            groups[11].clear();
            ReadMileageCashShopItems(groups[11]);
        } else if (tag == "[package]" || tag == "[national package]") {
            if (tag == "[package]") {
                groups[7].clear();
            }
            ReadPackageCashShopItems(groups[7]);
        } else if (tag == "[cart disallowance]") {
            ReadCashShopDisallowance(groups, 0x52);
        } else if (tag == "[gift disallowance]") {
            ReadCashShopDisallowance(groups, 0x50);
        } else if (tag == "[gift only]") {
            for (;;) {
                const int goods_no = ScanInt(&success);
                if (!success) {
                    break;
                }
                if (goods_no >= 0) {
                    gift_only_goods.push_back(
                        static_cast<unsigned int>(goods_no));
                }
            }
        } else if (tag == "[cerapoint disallowance]") {
            ReadCashShopDisallowance(groups, 0x51);
        } else if (tag == "[max gift count]") {
            const int value = ScanInt(&success);
            if (!success) {
                break;
            }
            if (max_gift_count) {
                *max_gift_count = value;
            }
        } else if (tag == "[bonus item info]") {
            if (!ReadCashShopBonusItems(bonus_items)) {
                return false;
            }
        } else if (tag == "[not stackable buy]") {
            ApplyNotStackableBuy(groups);
        } else if (tag == "[recommended avatar list]") {
            ReadRecommendedAvatarList(recommendations);
        } else if (tag == "[restricted premium map]") {
            ConsumeCashShopIntPairs();
        } else if (tag == "[immediately adaptive product]") {
            for (;;) {
                ScanInt(&success);
                if (!success) {
                    break;
                }
            }
        } else if (tag == "[avatar coin]") {
            ReadGenericCashShopItems(groups[0], 2, 2);
        } else if (tag == "[exceptional product price in first server]") {
            ConsumeCashShopIntPairs();
        }
    }

    groups[2].insert(groups[2].end(),
                     service_items.begin(),
                     service_items.end());

    MarkFeaturedGoodsGiftDisallowed(groups);
    for (std::vector<unsigned int>::const_iterator goods_no =
             gift_only_goods.begin();
         goods_no != gift_only_goods.end(); ++goods_no) {
        cash_shop_0828::register_gift_only_goods(*goods_no);
    }
    return true;
}

// IDA_EA: 0x8510D12
// IDA_NAME: _ZN9CItemListC2Ev
// DEMANGLED: CItemList::CItemList(void)
CItemList::CItemList() {}

// IDA_EA: 0x8510DF6
// IDA_NAME: _ZN9CItemListD2Ev
// DEMANGLED: CItemList::~CItemList()
CItemList::~CItemList() {
    destroy();
}

// IDA_EA: 0x8510F06
// IDA_NAME: _ZN9CItemList7destroyEv
// DEMANGLED: CItemList::destroy(void)
CItemList* CItemList::destroy() {
    cash_shop_0828::clear_gift_only_goods();
    for (CItemIndexList::iterator it = items_by_index_.begin();
         it != items_by_index_.end(); ++it) {
        DeleteItemThroughOldVtable(it->second);
    }
    items_by_index_.clear();

    if (item_grade_list_ != 0) {
        for (int grade = 200; grade >= 0; --grade) {
            item_grade_list_[grade].~CItemGradeList();
        }
        ::operator delete[](
            reinterpret_cast<unsigned char*>(item_grade_list_) - 4u);
        item_grade_list_ = 0;
    }
    return this;
}

// IDA_EA: 0x851129E
// IDA_NAME: _ZN9CItemList11insert_itemEiiP5CItem
// DEMANGLED: CItemList::insert_item(int, int, CItem *)
int CItemList::insert_item(int grade, int item_index, CItem* item) {
    if (items_by_index_.find(item_index) != items_by_index_.end()) {
        LogManager::logFormat(1, "item.cpp",
                              "bool CItemList::insert_item(int, int, CItem*)",
                              0x81F,
                              "Insert Fail Item Group[%d] Index: %d",
                              grade, item_index);
        return 0;
    }

    item_grade_list_[grade].insert(std::make_pair(item_index, item));
    items_by_index_.insert(std::make_pair(item_index, item));

    const unsigned long quest_drop_bonus =
        static_cast<unsigned long>(item->GetQuestItemDropBouns());
    if (quest_drop_bonus != 0) {
        quest_drop_bonus_items_.insert(
            std::make_pair(quest_drop_bonus, item_index));
    }

    if (item->vtable_->is_random_option(item) != 0) {
        const BaseItemKey key(static_cast<uint8_t>(item->get_rarity()),
                              static_cast<uint8_t>(item->getUsableLevel()),
                              static_cast<uint16_t>(item->getItemGroupName()));
        const int base_item_key = *reinterpret_cast<const int*>(&key);
        BaseItemValue value;
        value.grade = item->get_grade();
        value.item_index = static_cast<unsigned long>(item->get_index());
        (*RandomBaseItems())[base_item_key].insert(
            std::make_pair(value.grade, value));
    }
    return 1;
}

// IDA_EA: 0x869476E
// IDA_NAME: _ZNK9CItemList21GetQuestItemDropBounsERKm
// DEMANGLED: CItemList::GetQuestItemDropBouns(unsigned long const&)const
int CItemList::GetQuestItemDropBouns(
    const unsigned long& item_drop_bonus) const {
    std::map<unsigned long, int>::const_iterator it =
        quest_drop_bonus_items_.find(item_drop_bonus);
    return it == quest_drop_bonus_items_.end() ? 0 : it->second;
}

// IDA_EA: 0x8510FFA
// IDA_NAME: _ZN9CItemList4initEv
// DEMANGLED: CItemList::init(void)
int CItemList::init() {
    cash_shop_0828::clear_gift_only_goods();
    unsigned char* const allocation = static_cast<unsigned char*>(
        ::operator new[](4u + 201u * sizeof(CItemGradeList)));
    *reinterpret_cast<unsigned int*>(allocation) = 201u;
    CItemGradeList* const grades =
        reinterpret_cast<CItemGradeList*>(allocation + 4u);
    for (int grade = 0; grade <= 200; ++grade) {
        new (&grades[grade]) CItemGradeList();
    }
    item_grade_list_ = grades;

    if (load_item_files() != 1) {
        LogItemInitFailure(0x792, "load_item_files() fail");
        return 0;
    }

    GlobalData::s_pCompoundBindCube->after_addItem();

    if (Load_Goods_File(G_CDataManager()->GetCeraShopGoodsName()) != 1) {
        LogItemInitFailure(0x7A5,
                           "Load_Goods_File() of ItemVendingMachine fail ");
        return 0;
    }
    if (Load_Goods_File(G_CDataManager()->SuddenShopFileName1()) != 1) {
        LogItemInitFailure(
            0x7C8, "SuddenShopFileName1() of ItemVendingMachine fail ");
        return 0;
    }
    if (Load_Goods_File(G_CDataManager()->SuddenShopFileName2()) != 1) {
        LogItemInitFailure(
            0x7CE, "SuddenShopFileName2() of ItemVendingMachine fail ");
        return 0;
    }
    if (Load_Goods_File(G_CDataManager()->SuddenShopFileName3()) != 1) {
        LogItemInitFailure(
            0x7D4, "SuddenShopFileName3() of ItemVendingMachine fail ");
        return 0;
    }

    return 1;
}

// IDA_EA: 0x85123DA
// IDA_NAME: _ZN9CItemList15Load_Goods_FileEPKc
// DEMANGLED: CItemList::Load_Goods_File(char const*)
bool CItemList::Load_Goods_File(const char* filename) {
    std::vector<STCeraShopItem> groups[16];
    if (importCashShopItemList(
            const_cast<char*>(filename), groups, cera_shop_bonus_items_, 0,
            0, 0, 0) != 1) {
        LogManager::logFormat(
            1, "item.cpp", "bool CItemList::Load_Goods_File(const char*)",
            0xA76, "importCashShopItemList() fail file - %s", filename);
        return false;
    }

    sync_script::GetInstanceSyncScript()->truncateCeraShopItemTable();
    for (int group = 0; group <= 15; ++group) {
        for (std::vector<STCeraShopItem>::const_iterator item =
                 groups[group].begin();
             item != groups[group].end(); ++item) {
            CCeraShopGoods* const goods = new CCeraShopGoods(*item);
            goods_by_index_[item->compare_key] = goods;

            int effective_cera_price = 0;
            if (item->item_idx == 1) {
                effective_cera_price = item->cera_price;
            } else {
                CEquipItem* const related_item =
                    reinterpret_cast<CEquipItem*>(static_cast<uintptr_t>(
                        goods->GetRelatedItem()));
                const std::vector<stAvatarTypeSelect_t>* const selections =
                    reinterpret_cast<
                        const std::vector<stAvatarTypeSelect_t>*>(
                        related_item->getAvatarTypeSelect());
                effective_cera_price = item->cera_price;
                if (item->group_idx == 0 &&
                    static_cast<uint32_t>(item->compare_key) <
                        static_cast<uint32_t>(-1) &&
                    !selections->empty()) {
                    effective_cera_price =
                        (*selections)[item->expiry_date - 1]
                            .cera_price;
                }
            }

            if (effective_cera_price != 0 &&
                static_cast<uint16_t>(item->expiry_date) <= 1u) {
                CItem* const related_item =
                    G_CDataManager()->find_item(item->item_idx);
                if (related_item && item->compare_key != -1) {
                    if (sync_script::GetInstanceSyncScript()
                            ->insertCeraShopItemIntoDB(
                                static_cast<unsigned int>(item->compare_key),
                                static_cast<unsigned int>(item->item_idx),
                                item->stack_count, effective_cera_price,
                                item->expiry_date,
                                related_item->GetItemName(),
                                related_item->GetItemIconName()) != 1) {
                        LogManager::logFormat(
                            1, "item.cpp",
                            "bool CItemList::Load_Goods_File(const char*)",
                            0xB01,
                            "insertCeraShopItemIntoDB() fail IPG - %d",
                            item->compare_key);
                        return false;
                    }
                }
            }

            if (item->featured_idx) {
                restrict_codes_by_item_.insert(std::make_pair(
                    static_cast<unsigned int>(item->item_idx),
                    item->featured_idx));
            }
        }
    }
    return true;
}
