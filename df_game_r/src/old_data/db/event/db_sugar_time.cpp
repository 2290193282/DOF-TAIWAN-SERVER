#include "old_data/db/event/db_sugar_time.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/taiwan/taiwan_db_payloads.h"

#include <cstring>
#include <ctime>
#include <limits>

namespace sugar_time_db {
namespace {

const char kStateTable[] = "sugar_time_state_0828";
const char kCampaignTable[] = "sugar_time_campaign_0828";
const unsigned int kCampaignId = 1u;
const char kMailTitle[] = "Sugar Time Reward";
const unsigned long long kDailyDeliveryNamespace =
    0xEC00000000000000ULL;
const unsigned long long kFinalDeliveryNamespace =
    0xED00000000000000ULL;

const uint32_t kDailySeconds[kDailyRewardCount] = {
    900u, 1800u, 3600u, 7200u,
};
const uint32_t kDailyItems[kDailyRewardCount] = {
    690001615u, 690001616u, 690001617u, 690001618u,
};
const uint32_t kGoalSeconds[kGoalRewardCount] = {
    90000u, 180000u, 270000u,
};
const uint32_t kGoalItems[kGoalRewardCount] = {
    690001627u, 690001628u, 690001629u,
};

struct PersistedState {
    PersistedState()
        : day_stamp(0u), last_accounted_epoch(0u),
          daily_elapsed(0u), total_elapsed(0u), revision(0u),
          selected_goal(0u), daily_claimed_count(0u),
          final_claimed(0u) {}

    uint32_t day_stamp;
    uint32_t last_accounted_epoch;
    uint32_t daily_elapsed;
    uint32_t total_elapsed;
    uint32_t revision;
    uint32_t selected_goal;
    uint8_t daily_claimed_count;
    uint8_t final_claimed;
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 &&
           server_group < kMaximumServerGroupCount;
}

MySQL* GetDatabase(int handle, int server_group) {
    DBMgr* const manager = GlobalData::s_db_mgr;
    if (!manager || !IsValidServerGroup(server_group)) {
        return 0;
    }
    const ENUM_DB_HANDLE_IDX handle_index =
        static_cast<ENUM_DB_HANDLE_IDX>(handle);
    const ENUM_SERVER_GROUP group =
        static_cast<ENUM_SERVER_GROUP>(server_group);
    return manager->isActiveGlobalDB()
        ? manager->GetDBHandleServerGroup(handle_index, group)
        : manager->GetDBHandle(handle_index, group);
}

bool Begin(MySQL* database) {
    return database && database->set_query("start transaction") &&
           database->exec(true);
}

bool Commit(MySQL* database) {
    return database && database->set_query("commit") &&
           database->exec(true);
}

bool Rollback(MySQL* database) {
    return database && database->set_query("rollback") &&
           database->exec(true);
}

bool IsOwnedCharacter(MySQL* database, const Message& message) {
    if (!database ||
        !database->set_query(
            "seLect 1 from charac_info where charac_no=%u and m_id=%u "
            "and delete_flag=0 limit 2",
            static_cast<unsigned int>(message.charac_no),
            static_cast<unsigned int>(message.account_id)) ||
        !database->exec(true)) {
        return false;
    }
    return database->get_n_rows() == 1 && database->fetch();
}

bool IsValidReward(const MailReward& reward,
                   uint32_t item_id) {
    return reward.item_id == item_id && reward.add_info == 1u &&
           reward.avatar_flag <= 1u && reward.creature_flag <= 1u;
}

bool IsGoal(uint32_t seconds) {
    for (unsigned int index = 0u; index < kGoalRewardCount; ++index) {
        if (kGoalSeconds[index] == seconds) {
            return true;
        }
    }
    return false;
}

int GoalIndex(uint32_t seconds) {
    for (unsigned int index = 0u; index < kGoalRewardCount; ++index) {
        if (kGoalSeconds[index] == seconds) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

bool IsValidConfig(const Message& message) {
    if (message.event_key == 0u) {
        return false;
    }
    for (unsigned int index = 0u; index < kDailyRewardCount; ++index) {
        if (message.process_seconds[index] != kDailySeconds[index] ||
            !IsValidReward(message.daily_rewards[index],
                           kDailyItems[index])) {
            return false;
        }
    }
    for (unsigned int index = 0u; index < kGoalRewardCount; ++index) {
        if (message.goal_seconds[index] != kGoalSeconds[index] ||
            !IsValidReward(message.goal_rewards[index],
                           kGoalItems[index])) {
            return false;
        }
    }
    return true;
}

bool IsValidRequest(const Message& message) {
    if (message.account_id == 0u || message.charac_no == 0u ||
        message.session_token == 0u || message.observed_epoch == 0u ||
        !IsValidServerGroup(message.server_group) ||
        !IsValidConfig(message) || message.daily_reset_hour > 23u ||
        message.day_stamp == 0u || message.final_claimed > 1u ||
        message.daily_claimed_count > kDailyRewardCount) {
        return false;
    }
    switch (message.operation) {
        case OPERATION_LOAD:
        case OPERATION_CHECKPOINT:
        case OPERATION_CLAIM_FINAL:
            return message.requested_stage == 0u;
        case OPERATION_CLAIM_DAY:
            return message.requested_stage >= 1u &&
                   message.requested_stage <= kDailyRewardCount;
        case OPERATION_PICK_FINAL:
            return message.requested_stage == 0u &&
                   IsGoal(message.selected_goal_seconds);
        default:
            return false;
    }
}

bool InsertState(MySQL* database,
                 const Message& message,
                 uint32_t activity_day) {
    return database && database->set_query(
        "inSert ignore into %s(server_group,m_id,event_key,event_day,"
        "last_accounted_epoch,daily_elapsed_seconds,"
        "daily_claimed_count,total_elapsed_seconds,"
        "selected_goal_seconds,final_claimed,revision,created_at) values("
        "%u,%u,%u,str_to_date('%u','%%Y%%m%%d'),0,0,0,0,0,0,0,now())",
        kStateTable,
        static_cast<unsigned int>(message.server_group),
        static_cast<unsigned int>(message.account_id),
        static_cast<unsigned int>(message.event_key),
        static_cast<unsigned int>(activity_day)) &&
        database->exec(true);
}

bool SelectStateForUpdate(MySQL* database,
                          const Message& message,
                          PersistedState& state) {
    if (!database || !database->set_query(
            "seLect date_format(event_day,'%%Y%%m%%d')+0,"
            "last_accounted_epoch,daily_elapsed_seconds,"
            "daily_claimed_count,total_elapsed_seconds,"
            "selected_goal_seconds,final_claimed,revision from %s "
            "where server_group=%u and m_id=%u and event_key=%u "
            "for update",
            kStateTable,
            static_cast<unsigned int>(message.server_group),
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(message.event_key)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    unsigned int day = 0u;
    unsigned int last = 0u;
    unsigned int daily = 0u;
    unsigned int claimed = 0u;
    unsigned int total = 0u;
    unsigned int selected = 0u;
    unsigned int final_claimed = 0u;
    unsigned int revision = 0u;
    if (!database->get_uint(0, day) ||
        !database->get_uint(1, last) ||
        !database->get_uint(2, daily) ||
        !database->get_uint(3, claimed) ||
        !database->get_uint(4, total) ||
        !database->get_uint(5, selected) ||
        !database->get_uint(6, final_claimed) ||
        !database->get_uint(7, revision) || day == 0u ||
        claimed > kDailyRewardCount || final_claimed > 1u ||
        (selected != 0u && !IsGoal(static_cast<uint32_t>(selected)))) {
        return false;
    }
    state.day_stamp = static_cast<uint32_t>(day);
    state.last_accounted_epoch = static_cast<uint32_t>(last);
    state.daily_elapsed = static_cast<uint32_t>(daily);
    state.daily_claimed_count = static_cast<uint8_t>(claimed);
    state.total_elapsed = static_cast<uint32_t>(total);
    state.selected_goal = static_cast<uint32_t>(selected);
    state.final_claimed = static_cast<uint8_t>(final_claimed);
    state.revision = static_cast<uint32_t>(revision);
    return true;
}

bool ReadCampaignClock(MySQL* database,
                       const Message& message,
                       uint32_t& epoch,
                       uint32_t& day_stamp,
                       bool& active) {
    epoch = 0u;
    day_stamp = 0u;
    active = false;
    if (!database || !database->set_query(
            "seLect unix_timestamp(),"
            "date_format(date_sub(now(),interval reset_hour hour),"
            "'%%Y%%m%%d')+0,reset_hour from %s where campaign_id=%u "
            "and enabled=1 and event_key=%u and reset_hour=%u "
            "and starts_at is not null and ends_at is not null "
            "and now()>=starts_at and now()<ends_at limit 2 for update",
            kCampaignTable, kCampaignId,
            static_cast<unsigned int>(message.event_key),
            static_cast<unsigned int>(message.daily_reset_hour)) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) {
        return true;
    }
    if (database->get_n_rows() != 1 || !database->fetch()) {
        return false;
    }
    unsigned int epoch_value = 0u;
    unsigned int day_value = 0u;
    unsigned int reset_hour = 0u;
    if (!database->get_uint(0, epoch_value) ||
        !database->get_uint(1, day_value) ||
        !database->get_uint(2, reset_hour) || epoch_value == 0u ||
        day_value == 0u ||
        reset_hour != static_cast<unsigned int>(message.daily_reset_hour)) {
        return false;
    }
    epoch = static_cast<uint32_t>(epoch_value);
    day_stamp = static_cast<uint32_t>(day_value);
    active = true;
    return true;
}

uint32_t ActivityDayStamp(uint32_t epoch, uint8_t reset_hour) {
    const uint32_t shift = static_cast<uint32_t>(reset_hour) * 3600u;
    if (epoch <= shift) {
        return 0u;
    }
    const time_t value = static_cast<time_t>(epoch - shift);
    tm local_time;
    tm* const parsed = localtime_r(&value, &local_time);
    return parsed
        ? static_cast<uint32_t>((parsed->tm_year + 1900) * 10000 +
                                (parsed->tm_mon + 1) * 100 +
                                parsed->tm_mday)
        : 0u;
}

uint32_t SecondsSinceActivityDayStart(uint32_t epoch,
                                      uint8_t reset_hour) {
    const uint32_t shift = static_cast<uint32_t>(reset_hour) * 3600u;
    if (epoch <= shift) {
        return 0u;
    }
    const time_t value = static_cast<time_t>(epoch - shift);
    tm local_time;
    tm* const parsed = localtime_r(&value, &local_time);
    return parsed
        ? static_cast<uint32_t>(parsed->tm_hour * 3600 +
                                parsed->tm_min * 60 +
                                parsed->tm_sec)
        : 0u;
}

bool CalculateDeltas(uint32_t last,
                     uint32_t observed,
                     uint8_t reset_hour,
                     uint32_t& daily_delta,
                     uint32_t& total_delta) {
    daily_delta = 0u;
    total_delta = 0u;
    if (observed == 0u || observed <= last) {
        return observed != 0u;
    }
    total_delta = observed - last;
    if (total_delta > 604800u) {
        return false;
    }
    if (last == 0u) {
        total_delta = 0u;
        return true;
    }
    const uint32_t previous_day = ActivityDayStamp(last, reset_hour);
    const uint32_t observed_day = ActivityDayStamp(observed, reset_hour);
    if (previous_day == 0u || observed_day == 0u) {
        return false;
    }
    if (previous_day == observed_day) {
        daily_delta = total_delta;
    } else {
        daily_delta = SecondsSinceActivityDayStart(observed, reset_hour);
        if (daily_delta > total_delta) {
            daily_delta = total_delta;
        }
    }
    return true;
}

bool SameState(const PersistedState& left,
               const PersistedState& right) {
    return left.day_stamp == right.day_stamp &&
           left.last_accounted_epoch == right.last_accounted_epoch &&
           left.daily_elapsed == right.daily_elapsed &&
           left.total_elapsed == right.total_elapsed &&
           left.revision == right.revision &&
           left.selected_goal == right.selected_goal &&
           left.daily_claimed_count == right.daily_claimed_count &&
           left.final_claimed == right.final_claimed;
}

bool UpdateState(MySQL* database,
                 const Message& message,
                 const PersistedState& before,
                 const PersistedState& after) {
    if (SameState(before, after)) {
        return true;
    }
    if (!database) {
        return false;
    }
    return database->set_query(
        "upDate %s set event_day=str_to_date('%u','%%Y%%m%%d'),"
        "last_accounted_epoch=%u,"
        "daily_elapsed_seconds=%u,daily_claimed_count=%u,"
        "total_elapsed_seconds=%u,selected_goal_seconds=%u,"
        "final_claimed=%u,revision=%u where server_group=%u and "
        "m_id=%u and event_key=%u and revision=%u",
        kStateTable,
        static_cast<unsigned int>(after.day_stamp),
        static_cast<unsigned int>(after.last_accounted_epoch),
        static_cast<unsigned int>(after.daily_elapsed),
        static_cast<unsigned int>(after.daily_claimed_count),
        static_cast<unsigned int>(after.total_elapsed),
        static_cast<unsigned int>(after.selected_goal),
        static_cast<unsigned int>(after.final_claimed),
        static_cast<unsigned int>(after.revision),
        static_cast<unsigned int>(message.server_group),
        static_cast<unsigned int>(message.account_id),
        static_cast<unsigned int>(message.event_key),
        static_cast<unsigned int>(before.revision)) &&
        database->exec(true) && database->getAffectedRowCount() == 1u;
}

void CopyState(const PersistedState& state, Message& response) {
    response.day_stamp = state.day_stamp;
    response.daily_elapsed_seconds = state.daily_elapsed;
    response.daily_claimed_count = state.daily_claimed_count;
    response.total_elapsed_seconds = state.total_elapsed;
    response.selected_goal_seconds = state.selected_goal;
    response.final_claimed = state.final_claimed;
    response.revision = state.revision;
}

void Mix32(unsigned long long& hash, uint32_t value) {
    for (unsigned int byte = 0u; byte < 4u; ++byte) {
        hash ^= static_cast<unsigned long long>(
            (value >> (byte * 8u)) & 0xFFu);
        hash *= 1099511628211ULL;
    }
}

unsigned long long DeliveryKey(const Message& message,
                               bool final_reward,
                               uint32_t discriminator) {
    unsigned long long hash = 1469598103934665603ULL;
    Mix32(hash, static_cast<uint32_t>(message.server_group));
    Mix32(hash, message.account_id);
    Mix32(hash, message.event_key);
    Mix32(hash, discriminator);
    Mix32(hash, final_reward ? 0x46494E31u : 0x44415931u);
    return (final_reward ? kFinalDeliveryNamespace :
                           kDailyDeliveryNamespace) |
           (hash & 0x00FFFFFFFFFFFFFFULL);
}

bool FindDelivery(MySQL* database,
                  const Message& message,
                  const MailReward& reward,
                  bool final_reward,
                  uint32_t discriminator,
                  bool& found) {
    found = false;
    if (!database || !database->set_query(
            "seLect receive_charac_no,item_id,add_info from postal "
            "where auction_id=%llu limit 2",
            DeliveryKey(message, final_reward, discriminator)) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) {
        return true;
    }
    unsigned int receiver = 0u;
    unsigned int item_id = 0u;
    unsigned int add_info = 0u;
    // auction_id is account/event/reward scoped and globally unique.  A
    // committed delivery remains authoritative even when a lost ACK is
    // retried after the account selects a different character: validate the
    // original recipient record, but never require it to equal the current
    // request character.
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, receiver) ||
        !database->get_uint(1, item_id) ||
        !database->get_uint(2, add_info) ||
        receiver == 0u || item_id != reward.item_id ||
        add_info != reward.add_info) {
        return false;
    }
    found = true;
    return true;
}

bool InsertDelivery(MySQL* database,
                    const Message& message,
                    const MailReward& reward,
                    bool final_reward,
                    uint32_t discriminator) {
    char escaped_title[sizeof(kMailTitle) * 2u + 1u] = {};
    if (!database ||
        database->escape_string(escaped_title, kMailTitle) < 0) {
        return false;
    }
    char* const item_guid = database->blob_to_str(
        0, const_cast<uint8_t*>(reward.item_guid),
        static_cast<int>(sizeof(reward.item_guid)));
    char* const random_option = database->blob_to_str(
        1, const_cast<uint8_t*>(reward.random_option),
        static_cast<int>(sizeof(reward.random_option)));
    return item_guid && random_option && database->set_query(
        "inSert into postal(occ_time,send_charac_no,receive_charac_no,"
        "seal_flag,item_id,add_info,endurance,upgrade,amplify_option,"
        "amplify_value,gold,send_charac_name,creature_flag,avata_flag,"
        "letter_id,extend_info,item_guid,auction_id,random_option,"
        "chronicle_value,seperate_upgrade,unlimit_flag,type) values("
        "now(),0,%u,%u,%u,%u,%u,%u,%u,%u,0,'%s',%u,%u,0,%u,"
        "'%s',%llu,'%s',%u,%u,0,0)",
        static_cast<unsigned int>(message.charac_no),
        static_cast<unsigned int>(reward.seal_flag),
        static_cast<unsigned int>(reward.item_id),
        static_cast<unsigned int>(reward.add_info),
        static_cast<unsigned int>(reward.endurance),
        static_cast<unsigned int>(reward.item_attr),
        static_cast<unsigned int>(reward.amplify_option),
        static_cast<unsigned int>(reward.amplify_value),
        escaped_title,
        static_cast<unsigned int>(reward.creature_flag),
        static_cast<unsigned int>(reward.avatar_flag),
        static_cast<unsigned int>(reward.extend_info),
        item_guid,
        DeliveryKey(message, final_reward, discriminator),
        random_option,
        static_cast<unsigned int>(reward.chronicle_value),
        static_cast<unsigned int>(reward.separate_upgrade)) &&
        database->exec(true);
}

bool EnsureDelivery(MySQL* database,
                    const Message& message,
                    const MailReward& reward,
                    bool final_reward,
                    uint32_t discriminator,
                    bool& inserted) {
    inserted = false;
    bool found = false;
    if (!FindDelivery(database, message, reward, final_reward,
                      discriminator, found)) {
        return false;
    }
    if (found) {
        return true;
    }
    if (!InsertDelivery(database, message, reward, final_reward,
                        discriminator)) {
        return FindDelivery(database, message, reward, final_reward,
                            discriminator, found) && found;
    }
    inserted = true;
    return true;
}

bool HandleLoad(MySQL* database, Message& response) {
    if (!IsOwnedCharacter(database, response) || !Begin(database)) {
        Rollback(database);
        return false;
    }
    uint32_t database_epoch = 0u;
    uint32_t current_day = 0u;
    bool campaign_active = false;
    if (!ReadCampaignClock(database, response, database_epoch,
                           current_day, campaign_active)) {
        Rollback(database);
        return false;
    }
    if (!campaign_active) {
        Rollback(database);
        response.result = RESULT_CAMPAIGN_INACTIVE;
        return true;
    }
    PersistedState before;
    if (!InsertState(database, response, current_day) ||
        !SelectStateForUpdate(database, response, before) ||
        response.observed_epoch > database_epoch + 300u) {
        Rollback(database);
        return false;
    }
    PersistedState after = before;
    const bool day_changed = before.day_stamp != current_day;
    if (day_changed) {
        after.day_stamp = current_day;
        after.daily_elapsed = 0u;
        after.daily_claimed_count = 0u;
    }
    if (response.observed_epoch > after.last_accounted_epoch) {
        after.last_accounted_epoch = response.observed_epoch;
    }
    const bool business_changed = day_changed;
    if (business_changed) {
        if (before.revision == std::numeric_limits<uint32_t>::max()) {
            Rollback(database);
            return false;
        }
        after.revision = before.revision + 1u;
    }
    if (!UpdateState(database, response, before, after) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    CopyState(after, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleMutation(MySQL* database,
                    MySQL* mail_database,
                    Message& response) {
    if (!IsOwnedCharacter(database, response) || !Begin(database)) {
        Rollback(database);
        return false;
    }
    uint32_t database_epoch = 0u;
    uint32_t current_day = 0u;
    bool campaign_active = false;
    if (!ReadCampaignClock(database, response, database_epoch,
                           current_day, campaign_active)) {
        Rollback(database);
        return false;
    }
    if (!campaign_active) {
        Rollback(database);
        response.result = RESULT_CAMPAIGN_INACTIVE;
        return true;
    }
    PersistedState before;
    if (!InsertState(database, response, current_day) ||
        !SelectStateForUpdate(database, response, before) ||
        response.observed_epoch > database_epoch + 300u) {
        Rollback(database);
        return false;
    }
    PersistedState after = before;
    const bool day_changed = before.day_stamp != current_day;
    if (day_changed) {
        after.day_stamp = current_day;
        after.daily_elapsed = 0u;
        after.daily_claimed_count = 0u;
    }

    uint32_t daily_delta = 0u;
    uint32_t total_delta = 0u;
    if (!CalculateDeltas(after.last_accounted_epoch,
                         response.observed_epoch,
                         response.daily_reset_hour,
                         daily_delta, total_delta)) {
        Rollback(database);
        return false;
    }
    if (ActivityDayStamp(response.observed_epoch,
                         response.daily_reset_hour) != current_day) {
        daily_delta = 0u;
    }
    if (response.observed_epoch > after.last_accounted_epoch) {
        after.last_accounted_epoch = response.observed_epoch;
    }
    if (after.daily_elapsed >
            std::numeric_limits<uint32_t>::max() - daily_delta ||
        after.total_elapsed >
            std::numeric_limits<uint32_t>::max() - total_delta) {
        Rollback(database);
        return false;
    }
    after.daily_elapsed += daily_delta;
    after.total_elapsed += total_delta;

    if (response.operation == OPERATION_CLAIM_DAY) {
        const uint8_t stage = response.requested_stage;
        if (!mail_database || stage == 0u ||
            stage > kDailyRewardCount ||
            stage > static_cast<uint8_t>(
                        after.daily_claimed_count + 1u) ||
            (stage == static_cast<uint8_t>(
                          after.daily_claimed_count + 1u) &&
             after.daily_elapsed < response.process_seconds[stage - 1u])) {
            CopyState(before, response);
            response.result = RESULT_INVALID_ELIGIBILITY;
            Rollback(database);
            return true;
        }
        bool inserted = false;
        if (!EnsureDelivery(mail_database, response,
                            response.daily_rewards[stage - 1u], false,
                            after.day_stamp * 10u + stage, inserted)) {
            Rollback(database);
            return false;
        }
        response.new_mail = inserted ? 1u : 0u;
        if (stage <= after.daily_claimed_count) {
            response.replayed = 1u;
        } else {
            after.daily_claimed_count = stage;
        }
    } else if (response.operation == OPERATION_PICK_FINAL) {
        const uint32_t requested = response.selected_goal_seconds;
        if (!IsGoal(requested) ||
            (after.selected_goal != 0u &&
             after.selected_goal != requested)) {
            CopyState(before, response);
            response.result = RESULT_INVALID_ELIGIBILITY;
            Rollback(database);
            return true;
        }
        if (after.selected_goal == requested) {
            response.replayed = 1u;
        } else {
            after.selected_goal = requested;
        }
    } else if (response.operation == OPERATION_CLAIM_FINAL) {
        const int goal_index = GoalIndex(after.selected_goal);
        if (!mail_database || goal_index < 0 ||
            after.total_elapsed < after.selected_goal) {
            CopyState(before, response);
            response.result = RESULT_INVALID_ELIGIBILITY;
            Rollback(database);
            return true;
        }
        bool inserted = false;
        if (!EnsureDelivery(
                mail_database, response,
                response.goal_rewards[static_cast<unsigned int>(goal_index)],
                true, after.selected_goal, inserted)) {
            Rollback(database);
            return false;
        }
        response.new_mail = inserted ? 1u : 0u;
        if (after.final_claimed != 0u) {
            response.replayed = 1u;
        } else {
            after.final_claimed = 1u;
        }
    } else if (response.operation != OPERATION_CHECKPOINT) {
        Rollback(database);
        return false;
    }

    const bool business_changed = day_changed ||
        before.daily_elapsed != after.daily_elapsed ||
        before.daily_claimed_count != after.daily_claimed_count ||
        before.total_elapsed != after.total_elapsed ||
        before.selected_goal != after.selected_goal ||
        before.final_claimed != after.final_claimed;
    if (business_changed) {
        if (before.revision == std::numeric_limits<uint32_t>::max()) {
            Rollback(database);
            return false;
        }
        after.revision = before.revision + 1u;
    }
    if (!UpdateState(database, response, before, after) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    CopyState(after, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool SendResult(int user_idx, const Message& response) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx < 0 || !pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    Message* const destination = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!destination) {
        return false;
    }
    *destination = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

}  // namespace

void ClearMessage(Message& message) {
    std::memset(&message, 0, sizeof(message));
    message.server_group = -1;
    message.result = RESULT_DATABASE_FAILURE;
}

DBSugarTime::DBSugarTime()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBSugarTime::~DBSugarTime() {}

bool DBSugarTime::QueueRequest(int user_idx,
                                   const Message& request) {
    if (user_idx < 0 || !IsValidRequest(request)) {
        return false;
    }
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    Message* const destination = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!destination) {
        return false;
    }
    *destination = request;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

int DBSugarTime::dispatch(int signal,
                              int user_idx,
                              Stream* stream) {
    (void)signal;
    if (user_idx < 0 || !stream ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return 0;
    }
    Message request;
    if (stream->get_binary(&request,
                           static_cast<int>(sizeof(request))) != 1 ||
        !IsValidRequest(request)) {
        return 0;
    }
    Message response = request;
    response.result = RESULT_DATABASE_FAILURE;
    response.replayed = 0u;
    response.new_mail = 0u;
    MySQL* const game_database = GetDatabase(DB_HANDLE_2,
                                             request.server_group);
    bool handled = false;
    if (game_database && request.operation == OPERATION_LOAD) {
        handled = HandleLoad(game_database, response);
    } else if (game_database) {
        MySQL* mail_database = 0;
        if (request.operation == OPERATION_CLAIM_DAY ||
            request.operation == OPERATION_CLAIM_FINAL) {
            mail_database = GetDatabase(DB_HANDLE_3,
                                        request.server_group);
            if (!mail_database) {
                return SendResult(user_idx, response) ? 1 : 0;
            }
        }
        handled = HandleMutation(game_database, mail_database, response);
    }
    if (!handled) {
        response.result = RESULT_DATABASE_FAILURE;
    }
    return SendResult(user_idx, response) ? 1 : 0;
}

}  // namespace sugar_time_db_0828
