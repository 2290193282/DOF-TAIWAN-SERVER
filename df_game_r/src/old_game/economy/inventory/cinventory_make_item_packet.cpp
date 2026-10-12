#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/appearance/pet/creature.h"
#include "common/config.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/os_api.h"
#include "old_game/economy/inventory/inventory_public_abi.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>


extern int GetIntegratedPvPItemAttrAt(const Inven_Item& item,
                                      int inventory_type,
                                      int slot)
    __asm__("_Z24GetIntegratedPvPItemAttrRK10Inven_Item10INVEN_TYPEi");
// IDA_EA: 0x84FC6BC
// IDA_NAME: _ZNK10CInventory14MakeItemPacketE10INVEN_TYPEiR11PacketGuard
// DEMANGLED: CInventory::MakeItemPacket(INVEN_TYPE,int,PacketGuard &)const
// Active reconstructed-tree ABI:
// _ZNK10CInventory14MakeItemPacketE10INVEN_TYPEiR11PacketGuardE.
int CInventory::MakeItemPacket(INVEN_TYPE inventory_type,
                               int slot,
                               PacketGuard& packet) const {
    const unsigned int item_slot = static_cast<unsigned int>(slot);

    if (inventory_type == 1) {
        packet.put_short(slot);
        if (slot != 0) {
            if (slot == 1) {
                packet.put_int(1);
                packet.put_int(coin_ + pay_coin_ + event_coin_);
                packet.put_byte(0);
                packet.put_short(0);
                packet.put_byte(0);
                packet.put_int(0);
                packet.put_byte(0);
                packet.put_short(0);
                packet.put_byte(0);
                packet.put_packet(g_emptySlot);
            } else if (slot == 2) {
                packet.put_int(2);
                packet.put_int(parent_->GetWinPoint());
                packet.put_byte(0);
                packet.put_short(0);
                packet.put_byte(0);
                packet.put_int(0);
                packet.put_byte(0);
                packet.put_short(0);
                packet.put_byte(0);
                packet.put_packet(g_emptySlot);
            } else {
                const Inven_Item* const item = &stackable_items_[item_slot];
                if (item->item_id_ != 0) {
                    packet.put_int(item->item_id_);
                    packet.put_int(item->add_info_);
                    packet.put_byte(GetIntegratedPvPItemAttrAt(
                        *item, inventory_type, slot));
                    packet.put_short(item->avatar_select_);
                    packet.put_byte(item->seal_or_state_);
                    if (G_GameWorld()->IsEnchantRevisionChannel()) {
                        packet.put_int(0);
                    } else {
                        packet.put_int(item->extend_info_);
                    }
                    const stAmplifyOption_t* const amplify =
                        &item->amplify_option_;
                    packet.put_byte(amplify->getAbilityType());
                    packet.put_short(amplify->getAbilityValue());
                    CDataManager* const data_manager = G_CDataManager();
                    (data_manager->get_item_upgrade())->make3rdChroniclePacket(
                        parent_,
                        *item,
                        &packet);
                    packet.put_packet(*item);
                } else {
                    packet.put_item_idx(static_cast<unsigned long>(-1));
                    packet.put_int(item->add_info_);
                    packet.put_byte(GetIntegratedPvPItemAttrAt(
                        *item, inventory_type, slot));
                    packet.put_short(item->avatar_select_);
                    packet.put_byte(item->seal_or_state_);
                    if (G_GameWorld()->IsEnchantRevisionChannel()) {
                        packet.put_int(0);
                    } else {
                        packet.put_int(item->extend_info_);
                    }
                    const stAmplifyOption_t* const amplify =
                        &item->amplify_option_;
                    packet.put_byte(amplify->getAbilityType());
                    packet.put_short(amplify->getAbilityValue());
                    packet.put_byte(0);
                    packet.put_packet(*item);
                }
            }
        } else {
            packet.put_int(0);
            packet.put_int(money_);
            packet.put_byte(0);
            packet.put_short(0);
            packet.put_byte(0);
            packet.put_int(0);
            packet.put_byte(0);
            packet.put_short(0);
            packet.put_byte(0);
            packet.put_packet(g_emptySlot);
        }
        return 1;
    }

    if (inventory_type == 2) {
        const Inven_Item* const item = &avatar_items_[item_slot];
        if (item->item_id_ == 0) {
            cMyTrace trace(
                "bool CInventory::MakeItemPacket(INVEN_TYPE, int, PacketGuard&) const",
                1952,
                5);
            trace("[%s][%d]",
                  "bool CInventory::MakeItemPacket(INVEN_TYPE, int, PacketGuard&) const");
            return 0;
        }

        packet.put_short(slot);
        packet.put_int(item->item_id_);
        const WongWork::CAvatarItemMgr* const avatar_manager =
            &avatar_item_manager_;
        packet.put_int(avatar_manager->GetRemainDate(
            item->add_info_,
            OS_API::GetDateTimeTick()));
        packet.put_byte(item->GetItemAttr());
        packet.put_short(item->avatar_select_);
        packet.put_byte(item->seal_or_state_);
        packet.put_int(item->extend_info_);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_int(30);
        const stAvatarEmblemInfo_t* const jewel =
            avatar_manager->getJewelSocketData(
                item->add_info_);
        // CAvatarItemMgr returns null for an avatar with no socket record.
        // NOTI13/14 still consume the fixed 30-byte socket payload, so use
        // the official value type's initialized empty representation rather
        // than passing null to PacketBuf::put_binary.
        stAvatarEmblemInfo_t empty_jewel;
        empty_jewel.init();
        packet.put_binary(reinterpret_cast<const char*>(
                              jewel ? jewel : &empty_jewel),
                          30);
        packet.put_int(4);
        const stAvatarExpansionInfo_t* const expansion =
            avatar_manager->GetExpansionInfo(
                item->add_info_);
        if (expansion) {
            packet.put_binary(reinterpret_cast<const char*>(expansion), 4);
        } else {
            stAvatarExpansionInfo_t empty_expansion;
            empty_expansion.init();
            packet.put_binary(reinterpret_cast<const char*>(&empty_expansion),
                              4);
        }
        // 0828 NOTI13 avatar records carry the third Chronicle list here.
        // Serialize it through the native CItemUpgrade owner before the
        // shared Inven_Item tail.
        (G_CDataManager()->get_item_upgrade())->make3rdChroniclePacket(
            parent_, *item, &packet);
        packet.put_packet(*item);
        return 1;
    }

    if (inventory_type == 3) {
        const Inven_Item* const item = &creature_items_[item_slot];
        if (item->item_id_ == 0) {
            cMyTrace trace(
                "bool CInventory::MakeItemPacket(INVEN_TYPE, int, PacketGuard&) const",
                2049,
                5);
            trace("[%s][%d]",
                  "bool CInventory::MakeItemPacket(INVEN_TYPE, int, PacketGuard&) const");
            return 0;
        }

        packet.put_short(slot);
        packet.put_int(item->item_id_);
        packet.put_int(item->add_info_);
        packet.put_byte(item->GetItemAttr());
        packet.put_short(item->avatar_select_);
        packet.put_byte(item->seal_or_state_);
        packet.put_int(item->extend_info_);
        packet.put_byte(0);
        packet.put_short(0);
        // NOTI13 itemspace 7 has a variable tail: the client consumes the
        // remain date only for a creature prototype. Resolve the CItem owner
        // instead of treating the persisted category as a wire discriminator.
        CItem* const item_script = G_CDataManager()->find_item(
            static_cast<unsigned int>(item->item_id_));
        if (item_script && item_script->vtable_ &&
            item_script->vtable_->get_item_type(item_script) ==
                oldsrv::config::kEquipmentTypeCreature) {
            const user_creature::CCreatureMgr* const creature_manager =
                GetCreatureMgrR();
            packet.put_int(creature_manager->GetRemainDate(
                item->add_info_,
                OS_API::GetDateTimeTick()));
        }
        // The type=7 reader keeps one client-only byte between the optional
        // remain date and the shared Inven_Item tail.  Unlike itemspaces 0/1/2,
        // this record does not carry the third Chronicle list here.
        packet.put_byte(0);
        packet.put_packet(*item);
        return 1;
    }

    if (inventory_type != 0) {
        return 0;
    }

    const Inven_Item* const item = &equipment_items_[item_slot];
    if (item->item_id_ == 0) {
        cMyTrace trace(
            "bool CInventory::MakeItemPacket(INVEN_TYPE, int, PacketGuard&) const",
            2180,
            5);
        trace("[%s][%d]",
              "bool CInventory::MakeItemPacket(INVEN_TYPE, int, PacketGuard&) const");
        return 0;
    }

    packet.put_short(slot);
    packet.put_int(item->item_id_);
    packet.put_int(item->add_info_);
    if (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
        !G_CDataManager()->GetUpgradeRevisionPvPChannel()) {
        unsigned char item_attr = 0;
        const unsigned int reseal_equipment_mask =
            oldsrv::config::kOrdinaryEquipmentTypeMask &
            ~(1u << oldsrv::config::kEquipmentTypeTitle);
        if (item_slot < oldsrv::config::kEquipmentTypeCount &&
            ((1u << item_slot) & reseal_equipment_mask) != 0u) {
            item_attr = static_cast<unsigned char>(32 * item->GetReSealCount());
        } else {
            item_attr = static_cast<unsigned char>(item->GetItemAttr());
        }
        packet.put_byte(item_attr);
    } else {
        packet.put_byte(item->GetItemAttr());
    }
    packet.put_short(item->avatar_select_);
    packet.put_byte(item->seal_or_state_);
    if (G_GameWorld()->IsEnchantRevisionChannel()) {
        packet.put_int(0);
    } else {
        packet.put_int(item->extend_info_);
    }
    const stAmplifyOption_t* const amplify = &item->amplify_option_;
    packet.put_byte(amplify->getAbilityType());
    packet.put_short(amplify->getAbilityValue());
    if (item->isAvatarItemType()) {
        const WongWork::CAvatarItemMgr* const avatar_manager =
            &avatar_item_manager_;
        packet.put_int(30);
        const stAvatarEmblemInfo_t* const jewel =
            avatar_manager->getJewelSocketData(
                item->add_info_);
        // The socket block is mandatory even when the avatar has no emblem
        // owner entry; serialize the typed empty record in that case.
        stAvatarEmblemInfo_t empty_jewel;
        empty_jewel.init();
        packet.put_binary(reinterpret_cast<const char*>(
                              jewel ? jewel : &empty_jewel),
                          30);
        packet.put_int(4);
        const stAvatarExpansionInfo_t* const expansion =
            avatar_manager->GetExpansionInfo(
                item->add_info_);
        if (expansion) {
            packet.put_binary(reinterpret_cast<const char*>(expansion), 4);
        } else {
            stAvatarExpansionInfo_t empty_expansion;
            empty_expansion.init();
            packet.put_binary(reinterpret_cast<const char*>(&empty_expansion),
                              4);
        }
    }
    if (item->item_category_ == 5) {
        const user_creature::CCreatureMgr* const creature_manager =
            GetCreatureMgrR();
        packet.put_int(creature_manager->GetRemainDate(
            item->add_info_,
            OS_API::GetDateTimeTick()));
    }
    CDataManager* const data_manager = G_CDataManager();
    (data_manager->get_item_upgrade())->make3rdChroniclePacket(
        parent_,
        *item,
        &packet);
    packet.put_packet(*item);
    return 1;
}
