#include "old_data/db/event/db_king_of_pick.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"

#include <algorithm>
#include <cstring>
#include <limits>

namespace king_of_pick_db {
namespace {

const char kStateTable[] = "king_of_pick_account_0828";
const char kHistoryTable[] = "king_of_pick_history_0828";
const char kProperTable[] = "king_of_pick_proper_0828";
const int kServerGroupCount = 0x12;
const unsigned int kDeliveryRetrySeconds = 60u;
const uint32_t kExpectedItemIds[kPoolItemCount] = {
    690001492u, 690001493u, 690001494u, 690001495u,
};
const uint32_t kExpectedItemCounts[kPoolItemCount] = {10u, 10u, 5u, 75u};

struct ClockKeys {
    ClockKeys() : day(0u), week(0u) {}
    uint32_t day;
    uint32_t week;
};

struct StateRow {
    StateRow()
        : week(0u), day(0u), remaining(0u), machine_claimed(0u),
          proper_clear_count(0u), revision(0u) {}
    uint32_t week;
    uint32_t day;
    uint32_t remaining;
    uint32_t machine_claimed;
    uint32_t proper_clear_count;
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

bool GetClockKeys(MySQL* database,
                  uint32_t request_day_key,
                  uint32_t request_week_or_epoch,
                  ClockKeys& source) {
    if (!database || request_week_or_epoch == 0u) {
        return false;
    }
    unsigned int current_day = 0u;
    unsigned int current_week = 0u;
    unsigned int source_day = 0u;
    unsigned int source_week = 0u;
    if (request_day_key == 0u) {
        const unsigned int source_epoch =
            static_cast<unsigned int>(request_week_or_epoch);
        if (!database->set_query(
                "seLect to_days(date_sub(now(),interval 6 hour)),"
                "to_days(date_sub(now(),interval 6 hour))-"
                "mod(weekday(date_sub(now(),interval 6 hour))+4,7),"
                "to_days(date_sub(from_unixtime(%u),interval 6 hour)),"
                "to_days(date_sub(from_unixtime(%u),interval 6 hour))-"
                "mod(weekday(date_sub(from_unixtime(%u),interval 6 hour))+4,7)",
                source_epoch,
                source_epoch,
                source_epoch) ||
            !database->exec(true) || database->get_n_rows() != 1 ||
            !database->fetch() || !database->get_uint(0, current_day) ||
            !database->get_uint(1, current_week) ||
            !database->get_uint(2, source_day) ||
            !database->get_uint(3, source_week)) {
            return false;
        }
    } else {
        if (!database->set_query(
                "seLect to_days(date_sub(now(),interval 6 hour)),"
                "to_days(date_sub(now(),interval 6 hour))-"
                "mod(weekday(date_sub(now(),interval 6 hour))+4,7)") ||
            !database->exec(true) || database->get_n_rows() != 1 ||
            !database->fetch() || !database->get_uint(0, current_day) ||
            !database->get_uint(1, current_week)) {
            return false;
        }
        source_day = static_cast<unsigned int>(request_day_key);
        source_week = static_cast<unsigned int>(request_week_or_epoch);
    }
    if (current_day == 0u ||
        current_week == 0u || source_day == 0u || source_week == 0u ||
        current_day < current_week || current_day > current_week + 6u ||
        source_day < source_week || source_day > source_week + 6u ||
        source_day > current_day || current_day - source_day > 1u) {
        return false;
    }
    source.day = source_day;
    source.week = source_week;
    return true;
}

bool IsOwnedCharacter(MySQL* database,
                      const Message& message,
                      unsigned int* persisted_level) {
    if (!database ||
        !database->set_query(
            "seLect lev from charac_info where charac_no=%u and m_id=%u "
            "and delete_flag=0 limit 2",
            static_cast<unsigned int>(message.charac_no),
            static_cast<unsigned int>(message.account_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    unsigned int level = 0u;
    if (!database->get_uint(0, level) || level == 0u || level > 255u) {
        return false;
    }
    if (persisted_level) {
        *persisted_level = level;
    }
    return true;
}

bool IsValidMailReward(const MailReward& reward) {
    return reward.item_id != 0u && reward.add_info == 1u &&
           reward.avatar_flag <= 1u && reward.creature_flag <= 1u;
}

bool ValidatePool(const Message& message) {
    bool found[kPoolItemCount] = {false, false, false, false};
    uint32_t total = 0u;
    for (unsigned int pool_index = 0; pool_index < kPoolItemCount;
         ++pool_index) {
        const PoolReward& pool = message.pool[pool_index];
        if (!IsValidMailReward(pool.reward)) {
            return false;
        }
        bool matched = false;
        for (unsigned int expected = 0; expected < kPoolItemCount;
             ++expected) {
            if (pool.reward.item_id == kExpectedItemIds[expected] &&
                pool.initial_count == kExpectedItemCounts[expected] &&
                !found[expected]) {
                found[expected] = true;
                matched = true;
                break;
            }
        }
        if (!matched ||
            total > std::numeric_limits<uint32_t>::max() -
                        pool.initial_count) {
            return false;
        }
        total += pool.initial_count;
    }
    return total == kMaximumHistoryCount &&
           message.mail_title[kMailTitleBytes - 1] == 0 &&
           message.mail_content[kMailContentBytes - 1] == 0 &&
           message.mail_title[0] != 0 && message.mail_content[0] != 0;
}

const MailReward* FindReward(const Message& message, uint32_t item_id) {
    for (unsigned int index = 0; index < kPoolItemCount; ++index) {
        if (message.pool[index].reward.item_id == item_id) {
            return &message.pool[index].reward;
        }
    }
    return 0;
}

bool ReadStateRow(MySQL* database, StateRow& state) {
    unsigned int week = 0u;
    unsigned int day = 0u;
    unsigned int remaining = 0u;
    unsigned int machine = 0u;
    unsigned int proper = 0u;
    unsigned int revision = 0u;
    if (!database->get_uint(0, week) || !database->get_uint(1, day) ||
        !database->get_uint(2, remaining) ||
        !database->get_uint(3, machine) ||
        !database->get_uint(4, proper) ||
        !database->get_uint(5, revision) || week == 0u || day < week ||
        day > week + 6u || remaining > kMaximumHistoryCount ||
        machine > 1u || proper > 10u) {
        return false;
    }
    state.week = week;
    state.day = day;
    state.remaining = remaining;
    state.machine_claimed = machine;
    state.proper_clear_count = proper;
    state.revision = revision;
    return true;
}

bool LockCurrentState(MySQL* database,
                      const Message& message,
                      const ClockKeys& clock,
                      StateRow& state) {
    if (!database ||
        !database->set_query(
            "inSert ignore into %s(m_id,week_key,day_key,"
            "remaining_pick_chance,daily_machine_claimed,"
            "daily_proper_clear_count,revision,created_at) "
            "values(%u,%u,%u,0,0,0,0,now())",
            kStateTable,
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(clock.week),
            static_cast<unsigned int>(clock.day)) ||
        !database->exec(true) ||
        !database->set_query(
            "seLect week_key,day_key,remaining_pick_chance,"
            "daily_machine_claimed,daily_proper_clear_count,revision "
            "from %s where m_id=%u for update",
            kStateTable,
            static_cast<unsigned int>(message.account_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() || !ReadStateRow(database, state)) {
        return false;
    }

    if (state.week > clock.week || state.day > clock.day) {
        return false;
    }
    if (state.week == clock.week && state.day == clock.day) {
        return true;
    }
    if (state.revision == std::numeric_limits<uint32_t>::max()) {
        return false;
    }

    const bool new_week = state.week != clock.week;
    const unsigned int remaining = new_week ? 0u : state.remaining;
    if (!database->set_query(
            "upDate %s set week_key=%u,day_key=%u,"
            "remaining_pick_chance=%u,daily_machine_claimed=0,"
            "daily_proper_clear_count=0,revision=%u where m_id=%u "
            "and week_key=%u and day_key=%u and revision=%u",
            kStateTable,
            static_cast<unsigned int>(clock.week),
            static_cast<unsigned int>(clock.day),
            remaining,
            static_cast<unsigned int>(state.revision + 1u),
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(state.week),
            static_cast<unsigned int>(state.day),
            static_cast<unsigned int>(state.revision)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u) {
        return false;
    }
    state.week = clock.week;
    state.day = clock.day;
    state.remaining = remaining;
    state.machine_claimed = 0u;
    state.proper_clear_count = 0u;
    ++state.revision;
    return true;
}

bool LoadSnapshot(MySQL* database,
                  const ClockKeys& clock,
                  Message& response) {
    if (!database ||
        !database->set_query(
            "seLect week_key,day_key,remaining_pick_chance,"
            "daily_machine_claimed,daily_proper_clear_count,revision "
            "from %s where m_id=%u",
            kStateTable,
            static_cast<unsigned int>(response.account_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    StateRow state;
    if (!ReadStateRow(database, state) || state.week != clock.week ||
        state.day != clock.day) {
        return false;
    }

    std::memset(response.history, 0, sizeof(response.history));
    response.history_count = 0u;
    if (!database->set_query(
            "seLect selected_position,reward_item_id from %s "
            "where m_id=%u and week_key=%u and delivery_state=2 "
            "order by selected_position",
            kHistoryTable,
            static_cast<unsigned int>(response.account_id),
            static_cast<unsigned int>(clock.week)) ||
        !database->exec(true)) {
        return false;
    }
    const int rows = database->get_n_rows();
    if (rows < 0 || rows > kMaximumHistoryCount) {
        return false;
    }
    for (int row = 0; row < rows; ++row) {
        unsigned int position = 0u;
        unsigned int item_id = 0u;
        if (!database->fetch() || !database->get_uint(0, position) ||
            !database->get_uint(1, item_id) ||
            position >= kMaximumHistoryCount ||
            !FindReward(response, item_id)) {
            return false;
        }
        HistoryEntry& destination = response.history[row];
        destination.selected_position =
            static_cast<uint16_t>(position);
        destination.reward_item_id = item_id;
    }
    response.history_count = static_cast<uint16_t>(rows);
    response.week_key = state.week;
    response.day_key = state.day;
    response.remaining_pick_chance =
        static_cast<uint16_t>(state.remaining);
    response.daily_proper_clear_count =
        static_cast<uint16_t>(state.proper_clear_count);
    response.revision = state.revision;
    return true;
}

bool LockOrSeedProperWatermark(MySQL* database,
                               const Message& message,
                               const ClockKeys& clock,
                               unsigned int initial_serial,
                               bool& seeded,
                               unsigned int& last_serial) {
    seeded = false;
    last_serial = 0u;
    if (!database ||
        !database->set_query(
            "seLect last_clear_serial from %s where m_id=%u and day_key=%u "
            "and charac_no=%u for update",
            kProperTable,
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(clock.day),
            static_cast<unsigned int>(message.charac_no)) ||
        !database->exec(true)) {
        return false;
    }
    const int rows = database->get_n_rows();
    if (rows == 1) {
        return database->fetch() &&
               database->get_uint(0, last_serial);
    }
    if (rows != 0 ||
        !database->set_query(
            "inSert into %s(m_id,day_key,charac_no,last_clear_serial,"
            "created_at) values(%u,%u,%u,%u,now())",
            kProperTable,
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(clock.day),
            static_cast<unsigned int>(message.charac_no),
            initial_serial) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u) {
        return false;
    }
    seeded = true;
    last_serial = initial_serial;
    return true;
}

bool ApplyProperClearDelta(MySQL* database,
                           const Message& message,
                           const ClockKeys& clock,
                           unsigned int last_serial,
                           unsigned int current_serial,
                           bool award_eligible,
                           StateRow& state,
                           unsigned int& awarded) {
    awarded = 0u;
    if (!database || current_serial < last_serial) {
        return false;
    }
    if (current_serial == last_serial) {
        return true;
    }

    const unsigned int serial_delta = current_serial - last_serial;
    const unsigned int available =
        state.proper_clear_count < 10u ? 10u - state.proper_clear_count : 0u;
    // A below-minimum-level load still advances its source watermark so those
    // clears can never be backfilled after a later level-up, but grants no
    // event chance.  Live eligible clears pass true here.
    awarded = award_eligible ? std::min(serial_delta, available) : 0u;
    const unsigned int capacity =
        state.remaining < kMaximumHistoryCount
            ? kMaximumHistoryCount - state.remaining
            : 0u;
    const unsigned int chance_delta = std::min(awarded, capacity);
    if (!database->set_query(
            "upDate %s set last_clear_serial=%u where m_id=%u and "
            "day_key=%u and charac_no=%u and last_clear_serial=%u",
            kProperTable,
            current_serial,
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(clock.day),
            static_cast<unsigned int>(message.charac_no),
            last_serial) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u) {
        return false;
    }
    if (awarded == 0u) {
        return true;
    }
    if (state.revision == std::numeric_limits<uint32_t>::max() ||
        !database->set_query(
            "upDate %s set remaining_pick_chance=%u,"
            "daily_proper_clear_count=%u,revision=%u where m_id=%u "
            "and week_key=%u and day_key=%u and revision=%u",
            kStateTable,
            static_cast<unsigned int>(state.remaining + chance_delta),
            static_cast<unsigned int>(state.proper_clear_count + awarded),
            static_cast<unsigned int>(state.revision + 1u),
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(clock.week),
            static_cast<unsigned int>(clock.day),
            static_cast<unsigned int>(state.revision)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u) {
        return false;
    }
    state.remaining += chance_delta;
    state.proper_clear_count += awarded;
    ++state.revision;
    return true;
}

bool HandleLoad(MySQL* database, Message& response) {
    ClockKeys clock;
    if (!IsOwnedCharacter(database, response, 0) ||
        !GetClockKeys(database, response.day_key, response.week_key, clock) ||
        !database->set_query("start transaction") ||
        !database->exec(true)) {
        return false;
    }
    StateRow state;
    bool seeded = false;
    unsigned int last_serial = 0u;
    unsigned int awarded = 0u;
    const bool has_non_reward_boundary =
        response.has_non_reward_clear_boundary == 1u;
    const unsigned int current_serial =
        static_cast<unsigned int>(response.proper_clear_serial);
    const unsigned int initial_serial =
        has_non_reward_boundary
            ? static_cast<unsigned int>(response.non_reward_clear_serial)
            : current_serial;
    if (!LockCurrentState(database, response, clock, state) ||
        !LockOrSeedProperWatermark(
            database,
            response,
            clock,
            initial_serial,
            seeded,
            last_serial)) {
        Rollback(database);
        return false;
    }

    // A boundary-carrying load separates ineligible/source-baseline clears
    // from the latest serial observed when this retry was queued.  Advance to
    // the boundary without a reward, then account only the eligible suffix in
    // the same row-locked transaction.
    unsigned int reconcile_serial = last_serial;
    if (has_non_reward_boundary && reconcile_serial < initial_serial) {
        if (!ApplyProperClearDelta(database,
                                   response,
                                   clock,
                                   reconcile_serial,
                                   initial_serial,
                                   false,
                                   state,
                                   awarded)) {
            Rollback(database);
            return false;
        }
        reconcile_serial = initial_serial;
    }
    if (current_serial < reconcile_serial ||
        !ApplyProperClearDelta(database,
                               response,
                               clock,
                               reconcile_serial,
                               current_serial,
                               response.character_level > 19u,
                               state,
                               awarded) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    if (!LoadSnapshot(database, clock, response)) {
        return false;
    }
    response.result = static_cast<uint8_t>(kResultOk);
    return true;
}

bool HandleDailyRefill(MySQL* database, Message& response) {
    unsigned int persisted_level = 0u;
    ClockKeys clock;
    if (!ValidatePool(response)) {
        response.result = static_cast<uint8_t>(kResultRewardUnavailable);
        return true;
    }
    if (response.character_level <= 19u ||
        !IsOwnedCharacter(database, response, &persisted_level)) {
        response.result = static_cast<uint8_t>(kResultInvalidEligibility);
        return true;
    }
    if (!GetClockKeys(database,
                      response.day_key,
                      response.week_key,
                      clock) ||
        !database->set_query("start transaction") ||
        !database->exec(true)) {
        return false;
    }
    StateRow state;
    if (!LockCurrentState(database, response, clock, state)) {
        Rollback(database);
        return false;
    }
    if (state.machine_claimed == 1u) {
        response.replayed = 1u;
        response.result = static_cast<uint8_t>(kResultOk);
        if (!Commit(database) || !LoadSnapshot(database, clock, response)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (state.remaining >= kMaximumHistoryCount ||
        state.revision == std::numeric_limits<uint32_t>::max()) {
        response.result = static_cast<uint8_t>(kResultDailyLimit);
        Rollback(database);
        return true;
    }
    if (!database->set_query(
            "upDate %s set remaining_pick_chance=%u,"
            "daily_machine_claimed=1,revision=%u where m_id=%u "
            "and week_key=%u and day_key=%u and revision=%u "
            "and daily_machine_claimed=0",
            kStateTable,
            static_cast<unsigned int>(state.remaining + 1u),
            static_cast<unsigned int>(state.revision + 1u),
            static_cast<unsigned int>(response.account_id),
            static_cast<unsigned int>(clock.week),
            static_cast<unsigned int>(clock.day),
            static_cast<unsigned int>(state.revision)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database) || !LoadSnapshot(database, clock, response)) {
        Rollback(database);
        return false;
    }
    response.replayed = 0u;
    response.result = static_cast<uint8_t>(kResultOk);
    return true;
}

bool HandleProperClear(MySQL* database, Message& response) {
    ClockKeys clock;
    if (!ValidatePool(response)) {
        response.result = static_cast<uint8_t>(kResultRewardUnavailable);
        return true;
    }
    if (response.proper_clear_serial == 0u ||
        !IsOwnedCharacter(database, response, 0)) {
        response.result = static_cast<uint8_t>(kResultInvalidEligibility);
        return true;
    }
    if (!GetClockKeys(database,
                      response.day_key,
                      response.week_key,
                      clock) ||
        !database->set_query("start transaction") ||
        !database->exec(true)) {
        return false;
    }
    StateRow state;
    const unsigned int current_serial =
        static_cast<unsigned int>(response.proper_clear_serial);
    const bool has_non_reward_boundary =
        response.has_non_reward_clear_boundary == 1u;
    const unsigned int initial_serial =
        has_non_reward_boundary
            ? static_cast<unsigned int>(response.non_reward_clear_serial)
            : current_serial - 1u;
    bool seeded = false;
    unsigned int last_serial = 0u;
    if (!LockCurrentState(database, response, clock, state) ||
        !LockOrSeedProperWatermark(database,
                                   response,
                                   clock,
                                   initial_serial,
                                   seeded,
                                   last_serial)) {
        Rollback(database);
        return false;
    }
    unsigned int reconcile_serial = last_serial;
    unsigned int awarded = 0u;
    if (has_non_reward_boundary && reconcile_serial < initial_serial) {
        if (!ApplyProperClearDelta(database,
                                   response,
                                   clock,
                                   reconcile_serial,
                                   initial_serial,
                                   false,
                                   state,
                                   awarded)) {
            Rollback(database);
            return false;
        }
        reconcile_serial = initial_serial;
    }
    if (current_serial < reconcile_serial) {
        response.result = static_cast<uint8_t>(kResultInvalidEligibility);
        Rollback(database);
        return true;
    }
    if (!ApplyProperClearDelta(database,
                               response,
                               clock,
                               reconcile_serial,
                               current_serial,
                               response.character_level > 19u,
                               state,
                               awarded)) {
        Rollback(database);
        return false;
    }
    if (!Commit(database) || !LoadSnapshot(database, clock, response)) {
        Rollback(database);
        return false;
    }
    response.replayed = awarded == 0u ? 1u : 0u;
    response.result = static_cast<uint8_t>(kResultOk);
    return true;
}

bool CountPool(MySQL* database,
               const Message& message,
               const ClockKeys& clock,
               uint32_t used[kPoolItemCount],
               uint32_t& total_used) {
    std::memset(used, 0, sizeof(uint32_t) * kPoolItemCount);
    total_used = 0u;
    if (!database ||
        !database->set_query(
            "seLect reward_item_id,count(*) from %s where m_id=%u "
            "and week_key=%u group by reward_item_id",
            kHistoryTable,
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(clock.week)) ||
        !database->exec(true)) {
        return false;
    }
    const int rows = database->get_n_rows();
    if (rows < 0 || rows > kPoolItemCount) {
        return false;
    }
    for (int row = 0; row < rows; ++row) {
        unsigned int item_id = 0u;
        unsigned int count = 0u;
        if (!database->fetch() || !database->get_uint(0, item_id) ||
            !database->get_uint(1, count)) {
            return false;
        }
        bool matched = false;
        for (unsigned int index = 0; index < kPoolItemCount; ++index) {
            if (item_id == message.pool[index].reward.item_id &&
                count <= message.pool[index].initial_count) {
                used[index] = count;
                matched = true;
                break;
            }
        }
        if (!matched || total_used > kMaximumHistoryCount - count) {
            return false;
        }
        total_used += count;
    }
    return total_used <= kMaximumHistoryCount;
}

bool ChooseReward(MySQL* database,
                  const Message& message,
                  const ClockKeys& clock,
                  uint32_t& reward_item_id) {
    uint32_t used[kPoolItemCount];
    uint32_t total_used = 0u;
    if (!CountPool(database, message, clock, used, total_used) ||
        total_used >= kMaximumHistoryCount) {
        return false;
    }
    const uint32_t remaining_total = kMaximumHistoryCount - total_used;
    if (!database->set_query(
            "seLect floor(rand()*%u)",
            static_cast<unsigned int>(remaining_total)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    unsigned int ticket = 0u;
    if (!database->get_uint(0, ticket) || ticket >= remaining_total) {
        return false;
    }
    for (unsigned int index = 0; index < kPoolItemCount; ++index) {
        const uint32_t available =
            message.pool[index].initial_count - used[index];
        if (ticket < available) {
            reward_item_id = message.pool[index].reward.item_id;
            return true;
        }
        ticket -= available;
    }
    return false;
}

bool FindPhysicalMail(MySQL* mail_database,
                      unsigned long long delivery_key,
                      const MailReward& reward,
                      bool& found) {
    found = false;
    if (!mail_database ||
        !mail_database->set_query(
            "seLect receive_charac_no,item_id,add_info from postal where "
            "auction_id=%llu limit 2",
            delivery_key) ||
        !mail_database->exec(true)) {
        return false;
    }
    const int rows = mail_database->get_n_rows();
    if (rows == 0) {
        return true;
    }
    unsigned int receive_charac_no = 0u;
    unsigned int item_id = 0u;
    unsigned int add_info = 0u;
    if (rows != 1 || !mail_database->fetch() ||
        !mail_database->get_uint(0, receive_charac_no) ||
        !mail_database->get_uint(1, item_id) ||
        !mail_database->get_uint(2, add_info) ||
        receive_charac_no == 0u ||
        item_id != reward.item_id || add_info != reward.add_info) {
        return false;
    }
    found = true;
    return true;
}

bool InsertPostal(MySQL* mail_database,
                  const Message& message,
                  unsigned long long delivery_key,
                  const MailReward& reward) {
    char escaped_title[kMailTitleBytes * 2 + 1] = {};
    if (!mail_database ||
        mail_database->escape_string(escaped_title, message.mail_title) < 0) {
        return false;
    }
    char* const item_guid = mail_database->blob_to_str(
        0,
        const_cast<uint8_t*>(reward.item_guid),
        static_cast<int>(sizeof(reward.item_guid)));
    char* const random_option = mail_database->blob_to_str(
        1,
        const_cast<uint8_t*>(reward.random_option),
        static_cast<int>(sizeof(reward.random_option)));
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

bool MarkDeliveryConfirmed(MySQL* game_database,
                           const Message& message,
                           const ClockKeys& clock,
                           uint16_t position,
                           unsigned long long delivery_key) {
    if (!game_database ||
        !game_database->set_query(
            "upDate %s set delivery_state=2,mail_confirmed_at=now() "
            "where delivery_key=%llu and m_id=%u and week_key=%u "
            "and selected_position=%u and delivery_state=1",
            kHistoryTable,
            delivery_key,
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(clock.week),
            static_cast<unsigned int>(position)) ||
        !game_database->exec(true)) {
        return false;
    }
    if (game_database->getAffectedRowCount() == 1u) {
        return true;
    }
    if (!game_database->set_query(
            "seLect delivery_state from %s where delivery_key=%llu and "
            "m_id=%u and week_key=%u and selected_position=%u",
            kHistoryTable,
            delivery_key,
            static_cast<unsigned int>(message.account_id),
            static_cast<unsigned int>(clock.week),
            static_cast<unsigned int>(position)) ||
        !game_database->exec(true) || game_database->get_n_rows() != 1 ||
        !game_database->fetch()) {
        return false;
    }
    unsigned int state = 0u;
    return game_database->get_uint(0, state) && state == 2u;
}

bool CompleteDelivery(MySQL* game_database,
                      MySQL* mail_database,
                      const Message& message,
                      const ClockKeys& clock,
                      uint16_t position,
                      unsigned long long delivery_key,
                      uint32_t reward_item_id,
                      bool may_insert,
                      bool& new_mail) {
    new_mail = false;
    const MailReward* const reward = FindReward(message, reward_item_id);
    if (!reward) {
        return false;
    }
    bool found = false;
    if (!FindPhysicalMail(mail_database,
                          delivery_key,
                          *reward,
                          found)) {
        return false;
    }
    if (!found) {
        if (!may_insert) {
            return false;
        }
        // postal is MyISAM and cannot participate in a transaction with the
        // InnoDB outbox.  Insert the physical item first with no auxiliary
        // letter row; the deterministic auction_id then makes retries
        // discover that exact mail without leaving an orphan on failure.
        if (!InsertPostal(mail_database,
                          message,
                          delivery_key,
                          *reward)) {
            return false;
        }
        new_mail = true;
    }
    return MarkDeliveryConfirmed(game_database,
                                 message,
                                 clock,
                                 position,
                                 delivery_key);
}

bool HandleDraw(MySQL* game_database,
                MySQL* mail_database,
                Message& response) {
    unsigned int persisted_level = 0u;
    ClockKeys clock;
    if (!ValidatePool(response)) {
        response.result = static_cast<uint8_t>(kResultRewardUnavailable);
        return true;
    }
    if (response.requested_position >= kMaximumHistoryCount ||
        response.character_level <= 19u ||
        !IsOwnedCharacter(game_database, response, &persisted_level)) {
        response.result = static_cast<uint8_t>(kResultInvalidEligibility);
        return true;
    }
    if (!GetClockKeys(game_database,
                      response.day_key,
                      response.week_key,
                      clock) ||
        !game_database->set_query("start transaction") ||
        !game_database->exec(true)) {
        return false;
    }
    StateRow state;
    if (!LockCurrentState(game_database, response, clock, state) ||
        !game_database->set_query(
            "seLect delivery_key,reward_item_id,delivery_state,"
            "delivery_lease_at<=date_sub(now(),interval %u second) "
            "from %s where m_id=%u and week_key=%u and "
            "selected_position=%u for update",
            kDeliveryRetrySeconds,
            kHistoryTable,
            static_cast<unsigned int>(response.account_id),
            static_cast<unsigned int>(clock.week),
            static_cast<unsigned int>(response.requested_position)) ||
        !game_database->exec(true)) {
        Rollback(game_database);
        return false;
    }

    unsigned long long delivery_key = 0ULL;
    uint32_t reward_item_id = 0u;
    bool may_insert = false;
    bool already_confirmed = false;
    const int rows = game_database->get_n_rows();
    if (rows == 1) {
        unsigned int item_id = 0u;
        unsigned int delivery_state = 0u;
        unsigned int lease_expired = 0u;
        if (!game_database->fetch() ||
            !game_database->get_ulonglong(0, delivery_key) ||
            !game_database->get_uint(1, item_id) ||
            !game_database->get_uint(2, delivery_state) ||
            !game_database->get_uint(3, lease_expired) ||
            delivery_key == 0ULL || !FindReward(response, item_id) ||
            (delivery_state != 1u && delivery_state != 2u) ||
            lease_expired > 1u) {
            Rollback(game_database);
            return false;
        }
        reward_item_id = item_id;
        response.replayed = 1u;
        if (delivery_state == 2u) {
            already_confirmed = true;
        } else if (lease_expired == 1u) {
            if (!game_database->set_query(
                    "upDate %s set delivery_lease_at=now() where "
                    "delivery_key=%llu and delivery_state=1 and "
                    "delivery_lease_at<=date_sub(now(),interval %u second)",
                    kHistoryTable,
                    delivery_key,
                    kDeliveryRetrySeconds) ||
                !game_database->exec(true) ||
                game_database->getAffectedRowCount() != 1u) {
                Rollback(game_database);
                return false;
            }
            may_insert = true;
        }
    } else if (rows == 0) {
        if (state.remaining == 0u) {
            response.result = static_cast<uint8_t>(kResultNoPickChance);
            Rollback(game_database);
            return true;
        }
        if (state.revision == std::numeric_limits<uint32_t>::max() ||
            !ChooseReward(game_database,
                          response,
                          clock,
                          reward_item_id) ||
            !game_database->set_query(
                "inSert into %s(m_id,week_key,selected_position,"
                "reward_item_id,claim_revision,delivery_state,"
                "delivery_lease_at,created_at) values(%u,%u,%u,%u,%u,"
                "1,now(),now())",
                kHistoryTable,
                static_cast<unsigned int>(response.account_id),
                static_cast<unsigned int>(clock.week),
                static_cast<unsigned int>(response.requested_position),
                static_cast<unsigned int>(reward_item_id),
                static_cast<unsigned int>(state.revision + 1u)) ||
            !game_database->exec(true) ||
            game_database->getAffectedRowCount() != 1u ||
            !game_database->set_query(
                "seLect delivery_key from %s where m_id=%u and week_key=%u "
                "and selected_position=%u",
                kHistoryTable,
                static_cast<unsigned int>(response.account_id),
                static_cast<unsigned int>(clock.week),
                static_cast<unsigned int>(response.requested_position)) ||
            !game_database->exec(true) ||
            game_database->get_n_rows() != 1 ||
            !game_database->fetch() ||
            !game_database->get_ulonglong(0, delivery_key) ||
            delivery_key == 0ULL ||
            !game_database->set_query(
                "upDate %s set remaining_pick_chance=%u,revision=%u "
                "where m_id=%u and week_key=%u and day_key=%u and "
                "remaining_pick_chance=%u and revision=%u",
                kStateTable,
                static_cast<unsigned int>(state.remaining - 1u),
                static_cast<unsigned int>(state.revision + 1u),
                static_cast<unsigned int>(response.account_id),
                static_cast<unsigned int>(clock.week),
                static_cast<unsigned int>(clock.day),
                static_cast<unsigned int>(state.remaining),
                static_cast<unsigned int>(state.revision)) ||
            !game_database->exec(true) ||
            game_database->getAffectedRowCount() != 1u) {
            Rollback(game_database);
            return false;
        }
        response.replayed = 0u;
        may_insert = true;
    } else {
        Rollback(game_database);
        return false;
    }

    if (!Commit(game_database)) {
        Rollback(game_database);
        return false;
    }

    if (!already_confirmed) {
        bool inserted_mail = false;
        if (!CompleteDelivery(game_database,
                              mail_database,
                              response,
                              clock,
                              response.requested_position,
                              delivery_key,
                              reward_item_id,
                              may_insert,
                              inserted_mail)) {
            response.result = static_cast<uint8_t>(
                may_insert ? kResultOperationFailed
                           : kResultDeliveryPending);
            return true;
        }
        response.new_mail = inserted_mail ? 1u : 0u;
    }
    if (!LoadSnapshot(game_database, clock, response)) {
        return false;
    }
    bool position_found = false;
    for (unsigned int index = 0; index < response.history_count; ++index) {
        if (response.history[index].selected_position ==
                response.requested_position &&
            response.history[index].reward_item_id == reward_item_id) {
            position_found = true;
            break;
        }
    }
    if (!position_found) {
        return false;
    }
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

}  // namespace

void ClearMessage(Message& message) {
    std::memset(&message, 0, sizeof(message));
    message.server_group = -1;
    message.requested_position = 0xffffu;
    message.result = static_cast<uint8_t>(kResultOperationFailed);
}

bool Queue(int user_idx, const Message& message) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (user_idx == -1 || message.account_id == 0u ||
        message.charac_no == 0u || message.session_token == 0u ||
        message.request_id == 0u ||
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

DB_KingOfPick::DB_KingOfPick() : DBDispatcher() {}

DB_KingOfPick::~DB_KingOfPick() {}

bool DB_KingOfPick::dispatch(int signal,
                                  int user_idx,
                                  Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }
    const Message* const request =
        reinterpret_cast<const Message*>(stream->get_buffer());
    if (!request || user_idx == -1 || request->account_id == 0u ||
        request->charac_no == 0u || request->session_token == 0u ||
        request->request_id == 0u || request->week_key == 0u ||
        (request->day_key != 0u &&
         (request->day_key < request->week_key ||
          request->day_key > request->week_key + 6u)) ||
        request->has_non_reward_clear_boundary > 1u ||
        request->reserved_50[0] != 0u || request->reserved_50[1] != 0u ||
        (request->operation != kOperationLoad &&
         request->operation != kOperationProperDungeonClear &&
         (request->has_non_reward_clear_boundary != 0u ||
          request->non_reward_clear_serial != 0u)) ||
        ((request->operation == kOperationLoad ||
          request->operation == kOperationProperDungeonClear) &&
         request->has_non_reward_clear_boundary == 0u &&
         request->non_reward_clear_serial != 0u) ||
        !IsValidServerGroup(request->server_group)) {
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
        handled = game_database && HandleLoad(game_database, response);
    } else if (request->operation == kOperationDailyRefill) {
        handled = game_database &&
                  HandleDailyRefill(game_database, response);
    } else if (request->operation == kOperationProperDungeonClear) {
        handled = game_database && HandleProperClear(game_database, response);
    } else if (request->operation == kOperationDraw) {
        MySQL* const mail_database = GetDatabase(DB_HANDLE_3,
                                                 request->server_group);
        handled = game_database && mail_database &&
                  HandleDraw(game_database, mail_database, response);
    }
    if (!handled) {
        response.result = static_cast<uint8_t>(kResultOperationFailed);
    }
    return SendResult(user_idx, response);
}

}  // namespace king_of_pick_db_0828
