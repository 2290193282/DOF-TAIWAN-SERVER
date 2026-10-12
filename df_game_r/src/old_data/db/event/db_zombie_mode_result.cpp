#include "old_data/db/event/db_zombie_mode_result.h"

#include "common/manager/db_common_public_abi.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/db/core/db_legacy_character_snapshot.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"

#include <cstring>

namespace zombie_mode {
namespace {

const char kDayTable[] = "zombie_mode_account_day_0828";
const char kEntryTable[] = "zombie_mode_entry_0828";
const char kClaimTable[] = "zombie_mode_claim_0828";
const char kMailTitle[] = "Zombie Mode Reward";
const unsigned long long kMailNamespace = 0xD826000000000000ULL;
const unsigned long long kMailPayloadMask = 0x0000FFFFFFFFFFFFULL;

const uint16_t kFullRewardCounts[kMaximumSeats] = {
    13u, 10u, 7u, 5u, 2u, 1u, 1u, 1u,
};
const uint16_t kLimitedRewardCounts[kMaximumSeats] = {
    4u, 3u, 2u, 1u, 1u, 1u, 1u, 1u,
};
const uint32_t kRewardExperience[kMaximumSeats] = {
    200u, 150u, 120u, 100u, 50u, 25u, 25u, 25u,
};

struct StoredEntry {
    StoredEntry()
        : activity_day(0u), daily_ordinal(0u), account_id(0u),
          seat(0u), immunity(0u), state(0u) {}
    uint32_t activity_day;
    uint32_t daily_ordinal;
    uint32_t account_id;
    uint32_t seat;
    uint32_t immunity;
    uint32_t state;
};

struct StoredClaim {
    StoredClaim()
        : claim_id(0ULL), activity_day(0u), daily_ordinal(0u),
          account_id(0u), seat(0u), role(0u), outcome(0u), rank(0u),
          limited(0u), immunity(0u), last_survivor(0u),
          telemetry_consistent(0u), damage(0u), attacks(0u),
          serious_attacks(0u), killed_humans(0u), killed_zombies(0u),
          survival_seconds(0u), total_score(0u), reward_item_id(0u),
          reward_item_count(0u), reward_exp(0u), state(0u),
          delivery_key(0ULL), baseline(), target() {}
    unsigned long long claim_id;
    uint32_t activity_day;
    uint32_t daily_ordinal;
    uint32_t account_id;
    uint32_t seat;
    uint32_t role;
    uint32_t outcome;
    uint32_t rank;
    uint32_t limited;
    uint32_t immunity;
    uint32_t last_survivor;
    uint32_t telemetry_consistent;
    uint32_t damage;
    uint32_t attacks;
    uint32_t serious_attacks;
    uint32_t killed_humans;
    uint32_t killed_zombies;
    uint32_t survival_seconds;
    uint32_t total_score;
    uint32_t reward_item_id;
    uint32_t reward_item_count;
    uint32_t reward_exp;
    uint32_t state;
    unsigned long long delivery_key;
    CharacterSnapshot baseline;
    CharacterSnapshot target;
};

bool IsValidServerGroup(int group) {
    return group >= 0 && group < kMaximumServerGroupCount;
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

unsigned long long JoinDeliveryKey(const Message& message) {
    return (static_cast<unsigned long long>(message.delivery_key_hi) << 32u) |
           static_cast<unsigned long long>(message.delivery_key_lo);
}

bool IsZeroSnapshot(const CharacterSnapshot& value) {
    CharacterSnapshot zero;
    std::memset(&zero, 0, sizeof(zero));
    return EqualSnapshot(value, zero);
}

bool IsPlausibleTarget(const CharacterSnapshot& baseline,
                       const CharacterSnapshot& target,
                       uint32_t reward_exp) {
    const unsigned long long maximum_experience =
        static_cast<unsigned long long>(baseline.experience) + reward_exp;
    return reward_exp > 0u && reward_exp <= kRewardExperience[0] &&
           baseline.level > 0u && baseline.level <= kMaximumCharacterLevel &&
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

uint16_t RewardCount(uint8_t rank, bool limited) {
    return limited ? kLimitedRewardCounts[rank]
                   : kFullRewardCounts[rank];
}

MySQL* GetPartitionDatabase(int handle_index, int server_group) {
    DBMgr* const manager = GlobalData::s_db_mgr;
    if (!manager || !IsValidServerGroup(server_group)) return 0;
    const ENUM_DB_HANDLE_IDX handle =
        static_cast<ENUM_DB_HANDLE_IDX>(handle_index);
    const ENUM_SERVER_GROUP group =
        static_cast<ENUM_SERVER_GROUP>(server_group);
    return manager->isActiveGlobalDB()
               ? manager->GetDBHandleServerGroup(handle, group)
               : manager->GetDBHandle(handle, group);
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

bool LoadActivityDay(MySQL* database, uint32_t& activity_day) {
    return database &&
           database->set_query(
               "seLect cast(date_format(date_sub(now(),interval 6 hour),"
               "'%%Y%%m%%d') as unsigned)") &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, activity_day) &&
           activity_day != 0u;
}

const char* GetSnapshotSkillTable() {
    const char* const table = get_skill_table_name(0);
    if (!table ||
        (std::strcmp(table, "skill") != 0 &&
         std::strcmp(table, "skill_fair_pvp") != 0)) {
        return 0;
    }
    return table;
}

int LoadCharacterSnapshot(MySQL* game_database,
                          MySQL* skill_database,
                          uint32_t account_id,
                          uint32_t charac_no,
                          CharacterSnapshot& snapshot) {
    // The caller retains the existing handle-2 transaction.  Handle 3 is a
    // read-only partition lookup; publish no mixed snapshot unless both
    // partitions return exactly one complete row.
    CharacterSnapshot loaded;
    std::memset(&loaded, 0, sizeof(loaded));
    if (!game_database || !skill_database ||
        !game_database->set_query(
            "seLect ci.lev,cs.exp from charac_info ci "
            "inner join charac_stat cs on cs.charac_no=ci.charac_no "
            "where ci.charac_no=%u and ci.m_id=%u and ci.delete_flag=0 "
            "limit 2",
            static_cast<unsigned int>(charac_no),
            static_cast<unsigned int>(account_id)) ||
        !game_database->exec(true)) {
        return -1;
    }
    if (game_database->get_n_rows() == 0) return 0;
    if (game_database->get_n_rows() != 1 || !game_database->fetch() ||
        !game_database->get_uint(0, loaded.level) ||
        !game_database->get_uint(1, loaded.experience) ||
        loaded.level == 0u || loaded.level > kMaximumCharacterLevel) {
        return -1;
    }

    const char* const skill_table = GetSnapshotSkillTable();
    if (!skill_table ||
        !skill_database->set_query(
            "seLect remain_sp,remain_sp_2nd,remain_sfp_1st,"
            "remain_sfp_2nd from %s where charac_no=%u limit 2",
            skill_table,
            static_cast<unsigned int>(charac_no)) ||
        !skill_database->exec(true)) {
        return -1;
    }
    if (skill_database->get_n_rows() == 0) return 0;
    if (skill_database->get_n_rows() != 1 || !skill_database->fetch() ||
        !skill_database->get_uint(0, loaded.remain_sp_first) ||
        !skill_database->get_uint(1, loaded.remain_sp_second) ||
        !skill_database->get_uint(2, loaded.remain_sfp_first) ||
        !skill_database->get_uint(3, loaded.remain_sfp_second)) {
        return -1;
    }
    snapshot = loaded;
    return 1;
}

int LoadEntry(MySQL* database,
              const Message& request,
              bool for_update,
              StoredEntry& entry) {
    if (!database ||
        !database->set_query(
            "seLect activity_day,daily_ordinal,m_id,seat,"
            "immunity_at_entry,entry_state from %s where server_group=%u "
            "and round_token_hi=%u and round_token_lo=%u and charac_no=%u%s",
            kEntryTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.round_token_hi),
            static_cast<unsigned int>(request.round_token_lo),
            static_cast<unsigned int>(request.charac_no),
            for_update ? " for update" : "") ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) return 0;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, entry.activity_day) ||
        !database->get_uint(1, entry.daily_ordinal) ||
        !database->get_uint(2, entry.account_id) ||
        !database->get_uint(3, entry.seat) ||
        !database->get_uint(4, entry.immunity) ||
        !database->get_uint(5, entry.state) ||
        entry.activity_day == 0u || entry.daily_ordinal == 0u ||
        entry.seat >= kMaximumSeats || entry.immunity > 1u ||
        entry.state > 1u) {
        return -1;
    }
    return 1;
}

bool EntryMatches(const StoredEntry& entry, const Message& request) {
    return entry.account_id == request.account_id &&
           entry.seat == request.seat &&
           entry.immunity == request.immunity_at_entry &&
           (request.daily_ordinal == 0u ||
            entry.daily_ordinal == request.daily_ordinal);
}

bool ReserveEntry(MySQL* database,
                  MySQL* skill_database,
                  const Message& request,
                  Message& response) {
    if (!Begin(database)) return false;
    uint32_t activity_day = 0u;
    CharacterSnapshot character;
    if (!LoadActivityDay(database, activity_day) ||
        LoadCharacterSnapshot(database, skill_database, request.account_id,
                              request.charac_no, character) != 1) {
        Rollback(database);
        response.result = RESULT_IDENTITY;
        return true;
    }

    StoredEntry entry;
    const int existing = LoadEntry(database, request, true, entry);
    if (existing < 0) {
        Rollback(database);
        return false;
    }
    if (existing == 1) {
        if (!EntryMatches(entry, request) ||
            !Commit(database)) {
            Rollback(database);
            response.result = RESULT_POLICY;
            return true;
        }
        response.daily_ordinal =
            static_cast<uint16_t>(entry.daily_ordinal);
        response.replayed = 1u;
        response.result = RESULT_SUCCESS;
        return true;
    }

    if (!database->set_query(
            "inSert into %s(activity_day,m_id,entry_count,updated_at) "
            "values(%u,%u,0,now()) on duplicate key update m_id=values(m_id)",
            kDayTable,
            static_cast<unsigned int>(activity_day),
            static_cast<unsigned int>(request.account_id)) ||
        !database->exec(true) ||
        !database->set_query(
            "seLect entry_count from %s where activity_day=%u and m_id=%u "
            "for update",
            kDayTable,
            static_cast<unsigned int>(activity_day),
            static_cast<unsigned int>(request.account_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        Rollback(database);
        return false;
    }
    uint32_t count = 0u;
    if (!database->get_uint(0, count) || count >= 65535u) {
        Rollback(database);
        response.result = RESULT_POLICY;
        return true;
    }
    const uint32_t ordinal = count + 1u;
    if (!database->set_query(
            "upDate %s set entry_count=%u,updated_at=now() where "
            "activity_day=%u and m_id=%u and entry_count=%u",
            kDayTable,
            static_cast<unsigned int>(ordinal),
            static_cast<unsigned int>(activity_day),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(count)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "inSert into %s(server_group,round_token_hi,round_token_lo,"
            "charac_no,m_id,activity_day,daily_ordinal,seat,"
            "immunity_at_entry,entry_state,created_at,updated_at) "
            "values(%u,%u,%u,%u,%u,%u,%u,%u,%u,0,now(),now())",
            kEntryTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.round_token_hi),
            static_cast<unsigned int>(request.round_token_lo),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(activity_day),
            static_cast<unsigned int>(ordinal),
            static_cast<unsigned int>(request.seat),
            static_cast<unsigned int>(request.immunity_at_entry)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.daily_ordinal = static_cast<uint16_t>(ordinal);
    response.result = RESULT_SUCCESS;
    return true;
}

int LoadClaim(MySQL* database,
              const Message& request,
              bool for_update,
              StoredClaim& claim) {
    if (!database ||
        !database->set_query(
            "seLect claim_id,activity_day,daily_ordinal,m_id,seat,role,"
            "outcome,finish_rank,limited_reward,immunity_at_entry,"
            "last_survivor,telemetry_consistent,damage,attacks,"
            "serious_attacks,killed_humans,killed_zombies,survival_seconds,"
            "total_score,reward_item_id,reward_item_count,reward_exp,"
            "claim_state,delivery_key,baseline_level,baseline_exp,"
            "baseline_sp_first,baseline_sp_second,baseline_sfp_first,"
            "baseline_sfp_second,target_level,target_exp,target_sp_first,"
            "target_sp_second,target_sfp_first,target_sfp_second from %s "
            "where server_group=%u and round_token_hi=%u and "
            "round_token_lo=%u and charac_no=%u%s",
            kClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.round_token_hi),
            static_cast<unsigned int>(request.round_token_lo),
            static_cast<unsigned int>(request.charac_no),
            for_update ? " for update" : "") ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) return 0;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_ulonglong(0, claim.claim_id) ||
        !database->get_uint(1, claim.activity_day) ||
        !database->get_uint(2, claim.daily_ordinal) ||
        !database->get_uint(3, claim.account_id) ||
        !database->get_uint(4, claim.seat) ||
        !database->get_uint(5, claim.role) ||
        !database->get_uint(6, claim.outcome) ||
        !database->get_uint(7, claim.rank) ||
        !database->get_uint(8, claim.limited) ||
        !database->get_uint(9, claim.immunity) ||
        !database->get_uint(10, claim.last_survivor) ||
        !database->get_uint(11, claim.telemetry_consistent) ||
        !database->get_uint(12, claim.damage) ||
        !database->get_uint(13, claim.attacks) ||
        !database->get_uint(14, claim.serious_attacks) ||
        !database->get_uint(15, claim.killed_humans) ||
        !database->get_uint(16, claim.killed_zombies) ||
        !database->get_uint(17, claim.survival_seconds) ||
        !database->get_uint(18, claim.total_score) ||
        !database->get_uint(19, claim.reward_item_id) ||
        !database->get_uint(20, claim.reward_item_count) ||
        !database->get_uint(21, claim.reward_exp) ||
        !database->get_uint(22, claim.state) ||
        !database->get_ulonglong(23, claim.delivery_key) ||
        !database->get_uint(24, claim.baseline.level) ||
        !database->get_uint(25, claim.baseline.experience) ||
        !database->get_uint(26, claim.baseline.remain_sp_first) ||
        !database->get_uint(27, claim.baseline.remain_sp_second) ||
        !database->get_uint(28, claim.baseline.remain_sfp_first) ||
        !database->get_uint(29, claim.baseline.remain_sfp_second) ||
        !database->get_uint(30, claim.target.level) ||
        !database->get_uint(31, claim.target.experience) ||
        !database->get_uint(32, claim.target.remain_sp_first) ||
        !database->get_uint(33, claim.target.remain_sp_second) ||
        !database->get_uint(34, claim.target.remain_sfp_first) ||
        !database->get_uint(35, claim.target.remain_sfp_second) ||
        claim.claim_id == 0ULL || claim.daily_ordinal == 0u ||
        claim.seat >= kMaximumSeats || claim.role > ROLE_ZOMBIE ||
        claim.outcome > OUTCOME_DRAW || claim.rank >= kMaximumSeats ||
        claim.limited > 1u || claim.immunity > 1u ||
        claim.last_survivor > 1u || claim.telemetry_consistent > 1u ||
        claim.state > CLAIM_STATE_COMMITTED) {
        return -1;
    }
    return 1;
}

bool ClaimMatches(const StoredClaim& claim, const Message& request) {
    return claim.account_id == request.account_id &&
           claim.seat == request.seat && claim.role == request.role &&
           claim.outcome == request.outcome && claim.rank == request.rank &&
           claim.limited == request.limited_reward &&
           claim.immunity == request.immunity_at_entry &&
           claim.last_survivor == request.last_survivor &&
           claim.telemetry_consistent == request.telemetry_consistent &&
           claim.damage == request.damage &&
           claim.attacks == request.attacks &&
           claim.serious_attacks == request.serious_attacks &&
           claim.killed_humans == request.killed_humans &&
           claim.killed_zombies == request.killed_zombies &&
           claim.survival_seconds == request.survival_seconds &&
           claim.total_score == request.total_score &&
           claim.reward_item_id == request.reward_item_id &&
           claim.reward_item_count == request.reward_item_count &&
           claim.reward_exp == request.reward_exp &&
           (request.daily_ordinal == 0u ||
            claim.daily_ordinal == request.daily_ordinal);
}

void FillClaim(const StoredClaim& claim, Message& response) {
    response.daily_ordinal = static_cast<uint16_t>(claim.daily_ordinal);
    response.seat = static_cast<uint16_t>(claim.seat);
    response.role = static_cast<uint8_t>(claim.role);
    response.outcome = static_cast<uint8_t>(claim.outcome);
    response.rank = static_cast<uint8_t>(claim.rank);
    response.limited_reward = static_cast<uint8_t>(claim.limited);
    response.immunity_at_entry = static_cast<uint8_t>(claim.immunity);
    response.last_survivor = static_cast<uint8_t>(claim.last_survivor);
    response.telemetry_consistent =
        static_cast<uint8_t>(claim.telemetry_consistent);
    response.damage = claim.damage;
    response.attacks = claim.attacks;
    response.serious_attacks = claim.serious_attacks;
    response.killed_humans = claim.killed_humans;
    response.killed_zombies = claim.killed_zombies;
    response.survival_seconds = claim.survival_seconds;
    response.total_score = claim.total_score;
    response.reward_item_id = claim.reward_item_id;
    response.reward_item_count =
        static_cast<uint16_t>(claim.reward_item_count);
    response.reward_exp = claim.reward_exp;
    response.claim_state = static_cast<uint8_t>(claim.state);
    response.delivery_key_hi =
        static_cast<uint32_t>(claim.delivery_key >> 32u);
    response.delivery_key_lo =
        static_cast<uint32_t>(claim.delivery_key & 0xffffffffULL);
    response.baseline = claim.baseline;
    response.target = claim.target;
}

bool ReserveClaim(MySQL* database,
                  MySQL* skill_database,
                  const Message& request,
                  Message& response) {
    if (!Begin(database)) return false;
    StoredEntry entry;
    const int entry_result = LoadEntry(database, request, true, entry);
    if (entry_result != 1 || !EntryMatches(entry, request)) {
        Rollback(database);
        response.result = entry_result < 0 ? RESULT_DATABASE : RESULT_POLICY;
        return entry_result >= 0;
    }
    const bool limited =
        entry.daily_ordinal > kDailyFullRewardLimit && entry.immunity == 0u;
    if (request.rank >= kMaximumSeats ||
        request.limited_reward != static_cast<uint8_t>(limited) ||
        request.reward_item_id != kRewardItemId ||
        request.reward_item_count != RewardCount(request.rank, limited) ||
        request.reward_exp != kRewardExperience[request.rank]) {
        Rollback(database);
        response.result = RESULT_POLICY;
        return true;
    }

    StoredClaim claim;
    int existing = LoadClaim(database, request, true, claim);
    if (existing < 0) {
        Rollback(database);
        return false;
    }
    if (existing == 0) {
        CharacterSnapshot baseline;
        if (LoadCharacterSnapshot(database, skill_database,
                                  request.account_id,
                                  request.charac_no, baseline) != 1 ||
            !database->set_query(
                "inSert into %s(server_group,round_token_hi,round_token_lo,"
                "charac_no,m_id,activity_day,daily_ordinal,seat,role,outcome,"
                "finish_rank,limited_reward,immunity_at_entry,last_survivor,"
                "telemetry_consistent,damage,attacks,serious_attacks,"
                "killed_humans,killed_zombies,survival_seconds,total_score,"
                "reward_item_id,reward_item_count,reward_exp,claim_state,"
                "delivery_key,baseline_level,baseline_exp,baseline_sp_first,"
                "baseline_sp_second,baseline_sfp_first,baseline_sfp_second,"
                "target_level,target_exp,target_sp_first,target_sp_second,"
                "target_sfp_first,target_sfp_second,created_at,updated_at) "
                "values(%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,"
                "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,0,0,%u,%u,%u,%u,%u,%u,"
                "0,0,0,0,0,0,now(),now())",
                kClaimTable,
                static_cast<unsigned int>(request.server_group),
                static_cast<unsigned int>(request.round_token_hi),
                static_cast<unsigned int>(request.round_token_lo),
                static_cast<unsigned int>(request.charac_no),
                static_cast<unsigned int>(request.account_id),
                static_cast<unsigned int>(entry.activity_day),
                static_cast<unsigned int>(entry.daily_ordinal),
                static_cast<unsigned int>(request.seat),
                static_cast<unsigned int>(request.role),
                static_cast<unsigned int>(request.outcome),
                static_cast<unsigned int>(request.rank),
                static_cast<unsigned int>(request.limited_reward),
                static_cast<unsigned int>(request.immunity_at_entry),
                static_cast<unsigned int>(request.last_survivor),
                static_cast<unsigned int>(request.telemetry_consistent),
                static_cast<unsigned int>(request.damage),
                static_cast<unsigned int>(request.attacks),
                static_cast<unsigned int>(request.serious_attacks),
                static_cast<unsigned int>(request.killed_humans),
                static_cast<unsigned int>(request.killed_zombies),
                static_cast<unsigned int>(request.survival_seconds),
                static_cast<unsigned int>(request.total_score),
                static_cast<unsigned int>(request.reward_item_id),
                static_cast<unsigned int>(request.reward_item_count),
                static_cast<unsigned int>(request.reward_exp),
                static_cast<unsigned int>(baseline.level),
                static_cast<unsigned int>(baseline.experience),
                static_cast<unsigned int>(baseline.remain_sp_first),
                static_cast<unsigned int>(baseline.remain_sp_second),
                static_cast<unsigned int>(baseline.remain_sfp_first),
                static_cast<unsigned int>(baseline.remain_sfp_second)) ||
            !database->exec(true) || database->getAffectedRowCount() != 1u) {
            Rollback(database);
            return false;
        }
        existing = LoadClaim(database, request, true, claim);
        if (existing != 1 || claim.claim_id > kMailPayloadMask) {
            Rollback(database);
            return false;
        }
        claim.delivery_key = kMailNamespace | claim.claim_id;
        if (!database->set_query(
                "upDate %s set delivery_key=%llu,updated_at=now() where "
                "claim_id=%llu and delivery_key=0",
                kClaimTable, claim.delivery_key, claim.claim_id) ||
            !database->exec(true) || database->getAffectedRowCount() != 1u) {
            Rollback(database);
            return false;
        }
    } else {
        response.replayed = 1u;
    }
    if (!ClaimMatches(claim, request) || claim.delivery_key == 0ULL ||
        (entry.state == 0u &&
         (!database->set_query(
              "upDate %s set entry_state=1,settled_at=now(),updated_at=now() "
              "where server_group=%u and round_token_hi=%u and "
              "round_token_lo=%u and charac_no=%u and entry_state=0",
              kEntryTable,
              static_cast<unsigned int>(request.server_group),
              static_cast<unsigned int>(request.round_token_hi),
              static_cast<unsigned int>(request.round_token_lo),
              static_cast<unsigned int>(request.charac_no)) ||
          !database->exec(true) || database->getAffectedRowCount() != 1u)) ||
        !Commit(database)) {
        Rollback(database);
        response.result = RESULT_POLICY;
        return true;
    }
    FillClaim(claim, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool FindMail(MySQL* database,
              unsigned long long delivery_key,
              uint32_t charac_no,
              const StoredClaim& claim,
              bool& found) {
    found = false;
    if (!database ||
        !database->set_query(
            "seLect receive_charac_no,item_id,add_info from postal where "
            "auction_id=%llu limit 2", delivery_key) ||
        !database->exec(true)) return false;
    if (database->get_n_rows() == 0) return true;
    uint32_t receiver = 0u;
    uint32_t item_id = 0u;
    uint32_t count = 0u;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, receiver) ||
        !database->get_uint(1, item_id) ||
        !database->get_uint(2, count) ||
        receiver != charac_no || item_id != claim.reward_item_id ||
        count != claim.reward_item_count) return false;
    found = true;
    return true;
}

bool InsertMail(MySQL* database,
                unsigned long long delivery_key,
                uint32_t charac_no,
                const StoredClaim& claim) {
    uint8_t item_guid[10] = {};
    uint8_t random_option[14] = {};
    char escaped_title[sizeof(kMailTitle) * 2u + 1u] = {};
    if (!database || database->escape_string(escaped_title, kMailTitle) < 0)
        return false;
    char* const guid = database->blob_to_str(
        0, item_guid, static_cast<int>(sizeof(item_guid)));
    char* const random = database->blob_to_str(
        1, random_option, static_cast<int>(sizeof(random_option)));
    return guid && random &&
           database->set_query(
               "inSert into postal(occ_time,send_charac_no,"
               "receive_charac_no,seal_flag,item_id,add_info,endurance,"
               "upgrade,amplify_option,amplify_value,gold,send_charac_name,"
               "creature_flag,avata_flag,letter_id,extend_info,item_guid,"
               "auction_id,random_option,chronicle_value,seperate_upgrade,"
               "unlimit_flag,type) values(now(),0,%u,0,%u,%u,0,0,0,0,0,"
               "'%s',0,0,0,0,'%s',%llu,'%s',0,0,0,0)",
               static_cast<unsigned int>(charac_no),
               static_cast<unsigned int>(claim.reward_item_id),
               static_cast<unsigned int>(claim.reward_item_count),
               escaped_title, guid, delivery_key, random) &&
           database->exec(true);
}

bool EnsureMail(MySQL* database,
                uint32_t charac_no,
                const StoredClaim& claim) {
    bool found = false;
    if (!FindMail(database, claim.delivery_key, charac_no, claim, found))
        return false;
    if (found) return true;
    if (InsertMail(database, claim.delivery_key, charac_no, claim)) return true;
    return FindMail(database, claim.delivery_key, charac_no, claim, found) &&
           found;
}

bool StoreTargetPhase(MySQL* claim_database,
                      const Message& request,
                      Message& response) {
    if (!Begin(claim_database)) return false;
    StoredClaim claim;
    const int loaded = LoadClaim(claim_database, request, true, claim);
    if (loaded != 1 || !ClaimMatches(claim, request) ||
        !EqualSnapshot(claim.baseline, request.baseline)) {
        Rollback(claim_database);
        response.result = RESULT_STALE_CHARACTER;
        return loaded >= 0;
    }
    if (claim.state == CLAIM_STATE_COMMITTED) {
        if (!Commit(claim_database)) {
            Rollback(claim_database);
            return false;
        }
        FillClaim(claim, response);
        response.replayed = 1u;
        response.result = RESULT_SUCCESS;
        return true;
    }
    if (request.claim_state != CLAIM_STATE_RESERVED) {
        Rollback(claim_database);
        response.result = RESULT_STALE_CHARACTER;
        return true;
    }
    if (claim.state == CLAIM_STATE_RESERVED) {
        if (!IsPlausibleTarget(claim.baseline, request.target,
                               claim.reward_exp) ||
            !claim_database->set_query(
                "upDate %s set claim_state=1,target_level=%u,"
                "target_exp=%u,target_sp_first=%u,target_sp_second=%u,"
                "target_sfp_first=%u,target_sfp_second=%u,updated_at=now() "
                "where claim_id=%llu and claim_state=0",
                kClaimTable,
                static_cast<unsigned int>(request.target.level),
                static_cast<unsigned int>(request.target.experience),
                static_cast<unsigned int>(request.target.remain_sp_first),
                static_cast<unsigned int>(request.target.remain_sp_second),
                static_cast<unsigned int>(request.target.remain_sfp_first),
                static_cast<unsigned int>(request.target.remain_sfp_second),
                claim.claim_id) ||
            !claim_database->exec(true) ||
            claim_database->getAffectedRowCount() != 1u) {
            Rollback(claim_database);
            response.result = RESULT_STALE_CHARACTER;
            return true;
        }
        claim.target = request.target;
        claim.state = CLAIM_STATE_TARGET_STORED;
    } else if (!EqualSnapshot(claim.target, request.target)) {
        Rollback(claim_database);
        response.result = RESULT_STALE_CHARACTER;
        return true;
    } else {
        response.replayed = 1u;
    }
    if (!Commit(claim_database)) {
        Rollback(claim_database);
        return false;
    }
    FillClaim(claim, response);
    response.result = RESULT_SUCCESS;
    return true;
}

enum QueueCanonicalApplyResult {
    QUEUE_CANONICAL_DATABASE_FAILURE = -1,
    QUEUE_CANONICAL_RESPONSE_READY = 0,
    QUEUE_CANONICAL_ASYNC_PENDING = 1,
};

int QueueCanonicalApplyPhase(MySQL* claim_database,
                             int user_idx,
                             const Message& request,
                             Message& response) {
    if (!Begin(claim_database)) return QUEUE_CANONICAL_DATABASE_FAILURE;
    StoredClaim claim;
    const int loaded = LoadClaim(claim_database, request, true, claim);
    if (loaded != 1 || !ClaimMatches(claim, request) ||
        !EqualSnapshot(claim.baseline, request.baseline) ||
        !EqualSnapshot(claim.target, request.target) ||
        claim.delivery_key != JoinDeliveryKey(request)) {
        Rollback(claim_database);
        response.result = RESULT_STALE_CHARACTER;
        return loaded >= 0 ? QUEUE_CANONICAL_RESPONSE_READY
                           : QUEUE_CANONICAL_DATABASE_FAILURE;
    }
    if (claim.state == CLAIM_STATE_COMMITTED) {
        if (!Commit(claim_database)) {
            Rollback(claim_database);
            return QUEUE_CANONICAL_DATABASE_FAILURE;
        }
        FillClaim(claim, response);
        response.replayed = 1u;
        response.result = RESULT_SUCCESS;
        return QUEUE_CANONICAL_RESPONSE_READY;
    }
    if (claim.state != CLAIM_STATE_TARGET_STORED) {
        Rollback(claim_database);
        response.result = RESULT_STALE_CHARACTER;
        return QUEUE_CANONICAL_RESPONSE_READY;
    }
    if (!Commit(claim_database)) {
        Rollback(claim_database);
        return QUEUE_CANONICAL_DATABASE_FAILURE;
    }
    return legacy_character_snapshot::QueueZombieModeApply(
               user_idx, request)
               ? QUEUE_CANONICAL_ASYNC_PENDING
               : QUEUE_CANONICAL_DATABASE_FAILURE;
}

bool FinalizeClaim(MySQL* claim_database,
                   MySQL* mail_database,
                   const Message& request,
                   Message& response) {
    if (!Begin(claim_database)) return false;
    StoredClaim claim;
    const int loaded = LoadClaim(claim_database, request, true, claim);
    if (loaded != 1 || !ClaimMatches(claim, request) ||
        !EqualSnapshot(claim.baseline, request.baseline) ||
        !EqualSnapshot(claim.target, request.target) ||
        claim.delivery_key != JoinDeliveryKey(request)) {
        Rollback(claim_database);
        response.operation = OPERATION_COMMIT;
        response.result = RESULT_STALE_CHARACTER;
        return loaded >= 0;
    }
    response.operation = OPERATION_COMMIT;
    if (claim.state == CLAIM_STATE_COMMITTED) {
        if (!Commit(claim_database)) {
            Rollback(claim_database);
            return false;
        }
        FillClaim(claim, response);
        response.replayed = 1u;
        response.result = RESULT_SUCCESS;
        return true;
    }
    if (claim.state != CLAIM_STATE_TARGET_STORED) {
        Rollback(claim_database);
        response.result = RESULT_STALE_CHARACTER;
        return true;
    }
    if (request.result != RESULT_SUCCESS) {
        if (!Commit(claim_database)) {
            Rollback(claim_database);
            return false;
        }
        FillClaim(claim, response);
        response.result = RESULT_DATABASE;
        return true;
    }
    if (!mail_database ||
        !EnsureMail(mail_database, request.charac_no, claim) ||
        !claim_database->set_query(
            "upDate %s set claim_state=2,committed_at=now(),updated_at=now() "
            "where claim_id=%llu and claim_state=1 and delivery_key=%llu",
            kClaimTable, claim.claim_id, claim.delivery_key) ||
        !claim_database->exec(true) ||
        claim_database->getAffectedRowCount() != 1u ||
        !Commit(claim_database)) {
        // The canonical snapshot was already acknowledged by signal 828.
        // Postal remains idempotent through its deterministic auction_id, so
        // a retry may safely repair a crash between postal and sidecar COMMIT.
        Rollback(claim_database);
        return false;
    }
    claim.state = CLAIM_STATE_COMMITTED;
    FillClaim(claim, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool RecoverClaim(MySQL* database,
                  const Message& request,
                  Message& response) {
    if (!database ||
        !database->set_query(
            "seLect server_group,round_token_hi,round_token_lo from %s "
            "where m_id=%u and charac_no=%u and claim_state<2 "
            "order by claim_id limit 2",
            kClaimTable,
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true)) return false;
    if (database->get_n_rows() == 0) {
        response.result = RESULT_NOT_FOUND;
        return true;
    }
    uint32_t group = 0u;
    if (database->get_n_rows() > 1 || !database->fetch() ||
        !database->get_uint(0, group) ||
        !database->get_uint(1, response.round_token_hi) ||
        !database->get_uint(2, response.round_token_lo) ||
        group != static_cast<uint32_t>(request.server_group)) {
        response.result = RESULT_POLICY;
        return true;
    }
    StoredClaim claim;
    if (LoadClaim(database, response, false, claim) != 1 ||
        claim.account_id != request.account_id) return false;
    FillClaim(claim, response);
    response.replayed = 1u;
    response.result = RESULT_SUCCESS;
    return true;
}

bool IsValidRequest(const Message& request,
                    int user_idx,
                    bool allow_finalize) {
    if (user_idx < 0 || request.reply_user_idx != user_idx ||
        request.account_id == 0u || request.charac_no == 0u ||
        request.request_token == 0u ||
        !IsValidServerGroup(request.server_group) ||
        request.operation < OPERATION_BEGIN_ENTRY ||
        request.operation >
            (allow_finalize ? OPERATION_FINALIZE : OPERATION_RECOVER) ||
        (request.operation == OPERATION_FINALIZE
             ? (request.result != RESULT_SUCCESS &&
                request.result != RESULT_DATABASE)
             : request.result != RESULT_SUCCESS) ||
        request.replayed != 0u ||
        request.reserved[0] != 0u || request.reserved[1] != 0u ||
        request.reserved[2] != 0u) return false;

    const bool have_round = request.round_token_hi != 0u &&
                            request.round_token_lo != 0u;
    if (request.operation == OPERATION_RECOVER) {
        return !have_round && request.seat == 0u &&
               request.daily_ordinal == 0u &&
               request.reward_item_count == 0u &&
               request.damage == 0u && request.attacks == 0u &&
               request.serious_attacks == 0u &&
               request.killed_humans == 0u &&
               request.killed_zombies == 0u &&
               request.survival_seconds == 0u &&
               request.total_score == 0u &&
               request.reward_item_id == 0u && request.reward_exp == 0u &&
               request.delivery_key_hi == 0u &&
               request.delivery_key_lo == 0u &&
               IsZeroSnapshot(request.baseline) &&
               IsZeroSnapshot(request.target);
    }
    if (!have_round || request.seat >= kMaximumSeats ||
        request.role > ROLE_ZOMBIE || request.immunity_at_entry > 1u)
        return false;
    if (request.operation == OPERATION_BEGIN_ENTRY) {
        return request.daily_ordinal == 0u &&
               request.reward_item_count == 0u && request.damage == 0u &&
               request.attacks == 0u && request.serious_attacks == 0u &&
               request.killed_humans == 0u &&
               request.killed_zombies == 0u &&
               request.survival_seconds == 0u &&
               request.total_score == 0u && request.reward_item_id == 0u &&
               request.reward_exp == 0u &&
               request.delivery_key_hi == 0u &&
               request.delivery_key_lo == 0u &&
               IsZeroSnapshot(request.baseline) &&
               IsZeroSnapshot(request.target);
    }
    if (request.operation == OPERATION_SETTLE) {
        if (request.outcome > OUTCOME_DRAW ||
            request.rank >= kMaximumSeats ||
            request.limited_reward > 1u || request.last_survivor > 1u ||
            request.telemetry_consistent > 1u ||
            request.reward_item_id != kRewardItemId ||
            request.reward_item_count == 0u || request.reward_exp == 0u ||
            request.delivery_key_hi != 0u ||
            request.delivery_key_lo != 0u ||
            !IsZeroSnapshot(request.baseline) ||
            !IsZeroSnapshot(request.target)) return false;
        const bool limited = request.limited_reward != 0u;
        return request.reward_item_count == RewardCount(request.rank, limited) &&
               request.reward_exp == kRewardExperience[request.rank];
    }
    return (request.operation == OPERATION_COMMIT ||
            request.operation == OPERATION_FINALIZE) &&
           request.outcome <= OUTCOME_DRAW &&
           request.rank < kMaximumSeats &&
           request.limited_reward <= 1u && request.last_survivor <= 1u &&
           request.telemetry_consistent <= 1u &&
           request.reward_item_id == kRewardItemId &&
           request.reward_item_count != 0u && request.reward_exp != 0u &&
           (request.operation == OPERATION_FINALIZE
                ? request.claim_state == CLAIM_STATE_TARGET_STORED
                : (request.claim_state == CLAIM_STATE_RESERVED ||
                   request.claim_state == CLAIM_STATE_TARGET_STORED)) &&
           !IsZeroSnapshot(request.baseline) &&
           !IsZeroSnapshot(request.target);
}

bool QueueResponse(int user_idx, const Message& response) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    Message* const output = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!output) return false;
    *output = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

}  // namespace

DBZombieModeResult::DBZombieModeResult()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBZombieModeResult::~DBZombieModeResult() {}

bool DBZombieModeResult::QueueRequest(int user_idx,
                                          const Message& request) {
    if (!IsValidRequest(request, user_idx, false)) return false;
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    Message* const output = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!output) return false;
    *output = request;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

int DBZombieModeResult::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    if (!stream || stream->length() != static_cast<int>(sizeof(Message)))
        return 0;
    Message request;
    if (stream->get_binary(&request, static_cast<int>(sizeof(request))) != 1 ||
        !IsValidRequest(request, user_idx, true)) return 0;

    Message response = request;
    response.result = request.operation == OPERATION_FINALIZE
                          ? request.result
                          : RESULT_DATABASE;
    if (request.operation == OPERATION_FINALIZE) {
        // Every game-thread response remains the existing COMMIT ABI.  The
        // FINALIZE opcode is private to the DB_Q -> queue-6 continuation.
        response.operation = OPERATION_COMMIT;
    }
    response.replayed = 0u;
    MySQL* const game_database =
        GetPartitionDatabase(2, request.server_group);
    MySQL* const skill_and_mail_database =
        GetPartitionDatabase(3, request.server_group);
    bool handled = false;
    if (game_database) {
        if (request.operation == OPERATION_BEGIN_ENTRY) {
            handled = ReserveEntry(game_database, skill_and_mail_database,
                                   request, response);
        } else if (request.operation == OPERATION_SETTLE) {
            handled = ReserveClaim(game_database, skill_and_mail_database,
                                   request, response);
        } else if (request.operation == OPERATION_COMMIT) {
            if (request.claim_state == CLAIM_STATE_RESERVED) {
                handled = StoreTargetPhase(game_database,
                                           request,
                                           response);
            } else {
                const int queued = QueueCanonicalApplyPhase(
                    game_database, user_idx, request, response);
                if (queued == QUEUE_CANONICAL_ASYNC_PENDING) {
                    return 1;
                }
                handled = queued == QUEUE_CANONICAL_RESPONSE_READY;
            }
        } else if (request.operation == OPERATION_RECOVER) {
            handled = RecoverClaim(game_database, request, response);
        } else if (request.operation == OPERATION_FINALIZE) {
            handled = FinalizeClaim(game_database,
                                    skill_and_mail_database,
                                    request,
                                    response);
        }
    }
    if (!handled) response.result = RESULT_DATABASE;
    (void)QueueResponse(user_idx, response);
    return 1;
}

}  // namespace zombie_mode_0828
