#include "common/manager/global_data.h"
#include "common/manager/user.h"
#include "old_game/player/core/cuser.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/system_time.h"
#include "old_game/social/party/party.h"

#include <stdint.h>
#include <time.h>


// IDA_EA: 0x868F1C0
// IDA_NAME: _ZN5CUser18NextCharacMoneyLogEi
// DEMANGLED: CUser::NextCharacMoneyLog(int)
int CUser::NextCharacMoneyLog(int timer) {
    int result = static_cast<int>(charac_money_logs_[money_log_index_].tick);
    if (result) {
        struct tm current_day;
        struct tm recorded_day;
        const time_t current = static_cast<time_t>(timer);
        localtime_r(&current, &current_day);
        const time_t recorded = static_cast<time_t>(
            charac_money_logs_[money_log_index_].tick);
        localtime_r(&recorded, &recorded_day);
        result = recorded_day.tm_mday;
        if (current_day.tm_mday != recorded_day.tm_mday) {
            result = static_cast<int>(++money_log_index_);
            if (result > 6) {
                money_log_index_ = 6;
                result = static_cast<int>(reinterpret_cast<uintptr_t>(this));
            }
        }
    }
    return result;
}

// IDA_EA: 0x868F3BA
// IDA_NAME: _ZN5CUser21exceptedStateMoneyLogEv
// DEMANGLED: CUser::exceptedStateMoneyLog(void)
bool CUser::exceptedStateMoneyLog() {
    CParty* const party = static_cast<CParty*>(GetParty());
    if (party) {
        if (party->battle_field_.village_monster_uid_) {
            return 1;
        }
        const CDungeon* const dungeon = party->battle_field_.dungeon_;
        if (dungeon && dungeon->circulation_statistic_class_) {
            return 1;
        }
    }
    if (get_state() == 8) {
        return 1;
    }
    return getCurCharacVill() == 7;
}

// IDA_EA: 0x868F266
// IDA_NAME: _ZN5CUser12AddMoneyPlusEi
// DEMANGLED: CUser::AddMoneyPlus(int)
int* CUser::AddMoneyPlus(int money) {
    if (exceptedStateMoneyLog()) {
        return reinterpret_cast<int*>(1);
    }
    const time_t timer = static_cast<time_t>(GlobalData::system_time()->getCurSec());
    NextCharacMoneyLog(static_cast<int>(timer));
    CharacMoneyLog& entry = charac_money_logs_[money_log_index_];
    entry.tick = static_cast<uint32_t>(timer);
    entry.plus_money += static_cast<uint32_t>(money);
    return reinterpret_cast<int*>(&entry.plus_money);
}

// IDA_EA: 0x868F310
// IDA_NAME: _ZN5CUser13AddMoneyMinusEi
// DEMANGLED: CUser::AddMoneyMinus(int)
int* CUser::AddMoneyMinus(int money) {
    if (exceptedStateMoneyLog()) {
        return reinterpret_cast<int*>(1);
    }
    const time_t timer = static_cast<time_t>(GlobalData::system_time()->getCurSec());
    NextCharacMoneyLog(static_cast<int>(timer));
    CharacMoneyLog& entry = charac_money_logs_[money_log_index_];
    entry.tick = static_cast<uint32_t>(timer);
    entry.minus_money += static_cast<uint32_t>(money);
    return reinterpret_cast<int*>(&entry.minus_money);
}
