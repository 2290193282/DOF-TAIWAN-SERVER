#include "old_game/player/setting/character_option.h"
#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include "old_game/player/core/charac_visible_flag.h"
#include "old_game/world/region/game_world.h"
#include "old_game/player/core/character_db_payloads.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_game/player/core/character_view_data.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_data/PVF/equipment/equipment_script.h"
#include "old_data/PVF/town/town_script.h"
#include "old_data/db/account/login_payloads.h"
#include "old_data/db/character/create_charac_payload.h"
#include "old_data/db/character/load_charac_payload.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/progression/quest/quest_skill_value_types.h"
#include "core/config/env/environment.h"
#include "common/manager/common_time_game_option_abi.h"
#include "common/manager/data_manager.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_tools.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/history_log_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/play_info_public_abi.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "common/manager/reliable_person.h"
#include "common/manager/user.h"
#include "common/manager/user_history_log_abi.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "common/manager/system_time.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <map>
#include <new>
#include <stdint.h>
#include <vector>

// IDA_EA: 0x8695C48
// IDA_NAME: _ZN18ContinuousItemInfoC2Ev
// DEMANGLED: ContinuousItemInfo::ContinuousItemInfo(void)
ContinuousItemInfo::ContinuousItemInfo()
    : item_index(0), expire_tick(0) {}

// IDA_EA: 0x86A04B2
// IDA_NAME: _ZN18ContinuousItemInfoD2Ev
// DEMANGLED: ContinuousItemInfo::~ContinuousItemInfo()
ContinuousItemInfo::~ContinuousItemInfo() {}

// IDA_EA: 0x8695C78
// IDA_NAME: _ZN18WpBonusPointResultC2Ev
// DEMANGLED: WpBonusPointResult::WpBonusPointResult(void)
WpBonusPointResult::WpBonusPointResult() {
    clear();
}

// IDA_EA: 0x8695C8C
// IDA_NAME: _ZN18WpBonusPointResult5clearEv
// DEMANGLED: WpBonusPointResult::clear(void)
WpBonusPointResult* WpBonusPointResult::clear() {
    normal = 0;
    premium = 0;
    return this;
}

// IDA_EA: 0x8695CA4
// IDA_NAME: _ZN19ExpBonusPointResultC2Ev
// DEMANGLED: ExpBonusPointResult::ExpBonusPointResult(void)
ExpBonusPointResult::ExpBonusPointResult() {
    clear();
}

// IDA_EA: 0x8695CB8
// IDA_NAME: _ZN19ExpBonusPointResult5clearEv
// DEMANGLED: ExpBonusPointResult::clear(void)
ExpBonusPointResult* ExpBonusPointResult::clear() {
    normal = 0;
    premium = 0;
    return this;
}

// IDA_EA: 0x80FDF20
// IDA_NAME: _ZNK15CUserCharacInfo14get_charac_jobEv
// DEMANGLED: CUserCharacInfo::get_charac_job(void)const
int CUserCharacInfo::get_charac_job() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.job_;
    }
    return 0;
}

// IDA_EA: 0x817A188
// IDA_NAME: _ZNK15CUserCharacInfo17getCurCharacMoneyEv
// DEMANGLED: CUserCharacInfo::getCurCharacMoney(void)const
int CUserCharacInfo::getCurCharacMoney() const {
    const _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }
    return getCurCharacInvenR()->get_money();
}

// IDA_EA: 0x822F27A
// IDA_NAME: _ZN15CUserCharacInfo15enableSaveSkillEv
// DEMANGLED: CUserCharacInfo::enableSaveSkill(void)
CUserCharacInfo* CUserCharacInfo::enableSaveSkill() {
    save_skill_ = 1;
    return this;
}

// IDA_EA: 0x822F262
// IDA_NAME: _ZN15CUserCharacInfo20enableSaveCharacInfoEv
// DEMANGLED: CUserCharacInfo::enableSaveCharacInfo(void)
CUserCharacInfo* CUserCharacInfo::enableSaveCharacInfo() {
    save_charac_info_ = 1;
    return this;
}

// IDA_EA: 0x822F26E
// IDA_NAME: _ZN15CUserCharacInfo26enableSaveCharacManageInfoEv
// DEMANGLED: CUserCharacInfo::enableSaveCharacManageInfo(void)
CUserCharacInfo* CUserCharacInfo::enableSaveCharacManageInfo() {
    save_charac_manage_info_ = 1;
    return this;
}

// IDA_EA: 0x822F286
// IDA_NAME: _ZN15CUserCharacInfo13enableSavePvPEv
// DEMANGLED: CUserCharacInfo::enableSavePvP(void)
CUserCharacInfo* CUserCharacInfo::enableSavePvP() {
    save_pvp_ = 1;
    return this;
}

// IDA_EA: 0x819A870
// IDA_NAME: _ZN15CUserCharacInfo20enableSaveCharacStatEv
// DEMANGLED: CUserCharacInfo::enableSaveCharacStat(void)
CUserCharacInfo* CUserCharacInfo::enableSaveCharacStat() {
    save_charac_stat_ = 1;
    return this;
}

// IDA_EA: 0x822F140
// IDA_NAME: _ZN15CUserCharacInfo18getCurCharacSkillWEv
// DEMANGLED: CUserCharacInfo::getCurCharacSkillW(void)
SkillSlot* CUserCharacInfo::getCurCharacSkillW() {
    enableSaveSkill();
    return current_charac_ ? &current_charac_->fields_.skill_slot_ : 0;
}

// IDA_EA: 0x822F202
// IDA_NAME: _ZNK15CUserCharacInfo22getCurCharacCreateTimeEv
// DEMANGLED: CUserCharacInfo::getCurCharacCreateTime(void)const
int CUserCharacInfo::getCurCharacCreateTime() const {
    const _Charac_info* const cur = current_charac_;
    const CommonTime& create_time = cur->fields_.create_time_;

    std::tm tm_value;
    std::memset(&tm_value, 0, sizeof(tm_value));
    tm_value.tm_sec = 0;
    tm_value.tm_min = static_cast<signed char>(create_time.minute);
    tm_value.tm_hour = static_cast<signed char>(create_time.hour);
    tm_value.tm_mday = static_cast<signed char>(create_time.day);
    tm_value.tm_mon = static_cast<signed char>(create_time.month) - 1;
    tm_value.tm_year = static_cast<signed char>(create_time.year) + 100;
    tm_value.tm_isdst = 0;
    return static_cast<int>(std::mktime(&tm_value));
}

// IDA_EA: 0x822F292
// IDA_NAME: _ZN15CUserCharacInfo19setSaveCharacOptionEb
// DEMANGLED: CUserCharacInfo::setSaveCharacOption(bool)
CUserCharacInfo* CUserCharacInfo::setSaveCharacOption(bool save) {
    save_charac_option_ = save ? 1 : 0;
    return this;
}

// IDA_EA: 0x8645564
// IDA_NAME: _ZNK15CUserCharacInfo16getCurCharacVillEv
// DEMANGLED: CUserCharacInfo::getCurCharacVill(void)const
int CUserCharacInfo::getCurCharacVill() const {
    const _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }

    GameWorld* world = G_GameWorld();
    if (world->IsPvPVillageMapChannel()) {
        return 10;
    }
    // The 0828 type-19 channel owns the dedicated Zombie town.  Resolve its
    // index from the loaded Town.lst instead of baking the current PVF index
    // (16) into the server, and keep the character's persisted village intact
    // so leaving the special channel returns to the original town.
    if (world->GetChannelType() == 19) {
        const int zombie_town =
            g_townScriptFileList.FindIndexByFullScan("ZombiMode.twn");
        if (zombie_town > 0 && world->GetVillage(zombie_town) != 0) {
            return zombie_town;
        }
    }
    world = G_GameWorld();
    if (world->IsPVPChannel()) {
        return cur->fields_.pvp_village_;
    }
    return cur->fields_.village_;
}

// IDA_EA: 0x86455BC
// IDA_NAME: _ZN15CUserCharacInfo16setCurCharacVillEc
// DEMANGLED: CUserCharacInfo::setCurCharacVill(char)
int CUserCharacInfo::setCurCharacVill(char village) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        // Town 16 is a channel-local projection for type 19.  Moving between
        // its two areas must not overwrite the character's persistent village
        // or previous-village fields; leaving the Zombie channel must return
        // to the town stored by the normal channel.
        if (G_GameWorld()->GetChannelType() == 19) {
            return result;
        }
        cur->fields_.previous_village_ = cur->fields_.village_;
        GameWorld* world = G_GameWorld();
        if (world->IsPVPChannel()) {
            cur->fields_.pvp_village_ = 2;
        } else {
            enableSaveCharacStat();
            result = reinterpret_cast<int>(cur);
            cur->fields_.village_ =
                static_cast<unsigned char>(village);
        }
    }
    return result;
}

// IDA_EA: 0x8645620
// IDA_NAME: _ZN15CUserCharacInfo14UseFinishPointEi
// DEMANGLED: CUserCharacInfo::UseFinishPoint(int)
int CUserCharacInfo::UseFinishPoint(int point) {
    if (!current_charac_) {
        cMyTrace trace("unsigned int CUserCharacInfo::UseFinishPoint(int)",
                       544,
                       5);
        trace("WARAREA CUserCharacInfo::UseFinishPoint() : m_selected is NULL");
        return 0;
    }

    ChangeFinishPoint(-point);
    static_cast<CUser*>(this)->history_log().FPSub(
        point,
        current_charac_->fields_.finish_point_,
        current_charac_->fields_.finish_point_total_,
        static_cast<eFPSubReason>(0));
    return current_charac_->fields_.finish_point_;
}

// IDA_EA: 0x8645764
// IDA_NAME: _ZN15CUserCharacInfo14GetFinishPointEv
// DEMANGLED: CUserCharacInfo::GetFinishPoint(void)
int CUserCharacInfo::GetFinishPoint() {
    const _Charac_info* const charac = getCurCharacR();
    if (charac) {
        return charac->fields_.finish_point_;
    }

    cMyTrace trace("int CUserCharacInfo::GetFinishPoint()", 580, 5);
    trace("WARAREA CUserCharacInfo::GetFinishPoint() : m_selected is NULL");
    return 0;
}

// IDA_EA: 0x86457C0
// IDA_NAME: _ZN15CUserCharacInfo17ChangeFinishPointEi
// DEMANGLED: CUserCharacInfo::ChangeFinishPoint(int)
int CUserCharacInfo::ChangeFinishPoint(int point) {
    _Charac_info* const charac = current_charac_;
    if (!charac) {
        cMyTrace trace("void CUserCharacInfo::ChangeFinishPoint(int)",
                       589,
                       5);
        return trace(
            "WARAREA CUserCharacInfo::ChangeFinishPoint() : m_selected is NULL");
    }

    if (point > 0) {
        charac->fields_.finish_point_total_ += point;
    }
    charac->fields_.finish_point_ += point;
    if (charac->fields_.finish_point_ < 0) {
        charac->fields_.finish_point_ = 0;
        return static_cast<int>(reinterpret_cast<uintptr_t>(charac));
    }
    return charac->fields_.finish_point_;
}

// IDA_EA: 0x86459C4
// IDA_NAME: _ZN15CUserCharacInfo11WarAreaKillEiif
// DEMANGLED: CUserCharacInfo::WarAreaKill(int,int,float)
int CUserCharacInfo::WarAreaKill(int monster_kind,
                                 int point,
                                 float multiplier) {
    _Charac_info* const charac = current_charac_;
    if (!charac) {
        cMyTrace trace("void CUserCharacInfo::WarAreaKill(int, int, float)",
                       644,
                       5);
        return trace(
            "WARAREA CUserCharacInfo::WarAreaKill() : m_selected is NULL");
    }

    ChangeFinishPoint(point);
    charac->fields_.finish_point_detail_[monster_kind] += multiplier;
    return static_cast<CUser*>(this)->history_log().FPAdd(
        point,
        charac->fields_.finish_point_,
        charac->fields_.finish_point_total_,
        static_cast<eFPAddReason>(monster_kind == 4));
}

// IDA_EA: 0x86458CE
// IDA_NAME: _ZN15CUserCharacInfo19GetFinishPointTotalEv
// DEMANGLED: CUserCharacInfo::GetFinishPointTotal(void)
int CUserCharacInfo::GetFinishPointTotal() {
    const _Charac_info* const charac = getCurCharacR();
    if (charac) {
        return charac->fields_.finish_point_total_;
    }

    cMyTrace trace("int CUserCharacInfo::GetFinishPointTotal()", 618, 5);
    trace("WARAREA CUserCharacInfo::GetFinishPointTotal() : m_selected is NULL");
    return 0;
}

// IDA_EA: 0x864592A
// IDA_NAME: _ZN15CUserCharacInfo15InitFinishPointEv
// DEMANGLED: CUserCharacInfo::InitFinishPoint(void)
int CUserCharacInfo::InitFinishPoint() {
    _Charac_info* const charac = current_charac_;
    if (!charac) {
        return 0;
    }

    charac->fields_.finish_point_ = 0;
    charac->fields_.finish_point_total_ = 0;
    std::memset(charac->fields_.finish_point_detail_,
                0,
                sizeof(charac->fields_.finish_point_detail_));
    return static_cast<CUser*>(this)->history_log().FPSub(
        0,
        charac->fields_.finish_point_,
        charac->fields_.finish_point_total_,
        static_cast<eFPSubReason>(1));
}

// IDA_EA: 0x8645AA0
// IDA_NAME: _ZN15CUserCharacInfo16DecreaseWarPointEi
// DEMANGLED: CUserCharacInfo::DecreaseWarPoint(int)
int CUserCharacInfo::DecreaseWarPoint(int point) {
    _Charac_info* const charac = current_charac_;
    if (!charac) {
        cMyTrace trace("void CUserCharacInfo::DecreaseWarPoint(int)",
                       663,
                       5);
        return trace(
            "WARAREA CUserCharacInfo::DecreaseWarPoint() : m_selected is NULL");
    }

    if (point > 0) {
        charac->fields_.finish_point_total_ -= point;
        if (charac->fields_.finish_point_total_ < 0) {
            charac->fields_.finish_point_total_ = 0;
        }
    } else {
        charac->fields_.finish_point_total_ = 0;
    }

    return static_cast<CUser*>(this)->history_log().FPSub(
        point,
        charac->fields_.finish_point_,
        charac->fields_.finish_point_total_,
        static_cast<eFPSubReason>(3));
}

// IDA_EA: 0x868EFAE
// IDA_NAME: _ZN15CUserCharacInfo18SetCurCharacOptionER15CharacterOption
// DEMANGLED: CUserCharacInfo::SetCurCharacOption(CharacterOption &)
void* CUserCharacInfo::SetCurCharacOption(CharacterOption& option) {
    _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }
    return std::memcpy(&cur->fields_.character_option_, &option,
                       sizeof(CharacterOption));
}

// IDA_EA: 0x80DA27E
// IDA_NAME: _ZNK15CUserCharacInfo18getCurCharacInvenREv
// DEMANGLED: CUserCharacInfo::getCurCharacInvenR(void)const
CInventory* CUserCharacInfo::getCurCharacInvenR() const {
    _Charac_info* const cur = current_charac_;
    return cur ? &cur->fields_.inventory_ : 0;
}

// IDA_EA: 0x80DA2AC
// IDA_NAME: _ZN15CUserCharacInfo15enableSaveInvenEv
// DEMANGLED: CUserCharacInfo::enableSaveInven(void)
CUserCharacInfo* CUserCharacInfo::enableSaveInven() {
    save_inventory_ = 1;
    return this;
}

// IDA_EA: 0x80DA28E
// IDA_NAME: _ZN15CUserCharacInfo18getCurCharacInvenWEv
// DEMANGLED: CUserCharacInfo::getCurCharacInvenW(void)
CInventory* CUserCharacInfo::getCurCharacInvenW() {
    enableSaveInven();
    _Charac_info* const cur = current_charac_;
    return cur ? &cur->fields_.inventory_ : 0;
}

// IDA_EA: 0x80DA2DA
// IDA_NAME: _ZN15CUserCharacInfo16IsCurCharacGhostEv
// DEMANGLED: CUserCharacInfo::IsCurCharacGhost(void)
int CUserCharacInfo::IsCurCharacGhost() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.ghost_state_;
    }
    return 0;
}

// IDA_EA: 0x8110C94
// IDA_NAME: _ZNK15CUserCharacInfo23getCurCharFirstGrowTypeEv
// DEMANGLED: CUserCharacInfo::getCurCharFirstGrowType(void)const
int CUserCharacInfo::getCurCharFirstGrowType() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return static_cast<signed char>(cur->fields_.grow_type_.growth_type << 4) >> 4;
    }
    return 0;
}

// IDA_EA: 0x8110CBA
// IDA_NAME: _ZN15CUserCharacInfo23getCurCharacUsedFatigueEv
// DEMANGLED: CUserCharacInfo::getCurCharacUsedFatigue(void)
int CUserCharacInfo::getCurCharacUsedFatigue() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.used_fatigue_;
    }
    return 0;
}

// IDA_EA: 0x8110CDA
// IDA_NAME: _ZN15CUserCharacInfo29IncCurCharacEventAddItemCountEt
// DEMANGLED: CUserCharacInfo::IncCurCharacEventAddItemCount(unsigned short)
int CUserCharacInfo::IncCurCharacEventAddItemCount(uint16_t count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.event_add_item_count_ +=
            count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8110D12
// IDA_NAME: _ZNK15CUserCharacInfo29getCurUseBringUpMaterialTotalEv
// DEMANGLED: CUserCharacInfo::getCurUseBringUpMaterialTotal(void)const
int CUserCharacInfo::getCurUseBringUpMaterialTotal() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.event_bring_up_material_total_;
    }
    return 0;
}

// IDA_EA: 0x8110D36
// IDA_NAME: _ZN15CUserCharacInfo31resetCurUseBringUpMaterialCountEv
// DEMANGLED: CUserCharacInfo::resetCurUseBringUpMaterialCount(void)
int CUserCharacInfo::resetCurUseBringUpMaterialCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.event_bring_up_material_use_count_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8110D52
// IDA_NAME: _ZNK15CUserCharacInfo29getCurUseBringUpMaterialCountEv
// DEMANGLED: CUserCharacInfo::getCurUseBringUpMaterialCount(void)const
int CUserCharacInfo::getCurUseBringUpMaterialCount() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.event_bring_up_material_use_count_;
    }
    return 0;
}

// IDA_EA: 0x8110D76
// IDA_NAME: _ZNK15CUserCharacInfo20getBringUpRewardFlagEv
// DEMANGLED: CUserCharacInfo::getBringUpRewardFlag(void)const
int CUserCharacInfo::getBringUpRewardFlag() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.event_bring_up_reward_flag_;
    }
    return 0;
}

// IDA_EA: 0x8110D9A
// IDA_NAME: _ZN15CUserCharacInfo20setBringUpRewardFlagEb
// DEMANGLED: CUserCharacInfo::setBringUpRewardFlag(bool)
int CUserCharacInfo::setBringUpRewardFlag(bool flag) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.event_bring_up_reward_flag_ = flag ? 1 : 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x811EDA6
// IDA_NAME: _ZN15CUserCharacInfo26getUpgradeSeparateTrueFlagEv
// DEMANGLED: CUserCharacInfo::getUpgradeSeparateTrueFlag(void)
int CUserCharacInfo::getUpgradeSeparateTrueFlag() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.upgrade_separate_true_flag_;
    }
    return 0;
}

// IDA_EA: 0x811EDCA
// IDA_NAME: _ZN15CUserCharacInfo27getUpgradeSeparateFlaseFlagEv
// DEMANGLED: CUserCharacInfo::getUpgradeSeparateFlaseFlag(void)
int CUserCharacInfo::getUpgradeSeparateFlaseFlag() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.upgrade_separate_false_flag_;
    }
    return 0;
}

// IDA_EA: 0x812CDBE
// IDA_NAME: _ZN15CUserCharacInfo13getCurCharacWEv
// DEMANGLED: CUserCharacInfo::getCurCharacW(void)
_Charac_info* CUserCharacInfo::getCurCharacW() {
    return current_charac_;
}

// IDA_EA: 0x84EBEA8
// IDA_NAME: _ZNK15CUserCharacInfo13getTagCharacREv
// DEMANGLED: CUserCharacInfo::getTagCharacR(void)const
const _Charac_info* CUserCharacInfo::getTagCharacR() const {
    return tag_charac_;
}

// IDA_EA: 0x822F110
// IDA_NAME: _ZNK15CUserCharacInfo18getTagCharacInvenREv
// DEMANGLED: CUserCharacInfo::getTagCharacInvenR(void)const
CInventory* CUserCharacInfo::getTagCharacInvenR() const {
    return tag_charac_ ? &tag_charac_->fields_.inventory_ : 0;
}

// IDA_EA: 0x822F120
// IDA_NAME: _ZNK15CUserCharacInfo18getTagCharacSkillREv
// DEMANGLED: CUserCharacInfo::getTagCharacSkillR(void)const
SkillSlot* CUserCharacInfo::getTagCharacSkillR() const {
    return tag_charac_ ? &tag_charac_->fields_.skill_slot_ : 0;
}

// IDA_EA: 0x84EBEB4
// IDA_NAME: _ZN15CUserCharacInfo18getTagCharacSkillWEv
// DEMANGLED: CUserCharacInfo::getTagCharacSkillW(void)
void* CUserCharacInfo::getTagCharacSkillW() {
    return tag_charac_ ? &tag_charac_->fields_.skill_slot_ : 0;
}

// IDA_EA: 0x822F130
// IDA_NAME: _ZNK15CUserCharacInfo18getCurCharacSkillREv
// DEMANGLED: CUserCharacInfo::getCurCharacSkillR(void)const
SkillSlot* CUserCharacInfo::getCurCharacSkillR() const {
    return current_charac_
               ? const_cast<SkillSlot*>(&current_charac_->fields_.skill_slot_)
               : 0;
}

// IDA_EA: 0x812CDCA
// IDA_NAME: _ZN15CUserCharacInfo19enableSaveCharacEtcEv
// DEMANGLED: CUserCharacInfo::enableSaveCharacEtc(void)
CUserCharacInfo* CUserCharacInfo::enableSaveCharacEtc() {
    save_charac_etc_ = 1;
    return this;
}

// IDA_EA: 0x812CDD6
// IDA_NAME: _ZNK15CUserCharacInfo22getCurCharacMaxFatigueEv
// DEMANGLED: CUserCharacInfo::getCurCharacMaxFatigue(void)const
int CUserCharacInfo::getCurCharacMaxFatigue() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.maximum_fatigue_;
    }
    return 0;
}

// IDA_EA: 0x812CDF6
// IDA_NAME: _ZN15CUserCharacInfo29setCurCharacMaxPremiumFatigueEt
// DEMANGLED: CUserCharacInfo::setCurCharacMaxPremiumFatigue(unsigned short)
int CUserCharacInfo::setCurCharacMaxPremiumFatigue(uint16_t fatigue) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacEtc();
        cur->fields_.maximum_premium_fatigue_ =
            fatigue;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x814AA52
// IDA_NAME: _ZN15CUserCharacInfo15enableSaveQuestEv
// DEMANGLED: CUserCharacInfo::enableSaveQuest(void)
CUserCharacInfo* CUserCharacInfo::enableSaveQuest() {
    save_quest_ = 1;
    return this;
}

// IDA_EA: 0x8151A84
// IDA_NAME: _ZNK15CUserCharacInfo18getCurCharacCargoREv
// DEMANGLED: CUserCharacInfo::getCurCharacCargoR(void)const
CCargo* CUserCharacInfo::getCurCharacCargoR() const {
    _Charac_info* const cur = current_charac_;
    return cur ? &cur->fields_.cargo_ : 0;
}

// IDA_EA: 0x8151AB2
// IDA_NAME: _ZN15CUserCharacInfo15enableSaveCargoEv
// DEMANGLED: CUserCharacInfo::enableSaveCargo(void)
CUserCharacInfo* CUserCharacInfo::enableSaveCargo() {
    save_cargo_ = 1;
    return this;
}

// IDA_EA: 0x8151A94
// IDA_NAME: _ZN15CUserCharacInfo18getCurCharacCargoWEv
// DEMANGLED: CUserCharacInfo::getCurCharacCargoW(void)
CCargo* CUserCharacInfo::getCurCharacCargoW() {
    enableSaveCargo();
    _Charac_info* const cur = current_charac_;
    return cur ? &cur->fields_.cargo_ : 0;
}

// IDA_EA: 0x8151ABE
// IDA_NAME: _ZNK15CUserCharacInfo15getCurCharacExpEv
// DEMANGLED: CUserCharacInfo::getCurCharacExp(void)const
int CUserCharacInfo::getCurCharacExp() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.experience_;
    }
    return 0;
}

// IDA_EA: 0x815741C
// IDA_NAME: _ZNK15CUserCharacInfo20getCurCharacGrowTypeEv
// DEMANGLED: CUserCharacInfo::getCurCharacGrowType(void)const
int CUserCharacInfo::getCurCharacGrowType() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.grow_type_.growth_type;
    }
    return 0;
}

// IDA_EA: 0x8162338
// IDA_NAME: _ZN15CUserCharacInfo16IsFirstTimeLoginEv
// DEMANGLED: CUserCharacInfo::IsFirstTimeLogin(void)
int CUserCharacInfo::IsFirstTimeLogin() {
    const _Charac_info* const cur = current_charac_;
    return cur->fields_.first_time_login_;
}

// IDA_EA: 0x84EBEFA
// IDA_NAME: _ZN15CUserCharacInfo17SetFirstTimeLoginEb
// DEMANGLED: CUserCharacInfo::SetFirstTimeLogin(bool)
int CUserCharacInfo::SetFirstTimeLogin(bool flag) {
    _Charac_info* const cur = current_charac_;
    cur->fields_.first_time_login_ = flag ? 1 : 0;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x817A17C
// IDA_NAME: _ZN15CUserCharacInfo11GetWinPointEv
// DEMANGLED: CUserCharacInfo::GetWinPoint(void)
int CUserCharacInfo::GetWinPoint() {
    return pvp_result_.values[22];
}

// IDA_EA: 0x84EC002
// IDA_NAME: _ZN15CUserCharacInfo26setCurCharacTradeGoldDailyEj
// DEMANGLED: CUserCharacInfo::setCurCharacTradeGoldDaily(unsigned int)
CReliablePerson* CUserCharacInfo::setCurCharacTradeGoldDaily(uint32_t gold) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    return cur ? cur->fields_.reliable_person_.SetTradeGoldDaily(gold) : 0;
}

// IDA_EA: 0x84EC02E
// IDA_NAME: _ZN15CUserCharacInfo17SetReliablePersonEb
// DEMANGLED: CUserCharacInfo::SetReliablePerson(bool)
CUserCharacInfo* CUserCharacInfo::SetReliablePerson(bool flag) {
    reliable_person_ = flag;
    return this;
}

// IDA_EA: 0x84EC04A
// IDA_NAME: _ZN15CUserCharacInfo21SetOverTradeGoldCountEj
// DEMANGLED: CUserCharacInfo::SetOverTradeGoldCount(unsigned int)
CUserCharacInfo* CUserCharacInfo::SetOverTradeGoldCount(uint32_t count) {
    over_trade_gold_count_ = count;
    return this;
}

// IDA_EA: 0x81803CA
// IDA_NAME: _ZNK15CUserCharacInfo21getCurCharacInvenRefREv
// DEMANGLED: CUserCharacInfo::getCurCharacInvenRefR(void)const
CInventory* CUserCharacInfo::getCurCharacInvenRefR() const {
    return current_charac_
               ? const_cast<CInventory*>(&current_charac_->fields_.inventory_)
               : 0;
}

// IDA_EA: 0x819340A
// IDA_NAME: _ZN15CUserCharacInfo21getCurCharacInvenRefWEv
// DEMANGLED: CUserCharacInfo::getCurCharacInvenRefW(void)
void* CUserCharacInfo::getCurCharacInvenRefW() {
    enableSaveInven();
    _Charac_info* const cur = current_charac_;
    return cur ? &cur->fields_.inventory_ : 0;
}

// IDA_EA: 0x819A87C
// IDA_NAME: _ZN15CUserCharacInfo15setCurCharacExpEi
// DEMANGLED: CUserCharacInfo::setCurCharacExp(int)
int CUserCharacInfo::setCurCharacExp(int exp) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.experience_ = exp;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x819EE4A
// IDA_NAME: _ZNK15CUserCharacInfo13get_pvp_gradeEv
// DEMANGLED: CUserCharacInfo::get_pvp_grade(void)const
int CUserCharacInfo::get_pvp_grade() const {
    return pvp_result_.values[5];
}

// IDA_EA: 0x822F0B0
// IDA_NAME: _ZN15CUserCharacInfo29setCurCharacStrikerSkillIndexEh
// DEMANGLED: CUserCharacInfo::setCurCharacStrikerSkillIndex(unsigned char)
int CUserCharacInfo::setCurCharacStrikerSkillIndex(uint8_t index) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        save_charac_manage_info_ = 1;
        cur->fields_.striker_skill_index_ = index;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F0E4
// IDA_NAME: _ZN15CUserCharacInfo23setCurCharacTagCharacNoEj
// DEMANGLED: CUserCharacInfo::setCurCharacTagCharacNo(unsigned int)
int CUserCharacInfo::setCurCharacTagCharacNo(uint32_t charac_no) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        save_charac_manage_info_ = 1;
        cur->fields_.tag_character_no_ =
            charac_no;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F2AE
// IDA_NAME: _ZNK15CUserCharacInfo19getCurCharacFatigueEv
// DEMANGLED: CUserCharacInfo::getCurCharacFatigue(void)const
int CUserCharacInfo::getCurCharacFatigue() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.fatigue_;
    }
    return 0;
}

// IDA_EA: 0x822F2CE
// IDA_NAME: _ZN15CUserCharacInfo19setCurCharacFatigueEi
// DEMANGLED: CUserCharacInfo::setCurCharacFatigue(int)
int CUserCharacInfo::setCurCharacFatigue(int fatigue) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.fatigue_ = fatigue;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F2F8
// IDA_NAME: _ZN15CUserCharacInfo25getUserEventCharacterFlagEv
// DEMANGLED: CUserCharacInfo::getUserEventCharacterFlag(void)
int CUserCharacInfo::getUserEventCharacterFlag() {
    return user_event_character_flag_;
}

// IDA_EA: 0x822F308
// IDA_NAME: _ZN15CUserCharacInfo26SetCurCharacSkillTreeIndexEc
// DEMANGLED: CUserCharacInfo::SetCurCharacSkillTreeIndex(char)
int CUserCharacInfo::SetCurCharacSkillTreeIndex(char index) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacInfo();
        cur->fields_.skill_tree_index_ =
            static_cast<unsigned char>(index);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F33C
// IDA_NAME: _ZNK15CUserCharacInfo26GetCurCharacSkillTreeIndexEv
// DEMANGLED: CUserCharacInfo::GetCurCharacSkillTreeIndex(void)const
int CUserCharacInfo::GetCurCharacSkillTreeIndex() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.skill_tree_index_;
    }
    return 0;
}

// IDA_EA: 0x822F362
// IDA_NAME: _ZN15CUserCharacInfo34setCurCharacHelpAbuseComputedRatioEj
// DEMANGLED: CUserCharacInfo::setCurCharacHelpAbuseComputedRatio(unsigned int)
int CUserCharacInfo::setCurCharacHelpAbuseComputedRatio(uint32_t ratio) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.help_abuse_computed_ratio_ =
            ratio;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F380
// IDA_NAME: _ZN15CUserCharacInfo17GetReliablePersonEv
// DEMANGLED: CUserCharacInfo::GetReliablePerson(void)
int CUserCharacInfo::GetReliablePerson() {
    return reliable_person_;
}

// IDA_EA: 0x822F390
// IDA_NAME: _ZNK15CUserCharacInfo19getCurCharacStaminaEv
// DEMANGLED: CUserCharacInfo::getCurCharacStamina(void)const
int CUserCharacInfo::getCurCharacStamina() const {
    const _Charac_info* const cur = current_charac_;
    return cur->fields_.stamina_;
}

// IDA_EA: 0x822F3A0
// IDA_NAME: _ZNK15CUserCharacInfo27get_charac_10_level_sectionEv
// DEMANGLED: CUserCharacInfo::get_charac_10_level_section(void)const
int CUserCharacInfo::get_charac_10_level_section() const {
    const _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 10;
    }
    const int16_t level =
        cur->fields_.level_;
    const int16_t remainder = level % 10;
    if (remainder) {
        return level - remainder + 10;
    }
    return level;
}

// IDA_EA: 0x822F44E
// IDA_NAME: _ZN15CUserCharacInfo18setCharacViewStateEi
// DEMANGLED: CUserCharacInfo::setCharacViewState(int)
int CUserCharacInfo::setCharacViewState(int view_state) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.character_view_state_ = view_state;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F46C
// IDA_NAME: _ZN15CUserCharacInfo19get_charac_guildkeyEv
// DEMANGLED: CUserCharacInfo::get_charac_guildkey(void)
int CUserCharacInfo::get_charac_guildkey() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.guild_id_;
    }
    return 0;
}

// IDA_EA: 0x822F48E
// IDA_NAME: _ZN15CUserCharacInfo20resetGuildCreateFlagEv
// DEMANGLED: CUserCharacInfo::resetGuildCreateFlag(void)
int CUserCharacInfo::resetGuildCreateFlag() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.guild_create_flag_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F4AC
// IDA_NAME: _ZN15CUserCharacInfo22resetTempGuildNameAddrEv
// DEMANGLED: CUserCharacInfo::resetTempGuildNameAddr(void)
void* CUserCharacInfo::resetTempGuildNameAddr() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        std::memset(cur->fields_.guild_create_name_, 0,
                    sizeof(cur->fields_.guild_create_name_));
        return std::memset(cur->fields_.guild_create_address_, 0,
                           sizeof(cur->fields_.guild_create_address_));
    }
    return cur;
}

// IDA_EA: 0x822F504
// IDA_NAME: _ZN15CUserCharacInfo18getCreateGuildNameEv
// DEMANGLED: CUserCharacInfo::getCreateGuildName(void)
char* CUserCharacInfo::getCreateGuildName() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.guild_create_name_;
    }
    return 0;
}

// IDA_EA: 0x822F526
// IDA_NAME: _ZN15CUserCharacInfo18getCreateGuildAddrEv
// DEMANGLED: CUserCharacInfo::getCreateGuildAddr(void)
char* CUserCharacInfo::getCreateGuildAddr() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.guild_create_address_;
    }
    return 0;
}

// IDA_EA: 0x822F548
// IDA_NAME: _ZN15CUserCharacInfo19getGuildMemberGradeEv
// DEMANGLED: CUserCharacInfo::getGuildMemberGrade(void)
int CUserCharacInfo::getGuildMemberGrade() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.guild_member_grade_;
    }
    return 0;
}

// IDA_EA: 0x85BFB64
// IDA_NAME: _ZN15CUserCharacInfo24set_level_before_dungeonEs
// DEMANGLED: CUserCharacInfo::set_level_before_dungeon(short)
int CUserCharacInfo::set_level_before_dungeon(int16_t level) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.level_before_dungeon_ =
            level;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8645D52
// IDA_NAME: _ZNK15CUserCharacInfo31get_charac_level_before_dungeonEv
// DEMANGLED: CUserCharacInfo::get_charac_level_before_dungeon(void)const
int CUserCharacInfo::get_charac_level_before_dungeon() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.level_before_dungeon_;
    }
    return 0;
}

// IDA_EA: 0x8645D96
// IDA_NAME: _ZN15CUserCharacInfo26reset_level_before_dungeonEv
// DEMANGLED: CUserCharacInfo::reset_level_before_dungeon(void)
int CUserCharacInfo::reset_level_before_dungeon() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.level_before_dungeon_ =
            0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F56C
// IDA_NAME: _ZN15CUserCharacInfo22get_guild_create_rightEv
// DEMANGLED: CUserCharacInfo::get_guild_create_right(void)
int CUserCharacInfo::get_guild_create_right() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.guild_right_;
    }
    return 0;
}

// IDA_EA: 0x822F590
// IDA_NAME: _ZN15CUserCharacInfo22set_guild_create_rightEh
// DEMANGLED: CUserCharacInfo::set_guild_create_right(unsigned char)
int CUserCharacInfo::set_guild_create_right(uint8_t right) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacEtc();
        cur->fields_.guild_right_ = right;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC07C
// IDA_NAME: _ZN15CUserCharacInfo18setGuildCreateFlagEt
// DEMANGLED: CUserCharacInfo::setGuildCreateFlag(unsigned short)
int CUserCharacInfo::setGuildCreateFlag(uint16_t flag) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.guild_create_flag_ |=
            flag;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x866C0DA
// IDA_NAME: _ZN15CUserCharacInfo20isSetGuildCreateFlagEt
// DEMANGLED: CUserCharacInfo::isSetGuildCreateFlag(unsigned short)
int CUserCharacInfo::isSetGuildCreateFlag(uint16_t flag) {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return (cur->fields_.guild_create_flag_ &
                flag) == flag;
    }
    return 0;
}

// IDA_EA: 0x866C11E
// IDA_NAME: _ZN15CUserCharacInfo18resetCharGuildInfoEv
// DEMANGLED: CUserCharacInfo::resetCharGuildInfo(void)
int CUserCharacInfo::resetCharGuildInfo() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.guild_id_ = 0;
        cur->fields_.guild_temporary_experience_ = 0;
        cur->fields_.guild_experience_ = 0;
        cur->fields_.guild_member_grade_ = 0;
        cur->fields_.guild_right_ = 0;
        cur->fields_.guild_create_flag_ = 0;
        cur->fields_.guild_today_experience_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC0B4
// IDA_NAME: _ZN15CUserCharacInfo19setGuildMemberGradeEh
// DEMANGLED: CUserCharacInfo::setGuildMemberGrade(unsigned char)
int CUserCharacInfo::setGuildMemberGrade(uint8_t grade) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.guild_member_grade_ = grade;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x85BFB8E
// IDA_NAME: _ZN15CUserCharacInfo18get_guild_temp_expEv
// DEMANGLED: CUserCharacInfo::get_guild_temp_exp(void)
int CUserCharacInfo::get_guild_temp_exp() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.guild_temporary_experience_;
    }
    return 0;
}

// IDA_EA: 0x8645C18
// IDA_NAME: _ZN15CUserCharacInfo18add_guild_temp_expEi
// DEMANGLED: CUserCharacInfo::add_guild_temp_exp(int)
int CUserCharacInfo::add_guild_temp_exp(int exp) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        result = cur->fields_.guild_id_;
        if (result) {
            result = reinterpret_cast<int>(cur);
            cur->fields_.guild_temporary_experience_ +=
                static_cast<uint16_t>(exp);
        }
    }
    return result;
}

// IDA_EA: 0x8645C58
// IDA_NAME: _ZN15CUserCharacInfo20reset_guild_temp_expEv
// DEMANGLED: CUserCharacInfo::reset_guild_temp_exp(void)
int CUserCharacInfo::reset_guild_temp_exp() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.guild_temporary_experience_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F5C4
// IDA_NAME: _ZN15CUserCharacInfo20get_charac_memberkeyEv
// DEMANGLED: CUserCharacInfo::get_charac_memberkey(void)
int CUserCharacInfo::get_charac_memberkey() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.member_key_;
    }
    return 0;
}

// IDA_EA: 0x84EC0DC
// IDA_NAME: _ZN15CUserCharacInfo20set_charac_memberkeyEj
// DEMANGLED: CUserCharacInfo::set_charac_memberkey(unsigned int)
int CUserCharacInfo::set_charac_memberkey(uint32_t key) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.member_key_ = key;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F5E6
// IDA_NAME: _ZN15CUserCharacInfo23is_connect_upper_memberEv
// DEMANGLED: CUserCharacInfo::is_connect_upper_member(void)
int CUserCharacInfo::is_connect_upper_member() {
    const _Charac_info* const cur = current_charac_;
    return cur &&
           cur->fields_.upper_member_key_;
}

// IDA_EA: 0x84EC0FA
// IDA_NAME: _ZN15CUserCharacInfo24set_connect_upper_memberEj
// DEMANGLED: CUserCharacInfo::set_connect_upper_member(unsigned int)
int CUserCharacInfo::set_connect_upper_member(uint32_t member_key) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.upper_member_key_ =
            member_key;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x85BFBB4
// IDA_NAME: _ZN15CUserCharacInfo24get_connect_upper_memberEv
// DEMANGLED: CUserCharacInfo::get_connect_upper_member(void)
int CUserCharacInfo::get_connect_upper_member() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.upper_member_key_;
    }
    return 0;
}

// IDA_EA: 0x822F612
// IDA_NAME: _ZNK15CUserCharacInfo33get_member_pay_tex_money_to_upperEv
// DEMANGLED: CUserCharacInfo::get_member_pay_tex_money_to_upper(void)const
int CUserCharacInfo::get_member_pay_tex_money_to_upper() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.tax_money_to_upper_;
    }
    return 0;
}

// IDA_EA: 0x822F634
// IDA_NAME: _ZN15CUserCharacInfo33set_member_pay_tex_money_to_upperEi
// DEMANGLED: CUserCharacInfo::set_member_pay_tex_money_to_upper(int)
int CUserCharacInfo::set_member_pay_tex_money_to_upper(int value) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.tax_money_to_upper_ = value;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F652
// IDA_NAME: _ZNK15CUserCharacInfo35get_member_pay_tex_fatigue_to_upperEv
// DEMANGLED: CUserCharacInfo::get_member_pay_tex_fatigue_to_upper(void)const
int CUserCharacInfo::get_member_pay_tex_fatigue_to_upper() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.tax_fatigue_to_upper_;
    }
    return 0;
}

// IDA_EA: 0x822F674
// IDA_NAME: _ZN15CUserCharacInfo35set_member_pay_tex_fatigue_to_upperEi
// DEMANGLED: CUserCharacInfo::set_member_pay_tex_fatigue_to_upper(int)
int CUserCharacInfo::set_member_pay_tex_fatigue_to_upper(int value) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.tax_fatigue_to_upper_ = value;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F692
// IDA_NAME: _ZN15CUserCharacInfo12GetLoginTickEv
// DEMANGLED: CUserCharacInfo::GetLoginTick(void)
int CUserCharacInfo::GetLoginTick() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.login_tick_;
    }
    return 0;
}

// IDA_EA: 0x84EC118
// IDA_NAME: _ZN15CUserCharacInfo12SetLoginTickEl
// DEMANGLED: CUserCharacInfo::SetLoginTick(long)
int CUserCharacInfo::SetLoginTick(long tick) {
    _Charac_info* const cur = current_charac_;
    cur->fields_.login_tick_ = tick;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC12C
// IDA_NAME: _ZN15CUserCharacInfo22SetCharacStartPlayTickEl
// DEMANGLED: CUserCharacInfo::SetCharacStartPlayTick(long)
CUserCharacInfo* CUserCharacInfo::SetCharacStartPlayTick(long tick) {
    lifecycle_tick_ = tick;
    return this;
}

// IDA_EA: 0x822F6B4
// IDA_NAME: _ZN15CUserCharacInfo23setCurrCharacBlackCountEj
// DEMANGLED: CUserCharacInfo::setCurrCharacBlackCount(unsigned int)
int CUserCharacInfo::setCurrCharacBlackCount(uint32_t count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.black_count_ = count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F6D2
// IDA_NAME: _ZN15CUserCharacInfo15GetAssaultPlaceEv
// DEMANGLED: CUserCharacInfo::GetAssaultPlace(void)
int CUserCharacInfo::GetAssaultPlace() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.assault_place_;
    }
    return 0;
}

// IDA_EA: 0x822F6F4
// IDA_NAME: _ZN15CUserCharacInfo22GetCurCharacSuperStateEv
// DEMANGLED: CUserCharacInfo::GetCurCharacSuperState(void)
int CUserCharacInfo::GetCurCharacSuperState() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.super_state_;
    }
    return 0;
}

// IDA_EA: 0x822F718
// IDA_NAME: _ZN15CUserCharacInfo22SetCurCharacChaosPointEi
// DEMANGLED: CUserCharacInfo::SetCurCharacChaosPoint(int)
int CUserCharacInfo::SetCurCharacChaosPoint(int point) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.chaos_point_ = point;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F736
// IDA_NAME: _ZN15CUserCharacInfo19SetCurCharacEmotionEs
// DEMANGLED: CUserCharacInfo::SetCurCharacEmotion(short)
int CUserCharacInfo::SetCurCharacEmotion(int16_t emotion) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    cur->fields_.emotion_ = emotion;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F762
// IDA_NAME: _ZN15CUserCharacInfo25SetCurRevengeDungeonCountEh
// DEMANGLED: CUserCharacInfo::SetCurRevengeDungeonCount(unsigned char)
int CUserCharacInfo::SetCurRevengeDungeonCount(uint8_t count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.revenge_dungeon_count_ = count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F78A
// IDA_NAME: _ZNK15CUserCharacInfo25GetCurRevengeDungeonCountEv
// DEMANGLED: CUserCharacInfo::GetCurRevengeDungeonCount(void)const
int CUserCharacInfo::GetCurRevengeDungeonCount() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.revenge_dungeon_count_;
    }
    return 0;
}

// IDA_EA: 0x822F7AE
// IDA_NAME: _ZN15CUserCharacInfo25GetCurCharacChaosKillTimeEv
// DEMANGLED: CUserCharacInfo::GetCurCharacChaosKillTime(void)
int CUserCharacInfo::GetCurCharacChaosKillTime() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.chaos_kill_time_;
    }
    return 0;
}

// IDA_EA: 0x822F7D0
// IDA_NAME: _ZN15CUserCharacInfo11SetOpencoinEc
// DEMANGLED: CUserCharacInfo::SetOpencoin(char)
int CUserCharacInfo::SetOpencoin(char open_coin) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        result = cur->fields_.open_coin_flag_;
        if (static_cast<unsigned char>(result) !=
            static_cast<unsigned char>(open_coin)) {
            enableSaveCharacStat();
            cur->fields_.open_coin_flag_ =
                static_cast<unsigned char>(open_coin);
            result = reinterpret_cast<int>(cur);
        }
    }
    return result;
}

// IDA_EA: 0x822F816
// IDA_NAME: _ZN15CUserCharacInfo11GetOpencoinEv
// DEMANGLED: CUserCharacInfo::GetOpencoin(void)
int CUserCharacInfo::GetOpencoin() {
    const _Charac_info* const cur = current_charac_;
    return cur->fields_.open_coin_flag_;
}

// IDA_EA: 0x822F23C
// IDA_NAME: _ZNK15CUserCharacInfo24getCurCharSecondGrowTypeEv
// DEMANGLED: CUserCharacInfo::getCurCharSecondGrowType(void)const
char CUserCharacInfo::getCurCharSecondGrowType() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return static_cast<char>(
            static_cast<signed char>(2 * cur->fields_.grow_type_.growth_type) >>
            5);
    }
    return 0;
}

// IDA_EA: 0x822F828
// IDA_NAME: _ZN15CUserCharacInfo21GetCurCharacLuckPointEv
// DEMANGLED: CUserCharacInfo::GetCurCharacLuckPoint(void)
int CUserCharacInfo::GetCurCharacLuckPoint() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.luck_point_;
    }
    return 0;
}

// IDA_EA: 0x822F894
// IDA_NAME: _ZNK15CUserCharacInfo25GetCurCharacExpertJobTypeEv
// DEMANGLED: CUserCharacInfo::GetCurCharacExpertJobType(void)const
int CUserCharacInfo::GetCurCharacExpertJobType() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.expert_job_;
    }
    return 0;
}

// IDA_EA: 0x822F8B6
// IDA_NAME: _ZN15CUserCharacInfo21SetCurCharacExpertJobEPN10expert_job10CExpertJobE
// DEMANGLED: CUserCharacInfo::SetCurCharacExpertJob(expert_job::CExpertJob *)
int CUserCharacInfo::SetCurCharacExpertJob(
    expert_job::CExpertJob* expert_job) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.expert_job_object_ =
            expert_job;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F8D4
// IDA_NAME: _ZNK15CUserCharacInfo21GetCurCharacExpertJobEv
// DEMANGLED: CUserCharacInfo::GetCurCharacExpertJob(void)const
expert_job::CExpertJob* CUserCharacInfo::GetCurCharacExpertJob() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.expert_job_object_;
    }
    return 0;
}

// IDA_EA: 0x864781E
// IDA_NAME: _ZN15CUserCharacInfo19IsBusyDisjointStoreEv
// DEMANGLED: CUserCharacInfo::IsBusyDisjointStore(void)
int CUserCharacInfo::IsBusyDisjointStore() {
    expert_job::CExpertJob* const current_expert_job =
        GetCurCharacExpertJob();
    if (current_expert_job) {
        if (current_expert_job->GetType() == 3) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8568DEA
// IDA_NAME: _ZN15CUserCharacInfo33GetCurCharacOnlinePreliminaryTeamEv
// DEMANGLED: CUserCharacInfo::GetCurCharacOnlinePreliminaryTeam(void)
online_preliminary::COnlinePreliminaryTeam*
CUserCharacInfo::GetCurCharacOnlinePreliminaryTeam() {
    _Charac_info* const cur = current_charac_;
    return cur ? reinterpret_cast<online_preliminary::COnlinePreliminaryTeam*>(
                     static_cast<uintptr_t>(
                         cur->fields_.online_preliminary_team_handle_))
               : 0;
}

// IDA_EA: 0x8645DE2
// IDA_NAME: _ZNK15CUserCharacInfo35GetCurCharacOnlinePreliminaryTeamIdEv
// DEMANGLED: CUserCharacInfo::GetCurCharacOnlinePreliminaryTeamId(void)const
int CUserCharacInfo::GetCurCharacOnlinePreliminaryTeamId() const {
    const _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }

    online_preliminary::COnlinePreliminaryTeam* const team =
        reinterpret_cast<online_preliminary::COnlinePreliminaryTeam*>(
            static_cast<uintptr_t>(
                cur->fields_.online_preliminary_team_handle_));
    return team ? team->GetTeamId() : 0;
}

// IDA_EA: 0x8588DBA
// IDA_NAME: _ZN15CUserCharacInfo33SetCurCharacOnlinePreliminaryTeamEPN18online_preliminary22COnlinePreliminaryTeamE
// DEMANGLED: CUserCharacInfo::SetCurCharacOnlinePreliminaryTeam(online_preliminary::COnlinePreliminaryTeam *)
int CUserCharacInfo::SetCurCharacOnlinePreliminaryTeam(
    online_preliminary::COnlinePreliminaryTeam* team) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.online_preliminary_team_handle_ =
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(team));
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x849F53E
// IDA_NAME: _ZN15CUserCharacInfo25SetCurCharacExpertJobTypeEi
// DEMANGLED: CUserCharacInfo::SetCurCharacExpertJobType(int)
int CUserCharacInfo::SetCurCharacExpertJobType(int type) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacInfo();
        cur->fields_.expert_job_ = type;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x849F56A
// IDA_NAME: _ZN15CUserCharacInfo24SetCurCharacExpertJobExpEi
// DEMANGLED: CUserCharacInfo::SetCurCharacExpertJobExp(int)
int CUserCharacInfo::SetCurCharacExpertJobExp(int exp) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.expert_job_experience_ = exp;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8375026
// IDA_NAME: _ZNK15CUserCharacInfo24GetCurCharacExpertJobExpEv
// DEMANGLED: CUserCharacInfo::GetCurCharacExpertJobExp(void)const
int CUserCharacInfo::GetCurCharacExpertJobExp() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.expert_job_experience_;
    }
    return 0;
}

// IDA_EA: 0x822F8F6
// IDA_NAME: _ZN15CUserCharacInfo16isDisguiseCharacEv
// DEMANGLED: CUserCharacInfo::isDisguiseCharac(void)
int CUserCharacInfo::isDisguiseCharac() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.disguise_character_;
    }
    return 0;
}

// IDA_EA: 0x822F976
// IDA_NAME: _ZN15CUserCharacInfo15setDisguiseKindEh
// DEMANGLED: CUserCharacInfo::setDisguiseKind(unsigned char)
int CUserCharacInfo::setDisguiseKind(uint8_t kind) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.disguise_kind_ = kind;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F99E
// IDA_NAME: _ZN15CUserCharacInfo16setDisguiseIndexEt
// DEMANGLED: CUserCharacInfo::setDisguiseIndex(unsigned short)
int CUserCharacInfo::setDisguiseIndex(uint16_t index) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.disguise_index_ =
            index;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F91A
// IDA_NAME: _ZN15CUserCharacInfo17setDisguiseCharacEbht
// DEMANGLED: CUserCharacInfo::setDisguiseCharac(bool,unsigned char,unsigned short)
int CUserCharacInfo::setDisguiseCharac(bool flag,
                                       uint8_t kind,
                                       uint16_t index) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.disguise_character_ = flag ? 1 : 0;
        setDisguiseKind(kind);
        return setDisguiseIndex(index);
    }
    return 0;
}

// IDA_EA: 0x822F9C8
// IDA_NAME: _ZN15CUserCharacInfo20GetUserPowerWarPointEv
// DEMANGLED: CUserCharacInfo::GetUserPowerWarPoint(void)
int CUserCharacInfo::GetUserPowerWarPoint() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.power_war_point_;
    }
    return 0;
}

// IDA_EA: 0x8687EFC
// IDA_NAME: _ZN15CUserCharacInfo21IncreasePowerWarPointEs
// DEMANGLED: CUserCharacInfo::IncreasePowerWarPoint(short)
int CUserCharacInfo::IncreasePowerWarPoint(int16_t value) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        enableSaveCharacStat();
        int16_t point = static_cast<int16_t>(
            cur->fields_.power_war_point_ + value);
        cur->fields_.power_war_point_ = point;
        result = static_cast<uint16_t>(point);
        if (point > 9999) {
            cur->fields_.power_war_point_ = 9999;
            result = reinterpret_cast<int>(cur);
        }
    }
    return result;
}

// IDA_EA: 0x8687F66
// IDA_NAME: _ZN15CUserCharacInfo21DecreasePowerWarPointEs
// DEMANGLED: CUserCharacInfo::DecreasePowerWarPoint(short)
int CUserCharacInfo::DecreasePowerWarPoint(int16_t value) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        enableSaveCharacStat();
        int16_t point = static_cast<int16_t>(
            cur->fields_.power_war_point_ - value);
        cur->fields_.power_war_point_ = point;
        result = static_cast<uint16_t>(point);
        if ((result & 0x8000u) != 0) {
            cur->fields_.power_war_point_ = 0;
            result = reinterpret_cast<int>(cur);
        }
    }
    return result;
}

// IDA_EA: 0x822F9EC
// IDA_NAME: _ZN15CUserCharacInfo33updateLastItemLimitEditionBuyTimeEl
// DEMANGLED: CUserCharacInfo::updateLastItemLimitEditionBuyTime(long)
int CUserCharacInfo::updateLastItemLimitEditionBuyTime(long time) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.item_limit_buy_time_ =
            time;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822FA0A
// IDA_NAME: _ZNK15CUserCharacInfo30getLastItemLimitEditionBuyTimeEv
// DEMANGLED: CUserCharacInfo::getLastItemLimitEditionBuyTime(void)const
int CUserCharacInfo::getLastItemLimitEditionBuyTime() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.item_limit_buy_time_;
    }
    return 0;
}

// IDA_EA: 0x822FA2C
// IDA_NAME: _ZN15CUserCharacInfo26setUpgradeSeparateTrueFlagEc
// DEMANGLED: CUserCharacInfo::setUpgradeSeparateTrueFlag(char)
int CUserCharacInfo::setUpgradeSeparateTrueFlag(char flag) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.upgrade_separate_true_flag_ =
            static_cast<unsigned char>(flag);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822FA54
// IDA_NAME: _ZN15CUserCharacInfo27setUpgradeSeparateFalseFlagEc
// DEMANGLED: CUserCharacInfo::setUpgradeSeparateFalseFlag(char)
int CUserCharacInfo::setUpgradeSeparateFalseFlag(char flag) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.upgrade_separate_false_flag_ =
            static_cast<unsigned char>(flag);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822FA7C
// IDA_NAME: _ZN15CUserCharacInfo28getCurChannelEquipslotSwitchEv
// DEMANGLED: CUserCharacInfo::getCurChannelEquipslotSwitch(void)
int CUserCharacInfo::getCurChannelEquipslotSwitch() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.channel_equipslot_switch_;
    }
    return -1;
}

// IDA_EA: 0x822FAA0
// IDA_NAME: _ZN15CUserCharacInfo27getCurExpandEquipslotSwitchEv
// DEMANGLED: CUserCharacInfo::getCurExpandEquipslotSwitch(void)
int CUserCharacInfo::getCurExpandEquipslotSwitch() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.expand_equipslot_switch_;
    }
    return -1;
}

// IDA_EA: 0x822FAC4
// IDA_NAME: _ZN15CUserCharacInfo29incCurUseBringUpMaterialCountEv
// DEMANGLED: CUserCharacInfo::incCurUseBringUpMaterialCount(void)
int CUserCharacInfo::incCurUseBringUpMaterialCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        ++cur->fields_.event_bring_up_material_use_count_;
        ++cur->fields_.event_bring_up_material_total_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822FB00
// IDA_NAME: _ZNK15CUserCharacInfo23get_growth_power_rewardEv
// DEMANGLED: CUserCharacInfo::get_growth_power_reward(void)const
int CUserCharacInfo::get_growth_power_reward() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.growth_power_reward_;
    }
    return 0;
}

// IDA_EA: 0x822FB24
// IDA_NAME: _ZN15CUserCharacInfo23set_growth_power_rewardEi
// DEMANGLED: CUserCharacInfo::set_growth_power_reward(int)
int CUserCharacInfo::set_growth_power_reward(int reward) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.growth_power_reward_ =
            static_cast<unsigned char>(reward);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822FB50
// IDA_NAME: _ZN15CUserCharacInfo33set_growth_power_exp_reward_rightEb
// DEMANGLED: CUserCharacInfo::set_growth_power_exp_reward_right(bool)
int CUserCharacInfo::set_growth_power_exp_reward_right(bool right) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.growth_power_exp_reward_right_ = right ? 1 : 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822FB78
// IDA_NAME: _ZN15CUserCharacInfo23setSeriaRoomAniDecoInfoE17seriaRoom_AniDeco
// DEMANGLED: CUserCharacInfo::setSeriaRoomAniDecoInfo(seriaRoom_AniDeco)
int CUserCharacInfo::setSeriaRoomAniDecoInfo(seriaRoom_AniDeco deco_info) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        cur->fields_.seria_room_ani_deco_ = deco_info;
    }
    return result;
}

// IDA_EA: 0x822FB96
// IDA_NAME: _ZN15CUserCharacInfo23getSeriaRoomAniDecoInfoEv
// DEMANGLED: CUserCharacInfo::getSeriaRoomAniDecoInfo(void)
seriaRoom_AniDeco CUserCharacInfo::getSeriaRoomAniDecoInfo() {
    seriaRoom_AniDeco result;
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        result = cur->fields_.seria_room_ani_deco_;
    } else {
        result.receive_flow = 0;
        result.deco_flow = 0xFFFFu;
    }
    return result;
}

// IDA_EA: 0x8260494
// IDA_NAME: _ZN15CUserCharacInfo29setConctractOfCubePremiumInfoEbh
// DEMANGLED: CUserCharacInfo::setConctractOfCubePremiumInfo(bool,unsigned char)
int CUserCharacInfo::setConctractOfCubePremiumInfo(bool enabled,
                                                    uint8_t premium_type) {
    _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }
    cur->fields_.contract_cube_premium_saved_ = 1;
    cur->fields_.contract_cube_premium_enabled_ = enabled ? 1 : 0;
    cur->fields_.contract_cube_premium_type_ = premium_type;
    return 1;
}

// IDA_EA: 0x84EC580
// IDA_NAME: _ZN15CUserCharacInfo29initContractOfCubePremiumInfoEbh
// DEMANGLED: CUserCharacInfo::initContractOfCubePremiumInfo(bool,unsigned char)
int CUserCharacInfo::initContractOfCubePremiumInfo(bool enabled,
                                                    uint8_t premium_type) {
    _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }
    cur->fields_.contract_cube_premium_saved_ = 0;
    cur->fields_.contract_cube_premium_enabled_ = enabled ? 1 : 0;
    cur->fields_.contract_cube_premium_type_ = premium_type;
    return 1;
}

// IDA_EA: 0x827DEFA
// IDA_NAME: _ZN15CUserCharacInfo17IsEndLetheAtLoginEv
// DEMANGLED: CUserCharacInfo::IsEndLetheAtLogin(void)
int CUserCharacInfo::IsEndLetheAtLogin() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.end_lethe_at_login_;
    }
    return 0;
}

// IDA_EA: 0x8284490
// IDA_NAME: _ZN15CUserCharacInfo28set_aura_avatar_option_valueEii
// DEMANGLED: CUserCharacInfo::set_aura_avatar_option_value(int,int)
int CUserCharacInfo::set_aura_avatar_option_value(int index, int value) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur && static_cast<unsigned int>(index) <= 2u) {
        cur->fields_.aura_avatar_option_.values[index] = value;
    }
    return result;
}

// IDA_EA: 0x84EBEDE
// IDA_NAME: _ZN15CUserCharacInfo27reset_is_end_lethe_at_loginEv
// DEMANGLED: CUserCharacInfo::reset_is_end_lethe_at_login(void)
int CUserCharacInfo::reset_is_end_lethe_at_login() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.end_lethe_at_login_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82A66AA
// IDA_NAME: _ZNK15CUserCharacInfo24getCurCharacLastPlayTickEv
// DEMANGLED: CUserCharacInfo::getCurCharacLastPlayTick(void)const
int CUserCharacInfo::getCurCharacLastPlayTick() const {
    const _Charac_info* const cur = current_charac_;
    return cur->fields_.last_play_tick_;
}

// IDA_EA: 0x82A66B8
// IDA_NAME: _ZN15CUserCharacInfo34getCurCharacEventCharacterGrowtypeEv
// DEMANGLED: CUserCharacInfo::getCurCharacEventCharacterGrowtype(void)
int CUserCharacInfo::getCurCharacEventCharacterGrowtype() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.event_character_grow_type_;
    }
    return 0;
}

// IDA_EA: 0x82A66DC
// IDA_NAME: _ZN15CUserCharacInfo23getCurCharacGuildSecedeEv
// DEMANGLED: CUserCharacInfo::getCurCharacGuildSecede(void)
int CUserCharacInfo::getCurCharacGuildSecede() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.guild_secede_;
    }
    return 0;
}

// IDA_EA: 0x82A6700
// IDA_NAME: _ZN15CUserCharacInfo23setCurCharacGuildSecedeEc
// DEMANGLED: CUserCharacInfo::setCurCharacGuildSecede(char)
int CUserCharacInfo::setCurCharacGuildSecede(char secede) {
    _Charac_info* const cur = current_charac_;
    cur->fields_.guild_secede_ = static_cast<unsigned char>(secede);
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82A671E
// IDA_NAME: _ZN15CUserCharacInfo19set_charac_guildkeyEj
// DEMANGLED: CUserCharacInfo::set_charac_guildkey(unsigned int)
int CUserCharacInfo::set_charac_guildkey(uint32_t guild_key) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.guild_id_ =
            guild_key;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82A673C
// IDA_NAME: _ZN15CUserCharacInfo13get_guild_expEv
// DEMANGLED: CUserCharacInfo::get_guild_exp(void)
int CUserCharacInfo::get_guild_exp() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.guild_experience_;
    }
    return 0;
}

// IDA_EA: 0x8645C76
// IDA_NAME: _ZN15CUserCharacInfo13add_guild_expEi
// DEMANGLED: CUserCharacInfo::add_guild_exp(int)
int CUserCharacInfo::add_guild_exp(int exp) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        result = cur->fields_.guild_id_;
        if (result) {
            result = reinterpret_cast<int>(cur);
            cur->fields_.guild_experience_ += exp;
        }
    }
    return result;
}

// IDA_EA: 0x8645CB4
// IDA_NAME: _ZN15CUserCharacInfo15reset_guild_expEv
// DEMANGLED: CUserCharacInfo::reset_guild_exp(void)
int CUserCharacInfo::reset_guild_exp() {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        result = cur->fields_.guild_id_;
        if (result) {
            result = reinterpret_cast<int>(cur);
            cur->fields_.guild_experience_ = 0;
        }
    }
    return result;
}

// IDA_EA: 0x8645DB4
// IDA_NAME: _ZN15CUserCharacInfo13set_guild_expEi
// DEMANGLED: CUserCharacInfo::set_guild_exp(int)
int CUserCharacInfo::set_guild_exp(int exp) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        result = cur->fields_.guild_id_;
        if (result) {
            result = reinterpret_cast<int>(cur);
            cur->fields_.guild_experience_ = exp;
        }
    }
    return result;
}

// IDA_EA: 0x82F0914
// IDA_NAME: _ZN15CUserCharacInfo19setCurCharacStaminaEh
// DEMANGLED: CUserCharacInfo::setCurCharacStamina(unsigned char)
int CUserCharacInfo::setCurCharacStamina(uint8_t stamina) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    cur->fields_.stamina_ = stamina;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F093A
// IDA_NAME: _ZN15CUserCharacInfo25setCurCharacBackupStaminaEh
// DEMANGLED: CUserCharacInfo::setCurCharacBackupStamina(unsigned char)
int CUserCharacInfo::setCurCharacBackupStamina(uint8_t stamina) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.backup_stamina_ = stamina;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0960
// IDA_NAME: _ZNK15CUserCharacInfo23getCurrCharacBlackCountEv
// DEMANGLED: CUserCharacInfo::getCurrCharacBlackCount(void)const
int CUserCharacInfo::getCurrCharacBlackCount() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.black_count_;
    }
    return 0;
}

// IDA_EA: 0x82F0982
// IDA_NAME: _ZN15CUserCharacInfo23SetPowerWarLastPlayTimeEl
// DEMANGLED: CUserCharacInfo::SetPowerWarLastPlayTime(long)
int CUserCharacInfo::SetPowerWarLastPlayTime(long time) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.power_war_.dword_08 =
            time;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8481052
// IDA_NAME: _ZNK15CUserCharacInfo32getCurCharacLastPlayTickPowerWarEv
// DEMANGLED: CUserCharacInfo::getCurCharacLastPlayTickPowerWar(void)const
int CUserCharacInfo::getCurCharacLastPlayTickPowerWar() const {
    const _Charac_info* const cur = current_charac_;
    return cur->fields_.power_war_last_play_time_;
}

// IDA_EA: 0x8481060
// IDA_NAME: _ZN15CUserCharacInfo32setCurCharacLastPlayTickPowerWarEl
// DEMANGLED: CUserCharacInfo::setCurCharacLastPlayTickPowerWar(long)
int CUserCharacInfo::setCurCharacLastPlayTickPowerWar(long tick) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    cur->fields_.power_war_last_play_time_ = tick;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F09A0
// IDA_NAME: _ZNK15CUserCharacInfo14isJoinPowerWarEv
// DEMANGLED: CUserCharacInfo::isJoinPowerWar(void)const
int CUserCharacInfo::isJoinPowerWar() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.power_war_.byte_02;
    }
    return 0;
}

// IDA_EA: 0x8481080
// IDA_NAME: _ZN15CUserCharacInfo22setCurrCharacJoinPowerEc
// DEMANGLED: CUserCharacInfo::setCurrCharacJoinPower(char)
int CUserCharacInfo::setCurrCharacJoinPower(char join) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.power_war_.byte_02 = static_cast<unsigned char>(join);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F09C4
// IDA_NAME: _ZN15CUserCharacInfo13setPowerWarHPEt
// DEMANGLED: CUserCharacInfo::setPowerWarHP(unsigned short)
int CUserCharacInfo::setPowerWarHP(uint16_t hp) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.power_war_.word_00 = hp;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F09EE
// IDA_NAME: _ZN15CUserCharacInfo13getPowerWarHPEv
// DEMANGLED: CUserCharacInfo::getPowerWarHP(void)
int CUserCharacInfo::getPowerWarHP() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.power_war_.word_00;
    }
    return 0;
}

// IDA_EA: 0x82F0A12
// IDA_NAME: _ZNK15CUserCharacInfo20getStraightVictoriesEv
// DEMANGLED: CUserCharacInfo::getStraightVictories(void)const
int CUserCharacInfo::getStraightVictories() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.straight_victories_;
    }
    return 0;
}

// IDA_EA: 0x84B42F4
// IDA_NAME: _ZN15CUserCharacInfo20setStraightVictoriesEt
// DEMANGLED: CUserCharacInfo::setStraightVictories(unsigned short)
int CUserCharacInfo::setStraightVictories(uint16_t victories) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.straight_victories_ =
            victories;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0A36
// IDA_NAME: _ZN15CUserCharacInfo20incStraightVictoriesEv
// DEMANGLED: CUserCharacInfo::incStraightVictories(void)
int CUserCharacInfo::incStraightVictories() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        ++cur->fields_.straight_victories_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0A5C
// IDA_NAME: _ZN15CUserCharacInfo22resetStraightVictoriesEv
// DEMANGLED: CUserCharacInfo::resetStraightVictories(void)
int CUserCharacInfo::resetStraightVictories() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.straight_victories_ =
            0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0A7A
// IDA_NAME: _ZN15CUserCharacInfo15SetAssaultPlaceEi
// DEMANGLED: CUserCharacInfo::SetAssaultPlace(int)
int CUserCharacInfo::SetAssaultPlace(int place) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.assault_place_ = place;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0A98
// IDA_NAME: _ZN15CUserCharacInfo15SetAssaultCountEi
// DEMANGLED: CUserCharacInfo::SetAssaultCount(int)
int CUserCharacInfo::SetAssaultCount(int count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.assault_count_ = count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0AB6
// IDA_NAME: _ZN15CUserCharacInfo15GetAssaultCountEv
// DEMANGLED: CUserCharacInfo::GetAssaultCount(void)
int CUserCharacInfo::GetAssaultCount() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.assault_count_;
    }
    return 0;
}

// IDA_EA: 0x82F0AD8
// IDA_NAME: _ZN15CUserCharacInfo22GetCurCharacChaosPointEv
// DEMANGLED: CUserCharacInfo::GetCurCharacChaosPoint(void)
int CUserCharacInfo::GetCurCharacChaosPoint() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.chaos_point_;
    }
    return 0;
}

// IDA_EA: 0x8645F80
// IDA_NAME: _ZN15CUserCharacInfo22AddCurCharacChaosPointEi
// DEMANGLED: CUserCharacInfo::AddCurCharacChaosPoint(int)
int CUserCharacInfo::AddCurCharacChaosPoint(int point) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.chaos_point_ += point;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8645FB8
// IDA_NAME: _ZN15CUserCharacInfo22SubCurCharacChaosPointEi
// DEMANGLED: CUserCharacInfo::SubCurCharacChaosPoint(int)
int CUserCharacInfo::SubCurCharacChaosPoint(int point) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.chaos_point_ -= point;
        result = cur->fields_.chaos_point_;
        if (result < 0) {
            result = reinterpret_cast<int>(cur);
            cur->fields_.chaos_point_ = 0;
        }
    }
    return result;
}

// IDA_EA: 0x8646010
// IDA_NAME: _ZN15CUserCharacInfo24GmSetCurCharacChaosPointEi
// DEMANGLED: CUserCharacInfo::GmSetCurCharacChaosPoint(int)
int CUserCharacInfo::GmSetCurCharacChaosPoint(int point) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.chaos_point_ = point;
        if (cur->fields_.chaos_point_ < 0) {
            cur->fields_.chaos_point_ = 0;
        }
        result = cur->fields_.chaos_point_;
        if (result > 100) {
            result = reinterpret_cast<int>(cur);
            cur->fields_.chaos_point_ = 100;
        }
    }
    return result;
}

// IDA_EA: 0x84810B4
// IDA_NAME: _ZN15CUserCharacInfo26GetCurCharacChaosStateTimeEv
// DEMANGLED: CUserCharacInfo::GetCurCharacChaosStateTime(void)
int CUserCharacInfo::GetCurCharacChaosStateTime() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.assault_count_;
    }
    return 0;
}

// IDA_EA: 0x82F0AFA
// IDA_NAME: _ZN15CUserCharacInfo26GetCurCharacChaosModeCountEv
// DEMANGLED: CUserCharacInfo::GetCurCharacChaosModeCount(void)
int CUserCharacInfo::GetCurCharacChaosModeCount() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.chaos_mode_count_;
    }
    return 0;
}

// IDA_EA: 0x82F0B1C
// IDA_NAME: _ZN15CUserCharacInfo26GetCurCharacChaosKillCountEv
// DEMANGLED: CUserCharacInfo::GetCurCharacChaosKillCount(void)
int CUserCharacInfo::GetCurCharacChaosKillCount() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.chaos_kill_count_;
    }
    return 0;
}

// IDA_EA: 0x82F0B3E
// IDA_NAME: _ZN15CUserCharacInfo25GetCurCharacChaosDieCountEv
// DEMANGLED: CUserCharacInfo::GetCurCharacChaosDieCount(void)
int CUserCharacInfo::GetCurCharacChaosDieCount() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.chaos_die_count_;
    }
    return 0;
}

// IDA_EA: 0x82F0B60
// IDA_NAME: _ZN15CUserCharacInfo22SetCurCharacSuperStateEb
// DEMANGLED: CUserCharacInfo::SetCurCharacSuperState(bool)
int CUserCharacInfo::SetCurCharacSuperState(bool state) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.super_state_ = state ? 1 : 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0B88
// IDA_NAME: _ZN15CUserCharacInfo24GetCurCharacChaosDieTimeEv
// DEMANGLED: CUserCharacInfo::GetCurCharacChaosDieTime(void)
int CUserCharacInfo::GetCurCharacChaosDieTime() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.chaos_die_time_;
    }
    return 0;
}

// IDA_EA: 0x82F0BAA
// IDA_NAME: _ZN15CUserCharacInfo26SetCurCharacChaosModeCountEi
// DEMANGLED: CUserCharacInfo::SetCurCharacChaosModeCount(int)
int CUserCharacInfo::SetCurCharacChaosModeCount(int count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.chaos_mode_count_ = count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0BC8
// IDA_NAME: _ZN15CUserCharacInfo26SetCurCharacChaosKillCountEi
// DEMANGLED: CUserCharacInfo::SetCurCharacChaosKillCount(int)
int CUserCharacInfo::SetCurCharacChaosKillCount(int count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.chaos_kill_count_ = count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0BE6
// IDA_NAME: _ZN15CUserCharacInfo25SetCurCharacChaosDieCountEi
// DEMANGLED: CUserCharacInfo::SetCurCharacChaosDieCount(int)
int CUserCharacInfo::SetCurCharacChaosDieCount(int count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.chaos_die_count_ = count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0C04
// IDA_NAME: _ZN15CUserCharacInfo24SetCurCharacChaosDieTimeEi
// DEMANGLED: CUserCharacInfo::SetCurCharacChaosDieTime(int)
int CUserCharacInfo::SetCurCharacChaosDieTime(int time) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.chaos_die_time_ = time;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0C30
// IDA_NAME: _ZN15CUserCharacInfo17SetCurCharacGhostEb
// DEMANGLED: CUserCharacInfo::SetCurCharacGhost(bool)
int CUserCharacInfo::SetCurCharacGhost(bool ghost) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.ghost_state_ = ghost ? 1 : 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0C64
// IDA_NAME: _ZN15CUserCharacInfo26IncCurCharacChaosModeCountEv
// DEMANGLED: CUserCharacInfo::IncCurCharacChaosModeCount(void)
int CUserCharacInfo::IncCurCharacChaosModeCount() {
    enableSaveCharacStat();
    const int count = GetCurCharacChaosModeCount();
    return SetCurCharacChaosModeCount(count + 1);
}

// IDA_EA: 0x82F0C94
// IDA_NAME: _ZN15CUserCharacInfo26IncCurCharacChaosKillCountEv
// DEMANGLED: CUserCharacInfo::IncCurCharacChaosKillCount(void)
int CUserCharacInfo::IncCurCharacChaosKillCount() {
    enableSaveCharacStat();
    const int count = GetCurCharacChaosKillCount();
    return SetCurCharacChaosKillCount(count + 1);
}

// IDA_EA: 0x82F0CC4
// IDA_NAME: _ZN15CUserCharacInfo25IncCurCharacChaosDieCountEv
// DEMANGLED: CUserCharacInfo::IncCurCharacChaosDieCount(void)
int CUserCharacInfo::IncCurCharacChaosDieCount() {
    enableSaveCharacStat();
    const int count = GetCurCharacChaosDieCount();
    return SetCurCharacChaosDieCount(count + 1);
}

// IDA_EA: 0x82F0CF4
// IDA_NAME: _ZN15CUserCharacInfo24IncCurCharacAssaultCountEv
// DEMANGLED: CUserCharacInfo::IncCurCharacAssaultCount(void)
int CUserCharacInfo::IncCurCharacAssaultCount() {
    enableSaveCharacStat();
    const int count = GetAssaultCount();
    return SetAssaultCount(count + 1);
}

// IDA_EA: 0x82F0D24
// IDA_NAME: _ZN15CUserCharacInfo21GetUserStateByAssaultEv
// DEMANGLED: CUserCharacInfo::GetUserStateByAssault(void)
int CUserCharacInfo::GetUserStateByAssault() {
    const _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }
    if (cur->fields_.super_state_) {
        return 6;
    }
    if (cur->fields_.ghost_state_) {
        return 4;
    }
    if (cur->fields_.assault_place_) {
        return 5;
    }
    return 0;
}

// IDA_EA: 0x82F0D80
// IDA_NAME: _ZN15CUserCharacInfo24SetCurCharacLeaveTimeoutEb
// DEMANGLED: CUserCharacInfo::SetCurCharacLeaveTimeout(bool)
int CUserCharacInfo::SetCurCharacLeaveTimeout(bool timeout) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.leave_timeout_state_ = timeout ? 1 : 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0DA8
// IDA_NAME: _ZN15CUserCharacInfo25SetCurCharacChaosKillTimeEi
// DEMANGLED: CUserCharacInfo::SetCurCharacChaosKillTime(int)
int CUserCharacInfo::SetCurCharacChaosKillTime(int time) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.chaos_kill_time_ = time;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0DC6
// IDA_NAME: _ZN15CUserCharacInfo23IncPowerWarAssaultCountEv
// DEMANGLED: CUserCharacInfo::IncPowerWarAssaultCount(void)
int CUserCharacInfo::IncPowerWarAssaultCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        ++cur->fields_.power_war_assault_count_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x82F0DF8
// IDA_NAME: _ZN15CUserCharacInfo30IncPowerWarAssaultVictoryCountEv
// DEMANGLED: CUserCharacInfo::IncPowerWarAssaultVictoryCount(void)
int CUserCharacInfo::IncPowerWarAssaultVictoryCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        ++cur->fields_.power_war_victory_count_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC30C
// IDA_NAME: _ZN15CUserCharacInfo36SetCurCharacLuckPointUpdateTimeCountEi
// DEMANGLED: CUserCharacInfo::SetCurCharacLuckPointUpdateTimeCount(int)
int CUserCharacInfo::SetCurCharacLuckPointUpdateTimeCount(int count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.luck_point_update_count_ =
            count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC32A
// IDA_NAME: _ZN15CUserCharacInfo18SetOneDayLetheFlagEh
// DEMANGLED: CUserCharacInfo::SetOneDayLetheFlag(unsigned char)
int CUserCharacInfo::SetOneDayLetheFlag(uint8_t flag) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.one_day_lethe_flag_ = flag;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x85C4B8C
// IDA_NAME: _ZNK15CUserCharacInfo18GetOneDayLetheFlagEv
// DEMANGLED: CUserCharacInfo::GetOneDayLetheFlag(void)const
int CUserCharacInfo::GetOneDayLetheFlag() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.one_day_lethe_flag_;
    }
    return 0;
}

// IDA_EA: 0x84EC352
// IDA_NAME: _ZN15CUserCharacInfo18SetIsInitSkillFlagEb
// DEMANGLED: CUserCharacInfo::SetIsInitSkillFlag(bool)
int CUserCharacInfo::SetIsInitSkillFlag(bool flag) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.initialized_skill_flag_ = flag ? 1 : 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC37A
// IDA_NAME: _ZN15CUserCharacInfo21SetOneDayLetheFlag2NDEh
// DEMANGLED: CUserCharacInfo::SetOneDayLetheFlag2ND(unsigned char)
int CUserCharacInfo::SetOneDayLetheFlag2ND(uint8_t flag) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.one_day_lethe_second_flag_ = flag;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x85C4BB0
// IDA_NAME: _ZNK15CUserCharacInfo21GetOneDayLetheFlag2NDEv
// DEMANGLED: CUserCharacInfo::GetOneDayLetheFlag2ND(void)const
int CUserCharacInfo::GetOneDayLetheFlag2ND() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.one_day_lethe_second_flag_;
    }
    return 0;
}

// IDA_EA: 0x84EC3A2
// IDA_NAME: _ZN15CUserCharacInfo21SetIsInitSkillFlag2NDEb
// DEMANGLED: CUserCharacInfo::SetIsInitSkillFlag2ND(bool)
int CUserCharacInfo::SetIsInitSkillFlag2ND(bool flag) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.initialized_skill_second_flag_ = flag ? 1 : 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC3CA
// IDA_NAME: _ZN15CUserCharacInfo26ResetPowerWarAssaultRecordEv
// DEMANGLED: CUserCharacInfo::ResetPowerWarAssaultRecord(void)
int CUserCharacInfo::ResetPowerWarAssaultRecord() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.power_war_assault_count_ = 0;
        cur->fields_.power_war_victory_count_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC408
// IDA_NAME: _ZN15CUserCharacInfo23GetPowerWarAssaultCountEv
// DEMANGLED: CUserCharacInfo::GetPowerWarAssaultCount(void)
int CUserCharacInfo::GetPowerWarAssaultCount() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.power_war_assault_count_;
    }
    return 0;
}

// IDA_EA: 0x84EC42A
// IDA_NAME: _ZN15CUserCharacInfo30GetPowerWarAssaultVictoryCountEv
// DEMANGLED: CUserCharacInfo::GetPowerWarAssaultVictoryCount(void)
int CUserCharacInfo::GetPowerWarAssaultVictoryCount() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.power_war_victory_count_;
    }
    return 0;
}

// IDA_EA: 0x84EC44C
// IDA_NAME: _ZN15CUserCharacInfo22SetCurExpEquipSlotStatEc
// DEMANGLED: CUserCharacInfo::SetCurExpEquipSlotStat(char)
int CUserCharacInfo::SetCurExpEquipSlotStat(char stat) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.exp_equip_slot_state_ =
            static_cast<unsigned char>(stat);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC474
// IDA_NAME: _ZN15CUserCharacInfo22getLinkCharacSlotIndexEv
// DEMANGLED: CUserCharacInfo::getLinkCharacSlotIndex(void)
int CUserCharacInfo::getLinkCharacSlotIndex() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.link_charac_slot_index_;
    }
    return -1;
}

// IDA_EA: 0x84EC498
// IDA_NAME: _ZN15CUserCharacInfo29setCurUseBringUpMaterialTotalEh
// DEMANGLED: CUserCharacInfo::setCurUseBringUpMaterialTotal(unsigned char)
int CUserCharacInfo::setCurUseBringUpMaterialTotal(uint8_t total) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.event_bring_up_material_total_ = total;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x863BDC4
// IDA_NAME: _ZN15CUserCharacInfo11isSaveInvenEv
// DEMANGLED: CUserCharacInfo::isSaveInven(void)
int CUserCharacInfo::isSaveInven() {
    return save_inventory_;
}

// IDA_EA: 0x863BDD0
// IDA_NAME: _ZN15CUserCharacInfo36GetCurCharacLuckPointUpdateTimeCountEv
// DEMANGLED: CUserCharacInfo::GetCurCharacLuckPointUpdateTimeCount(void)
int CUserCharacInfo::GetCurCharacLuckPointUpdateTimeCount() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.luck_point_update_count_;
    }
    return 0;
}

// IDA_EA: 0x863BDF2
// IDA_NAME: _ZN15CUserCharacInfo36IncCurCharacLuckPointUpdateTimeCountEv
// DEMANGLED: CUserCharacInfo::IncCurCharacLuckPointUpdateTimeCount(void)
int CUserCharacInfo::IncCurCharacLuckPointUpdateTimeCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        ++cur->fields_.luck_point_update_count_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8646496
// IDA_NAME: _ZN15CUserCharacInfo34IsAvailableCurCharacTradeGoldDailyEj
// DEMANGLED: CUserCharacInfo::IsAvailableCurCharacTradeGoldDaily(unsigned int)
bool CUserCharacInfo::IsAvailableCurCharacTradeGoldDaily(
    uint32_t add_gold) {
    const _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }

    const int level = get_charac_level();
    const uint32_t limit =
        10000u * static_cast<uint32_t>(level) *
        static_cast<uint32_t>(get_charac_level());
    const uint32_t used = static_cast<uint32_t>(
        cur->fields_.reliable_person_.GetTradeGoldDaily());
    if (add_gold + used <= limit) {
        return 1;
    }

    const int used_for_log =
        cur->fields_.reliable_person_.GetTradeGoldDaily();
    const int level_for_log = get_charac_level();
    const char* name = getCurCharacName();
    cMyTrace trace(
        "bool CUserCharacInfo::IsAvailableCurCharacTradeGoldDaily(unsigned int)",
        1009,
        0);
    trace("TRADE_RESTIRCTION charac_name(%s), charac_level(%d), "
          "limit_gold(%u), add_gold(%u), use_gold(%u)",
          name,
          level_for_log,
          limit,
          add_gold,
          static_cast<uint32_t>(used_for_log));
    return 0;
}

// IDA_EA: 0x864670A
// IDA_NAME: _ZN15CUserCharacInfo21SetCurCharacLuckPointEi
// DEMANGLED: CUserCharacInfo::SetCurCharacLuckPoint(int)
int CUserCharacInfo::SetCurCharacLuckPoint(int luck_point) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        int value = cur->fields_.luck_point_;
        if (luck_point - value == 100000) {
            cur->fields_.luck_point_ = 100000;
        } else {
            value = luck_point;
            if (value < 0) {
                value = 0;
            } else if (value > 99999) {
                value = 99999;
            }
            cur->fields_.luck_point_ = value;
        }
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F84A
// IDA_NAME: _ZN15CUserCharacInfo21IncCurCharacLuckPointEi
// DEMANGLED: CUserCharacInfo::IncCurCharacLuckPoint(int)
char CUserCharacInfo::IncCurCharacLuckPoint(int delta) {
    enableSaveCharacStat();
    if (GetCurCharacLuckPoint() == 100000) {
        return 1;
    }
    const int luck_point = GetCurCharacLuckPoint();
    return static_cast<char>(SetCurCharacLuckPoint(delta + luck_point));
}

// IDA_EA: 0x8655BCE
// IDA_NAME: _ZN15CUserCharacInfo24calcCurCharacSchoolPointEi
// DEMANGLED: CUserCharacInfo::calcCurCharacSchoolPoint(int)
unsigned int CUserCharacInfo::calcCurCharacSchoolPoint(
    int used_fatigue_before) {
    static const int point[24] = {
        0, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4, 4,
        5, 5, 5, 5
    };

    const unsigned int current_used = static_cast<unsigned int>(
        static_cast<uint16_t>(getCurCharacUsedFatigue()));
    const unsigned int current_group = current_used / 10u;
    if (used_fatigue_before / 10 !=
        static_cast<int>(current_group)) {
        unsigned int index = current_group;
        if (index > 23u) {
            index = 23u;
        }
        _Charac_info* const cur = getCurCharacW();
        cur->fields_.school_point_ +=
            point[index];
        return static_cast<unsigned int>(
            reinterpret_cast<uintptr_t>(cur));
    }
    return current_group;
}

// IDA_EA: 0x855100A
// IDA_NAME: _ZN15CUserCharacInfo21DecCurCharacLuckPointEi
// DEMANGLED: CUserCharacInfo::DecCurCharacLuckPoint(int)
int CUserCharacInfo::DecCurCharacLuckPoint(int deduction_rate) {
    enableSaveCharacStat();
    const int luck_point = GetCurCharacLuckPoint();
    const int percentage = GetTenThousandPercentage(
        GetCurCharacLuckPoint(), deduction_rate);
    return SetCurCharacLuckPoint(luck_point - percentage);
}

// IDA_EA: 0x86467A0
// IDA_NAME: _ZNK15CUserCharacInfo25GetCurCharacMaxEquipLevelEv
// DEMANGLED: CUserCharacInfo::GetCurCharacMaxEquipLevel(void)const
int CUserCharacInfo::GetCurCharacMaxEquipLevel() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.maximum_equip_level_;
    }
    return 0;
}

// IDA_EA: 0x86467C2
// IDA_NAME: _ZN15CUserCharacInfo25SetCurCharacMaxEquipLevelEi
// DEMANGLED: CUserCharacInfo::SetCurCharacMaxEquipLevel(int)
int CUserCharacInfo::SetCurCharacMaxEquipLevel(int level) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacManageInfo();
        cur->fields_.maximum_equip_level_ = level;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x822F15E
// IDA_NAME: _ZNK15CUserCharacInfo16getPVPResultRefREv
// DEMANGLED: CUserCharacInfo::getPVPResultRefR(void)const
const PvpResultType* CUserCharacInfo::getPVPResultRefR() const {
    return &pvp_result_;
}

// IDA_EA: 0x822F16A
// IDA_NAME: _ZN15CUserCharacInfo13getPVPResultWEv
// DEMANGLED: CUserCharacInfo::getPVPResultW(void)
PvpResultType* CUserCharacInfo::getPVPResultW() {
    enableSavePvP();
    return &pvp_result_;
}

// IDA_EA: 0x84EBEC4
// IDA_NAME: _ZN15CUserCharacInfo16getPVPResultRefWEv
// DEMANGLED: CUserCharacInfo::getPVPResultRefW(void)
PvpResultType* CUserCharacInfo::getPVPResultRefW() {
    enableSavePvP();
    return &pvp_result_;
}

// IDA_EA: 0x850D488
// IDA_NAME: _ZNK15CUserCharacInfo13getPVPResultREv
// DEMANGLED: CUserCharacInfo::getPVPResultR(void)const
const PvpResultType* CUserCharacInfo::getPVPResultR() const {
    return &pvp_result_;
}

// IDA_EA: 0x85DF99A
// IDA_NAME: _ZNK15CUserCharacInfo24get_pvp_result_exp_pointEv
// DEMANGLED: CUserCharacInfo::get_pvp_result_exp_point(void)const
int CUserCharacInfo::get_pvp_result_exp_point() const {
    return pvp_result_.values[2];
}

// IDA_EA: 0x85DF9A6
// IDA_NAME: _ZNK15CUserCharacInfo27get_pvp_result_oldexp_pointEv
// DEMANGLED: CUserCharacInfo::get_pvp_result_oldexp_point(void)const
int CUserCharacInfo::get_pvp_result_oldexp_point() const {
    return pvp_result_.values[3];
}

// IDA_EA: 0x822F184
// IDA_NAME: _ZN15CUserCharacInfo22setDemensionInoutValueEcc
// DEMANGLED: CUserCharacInfo::setDemensionInoutValue(char,char)
int CUserCharacInfo::setDemensionInoutValue(char index, char value) {
    _Charac_info* const cur = current_charac_;
    int result = value;
    if (static_cast<uint8_t>(index) <= 5u) {
        cur->fields_.dimension_dirty_ = 1;
        result = index;
        cur->fields_.dimension_inout_[static_cast<uint8_t>(index)] =
            static_cast<unsigned char>(value);
    }
    return result;
}

// IDA_EA: 0x85BF9D8
// IDA_NAME: _ZN15CUserCharacInfo22getDemensionInoutValueEc
// DEMANGLED: CUserCharacInfo::getDemensionInoutValue(char)
int CUserCharacInfo::getDemensionInoutValue(char index) {
    const _Charac_info* const cur = current_charac_;
    if (index < 6u) {
        return cur->fields_.dimension_inout_[static_cast<uint8_t>(index)];
    }
    return 0;
}

// IDA_EA: 0x822F1CC
// IDA_NAME: _ZN15CUserCharacInfo21setUltimateInoutValueEcc
// DEMANGLED: CUserCharacInfo::setUltimateInoutValue(char,char)
int CUserCharacInfo::setUltimateInoutValue(char index, char value) {
    _Charac_info* const cur = current_charac_;
    cur->fields_.ultimate_inout_dirty_ = 1;
    cur->fields_.ultimate_inout_[static_cast<uint8_t>(index)] =
        static_cast<unsigned char>(value);
    return index;
}

// IDA_EA: 0x848ED5A
// IDA_NAME: _ZN15CUserCharacInfo34setCurCharacEventCharacterGrowtypeEc
// DEMANGLED: CUserCharacInfo::setCurCharacEventCharacterGrowtype(char)
int CUserCharacInfo::setCurCharacEventCharacterGrowtype(char grow_type) {
    enableSaveCharacEtc();
    _Charac_info* const cur = current_charac_;
    cur->fields_.event_character_grow_type_ =
        static_cast<unsigned char>(grow_type);
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x849AAB0
// IDA_NAME: _ZN15CUserCharacInfo28setCurChannelEquipslotSwitchEc
// DEMANGLED: CUserCharacInfo::setCurChannelEquipslotSwitch(char)
int CUserCharacInfo::setCurChannelEquipslotSwitch(char sw) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.channel_equipslot_switch_ =
            static_cast<unsigned char>(sw);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x85BFA0C
// IDA_NAME: _ZN15CUserCharacInfo22subDemensionInoutValueEc
// DEMANGLED: CUserCharacInfo::subDemensionInoutValue(char)
int CUserCharacInfo::subDemensionInoutValue(char index) {
    _Charac_info* const cur = current_charac_;
    int result = index;
    if (static_cast<uint8_t>(index) <= 5u) {
        cur->fields_.dimension_dirty_ = 1;
        result = static_cast<signed char>(index);
        --cur->fields_.dimension_inout_[static_cast<uint8_t>(index)];
    }
    return result;
}

// IDA_EA: 0x85BFA56
// IDA_NAME: _ZN15CUserCharacInfo21subUltimateInoutValueEc
// DEMANGLED: CUserCharacInfo::subUltimateInoutValue(char)
int CUserCharacInfo::subUltimateInoutValue(char index) {
    _Charac_info* const cur = current_charac_;
    cur->fields_.ultimate_inout_dirty_ = 1;
    --cur->fields_.ultimate_inout_[static_cast<uint8_t>(index)];
    return index;
}

// IDA_EA: 0x85BFA8E
// IDA_NAME: _ZN15CUserCharacInfo23getBloodBestRoundRecordEv
// DEMANGLED: CUserCharacInfo::getBloodBestRoundRecord(void)
int CUserCharacInfo::getBloodBestRoundRecord() {
    const _Charac_info* const cur = current_charac_;
    return cur->fields_.blood_best_round_record_;
}

// IDA_EA: 0x85BFAA0
// IDA_NAME: _ZN15CUserCharacInfo22getBloodBestTimeRecordEv
// DEMANGLED: CUserCharacInfo::getBloodBestTimeRecord(void)
int CUserCharacInfo::getBloodBestTimeRecord() {
    const _Charac_info* const cur = current_charac_;
    return cur->fields_.blood_best_time_record_;
}

// IDA_EA: 0x864E1F2
// IDA_NAME: _ZN15CUserCharacInfo14resetSaveFlagsEv
// DEMANGLED: CUserCharacInfo::resetSaveFlags(void)
CUserCharacInfo* CUserCharacInfo::resetSaveFlags() {
    save_charac_info_ = 0;
    save_charac_stat_ = 0;
    save_charac_etc_ = 0;
    save_charac_manage_info_ = 0;
    save_inventory_ = 0;
    save_skill_ = 0;
    save_pvp_ = 0;
    save_quest_ = 0;
    reset_only_08_ = 0;
    save_dungeon_clear_ = 0;
    save_battle_record_ = 0;
    save_cargo_ = 0;
    reset_only_0c_ = 0;
    save_charac_option_ = 0;
    return this;
}

// IDA_EA: 0x864E25C
// IDA_NAME: _ZN15CUserCharacInfo12setCurCharacEP12_Charac_info
// DEMANGLED: CUserCharacInfo::setCurCharac(_Charac_info *)
CUserCharacInfo* CUserCharacInfo::setCurCharac(_Charac_info* charac) {
    enableSaveCharacInfo();
    enableSaveCharacStat();
    enableSaveCharacEtc();
    current_charac_ = charac;
    return this;
}

// IDA_EA: 0x864E28E
// IDA_NAME: _ZN15CUserCharacInfo12setTagCharacEP12_Charac_info
// DEMANGLED: CUserCharacInfo::setTagCharac(_Charac_info *)
CUserCharacInfo* CUserCharacInfo::setTagCharac(_Charac_info* charac) {
    tag_charac_ = charac;
    return this;
}

// IDA_EA: 0x864E29C
// IDA_NAME: _ZN15CUserCharacInfo11useWinPointEi12eWPSubReason
// DEMANGLED: CUserCharacInfo::useWinPoint(int,eWPSubReason)
CUserCharacInfo* CUserCharacInfo::useWinPoint(int point,
                                              eWPSubReason /*reason*/) {
    enableSavePvP();
    int& win_point = pvp_result_.values[22];
    if (win_point < point) {
        win_point = 0;
    } else {
        win_point -= point;
    }
    return this;
}

// IDA_EA: 0x864FCC6
// IDA_NAME: _ZN5CUser11UseWinPointEi12eWPSubReason
// DEMANGLED: CUser::UseWinPoint(int,eWPSubReason)
int CUser::UseWinPoint(int point, eWPSubReason reason) {
    useWinPoint(point, reason);
    if (point) {
        history_log().WPSub(getPVPResultR()->values[22], point, reason);
    }
    return getPVPResultR()->values[22];
}

// IDA_EA: 0x864FD2C
// IDA_NAME: _ZN5CUser12gainWinPointEi12eWPAddReason
// DEMANGLED: CUser::gainWinPoint(int,eWPAddReason)
void CUser::gainWinPoint(int point, eWPAddReason reason) {
    if (point) {
        getPVPResultW()->values[22] += point;
        history_log().WPAdd(getPVPResultR()->values[22], point, reason);
    }
}

// IDA_EA: 0x864E2D8
// IDA_NAME: _ZN15CUserCharacInfo20setCurCharGrowthTypeEcc
// DEMANGLED: CUserCharacInfo::setCurCharGrowthType(char,char)
int CUserCharacInfo::setCurCharGrowthType(char growth, char second) {
    int result = second;
    if (growth <= 6 && second <= 2) {
        enableSaveCharacInfo();
        _Charac_info* const cur = current_charac_;
        cur->fields_.grow_type_.growth_type =
            (static_cast<unsigned char>((16 * growth) >> 4) & 0x0F) |
            (cur->fields_.grow_type_.growth_type & 0xF0);
        result =
            (16 * (((32 * second) >> 5) & 7)) |
            (cur->fields_.grow_type_.growth_type & 0x8F);
        cur->fields_.grow_type_.growth_type =
            static_cast<unsigned char>(result);
    }
    return result;
}

// IDA_EA: 0x864E34E
// IDA_NAME: _ZN15CUserCharacInfo28setCurCharChangingGrowthTypeEcc
// DEMANGLED: CUserCharacInfo::setCurCharChangingGrowthType(char,char)
int CUserCharacInfo::setCurCharChangingGrowthType(char growth, char second) {
    int result = second;
    if (growth <= 6 && second <= 2) {
        _Charac_info* const cur = current_charac_;
        cur->fields_.changing_grow_type_.growth_type =
            (static_cast<unsigned char>((16 * growth) >> 4) & 0x0F) |
            (cur->fields_.changing_grow_type_.growth_type & 0xF0);
        result =
            (16 * (((32 * second) >> 5) & 7)) |
            (cur->fields_.changing_grow_type_.growth_type & 0x8F);
        cur->fields_.changing_grow_type_.growth_type =
            static_cast<unsigned char>(result);
    }
    return result;
}

// IDA_EA: 0x864E3FA
// IDA_NAME: _ZN15CUserCharacInfo31calcHelpAbuseRatioBeforeLevelUpEi
// DEMANGLED: CUserCharacInfo::calcHelpAbuseRatioBeforeLevelUp(int)
void CUserCharacInfo::calcHelpAbuseRatioBeforeLevelUp(int /*level*/) {}

// IDA_EA: 0x864E402
// IDA_NAME: _ZN15CUserCharacInfo18calcHelpAbuseRatioEjb
// DEMANGLED: CUserCharacInfo::calcHelpAbuseRatio(unsigned int,bool)
int CUserCharacInfo::calcHelpAbuseRatio(uint32_t /*ratio*/, bool flag) {
    return flag ? 1 : 0;
}

// IDA_EA: 0x8696018
// IDA_NAME: _ZNK15CUserCharacInfo29getCurCharacStrikerSkillIndexEv
// DEMANGLED: CUserCharacInfo::getCurCharacStrikerSkillIndex(void)const
int CUserCharacInfo::getCurCharacStrikerSkillIndex() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.striker_skill_index_;
    }
    return 0;
}

// IDA_EA: 0x869603C
// IDA_NAME: _ZNK15CUserCharacInfo23getCurCharacTagCharacNoEv
// DEMANGLED: CUserCharacInfo::getCurCharacTagCharacNo(void)const
int CUserCharacInfo::getCurCharacTagCharacNo() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.tag_character_no_;
    }
    return 0;
}

// IDA_EA: 0x869605E
// IDA_NAME: _ZN15CUserCharacInfo13getTagCharacWEv
// DEMANGLED: CUserCharacInfo::getTagCharacW(void)
_Charac_info* CUserCharacInfo::getTagCharacW() {
    return tag_charac_;
}

// IDA_EA: 0x869606A
// IDA_NAME: _ZN15CUserCharacInfo18getTagCharacInvenWEv
// DEMANGLED: CUserCharacInfo::getTagCharacInvenW(void)
CInventory* CUserCharacInfo::getTagCharacInvenW() {
    return &tag_charac_->fields_.inventory_;
}

// IDA_EA: 0x869607A
// IDA_NAME: _ZNK15CUserCharacInfo20getTagCharacAddInfoREv
// DEMANGLED: CUserCharacInfo::getTagCharacAddInfoR(void)const
_Additioal_info* CUserCharacInfo::getTagCharacAddInfoR() const {
    return &tag_charac_->fields_.additional_info_;
}

// IDA_EA: 0x869608A
// IDA_NAME: _ZN15CUserCharacInfo18getTagCharacCargoWEv
// DEMANGLED: CUserCharacInfo::getTagCharacCargoW(void)
CCargo* CUserCharacInfo::getTagCharacCargoW() {
    return &tag_charac_->fields_.cargo_;
}

// IDA_EA: 0x869609A
// IDA_NAME: _ZNK15CUserCharacInfo18getTagCharacCargoREv
// DEMANGLED: CUserCharacInfo::getTagCharacCargoR(void)const
CCargo* CUserCharacInfo::getTagCharacCargoR() const {
    return &tag_charac_->fields_.cargo_;
}

// IDA_EA: 0x84B42BC
// IDA_NAME: _ZNK15CUserCharacInfo20getCurCharacAddInfoREv
// DEMANGLED: CUserCharacInfo::getCurCharacAddInfoR(void)const
_Additioal_info* CUserCharacInfo::getCurCharacAddInfoR() const {
    return &current_charac_->fields_.additional_info_;
}

// IDA_EA: 0x86960AA
// IDA_NAME: _ZNK15CUserCharacInfo23getCurCharacAddInfoRefREv
// DEMANGLED: CUserCharacInfo::getCurCharacAddInfoRefR(void)const
_Additioal_info* CUserCharacInfo::getCurCharacAddInfoRefR() const {
    return &current_charac_->fields_.additional_info_;
}

// IDA_EA: 0x86960BA
// IDA_NAME: _ZN15CUserCharacInfo20getCurCharacAddInfoWEv
// DEMANGLED: CUserCharacInfo::getCurCharacAddInfoW(void)
_Additioal_info* CUserCharacInfo::getCurCharacAddInfoW() {
    enableSaveCharacInfo();
    return &current_charac_->fields_.additional_info_;
}

// IDA_EA: 0x86960D8
// IDA_NAME: _ZN15CUserCharacInfo23getCurCharacAddInfoRefWEv
// DEMANGLED: CUserCharacInfo::getCurCharacAddInfoRefW(void)
_Additioal_info* CUserCharacInfo::getCurCharacAddInfoRefW() {
    enableSaveCharacInfo();
    return &current_charac_->fields_.additional_info_;
}

// IDA_EA: 0x86960F6
// IDA_NAME: _ZN15CUserCharacInfo24setCurCharacLastPlayTickEl
// DEMANGLED: CUserCharacInfo::setCurCharacLastPlayTick(long)
int CUserCharacInfo::setCurCharacLastPlayTick(long tick) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    cur->fields_.last_play_tick_ = static_cast<uint32_t>(tick);
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696116
// IDA_NAME: _ZN15CUserCharacInfo25IsSaveDemensionInoutValueEv
// DEMANGLED: CUserCharacInfo::IsSaveDemensionInoutValue(void)
int CUserCharacInfo::IsSaveDemensionInoutValue() {
    const _Charac_info* const cur = current_charac_;
    return cur->fields_.dimension_dirty_;
}

// IDA_EA: 0x8696128
// IDA_NAME: _ZN15CUserCharacInfo26SetSaveDemensionInoutValueEb
// DEMANGLED: CUserCharacInfo::SetSaveDemensionInoutValue(bool)
int CUserCharacInfo::SetSaveDemensionInoutValue(bool flag) {
    _Charac_info* const cur = current_charac_;
    cur->fields_.dimension_dirty_ = flag ? 1 : 0;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696146
// IDA_NAME: _ZN15CUserCharacInfo21IsSaveBloodInoutValueEv
// DEMANGLED: CUserCharacInfo::IsSaveBloodInoutValue(void)
int CUserCharacInfo::IsSaveBloodInoutValue() {
    const _Charac_info* const cur = current_charac_;
    return cur->fields_.ultimate_inout_dirty_;
}

// IDA_EA: 0x8696158
// IDA_NAME: _ZN15CUserCharacInfo22SetSaveBloodInoutValueEb
// DEMANGLED: CUserCharacInfo::SetSaveBloodInoutValue(bool)
int CUserCharacInfo::SetSaveBloodInoutValue(bool flag) {
    _Charac_info* const cur = current_charac_;
    cur->fields_.ultimate_inout_dirty_ = flag ? 1 : 0;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696176
// IDA_NAME: _ZN15CUserCharacInfo21getUltimateInoutValueEc
// DEMANGLED: CUserCharacInfo::getUltimateInoutValue(char)
int CUserCharacInfo::getUltimateInoutValue(char index) {
    const _Charac_info* const cur = current_charac_;
    return cur->fields_.ultimate_inout_[static_cast<uint8_t>(index)];
}

// IDA_EA: 0x8696196
// IDA_NAME: _ZN15CUserCharacInfo23isBloodBestRecordUpdateEv
// DEMANGLED: CUserCharacInfo::isBloodBestRecordUpdate(void)
int CUserCharacInfo::isBloodBestRecordUpdate() {
    const _Charac_info* const cur = current_charac_;
    return cur->fields_.blood_best_record_update_;
}

// IDA_EA: 0x86961A8
// IDA_NAME: _ZN15CUserCharacInfo24setBloodBestRecordUpdateEb
// DEMANGLED: CUserCharacInfo::setBloodBestRecordUpdate(bool)
int CUserCharacInfo::setBloodBestRecordUpdate(bool flag) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.blood_best_record_update_ = flag ? 1 : 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x86961D0
// IDA_NAME: _ZNK15CUserCharacInfo21getCurCharacVill4SaveEv
// DEMANGLED: CUserCharacInfo::getCurCharacVill4Save(void)const
int CUserCharacInfo::getCurCharacVill4Save() const {
    const _Charac_info* const cur = current_charac_;
    if (!cur) return 0;

    const int current_village = cur->fields_.village_;
    Village* const current = G_GameWorld()->GetVillage(current_village);
    if (current && current->has_gate_area()) {
        return current_village;
    }

    // A PVF town without [gate] is a transient destination (for example
    // City Hall).  It must remain the live routing village while the player
    // is inside it, but it is not a safe reconnect point.  Persist the saved
    // previous town only when that town has a real PVF gate.
    const int previous_village = cur->fields_.previous_village_;
    Village* const previous = G_GameWorld()->GetVillage(previous_village);
    return previous && previous->has_gate_area() ? previous_village
                                                  : current_village;
}

// IDA_EA: 0x86961E0
// IDA_NAME: _ZNK15CUserCharacInfo20getCurCharacVillPrevEv
// DEMANGLED: CUserCharacInfo::getCurCharacVillPrev(void)const
int CUserCharacInfo::getCurCharacVillPrev() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.previous_village_;
    }
    return 0;
}

// IDA_EA: 0x8696200
// IDA_NAME: _ZNK15CUserCharacInfo19getBattleRecordRefREv
// DEMANGLED: CUserCharacInfo::getBattleRecordRefR(void)const
std::map<int, GameResultType>* CUserCharacInfo::getBattleRecordRefR() const {
    return const_cast<CUserBattleRecordMap*>(&battle_records_);
}

// IDA_EA: 0x8696272
// IDA_NAME: _ZN15CUserCharacInfo22enableSaveBattleRecordEv
// DEMANGLED: CUserCharacInfo::enableSaveBattleRecord(void)
CUserCharacInfo* CUserCharacInfo::enableSaveBattleRecord() {
    save_battle_record_ = 1;
    return this;
}

// IDA_EA: 0x869620E
// IDA_NAME: _ZN15CUserCharacInfo19getBattleRecordRefWEv
// DEMANGLED: CUserCharacInfo::getBattleRecordRefW(void)
std::map<int, GameResultType>* CUserCharacInfo::getBattleRecordRefW() {
    enableSaveBattleRecord();
    return &battle_records_;
}

// IDA_EA: 0x84B42E8
// IDA_NAME: _ZN15CUserCharacInfo22enableSaveDungeonClearEv
// DEMANGLED: CUserCharacInfo::enableSaveDungeonClear(void)
CUserCharacInfo* CUserCharacInfo::enableSaveDungeonClear() {
    save_dungeon_clear_ = 1;
    return this;
}

// IDA_EA: 0x85BFAB2
// IDA_NAME: _ZNK15CUserCharacInfo19getDungeonClearRefREv
// DEMANGLED: CUserCharacInfo::getDungeonClearRefR(void)const
void* CUserCharacInfo::getDungeonClearRefR() const {
    return const_cast<WongWork::CDungeonClear*>(&dungeon_clear_);
}

// IDA_EA: 0x84B42CC
// IDA_NAME: _ZN15CUserCharacInfo19getDungeonClearRefWEv
// DEMANGLED: CUserCharacInfo::getDungeonClearRefW(void)
WongWork::CDungeonClear* CUserCharacInfo::getDungeonClearRefW() {
    enableSaveDungeonClear();
    return &dungeon_clear_;
}

// IDA_EA: 0x84EBF18
// IDA_NAME: _ZN15CUserCharacInfo16disableSaveInvenEv
// DEMANGLED: CUserCharacInfo::disableSaveInven(void)
CUserCharacInfo* CUserCharacInfo::disableSaveInven() {
    save_inventory_ = 0;
    return this;
}

// IDA_EA: 0x84EBF24
// IDA_NAME: _ZN15CUserCharacInfo16disableSaveSkillEv
// DEMANGLED: CUserCharacInfo::disableSaveSkill(void)
CUserCharacInfo* CUserCharacInfo::disableSaveSkill() {
    save_skill_ = 0;
    return this;
}

// IDA_EA: 0x84EBF30
// IDA_NAME: _ZN15CUserCharacInfo14disableSavePvPEv
// DEMANGLED: CUserCharacInfo::disableSavePvP(void)
CUserCharacInfo* CUserCharacInfo::disableSavePvP() {
    save_pvp_ = 0;
    return this;
}

// IDA_EA: 0x84EBF3C
// IDA_NAME: _ZN15CUserCharacInfo23disableSaveDungeonClearEv
// DEMANGLED: CUserCharacInfo::disableSaveDungeonClear(void)
CUserCharacInfo* CUserCharacInfo::disableSaveDungeonClear() {
    save_dungeon_clear_ = 0;
    return this;
}

// IDA_EA: 0x84EBF48
// IDA_NAME: _ZN15CUserCharacInfo16disableSaveCargoEv
// DEMANGLED: CUserCharacInfo::disableSaveCargo(void)
CUserCharacInfo* CUserCharacInfo::disableSaveCargo() {
    save_cargo_ = 0;
    return this;
}

// IDA_EA: 0x84EBFE6
// IDA_NAME: _ZN15CUserCharacInfo25setUserEventCharacterFlagEh
// DEMANGLED: CUserCharacInfo::setUserEventCharacterFlag(unsigned char)
CUserCharacInfo* CUserCharacInfo::setUserEventCharacterFlag(uint8_t flag) {
    user_event_character_flag_ = flag;
    return this;
}

// IDA_EA: 0x869622A
// IDA_NAME: _ZN15CUserCharacInfo21disableSaveCharacInfoEv
// DEMANGLED: CUserCharacInfo::disableSaveCharacInfo(void)
CUserCharacInfo* CUserCharacInfo::disableSaveCharacInfo() {
    save_charac_info_ = 0;
    return this;
}

// IDA_EA: 0x8696236
// IDA_NAME: _ZN15CUserCharacInfo21disableSaveCharacStatEv
// DEMANGLED: CUserCharacInfo::disableSaveCharacStat(void)
CUserCharacInfo* CUserCharacInfo::disableSaveCharacStat() {
    save_charac_stat_ = 0;
    return this;
}

// IDA_EA: 0x8696242
// IDA_NAME: _ZN15CUserCharacInfo20disableSaveCharacEtcEv
// DEMANGLED: CUserCharacInfo::disableSaveCharacEtc(void)
CUserCharacInfo* CUserCharacInfo::disableSaveCharacEtc() {
    save_charac_etc_ = 0;
    return this;
}

// IDA_EA: 0x869624E
// IDA_NAME: _ZN15CUserCharacInfo27disableSaveCharacManageInfoEv
// DEMANGLED: CUserCharacInfo::disableSaveCharacManageInfo(void)
CUserCharacInfo* CUserCharacInfo::disableSaveCharacManageInfo() {
    save_charac_manage_info_ = 0;
    return this;
}

// IDA_EA: 0x869625A
// IDA_NAME: _ZN15CUserCharacInfo16disableSaveQuestEv
// DEMANGLED: CUserCharacInfo::disableSaveQuest(void)
CUserCharacInfo* CUserCharacInfo::disableSaveQuest() {
    save_quest_ = 0;
    return this;
}

// IDA_EA: 0x8696266
// IDA_NAME: _ZN15CUserCharacInfo23disableSaveBattleRecordEv
// DEMANGLED: CUserCharacInfo::disableSaveBattleRecord(void)
CUserCharacInfo* CUserCharacInfo::disableSaveBattleRecord() {
    save_battle_record_ = 0;
    return this;
}

// IDA_EA: 0x869627E
// IDA_NAME: _ZN15CUserCharacInfo16isSaveCharacInfoEv
// DEMANGLED: CUserCharacInfo::isSaveCharacInfo(void)
int CUserCharacInfo::isSaveCharacInfo() {
    return save_charac_info_;
}

// IDA_EA: 0x869628A
// IDA_NAME: _ZN15CUserCharacInfo16IsSaveCharacStatEv
// DEMANGLED: CUserCharacInfo::IsSaveCharacStat(void)
int CUserCharacInfo::IsSaveCharacStat() {
    return save_charac_stat_;
}

// IDA_EA: 0x8696296
// IDA_NAME: _ZN15CUserCharacInfo15isSaveCharacEtcEv
// DEMANGLED: CUserCharacInfo::isSaveCharacEtc(void)
int CUserCharacInfo::isSaveCharacEtc() {
    return save_charac_etc_;
}

// IDA_EA: 0x86962A2
// IDA_NAME: _ZN15CUserCharacInfo22IsSaveCharacManageInfoEv
// DEMANGLED: CUserCharacInfo::IsSaveCharacManageInfo(void)
int CUserCharacInfo::IsSaveCharacManageInfo() {
    return save_charac_manage_info_;
}

// IDA_EA: 0x86962AE
// IDA_NAME: _ZN15CUserCharacInfo11isSaveSkillEv
// DEMANGLED: CUserCharacInfo::isSaveSkill(void)
int CUserCharacInfo::isSaveSkill() {
    return save_skill_;
}

// IDA_EA: 0x86962BA
// IDA_NAME: _ZN15CUserCharacInfo9isSavePvPEv
// DEMANGLED: CUserCharacInfo::isSavePvP(void)
int CUserCharacInfo::isSavePvP() {
    return save_pvp_;
}

// IDA_EA: 0x86962C6
// IDA_NAME: _ZN15CUserCharacInfo11isSaveQuestEv
// DEMANGLED: CUserCharacInfo::isSaveQuest(void)
int CUserCharacInfo::isSaveQuest() {
    return save_quest_;
}

// IDA_EA: 0x86962D2
// IDA_NAME: _ZN15CUserCharacInfo18isSaveDungeonClearEv
// DEMANGLED: CUserCharacInfo::isSaveDungeonClear(void)
int CUserCharacInfo::isSaveDungeonClear() {
    return save_dungeon_clear_;
}

// IDA_EA: 0x86962DE
// IDA_NAME: _ZN15CUserCharacInfo18isSaveBattleRecordEv
// DEMANGLED: CUserCharacInfo::isSaveBattleRecord(void)
int CUserCharacInfo::isSaveBattleRecord() {
    return save_battle_record_;
}

// IDA_EA: 0x86962EA
// IDA_NAME: _ZN15CUserCharacInfo11isSaveCargoEv
// DEMANGLED: CUserCharacInfo::isSaveCargo(void)
int CUserCharacInfo::isSaveCargo() {
    return save_cargo_;
}

// IDA_EA: 0x86962F6
// IDA_NAME: _ZN15CUserCharacInfo18isSaveCharacOptionEv
// DEMANGLED: CUserCharacInfo::isSaveCharacOption(void)
int CUserCharacInfo::isSaveCharacOption() {
    return save_charac_option_;
}

// IDA_EA: 0x8696306
// IDA_NAME: _ZNK15CUserCharacInfo10GetFatigueEv
// DEMANGLED: CUserCharacInfo::GetFatigue(void)const
int CUserCharacInfo::GetFatigue() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.fatigue_;
    }
    return 0;
}

// IDA_EA: 0x8696326
// IDA_NAME: _ZN15CUserCharacInfo30decCurCharacFatigueGrownUpBuffEj
// DEMANGLED: CUserCharacInfo::decCurCharacFatigueGrownUpBuff(unsigned int)
int CUserCharacInfo::decCurCharacFatigueGrownUpBuff(uint32_t value) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        const uint16_t buff = cur->fields_.fatigue_grownup_buff_;
        if (buff <= value) {
            cur->fields_.fatigue_grownup_buff_ = 0;
        } else {
            cur->fields_.fatigue_grownup_buff_ =
                static_cast<uint16_t>(buff - value);
        }
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696386
// IDA_NAME: _ZN15CUserCharacInfo29resetCharacFatigueGrownUpBuffEv
// DEMANGLED: CUserCharacInfo::resetCharacFatigueGrownUpBuff(void)
int CUserCharacInfo::resetCharacFatigueGrownUpBuff() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.fatigue_grownup_buff_ =
            0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84A852C
// IDA_NAME: _ZNK15CUserCharacInfo30getCurCharacFatigueGrownUpBuffEv
// DEMANGLED: CUserCharacInfo::getCurCharacFatigueGrownUpBuff(void)const
int CUserCharacInfo::getCurCharacFatigueGrownUpBuff() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.fatigue_grownup_buff_;
    }
    return 0;
}

// IDA_EA: 0x84A854C
// IDA_NAME: _ZN15CUserCharacInfo30setCurCharacFatigueGrownUpBuffEt
// DEMANGLED: CUserCharacInfo::setCurCharacFatigueGrownUpBuff(unsigned short)
int CUserCharacInfo::setCurCharacFatigueGrownUpBuff(uint16_t fatigue) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.fatigue_grownup_buff_ =
            fatigue;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84A857E
// IDA_NAME: _ZN15CUserCharacInfo25GetFatigueBatteryChargingEv
// DEMANGLED: CUserCharacInfo::GetFatigueBatteryCharging(void)
int CUserCharacInfo::GetFatigueBatteryCharging() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.fatigue_battery_charging_;
    }
    return 0;
}

// IDA_EA: 0x86566E4
// IDA_NAME: _ZN15CUserCharacInfo25IncFatigueBatteryChargingEs
// DEMANGLED: CUserCharacInfo::IncFatigueBatteryCharging(short)
int CUserCharacInfo::IncFatigueBatteryCharging(int16_t fatigue) {
    _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }

    enableSaveCharacStat();
    const int16_t previous = cur->fields_.fatigue_battery_charging_;
    int16_t charging = static_cast<int16_t>(previous + fatigue);

    const uint16_t maximum =
        G_CDataManager()
            ->get_server_parameter_script()
            ->get_fatigue_battery_constant_table()
            .limit_fatigue_battery_charging_amount_;
    if (charging < previous || charging > static_cast<int>(maximum)) {
        charging = static_cast<int16_t>(maximum);
    }
    cur->fields_.fatigue_battery_charging_ = charging;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8656784
// IDA_NAME: _ZN15CUserCharacInfo25DecFatigueBatteryChargingEs
// DEMANGLED: CUserCharacInfo::DecFatigueBatteryCharging(short)
int CUserCharacInfo::DecFatigueBatteryCharging(int16_t fatigue) {
    _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }

    enableSaveCharacStat();
    const int16_t previous = cur->fields_.fatigue_battery_charging_;
    int16_t charging = static_cast<int16_t>(previous - fatigue);
    if (charging < 0 || charging > previous) {
        cur->fields_.fatigue_battery_charging_ = 0;
        return reinterpret_cast<int>(cur);
    }
    cur->fields_.fatigue_battery_charging_ = charging;
    return static_cast<uint16_t>(charging);
}

// IDA_EA: 0x84EBF54
// IDA_NAME: _ZN15CUserCharacInfo30incCurCharacMemberBonusFatigueEs
// DEMANGLED: CUserCharacInfo::incCurCharacMemberBonusFatigue(short)
int CUserCharacInfo::incCurCharacMemberBonusFatigue(int16_t fatigue) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    cur->fields_.member_bonus_fatigue_ +=
        fatigue;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EBF90
// IDA_NAME: _ZN15CUserCharacInfo32resetCurCharacMemberBonusFatigueEv
// DEMANGLED: CUserCharacInfo::resetCurCharacMemberBonusFatigue(void)
int CUserCharacInfo::resetCurCharacMemberBonusFatigue() {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    cur->fields_.member_bonus_fatigue_ = 0;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EBFB2
// IDA_NAME: _ZN15CUserCharacInfo30getCurCharacMemberBonusFatigueEv
// DEMANGLED: CUserCharacInfo::getCurCharacMemberBonusFatigue(void)
int CUserCharacInfo::getCurCharacMemberBonusFatigue() {
    _Charac_info* const cur = current_charac_;
    return cur->fields_.member_bonus_fatigue_;
}

// IDA_EA: 0x8696664
// IDA_NAME: _ZN15CUserCharacInfo19addCurcharacStaminaEh
// DEMANGLED: CUserCharacInfo::addCurcharacStamina(unsigned char)
int CUserCharacInfo::addCurcharacStamina(uint8_t value) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    cur->fields_.stamina_ =
        static_cast<unsigned char>(cur->fields_.stamina_ + value);
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696694
// IDA_NAME: _ZNK15CUserCharacInfo26get_charac_party_bonus_expEv
// DEMANGLED: CUserCharacInfo::get_charac_party_bonus_exp(void)const
int CUserCharacInfo::get_charac_party_bonus_exp() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.party_bonus_experience_;
    }
    return 0;
}

// IDA_EA: 0x8696738
// IDA_NAME: _ZN15CUserCharacInfo17setCurCharacLevelEi
// DEMANGLED: CUserCharacInfo::setCurCharacLevel(int)
int CUserCharacInfo::setCurCharacLevel(int level) {
    enableSaveCharacInfo();
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.level_ =
            static_cast<int16_t>(level);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696762
// IDA_NAME: _ZN15CUserCharacInfo17incCurCharacLevelEv
// DEMANGLED: CUserCharacInfo::incCurCharacLevel(void)
int CUserCharacInfo::incCurCharacLevel() {
    enableSaveCharacInfo();
    _Charac_info* const cur = current_charac_;
    if (cur) {
        ++cur->fields_.level_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696790
// IDA_NAME: _ZN15CUserCharacInfo17decCurCharacLevelEv
// DEMANGLED: CUserCharacInfo::decCurCharacLevel(void)
int CUserCharacInfo::decCurCharacLevel() {
    enableSaveCharacInfo();
    _Charac_info* const cur = current_charac_;
    if (cur) {
        --cur->fields_.level_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x86967BE
// IDA_NAME: _ZN15CUserCharacInfo15addCurCharacExpEi
// DEMANGLED: CUserCharacInfo::addCurCharacExp(int)
int CUserCharacInfo::addCurCharacExp(int exp) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.experience_ += exp;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x86967F0
// IDA_NAME: _ZN15CUserCharacInfo20subtractCurCharacExpEi
// DEMANGLED: CUserCharacInfo::subtractCurCharacExp(int)
int CUserCharacInfo::subtractCurCharacExp(int exp) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.experience_ -= exp;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84699E6
// IDA_NAME: _ZN15CUserCharacInfo26set_charac_party_bonus_expEi
// DEMANGLED: CUserCharacInfo::set_charac_party_bonus_exp(int)
int CUserCharacInfo::set_charac_party_bonus_exp(int exp) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.party_bonus_experience_ = exp;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8469A02
// IDA_NAME: _ZN15CUserCharacInfo32set_charac_fatigue_buf_bonus_expEi
// DEMANGLED: CUserCharacInfo::set_charac_fatigue_buf_bonus_exp(int)
int CUserCharacInfo::set_charac_fatigue_buf_bonus_exp(int exp) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.fatigue_buff_bonus_experience_ = exp;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8469A20
// IDA_NAME: _ZN15CUserCharacInfo27set_charac_member_bonus_expEi
// DEMANGLED: CUserCharacInfo::set_charac_member_bonus_exp(int)
int CUserCharacInfo::set_charac_member_bonus_exp(int exp) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.member_bonus_experience_ = exp;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8469A3E
// IDA_NAME: _ZN15CUserCharacInfo30set_charac_seria_buf_bonus_expEi
// DEMANGLED: CUserCharacInfo::set_charac_seria_buf_bonus_exp(int)
int CUserCharacInfo::set_charac_seria_buf_bonus_exp(int exp) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.seria_buff_bonus_experience_ = exp;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC05C
// IDA_NAME: _ZNK15CUserCharacInfo14get_charac_expEv
// DEMANGLED: CUserCharacInfo::get_charac_exp(void)const
int CUserCharacInfo::get_charac_exp() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.experience_;
    }
    return 0;
}

// IDA_EA: 0x84EC4C0
// IDA_NAME: _ZN15CUserCharacInfo22set_aura_avatar_optionER18stAuraAvatarOption
// DEMANGLED: CUserCharacInfo::set_aura_avatar_option(stAuraAvatarOption &)
int CUserCharacInfo::set_aura_avatar_option(stAuraAvatarOption& option) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        cur->fields_.aura_avatar_option_ = option;
    }
    return result;
}

// IDA_EA: 0x84EC4F2
// IDA_NAME: _ZN15CUserCharacInfo15setCharacOptionER15CharacterOption
// DEMANGLED: CUserCharacInfo::setCharacOption(CharacterOption &)
CharacterOption* CUserCharacInfo::setCharacOption(CharacterOption& option) {
    _Charac_info* const cur = current_charac_;
    CharacterOption* result = reinterpret_cast<CharacterOption*>(cur);
    if (cur) {
        result = &cur->fields_.character_option_;
        *result = option;
    }
    return result;
}

// IDA_EA: 0x8690ED8
// IDA_NAME: _ZN15CUserCharacInfo25resetBlueMarbleEnterCountEv
// DEMANGLED: CUserCharacInfo::resetBlueMarbleEnterCount(void)
int CUserCharacInfo::resetBlueMarbleEnterCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.blue_marble_enter_count_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8690EF4
// IDA_NAME: _ZN15CUserCharacInfo23setBlueMarbleEnterCountEh
// DEMANGLED: CUserCharacInfo::setBlueMarbleEnterCount(unsigned char)
int CUserCharacInfo::setBlueMarbleEnterCount(uint8_t count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.blue_marble_enter_count_ = count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8690F1C
// IDA_NAME: _ZN15CUserCharacInfo28increaseBlueMarbleEnterCountEv
// DEMANGLED: CUserCharacInfo::increaseBlueMarbleEnterCount(void)
int CUserCharacInfo::increaseBlueMarbleEnterCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        ++cur->fields_.blue_marble_enter_count_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8690F42
// IDA_NAME: _ZNK15CUserCharacInfo23getBlueMarbleEnterCountEv
// DEMANGLED: CUserCharacInfo::getBlueMarbleEnterCount(void)const
int CUserCharacInfo::getBlueMarbleEnterCount() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.blue_marble_enter_count_;
    }
    return 0;
}

// IDA_EA: 0x8690F66
// IDA_NAME: _ZN15CUserCharacInfo28resetBlueMarbleUserDeadCountEv
// DEMANGLED: CUserCharacInfo::resetBlueMarbleUserDeadCount(void)
int CUserCharacInfo::resetBlueMarbleUserDeadCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.blue_marble_dead_count_ =
            0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8690F86
// IDA_NAME: _ZN15CUserCharacInfo31increaseBlueMarbleUserDeadCountEv
// DEMANGLED: CUserCharacInfo::increaseBlueMarbleUserDeadCount(void)
int CUserCharacInfo::increaseBlueMarbleUserDeadCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        ++cur->fields_.blue_marble_dead_count_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8690FAA
// IDA_NAME: _ZNK15CUserCharacInfo26getBlueMarbleUserDeadCountEv
// DEMANGLED: CUserCharacInfo::getBlueMarbleUserDeadCount(void)const
int CUserCharacInfo::getBlueMarbleUserDeadCount() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.blue_marble_dead_count_;
    }
    return 0;
}

// IDA_EA: 0x86963B0
// IDA_NAME: _ZNK15CUserCharacInfo26getCurCharacPremiumFatigueEv
// DEMANGLED: CUserCharacInfo::getCurCharacPremiumFatigue(void)const
int CUserCharacInfo::getCurCharacPremiumFatigue() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.premium_fatigue_;
    }
    return 0;
}

// IDA_EA: 0x86963D0
// IDA_NAME: _ZN15CUserCharacInfo26setCurCharacPremiumFatigueEt
// DEMANGLED: CUserCharacInfo::setCurCharacPremiumFatigue(unsigned short)
int CUserCharacInfo::setCurCharacPremiumFatigue(uint16_t fatigue) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.premium_fatigue_ =
            fatigue;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696402
// IDA_NAME: _ZNK15CUserCharacInfo29getCurCharacMaxPremiumFatigueEv
// DEMANGLED: CUserCharacInfo::getCurCharacMaxPremiumFatigue(void)const
int CUserCharacInfo::getCurCharacMaxPremiumFatigue() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.maximum_premium_fatigue_;
    }
    return 0;
}

// IDA_EA: 0x8696422
// IDA_NAME: _ZN15CUserCharacInfo23incCurCharacUsedFatigueEt
// DEMANGLED: CUserCharacInfo::incCurCharacUsedFatigue(unsigned short)
int CUserCharacInfo::incCurCharacUsedFatigue(uint16_t fatigue) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.used_fatigue_ +=
            fatigue;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x869645E
// IDA_NAME: _ZN15CUserCharacInfo25resetCurCharacUsedFatigueEv
// DEMANGLED: CUserCharacInfo::resetCurCharacUsedFatigue(void)
int CUserCharacInfo::resetCurCharacUsedFatigue() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.used_fatigue_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696488
// IDA_NAME: _ZN15CUserCharacInfo28ResetCurCharacEventItemCountEv
// DEMANGLED: CUserCharacInfo::ResetCurCharacEventItemCount(void)
int CUserCharacInfo::ResetCurCharacEventItemCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.event_item_count_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x86964A6
// IDA_NAME: _ZN15CUserCharacInfo26GetCurCharacEventItemCountEv
// DEMANGLED: CUserCharacInfo::GetCurCharacEventItemCount(void)
int CUserCharacInfo::GetCurCharacEventItemCount() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.event_item_count_;
    }
    return 0;
}

// IDA_EA: 0x86964CA
// IDA_NAME: _ZN15CUserCharacInfo31ResetCurCharacEventAddItemCountEv
// DEMANGLED: CUserCharacInfo::ResetCurCharacEventAddItemCount(void)
int CUserCharacInfo::ResetCurCharacEventAddItemCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.event_add_item_count_ =
            0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x86964E8
// IDA_NAME: _ZN15CUserCharacInfo29GetCurCharacEventAddItemCountEv
// DEMANGLED: CUserCharacInfo::GetCurCharacEventAddItemCount(void)
int CUserCharacInfo::GetCurCharacEventAddItemCount() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.event_add_item_count_;
    }
    return 0;
}

// IDA_EA: 0x869650C
// IDA_NAME: _ZN15CUserCharacInfo25resetCurCharacSchoolPointEv
// DEMANGLED: CUserCharacInfo::resetCurCharacSchoolPoint(void)
int CUserCharacInfo::resetCurCharacSchoolPoint() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.school_point_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696528
// IDA_NAME: _ZN15CUserCharacInfo23getCurCharacSchoolPointEv
// DEMANGLED: CUserCharacInfo::getCurCharacSchoolPoint(void)
int CUserCharacInfo::getCurCharacSchoolPoint() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.school_point_;
    }
    return 0;
}

// IDA_EA: 0x8696548
// IDA_NAME: _ZNK15CUserCharacInfo33getCurCharacDungeonHelpAbuseRatioEv
// DEMANGLED: CUserCharacInfo::getCurCharacDungeonHelpAbuseRatio(void)const
int CUserCharacInfo::getCurCharacDungeonHelpAbuseRatio() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.help_abuse_ratio_;
    }
    return 0;
}

// IDA_EA: 0x869656A
// IDA_NAME: _ZNK15CUserCharacInfo31getCurCharacDungeonHelpAbuseExpEv
// DEMANGLED: CUserCharacInfo::getCurCharacDungeonHelpAbuseExp(void)const
int CUserCharacInfo::getCurCharacDungeonHelpAbuseExp() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.help_abuse_experience_;
    }
    return 0;
}

// IDA_EA: 0x864E3BA
// IDA_NAME: _ZN15CUserCharacInfo16get_level_up_expEi
// DEMANGLED: CUserCharacInfo::get_level_up_exp(int)
int CUserCharacInfo::get_level_up_exp(int level) {
    CDataManager* data_manager = G_CDataManager();
    const int next_exp = data_manager->get_level_exp(level + 1);
    return next_exp - data_manager->get_level_exp(level);
}

// IDA_EA: 0x8658A4E
// IDA_NAME: _ZN15CUserCharacInfo26calc_disp_help_abuse_ratioEi
// DEMANGLED: CUserCharacInfo::calc_disp_help_abuse_ratio(int)
int CUserCharacInfo::calc_disp_help_abuse_ratio(int level_delta) {
    const int charac_level = get_charac_level();
    const int level_up_exp = get_level_up_exp(charac_level);
    const long double help_exp =
        static_cast<unsigned int>(getCurCharacDungeonHelpAbuseExp());
    const long double exp_ratio =
        help_exp / static_cast<long double>(level_up_exp);
    const int total_ratio =
        static_cast<int>(
            static_cast<unsigned int>(
                getCurCharacDungeonHelpAbuseRatio()) +
            exp_ratio * 1000000.0L);

    int display_ratio = 0;
    if (level_delta + get_charac_level() > 0) {
        display_ratio =
            total_ratio /
            (get_charac_level() + level_delta);
    }

    const int max_ratio = 1000000;
    return std::min(max_ratio, display_ratio);
}

// IDA_EA: 0x84EC13E
// IDA_NAME: _ZN15CUserCharacInfo27needCurCharacCheckOverEquipEv
// DEMANGLED: CUserCharacInfo::needCurCharacCheckOverEquip(void)
int CUserCharacInfo::needCurCharacCheckOverEquip() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.need_check_over_equip_item_;
    }
    return 0;
}

// IDA_EA: 0x84EC162
// IDA_NAME: _ZN15CUserCharacInfo27resetCurCharacOverEquipFlagEv
// DEMANGLED: CUserCharacInfo::resetCurCharacOverEquipFlag(void)
int CUserCharacInfo::resetCurCharacOverEquipFlag() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.need_check_over_equip_item_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x850D494
// IDA_NAME: _ZNK15CUserCharacInfo34getCurCharacHelpAbuseComputedRatioEv
// DEMANGLED: CUserCharacInfo::getCurCharacHelpAbuseComputedRatio(void)const
int CUserCharacInfo::getCurCharacHelpAbuseComputedRatio() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.help_abuse_computed_ratio_;
    }
    return 0;
}

// IDA_EA: 0x869658C
// IDA_NAME: _ZN15CUserCharacInfo32getCurCharacEscaladeTutorialFlagEv
// DEMANGLED: CUserCharacInfo::getCurCharacEscaladeTutorialFlag(void)
const uint8_t* CUserCharacInfo::getCurCharacEscaladeTutorialFlag() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.escalade_tutorial_flag_;
    }
    return 0;
}

// IDA_EA: 0x864751E
// IDA_NAME: _ZN15CUserCharacInfo32getCurCharacEscaladeTutorialFlagEj
// DEMANGLED: CUserCharacInfo::getCurCharacEscaladeTutorialFlag(unsigned int)
bool CUserCharacInfo::getCurCharacEscaladeTutorialFlag(uint32_t flag) {
    const _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }
    if (flag <= 0x1Fu) {
        return (cur->fields_.tutorial_flag_ &
                (1u << flag)) != 0;
    }
    if (flag > 0x7Fu) {
        return 0;
    }
    const uint32_t index = (flag - 32u) / 8u;
    if (index < 0x10u) {
        return (cur->fields_.escalade_tutorial_flag_[index] >>
                ((flag - 32u) % 8u)) &
               1u;
    }
    return 0;
}

// IDA_EA: 0x86965AC
// IDA_NAME: _ZN15CUserCharacInfo26getCurCharacTradeGoldTotalEv
// DEMANGLED: CUserCharacInfo::getCurCharacTradeGoldTotal(void)
int CUserCharacInfo::getCurCharacTradeGoldTotal() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.reliable_person_.GetTradeGoldTotal();
    }
    return 0;
}

// IDA_EA: 0x86965D6
// IDA_NAME: _ZN15CUserCharacInfo33getCurCharacTradeGoldTotalBillionEv
// DEMANGLED: CUserCharacInfo::getCurCharacTradeGoldTotalBillion(void)
int CUserCharacInfo::getCurCharacTradeGoldTotalBillion() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.reliable_person_.GetTradeGoldTotalBillion();
    }
    return 0;
}

// IDA_EA: 0x8696600
// IDA_NAME: _ZN15CUserCharacInfo26getCurCharacTradeGoldDailyEv
// DEMANGLED: CUserCharacInfo::getCurCharacTradeGoldDaily(void)
int CUserCharacInfo::getCurCharacTradeGoldDaily() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.reliable_person_.GetTradeGoldDaily();
    }
    return 0;
}

// IDA_EA: 0x869662A
// IDA_NAME: _ZN15CUserCharacInfo32IncrementCurCharacTradeGoldDailyEj
// DEMANGLED: CUserCharacInfo::IncrementCurCharacTradeGoldDaily(unsigned int)
CReliablePerson* CUserCharacInfo::IncrementCurCharacTradeGoldDaily(
    uint32_t gold) {
    enableSaveCharacStat();
    _Charac_info* const cur = current_charac_;
    return cur->fields_.reliable_person_.IncrementTradeGold(gold);
}

// IDA_EA: 0x8696656
// IDA_NAME: _ZN15CUserCharacInfo21GetOverTradeGoldCountEv
// DEMANGLED: CUserCharacInfo::GetOverTradeGoldCount(void)
int CUserCharacInfo::GetOverTradeGoldCount() {
    return over_trade_gold_count_;
}

// IDA_EA: 0x8696822
// IDA_NAME: _ZNK15CUserCharacInfo21getRequestRenameStateEv
// DEMANGLED: CUserCharacInfo::getRequestRenameState(void)const
int CUserCharacInfo::getRequestRenameState() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.character_view_state_;
    }
    return 0;
}

// IDA_EA: 0x8696844
// IDA_NAME: _ZN15CUserCharacInfo19get_guild_today_expEv
// DEMANGLED: CUserCharacInfo::get_guild_today_exp(void)
int CUserCharacInfo::get_guild_today_exp() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.guild_today_experience_;
    }
    return 0;
}

// IDA_EA: 0x8645CE4
// IDA_NAME: _ZN15CUserCharacInfo19add_guild_today_expEi
// DEMANGLED: CUserCharacInfo::add_guild_today_exp(int)
int CUserCharacInfo::add_guild_today_exp(int exp) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        result = cur->fields_.guild_id_;
        if (result) {
            result = reinterpret_cast<int>(cur);
            cur->fields_.guild_today_experience_ += exp;
        }
    }
    return result;
}

// IDA_EA: 0x8645D22
// IDA_NAME: _ZN15CUserCharacInfo21reset_guild_today_expEv
// DEMANGLED: CUserCharacInfo::reset_guild_today_exp(void)
int CUserCharacInfo::reset_guild_today_exp() {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        result = cur->fields_.guild_id_;
        if (result) {
            result = reinterpret_cast<int>(cur);
            cur->fields_.guild_today_experience_ = 0;
        }
    }
    return result;
}

// IDA_EA: 0x8645B80
// IDA_NAME: _ZN15CUserCharacInfo25set_uppermember_exp_levelEh
// DEMANGLED: CUserCharacInfo::set_uppermember_exp_level(unsigned char)
int CUserCharacInfo::set_uppermember_exp_level(uint8_t level) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        unsigned char& current = cur->fields_.upper_member_experience_level_;
        if (level && current > level) {
            LogManager::logFormat(
                1,
                "user.cpp",
                "void CUserCharacInfo::set_uppermember_exp_level(unsigned char)",
                0x2D2,
                "set member exp level error , name(%s) , level(%d), new level(%d)",
                cur->fields_.character_name_,
                current,
                level);
        } else {
            current = level;
        }
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696866
// IDA_NAME: _ZN15CUserCharacInfo25get_uppermember_exp_levelEv
// DEMANGLED: CUserCharacInfo::get_uppermember_exp_level(void)
int CUserCharacInfo::get_uppermember_exp_level() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.upper_member_experience_level_;
    }
    return 0;
}

// IDA_EA: 0x869688A
// IDA_NAME: _ZNK15CUserCharacInfo32get_charac_fatigue_buf_bonus_expEv
// DEMANGLED: CUserCharacInfo::get_charac_fatigue_buf_bonus_exp(void)const
int CUserCharacInfo::get_charac_fatigue_buf_bonus_exp() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.fatigue_buff_bonus_experience_;
    }
    return 0;
}

// IDA_EA: 0x86968AC
// IDA_NAME: _ZNK15CUserCharacInfo27get_charac_member_bonus_expEv
// DEMANGLED: CUserCharacInfo::get_charac_member_bonus_exp(void)const
int CUserCharacInfo::get_charac_member_bonus_exp() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.member_bonus_experience_;
    }
    return 0;
}

// IDA_EA: 0x86968CE
// IDA_NAME: _ZN15CUserCharacInfo33add_member_pay_tex_money_to_upperEi
// DEMANGLED: CUserCharacInfo::add_member_pay_tex_money_to_upper(int)
int CUserCharacInfo::add_member_pay_tex_money_to_upper(int value) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.tax_money_to_upper_ += value;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x86968F8
// IDA_NAME: _ZN15CUserCharacInfo35add_member_pay_tex_fatigue_to_upperEi
// DEMANGLED: CUserCharacInfo::add_member_pay_tex_fatigue_to_upper(int)
int CUserCharacInfo::add_member_pay_tex_fatigue_to_upper(int value) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.tax_fatigue_to_upper_ +=
            value;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696922
// IDA_NAME: _ZN15CUserCharacInfo22GetCharacStartPlayTickEv
// DEMANGLED: CUserCharacInfo::GetCharacStartPlayTick(void)
int CUserCharacInfo::GetCharacStartPlayTick() {
    return lifecycle_tick_;
}

// IDA_EA: 0x8645538
// IDA_NAME: _ZN15CUserCharacInfo17GetCharacPlayTickEv
// DEMANGLED: CUserCharacInfo::GetCharacPlayTick(void)
int CUserCharacInfo::GetCharacPlayTick() {
    const int cur_sec = GlobalData::system_time()->getCurSec();
    return cur_sec - GetCharacStartPlayTick();
}

// IDA_EA: 0x8696930
// IDA_NAME: _ZN15CUserCharacInfo20GetCurCharacChaosExpEv
// DEMANGLED: CUserCharacInfo::GetCurCharacChaosExp(void)
int CUserCharacInfo::GetCurCharacChaosExp() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.chaos_experience_;
    }
    return 0;
}

// IDA_EA: 0x8696952
// IDA_NAME: _ZN15CUserCharacInfo19GetCurCharacEmotionEv
// DEMANGLED: CUserCharacInfo::GetCurCharacEmotion(void)
int CUserCharacInfo::GetCurCharacEmotion() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.emotion_;
    }
    return 0;
}

// IDA_EA: 0x8696976
// IDA_NAME: _ZNK15CUserCharacInfo23IsVillageAttackDBUpdateEv
// DEMANGLED: CUserCharacInfo::IsVillageAttackDBUpdate(void)const
int CUserCharacInfo::IsVillageAttackDBUpdate() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.village_attack_update_flag_;
    }
    return 0;
}

// IDA_EA: 0x869699A
// IDA_NAME: _ZN15CUserCharacInfo31SetCurCharacAssaultFatigueCountEi
// DEMANGLED: CUserCharacInfo::SetCurCharacAssaultFatigueCount(int)
int CUserCharacInfo::SetCurCharacAssaultFatigueCount(int count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.assault_fatigue_count_ = count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x86969B8
// IDA_NAME: _ZN15CUserCharacInfo31GetCurCharacAssaultFatigueCountEv
// DEMANGLED: CUserCharacInfo::GetCurCharacAssaultFatigueCount(void)
int CUserCharacInfo::GetCurCharacAssaultFatigueCount() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.assault_fatigue_count_;
    }
    return 0;
}

// IDA_EA: 0x86969DA
// IDA_NAME: _ZN15CUserCharacInfo31IncCurCharacAssaultFatigueCountEv
// DEMANGLED: CUserCharacInfo::IncCurCharacAssaultFatigueCount(void)
int CUserCharacInfo::IncCurCharacAssaultFatigueCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        ++cur->fields_.assault_fatigue_count_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x85BFC5A
// IDA_NAME: _ZN15CUserCharacInfo28SetCurCharacDungeonPlayCountEi
// DEMANGLED: CUserCharacInfo::SetCurCharacDungeonPlayCount(int)
int CUserCharacInfo::SetCurCharacDungeonPlayCount(int count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.dungeon_play_count_ = count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x85BFC78
// IDA_NAME: _ZNK15CUserCharacInfo28GetCurCharacDungeonPlayCountEv
// DEMANGLED: CUserCharacInfo::GetCurCharacDungeonPlayCount(void)const
int CUserCharacInfo::GetCurCharacDungeonPlayCount() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.dungeon_play_count_;
    }
    return 0;
}

// IDA_EA: 0x85BFC9A
// IDA_NAME: _ZN15CUserCharacInfo28IncCurCharacDungeonPlayCountEv
// DEMANGLED: CUserCharacInfo::IncCurCharacDungeonPlayCount(void)
int CUserCharacInfo::IncCurCharacDungeonPlayCount() {
    enableSaveCharacStat();
    const int count = GetCurCharacDungeonPlayCount();
    return SetCurCharacDungeonPlayCount(count + 1);
}

// IDA_EA: 0x8335BA8
// IDA_NAME: _ZNK15CUserCharacInfo24GetCurConditionEventStepEv
// DEMANGLED: CUserCharacInfo::GetCurConditionEventStep(void)const
int CUserCharacInfo::GetCurConditionEventStep() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.condition_event_step_;
    }
    return 0;
}

// IDA_EA: 0x84EC256
// IDA_NAME: _ZN15CUserCharacInfo24SetCurConditionEventStepEs
// DEMANGLED: CUserCharacInfo::SetCurConditionEventStep(short)
int CUserCharacInfo::SetCurConditionEventStep(int16_t step) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.condition_event_step_ =
            step;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC23A
// IDA_NAME: _ZN15CUserCharacInfo26ClearCurConditionEventStepEv
// DEMANGLED: CUserCharacInfo::ClearCurConditionEventStep(void)
int CUserCharacInfo::ClearCurConditionEventStep() {
    return SetCurConditionEventStep(0);
}

// IDA_EA: 0x8335BCC
// IDA_NAME: _ZN15CUserCharacInfo24IncCurConditionEventStepEv
// DEMANGLED: CUserCharacInfo::IncCurConditionEventStep(void)
int CUserCharacInfo::IncCurConditionEventStep() {
    _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }
    uint16_t step = static_cast<uint16_t>(
        cur->fields_.condition_event_step_ + 1u);
    if (static_cast<int16_t>(step) > 255) {
        step = 255;
    }
    cur->fields_.condition_event_step_ = step;
    return step;
}

// IDA_EA: 0x8335C14
// IDA_NAME: _ZNK15CUserCharacInfo30GetCurConditionEventRewardStepEv
// DEMANGLED: CUserCharacInfo::GetCurConditionEventRewardStep(void)const
int CUserCharacInfo::GetCurConditionEventRewardStep() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.condition_event_reward_step_;
    }
    return 0;
}

// IDA_EA: 0x84EC29C
// IDA_NAME: _ZN15CUserCharacInfo30SetCurConditionEventRewardStepEs
// DEMANGLED: CUserCharacInfo::SetCurConditionEventRewardStep(short)
int CUserCharacInfo::SetCurConditionEventRewardStep(int16_t step) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.condition_event_reward_step_ = step;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC280
// IDA_NAME: _ZN15CUserCharacInfo32ClearCurConditionEventRewardStepEv
// DEMANGLED: CUserCharacInfo::ClearCurConditionEventRewardStep(void)
int CUserCharacInfo::ClearCurConditionEventRewardStep() {
    return SetCurConditionEventRewardStep(0);
}

// IDA_EA: 0x8335C38
// IDA_NAME: _ZN15CUserCharacInfo30IncCurConditionEventRewardStepEv
// DEMANGLED: CUserCharacInfo::IncCurConditionEventRewardStep(void)
int CUserCharacInfo::IncCurConditionEventRewardStep() {
    _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }
    uint16_t step = static_cast<uint16_t>(
        cur->fields_.condition_event_reward_step_ + 1u);
    if (static_cast<int16_t>(step) > 255) {
        step = 255;
    }
    cur->fields_.condition_event_reward_step_ = step;
    return step;
}

// IDA_EA: 0x8335C80
// IDA_NAME: _ZNK15CUserCharacInfo26GetProperDungeonClearCountEv
// DEMANGLED: CUserCharacInfo::GetProperDungeonClearCount(void)const
int CUserCharacInfo::GetProperDungeonClearCount() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.proper_dungeon_clear_count_;
    }
    return 0;
}

// IDA_EA: 0x84EC2E2
// IDA_NAME: _ZN15CUserCharacInfo26SetProperDungeonClearCountEs
// DEMANGLED: CUserCharacInfo::SetProperDungeonClearCount(short)
int CUserCharacInfo::SetProperDungeonClearCount(int16_t count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.proper_dungeon_clear_count_ = count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC2C6
// IDA_NAME: _ZN15CUserCharacInfo28ClearProperDungeonClearCountEv
// DEMANGLED: CUserCharacInfo::ClearProperDungeonClearCount(void)
int CUserCharacInfo::ClearProperDungeonClearCount() {
    return SetProperDungeonClearCount(0);
}

// IDA_EA: 0x85BFBFC
// IDA_NAME: _ZN15CUserCharacInfo26IncProperDungeonClearCountEv
// DEMANGLED: CUserCharacInfo::IncProperDungeonClearCount(void)
int CUserCharacInfo::IncProperDungeonClearCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        ++cur->fields_.proper_dungeon_clear_count_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8335CA4
// IDA_NAME: _ZN15CUserCharacInfo19IsProperDungeonUserEv
// DEMANGLED: CUserCharacInfo::IsProperDungeonUser(void)
int CUserCharacInfo::IsProperDungeonUser() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.proper_dungeon_user_;
    }
    return 0;
}

// IDA_EA: 0x85BFC22
// IDA_NAME: _ZN15CUserCharacInfo20SetProperDungeonUserEv
// DEMANGLED: CUserCharacInfo::SetProperDungeonUser(void)
int CUserCharacInfo::SetProperDungeonUser() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.proper_dungeon_user_ = 1;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x85BFC3E
// IDA_NAME: _ZN15CUserCharacInfo22ClearProperDungeonUserEv
// DEMANGLED: CUserCharacInfo::ClearProperDungeonUser(void)
int CUserCharacInfo::ClearProperDungeonUser() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.proper_dungeon_user_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x85BFAEC
// IDA_NAME: _ZNK15CUserCharacInfo20getMemberDungeonFlagEv
// DEMANGLED: CUserCharacInfo::getMemberDungeonFlag(void)const
int CUserCharacInfo::getMemberDungeonFlag() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.member_dungeon_flag_;
    }
    return 1;
}

// IDA_EA: 0x85BFB10
// IDA_NAME: _ZN15CUserCharacInfo20setMemberDungeonFlagEb
// DEMANGLED: CUserCharacInfo::setMemberDungeonFlag(bool)
int CUserCharacInfo::setMemberDungeonFlag(bool flag) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.member_dungeon_flag_ = flag;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x85BFBD6
// IDA_NAME: _ZN15CUserCharacInfo28DecreaseRevengeDungeonTicketEv
// DEMANGLED: CUserCharacInfo::DecreaseRevengeDungeonTicket(void)
int CUserCharacInfo::DecreaseRevengeDungeonTicket() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        --cur->fields_.revenge_dungeon_count_;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC1A0
// IDA_NAME: _ZN15CUserCharacInfo23SetCurCharacRevengeDataEhh
// DEMANGLED: CUserCharacInfo::SetCurCharacRevengeData(unsigned char,unsigned char)
int CUserCharacInfo::SetCurCharacRevengeData(uint8_t attack_count,
                                              uint8_t revenge_count) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.village_attack_count_ = attack_count;
        cur->fields_.revenge_dungeon_count_ = revenge_count;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC1DE
// IDA_NAME: _ZN15CUserCharacInfo22VillageAttack_DBUpdateEv
// DEMANGLED: CUserCharacInfo::VillageAttack_DBUpdate(void)
int CUserCharacInfo::VillageAttack_DBUpdate() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.village_attack_update_flag_ = 1;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC1FA
// IDA_NAME: _ZN15CUserCharacInfo25VillageAttack_NotDBUpdateEv
// DEMANGLED: CUserCharacInfo::VillageAttack_NotDBUpdate(void)
int CUserCharacInfo::VillageAttack_NotDBUpdate() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.village_attack_update_flag_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EC216
// IDA_NAME: _ZNK15CUserCharacInfo24GetCurVillageAttackCountEv
// DEMANGLED: CUserCharacInfo::GetCurVillageAttackCount(void)const
int CUserCharacInfo::GetCurVillageAttackCount() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.village_attack_count_;
    }
    return 0;
}

// IDA_EA: 0x85BFAC0
// IDA_NAME: _ZN15CUserCharacInfo32setCurCharacLastPlayDungeonIndexEj
// DEMANGLED: CUserCharacInfo::setCurCharacLastPlayDungeonIndex(unsigned int)
int CUserCharacInfo::setCurCharacLastPlayDungeonIndex(uint32_t index) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.last_play_dungeon_index_ =
            index;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x84EBFC4
// IDA_NAME: _ZN15CUserCharacInfo32getCurCharacLastPlayDungeonIndexEv
// DEMANGLED: CUserCharacInfo::getCurCharacLastPlayDungeonIndex(void)
int CUserCharacInfo::getCurCharacLastPlayDungeonIndex() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.last_play_dungeon_index_;
    }
    return 0;
}

// IDA_EA: 0x86969FE
// IDA_NAME: _ZN15CUserCharacInfo30ResetCurCharacDungeonPlayCountEv
// DEMANGLED: CUserCharacInfo::ResetCurCharacDungeonPlayCount(void)
int CUserCharacInfo::ResetCurCharacDungeonPlayCount() {
    enableSaveCharacStat();
    return SetCurCharacDungeonPlayCount(0);
}

// IDA_EA: 0x8696A24
// IDA_NAME: _ZN15CUserCharacInfo28GetCurCharacCoolTimeItemListEv
// DEMANGLED: CUserCharacInfo::GetCurCharacCoolTimeItemList(void)
std::vector<ContinuousItemInfo>* CUserCharacInfo::GetCurCharacCoolTimeItemList() {
    return &current_charac_->fields_.cool_time_items_;
}

// IDA_EA: 0x8696A34
// IDA_NAME: _ZNK15CUserCharacInfo29GetCurCharacCoolTimeItemListREv
// DEMANGLED: CUserCharacInfo::GetCurCharacCoolTimeItemListR(void)const
std::vector<ContinuousItemInfo>* CUserCharacInfo::GetCurCharacCoolTimeItemListR() const {
    return &current_charac_->fields_.cool_time_items_;
}

// IDA_EA: 0x8696A44
// IDA_NAME: _ZN15CUserCharacInfo26GetCurCharacEffectItemListEv
// DEMANGLED: CUserCharacInfo::GetCurCharacEffectItemList(void)
std::vector<ContinuousItemInfo>* CUserCharacInfo::GetCurCharacEffectItemList() {
    return &current_charac_->fields_.effect_items_;
}

// IDA_EA: 0x85D19FA
// IDA_NAME: _ZNK15CUserCharacInfo27GetCurCharacEffectItemListREv
// DEMANGLED: CUserCharacInfo::GetCurCharacEffectItemListR(void)const
std::vector<ContinuousItemInfo>* CUserCharacInfo::GetCurCharacEffectItemListR() const {
    return &current_charac_->fields_.effect_items_;
}

// IDA_EA: 0x8696A54
// IDA_NAME: _ZN15CUserCharacInfo22SetIsAffectedExpDoubleEb
// DEMANGLED: CUserCharacInfo::SetIsAffectedExpDouble(bool)
int CUserCharacInfo::SetIsAffectedExpDouble(bool flag) {
    _Charac_info* const cur = current_charac_;
    cur->fields_.affected_by_exp_double_ = flag ? 1 : 0;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x85BFCF6
// IDA_NAME: _ZN15CUserCharacInfo17IsExpItemAffectedEv
// DEMANGLED: CUserCharacInfo::IsExpItemAffected(void)
int CUserCharacInfo::IsExpItemAffected() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.affected_by_exp_double_;
    }
    return 0;
}

// IDA_EA: 0x85BFCCA
// IDA_NAME: _ZN15CUserCharacInfo16GetExpAffectRateEv
// DEMANGLED: CUserCharacInfo::GetExpAffectRate(void)
float CUserCharacInfo::GetExpAffectRate() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.exp_affect_rate_;
    }
    return 0.0f;
}

// IDA_EA: 0x863BE1C
// IDA_NAME: _ZN15CUserCharacInfo22SetExpAffectItemEffectEbf
// DEMANGLED: CUserCharacInfo::SetExpAffectItemEffect(bool,float)
int CUserCharacInfo::SetExpAffectItemEffect(bool affected, float rate) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.affected_by_exp_double_ = affected ? 1 : 0;
        cur->fields_.exp_affect_rate_ = rate;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696A72
// IDA_NAME: _ZNK15CUserCharacInfo23GetCurCharacBackupSkillEv
// DEMANGLED: CUserCharacInfo::GetCurCharacBackupSkill(void)const
SkillSlot* CUserCharacInfo::GetCurCharacBackupSkill() const {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        return reinterpret_cast<SkillSlot*>(
            &cur->fields_.premium_lethe_first_);
    }
    return 0;
}

// IDA_EA: 0x8696A94
// IDA_NAME: _ZNK15CUserCharacInfo26GetCurCharacBackupSkill2NDEv
// DEMANGLED: CUserCharacInfo::GetCurCharacBackupSkill2ND(void)const
SkillSlot* CUserCharacInfo::GetCurCharacBackupSkill2ND() const {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        return reinterpret_cast<SkillSlot*>(
            &cur->fields_.premium_lethe_second_);
    }
    return 0;
}

// IDA_EA: 0x8696AB6
// IDA_NAME: _ZN15CUserCharacInfo15getDisguiseKindEv
// DEMANGLED: CUserCharacInfo::getDisguiseKind(void)
int CUserCharacInfo::getDisguiseKind() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.disguise_kind_;
    }
    return 0;
}

// IDA_EA: 0x8696ADA
// IDA_NAME: _ZN15CUserCharacInfo16getDisguiseIndexEv
// DEMANGLED: CUserCharacInfo::getDisguiseIndex(void)
int CUserCharacInfo::getDisguiseIndex() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.disguise_index_;
    }
    return 0;
}

// IDA_EA: 0x8696AFE
// IDA_NAME: _ZN15CUserCharacInfo21GetCurExEquipSlotStatEv
// DEMANGLED: CUserCharacInfo::GetCurExEquipSlotStat(void)
int CUserCharacInfo::GetCurExEquipSlotStat() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.exp_equip_slot_state_;
    }
    return 0;
}

// IDA_EA: 0x8696B22
// IDA_NAME: _ZN15CUserCharacInfo22setLinkCharacSlotIndexEc
// DEMANGLED: CUserCharacInfo::setLinkCharacSlotIndex(char)
int CUserCharacInfo::setLinkCharacSlotIndex(char index) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.link_charac_slot_index_ =
            static_cast<unsigned char>(index);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696B4A
// IDA_NAME: _ZN15CUserCharacInfo17getCharacLinkTypeEv
// DEMANGLED: CUserCharacInfo::getCharacLinkType(void)
int CUserCharacInfo::getCharacLinkType() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.link_type_;
    }
    return 0;
}

// IDA_EA: 0x8696B6E
// IDA_NAME: _ZN15CUserCharacInfo21getCharacLinkBonusExpEv
// DEMANGLED: CUserCharacInfo::getCharacLinkBonusExp(void)
int CUserCharacInfo::getCharacLinkBonusExp() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.link_bonus_experience_;
    }
    return 0;
}

// IDA_EA: 0x8696B90
// IDA_NAME: _ZN15CUserCharacInfo21setCharacLinkBonusExpEj
// DEMANGLED: CUserCharacInfo::setCharacLinkBonusExp(unsigned int)
int CUserCharacInfo::setCharacLinkBonusExp(uint32_t exp) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.link_bonus_experience_ = exp;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696BAE
// IDA_NAME: _ZN15CUserCharacInfo22getCharacLinkBonusGoldEv
// DEMANGLED: CUserCharacInfo::getCharacLinkBonusGold(void)
int CUserCharacInfo::getCharacLinkBonusGold() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.link_bonus_gold_;
    }
    return 0;
}

// IDA_EA: 0x8696BD0
// IDA_NAME: _ZN15CUserCharacInfo22setCharacLinkBonusGoldEj
// DEMANGLED: CUserCharacInfo::setCharacLinkBonusGold(unsigned int)
int CUserCharacInfo::setCharacLinkBonusGold(uint32_t gold) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.link_bonus_gold_ =
            gold;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696BEE
// IDA_NAME: _ZNK15CUserCharacInfo15getSkillCommandEv
// DEMANGLED: CUserCharacInfo::getSkillCommand(void)const
std::map<int, STSkillCommandData>* CUserCharacInfo::getSkillCommand() const {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        return &cur->fields_.skill_commands_;
    }
    return 0;
}

// IDA_EA: 0x8696C10
// IDA_NAME: _ZN15CUserCharacInfo19getCurEquipslotFlagEv
// DEMANGLED: CUserCharacInfo::getCurEquipslotFlag(void)
int CUserCharacInfo::getCurEquipslotFlag() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.equipslot_flag_;
    }
    return -1;
}

// IDA_EA: 0x8696C34
// IDA_NAME: _ZN15CUserCharacInfo19setCurEquipslotFlagEc
// DEMANGLED: CUserCharacInfo::setCurEquipslotFlag(char)
int CUserCharacInfo::setCurEquipslotFlag(char flag) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.equipslot_flag_ = static_cast<unsigned char>(flag);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x8696C68
// IDA_NAME: _ZNK15CUserCharacInfo30get_charac_seria_buf_bonus_expEv
// DEMANGLED: CUserCharacInfo::get_charac_seria_buf_bonus_exp(void)const
int CUserCharacInfo::get_charac_seria_buf_bonus_exp() const {
    _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }
    const int value =
        cur->fields_.seria_buff_bonus_experience_;
    cur->fields_.seria_buff_bonus_experience_ = 0;
    return value;
}

// IDA_EA: 0x8696CA4
// IDA_NAME: _ZNK15CUserCharacInfo32is_growth_power_exp_reward_rightEv
// DEMANGLED: CUserCharacInfo::is_growth_power_exp_reward_right(void)const
int CUserCharacInfo::is_growth_power_exp_reward_right() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.growth_power_exp_reward_right_;
    }
    return 0;
}

// IDA_EA: 0x8696CC8
// IDA_NAME: _ZN15CUserCharacInfo28getContractOfCubePremiumInfoERhS0_
// DEMANGLED: CUserCharacInfo::getContractOfCubePremiumInfo(unsigned char &,unsigned char &)
int CUserCharacInfo::getContractOfCubePremiumInfo(uint8_t& enabled,
                                                   uint8_t& premium_type) {
    const _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }
    enabled = cur->fields_.contract_cube_premium_enabled_;
    premium_type = cur->fields_.contract_cube_premium_type_;
    return 1;
}

// IDA_EA: 0x8696D18
// IDA_NAME: _ZNK15CUserCharacInfo22isSavedPremiumCubeInfoEv
// DEMANGLED: CUserCharacInfo::isSavedPremiumCubeInfo(void)const
int CUserCharacInfo::isSavedPremiumCubeInfo() const {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.contract_cube_premium_saved_;
    }
    return 0;
}

// IDA_EA: 0x8696D3C
// IDA_NAME: _ZN15CUserCharacInfo27disableSavedPremiumCubeInfoEv
// DEMANGLED: CUserCharacInfo::disableSavedPremiumCubeInfo(void)
int CUserCharacInfo::disableSavedPremiumCubeInfo() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.contract_cube_premium_saved_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x86B087C
// IDA_NAME: _ZN15CUserCharacInfo20SetIsEndLetheAtLoginEv
// DEMANGLED: CUserCharacInfo::SetIsEndLetheAtLogin(void)
int CUserCharacInfo::SetIsEndLetheAtLogin() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.end_lethe_at_login_ = 1;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x86B4BE6
// IDA_NAME: _ZN15CUserCharacInfo21IncVillageAttackCountEv
// DEMANGLED: CUserCharacInfo::IncVillageAttackCount(void)
int CUserCharacInfo::IncVillageAttackCount() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        ++cur->fields_.village_attack_count_;
        cur->fields_.village_attack_update_flag_ = 1;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x86D200E
// IDA_NAME: _ZN15CUserCharacInfo23GetPowerWarLastPlayTimeEv
// DEMANGLED: CUserCharacInfo::GetPowerWarLastPlayTime(void)
int CUserCharacInfo::GetPowerWarLastPlayTime() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.power_war_.dword_08;
    }
    return 0;
}

// IDA_EA: 0x86D2030
// IDA_NAME: _ZN15CUserCharacInfo36GetPowerWarProcessInfoLastUpdateTimeEv
// DEMANGLED: CUserCharacInfo::GetPowerWarProcessInfoLastUpdateTime(void)
int CUserCharacInfo::GetPowerWarProcessInfoLastUpdateTime() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.power_war_.dword_04;
    }
    return 0;
}

// IDA_EA: 0x84EC182
// IDA_NAME: _ZN15CUserCharacInfo36SetPowerWarProcessInfoLastUpdateTimeEl
// DEMANGLED: CUserCharacInfo::SetPowerWarProcessInfoLastUpdateTime(long)
int CUserCharacInfo::SetPowerWarProcessInfoLastUpdateTime(long current_sec) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.power_war_.dword_04 =
            current_sec;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x859503C
// IDA_NAME: _ZN15CUserCharacInfo23IsCurCharacLeaveTimeoutEv
// DEMANGLED: CUserCharacInfo::IsCurCharacLeaveTimeout(void)
int CUserCharacInfo::IsCurCharacLeaveTimeout() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.leave_timeout_state_;
    }
    return 0;
}

// IDA_EA: 0x85BFB44
// IDA_NAME: _ZN15CUserCharacInfo24getCurCharacTutorialFlagEv
// DEMANGLED: CUserCharacInfo::getCurCharacTutorialFlag(void)
int CUserCharacInfo::getCurCharacTutorialFlag() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.tutorial_flag_;
    }
    return 0;
}

// IDA_EA: 0x864739A
// IDA_NAME: _ZN15CUserCharacInfo23resetCharacTutorialFlagEv
// DEMANGLED: CUserCharacInfo::resetCharacTutorialFlag(void)
void* CUserCharacInfo::resetCharacTutorialFlag() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.tutorial_flag_ = 0;
        return std::memset(cur->fields_.escalade_tutorial_flag_, 0,
                           sizeof(cur->fields_.escalade_tutorial_flag_));
    }
    return cur;
}

// IDA_EA: 0x86473E6
// IDA_NAME: _ZN15CUserCharacInfo24setCurCharacTutorialFlagEj
// DEMANGLED: CUserCharacInfo::setCurCharacTutorialFlag(unsigned int)
int CUserCharacInfo::setCurCharacTutorialFlag(uint32_t flag) {
    _Charac_info* const cur = current_charac_;
    if (!cur) {
        return 0;
    }
    enableSaveCharacStat();
    if (flag <= 0x1Fu) {
        uint32_t bits = cur->fields_.tutorial_flag_;
        const uint32_t mask = 1u << flag;
        if ((bits & mask) != 0) {
            return 0;
        }
        bits |= mask;
        cur->fields_.tutorial_flag_ = bits;
        return 1;
    }
    if (flag > 0x7Fu) {
        return 0;
    }
    const uint32_t index = (flag - 32u) / 8u;
    const uint32_t bit = (flag - 32u) % 8u;
    if (index >= 0x10u) {
        return 0;
    }
    unsigned char& value = cur->fields_.escalade_tutorial_flag_[index];
    const unsigned char mask = static_cast<unsigned char>(1u << bit);
    if ((value & mask) != 0) {
        return 0;
    }
    value |= mask;
    return 1;
}

// IDA_EA: 0x86D2052
// IDA_NAME: _ZN15CUserCharacInfo24ResetRevengeDungeonClearEv
// DEMANGLED: CUserCharacInfo::ResetRevengeDungeonClear(void)
int CUserCharacInfo::ResetRevengeDungeonClear() {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.revenge_dungeon_count_ = 0;
        cur->fields_.village_attack_count_ = 0;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x868EED2
// IDA_NAME: _ZN15CUserCharacInfo27GetCurCharacChaosResponTimeEv
// DEMANGLED: CUserCharacInfo::GetCurCharacChaosResponTime(void)
int CUserCharacInfo::GetCurCharacChaosResponTime() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.chaos_response_time_late_;
    }
    return 0;
}

// IDA_EA: 0x868EEF4
// IDA_NAME: _ZN15CUserCharacInfo27SetCurCharacChaosResponTimeEi
// DEMANGLED: CUserCharacInfo::SetCurCharacChaosResponTime(int)
int CUserCharacInfo::SetCurCharacChaosResponTime(int time) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        enableSaveCharacStat();
        cur->fields_.chaos_response_time_late_ =
            time;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x868EF20
// IDA_NAME: _ZN15CUserCharacInfo24GetCurCharacAssaultMoneyEv
// DEMANGLED: CUserCharacInfo::GetCurCharacAssaultMoney(void)
int CUserCharacInfo::GetCurCharacAssaultMoney() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.assault_money_;
    }
    return 0;
}

// IDA_EA: 0x868EF42
// IDA_NAME: _ZN15CUserCharacInfo24SetCurCharacAssaultMoneyEi
// DEMANGLED: CUserCharacInfo::SetCurCharacAssaultMoney(int)
int CUserCharacInfo::SetCurCharacAssaultMoney(int money) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.assault_money_ =
            money;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x868EF60
// IDA_NAME: _ZN15CUserCharacInfo27GetCurCharacAssaultedUserIdEv
// DEMANGLED: CUserCharacInfo::GetCurCharacAssaultedUserId(void)
int CUserCharacInfo::GetCurCharacAssaultedUserId() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.assaulted_user_id_;
    }
    return 0;
}

// IDA_EA: 0x868EF84
// IDA_NAME: _ZN15CUserCharacInfo27SetCurCharacAssaultedUserIdEt
// DEMANGLED: CUserCharacInfo::SetCurCharacAssaultedUserId(unsigned short)
int CUserCharacInfo::SetCurCharacAssaultedUserId(uint16_t unique_id) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.assaulted_user_id_ = unique_id;
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x868BDAC
// IDA_NAME: _ZN15CUserCharacInfo12SetInvisibleEv
// DEMANGLED: CUserCharacInfo::SetInvisible(void)
int CUserCharacInfo::SetInvisible() {
    _Charac_info* const cur = current_charac_;
    cur->fields_.visible_flag_.visible_flag &=
        static_cast<unsigned char>(~1u);
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x868BDC8
// IDA_NAME: _ZN15CUserCharacInfo10SetVisibleEv
// DEMANGLED: CUserCharacInfo::SetVisible(void)
int CUserCharacInfo::SetVisible() {
    _Charac_info* const cur = current_charac_;
    cur->fields_.visible_flag_.visible_flag |= 1u;
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x868BDE4
// IDA_NAME: _ZN15CUserCharacInfo25get_charac_visible_valuesEv
// DEMANGLED: CUserCharacInfo::get_charac_visible_values(void)
int CUserCharacInfo::get_charac_visible_values() {
    const _Charac_info* const cur = current_charac_;
    if (cur) {
        return cur->fields_.visible_flag_.visible_flag;
    }
    return 0;
}

// IDA_EA: 0x868BE08
// IDA_NAME: _ZN15CUserCharacInfo25set_charac_visible_valuesEc
// DEMANGLED: CUserCharacInfo::set_charac_visible_values(char)
int CUserCharacInfo::set_charac_visible_values(char values) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.visible_flag_.visible_flag =
            static_cast<unsigned char>(values);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x868BE30
// IDA_NAME: _ZN15CUserCharacInfo30set_charac_visible_grow_avatarEb
// DEMANGLED: CUserCharacInfo::set_charac_visible_grow_avatar(bool)
int CUserCharacInfo::set_charac_visible_grow_avatar(bool visible) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        result = (2 * ((static_cast<signed char>(visible << 7) >> 7) & 1)) |
                 (cur->fields_.visible_flag_.visible_flag & 0xFD);
        cur->fields_.visible_flag_.visible_flag =
            static_cast<unsigned char>(result);
    }
    return result;
}

// IDA_EA: 0x868BE70
// IDA_NAME: _ZN15CUserCharacInfo28set_charac_visible_teleport_Eb
// DEMANGLED: CUserCharacInfo::set_charac_visible_teleport_(bool)
int CUserCharacInfo::set_charac_visible_teleport_(bool visible) {
    _Charac_info* const cur = current_charac_;
    int result = reinterpret_cast<int>(cur);
    if (cur) {
        result = (4 * ((static_cast<signed char>(visible << 7) >> 7) & 1)) |
                 (cur->fields_.visible_flag_.visible_flag & 0xFB);
        cur->fields_.visible_flag_.visible_flag =
            static_cast<unsigned char>(result);
    }
    return result;
}

// IDA_EA: 0x868BEB4
// IDA_NAME: _ZN15CUserCharacInfo30get_charac_visible_grow_avatarEv
// DEMANGLED: CUserCharacInfo::get_charac_visible_grow_avatar(void)
int CUserCharacInfo::get_charac_visible_grow_avatar() {
    const _Charac_info* const cur = current_charac_;
    return cur &&
           ((cur->fields_.visible_flag_.visible_flag & 2u) != 0);
}

// IDA_EA: 0x868BEE4
// IDA_NAME: _ZN15CUserCharacInfo18IsCurCharacVisibleEv
// DEMANGLED: CUserCharacInfo::IsCurCharacVisible(void)
int CUserCharacInfo::IsCurCharacVisible() {
    const _Charac_info* const cur = current_charac_;
    return cur &&
           ((cur->fields_.visible_flag_.visible_flag & 1u) != 0);
}

// IDA_EA: 0x868E1F6
// IDA_NAME: _ZN15CUserCharacInfo18setPremiumBonusExpEji
// DEMANGLED: CUserCharacInfo::setPremiumBonusExp(unsigned int,int)
int CUserCharacInfo::setPremiumBonusExp(uint32_t reward_exp,
                                        int bonus_rate) {
    _Charac_info* const charac = current_charac_;
    if (!charac) {
        return 0;
    }
    charac->fields_.premium_bonus_experience_ =
        static_cast<int>(static_cast<long double>(reward_exp) *
                         (static_cast<long double>(bonus_rate) / 100.0L));
    return charac->fields_.premium_bonus_experience_;
}

// IDA_EA: 0x868E262
// IDA_NAME: _ZNK15CUserCharacInfo18getPremiumBonusExpEv
// DEMANGLED: CUserCharacInfo::getPremiumBonusExp(void)const
int CUserCharacInfo::getPremiumBonusExp() const {
    if (current_charac_) {
        return current_charac_->fields_.premium_bonus_experience_;
    }
    return 0;
}

// IDA_EA: 0x868E284
// IDA_NAME: _ZN15CUserCharacInfo20resetPremiumBonusExpEv
// DEMANGLED: CUserCharacInfo::resetPremiumBonusExp(void)
int CUserCharacInfo::resetPremiumBonusExp() {
    if (current_charac_) {
        current_charac_->fields_.premium_bonus_experience_ = 0;
    }
    return reinterpret_cast<int>(current_charac_);
}

// IDA_EA: 0x868E2A4
// IDA_NAME: _ZN15CUserCharacInfo22setWeekendBonusFatigueEt
// DEMANGLED: CUserCharacInfo::setWeekendBonusFatigue(unsigned short)
int CUserCharacInfo::setWeekendBonusFatigue(uint16_t fatigue) {
    _Charac_info* const cur = current_charac_;
    if (cur) {
        cur->fields_.maximum_fatigue_ =
            static_cast<uint16_t>(fatigue + DEFAULT_MAX_FATIGUE);
    }
    return reinterpret_cast<int>(cur);
}

// IDA_EA: 0x868E2D2
// IDA_NAME: _ZN15CUserCharacInfo18setWeekendBonusExpEji
// DEMANGLED: CUserCharacInfo::setWeekendBonusExp(unsigned int,int)
int CUserCharacInfo::setWeekendBonusExp(uint32_t reward_exp,
                                        int bonus_rate) {
    int result = reinterpret_cast<int>(current_charac_);
    if (current_charac_) {
        result = static_cast<int>(static_cast<long double>(reward_exp) *
                                  (static_cast<long double>(bonus_rate) /
                                   100.0L));
        current_charac_->fields_.weekend_bonus_experience_ = result;
    }
    return result;
}

// IDA_EA: 0x868E328
// IDA_NAME: _ZN15CUserCharacInfo18getWeekendBonusExpEv
// DEMANGLED: CUserCharacInfo::getWeekendBonusExp(void)
int CUserCharacInfo::getWeekendBonusExp() {
    if (current_charac_) {
        return current_charac_->fields_.weekend_bonus_experience_;
    }
    return 0;
}

// IDA_EA: 0x868E34A
// IDA_NAME: _ZN15CUserCharacInfo20resetWeekendBonusExpEv
// DEMANGLED: CUserCharacInfo::resetWeekendBonusExp(void)
int CUserCharacInfo::resetWeekendBonusExp() {
    if (current_charac_) {
        current_charac_->fields_.weekend_bonus_experience_ = 0;
    }
    return reinterpret_cast<int>(current_charac_);
}

// IDA_EA: 0x868EE26
// IDA_NAME: _ZN15CUserCharacInfo15getWpBonusPointER18WpBonusPointResult
// DEMANGLED: CUserCharacInfo::getWpBonusPoint(WpBonusPointResult &)
int CUserCharacInfo::getWpBonusPoint(WpBonusPointResult& result_out) {
    int result = reinterpret_cast<int>(current_charac_);
    if (current_charac_) {
        result = current_charac_->fields_.wp_bonus_point_.normal;
        result_out.normal = current_charac_->fields_.wp_bonus_point_.normal;
        result_out.premium = current_charac_->fields_.wp_bonus_point_.premium;
    }
    return result;
}

// IDA_EA: 0x868EE50
// IDA_NAME: _ZN15CUserCharacInfo16getExpBonusPointER19ExpBonusPointResult
// DEMANGLED: CUserCharacInfo::getExpBonusPoint(ExpBonusPointResult &)
int CUserCharacInfo::getExpBonusPoint(ExpBonusPointResult& result_out) {
    int result = reinterpret_cast<int>(current_charac_);
    if (current_charac_) {
        result = current_charac_->fields_.exp_bonus_point_.normal;
        result_out.normal = current_charac_->fields_.exp_bonus_point_.normal;
        result_out.premium = current_charac_->fields_.exp_bonus_point_.premium;
    }
    return result;
}

// IDA_EA: 0x868EE7A
// IDA_NAME: _ZN15CUserCharacInfo15resetBonusPointEv
// DEMANGLED: CUserCharacInfo::resetBonusPoint(void)
void* CUserCharacInfo::resetBonusPoint() {
    if (current_charac_) {
        current_charac_->fields_.wp_bonus_point_.normal = 0;
        current_charac_->fields_.wp_bonus_point_.premium = 0;
        current_charac_->fields_.exp_bonus_point_.normal = 0;
        current_charac_->fields_.exp_bonus_point_.premium = 0;
        return &current_charac_->fields_.exp_bonus_point_;
    }
    return current_charac_;
}

// IDA_EA: 0x868EC5C
// IDA_NAME: _ZN15CUserCharacInfo15checkBonusPointEv
// DEMANGLED: CUserCharacInfo::checkBonusPoint(void)
char CUserCharacInfo::checkBonusPoint() {
    if (!current_charac_) {
        return 0;
    }

    resetBonusPoint();
    CInventory* const inventory = getCurCharacInvenR();
    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
         ++slot) {
        Inven_Item* const inventory_item = inventory->GetInvenRef(0, slot);
        if (!inventory_item || !inventory_item->item_id_) {
            continue;
        }

        CEquipItem* const equipment = static_cast<CEquipItem*>(
            G_CDataManager()->find_item(inventory_item->item_id_));
        if (!equipment) {
            continue;
        }

        const WpBonusPointData* const wp = equipment->getWpBonusPoint();
        if (wp->type == 0) {
            current_charac_->fields_.wp_bonus_point_.normal += wp->value;
        } else if (wp->type == 1) {
            current_charac_->fields_.wp_bonus_point_.premium += wp->value;
        }

        const ExpBonusPointData* const exp = equipment->getExpBonusPoint();
        if (exp->type == 0) {
            current_charac_->fields_.exp_bonus_point_.normal += exp->value;
        } else if (exp->type == 1) {
            current_charac_->fields_.exp_bonus_point_.premium += exp->value;
        }
    }
    return 0;
}

// IDA_EA: 0x864CD92
// IDA_NAME: _ZN5CUser12SelectCharacEii
// DEMANGLED: CUser::SelectCharac(int,int)
int CUser::SelectCharac(int slot, int tag_slot) {
    if (getCurCharacR()) {
        cMyTrace trace("bool CUser::SelectCharac(int, int)", 4603, 5);
        trace("CUser::SelectCharac m_selected is not null");
        return 0;
    }

    const int charac_count = static_cast<int>(characters_.size());

    if (slot < 0 || charac_count <= slot) {
        cMyTrace trace("bool CUser::SelectCharac(int, int)", 4678, 5);
        trace("CUser::SelectCharac :charac_index : %d", slot);
        return 0;
    }

    setCurCharac(&characters_[static_cast<std::size_t>(slot)]);
    if (tag_slot >= 0 && charac_count > tag_slot) {
        setTagCharac(&characters_[static_cast<std::size_t>(tag_slot)]);
        getTagCharacInvenR()->set_parent(this);
        getTagCharacSkillR()->set_parent(this);
    }

    const _Charac_info* const cur_charac = getCurCharacR();
    if (!cur_charac) {
        cMyTrace trace("bool CUser::SelectCharac(int, int)", 4620, 5);
        trace("CUser::SelectCharac m_selected is null at %d", slot);
        return 0;
    }

    const unsigned char stamina = cur_charac->fields_.stamina_;
    if (stamina <= 99u) {
        CSystemTime* system_time = GlobalData::system_time();
        const int cur_sec = system_time ? system_time->getCurSec() : 0;
        int elapsed = static_cast<int>(
            std::difftime(static_cast<time_t>(cur_sec),
                          static_cast<time_t>(
                              getCurCharacLastPlayTick())));
        elapsed = std::max(0, std::min(600, elapsed));
        const int recovered = std::min(
            100,
            static_cast<int>(stamina) + (100 * elapsed / 600));
        setCurCharacStamina(static_cast<uint8_t>(recovered));
    }

    const int grow_type =
        static_cast<signed char>(getCurCharacGrowType());
    const int charac_level = get_charac_level();
    const int charac_job = get_charac_job();
    HistoryLog::WriteSelectCh(history_stream_,
                              getCurCharacName(),
                              charac_job,
                              charac_level,
                              grow_type);

    getCurCharacInvenR()->set_parent(this);
    getCurCharacSkillR()->set_parent(this);
    reinterpret_cast<UserQuest*>(getCurCharacQuestR())->set_parent(this);
    getCurCharacCargoR()->set_parent(this);

    GameWorld* world = G_GameWorld();
    if (world) {
        ++world->occupation_counts_[charac_job];
    }
    return 1;
}

// IDA_EA: 0x8651B7A
// IDA_NAME: _ZN5CUser16ChangeCharacSlotEjj
// DEMANGLED: CUser::ChangeCharacSlot(unsigned int,unsigned int)
void* CUser::ChangeCharacSlot(unsigned int src_slot, unsigned int dst_slot) {
    const bool invalid = characters_.size() <= src_slot ||
                         characters_.size() <= dst_slot;
    if (invalid) {
        return reinterpret_cast<void*>(1);
    }

    const _Charac_info moved = characters_[src_slot];
    if (src_slot >= dst_slot) {
        characters_.insert(characters_.begin() + dst_slot, moved);
        characters_.erase(characters_.begin() + src_slot + 1u);
    } else {
        characters_.insert(characters_.begin() + dst_slot + 1u, moved);
        characters_.erase(characters_.begin() + src_slot);
    }

    // CLIENT_IDA_EVIDENCE: CMD309 success sub_138D070 sends two CMD292
    // moves after it establishes the link pair.  Each _Charac_info record
    // moves with its old numeric partner slot, so translate every active
    // partner index through the same permutation.  Without this step the
    // final pair reaches slots 0/1 while still pointing at its pre-move
    // positions and later NOTI2/basic-info responses advertise stale links.
    for (std::vector<_Charac_info>::iterator character = characters_.begin();
         character != characters_.end();
         ++character) {
        unsigned int partner =
            character->fields_.link_charac_slot_index_;
        if (partner == 0xFFu || partner >= characters_.size()) {
            continue;
        }

        if (src_slot >= dst_slot) {
            if (partner == src_slot) {
                partner = dst_slot;
            } else if (partner >= dst_slot && partner < src_slot) {
                ++partner;
            }
        } else {
            if (partner == src_slot) {
                partner = dst_slot;
            } else if (partner > src_slot && partner <= dst_slot) {
                --partner;
            }
        }
        character->fields_.link_charac_slot_index_ =
            static_cast<uint8_t>(partner);
    }
    return character_view_.enableSaveCharacView();
}

// IDA_EA: 0x86822B8
// IDA_NAME: _ZN12_Charac_info5ResetEv
// DEMANGLED: _Charac_info::Reset(void)
_Charac_info* _Charac_info::Reset() {
    // Reset is deliberately member-selective.  The old body does not blank
    // the whole record because its containers and persistence-only values
    // keep their own lifetime/invariants.
    fields_.character_no_ = 0;
    std::memset(fields_.character_name_, 0, sizeof(fields_.character_name_));
    fields_.village_ = 0;
    fields_.pvp_village_ = 0;
    fields_.village_state_ = 0;
    fields_.job_ = 0;
    fields_.level_ = 0;
    fields_.grow_type_.growth_type = 0;
    fields_.changing_grow_type_.growth_type = 0;
    fields_.experience_ = 0;
    fields_.party_bonus_experience_ = 0;
    fields_.fatigue_ = 0;
    fields_.maximum_fatigue_ = 0;
    fields_.fatigue_grownup_buff_ = 0;
    fields_.create_time_.clear();
    fields_.fatigue_battery_charging_ = 0;
    std::memset(fields_.escalade_tutorial_flag_, 0,
                sizeof(fields_.escalade_tutorial_flag_));
    fields_.school_point_ = 0;
    fields_.premium_fatigue_ = 0;
    fields_.maximum_premium_fatigue_ = 0;
    fields_.last_play_tick_ = 0;
    fields_.power_war_last_play_time_ = 0;
    fields_.forbid_user_to_play_ = 0;
    fields_.forbid_remaining_seconds_ = 0;
    fields_.additional_info_.clear();
    fields_.inventory_.reset();
    fields_.skill_slot_.clear_all_skills_both();
    fields_.cargo_.reset();

    fields_.guild_id_ = 0;
    fields_.guild_right_ = 0;
    fields_.guild_experience_ = 0;
    fields_.guild_temporary_experience_ = 0;
    fields_.guild_today_experience_ = 0;
    fields_.guild_member_grade_ = 0;
    fields_.help_abuse_ratio_ = 0;
    fields_.help_abuse_experience_ = 0;
    fields_.level_before_dungeon_ = 0;
    fields_.help_abuse_computed_ratio_ = 0;
    fields_.login_tick_ = 0;
    fields_.finish_point_ = 0;
    fields_.finish_point_total_ = 0;
    std::memset(fields_.finish_point_detail_, 0,
                sizeof(fields_.finish_point_detail_));
    fields_.member_key_ = 0;
    fields_.upper_member_key_ = 0;
    fields_.member_bonus_experience_ = 0;
    fields_.fatigue_buff_bonus_experience_ = 0;
    fields_.upper_member_experience_level_ = 0;
    fields_.tax_money_to_upper_ = 0;
    fields_.tax_fatigue_to_upper_ = 0;
    fields_.need_check_over_equip_item_ = 0;
    fields_.black_count_ = 0;

    fields_.assault_place_ = 0;
    fields_.assault_count_ = 0;
    fields_.chaos_point_ = 0;
    fields_.chaos_experience_ = 0;
    fields_.chaos_mode_count_ = 0;
    fields_.chaos_kill_count_ = 0;
    fields_.chaos_die_count_ = 0;
    fields_.chaos_die_time_ = 0;
    fields_.assault_session_state_ = 0;
    fields_.super_state_ = 0;
    fields_.ghost_state_ = 0;
    fields_.leave_timeout_state_ = 0;
    fields_.chaos_kill_time_ = 0;
    fields_.assault_fatigue_count_ = 0;
    fields_.open_coin_flag_ = 0;
    fields_.luck_point_ = 0;
    fields_.luck_point_update_count_ = 0;
    fields_.dungeon_play_count_ = 0;
    fields_.expert_job_ = 0;
    fields_.expert_job_experience_ = 0;
    fields_.expert_job_object_ = 0;
    fields_.online_preliminary_team_handle_ = 0;
    fields_.power_war_.word_00 = 0;
    fields_.power_war_.byte_02 = 0;
    fields_.power_war_.byte_03 = 1;
    fields_.power_war_.dword_04 = 0;
    fields_.power_war_.dword_08 = 0;
    fields_.power_war_state_0e99_ = 0;

    fields_.affected_by_exp_double_ = 0;
    fields_.initialized_skill_flag_ = 0;
    fields_.first_time_login_ = 0;
    fields_.dimension_dirty_ = 0;
    std::memset(fields_.dimension_inout_, 0,
                sizeof(fields_.dimension_inout_));
    fields_.ultimate_inout_dirty_ = 0;
    std::memset(fields_.ultimate_inout_, 0,
                sizeof(fields_.ultimate_inout_));
    fields_.blood_inout_count_ = 0;
    fields_.blood_best_record_update_ = 0;
    fields_.blood_best_round_record_ = -1;
    fields_.blood_best_time_record_ = 400000000;
    fields_.blood_best_records_.clear();
    fields_.persistent_state_0ee4_ = 0;
    fields_.exp_affect_rate_ = 0.0f;
    fields_.disguise_character_ = 0;
    fields_.disguise_kind_ = 0;
    fields_.disguise_index_ = 0;
    std::memset(&fields_.premium_lethe_first_, 0,
                sizeof(fields_.premium_lethe_first_));

    fields_.power_war_point_ = 0;
    fields_.power_war_assault_count_ = 0;
    fields_.power_war_victory_count_ = 0;
    fields_.straight_victories_ = 0;
    fields_.end_lethe_at_login_ = 0;
    fields_.emotion_ = 0;
    fields_.item_limit_buy_time_ = 0;
    fields_.condition_event_step_ = 0;
    fields_.condition_event_reward_step_ = 0;
    fields_.proper_dungeon_clear_count_ = 0;
    fields_.proper_dungeon_user_ = 0;
    fields_.skill_tree_index_ = -1;
    fields_.event_item_count_ = 0;
    fields_.event_add_item_count_ = 0;
    fields_.exp_equip_slot_state_ = 0;
    fields_.member_dungeon_flag_ = 1;
    fields_.village_attack_count_ = 0;
    fields_.revenge_dungeon_count_ = 0;
    fields_.village_attack_update_flag_ = 0;
    fields_.link_charac_slot_index_ = 0xFFu;
    fields_.link_type_ = 0;
    fields_.link_bonus_experience_ = 0;
    fields_.link_bonus_gold_ = 0;
    fields_.link_state_124f_ = 0;
    fields_.link_runtime_flags_ = 0;
    fields_.skill_commands_.clear();

    fields_.member_bonus_fatigue_ = 0;
    fields_.guild_secede_ = 0;
    fields_.last_play_dungeon_index_ = 0;
    fields_.upgrade_separate_true_flag_ = 0;
    fields_.upgrade_separate_false_flag_ = 0;
    fields_.equipslot_flag_ = 0;
    fields_.channel_equipslot_switch_ = 0;
    fields_.expand_equipslot_switch_ = 0;
    fields_.event_bring_up_reward_flag_ = 0;
    fields_.event_bring_up_material_use_count_ = 0;
    fields_.event_bring_up_material_total_ = 0;
    fields_.visible_flag_.reset();
    fields_.growth_power_exp_reward_right_ = 0;
    fields_.seria_buff_bonus_experience_ = 0;
    fields_.growth_power_reward_ = 0;
    fields_.aura_avatar_option_.reset();
    fields_.premium_bonus_experience_ = 0;
    fields_.weekend_bonus_experience_ = 0;
    std::memset(fields_.bonus_point_prefix_, 0,
                sizeof(fields_.bonus_point_prefix_));
    fields_.wp_bonus_point_.clear();
    fields_.exp_bonus_point_.clear();
    fields_.chaos_response_time_late_ = 0;
    fields_.assault_money_ = 0;
    fields_.assaulted_user_id_ = 0;
    std::memset(&fields_.character_option_, 0,
                sizeof(fields_.character_option_));
    fields_.striker_skill_index_ = 0;
    fields_.tag_character_no_ = 0;
    fields_.tag_charac_info_loaded_ = 0;
    fields_.blue_marble_enter_count_ = 0;
    fields_.blue_marble_dead_count_ = 0;
    std::memset(fields_.blue_marble_to_max_equip_state_, 0,
                sizeof(fields_.blue_marble_to_max_equip_state_));
    fields_.maximum_equip_level_ = 0;
    fields_.character_view_state_ = 0;
    fields_.seria_room_ani_deco_.receive_flow = 0;
    fields_.seria_room_ani_deco_.deco_flow = 0;
    fields_.contract_cube_premium_enabled_ = 0;
    fields_.contract_cube_premium_type_ = 0;
    fields_.contract_cube_premium_saved_ = 0;
    return this;
}

// IDA_EA: 0x8695CF8
// IDA_NAME: _ZN12_Charac_infoC2Ev
// DEMANGLED: _Charac_info::_Charac_info(void)
_Charac_info::_Charac_info() {
    Reset();
}

// IDA_EA: 0x8697982
// IDA_NAME: _ZN12_Charac_infoD2Ev
// DEMANGLED: _Charac_info::~_Charac_info()
_Charac_info::~_Charac_info() {}

// IDA_EA: 0x8699820
// IDA_NAME: _ZN12_Charac_infoC2ERKS_
// DEMANGLED: _Charac_info::_Charac_info(_Charac_info const&)
_Charac_info::_Charac_info(const _Charac_info& other)
    : fields_(other.fields_) {}

// IDA_EA: 0x869DA62
// IDA_NAME: _ZN12_Charac_infoaSERKS_
// DEMANGLED: _Charac_info::operator=(_Charac_info const&)
_Charac_info& _Charac_info::operator=(const _Charac_info& other) {
    if (this != &other) {
        fields_ = other.fields_;
    }
    return *this;
}

// IDA_EA: 0x868745E
// IDA_NAME: _ZN5CUser19_processLevelRewardEjjc
// DEMANGLED: CUser::_processLevelReward(unsigned int,unsigned int,char)
int CUser::_processLevelReward(unsigned int charac_level,
                               unsigned int charac_no,
                               char job) {
    CLevelRewardTableMgr* manager = CLevelRewardTableMgr::GetInstance();
    CLevelRewardTable* table = 0;
    int result = manager->IsOn() ^ 1;
    if (static_cast<unsigned char>(result)) {
        return result;
    }

    table = manager->GetProperLevelRewardTableR();
    if (!table) {
        return 0;
    }

    result = manager->IsChallengingJob(job) ^ 1;
    if (static_cast<unsigned char>(result)) {
        return result;
    }

    const int job_index = table->GetProperJobIndex(job);
    const int level_index = table->GetProperLevelIndex(charac_level);
    if (level_index < 0 || job_index < 0) {
        return level_index;
    }

    const int reward_item_no = table->GetRewardItemNo(job_index, level_index);
    const int item_count = table->GetItemCount(job_index, level_index);
    if (item_count < 0) {
        return item_count;
    }

    char title[21];
    char text[256];
    std::strncpy(title, table->GetMailTitle(level_index), 0x14u);
    std::strncpy(text, table->GetMailString(level_index), 0xFFu);

    Inven_Item item;
    int money = 0;
    if (reward_item_no) {
        if (reward_item_no == 1) {
            item.item_id_ = 1;
            item.item_category_ = 2;
            item.set_add_info(item_count);
            item.avatar_select_ = 1;
            item.ResetItemAttr();
        } else {
            CItem* item_proto = G_CDataManager()->find_item(reward_item_no);
            if (!item_proto) {
                return 0;
            }
            item.item_id_ = reward_item_no;
            item_proto->vtable_->make_item(item_proto, item);
            item.set_add_info(item_count);
        }
    } else {
        money = item_count;
    }

    const int server_group = GetServerGroup();
    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        static_cast<unsigned int>(money),
        charac_no,
        text,
        static_cast<int>(std::strlen(text)),
        0,
        static_cast<ENUM_SERVER_GROUP>(server_group),
        false,
        false);
}

// IDA_EA: 0x864A5AA
// IDA_NAME: _ZN5CUser24_processUnlimitChallengeEjc
// DEMANGLED: CUser::_processUnlimitChallenge(unsigned int,char)
int CUser::_processUnlimitChallenge(unsigned int charac_no, char job) {
    CLevelRewardTableMgr* manager = CLevelRewardTableMgr::GetInstance();
    int result = reinterpret_cast<int>(manager);
    if (!manager) {
        return result;
    }

    result = manager->IsOn() ^ 1;
    if (static_cast<unsigned char>(result)) {
        return result;
    }

    CEnvironment* environment = G_CEnvironment();
    const int server_id = environment->get_server_group();
    result = manager->IsOnServer(server_id) ^ 1;
    if (static_cast<unsigned char>(result)) {
        return result;
    }

    result = manager->IsHaveStartLevelReward() ^ 1;
    if (static_cast<unsigned char>(result)) {
        return result;
    }

    return _processLevelReward(1u, charac_no, job);
}

// IDA_EA: 0x864A432
// IDA_NAME: _ZN5CUser18_onCreateCharacterEjPcc
// DEMANGLED: CUser::_onCreateCharacter(unsigned int,char *,char)
int CUser::_onCreateCharacter(unsigned int charac_no,
                              char* charac_name,
                              char job) {
    (void)charac_name;
    CSystemTime* system_time = GlobalData::system_time();
    if (job == oldsrv::config::kCharacterJobPriest && system_time &&
        system_time->getCurSec() <= 1228338000) {
        Inven_Item item;
        item.item_id_ = 7449;
        CDataManager* data_manager = G_CDataManager();
        CItemList* item_list = data_manager->get_item_list();
        item_list->create_item(static_cast<ENUM_ITEM_CREATE_TYPE>(3), item, 1);
        if (item.item_id_ == -1) {
            return -1;
        }

        char title[29];
        char text[256];
        const char* title_src =
            g_scriptStringManager_.findString(4, "game_server_msg_56", 0);
        std::strncpy(title, title_src, 0x14u);
        const char* text_src =
            g_scriptStringManager_.findString(4, "game_server_msg_57", 0);
        std::strncpy(text, text_src, 0xFFu);
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            title,
            item,
            0,
            charac_no,
            text,
            static_cast<int>(std::strlen(text)),
            0,
            static_cast<ENUM_SERVER_GROUP>(0),
            false,
            false);
    }
    return _processUnlimitChallenge(charac_no, job);
}

// IDA_EA: 0x8649C6A
// IDA_NAME: _ZN5CUser13SetCharacInfoEiPc
// DEMANGLED: CUser::SetCharacInfo(int,char *)
//
// Inter_LoginInfo supplies SIG_LOGIN_DATA::charac_view at +0xE0.  The old
// body deliberately does not clamp its count here: the DB producer owns that
    // validation and the loop consumes the native record stride.
void CUser::SetCharacInfo(int charac_count, char* charac_view_data) {
    const SIG_LOGIN_DATA::CharacViewEntry* const entries =
        reinterpret_cast<const SIG_LOGIN_DATA::CharacViewEntry*>(
            charac_view_data);
    const unsigned int current_sec = GlobalData::system_time()->getCurSec();

    // IDA allocates this exact 0x70C DB payload, clears it, and only sends it
    // if a separately populated count becomes nonzero.  No path in this
    // function populates that payload; retaining it here preserves the old
    // no-send state instead of fabricating an avatar-item DB request.
    unsigned char avatar_delete_payload[0x70Cu];
    std::memset(avatar_delete_payload, 0, sizeof(avatar_delete_payload));

    for (int index = 0; index < charac_count; ++index) {
        const SIG_LOGIN_DATA::CharacViewEntry& entry = entries[index];
        _Charac_info character;
        character.Reset();

        character.fields_.character_no_ =
            static_cast<int32_t>(entry.charac_no);
        // The old call uses the 30-byte destination capacity; the serialized
        // identity itself owns a 20-byte NUL-terminated character name.
        std::strncpy(character.fields_.character_name_,
                     entry.identity.character_name,
                     sizeof(character.fields_.character_name_));
        character.fields_.job_ = static_cast<int8_t>(entry.identity.job);
        character.fields_.grow_type_.growth_type =
            static_cast<int8_t>(entry.identity.grow_type);
        character.fields_.level_ = entry.identity.level;

        // OFFICIAL_IDA_EVIDENCE: CUser::SetCharacInfo @0x8649C6A writes each
        // charac-view item id to CInventory::equipment_items_ first
        // (ida_full_restore CUser.cpp:4770..4771), then copies the same record
        // to AvatarVisibleData (4772..4777).  These are item-id-only selection
        // placeholders: do not manufacture category, GUID, lock, durability or
        // option state.  Signal 8 replaces the complete equipment table before
        // the selected character can enter the world or pass a persistence
        // fence.  The target 0828 record widens the original 0..11 loop by the
        // proven inserted weapon-avatar slot, so the semantic bound is the
        // complete character-view equipment range 0..12.
        Inven_Item* const equipment =
            character.fields_.inventory_.equipment_items_;
        CInventory::AvatarVisibleData* const visible =
            character.fields_.inventory_.avatar_visible_data_;
        for (unsigned int item_index = 0;
             item_index <
                 oldsrv::config::kCharacterViewEquipmentTypeCount;
             ++item_index) {
            const SIG_LOGIN_DATA::CharacViewEntry::VisibleItem& source =
                entry.visible_items[item_index];
            equipment[item_index].item_id_ =
                static_cast<int32_t>(source.item_id);
            visible[item_index].item_id = source.item_id;
            visible[item_index].avatar_expansion_0 = source.option;
            visible[item_index].avatar_expansion_1 = source.color;
        }

        character.fields_.character_view_state_ =
            (entry.request_rename_state == 1 ||
             entry.request_rename_state == 0xFFu)
                ? static_cast<uint32_t>(entry.request_rename_state)
                : 0u;
        character.fields_.create_time_.SetCreateTime(
            static_cast<long>(entry.create_time));
        character.fields_.chaos_die_time_ = entry.rename_expire_time;
        character.fields_.ghost_state_ =
            entry.visible_state > current_sec ? 1u : 0u;
        character.fields_.chaos_response_time_late_ = entry.visible_state;
        character.fields_.user_event_character_flag_ =
            (entry.identity.state_flags & 0x01u) != 0u;
        character.fields_.striker_skill_index_ = entry.identity.tag_skill;
        character.fields_.tag_character_no_ = entry.identity.tag_charac_no;

        character.fields_.visible_flag_.visible_flag =
            static_cast<uint8_t>(
            (character.fields_.visible_flag_.visible_flag | 0x01u) |
            ((entry.identity.state_flags & 0x10u) != 0u ? 0x02u : 0u));

        character.fields_.inventory_.SetInventoryMemory(&inventory_memory_);
        character.fields_.cargo_.SetInventoryMemory(&inventory_memory_);
        characters_.push_back(character);
    }

    // Signal 824 owns the asynchronous relationship preload.  The request is
    // pointer-free and goes only to NATIVE_0828_DB_Q; its Inter reply verifies
    // the new character-list session before writing any link slot state.
    RestoreLinkCharactersFromDatabase();

    const int32_t avatar_delete_count =
        *reinterpret_cast<const int32_t*>(avatar_delete_payload);
    if (avatar_delete_count > 0) {
        if (avatar_delete_count <= 113) {
            CStreamGuard stream_guard(
                GlobalData::stream_pool()->Acquire("user.cpp", 3251), true);
            *stream_guard << 51;
            *stream_guard << GetUID();
            stream_guard->put_binary(avatar_delete_payload,
                                     sizeof(avatar_delete_payload));
            GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, stream_guard);
        } else {
            cMyTrace trace("void CUser::SetCharacInfo(int, char*)", 3247, 5);
            trace("SetCharacInfo AVATAR INFO PROBLEM items<%d> user_no<%s> "
                  "characnum<%d>",
                  avatar_delete_count,
                  NumberToString(static_cast<unsigned int>(get_acc_id()), 0),
                  charac_count);
        }
    }
}

// IDA_EA: 0x864A186
// IDA_NAME: _ZN5CUser12CreateCharacEP17SIG_CREATE_CHARAC
// DEMANGLED: CUser::CreateCharac(SIG_CREATE_CHARAC *)
int CUser::CreateCharac(SIG_CREATE_CHARAC* create_payload) {
    SIG_CREATE_CHARAC& payload = *create_payload;
    _Charac_info charac_info;
    charac_info.Reset();

    charac_info.fields_.character_no_ =
        static_cast<int32_t>(payload.character_no);
    std::strncpy(charac_info.fields_.character_name_,
                 payload.character_name,
                 sizeof(charac_info.fields_.character_name_));
    charac_info.fields_.village_ =
        payload.initial_village ? payload.initial_village : 1;
    charac_info.fields_.pvp_village_ = 2;
    charac_info.fields_.village_state_ = 0;
    charac_info.fields_.previous_village_ = 0;
    charac_info.fields_.job_ = payload.job;

    CDataManager* const data_manager = G_CDataManager();
    CCharacList* const character_table =
        data_manager ? data_manager->get_character_list() : 0;
    if (character_table) {
        const int job_index = static_cast<int>(payload.job);
        charac_info.fields_.additional_info_ =
            character_table->characters_[job_index].base_status_[0];
    }

    charac_info.fields_.level_ = 1;
    charac_info.fields_.experience_ = 0;
    charac_info.fields_.grow_type_.growth_type = 0;
    charac_info.fields_.stamina_ = 100;
    charac_info.fields_.create_time_.SetCurTime();
    charac_info.fields_.fatigue_ = 0;
    charac_info.fields_.maximum_fatigue_ =
        static_cast<uint16_t>(DEFAULT_MAX_FATIGUE);
    charac_info.fields_.fatigue_grownup_buff_ = 0;
    charac_info.fields_.premium_fatigue_ = 0;
    charac_info.fields_.forbid_user_to_play_ = 0;
    charac_info.fields_.forbid_remaining_seconds_ = 0;
    charac_info.fields_.help_abuse_ratio_ = 0;
    charac_info.fields_.help_abuse_experience_ = 0;
    charac_info.fields_.power_war_.byte_02 = 0;
    charac_info.fields_.skill_tree_index_ = -1;
    charac_info.fields_.user_event_character_flag_ =
        payload.user_event_character_flag != 0;

    charac_info.fields_.inventory_.SetInventoryMemory(&inventory_memory_);
    charac_info.fields_.cargo_.SetInventoryMemory(&inventory_memory_);

    characters_.push_back(charac_info);
    const int new_slot = static_cast<int>(characters_.size()) - 1;
    CUser_history_log(this)->CharacAdd(new_slot);
    character_view_.enableSaveCharacView();
    _onCreateCharacter(payload.character_no,
                       payload.character_name,
                       payload.job);
    return 0;
}

// IDA_EA: 0x864AC1A
// IDA_NAME: _ZN5CUser19setCharacInfoDetailEijP11CHARAC_DATA
// DEMANGLED: CUser::setCharacInfoDetail(int,unsigned int,CHARAC_DATA *)
int CUser::setCharacInfoDetail(int charac_slot,
                               unsigned int charac_no,
                               CHARAC_DATA* data) {
    if (charac_slot < 0 || static_cast<int>(characters_.size()) <= charac_slot) {
        return 0;
    }

    _Charac_info& character =
        characters_[static_cast<std::vector<_Charac_info>::size_type>(
            charac_slot)];
    _Charac_info::Fields& fields = character.fields_;
    _Charac_info::Fields& objects = character.fields_;
    CHARAC_DATA& source = *data;
    if (static_cast<unsigned int>(fields.character_no_) != charac_no) {
        return 0;
    }

    fields.character_no_ = source.character_no;
    if (G_GameWorld()->GetChannelType() != 7) {
        std::strncpy(fields.character_name_, source.character_name,
                     sizeof(fields.character_name_));
    }

    fields.village_ = static_cast<uint8_t>(source.village);
    if (source.village == 7) {
        fields.village_ = 2;
        fields.village_state_ = 7;
    }
    if (source.village == 8) {
        const uint8_t previous = static_cast<uint8_t>(source.village);
        fields.village_ = static_cast<uint8_t>(source.previous_village);
        fields.previous_village_ = previous;
    }
    if (source.village >= 12) {
        const int loaded_village_index =
            static_cast<unsigned char>(source.village);
        Village* const loaded_village =
            G_GameWorld()->GetVillage(loaded_village_index);
        if (!loaded_village || loaded_village->area_count <= 0) {
            fields.village_ = 11;
        }
    }

    fields.pvp_village_ = 2;
    fields.job_ = source.job;
    fields.level_ = source.level;
    fields.experience_ = source.experience;
    fields.grow_type_.growth_type = source.grow_type;
    fields.stamina_ = source.hp;
    objects.additional_info_ = source.additional_info;

    const int first_grow_type =
        (static_cast<signed char>(2 * fields.grow_type_.growth_type)) >> 5;
    const int second_grow_type =
        (static_cast<signed char>(16 * fields.grow_type_.growth_type)) >> 4;
    global::tools::InitializeCharacStat::initCharacStat(
        objects.additional_info_,
        static_cast<unsigned int>(source.character_no),
        static_cast<int>(static_cast<signed char>(fields.job_)),
        fields.level_,
        second_grow_type,
        first_grow_type);

    fields.fatigue_ = source.fatigue;
    fields.maximum_fatigue_ = source.maximum_fatigue;
    fields.fatigue_grownup_buff_ = source.fatigue_grownup_buff;

    CEventBase* repeat_event =
        GlobalData::s_event_manager
            ? GlobalData::s_event_manager->GetRepeatEvent(2)
            : 0;
    if (repeat_event) {
        if (repeat_event->IsEventing(0)) {
            CMaxFatigueFactorEvent* max_fatigue_event =
                static_cast<CMaxFatigueFactorEvent*>(repeat_event);
            fields.maximum_fatigue_ = static_cast<uint16_t>(
                DEFAULT_MAX_FATIGUE *
                max_fatigue_event->GetMaxFatigueFactor() / 100);
        } else {
            fields.maximum_fatigue_ =
                static_cast<uint16_t>(DEFAULT_MAX_FATIGUE);
        }
    }

    CEventBase* weekend_event =
        GlobalData::s_event_manager
            ? GlobalData::s_event_manager->GetRepeatEvent(87)
            : 0;
    if (weekend_event && weekend_event->IsEventing(0)) {
        fields.maximum_fatigue_ = static_cast<uint16_t>(
            fields.maximum_fatigue_ +
            G_CDataManager()
                ->get_server_parameter_script()
                ->get_weekend_bonus_fatigue());
    }

    if (source.fatigue_battery_charging < 0) {
        fields.fatigue_battery_charging_ = 0;
        cMyTrace trace(
            "bool CUser::setCharacInfoDetail(int, unsigned int, CHARAC_DATA*)",
            3993,
            0);
        trace("FATIGUE BATTERY Error : Char No(%d) %d",
              source.character_no,
              source.fatigue_battery_charging);
    } else {
        fields.fatigue_battery_charging_ =
            source.fatigue_battery_charging;
    }

    fields.help_abuse_ratio_ = 0;
    fields.help_abuse_experience_ = 0;
    fields.last_play_tick_ = source.last_play_time;
    {
        cMyTrace trace(
            "bool CUser::setCharacInfoDetail(int, unsigned int, CHARAC_DATA*)",
            4016,
            0);
        trace("CharLastPlayTick char(%d) t(%d)",
              charac_no,
              static_cast<int32_t>(source.power_war_last_play_time));
    }
    fields.power_war_last_play_time_ = source.power_war_last_play_time;
    fields.guild_id_ = source.guild_id;
    fields.guild_right_ = source.guild_right;
    fields.premium_fatigue_ = source.premium_fatigue;
    fields.maximum_premium_fatigue_ = source.maximum_premium_fatigue;
    if (source.member_flag) {
        fields.member_key_ = static_cast<uint32_t>(source.character_no);
    }

    fields.create_time_.SetCreateTime(source.create_time);
    fields.tutorial_flag_ = source.tutorial_flag;
    std::memcpy(fields.escalade_tutorial_flag_,
                source.escalade_tutorial_flag,
                sizeof(fields.escalade_tutorial_flag_));
    fields.reliable_person_.InitTradeGold(source.trade_gold_total,
                                          source.trade_gold_total_billion,
                                          source.trade_gold_daily);
    {
        cMyTrace trace(
            "bool CUser::setCharacInfoDetail(int, unsigned int, CHARAC_DATA*)",
            4055,
            0);
        trace("TRADE_RESTIRCTION charac_name(%s), use_gold(%u)",
              fields.character_name_,
              source.trade_gold_daily);
    }

    fields.chaos_point_ = source.chaos_point;
    fields.chaos_experience_ = source.chaos_experience;
    fields.chaos_mode_count_ = source.chaos_mode_count;
    fields.chaos_kill_count_ = source.chaos_kill_count;
    fields.chaos_die_count_ = source.chaos_die_count;
    fields.chaos_die_time_ = source.chaos_die_time;
    fields.assault_count_ = source.assault_count;
    fields.chaos_kill_time_ = source.chaos_kill_time;
    fields.open_coin_flag_ = static_cast<uint8_t>(source.open_flag);
    fields.luck_point_ = source.luck_point;
    fields.dungeon_play_count_ = source.dungeon_play_count;
    fields.expert_job_ = source.expert_job;
    fields.expert_job_experience_ = source.expert_job_experience;
    fields.power_war_point_ = source.power_war_point;
    fields.power_war_assault_count_ = source.power_war_assault_count;
    fields.power_war_victory_count_ = source.power_war_victory_count;
    fields.skill_tree_index_ = source.skill_tree_index;
    fields.exp_equip_slot_state_ = source.add_slot_flag;
    fields.event_character_grow_type_ = source.event_character_grow_type;
    fields.member_dungeon_flag_ = source.member_dungeon_flag != 0;
    fields.member_bonus_fatigue_ = source.member_bonus_fatigue;
    fields.guild_secede_ = source.guild_secede;
    fields.last_play_dungeon_index_ = source.last_play_dungeon_index;
    fields.equipslot_flag_ = source.add_equipslot_flag;
    fields.channel_equipslot_switch_ = source.channel_equipslot_switch;
    fields.expand_equipslot_switch_ = source.expand_equipslot_switch;
    fields.event_bring_up_reward_flag_ =
        source.event_bring_up_reward_flag;
    fields.event_bring_up_material_total_ =
        source.event_bring_up_material_count;
    fields.growth_power_reward_ = source.growth_power_reward;
    fields.chaos_response_time_late_ = source.chaos_response_time;
    return 1;
}

// IDA_EA: 0x864A63A
// IDA_NAME: _ZN5CUser12DeleteCharacEj
// DEMANGLED: CUser::DeleteCharac(unsigned int)
CCharacterView* CUser::DeleteCharac(unsigned int charac_no) {
    std::vector<_Charac_info>::iterator erase_it = characters_.begin();
    bool found = false;

    for (int slot = 0; slot < static_cast<int>(characters_.size()); ++slot) {
        _Charac_info& character =
            characters_[static_cast<std::vector<_Charac_info>::size_type>(
                slot)];
        _Charac_info::Fields& fields = character.fields_;
        if (static_cast<unsigned int>(fields.character_no_) == charac_no) {
            HistoryLog::WriteDeleteCh(
                history_stream_,
                fields.character_name_,
                fields.job_,
                fields.level_,
                fields.grow_type_.growth_type);
            CUser_history_log(this)->CharacDel(slot);
            found = true;
        }

        if (fields.tag_character_no_ == charac_no) {
            fields.tag_character_no_ = 0;
        }

        if (!found) {
            ++erase_it;
        }
    }

    if (found) {
        characters_.erase(erase_it);
        CalcurateManageLevel();
    }

    return character_view_.enableSaveCharacView();
}
