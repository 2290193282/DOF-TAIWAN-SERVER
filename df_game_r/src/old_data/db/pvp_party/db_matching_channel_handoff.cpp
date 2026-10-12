#include "old_data/db/pvp_party/db_matching_channel_handoff.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "old_game/pvp/match/zombie_matching_runtime_policy.h"

#include <arpa/inet.h>
#include <cstring>

namespace matching_channel_handoff_db {
namespace {

const char kTable[] = "matching_channel_handoff_0828";
const char kCohortTable[] = "matching_channel_cohort_0828";

enum {
    kRouteFreshnessSeconds = 60,
    kCohortStateRouting = 1,
    kCohortStateReserved = 2,
    kCohortStateArmed = 3,
    kCohortStateClaiming = 4,
    kCohortStateClaimed = 5,
    kCohortStateActive = 6,
    kCohortStateComplete = 7,
    kCohortStateAborted = 8,
    kCohortStateExpired = 9,
};

struct CohortSnapshot {
    CohortSnapshot()
        : cohort_id(0u),
          leader_account_id(0u),
          leader_charac_no(0u),
          authority_token(0u),
          source_server_group(-1),
          source_channel(-1),
          target_gc_no(0u),
          target_server_group(-1),
          target_channel(-1),
          endpoint_key(0u),
          endpoint_port_like(0u),
          expected_count(0u),
          reserved_count(0u),
          armed_count(0u),
          claimed_count(0u),
          state(0u),
          active(false) {}

    uint32_t cohort_id;
    uint32_t leader_account_id;
    uint32_t leader_charac_no;
    uint32_t authority_token;
    int source_server_group;
    int source_channel;
    uint32_t target_gc_no;
    int target_server_group;
    int target_channel;
    uint32_t endpoint_key;
    uint32_t endpoint_port_like;
    unsigned int expected_count;
    unsigned int reserved_count;
    unsigned int armed_count;
    unsigned int claimed_count;
    unsigned int state;
    bool active;
};

bool IsServerGroup(int value) {
    return value >= 0 && value < kMaximumServerGroupCount;
}

bool IsChannel(int value) {
    return value > 0 && value <= 65535;
}

bool IsCohortSize(unsigned int value) {
    return zombie_matching_runtime_policy::IsRuntimeCohortSize(value);
}

bool HasKnownCohortFlags(uint8_t flags) {
    return (flags & ~static_cast<uint8_t>(COHORT_FLAG_ALL)) == 0u;
}

bool IsValidRouteFields(const Message& message) {
    return message.field1 != 0u &&
           IsServerGroup(message.source_server_group) &&
           IsServerGroup(message.target_server_group) &&
           message.source_server_group == message.target_server_group &&
           IsChannel(message.source_channel) &&
           IsChannel(message.target_channel) &&
           message.field0 ==
               static_cast<uint8_t>(message.target_server_group) &&
           message.matching_bound == kChannelMatchingBound &&
           message.matching_mode == kZombieMatchingMode &&
           message.endpoint_key != 0u &&
           message.endpoint_key != 0xffffffffu &&
           message.endpoint_port_like > 0u &&
           message.endpoint_port_like <= 65535u &&
           IsCohortSize(message.cohort_size) &&
           message.cohort_slot < message.cohort_size;
}

bool IsValidRequest(const Message& message) {
    if (!message.account_id || !message.charac_no ||
        !message.session_token || !message.request_token ||
        message.reply_required > 1u ||
        !HasKnownCohortFlags(message.cohort_flags) ||
        message.operation < OPERATION_RESERVE ||
        message.operation > OPERATION_COMPLETE_COHORT) {
        return false;
    }

    if (message.operation == OPERATION_ROUTE_COHORT) {
        return IsServerGroup(message.source_server_group) &&
               IsChannel(message.source_channel) &&
               message.matching_bound == kChannelMatchingBound &&
               message.matching_mode == kZombieMatchingMode &&
               IsCohortSize(message.cohort_size) &&
               message.cohort_slot == 0u &&
               message.cohort_flags == 0u &&
               message.handoff_id == 0u && message.field0 == 0u &&
               message.field1 == 0u && message.endpoint_key == 0u &&
               message.endpoint_port_like == 0u && message.field4 == 0u &&
               message.target_server_group == 0 &&
               message.target_channel == 0 &&
               message.state == STATE_NONE && !message.source_cleaned;
    }

    if (message.operation == OPERATION_ABORT_COHORT) {
        if (message.handoff_id == 0u) {
            return message.field1 != 0u &&
                   IsServerGroup(message.source_server_group) &&
                   IsChannel(message.source_channel) &&
                   message.field4 == 0u && message.state == STATE_NONE &&
                   !message.source_cleaned &&
                   IsCohortSize(message.cohort_size) &&
                   message.cohort_slot == 0u;
        }
        return message.field4 == message.handoff_id &&
               IsValidRouteFields(message) &&
               message.cohort_slot == 0u &&
               message.state == STATE_CLAIMED && message.source_cleaned;
    }

    if (message.operation == OPERATION_ACTIVATE_COHORT ||
        message.operation == OPERATION_COMPLETE_COHORT) {
        return message.handoff_id != 0u &&
               message.field4 == message.handoff_id &&
               IsValidRouteFields(message) &&
               message.cohort_slot == 0u &&
               message.state == STATE_CLAIMED && message.source_cleaned;
    }

    // CMD521 carries only NOTI606.field0/field4.  Its target-side claim is
    // deliberately validated against the stored ticket, so it must not invent
    // source-route or endpoint fields that are absent from the client wire.
    if (message.operation == OPERATION_CLAIM) {
        return message.handoff_id != 0u &&
               message.field4 == message.handoff_id &&
               message.field0 ==
                   static_cast<uint8_t>(message.target_server_group) &&
               IsServerGroup(message.target_server_group) &&
               IsChannel(message.target_channel) &&
               message.state == STATE_NONE &&
               !message.source_cleaned;
    }
    if (!IsValidRouteFields(message)) {
        return false;
    }
    if (message.operation == OPERATION_RESERVE) {
        return message.handoff_id == 0u && message.field4 == 0u &&
               message.state == STATE_NONE && !message.source_cleaned;
    }
    if (!message.handoff_id || message.field4 != message.handoff_id) {
        return false;
    }
    if (message.operation == OPERATION_ARM) {
        return message.state == STATE_RESERVED && message.source_cleaned;
    }
    return message.state == STATE_RESERVED || message.state == STATE_ARMED;
}

MySQL* GetDatabase(const Message& message) {
    const int group = message.operation == OPERATION_CLAIM
                          ? message.target_server_group
                          : message.source_server_group;
    return GlobalData::s_db_mgr && IsServerGroup(group)
               ? GlobalData::s_db_mgr->GetDBHandle(
                     DB_HANDLE_2, static_cast<ENUM_SERVER_GROUP>(group))
               : 0;
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

bool ExpireActive(MySQL* database, uint32_t account_id) {
    return database && database->set_query(
               "upDate %s set state=%u,active_slot=null,updated_at=now() "
               "where m_id=%u and active_slot=1 and expires_at<now()",
               kTable, static_cast<unsigned int>(STATE_EXPIRED),
               static_cast<unsigned int>(account_id)) &&
           database->exec(true);
}

bool ReadLastInsertId(MySQL* database, uint32_t& value) {
    return database && database->set_query("seLect last_insert_id()") &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, value) && value != 0u;
}

uint8_t MakeCohortFlags(const CohortSnapshot& cohort) {
    uint8_t flags = 0u;
    if (cohort.expected_count != 0u &&
        cohort.reserved_count == cohort.expected_count) {
        flags |= COHORT_FLAG_ALL_RESERVED;
    }
    if (cohort.expected_count != 0u &&
        cohort.armed_count == cohort.expected_count) {
        flags |= COHORT_FLAG_ALL_ARMED;
    }
    if (cohort.expected_count != 0u &&
        cohort.claimed_count == cohort.expected_count) {
        flags |= COHORT_FLAG_ALL_CLAIMED;
    }
    return flags;
}

bool LoadCohort(MySQL* database,
                uint32_t cohort_id,
                bool for_update,
                CohortSnapshot& cohort) {
    int active_value = 0;
    if (!database || cohort_id == 0u || !database->set_query(
            "seLect cohort_id,leader_account_id,leader_charac_no,"
            "authority_token,source_server_group,source_channel,target_gc_no,"
            "target_server_group,target_channel,endpoint_key,"
            "endpoint_port_like,expected_count,reserved_count,armed_count,"
            "claimed_count,state,if(active_slot is null or expires_at<now(),"
            "0,active_slot) "
            "from %s where cohort_id=%u%s",
            kCohortTable, static_cast<unsigned int>(cohort_id),
            for_update ? " for update" : "") ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() ||
        !database->get_uint(0, cohort.cohort_id) ||
        !database->get_uint(1, cohort.leader_account_id) ||
        !database->get_uint(2, cohort.leader_charac_no) ||
        !database->get_uint(3, cohort.authority_token) ||
        !database->get_int(4, cohort.source_server_group) ||
        !database->get_int(5, cohort.source_channel) ||
        !database->get_uint(6, cohort.target_gc_no) ||
        !database->get_int(7, cohort.target_server_group) ||
        !database->get_int(8, cohort.target_channel) ||
        !database->get_uint(9, cohort.endpoint_key) ||
        !database->get_uint(10, cohort.endpoint_port_like) ||
        !database->get_uint(11, cohort.expected_count) ||
        !database->get_uint(12, cohort.reserved_count) ||
        !database->get_uint(13, cohort.armed_count) ||
        !database->get_uint(14, cohort.claimed_count) ||
        !database->get_uint(15, cohort.state) ||
        !database->get_int(16, active_value)) {
        return false;
    }
    cohort.active = active_value == 1;
    return cohort.cohort_id == cohort_id && cohort.leader_account_id != 0u &&
           cohort.leader_charac_no != 0u && cohort.authority_token != 0u &&
           IsServerGroup(cohort.source_server_group) &&
           IsChannel(cohort.source_channel) && cohort.target_gc_no != 0u &&
           IsServerGroup(cohort.target_server_group) &&
           cohort.target_server_group == cohort.source_server_group &&
           IsChannel(cohort.target_channel) && cohort.endpoint_key != 0u &&
           cohort.endpoint_key != 0xffffffffu &&
           cohort.endpoint_port_like > 0u &&
           cohort.endpoint_port_like <= 65535u &&
           IsCohortSize(cohort.expected_count) &&
           cohort.reserved_count <= cohort.expected_count &&
           cohort.armed_count <= cohort.reserved_count &&
           cohort.claimed_count <= cohort.armed_count &&
           cohort.state >= kCohortStateRouting &&
           cohort.state <= kCohortStateExpired &&
           active_value >= 0 && active_value <= 1;
}

void ApplyCohort(const CohortSnapshot& cohort, Message& response) {
    response.field0 = static_cast<uint8_t>(cohort.target_server_group);
    response.field1 = cohort.cohort_id;
    response.endpoint_key = cohort.endpoint_key;
    response.endpoint_port_like = cohort.endpoint_port_like;
    response.source_server_group = cohort.source_server_group;
    response.source_channel = cohort.source_channel;
    response.target_server_group = cohort.target_server_group;
    response.target_channel = cohort.target_channel;
    response.matching_bound = kChannelMatchingBound;
    response.matching_mode = kZombieMatchingMode;
    response.cohort_size = static_cast<uint8_t>(cohort.expected_count);
    response.cohort_flags = MakeCohortFlags(cohort);
}

bool SameCohortAuthority(const CohortSnapshot& cohort,
                         const Message& request) {
    return cohort.leader_account_id == request.account_id &&
           cohort.leader_charac_no == request.charac_no &&
           cohort.authority_token == request.session_token &&
           cohort.source_server_group == request.source_server_group &&
           cohort.source_channel == request.source_channel &&
           cohort.expected_count == request.cohort_size;
}

bool SameCohortRoute(const CohortSnapshot& cohort,
                     const Message& request) {
    return cohort.cohort_id == request.field1 &&
           cohort.source_server_group == request.source_server_group &&
           cohort.source_channel == request.source_channel &&
           cohort.target_server_group == request.target_server_group &&
           cohort.target_channel == request.target_channel &&
           cohort.endpoint_key == request.endpoint_key &&
           cohort.endpoint_port_like == request.endpoint_port_like &&
           cohort.expected_count == request.cohort_size &&
           request.field0 ==
               static_cast<uint8_t>(cohort.target_server_group);
}

bool ExpireCohorts(MySQL* database) {
    if (!database || !database->set_query(
            "upDate %s set state=%u,active_slot=null,updated_at=now() "
            "where active_slot=1 and expires_at<now()",
            kCohortTable,
            static_cast<unsigned int>(kCohortStateExpired)) ||
        !database->exec(true)) {
        return false;
    }
    return database->set_query(
               "upDate %s h inner join %s c on c.cohort_id=h.field1 "
               "set h.state=%u,h.active_slot=null,h.updated_at=now() "
               "where c.state=%u and h.active_slot=1 and h.state in (%u,%u)",
               kTable, kCohortTable,
               static_cast<unsigned int>(STATE_EXPIRED),
               static_cast<unsigned int>(kCohortStateExpired),
               static_cast<unsigned int>(STATE_RESERVED),
               static_cast<unsigned int>(STATE_ARMED)) &&
           database->exec(true);
}

bool ReadTicketCohortId(MySQL* database,
                        uint32_t handoff_id,
                        uint32_t& cohort_id) {
    cohort_id = 0u;
    return database && handoff_id != 0u && database->set_query(
               "seLect field1 from %s where handoff_id=%u",
               kTable, static_cast<unsigned int>(handoff_id)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, cohort_id) &&
           cohort_id != 0u;
}

bool LoadTicket(MySQL* database,
                uint32_t handoff_id,
                bool for_update,
                Message& response,
                bool& active) {
    unsigned int state = 0u;
    unsigned int source_cleaned = 0u;
    int active_value = 0;
    if (!database || !database->set_query(
            "seLect m_id,charac_no,field0,field1,endpoint_key,"
            "endpoint_port_like,field4,source_server_group,source_channel,"
            "target_server_group,target_channel,matching_bound,"
            "matching_mode,cohort_slot,cohort_size,state,source_cleaned,"
            "if(active_slot is null,0,active_slot) from %s "
            "where handoff_id=%u%s",
            kTable, static_cast<unsigned int>(handoff_id),
            for_update ? " for update" : "") ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() ||
        !database->get_uint(0, response.account_id) ||
        !database->get_uint(1, response.charac_no) ||
        !database->get_ubyte(2, response.field0) ||
        !database->get_uint(3, response.field1) ||
        !database->get_uint(4, response.endpoint_key) ||
        !database->get_uint(5, response.endpoint_port_like) ||
        !database->get_uint(6, response.field4) ||
        !database->get_int(7, response.source_server_group) ||
        !database->get_int(8, response.source_channel) ||
        !database->get_int(9, response.target_server_group) ||
        !database->get_int(10, response.target_channel) ||
        !database->get_ubyte(11, response.matching_bound) ||
        !database->get_ushort(12, response.matching_mode) ||
        !database->get_ubyte(13, response.cohort_slot) ||
        !database->get_ubyte(14, response.cohort_size) ||
        !database->get_uint(15, state) ||
        !database->get_uint(16, source_cleaned) ||
        !database->get_int(17, active_value) || state > STATE_EXPIRED ||
        source_cleaned > 1u || active_value < 0 || active_value > 1) {
        return false;
    }
    response.handoff_id = handoff_id;
    response.state = static_cast<uint8_t>(state);
    response.source_cleaned = static_cast<uint8_t>(source_cleaned);
    active = active_value == 1;
    return response.field4 == handoff_id &&
           IsServerGroup(response.source_server_group) &&
           IsServerGroup(response.target_server_group) &&
           response.source_server_group == response.target_server_group &&
           IsChannel(response.source_channel) &&
           IsChannel(response.target_channel) &&
           response.matching_bound == kChannelMatchingBound &&
           response.matching_mode == kZombieMatchingMode &&
           IsCohortSize(response.cohort_size) &&
           response.cohort_slot < response.cohort_size &&
           response.endpoint_key != 0u && response.endpoint_port_like > 0u &&
           response.endpoint_port_like <= 65535u;
}

bool SameOwnerAndRoute(const Message& lhs, const Message& rhs) {
    return lhs.account_id == rhs.account_id &&
           lhs.charac_no == rhs.charac_no && lhs.field0 == rhs.field0 &&
           lhs.field1 == rhs.field1 &&
           lhs.endpoint_key == rhs.endpoint_key &&
           lhs.endpoint_port_like == rhs.endpoint_port_like &&
           lhs.source_server_group == rhs.source_server_group &&
           lhs.source_channel == rhs.source_channel &&
           lhs.target_server_group == rhs.target_server_group &&
           lhs.target_channel == rhs.target_channel &&
           lhs.matching_bound == rhs.matching_bound &&
           lhs.matching_mode == rhs.matching_mode &&
           lhs.cohort_slot == rhs.cohort_slot &&
           lhs.cohort_size == rhs.cohort_size;
}

bool LoadActiveForAccount(MySQL* database,
                          uint32_t account_id,
                          Message& response) {
    uint32_t handoff_id = 0u;
    if (!database || !database->set_query(
            "seLect handoff_id from %s where m_id=%u and active_slot=1 "
            "limit 2 for update", kTable,
            static_cast<unsigned int>(account_id)) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) {
        return true;
    }
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, handoff_id) || !handoff_id) {
        return false;
    }
    bool active = false;
    return LoadTicket(database, handoff_id, true, response, active) && active;
}

bool LoadActiveCohortForLeader(MySQL* database,
                               uint32_t account_id,
                               CohortSnapshot& cohort) {
    uint32_t cohort_id = 0u;
    if (!database || !database->set_query(
            "seLect cohort_id from %s where leader_account_id=%u and "
            "active_slot=1 limit 2 for update",
            kCohortTable, static_cast<unsigned int>(account_id)) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) return true;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, cohort_id) || cohort_id == 0u) {
        return false;
    }
    return LoadCohort(database, cohort_id, true, cohort) && cohort.active;
}

bool HandleRouteCohort(MySQL* database,
                       const Message& request,
                       Message& response) {
    if (!Begin(database)) return false;
    if (!ExpireCohorts(database)) {
        Rollback(database);
        return false;
    }

    CohortSnapshot active;
    if (!LoadActiveCohortForLeader(database, request.account_id, active)) {
        Rollback(database);
        return false;
    }
    if (active.cohort_id != 0u) {
        response = request;
        ApplyCohort(active, response);
        response.result =
            (active.state == kCohortStateRouting &&
             SameCohortAuthority(active, request))
                ? RESULT_SUCCESS : RESULT_BUSY;
        return Commit(database);
    }

    uint32_t target_gc_no = 0u;
    char target_ip[16];
    std::memset(target_ip, 0, sizeof(target_ip));
    int target_port = 0;
    int target_group = -1;
    int target_channel = -1;
    int current_users = -1;
    int maximum_users = -1;
    if (!database->set_query(
            "seLect g.gc_no,g.gc_ip,g.gc_port,g.gc_game,g.gc_ch_group,"
            "g.gc_now,g.gc_max from game_channel g where g.gc_game=%d "
            "and g.gc_type=19 and g.gc_ch_group<>%d and g.gc_ip<>'' "
            "and g.gc_port between 1 and 65535 and g.gc_now>=0 "
            "and g.gc_max>=%u and g.gc_now<=g.gc_max-%u "
            "and g.gc_up_time>=date_sub(now(),interval %u second) "
            "and not exists (select 1 from %s c where "
            "c.target_server_group=g.gc_game and "
            "c.target_channel=g.gc_ch_group and c.active_slot=1 and "
            "c.expires_at>=now()) order by g.gc_now asc,g.gc_no asc "
            "limit 1 for update",
            request.source_server_group, request.source_channel,
            static_cast<unsigned int>(request.cohort_size),
            static_cast<unsigned int>(request.cohort_size),
            static_cast<unsigned int>(kRouteFreshnessSeconds),
            kCohortTable) ||
        !database->exec(true)) {
        Rollback(database);
        return false;
    }
    if (database->get_n_rows() == 0) {
        response = request;
        response.result = RESULT_BUSY;
        return Commit(database);
    }
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, target_gc_no) || target_gc_no == 0u ||
        !database->get_str(1, target_ip, sizeof(target_ip)) ||
        !database->get_int(2, target_port) ||
        !database->get_int(3, target_group) ||
        !database->get_int(4, target_channel) ||
        !database->get_int(5, current_users) ||
        !database->get_int(6, maximum_users) ||
        target_group != request.source_server_group ||
        !IsChannel(target_channel) || target_channel == request.source_channel ||
        target_port < 1 || target_port > 65535 || current_users < 0 ||
        maximum_users < static_cast<int>(request.cohort_size) ||
        current_users > maximum_users - static_cast<int>(request.cohort_size)) {
        Rollback(database);
        return false;
    }

    const uint32_t endpoint_key =
        static_cast<uint32_t>(::inet_addr(target_ip));
    if (endpoint_key == 0u || endpoint_key == 0xffffffffu) {
        Rollback(database);
        return false;
    }

    uint32_t cohort_id = 0u;
    if (!database->set_query(
            "inSert into %s(leader_account_id,leader_charac_no,"
            "authority_token,source_server_group,source_channel,target_gc_no,"
            "target_server_group,target_channel,endpoint_key,"
            "endpoint_port_like,expected_count,reserved_count,armed_count,"
            "claimed_count,state,active_slot,created_at,ready_at,"
            "activated_at,expires_at,updated_at) values(%u,%u,%u,%d,%d,"
            "%u,%d,%d,%u,%u,%u,0,0,0,%u,1,now(),null,null,"
            "date_add(now(),interval %u second),now())",
            kCohortTable,
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.session_token),
            request.source_server_group, request.source_channel,
            static_cast<unsigned int>(target_gc_no), target_group,
            target_channel, static_cast<unsigned int>(endpoint_key),
            static_cast<unsigned int>(target_port),
            static_cast<unsigned int>(request.cohort_size),
            static_cast<unsigned int>(kCohortStateRouting),
            static_cast<unsigned int>(kTicketLifetimeSeconds)) ||
        !database->exec(true) || !ReadLastInsertId(database, cohort_id)) {
        Rollback(database);
        return false;
    }

    CohortSnapshot created;
    if (!LoadCohort(database, cohort_id, true, created) ||
        !created.active || !SameCohortAuthority(created, request)) {
        Rollback(database);
        return false;
    }
    response = request;
    ApplyCohort(created, response);
    response.result = RESULT_SUCCESS;
    return Commit(database);
}

bool HandleReserve(MySQL* database,
                   const Message& request,
                   Message& response) {
    if (!Begin(database)) return false;
    CohortSnapshot cohort;
    if (!LoadCohort(database, request.field1, true, cohort)) {
        Rollback(database);
        return false;
    }
    if (!cohort.active ||
        (cohort.state != kCohortStateRouting &&
         cohort.state != kCohortStateReserved) ||
        !SameCohortRoute(cohort, request)) {
        response = request;
        response.result = RESULT_STALE;
        return Commit(database);
    }
    if (!ExpireActive(database, request.account_id)) {
        Rollback(database);
        return false;
    }

    Message existing = request;
    existing.handoff_id = 0u;
    if (!LoadActiveForAccount(database, request.account_id, existing)) {
        Rollback(database);
        return false;
    }
    if (existing.handoff_id != 0u) {
        response = existing;
        response.session_token = request.session_token;
        response.request_token = request.request_token;
        response.operation = request.operation;
        response.reply_required = request.reply_required;
        response.result = SameOwnerAndRoute(existing, request)
                              ? RESULT_SUCCESS : RESULT_BUSY;
        ApplyCohort(cohort, response);
        response.cohort_slot = existing.cohort_slot;
        return Commit(database);
    }

    uint32_t inserted_handoff_id = 0u;
    if (!database->set_query(
            "inSert into %s(m_id,charac_no,field0,field1,endpoint_key,"
            "endpoint_port_like,field4,source_server_group,source_channel,"
            "target_server_group,target_channel,matching_bound,"
            "matching_mode,cohort_slot,cohort_size,state,source_cleaned,"
            "active_slot,created_at,"
            "expires_at,updated_at) values(%u,%u,%u,%u,%u,%u,0,%d,%d,"
            "%d,%d,%u,%u,%u,%u,%u,0,1,now(),"
            "date_add(now(),interval %u second),"
            "now())",
            kTable, static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.field0),
            static_cast<unsigned int>(request.field1),
            static_cast<unsigned int>(request.endpoint_key),
            static_cast<unsigned int>(request.endpoint_port_like),
            request.source_server_group, request.source_channel,
            request.target_server_group, request.target_channel,
            static_cast<unsigned int>(request.matching_bound),
            static_cast<unsigned int>(request.matching_mode),
            static_cast<unsigned int>(request.cohort_slot),
            static_cast<unsigned int>(request.cohort_size),
            static_cast<unsigned int>(STATE_RESERVED),
            static_cast<unsigned int>(kTicketLifetimeSeconds)) ||
        !database->exec(true) ||
        !ReadLastInsertId(database, inserted_handoff_id) ||
        !database->set_query(
            "upDate %s set field4=handoff_id where handoff_id=%u and "
            "field4=0", kTable,
            static_cast<unsigned int>(inserted_handoff_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u) {
        Rollback(database);
        return false;
    }

    // MySQL 5.5 evaluates a single-table UPDATE SET list from left to right.
    // The barrier predicates therefore compare the counter after the preceding
    // increment directly.  Adding one again would advance the cohort one
    // member early and violate SQL045's state/count trigger invariant.
    if (!database->set_query(
            "upDate %s set reserved_count=reserved_count+1,"
            "state=if(reserved_count=expected_count,%u,state),"
            "ready_at=if(reserved_count=expected_count,now(),ready_at),"
            "updated_at=now() where cohort_id=%u and active_slot=1 and "
            "state=%u and reserved_count<expected_count and expires_at>=now()",
            kCohortTable,
            static_cast<unsigned int>(kCohortStateReserved),
            static_cast<unsigned int>(request.field1),
            static_cast<unsigned int>(kCohortStateRouting)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !LoadCohort(database, request.field1, true, cohort)) {
        Rollback(database);
        return false;
    }
    const bool at_reserved_barrier =
        cohort.reserved_count == cohort.expected_count;
    if ((at_reserved_barrier &&
         cohort.state != kCohortStateReserved) ||
        (!at_reserved_barrier &&
         cohort.state != kCohortStateRouting)) {
        Rollback(database);
        return false;
    }
    response = request;
    response.handoff_id = inserted_handoff_id;
    response.field4 = inserted_handoff_id;
    response.state = STATE_RESERVED;
    response.result = RESULT_SUCCESS;
    ApplyCohort(cohort, response);
    response.cohort_slot = request.cohort_slot;
    return Commit(database);
}

bool HandleArm(MySQL* database,
               const Message& request,
               Message& response) {
    if (!Begin(database)) return false;
    CohortSnapshot cohort;
    if (!LoadCohort(database, request.field1, true, cohort)) {
        Rollback(database);
        return false;
    }
    bool active = false;
    Message stored = request;
    if (!LoadTicket(database, request.handoff_id, true, stored, active)) {
        Rollback(database);
        return false;
    }
    response = stored;
    response.session_token = request.session_token;
    response.request_token = request.request_token;
    response.operation = request.operation;
    response.reply_required = request.reply_required;
    if (!SameOwnerAndRoute(stored, request) ||
        !SameCohortRoute(cohort, request)) {
        response.result = RESULT_NOT_OWNER;
    } else if (stored.state == STATE_ARMED && stored.source_cleaned) {
        response.result = RESULT_SUCCESS;
    } else if (!active || !cohort.active ||
               cohort.state != kCohortStateReserved ||
               stored.state != STATE_RESERVED) {
        response.result = RESULT_STALE;
    } else if (!database->set_query(
                   "upDate %s set state=%u,source_cleaned=1,armed_at=now(),"
                   "updated_at=now() where handoff_id=%u and state=%u and "
                   "active_slot=1 and expires_at>=now()",
                   kTable, static_cast<unsigned int>(STATE_ARMED),
                   static_cast<unsigned int>(request.handoff_id),
                   static_cast<unsigned int>(STATE_RESERVED)) ||
               !database->exec(true)) {
        Rollback(database);
        return false;
    } else if (database->getAffectedRowCount() == 1u) {
        // See HandleReserve: armed_count is already incremented when the
        // barrier expression is evaluated by MySQL 5.5.
        if (!database->set_query(
                "upDate %s set armed_count=armed_count+1,"
                "state=if(armed_count=expected_count,%u,state),"
                "updated_at=now() where cohort_id=%u and active_slot=1 and "
                "state=%u and reserved_count=expected_count and "
                "armed_count<expected_count and expires_at>=now()",
                kCohortTable,
                static_cast<unsigned int>(kCohortStateArmed),
                static_cast<unsigned int>(request.field1),
                static_cast<unsigned int>(kCohortStateReserved)) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u ||
            !LoadCohort(database, request.field1, true, cohort)) {
            Rollback(database);
            return false;
        }
        const bool at_armed_barrier =
            cohort.armed_count == cohort.expected_count;
        if ((at_armed_barrier &&
             cohort.state != kCohortStateArmed) ||
            (!at_armed_barrier &&
             cohort.state != kCohortStateReserved)) {
            Rollback(database);
            return false;
        }
        response.state = STATE_ARMED;
        response.source_cleaned = 1u;
        response.result = RESULT_SUCCESS;
    } else {
        response.result = RESULT_STALE;
    }
    ApplyCohort(cohort, response);
    response.cohort_slot = stored.cohort_slot;
    return Commit(database);
}

bool HandleClaim(MySQL* database,
                 const Message& request,
                 Message& response) {
    if (!Begin(database)) return false;
    uint32_t cohort_id = 0u;
    if (!ReadTicketCohortId(database, request.handoff_id, cohort_id)) {
        Rollback(database);
        return false;
    }
    CohortSnapshot cohort;
    if (!LoadCohort(database, cohort_id, true, cohort)) {
        Rollback(database);
        return false;
    }
    bool active = false;
    Message stored = request;
    if (!LoadTicket(database, request.handoff_id, true, stored, active)) {
        Rollback(database);
        return false;
    }
    response = stored;
    response.session_token = request.session_token;
    response.request_token = request.request_token;
    response.operation = request.operation;
    response.reply_required = request.reply_required;

    const bool owner = stored.account_id == request.account_id &&
                       stored.charac_no == request.charac_no &&
                       stored.field0 == request.field0 &&
                       stored.field4 == request.field4 &&
                       SameCohortRoute(cohort, stored);
    const bool target = stored.target_server_group ==
                            request.target_server_group &&
                        stored.target_channel == request.target_channel;
    if (!owner) {
        response.result = RESULT_NOT_OWNER;
    } else if (!target) {
        response.result = RESULT_WRONG_TARGET;
    } else if (stored.state == STATE_CLAIMED) {
        response.result = RESULT_REPLAY;
    } else if (!active || !cohort.active ||
               (cohort.state != kCohortStateArmed &&
                cohort.state != kCohortStateClaiming) ||
               stored.state != STATE_ARMED ||
               !stored.source_cleaned) {
        response.result = RESULT_STALE;
    } else if (!database->set_query(
                   "upDate %s set state=%u,active_slot=null,claimed_at=now(),"
                   "updated_at=now() where handoff_id=%u and state=%u and "
                   "source_cleaned=1 and active_slot=1 and expires_at>=now()",
                   kTable, static_cast<unsigned int>(STATE_CLAIMED),
                   static_cast<unsigned int>(request.handoff_id),
                   static_cast<unsigned int>(STATE_ARMED)) ||
               !database->exec(true)) {
        Rollback(database);
        return false;
    } else if (database->getAffectedRowCount() == 1u) {
        // See HandleReserve: claimed_count is already incremented when the
        // barrier expression is evaluated by MySQL 5.5.
        if (!database->set_query(
                "upDate %s set claimed_count=claimed_count+1,"
                "state=if(claimed_count=expected_count,%u,%u),"
                "updated_at=now() where cohort_id=%u and active_slot=1 and "
                "state in (%u,%u) and armed_count=expected_count and "
                "claimed_count<expected_count and expires_at>=now()",
                kCohortTable,
                static_cast<unsigned int>(kCohortStateClaimed),
                static_cast<unsigned int>(kCohortStateClaiming),
                static_cast<unsigned int>(cohort_id),
                static_cast<unsigned int>(kCohortStateArmed),
                static_cast<unsigned int>(kCohortStateClaiming)) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u ||
            !LoadCohort(database, cohort_id, true, cohort)) {
            Rollback(database);
            return false;
        }
        const bool at_claimed_barrier =
            cohort.claimed_count == cohort.expected_count;
        if ((at_claimed_barrier &&
             cohort.state != kCohortStateClaimed) ||
            (!at_claimed_barrier &&
             cohort.state != kCohortStateClaiming)) {
            Rollback(database);
            return false;
        }
        response.state = STATE_CLAIMED;
        response.result = RESULT_SUCCESS;
    } else {
        response.result = RESULT_STALE;
    }
    ApplyCohort(cohort, response);
    response.cohort_slot = stored.cohort_slot;
    return Commit(database);
}

bool HandleCancel(MySQL* database,
                  const Message& request,
                  Message& response) {
    if (!Begin(database)) return false;
    CohortSnapshot cohort;
    if (!LoadCohort(database, request.field1, true, cohort)) {
        Rollback(database);
        return false;
    }
    bool active = false;
    Message stored = request;
    if (!LoadTicket(database, request.handoff_id, true, stored, active)) {
        Rollback(database);
        return false;
    }
    response = stored;
    response.session_token = request.session_token;
    response.request_token = request.request_token;
    response.operation = request.operation;
    response.reply_required = request.reply_required;
    if (!SameOwnerAndRoute(stored, request) ||
        !SameCohortRoute(cohort, request)) {
        response.result = RESULT_NOT_OWNER;
    } else if (cohort.state == kCohortStateAborted &&
               stored.state == STATE_CANCELED) {
        response.result = RESULT_SUCCESS;
    } else if (!active || (stored.state != STATE_RESERVED &&
                           stored.state != STATE_ARMED)) {
        response.result = stored.state == STATE_CANCELED
                              ? RESULT_SUCCESS : RESULT_STALE;
    } else if (!database->set_query(
                   "upDate %s set state=%u,active_slot=null,updated_at=now() "
                   "where field1=%u and active_slot=1 and state in (%u,%u)",
                   kTable, static_cast<unsigned int>(STATE_CANCELED),
                   static_cast<unsigned int>(request.field1),
                   static_cast<unsigned int>(STATE_RESERVED),
                   static_cast<unsigned int>(STATE_ARMED)) ||
               !database->exec(true) ||
               !database->set_query(
                   "upDate %s set state=%u,active_slot=null,updated_at=now() "
                   "where cohort_id=%u and active_slot=1 and state between "
                   "%u and %u",
                   kCohortTable,
                   static_cast<unsigned int>(kCohortStateAborted),
                   static_cast<unsigned int>(request.field1),
                   static_cast<unsigned int>(kCohortStateRouting),
                   static_cast<unsigned int>(kCohortStateActive)) ||
               !database->exec(true) ||
               !LoadCohort(database, request.field1, true, cohort)) {
        Rollback(database);
        return false;
    } else {
        response.state = STATE_CANCELED;
        response.result = cohort.state == kCohortStateAborted
                              ? RESULT_SUCCESS : RESULT_STALE;
    }
    ApplyCohort(cohort, response);
    response.cohort_slot = stored.cohort_slot;
    return Commit(database);
}

bool CancelActiveCohortTickets(MySQL* database, uint32_t cohort_id) {
    return database && database->set_query(
               "upDate %s set state=%u,active_slot=null,updated_at=now() "
               "where field1=%u and active_slot=1 and state in (%u,%u)",
               kTable, static_cast<unsigned int>(STATE_CANCELED),
               static_cast<unsigned int>(cohort_id),
               static_cast<unsigned int>(STATE_RESERVED),
               static_cast<unsigned int>(STATE_ARMED)) &&
           database->exec(true);
}

bool HandleCohortControl(MySQL* database,
                         const Message& request,
                         Message& response) {
    if (!Begin(database)) return false;
    CohortSnapshot cohort;
    if (!LoadCohort(database, request.field1, true, cohort)) {
        Rollback(database);
        return false;
    }
    response = request;
    ApplyCohort(cohort, response);
    bool control_authorized = false;
    const bool uses_target_proof = request.handoff_id != 0u;
    if (!uses_target_proof) {
        control_authorized = SameCohortAuthority(cohort, request);
        response.cohort_slot = 0u;
        response.handoff_id = 0u;
        response.field4 = 0u;
        response.state = STATE_NONE;
        response.source_cleaned = 0u;
    } else {
        bool proof_active = false;
        Message proof = request;
        if (!LoadTicket(database, request.handoff_id, true, proof,
                        proof_active)) {
            Rollback(database);
            return false;
        }
        (void)proof_active;
        control_authorized =
            proof.account_id == cohort.leader_account_id &&
            proof.charac_no == cohort.leader_charac_no &&
            proof.account_id == request.account_id &&
            proof.charac_no == request.charac_no &&
            proof.field1 == cohort.cohort_id && proof.cohort_slot == 0u &&
            proof.cohort_size == cohort.expected_count &&
            proof.state == STATE_CLAIMED && proof.source_cleaned &&
            proof.field4 == request.field4 &&
            SameOwnerAndRoute(proof, request) &&
            SameCohortRoute(cohort, request);
        response.cohort_slot = 0u;
        response.handoff_id = proof.handoff_id;
        response.field4 = proof.field4;
        response.state = STATE_CLAIMED;
        response.source_cleaned = 1u;
    }

    if (!control_authorized) {
        response.result = RESULT_NOT_OWNER;
        return Commit(database);
    }

    if (request.operation == OPERATION_ABORT_COHORT) {
        if (cohort.state == kCohortStateAborted) {
            response.result = RESULT_SUCCESS;
            return Commit(database);
        }
        if (cohort.state == kCohortStateComplete ||
            cohort.state == kCohortStateExpired) {
            response.result = RESULT_STALE;
            return Commit(database);
        }
        if (!CancelActiveCohortTickets(database, cohort.cohort_id) ||
            !database->set_query(
                "upDate %s set state=%u,active_slot=null,updated_at=now() "
                "where cohort_id=%u and active_slot=1 and state between "
                "%u and %u",
                kCohortTable,
                static_cast<unsigned int>(kCohortStateAborted),
                static_cast<unsigned int>(cohort.cohort_id),
                static_cast<unsigned int>(kCohortStateRouting),
                static_cast<unsigned int>(kCohortStateActive)) ||
            !database->exec(true) ||
            !LoadCohort(database, cohort.cohort_id, true, cohort)) {
            Rollback(database);
            return false;
        }
        response.result = cohort.state == kCohortStateAborted
                              ? RESULT_SUCCESS : RESULT_STALE;
    } else if (request.operation == OPERATION_ACTIVATE_COHORT) {
        if (cohort.state == kCohortStateActive) {
            response.result = RESULT_SUCCESS;
        } else if (cohort.state != kCohortStateClaimed || !cohort.active ||
                   cohort.claimed_count != cohort.expected_count) {
            response.result = RESULT_STALE;
        } else if (!database->set_query(
                       "upDate %s set state=%u,activated_at=now(),"
                       "expires_at=date_add(now(),interval %u second),"
                       "updated_at=now() where cohort_id=%u and "
                       "active_slot=1 and state=%u and "
                       "claimed_count=expected_count and expires_at>=now()",
                       kCohortTable,
                       static_cast<unsigned int>(kCohortStateActive),
                       static_cast<unsigned int>(
                           kActiveCohortLifetimeSeconds),
                       static_cast<unsigned int>(cohort.cohort_id),
                       static_cast<unsigned int>(kCohortStateClaimed)) ||
                   !database->exec(true) ||
                   database->getAffectedRowCount() != 1u ||
                   !LoadCohort(database, cohort.cohort_id, true, cohort)) {
            Rollback(database);
            return false;
        } else {
            response.result = RESULT_SUCCESS;
        }
    } else if (request.operation == OPERATION_COMPLETE_COHORT) {
        if (cohort.state == kCohortStateComplete) {
            response.result = RESULT_SUCCESS;
        } else if (cohort.state != kCohortStateActive || !cohort.active) {
            response.result = RESULT_STALE;
        } else if (!database->set_query(
                       "upDate %s set state=%u,active_slot=null,"
                       "updated_at=now() where cohort_id=%u and "
                       "active_slot=1 and state=%u",
                       kCohortTable,
                       static_cast<unsigned int>(kCohortStateComplete),
                       static_cast<unsigned int>(cohort.cohort_id),
                       static_cast<unsigned int>(kCohortStateActive)) ||
                   !database->exec(true) ||
                   database->getAffectedRowCount() != 1u ||
                   !LoadCohort(database, cohort.cohort_id, true, cohort)) {
            Rollback(database);
            return false;
        } else {
            response.result = RESULT_SUCCESS;
        }
    } else {
        Rollback(database);
        return false;
    }

    ApplyCohort(cohort, response);
    if (!uses_target_proof) {
        response.cohort_slot = 0u;
        response.handoff_id = 0u;
        response.field4 = 0u;
        response.state = STATE_NONE;
        response.source_cleaned = 0u;
    }
    return Commit(database);
}

bool SendResult(int user_idx, const Message& response) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    Message* const destination = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!destination) return false;
    *destination = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

}  // namespace

DBMatchingChannelHandoff::DBMatchingChannelHandoff()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBMatchingChannelHandoff::~DBMatchingChannelHandoff() {}

bool DBMatchingChannelHandoff::QueueRequest(
    int user_idx, const Message& request) {
    if (user_idx < 0 || !IsValidRequest(request)) return false;
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    Message* const destination = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!destination) return false;
    *destination = request;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

int DBMatchingChannelHandoff::dispatch(int signal,
                                            int user_idx,
                                            Stream* stream) {
    (void)signal;
    if (user_idx < 0 || !stream ||
        stream->length() != static_cast<int>(sizeof(Message))) return 0;
    Message request;
    if (stream->get_binary(&request, static_cast<int>(sizeof(request))) != 1 ||
        !IsValidRequest(request)) return 0;

    Message response = request;
    response.result = RESULT_DATABASE_FAILURE;
    MySQL* const database = GetDatabase(request);
    bool handled = false;
    if (database) {
        switch (request.operation) {
        case OPERATION_ROUTE_COHORT:
            handled = HandleRouteCohort(database, request, response);
            break;
        case OPERATION_RESERVE:
            handled = HandleReserve(database, request, response);
            break;
        case OPERATION_ARM:
            handled = HandleArm(database, request, response);
            break;
        case OPERATION_CLAIM:
            handled = HandleClaim(database, request, response);
            break;
        case OPERATION_CANCEL:
            handled = HandleCancel(database, request, response);
            break;
        case OPERATION_ABORT_COHORT:
        case OPERATION_ACTIVATE_COHORT:
        case OPERATION_COMPLETE_COHORT:
            handled = HandleCohortControl(database, request, response);
            break;
        default:
            break;
        }
    }
    if (!handled) {
        // Handlers may have populated response before a late SQL/commit
        // failure.  Return the original valid request shape so the game-side
        // bounded retry cannot inherit a half-created handoff id.
        response = request;
        response.result = RESULT_DATABASE_FAILURE;
    }
    return SendResult(user_idx, response) ? 1 : 0;
}

}  // namespace matching_channel_handoff_db_0828
