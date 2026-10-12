#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x86800C6
// IDA_NAME: _ZN5CUser17onSelectCharacterEv
// DEMANGLED: CUser::onSelectCharacter(void)
int CUser::onSelectCharacter() {
    auto_client_rate_ = 100;
    mcap_manager_.resetIncRate();

    int equipped_count = 0;
    int rare_equipped_count = 0;
    CInventory* const inventory = getCurCharacInvenR();

    for (int slot = oldsrv::config::kAvatarEquipmentTypeFirst;
         slot <= oldsrv::config::kAvatarEquipmentTypeLast;
         ++slot) {
        const Inven_Item equipped = inventory->GetInvenSlot(0, slot);
        if (!equipped.item_id_) {
            continue;
        }

        ++equipped_count;
        const CItem* const item = G_CDataManager()->find_item(equipped.item_id_);
        if (item && item->get_rarity() == 2) {
            ++rare_equipped_count;
        }
    }

    if (equipped_count > 0) {
        auto_client_rate_ -= 40;
        auto_client_rate_ -= 10 * equipped_count;
    }
    mcap_manager_.decIncRate(100 * rare_equipped_count);

    int avatar_count = 0;
    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++slot) {
        if (slot == oldsrv::config::kEquipmentTypeTitle) {
            continue;
        }
        if (inventory->GetInvenSlot(0, slot).item_id_) {
            ++avatar_count;
        }
    }

    if (avatar_count > 8) {
        auto_client_rate_ -= 20;
    }
    if (get_pvp_grade() > 9) {
        auto_client_rate_ -= 20;
    }
    if (auto_client_rate_ < 0) {
        auto_client_rate_ = 0;
    }
    return auto_client_rate_;
}
