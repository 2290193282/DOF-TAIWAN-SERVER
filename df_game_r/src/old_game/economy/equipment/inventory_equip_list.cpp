#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/equipment/clone_title_runtime.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"

#include <cstring>
#include <stdint.h>

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/os_api.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "common/manager/user.h"
#include "old_game/player/core/cuser.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/interface_packet_buf.h"
#include "infra/network/packet/packet_guard.h"


namespace {

const unsigned int kInventoryFullNormalScanCount =
    oldsrv::config::kEquipmentTypeCount;
const unsigned int kInventoryCompactNormalScanCount =
    oldsrv::config::kEquipmentTypeTitle + 1u;
const unsigned int kInventoryExpandedScanCount =
    oldsrv::config::kOrdinaryEquipmentTypeCount;
const unsigned int kFullAvatarSkinEquipmentSlot =
    oldsrv::config::kEquipmentTypeSkinAvatar;

const Inven_Item* ItemAt(const Inven_Item* table, unsigned int index) {
    return &table[index];
}

bool ShouldUsePvpUpgradeRule() {
    return G_GameWorld()->IsIntegratedPvPBaseChannel() &&
           !G_CDataManager()->GetUpgradeRevisionPvPChannel();
}

unsigned char FullItemAttr(const CUser* owner,
                           const Inven_Item* item,
                           int network_slot) {
    if (ShouldUsePvpUpgradeRule()) {
        if (network_slot == oldsrv::config::kEquipmentTypeWeapon ||
            (network_slot >= oldsrv::config::kEquipmentTypeCoat &&
             network_slot <= oldsrv::config::kEquipmentTypeMagicStone)) {
            return static_cast<unsigned char>(32 * item->GetReSealCount());
        }
        return static_cast<unsigned char>(item->GetItemAttr());
    }
    const int projected_upgrade =
        champion_break::GetProjectedEquipmentUpgrade(
            const_cast<CUser*>(owner), network_slot, item->item_id_,
            item->globally_unique_id_, item->GetUpgrade());
    return static_cast<unsigned char>(
        (item->GetItemAttr() & 0xe0) | (projected_upgrade & 0x1f));
}

unsigned char CompactUpgradeByte(const CUser* owner,
                                 const Inven_Item* item,
                                 int network_slot) {
    unsigned char upgrade = 0;
    if (ShouldUsePvpUpgradeRule()) {
        if (network_slot != oldsrv::config::kEquipmentTypeWeapon) {
            if (network_slot < oldsrv::config::kEquipmentTypeWeapon ||
                network_slot < oldsrv::config::kEquipmentTypeCoat ||
                network_slot > oldsrv::config::kEquipmentTypeMagicStone) {
                upgrade = static_cast<unsigned char>(item->GetUpgrade());
            }
        }
    } else {
        upgrade = static_cast<unsigned char>(
            champion_break::GetProjectedEquipmentUpgrade(
                const_cast<CUser*>(owner), network_slot, item->item_id_,
                item->globally_unique_id_, item->GetUpgrade()));
    }

    const bool has_amplify =
        static_cast<unsigned char>(
            item->amplify_option_.getAbilityType()) != 0;
    return static_cast<unsigned char>((has_amplify ? 1u : 0u) |
                                      (2u * (upgrade & 0x7fu)));
}

int EnchantValue(const Inven_Item* item) {
    if (G_GameWorld()->IsEnchantRevisionChannel()) {
        return 0;
    }
    return item->extend_info_;
}

const stAvatarExpansionInfo_t* CachedAvatarExpansion(
    const CInventory::AvatarVisibleData* visible_data,
    unsigned int raw_slot,
    stAvatarExpansionInfo_t& scratch) {
    scratch.init();
    if (raw_slot <= oldsrv::config::kAvatarEquipmentTypeLast) {
        std::memcpy(&scratch,
                    &visible_data[raw_slot].avatar_expansion_0,
                    sizeof(scratch));
    }
    return &scratch;
}

const stAvatarExpansionInfo_t* RuntimeAvatarExpansion(
    const Inven_Item* equipment_items,
    const WongWork::CAvatarItemMgr* avatar_manager,
    const Inven_Item* item,
    unsigned int raw_slot,
    stAvatarExpansionInfo_t& scratch) {
    scratch.init();
    if (raw_slot <= oldsrv::config::kAvatarEquipmentTypeLast &&
        equipment_items[raw_slot].item_id_ != 0) {
        const int avatar_key = item->add_info_;
        const stAvatarExpansionInfo_t* const expansion =
            avatar_manager->GetExpansionInfo(avatar_key);
        if (expansion) {
            std::memcpy(&scratch, expansion, sizeof(scratch));
        }
    }
    return &scratch;
}

const Inven_Item* SelectItemTable(const CUser* parent,
                                  const Inven_Item* equipment_items,
                                  int userinfo,
                                  int equip_slot) {
    if (userinfo != 2 && equip_slot != 0) {
        if (!parent) {
            return 0;
        }
        CExpandEquipslot* expand = reinterpret_cast<CExpandEquipslot*>(
            parent->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
        if (!expand) {
            return 0;
        }
        return expand->getExpandEquipslotW(
            static_cast<ENUM_EQUIPSLOT>(equip_slot));
    }
    return equipment_items;
}

void PutFullAvatarTail(
    const WongWork::CAvatarItemMgr* avatar_manager,
    const CInventory::AvatarVisibleData* visible_data,
    const Inven_Item* item,
    unsigned int raw_slot,
    InterfacePacketBuf* packet) {
    packet->put_int(30);
    const int avatar_key = item->add_info_;
    const stAvatarEmblemInfo_t* const jewel =
        avatar_manager->getJewelSocketData(avatar_key);
    // 穿戴与选择角色的 reader 即使没有徽章记录，也会读取固定长度区块。
    // 回退值使用官方 30 字节类型，不使用临时字节数组。
    stAvatarEmblemInfo_t empty_jewel;
    empty_jewel.init();
    packet->put_binary(reinterpret_cast<const char*>(
                           jewel ? jewel : &empty_jewel),
                       30);
    packet->put_int(4);

    stAvatarExpansionInfo_t expansion;
    const stAvatarExpansionInfo_t* const runtime_expansion =
        avatar_manager->GetExpansionInfo(avatar_key);
    if (runtime_expansion) {
        packet->put_binary(
            reinterpret_cast<const char*>(runtime_expansion), 4);
    } else {
        packet->put_binary(
            reinterpret_cast<const char*>(
                CachedAvatarExpansion(visible_data, raw_slot, expansion)),
            4);
    }
}

void PutFullChronicleTail(CUser* parent,
                          const Inven_Item* item,
                          PacketGuard* packet) {
    CDataManager* data_manager = G_CDataManager();
    WongWork::CItemUpgrade* item_upgrade =
        data_manager ? data_manager->get_item_upgrade() : 0;
    item_upgrade->make3rdChroniclePacket(
        parent,
        *item,
        packet);
    // CLIENT_IDA_EVIDENCE: ParseEquippedItemDetailList @0x5A3962 reads
    // exactly one u32 between the third-Chronicle block and the random-option
    // block. PacketBuf::put_packet owns that field and serializes the real
    // ChronicleRandomOptionInfo::value_; do not add a second placeholder here.
    packet->put_packet(*item);
}

}  // namespace

// The minimum USERINFO reader consumes this Equipment.lst item id after the
// plain appearance list and forwards it to
// ActorRebuildOwnedEquipmentOverlayFromItemId. Zero clears the overlay.
uint32_t CInventory::GetFullAvatarEquipmentItemId() const {
    const Inven_Item& skin =
        equipment_items_[kFullAvatarSkinEquipmentSlot];
    if (skin.item_id_ <= 0) {
        return 0;
    }

    CDataManager* const data_manager = G_CDataManager();
    CItem* const prototype = data_manager
        ? data_manager->find_item(skin.item_id_)
        : 0;
    if (!prototype || !prototype->vtable_ ||
        !prototype->vtable_->is_avatar_item ||
        !prototype->vtable_->is_avatar_item(prototype)) {
        return 0;
    }

    const CEquipItem* const equipment =
        static_cast<const CEquipItem*>(prototype);
    return equipment->IsFullAvatar()
        ? static_cast<uint32_t>(skin.item_id_)
        : 0u;
}

// IDA_EA: 0x84FE2E2
// IDA_NAME: _ZNK10CInventory13MakeEquipListEPvb13ENUM_USERINFO14ENUM_EQUIPSLOT
// DEMANGLED: CInventory::MakeEquipList(void *,bool,ENUM_USERINFO,ENUM_EQUIPSLOT)const
int CInventory::MakeEquipList(void* packet_storage,
                              bool full,
                              ENUM_USERINFO userinfo,
                              ENUM_EQUIPSLOT equip_slot) const {
    PacketGuard* packet_guard =
        reinterpret_cast<PacketGuard*>(packet_storage);
    if (!packet_guard) {
        return 0;
    }
    InterfacePacketBuf* packet = packet_guard;

    const int userinfo_value = static_cast<int>(userinfo);
    const int equip_slot_value = static_cast<int>(equip_slot);

    const unsigned int scan_count = equip_slot_value != 0
        ? kInventoryExpandedScanCount
        : (full ? kInventoryFullNormalScanCount
                : kInventoryCompactNormalScanCount);
    int count_index = packet->get_index();
    int output_count = 0;
    packet->put_byte(output_count);

    const int tick = OS_API::GetDateTimeTick();
    const Inven_Item* item_table =
        SelectItemTable(parent_, equipment_items_, userinfo_value,
                        equip_slot_value);
    if (!item_table) {
        packet->put_byte(count_index, output_count);
        if (full) {
            // ParseEquippedItemDetailList @0x5A3D0F reads this even for an
            // empty list. @0x5A3D29 forwards it to the same clone-title
            // manager setter used by CMD562 handler sub_128BC70.
            packet->put_int(clone_title::GetForInventory(parent_, this));
        } else if (userinfo_value != 2) {
            if (userinfo_value == 0) {
                packet->put_int(GetFullAvatarEquipmentItemId());
            }
            packet->put_byte(0);
        }
        return 1;
    }

    for (unsigned int raw_slot = 0; raw_slot < scan_count; ++raw_slot) {
        const Inven_Item* item = ItemAt(item_table, raw_slot);
        if (item->item_id_ == 0) {
            continue;
        }

        const int network_slot =
            equip_slot_value != 0
                ? static_cast<int>(
                      raw_slot + oldsrv::config::kOrdinaryEquipmentTypeFirst)
                                  : static_cast<int>(raw_slot);
        packet->put_byte(network_slot);

        if (full) {
            packet->put_int(item->item_id_);
            if (network_slot > oldsrv::config::kAvatarEquipmentTypeLast) {
                packet->put_int(item->add_info_);
            } else {
                packet->put_int(avatar_item_manager_.GetRemainDate(
                    item->add_info_, tick));
            }
            packet->put_byte(FullItemAttr(parent_, item, network_slot));
            packet->put_short(item->avatar_select_);
        } else {
            if (userinfo_value == 2 &&
                GetClearAvatar(network_slot) != 0) {
                packet->put_int(GetClearAvatar(network_slot));
            } else {
                packet->put_int(item->item_id_);
            }
            // CLIENT_IDA_EVIDENCE: ParsePlainCharacterAppearanceRecords calls
            // the format-31 item decoder for every USERINFO type-2 item. That
            // decoder consumes a u32 payload length followed by the payload,
            // just like the minimum USERINFO path.
            if (userinfo_value == 2) {
                stAvatarExpansionInfo_t expansion;
                packet->put_int(4);
                packet->put_binary(
                    reinterpret_cast<const char*>(
                        CachedAvatarExpansion(avatar_visible_data_, raw_slot,
                                              expansion)),
                    4);
            } else if (userinfo_value == 0) {
                stAvatarExpansionInfo_t expansion;
                packet->put_int(4);
                packet->put_binary(
                    reinterpret_cast<const char*>(
                        RuntimeAvatarExpansion(equipment_items_,
                                               &avatar_item_manager_, item,
                                               raw_slot,
                                               expansion)),
                    4);
            }
            packet->put_byte(
                CompactUpgradeByte(parent_, item, network_slot));
        }

        packet->put_int(GetClearAvatar(network_slot));
        packet->put_int(EnchantValue(item));

        if (full) {
            packet->put_byte(
                static_cast<unsigned char>(
                    item->amplify_option_.getAbilityType()));
            packet->put_short(
                static_cast<unsigned short>(
                    item->amplify_option_.getAbilityValue()));

            if (item->isAvatarItemType()) {
                PutFullAvatarTail(&avatar_item_manager_,
                                  avatar_visible_data_, item, raw_slot,
                                  packet);
            }
            if (item->item_category_ == 5) {
                packet->put_int(creature_manager_.GetRemainDate(
                    item->add_info_,
                    OS_API::GetDateTimeTick()));
            }
            PutFullChronicleTail(parent_, item, packet_guard);
        }

        ++output_count;
    }

    packet->put_byte(count_index, output_count);
    if (full) {
        // This full-detail tail is clone-title state.  The compact/minimum
        // tail below remains the independent slot-8 full-avatar overlay ID.
        packet->put_int(clone_title::GetForInventory(parent_, this));
    } else if (userinfo_value != 2) {
        // The 0627 compact writer appended UpgradeSeparate here.  The native
        // 0828 USERINFO-2 reader returns immediately after the appearance
        // record loop (DNF.exe 0x733E90), and its caller reads guild_id as the
        // next u32 (0x73740E).  Therefore type 2 must not emit that legacy
        // byte; doing so shifts every remaining character-list field.
        if (userinfo_value == 0) {
            packet->put_int(GetFullAvatarEquipmentItemId());
        }
        if (ShouldUsePvpUpgradeRule()) {
            packet->put_byte(0);
        } else {
            packet->put_byte(
                item_table[oldsrv::config::kEquipmentTypeWeapon]
                    .upgrade_separate_.GetUpgradeSeparate());
        }
    }
    return 1;
}

// IDA_EA: 0x850D374
// IDA_NAME: _ZNK10CInventory14GetClearAvatarEi
// DEMANGLED: CInventory::GetClearAvatar(int)const
int CInventory::GetClearAvatar(int slot) const {
    if (slot < oldsrv::config::kAvatarEquipmentTypeFirst ||
        slot >= oldsrv::config::kPersistedClearAvatarCount) {
        return 0;
    }
    return clear_avatar_[slot];
}

// IDA_EA: 0x869498A
// IDA_NAME: _ZNK10CInventory14GetClearAvatarEPm
// DEMANGLED: CInventory::GetClearAvatar(unsigned long *)const
bool CInventory::GetClearAvatar(unsigned long* clear_avatar) const {
    bool result = false;
    for (int slot = 0;; ++slot) {
        result = slot < oldsrv::config::kPersistedClearAvatarCount;
        if (slot >= oldsrv::config::kPersistedClearAvatarCount) {
            break;
        }
        clear_avatar[slot] =
            static_cast<unsigned long>(GetClearAvatar(slot));
    }
    return result;
}
