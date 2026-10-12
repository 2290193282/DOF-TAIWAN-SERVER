#include "old_data/PVF/equipment/item_upgrade.h"

#include <stdint.h>

#include <vector>

#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/user.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

namespace WongWork {

// IDA_EA: 0x854678A
// IDA_NAME: _ZN8WongWork12CItemUpgrade15_GetUpgradeInfoEPK10Inven_ItemPK5CItemRK15upgrade_table_tP14upgrade_info_ti
// DEMANGLED: WongWork::CItemUpgrade::_GetUpgradeInfo(Inven_Item const*,CItem const*,upgrade_table_t const&,upgrade_info_t *,int)
int CItemUpgrade::_GetUpgradeInfo(const Inven_Item* target,
                                  const CItem* item,
                                  const upgrade_table_t& table,
                                  upgrade_info_t* output,
                                  int charac_level) {
    if (static_cast<uint8_t>(target->GetUpgrade()) > 30u) {
        return 0;
    }

    const uint8_t growth_grade = static_cast<uint8_t>(
        static_cast<const CEquipItem*>(item)->getGrowthGrade(charac_level));
    const ENUM_EQUIPMENTTYPE equipment_type =
        static_cast<ENUM_EQUIPMENTTYPE>(item->vtable_->get_item_type(item));
    const uint8_t upgrade = static_cast<uint8_t>(target->GetUpgrade());
    const uint8_t rarity = static_cast<uint8_t>(item->get_rarity());
    return table.get_upgrade_info(
        output, rarity, growth_grade, upgrade, equipment_type);
}

// IDA_EA: 0x8546824
// IDA_NAME: _ZN8WongWork12CItemUpgrade14_NoticeUpgradeEPK5CUserRK10Inven_Itemb
// DEMANGLED: WongWork::CItemUpgrade::_NoticeUpgrade(CUser const*,Inven_Item const&,bool)
void CItemUpgrade::_NoticeUpgrade(const CUser* user,
                                  const Inven_Item& target,
                                  bool succeeded) {
    PacketGuard packet;
    packet.put_header(0, 86);
    packet.put_byte(1);
    packet.put_byte(succeeded);
    packet.put_short((const_cast<CUser*>(user))->get_unique_id());
    packet.put_int(static_cast<int>(target.item_id_));
    packet.put_byte(target.GetUpgrade());
    target.chronicle_random_option_.random_option()
        .put_packet_random_option(packet);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x8548C3C
// IDA_NAME: _ZNK8WongWork12CItemUpgrade20_CheckTargetUsableLvERK10Inven_ItemS3_
// DEMANGLED: WongWork::CItemUpgrade::_CheckTargetUsableLv(Inven_Item const&,Inven_Item const&)const
bool CItemUpgrade::_CheckTargetUsableLv(
    const Inven_Item& upgrade_item,
    const Inven_Item& target) const {
    CItem* const item = G_CDataManager()->find_item(
        static_cast<int>(upgrade_item.item_id_));
    if (!item) {
        return false;
    }
    if (static_cast<uint8_t>(item->is_stackable()) != 1u) {
        return true;
    }

    const CStackableItem* const stackable =
        static_cast<const CStackableItem*>(item);
    if (!stackable->has_usable_item_level_range()) {
        return true;
    }

    CItem* const target_item = G_CDataManager()->find_item(
        static_cast<int>(target.item_id_));
    if (!target_item || !target.isEquipableItemType()) {
        return false;
    }

    const int usable_level = target_item->getUsableLevel();
    return stackable->usable_item_level_min() <= usable_level &&
           usable_level <= stackable->usable_item_level_max();
}

// IDA_EA: 0x8548D3C
// IDA_NAME: _ZNK8WongWork12CItemUpgrade24_ProcPortableUpgradeItemERK10Inven_ItemS3_R14upgrade_info_t
// DEMANGLED: WongWork::CItemUpgrade::_ProcPortableUpgradeItem(Inven_Item const&,Inven_Item const&,upgrade_info_t &)const
int CItemUpgrade::_ProcPortableUpgradeItem(
    const Inven_Item& upgrade_item,
    const Inven_Item& target,
    upgrade_info_t& output) const {
    CItem* const item = G_CDataManager()->find_item(
        static_cast<int>(upgrade_item.item_id_));
    if (!item) {
        return 4;
    }
    if (static_cast<uint8_t>(item->is_stackable()) != 1u) {
        return 0;
    }

    const CStackableItem* const stackable =
        static_cast<const CStackableItem*>(item);
    if (!stackable->verifyStackableAction(
            static_cast<StackableAction::Type>(53))) {
        return 0;
    }

    const std::vector<int>& action_values = stackable->action_values();
    if (action_values.size() != 2u) {
        return 4;
    }

    const uint8_t upgrade = static_cast<uint8_t>(target.GetUpgrade());
    if (upgrade < action_values[0] || upgrade > action_values[1]) {
        return 19;
    }

    output.calculated_cost = 0;
    output.event_material_item_count = 1;
    output.event_material_item_index =
        static_cast<int>(upgrade_item.item_id_);
    return 0;
}

// IDA_EA: 0x8548E68
// IDA_NAME: _ZN8WongWork12CItemUpgrade29_getUpgradeCountByUpgradeItemERK10Inven_ItemS3_R10ENUM_ERROR
// DEMANGLED: WongWork::CItemUpgrade::_getUpgradeCountByUpgradeItem(Inven_Item const&,Inven_Item const&,ENUM_ERROR &)
int CItemUpgrade::_getUpgradeCountByUpgradeItem(
    const Inven_Item& target,
    const Inven_Item& upgrade_item,
    ENUM_ERROR& error) {
    uint16_t fixed_count = 0;
    if (!target.amplify_option_.hasAbility()) {
        switch (upgrade_item.item_id_) {
        case 7273u:
            fixed_count = 7;
            break;
        case 7274u:
            fixed_count = 10;
            break;
        case 7275u:
            fixed_count = 12;
            break;
        case 7465u:
            fixed_count = 7;
            break;
        case 7743u:
            fixed_count = 3;
            break;
        case 7744u:
            fixed_count = 5;
            break;
        case 7814u:
            fixed_count = 13;
            break;
        default:
            fixed_count = 0;
            break;
        }
    }

    CItem* const item = G_CDataManager()->find_item(
        static_cast<int>(upgrade_item.item_id_));
    if (!item || !static_cast<uint8_t>(item->is_stackable())) {
        return fixed_count;
    }

    CStackableItem* const stackable = static_cast<CStackableItem*>(item);
    if (static_cast<uint8_t>(stackable->IsReinforceTicket())) {
        const unsigned int value = static_cast<unsigned int>(
            static_cast<int>(static_cast<int16_t>(
                stackable->GetReinforceVaule())));
        return value < 32u ? static_cast<uint16_t>(value) : 0;
    }

    if (static_cast<uint8_t>(stackable->IsAmplifyReinforceTicket())) {
        if (!target.amplify_option_.hasAbility()) {
            error = static_cast<ENUM_ERROR>(19);
            return 0;
        }
        const unsigned int value = static_cast<unsigned int>(
            static_cast<int>(static_cast<int16_t>(
                stackable->GetAmplifyReinforceValue())));
        return value < 32u ? static_cast<uint16_t>(value) : 0;
    }

    return fixed_count;
}

// IDA_EA: 0x8548FFA
// IDA_NAME: _ZN8WongWork12CItemUpgrade28_getUpgradeRateByUpgradeItemERK10Inven_ItemS3_R10ENUM_ERROR
// DEMANGLED: WongWork::CItemUpgrade::_getUpgradeRateByUpgradeItem(Inven_Item const&,Inven_Item const&,ENUM_ERROR &)
int CItemUpgrade::_getUpgradeRateByUpgradeItem(
    const Inven_Item& target,
    const Inven_Item& upgrade_item,
    ENUM_ERROR& error) {
    if (!target.amplify_option_.hasAbility()) {
        switch (upgrade_item.item_id_) {
        case 7273u:
        case 7274u:
        case 7275u:
        case 7465u:
        case 7743u:
        case 7744u:
        case 7814u:
            return 100;
        default:
            break;
        }
    }

    CItem* const item = G_CDataManager()->find_item(
        static_cast<int>(upgrade_item.item_id_));
    if (!item || !static_cast<uint8_t>(item->is_stackable())) {
        return 0;
    }

    CStackableItem* const stackable = static_cast<CStackableItem*>(item);
    if (static_cast<uint8_t>(stackable->IsReinforceTicket())) {
        const uint8_t rate =
            static_cast<uint8_t>(stackable->GetReinforceRate());
        return rate <= 100u ? rate : 0;
    }

    if (static_cast<uint8_t>(stackable->IsAmplifyReinforceTicket())) {
        if (!target.amplify_option_.hasAbility()) {
            error = static_cast<ENUM_ERROR>(19);
            return 0;
        }
        const int8_t signed_rate =
            static_cast<int8_t>(stackable->GetAmplifyReinforceRate());
        const unsigned int rate = static_cast<unsigned int>(
            static_cast<int>(signed_rate));
        return rate <= 100u ? static_cast<uint8_t>(rate) : 0;
    }

    return 0;
}

}  // namespace WongWork
