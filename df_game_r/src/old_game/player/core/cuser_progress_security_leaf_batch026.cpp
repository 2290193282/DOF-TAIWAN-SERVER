#include "old_game/player/core/cuser.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "common/manager/global_data.h"
#include "old_data/db/event/db_insert_give_box_event_reward.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/premium_runtime.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x863BD5C
// IDA_NAME: _ZNK8WongWork12CUserPremium20GetTotalPremiumCountEv
// DEMANGLED: WongWork::CUserPremium::GetTotalPremiumCount(void)const
int WongWork::CUserPremium::GetTotalPremiumCount() const {
    return advantage_premium_count_ + normal_premium_count_;
}

// IDA_EA: 0x863BEA2
// IDA_NAME: _ZNK5CUser13IsPremiumUserEv
// DEMANGLED: CUser::IsPremiumUser(void)const
bool CUser::IsPremiumUser() const {
    return user_premium_.GetTotalPremiumCount() != 0;
}

// IDA_EA: 0x8649B44
// IDA_NAME: _ZN5CUser22VerifyCleanPadVeriDataEj
// DEMANGLED: CUser::VerifyCleanPadVeriData(unsigned int)
bool CUser::VerifyCleanPadVeriData(unsigned int verification_data) {
    return verification_data ==
           static_cast<unsigned int>(clean_pad_verification_operation_) +
               clean_pad_verification_first_ +
               clean_pad_verification_second_;
}

// IDA_EA: 0x868DD18
// IDA_NAME: _ZN5CUser16setUseCraneStartEb
// DEMANGLED: CUser::setUseCraneStart(bool)
CUser* CUser::setUseCraneStart(bool started) {
    use_crane_start_ = static_cast<uint8_t>(started);
    return this;
}

// IDA_EA: 0x868DD34
// IDA_NAME: _ZNK5CUser15isUseCraneStartEv
// DEMANGLED: CUser::isUseCraneStart(void)const
int CUser::isUseCraneStart() const {
    return use_crane_start_;
}

// IDA_EA: 0x868F5CA
// IDA_NAME: _ZN5CUser25SetProperLevelDungeonUserEv
// DEMANGLED: CUser::SetProperLevelDungeonUser(void)
CUser* CUser::SetProperLevelDungeonUser() {
    proper_level_dungeon_user_ = 1u;
    return this;
}

// IDA_EA: 0x868F5DA
// IDA_NAME: _ZN5CUser27ClearProperLevelDungeonUserEv
// DEMANGLED: CUser::ClearProperLevelDungeonUser(void)
CUser* CUser::ClearProperLevelDungeonUser() {
    proper_level_dungeon_user_ = 0u;
    return this;
}

// IDA_EA: 0x868F5EA
// IDA_NAME: _ZN5CUser24IsProperLevelDungeonUserEv
// DEMANGLED: CUser::IsProperLevelDungeonUser(void)
int CUser::IsProperLevelDungeonUser() {
    return proper_level_dungeon_user_;
}

// IDA_EA: 0x868F5FA
// IDA_NAME: _ZN5CUser27ProcProperLevelDungeonClearEv
// DEMANGLED: CUser::ProcProperLevelDungeonClear(void)
void CUser::ProcProperLevelDungeonClear() {
    GiveMeBoxReward();
}

// IDA_EA: 0x8693758
// IDA_NAME: _ZN5CUser15GiveMeBoxRewardEv
// DEMANGLED: CUser::GiveMeBoxReward(void)
void CUser::GiveMeBoxReward() {
    DBInsertGiveBoxEventReward::makeRequest(this);
}

// IDA_EA: 0x868FA56
// IDA_NAME: _ZN5CUser26enableSaveInformNoticeFlagEv
// DEMANGLED: CUser::enableSaveInformNoticeFlag(void)
CUser* CUser::enableSaveInformNoticeFlag() {
    inform_notice_.save_pending = 1;
    return this;
}

// IDA_EA: 0x8692B04
// IDA_NAME: _ZN5CUser12SetCeraPointEj
// DEMANGLED: CUser::SetCeraPoint(unsigned int)
CUser* CUser::SetCeraPoint(unsigned int cera_point) {
    cera_point_ = cera_point;
    return this;
}

// IDA_EA: 0x8692B58
// IDA_NAME: _ZN5CUser9SetPuUserEb
// DEMANGLED: CUser::SetPuUser(bool)
CUser* CUser::SetPuUser(bool pu_user) {
    pu_user_ = static_cast<uint8_t>(pu_user);
    return this;
}

// IDA_EA: 0x8692B74
// IDA_NAME: _ZN5CUser9GetPuUserEv
// DEMANGLED: CUser::GetPuUser(void)
int CUser::GetPuUser() {
    return pu_user_;
}

// IDA_EA: 0x8692F46
// IDA_NAME: _ZN5CUser24setAllowLocationGeoStateEb
// DEMANGLED: CUser::setAllowLocationGeoState(bool)
CUser* CUser::setAllowLocationGeoState(bool allowed) {
    allow_location_geo_state_ = static_cast<uint8_t>(allowed);
    return this;
}

// IDA_EA: 0x8693178
// IDA_NAME: _ZN5CUser22recipeForceProbabilityEi
// DEMANGLED: CUser::recipeForceProbability(int)
CUser* CUser::recipeForceProbability(int probability) {
    recipe_force_probability_ = probability;
    return this;
}

// IDA_EA: 0x869318A
// IDA_NAME: _ZN5CUser20getRecipeProbabilityEv
// DEMANGLED: CUser::getRecipeProbability(void)
int CUser::getRecipeProbability() {
    return recipe_force_probability_;
}

// IDA_EA: 0x8689850
// IDA_NAME: _ZN5CUser28SetTradePunishOverThirtyDaysEb
// DEMANGLED: CUser::SetTradePunishOverThirtyDays(bool)
CUser* CUser::SetTradePunishOverThirtyDays(bool over_thirty_days) {
    std::printf(" CUser::SetTradePunishOverThirtydays(%c) \n",
                over_thirty_days);
    trade_punish_over_thirty_days_ = over_thirty_days;
    return this;
}

// IDA_EA: 0x8689880
// IDA_NAME: _ZN5CUser27isTradePunishOverThirtyDaysEv
// DEMANGLED: CUser::isTradePunishOverThirtyDays(void)
int CUser::isTradePunishOverThirtyDays() {
    return trade_punish_over_thirty_days_;
}

// IDA_EA: 0x86898C2
// IDA_NAME: _ZN5CUser18DelTradePunishTypeEi
// DEMANGLED: CUser::DelTradePunishType(int)
int CUser::DelTradePunishType(int punish_type) {
    int& current = trade_punish_type_;
    std::printf("DelTradePunishType : (%d) // Prev TradePunishType:  %d\n",
                punish_type,
                current);
    current &= ~punish_type;
    return std::printf("After TradePunishType:  %d\n", current);
}

// IDA_EA: 0x867CB70
// IDA_NAME: _ZN5CUser22setPlayingDungeonIndexEi
// DEMANGLED: CUser::setPlayingDungeonIndex(int)
CUser* CUser::setPlayingDungeonIndex(int dungeon_index) {
    playing_dungeon_index_ = dungeon_index;
    return this;
}

// IDA_EA: 0x867CBC4
// IDA_NAME: _ZN5CUser15setLastPacketIDEi
// DEMANGLED: CUser::setLastPacketID(int)
CUser* CUser::setLastPacketID(int packet_id) {
    last_packet_identity_.value = static_cast<uint32_t>(packet_id);
    return this;
}

// IDA_EA: 0x867CBD6
// IDA_NAME: _ZN5CUser15getLastPacketIDEv
// DEMANGLED: CUser::getLastPacketID(void)
int CUser::getLastPacketID() {
    return static_cast<int32_t>(last_packet_identity_.value);
}

// IDA_EA: 0x866C1CA
// IDA_NAME: _ZN5CUser19IsCompleteGuildFundEv
// DEMANGLED: CUser::IsCompleteGuildFund(void)
bool CUser::IsCompleteGuildFund() {
    return (guild_info_.guild_level > 15u &&
            guild_info_.guild_fund > 9999999u) ||
           guild_info_.guild_fund > 19999999u;
}

// IDA_EA: 0x8697318
// IDA_NAME: _ZN5CUser22UpdateTutorialSkipableEv
// DEMANGLED: CUser::UpdateTutorialSkipable(void)
CUser* CUser::UpdateTutorialSkipable() {
    enableSaveCharacStat();
    return SetTutorialSkipable(1u);
}

// IDA_EA: 0x8568E9C
// IDA_NAME: _ZN5CUser12UpdateRatingEf
// DEMANGLED: CUser::UpdateRating(float)
CUser* CUser::UpdateRating(float rating) {
    enableSaveCharacStat();
    return SetRating(rating);
}

// IDA_EA: 0x8230172
// IDA_NAME: _ZN5CUser13IsGuildMasterEv
// DEMANGLED: CUser::IsGuildMaster(void)
bool CUser::IsGuildMaster() {
    return get_charac_guildkey() != 0 &&
           getCurCharacNo() ==
               static_cast<int>(guild_info_.guild_master_charac_no);
}

// IDA_EA: 0x8692B16
// IDA_NAME: _ZN5CUser12UseCeraPointEj
// DEMANGLED: CUser::UseCeraPoint(unsigned int)
int CUser::UseCeraPoint(unsigned int cera_point) {
    unsigned int& current = cera_point_;
    if (current < cera_point) {
        current = 0;
    } else {
        current -= cera_point;
    }
    return current;
}

// IDA_EA: 0x8230100
// IDA_NAME: _ZN5CUser19get_pvp_exp_of_weekEv
// DEMANGLED: CUser::get_pvp_exp_of_week(void)
int CUser::get_pvp_exp_of_week() {
    const PvpResultType* const result = getPVPResultRefR();
    if (result->values[4] == -1) {
        return result->values[2];
    }
    return result->values[2] - result->values[4];
}

// IDA_EA: 0x867ECF0
// IDA_NAME: _ZN5CUser8SetBirthEPKc
// DEMANGLED: CUser::SetBirth(char const*)
CUser* CUser::SetBirth(const char* birth) {
    char part[3] = {0, 0, 0};
    std::strncpy(part, birth + 2, 2u);
    birth_month_ = static_cast<unsigned char>(std::atoi(part));
    std::strncpy(part, birth + 4, 2u);
    birth_day_ = static_cast<unsigned char>(std::atoi(part));
    return this;
}

// IDA_EA: 0x867EC66
// IDA_NAME: _ZN5CUser14IsOverBirthDayEv
// DEMANGLED: CUser::IsOverBirthDay(void)
bool CUser::IsOverBirthDay() {
    const time_t now =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    struct tm local_tm;
    const struct tm* const current = localtime_r(&now, &local_tm);
    const int month = current->tm_mon + 1;
    const int day = current->tm_mday;
    if (birth_month_ < month) {
        return true;
    }
    return birth_month_ == month && birth_day_ < day;
}

// IDA_EA: 0x866AF86
// IDA_NAME: _ZN5CUser15get_skill_countEv
// DEMANGLED: CUser::get_skill_count(void)
int CUser::get_skill_count() {
    return getCurCharacSkillR()->get_skill_count();
}
