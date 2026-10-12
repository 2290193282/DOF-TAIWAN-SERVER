#include "old_data/db/event/db_exp_keeping_event.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_legacy_character_snapshot.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"

#include <cstring>
#include <limits>

namespace exp_keeping {
namespace {

const char kCampaignTable[] = "exp_keeping_campaign_0828";
const char kDayTable[] = "exp_keeping_account_day_0828";
const char kSourceTable[] = "exp_keeping_source_0828";
const char kOutboxTable[] = "exp_keeping_claim_outbox_0828";

struct CampaignClock {
    CampaignClock()
        : reset_hour(0u),
          daily_cap(0u),
          earn_open(0u),
          claim_open(0u),
          activity_day(0u),
          previous_day(0u) {}

    uint32_t reset_hour;
    uint32_t daily_cap;
    uint32_t earn_open;
    uint32_t claim_open;
    uint32_t activity_day;
    uint32_t previous_day;
};

struct DayState {
    DayState() : kept_experience(0u), claim_state(0u) {}
    uint32_t kept_experience;
    uint32_t claim_state;
};

struct OutboxState {
    OutboxState()
        : token(0u),
          account_id(0u),
          source_day(0u),
          target_charac_no(0u),
          amount(0u),
          state(0u),
          baseline(),
          target() {}

    unsigned long long token;
    uint32_t account_id;
    uint32_t source_day;
    uint32_t target_charac_no;
    uint32_t amount;
    uint32_t state;
    CharacterSnapshot baseline;
    CharacterSnapshot target;
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

bool IsDisabledDungeon(uint32_t dungeon_index) {
    return dungeon_index == 3506u ||
           (dungeon_index >= 10000u && dungeon_index <= 10003u);
}

unsigned long long JoinToken(const SplitToken& token) {
    return (static_cast<unsigned long long>(token.high) << 32u) |
           static_cast<unsigned long long>(token.low);
}

SplitToken Split(unsigned long long token) {
    SplitToken result;
    result.low = static_cast<uint32_t>(token & 0xffffffffULL);
    result.high = static_cast<uint32_t>(token >> 32u);
    return result;
}

bool EqualSnapshot(const CharacterSnapshot& lhs,
                   const CharacterSnapshot& rhs) {
    return lhs.level == rhs.level &&
           lhs.experience == rhs.experience &&
           lhs.remain_sp_first == rhs.remain_sp_first &&
           lhs.remain_sp_second == rhs.remain_sp_second &&
           lhs.remain_sfp_first == rhs.remain_sfp_first &&
           lhs.remain_sfp_second == rhs.remain_sfp_second;
}

bool IsZeroSnapshot(const CharacterSnapshot& value) {
    CharacterSnapshot zero;
    std::memset(&zero, 0, sizeof(zero));
    return EqualSnapshot(value, zero);
}

bool IsPlausibleTarget(const CharacterSnapshot& baseline,
                       const CharacterSnapshot& target,
                       uint32_t amount) {
    const unsigned long long maximum_experience =
        static_cast<unsigned long long>(baseline.experience) +
        static_cast<unsigned long long>(amount);
    return amount > 0u && amount <= kDailyExperienceCap &&
           baseline.level > kMinimumRewardLevelExclusive &&
           baseline.level < kMaximumCharacterLevel &&
           target.level >= baseline.level &&
           target.level <= kMaximumCharacterLevel &&
           target.experience >= baseline.experience &&
           static_cast<unsigned long long>(target.experience) <=
               maximum_experience &&
           target.remain_sp_first >= baseline.remain_sp_first &&
           target.remain_sp_second >= baseline.remain_sp_second &&
           target.remain_sfp_first >= baseline.remain_sfp_first &&
           target.remain_sfp_second >= baseline.remain_sfp_second;
}

MySQL* GetGameDatabase(int server_group) {
    DBMgr* const manager = GlobalData::s_db_mgr;
    if (!manager || !IsValidServerGroup(server_group)) {
        return 0;
    }
    const ENUM_SERVER_GROUP group =
        static_cast<ENUM_SERVER_GROUP>(server_group);
    // A reward transaction may never use DBMgr's log-DB fallback when a
    // global partition mapping is absent.
    return manager->isActiveGlobalDB()
               ? manager->GetDBHandleServerGroup(DB_HANDLE_2, group)
               : manager->GetDBHandle(DB_HANDLE_2, group);
}

bool Begin(MySQL* database) {
    return database && database->set_query("start transaction") &&
           database->exec(true);
}

void Rollback(MySQL* database) {
    if (database && database->set_query("rollback")) {
        (void)database->exec(true);
    }
}

bool Commit(MySQL* database) {
    return database && database->set_query("commit") &&
           database->exec(true);
}

bool LoadCampaignClock(MySQL* database, CampaignClock& clock) {
    if (!database ||
        !database->set_query(
            "seLect reset_hour,daily_exp_cap,"
            "if(now()>=earn_start_at and now()<earn_end_at,1,0),"
            "if(now()>=date_add(earn_start_at,interval 1 day) "
            "and now()<claim_end_at,1,0),"
            "cast(date_format(date_sub(now(),interval reset_hour hour),"
            "'%%Y%%m%%d') as unsigned),"
            "cast(date_format(date_sub(date_sub(now(),"
            "interval reset_hour hour),interval 1 day),'%%Y%%m%%d') "
            "as unsigned) from %s where event_id=%u and enabled=1 "
            "and now()<claim_end_at limit 2",
            kCampaignTable,
            static_cast<unsigned int>(kEventId)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() ||
        !database->get_uint(0, clock.reset_hour) ||
        !database->get_uint(1, clock.daily_cap) ||
        !database->get_uint(2, clock.earn_open) ||
        !database->get_uint(3, clock.claim_open) ||
        !database->get_uint(4, clock.activity_day) ||
        !database->get_uint(5, clock.previous_day)) {
        return false;
    }
    return clock.reset_hour == kResetHour &&
           clock.daily_cap == kDailyExperienceCap &&
           clock.earn_open <= 1u && clock.claim_open <= 1u &&
           clock.activity_day != 0u && clock.previous_day != 0u;
}

// -1 is a query/shape failure, 0 is a clean not-found result, and 1 is one
// exact owned-character row.
int LoadCharacterSnapshot(MySQL* database,
                          uint32_t account_id,
                          uint32_t charac_no,
                          CharacterSnapshot& snapshot) {
    if (!database ||
        !database->set_query(
            "seLect ci.lev,cs.exp,s.remain_sp,s.remain_sp_2nd,"
            "s.remain_sfp_1st,s.remain_sfp_2nd "
            "from charac_info ci inner join charac_stat cs "
            "on cs.charac_no=ci.charac_no inner join skill s "
            "on s.charac_no=ci.charac_no where ci.charac_no=%u "
            "and ci.m_id=%u and ci.delete_flag=0 limit 2",
            static_cast<unsigned int>(charac_no),
            static_cast<unsigned int>(account_id)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) {
        return 0;
    }
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, snapshot.level) ||
        !database->get_uint(1, snapshot.experience) ||
        !database->get_uint(2, snapshot.remain_sp_first) ||
        !database->get_uint(3, snapshot.remain_sp_second) ||
        !database->get_uint(4, snapshot.remain_sfp_first) ||
        !database->get_uint(5, snapshot.remain_sfp_second)) {
        return -1;
    }
    return 1;
}

int LoadCharacterJobAndLevel(MySQL* database,
                             uint32_t account_id,
                             uint32_t charac_no,
                             uint32_t& job,
                             uint32_t& level) {
    if (!database ||
        !database->set_query(
            "seLect job,lev from charac_info where charac_no=%u "
            "and m_id=%u and delete_flag=0 limit 2",
            static_cast<unsigned int>(charac_no),
            static_cast<unsigned int>(account_id)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) {
        return 0;
    }
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, job) ||
        !database->get_uint(1, level)) {
        return -1;
    }
    return 1;
}

int LoadDay(MySQL* database,
            uint32_t account_id,
            uint32_t activity_day,
            bool for_update,
            DayState& state) {
    if (!database ||
        !database->set_query(
            "seLect kept_experience,claim_state from %s where m_id=%u "
            "and activity_day=%u%s",
            kDayTable,
            static_cast<unsigned int>(account_id),
            static_cast<unsigned int>(activity_day),
            for_update ? " for update" : "") ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) {
        return 0;
    }
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, state.kept_experience) ||
        !database->get_uint(1, state.claim_state) ||
        state.kept_experience > kDailyExperienceCap ||
        state.claim_state > 2u) {
        return -1;
    }
    return 1;
}

bool FillVisibleState(MySQL* database,
                      const CampaignClock& clock,
                      Message& response) {
    DayState previous;
    const int previous_result = LoadDay(database,
                                        response.account_id,
                                        clock.previous_day,
                                        false,
                                        previous);
    if (previous_result < 0) {
        return false;
    }
    if (previous_result == 1 && previous.kept_experience > 0u &&
        previous.claim_state < 2u) {
        response.activity_day = clock.previous_day;
        response.kept_experience = previous.kept_experience;
        response.reward_available = clock.claim_open ? 1u : 0u;
        response.claim_state = static_cast<uint8_t>(
            previous.claim_state == 0u ? CLAIM_STATE_NONE
                                       : CLAIM_STATE_RESERVED);
        return true;
    }

    DayState current;
    const int current_result = LoadDay(database,
                                       response.account_id,
                                       clock.activity_day,
                                       false,
                                       current);
    if (current_result < 0) {
        return false;
    }
    response.activity_day = clock.activity_day;
    response.kept_experience = current_result == 1
                                   ? current.kept_experience
                                   : 0u;
    response.reward_available = 0u;
    response.claim_state = CLAIM_STATE_NONE;
    return true;
}

bool HandleLoad(MySQL* database, Message& response) {
    CampaignClock clock;
    if (!LoadCampaignClock(database, clock)) {
        response.result = RESULT_EVENT_CLOSED;
        response.kept_experience = 0u;
        response.reward_available = 0u;
        return true;
    }
    uint32_t job = 0u;
    uint32_t level = 0u;
    const int character = LoadCharacterJobAndLevel(database,
                                                   response.account_id,
                                                   response.charac_no,
                                                   job,
                                                   level);
    (void)job;
    if (character < 0) {
        return false;
    }
    if (character == 0 || level == 0u ||
        level > kMaximumCharacterLevel) {
        response.result = RESULT_NOT_ELIGIBLE;
        response.kept_experience = 0u;
        response.reward_available = 0u;
        return true;
    }
    if (!clock.earn_open && !clock.claim_open) {
        response.result = RESULT_EVENT_CLOSED;
        response.kept_experience = 0u;
        response.reward_available = 0u;
        return true;
    }
    if (!FillVisibleState(database, clock, response)) {
        return false;
    }
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleAccumulate(MySQL* database,
                      const Message& request,
                      Message& response) {
    if (!Begin(database)) {
        return false;
    }
    CampaignClock clock;
    if (!LoadCampaignClock(database, clock)) {
        Rollback(database);
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }
    if (!clock.earn_open) {
        Rollback(database);
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }

    uint32_t job = 0u;
    uint32_t level = 0u;
    const int character = LoadCharacterJobAndLevel(database,
                                                   request.account_id,
                                                   request.charac_no,
                                                   job,
                                                   level);
    if (character < 0) {
        Rollback(database);
        return false;
    }
    if (character == 0 || job != kFemaleFighterJob || level == 0u ||
        level > kMaximumCharacterLevel ||
        request.source_level <= kMinimumKeepingLevelExclusive ||
        request.source_level >= kMaximumCharacterLevel ||
        IsDisabledDungeon(request.source_dungeon)) {
        Rollback(database);
        response.result = RESULT_NOT_ELIGIBLE;
        return true;
    }

    DayState previous;
    const int previous_result = LoadDay(database,
                                        request.account_id,
                                        clock.previous_day,
                                        true,
                                        previous);
    if (previous_result < 0) {
        Rollback(database);
        return false;
    }
    if (previous_result == 1 && previous.kept_experience > 0u &&
        previous.claim_state < 2u) {
        // The official rule blocks today's earning until yesterday's reward
        // has been claimed.  Two-day-old rows never participate here.
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        response.activity_day = clock.previous_day;
        response.kept_experience = previous.kept_experience;
        response.reward_available = clock.claim_open ? 1u : 0u;
        response.result = RESULT_PENDING;
        return true;
    }

    DayState current;
    const int current_result = LoadDay(database,
                                       request.account_id,
                                       clock.activity_day,
                                       true,
                                       current);
    if (current_result < 0) {
        Rollback(database);
        return false;
    }
    if (current_result == 1 && current.claim_state != 0u) {
        // A day row may only accumulate before its reservation transition.
        // Treat an impossible current-day state as corruption; otherwise the
        // source journal could commit while the guarded day UPDATE affects 0.
        Rollback(database);
        return false;
    }

    const unsigned long long source_token = JoinToken(request.source_token);
    if (!database->set_query(
            "seLect kept_experience from %s where m_id=%u "
            "and activity_day=%u and source_token=%llu",
            kSourceTable,
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(clock.activity_day),
            source_token) ||
        !database->exec(true)) {
        Rollback(database);
        return false;
    }
    if (database->get_n_rows() > 1) {
        Rollback(database);
        return false;
    }
    const bool duplicate = database->get_n_rows() == 1;
    uint32_t new_total = current_result == 1
                             ? current.kept_experience
                             : 0u;
    if (!duplicate && new_total < clock.daily_cap) {
        const uint32_t room = clock.daily_cap - new_total;
        const uint32_t accepted = request.source_experience < room
                                      ? request.source_experience
                                      : room;
        if (accepted > 0u &&
            (!database->set_query(
                 "inSert into %s(m_id,activity_day,source_token,charac_no,"
                 "source_level,dungeon_index,kept_experience,created_at) "
                 "values(%u,%u,%llu,%u,%u,%u,%u,now())",
                 kSourceTable,
                 static_cast<unsigned int>(request.account_id),
                 static_cast<unsigned int>(clock.activity_day),
                 source_token,
                 static_cast<unsigned int>(request.charac_no),
                 static_cast<unsigned int>(request.source_level),
                 static_cast<unsigned int>(request.source_dungeon),
                 static_cast<unsigned int>(accepted)) ||
             !database->exec(true) ||
             database->getAffectedRowCount() != 1u)) {
            Rollback(database);
            return false;
        }
        if (accepted > 0u) {
            new_total += accepted;
            if (current_result == 0) {
                if (!database->set_query(
                        "inSert into %s(m_id,activity_day,kept_experience,"
                        "claim_state,created_at,updated_at) "
                        "values(%u,%u,%u,0,now(),now())",
                        kDayTable,
                        static_cast<unsigned int>(request.account_id),
                        static_cast<unsigned int>(clock.activity_day),
                        static_cast<unsigned int>(new_total)) ||
                    !database->exec(true) ||
                    database->getAffectedRowCount() != 1u) {
                    Rollback(database);
                    return false;
                }
            } else if (!database->set_query(
                           "upDate %s set kept_experience=%u,updated_at=now() "
                           "where m_id=%u and activity_day=%u "
                           "and claim_state=0",
                           kDayTable,
                           static_cast<unsigned int>(new_total),
                           static_cast<unsigned int>(request.account_id),
                        static_cast<unsigned int>(clock.activity_day)) ||
                       !database->exec(true) ||
                       database->getAffectedRowCount() != 1u) {
                Rollback(database);
                return false;
            }
        }
    }

    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    response.activity_day = clock.activity_day;
    response.kept_experience = new_total;
    response.reward_available = 0u;
    response.claim_state = CLAIM_STATE_NONE;
    response.replayed = duplicate ? 1u : 0u;
    response.result = RESULT_SUCCESS;
    return true;
}

int LoadOutboxByDay(MySQL* database,
                    uint32_t account_id,
                    uint32_t source_day,
                    bool for_update,
                    OutboxState& state) {
    if (!database ||
        !database->set_query(
            "seLect claim_token,m_id,source_day,target_charac_no,amount,"
            "claim_state,baseline_level,baseline_exp,baseline_sp_first,"
            "baseline_sp_second,baseline_sfp_first,baseline_sfp_second,"
            "target_level,target_exp,target_sp_first,target_sp_second,"
            "target_sfp_first,target_sfp_second from %s where m_id=%u "
            "and source_day=%u%s",
            kOutboxTable,
            static_cast<unsigned int>(account_id),
            static_cast<unsigned int>(source_day),
            for_update ? " for update" : "") ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) {
        return 0;
    }
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_ulonglong(0, state.token) ||
        !database->get_uint(1, state.account_id) ||
        !database->get_uint(2, state.source_day) ||
        !database->get_uint(3, state.target_charac_no) ||
        !database->get_uint(4, state.amount) ||
        !database->get_uint(5, state.state) ||
        !database->get_uint(6, state.baseline.level) ||
        !database->get_uint(7, state.baseline.experience) ||
        !database->get_uint(8, state.baseline.remain_sp_first) ||
        !database->get_uint(9, state.baseline.remain_sp_second) ||
        !database->get_uint(10, state.baseline.remain_sfp_first) ||
        !database->get_uint(11, state.baseline.remain_sfp_second) ||
        !database->get_uint(12, state.target.level) ||
        !database->get_uint(13, state.target.experience) ||
        !database->get_uint(14, state.target.remain_sp_first) ||
        !database->get_uint(15, state.target.remain_sp_second) ||
        !database->get_uint(16, state.target.remain_sfp_first) ||
        !database->get_uint(17, state.target.remain_sfp_second) ||
        state.token == 0u || state.state > 2u) {
        return -1;
    }
    return 1;
}

int LoadOutboxByToken(MySQL* database,
                      unsigned long long token,
                      bool for_update,
                      OutboxState& state) {
    if (!database ||
        !database->set_query(
            "seLect claim_token,m_id,source_day,target_charac_no,amount,"
            "claim_state,baseline_level,baseline_exp,baseline_sp_first,"
            "baseline_sp_second,baseline_sfp_first,baseline_sfp_second,"
            "target_level,target_exp,target_sp_first,target_sp_second,"
            "target_sfp_first,target_sfp_second from %s "
            "where claim_token=%llu%s",
            kOutboxTable,
            token,
            for_update ? " for update" : "") ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) {
        return 0;
    }
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_ulonglong(0, state.token) ||
        !database->get_uint(1, state.account_id) ||
        !database->get_uint(2, state.source_day) ||
        !database->get_uint(3, state.target_charac_no) ||
        !database->get_uint(4, state.amount) ||
        !database->get_uint(5, state.state) ||
        !database->get_uint(6, state.baseline.level) ||
        !database->get_uint(7, state.baseline.experience) ||
        !database->get_uint(8, state.baseline.remain_sp_first) ||
        !database->get_uint(9, state.baseline.remain_sp_second) ||
        !database->get_uint(10, state.baseline.remain_sfp_first) ||
        !database->get_uint(11, state.baseline.remain_sfp_second) ||
        !database->get_uint(12, state.target.level) ||
        !database->get_uint(13, state.target.experience) ||
        !database->get_uint(14, state.target.remain_sp_first) ||
        !database->get_uint(15, state.target.remain_sp_second) ||
        !database->get_uint(16, state.target.remain_sfp_first) ||
        !database->get_uint(17, state.target.remain_sfp_second) ||
        state.token != token || state.state > 2u) {
        return -1;
    }
    return 1;
}

void FillOutboxResponse(const OutboxState& state, Message& response) {
    response.activity_day = state.source_day;
    response.kept_experience = state.amount;
    response.claim_token = Split(state.token);
    response.baseline = state.baseline;
    response.target = state.target;
    response.reward_available = state.state == 2u ? 0u : 1u;
    response.claim_state = static_cast<uint8_t>(
        state.state == 0u ? CLAIM_STATE_RESERVED
                          : state.state == 1u ? CLAIM_STATE_TARGET_STORED
                                              : CLAIM_STATE_COMMITTED);
}

bool HandleReserve(MySQL* database, Message& response) {
    if (!Begin(database)) {
        return false;
    }
    CampaignClock clock;
    if (!LoadCampaignClock(database, clock)) {
        Rollback(database);
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }
    if (!clock.claim_open) {
        Rollback(database);
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }

    CharacterSnapshot baseline;
    const int character = LoadCharacterSnapshot(database,
                                                response.account_id,
                                                response.charac_no,
                                                baseline);
    if (character < 0) {
        Rollback(database);
        return false;
    }
    if (character == 0 ||
        baseline.level <= kMinimumRewardLevelExclusive ||
        baseline.level >= kMaximumCharacterLevel) {
        Rollback(database);
        response.result = RESULT_NOT_ELIGIBLE;
        return true;
    }

    DayState day;
    const int day_result = LoadDay(database,
                                   response.account_id,
                                   clock.previous_day,
                                   true,
                                   day);
    if (day_result < 0) {
        Rollback(database);
        return false;
    }
    if (day_result == 0 || day.kept_experience == 0u) {
        Rollback(database);
        response.result = RESULT_NOTHING_TO_CLAIM;
        return true;
    }

    OutboxState outbox;
    int outbox_result = LoadOutboxByDay(database,
                                        response.account_id,
                                        clock.previous_day,
                                        true,
                                        outbox);
    if (outbox_result < 0) {
        Rollback(database);
        return false;
    }
    if (outbox_result == 0) {
        if (day.claim_state != 0u ||
            !database->set_query(
                "inSert into %s(m_id,source_day,target_charac_no,amount,"
                "claim_state,baseline_level,baseline_exp,"
                "baseline_sp_first,baseline_sp_second,baseline_sfp_first,"
                "baseline_sfp_second,created_at,updated_at) "
                "values(%u,%u,%u,%u,0,%u,%u,%u,%u,%u,%u,now(),now())",
                kOutboxTable,
                static_cast<unsigned int>(response.account_id),
                static_cast<unsigned int>(clock.previous_day),
                static_cast<unsigned int>(response.charac_no),
                static_cast<unsigned int>(day.kept_experience),
                static_cast<unsigned int>(baseline.level),
                static_cast<unsigned int>(baseline.experience),
                static_cast<unsigned int>(baseline.remain_sp_first),
                static_cast<unsigned int>(baseline.remain_sp_second),
                static_cast<unsigned int>(baseline.remain_sfp_first),
                static_cast<unsigned int>(baseline.remain_sfp_second)) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u ||
            !database->set_query(
                "upDate %s set claim_state=1,updated_at=now() "
                "where m_id=%u and activity_day=%u and claim_state=0",
                kDayTable,
                static_cast<unsigned int>(response.account_id),
                static_cast<unsigned int>(clock.previous_day)) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u) {
            Rollback(database);
            return false;
        }
        outbox_result = LoadOutboxByDay(database,
                                        response.account_id,
                                        clock.previous_day,
                                        true,
                                        outbox);
        if (outbox_result != 1) {
            Rollback(database);
            return false;
        }
    } else {
        response.replayed = 1u;
    }

    if (outbox.target_charac_no != response.charac_no ||
        outbox.amount != day.kept_experience) {
        Rollback(database);
        response.result = RESULT_PENDING;
        return true;
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    FillOutboxResponse(outbox, response);
    response.result = RESULT_SUCCESS;
    return true;
}

enum StoreTargetResult {
    STORE_TARGET_DATABASE_FAILURE = -1,
    STORE_TARGET_RESPONSE_READY = 0,
    STORE_TARGET_DURABLE = 1,
};

int StoreTargetPhase(MySQL* database,
                     const Message& request,
                     OutboxState& outbox,
                     Message& response) {
    if (!Begin(database)) {
        return STORE_TARGET_DATABASE_FAILURE;
    }
    const unsigned long long token = JoinToken(request.claim_token);
    const int loaded = LoadOutboxByToken(database, token, true, outbox);
    if (loaded != 1) {
        Rollback(database);
        if (loaded == 0) {
            response.result = RESULT_STALE_CHARACTER;
            return STORE_TARGET_RESPONSE_READY;
        }
        return STORE_TARGET_DATABASE_FAILURE;
    }
    if (outbox.account_id != request.account_id ||
        outbox.target_charac_no != request.charac_no ||
        outbox.amount == 0u || outbox.amount > kDailyExperienceCap ||
        !EqualSnapshot(outbox.baseline, request.baseline)) {
        Rollback(database);
        response.result = RESULT_STALE_CHARACTER;
        return STORE_TARGET_RESPONSE_READY;
    }
    if (outbox.state == 2u) {
        if (!Commit(database)) {
            Rollback(database);
            return STORE_TARGET_DATABASE_FAILURE;
        }
        FillOutboxResponse(outbox, response);
        response.replayed = 1u;
        response.result = RESULT_SUCCESS;
        return STORE_TARGET_RESPONSE_READY;
    }
    if (outbox.state == 0u) {
        if (!IsPlausibleTarget(outbox.baseline,
                               request.target,
                               outbox.amount) ||
            !database->set_query(
                "upDate %s set claim_state=1,target_level=%u,"
                "target_exp=%u,target_sp_first=%u,target_sp_second=%u,"
                "target_sfp_first=%u,target_sfp_second=%u,updated_at=now() "
                "where claim_token=%llu and claim_state=0",
                kOutboxTable,
                static_cast<unsigned int>(request.target.level),
                static_cast<unsigned int>(request.target.experience),
                static_cast<unsigned int>(request.target.remain_sp_first),
                static_cast<unsigned int>(request.target.remain_sp_second),
                static_cast<unsigned int>(request.target.remain_sfp_first),
                static_cast<unsigned int>(request.target.remain_sfp_second),
                token) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u) {
            Rollback(database);
            response.result = RESULT_STALE_CHARACTER;
            return STORE_TARGET_RESPONSE_READY;
        }
        outbox.target = request.target;
        outbox.state = 1u;
    } else if (!EqualSnapshot(outbox.target, request.target) ||
               IsZeroSnapshot(outbox.target)) {
        Rollback(database);
        response.result = RESULT_STALE_CHARACTER;
        return STORE_TARGET_RESPONSE_READY;
    } else {
        response.replayed = 1u;
    }
    if (!Commit(database)) {
        Rollback(database);
        return STORE_TARGET_DATABASE_FAILURE;
    }
    return STORE_TARGET_DURABLE;
}

enum QueueCanonicalApplyResult {
    QUEUE_CANONICAL_DATABASE_FAILURE = -1,
    QUEUE_CANONICAL_RESPONSE_READY = 0,
    QUEUE_CANONICAL_ASYNC_PENDING = 1,
};

int QueueCanonicalApplyPhase(MySQL* database,
                             int user_idx,
                             const Message& request,
                             Message& response) {
    if (!Begin(database)) {
        return QUEUE_CANONICAL_DATABASE_FAILURE;
    }
    OutboxState outbox;
    const unsigned long long token = JoinToken(request.claim_token);
    const int loaded = LoadOutboxByToken(database, token, true, outbox);
    if (loaded != 1 || outbox.account_id != request.account_id ||
        outbox.target_charac_no != request.charac_no) {
        Rollback(database);
        if (loaded == 0) {
            response.result = RESULT_STALE_CHARACTER;
            return QUEUE_CANONICAL_RESPONSE_READY;
        }
        return QUEUE_CANONICAL_DATABASE_FAILURE;
    }
    if (outbox.state == 2u) {
        if (!Commit(database)) {
            Rollback(database);
            return QUEUE_CANONICAL_DATABASE_FAILURE;
        }
        FillOutboxResponse(outbox, response);
        response.replayed = 1u;
        response.result = RESULT_SUCCESS;
        return QUEUE_CANONICAL_RESPONSE_READY;
    }
    if (outbox.state != 1u ||
        !EqualSnapshot(outbox.baseline, request.baseline) ||
        !EqualSnapshot(outbox.target, request.target)) {
        Rollback(database);
        response.result = RESULT_STALE_CHARACTER;
        return QUEUE_CANONICAL_RESPONSE_READY;
    }
    if (!Commit(database)) {
        Rollback(database);
        return QUEUE_CANONICAL_DATABASE_FAILURE;
    }
    // The canonical legacy tables are never written by queue 6.  Signal 828
    // executes behind every already-enqueued legacy save and returns an
    // actual apply-and-verify continuation to this native owner.
    return legacy_character_snapshot::QueueExpKeepingApply(
               user_idx, request)
               ? QUEUE_CANONICAL_ASYNC_PENDING
               : QUEUE_CANONICAL_DATABASE_FAILURE;
}

bool FinalizeTargetPhase(MySQL* database,
                         const Message& request,
                         Message& response) {
    if (!Begin(database)) {
        return false;
    }
    OutboxState outbox;
    const unsigned long long token = JoinToken(request.claim_token);
    const int loaded = LoadOutboxByToken(database, token, true, outbox);
    if (loaded != 1 || outbox.account_id != request.account_id ||
        outbox.target_charac_no != request.charac_no ||
        !EqualSnapshot(outbox.baseline, request.baseline) ||
        !EqualSnapshot(outbox.target, request.target)) {
        Rollback(database);
        response.operation = OPERATION_COMMIT;
        response.result = RESULT_STALE_CHARACTER;
        return loaded >= 0;
    }
    response.operation = OPERATION_COMMIT;
    if (outbox.state == 2u) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        FillOutboxResponse(outbox, response);
        response.replayed = 1u;
        response.result = RESULT_SUCCESS;
        return true;
    }
    if (outbox.state != 1u) {
        Rollback(database);
        response.result = RESULT_STALE_CHARACTER;
        return true;
    }
    if (request.result != RESULT_SUCCESS) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        FillOutboxResponse(outbox, response);
        response.result = RESULT_DATABASE_FAILURE;
        return true;
    }
    if (
        !database->set_query(
            "upDate %s set claim_state=2,claimed_charac_no=%u,"
            "updated_at=now() where m_id=%u and activity_day=%u "
            "and claim_state=1",
            kDayTable,
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(outbox.source_day)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate %s set claim_state=2,committed_at=now(),"
            "updated_at=now() where claim_token=%llu and claim_state=1",
            kOutboxTable,
            token) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        // The canonical MyISAM snapshot has already been verified by the
        // signal-828 legacy worker.  The immutable target remains state 1,
        // so retrying phase B replays the same absolute apply before another
        // finalization attempt.
        Rollback(database);
        return false;
    }
    outbox.state = 2u;
    FillOutboxResponse(outbox, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleCommit(MySQL* database,
                  const Message& request,
                  Message& response) {
    if (request.claim_state == CLAIM_STATE_RESERVED) {
        // Phase A must durably publish the immutable target before the game
        // thread invokes rewardExp.  A lost reply is therefore replayed as
        // TARGET_STORED; it must never fall through and mutate MyISAM in the
        // same request that first records the target.
        OutboxState stored;
        const int stored_result =
            StoreTargetPhase(database, request, stored, response);
        if (stored_result == STORE_TARGET_DATABASE_FAILURE) {
            return false;
        }
        if (stored_result == STORE_TARGET_RESPONSE_READY) {
            return true;
        }
        FillOutboxResponse(stored, response);
        response.result = RESULT_SUCCESS;
        return true;
    }
    response.result = RESULT_STALE_CHARACTER;
    return true;
}

bool IsValidMessage(const Message& request, bool allow_finalize) {
    if (request.account_id == 0u || request.charac_no == 0u ||
        request.session_token == 0u || request.request_token == 0u ||
        request.event_id != kEventId ||
        !IsValidServerGroup(request.server_group) ||
        request.operation < OPERATION_LOAD ||
        request.operation >
            (allow_finalize ? OPERATION_FINALIZE : OPERATION_COMMIT) ||
        request.reserved[0] != 0u || request.reserved[1] != 0u) {
        return false;
    }
    if (request.operation == OPERATION_ACCUMULATE) {
        return request.source_experience > 0u &&
               request.source_experience <=
                   static_cast<uint32_t>(std::numeric_limits<int>::max()) &&
               request.source_dungeon != 0u &&
               request.source_level > kMinimumKeepingLevelExclusive &&
               request.source_level < kMaximumCharacterLevel &&
               JoinToken(request.source_token) != 0u &&
               JoinToken(request.claim_token) == 0u &&
               IsZeroSnapshot(request.baseline) &&
               IsZeroSnapshot(request.target);
    }
    if (request.operation == OPERATION_COMMIT ||
        request.operation == OPERATION_FINALIZE) {
        if (request.operation == OPERATION_FINALIZE &&
            (request.claim_state != CLAIM_STATE_TARGET_STORED ||
             (request.result != RESULT_SUCCESS &&
              request.result != RESULT_DATABASE_FAILURE) ||
             request.replayed != 0u)) {
            return false;
        }
        return JoinToken(request.claim_token) != 0u &&
               JoinToken(request.source_token) == 0u &&
               request.source_experience == 0u &&
               request.source_dungeon == 0u &&
               request.source_level == 0u &&
               (request.claim_state == CLAIM_STATE_RESERVED ||
                request.claim_state == CLAIM_STATE_TARGET_STORED) &&
               !IsZeroSnapshot(request.baseline) &&
               !IsZeroSnapshot(request.target);
    }
    return request.source_experience == 0u &&
           request.source_dungeon == 0u &&
           request.source_level == 0u &&
           JoinToken(request.source_token) == 0u &&
           JoinToken(request.claim_token) == 0u &&
           IsZeroSnapshot(request.baseline) &&
           IsZeroSnapshot(request.target);
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

DBExpKeepingEvent::DBExpKeepingEvent()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBExpKeepingEvent::~DBExpKeepingEvent() {}

bool DBExpKeepingEvent::QueueRequest(int user_idx,
                                         const Message& request) {
    if (user_idx < 0 || !IsValidMessage(request, false)) {
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

int DBExpKeepingEvent::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    if (user_idx < 0 || !stream ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return 0;
    }
    Message request;
    if (stream->get_binary(&request, static_cast<int>(sizeof(request))) != 1 ||
        !IsValidMessage(request, true)) {
        return 0;
    }
    Message response = request;
    response.result = request.operation == OPERATION_FINALIZE
                          ? request.result
                          : RESULT_DATABASE_FAILURE;
    if (request.operation == OPERATION_FINALIZE) {
        // FINALIZE is private to DB_Q -> queue 6.  Game/Inter keeps the
        // established COMMIT response ABI even when the native DB handle is
        // unavailable before FinalizeTargetPhase can run.
        response.operation = OPERATION_COMMIT;
    }
    response.activity_day = 0u;
    response.kept_experience = 0u;
    response.reward_available = 0u;
    // A DB failure after the durable-target boundary must preserve which
    // idempotent COMMIT phase the game should replay.  Non-COMMIT responses
    // never expose a claim phase.
    response.claim_state = (request.operation == OPERATION_COMMIT ||
                            request.operation == OPERATION_FINALIZE)
                               ? request.claim_state
                               : CLAIM_STATE_NONE;
    response.replayed = 0u;

    MySQL* const database = GetGameDatabase(request.server_group);
    bool handled = false;
    if (database) {
        switch (request.operation) {
            case OPERATION_LOAD:
                handled = HandleLoad(database, response);
                break;
            case OPERATION_ACCUMULATE:
                handled = HandleAccumulate(database, request, response);
                break;
            case OPERATION_RESERVE:
                handled = HandleReserve(database, response);
                break;
            case OPERATION_COMMIT:
                if (request.claim_state == CLAIM_STATE_TARGET_STORED) {
                    const int queued = QueueCanonicalApplyPhase(
                        database, user_idx, request, response);
                    if (queued == QUEUE_CANONICAL_ASYNC_PENDING) {
                        return 1;
                    }
                    handled = queued == QUEUE_CANONICAL_RESPONSE_READY;
                } else {
                    handled = HandleCommit(database, request, response);
                }
                break;
            case OPERATION_FINALIZE:
                handled = FinalizeTargetPhase(database, request, response);
                break;
            default:
                break;
        }
    }
    if (!handled) {
        response.result = RESULT_DATABASE_FAILURE;
    }
    return SendResult(user_idx, response) ? 1 : 0;
}

}  // namespace exp_keeping_0828
