#include "old_game/gm/core/game_master_mgr_runtime.h"

#include "old_game/gm/dungeon/game_master_pvp_dungeon_commands.h"
#include "old_game/gm/dungeon/game_master_pcroom_reward_reset.h"
#include "old_game/gm/core/game_master_event_commands.h"
#include "old_game/gm/player/game_master_delete_account_cargo.h"
#include "old_game/gm/player/game_master_character_select_commands.h"
#include "old_game/gm/core/game_master_npc_shop_commands.h"
#include "old_game/gm/core/game_master_billing_recharge.h"
#include "old_game/gm/core/game_master_hero_mission.h"
#include "old_game/gm/progression/game_master_level_up.h"
#include "old_game/gm/item/game_master_get_cash_inven_command.h"
#include "old_game/gm/item/game_master_inventory_clear_commands.h"
#include "old_game/gm/core/game_master_mobile_restrict.h"
#include "old_game/gm/core/game_master_premium_delete_command.h"
#include "old_game/gm/core/game_master_recipe_success.h"
#include "old_game/gm/core/game_master_set_multibox_fail_count.h"
#include "old_game/gm/world/game_master_village_attacked_commands.h"

namespace game_master {

CGameMasterMgr::CGameMasterMgr()
    : senior_(new CSeniorGameMaster(0)),
      junior_(new CJuniorGameMaster(senior_)),
      command_map_() {
    // Tail of the old C2 @0x82a8a28.  Every key and concrete object below is
    // present in the original table and has a completed owner-side command
    // implementation.  Do not add placeholders for the remaining keys.
    // Numeric registrations below are verified against the C2 stores at
    // 0x82A9078/0x82A90B9/0x82A90FA/0x82A91F0/0x82A923F and the later C2
    // entries for 82 and 118.  The event keys 46/47/102/125/127/128/130
    // correspond to the later C2 stores and are registered only after their
    // complete owner-side transactions were restored.
    command_map_.add(1, new CMoveCharacCmd());
    command_map_.add(2, new CPvPStartCmd());
    command_map_.add(9, new CCharacStatCmd());
    command_map_.add(30, new CVillageAttackedStartCmd());
    command_map_.add(32, new CVillageAttackedFightCmd());
    command_map_.add(33, new CVillageAttackedStopCmd());
    command_map_.add(35, new CChangeNPCMaxDayCmd());
    command_map_.add(36, new CChangeNPCFavorCmd());
    command_map_.add(37, new CClearInventoryCmd());
    command_map_.add(38, new CClearAvatarInvenCmd());
    command_map_.add(45, new CChangeCharacName());
    command_map_.add(46, new CStartEvent());
    command_map_.add(47, new CEndEvent());
    command_map_.add(48, new CSecretShopCheckLottery());
    command_map_.add(49, new CSecretShopNpc());
    command_map_.add(52, new CItemDropInDungeon());
    command_map_.add(54, new CDeleteAccountCargo());
    command_map_.add(60, new CUserPremiumDelete());
    command_map_.add(63, new CAdjustCharacStat());
    command_map_.add(70, new CGetCashInven());
    command_map_.add(82, new COneADayItemShop());
    command_map_.add(102, new CInGameEventSetDay());
    command_map_.add(118, new NpcLimitBuyItem());
    command_map_.add(120, new DungeonDifficulty());
    command_map_.add(125, new BlueMarbleSet());
    command_map_.add(127, new CGrowhWeaponEvent());
    command_map_.add(128, new CClearRewardTest());
    command_map_.add(130, new CBingoGmTest());
    command_map_.add("bongfail", new CSetMultiboxFailCnt());
    command_map_.add("lv", new CLevelUp());
    command_map_.add("recipeprob", new CRecipeSuccess());
    command_map_.add("cash", new CBillingRechargeCommand());
    command_map_.add("restrict", new CMobileRestrictCommand());
    command_map_.add("pcroomrewardreset", new CPcroomRewardReset());
    command_map_.add("heromission", new CHeroMissionCommand());
}

CGameMasterMgr::~CGameMasterMgr() {
    delete junior_;
    delete senior_;
}

int CGameMasterMgr::HandleRequest(CCommand* command) {
    return junior_ ? junior_->HandleRequest(command) : 0;
}

CCommand* CGameMasterMgr::GetCommand(int command_id) const {
    // CCommandMap's original lookup ABI is non-const.  It performs no
    // mutation, and the manager's public convenience wrapper remains const.
    return const_cast<CCommandMap&>(command_map_).get(command_id);
}

CCommand* CGameMasterMgr::GetCommand(const std::string& command_name) const {
    // The original provider accepts a mutable std::string lvalue.  Keep the
    // manager's caller-facing const-reference API without changing the ABI
    // symbol exported by CCommandMap itself.
    std::string lookup_key(command_name);
    return const_cast<CCommandMap&>(command_map_).get(lookup_key);
}

}  // namespace game_master
