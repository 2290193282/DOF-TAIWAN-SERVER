#include "old_data/db/event/db_event_attendance.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"

#include <cstring>
#include <ctime>
#include <limits>

namespace event_attendance_db {
namespace {

const char kStateTable[] = "event_attendance_reward_0828";
const char kMailTitle[] = "Event Attendance Reward";
const int kServerGroupCount = 0x12;
const unsigned long long kDailyDeliveryNamespace =
    0xE600000000000000ULL;
const unsigned long long kSumDeliveryNamespace =
    0xE700000000000000ULL;

struct PersistedState {
    PersistedState()
        : day_stamp(0u), last_accounted_epoch(0u),
          daily_elapsed(0u), claimed_mask(0u),
          total_elapsed(0u), sum_claimed_mask(0u), revision(0u) {}

    uint32_t day_stamp;
    uint32_t last_accounted_epoch;
    uint32_t daily_elapsed;
    uint8_t claimed_mask;
    uint32_t total_elapsed;
    uint8_t sum_claimed_mask;
    uint32_t revision;
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kServerGroupCount;
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
    // Reward state and mail must never use DBMgr's active-global fallback to
    // DB_HANDLE_LOG.  Require the configured game/mail partition handles.
    return manager->isActiveGlobalDB()
               ? manager->GetDBHandleServerGroup(handle_index, group)
               : manager->GetDBHandle(handle_index, group);
}

bool Rollback(MySQL* database) {
    return database && database->set_query("rollback") &&
           database->exec(true);
}

bool Commit(MySQL* database) {
    return database && database->set_query("commit") &&
           database->exec(true);
}

bool IsOwnedCharacter(MySQL* game_database, const Message& message) {
    if (!game_database ||
        !game_database->set_query(
            "seLect 1 from charac_info where charac_no=%u and m_id=%u "
            "and delete_flag=0 limit 2",
            static_cast<unsigned int>(message.charac_no),
            static_cast<unsigned int>(message.account_id)) ||
        !game_database->exec(true)) {
        return false;
    }
    return game_database->get_n_rows() == 1 && game_database->fetch();
}

bool IsValidReward(const MailReward& reward) {
    return reward.item_id != 0u && reward.add_info != 0u &&
           reward.add_info <=
               static_cast<uint32_t>(std::numeric_limits<int>::max()) &&
           reward.avatar_flag <= 1u && reward.creature_flag <= 1u;
}

bool IsValidConfig(const Message& message) {
    if (message.event_key == 0u) {
        return false;
    }
    uint32_t previous = 0u;
    for (unsigned int index = 0; index < kDailyRewardCount; ++index) {
        if (message.process_seconds[index] <= previous ||
            !IsValidReward(message.daily_rewards[index])) {
            return false;
        }
        previous = message.process_seconds[index];
    }
    previous = 0u;
    for (unsigned int index = 0; index < kSumRewardCount; ++index) {
        if (message.sum_seconds[index] <= previous ||
            !IsValidReward(message.sum_rewards[index])) {
            return false;
        }
        previous = message.sum_seconds[index];
    }
    return true;
}

bool IsValidMutation(const Message& message) {
    if (!IsValidConfig(message) ||
        message.observed_epoch == 0u ||
        message.client_stage > kDailyRewardCount) {
        return false;
    }
    if (message.operation == kOperationCheckpoint) {
        return message.requested_tier == 0u;
    }
    return message.operation == kOperationClaim &&
           message.requested_tier >= 1u &&
           message.requested_tier <= kDailyRewardCount &&
           message.requested_tier <= message.client_stage;
}

void CopyStateToResponse(const PersistedState& state, Message& response) {
    response.day_stamp = state.day_stamp;
    response.daily_elapsed_seconds = state.daily_elapsed;
    response.total_elapsed_seconds = state.total_elapsed;
    response.revision = state.revision;
    response.claimed_mask = state.claimed_mask;
    response.sum_claimed_mask = state.sum_claimed_mask;
}

bool InsertStateRow(MySQL* game_database, const Message& message) {
    return game_database &&
           game_database->set_query(
               "inSert ignore into %s(m_id,event_key,event_day,"
               "last_accounted_epoch,"
               "daily_elapsed_seconds,claimed_mask,total_elapsed_seconds,"
               "sum_claimed_mask,revision,created_at) values("
               "%u,%u,curdate(),0,0,0,0,0,0,now())",
               kStateTable,
               static_cast<unsigned int>(message.account_id),
               static_cast<unsigned int>(message.event_key)) &&
           game_database->exec(true);
}

bool SelectStateForUpdate(MySQL* game_database,
                          const Message& message,
                          PersistedState& state) {
    if (!game_database ||
        !game_database->set_query(
            "seLect date_format(event_day,'%%Y%%m%%d')+0,"
            "last_accounted_epoch,daily_elapsed_seconds,claimed_mask,"
            "total_elapsed_seconds,"
            "sum_claimed_mask,revision from %s where m_id=%u and "
            "event_key=%u for update",
            kStateTable,
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(message.event_key)) ||
        !game_database->exec(true) || game_database->get_n_rows() != 1 ||
        !game_database->fetch()) {
        return false;
    }
    unsigned int day_stamp = 0u;
    unsigned int last_accounted_epoch = 0u;
    unsigned int daily_elapsed = 0u;
    unsigned int claimed_mask = 0u;
    unsigned int total_elapsed = 0u;
    unsigned int sum_claimed_mask = 0u;
    unsigned int revision = 0u;
    if (!game_database->get_uint(0, day_stamp) ||
        !game_database->get_uint(1, last_accounted_epoch) ||
        !game_database->get_uint(2, daily_elapsed) ||
        !game_database->get_uint(3, claimed_mask) ||
        !game_database->get_uint(4, total_elapsed) ||
        !game_database->get_uint(5, sum_claimed_mask) ||
        !game_database->get_uint(6, revision) ||
        claimed_mask > 0x0Fu || sum_claimed_mask > 0x07u ||
        day_stamp == 0u) {
        return false;
    }
    state.day_stamp = static_cast<uint32_t>(day_stamp);
    state.last_accounted_epoch =
        static_cast<uint32_t>(last_accounted_epoch);
    state.daily_elapsed = static_cast<uint32_t>(daily_elapsed);
    state.claimed_mask = static_cast<uint8_t>(claimed_mask);
    state.total_elapsed = static_cast<uint32_t>(total_elapsed);
    state.sum_claimed_mask = static_cast<uint8_t>(sum_claimed_mask);
    state.revision = static_cast<uint32_t>(revision);
    return true;
}

bool CurrentDayStamp(MySQL* game_database, uint32_t& day_stamp) {
    if (!game_database ||
        !game_database->set_query(
            "seLect date_format(curdate(),'%%Y%%m%%d')+0") ||
        !game_database->exec(true) || game_database->get_n_rows() != 1 ||
        !game_database->fetch()) {
        return false;
    }
    unsigned int value = 0u;
    if (!game_database->get_uint(0, value) || value == 0u) {
        return false;
    }
    day_stamp = static_cast<uint32_t>(value);
    return true;
}

uint32_t LocalDayStamp(uint32_t epoch) {
    const time_t value = static_cast<time_t>(epoch);
    tm local_time;
    tm* const parsed = localtime_r(&value, &local_time);
    return parsed
               ? static_cast<uint32_t>((parsed->tm_year + 1900) * 10000 +
                                       (parsed->tm_mon + 1) * 100 +
                                       parsed->tm_mday)
               : 0u;
}

uint32_t SecondsSinceLocalMidnight(uint32_t epoch) {
    const time_t value = static_cast<time_t>(epoch);
    tm local_time;
    tm* const parsed = localtime_r(&value, &local_time);
    return parsed ? static_cast<uint32_t>(parsed->tm_hour * 3600 +
                                          parsed->tm_min * 60 +
                                          parsed->tm_sec)
                  : 0u;
}

bool CalculateAuthoritativeDeltas(uint32_t last_accounted_epoch,
                                  uint32_t observed_epoch,
                                  uint32_t& daily_delta,
                                  uint32_t& total_delta) {
    daily_delta = 0u;
    total_delta = 0u;
    if (observed_epoch == 0u || observed_epoch <= last_accounted_epoch) {
        return observed_epoch != 0u;
    }
    total_delta = observed_epoch - last_accounted_epoch;
    if (total_delta > 604800u) {
        // A load resets the high-watermark without accruing offline time, so
        // a live mutation should never span more than one week.
        return false;
    }
    if (last_accounted_epoch == 0u) {
        total_delta = 0u;
        return true;
    }
    const uint32_t previous_day = LocalDayStamp(last_accounted_epoch);
    const uint32_t observed_day = LocalDayStamp(observed_epoch);
    if (previous_day == 0u || observed_day == 0u) {
        return false;
    }
    if (previous_day == observed_day) {
        daily_delta = total_delta;
    } else {
        daily_delta = SecondsSinceLocalMidnight(observed_epoch);
        if (daily_delta > total_delta) {
            daily_delta = total_delta;
        }
    }
    return true;
}

bool UpdateState(MySQL* game_database,
                 const Message& message,
                 const PersistedState& before,
                 const PersistedState& after,
                 bool day_changed) {
    if (before.day_stamp == after.day_stamp &&
        before.last_accounted_epoch == after.last_accounted_epoch &&
        before.daily_elapsed == after.daily_elapsed &&
        before.claimed_mask == after.claimed_mask &&
        before.total_elapsed == after.total_elapsed &&
        before.sum_claimed_mask == after.sum_claimed_mask &&
        before.revision == after.revision) {
        return true;
    }
    if (!game_database) {
        return false;
    }
    if (day_changed) {
        if (!game_database->set_query(
                "upDate %s set event_day=curdate(),"
                "last_accounted_epoch=%u,"
                "daily_elapsed_seconds=%u,claimed_mask=%u,"
                "total_elapsed_seconds=%u,sum_claimed_mask=%u,revision=%u "
                "where m_id=%u and event_key=%u and revision=%u",
                kStateTable,
                static_cast<unsigned int>(after.last_accounted_epoch),
                static_cast<unsigned int>(after.daily_elapsed),
                static_cast<unsigned int>(after.claimed_mask),
                static_cast<unsigned int>(after.total_elapsed),
                static_cast<unsigned int>(after.sum_claimed_mask),
                static_cast<unsigned int>(after.revision),
                static_cast<unsigned int>(message.account_id),
                static_cast<unsigned int>(message.event_key),
                static_cast<unsigned int>(before.revision))) {
            return false;
        }
    } else if (!game_database->set_query(
                   "upDate %s set last_accounted_epoch=%u,"
                   "daily_elapsed_seconds=%u,claimed_mask=%u,"
                   "total_elapsed_seconds=%u,sum_claimed_mask=%u,revision=%u "
                   "where m_id=%u and event_key=%u and revision=%u",
                   kStateTable,
                   static_cast<unsigned int>(after.last_accounted_epoch),
                   static_cast<unsigned int>(after.daily_elapsed),
                   static_cast<unsigned int>(after.claimed_mask),
                   static_cast<unsigned int>(after.total_elapsed),
                   static_cast<unsigned int>(after.sum_claimed_mask),
                   static_cast<unsigned int>(after.revision),
                   static_cast<unsigned int>(message.account_id),
                   static_cast<unsigned int>(message.event_key),
                   static_cast<unsigned int>(before.revision))) {
        return false;
    }
    return game_database->exec(true) &&
           game_database->getAffectedRowCount() == 1u;
}

void Mix32(unsigned long long& hash, uint32_t value) {
    for (unsigned int byte = 0; byte < 4u; ++byte) {
        hash ^= static_cast<unsigned long long>((value >> (byte * 8u)) &
                                                0xFFu);
        hash *= 1099511628211ULL;
    }
}

unsigned long long DeliveryKey(const Message& message,
                               bool cumulative,
                               uint32_t discriminator) {
    unsigned long long hash = 1469598103934665603ULL;
    Mix32(hash, message.account_id);
    Mix32(hash, message.event_key);
    Mix32(hash, discriminator);
    Mix32(hash, cumulative ? 0x53554D31u : 0x44415931u);
    const unsigned long long payload = hash & 0x00FFFFFFFFFFFFFFULL;
    return (cumulative ? kSumDeliveryNamespace :
                         kDailyDeliveryNamespace) | payload;
}

bool FindDelivery(MySQL* mail_database,
                  const Message& message,
                  const MailReward& reward,
                  bool cumulative,
                  uint32_t discriminator,
                  bool& found) {
    found = false;
    const unsigned long long delivery_key =
        DeliveryKey(message, cumulative, discriminator);
    if (!mail_database ||
        !mail_database->set_query(
            "seLect item_id,add_info from postal where auction_id=%llu "
            "limit 2",
            delivery_key) ||
        !mail_database->exec(true)) {
        return false;
    }
    const int rows = mail_database->get_n_rows();
    if (rows == 0) {
        return true;
    }
    unsigned int item_id = 0u;
    unsigned int add_info = 0u;
    if (rows != 1 || !mail_database->fetch() ||
        !mail_database->get_uint(0, item_id) ||
        !mail_database->get_uint(1, add_info) ||
        item_id != reward.item_id || add_info != reward.add_info) {
        return false;
    }
    found = true;
    return true;
}

bool InsertPostal(MySQL* mail_database,
                  const Message& message,
                  const MailReward& reward,
                  bool cumulative,
                  uint32_t discriminator) {
    char escaped_title[sizeof(kMailTitle) * 2 + 1] = {};
    if (!mail_database ||
        mail_database->escape_string(escaped_title, kMailTitle) < 0) {
        return false;
    }
    char* const item_guid = mail_database->blob_to_str(
        0, const_cast<uint8_t*>(reward.item_guid),
        static_cast<int>(sizeof(reward.item_guid)));
    char* const random_option = mail_database->blob_to_str(
        1, const_cast<uint8_t*>(reward.random_option),
        static_cast<int>(sizeof(reward.random_option)));
    const unsigned long long delivery_key =
        DeliveryKey(message, cumulative, discriminator);
    if (!item_guid || !random_option ||
        !mail_database->set_query(
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
            delivery_key,
            random_option,
            static_cast<unsigned int>(reward.chronicle_value),
            static_cast<unsigned int>(reward.separate_upgrade)) ||
        !mail_database->exec(true)) {
        return false;
    }
    return true;
}

bool EnsureDelivery(MySQL* mail_database,
                    const Message& message,
                    const MailReward& reward,
                    bool cumulative,
                    uint32_t discriminator,
                    bool& inserted) {
    inserted = false;
    bool found = false;
    if (!FindDelivery(mail_database,
                      message,
                      reward,
                      cumulative,
                      discriminator,
                      found)) {
        return false;
    }
    if (found) {
        return true;
    }
    if (!InsertPostal(mail_database,
                      message,
                      reward,
                      cumulative,
                      discriminator)) {
        // A duplicate key is not expected because postal is legacy MyISAM
        // and the index is deliberately non-unique.  Re-read regardless so
        // an external concurrent reconciler remains harmless.
        return FindDelivery(mail_database,
                            message,
                            reward,
                            cumulative,
                            discriminator,
                            found) && found;
    }
    inserted = true;
    return true;
}

uint8_t MaturedSumMask(const Message& message, uint32_t total_elapsed) {
    uint8_t mask = 0u;
    for (unsigned int index = 0; index < kSumRewardCount; ++index) {
        if (total_elapsed >= message.sum_seconds[index]) {
            mask = static_cast<uint8_t>(mask | (1u << index));
        }
    }
    return mask;
}

bool EnsureSumRewards(MySQL* mail_database,
                      Message& response,
                      PersistedState& state,
                      uint8_t desired_mask) {
    for (unsigned int index = 0; index < kSumRewardCount; ++index) {
        const uint8_t bit = static_cast<uint8_t>(1u << index);
        if ((desired_mask & bit) == 0u) {
            continue;
        }
        bool inserted = false;
        if (!EnsureDelivery(mail_database,
                            response,
                            response.sum_rewards[index],
                            true,
                            static_cast<uint32_t>(index + 1u),
                            inserted)) {
            return false;
        }
        if (inserted) {
            response.new_mail = 1u;
        }
        state.sum_claimed_mask =
            static_cast<uint8_t>(state.sum_claimed_mask | bit);
    }
    return true;
}

bool LoadState(MySQL* game_database, Message& response) {
    if (!IsValidConfig(response) || response.observed_epoch == 0u ||
        !IsOwnedCharacter(game_database, response) ||
        !game_database->set_query("start transaction") ||
        !game_database->exec(true) ||
        !InsertStateRow(game_database, response)) {
        Rollback(game_database);
        return false;
    }
    PersistedState before;
    uint32_t current_day = 0u;
    if (!SelectStateForUpdate(game_database, response, before) ||
        !CurrentDayStamp(game_database, current_day)) {
        Rollback(game_database);
        return false;
    }
    PersistedState after = before;
    const bool day_changed = before.day_stamp != current_day;
    if (day_changed) {
        if (before.revision == std::numeric_limits<uint32_t>::max()) {
            Rollback(game_database);
            return false;
        }
        after.day_stamp = current_day;
        after.daily_elapsed = 0u;
        after.claimed_mask = 0u;
        after.revision = before.revision + 1u;
    }
    // Login is a new online accounting boundary.  Move the durable
    // high-watermark forward without adding the offline interval.
    if (response.observed_epoch > after.last_accounted_epoch) {
        after.last_accounted_epoch = response.observed_epoch;
    }
    if (!UpdateState(game_database,
                     response,
                     before,
                     after,
                     day_changed) ||
        !Commit(game_database)) {
        Rollback(game_database);
        return false;
    }
    CopyStateToResponse(after, response);
    response.result = static_cast<uint8_t>(kResultOk);
    return true;
}

bool Mutate(MySQL* game_database,
            MySQL* mail_database,
            Message& response) {
    if (!IsValidMutation(response)) {
        response.result = static_cast<uint8_t>(kResultRewardUnavailable);
        return true;
    }
    if (!IsOwnedCharacter(game_database, response)) {
        response.result = static_cast<uint8_t>(kResultInvalidEligibility);
        return true;
    }
    if (!game_database->set_query("start transaction") ||
        !game_database->exec(true) ||
        !InsertStateRow(game_database, response)) {
        Rollback(game_database);
        return false;
    }

    PersistedState before;
    uint32_t current_day = 0u;
    if (!SelectStateForUpdate(game_database, response, before) ||
        !CurrentDayStamp(game_database, current_day)) {
        Rollback(game_database);
        return false;
    }
    PersistedState after = before;
    const bool day_changed = before.day_stamp != current_day;
    if (day_changed) {
        after.day_stamp = current_day;
        after.daily_elapsed = 0u;
        after.claimed_mask = 0u;
    }

    // The row-locked DB high-watermark, not the possibly stale game snapshot,
    // is the accounting authority.  This makes queued periodic/final
    // checkpoints commutative: whichever arrives second adds only the
    // remaining interval, and a late older observation becomes a replay.
    uint32_t authoritative_daily_delta = 0u;
    uint32_t authoritative_total_delta = 0u;
    if (!CalculateAuthoritativeDeltas(after.last_accounted_epoch,
                                      response.observed_epoch,
                                      authoritative_daily_delta,
                                      authoritative_total_delta)) {
        Rollback(game_database);
        return false;
    }
    const uint32_t observed_day = LocalDayStamp(response.observed_epoch);
    if (observed_day == 0u) {
        Rollback(game_database);
        return false;
    }
    const uint32_t daily_delta = observed_day == current_day
                                     ? authoritative_daily_delta
                                     : 0u;
    response.daily_delta_seconds = daily_delta;
    response.total_delta_seconds = authoritative_total_delta;
    if (response.observed_epoch > after.last_accounted_epoch) {
        after.last_accounted_epoch = response.observed_epoch;
    }
    const bool claim_already =
        response.operation == kOperationClaim &&
        (after.claimed_mask & static_cast<uint8_t>(
             1u << (response.requested_tier - 1u))) != 0u;

    // A request observed before midnight can arrive after CURDATE changes.
    // Its total seconds remain valid, but none may leak into the new day's
    // four-tier counter.
    if (after.daily_elapsed >
            std::numeric_limits<uint32_t>::max() -
                daily_delta ||
        after.total_elapsed >
            std::numeric_limits<uint32_t>::max() -
                authoritative_total_delta) {
        Rollback(game_database);
        return false;
    }
    after.daily_elapsed += daily_delta;
    after.total_elapsed += authoritative_total_delta;

    if (response.operation == kOperationClaim) {
        const unsigned int reward_index = response.requested_tier - 1u;
        const uint8_t bit = static_cast<uint8_t>(1u << reward_index);
        if (after.daily_elapsed < response.process_seconds[reward_index]) {
            response.result =
                static_cast<uint8_t>(kResultInvalidEligibility);
            CopyStateToResponse(before, response);
            Rollback(game_database);
            return true;
        }
        bool inserted = false;
        if (!EnsureDelivery(mail_database,
                            response,
                            response.daily_rewards[reward_index],
                            false,
                            after.day_stamp * 10u +
                                response.requested_tier,
                            inserted)) {
            Rollback(game_database);
            return false;
        }
        if (inserted) {
            response.new_mail = 1u;
        }
        if (!claim_already) {
            after.claimed_mask =
                static_cast<uint8_t>(after.claimed_mask | bit);
        }
        response.replayed = claim_already ? 1u : 0u;
    } else {
        response.replayed = authoritative_total_delta == 0u ? 1u : 0u;
    }

    const uint8_t matured_sum = MaturedSumMask(response,
                                                after.total_elapsed);
    if (!EnsureSumRewards(mail_database, response, after, matured_sum)) {
        if (response.operation != kOperationCheckpoint) {
            Rollback(game_database);
            return false;
        }
        // Time/high-watermark durability must not depend on DB3 availability.
        // A checkpoint leaves any unconfirmed sum bit clear; the next
        // checkpoint/claim deterministically retries the same postal key.
    }

    const bool changed = day_changed ||
        before.daily_elapsed != after.daily_elapsed ||
        before.claimed_mask != after.claimed_mask ||
        before.total_elapsed != after.total_elapsed ||
        before.sum_claimed_mask != after.sum_claimed_mask;
    if (changed) {
        if (before.revision == std::numeric_limits<uint32_t>::max()) {
            Rollback(game_database);
            return false;
        }
        after.revision = before.revision + 1u;
    }
    if (!UpdateState(game_database,
                     response,
                     before,
                     after,
                     day_changed) ||
        !Commit(game_database)) {
        Rollback(game_database);
        return false;
    }
    CopyStateToResponse(after, response);
    response.result = static_cast<uint8_t>(kResultOk);
    return true;
}

bool SendResult(int user_idx, const Message& response) {
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
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const payload = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!payload) {
        return false;
    }
    *payload = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

bool FinishRequest(int outer_user_idx,
                   const Message& request,
                   const Message& response,
                   bool handled) {
    const int reply_user_idx = request.operation == kOperationClaim
                                   ? outer_user_idx
                                   : request.reply_user_idx;
    // A load/checkpoint transaction never depends on a pooled CUser still
    // existing.  Loads and live periodic checkpoints carry a separate
    // best-effort reply UID; final disconnect/event-end checkpoints do not.
    return reply_user_idx >= 0 ? SendResult(reply_user_idx, response)
                               : handled;
}

}  // namespace

void ClearMessage(Message& message) {
    std::memset(&message, 0, sizeof(message));
    message.server_group = -1;
    message.reply_user_idx = -1;
    message.result = static_cast<uint8_t>(kResultOperationFailed);
}

bool Queue(int user_idx, const Message& message) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    const bool load = message.operation == kOperationLoad;
    const bool checkpoint = message.operation == kOperationCheckpoint;
    const bool userless = load || checkpoint;
    if ((load && (user_idx != -1 || message.reply_user_idx < 0)) ||
        (checkpoint && (user_idx != -1 || message.reply_user_idx < -1)) ||
        (!userless && (user_idx == -1 || message.reply_user_idx != -1)) ||
        message.account_id == 0u ||
        message.charac_no == 0u || message.event_key == 0u ||
        message.session_token == 0u ||
        !IsValidServerGroup(message.server_group) || !pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const payload = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!payload) {
        return false;
    }
    *payload = message;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

DB_EventAttendance::DB_EventAttendance() : DBDispatcher() {}

DB_EventAttendance::~DB_EventAttendance() {}

bool DB_EventAttendance::dispatch(int signal,
                                      int user_idx,
                                      Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }
    const Message* const request =
        reinterpret_cast<const Message*>(stream->get_buffer());
    if (!request || request->account_id == 0u ||
        request->charac_no == 0u || request->event_key == 0u ||
        request->session_token == 0u ||
        !IsValidServerGroup(request->server_group) ||
        (request->operation == kOperationLoad
             ? (user_idx != -1 || request->reply_user_idx < 0)
             : request->operation == kOperationCheckpoint
                   ? (user_idx != -1 || request->reply_user_idx < -1)
                   : (user_idx == -1 || request->reply_user_idx != -1))) {
        return false;
    }

    Message response = *request;
    response.result = static_cast<uint8_t>(kResultOperationFailed);
    response.replayed = 0u;
    response.new_mail = 0u;
    MySQL* const game_database = GetDatabase(DB_HANDLE_2,
                                             request->server_group);
    bool handled = false;
    if (request->operation == kOperationLoad) {
        handled = game_database && LoadState(game_database, response);
    } else if (request->operation == kOperationCheckpoint ||
               request->operation == kOperationClaim) {
        MySQL* const mail_database = GetDatabase(DB_HANDLE_3,
                                                 request->server_group);
        handled = game_database &&
                  (request->operation == kOperationCheckpoint ||
                   mail_database) &&
                  Mutate(game_database, mail_database, response);
    }
    if (!handled) {
        response.result = static_cast<uint8_t>(kResultOperationFailed);
    }
    return FinishRequest(user_idx, *request, response, handled);
}

}  // namespace event_attendance_db_0828
