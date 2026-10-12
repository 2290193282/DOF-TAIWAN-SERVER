#include "old_game/economy/shop/npc_runtime.h"

#include <stddef.h>

#include <cstdio>
#include <cstring>

#include "common/config.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/ipg_runtime.h"
#include "old_data/PVF/data/cash_shop_gift_only_0828.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/log/format/trace.h"
#include "old_data/db/auction_cashshop/db_save_featured_goods_idx.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_params.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "common/manager/os_api.h"
#include "common/manager/system_time.h"

namespace {

struct SpecialItemRule {
    int item_id;
    int special_type;
    int special_value;
    bool writes_unique;
    bool unique;
};

const SpecialItemRule kSpecialItemRules[] = {
    {5, 1, 24, false, false},
    {6, 1, 152, false, false},
    {9, 2, 1, false, false},
    {11, 2, 3, false, false},
    {16, 4, 8, false, false},
    {17, 3, 7, false, false},
    {18, 2, 9, true, true},
    {19, 2, 10, true, true},
    {20, 2, 11, true, true},
    {22, 2, 13, true, false},
    {23, 2, 14, true, false},
    {29, 2, 16, true, false},
    {30, 2, 17, true, true},
    {31, 2, 18, true, true},
    {32, 2, 19, true, true},
    {33, 2, 20, true, true},
    {34, 2, 21, true, true},
    {35, 2, 22, true, true},
    {43, 2, 23, true, true},
    {44, 2, 24, true, true},
    {45, 2, 25, true, true},
    {46, 2, 26, true, true},
    {47, 2, 27, true, true},
    {50, 7, 16, false, false},
    {68, 1, 72, false, false},
    {110, 1, 40, false, false},
    {137, 2, 29, true, true},
    {193, 2, 30, true, true},
    {194, 2, 31, true, true},
    {195, 9, 0, false, false},
    {196, 2, 32, true, true},
    {197, 2, 33, true, true},
    {198, 2, 33, true, true},
    {200, 2, 40, true, true},
    {720, 2, 35, true, true},
    {741, 2, 36, true, true},
    {742, 2, 37, true, true},
    {743, 2, 38, true, true},
    {744, 2, 39, true, true},
    {820, 2, 41, true, true},
    {821, 10, 0, false, false},
    {920, 2, 44, true, true},
    {2660012, 2, 45, true, true},
    {2660013, 2, 46, true, true},
    {2660050, 2, 47, true, true},
    {2660051, 2, 48, true, true},
    {2660235, 7, 16, false, false},
    {2660236, 7, 16, false, false},
    {2660239, 8, 1, false, false},
    {2660290, 2, 33, true, true},
    {2660296, 11, 8, false, false},
    {2660297, 11, 8, false, false},
    {2660358, 2, 33, true, true},
    {2660401, 2, 35, true, true},
    {2660402, 2, 38, true, true},
    {2660408, 2, 79, true, true},
    {2660409, 2, 76, true, true},
    {2660410, 2, 77, true, true},
    {2660411, 2, 78, true, true},
    {2660543, 2, 80, true, true},
    {2660544, 2, 83, true, true},
    {2660545, 2, 84, true, true},
    {2660546, 2, 87, true, true},
    {2660703, 2, 79, true, true},
    {2670009, 1, 40, false, false},
    {2670266, 2, 70, true, true},
    {2675025, 2, 17, true, true},
    {2675026, 2, 23, true, true},
    {2675028, 2, 31, true, true},
    {2675057, 4, 42, false, false},
    {2675058, 2, 43, true, true},
    {10000387, 2, 92, true, true},
    {10000388, 2, 89, true, true},
    {10000389, 2, 90, true, true},
    {10000390, 2, 91, true, true},
    {10000391, 2, 93, true, true},
    {690000221, 13, 10, true, true},
    {690000222, 13, 50, true, true},
    {690000223, 13, 100, true, true},
    {690000224, 13, 500, true, true},
    {690000225, 13, 1000, true, true},
};

const CItem* RelatedItem(const CCeraShopGoods* goods) {
    return goods ? reinterpret_cast<const CItem*>(
                       static_cast<uintptr_t>(goods->GetRelatedItem())) : 0;
}

bool FillAvatarPrices(const CCeraShopGoods* goods,
                      unsigned char avatar_select,
                      unsigned char select_type,
                      stAvatarTypeSelect_t& selection,
                      int& cera,
                      int& gold,
                      int& win_point) {
    const CItem* const related = RelatedItem(goods);
    if (!related || !related->vtable_ ||
        !related->vtable_->is_avatar_item ||
        !related->vtable_->is_avatar_item(related)) {
        return false;
    }
    if (!selection.init() || select_type > 0x63u) {
        return false;
    }
    const CEquipItem* const equipment =
        static_cast<const CEquipItem*>(related);
    const std::vector<stAvatarTypeSelect_t>& selections =
        equipment->avatar_type_selects_;
    if (avatar_select >= selections.size()) {
        return false;
    }
    selection = selections.at(avatar_select);
    cera = selection.cera_price;
    gold = selection.gold_price;
    win_point = selection.win_point_price;
    (void)select_type;
    return true;
}

void SetItemString(const CCeraShopGoods* goods, char* destination, size_t size) {
    if (!destination || !size) {
        return;
    }
    destination[0] = '\0';
    if (!goods) {
        return;
    }
    const int item_index = goods->GetItemIdx();
    const int stack_count = goods->GetStackCount();
    switch (item_index) {
    case 1:
        std::snprintf(destination, size, "Continue Coin %d", stack_count);
        return;
    case 202:
        std::snprintf(destination, size, "Mystic Coin %d", stack_count);
        return;
    case 8:
        std::snprintf(destination, size, "Hands of Remy %d", stack_count);
        return;
    case 15:
        std::snprintf(destination, size, "Kaleido Box %d", stack_count);
        return;
    case 1085:
        std::snprintf(destination, size, "Manaade %d", stack_count);
        return;
    case 14:
        std::snprintf(destination, size, "Golden Wax Candle %d", stack_count);
        return;
    case 216:
        std::snprintf(destination, size, "Mystic Cube %d", stack_count);
        return;
    default:
        break;
    }
    const CItem* const related = RelatedItem(goods);
    if (!related) {
        return;
    }
    const char* const item_name = related->GetItemName();
    if (!item_name) {
        return;
    }
    if (related->is_stackable() && stack_count > 1) {
        std::snprintf(destination, size, "%s Count %d", item_name,
                      stack_count);
    } else {
        std::strncpy(destination, item_name, size - 1);
        destination[size - 1] = '\0';
    }
}

}  // namespace

// IDA_EA: 0x817B0B6
// IDA_NAME: _ZN23tagPackageItemSelectionD2Ev
// DEMANGLED: tagPackageItemSelection::~tagPackageItemSelection()
tagPackageItemSelection::~tagPackageItemSelection() {}

// IDA_EA: 0x817A244
// IDA_NAME: _ZN8WongWork9CCeraShop21stCeraShopItemParam_tC2Ev
// DEMANGLED: WongWork::CCeraShop::stCeraShopItemParam_t::stCeraShopItemParam_t(void)
WongWork::CCeraShop::stCeraShopItemParam_t::stCeraShopItemParam_t()
    : selected_items(), package_item_selections() {
    selected_items.clear();
    package_item_selections.clear();
}

// IDA_EA: 0x817A2FC
// IDA_NAME: _ZN8WongWork9CCeraShop21stCeraShopItemParam_tD2Ev
// DEMANGLED: WongWork::CCeraShop::stCeraShopItemParam_t::~stCeraShopItemParam_t()
WongWork::CCeraShop::stCeraShopItemParam_t::~stCeraShopItemParam_t() {}

// IDA_EA: 0x8325A30
// IDA_NAME: _ZN8WongWork9CCeraShop15SaveFeaturedIdxEP5CUseriii
// DEMANGLED: WongWork::CCeraShop::SaveFeaturedIdx(CUser *,int,int,int)
void WongWork::CCeraShop::SaveFeaturedIdx(CUser* user,
                                          int featured_index,
                                          int current_seconds,
                                          int source) {
    if (!user) {
        return;
    }
    DB_SaveFeaturedGoodsIdx::makeRequest(user->get_acc_id(),
                                         user->get_charac_no(-1),
                                         featured_index,
                                         current_seconds,
                                         source);
}

// IDA_EA: 0x8323978
// IDA_NAME: _ZN8WongWork9CCeraShop12ProcessErrorEP5CUsertijiic
// DEMANGLED: WongWork::CCeraShop::ProcessError(CUser *,unsigned short,int,unsigned int,int,int,char)
void WongWork::CCeraShop::ProcessError(CUser* user,
                                       unsigned short error,
                                       int result,
                                       unsigned int goods_no,
                                       int raw_1,
                                       int raw_2,
                                       char gift) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_BUY_CERASHOP_ITEM);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_short(error);
    packet.put_byte(gift);
    packet.put_int(result);
    packet.put_int(static_cast<int>(goods_no));
    packet.put_int(raw_1);
    packet.put_int(raw_2);
    packet.put_int(-1);
    packet.finalize(true);
    user->Send(packet);

    cMyTrace trace(
        "void WongWork::CCeraShop::ProcessError(CUser*, short unsigned int, int, unsigned int, int, int, char)",
        3114,
        0);
    trace("NEXON BILLING : Buy Error(%d,%u)", error);
}

// IDA_EA: 0x8323F2A
// IDA_NAME: _ZN8WongWork9CCeraShop20checkBuyingConditionEP5CUserjmjtht
// DEMANGLED: WongWork::CCeraShop::checkBuyingCondition(CUser *,unsigned int,unsigned long,unsigned int,unsigned short,unsigned char,unsigned short)
int WongWork::CCeraShop::checkBuyingCondition(
    CUser* user,
    unsigned int ipg_no,
    unsigned long related_item,
    unsigned int related_item_count,
    unsigned short avatar_type_select,
    unsigned char raw_select,
    unsigned short raw_select_2) {
    return _CheckBuyingCondition(user,
                                 ipg_no,
                                 related_item,
                                 related_item_count,
                                 avatar_type_select,
                                 raw_select,
                                 raw_select_2);
}

// IDA_EA: 0x8324966
// IDA_NAME: _ZN8WongWork9CCeraShop26_CheckConditionPutonAvatarEP5CUser
// DEMANGLED: WongWork::CCeraShop::_CheckConditionPutonAvatar(CUser *)
int WongWork::CCeraShop::_CheckConditionPutonAvatar(CUser* user) {
    CInventory* const inventory = user->getCurCharacInvenR();
    for (int slot = oldsrv::config::kAvatarEquipmentTypeFirst;
         slot <= oldsrv::config::kAvatarEquipmentTypeLast;
         ++slot) {
        const Inven_Item* const item = inventory->GetInvenRef(0, slot);
        if (item && item->getKey()) {
            return 1;
        }
    }

    for (int slot = 0;
         slot <= oldsrv::config::kAvatarInventoryLastSlot;
         ++slot) {
        const Inven_Item* const item = inventory->GetInvenRef(2, slot);
        if (item && item->getKey()) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8323F86
// IDA_NAME: _ZN8WongWork9CCeraShop21_CheckBuyingConditionEP5CUserjmjtht
// DEMANGLED: WongWork::CCeraShop::_CheckBuyingCondition(CUser *,unsigned int,unsigned long,unsigned int,unsigned short,unsigned char,unsigned short)
int WongWork::CCeraShop::_CheckBuyingCondition(
    CUser* user,
    unsigned int ipg_no,
    unsigned long related_item,
    unsigned int related_item_count,
    unsigned short avatar_type_select,
    unsigned char raw_select,
    unsigned short raw_select_2) {
    (void)related_item_count;
    (void)raw_select;

    int condition = 0;
    if (ipg_no == 110013u) {
        condition = 1;
    } else if (ipg_no == 115004u) {
        condition = 3;
    }

    const unsigned int item_id = static_cast<unsigned int>(related_item);
    if (item_id != 1u) {
        CItem* const item = G_CDataManager()->find_item(
            static_cast<int>(item_id));
        if (!item) {
            return 0;
        }
    if (item->vtable_->is_avatar_item(item)) {
            const CEquipItem* const equipment =
                static_cast<const CEquipItem*>(item);
            if (!raw_select_2) {
                return 0;
            }
            const std::vector<stAvatarTypeSelect_t>* const avatar_select =
                reinterpret_cast<const std::vector<stAvatarTypeSelect_t>*>(
                    equipment->getAvatarTypeSelect());
            if (raw_select_2 >= avatar_select->size() ||
                raw_select_2 != avatar_type_select) {
                return 0;
            }
            const int socket_count =
                avatar_select->at(raw_select_2).socket_count;
            if (avatar_type_select != 4u && socket_count) {
                return 0;
            }
        }
    }

    // The item-index decision tree is the one at 0x8323F86.  It runs after
    // the two IPG rules above and deliberately overwrites their condition.
    switch (item_id) {
    case 50u:
    case 57u:
    case 58u:
    case 59u:
    case 60u:
    case 61u:
    case 62u:
    case 2660235u:
    case 2660236u:
        condition = 4;
        break;
    case 51u:
    case 2660234u:
        condition = 5;
        break;
    case 63u:
    case 64u:
    case 65u:
    case 66u:
    case 67u:
    case 69u:
    case 70u:
    case 71u:
    case 72u:
    case 73u:
        condition = 6;
        break;
    case 740u:
    case 821u:
    case 928u:
    case 7906u:
    case 2660043u:
        condition = 11;
        break;
    case 2660239u:
        condition = 12;
        break;
    case 2660296u:
    case 2660297u:
        condition = 13;
        break;
    case 2660701u:
    case 2660702u:
    case 2660779u:
        condition = 16;
        break;
    default:
        if (item_id >= 839u && item_id <= 841u) {
            condition = 11;
        }
        break;
    }

    if (!condition) {
        return 1;
    }

    CInventory* const inventory = user->getCurCharacInvenR();
    CCargo* const cargo = user->getCurCharacCargoR();
    switch (condition) {
    case 1:
        return _CheckConditionPutonAvatar(user);
    case 3: {
        const int created_at = user->getCurCharacCreateTime();
        return created_at >= 1166648400 && created_at < 1169067600;
    }
    case 4: {
        int required_capacity = -1;
        switch (item_id) {
        case 50u:
            required_capacity = 8;
            break;
        case 57u:
            required_capacity = 24;
            break;
        case 58u:
            required_capacity = 40;
            break;
        case 59u:
            required_capacity = 56;
            break;
        case 60u:
            required_capacity = 72;
            break;
        case 61u:
            required_capacity = 88;
            break;
        case 62u:
            required_capacity = 104;
            break;
        case 2660235u:
            required_capacity = 120;
            break;
        case 2660236u:
            required_capacity = 136;
            break;
        default:
            return 0;
        }
        return cargo->GetCapacity() == required_capacity;
    }
    case 5: {
        if (inventory->check_item_exist(static_cast<int>(item_id)) != -1 ||
            cargo->check_item_exist(static_cast<int>(item_id)) != -1) {
            return 0;
        }
        Inven_Item candidate;
        candidate.item_id_ = static_cast<int>(item_id);
        G_CDataManager()->get_item_list()->create_item(
            ENUM_ITEM_CREATE_TYPE_DEFAULT, candidate, 1);
        return inventory->tryInsertItemIntoInventory(candidate) >= 0;
    }
    case 6: {
        const int created_at = user->getCurCharacCreateTime();
        if (created_at < 1185397200 || created_at >= 1188421200) {
            return 0;
        }
        const int event_items[] = {26070, 26071, 26072, 26073, 26074};
        for (unsigned int index = 0;
             index != sizeof(event_items) / sizeof(event_items[0]);
             ++index) {
            if (inventory->check_item_exist(event_items[index]) != -1 ||
                cargo->check_item_exist(event_items[index]) != -1) {
                return 0;
            }
        }
        return 1;
    }
    case 8:
        return user->get_charac_level() > 14;
    case 11:
        return user->GetCurCharacSkillTreeIndex() == -1;
    case 12: {
        CEnvironment* const environment = G_CEnvironment();
        if (environment->get_server_type() == 2) {
            return 0;
        }
        switch (environment->get_server_group()) {
        case 5:
        case 11:
        case 12:
            return 0;
        default:
            return static_cast<unsigned char>(user->getCharacSlotLimit()) <=
                   0x14u;
        }
    }
    case 13: {
        const int inventory_capacity = inventory->get_inventory_capacity();
        if (item_id == 2660296u) {
            return inventory_capacity == 0;
        }
        return item_id == 2660297u && inventory_capacity == 8;
    }
    case 16:
        if (user->get_charac_job() ==
                oldsrv::config::kCharacterJobDemonicSwordman ||
            user->get_charac_job() ==
                oldsrv::config::kCharacterJobCreatorMage) {
            return 0;
        }
        return user->verifyStackableItemActionPlace(
            static_cast<StackableAction::Type>(46), item_id);
    default:
        return 0;
    }
}

// IDA_EA: 0x8324A1E
// IDA_NAME: _ZN8WongWork9CCeraShop13IsSpecialItemEiRiS1_Rb
// DEMANGLED: WongWork::CCeraShop::IsSpecialItem(int,int &,int &,bool &)
int WongWork::CCeraShop::IsSpecialItem(int item_id,
                                       int& special_type,
                                       int& special_value,
                                       bool& unique) {
    if (item_id >= 57 && item_id <= 62) {
        special_type = 7;
        special_value = 16;
        return 1;
    }

    for (size_t i = 0;
         i < sizeof(kSpecialItemRules) / sizeof(kSpecialItemRules[0]);
         ++i) {
        if (kSpecialItemRules[i].item_id == item_id) {
            special_type = kSpecialItemRules[i].special_type;
            special_value = kSpecialItemRules[i].special_value;
            if (kSpecialItemRules[i].writes_unique) {
                unique = kSpecialItemRules[i].unique;
            }
            return 1;
        }
    }

    CDataManager* const data_manager = G_CDataManager();
    CItem* const item = data_manager ? data_manager->find_item(item_id) : 0;
    if (!item || !static_cast<unsigned char>(item->is_stackable())) {
        return 0;
    }

    typedef int (*GetTypeFunction)(const CItem*);
    void* const* vtable = *reinterpret_cast<void* const* const*>(item);
    const int stackable_type =
        reinterpret_cast<GetTypeFunction>(vtable[3])(item);
    if (stackable_type == 38) {
        return 0;
    }
    if (stackable_type == 14) {
        special_type = 5;
        unique = true;
        return 1;
    }
    if (stackable_type == 15 || stackable_type == 16) {
        special_type = 6;
        unique = true;
        return 1;
    }
    if (stackable_type == 34) {
        special_type = 12;
        return 1;
    }
    return 0;
}

// IDA_EA: 0x81769F6
// IDA_NAME: _ZN18ItemVendingMachine19ProcessBuyRequestExEP5CUserRK21MSG_BUY_CERASHOP_ITEM
// DEMANGLED: ItemVendingMachine::ProcessBuyRequestEx(CUser *,MSG_BUY_CERASHOP_ITEM const&)
int ItemVendingMachine::ProcessBuyRequestEx(
    CUser* user,
    const MSG_BUY_CERASHOP_ITEM& message) {
    if (!user) {
        return 0;
    }

    const unsigned int item_count = message.item_count;

    const CCeraShopGoods* goods[36] = {};
    int group_index = -1;
    int total_gold = 0;
    int total_win_point = 0;
    unsigned int total_cera = 0;
    int total_mileage = 0;
    user->resetUsedCera();

    for (unsigned int index = 0; index < item_count; ++index) {
        const CeraShopBuyItem& item = message.items[index];
        goods[index] = G_CDataManager()->FindGoods(item.goods_no);
        if (!goods[index]) {
            cera_shop_.ProcessError(user,
                                    0x15u,
                                    group_index,
                                    item.goods_no,
                                    static_cast<int>(index + 1),
                                    static_cast<char>(item_count),
                                    static_cast<char>(message.gift));
            continue;
        }

        group_index = goods[index]->GetGroupIdx();
        if (!user->IsGameMasterMode() &&
            (goods[index]->GetSticker() == 99 ||
             (message.gift == 1 && goods[index]->isGiftDisallowance_()) ||
             (message.gift != 1 &&
              cash_shop_0828::is_gift_only_goods(item.goods_no)))) {
            cera_shop_.ProcessError(user,
                                    8u,
                                    group_index,
                                    item.goods_no,
                                    static_cast<int>(index + 1),
                                    static_cast<char>(item_count),
                                    static_cast<char>(message.gift));
            goods[index] = 0;
            continue;
        }

        // IDA_EA: 0x8176D0A.  This restriction gate precedes the ordinary
        // featured-goods eligibility test and records the pending purchase
        // count in CUser's named CeraShop restriction maps.
        if (!CerashopAddRestrict::Manager::IsBuyableRestrictItem(
                user, item.goods_no, 1, static_cast<int>(index))) {
            cera_shop_.ProcessError(user,
                                    0x71u,
                                    group_index,
                                    item.goods_no,
                                    static_cast<int>(index + 1),
                                    static_cast<char>(item_count),
                                    static_cast<char>(message.gift));
            goods[index] = 0;
            continue;
        }

        if (group_index != 1 &&
            !(group_index == 11 && goods[index]->GetSubGroupIndex() == 1) &&
            goods[index]->GetRelatedItem() == 0) {
            cera_shop_.ProcessError(user,
                                    0x11u,
                                    group_index,
                                    item.goods_no,
                                    static_cast<int>(index + 1),
                                    static_cast<char>(item_count),
                                    static_cast<char>(message.gift));
            goods[index] = 0;
            continue;
        }

        int condition_error = 0;
        if (!cera_shop_._CheckBuyingCondition(
                user,
                goods[index],
                item.goods_no,
                item.select_type,
                item.avatar_select,
                static_cast<char>(message.gift),
                condition_error)) {
            cera_shop_.ProcessError(user,
                                    static_cast<unsigned short>(condition_error),
                                    group_index,
                                    item.goods_no,
                                    static_cast<int>(index + 1),
                                    static_cast<char>(item_count),
                                    static_cast<char>(message.gift));
            goods[index] = 0;
            continue;
        }

        int cera = 0;
        int gold = 0;
        int win_point = 0;
        stAvatarTypeSelect_t avatar_selection;
        if (group_index) {
            cera = goods[index]->GetCeraPrice();
            gold = goods[index]->GetGoldPrice();
            win_point = goods[index]->GetWinPoint();
        } else if (!FillAvatarPrices(goods[index],
                                     item.avatar_select,
                                     item.select_type,
                                     avatar_selection,
                                     cera,
                                     gold,
                                     win_point)) {
            cera_shop_.ProcessError(user,
                                    0x17u,
                                    group_index,
                                    item.goods_no,
                                    static_cast<int>(index + 1),
                                    static_cast<char>(item_count),
                                    static_cast<char>(message.gift));
            goods[index] = 0;
            continue;
        }

        total_cera += static_cast<unsigned int>(cera);
        total_gold += gold;
        total_win_point += win_point;
        total_mileage += goods[index]->GetMileagePrice();
        if (message.buy_type == 1 && goods[index]->isCeraPointDisallowance_()) {
            cera_shop_.ProcessError(user,
                                    0x138bu,
                                    group_index,
                                    item.goods_no,
                                    static_cast<int>(index + 1),
                                    static_cast<char>(item_count),
                                    static_cast<char>(message.gift));
            return 1;
        }
    }

    if (!cera_shop_._CheckValidCount(goods)) {
        cera_shop_.ProcessError(user,
                                1u,
                                -1,
                                item_count ? message.items[0].goods_no : 0,
                                1,
                                1,
                                static_cast<char>(message.gift));
        return 1;
    }

    int error = 0;
    if (user->GetCera() < static_cast<int>(total_cera)) {
        error = 11;
    } else if (user->getCurCharacMoney() < total_gold) {
        error = 10;
    } else if (user->GetWinPoint() < total_win_point) {
        error = 12;
    } else if (user->GetMileage() < total_mileage) {
        error = 1005;
    }
    if (message.buy_type == 1) {
        error = user->GetCeraPoint() < static_cast<int>(total_cera) ? 5002 : 0;
    }
    if (error) {
        cera_shop_.ProcessError(user,
                                static_cast<unsigned short>(error),
                                -1,
                                0,
                                1,
                                1,
                                static_cast<char>(message.gift));
        return 1;
    }

    int available_gold = user->getCurCharacMoney();
    int available_win_point = user->GetWinPoint();
    int available_mileage = user->GetMileage();
    int available_cera = user->GetCera();
    int available_cera_point = user->GetCeraPoint();
    const int current_seconds = GlobalData::s_systemTime_.getCurSec();

    for (unsigned int index = 0; index < item_count; ++index) {
        if (!goods[index]) {
            continue;
        }
        const CeraShopBuyItem& item = message.items[index];
        const int current_group = goods[index]->GetGroupIdx();
        int cera = 0;
        int gold = 0;
        int win_point = 0;
        stAvatarTypeSelect_t avatar_selection;
        if (current_group) {
            cera = goods[index]->GetCeraPrice();
            gold = goods[index]->GetGoldPrice();
            win_point = goods[index]->GetWinPoint();
        } else if (!FillAvatarPrices(goods[index],
                                     item.avatar_select,
                                     item.select_type,
                                     avatar_selection,
                                     cera,
                                     gold,
                                     win_point)) {
            cera_shop_.ProcessError(user,
                                    0x17u,
                                    current_group,
                                    item.goods_no,
                                    static_cast<int>(index + 1),
                                    static_cast<char>(item_count),
                                    static_cast<char>(message.gift));
            goods[index] = 0;
            continue;
        }

        const int mileage = goods[index]->GetMileagePrice();
        int item_error = 0;
        if (message.buy_type == 1) {
            if (available_cera_point < cera) item_error = 5002;
        } else if (available_cera < cera) {
            item_error = 11;
        }
        if (available_gold < gold) item_error = 10;
        if (available_win_point < win_point) item_error = 12;
        if (available_mileage < mileage) item_error = 1005;
        if (user->IsRestrictedGoods(goods[index]->GetFeaturedIdx())) {
            item_error = 0x14;
        }
        if (item_error) {
            cera_shop_.ProcessError(user,
                                    static_cast<unsigned short>(item_error),
                                    current_group,
                                    item.goods_no,
                                    static_cast<int>(index + 1),
                                    static_cast<char>(item_count),
                                    static_cast<char>(message.gift));
            goods[index] = 0;
            continue;
        }

        if (message.buy_type == 1) {
            available_cera_point -= cera;
        } else {
            available_cera -= cera;
        }
        available_gold -= gold;
        available_win_point -= win_point;
        available_mileage -= mileage;

        CStreamGuard guard(
            GlobalData::s_stream_pool->Acquire(
                "localjapan/AppendVendingMachine.cpp", 971),
            false);
        stCeraShopIPGRequest_t& request =
            *guard.GetInBuffer<stCeraShopIPGRequest_t>();
        // 0x8177D74..0x8177FED writes only these fields.  In particular it
        // does not clear the acquired Stream record before populating it.
        request.goods_no = item.goods_no;
        request.group_index = static_cast<uint32_t>(current_group);
        request.item_index = static_cast<uint32_t>(goods[index]->GetItemIdx());
        request.stack_count = static_cast<uint16_t>(goods[index]->GetStackCount());
        request.cera_price = cera;
        request.win_point = win_point;
        request.gold_price = gold;
        request.avatar_selection_value =
            current_group ? 0 : avatar_selection.usable_period;
        request.item_count = item_count;
        request.item_order = index + 1;
        request.featured_index =
            static_cast<uint32_t>(goods[index]->GetFeaturedIdx());
        request.select_type = item.select_type;
        request.avatar_select = item.avatar_select;
        request.current_seconds = current_seconds;
        request.gift = message.gift;
        request.post_recipient_wire_word = message.post_recipient_wire_word;
        request.buy_type = message.buy_type;
        std::memcpy(request.recipient_name_suffix,
                    message.receiver_name + 26,
                    sizeof(request.recipient_name_suffix));
        request.option_count = item.option_count;
        for (unsigned int option = 0; option < item.option_count && option < 35u;
             ++option) {
            request.options[option].item_id = item.options[option].item_id;
            request.options[option].selected = item.options[option].select;
        }
        request.additional_item_count = item.additional_item_count;
        for (unsigned int additional = 0;
             additional < item.additional_item_count && additional < 35u;
             ++additional) {
            request.additional_items[additional].item_id =
                item.additional_items[additional].item_id;
            request.additional_items[additional].first_option =
                item.additional_items[additional].first_option;
            request.additional_items[additional].second_option =
                item.additional_items[additional].second_option;
        }

        char product_id[20] = {};
        char price[36] = {};
        char order_id[8] = {};
        char description[1024] = {};
        char unique_no[21] = {};
        std::snprintf(product_id, sizeof(product_id), "%d",
                      goods[index]->GetItemIdx());
        std::snprintf(price, sizeof(price), "%d", cera);
        std::snprintf(order_id, sizeof(order_id), "%u", item.goods_no);
        SetItemString(goods[index], description, sizeof(description));
        WongWork::CGenUniqueNo::GenUniqueNo(unique_no);

        const char* const character_name = user->getCurCharacName();
        if (cera) {
            if (message.buy_type == 1) {
                GlobalData::s_pIPGHelper->IPGOutputPoint(
                    user,
                    order_id,
                    price,
                    unique_no,
                    description,
                    character_name,
                    guard.Get());
            } else {
                GlobalData::s_pIPGHelper->IPGOutput(
                    user,
                    order_id,
                    price,
                    unique_no,
                    description,
                    character_name,
                    guard.Get());
            }
        } else {
            GlobalData::s_pIPGHelper->IPGOutputSimulation(
                user,
                order_id,
                price,
                unique_no,
                description,
                character_name,
                guard.Get());
        }
    }
    return 1;
}

// IDA_EA: 0x854C4D0
// IDA_NAME: _ZN18ItemVendingMachine7BuyItemEP5CUserRK21MSG_BUY_CERASHOP_ITEM
// DEMANGLED: ItemVendingMachine::BuyItem(CUser *,MSG_BUY_CERASHOP_ITEM const&)
int ItemVendingMachine::BuyItem(CUser* user,
                                const MSG_BUY_CERASHOP_ITEM& message) {
    ProcessBuyRequestEx(user, message);
    return 1;
}
