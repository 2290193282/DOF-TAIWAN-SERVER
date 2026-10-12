#include "old_data/PVF/genuine_upgrade_table.h"

#include "common/config.h"
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/mt_rand.h"

namespace {

UpgradeSeparateInfo& SeparateUpgrade(Inven_Item* item) {
    return item->upgrade_separate_;
}

const UpgradeSeparateInfo& SeparateUpgrade(const Inven_Item& item) {
    return item.upgrade_separate_;
}

uint16_t InvenItemEndurance(const Inven_Item& item) {
    return item.avatar_select_;
}

int ItemType(const CItem* item) {
    return item->vtable_->get_item_type(item);
}

bool IsRandomOptionItem(const CItem* item) {
    return item->vtable_->is_random_option(item) != 0;
}

}  // namespace

namespace WongWork {

// IDA_EA: 0x80FC834
// IDA_NAME: _ZN8WongWork12IItemUpgradeC2Ev
// DEMANGLED: WongWork::IItemUpgrade::IItemUpgrade(void)
IItemUpgrade::IItemUpgrade() {}

// IDA_EA: 0x80FC842
// IDA_NAME: _ZN8WongWork12IItemUpgradeD2Ev
// DEMANGLED: WongWork::IItemUpgrade::~IItemUpgrade()
IItemUpgrade::~IItemUpgrade() {}

// IDA_EA: 0x80FC850
// IDA_NAME: _ZN8WongWork12IItemUpgrade19item_upgrade_systemEP5CUserP16MSG_ITEM_UPGRADEi
// DEMANGLED: WongWork::IItemUpgrade::item_upgrade_system(CUser *,MSG_ITEM_UPGRADE *,int)
void IItemUpgrade::item_upgrade_system(CUser* user,
                                       MSG_ITEM_UPGRADE* message,
                                       int packet_id) {
    PacketGuard response;
    response.put_header(1, packet_id);
    const int error = ProcUpgrade(user, message, &response);
    if (error != 0) {
        user->SendCmdErrorPacket2Byte(
            static_cast<ENUM_CMDPACKET>(packet_id),
            static_cast<uint16_t>(error));
        return;
    }
    response.finalize(true);
    user->Send(response);
}

// IDA_EA: 0x811DFC0
// IDA_NAME: _ZN8WongWork21CItemUpgrade_SeparateC2Ev
// DEMANGLED: WongWork::CItemUpgrade_Separate::CItemUpgrade_Separate(void)
CItemUpgrade_Separate::CItemUpgrade_Separate()
    : IItemUpgrade(),
      genuine_upgrade_table_(),
      random_(0),
      random_limit_(100000u) {
    const unsigned long seed = static_cast<unsigned long>(::time(0));
    random_ = new CMTRand(seed);
    for (int index = 0; index <= 99; ++index) {
        random_->randInt(random_limit_);
    }
}

// IDA_EA: 0x811E0BE
// IDA_NAME: _ZN8WongWork21CItemUpgrade_SeparateD2Ev
// DEMANGLED: WongWork::CItemUpgrade_Separate::~CItemUpgrade_Separate()
CItemUpgrade_Separate::~CItemUpgrade_Separate() {
    // The old D2 directly releases the CMTRand allocation before destroying
    // the embedded GenuineDamageUpgradeTable and its non-virtual base.
    ::operator delete(random_);
    random_ = 0;
}

// IDA_EA: 0x811E138
// IDA_NAME: _ZNK8WongWork21CItemUpgrade_Separate15_CheckConditionEPK5CUserRK10Inven_ItemPK5CItemS6_RK25GenuineDamageUpgradeTablei
int CItemUpgrade_Separate::_CheckCondition(
    const CUser* user,
    const Inven_Item& target,
    const CItem* item,
    const Inven_Item& material,
    const GenuineDamageUpgradeTable& table,
    int charac_level) const {
    (void)user;
    const int growth_grade =
        static_cast<const CEquipItem*>(item)->getGrowthGrade(charac_level);
    const std::map<int, GenuineDamageUpgradeMaterialInfo>::const_iterator
        material_info = table.material_by_grade.find(growth_grade);
    if (material_info == table.material_by_grade.end()) {
        return 1;
    }
    if (material.item_id_ != material_info->second.item_index) {
        return 22;
    }

    const int required_material_count = table.getGenuineUpgradeMaterialCnt(
        static_cast<unsigned char>(growth_grade),
        SeparateUpgrade(target).GetUpgradeSeparate(),
        static_cast<ENUM_EQUIPMENTTYPE>(ItemType(item)),
        static_cast<ENUM_RARITY>(item->get_rarity()));
    if (material.add_info_ < required_material_count) {
        return 22;
    }
    if (ItemType(item) != oldsrv::config::kEquipmentTypeWeapon) {
        return 19;
    }
    if (InvenItemEndurance(target) !=
        static_cast<unsigned int>(
            static_cast<const CEquipItem*>(item)->get_endurance())) {
        return 7;
    }
    return SeparateUpgrade(target).GetUpgradeSeparate() + 1 <=
                   table.separate_upgrade_max
               ? 0
               : 95;
}

// IDA_EA: 0x811E2CE
// IDA_NAME: _ZN8WongWork21CItemUpgrade_Separate15_GetPenaltyTypeEh11ENUM_RARITYRK25GenuineDamageUpgradeTablePK24GenuineDamageUpgradeInfo
int CItemUpgrade_Separate::_GetPenaltyType(
    unsigned char separate_upgrade,
    ENUM_RARITY rarity,
    const GenuineDamageUpgradeTable& table,
    const GenuineDamageUpgradeInfo* upgrade_info) {
    (void)separate_upgrade;
    (void)rarity;
    (void)table;
    (void)upgrade_info;
    return 1;
}

// IDA_EA: 0x811E2E2
// IDA_NAME: _ZN8WongWork21CItemUpgrade_Separate15_GetUpgradeInfoEPK10Inven_ItemPK5CItemRK25GenuineDamageUpgradeTablePNS_13STUpgradeInfoEi
int CItemUpgrade_Separate::_GetUpgradeInfo(
    const Inven_Item& target,
    const CItem* item,
    const GenuineDamageUpgradeTable& table,
    STUpgradeInfo* result,
    int charac_level) {
    const int separate_upgrade =
        SeparateUpgrade(target).GetUpgradeSeparate();
    if (separate_upgrade > table.separate_upgrade_max) {
        return 0;
    }

    const int material_count = table.getGenuineUpgradeMaterialCnt(
        static_cast<unsigned char>(
            static_cast<const CEquipItem*>(item)->getGrowthGrade(charac_level)),
        separate_upgrade,
        static_cast<ENUM_EQUIPMENTTYPE>(ItemType(item)),
        static_cast<ENUM_RARITY>(item->get_rarity()));
    if (material_count < 0) {
        return 0;
    }

    const std::map<int, GenuineDamageUpgradeInfo>::const_iterator info =
        table.info_by_upgrade.find(separate_upgrade + 1);
    if (info == table.info_by_upgrade.end()) {
        return 0;
    }
    result->material_count = material_count;
    result->success_rate_10000 =
        static_cast<int>(info->second.material_rate * 10000.0f);
    return 1;
}

// IDA_EA: 0x811E468
// IDA_NAME: _ZN8WongWork21CItemUpgrade_Separate14_DoProcUpgradeEP5CUserP10Inven_ItemPKNS_13STUpgradeInfoE
int CItemUpgrade_Separate::_DoProcUpgrade(CUser* user,
                                          Inven_Item* target,
                                          const STUpgradeInfo& info) {
    if (user->IsGameMasterMode()) {
        if (user->getUpgradeSeparateTrueFlag() == 1) {
            SeparateUpgrade(target).IncUpgradeSeparate();
            return 1;
        }
        if (user->getUpgradeSeparateFlaseFlag() == 1) {
            return 0;
        }
    }

    if (info.success_rate_10000 != 0 &&
        static_cast<unsigned int>(10 * info.success_rate_10000) <
            random_->randInt(random_limit_)) {
        return 0;
    }
    SeparateUpgrade(target).IncUpgradeSeparate();
    return 1;
}

// IDA_EA: 0x811E53A
// IDA_NAME: _ZN8WongWork21CItemUpgrade_Separate14_NoticeUpgradeEPK5CUserRK10Inven_Itemb
void CItemUpgrade_Separate::_NoticeUpgrade(const CUser* user,
                                           const Inven_Item& target,
                                           bool succeeded) {
    PacketGuard packet;
    packet.put_header(0, 86);
    packet.put_byte(10);
    packet.put_byte(succeeded);
    packet.put_short(user->get_unique_id());
    packet.put_int(target.item_id_);
    packet.put_byte(target.GetUpgrade());
    packet.put_byte(SeparateUpgrade(target).GetUpgradeSeparate());
    target.chronicle_random_option_.random_option().put_packet_random_option(
        packet);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x811E66E
// IDA_NAME: _ZN8WongWork21CItemUpgrade_Separate11ProcUpgradeEP5CUserP16MSG_ITEM_UPGRADEP11PacketGuard
int CItemUpgrade_Separate::ProcUpgrade(CUser* user,
                                       MSG_ITEM_UPGRADE* message,
                                       PacketGuard* packet) {
    CInventory* const inventory = user->getCurCharacInvenW();
    const uint16_t target_slot = message->body_.target_slot;
    Inven_Item target = inventory->GetInvenSlot(1, target_slot);
    if (target.isEmpty() ||
        target.item_id_ !=
            static_cast<int>(message->body_.target_item_id)) {
        return 4;
    }

    const uint16_t material_slot =
        message->body_.material_slot;
    const Inven_Item material = inventory->GetInvenSlot(1, material_slot);
    if (material.isEmpty()) {
        return 22;
    }

    CItem* const item = G_CDataManager()->find_item(target.item_id_);
    if (!item) {
        return 4;
    }

    STUpgradeInfo upgrade_info;
    const GenuineDamageUpgradeTable* const table = getUpgradeInfoTable();
    if (_GetUpgradeInfo(target, item, *table, &upgrade_info,
                        user->get_charac_level()) != 1) {
        return 13;
    }
    const int condition = _CheckCondition(user, target, item, material, *table,
                                          user->get_charac_level());
    if (condition != 0) {
        return condition;
    }

    const int old_separate_upgrade =
        SeparateUpgrade(target).GetUpgradeSeparate();
    const bool succeeded = _DoProcUpgrade(user, &target, upgrade_info) != 0;
    if (inventory->delete_item(static_cast<INVEN_TYPE>(1), material_slot,
                               upgrade_info.material_count,
                               static_cast<eItemDelReason>(12), true) != 1) {
        return 17;
    }

    packet->put_byte(1);
    packet->put_short(material_slot);
    packet->put_int(
        inventory->GetInvenSlot(1, material_slot).get_add_info());
    packet->put_byte(old_separate_upgrade);
    if (SeparateUpgrade(target).GetUpgradeSeparate() > 5) {
        _NoticeUpgrade(user, target, succeeded);
    }
    packet->put_byte(succeeded ? 0 : 1);
    packet->put_byte(SeparateUpgrade(target).GetUpgradeSeparate());
    packet->put_short(target_slot);

    if (target.item_id_ != 0) {
        inventory->update_item(static_cast<INVEN_TYPE>(1), target_slot, target);
    }

    user->history_log().SeparateUpgradeItem(
        item->get_index(), old_separate_upgrade,
        SeparateUpgrade(target).GetUpgradeSeparate(), succeeded);
    const char* const item_name = IsRandomOptionItem(item)
        ? message->body_.guild_item_name
        : item->GetItemName();
    user->makeGuildItemSeparateUpgradMessage(
        item_name, old_separate_upgrade,
        SeparateUpgrade(target).GetUpgradeSeparate(), succeeded,
        target.GetUpgrade());
    return 0;
}

}  // namespace WongWork
