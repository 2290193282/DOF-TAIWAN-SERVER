#include "old_game/player/core/cuser.h"

#include <stdint.h>


// IDA_EA: 0x80FDF44
// IDA_NAME: _ZN5CUser30set_multiboxLotteryItemFailCntEi
// DEMANGLED: CUser::set_multiboxLotteryItemFailCnt(int)
CUser* CUser::set_multiboxLotteryItemFailCnt(int count) {
    multibox_lottery_item_fail_count_ = count;
    return this;
}

// IDA_EA: 0x80FDF56
// IDA_NAME: _ZN5CUser30inc_multiboxLotteryItemFailCntEv
// DEMANGLED: CUser::inc_multiboxLotteryItemFailCnt(void)
int CUser::inc_multiboxLotteryItemFailCnt() {
    return ++multibox_lottery_item_fail_count_;
}

// IDA_EA: 0x8101048
// IDA_NAME: _ZN5CUser11GetUserNameEv
// DEMANGLED: CUser::GetUserName(void)
char* CUser::GetUserName() {
    return user_name_;
}

// IDA_EA: 0x8101064
// IDA_NAME: _ZN5CUser12GetUserEMailEv
// DEMANGLED: CUser::GetUserEMail(void)
char* CUser::GetUserEMail() {
    return user_email_;
}

// IDA_EA: 0x811BAB0
// IDA_NAME: _ZN5CUser22setPcRoomPlayStartTimeEj
// DEMANGLED: CUser::setPcRoomPlayStartTime(unsigned int)
CUser* CUser::setPcRoomPlayStartTime(unsigned int start_time) {
    pc_room_play_start_time_ = start_time;
    return this;
}

// IDA_EA: 0x811EDEE
// IDA_NAME: _ZNK5CUser16IsGameMasterModeEv
// DEMANGLED: CUser::IsGameMasterMode(void)const
int CUser::IsGameMasterMode() const {
    return game_master_mode_;
}

// IDA_EA: 0x814AA8C
// IDA_NAME: _ZN5CUser14GetRevivalTimeEv
// DEMANGLED: CUser::GetRevivalTime(void)
int CUser::GetRevivalTime() {
    return static_cast<int>(revival_time_);
}

// IDA_EA: 0x817A1B4
// IDA_NAME: _ZN5CUser7SetCeraEj
// DEMANGLED: CUser::SetCera(unsigned int)
CUser* CUser::SetCera(unsigned int cera) {
    cera_ = cera;
    return this;
}

// IDA_EA: 0x817A1C6
// IDA_NAME: _ZN5CUser7UseCeraEj
// DEMANGLED: CUser::UseCera(unsigned int)
int CUser::UseCera(unsigned int cera) {
    unsigned int& current = cera_;
    if (current < cera) {
        current = 0;
    } else {
        current -= cera;
    }
    return static_cast<int>(current);
}

// IDA_EA: 0x817A208
// IDA_NAME: _ZN5CUser13resetUsedCeraEv
// DEMANGLED: CUser::resetUsedCera(void)
CUser* CUser::resetUsedCera() {
    used_cera_ = 0;
    return this;
}

// IDA_EA: 0x817A21A
// IDA_NAME: _ZN5CUser11addUsedCeraEj
// DEMANGLED: CUser::addUsedCera(unsigned int)
CUser* CUser::addUsedCera(unsigned int cera) {
    used_cera_ += cera;
    return this;
}

// IDA_EA: 0x817A236
// IDA_NAME: _ZN5CUser11getUsedCeraEv
// DEMANGLED: CUser::getUsedCera(void)
int CUser::getUsedCera() {
    return static_cast<int>(used_cera_);
}

// IDA_EA: 0x822FC5C
// IDA_NAME: _ZN5CUser27setBreakAwayDungeonClearCntEi
// DEMANGLED: CUser::setBreakAwayDungeonClearCnt(int)
CUser* CUser::setBreakAwayDungeonClearCnt(int count) {
    break_away_dungeon_clear_count_ = count;
    return this;
}

// IDA_EA: 0x822FC7C
// IDA_NAME: _ZN5CUser21setBreakAwayQuestTimeEj
// DEMANGLED: CUser::setBreakAwayQuestTime(unsigned int)
CUser* CUser::setBreakAwayQuestTime(unsigned int time) {
    break_away_quest_time_ = static_cast<int32_t>(time);
    return this;
}

// IDA_EA: 0x822FCB8
// IDA_NAME: _ZN5CUser13getBackupSeedEv
// DEMANGLED: CUser::getBackupSeed(void)
int CUser::getBackupSeed() {
    return static_cast<int>(backup_seed_);
}

// IDA_EA: 0x822FCF2
// IDA_NAME: _ZN5CUser30get_multiboxLotteryItemFailCntEv
// DEMANGLED: CUser::get_multiboxLotteryItemFailCnt(void)
int CUser::get_multiboxLotteryItemFailCnt() {
    return multibox_lottery_item_fail_count_;
}

// IDA_EA: 0x822FD00
// IDA_NAME: _ZN5CUser20set_local_ip_addressEi
// DEMANGLED: CUser::set_local_ip_address(int)
CUser* CUser::set_local_ip_address(int address) {
    local_ip_address_ = static_cast<uint32_t>(address);
    return this;
}

// IDA_EA: 0x822FD12
// IDA_NAME: _ZN5CUser13getWebAddressEv
// DEMANGLED: CUser::getWebAddress(void)
char* CUser::getWebAddress() {
    return web_address_;
}

// IDA_EA: 0x822FD9C
// IDA_NAME: _ZN5CUser22IncChecksumAnswerCountEv
// DEMANGLED: CUser::IncChecksumAnswerCount(void)
CUser* CUser::IncChecksumAnswerCount() {
    ++checksum_answer_count_;
    return this;
}

// IDA_EA: 0x822FDF4
// IDA_NAME: _ZN5CUser15GetSeedFromDateEv
// DEMANGLED: CUser::GetSeedFromDate(void)
int CUser::GetSeedFromDate() {
    return static_cast<int>(accept_seed_from_date_);
}

// IDA_EA: 0x822FE10
// IDA_NAME: _ZN5CUser12SetLoginTimeEl
// DEMANGLED: CUser::SetLoginTime(long)
CUser* CUser::SetLoginTime(long login_time) {
    login_time_ = static_cast<uint32_t>(login_time);
    return this;
}

// IDA_EA: 0x822FE22
// IDA_NAME: _ZN5CUser16SetLogInOutStateEh
// DEMANGLED: CUser::SetLogInOutState(unsigned char)
CUser* CUser::SetLogInOutState(unsigned char state) {
    login_out_state_ = state;
    return this;
}

// IDA_EA: 0x822FEA4
// IDA_NAME: _ZN5CUser16SetGMUpgradeModeE20ENUM_GM_ITEM_UPGRADE
// DEMANGLED: CUser::SetGMUpgradeMode(ENUM_GM_ITEM_UPGRADE)
CUser* CUser::SetGMUpgradeMode(ENUM_GM_ITEM_UPGRADE mode) {
    gm_upgrade_mode_ = static_cast<int>(mode);
    return this;
}

// IDA_EA: 0x82300BE
// IDA_NAME: _ZN5CUser6SetAgeEj
// DEMANGLED: CUser::SetAge(unsigned int)
CUser* CUser::SetAge(unsigned int age) {
    age_ = static_cast<int32_t>(age);
    return this;
}

// IDA_EA: 0x8230152
// IDA_NAME: _ZN5CUser17decre_check_countEv
// DEMANGLED: CUser::decre_check_count(void)
CUser* CUser::decre_check_count() {
    connection_check_count_ = 0;
    return this;
}

// IDA_EA: 0x82301BC
// IDA_NAME: _ZN5CUser26DecreaseGuildPowerWarPointEj
// DEMANGLED: CUser::DecreaseGuildPowerWarPoint(unsigned int)
CUser* CUser::DecreaseGuildPowerWarPoint(unsigned int point) {
    guild_info_.guild_power_war_point -= point;
    return this;
}

// IDA_EA: 0x82301EC
// IDA_NAME: _ZN5CUser21GetGuildCargoCapacityEv
// DEMANGLED: CUser::GetGuildCargoCapacity(void)
int CUser::GetGuildCargoCapacity() {
    return static_cast<int>(guild_cargo_capacity_);
}

// IDA_EA: 0x82301FA
// IDA_NAME: _ZN5CUser15SetDebugCommandEi
// DEMANGLED: CUser::SetDebugCommand(int)
CUser* CUser::SetDebugCommand(int command) {
    debug_command_ = command;
    return this;
}

// IDA_EA: 0x8230246
// IDA_NAME: _ZN5CUser11OnRecvEventEv
// DEMANGLED: CUser::OnRecvEvent(void)
CUser* CUser::OnRecvEvent() {
    received_event_ = 1;
    return this;
}

// IDA_EA: 0x8230256
// IDA_NAME: _ZN5CUser19SellItemTimeCheckerEjj
// DEMANGLED: CUser::SellItemTimeChecker(unsigned int,unsigned int)
int CUser::SellItemTimeChecker(unsigned int current_tick,
                               unsigned int interval) {
    unsigned int& last_tick = last_sell_item_tick_;
    unsigned int& count = sell_item_time_count_;
    if (current_tick - last_tick > interval) {
        count = 0;
        last_tick = current_tick;
    } else {
        ++count;
    }
    return static_cast<int>(count);
}

// IDA_EA: 0x82307BA
// IDA_NAME: _ZN5CUser18getLastLotteryTimeEv
// DEMANGLED: CUser::getLastLotteryTime(void)
int CUser::getLastLotteryTime() {
    return static_cast<int>(last_lottery_time_);
}

// IDA_EA: 0x82307C8
// IDA_NAME: _ZN5CUser18setLastLotteryTimeEm
// DEMANGLED: CUser::setLastLotteryTime(unsigned long)
CUser* CUser::setLastLotteryTime(unsigned long lottery_time) {
    last_lottery_time_ = static_cast<uint32_t>(lottery_time);
    return this;
}

// IDA_EA: 0x82307DA
// IDA_NAME: _ZN5CUser20resetCleanpadFailCntEv
// DEMANGLED: CUser::resetCleanpadFailCnt(void)
CUser* CUser::resetCleanpadFailCnt() {
    human_certify_error_count_ = 0;
    return this;
}

// IDA_EA: 0x82307EC
// IDA_NAME: _ZN5CUser21isCleanPadVerifyLimitEv
// DEMANGLED: CUser::isCleanPadVerifyLimit(void)
bool CUser::isCleanPadVerifyLimit() {
    return human_certify_error_count_ > 2u;
}

// IDA_EA: 0x823082A
// IDA_NAME: _ZN5CUser21IncreaseUsedCoinCountEv
// DEMANGLED: CUser::IncreaseUsedCoinCount(void)
CUser* CUser::IncreaseUsedCoinCount() {
    ++used_coin_count_;
    return this;
}

// IDA_EA: 0x8230844
// IDA_NAME: _ZN5CUser31SetDeleteEventDungeonInvitationEb
// DEMANGLED: CUser::SetDeleteEventDungeonInvitation(bool)
CUser* CUser::SetDeleteEventDungeonInvitation(bool remove_invitation) {
    delete_event_dungeon_invitation_ = remove_invitation;
    return this;
}

// IDA_EA: 0x823089C
// IDA_NAME: _ZNK5CUser25getCharacAntibotSerialNumEv
// DEMANGLED: CUser::getCharacAntibotSerialNum(void)const
int CUser::getCharacAntibotSerialNum() const {
    return charac_antibot_serial_number_;
}

// IDA_EA: 0x82308AA
// IDA_NAME: _ZN5CUser25setCharacAntibotSerialNumEi
// DEMANGLED: CUser::setCharacAntibotSerialNum(int)
CUser* CUser::setCharacAntibotSerialNum(int serial_number) {
    charac_antibot_serial_number_ = serial_number;
    return this;
}

// IDA_EA: 0x82308BC
// IDA_NAME: _ZN5CUser23InitReceivedRequestTypeEv
// DEMANGLED: CUser::InitReceivedRequestType(void)
CUser* CUser::InitReceivedRequestType() {
    received_request_type_ = -1;
    return this;
}

// IDA_EA: 0x82308CC
// IDA_NAME: _ZN5CUser22SetReceivedRequestTypeEc
// DEMANGLED: CUser::SetReceivedRequestType(char)
CUser* CUser::SetReceivedRequestType(char request_type) {
    received_request_type_ = request_type;
    return this;
}

// IDA_EA: 0x82308E8
// IDA_NAME: _ZNK5CUser22GetReceivedRequestTypeEv
// DEMANGLED: CUser::GetReceivedRequestType(void)const
int CUser::GetReceivedRequestType() const {
    return static_cast<unsigned char>(received_request_type_);
}

// IDA_EA: 0x826532E
// IDA_NAME: _ZN5CUser15GetDebugCommandEv
// DEMANGLED: CUser::GetDebugCommand(void)
int CUser::GetDebugCommand() {
    return debug_command_;
}

// IDA_EA: 0x8275684
// IDA_NAME: _ZN5CUser16GetNonClientFlagEv
// DEMANGLED: CUser::GetNonClientFlag(void)
int CUser::GetNonClientFlag() {
    return non_client_flag_;
}

// IDA_EA: 0x8275694
// IDA_NAME: _ZN5CUser16SetNonClientFlagEb
// DEMANGLED: CUser::SetNonClientFlag(bool)
CUser* CUser::SetNonClientFlag(bool flag) {
    non_client_flag_ = flag;
    return this;
}

// IDA_EA: 0x82756B0
// IDA_NAME: _ZN5CUser19GetNonClientRandIntEv
// DEMANGLED: CUser::GetNonClientRandInt(void)
int CUser::GetNonClientRandInt() {
    return non_client_random_value_;
}

// IDA_EA: 0x82756BE
// IDA_NAME: _ZN5CUser19SetNonClientRandIntEi
// DEMANGLED: CUser::SetNonClientRandInt(int)
CUser* CUser::SetNonClientRandInt(int random_value) {
    non_client_random_value_ = random_value;
    return this;
}

// IDA_EA: 0x84ECB00
// IDA_NAME: _ZNK5CUser12IsPCRoomUserEv
// DEMANGLED: CUser::IsPCRoomUser(void)const
int CUser::IsPCRoomUser() const {
    // This is an intentional original leaf: monitor-login's CMD1 reply uses
    // its fixed zero, rather than deriving a value from GetPCRoomNo().
    return 0;
}

// IDA_EA: 0x827DF1E
// IDA_NAME: _ZN5CUser18incCharacSlotLimitEh
// DEMANGLED: CUser::incCharacSlotLimit(unsigned char)
CUser* CUser::incCharacSlotLimit(unsigned char count) {
    charac_slot_limit_ += count;
    return this;
}

// IDA_EA: 0x828715C
// IDA_NAME: _ZN5CUser14setStdDropRateEi
// DEMANGLED: CUser::setStdDropRate(int)
CUser* CUser::setStdDropRate(int rate) {
    std_drop_rate_ = rate;
    return this;
}

// IDA_EA: 0x8288E7E
// IDA_NAME: _ZN5CUser13isRestingUserEv
// DEMANGLED: CUser::isRestingUser(void)
int CUser::isRestingUser() {
    return resting_user_;
}

// IDA_EA: 0x82A676A
// IDA_NAME: _ZN5CUser27SetSaveRentalInfoToExchangeEb
// DEMANGLED: CUser::SetSaveRentalInfoToExchange(bool)
CUser* CUser::SetSaveRentalInfoToExchange(bool save) {
    save_rental_info_to_exchange_ = save;
    return this;
}
