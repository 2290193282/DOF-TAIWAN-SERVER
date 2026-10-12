#include "old_data/PVF/equipment/item_upgrade.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/equipment_upgrade/db_req_item_upgrade_log.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/mt_rand.h"

namespace WongWork {

// IDA_EA: 0x854AC8A
// IDA_NAME: _ZN8WongWork12CItemUpgrade24ProcAmplifyRandomUpgradeEP5CUserR10Inven_ItemS4_iPKc
// DEMANGLED: WongWork::CItemUpgrade::ProcAmplifyRandomUpgrade(CUser *, Inven_Item &, Inven_Item &, int, char const*)
int CItemUpgrade::ProcAmplifyRandomUpgrade(CUser* user,
                                           Inven_Item& target,
                                           Inven_Item& material,
                                           int upgrade,
                                           const char* guild_item_name) {
    (void)material;

    const int old_upgrade = target.GetUpgrade();
    const int result = prepareAmplifyRandomUpgrade(target, upgrade);
    if (result != 0) {
        return result;
    }
    commitAmplifyRandomUpgrade(
        user, target, old_upgrade, guild_item_name);
    return 0;
}

int CItemUpgrade::prepareAmplifyRandomUpgrade(Inven_Item& target,
                                              int upgrade) {
    CDataManager* const data_manager = G_CDataManager();
    CItemAmplifier* const amplifier = GlobalData::s_itemAmplifier_;
    if (!data_manager || !amplifier || !amplifier->isScriptReady() ||
        upgrade < 0 || static_cast<unsigned int>(upgrade) >= 32u) {
        return 4;
    }

    CItem* const item = data_manager->find_item(target.item_id_);
    if (!item || !target.amplify_option_.hasAbility()) {
        return 4;
    }

    const upgrade_table_t* const table = amplifier->getUpgradeInfoTable();
    if (!table || table->upgrade_infos.empty()) {
        return 4;
    }
    target.SetUpgrade(static_cast<uint8_t>(upgrade));
    return 0;
}

void CItemUpgrade::commitAmplifyRandomUpgrade(
    CUser* user,
    const Inven_Item& target,
    int old_upgrade,
    const char* guild_item_name) {
    CDataManager* const data_manager = G_CDataManager();
    CItemAmplifier* const amplifier = GlobalData::s_itemAmplifier_;
    if (!user || !data_manager || !amplifier ||
        !amplifier->isScriptReady()) {
        return;
    }

    CItem* const item = data_manager->find_item(target.item_id_);
    const upgrade_table_t* const table = amplifier->getUpgradeInfoTable();
    if (!item || !table || table->upgrade_infos.empty()) {
        return;
    }

    CValueStatistic* const statistic = GetInstanceValueStatistic();
    if (statistic) {
        statistic->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(14), user, 0);
    }
    if (target.GetUpgrade() >= table->notice_upgrade_level) {
        _NoticeUpgrade(user, target, true);
    }
    if (target.GetUpgrade() > 12 && G_CEnvironment()) {
        DB_ReqItemUpgradeLog::makeRequest(
            static_cast<unsigned int>(G_CEnvironment()->get_server_group()),
            static_cast<unsigned int>(target.item_id_),
            static_cast<unsigned int>(user->getCurCharacNo()),
            user->getCurCharacName(),
            static_cast<unsigned int>(target.GetUpgrade()),
            true);
    }

    user->history_log().UpgradeItem(
        item->get_index(), old_upgrade, target.GetUpgrade(), true);
    const char* const name = item->vtable_->is_random_option(item)
                                 ? (guild_item_name ? guild_item_name : "")
                                 : item->GetItemName();
    user->makeGuildItemUpgradMessage(name, old_upgrade, target.GetUpgrade(), true);
}

// IDA_EA: 0x854AF38
// IDA_NAME: _ZN8WongWork12CItemUpgrade26enableAmplifyRandomUpgradeEP5CUserR10Inven_ItemS4_Ri
// DEMANGLED: WongWork::CItemUpgrade::enableAmplifyRandomUpgrade(CUser *, Inven_Item &, Inven_Item &, int &)
int CItemUpgrade::enableAmplifyRandomUpgrade(CUser* user,
                                             Inven_Item& target,
                                             Inven_Item& material,
                                             int& upgrade) {
    CDataManager* const data_manager = G_CDataManager();
    CItemAmplifier* const amplifier = GlobalData::s_itemAmplifier_;
    if (!user || !data_manager || !amplifier ||
        !amplifier->isScriptReady() || !random_) {
        return 4;
    }

    CItem* const target_item = data_manager->find_item(target.item_id_);
    if (!target_item) {
        return 4;
    }

    const upgrade_table_t* const table = amplifier->getUpgradeInfoTable();
    if (!table || table->upgrade_infos.empty()) {
        return 13;
    }
    upgrade_info_t info;
    if (_GetUpgradeInfo(&target,
                        target_item,
                        *table,
                        &info,
                        user->get_charac_level()) != 1) {
        return 13;
    }

    // A random amplification ticket replaces the ordinary amplification
    // material and cost for this operation.  The 0627 dispatcher writes these
    // three upgrade_info_t fields before calling _CheckCondition; without the
    // override every valid golden book is rejected as the wrong material (22).
    info.event_material_item_index = static_cast<int>(material.item_id_);
    info.event_material_item_count = 1;
    info.calculated_cost = 0;

    if (target_item->vtable_->is_impossible_amplify(target_item)) {
        return 19;
    }

    const int condition = _CheckCondition(
        user, target, target_item, material, *table, info);
    if (condition != 0) {
        return condition;
    }

    CItem* const material_item = data_manager->find_item(material.item_id_);
    if (!material_item || material_item->is_stackable() != 1) {
        return 4;
    }
    CStackableItem* const ticket = static_cast<CStackableItem*>(material_item);
    if (ticket->is_random_amplify_reinforce_ticket() != 1 ||
        !ticket->has_valid_random_amplify_reinforce_table()) {
        return 4;
    }

    const unsigned long roll_limit = 100;
    upgrade = ticket->get_amplify_reinforce_count(
        static_cast<int>(random_->randInt(roll_limit)));
    return static_cast<unsigned int>(upgrade) < 32u ? 0 : 4;
}

}  // namespace WongWork
