#include "old_game/player/core/cuser.h"

#include <cstring>


// IDA_EA: 0x816EE1E
// IDA_NAME: _ZN5CUser17getGarenaAuthDataEv
// DEMANGLED: CUser::getGarenaAuthData(void)
Taiwan::GarenaAuthData* CUser::getGarenaAuthData() {
    return &garena_auth_data_;
}

// IDA_EA: 0x822FD20
// IDA_NAME: _ZN5CUser15getSecurityCardEv
// DEMANGLED: CUser::getSecurityCard(void)
WongWork::CSecurityCard* CUser::getSecurityCard() {
    return &security_card_;
}

// IDA_EA: 0x82300AC
// IDA_NAME: _ZN5CUser14SetServerGroupE17ENUM_SERVER_GROUP
// DEMANGLED: CUser::SetServerGroup(ENUM_SERVER_GROUP)
CUser* CUser::SetServerGroup(ENUM_SERVER_GROUP server_group) {
    server_group_ = static_cast<int>(server_group);
    return this;
}

// IDA_EA: 0x82300D0
// IDA_NAME: _ZNK5CUser7IsStateE8ch_state
// DEMANGLED: CUser::IsState(ch_state)const
bool CUser::IsState(ch_state state) const {
    return static_cast<int>(state_) == static_cast<int>(state);
}

// IDA_EA: 0x82604EC
// IDA_NAME: _ZNK5CUser21getCurCharacQuestRefREv
// DEMANGLED: CUser::getCurCharacQuestRefR(void)const
UserQuest* CUser::getCurCharacQuestRefR() const {
    return &const_cast<CUser*>(this)->quest_;
}

// IDA_EA: 0x831D4DE
// IDA_NAME: _ZN5CUser23setBreakAwayRewardOrderE28ENUM_BREAK_AWAY_REWARD_ORDER
// DEMANGLED: CUser::setBreakAwayRewardOrder(ENUM_BREAK_AWAY_REWARD_ORDER)
CUser* CUser::setBreakAwayRewardOrder(ENUM_BREAK_AWAY_REWARD_ORDER order) {
    break_away_reward_order_ = static_cast<int>(order);
    return this;
}

// IDA_EA: 0x831D4F0
// IDA_NAME: _ZN5CUser23getBreakAwayRewardOrderEv
// DEMANGLED: CUser::getBreakAwayRewardOrder(void)
int CUser::getBreakAwayRewardOrder() {
    return break_away_reward_order_;
}

// IDA_EA: 0x831D4FE
// IDA_NAME: _ZN5CUser22getBreakAwayAccureCeraEv
// DEMANGLED: CUser::getBreakAwayAccureCera(void)
int CUser::getBreakAwayAccureCera() {
    return break_away_accure_cera_;
}

// IDA_EA: 0x831D50C
// IDA_NAME: _ZN5CUser22setBreakAwayRewardDataEb
// DEMANGLED: CUser::setBreakAwayRewardData(bool)
CUser* CUser::setBreakAwayRewardData(bool loaded) {
    break_away_reward_data_ = loaded;
    return this;
}

// IDA_EA: 0x832815A
// IDA_NAME: _ZN5CUser22ClearChildrensDayEventEv
// DEMANGLED: CUser::ClearChildrensDayEvent(void)
void CUser::ClearChildrensDayEvent() {
    if (childrens_day_event_.target_mask_ == 0u) {
        return;
    }

    childrens_day_event_.target_mask_ = 0u;
    for (int index = 0; index <= 9; ++index) {
        childrens_day_event_.target_item_slots_[index] = -1;
        childrens_day_event_.target_item_periods_[index] = -1;
    }
}

// IDA_EA: 0x83281BE
// IDA_NAME: _ZN5CUser25SetChildrensDayEventValueEiii
// DEMANGLED: CUser::SetChildrensDayEventValue(int,int,int)
void CUser::SetChildrensDayEventValue(int index,
                                      int item_slot,
                                      int item_period) {
    if (index > 9) {
        return;
    }

    childrens_day_event_.target_mask_ |=
        static_cast<unsigned char>(1u << index);
    childrens_day_event_.target_item_slots_[index] = item_slot;
    childrens_day_event_.target_item_periods_[index] = item_period;
}

// IDA_EA: 0x8328220
// IDA_NAME: _ZN5CUser30IsOKChildrensDayEventConditionEv
// DEMANGLED: CUser::IsOKChildrensDayEventCondition(void)
bool CUser::IsOKChildrensDayEventCondition() {
    return childrens_day_event_.target_mask_ == 0xffu;
}

// IDA_EA: 0x8328240
// IDA_NAME: _ZN5CUser34GetChildrensDayEventTargetItemSlotEi
// DEMANGLED: CUser::GetChildrensDayEventTargetItemSlot(int)
int CUser::GetChildrensDayEventTargetItemSlot(int index) {
    if (index > 9) {
        return -1;
    }
    return childrens_day_event_.target_item_slots_[index];
}

// IDA_EA: 0x8328262
// IDA_NAME: _ZN5CUser36GetChildrensDayEventTargetItemPeriodEi
// DEMANGLED: CUser::GetChildrensDayEventTargetItemPeriod(int)
int CUser::GetChildrensDayEventTargetItemPeriod(int index) {
    if (index > 9) {
        return -1;
    }
    return childrens_day_event_.target_item_periods_[index];
}

// IDA_EA: 0x84B431E
// IDA_NAME: _ZN5CUser22SetGameMasterCharacterEP20CGameMasterCharacter
// DEMANGLED: CUser::SetGameMasterCharacter(CGameMasterCharacter *)
CUser* CUser::SetGameMasterCharacter(CGameMasterCharacter* character) {
    game_master_character_ = character;
    return this;
}

// IDA_EA: 0x84B9A82
// IDA_NAME: _ZN5CUser16setHackUserTimerEv
// DEMANGLED: CUser::setHackUserTimer(void)
CUser* CUser::setHackUserTimer() {
    hack_user_timer_ = 1u;
    return this;
}

// IDA_EA: 0x84B9A92
// IDA_NAME: _ZN5CUser15setHackUserFlagEv
// DEMANGLED: CUser::setHackUserFlag(void)
CUser* CUser::setHackUserFlag() {
    hack_user_flag_ = 1u;
    return this;
}

// IDA_EA: 0x84EC60E
// IDA_NAME: _ZN5CUser31CheckLoadRentalInfoFromExchangeEv
// DEMANGLED: CUser::CheckLoadRentalInfoFromExchange(void)
int CUser::CheckLoadRentalInfoFromExchange() {
    return load_rental_info_from_exchange_;
}

// IDA_EA: 0x84EC65E
// IDA_NAME: _ZN5CUser19getBreakAwayRewardsEv
// DEMANGLED: CUser::getBreakAwayRewards(void)
STBreakAwayRewards* CUser::getBreakAwayRewards() {
    return &break_away_rewards_;
}

// IDA_EA: 0x84EC722
// IDA_NAME: _ZN5CUser31setOnlinePreliminaryServerGroupE17ENUM_SERVER_GROUP
// DEMANGLED: CUser::setOnlinePreliminaryServerGroup(ENUM_SERVER_GROUP)
CUser* CUser::setOnlinePreliminaryServerGroup(
    ENUM_SERVER_GROUP server_group) {
    online_preliminary_server_group_ = static_cast<int>(server_group);
    return this;
}

// IDA_EA: 0x84EC940
// IDA_NAME: _ZN5CUser25setTotalHackUserTradeGoldEy
// DEMANGLED: CUser::setTotalHackUserTradeGold(unsigned long long)
unsigned long long CUser::setTotalHackUserTradeGold(unsigned long long gold) {
    total_hack_user_trade_gold_ = gold;
    return gold;
}

// IDA_EA: 0x84ECA54
// IDA_NAME: _ZN5CUser13SetDailyBadgeEPs
// DEMANGLED: CUser::SetDailyBadge(short *)
void CUser::SetDailyBadge(short* badges) {
    for (int index = 0; index <= 2; ++index) {
        daily_badges_[index] = static_cast<uint16_t>(badges[index]);
    }
}

// IDA_EA: 0x84ECA90
// IDA_NAME: _ZN5CUser11SetSchoolNoEj
// DEMANGLED: CUser::SetSchoolNo(unsigned int)
CUser* CUser::SetSchoolNo(unsigned int school_no) {
    character_statistic_.school_no_ = school_no;
    return this;
}

// IDA_EA: 0x84ECAA2
// IDA_NAME: _ZN5CUser9SetRatingEf
// DEMANGLED: CUser::SetRating(float)
CUser* CUser::SetRating(float rating) {
    std::memcpy(&character_statistic_.rating_storage_, &rating,
                sizeof(rating));
    return this;
}

// IDA_EA: 0x84ECAB4
// IDA_NAME: _ZN5CUser13SetSchoolNameEPKc
// DEMANGLED: CUser::SetSchoolName(char const*)
std::string& CUser::SetSchoolName(const char* school_name) {
    return character_string_.assign(school_name);
}

// IDA_EA: 0x84ECE20
// IDA_NAME: _ZN5CUser24setCharacLinkMessageFlagEb
// DEMANGLED: CUser::setCharacLinkMessageFlag(bool)
CUser* CUser::setCharacLinkMessageFlag(bool enabled) {
    charac_link_message_flag_ = enabled;
    return this;
}

// IDA_EA: 0x84ECE3C
// IDA_NAME: _ZN5CUser14SetGuildDBInfoER17STGuildDBInfoOnly
// DEMANGLED: CUser::SetGuildDBInfo(STGuildDBInfoOnly &)
void* CUser::SetGuildDBInfo(STGuildDBInfoOnly& info) {
    return std::memcpy(&guild_info_, &info, sizeof(guild_info_));
}

// IDA_EA: 0x84ECE64
// IDA_NAME: _ZN5CUser15ChangeGuildNameEPc
// DEMANGLED: CUser::ChangeGuildName(char *)
void* CUser::ChangeGuildName(char* guild_name) {
    std::memset(guild_info_.guild_name, 0, sizeof(guild_info_.guild_name));
    return std::memcpy(guild_info_.guild_name, guild_name, 0x16u);
}

// IDA_EA: 0x84ECEAC
// IDA_NAME: _ZN5CUser16SetGuildAgitFlagEb
// DEMANGLED: CUser::SetGuildAgitFlag(bool)
CUser* CUser::SetGuildAgitFlag(bool enabled) {
    guild_info_.guild_agit_flag = enabled;
    return this;
}

// IDA_EA: 0x850D522
// IDA_NAME: _ZN5CUser14getGmQuestFlagEv
// DEMANGLED: CUser::getGmQuestFlag(void)
int CUser::getGmQuestFlag() {
    return gm_quest_flag_;
}

// IDA_EA: 0x855105E
// IDA_NAME: _ZN5CUser22GetGameMasterCharacterEv
// DEMANGLED: CUser::GetGameMasterCharacter(void)
CGameMasterCharacter* CUser::GetGameMasterCharacter() {
    return game_master_character_;
}

// IDA_EA: 0x8568E0C
// IDA_NAME: _ZN5CUser13GetDailyBadgeEPs
// DEMANGLED: CUser::GetDailyBadge(short *)
void CUser::GetDailyBadge(short* badges) {
    for (int index = 0; index <= 2; ++index) {
        badges[index] = static_cast<short>(daily_badges_[index]);
    }
}

// IDA_EA: 0x8568E48
// IDA_NAME: _ZN5CUser13GetDailyBadgeE15ENUM_BADGE_TYPE
// DEMANGLED: CUser::GetDailyBadge(ENUM_BADGE_TYPE)
int CUser::GetDailyBadge(ENUM_BADGE_TYPE badge_type) {
    const int index = static_cast<int>(badge_type);
    return daily_badges_[index];
}

// IDA_EA: 0x8568E5E
// IDA_NAME: _ZN5CUser13AddDailyBadgeE15ENUM_BADGE_TYPE
// DEMANGLED: CUser::AddDailyBadge(ENUM_BADGE_TYPE)
void CUser::AddDailyBadge(ENUM_BADGE_TYPE badge_type) {
    const int index = static_cast<int>(badge_type);
    ++daily_badges_[index];
}

// IDA_EA: 0x857F406
// IDA_NAME: _ZN5CUser21set_public_ip_addressEi
// DEMANGLED: CUser::set_public_ip_address(int)
CUser* CUser::set_public_ip_address(int address) {
    public_ip_address_ = static_cast<uint32_t>(address);
    return this;
}

// IDA_EA: 0x8595060
// IDA_NAME: _ZN5CUser10backupSeedEv
// DEMANGLED: CUser::backupSeed(void)
CUser* CUser::backupSeed() {
    backup_seed_ = accept_random_nonce_;
    return this;
}

// IDA_EA: 0x8595078
// IDA_NAME: _ZN5CUser10SetPowerUpEb
// DEMANGLED: CUser::SetPowerUp(bool)
CUser* CUser::SetPowerUp(bool enabled) {
    power_up_ = enabled;
    return this;
}

// IDA_EA: 0x85DFA56
// IDA_NAME: _ZN5CUser30get_guildwar_point_per_pvpplayEv
// DEMANGLED: CUser::get_guildwar_point_per_pvpplay(void)
int CUser::get_guildwar_point_per_pvpplay() {
    return guild_pvp_point_;
}

// IDA_EA: 0x863BE74
// IDA_NAME: _ZN5CUser20GetCheckConnIntervalEv
// DEMANGLED: CUser::GetCheckConnInterval(void)
int CUser::GetCheckConnInterval() {
    return static_cast<int>(connection_check_interval_);
}

// IDA_EA: 0x863BE82
// IDA_NAME: _ZN5CUser21SetResumeChecksumTimeEi
// DEMANGLED: CUser::SetResumeChecksumTime(int)
CUser* CUser::SetResumeChecksumTime(int resume_time) {
    resume_checksum_time_ = static_cast<uint32_t>(resume_time);
    return this;
}

// IDA_EA: 0x863BF46
// IDA_NAME: _ZN5CUser17incre_check_countEv
// DEMANGLED: CUser::incre_check_count(void)
CUser* CUser::incre_check_count() {
    ++connection_check_count_;
    return this;
}

// IDA_EA: 0x863BF62
// IDA_NAME: _ZN5CUser17get_connect_countEv
// DEMANGLED: CUser::get_connect_count(void)
int CUser::get_connect_count() {
    return connection_check_count_;
}

// IDA_EA: 0x863BF72
// IDA_NAME: _ZN5CUser19isSendMailCertifiedEv
// DEMANGLED: CUser::isSendMailCertified(void)
int CUser::isSendMailCertified() {
    return mail_send_certified_;
}

// IDA_EA: 0x863BF82
// IDA_NAME: _ZN5CUser23getHumanCertifyTimerKeyEv
// DEMANGLED: CUser::getHumanCertifyTimerKey(void)
int CUser::getHumanCertifyTimerKey() {
    return static_cast<int>(human_certify_timer_key_);
}

// IDA_EA: 0x863BF90
// IDA_NAME: _ZN5CUser11IsRecvEventEv
// DEMANGLED: CUser::IsRecvEvent(void)
int CUser::IsRecvEvent() {
    return received_event_;
}

// IDA_EA: 0x863BFA0
// IDA_NAME: _ZN5CUser13isARSUserKickEv
// DEMANGLED: CUser::isARSUserKick(void)
int CUser::isARSUserKick() {
    return ars_user_kick_;
}

// IDA_EA: 0x868F716
// IDA_NAME: _ZN5CUser16getForceDropFlagEv
// DEMANGLED: CUser::getForceDropFlag(void)
int CUser::getForceDropFlag() {
    return force_drop_flag_;
}

// IDA_EA: 0x868995A
// IDA_NAME: _ZN5CUser18GetTradePunishTypeEv
// DEMANGLED: CUser::GetTradePunishType(void)
int CUser::GetTradePunishType() {
    return trade_punish_type_;
}

// IDA_EA: 0x8692A08
// IDA_NAME: _ZN5CUser13SetBuyingGoldEb
// DEMANGLED: CUser::SetBuyingGold(bool)
CUser* CUser::SetBuyingGold(bool buying) {
    buying_gold_ = buying;
    return this;
}

// IDA_EA: 0x8692A24
// IDA_NAME: _ZN5CUser13GetBuyingGoldEv
// DEMANGLED: CUser::GetBuyingGold(void)
int CUser::GetBuyingGold() {
    return buying_gold_;
}

// IDA_EA: 0x8692AF6
// IDA_NAME: _ZN5CUser12GetCeraPointEv
// DEMANGLED: CUser::GetCeraPoint(void)
int CUser::GetCeraPoint() {
    return static_cast<int>(cera_point_);
}
