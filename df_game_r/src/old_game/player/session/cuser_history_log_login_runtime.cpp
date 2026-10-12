#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include <stdint.h>

#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"

namespace {

// IDA cUserHistoryLog::Login @0x8683D86 obtains these two physical spaces
// through CInventory::GetInvenRef.  The values are the original inventory
// space selectors, not a surrogate CUser layout.
const int kEquipmentInventorySpace = 0;
const int kAvatarInventorySpace = 2;

}  // namespace

// IDA_EA: 0x8683D86
// IDA_NAME: _ZN15cUserHistoryLog5LoginEPKcS1_iihi
// DEMANGLED: cUserHistoryLog::Login(
//     char const*,char const*,int,int,unsigned char,int)
int cUserHistoryLog::Login(const char* web_address,
                           const char* peer_ip,
                           int peer_port,
                           int channel_no,
                           unsigned char login_out_state,
                           int server_group) {
    // The original receives this address from Inter_LoadEtc but writes the
    // common header through CUser::LogHistory, so it does not consume the
    // parameter again in this body.
    (void)web_address;

    int login_kind = 0;
    int weapon_upgrade = 0;
    int weapon_item_id = 0;
    int other_equipment_upgrade_total = 0;
    int common_or_lower_equipment_count = 0;

    CInventory* const current_inventory = owner_->getCurCharacInvenR();
    const int money = current_inventory->get_money();

    for (int slot = oldsrv::config::kEquipmentTypeWeapon;
         slot <= oldsrv::config::kEquipmentTypeWrist;
         ++slot) {
        CInventory* const inventory = owner_->getCurCharacInvenR();
        const Inven_Item* const inventory_item =
            inventory->GetInvenRef(kEquipmentInventorySpace, slot);
        if (!inventory_item || !inventory_item->item_id_) {
            continue;
        }

        if (slot == oldsrv::config::kEquipmentTypeWeapon) {
            weapon_upgrade =
                static_cast<unsigned char>(inventory_item->GetUpgrade());
            weapon_item_id = inventory_item->item_id_;
        } else {
            other_equipment_upgrade_total +=
                static_cast<unsigned char>(inventory_item->GetUpgrade());
        }

        CItem* const item = G_CDataManager()->find_item(inventory_item->item_id_);
        if (item && static_cast<unsigned int>(item->get_rarity()) <= 1u) {
            ++common_or_lower_equipment_count;
        }
    }

    int avatar_item_count = 0;
    for (int slot = 0; slot <= 115; ++slot) {
        CInventory* const inventory = owner_->getCurCharacInvenR();
        const Inven_Item* const inventory_item =
            inventory->GetInvenRef(kAvatarInventorySpace, slot);
        if (inventory_item && inventory_item->item_id_) {
            ++avatar_item_count;
        }
    }

    const _Charac_info* const current_character = owner_->getCurCharacR();
    const uint32_t last_play_tick = current_character->fields_.last_play_tick_;
    const int first_login = owner_->GetFirstLogin();
    if (last_play_tick || !first_login) {
        if (login_out_state || !last_play_tick) {
            if (login_out_state == 1 && last_play_tick) {
                login_kind = 2;
            } else if (login_out_state == 2 && last_play_tick) {
                login_kind = 3;
            }
        } else {
            login_kind = 1;
        }
    }

    const int expert_job_type_for_script = owner_->GetCurCharacExpertJobType();
    STExpertJobScript* const expert_job_script =
        G_CDataManager()->GetExpertJobScript(expert_job_type_for_script);
    const void* expert_job_level = 0;
    if (expert_job_script) {
        expert_job_level = reinterpret_cast<const void*>(
            static_cast<uintptr_t>(expert_job_script->GetLevel(
                static_cast<uint32_t>(owner_->GetCurCharacExpertJobExp()))));
    }

    const int expert_job_type = owner_->GetCurCharacExpertJobType();
    const int second_grow_type = owner_->getCurCharSecondGrowType();
    const int first_grow_type = owner_->getCurCharFirstGrowType();
    const int character_level = owner_->get_charac_level();
    const int character_job = owner_->get_charac_job();
    return owner_->LogHistory(
        "IP+,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%p",
        peer_ip,
        peer_port,
        channel_no,
        login_out_state,
        server_group,
        character_job,
        character_level,
        first_grow_type,
        second_grow_type,
        login_kind,
        money,
        weapon_upgrade,
        weapon_item_id,
        other_equipment_upgrade_total,
        common_or_lower_equipment_count,
        avatar_item_count,
        expert_job_type,
        expert_job_level);
}
