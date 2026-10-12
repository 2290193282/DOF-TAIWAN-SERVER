#include "old_data/PVF/equipment/item_upgrade.h"

#include "common/config.h"
#include <algorithm>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "common/manager/global_effect_manager_abi.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/event/event_upgrade_jar_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/mt_rand.h"

namespace WongWork {

// IDA_EA: 0x854755A
// IDA_NAME: _ZN8WongWork12CItemUpgrade14_DoProcUpgradeEP5CUserP10Inven_ItemPK14upgrade_info_t
// DEMANGLED: WongWork::CItemUpgrade::_DoProcUpgrade(CUser *,Inven_Item *,upgrade_info_t const*)
int CItemUpgrade::_DoProcUpgrade(CUser* user,
                                 Inven_Item* target,
                                 const upgrade_info_t* info) {
    unsigned int probability =
        static_cast<unsigned int>(info->parsed_values_08[0]);
    GlobalData::s_pEvent_UpGrade_Jar->GetProbability(
        info->event_material_item_index, probability);

    if (probability != 0u &&
        static_cast<uint8_t>(
            GlobalData::s_globalEffectManager_->isAffectedEffect(3u))) {
        const float effect_value =
            GlobalData::s_globalEffectManager_->getEffectValue(3u);
        const int reduction =
            static_cast<int>(
                static_cast<long double>(random_limit_) * effect_value) -
            static_cast<int>(random_limit_);
        const unsigned int adjusted = probability - reduction;
        probability = std::max(0u, adjusted);
    }

    if (probability != 0u &&
        static_cast<uint8_t>(
            expert_job::CAlchemist::IsExistContinuousEffectItem(
                user, 2600025))) {
        const float random_limit = static_cast<float>(random_limit_);
        const int reduction = static_cast<int>(
            expert_job::CAlchemist::GetItemUpgradeSuccessRate() *
            random_limit);
        const unsigned int adjusted = probability - reduction;
        probability = std::max(0u, adjusted);
    }

    const Inven_Item equipped =
        user->getCurCharacInvenR()->GetInvenSlot(
            0, oldsrv::config::kEquipmentTypeTitle);
    CItem* const equipped_item = G_CDataManager()->find_item(
        static_cast<int>(equipped.item_id_));
    if (equipped_item && probability != 0u) {
        probability -= static_cast<CEquipItem*>(equipped_item)
                           ->getUpgradeProbIncrease();
        if (random_limit_ < probability) {
            probability = 0u;
        }
    }

    if (probability != 0u) {
        const unsigned long limit = random_limit_;
        if (random_->randInt(limit) < probability) {
            return 0;
        }
    }

    target->IncUpgrade();
    return 1;
}

}  // namespace WongWork
