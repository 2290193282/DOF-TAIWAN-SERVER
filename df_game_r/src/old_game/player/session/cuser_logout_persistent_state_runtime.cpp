#include "old_game/player/core/cuser.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/db/account/login_payloads.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"

#include <map>
#include <utility>


// IDA_EA: 0x8658A3E
// IDA_NAME: _ZN5CUser20checkLogOutCorrectlyEv
// DEMANGLED: CUser::checkLogOutCorrectly(void)
CUser* CUser::checkLogOutCorrectly() {
    ting_flag_ = 1;
    return this;
}

// IDA_EA: 0x868D32A
// IDA_NAME: _ZN5CUser24set_server_fatigue_countEs
// DEMANGLED: CUser::set_server_fatigue_count(short)
void CUser::set_server_fatigue_count(short count) {
    std::map<unsigned char, short>& fatigue_by_day = fatigue_day_data_;
    const unsigned char today = static_cast<unsigned char>(
        G_CDataManager()->get_event_script_mng()->get_today_time_day());
    std::map<unsigned char, short>::iterator it = fatigue_by_day.find(today);
    if (it != fatigue_by_day.end()) {
        it->second = static_cast<short>(it->second + count);
    } else {
        fatigue_by_day.insert(std::make_pair(today, count));
    }
}

// IDA_EA: 0x868D3FE
// IDA_NAME: _ZN5CUser24get_server_fatigue_countEh
// DEMANGLED: CUser::get_server_fatigue_count(unsigned char)
int CUser::get_server_fatigue_count(unsigned char day) {
    std::map<unsigned char, short>& fatigue_by_day = fatigue_day_data_;
    std::map<unsigned char, short>::iterator it = fatigue_by_day.find(day);
    if (it == fatigue_by_day.end()) {
        return 0;
    }
    return static_cast<unsigned short>(it->second);
}

// IDA_EA: 0x868D476
// IDA_NAME: _ZN5CUser25reset_server_fatigue_dataEv
// DEMANGLED: CUser::reset_server_fatigue_data(void)
void CUser::reset_server_fatigue_data() {
    fatigue_day_db_ = 0;
    fatigue_count_db_ = 0;
    fatigue_day_data_.clear();
}

// IDA_EA: 0x868D4A2
// IDA_NAME: _ZN5CUser20set_fatigue_count_dbEii
// DEMANGLED: CUser::set_fatigue_count_db(int,int)
CUser* CUser::set_fatigue_count_db(int day, int count) {
    fatigue_day_db_ = static_cast<unsigned char>(day);
    fatigue_count_db_ = static_cast<unsigned char>(count);
    return this;
}

// IDA_EA: 0x868D4C4
// IDA_NAME: _ZN5CUser20get_fatigue_count_dbEi
// DEMANGLED: CUser::get_fatigue_count_db(int)
int CUser::get_fatigue_count_db(int day) {
    if (fatigue_day_db_ == day) {
        return fatigue_count_db_;
    }
    return 0;
}

// IDA_EA: 0x868D4F0
// IDA_NAME: _ZN5CUser24get_ingame_event_historyEv
// DEMANGLED: CUser::get_ingame_event_history(void)
std::map<int, std::pair<int, int> >* CUser::get_ingame_event_history() {
    return &pair_int_data_;
}

// IDA_EA: 0x868D4FE
// IDA_NAME: _ZN5CUser31get_ingame_event_history_updateEv
// DEMANGLED: CUser::get_ingame_event_history_update(void)
std::map<int, bool>* CUser::get_ingame_event_history_update() {
    return &flag_map_;
}

// IDA_EA: 0x868D50C
// IDA_NAME: _ZN5CUser24get_ingame_event_historyEiRSt4pairIiiE
// DEMANGLED: CUser::get_ingame_event_history(int,std::pair<int,int> &)
bool CUser::get_ingame_event_history(int type,
                                     std::pair<int, int>& history) {
    std::map<int, std::pair<int, int> >& histories =
        *get_ingame_event_history();
    std::map<int, std::pair<int, int> >::iterator it = histories.find(type);
    if (it == histories.end()) {
        return false;
    }
    history = it->second;
    return true;
}

// IDA_EA: 0x868D580
// IDA_NAME: _ZN5CUser24set_ingame_event_historyEiii
// DEMANGLED: CUser::set_ingame_event_history(int,int,int)
void CUser::set_ingame_event_history(int type, int first, int second) {
    std::map<int, std::pair<int, int> >& histories =
        *get_ingame_event_history();
    std::map<int, std::pair<int, int> >::iterator it = histories.find(type);
    if (it != histories.end()) {
        it->second.first = first;
        it->second.second = second;
    } else {
        histories.insert(std::make_pair(type, std::make_pair(first, second)));
    }
    update_ingame_event_history(type);
}

// IDA_EA: 0x868D72C
// IDA_NAME: _ZN5CUser27update_ingame_event_historyEi
// DEMANGLED: CUser::update_ingame_event_history(int)
void CUser::update_ingame_event_history(int type) {
    (*get_ingame_event_history_update())[type] = true;
}

// IDA_EA: 0x868D750
// IDA_NAME: _ZN5CUser26reset_ingame_event_historyEv
// DEMANGLED: CUser::reset_ingame_event_history(void)
void CUser::reset_ingame_event_history() {
    get_ingame_event_history()->clear();
}

// IDA_EA: 0x868D768
// IDA_NAME: _ZN5CUser33reset_ingame_event_history_updateEv
// DEMANGLED: CUser::reset_ingame_event_history_update(void)
void CUser::reset_ingame_event_history_update() {
    get_ingame_event_history_update()->clear();
}

// IDA_EA: 0x869126E
// IDA_NAME: _ZN5CUser12SetBingoDataER14SIG_LOGIN_DATA
// DEMANGLED: CUser::SetBingoData(SIG_LOGIN_DATA &)
CUser* CUser::SetBingoData(SIG_LOGIN_DATA& login_data) {
    bingo_data_.clear();
    bingo_data_.bingo_count = static_cast<uint32_t>(login_data.bingo_board);
    bingo_data_.selected_count = login_data.bingo_reward != 0;
    return this;
}

// IDA_EA: 0x869714C
// IDA_NAME: _ZN5CUser14CharacMoneyLogC2Ev
// DEMANGLED: CUser::CharacMoneyLog::CharacMoneyLog(void)
CUser::CharacMoneyLog::CharacMoneyLog() {
    reset();
}

// IDA_EA: 0x8697160
// IDA_NAME: _ZN5CUser14CharacMoneyLog5resetEv
// DEMANGLED: CUser::CharacMoneyLog::reset(void)
CUser::CharacMoneyLog* CUser::CharacMoneyLog::reset() {
    plus_money = 0;
    minus_money = 0;
    tick = 0;
    return this;
}

// IDA_EA: 0x8697182
// IDA_NAME: _ZN5CUser13resetMoneyLogEv
// DEMANGLED: CUser::resetMoneyLog(void)
CUser* CUser::resetMoneyLog() {
    for (int index = 0; index <= 6; ++index) {
        charac_money_logs_[index].reset();
    }
    money_log_index_ = 0;
    return this;
}

// IDA_EA: 0x86971CE
// IDA_NAME: _ZN5CUser13GetFirstLoginEv
// DEMANGLED: CUser::GetFirstLogin(void)
int CUser::GetFirstLogin() {
    return first_login_;
}

// IDA_EA: 0x869721A
// IDA_NAME: _ZN5CUser24isJoinedSchoolPointEventEv
// DEMANGLED: CUser::isJoinedSchoolPointEvent(void)
int CUser::isJoinedSchoolPointEvent() {
    return joined_school_point_event_;
}

// IDA_EA: 0x869722A
// IDA_NAME: _ZN5CUser29DisableSaveMemberBonusFatigueEv
// DEMANGLED: CUser::DisableSaveMemberBonusFatigue(void)
CUser* CUser::DisableSaveMemberBonusFatigue() {
    save_member_bonus_fatigue_ = 0;
    return this;
}

// IDA_EA: 0x869723A
// IDA_NAME: _ZN5CUser30IsEnableSaveMemberBonusFatigueEv
// DEMANGLED: CUser::IsEnableSaveMemberBonusFatigue(void)
int CUser::IsEnableSaveMemberBonusFatigue() {
    return save_member_bonus_fatigue_;
}

// IDA_EA: 0x869727A
// IDA_NAME: _ZN5CUser20get_local_ip_addressEv
// DEMANGLED: CUser::get_local_ip_address(void)
int CUser::get_local_ip_address() {
    return static_cast<int>(local_ip_address_);
}

// IDA_EA: 0x8697288
// IDA_NAME: _ZN5CUser15resetPlayExpAddEv
// DEMANGLED: CUser::resetPlayExpAdd(void)
CUser* CUser::resetPlayExpAdd() {
    play_experience_ = 0;
    return this;
}

// IDA_EA: 0x869729A
// IDA_NAME: _ZN5CUser13incPlayExpAddEi
// DEMANGLED: CUser::incPlayExpAdd(int)
CUser* CUser::incPlayExpAdd(int experience) {
    play_experience_ += experience;
    return this;
}

// IDA_EA: 0x869730A
// IDA_NAME: _ZN5CUser21GetResumeChecksumTimeEv
// DEMANGLED: CUser::GetResumeChecksumTime(void)
int CUser::GetResumeChecksumTime() {
    return static_cast<int>(resume_checksum_time_);
}

// IDA_EA: 0x86973CC
// IDA_NAME: _ZN5CUser23isCharacLinkMessageFlagEv
// DEMANGLED: CUser::isCharacLinkMessageFlag(void)
int CUser::isCharacLinkMessageFlag() {
    return charac_link_message_flag_;
}

// IDA_EA: 0x8697516
// IDA_NAME: _ZN5CUser22ResetUsedCoinInDungeonEv
// DEMANGLED: CUser::ResetUsedCoinInDungeon(void)
CUser* CUser::ResetUsedCoinInDungeon() {
    used_coin_in_dungeon_ = 0;
    return this;
}

// IDA_EA: 0x869755C
// IDA_NAME: _ZN5CUser18setCharacSlotLimitEh
// DEMANGLED: CUser::setCharacSlotLimit(unsigned char)
CUser* CUser::setCharacSlotLimit(unsigned char limit) {
    charac_slot_limit_ = limit;
    return this;
}

// IDA_EA: 0x8697578
// IDA_NAME: _ZNK5CUser26getFlagPresentAvengerTitleEv
// DEMANGLED: CUser::getFlagPresentAvengerTitle(void)const
int CUser::getFlagPresentAvengerTitle() const {
    return present_avenger_title_;
}

// IDA_EA: 0x8697588
// IDA_NAME: _ZN5CUser33reset_update_ontime_last_recv_idxEv
// DEMANGLED: CUser::reset_update_ontime_last_recv_idx(void)
CUser* CUser::reset_update_ontime_last_recv_idx() {
    update_ontime_last_recv_idx_ = 0;
    return this;
}

// IDA_EA: 0x8697598
// IDA_NAME: _ZN5CUser30is_update_ontime_last_recv_idxEv
// DEMANGLED: CUser::is_update_ontime_last_recv_idx(void)
int CUser::is_update_ontime_last_recv_idx() {
    return update_ontime_last_recv_idx_;
}

// IDA_EA: 0x86975A8
// IDA_NAME: _ZN5CUser27get_server_fatigue_day_dataEv
// DEMANGLED: CUser::get_server_fatigue_day_data(void)
std::map<unsigned char, short>* CUser::get_server_fatigue_day_data() {
    return &fatigue_day_data_;
}

// IDA_EA: 0x86975B6
// IDA_NAME: _ZN5CUser27get_server_fatigue_day_sizeEv
// DEMANGLED: CUser::get_server_fatigue_day_size(void)
int CUser::get_server_fatigue_day_size() {
    return static_cast<int>(get_server_fatigue_day_data()->size());
}

// IDA_EA: 0x84ECF3E
// IDA_NAME: _ZN5CUser22setRestingUserRestrictEb
// DEMANGLED: CUser::setRestingUserRestrict(bool)
CUser* CUser::setRestingUserRestrict(bool restricted) {
    resting_user_ = restricted;
    return this;
}

// IDA_EA: 0x84ECF5A
// IDA_NAME: _ZN5CUser19setPunishTradeAlertEs
// DEMANGLED: CUser::setPunishTradeAlert(short)
CUser* CUser::setPunishTradeAlert(short alert) {
    punish_trade_alert_ = static_cast<uint16_t>(alert);
    return this;
}

// IDA_EA: 0x84ECF78
// IDA_NAME: _ZN5CUser13setSecuRewardEb
// DEMANGLED: CUser::setSecuReward(bool)
CUser* CUser::setSecuReward(bool reward) {
    hades_.security_state_.reward_enabled = reward;
    return this;
}

// IDA_EA: 0x84ECF94
// IDA_NAME: _ZN5CUser14NeedSecuRewardEv
// DEMANGLED: CUser::NeedSecuReward(void)
int CUser::NeedSecuReward() {
    return hades_.security_state_.reward_enabled;
}

// IDA_EA: 0x84ECFA4
// IDA_NAME: _ZN5CUser11setSecuTypeEh
// DEMANGLED: CUser::setSecuType(unsigned char)
CUser* CUser::setSecuType(unsigned char type) {
    hades_.security_state_.security_type = type;
    return this;
}

// IDA_EA: 0x84ECFC0
// IDA_NAME: _ZN5CUser13getRewardTypeEv
// DEMANGLED: CUser::getRewardType(void)
int CUser::getRewardType() {
    return hades_.security_state_.reward_type;
}

// IDA_EA: 0x84ECFD0
// IDA_NAME: _ZN5CUser13setRewardTypeEh
// DEMANGLED: CUser::setRewardType(unsigned char)
CUser* CUser::setRewardType(unsigned char type) {
    hades_.security_state_.reward_type = type;
    return this;
}

// IDA_EA: 0x84ECFEC
// IDA_NAME: _ZN5CUser12setSecuGradeEc
// DEMANGLED: CUser::setSecuGrade(char)
CUser* CUser::setSecuGrade(char grade) {
    hades_.security_state_.security_grade = grade;
    return this;
}

// IDA_EA: 0x868866E
// IDA_NAME: _ZN5CUser21isSecurityServiceUsedEv
// DEMANGLED: CUser::isSecurityServiceUsed(void)
int CUser::isSecurityServiceUsed() {
    if (getPad()->isActivate()) {
        return 1;
    }
    if (getSecurityCard()->isActivate()) {
        return 1;
    }
    return hades_.security_state_.security_grade != 0;
}

// IDA_EA: 0x84ED016
// IDA_NAME: _ZN5CUser26setFlagPresentAvengerTitleEb
// DEMANGLED: CUser::setFlagPresentAvengerTitle(bool)
CUser* CUser::setFlagPresentAvengerTitle(bool present) {
    present_avenger_title_ = present;
    return this;
}

// IDA_EA: 0x84ED032
// IDA_NAME: _ZN5CUser30setCheckPickUpRandomOptionItemEb
// DEMANGLED: CUser::setCheckPickUpRandomOptionItem(bool)
CUser* CUser::setCheckPickUpRandomOptionItem(bool check) {
    check_pick_up_random_option_item_ = check;
    return this;
}

// IDA_EA: 0x85BFD36
// IDA_NAME: _ZN5CUser27subBreakAwayDungeonClearCntEv
// DEMANGLED: CUser::subBreakAwayDungeonClearCnt(void)
CUser* CUser::subBreakAwayDungeonClearCnt() {
    --break_away_dungeon_clear_count_;
    return setBreakAwayDungeonClear(true);
}

// IDA_EA: 0x85BFDCA
// IDA_NAME: _ZN5CUser18IsTodayGuildMemberEv
// DEMANGLED: CUser::IsTodayGuildMember(void)
int CUser::IsTodayGuildMember() {
    return today_guild_member_;
}

// IDA_EA: 0x86C0B6E
// IDA_NAME: _ZN5CUser14ResetCoinCountEv
// DEMANGLED: CUser::ResetCoinCount(void)
CUser* CUser::ResetCoinCount() {
    used_coin_count_ = 0;
    return this;
}

// IDA_EA: 0x86C0B80
// IDA_NAME: _ZN5CUser16GetUsedCoinCountEv
// DEMANGLED: CUser::GetUsedCoinCount(void)
int CUser::GetUsedCoinCount() {
    return static_cast<int>(used_coin_count_);
}

// IDA_EA: 0x86D207C
// IDA_NAME: _ZN5CUser23resetAccountUsedFatigueEv
// DEMANGLED: CUser::resetAccountUsedFatigue(void)
CUser* CUser::resetAccountUsedFatigue() {
    account_used_fatigue_ = 0;
    return this;
}

// IDA_EA: 0x86D208E
// IDA_NAME: _ZN5CUser23incChattingMessageCountEi
// DEMANGLED: CUser::incChattingMessageCount(int)
CUser* CUser::incChattingMessageCount(int count) {
    chatting_message_count_ += count;
    return this;
}

// IDA_EA: 0x86D20AA
// IDA_NAME: _ZN5CUser23getChattingMessageCountEv
// DEMANGLED: CUser::getChattingMessageCount(void)
int CUser::getChattingMessageCount() {
    return chatting_message_count_;
}
