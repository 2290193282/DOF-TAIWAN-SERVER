#include "old_data/PVF/data/data_manager.h"

#include "old_data/PVF/common/arad_script_data.h"
#include "old_data/PVF/event/level_reward_scripts.h"

ARAD::SCRIPT::AradJumping_Script* CDataManager::get_arad_jumping_script() {
    return &arad_jumping_script_;
}

// This is deliberately a typed, non-IDA bridge.  The direct code-45 leaf
// call at 0x81993D4 reads CDataManager+0x18; the CDataManager C2 has already
// established quest_list_ as the named pointer at that exact i386 offset.
QuestList* CDataManager::get_arad_jumping_quest_list() {
    return quest_list_;
}

// This is deliberately a typed, non-IDA bridge.  The direct code-45 leaf
// call at 0x819994A reads CDataManager+0x0C; the CDataManager C2 has already
// established item_list_ as the named pointer at that exact i386 offset.
CItemList* CDataManager::get_arad_jumping_item_list() {
    return item_list_;
}

// This is deliberately a typed, non-IDA bridge.  Several old gameplay
// routines, including CCeraShop::_CheckBuyingCondition @0x8323F86, call
// CItemList::create_item through the CDataManager+0x0C item-list member.
// CDataManager::C2 establishes that same named member, so consumers should
// cross this B boundary rather than reintroduce a raw offset projection.
CItemList* CDataManager::get_item_list() {
    return item_list_;
}

stLevelUpRewardItemScript*
CDataManager::get_level_up_reward_item_script() {
    return &level_up_reward_item_script_;
}

stReturnUserRewardScript*
CDataManager::get_return_user_reward_script() {
    return &return_user_reward_script_;
}

stLevelUpRewardItemScript*
CDataManager::get_new_account_level_up_equip_reward_script() {
    return &new_account_level_up_to_job_script_.equip_rewards;
}
