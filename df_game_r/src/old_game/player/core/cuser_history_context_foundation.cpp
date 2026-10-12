#include "old_game/player/core/cuser.h"

#include <ctime>

#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8695FE4
// IDA_NAME: _ZN15cUserHistoryLogC2Ev
// DEMANGLED: cUserHistoryLog::cUserHistoryLog(void)
cUserHistoryLog::cUserHistoryLog()
    : owner_(0) {}

// IDA_EA: 0x8695FF2
// IDA_NAME: _ZN15cUserHistoryLog7SetUserEP5CUser
// DEMANGLED: cUserHistoryLog::SetUser(CUser *)
cUserHistoryLog* cUserHistoryLog::SetUser(CUser* user) {
    owner_ = user;
    return this;
}

// IDA_EA: 0x868888C
// IDA_NAME: _ZN15cUserHistoryLog5QPAddEiii
// DEMANGLED: cUserHistoryLog::QPAdd(int,int,int)
int cUserHistoryLog::QPAdd(int previous_quest_point,
                           int quest_index,
                           int added_quest_point) {
    return owner_->LogHistory("QP+,%d,%d,%d",
                              previous_quest_point,
                              quest_index,
                              added_quest_point);
}

// IDA_EA: 0x8685C4E
// IDA_NAME: _ZN15cUserHistoryLog16GuildChangeGradeEjPKcjjhh
// DEMANGLED: cUserHistoryLog::GuildChangeGrade(unsigned int,char const*,
//             unsigned int,unsigned int,unsigned char,unsigned char)
int cUserHistoryLog::GuildChangeGrade(unsigned int guild_key,
                                      const char* guild_name,
                                      unsigned int charac_no,
                                      unsigned int target_charac_no,
                                      unsigned char target_grade,
                                      unsigned char prior_grade) {
    std::tm local_time;
    const std::time_t current_time =
        static_cast<std::time_t>(GlobalData::s_systemTime_.getCurSec());
    std::tm* const time_info = localtime_r(&current_time, &local_time);
    return owner_->LogHistory(
        "GuildChangeGrade,\"%d/%d/%d\",%d,\"%s\",%d,%d,%d,%d",
        time_info->tm_year + 1900,
        time_info->tm_mon + 1,
        time_info->tm_mday,
        guild_key,
        guild_name,
        charac_no,
        target_charac_no,
        target_grade,
        prior_grade);
}

// IDA_EA: 0x8696000
// IDA_NAME: _ZN15cUserHistoryLog11getItemXPosEv
// DEMANGLED: cUserHistoryLog::getItemXPos(void)
int cUserHistoryLog::getItemXPos() {
    return item_x_;
}

// IDA_EA: 0x869600C
// IDA_NAME: _ZN15cUserHistoryLog11getItemYPosEv
// DEMANGLED: cUserHistoryLog::getItemYPos(void)
int cUserHistoryLog::getItemYPos() {
    return item_y_;
}
