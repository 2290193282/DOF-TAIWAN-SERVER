#include "old_game/player/core/cuser.h"

#include <cstring>


// IDA_EA: 0x82A6786
// IDA_NAME: _ZN5CUser22IsReturnUserFirstLoginEv
// DEMANGLED: CUser::IsReturnUserFirstLogin(void)
int CUser::IsReturnUserFirstLogin() {
    return return_user_first_login_;
}

// IDA_EA: 0x82A681E
// IDA_NAME: _ZN5CUser10SetSlotIDXEs
// DEMANGLED: CUser::SetSlotIDX(short)
CUser* CUser::SetSlotIDX(short slot_index) {
    slot_index_ = static_cast<uint16_t>(slot_index);
    return this;
}

// IDA_EA: 0x82A684C
// IDA_NAME: _ZN5CUser10SetIncreIDEs
// DEMANGLED: CUser::SetIncreID(short)
CUser* CUser::SetIncreID(short increment_id) {
    increment_id_ = static_cast<uint16_t>(increment_id);
    return this;
}

// IDA_EA: 0x82A686A
// IDA_NAME: _ZN5CUser13set_unique_idEt
// DEMANGLED: CUser::set_unique_id(unsigned short)
CUser* CUser::set_unique_id(unsigned short unique_id) {
    unique_id_ = unique_id;
    return this;
}

// IDA_EA: 0x82A6888
// IDA_NAME: _ZN5CUser19getPunishTradeAlertEv
// DEMANGLED: CUser::getPunishTradeAlert(void)
int CUser::getPunishTradeAlert() {
    return punish_trade_alert_;
}

// IDA_EA: 0x82F0F36
// IDA_NAME: _ZN5CUser17GetAutoClientRateEv
// DEMANGLED: CUser::GetAutoClientRate(void)
int CUser::GetAutoClientRate() {
    return auto_client_rate_;
}

// IDA_EA: 0x833F4AA
// IDA_NAME: _ZN5CUser22set_cera_item_order_noEj
// DEMANGLED: CUser::set_cera_item_order_no(unsigned int)
CUser* CUser::set_cera_item_order_no(unsigned int order_number) {
    cera_item_order_no_ = order_number;
    return this;
}

// IDA_EA: 0x8469A5C
// IDA_NAME: _ZN5CUser23setChattingMessageCountEi
// DEMANGLED: CUser::setChattingMessageCount(int)
CUser* CUser::setChattingMessageCount(int count) {
    chatting_message_count_ = count;
    return this;
}

// IDA_EA: 0x84EC61E
// IDA_NAME: _ZN5CUser13SetFirstLoginEv
// DEMANGLED: CUser::SetFirstLogin(void)
CUser* CUser::SetFirstLogin() {
    first_login_ = 1u;
    return this;
}

// IDA_EA: 0x84EC62E
// IDA_NAME: _ZN5CUser21getBreakAwayQuestTimeEv
// DEMANGLED: CUser::getBreakAwayQuestTime(void)
int CUser::getBreakAwayQuestTime() {
    return break_away_quest_time_;
}

// IDA_EA: 0x84EC63C
// IDA_NAME: _ZN5CUser22setBreakAwayAccureCeraEi
// DEMANGLED: CUser::setBreakAwayAccureCera(int)
CUser* CUser::setBreakAwayAccureCera(int cera) {
    break_away_accure_cera_ = cera;
    return this;
}

// IDA_EA: 0x84EC64E
// IDA_NAME: _ZN5CUser21isBreakAwayRewardDataEv
// DEMANGLED: CUser::isBreakAwayRewardData(void)
int CUser::isBreakAwayRewardData() {
    return break_away_reward_data_;
}

// IDA_EA: 0x84EC66C
// IDA_NAME: _ZN5CUser24getBreakAwayRewardItemIdEv
// DEMANGLED: CUser::getBreakAwayRewardItemId(void)
int CUser::getBreakAwayRewardItemId() {
    return static_cast<int>(break_away_rewards_.item_id);
}

// IDA_EA: 0x84EC67A
// IDA_NAME: _ZN5CUser26getBreakAwayRewardAItemCntEv
// DEMANGLED: CUser::getBreakAwayRewardAItemCnt(void)
int CUser::getBreakAwayRewardAItemCnt() {
    return static_cast<int>(break_away_rewards_.item_count);
}

// IDA_EA: 0x84EC688
// IDA_NAME: _ZN5CUser27getBreakAwayRewardItemOrderEv
// DEMANGLED: CUser::getBreakAwayRewardItemOrder(void)
int CUser::getBreakAwayRewardItemOrder() {
    return static_cast<int>(break_away_rewards_.reward_order);
}

// IDA_EA: 0x84EC696
// IDA_NAME: _ZN5CUser24setBreakAwayRewardItemIdEi
// DEMANGLED: CUser::setBreakAwayRewardItemId(int)
CUser* CUser::setBreakAwayRewardItemId(int item_id) {
    break_away_rewards_.item_id = static_cast<uint32_t>(item_id);
    return this;
}

// IDA_EA: 0x84EC6A8
// IDA_NAME: _ZN5CUser26setBreakAwayRewardAItemCntEi
// DEMANGLED: CUser::setBreakAwayRewardAItemCnt(int)
CUser* CUser::setBreakAwayRewardAItemCnt(int count) {
    break_away_rewards_.item_count = static_cast<uint32_t>(count);
    return this;
}

// IDA_EA: 0x84EC6BA
// IDA_NAME: _ZN5CUser31getUserBreakAwayRewardLuckPointEv
// DEMANGLED: CUser::getUserBreakAwayRewardLuckPoint(void)
int CUser::getUserBreakAwayRewardLuckPoint() {
    return static_cast<int>(break_away_rewards_.luck_point);
}

// IDA_EA: 0x84EC6C8
// IDA_NAME: _ZN5CUser31setUserBreakAwayRewardLuckPointEi
// DEMANGLED: CUser::setUserBreakAwayRewardLuckPoint(int)
CUser* CUser::setUserBreakAwayRewardLuckPoint(int luck_point) {
    break_away_rewards_.luck_point = static_cast<uint32_t>(luck_point);
    return this;
}

// IDA_EA: 0x84EC6DA
// IDA_NAME: _ZN5CUser25setJoinedSchoolPointEventEb
// DEMANGLED: CUser::setJoinedSchoolPointEvent(bool)
CUser* CUser::setJoinedSchoolPointEvent(bool joined) {
    joined_school_point_event_ = static_cast<uint8_t>(joined);
    return this;
}

// IDA_EA: 0x84EC6F6
// IDA_NAME: _ZN5CUser20setTournamentAccountEb
// DEMANGLED: CUser::setTournamentAccount(bool)
CUser* CUser::setTournamentAccount(bool tournament_account) {
    tournament_account_ = static_cast<uint8_t>(tournament_account);
    return this;
}

// IDA_EA: 0x84EC712
// IDA_NAME: _ZN5CUser19isTournamentAccountEv
// DEMANGLED: CUser::isTournamentAccount(void)
int CUser::isTournamentAccount() {
    return tournament_account_;
}

// IDA_EA: 0x84EC734
// IDA_NAME: _ZN5CUser28setOnlinePreliminaryCharacNoEj
// DEMANGLED: CUser::setOnlinePreliminaryCharacNo(unsigned int)
CUser* CUser::setOnlinePreliminaryCharacNo(unsigned int charac_no) {
    online_preliminary_charac_no_ = charac_no;
    return this;
}

// IDA_EA: 0x84EC746
// IDA_NAME: _ZN5CUser28setOnlinePreliminaryPlayTypeEi
// DEMANGLED: CUser::setOnlinePreliminaryPlayType(int)
CUser* CUser::setOnlinePreliminaryPlayType(int play_type) {
    online_preliminary_play_type_ = play_type;
    return this;
}

// IDA_EA: 0x84EC758
// IDA_NAME: _ZN5CUser28getOnlinePreliminaryPlayTypeEv
// DEMANGLED: CUser::getOnlinePreliminaryPlayType(void)
int CUser::getOnlinePreliminaryPlayType() {
    return online_preliminary_play_type_;
}

// IDA_EA: 0x84EC766
// IDA_NAME: _ZN5CUser11SetPCRoomNoEj
// DEMANGLED: CUser::SetPCRoomNo(unsigned int)
CUser* CUser::SetPCRoomNo(unsigned int pc_room_no) {
    pc_room_no_ = pc_room_no;
    return this;
}

// IDA_EA: 0x84EC778
// IDA_NAME: _ZN5CUser13SetReturnUserEv
// DEMANGLED: CUser::SetReturnUser(void)
CUser* CUser::SetReturnUser() {
    return_user_ = 1u;
    return this;
}

// IDA_EA: 0x84EC788
// IDA_NAME: _ZN5CUser15ResetReturnUserEv
// DEMANGLED: CUser::ResetReturnUser(void)
CUser* CUser::ResetReturnUser() {
    return_user_ = 0u;
    return this;
}

// IDA_EA: 0x84EC798
// IDA_NAME: _ZN5CUser27SetReturnUserFirstUserLoginEb
// DEMANGLED: CUser::SetReturnUserFirstUserLogin(bool)
CUser* CUser::SetReturnUserFirstUserLogin(bool first_login) {
    return_user_first_login_ = static_cast<uint8_t>(first_login);
    return this;
}

// IDA_EA: 0x84EC7B4
// IDA_NAME: _ZN5CUser23GetReturnUserExpireTimeEv
// DEMANGLED: CUser::GetReturnUserExpireTime(void)
int CUser::GetReturnUserExpireTime() {
    return return_user_expire_time_;
}

// IDA_EA: 0x84EC7C2
// IDA_NAME: _ZN5CUser23SetReturnUserExpireTimeEi
// DEMANGLED: CUser::SetReturnUserExpireTime(int)
CUser* CUser::SetReturnUserExpireTime(int expire_time) {
    return_user_expire_time_ = expire_time;
    return this;
}

// IDA_EA: 0x84EC7D4
// IDA_NAME: _ZN5CUser28EnableSaveMemberBonusFatigueEv
// DEMANGLED: CUser::EnableSaveMemberBonusFatigue(void)
CUser* CUser::EnableSaveMemberBonusFatigue() {
    save_member_bonus_fatigue_ = 1u;
    return this;
}

// IDA_EA: 0x84EC7E4
// IDA_NAME: _ZN5CUser28incAccountMemberBonusFatigueEs
// DEMANGLED: CUser::incAccountMemberBonusFatigue(short)
CUser* CUser::incAccountMemberBonusFatigue(short fatigue) {
    account_member_bonus_fatigue_ = static_cast<uint16_t>(
        account_member_bonus_fatigue_ + fatigue);
    return this;
}

// IDA_EA: 0x84EC812
// IDA_NAME: _ZN5CUser30resetAccountMemberBonusFatigueEv
// DEMANGLED: CUser::resetAccountMemberBonusFatigue(void)
CUser* CUser::resetAccountMemberBonusFatigue() {
    account_member_bonus_fatigue_ = 0u;
    return this;
}

// IDA_EA: 0x84EC824
// IDA_NAME: _ZN5CUser28getAccountMemberBonusFatigueEv
// DEMANGLED: CUser::getAccountMemberBonusFatigue(void)
int CUser::getAccountMemberBonusFatigue() {
    return account_member_bonus_fatigue_;
}

// IDA_EA: 0x84EC834
// IDA_NAME: _ZN5CUser13setPcRoomAuthEb
// DEMANGLED: CUser::setPcRoomAuth(bool)
CUser* CUser::setPcRoomAuth(bool authenticated) {
    pc_room_auth_ = static_cast<uint8_t>(authenticated);
    return this;
}

// IDA_EA: 0x84EC88C
// IDA_NAME: _ZN5CUser16getClientVersionEv
// DEMANGLED: CUser::getClientVersion(void)
int CUser::getClientVersion() {
    return static_cast<int>(client_version_);
}

// IDA_EA: 0x84EC89A
// IDA_NAME: _ZN5CUser16setClientVersionEj
// DEMANGLED: CUser::setClientVersion(unsigned int)
CUser* CUser::setClientVersion(unsigned int version) {
    client_version_ = version;
    return this;
}

// IDA_EA: 0x84EC90A
// IDA_NAME: _ZN5CUser21get_public_ip_addressEv
// DEMANGLED: CUser::get_public_ip_address(void)
int CUser::get_public_ip_address() {
    return static_cast<int>(public_ip_address_);
}

// IDA_EA: 0x84EC918
// IDA_NAME: _ZN5CUser13setWebAddressEPc
// DEMANGLED: CUser::setWebAddress(char *)
char* CUser::setWebAddress(char* address) {
    return std::strncpy(web_address_, address, sizeof(web_address_));
}

// IDA_EA: 0x84EC96A
// IDA_NAME: _ZN5CUser24setTotalHackUserTradeCntEj
// DEMANGLED: CUser::setTotalHackUserTradeCnt(unsigned int)
CUser* CUser::setTotalHackUserTradeCnt(unsigned int count) {
    total_hack_user_trade_count_ = count;
    return this;
}

// IDA_EA: 0x84EC97C
// IDA_NAME: _ZN5CUser28SetReportMannerlessUserCountEi
// DEMANGLED: CUser::SetReportMannerlessUserCount(int)
CUser* CUser::SetReportMannerlessUserCount(int count) {
    report_mannerless_user_count_ = count;
    return this;
}

// IDA_EA: 0x84EC98E
// IDA_NAME: _ZN5CUser22GetChecksumAnswerCountEv
// DEMANGLED: CUser::GetChecksumAnswerCount(void)
int CUser::GetChecksumAnswerCount() {
    return static_cast<int>(checksum_answer_count_);
}

// IDA_EA: 0x84EC9C8
// IDA_NAME: _ZN5CUser10setRegDateEj
// DEMANGLED: CUser::setRegDate(unsigned int)
CUser* CUser::setRegDate(unsigned int date) {
    registration_date_ = date;
    return this;
}

// IDA_EA: 0x84EC9DA
// IDA_NAME: _ZN5CUser10setRegTimeEl
// DEMANGLED: CUser::setRegTime(long)
CUser* CUser::setRegTime(long time) {
    registration_time_ = static_cast<uint32_t>(time);
    return this;
}

// IDA_EA: 0x84ECB0A
// IDA_NAME: _ZN5CUser16GetLogInOutStateEv
// DEMANGLED: CUser::GetLogInOutState(void)
int CUser::GetLogInOutState() {
    return login_out_state_;
}

// IDA_EA: 0x84ECB28
// IDA_NAME: _ZN5CUser14getStdDropRateEv
// DEMANGLED: CUser::getStdDropRate(void)
int CUser::getStdDropRate() {
    return std_drop_rate_;
}

// IDA_EA: 0x84ECEFE
// IDA_NAME: _ZN5CUser21SetGuildCargoCapacityEj
// DEMANGLED: CUser::SetGuildCargoCapacity(unsigned int)
CUser* CUser::SetGuildCargoCapacity(unsigned int capacity) {
    guild_cargo_capacity_ = capacity;
    return this;
}

// IDA_EA: 0x84ECF10
// IDA_NAME: _ZN5CUser19SetTodayGuildMemberEb
// DEMANGLED: CUser::SetTodayGuildMember(bool)
CUser* CUser::SetTodayGuildMember(bool today_member) {
    today_guild_member_ = static_cast<uint8_t>(today_member);
    return this;
}

// IDA_EA: 0x84ECF2C
// IDA_NAME: _ZN5CUser23setHumanCertifyErrorCntEi
// DEMANGLED: CUser::setHumanCertifyErrorCnt(int)
CUser* CUser::setHumanCertifyErrorCnt(int count) {
    human_certify_error_count_ = static_cast<uint32_t>(count);
    return this;
}
