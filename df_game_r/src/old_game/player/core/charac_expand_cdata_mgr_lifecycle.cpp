#include "old_game/player/core/charac_expand_data_mgr.h"

#include <cstring>
#include <new>

#include "old_game/dungeon/progress/best_clear_time_runtime.h"
#include "old_game/dungeon/instance/character_blood_dungeon_runtime.h"
#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"
#include "old_game/dungeon/ai/kill_monster_info_runtime.h"
#include "old_game/pvp/battlefield/pvp_live_event_data_runtime.h"
#include "old_game/dungeon/instance/tod_user_state_runtime.h"
#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/economy/inventory/item_dictionary_runtime.h"
#include "old_game/economy/inventory/item_lock_runtime.h"
#include "old_game/economy/inventory/redeem_item_runtime.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/progression/career/character_expert_job.h"
#include "old_game/progression/quest/character_mission_list.h"
#include "old_game/progression/skill/item_making_skill.h"
#include "old_game/player/core/achievement_runtime.h"
#include "old_game/player/core/cbooster_gage_runtime.h"
#include "old_game/player/core/character_inform_notice.h"
#include "old_game/player/session/online_preliminary_match_list_runtime.h"
#include "old_game/player/session/online_preliminary_member_runtime.h"
#include "old_game/player/core/title_book_runtime.h"

namespace charac_expand {

// IDA_EA: 0x832A778
// IDA_NAME: _ZN13charac_expand8CDataMgrC2Ev
// DEMANGLED: charac_expand::CDataMgr::CDataMgr(void)
CDataMgr::CDataMgr() {
    void* expert_job_storage =
        ::operator new(sizeof(expert_job::CCharacExpertJob));
    std::memset(expert_job_storage,
                0,
                sizeof(expert_job::CCharacExpertJob));
    slots_[0] =
        new (expert_job_storage) expert_job::CCharacExpertJob();

    void* blood_dungeon_storage =
        ::operator new(sizeof(CCharacBloodDungeon));
    std::memset(blood_dungeon_storage,
                0,
                sizeof(CCharacBloodDungeon));
    slots_[1] =
        new (blood_dungeon_storage) CCharacBloodDungeon();

    slots_[2] = new item_lock::CItemLock();
    slots_[3] = new CPvPLiveEventData();
    slots_[4] = new online_preliminary::COnlinePreliminaryMember();
    slots_[5] =
        new online_preliminary::COnlinePreliminaryMemberMatchList();
    slots_[6] = new Redeem_Item::CRedeemItem();
    slots_[7] = new fair_pvp::CFairPvPScore();

    void* mission_storage = ::operator new(sizeof(CMissionList_Charac));
    std::memset(mission_storage, 0, sizeof(CMissionList_Charac));
    try {
        slots_[8] = new (mission_storage) CMissionList_Charac();
    } catch (...) {
        ::operator delete(mission_storage);
        throw;
    }

    slots_[9] = new CExpandEquipslot();
    slots_[10] = new CItemMakingSkill();
    slots_[11] = new CQuestShop();

    // The old constructor creates slot 13 before slot 12.
    slots_[13] = new TOD_UserState();
    slots_[12] = new CKillMonsterInfo();

    slots_[14] = new CTitleBook();
    slots_[15] = new CAchievement();
    slots_[16] = new CItemDictionary();
    slots_[17] = new CBoosterGage();
    slots_[18] = new BestClearTime();
    slots_[19] = new CCharacInformNotice();
}

// IDA_EA: 0x832AC48
// IDA_NAME: _ZN13charac_expand8CDataMgrD2Ev
// DEMANGLED: charac_expand::CDataMgr::~CDataMgr()
CDataMgr::~CDataMgr() {
    for (int index = 0; index <= 19; ++index) {
        if (slots_[index]) {
            delete slots_[index];
        }
    }
}

}  // namespace charac_expand
