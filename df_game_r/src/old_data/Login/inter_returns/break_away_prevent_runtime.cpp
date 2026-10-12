#include "old_data/Login/inter_returns/break_away_prevent_runtime.h"

#include "old_data/Login/inter_returns/inter_load_etc_break_away_reward.h"

#include <algorithm>
#include <cstring>
#include <string>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_data/db/account/db_update_break_away_ruck_point.h"
#include "old_data/db/account/db_update_break_away_uv_list.h"
#include "old_data/db/account/clocalchina_db_break_away_reset.h"
#include "old_data/db/account/login_payloads.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

namespace break_away_prevent {

// IDA_EA: 0x831C3E0
// IDA_NAME: _ZN18break_away_prevent23CBreakAwayPreventSystemC2Ev
CBreakAwayPreventSystem::CBreakAwayPreventSystem()
    : SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER(),
      reward_data_(),
      reward_keys_(),
      uv_list_() {
    reward_data_.clear();
}

// IDA_EA: 0x831C486
// IDA_NAME: _ZN18break_away_prevent23CBreakAwayPreventSystemD2Ev
CBreakAwayPreventSystem::~CBreakAwayPreventSystem() {
}

// IDA_EA: 0x831C49E
// IDA_NAME: _ZN18break_away_prevent23CBreakAwayPreventSystem10loadDbDataEP37SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER
bool CBreakAwayPreventSystem::loadDbData(
    SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER* source) {
    *static_cast<SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER*>(this) = *source;
    reward_data_.clear();

    int key_count = 0;
    for (unsigned int index = 0; index < reward_count; ++index) {
        const SIG_BREAK_AWAY_PREVENT_REWARD_MANAGER& source_row =
            rewards_10[index];
        STRewardDataKey key;
        key.first = source_row.min_day;
        key.second = source_row.max_day;
        key.third = source_row.quest_id;

        // STRewardData is shared with the old event table.  In this exact
        // BreakAway use its five dwords are, in order: min, max, item,
        // add-info and luck point.  The old selectReward body reads them by
        // this physical order rather than by an alternate record type.
        STRewardData row;
        row.item_id = source_row.min_value;
        row.item_count = source_row.max_value;
        row.item_period = static_cast<int>(source_row.item_id);
        row.reward_type = source_row.add_info;
        row.reward_value = source_row.luck_point;

        std::map<STRewardDataKey, STRewardDataList>::iterator found =
            reward_data_.find(key);
        if (found != reward_data_.end()) {
            found->second.reward_data.push_back(row);
        } else {
            STRewardDataList list;
            list.reward_data.push_back(row);
            reward_data_.insert(std::make_pair(key, list));
            reward_keys_[key_count++] = key;
        }
    }
    return reward_count > 0;
}

// IDA_EA: 0x831C794
// IDA_NAME: _ZN18break_away_prevent23CBreakAwayPreventSystem17getDiffConnectDayEj
unsigned int CBreakAwayPreventSystem::getDiffConnectDay(
    unsigned int timestamp) {
    if (!timestamp) {
        return 0;
    }
    const long previous_schedule = getPrevScheduleTime(0, timestamp);
    const long now = GlobalData::system_time()->getCurSec();
    const long days = (now - previous_schedule) / 86400;
    return static_cast<unsigned int>(std::max(0l, days));
}

// IDA_EA: 0x831C810
// IDA_NAME: _ZN18break_away_prevent23CBreakAwayPreventSystem21calBreakAwayUserPointEv
bool CBreakAwayPreventSystem::calBreakAwayUserPoint() {
    int weekday_count = 0;
    int weekend_count = 0;
    for (unsigned int index = 15; index <= 28; ++index) {
        if (uv_list_[index] == '1') {
            if (index <= 21) {
                ++weekday_count;
            } else {
                ++weekend_count;
            }
        }
    }
    (void)weekday_count;
    return weekend_count == 1;
}

// IDA_EA: 0x831C872
// IDA_NAME: _ZN18break_away_prevent23CBreakAwayPreventSystem12selectRewardEiiP5CUser
int CBreakAwayPreventSystem::selectReward(int reward_kind,
                                          int reward_point,
                                          CUser* user) {
    const std::map<STRewardDataKey, STRewardDataList>::const_iterator found =
        reward_data_.find(reward_keys_[reward_kind]);
    if (found == reward_data_.end() || reward_point < 0) {
        return 0;
    }

    STBreakAwayRewards selected;
    selected.reward_order = static_cast<unsigned int>(reward_kind);
    for (std::list<STRewardData>::const_iterator it =
             found->second.reward_data.begin();
         it != found->second.reward_data.end();
         ++it) {
        const STRewardData& row = *it;
        selected.item_id = static_cast<unsigned int>(row.item_period);
        selected.item_count = static_cast<unsigned int>(row.reward_type);
        selected.luck_point = static_cast<unsigned int>(row.reward_value);
        selected.reward_point = static_cast<unsigned int>(reward_point);
        if (row.item_id > reward_point) {
            return 0;
        }
        if (row.item_count >= reward_point) {
            user->setBreakAwayRewards(selected);
            return 1;
        }
    }
    user->setBreakAwayRewards(selected);
    return 1;
}

// IDA_EA: 0x831CA5A
// IDA_NAME: _ZN18break_away_prevent23CBreakAwayPreventSystem15GM_UpdateUvListEjj
int CBreakAwayPreventSystem::GM_UpdateUvList(unsigned int account_id,
                                              unsigned int update_kind) {
    std::memset(uv_list_, 0, sizeof(uv_list_));
    if (update_kind >= 1 && update_kind <= 3) {
        std::memcpy(uv_list_, "000000000000000000111000010001",
                    sizeof(uv_list_));
    } else if (update_kind == 4) {
        std::memcpy(uv_list_, "000000000000000000111000001000",
                    sizeof(uv_list_));
    }
    DB_UpdateBreakAwayUvList::makeRequest(account_id, uv_list_);
    CLocalChina_DB_BreakAwayReset::makeRequest(account_id);
    return 1;
}

// IDA_EA: 0x831CB56
// IDA_NAME: _ZN18break_away_prevent23CBreakAwayPreventSystem12updateUvListEPK14SIG_LOGIN_DATA
int CBreakAwayPreventSystem::updateUvList(const SIG_LOGIN_DATA* login_data) {
    int update_day = static_cast<int>(
        getDiffConnectDay(login_data->break_away_last_update_time));
    const unsigned int last_play_day =
        getDiffConnectDay(static_cast<unsigned int>(login_data->last_play_time));

    if (!update_day) {
        if (login_data->break_away_last_update_time) {
            std::memset(uv_list_, 0, sizeof(uv_list_));
            std::memcpy(uv_list_, login_data->break_away_play_info,
                        sizeof(uv_list_));
            return 1;
        }
        update_day = last_play_day ? static_cast<int>(last_play_day) : 1;
    }

    if (update_day > 29) {
        std::memset(uv_list_, '0', sizeof(uv_list_));
        uv_list_[29] = '1';
        uv_list_[30] = 0;
    } else {
        std::string updated(login_data->break_away_play_info,
                            static_cast<size_t>(update_day),
                            0x1e);
        for (int index = 0; index < update_day - 1; ++index) {
            updated.push_back('0');
        }
        updated.push_back('1');
        if (updated.size() != 30u) {
            return 0;
        }
        std::memset(uv_list_, 0, sizeof(uv_list_));
        std::strncpy(uv_list_, updated.c_str(), updated.size());
    }

    DB_UpdateBreakAwayUvList::makeRequest(login_data->member_id, uv_list_);
    return 1;
}

// IDA_EA: 0x831CE5E
// IDA_NAME: _ZN18break_away_prevent23CBreakAwayPreventSystem14updateUsedCeraEjj
void CBreakAwayPreventSystem::updateUsedCera(unsigned int account_id,
                                              unsigned int used_cera) {
    DB_UpdateBreakAwayAccruedCera::makeRequest(account_id, used_cera);
}

// IDA_EA: 0x831CE78
// IDA_NAME: _ZN18break_away_prevent23CBreakAwayPreventSystem27startBreakAwayPreventSystemEPK14SIG_LOGIN_DATAP5CUser
int CBreakAwayPreventSystem::startBreakAwayPreventSystem(
    const SIG_LOGIN_DATA* login_data,
    CUser* user) {
    const unsigned int connect_days =
        getDiffConnectDay(login_data->break_away_last_reward_time);

    if (!login_data->break_away_last_reward_time ||
        connect_days >= static_cast<unsigned int>(next_reward_day)) {
        if (!calBreakAwayUserPoint()) {
            return 0;
        }
        if (login_data->break_away_accrue_cera >= 0) {
            if (selectReward(0, login_data->break_away_accrue_cera, user)) {
                user->setBreakAwayRewardData(true);
                return reinterpret_cast<int>(
                    user->setBreakAwayRewardOrder(
                        static_cast<ENUM_BREAK_AWAY_REWARD_ORDER>(1)));
            }
            return 0;
        }
        if (login_data->break_away_luck_point) {
            return login_data->break_away_luck_point;
        }
        return reinterpret_cast<int>(user->setBreakAwayRewardData(false));
    }

    if (login_data->break_away_add_info ||
        !login_data->break_away_quest_time ||
        login_data->break_away_first_reward_time <=
            login_data->break_away_second_reward_time) {
        if (!login_data->break_away_add_info &&
            login_data->break_away_quest_time &&
            login_data->break_away_first_reward_time <
                login_data->break_away_second_reward_time &&
            weekend_var_x <= static_cast<int>(connect_days) &&
            weekend_var_y >= static_cast<int>(connect_days)) {
            if (login_data->break_away_accrue_cera < 0) {
                if (login_data->break_away_luck_point) {
                    return login_data->break_away_luck_point;
                }
                return reinterpret_cast<int>(user->setBreakAwayRewardData(false));
            }
            if (selectReward(4, login_data->break_away_accrue_cera, user)) {
                return reinterpret_cast<int>(user->setBreakAwayRewardData(true));
            }
        }
        return 0;
    }

    if (weekday_var_a <= static_cast<int>(connect_days) &&
        weekday_var_b >= static_cast<int>(connect_days)) {
        if (login_data->break_away_accrue_cera < 0) {
            if (login_data->break_away_luck_point) {
                return login_data->break_away_luck_point;
            }
            return reinterpret_cast<int>(user->setBreakAwayRewardData(false));
        }
        if (selectReward(2, login_data->break_away_accrue_cera, user)) {
            user->setBreakAwayRewardData(true);
            return reinterpret_cast<int>(
                user->setBreakAwayRewardOrder(
                    static_cast<ENUM_BREAK_AWAY_REWARD_ORDER>(3)));
        }
    }
    return 0;
}

// IDA_EA: 0x831D164
// IDA_NAME: _ZN18break_away_prevent23CBreakAwayPreventSystem31giveBreakAwayDungeonClearRewardEP5CUser
// DEMANGLED: break_away_prevent::CBreakAwayPreventSystem::giveBreakAwayDungeonClearReward(CUser *)
int CBreakAwayPreventSystem::giveBreakAwayDungeonClearReward(CUser* user) {
    if (user->getBreakAwayRewardOrder() == 1) {
        if (!selectReward(1, user->getBreakAwayAccureCera(), user)) {
            return 0;
        }
        return Inter_LoadEtc::giveBreakAwayReward(user);
    }

    if (!selectReward(3, user->getBreakAwayAccureCera(), user)) {
        return 0;
    }
    return Inter_LoadEtc::giveBreakAwayReward(user);
}

}  // namespace break_away_prevent
