#include "old_game/economy/inventory/inventory.h"

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/global_tools.h"
#include "common/manager/money_log_abi.h"
#include "common/manager/os_api.h"
#include "common/manager/statistics_runtime.h"
#include "common/manager/user.h"
#include "common/manager/global_data.h"
#include "old_data/db/avatar_aura/avatar_item_payload.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "old_game/economy/inventory/slot_bound_checker.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/user_history_log_abi.h"
#include "old_game/player/core/cuser.h"
#include "old_game/activity/schedule/shooting_star_dimension_pocket.h"
#include "infra/security/policy/secu_gold_control_abi.h"

#include <stdint.h>


class CItem;

CDataManager* G_CDataManager();
extern int checkStackableLimit(unsigned long item_id, int stack_count)
    __asm__("_Z19checkStackableLimitmi");
extern int isGainedGoldFromDungeonReason(int reason)
    __asm__("_Z29isGainedGoldFromDungeonReason15eMoneyAddReason");
namespace {

// IDA 0x8508C2E writes only the leading delete-event fields before copying
// the complete 0x70C-byte stack payload to the DB stream.  The remaining
// bytes have no recovered field layout, so they stay opaque rather than being
// invented as a synthetic SIG_AVATAR_DELETE structure.
#pragma pack(push, 1)
struct AvatarDeleteEventOpaque {
    uint32_t event_type;  // +0x00: IDA writes 1.
    uint32_t charac_no;   // +0x04
    uint8_t slot;         // +0x08
    uint8_t opaque_tail[0x70Cu - 9u];
};
#pragma pack(pop)

static_assert(sizeof(AvatarDeleteEventOpaque) == 0x70Cu,
              "Avatar delete event must retain the IDA stream size");

int FinishInventoryInsert(CUser* user,
                              const Inven_Item& item,
                              int inserted_slot,
                              bool committed_grant) {
    if (inserted_slot >= 0 && committed_grant && user &&
        item.item_id_ == 690000771 &&
        item.get_add_info() > 0) {
        const uint32_t actual_added =
            static_cast<uint32_t>(item.get_add_info());
        // NOTI694 is a transient visual batch counter.  The client stores one
        // u8 and overwrites (rather than accumulates) it, so a large grant is
        // represented by one saturated visual batch while inventory remains
        // authoritative for the full amount.
        shooting_star_dimension_pocket::NotifyShootingStarReceived(
            user, static_cast<uint8_t>(actual_added > 255u
                                           ? 255u : actual_added));
    }
    return inserted_slot;
}

}  // namespace

// IDA_EA: 0x8509B9E
// IDA_NAME: _ZN10CInventory13AddAvatarItemEiibccPKc20eAvatarItemAddReasonbt
// DEMANGLED: CInventory::AddAvatarItem(int,int,bool,char,char,char const*,eAvatarItemAddReason,bool,unsigned short)
int CInventory::AddAvatarItem(int item_id,
                              int usable_period,
                              bool flag,
                              char first_flag,
                              char avatar_select,
                              const char* ipg_no,
                              eAvatarItemAddReason reason,
                              bool account_bound,
                              uint16_t hidden_option) {
    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item || !item->vtable_->is_avatar_item(item)) {
        return -1;
    }

    stAvatarEmblemInfo_t emblem;
    emblem.init();
    static_cast<const CEquipItem*>(item)->getAvatarSocket(avatar_select,
                                                          emblem);

    Inven_Item* const avatar_table = avatar_items_;
    int slot = 0;
    for (; slot <= oldsrv::config::kAvatarInventoryLastSlot; ++slot) {
        if (!avatar_table[slot].item_id_) {
            break;
        }
    }
    if (slot > oldsrv::config::kAvatarInventoryLastSlot) {
        return -1;
    }

    Inven_Item* const avatar = &avatar_table[slot];
    WongWork::CAvatarItemMgr* const avatar_manager = GetAvatarItemMgrW();
    const int avatar_key = avatar_manager->RegistNotAssignedAvatarItem(
        usable_period, OS_API::GetDateTimeTick(), ipg_no, account_bound,
        emblem);
    avatar->item_id_ = item_id;
    avatar->add_info_ = avatar_key;
    avatar->item_category_ = 8;
    avatar->avatar_select_ = static_cast<uint16_t>(first_flag);
    avatar->seal_or_state_ = 0;
    avatar->extend_info_ = 0;
    if (item->IsHiddenOption()) {
        avatar->extend_info_ = static_cast<int32_t>(hidden_option);
    }
    avatar->ResetItemAttr();

    stAvatarExpansionInfo_t expansion;
    expansion.init();
    const unsigned long add_info = static_cast<unsigned long>(
        static_cast<uint16_t>(avatar->extend_info_));
    _SaveAvatarItem(slot,
                    avatar_manager->GetExpireDate(avatar_key),
                    flag,
                    first_flag,
                    emblem,
                    ipg_no,
                    add_info,
                    expansion);

    CUser_history_log(parent_)->AvatarItemAdd(
        item_id,
        -1,
        ipg_no,
        reason);
    return slot;
}

// IDA_EA: 0x8509E78
// IDA_NAME: _ZN10CInventory15_SaveAvatarItemEiibcRK20stAvatarEmblemInfo_tPKcmRK23stAvatarExpansionInfo_t
// DEMANGLED: CInventory::_SaveAvatarItem(int,int,bool,char,stAvatarEmblemInfo_t const&,char const*,unsigned long,stAvatarExpansionInfo_t const&)
int CInventory::_SaveAvatarItem(int slot,
                                int expire_date,
                                bool flag,
                                char first_flag,
                                const stAvatarEmblemInfo_t& emblem,
                                const char* ipg_no,
                                unsigned long add_info,
                                const stAvatarExpansionInfo_t& expansion) {
    CUser* const user = parent_;
    if (!user) {
        return 0;
    }

    CStreamGuard guard(GlobalData::stream_pool()->Acquire("inventory.cpp", 9056),
                       true);
    *guard << 49;
    *guard << user->GetUID();
    SIG_AVATAR_INSERT* const payload =
        guard.GetInBuffer<SIG_AVATAR_INSERT>();
    const uint32_t item_id = static_cast<uint32_t>(avatar_items_[slot].item_id_);
    payload->set(user->get_charac_no(-1),
                 item_id,
                 slot + oldsrv::config::kAvatarInventorySlotBase,
                 expire_date,
                 flag,
                 first_flag,
                 ipg_no,
                 0,
                 add_info);
    payload->setJewelSocket(emblem);
    payload->expansion_info = expansion;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x8508C2E
// IDA_NAME: _ZNK10CInventory15SendAvatarEventEiiiii
// DEMANGLED: CInventory::SendAvatarEvent(int,int,int,int,int)const
void CInventory::SendAvatarEvent(int source,
                                 int first_slot,
                                 int second_slot,
                                 int value_1,
                                 int value_2) const {
    CUser* const user = parent_;
    if (!user) {
        cMyTrace trace(
            "void CInventory::SendAvatarEvent(int, int, int, int, int) const",
            8490,
            5);
        trace("CInventory::SendAvatarEven ERROR, m_pParent NULL ");
        return;
    }

    CStreamGuard guard(GlobalData::stream_pool()->Acquire("inventory.cpp", 8495),
                       true);
    const int charac_no = user->get_charac_no(-1);
    *guard << source;
    *guard << user->GetUID();

    if (source == 52) {
        SIG_AVATAR_RENEW payload;
        payload.charac_no = static_cast<uint32_t>(charac_no);
        payload.slot = static_cast<uint8_t>(first_slot);
        payload.days = static_cast<uint16_t>(second_slot);
        guard->put_binary(&payload, sizeof(payload));
    } else if (source > 52) {
        switch (source) {
            case 707: {
                SIG_AVATAR_OPTION_CHANGE* const payload =
                    guard.GetInBuffer<SIG_AVATAR_OPTION_CHANGE>();
                payload->charac_no = static_cast<uint32_t>(charac_no);
                payload->ui_id = static_cast<uint32_t>(first_slot);
                payload->ability_no = static_cast<uint8_t>(second_slot);
                break;
            }
            case 794: {
                SIG_AVATAR_COLOR_CHANGE* const payload =
                    guard.GetInBuffer<SIG_AVATAR_COLOR_CHANGE>();
                payload->charac_no = static_cast<uint32_t>(charac_no);
                payload->ui_id = static_cast<uint32_t>(first_slot);
                payload->color_1 = static_cast<uint16_t>(second_slot);
                payload->color_2 = static_cast<uint16_t>(value_1);
                break;
            }
            case 54: {
                SIG_AVATAR_CHANGE_STAT* const payload =
                    guard.GetInBuffer<SIG_AVATAR_CHANGE_STAT>();
                payload->ui_id = static_cast<uint32_t>(first_slot);
                payload->stat_or_item_id.stat = second_slot;
                payload->slot = static_cast<uint8_t>(value_1);
                payload->charac_or_loaded.charac_no =
                    static_cast<uint32_t>(charac_no);
                break;
            }
        }
    } else if (source == 50) {
        SIG_AVATAR_MOVE payload;
        payload.unknown_00 = static_cast<uint32_t>(charac_no);
        payload.first_ui_id = value_1;
        payload.second_ui_id = value_2;
        payload.second_slot = static_cast<uint8_t>(first_slot);
        payload.first_slot = static_cast<uint8_t>(second_slot);
        payload.first_clear_avatar_id =
            GetClearAvatar(static_cast<uint8_t>(second_slot)) ? value_2 : 0;
        payload.second_clear_avatar_id =
            GetClearAvatar(static_cast<uint8_t>(first_slot)) ? value_1 : 0;
        guard->put_binary(&payload, sizeof(payload));
    } else if (source > 50) {
        AvatarDeleteEventOpaque payload;
        payload.event_type = 1;
        payload.charac_no = static_cast<uint32_t>(charac_no);
        payload.slot = static_cast<uint8_t>(first_slot);
        guard->put_binary(&payload, sizeof(payload));
    } else if (source == 49) {
        SIG_AVATAR_INSERT payload;
        payload.charac_no = charac_no;
        payload.item_id = static_cast<uint32_t>(first_slot);
        payload.slot = static_cast<uint8_t>(second_slot);
        payload.expire_time = value_1;
        // IDA uses a byte store here, rather than a C++ bool conversion.
        *reinterpret_cast<uint8_t*>(&payload.obtain_from) =
            static_cast<uint8_t>(value_2);
        guard->put_binary(&payload, sizeof(payload));
    }

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
namespace {

const char kEmptyAvatarIpgNo[] = "";

void WriteAvatarHistory(CUser* parent,
                        WongWork::CAvatarItemMgr* avatar_manager,
                        const Inven_Item& item,
                        eAvatarItemAddReason reason) {
    const char* const ipg_no =
        avatar_manager->GetIPGAgencyNo(item.get_add_info());
    CUser_history_log(parent)->AvatarItemAdd(
        item.item_id_,
        item.get_add_info(),
        ipg_no ? ipg_no : kEmptyAvatarIpgNo,
        reason);
}

}  // namespace

// IDA_EA: 0x850367C
// IDA_NAME: _ZN10CInventory25insertAvatarIntoInventoryE10Inven_Item20eAvatarItemAddReasonb
// DEMANGLED: CInventory::insertAvatarIntoInventory(Inven_Item,eAvatarItemAddReason,bool)
int CInventory::insertAvatarIntoInventory(Inven_Item item,
                                          eAvatarItemAddReason reason,
                                          bool log_history) {
    Inven_Item* const avatar_table = avatar_items_;
    for (int slot = 0;
         slot <= oldsrv::config::kAvatarInventoryLastSlot;
         ++slot) {
        Inven_Item* const target = &avatar_table[slot];
        if (!target->item_id_) {
            *target = item;
            if (log_history) {
                WriteAvatarHistory(parent_, &avatar_item_manager_, *target,
                                   reason);
            }
            return slot;
        }
    }
    return -1;
}

// IDA_EA: 0x85037D8
// IDA_NAME: _ZN10CInventory28insertAvatarIntoSpecificSlotE10Inven_Itemi20eAvatarItemAddReasonb
// DEMANGLED: CInventory::insertAvatarIntoSpecificSlot(Inven_Item,int,eAvatarItemAddReason,bool)
int CInventory::insertAvatarIntoSpecificSlot(Inven_Item item,
                                            int requested_slot,
                                            eAvatarItemAddReason reason,
                                            bool log_history) {
    if (requested_slot > oldsrv::config::kAvatarInventoryLastSlot) {
        return -1;
    }

    Inven_Item* const avatar_table = avatar_items_;
    int target_slot = requested_slot;
    Inven_Item* target = &avatar_table[target_slot];
    if (target->item_id_) {
        target_slot = -1;
        for (int slot = 0;
             slot <= oldsrv::config::kAvatarInventoryLastSlot;
             ++slot) {
            target = &avatar_table[slot];
            if (!target->item_id_) {
                target_slot = slot;
                break;
            }
        }
        if (target_slot < 0) {
            return -1;
        }
    }

    *target = item;
    if (log_history) {
        WriteAvatarHistory(parent_, &avatar_item_manager_, *target, reason);
    }
    return target_slot;
}

// IDA_EA: 0x822D67C
// IDA_NAME: _ZN10CInventory7SetCoinEj
// DEMANGLED: CInventory::SetCoin(unsigned int)
CInventory* CInventory::SetCoin(unsigned int coin) {
    coin_ = coin;
    return this;
}

// IDA_EA: 0x8501886
// IDA_NAME: _ZNK10CInventory11StartEndPosEN10Inven_Item9ITEM_TYPEERiS2_
// DEMANGLED: CInventory::StartEndPos(Inven_Item::ITEM_TYPE,int &,int &)const
int CInventory::StartEndPos(Inven_Item::ITEM_TYPE item_type,
                            int& start,
                            int& end) const {
    const int found =
        g_slotBoundChecker.get_slot_bound(item_type, inventory_capacity_,
                                          start, end);
    if (!found) {
        start = 0;
        end = 0;
        return 1;
    }
    return 0;
}

// IDA_EA: 0x85018D2
// IDA_NAME: _ZNK10CInventory11GetItemTypeEi
// DEMANGLED: CInventory::GetItemType(int)const
int CInventory::GetItemType(int item_id) const {
    CItem* item = G_CDataManager()->find_item(item_id);
    if (!item) {
        if (item_id) {
            LogManager::logFormat(
                1,
                "inventory.cpp",
                "Inven_Item::ITEM_TYPE CInventory::GetItemType(int) const",
                0x13C2,
                "No Item %d",
                item_id);
        }
        return 0;
    }

    if (item->vtable_->is_avatar_item(item)) {
        return 8;
    }
    if (item->is_stackable() == 1) {
        const int type = item->vtable_->get_item_type(item);
        if (type == 9) {
            return 7;
        }
        if (type == 1) {
            return 3;
        }
        if (type == 2) {
            return 10;
        }
        if (type == 8) {
            return 4;
        }
        if (type == 20) {
            return 9;
        }
        return 2;
    }
    const int type = item->vtable_->get_item_type(item);
    if (type == oldsrv::config::kEquipmentTypeCreature) {
        return 5;
    }
    if (type >= oldsrv::config::kEquipmentTypeArtifactRed &&
        type <= oldsrv::config::kEquipmentTypeArtifactGreen) {
        return 6;
    }
    return 1;
}

// IDA_EA: 0x8505038
// IDA_NAME: _ZNK10CInventory15QuickEmptyCountEv
// DEMANGLED: CInventory::QuickEmptyCount(void)const
int CInventory::QuickEmptyCount() const {
    int empty_count = 0;
    for (int slot = 3; slot <= 8; ++slot) {
        if (!stackable_items_[slot].item_id_) {
            ++empty_count;
        }
    }
    return empty_count;
}

// IDA_EA: 0x8505080
// IDA_NAME: _ZNK10CInventory15InvenEmptycountEN10Inven_Item9ITEM_TYPEE
// DEMANGLED: CInventory::InvenEmptycount(Inven_Item::ITEM_TYPE)const
int CInventory::InvenEmptycount(Inven_Item::ITEM_TYPE item_type) const {
    int start = 0;
    int end = 0;
    StartEndPos(item_type, start, end);

    const Inven_Item* items = 0;
    if (IsCreatureItemType(static_cast<int>(item_type))) {
        items = creature_items_;
    } else if (static_cast<int>(item_type) == 8) {
        items = avatar_items_;
        start = 0;
        end = oldsrv::config::kAvatarInventoryLastSlot;
    } else {
        items = stackable_items_;
    }

    int empty_count = 0;
    for (int slot = start; slot <= end; ++slot) {
        if (!items[slot].item_id_) {
            ++empty_count;
        }
    }
    return empty_count;
}

// IDA_EA: 0x8504F64
// IDA_NAME: _ZNK10CInventory17check_empty_countEN10Inven_Item9ITEM_TYPEEi
// DEMANGLED: CInventory::check_empty_count(Inven_Item::ITEM_TYPE,int)const
int CInventory::check_empty_count(Inven_Item::ITEM_TYPE item_type,
                                  int required_count) const {
    int start = 0;
    int end = 0;
    int empty_count = 0;
    StartEndPos(item_type, start, end);

    const Inven_Item* const items =
        IsCreatureItemType(static_cast<int>(item_type))
            ? creature_items_
            : stackable_items_;
    for (int slot = start; slot <= end; ++slot) {
        if (!items[slot].item_id_) {
            ++empty_count;
        }
        if (empty_count >= required_count) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8505172
// IDA_NAME: _ZNK10CInventory16check_item_existEi
// DEMANGLED: CInventory::check_item_exist(int)const
int CInventory::check_item_exist(int item_id) const {
    for (int slot = 3; slot <= 8; ++slot) {
        if (stackable_items_[slot].item_id_ == item_id) {
            return slot;
        }
    }

    const int item_type_value = GetItemType(item_id);
    if (!item_type_value) {
        LogManager::logFormat(1,
                              "inventory.cpp",
                              "int CInventory::check_item_exist(int) const",
                              0x1aa4,
                              "Unknown Type %d",
                              item_type_value);
        return -1;
    }

    int start = 0;
    int end = 0;
    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type_value),
                start,
                end);
    const Inven_Item* items = 0;
    if (IsCreatureItemType(item_type_value)) {
        items = creature_items_;
    } else {
        items = stackable_items_;
    }
    for (int slot = start; slot <= end; ++slot) {
        if (items[slot].item_id_ == item_id) {
            return slot;
        }
    }
    return -1;
}

// IDA_EA: 0x84FBF2C
// IDA_NAME: _ZNK10CInventory12GetInvenDataEiR10Inven_Item
// DEMANGLED: CInventory::GetInvenData(int,Inven_Item &)const
int CInventory::GetInvenData(int item_id, Inven_Item& out) const {
    const int item_type = GetItemType(item_id);
    if (!item_type) {
        return -1;
    }

    int start = 0;
    int end = 0;
    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item_type), start, end);
    if (IsCreatureItemType(item_type)) {
        for (int slot = start; slot <= end; ++slot) {
            const Inven_Item* const item = &creature_items_[slot];
            if (static_cast<uint32_t>(item->item_id_) ==
                static_cast<uint32_t>(item_id)) {
                out = *item;
                return slot;
            }
        }
        return -1;
    }

    for (int slot = start; slot <= end; ++slot) {
        const Inven_Item* const item = &stackable_items_[slot];
        if (static_cast<uint32_t>(item->item_id_) ==
            static_cast<uint32_t>(item_id)) {
            out = *item;
            return slot;
        }
    }

    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(0), start, end);
    for (int slot = start; slot <= end; ++slot) {
        const Inven_Item* const item = &stackable_items_[slot];
        if (static_cast<uint32_t>(item->item_id_) ==
            static_cast<uint32_t>(item_id)) {
            out = *item;
            return slot;
        }
    }
    return -1;
}

// IDA_EA: 0x84FF762
// IDA_NAME: _ZN10CInventory9use_equipEi
// DEMANGLED: CInventory::use_equip(int)
int CInventory::use_equip(int slot) {
    if (slot < 0 || slot >= oldsrv::config::kEquipmentTypeCount) {
        return 1;
    }

    Inven_Item* const item = &equipment_items_[slot];
    if (item->item_category_ != 1) {
        return 19;
    }

    const unsigned int item_id = static_cast<uint32_t>(item->item_id_);
    CEquipItem* data_item = static_cast<CEquipItem*>(
        G_CDataManager()->find_item(item_id));
    if (!data_item) {
        return 17;
    }
    if (!item->avatar_select_ ||
        !data_item->get_endurance()) {
        return 11;
    }
    if (data_item->item_type_ <=
        oldsrv::config::kAvatarEquipmentTypeLast) {
        return 11;
    }
    if (!item_id) {
        return 17;
    }
    if (!item->avatar_select_) {
        return 22;
    }
    --item->avatar_select_;
    return 0;
}

// IDA_EA: 0x85000AE
// IDA_NAME: _ZN10CInventory11update_itemE10INVEN_TYPEi10Inven_Item
// DEMANGLED: CInventory::update_item(INVEN_TYPE,int,Inven_Item)
int CInventory::update_item(INVEN_TYPE inventory_type,
                            int slot,
                            Inven_Item item) {
    const int inventory_type_value = static_cast<int>(inventory_type);
    int result = CheckValidSlot(inventory_type_value, slot) ^ 1;
    if (!static_cast<unsigned char>(result)) {
        Inven_Item* const target = GetInvenRef(inventory_type_value, slot);
        *target = item;
        result = inventory_type_value;
    }
    return result;
}

// IDA_EA: 0x84FF29C
// IDA_NAME: _ZN10CInventory10gain_moneyEi15eMoneyAddReasonbi
// DEMANGLED: CInventory::gain_money(int,eMoneyAddReason,bool,int)
int CInventory::gain_money(int money,
                            eMoneyAddReason reason,
                            bool log_history,
                            int excluded_money) {
    int money_limit = 0x7fffffff;
    CUser* const user = parent_;
    if (user) {
        money_limit = G_CDataManager()->GetMoneyLimitPerLevel(
            user->get_charac_level(), user->get_acc_name());
    }
    const int before_money = static_cast<int>(money_);
    const unsigned int added_money = static_cast<unsigned int>(money);
    if (static_cast<int>(static_cast<unsigned int>(money_limit) - added_money) >=
        static_cast<int>(money_)) {
        money_ += added_money;
    } else {
        money_ = static_cast<uint32_t>(money_limit);
        const int charac_level = user->get_charac_level();
        const int charac_no = user->get_charac_no(-1);
        cMyTrace trace("int CInventory::gain_money(int, eMoneyAddReason, bool, int)",
                       3825, 4);
        trace("over money limit:m_id(%s),charac_no(%d),level(%d) gain_money inven(%d)+add(%d)",
              NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
              charac_no, charac_level, before_money, money);
    }
    if (user && isGainedGoldFromDungeonReason(static_cast<int>(reason))) {
        CDungeonGainedGold* const dungeon_gold =
            reinterpret_cast<CDungeonGainedGold*>(user->getDungeonGainedGold());
        const unsigned int hack_count = dungeon_gold->incGainedGold(added_money);
        for (unsigned int index = 0; index < hack_count; ++index) {
            reinterpret_cast<WongWork::CHackAnalyzer*>(user->getHackAnalyzer())
                ->addServerHackCnt(
                    user, static_cast<WongWork::ENUM_HACKTYPE>(0x2BFu),
                    1u, 0u, 0u);
        }
    }
    if (log_history && user) {
        CUser_history_log(user)->MoneyAdd(static_cast<int>(money_), money,
                                          reason);
    }
    if (user) {
        const int actual_gain = static_cast<int>(money_) - before_money;
        if (static_cast<int>(reason) == 4 || static_cast<int>(reason) == 26) {
            write_log_gain_money(static_cast<int>(reason),
                                 actual_gain - excluded_money, user);
        } else {
            write_log_gain_money(static_cast<int>(reason), actual_gain, user);
        }
        if (actual_gain > 0) {
            reinterpret_cast<Secu_GoldControl*>((user->GetGoldControl()))->AddGold(
                                     static_cast<unsigned int>(actual_gain),
                                     reason);
        }
    }
    return static_cast<int>(money_) - before_money;
}

// IDA_EA: 0x84FF8DE
// IDA_NAME: _ZN10CInventory8use_itemEiii
// DEMANGLED: CInventory::use_item(int,int,int)
int CInventory::use_item(int slot, int count, int reason) {
    if (count < 0) return 1;
    const int inventory_type = reason == 7 ? 3 : 1;
    if (reason != 7 &&
        ((slot < oldsrv::config::kMainInventoryQuickSlotFirst ||
          slot > oldsrv::config::kMainInventoryQuickSlotLast) &&
         (slot < oldsrv::config::kMainInventoryConsumableSlotFirst ||
          slot > oldsrv::config::kMainInventoryConsumableSlotLast)))
        return 4;
    Inven_Item* const item = GetInvenRef(inventory_type, slot);
    if (!item || !item->item_id_) return 23;
    CItem* const data = G_CDataManager()->find_item(
        item->item_id_);
    if (!data) return 1;
    if (reason != 7) {
        if (data->vtable_->get_item_type(data) == 18)
            return 0;
    }
    CUser* const user = parent_;
    if (user) {
        int price = 0;
        G_Store()->GetSellItemPrice(*item, data, static_cast<short>(count),
                                    false, price);
        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(21u), user,
            static_cast<unsigned int>(price));
    }
    const int current_count = item->add_info_;
    const int creature_result = creature_manager_.UseItem(item, slot);
    if (creature_result) return creature_result;
    if (current_count == count) {
        if (user) CUser_history_log(user)->ItemDel(
            static_cast<INVEN_TYPE>(inventory_type),
            0,
            count,
            *item,
            static_cast<eItemDelReason>(3));
        item->reset();
        return 0;
    }
    if (current_count <= count) {
        item->reset();
        if (user) CUser_history_log(user)->ItemDel(
            static_cast<INVEN_TYPE>(inventory_type),
            item->get_add_info(),
            count,
            *item,
            static_cast<eItemDelReason>(3));
        return 1;
    }
    item->add_info_ = current_count - count;
    if (user) CUser_history_log(user)->ItemDel(
        static_cast<INVEN_TYPE>(inventory_type),
        item->get_add_info(),
        count,
        *item,
        static_cast<eItemDelReason>(3));
    return 0;
}

// IDA_EA: 0x84FFFBA
// IDA_NAME: _ZNK10CInventory12try_use_itemEiii
// DEMANGLED: CInventory::try_use_item(int,int,int)const
int CInventory::try_use_item(int slot, int count, int reason) const {
    if (count < 0 || reason == 7) return 1;
    if ((slot < oldsrv::config::kMainInventoryQuickSlotFirst ||
         slot > oldsrv::config::kMainInventoryQuickSlotLast) &&
        (slot < oldsrv::config::kMainInventoryConsumableSlotFirst ||
         slot > oldsrv::config::kMainInventoryConsumableSlotLast)) return 4;
    const Inven_Item* const item = GetInvenRef(1, slot);
    if (!item->item_id_)
        return 23;
    CItem* const data = G_CDataManager()->find_item(
        item->item_id_);
    if (!data) return 1;
    if (data->vtable_->get_item_type(data) == 18)
        return 0;
    return item->get_add_info() < count;
}

// IDA_EA: 0x8500354
// IDA_NAME: _ZN10CInventory31isEquipmentPlaceableAtInventoryEi
// DEMANGLED: CInventory::isEquipmentPlaceableAtInventory(int)
bool CInventory::isEquipmentPlaceableAtInventory(int slot) {
    if (slot >= oldsrv::config::kMainInventoryQuickSlotFirst &&
        slot <= oldsrv::config::kMainInventoryQuickSlotLast) return true;
    return slot >= oldsrv::config::kMainInventoryEquipmentSlotFirst &&
           slot <= oldsrv::config::kMainInventoryEquipmentSlotLast;
}

// IDA_EA: 0x8500384
// IDA_NAME: _ZNK10CInventory15GetSlotItemTypeEi
// DEMANGLED: CInventory::GetSlotItemType(int)const
int CInventory::GetSlotItemType(int slot) const {
    Inven_Item::ITEM_TYPE item_type = Inven_Item::ITEM_TYPE_UNKNOWN;
    if (g_slotBoundChecker.get_item_type(slot, inventory_capacity_, item_type))
        return static_cast<int>(item_type);
    return 11;
}

// IDA_EA: 0x8501ECA
// IDA_NAME: _ZNK10CInventory26tryInsertItemIntoInventoryE10Inven_Item
// DEMANGLED: CInventory::tryInsertItemIntoInventory(Inven_Item)const
int CInventory::tryInsertItemIntoInventory(Inven_Item item) const {
    const int item_id = item.item_id_;
    if (!item_id)
        return item.get_add_info() >= 0 ? 0 : -1;
    if (item.item_category_ == 8) {
        const int inserted = tryInsertAvatarIntoInventory(item);
        return inserted < 0 ? -1 : inserted;
    }
    if (item.item_category_ != 1) {
        int inserted = tryStackInvenItemIntoQuickSlot(item);
        if (inserted >= 0) return inserted;
        if (inserted == -10) return -1;
        inserted = tryStackInvenItemIntoCorrespondingSpace(item);
        if (inserted >= 0) return inserted;
        if (inserted == -10) return -1;
    }
    if (item.item_category_ == 2) {
        int inserted = tryInsertInvenItemIntoQuickSlot(item);
        if (inserted >= 0) return inserted;
        if (inserted == -10) return -1;
        inserted = tryInsertInvenItemIntoCoresspondingSpace(item);
        return inserted >= 0 ? inserted : -1;
    }
    int inserted = tryInsertInvenItemIntoCoresspondingSpace(item);
    if (inserted >= 0) return inserted;
    inserted = tryInsertInvenItemIntoQuickSlot(item);
    return inserted >= 0 ? inserted : -1;
}

// IDA_EA: 0x8501AFA
// IDA_NAME: _ZNK10CInventory26tryInsertItemIntoEquipmentE10Inven_Item
// DEMANGLED: CInventory::tryInsertItemIntoEquipment(Inven_Item)const
int CInventory::tryInsertItemIntoEquipment(Inven_Item item) const {
    CItem* const data = G_CDataManager()->find_item(
        item.item_id_);
    if (!data) return -1;
    const int equipment_slot = data->vtable_->get_item_type(data);
    const Inven_Item* const target = GetInvenRef(0, equipment_slot);
    if (target->item_id_)
        return -2;
    return equipment_slot;
}

// IDA_EA: 0x8501CEA
// IDA_NAME: _ZNK10CInventory39tryStackInvenItemIntoCorrespondingSpaceE10Inven_Item
// DEMANGLED: CInventory::tryStackInvenItemIntoCorrespondingSpace(Inven_Item)const
int CInventory::tryStackInvenItemIntoCorrespondingSpace(Inven_Item item) const {
    if (item.item_category_ == 1) return -1;
    int start = 0, end = 0;
    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item.item_category_), start, end);
    const unsigned long item_id =
        static_cast<unsigned long>(static_cast<uint32_t>(item.item_id_));
    for (int slot = start; slot <= end; ++slot) {
        const Inven_Item* current = GetInvenRef(1, slot);
        if (static_cast<unsigned long>(static_cast<uint32_t>(current->item_id_)) !=
            item_id)
            continue;
        const int add_count = item.get_add_info();
        if (0x7fffffff - add_count <= current->get_add_info()) return -2;
        const int combined = current->get_add_info() + add_count;
        return checkStackableLimit(item_id, combined) ? slot : -10;
    }
    return -4;
}

// IDA_EA: 0x8501E12
// IDA_NAME: _ZNK10CInventory40tryInsertInvenItemIntoCoresspondingSpaceE10Inven_Item
// DEMANGLED: CInventory::tryInsertInvenItemIntoCoresspondingSpace(Inven_Item)const
int CInventory::tryInsertInvenItemIntoCoresspondingSpace(Inven_Item item) const {
    const unsigned long item_id =
        static_cast<unsigned long>(static_cast<uint32_t>(item.item_id_));
    if (!item.isEquipableItemType() &&
        !checkStackableLimit(item_id, item.get_add_info())) return -10;
    int start = 0, end = 0;
    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item.item_category_), start, end);
    for (int slot = start; slot <= end; ++slot) {
        const Inven_Item* current = GetInvenRef(1, slot);
        if (!current->item_id_) return slot;
    }
    return -2;
}

// IDA_EA: 0x8501B70
// IDA_NAME: _ZNK10CInventory31tryInsertInvenItemIntoQuickSlotE10Inven_Item
// DEMANGLED: CInventory::tryInsertInvenItemIntoQuickSlot(Inven_Item)const
int CInventory::tryInsertInvenItemIntoQuickSlot(Inven_Item item) const {
    const unsigned long item_id =
        static_cast<unsigned long>(static_cast<uint32_t>(item.item_id_));
    if (!item.isEquipableItemType() &&
        !checkStackableLimit(item_id, item.get_add_info())) return -10;
    for (int slot = 3; slot <= 8; ++slot) {
        const Inven_Item* const current = GetInvenRef(1, slot);
        if (!current->item_id_)
            return slot;
    }
    return -2;
}

// IDA_EA: 0x8501BF6
// IDA_NAME: _ZNK10CInventory30tryStackInvenItemIntoQuickSlotE10Inven_Item
// DEMANGLED: CInventory::tryStackInvenItemIntoQuickSlot(Inven_Item)const
int CInventory::tryStackInvenItemIntoQuickSlot(Inven_Item item) const {
    if (item.item_category_ == 1) return -1;
    const unsigned long item_id =
        static_cast<unsigned long>(static_cast<uint32_t>(item.item_id_));
    int slot = 3;
    for (; slot <= 8; ++slot) {
        const Inven_Item* const current = GetInvenRef(1, slot);
        if (static_cast<unsigned long>(static_cast<uint32_t>(current->item_id_)) ==
            item_id)
            break;
    }
    if (slot > 8) return -4;
    const Inven_Item* const current = GetInvenRef(1, slot);
    const int add_count = item.get_add_info();
    if (0x7fffffff - add_count <= current->get_add_info()) return -2;
    const int combined = current->get_add_info() + add_count;
    return checkStackableLimit(item_id, combined) ? slot : -10;
}

// IDA_EA: 0x85035F4
// IDA_NAME: _ZNK10CInventory28tryInsertAvatarIntoInventoryE10Inven_Item
// DEMANGLED: CInventory::tryInsertAvatarIntoInventory(Inven_Item)const
int CInventory::tryInsertAvatarIntoInventory(Inven_Item) const {
    for (int slot = 0;
         slot <= oldsrv::config::kAvatarInventoryLastSlot;
         ++slot) {
        const Inven_Item* const current = &avatar_items_[slot];
        if (!current->item_id_)
            return slot;
    }
    return -1;
}

// IDA_EA: 0x8502470
// IDA_NAME: _ZN10CInventory28insertInvenItemIntoQuickSlotE10Inven_Item14eItemAddReasonb
// DEMANGLED: CInventory::insertInvenItemIntoQuickSlot(Inven_Item,eItemAddReason,bool)
int CInventory::insertInvenItemIntoQuickSlot(Inven_Item item,
                                              eItemAddReason reason,
                                              bool log_history) {
    int slot = 3;
    for (; slot <= 8; ++slot) {
        Inven_Item* const target = GetInvenRef(1, slot);
        if (!target->item_id_) break;
    }
    if (slot > 8) return -2;
    const unsigned long item_id =
        static_cast<unsigned long>(static_cast<uint32_t>(item.item_id_));
    if (item.item_category_ != 1 &&
        !checkStackableLimit(item_id, item.get_add_info())) return -10;
    Inven_Item* const target = GetInvenRef(1, slot);
    *target = item;
    if (log_history) {
        CUser* const user = parent_;
        if (user) {
            if (item.item_category_ == 1) {
                reinterpret_cast<cUserHistoryLog*>(&user->history_log())->ItemAdd(
                    static_cast<INVEN_TYPE>(1), 1, 1, item, reason);
            } else {
                reinterpret_cast<cUserHistoryLog*>(&user->history_log())->ItemAdd(
                    static_cast<INVEN_TYPE>(1), target->get_add_info(),
                    item.get_add_info(), item, reason);
            }
        } else {
            LogManager::logFormat(
                1, "inventory.cpp",
                "int CInventory::insertInvenItemIntoQuickSlot(Inven_Item, eItemAddReason, bool)",
                0x1528, "History ERROR, m_pParent NULL, ItemAdd ");
        }
    }
    return slot;
}

// IDA_EA: 0x85025F8
// IDA_NAME: _ZN10CInventory37insertInvenItemIntoCoresspondingSpaceE10Inven_Item14eItemAddReasonb
// DEMANGLED: CInventory::insertInvenItemIntoCoresspondingSpace(Inven_Item,eItemAddReason,bool)
int CInventory::insertInvenItemIntoCoresspondingSpace(Inven_Item item,
                                                       eItemAddReason reason,
                                                       bool log_history) {
    int start = 0, end = 0;
    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item.item_category_), start, end);
    int slot = start;
    for (; slot <= end; ++slot) {
        Inven_Item* const target = GetInvenRef(1, slot);
        if (!target->item_id_) break;
    }
    if (slot > end) return -2;
    const unsigned long item_id =
        static_cast<unsigned long>(static_cast<uint32_t>(item.item_id_));
    if (item.item_category_ != 1 &&
        !checkStackableLimit(item_id, item.get_add_info())) return -10;
    Inven_Item* const target = GetInvenRef(1, slot);
    *target = item;
    if (log_history) {
        CUser* const user = parent_;
        if (user) {
            if (item.item_category_ == 1) {
                reinterpret_cast<cUserHistoryLog*>(&user->history_log())->ItemAdd(
                    static_cast<INVEN_TYPE>(1), 1, 1, item, reason);
            } else {
                reinterpret_cast<cUserHistoryLog*>(&user->history_log())->ItemAdd(
                    static_cast<INVEN_TYPE>(1), target->get_add_info(),
                    item.get_add_info(), item, reason);
            }
        } else {
            LogManager::logFormat(
                1, "inventory.cpp",
                "int CInventory::insertInvenItemIntoCoresspondingSpace(Inven_Item, eItemAddReason, bool)",
                0x155C, "History ERROR, m_pParent NULL, ItemAdd ");
        }
    }
    return slot;
}

// IDA_EA: 0x85027B2
// IDA_NAME: _ZN10CInventory27stackInvenItemIntoQuickSlotE10Inven_Item14eItemAddReasonb
// DEMANGLED: CInventory::stackInvenItemIntoQuickSlot(Inven_Item,eItemAddReason,bool)
int CInventory::stackInvenItemIntoQuickSlot(Inven_Item item,
                                             eItemAddReason reason,
                                             bool log_history) {
    if (item.item_category_ == 1) return -1;
    const unsigned long item_id =
        static_cast<unsigned long>(static_cast<uint32_t>(item.item_id_));
    int slot = 3;
    for (; slot <= 8; ++slot) {
        Inven_Item* const target = GetInvenRef(1, slot);
        if (static_cast<unsigned long>(static_cast<uint32_t>(target->item_id_)) ==
            item_id)
            break;
    }
    if (slot > 8) return -2;
    Inven_Item* const target = GetInvenRef(1, slot);
    const int added_count = item.get_add_info();
    const int old_count = target->get_add_info();
    if (0x7fffffff - added_count > old_count) {
        if (!checkStackableLimit(item_id, old_count + added_count)) return -10;
        target->add_info_ = old_count + added_count;
        if (log_history) {
            CUser* const user = parent_;
            if (user) {
                reinterpret_cast<cUserHistoryLog*>(&user->history_log())->ItemAdd(
                    static_cast<INVEN_TYPE>(1), target->get_add_info(),
                    item.get_add_info(), item, reason);
            } else {
                LogManager::logFormat(
                    1, "inventory.cpp",
                    "int CInventory::stackInvenItemIntoQuickSlot(Inven_Item, eItemAddReason, bool)",
                    0x1597, "History ERROR, m_pParent NULL, ItemAdd ");
            }
        }
    } else {
        if (log_history) {
            CUser* const user = parent_;
            if (user) {
                cMyTrace trace(
                    "int CInventory::stackInvenItemIntoQuickSlot(Inven_Item, eItemAddReason, bool)",
                    5500, 4);
                trace("INVEN INT_MAX setting 1 : m_id(%s),charac_id(%d) inven(%d)+add(%d) item_id(%d)",
                      NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
                      user->get_charac_no(-1), old_count, added_count,
                      static_cast<int>(item_id));
            }
        }
        target->add_info_ = 0;
    }
    return slot;
}

// IDA_EA: 0x8502A7E
// IDA_NAME: _ZN10CInventory36stackInvenItemIntoCorrespondingSpaceE10Inven_Item14eItemAddReasonb
// DEMANGLED: CInventory::stackInvenItemIntoCorrespondingSpace(Inven_Item,eItemAddReason,bool)
int CInventory::stackInvenItemIntoCorrespondingSpace(Inven_Item item,
                                                      eItemAddReason reason,
                                                      bool log_history) {
    if (item.item_category_ == 1) return -1;
    int start = 0, end = 0;
    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item.item_category_), start, end);
    const unsigned long item_id =
        static_cast<unsigned long>(static_cast<uint32_t>(item.item_id_));
    int slot = start;
    for (; slot <= end; ++slot) {
        Inven_Item* const target = GetInvenRef(1, slot);
        if (static_cast<unsigned long>(static_cast<uint32_t>(target->item_id_)) ==
            item_id)
            break;
    }
    if (slot > end) return -2;
    Inven_Item* const target = GetInvenRef(1, slot);
    const int added_count = item.get_add_info();
    const int old_count = target->get_add_info();
    if (0x7fffffff - added_count > old_count) {
        if (!checkStackableLimit(item_id, old_count + added_count)) return -10;
        target->add_info_ = old_count + added_count;
        if (log_history) {
            CUser* const user = parent_;
            if (user) {
                reinterpret_cast<cUserHistoryLog*>(&user->history_log())->ItemAdd(
                    static_cast<INVEN_TYPE>(1), target->get_add_info(),
                    item.get_add_info(), item, reason);
            } else {
                LogManager::logFormat(
                    1, "inventory.cpp",
                    "int CInventory::stackInvenItemIntoCorrespondingSpace(Inven_Item, eItemAddReason, bool)",
                    0x15D2, "History ERROR, m_pParent NULL, ItemAdd ");
            }
        }
    } else {
        if (log_history) {
            CUser* const user = parent_;
            if (user) {
                cMyTrace trace(
                    "int CInventory::stackInvenItemIntoCorrespondingSpace(Inven_Item, eItemAddReason, bool)",
                    5559, 4);
                trace("INVEN INT_MAX setting 2 : m_id(%s),charac_id(%d) inven(%d)+add(%d) item_id(%d)",
                      NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
                      user->get_charac_no(-1), old_count, added_count,
                      static_cast<int>(item_id));
            }
        }
        target->add_info_ = 0;
    }
    return slot;
}

// IDA_EA: 0x8503392
// IDA_NAME: _ZN10CInventory18doesExistEquipItemEii
// DEMANGLED: CInventory::doesExistEquipItem(int,int)
int CInventory::doesExistEquipItem(int item_id, int item_key) {
    for (int slot = oldsrv::config::kMainInventoryQuickSlotFirst;
         slot <= oldsrv::config::kMainInventoryQuestSlotLast;
         ++slot) {
        const Inven_Item* const item = GetInvenRef(1, slot);
        if (item->item_id_ == item_id && item->add_info_ == item_key &&
            item->item_category_ == 1) return 1;
    }
    return 0;
}

// IDA_EA: 0x8503418
// IDA_NAME: _ZN10CInventory17getExistEquipItemEiii
// DEMANGLED: CInventory::getExistEquipItem(int,int,int)
int CInventory::getExistEquipItem(int item_id, int item_key, int excluded_slot) {
    for (int slot = oldsrv::config::kMainInventoryQuickSlotFirst;
         slot <= oldsrv::config::kMainInventoryQuestSlotLast;
         ++slot) {
        const Inven_Item* const item = GetInvenRef(1, slot);
        if (slot != excluded_slot && item->item_category_ == 1 &&
            item->item_id_ == item_id && item->add_info_ == item_key)
            return slot;
    }
    return -1;
}

// IDA_EA: 0x85034AE
// IDA_NAME: _ZN10CInventory24removeDupliacteEquipItemEv
// DEMANGLED: CInventory::removeDupliacteEquipItem(void)
int CInventory::removeDupliacteEquipItem() {
    int slot = oldsrv::config::kMainInventoryQuickSlotFirst;
    for (; slot <= oldsrv::config::kMainInventoryQuestSlotLast; ++slot) {
        Inven_Item* const item = GetInvenRef(1, slot);
        if (item->item_category_ != 1) continue;
        if (getExistEquipItem(
                item->item_id_,
                item->add_info_,
                slot) != -1)
            break;
    }
    if (slot > oldsrv::config::kMainInventoryQuestSlotLast) return 0;
    Inven_Item* const duplicate = GetInvenRef(1, slot);
    CUser* const user = parent_;
    const int charac_no = user ? user->get_charac_no(-1) : 0;
    cMyTrace trace("bool CInventory::removeDupliacteEquipItem()", 5842, 5);
    trace("DUPLICATE ITEM REMOVED %d at %d of %d",
          duplicate->item_id_,
          slot, charac_no);
    duplicate->reset();
    return 1;
}

// IDA_EA: 0x8503638
// IDA_NAME: _ZNK10CInventory18getAvatarEmptySlotEii
// DEMANGLED: CInventory::getAvatarEmptySlot(int,int)const
int CInventory::getAvatarEmptySlot(int first_slot, int last_slot) const {
    for (int slot = first_slot; slot <= last_slot; ++slot) {
        const Inven_Item* const item = &avatar_items_[slot];
        if (!item->item_id_)
            return slot;
    }
    return -1;
}

// IDA_EA: 0x8502344
// IDA_NAME: _ZN10CInventory23insertItemIntoEquipmentE10Inven_Item14eItemAddReasonbb
// DEMANGLED: CInventory::insertItemIntoEquipment(Inven_Item,eItemAddReason,bool,bool)
int CInventory::insertItemIntoEquipment(Inven_Item item,
                                         eItemAddReason reason,
                                         bool,
                                         bool log_history) {
    const unsigned int item_id =
        static_cast<uint32_t>(item.item_id_);
    CItem* const data = G_CDataManager()->find_item(static_cast<int>(item_id));
    if (!data) return -1;
    const int equipment_slot = data->vtable_->get_item_type(data);
    Inven_Item* const target = GetInvenRef(0, equipment_slot);
    if (target->item_id_)
        return -2;
    *target = item;
    if (log_history) {
        CUser* const user = parent_;
        if (user) {
            reinterpret_cast<cUserHistoryLog*>(&user->history_log())->ItemAdd(
                static_cast<INVEN_TYPE>(1), 1, 1, item, reason);
        } else {
            LogManager::logFormat(
                1, "inventory.cpp",
                "int CInventory::insertItemIntoEquipment(Inven_Item, eItemAddReason, bool, bool)",
                0x14F9, "History ERROR, m_pParent NULL, ItemAdd ");
        }
    }
    return equipment_slot;
}

// IDA_EA: 0x8502D86
// IDA_NAME: _ZN10CInventory23insertItemIntoInventoryE10Inven_Item14eItemAddReasonbb
// DEMANGLED: CInventory::insertItemIntoInventory(Inven_Item,eItemAddReason,bool,bool)
int CInventory::insertItemIntoInventory(Inven_Item item,
                                         eItemAddReason reason,
                                         bool log_history,
                                         bool creature_flag) {
    const unsigned int item_id =
        static_cast<uint32_t>(item.item_id_);
    CUser* const user = parent_;
    if (item_id) {
        if (static_cast<int>(reason) != 7 && static_cast<int>(reason) != 2) {
            const int log_type = G_CDataManager()->IsDBLogItem(item_id);
            if (log_type != -1) {
                if (item.item_category_ == 1) {
                    user->AddDBLogItem(item_id, 1u,
                                       static_cast<ENUM_DBLOG_ITEM_TYPE>(log_type));
                } else if (item.item_category_ == 2) {
                    user->AddDBLogItem(
                        item_id, static_cast<unsigned int>(item.get_add_info()),
                        static_cast<ENUM_DBLOG_ITEM_TYPE>(log_type));
                }
            }
        }
        if (item.IsCreatureItemType()) {
            return FinishInventoryInsert(
                user, item,
                insertItemIntoCreature(item, reason, log_history,
                                       creature_flag),
                log_history);
        }
        if (item.item_category_ == 8) {
            char ipg_no[24];
            WongWork::CGenUniqueNo::genIPGNo(
                static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(6),
                static_cast<unsigned int>(user->get_charac_no(-1)), ipg_no);
            const int slot = user->addAvatarItem(
                static_cast<unsigned long>(item_id), 0,
                static_cast<char>(static_cast<uint8_t>(item.avatar_select_)),
                ipg_no, static_cast<eAvatarItemAddReason>(20));
            user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(1), slot);
            return FinishInventoryInsert(user, item, slot, log_history);
        }
        if (item.item_category_ != 1) {
            int inserted = stackInvenItemIntoQuickSlot(item, reason, log_history);
            if (inserted >= 0)
                return FinishInventoryInsert(user, item, inserted,
                                                 log_history);
            if (inserted == -10) return -1;
            inserted = stackInvenItemIntoCorrespondingSpace(item, reason,
                                                             log_history);
            if (inserted >= 0)
                return FinishInventoryInsert(user, item, inserted,
                                                 log_history);
            if (inserted == -10) return -1;
        }
        if (item.item_category_ == 2) {
            int inserted = insertInvenItemIntoQuickSlot(item, reason, log_history);
            if (inserted >= 0)
                return FinishInventoryInsert(user, item, inserted,
                                                 log_history);
            if (inserted == -10) return -1;
            inserted = insertInvenItemIntoCoresspondingSpace(item, reason,
                                                              log_history);
            return inserted >= 0
                       ? FinishInventoryInsert(user, item, inserted,
                                                   log_history)
                       : -1;
        }
        int inserted = insertInvenItemIntoCoresspondingSpace(item, reason,
                                                              log_history);
        if (inserted >= 0)
            return FinishInventoryInsert(user, item, inserted,
                                             log_history);
        inserted = insertInvenItemIntoQuickSlot(item, reason, log_history);
        return inserted >= 0
                   ? FinishInventoryInsert(user, item, inserted,
                                               log_history)
                   : -1;
    }
    if (item.get_add_info() < 0) return -1;
    if (item.get_add_info()) {
        gain_money(item.get_add_info(), static_cast<eMoneyAddReason>(7),
                   log_history, 0);
    }
    return 0;
}

// IDA_EA: 0x850A3B6
// IDA_NAME: _ZNK10CInventory20GetExistCreatureSlotER10Inven_Item
// DEMANGLED: CInventory::GetExistCreatureSlot(Inven_Item &)const
int CInventory::GetExistCreatureSlot(Inven_Item& item) const {
    int start = 0, end = 0;
    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item.item_category_), start, end);
    const unsigned int item_id =
        static_cast<uint32_t>(item.item_id_);
    for (int slot = start; slot <= end; ++slot) {
        const Inven_Item* const current = GetInvenRef(3, slot);
        if (static_cast<uint32_t>(current->item_id_) == item_id) return slot;
    }
    return -1;
}

// IDA_EA: 0x850A436
// IDA_NAME: _ZNK10CInventory20GetEmptyCreatureSlotER10Inven_Item
// DEMANGLED: CInventory::GetEmptyCreatureSlot(Inven_Item &)const
int CInventory::GetEmptyCreatureSlot(Inven_Item& item) const {
    int start = 0, end = 0;
    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(item.item_category_), start, end);
    for (int slot = start; slot <= end; ++slot) {
        const Inven_Item* const current = GetInvenRef(3, slot);
        if (!current->item_id_)
            return slot;
    }
    return -1;
}

// IDA_EA: 0x850A4B0
// IDA_NAME: _ZNK10CInventory16WriteCreatureLogE10INVEN_TYPEiiii14eItemAddReason
// DEMANGLED: CInventory::WriteCreatureLog(INVEN_TYPE,int,int,int,int,eItemAddReason)const
void CInventory::WriteCreatureLog(INVEN_TYPE inventory_type,
                                  int item_id,
                                  int resulting_count,
                                  int added_count,
                                  int item_type,
                                  eItemAddReason reason) const {
    CUser* const user = parent_;
    if (user) {
        reinterpret_cast<cUserHistoryLog*>(&user->history_log())
            ->CreatureItemAdd(inventory_type, item_id, resulting_count,
                              added_count, item_type, reason);
        return;
    }
    LogManager::logFormat(
        1, "inventory.cpp",
        "void CInventory::WriteCreatureLog(INVEN_TYPE, int, int, int, int, eItemAddReason) const",
        0x24CE, "History ERROR, m_pParent NULL, ItemAdd ");
}

// IDA_EA: 0x850A52C
// IDA_NAME: _ZN10CInventory31insertItemIntoCreatureStackableE10Inven_Item14eItemAddReasonb
// DEMANGLED: CInventory::insertItemIntoCreatureStackable(Inven_Item,eItemAddReason,bool)
int CInventory::insertItemIntoCreatureStackable(Inven_Item item,
                                                 eItemAddReason reason,
                                                 bool log_history) {
    const unsigned int item_id =
        static_cast<uint32_t>(item.item_id_);
    if (!item_id) return -1;
    const int existing_slot = GetExistCreatureSlot(item);
    if (existing_slot < 0 ||
        0x7fffffff - item.get_add_info() <=
            GetInvenRef(3, existing_slot)->get_add_info()) {
        const int empty_slot = GetEmptyCreatureSlot(item);
        if (empty_slot < 0) return -1;
        Inven_Item* const target = GetInvenRef(3, empty_slot);
        *target = item;
        if (log_history) {
            WriteCreatureLog(static_cast<INVEN_TYPE>(3), static_cast<int>(item_id),
                             target->get_add_info(), item.get_add_info(),
                             item.item_category_, reason);
        }
        return empty_slot;
    }
    Inven_Item* const target = GetInvenRef(3, existing_slot);
    const int old_count = target->get_add_info();
    const int added_count = item.get_add_info();
    if (!checkStackableLimit(
            static_cast<uint32_t>(target->item_id_),
            old_count + added_count)) return -10;
    target->add_info_ = old_count + added_count;
    if (log_history) {
        WriteCreatureLog(static_cast<INVEN_TYPE>(1), static_cast<int>(item_id),
                         target->get_add_info(), added_count,
                         item.item_category_, reason);
    }
    return existing_slot;
}

// IDA_EA: 0x850A762
// IDA_NAME: _ZN10CInventory31insertItemIntoCreatureEquipmentE10Inven_Item14eItemAddReasonb
// DEMANGLED: CInventory::insertItemIntoCreatureEquipment(Inven_Item,eItemAddReason,bool)
int CInventory::insertItemIntoCreatureEquipment(Inven_Item item,
                                                 eItemAddReason,
                                                 bool) {
    if (!item.item_id_)
        return -1;
    const int empty_slot = GetEmptyCreatureSlot(item);
    if (empty_slot < 0) return -1;
    Inven_Item* const target = GetInvenRef(3, empty_slot);
    *target = item;
    return empty_slot;
}

// IDA_EA: 0x850A7C4
// IDA_NAME: _ZN10CInventory31insertItemIntoArtifactEquipmentE10Inven_Item14eItemAddReasonb
// DEMANGLED: CInventory::insertItemIntoArtifactEquipment(Inven_Item,eItemAddReason,bool)
int CInventory::insertItemIntoArtifactEquipment(Inven_Item item,
                                                 eItemAddReason reason,
                                                 bool log_history) {
    const unsigned int item_id =
        static_cast<uint32_t>(item.item_id_);
    if (!item_id) return -1;
    const int empty_slot = GetEmptyCreatureSlot(item);
    if (empty_slot < 0) return -1;
    Inven_Item* const target = GetInvenRef(3, empty_slot);
    *target = item;
    if (log_history) {
        WriteCreatureLog(static_cast<INVEN_TYPE>(3), static_cast<int>(item_id),
                         1, 1, item.item_category_, reason);
    }
    return empty_slot;
}

// IDA_EA: 0x850A86A
// IDA_NAME: _ZN10CInventory22insertItemIntoCreatureER10Inven_Item14eItemAddReasonbb
// DEMANGLED: CInventory::insertItemIntoCreature(Inven_Item &,eItemAddReason,bool,bool)
int CInventory::insertItemIntoCreature(Inven_Item& item,
                                        eItemAddReason reason,
                                        bool log_history,
                                        bool) {
    switch (item.item_category_) {
        case 7:
            return insertItemIntoCreatureStackable(item, reason, log_history);
        case 5:
            return insertItemIntoCreatureEquipment(item, reason, log_history);
        case 6:
            return insertItemIntoArtifactEquipment(item, reason, log_history);
        default:
            LogManager::logFormat(
                1, "inventory.cpp",
                "int CInventory::insertItemIntoCreature(Inven_Item&, eItemAddReason, bool, bool)",
                0x2549,
                "History ERROR, unknown inven_Item type, insertItemIntoCreature ");
            return -1;
    }
}

// IDA_EA: 0x850AAA4
// IDA_NAME: _ZNK10CInventory25tryInsertItemIntoCreatureER10Inven_Item
// DEMANGLED: CInventory::tryInsertItemIntoCreature(Inven_Item &)const
int CInventory::tryInsertItemIntoCreature(Inven_Item& item) const {
    if (item.isEquipableItemType()) return GetEmptyCreatureSlot(item);
    const int existing_slot = GetExistCreatureSlot(item);
    return existing_slot < 0 ? GetEmptyCreatureSlot(item) : existing_slot;
}

// IDA_EA: 0x85039E8
// IDA_NAME: _ZN10CInventory24insert_item_special_slotE10Inven_Itemii15eMoneyAddReason14eItemAddReason
// DEMANGLED: CInventory::insert_item_special_slot(Inven_Item,int,int,eMoneyAddReason,eItemAddReason)
int CInventory::insert_item_special_slot(Inven_Item item,
                                         int item_count,
                                         int original_slot,
                                         eMoneyAddReason money_reason,
                                         eItemAddReason item_reason) {
    const unsigned int item_id =
        static_cast<uint32_t>(item.item_id_);
    if (!item_id) {
        if (item_count >= 0) {
            gain_money(item_count, money_reason, true, 0);
            return 0;
        }

        cMyTrace trace(
            "int CInventory::insert_item_special_slot(Inven_Item, int, int, eMoneyAddReason, eItemAddReason)",
            5954, 5);
        trace("[%s][%d]",
              "int CInventory::insert_item_special_slot(Inven_Item, int, int, eMoneyAddReason, eItemAddReason)");
        return -2;
    }

    CUser* const user = parent_;
    if (item.item_category_ == 1) {
        Inven_Item* const target = &stackable_items_[original_slot];
        if (target->item_id_)
            return -1;

        target->setCopy(item);
        if (user) {
            reinterpret_cast<cUserHistoryLog*>(&user->history_log())->ItemAdd(
                static_cast<INVEN_TYPE>(1), 1, 1, item, item_reason);
        }
        return original_slot;
    }

    for (int slot = 3; slot <= 8; ++slot) {
        Inven_Item* const target = &stackable_items_[slot];
        if (static_cast<uint32_t>(target->item_id_) != item_id)
            continue;

        const int previous_count = target->get_add_info();
        if (0x7fffffff - item_count >= previous_count) {
            target->set_add_info(previous_count + item_count);
        } else {
            if (user) {
                cMyTrace trace(
                    "int CInventory::insert_item_special_slot(Inven_Item, int, int, eMoneyAddReason, eItemAddReason)",
                    6012, 4);
                trace("INVEN INT_MAX setting 3 : m_id(%s),characNo(%d),inven(%d)+add(%d) item_id(%d)",
                      NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                                     0),
                      user->get_charac_no(-1), previous_count, item_count,
                      static_cast<int>(item_id));
            }
            target->set_add_info(0x7fffffff);
        }
        if (user) {
            reinterpret_cast<cUserHistoryLog*>(&user->history_log())->ItemAdd(
                static_cast<INVEN_TYPE>(1), target->get_add_info(),
                item.get_add_info(), item, item_reason);
        }
        return slot;
    }

    int start = 0;
    int end = 0;
    StartEndPos(static_cast<Inven_Item::ITEM_TYPE>(
                    item.item_category_),
                start, end);
    for (int slot = start; slot <= end; ++slot) {
        Inven_Item* const target = &stackable_items_[slot];
        if (static_cast<uint32_t>(target->item_id_) != item_id)
            continue;

        const int previous_count = target->get_add_info();
        if (0x7fffffff - item_count >= previous_count) {
            target->set_add_info(previous_count + item_count);
        } else {
            target->set_add_info(0x7fffffff);
            if (user) {
                cMyTrace trace(
                    "int CInventory::insert_item_special_slot(Inven_Item, int, int, eMoneyAddReason, eItemAddReason)",
                    6047, 4);
                trace("INT_MAX setting 3 : m_id(%s),characNo(%d),inven(%d)+add(%d)",
                      NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                                     0),
                      user->get_charac_no(-1), previous_count, item_count);
            }
        }
        if (user) {
            reinterpret_cast<cUserHistoryLog*>(&user->history_log())->ItemAdd(
                static_cast<INVEN_TYPE>(1), target->get_add_info(),
                item.get_add_info(), item, item_reason);
        }
        return slot;
    }

    Inven_Item* const target = &stackable_items_[original_slot];
    if (target->item_id_)
        return -1;

    target->setCopy(item);
    if (user) {
        reinterpret_cast<cUserHistoryLog*>(&user->history_log())->ItemAdd(
            static_cast<INVEN_TYPE>(1), target->get_add_info(),
            item.get_add_info(), item, item_reason);
    }
    return original_slot;
}

// IDA_EA: 0x850AB10
// IDA_NAME: _ZN10CInventory30insertCreatureIntoSpecificSlotE10Inven_Itemi14eItemAddReasonb
// DEMANGLED: CInventory::insertCreatureIntoSpecificSlot(Inven_Item,int,eItemAddReason,bool)
int CInventory::insertCreatureIntoSpecificSlot(Inven_Item item,
                                                int slot,
                                                eItemAddReason reason,
                                                bool log_history) {
    if (slot > oldsrv::config::kCreatureInventoryLastSlot) return -1;

    const unsigned int item_id =
        static_cast<uint32_t>(item.item_id_);
    if (!item_id) return -1;

    int inserted_slot = -1;
    Inven_Item* const requested = &creature_items_[slot];
    if (requested->item_id_) {
        if (!requested->isEquipableItemType()) {
            if (static_cast<uint32_t>(requested->item_id_) == item_id) {
                const int previous_count = requested->get_add_info();
                const int added_count = item.get_add_info();
                if (!checkStackableLimit(item_id, previous_count + added_count))
                    return -10;
                requested->set_add_info(previous_count + added_count);
                inserted_slot = slot;
            } else {
                for (int candidate = 0;
                     candidate <= oldsrv::config::kCreatureInventoryLastSlot;
                     ++candidate) {
                    Inven_Item* const target = &creature_items_[candidate];
                    if (!target->item_id_) {
                        target->setCopy(item);
                        inserted_slot = candidate;
                        break;
                    }
                    if (static_cast<uint32_t>(target->item_id_) == item_id) {
                        target->set_add_info(target->get_add_info() +
                                             item.get_add_info());
                        inserted_slot = candidate;
                        break;
                    }
                }
            }
        }
    } else {
        requested->setCopy(item);
        inserted_slot = slot;
    }

    if (inserted_slot >= 0 && item.item_category_ != 5 &&
        log_history) {
        Inven_Item* const target = &creature_items_[inserted_slot];
        CUser* const user = parent_;
        reinterpret_cast<cUserHistoryLog*>(&user->history_log())
            ->CreatureItemAdd(
                static_cast<INVEN_TYPE>(3), static_cast<int>(item_id),
                target->get_add_info(), item.get_add_info(),
                item.item_category_, reason);
    }
    return inserted_slot;
}
