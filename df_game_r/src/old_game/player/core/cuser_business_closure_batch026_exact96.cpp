#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include "core/config/env/environment.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/n_quest/quest_parameter_script.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/mail/mail_trade_value_types.h"
#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_game/activity/schedule/event_messages.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/mall/delivery_runtime.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/progression/career/premium_lethe_manager.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/charac_expand_data_mgr.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <list>
#include <stdint.h>

// IDA_EA: 0x822FBE6
// IDA_NAME: _ZN5CUser20IsWaitingStateCharacEj
// DEMANGLED: CUser::IsWaitingStateCharac(unsigned int)
bool CUser::IsWaitingStateCharac(unsigned int charac_no) {
    return mercenary_info_.IsWaitingStateCharac(charac_no);
}

// IDA_EA: 0x828B5C6
// IDA_NAME: _ZN5CUser11DeleteCargoEv
// DEMANGLED: CUser::DeleteCargo(void)
void* CUser::DeleteCargo() {
    return account_cargo_.Clear();
}

// IDA_EA: 0x831D528
// IDA_NAME: _ZN5CUser19setBreakAwayRewardsERK18STBreakAwayRewards
// DEMANGLED: CUser::setBreakAwayRewards(STBreakAwayRewards const&)
STBreakAwayRewards* CUser::setBreakAwayRewards(
    const STBreakAwayRewards& rewards) {
    break_away_rewards_ = rewards;
    return &break_away_rewards_;
}

// IDA_EA: 0x84EC5E6
// IDA_NAME: _ZN5CUser27UpdateCharacNameInMercenaryEjRKSs
// DEMANGLED: CUser::UpdateCharacNameInMercenary(unsigned int,std::string const&)
char* CUser::UpdateCharacNameInMercenary(
    unsigned int charac_no,
    const std::string& charac_name) {
    return mercenary_info_.UpdateCharacName(charac_no, charac_name);
}

// IDA_EA: 0x84EC8EA
// IDA_NAME: _ZN5CUser19getUserInfo4AntibotEv
// DEMANGLED: CUser::getUserInfo4Antibot(void)
char* CUser::getUserInfo4Antibot() {
    return antibot_user_info_.character_name;
}

// IDA_EA: 0x867D86E
// IDA_NAME: _ZN5CUser21SetAccountCargoDBDataEP10Inven_Itemjj
// DEMANGLED: CUser::SetAccountCargoDBData(Inven_Item *,unsigned int,unsigned int)
CAccountCargo* CUser::SetAccountCargoDBData(Inven_Item* items,
                                            unsigned int money,
                                            unsigned int capacity) {
    return account_cargo_.SetDBData(this, items, money, capacity);
}

// IDA_EA: 0x867F720
// IDA_NAME: _ZN5CUser21resetHumanCertifyDataEv
// DEMANGLED: CUser::resetHumanCertifyData(void)
void* CUser::resetHumanCertifyData() {
    return std::memset(clean_pad_captcha_text_, 0, 5u);
}

// IDA_EA: 0x8688328
// IDA_NAME: _ZN5CUser17AddSecuRewardItemEh
// DEMANGLED: CUser::AddSecuRewardItem(unsigned char)
int CUser::AddSecuRewardItem(unsigned char reward_type) {
    return reward_type;
}

// IDA_EA: 0x868E692
// IDA_NAME: _ZN5CUser23Return_Mercenary_CharacEjR25competition_reward_result
// DEMANGLED: CUser::Return_Mercenary_Charac(unsigned int,competition_reward_result &)
int CUser::Return_Mercenary_Charac(
    unsigned int charac_no,
    competition_reward_result& result) {
    return mercenary_info_.ReturnCharac(this, charac_no, result);
}

// IDA_EA: 0x868E6C0
// IDA_NAME: _ZN5CUser28Competition_Mercenary_CharacEjcc
// DEMANGLED: CUser::Competition_Mercenary_Charac(unsigned int,char,char)
bool CUser::Competition_Mercenary_Charac(unsigned int charac_no,
                                         char competition_type,
                                         char competition_count) {
    return mercenary_info_.CompetitionCharac(
        this, charac_no, competition_type, competition_count);
}

// IDA_EA: 0x868E544
// IDA_NAME: _ZN5CUser22SetCharacMercenaryInfoEiP21CHARAC_LOAD_MERCENARY
// DEMANGLED: CUser::SetCharacMercenaryInfo(int,CHARAC_LOAD_MERCENARY *)
bool CUser::SetCharacMercenaryInfo(int count,
                                   CHARAC_LOAD_MERCENARY* data) {
    int index = 0;
    for (; index < count && index <= 35; ++index) {
        mercenary_info_.AddCharac(data[index]);
    }
    return index < count && index <= 35;
}

// IDA_EA: 0x84EC8AC
// IDA_NAME: _ZN5CUser20fillUserInfo4AntibotEv
// DEMANGLED: CUser::fillUserInfo4Antibot(void)
char* CUser::fillUserInfo4Antibot() {
    antibot_user_info_.sentinel = -1;
    return std::strncpy(antibot_user_info_.character_name,
                        getCurCharacName(),
                        20u);
}

// IDA_EA: 0x84EC9EC
// IDA_NAME: _ZN5CUser17SetUserDetailInfoEPcS0_S0_
// DEMANGLED: CUser::SetUserDetailInfo(char *,char *,char *)
char* CUser::SetUserDetailInfo(char* first, char* second, char* third) {
    std::strncpy(user_name_, first, sizeof(user_name_));
    std::strncpy(ssn_, second, sizeof(ssn_));
    return std::strncpy(user_email_, third, sizeof(user_email_));
}

// IDA_EA: 0x8649BFC
// IDA_NAME: _ZN5CUser12set_acc_infoEjPKc
// DEMANGLED: CUser::set_acc_info(unsigned int,char const*)
int CUser::set_acc_info(unsigned int account_id, const char* account_name) {
    account_id_ = static_cast<int32_t>(account_id);
    const int copied = static_cast<int>(
        std::min<std::size_t>(std::strlen(account_name), 20u));
    char* const destination = account_name_;
    std::strncpy(destination, account_name, 20u);
    destination[copied] = 0;
    return copied;
}

// IDA_EA: 0x868B480
// IDA_NAME: _ZN5CUser22setCharacSlotCountInfoEhh
// DEMANGLED: CUser::setCharacSlotCountInfo(unsigned char,unsigned char)
int CUser::setCharacSlotCountInfo(unsigned char slot_effect_count,
                                  unsigned char slot_limit) {
    if (slot_effect_count && slot_limit) {
        setSlotEffectCount(slot_effect_count);
        return static_cast<int>(
            reinterpret_cast<intptr_t>(setCharacSlotLimit(slot_limit)));
    }
    const unsigned char default_count =
        static_cast<unsigned char>(getCharacSlotCountDefaultInfo());
    setSlotEffectCount(default_count);
    return static_cast<int>(
        reinterpret_cast<intptr_t>(setCharacSlotLimit(default_count)));
}

// IDA_EA: 0x8691CD0
// IDA_NAME: _ZN5CUser24getGrowthWeaponEventdataEv
// DEMANGLED: CUser::getGrowthWeaponEventdata(void)
char* CUser::getGrowthWeaponEventdata() {
    return reinterpret_cast<char*>(&growth_weapon_event_);
}

// IDA_EA: 0x8691DBC
// IDA_NAME: _ZN5CUser26getGrowthCreatureEventdataEv
// DEMANGLED: CUser::getGrowthCreatureEventdata(void)
GrowthCreatureEvent* CUser::getGrowthCreatureEventdata() {
    return &growth_creature_event_;
}

// IDA_EA: 0x8691E5C
// IDA_NAME: _ZN5CUser32getCharacLevelUpGift_AccountOnceEv
// DEMANGLED: CUser::getCharacLevelUpGift_AccountOnce(void)
CharacLevelUpGiftAccountOnce*
CUser::getCharacLevelUpGift_AccountOnce() {
    return &levelup_gifts_;
}

// IDA_EA: 0x86D20B8
// IDA_NAME: _ZN5CUser34ResetDailyCharacExpandDataMidnightEv
// DEMANGLED: CUser::ResetDailyCharacExpandDataMidnight(void)
bool CUser::ResetDailyCharacExpandDataMidnight() {
    return charac_expand_data_.ResetDailyMidnight();
}

// IDA_EA: 0x86D20D0
// IDA_NAME: _ZN5CUser26ResetDailyCharacExpandDataEv
// DEMANGLED: CUser::ResetDailyCharacExpandData(void)
bool CUser::ResetDailyCharacExpandData() {
    return charac_expand_data_.ResetDaily();
}

// IDA_EA: 0x866C196
// IDA_NAME: _ZN5CUser16resetGuildDBInfoEv
// DEMANGLED: CUser::resetGuildDBInfo(void)
void* CUser::resetGuildDBInfo() {
    resetCharGuildInfo();
    return std::memset(&guild_info_, 0, sizeof(guild_info_));
}

// IDA_EA: 0x86769B4
// IDA_NAME: _ZN5CUser11GetRentInfoER18SIG_LOAD_INVENTORY
// DEMANGLED: CUser::GetRentInfo(SIG_LOAD_INVENTORY &)
int CUser::GetRentInfo(SIG_LOAD_INVENTORY& inventory) {
    GetPcRoomRentInfo(&inventory.pc_room_rent);
    return GetDimensionRentInfo(&inventory.dimension_rent);
}

// IDA_EA: 0x8646912
// IDA_NAME: _ZN5CUser23OnItemUnlockWaitTimeoutEii
// DEMANGLED: CUser::OnItemUnlockWaitTimeout(int,int)
int CUser::OnItemUnlockWaitTimeout(int, int) {
    item_lock::CItemLock* const lock =
        reinterpret_cast<item_lock::CItemLock*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    return lock->OnItemUnlockWaitTimeout(this) ^ 1;
}

// IDA_EA: 0x8646D4A
// IDA_NAME: _ZN5CUser10DoItemLockEii
// DEMANGLED: CUser::DoItemLock(int,int)
int CUser::DoItemLock(int inventory_type, int slot) {
    item_lock::CItemLock* const lock =
        reinterpret_cast<item_lock::CItemLock*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    return lock->DoItemLock(this, inventory_type, slot);
}

// IDA_EA: 0x8646D82
// IDA_NAME: _ZN5CUser12DoItemUnlockEii
// DEMANGLED: CUser::DoItemUnlock(int,int)
int CUser::DoItemUnlock(int inventory_type, int slot) {
    item_lock::CItemLock* const lock =
        reinterpret_cast<item_lock::CItemLock*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    return lock->DoItemUnlock(this, inventory_type, slot);
}

// IDA_EA: 0x8646DBA
// IDA_NAME: _ZN5CUser18DoItemUnlockCancelEii
// DEMANGLED: CUser::DoItemUnlockCancel(int,int)
int CUser::DoItemUnlockCancel(int inventory_type, int slot) {
    item_lock::CItemLock* const lock =
        reinterpret_cast<item_lock::CItemLock*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    return lock->DoItemUnlockCancel(this, inventory_type, slot);
}

// IDA_EA: 0x8646942
// IDA_NAME: _ZNK5CUser13CheckItemLockEii
// DEMANGLED: CUser::CheckItemLock(int,int)const
bool CUser::CheckItemLock(int inventory_type, int slot) const {
    if (!getCurCharacR())
        return false;
    const Inven_Item* const item =
        getCurCharacInvenR()->GetInvenRef(inventory_type, slot);
    if (!item)
        return true;
    const stAmplifyOption_t* const amplify = &item->amplify_option_;
    const unsigned char item_lock =
        static_cast<unsigned char>(amplify->GetLock());
    if (!item_lock)
        return false;
    const item_lock::CItemLock* const lock =
        reinterpret_cast<const item_lock::CItemLock*>(
            GetCharacExpandDataR(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    return lock->CheckItemLock(item_lock);
}

// IDA_EA: 0x86469F0
// IDA_NAME: _ZNK5CUser22CheckItemLockSlotSpaceEii
// DEMANGLED: CUser::CheckItemLockSlotSpace(int,int)const
bool CUser::CheckItemLockSlotSpace(int inventory_type, int slot) const {
    if (!getCurCharacR())
        return false;
    const Inven_Item* const item = GetCurCharacInventoryRef(
        static_cast<unsigned short>(inventory_type),
        static_cast<unsigned short>(slot));
    if (!item)
        return false;
    const stAmplifyOption_t* const amplify = &item->amplify_option_;
    const unsigned char item_lock =
        static_cast<unsigned char>(amplify->GetLock());
    if (!item_lock)
        return false;
    const item_lock::CItemLock* const lock =
        reinterpret_cast<const item_lock::CItemLock*>(
            GetCharacExpandDataR(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    return lock->CheckItemLock(item_lock);
}

// IDA_EA: 0x86624F8
// IDA_NAME: _ZN5CUser15send_RedeemInfoEv
// DEMANGLED: CUser::send_RedeemInfo(void)
int CUser::send_RedeemInfo() {
    Redeem_Item::CRedeemItem* const redeem =
        reinterpret_cast<Redeem_Item::CRedeemItem*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(6)));
    return redeem->Send_RedeemList(this);
}

// IDA_EA: 0x8662528
// IDA_NAME: _ZN5CUser16send_MissionListEv
// DEMANGLED: CUser::send_MissionList(void)
int CUser::send_MissionList() {
    CMissionList_Charac* const missions =
        reinterpret_cast<CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    return missions->Send_MissionList(*this);
}

// IDA_EA: 0x8662558
// IDA_NAME: _ZN5CUser19SetMissionParameterEii
// DEMANGLED: CUser::SetMissionParameter(int,int)
charac_expand::CData* CUser::SetMissionParameter(int mission_kind,
                                                 int value) {
    if (!isGMUser())
        return 0;
    CMissionList_Charac* const missions =
        reinterpret_cast<CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    return missions->GM_SetMissionValue(mission_kind,
                                        static_cast<short>(value));
}

// IDA_EA: 0x866259E
// IDA_NAME: _ZN5CUser17ResetDailyMissionEv
// DEMANGLED: CUser::ResetDailyMission(void)
void CUser::ResetDailyMission() {
    CMissionList_Charac* const missions =
        reinterpret_cast<CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    missions->Reset_DailyMission();
}

// IDA_EA: 0x86625C8
// IDA_NAME: _ZN5CUser22UpdateMissionConditionERSt6vectorIsSaIsEERS0_IiSaIiEE
// DEMANGLED: CUser::UpdateMissionCondition(std::vector<short>&,std::vector<int>&)
char CUser::UpdateMissionCondition(std::vector<short>& mission_ids,
                                   std::vector<int>& values) {
    CMissionList_Charac* const missions =
        reinterpret_cast<CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    return missions->Update_RecvPacket_event(*this, mission_ids, values);
}

// IDA_EA: 0x8662606
// IDA_NAME: _ZN5CUser12ClearMissionEs
// DEMANGLED: CUser::ClearMission(short)
char CUser::ClearMission(short mission_kind) {
    if (!isGMUser())
        return 1;
    CMissionList_Charac* const missions =
        reinterpret_cast<CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    return missions->ClearMission(*this, mission_kind);
}

// IDA_EA: 0x866265A
// IDA_NAME: _ZN5CUser16ResetMissionListEv
// DEMANGLED: CUser::ResetMissionList(void)
char CUser::ResetMissionList() {
    if (!isGMUser())
        return 1;
    CMissionList_Charac* const missions =
        reinterpret_cast<CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    return missions->ResetMissionList(*this);
}

// IDA_EA: 0x86626A0
// IDA_NAME: _ZN5CUser16ClearMissionKindEs
// DEMANGLED: CUser::ClearMissionKind(short)
int CUser::ClearMissionKind(short mission_kind) {
    CMissionList_Charac* const missions =
        reinterpret_cast<CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    return missions->ClearMissionKind(*this, mission_kind);
}

// IDA_EA: 0x86626E0
// IDA_NAME: _ZNK5CUser33get_pvp_WinningRate_relateMissionEv
// DEMANGLED: CUser::get_pvp_WinningRate_relateMission(void)const
int CUser::get_pvp_WinningRate_relateMission() const {
    const CMissionList_Charac* const missions =
        reinterpret_cast<const CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    return missions->getWinningRate();
}

// IDA_EA: 0x866270A
// IDA_NAME: _ZNK5CUser18has_within_MissionEv
// DEMANGLED: CUser::has_within_Mission(void)const
int CUser::has_within_Mission() const {
    const CMissionList_Charac* const missions =
        reinterpret_cast<const CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    return missions->getIndex_byKind(27);
}

// IDA_EA: 0x866273C
// IDA_NAME: _ZN5CUser14Remove_MissionEs
// DEMANGLED: CUser::Remove_Mission(short)
charac_expand::CData* CUser::Remove_Mission(short mission_kind) {
    CMissionList_Charac* const missions =
        reinterpret_cast<CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    return missions->Remove_Mission(mission_kind);
}

// IDA_EA: 0x86627E0
// IDA_NAME: _ZNK5CUser29is_clear_stealingSkillMissionEv
// DEMANGLED: CUser::is_clear_stealingSkillMission(void)const
int CUser::is_clear_stealingSkillMission() const {
    const CMissionList_Charac* const missions =
        reinterpret_cast<const CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    return missions->isClearStealingSkillMission();
}

// IDA_EA: 0x866280A
// IDA_NAME: _ZN5CUser19Update_ComboMissionEis
// DEMANGLED: CUser::Update_ComboMission(int,short)
int CUser::Update_ComboMission(int mission_index, short value) {
    CMissionList_Charac* const missions =
        reinterpret_cast<CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    missions->Update_ComboMission_Event(*this, mission_index, value);
    return send_MissionList();
}

// IDA_EA: 0x8662ABA
// IDA_NAME: _ZN5CUser14set_skill_slotEPcS0_i
// DEMANGLED: CUser::set_skill_slot(char *,char *,int)
void* CUser::set_skill_slot(char* first, char* second, int size) {
    return getCurCharacSkillW()->set_skill_slot(first, second, size);
}

// IDA_EA: 0x864DEEA
// IDA_NAME: _ZN5CUser12SetPvpRecordER13PvpResultType
// DEMANGLED: CUser::SetPvpRecord(PvpResultType &)
int CUser::SetPvpRecord(PvpResultType& result) {
    getPVPResultW()->Set(result);
    return 1;
}

// IDA_EA: 0x865CF84
// IDA_NAME: _ZN5CUser15update_pvp_rankERK13PvpResultType
// DEMANGLED: CUser::update_pvp_rank(PvpResultType const&)
int CUser::update_pvp_rank(const PvpResultType& result) {
    getPVPResultRefW()->AddNewResult(result);
    return 1;
}

// IDA_EA: 0x865CFAC
// IDA_NAME: _ZN5CUser20update_old_pvp_pointEv
// DEMANGLED: CUser::update_old_pvp_point(void)
int CUser::update_old_pvp_point() {
    const int old_point = getPVPResultRefR()->values[2];
    getPVPResultRefW()->values[3] = old_point;
    return old_point;
}

// IDA_EA: 0x864E0B6
// IDA_NAME: _ZNK5CUser25getCurCharacInvenCheckSumEv
// DEMANGLED: CUser::getCurCharacInvenCheckSum(void)const
int CUser::getCurCharacInvenCheckSum() const {
    if (!getCurCharacR())
        return 0;
    return getCurCharacInvenR()->makeCheckSum(
        getPVPResultR()->values[22],
        static_cast<int>(accept_random_nonce_));
}

// IDA_EA: 0x864E114
// IDA_NAME: _ZNK5CUser25getCurCharacSkillCheckSumEv
// DEMANGLED: CUser::getCurCharacSkillCheckSum(void)const
int CUser::getCurCharacSkillCheckSum() const {
    if (!getCurCharacR())
        return 0;
    return getCurCharacSkillR()->makeCheckSum(
        static_cast<int>(accept_random_nonce_));
}

// IDA_EA: 0x867B4C4
// IDA_NAME: _ZN5CUser11ClearSkillsEv
// DEMANGLED: CUser::ClearSkills(void)
int CUser::ClearSkills() {
    if (game_master_mode_ != 1 || !getCurCharacR())
        return 0;
    getCurCharacSkillW()->debugCommandResetSkill();
    return 1;
}

// IDA_EA: 0x86550EE
// IDA_NAME: _ZN5CUser19defaultSkillCommandEv
// DEMANGLED: CUser::defaultSkillCommand(void)
int CUser::defaultSkillCommand() {
    std::map<int, STSkillCommandData>* const commands = getSkillCommand();
    if (!commands || commands->empty())
        return 0;
    commands->clear();
    enableSaveSkill();
    return 1;
}

// IDA_EA: 0x865CEC0
// IDA_NAME: _ZN5CUser31find_pvp_masterid_walkingout_meEj
// DEMANGLED: CUser::find_pvp_masterid_walkingout_me(unsigned int)
bool CUser::find_pvp_masterid_walkingout_me(unsigned int master_id) {
    if (pvp_master_ids_.empty())
        return false;
    return pvp_master_ids_.find(master_id) != pvp_master_ids_.end();
}

// IDA_EA: 0x864FAC4
// IDA_NAME: _ZN5CUser17IsGuildSkillLearnEi
// DEMANGLED: CUser::IsGuildSkillLearn(int)
int CUser::IsGuildSkillLearn(int skill_index) {
    const STGuildDBInfoOnly* const guild = GetGuildDBInfo();
    for (int index = 0; index < guild->skill_count; ++index) {
        if (guild->skills[index].skill_id == skill_index)
            return static_cast<unsigned char>(guild->skills[index].level);
    }
    return 0;
}

// IDA_EA: 0x866C414
// IDA_NAME: _ZN5CUser16IsSubGuildMasterEv
// DEMANGLED: CUser::IsSubGuildMaster(void)
int CUser::IsSubGuildMaster() {
    for (int index = 0; index <= 4; ++index) {
        if (guild_info_.sub_guild_master_character_no[index] ==
            static_cast<uint32_t>(getCurCharacNo()))
            return 1;
    }
    return 0;
}

// IDA_EA: 0x868FC46
// IDA_NAME: _ZN5CUser12initAllSkillEv
// DEMANGLED: CUser::initAllSkill(void)
char CUser::initAllSkill() {
    // The two migration gates at 0x868FC46/0x868FDBC both read the real
    // _Charac_info +0x7b last-play tick.  The typed field is the same physical
    // record, so this preserves the old timestamp comparison without a raw
    // character-layout projection at either consumer.
    if (getCurCharacR()->fields_.last_play_tick_ >= 1355259600u) {
        return 0;
    }

    getCurCharacSkillW()->clear_all_skills(ENUM_SKILL_TREE_0);
    getCurCharacSkillW()->set_parent(this);
    getCurCharacSkillW()->InitSkill(false, ENUM_SKILL_TREE_0);

    getCurCharacSkillW()->clear_all_skills(ENUM_SKILL_TREE_1);
    getCurCharacSkillW()->set_parent(this);
    getCurCharacSkillW()->InitSkill(true, ENUM_SKILL_TREE_1);

    if (!isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(33))) {
        return 0;
    }

    CPremiumLetheManager* const lethe_manager =
        G_CGameManager()->GetPremiumLetheManager();
    if (!lethe_manager) {
        return 0;
    }

    SetOneDayLetheFlag(CPremiumLetheManager::NOT_USE_LETHE_STATE);
    lethe_manager->BackupSkill(this, ENUM_SKILL_TREE_0);
    SetOneDayLetheFlag2ND(CPremiumLetheManager::NOT_USE_LETHE_STATE);
    return static_cast<char>(
        lethe_manager->BackupSkill(this, ENUM_SKILL_TREE_1));
}

// IDA_EA: 0x868FDBC
// IDA_NAME: _ZN5CUser11initTPSkillEv
// DEMANGLED: CUser::initTPSkill(void)
void CUser::initTPSkill() {
    if (getCurCharacR()->fields_.last_play_tick_ < 1342645200u) {
        getCurCharacSkillW()->initialize_sfp_skill();
        send_skill_info();
    }
}

// IDA_EA: 0x82302AC
// IDA_NAME: _ZN5CUser18GetItemTimeCheckerEjttj
// DEMANGLED: CUser::GetItemTimeChecker(unsigned int,unsigned short,unsigned short,unsigned int)
int CUser::GetItemTimeChecker(unsigned int current_tick,
                              unsigned short x,
                              unsigned short y,
                              unsigned int interval) {
    const short signed_x = static_cast<short>(x);
    const short signed_y = static_cast<short>(y);

    if (current_tick - item_time_check_.last_tick > interval) {
        item_time_check_.distance_x = 0;
        item_time_check_.distance_y = 0;
        item_time_check_.sample_count = 0;
        item_time_check_.last_tick = current_tick;
        item_time_check_.minimum_x = signed_x;
        item_time_check_.minimum_y = signed_y;
        item_time_check_.maximum_x = signed_x;
        item_time_check_.maximum_y = signed_y;
    } else {
        ++item_time_check_.sample_count;
        if (item_time_check_.minimum_x - signed_x > 0)
            item_time_check_.minimum_x = signed_x;
        if (item_time_check_.minimum_y - signed_y > 0)
            item_time_check_.minimum_y = signed_y;
        if (item_time_check_.maximum_x - signed_x < 0)
            item_time_check_.maximum_x = signed_x;
        if (item_time_check_.maximum_y - signed_y < 0)
            item_time_check_.maximum_y = signed_y;
        const short delta_x = static_cast<short>(
            item_time_check_.maximum_x - item_time_check_.minimum_x);
        const short delta_y = static_cast<short>(
            item_time_check_.maximum_y - item_time_check_.minimum_y);
        item_time_check_.distance_x =
            delta_x < 0 ? static_cast<short>(-delta_x) : delta_x;
        item_time_check_.distance_y =
            delta_y < 0 ? static_cast<short>(-delta_y) : delta_y;
    }
    return item_time_check_.sample_count;
}

// IDA_EA: 0x84ECBE0
// IDA_NAME: _ZNK5CUser24getCharacLevelByCharacNoEi
// DEMANGLED: CUser::getCharacLevelByCharacNo(int)const
int CUser::getCharacLevelByCharacNo(int charac_no) const {
    for (std::vector<_Charac_info>::const_iterator it = characters_.begin();
         it != characters_.end();
         ++it) {
        if (it->fields_.character_no_ == charac_no)
            return it->fields_.level_;
    }
    return -1;
}

// IDA_EA: 0x865CCC8
// IDA_NAME: _ZN5CUser20add_guild_point_itemEv
// DEMANGLED: CUser::add_guild_point_item(void)
void CUser::add_guild_point_item() {
    const int probability = G_CDataManager()
                                ->GetGuildParameterScriptForBuild()
                                ->guild_point_acquirement_ratio;
    if (static_cast<int>(get_rand_int(10000)) < probability)
        add_inventory_item(0xc84u);
}

// IDA_EA: 0x866E3A8
// IDA_NAME: _ZN5CUser21compute_level_penaltyEi
// DEMANGLED: CUser::compute_level_penalty(int)
int CUser::compute_level_penalty(int level_difference) {
    const QuestParameterScript* const quest_parameters =
        G_CDataManager()->get_quest_parameter_script();
    if (level_difference > 6 && level_difference <= 11)
        return quest_parameters->green_level_penalty;
    if (level_difference <= 11)
        return 100;
    return quest_parameters->grey_level_penalty;
}

// IDA_EA: 0x8671FDE
// IDA_NAME: _ZN5CUser17isSuccessCompoundEi
// DEMANGLED: CUser::isSuccessCompound(int)
bool CUser::isSuccessCompound(int probability) {
    if (getRecipeProbability() == 1)
        return true;
    if (getRecipeProbability() == 2)
        return false;
    return probability == 0 ||
           static_cast<int>(get_rand_int(100)) < probability;
}

// IDA_EA: 0x864FC3E
// IDA_NAME: _ZN5CUser19CheckLimitMiniumAgeEiRi
// DEMANGLED: CUser::CheckLimitMiniumAge(int,int &)
int CUser::CheckLimitMiniumAge(int minimum_age, int& unique_id) {
    if (GetAge() - 1 >= minimum_age &&
        (GetAge() - 1 != minimum_age || IsOverBirthDay()))
        return 1;
    unique_id = static_cast<unsigned short>(get_unique_id());
    return 0;
}

// IDA_EA: 0x866C21E
// IDA_NAME: _ZN5CUser18IsAddableGuildFundEi
// DEMANGLED: CUser::IsAddableGuildFund(int)
int CUser::IsAddableGuildFund(int amount) {
    if (guild_info_.guild_level <= 15u) {
        if (guild_info_.guild_fund + amount > 0x1312d00u)
            return 0;
    } else if (guild_info_.guild_fund + amount > 0x989680u) {
        return 0;
    }
    return 1;
}

// IDA_EA: 0x867F1A4
// IDA_NAME: _ZN5CUser14resetBlackListEv
// DEMANGLED: CUser::resetBlackList(void)
int CUser::resetBlackList() {
    const bool was_empty = black_list_.empty();
    if (!was_empty)
        black_list_.clear();
    return was_empty;
}

// IDA_EA: 0x867EE28
// IDA_NAME: _ZN5CUser14printBlackListEv
// DEMANGLED: CUser::printBlackList(void)
int CUser::printBlackList() {
    if (black_list_.empty())
        return 0;

    char blacklist_text[1024];
    std::memset(blacklist_text, 0, sizeof(blacklist_text));
    for (std::vector<unsigned int>::const_iterator it = black_list_.begin();
         it != black_list_.end(); ++it) {
        // The IDA body emits a leading comma for every element.  Appending at
        // the terminator retains that exact text without overlapping sprintf
        // source and destination buffers.
        std::sprintf(blacklist_text + std::strlen(blacklist_text),
                     ",%d",
                     static_cast<int>(*it));
    }
    std::sprintf(blacklist_text + std::strlen(blacklist_text), "\n");

    char log_path[256];
    std::memset(log_path, 0, sizeof(log_path));
    std::sprintf(log_path,
                 "./log/%s/BlackList",
                 G_CEnvironment()->get_file_name());
    CMyFileLog log("void CUser::printBlackList()", 32087);
    return log(log_path, "%s", blacklist_text);
}

// IDA_EA: 0x867EFAE
// IDA_NAME: _ZN5CUser19registerToBlackListEj
// DEMANGLED: CUser::registerToBlackList(unsigned int)
int CUser::registerToBlackList(unsigned int charac_no) {
    if (black_list_.size() <= 9u) {
        if (isBlackUser(charac_no) == 1)
            return 0;
        black_list_.push_back(charac_no);
        return 1;
    }

    LogManager::logFormat(1,
                          "user.cpp",
                          "bool CUser::registerToBlackList(unsigned int)",
                          32098,
                          "vt_black_list_.size() >= BLACKLIST_CNT_LIMIT");
    return 0;
}

// IDA_EA: 0x867F040
// IDA_NAME: _ZN5CUser17deleteToBlackListEj
// DEMANGLED: CUser::deleteToBlackList(unsigned int)
int CUser::deleteToBlackList(unsigned int charac_no) {
    if (black_list_.empty())
        return 0;

    for (std::vector<unsigned int>::iterator it = black_list_.begin();
         it != black_list_.end();
         ++it) {
        if (*it == charac_no) {
            black_list_.erase(it);
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x867F104
// IDA_NAME: _ZN5CUser11isBlackUserEj
// DEMANGLED: CUser::isBlackUser(unsigned int)
int CUser::isBlackUser(unsigned int charac_no) {
    if (black_list_.empty())
        return 0;

    for (std::vector<unsigned int>::const_iterator it = black_list_.begin();
         it != black_list_.end();
         ++it) {
        if (*it == charac_no)
            return 1;
    }
    return 0;
}

// IDA_EA: 0x864D90A
// IDA_NAME: _ZN5CUser21RemoveRestrictedGoodsEi
// DEMANGLED: CUser::RemoveRestrictedGoods(int)
bool CUser::RemoveRestrictedGoods(int item_id) {
    std::list<int>::iterator found =
        std::find(restricted_goods_.begin(), restricted_goods_.end(), item_id);
    if (found == restricted_goods_.end())
        return false;
    restricted_goods_.erase(found);
    return true;
}

// IDA_EA: 0x864F8EA
// IDA_NAME: _ZN5CUser26saveTaxMoneyForUpperMemberEi
// DEMANGLED: CUser::saveTaxMoneyForUpperMember(int)
char CUser::saveTaxMoneyForUpperMember(int tax_money) {
    char result = get_charac_memberkey() == 0;
    if (!result) {
        result = is_connect_upper_member();
        if (result && tax_money > 0)
            return add_member_pay_tex_money_to_upper(tax_money);
    }
    return result;
}

// IDA_EA: 0x86529CA
// IDA_NAME: _ZN5CUser21sendCharacLinkMessageEv
// DEMANGLED: CUser::sendCharacLinkMessage(void)
void CUser::sendCharacLinkMessage() {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 290);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x867B9DA
// IDA_NAME: _ZN5CUser17SendZeroExpPacketEv
// DEMANGLED: CUser::SendZeroExpPacket(void)
void CUser::SendZeroExpPacket() {
    PacketGuard packet;
    packet.put_header(0, 263);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x8686F58
// IDA_NAME: _ZN5CUser26SendUnlimitChallengePacketEv
// DEMANGLED: CUser::SendUnlimitChallengePacket(void)
void CUser::SendUnlimitChallengePacket() {
    PacketGuard packet;
    packet.put_header(0, 164);
    packet.put_byte(1);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x8690B58
// IDA_NAME: _ZN5CUser21SendManageLevelUpNotiEv
// DEMANGLED: CUser::SendManageLevelUpNoti(void)
void CUser::SendManageLevelUpNoti() {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 415);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x8690E36
// IDA_NAME: _ZN5CUser26SendEventMailAlaramTooltipEv
// DEMANGLED: CUser::SendEventMailAlaramTooltip(void)
void CUser::SendEventMailAlaramTooltip() {
    PacketGuard packet;
    packet.put_header(0, 429);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x86910B2
// IDA_NAME: _ZN5CUser17SendMaxEquipLevelEv
// DEMANGLED: CUser::SendMaxEquipLevel(void)
void CUser::SendMaxEquipLevel() {
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 438);
    packet.put_short(GetCurCharacMaxEquipLevel());
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x868F83C
// IDA_NAME: _ZN5CUser22sendNotiPcRoomPlayTimeEv
// DEMANGLED: CUser::sendNotiPcRoomPlayTime(void)
void CUser::sendNotiPcRoomPlayTime() {
    PacketGuard packet;
    packet.put_header(0, 424);
    packet.put_int(static_cast<int>(getTotalPcRoomPlayTime()));
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x868A9A4
// IDA_NAME: _ZN5CUser24sendReturnUserFirstLoginEv
// DEMANGLED: CUser::sendReturnUserFirstLogin(void)
void CUser::sendReturnUserFirstLogin() {
    if (!IsReturnUser() || !IsReturnUserFirstLogin())
        return;
    PacketGuard packet;
    packet.put_header(0, 444);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x8688F02
// IDA_NAME: _ZN5CUser26SendLimitItemUsageNotiInfoEiii
// DEMANGLED: CUser::SendLimitItemUsageNotiInfo(int,int,int)
void CUser::SendLimitItemUsageNotiInfo(int item_id, int limit, int used) {
    PacketGuard packet;
    packet.put_header(0, 287);
    packet.put_int(item_id);
    packet.put_int(limit);
    packet.put_int(used);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x867C844
// IDA_NAME: _ZN5CUser19SendMoneyFullReasonE22ENUM_MONEY_FULL_REASONmm
// DEMANGLED: CUser::SendMoneyFullReason(ENUM_MONEY_FULL_REASON,unsigned long,unsigned long)
int CUser::SendMoneyFullReason(ENUM_MONEY_FULL_REASON reason,
                               unsigned long current_money,
                               unsigned long requested_money) {
    PacketGuard packet;
    packet.put_header(0, 51);
    packet.put_byte(static_cast<int>(reason));
    packet.put_int(static_cast<int>(current_money));
    packet.put_int(static_cast<int>(requested_money));
    packet.finalize(true);
    return Send(packet);
}

// IDA_EA: 0x867BD84
// IDA_NAME: _ZN5CUser22SendCmdPeerErrorPacketE14ENUM_CMDPACKET22ENUM_PEER_REQUEST_TYPEhR11PacketGuard
// DEMANGLED: CUser::SendCmdPeerErrorPacket(ENUM_CMDPACKET,ENUM_PEER_REQUEST_TYPE,unsigned char,PacketGuard &)
int CUser::SendCmdPeerErrorPacket(ENUM_CMDPACKET command,
                                  ENUM_PEER_REQUEST_TYPE request_type,
                                  unsigned char error,
                                  PacketGuard& packet) {
    packet.clear();
    packet.put_header(1, static_cast<int>(command));
    packet.put_byte(0);
    packet.put_byte(error);
    packet.put_byte(static_cast<int>(request_type));
    if (static_cast<int>(request_type) == 4)
        packet.put_int(0);
    packet.finalize(true);
    return Send(packet);
}

// IDA_EA: 0x867BE2E
// IDA_NAME: _ZN5CUser22SendCmdPeerErrorPacketE14ENUM_CMDPACKET22ENUM_PEER_REQUEST_TYPEh
// DEMANGLED: CUser::SendCmdPeerErrorPacket(ENUM_CMDPACKET,ENUM_PEER_REQUEST_TYPE,unsigned char)
int CUser::SendCmdPeerErrorPacket(ENUM_CMDPACKET command,
                                  ENUM_PEER_REQUEST_TYPE request_type,
                                  unsigned char error) {
    PacketGuard packet;
    return SendCmdPeerErrorPacket(command, request_type, error, packet);
}

// IDA_EA: 0x867F1FE
// IDA_NAME: _ZN5CUser21StartRecvEventWaitingEv
// DEMANGLED: CUser::StartRecvEventWaiting(void)
int CUser::StartRecvEventWaiting() {
    received_event_ = 0;
    return G_TimerQueue()->InsertTimer(
        2,
        GetUID(),
        103,
        30,
        static_cast<unsigned short>(get_unique_id()),
        0);
}

// IDA_EA: 0x868856C
// IDA_NAME: _ZN5CUser23GetPowerWarPartyVictoryEv
// DEMANGLED: CUser::GetPowerWarPartyVictory(void)
int CUser::GetPowerWarPartyVictory() {
    CParty* const party = GetParty();
    if (party)
        return static_cast<unsigned short>(party->getStraightVictories());
    return static_cast<unsigned short>(getStraightVictories());
}

// IDA_EA: 0x8689D06
// IDA_NAME: _ZN5CUser19CalLevelUpItemCheckEm
// DEMANGLED: CUser::CalLevelUpItemCheck(unsigned long)
bool CUser::CalLevelUpItemCheck(unsigned long item_id) {
    const int level = get_charac_level();
    if (item_id == 2675388ul)
        return level >= oldsrv::config::kCharacterLevelMinimum &&
               level <= oldsrv::config::kCharacterLevelBeforeMaximum;
    if (item_id > 2675388ul) {
        if (item_id != 10000915ul && item_id != 690000097ul)
            return false;
        return level >= oldsrv::config::kCharacterLevelMinimum &&
               level <= oldsrv::config::kCharacterLevelBeforeMaximum;
    }
    return item_id == 8049ul && level > 18 && level <= 59;
}

// IDA_EA: 0x8691E70
// IDA_NAME: _ZN5CUser31processReturnUserQuestAutoClearEv
// DEMANGLED: CUser::processReturnUserQuestAutoClear(void)
int CUser::processReturnUserQuestAutoClear() {
    const int should_clear = IsReturnUser() && IsReturnUserFirstLogin();
    if (should_clear)
        return questAutoClear(get_charac_level() - 1);
    return should_clear;
}

// IDA_EA: 0x866CAC6
// IDA_NAME: _ZN5CUser13CheckQuestMapEi
// DEMANGLED: CUser::CheckQuestMap(int)
int CUser::CheckQuestMap(int quest_index) {
    if (!getCurCharacR())
        return 0;
    return getCurCharacQuestR()->get_rescue_questmap(quest_index);
}

// IDA_EA: 0x866D934
// IDA_NAME: _ZN5CUser17CheckDungeonClearEi
// DEMANGLED: CUser::CheckDungeonClear(int)
int CUser::CheckDungeonClear(int dungeon_index) {
    if (!getCurCharacR())
        return 0;
    return getCurCharacQuestR()->item_reward_quest(dungeon_index);
}

// IDA_EA: 0x86780FA
// IDA_NAME: _ZN5CUser15AddDungeonClearEii
// DEMANGLED: CUser::AddDungeonClear(int,int)
int CUser::AddDungeonClear(int dungeon_index, int dungeon_diff) {
    const WongWork::CDungeonClear* const clear =
        reinterpret_cast<const WongWork::CDungeonClear*>(
            getDungeonClearRefR());
    if (clear->getClearedDungeonDiff(
            static_cast<short>(dungeon_index)) >= dungeon_diff)
        return 0;
    getDungeonClearRefW()->addClearedDungeon(
        static_cast<short>(dungeon_index),
        static_cast<char>(dungeon_diff));
    return 1;
}

// IDA_EA: 0x868ACFE
// IDA_NAME: _ZN5CUser13setQuestPointEi
// DEMANGLED: CUser::setQuestPoint(int)
void CUser::setQuestPoint(int quest_point) {
    CQuestShop* const shop = static_cast<CQuestShop*>(
        GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
    if (shop) {
        shop->setQp(quest_point);
        shop->sendCharacQp(this, 0);
    }
}

// IDA_EA: 0x868AD52
// IDA_NAME: _ZN5CUser13getQuestPointEv
// DEMANGLED: CUser::getQuestPoint(void)
int CUser::getQuestPoint() {
    CQuestShop* const shop = static_cast<CQuestShop*>(
        GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
    return shop ? shop->getQp() : 0;
}

// IDA_EA: 0x868AF2C
// IDA_NAME: _ZN5CUser20sendCharacQuestPieceEv
// DEMANGLED: CUser::sendCharacQuestPiece(void)
void CUser::sendCharacQuestPiece() {
    CQuestShop* const shop = static_cast<CQuestShop*>(
        GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
    if (shop)
        shop->sendCharacQuestPiece(this);
}

// IDA_EA: 0x868B00A
// IDA_NAME: _ZN5CUser23SendItemMakingSkillInfoEv
// DEMANGLED: CUser::SendItemMakingSkillInfo(void)
int CUser::SendItemMakingSkillInfo() {
    const CItemMakingSkill* const item_making =
        reinterpret_cast<const CItemMakingSkill*>(
            GetCharacExpandDataR(static_cast<ENUM_CHARAC_EXPAND_TYPE>(10)));
    return item_making ? item_making->Send_ItemMakingSkillList(this) : 0;
}

// IDA_EA: 0x868B044
// IDA_NAME: _ZN5CUser21send_clear_quest_listEv
// DEMANGLED: CUser::send_clear_quest_list(void)
void CUser::send_clear_quest_list() {
    PacketGuard packet;
    packet.put_header(0, 355);
    packet.put_int(30000);
    const WongWork::STClearedQuest* const cleared =
        getCurCharacQuestR()->clear_quest.getClearedQuest();
    packet.put_binary(reinterpret_cast<const char*>(cleared), 30000);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x8688D26
// IDA_NAME: _ZN5CUser16GetItemLimitInfoEi
// DEMANGLED: CUser::GetItemLimitInfo(int)
LimitItemUsage* CUser::GetItemLimitInfo(int item_id) {
    for (std::vector<LimitItemUsage>::iterator it = limit_item_usage_.begin();
         it != limit_item_usage_.end();
         ++it) {
        if (it->item_id == item_id)
            return &*it;
    }
    return 0;
}

// IDA_EA: 0x867C80E
// IDA_NAME: _ZN5CUser13IsEquipAvatarEv
// DEMANGLED: CUser::IsEquipAvatar(void)
int CUser::IsEquipAvatar() {
    if (!getCurCharacR())
        return 0;
    return getCurCharacInvenR()->IsEquipAvatar();
}

// IDA_EA: 0x86910A6
// IDA_NAME: _ZNK5CUser27GetCharacCashAttendanceInfoER20SIG_ATTENDANCE_EVENT
// DEMANGLED: CUser::GetCharacCashAttendanceInfo(SIG_ATTENDANCE_EVENT &)const
SIG_ATTENDANCE_EVENT* CUser::GetCharacCashAttendanceInfo(
    SIG_ATTENDANCE_EVENT& attendance) const {
    attendance.can_attend = 0;
    return &attendance;
}

// IDA_EA: 0x868F900
// IDA_NAME: _ZN5CUser20loadInformNoticeFlagEPc
// DEMANGLED: CUser::loadInformNoticeFlag(char *)
void* CUser::loadInformNoticeFlag(char* flags) {
    void* result = resetInformNoticeFlag();
    if (!flags)
        return result;
    std::memcpy(inform_notice_.flags, flags, sizeof(inform_notice_.flags));
    return reinterpret_cast<void*>(
        static_cast<intptr_t>(sendInformNoticeFlag()));
}

// IDA_EA: 0x868F9E8
// IDA_NAME: _ZN5CUser21checkInformNoticeFlagEjj
// DEMANGLED: CUser::checkInformNoticeFlag(unsigned int,unsigned int)
bool CUser::checkInformNoticeFlag(unsigned int bit,
                                  unsigned int byte_index) {
    if (bit > 7u || byte_index > 15u)
        return false;
    const unsigned char value = inform_notice_.flags[byte_index];
    return ((value >> bit) & 1u) == 0u;
}

// IDA_EA: 0x84ED04E
// IDA_NAME: _ZN5CUser35SetCurCharacUsedGiftFatigueQuantityEj
// DEMANGLED: CUser::SetCurCharacUsedGiftFatigueQuantity(unsigned int)
CUser* CUser::SetCurCharacUsedGiftFatigueQuantity(unsigned int quantity) {
    if (used_gift_fatigue_ == quantity)
        return reinterpret_cast<CUser*>(
            static_cast<uintptr_t>(used_gift_fatigue_));
    used_gift_fatigue_ = quantity;
    return SetChangedGiftFatigueQuantity(true);
}

// IDA_EA: 0x84ED086
// IDA_NAME: _ZN5CUser37ResetCurCharacUsedGiftFatigueQuantityEv
// DEMANGLED: CUser::ResetCurCharacUsedGiftFatigueQuantity(void)
CUser* CUser::ResetCurCharacUsedGiftFatigueQuantity() {
    if (!used_gift_fatigue_)
        return 0;
    used_gift_fatigue_ = 0;
    return SetChangedGiftFatigueQuantity(true);
}

// IDA_EA: 0x868DDC2
// IDA_NAME: _ZNK5CUser19GetAuraAvatarOptionER18stAuraAvatarOption
// DEMANGLED: CUser::GetAuraAvatarOption(stAuraAvatarOption &)const
char CUser::GetAuraAvatarOption(stAuraAvatarOption& option) const {
    const _Charac_info* const charac = getCurCharacR();
    if (!charac)
        return 1;
    option = charac->fields_.aura_avatar_option_;
    return static_cast<char>(charac->fields_.aura_avatar_option_.values[2]);
}

// IDA_EA: 0x866463C
// IDA_NAME: _ZN5CUser20eventDeliveryBuyItemEiRK10Inven_Item
// DEMANGLED: CUser::eventDeliveryBuyItem(int,Inven_Item const&)
int CUser::eventDeliveryBuyItem(int item_id, const Inven_Item& item) {
    BuyItemDeliveryMsg message;
    message.setBaseData(2,
                        get_acc_id(),
                        getCurCharacNo(),
                        GetServerGroupToPvP());
    message.item_id = item_id;
    message.item_count = static_cast<int>(
        reinterpret_cast<intptr_t>(&item));
    return GlobalData::s_event_manager->eventDeliveryNotify(
        message.delivery_type, message);
}
