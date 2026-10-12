#include "old_data/db/pvp_party/db_pvp_tournament.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace pvp_tournament {
namespace {

const char kRootTable[] = "pvp_tournament_0828";
const char kParticipantTable[] = "pvp_tournament_participant_0828";
const char kMatchTable[] = "pvp_tournament_match_0828";

struct CharacterIdentity {
    CharacterIdentity()
        : account_id(0u), charac_no(0u), job(0u), grow_type(0u),
          level(0u), pvp_grade(0u), name() {
        std::memset(name, 0, sizeof(name));
    }

    uint32_t account_id;
    uint32_t charac_no;
    uint32_t job;
    uint32_t grow_type;
    uint32_t level;
    uint32_t pvp_grade;
    char name[kCharacterNameCapacity];
};

struct TeamSeed {
    TeamSeed() : team_id(0u), captain_charac_no(0u), seed(255u) {}
    uint32_t team_id;
    uint32_t captain_charac_no;
    uint32_t seed;
};

struct BracketNode {
    BracketNode()
        : match_id(0u), winner_team_id(0u), match_index(0u), round(0u) {}
    uint32_t match_id;
    uint32_t winner_team_id;
    uint8_t match_index;
    uint8_t round;
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

bool IsClientOperation(uint8_t operation) {
    return operation >= OPERATION_LOAD_REGISTRY &&
           operation <= OPERATION_FINALIZE_BRACKET;
}

bool IsBackgroundOperation(uint8_t operation) {
    return operation >= OPERATION_COMMIT_ROOM &&
           operation <= OPERATION_EXPIRE_SWEEP;
}

uint8_t ExpectedSubtype(uint8_t operation) {
    switch (operation) {
        case OPERATION_OPEN_DETAIL: return 0u;
        case OPERATION_CREATE: return 1u;
        case OPERATION_JOIN: return 2u;
        case OPERATION_LEAVE: return 3u;
        case OPERATION_INVITE_MEMBER: return 4u;
        case OPERATION_CANCEL: return 5u;
        case OPERATION_SAVE_SEEDS: return 6u;
        case OPERATION_START: return 7u;
        case OPERATION_RESERVE_ROOM: return 8u;
        case OPERATION_REMOVE_MEMBER: return 9u;
        case OPERATION_SET_SEED: return 10u;
        case OPERATION_FINALIZE_BRACKET: return 11u;
        default: return 0xffu;
    }
}

bool IsTerminated(const char* text, size_t capacity) {
    return text && std::memchr(text, 0, capacity) != 0;
}

size_t SafeLength(const char* text, size_t capacity) {
    if (!text) {
        return 0u;
    }
    size_t length = 0u;
    while (length < capacity && text[length] != 0) {
        ++length;
    }
    return length;
}

bool IsValidRequest(const Message& request, int outer_user_idx) {
    if (!IsValidServerGroup(request.server_group) ||
        request.operation < OPERATION_LOAD_REGISTRY ||
        request.operation > OPERATION_EXPIRE_SWEEP ||
        request.root_count != 0u || request.participant_count != 0u ||
        request.match_count != 0u || request.reserved[0] != 0u ||
        request.reserved[1] != 0u ||
        !IsTerminated(request.title, sizeof(request.title)) ||
        !IsTerminated(request.password, sizeof(request.password)) ||
        !IsTerminated(request.actor_name, sizeof(request.actor_name))) {
        return false;
    }
    if (IsClientOperation(request.operation)) {
        bool command_ok = false;
        if (request.operation == OPERATION_LOAD_REGISTRY) {
            command_ok = request.command_kind == COMMAND_NONE &&
                         request.request_subtype == 0u;
        } else if (request.operation == OPERATION_OPEN_DETAIL) {
            command_ok = request.request_subtype == 0u &&
                         (request.command_kind == COMMAND_544 ||
                          request.command_kind == COMMAND_545);
        } else {
            command_ok = request.command_kind == COMMAND_545 &&
                         request.request_subtype ==
                             ExpectedSubtype(request.operation);
        }
        return outer_user_idx >= 0 && request.reply_user_idx == outer_user_idx &&
               request.account_id != 0u && request.charac_no != 0u &&
               request.session_token != 0u && request.request_token != 0u &&
               command_ok;
    }
    if (!IsBackgroundOperation(request.operation) || outer_user_idx != -1 ||
        request.reply_user_idx != -1 || request.command_kind != COMMAND_NONE ||
        request.request_subtype != 0u || request.account_id != 0u ||
        request.charac_no != 0u || request.session_token != 0u ||
        request.request_token != 0u) {
        return false;
    }
    if (request.operation == OPERATION_EXPIRE_SWEEP) {
        return request.tournament_id == 0u && request.match_id == 0u &&
               request.room_token == 0u;
    }
    return request.tournament_id != 0u && request.match_id != 0u &&
           request.room_token != 0u;
}

MySQL* GetGameDatabase(int server_group) {
    DBMgr* const manager = GlobalData::s_db_mgr;
    if (!manager || !IsValidServerGroup(server_group)) {
        return 0;
    }
    const ENUM_SERVER_GROUP group =
        static_cast<ENUM_SERVER_GROUP>(server_group);
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

void ClearSnapshots(Message& message) {
    message.root_count = 0u;
    message.participant_count = 0u;
    message.match_count = 0u;
    std::memset(message.roots, 0, sizeof(message.roots));
    std::memset(message.participants, 0, sizeof(message.participants));
    std::memset(message.matches, 0, sizeof(message.matches));
}

void Fail(Message& response, uint8_t error) {
    response.error_code = error ? error : ERROR_GENERIC;
    response.command_result = 0u;
}

bool Escape(MySQL* database,
            const char* source,
            size_t capacity,
            char* destination,
            size_t destination_capacity) {
    if (!database || !source || !destination ||
        destination_capacity < capacity * 2u + 1u ||
        !IsTerminated(source, capacity)) {
        return false;
    }
    const int length = database->escape_string(destination, source);
    return length >= 0 &&
           static_cast<size_t>(length) < destination_capacity;
}

int LoadCharacter(MySQL* database,
                  uint32_t charac_no,
                  uint32_t expected_account_id,
                  CharacterIdentity& identity) {
    if (!database || charac_no == 0u ||
        !database->set_query(
            "seLect ci.m_id,ci.charac_no,ci.charac_name,ci.job,"
            "ci.grow_type,ci.lev,ifnull(pr.pvp_grade,0) "
            "from charac_info ci left join pvp_result pr "
            "on pr.charac_no=ci.charac_no where ci.charac_no=%u "
            "and ci.delete_flag=0%s limit 2",
            static_cast<unsigned int>(charac_no),
            expected_account_id ? " and ci.m_id>0" : "") ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) {
        return 0;
    }
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, identity.account_id) ||
        !database->get_uint(1, identity.charac_no) ||
        !database->get_str(2, identity.name, sizeof(identity.name)) ||
        !database->get_uint(3, identity.job) ||
        !database->get_uint(4, identity.grow_type) ||
        !database->get_uint(5, identity.level) ||
        !database->get_uint(6, identity.pvp_grade)) {
        return -1;
    }
    if (expected_account_id != 0u &&
        identity.account_id != expected_account_id) {
        return 0;
    }
    return 1;
}

bool ReadRootRow(MySQL* database, RootRecord& root) {
    if (!database ||
        !database->get_uint(0, root.tournament_id) ||
        !database->get_uint(1, root.scheduled_epoch) ||
        !database->get_uint(2, root.expires_epoch) ||
        !database->get_uint(3, root.organizer_charac_no) ||
        !database->get_uint(4, root.organizer_account_id) ||
        !database->get_uint(5, root.participant_count) ||
        !database->get_uint(6, root.team_count) ||
        !database->get_uint(7, root.version) ||
        !database->get_uint(8, root.winner_team_id) ||
        !database->get_ushort(9, root.qualification) ||
        !database->get_ubyte(10, root.state) ||
        !database->get_ubyte(11, root.private_flag) ||
        !database->get_ubyte(12, root.invite_only) ||
        !database->get_ubyte(13, root.bracket_ready) ||
        !database->get_ubyte(14, root.maximum_teams) ||
        !database->get_str(15, root.title, sizeof(root.title)) ||
        !database->get_str(16, root.organizer_name,
                           sizeof(root.organizer_name))) {
        return false;
    }
    std::memcpy(root.mode, "TEAM", 5u);
    root.reserved = 0u;
    return root.tournament_id != 0u && root.maximum_teams == 64u &&
           root.participant_count <= kMaximumParticipantCount &&
           root.team_count <= kMaximumTeamCount &&
           root.state >= TOURNAMENT_REGISTRATION &&
           root.state <= TOURNAMENT_CANCELLED_OR_EXPIRED;
}

bool SelectRoot(MySQL* database,
                uint32_t tournament_id,
                bool for_update,
                RootRecord& root) {
    if (!database || tournament_id == 0u ||
        !database->set_query(
            "seLect tournament_id,scheduled_epoch,expires_epoch,"
            "organizer_charac_no,organizer_m_id,participant_count,"
            "team_count,version,winner_team_id,qualification,state,"
            "private_flag,invite_only,bracket_ready,maximum_teams,title,"
            "organizer_name from %s where tournament_id=%u%s",
            kRootTable, static_cast<unsigned int>(tournament_id),
            for_update ? " for update" : "") ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    return ReadRootRow(database, root);
}

int FindCurrentTournament(MySQL* database,
                          uint32_t charac_no,
                          uint32_t& tournament_id) {
    tournament_id = 0u;
    if (!database || charac_no == 0u ||
        !database->set_query(
            "seLect tournament_id from %s where active_charac_no=%u "
            "limit 2", kParticipantTable,
            static_cast<unsigned int>(charac_no)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) {
        return 0;
    }
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, tournament_id) || tournament_id == 0u) {
        return -1;
    }
    return 1;
}

// The unique active_charac_no key is the durable cross-tournament mutex.
// Inserts use a no-op duplicate-key update so a concurrent winner does not
// turn an expected "already participating" race into ERROR_DATABASE.  This
// postcondition distinguishes our inserted row from the conflicting owner.
int VerifyActiveParticipantInsert(MySQL* database,
                                  uint32_t tournament_id,
                                  uint32_t charac_no) {
    if (!database || tournament_id == 0u || charac_no == 0u ||
        !database->set_query(
            "seLect 1 from %s where tournament_id=%u and charac_no=%u "
            "and approval=1 and active_charac_no=%u limit 2",
            kParticipantTable, static_cast<unsigned int>(tournament_id),
            static_cast<unsigned int>(charac_no),
            static_cast<unsigned int>(charac_no)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) {
        return 0;
    }
    return database->get_n_rows() == 1 ? 1 : -1;
}

// Lock the active-root index range before admitting a creator.  Under
// InnoDB REPEATABLE READ the locking range (including its end gap) makes the
// 64-root policy deterministic even when two DB workers create together.
int LockTournamentCapacity(MySQL* database, uint32_t now_epoch) {
    if (!database || now_epoch == 0u ||
        !database->set_query(
            "seLect tournament_id from %s where state in (3,4,5) "
            "and expires_epoch>%u order by state,expires_epoch,"
            "tournament_id limit %u for update", kRootTable,
            static_cast<unsigned int>(now_epoch),
            static_cast<unsigned int>(kMaximumTournamentCount + 1)) ||
        !database->exec(true)) {
        return -1;
    }
    const int rows = database->get_n_rows();
    if (rows < 0) {
        return -1;
    }
    if (rows >= kMaximumTournamentCount) {
        return 1;
    }
    return 0;
}

bool IsParticipant(MySQL* database,
                   uint32_t tournament_id,
                   uint32_t charac_no) {
    return database &&
           database->set_query(
               "seLect 1 from %s where tournament_id=%u and charac_no=%u "
               "and approval<>3 limit 1", kParticipantTable,
               static_cast<unsigned int>(tournament_id),
               static_cast<unsigned int>(charac_no)) &&
           database->exec(true) && database->get_n_rows() == 1;
}

bool CanView(MySQL* database,
             const RootRecord& root,
             uint32_t charac_no) {
    return root.private_flag == 0u ||
           root.organizer_charac_no == charac_no ||
           IsParticipant(database, root.tournament_id, charac_no);
}

bool LoadParticipants(MySQL* database,
                      uint32_t tournament_id,
                      Message& response) {
    if (!database ||
        !database->set_query(
            "seLect tournament_id,team_id,charac_no,m_id,charac_name,job,"
            "grow_type,level,pvp_grade,role,approval,seed,result from %s "
            "where tournament_id=%u and approval<>3 "
            "order by team_id,role desc,charac_no limit %u",
            kParticipantTable, static_cast<unsigned int>(tournament_id),
            static_cast<unsigned int>(kMaximumParticipantCount + 1)) ||
        !database->exec(true) ||
        database->get_n_rows() > kMaximumParticipantCount) {
        return false;
    }
    const int rows = database->get_n_rows();
    for (int index = 0; index < rows; ++index) {
        ParticipantRecord& participant = response.participants[index];
        uint32_t job = 0u;
        uint32_t grow = 0u;
        uint32_t level = 0u;
        uint32_t grade = 0u;
        uint32_t role = 0u;
        uint32_t approval = 0u;
        uint32_t seed = 0u;
        uint32_t result = 0u;
        if (!database->fetch() ||
            !database->get_uint(0, participant.tournament_id) ||
            !database->get_uint(1, participant.team_id) ||
            !database->get_uint(2, participant.charac_no) ||
            !database->get_uint(3, participant.account_id) ||
            !database->get_str(4, participant.name,
                               sizeof(participant.name)) ||
            !database->get_uint(5, job) ||
            !database->get_uint(6, grow) ||
            !database->get_uint(7, level) ||
            !database->get_uint(8, grade) ||
            !database->get_uint(9, role) ||
            !database->get_uint(10, approval) ||
            !database->get_uint(11, seed) ||
            !database->get_uint(12, result) ||
            job > 255u || grow > 255u || level > 255u || grade > 255u ||
            role > 2u || approval > 3u || seed > 255u || result > 255u) {
            return false;
        }
        participant.online_unique_id = 0u;
        participant.job = static_cast<uint8_t>(job);
        participant.grow_type = static_cast<uint8_t>(grow);
        participant.level = static_cast<uint8_t>(level);
        participant.pvp_grade = static_cast<uint8_t>(grade);
        participant.role = static_cast<uint8_t>(role);
        participant.approval = static_cast<uint8_t>(approval);
        participant.seed = static_cast<uint8_t>(seed);
        participant.result = static_cast<uint8_t>(result);
    }
    response.participant_count = static_cast<uint8_t>(rows);
    return true;
}

bool LoadMatches(MySQL* database,
                 uint32_t tournament_id,
                 Message& response) {
    if (!database ||
        !database->set_query(
            "seLect tournament_id,match_id,parent_match_id,"
            "left_child_match_id,right_child_match_id,left_team_id,"
            "right_team_id,winner_team_id,ifnull(room_token,0),"
            "match_index,round_no,state,parent_side from %s "
            "where tournament_id=%u order by match_index limit %u",
            kMatchTable, static_cast<unsigned int>(tournament_id),
            static_cast<unsigned int>(kMaximumMatchCount + 1)) ||
        !database->exec(true) || database->get_n_rows() > kMaximumMatchCount) {
        return false;
    }
    const int rows = database->get_n_rows();
    for (int index = 0; index < rows; ++index) {
        MatchRecord& match = response.matches[index];
        uint32_t match_index = 0u;
        uint32_t round = 0u;
        uint32_t state = 0u;
        uint32_t parent_side = 0u;
        if (!database->fetch() ||
            !database->get_uint(0, match.tournament_id) ||
            !database->get_uint(1, match.match_id) ||
            !database->get_uint(2, match.parent_match_id) ||
            !database->get_uint(3, match.left_child_match_id) ||
            !database->get_uint(4, match.right_child_match_id) ||
            !database->get_uint(5, match.left_team_id) ||
            !database->get_uint(6, match.right_team_id) ||
            !database->get_uint(7, match.winner_team_id) ||
            !database->get_uint(8, match.room_token) ||
            !database->get_uint(9, match_index) ||
            !database->get_uint(10, round) ||
            !database->get_uint(11, state) ||
            !database->get_uint(12, parent_side) || match_index > 62u ||
            round > 6u || state > MATCH_CANCELLED || parent_side > 2u) {
            return false;
        }
        match.match_index = static_cast<uint8_t>(match_index);
        match.round = static_cast<uint8_t>(round);
        match.state = static_cast<uint8_t>(state);
        match.parent_side = static_cast<uint8_t>(parent_side);
    }
    response.match_count = static_cast<uint8_t>(rows);
    return true;
}

bool FillTournamentSnapshot(MySQL* database,
                            uint32_t tournament_id,
                            Message& response) {
    ClearSnapshots(response);
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!SelectRoot(database, tournament_id, false, root)) {
        return false;
    }
    response.roots[0] = root;
    response.root_count = 1u;
    response.tournament_id = tournament_id;
    return LoadParticipants(database, tournament_id, response) &&
           LoadMatches(database, tournament_id, response);
}

bool LoadVisibleRegistry(MySQL* database,
                         uint32_t charac_no,
                         uint32_t preferred_tournament_id,
                         Message& response) {
    ClearSnapshots(response);
    if (!database ||
        !database->set_query(
            "seLect t.tournament_id,t.scheduled_epoch,t.expires_epoch,"
            "t.organizer_charac_no,t.organizer_m_id,t.participant_count,"
            "t.team_count,t.version,t.winner_team_id,t.qualification,"
            "t.state,t.private_flag,t.invite_only,t.bracket_ready,"
            "t.maximum_teams,t.title,t.organizer_name from %s t "
            "where t.state in (3,4,5) and t.expires_epoch>unix_timestamp() "
            "and (t.private_flag=0 or t.organizer_charac_no=%u or exists("
            "seLect 1 from %s p where p.tournament_id=t.tournament_id "
            "and p.charac_no=%u and p.approval<>3)) "
            "order by (t.tournament_id=%u) desc,t.state,"
            "t.scheduled_epoch,t.tournament_id limit %u",
            kRootTable, static_cast<unsigned int>(charac_no),
            kParticipantTable, static_cast<unsigned int>(charac_no),
            static_cast<unsigned int>(preferred_tournament_id),
            static_cast<unsigned int>(kMaximumTournamentCount)) ||
        !database->exec(true)) {
        return false;
    }
    const int rows = database->get_n_rows();
    if (rows < 0 || rows > kMaximumTournamentCount) {
        return false;
    }
    for (int index = 0; index < rows; ++index) {
        if (!database->fetch() ||
            !ReadRootRow(database, response.roots[index])) {
            return false;
        }
    }
    response.root_count = static_cast<uint8_t>(rows);
    return true;
}

bool RefreshCounts(MySQL* database, uint32_t tournament_id) {
    uint32_t participants = 0u;
    uint32_t teams = 0u;
    if (!database ||
        !database->set_query(
            "seLect count(*),count(distinct team_id) from %s "
            "where tournament_id=%u and approval=1",
            kParticipantTable, static_cast<unsigned int>(tournament_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() || !database->get_uint(0, participants) ||
        !database->get_uint(1, teams) ||
        participants > kMaximumParticipantCount || teams > kMaximumTeamCount ||
        !database->set_query(
            "upDate %s set participant_count=%u,team_count=%u,"
            "version=version+1 where tournament_id=%u",
            kRootTable, static_cast<unsigned int>(participants),
            static_cast<unsigned int>(teams),
            static_cast<unsigned int>(tournament_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u) {
        return false;
    }
    return true;
}

bool InvalidateBracket(MySQL* database, uint32_t tournament_id) {
    return database &&
           database->set_query("deLete from %s where tournament_id=%u",
                               kMatchTable,
                               static_cast<unsigned int>(tournament_id)) &&
           database->exec(true) &&
           database->set_query(
               "upDate %s set bracket_ready=0,winner_team_id=0,"
               "version=version+1 where tournament_id=%u",
               kRootTable, static_cast<unsigned int>(tournament_id)) &&
           database->exec(true) && database->getAffectedRowCount() == 1u;
}

bool ExpireRootLocked(MySQL* database,
                      RootRecord& root,
                      uint32_t now_epoch,
                      bool& expired) {
    expired = false;
    if (root.state != TOURNAMENT_REGISTRATION &&
        root.state != TOURNAMENT_RUNNING &&
        root.state != TOURNAMENT_FINISHED) {
        return true;
    }
    if (now_epoch < root.expires_epoch) {
        return true;
    }
    if (!database->set_query(
            "upDate %s set state=6,finished_epoch=%u,bracket_ready=0,"
            "winner_team_id=0,version=version+1 where tournament_id=%u "
            "and state in (3,4,5)", kRootTable,
            static_cast<unsigned int>(now_epoch),
            static_cast<unsigned int>(root.tournament_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate %s set approval=3,active_charac_no=null "
            "where tournament_id=%u and approval in (0,1)",
            kParticipantTable,
            static_cast<unsigned int>(root.tournament_id)) ||
        !database->exec(true) ||
        !database->set_query(
            "upDate %s set state=5,room_token=null,room_lease_epoch=null "
            "where tournament_id=%u and state<>4", kMatchTable,
            static_cast<unsigned int>(root.tournament_id)) ||
        !database->exec(true)) {
        return false;
    }
    root.state = TOURNAMENT_CANCELLED_OR_EXPIRED;
    expired = true;
    return true;
}

bool LockCurrentRoot(MySQL* database,
                     uint32_t charac_no,
                     uint32_t now_epoch,
                     RootRecord& root,
                     Message& response) {
    uint32_t tournament_id = 0u;
    const int current = FindCurrentTournament(database, charac_no,
                                              tournament_id);
    if (current < 0) {
        return false;
    }
    if (current == 0 || !SelectRoot(database, tournament_id, true, root)) {
        Fail(response, ERROR_TOURNAMENT_MISSING);
        return true;
    }
    bool expired = false;
    if (!ExpireRootLocked(database, root, now_epoch, expired)) {
        return false;
    }
    if (expired) {
        Fail(response, ERROR_ENDED);
    }
    response.tournament_id = root.tournament_id;
    return true;
}

bool IsOrganizer(const RootRecord& root, uint32_t charac_no) {
    return root.organizer_charac_no == charac_no;
}

bool CheckEligibility(const CharacterIdentity& identity,
                      uint16_t qualification,
                      Message& response) {
    if (identity.level < 20u) {
        Fail(response, ERROR_LEVEL);
        return false;
    }
    if (identity.pvp_grade < qualification) {
        Fail(response, ERROR_PVP_GRADE);
        return false;
    }
    return true;
}

uint8_t ResultForOperation(uint8_t operation) {
    switch (operation) {
        case OPERATION_CREATE: return 1u;
        case OPERATION_JOIN: return 2u;
        case OPERATION_LEAVE: return 3u;
        case OPERATION_CANCEL: return 5u;
        case OPERATION_FINALIZE_BRACKET: return 6u;
        default: return 4u;
    }
}

bool HandleLoad(MySQL* database,
                const Message& request,
                Message& response) {
    uint32_t current_id = 0u;
    int current = FindCurrentTournament(database, request.charac_no,
                                        current_id);
    if (current < 0) {
        return false;
    }
    if (current == 1) {
        if (!Begin(database)) return false;
        RootRecord root;
        std::memset(&root, 0, sizeof(root));
        bool expired = false;
        if (!SelectRoot(database, current_id, true, root) ||
            !ExpireRootLocked(database, root, request.expires_epoch,
                              expired) ||
            !Commit(database)) {
            Rollback(database);
            return false;
        }
        if (expired ||
            root.state == TOURNAMENT_CANCELLED_OR_EXPIRED ||
            root.state == TOURNAMENT_FINISHED) {
            current = 0;
            current_id = 0u;
        }
    }
    if (!LoadVisibleRegistry(database, request.charac_no,
                             current == 1 ? current_id : 0u, response)) {
        return false;
    }
    response.tournament_id = current == 1 ? current_id : 0u;
    if (current == 1) {
        // Registry is retained in roots; detail arrays are filled separately.
        return LoadParticipants(database, current_id, response) &&
               LoadMatches(database, current_id, response);
    }
    return true;
}

bool HandleOpen(MySQL* database,
                const Message& request,
                Message& response) {
    if (!Begin(database)) {
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!SelectRoot(database, request.tournament_id, true, root)) {
        Rollback(database);
        Fail(response, ERROR_TOURNAMENT_MISSING);
        return true;
    }
    bool expired = false;
    if (!ExpireRootLocked(database, root, request.expires_epoch, expired)) {
        Rollback(database);
        return false;
    }
    if (expired || root.state == TOURNAMENT_CANCELLED_OR_EXPIRED) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        Fail(response, ERROR_ENDED);
        return true;
    }
    if (!CanView(database, root, request.charac_no)) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool HandleCreate(MySQL* database,
                  const Message& request,
                  Message& response) {
    const uint32_t now_epoch = request.expires_epoch;
    const size_t title_length = SafeLength(request.title, sizeof(request.title));
    const size_t password_length = SafeLength(request.password,
                                              sizeof(request.password));
    if (title_length == 0u || title_length > 32u || password_length > 8u ||
        request.flag_a > 1u || request.flag_b > 1u ||
        request.qualification > 30u ||
        request.scheduled_epoch < now_epoch ||
        request.scheduled_epoch > now_epoch + kTournamentLifetimeSeconds) {
        Fail(response, request.scheduled_epoch < now_epoch ||
                       request.scheduled_epoch >
                           now_epoch + kTournamentLifetimeSeconds
                           ? ERROR_SCHEDULE : ERROR_PACKET);
        return true;
    }
    CharacterIdentity actor;
    const int character = LoadCharacter(database, request.charac_no,
                                        request.account_id, actor);
    if (character < 0) {
        return false;
    }
    if (character == 0) {
        Fail(response, ERROR_PLAYER_MISSING);
        return true;
    }
    if (!CheckEligibility(actor, request.qualification, response)) {
        return true;
    }

    char escaped_title[kTitleCapacity * 2 + 1] = {};
    char escaped_password[kPasswordCapacity * 2 + 1] = {};
    char escaped_name[kCharacterNameCapacity * 2 + 1] = {};
    if (!Escape(database, request.title, sizeof(request.title),
                escaped_title, sizeof(escaped_title)) ||
        !Escape(database, request.password, sizeof(request.password),
                escaped_password, sizeof(escaped_password)) ||
        !Escape(database, actor.name, sizeof(actor.name),
                escaped_name, sizeof(escaped_name)) || !Begin(database)) {
        return false;
    }
    uint32_t existing = 0u;
    const int current = FindCurrentTournament(database, request.charac_no,
                                              existing);
    if (current < 0) {
        Rollback(database);
        return false;
    }
    if (current == 1) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    const int capacity = LockTournamentCapacity(database, now_epoch);
    if (capacity < 0) {
        Rollback(database);
        return false;
    }
    if (capacity > 0) {
        Rollback(database);
        Fail(response, ERROR_NO_SLOT);
        return true;
    }
    const uint32_t expires = now_epoch + kTournamentLifetimeSeconds;
    const bool root_query = password_length == 0u
        ? database->set_query(
              "inSert into %s(organizer_charac_no,organizer_m_id,"
              "organizer_name,title,password_sha1,qualification,"
              "scheduled_epoch,created_epoch,expires_epoch,state,"
              "private_flag,invite_only,maximum_teams,"
              "maximum_team_members,participant_count,team_count,"
              "bracket_ready,winner_team_id,version) values("
              "%u,%u,'%s','%s',null,%u,%u,%u,%u,3,%u,%u,64,4,1,1,0,0,1)",
              kRootTable, static_cast<unsigned int>(actor.charac_no),
              static_cast<unsigned int>(actor.account_id), escaped_name,
              escaped_title, static_cast<unsigned int>(request.qualification),
              static_cast<unsigned int>(request.scheduled_epoch),
              static_cast<unsigned int>(now_epoch),
              static_cast<unsigned int>(expires),
              static_cast<unsigned int>(request.flag_a),
              static_cast<unsigned int>(request.flag_b))
        : database->set_query(
              "inSert into %s(organizer_charac_no,organizer_m_id,"
              "organizer_name,title,password_sha1,qualification,"
              "scheduled_epoch,created_epoch,expires_epoch,state,"
              "private_flag,invite_only,maximum_teams,"
              "maximum_team_members,participant_count,team_count,"
              "bracket_ready,winner_team_id,version) values("
              "%u,%u,'%s','%s',unhex(sha1('%s')),%u,%u,%u,%u,3,%u,%u,"
              "64,4,1,1,0,0,1)", kRootTable,
              static_cast<unsigned int>(actor.charac_no),
              static_cast<unsigned int>(actor.account_id), escaped_name,
              escaped_title, escaped_password,
              static_cast<unsigned int>(request.qualification),
              static_cast<unsigned int>(request.scheduled_epoch),
              static_cast<unsigned int>(now_epoch),
              static_cast<unsigned int>(expires),
              static_cast<unsigned int>(request.flag_a),
              static_cast<unsigned int>(request.flag_b));
    if (!root_query || !database->exec(true) ||
        database->getAffectedRowCount() != 1u ||
        !database->set_query("seLect last_insert_id()") ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        Rollback(database);
        return false;
    }
    unsigned long long inserted = 0u;
    if (!database->get_ulonglong(0, inserted) || inserted == 0u ||
        inserted > 0xffffffffULL) {
        Rollback(database);
        return false;
    }
    const uint32_t tournament_id = static_cast<uint32_t>(inserted);
    if (!database->set_query(
            "inSert into %s(tournament_id,team_id,charac_no,m_id,"
            "charac_name,job,grow_type,level,pvp_grade,role,approval,seed,"
            "result,active_charac_no,joined_epoch) values("
            "%u,1,%u,%u,'%s',%u,%u,%u,%u,2,1,255,0,%u,%u) "
            "on duplicate key upDate active_charac_no=active_charac_no",
            kParticipantTable, static_cast<unsigned int>(tournament_id),
            static_cast<unsigned int>(actor.charac_no),
            static_cast<unsigned int>(actor.account_id), escaped_name,
            static_cast<unsigned int>(actor.job),
            static_cast<unsigned int>(actor.grow_type),
            static_cast<unsigned int>(actor.level),
            static_cast<unsigned int>(actor.pvp_grade),
            static_cast<unsigned int>(actor.charac_no),
            static_cast<unsigned int>(now_epoch)) ||
        !database->exec(true)) {
        Rollback(database);
        return false;
    }
    const int active_insert = VerifyActiveParticipantInsert(
        database, tournament_id, actor.charac_no);
    if (active_insert < 0) {
        Rollback(database);
        return false;
    }
    if (active_insert == 0) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    response.tournament_id = tournament_id;
    response.command_result = 1u;
    return FillTournamentSnapshot(database, tournament_id, response);
}

bool PasswordMatches(MySQL* database,
                     uint32_t tournament_id,
                     const char* password) {
    char escaped[kPasswordCapacity * 2 + 1] = {};
    uint32_t matches = 0u;
    return Escape(database, password, kPasswordCapacity,
                  escaped, sizeof(escaped)) &&
           database->set_query(
               "seLect if(password_sha1 is null,1,"
               "password_sha1=unhex(sha1('%s'))) from %s "
               "where tournament_id=%u", escaped, kRootTable,
               static_cast<unsigned int>(tournament_id)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, matches) &&
           matches == 1u;
}

bool HandleJoin(MySQL* database,
                const Message& request,
                Message& response) {
    CharacterIdentity actor;
    const int character = LoadCharacter(database, request.charac_no,
                                        request.account_id, actor);
    if (character < 0) {
        return false;
    }
    if (character == 0) {
        Fail(response, ERROR_PLAYER_MISSING);
        return true;
    }
    char escaped_name[kCharacterNameCapacity * 2 + 1] = {};
    if (!Escape(database, actor.name, sizeof(actor.name), escaped_name,
                sizeof(escaped_name)) || !Begin(database)) {
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!SelectRoot(database, request.tournament_id, true, root)) {
        Rollback(database);
        Fail(response, ERROR_TOURNAMENT_MISSING);
        return true;
    }
    bool expired = false;
    if (!ExpireRootLocked(database, root, request.expires_epoch, expired)) {
        Rollback(database);
        return false;
    }
    if (expired || root.state == TOURNAMENT_CANCELLED_OR_EXPIRED) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        Fail(response, ERROR_ENDED);
        return true;
    }
    if (root.state != TOURNAMENT_REGISTRATION) {
        Rollback(database);
        Fail(response, ERROR_STATE);
        return true;
    }
    if (!CheckEligibility(actor, root.qualification, response)) {
        Rollback(database);
        return true;
    }
    if (!PasswordMatches(database, root.tournament_id, request.password)) {
        Rollback(database);
        Fail(response, ERROR_PASSWORD);
        return true;
    }

    uint32_t existing_tournament = 0u;
    const int active = FindCurrentTournament(database, actor.charac_no,
                                             existing_tournament);
    if (active < 0) {
        Rollback(database);
        return false;
    }
    if (active == 1) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }

    uint32_t team_id = 0u;
    uint32_t prior_approval = 99u;
    if (!database->set_query(
            "seLect team_id,approval from %s where tournament_id=%u "
            "and charac_no=%u limit 2", kParticipantTable,
            static_cast<unsigned int>(root.tournament_id),
            static_cast<unsigned int>(actor.charac_no)) ||
        !database->exec(true) || database->get_n_rows() > 1) {
        Rollback(database);
        return false;
    }
    if (database->get_n_rows() == 1 &&
        (!database->fetch() || !database->get_uint(0, team_id) ||
         !database->get_uint(1, prior_approval))) {
        Rollback(database);
        return false;
    }
    if (prior_approval == PARTICIPANT_ACTIVE) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (root.invite_only && prior_approval != PARTICIPANT_PENDING) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (root.participant_count >= kMaximumParticipantCount) {
        Rollback(database);
        Fail(response, ERROR_NO_SLOT);
        return true;
    }
    if (prior_approval != PARTICIPANT_PENDING) {
        // A removed historical row does not grant its old team membership.
        // Public registration always creates a fresh captain team.
        team_id = 0u;
        if (root.team_count >= kMaximumTeamCount) {
            Rollback(database);
            Fail(response, ERROR_NO_SLOT);
            return true;
        }
    }
    if (team_id == 0u) {
        if (!database->set_query(
                "seLect distinct team_id from %s where tournament_id=%u "
                "and approval=1 order by team_id", kParticipantTable,
                static_cast<unsigned int>(root.tournament_id)) ||
            !database->exec(true) ||
            database->get_n_rows() > kMaximumTeamCount) {
            Rollback(database);
            return false;
        }
        bool occupied[kMaximumTeamCount + 1] = {};
        const int rows = database->get_n_rows();
        for (int index = 0; index < rows; ++index) {
            uint32_t occupied_team = 0u;
            if (!database->fetch() ||
                !database->get_uint(0, occupied_team) ||
                occupied_team == 0u ||
                occupied_team > kMaximumTeamCount) {
                Rollback(database);
                return false;
            }
            occupied[occupied_team] = true;
        }
        for (uint32_t candidate = 1u;
             candidate <= kMaximumTeamCount; ++candidate) {
            if (!occupied[candidate]) {
                team_id = candidate;
                break;
            }
        }
    }
    if (team_id == 0u || team_id > kMaximumTeamCount) {
        Rollback(database);
        Fail(response, ERROR_NO_SLOT);
        return true;
    }

    if (prior_approval != 99u &&
        (!database->set_query(
             "deLete from %s where tournament_id=%u and charac_no=%u",
             kParticipantTable,
             static_cast<unsigned int>(root.tournament_id),
             static_cast<unsigned int>(actor.charac_no)) ||
         !database->exec(true))) {
        Rollback(database);
        return false;
    }
    const uint32_t role = prior_approval == PARTICIPANT_PENDING
                              ? PARTICIPANT_MEMBER
                              : PARTICIPANT_CAPTAIN;
    if (!database->set_query(
            "inSert into %s(tournament_id,team_id,charac_no,m_id,"
            "charac_name,job,grow_type,level,pvp_grade,role,approval,seed,"
            "result,active_charac_no,joined_epoch) values("
            "%u,%u,%u,%u,'%s',%u,%u,%u,%u,%u,1,255,0,%u,%u) "
            "on duplicate key upDate active_charac_no=active_charac_no",
            kParticipantTable,
            static_cast<unsigned int>(root.tournament_id),
            static_cast<unsigned int>(team_id),
            static_cast<unsigned int>(actor.charac_no),
            static_cast<unsigned int>(actor.account_id), escaped_name,
            static_cast<unsigned int>(actor.job),
            static_cast<unsigned int>(actor.grow_type),
            static_cast<unsigned int>(actor.level),
            static_cast<unsigned int>(actor.pvp_grade),
            static_cast<unsigned int>(role),
            static_cast<unsigned int>(actor.charac_no),
            static_cast<unsigned int>(request.expires_epoch)) ||
        !database->exec(true)) {
        Rollback(database);
        return false;
    }
    const int active_insert = VerifyActiveParticipantInsert(
        database, root.tournament_id, actor.charac_no);
    if (active_insert < 0) {
        Rollback(database);
        return false;
    }
    if (active_insert == 0) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (!RefreshCounts(database, root.tournament_id) ||
        !InvalidateBracket(database, root.tournament_id) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.command_result = 2u;
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool HandleLeave(MySQL* database,
                 const Message& request,
                 Message& response) {
    if (!Begin(database)) {
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!LockCurrentRoot(database, request.charac_no, request.expires_epoch,
                         root, response)) {
        Rollback(database);
        return false;
    }
    if (response.error_code != 0u) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (root.state != TOURNAMENT_REGISTRATION) {
        Rollback(database);
        Fail(response, ERROR_STATE);
        return true;
    }
    if (IsOrganizer(root, request.charac_no)) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    uint32_t team_id = 0u;
    uint32_t role = 0u;
    if (!database->set_query(
            "seLect team_id,role from %s where tournament_id=%u "
            "and charac_no=%u and approval=1 for update",
            kParticipantTable, static_cast<unsigned int>(root.tournament_id),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() || !database->get_uint(0, team_id) ||
        !database->get_uint(1, role) ||
        !database->set_query(
            "upDate %s set approval=3,active_charac_no=null,result=0 "
            "where tournament_id=%u and charac_no=%u and approval=1",
            kParticipantTable, static_cast<unsigned int>(root.tournament_id),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u) {
        Rollback(database);
        return false;
    }
    if (role == PARTICIPANT_CAPTAIN) {
        uint32_t promoted = 0u;
        if (!database->set_query(
                "seLect charac_no from %s where tournament_id=%u "
                "and team_id=%u and approval=1 order by charac_no limit 1",
                kParticipantTable,
                static_cast<unsigned int>(root.tournament_id),
                static_cast<unsigned int>(team_id)) ||
            !database->exec(true)) {
            Rollback(database);
            return false;
        }
        if (database->get_n_rows() == 1 &&
            (!database->fetch() || !database->get_uint(0, promoted) ||
             !database->set_query(
                 "upDate %s set role=1 where tournament_id=%u "
                 "and charac_no=%u", kParticipantTable,
                 static_cast<unsigned int>(root.tournament_id),
                 static_cast<unsigned int>(promoted)) ||
             !database->exec(true) ||
             database->getAffectedRowCount() != 1u)) {
            Rollback(database);
            return false;
        }
    }
    if (!RefreshCounts(database, root.tournament_id) ||
        !InvalidateBracket(database, root.tournament_id) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.command_result = 3u;
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool LoadActorTeamAndRole(MySQL* database,
                          uint32_t tournament_id,
                          uint32_t charac_no,
                          uint32_t& team_id,
                          uint32_t& role) {
    return database &&
           database->set_query(
               "seLect team_id,role from %s where tournament_id=%u "
               "and charac_no=%u and approval=1 limit 1",
               kParticipantTable, static_cast<unsigned int>(tournament_id),
               static_cast<unsigned int>(charac_no)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, team_id) &&
           database->get_uint(1, role);
}

bool HandleInvite(MySQL* database,
                  const Message& request,
                  Message& response) {
    CharacterIdentity target;
    const int target_result = LoadCharacter(database, request.target_charac_no,
                                            0u, target);
    if (target_result < 0) {
        return false;
    }
    if (target_result == 0) {
        Fail(response, ERROR_PLAYER_MISSING);
        return true;
    }
    char escaped_name[kCharacterNameCapacity * 2 + 1] = {};
    if (!Escape(database, target.name, sizeof(target.name), escaped_name,
                sizeof(escaped_name)) || !Begin(database)) {
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!LockCurrentRoot(database, request.charac_no, request.expires_epoch,
                         root, response)) {
        Rollback(database);
        return false;
    }
    if (response.error_code != 0u) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (root.state != TOURNAMENT_REGISTRATION) {
        Rollback(database);
        Fail(response, ERROR_STATE);
        return true;
    }
    if (!CheckEligibility(target, root.qualification, response)) {
        Rollback(database);
        return true;
    }
    uint32_t actor_team = 0u;
    uint32_t actor_role = 0u;
    if (!LoadActorTeamAndRole(database, root.tournament_id,
                              request.charac_no, actor_team, actor_role)) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (!IsOrganizer(root, request.charac_no) &&
        actor_role != PARTICIPANT_CAPTAIN) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    uint32_t active_id = 0u;
    const int active = FindCurrentTournament(database, target.charac_no,
                                             active_id);
    if (active < 0) {
        Rollback(database);
        return false;
    }
    if (active == 1) {
        Rollback(database);
        Fail(response, active_id == root.tournament_id
                           ? ERROR_ALREADY_TEAMMATE : ERROR_PERMISSION);
        return true;
    }
    uint32_t team_count = 0u;
    if (!database->set_query(
            "seLect count(*) from %s where tournament_id=%u and team_id=%u "
            "and approval=1", kParticipantTable,
            static_cast<unsigned int>(root.tournament_id),
            static_cast<unsigned int>(actor_team)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() || !database->get_uint(0, team_count)) {
        Rollback(database);
        return false;
    }
    if (team_count >= kMaximumTeamMemberCount ||
        root.participant_count >= kMaximumParticipantCount) {
        Rollback(database);
        Fail(response, ERROR_NO_SLOT);
        return true;
    }
    if (!database->set_query(
            "deLete from %s where tournament_id=%u and charac_no=%u "
            "and approval in (0,3)", kParticipantTable,
            static_cast<unsigned int>(root.tournament_id),
            static_cast<unsigned int>(target.charac_no)) ||
        !database->exec(true) ||
        !database->set_query(
            "inSert into %s(tournament_id,team_id,charac_no,m_id,"
            "charac_name,job,grow_type,level,pvp_grade,role,approval,seed,"
            "result,active_charac_no,joined_epoch) values("
            "%u,%u,%u,%u,'%s',%u,%u,%u,%u,0,1,255,0,%u,%u) "
            "on duplicate key upDate active_charac_no=active_charac_no",
            kParticipantTable,
            static_cast<unsigned int>(root.tournament_id),
            static_cast<unsigned int>(actor_team),
            static_cast<unsigned int>(target.charac_no),
            static_cast<unsigned int>(target.account_id), escaped_name,
            static_cast<unsigned int>(target.job),
            static_cast<unsigned int>(target.grow_type),
            static_cast<unsigned int>(target.level),
            static_cast<unsigned int>(target.pvp_grade),
            static_cast<unsigned int>(target.charac_no),
            static_cast<unsigned int>(request.expires_epoch)) ||
        !database->exec(true)) {
        Rollback(database);
        return false;
    }
    const int active_insert = VerifyActiveParticipantInsert(
        database, root.tournament_id, target.charac_no);
    if (active_insert < 0) {
        Rollback(database);
        return false;
    }
    if (active_insert == 0) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (!RefreshCounts(database, root.tournament_id) ||
        !InvalidateBracket(database, root.tournament_id) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.command_result = 4u;
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool HandleCancel(MySQL* database,
                  const Message& request,
                  Message& response) {
    if (!Begin(database)) {
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!LockCurrentRoot(database, request.charac_no, request.expires_epoch,
                         root, response)) {
        Rollback(database);
        return false;
    }
    if (response.error_code != 0u) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (!IsOrganizer(root, request.charac_no)) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    uint32_t live_matches = 0u;
    if (!database->set_query(
            "seLect count(*) from %s where tournament_id=%u and state=3",
            kMatchTable, static_cast<unsigned int>(root.tournament_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() || !database->get_uint(0, live_matches)) {
        Rollback(database);
        return false;
    }
    if (live_matches != 0u) {
        Rollback(database);
        Fail(response, ERROR_STATE);
        return true;
    }
    if (!database->set_query(
            "upDate %s set state=6,finished_epoch=%u,bracket_ready=0,"
            "winner_team_id=0,version=version+1 where tournament_id=%u "
            "and state in (3,4)", kRootTable,
            static_cast<unsigned int>(request.expires_epoch),
            static_cast<unsigned int>(root.tournament_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate %s set approval=3,active_charac_no=null "
            "where tournament_id=%u and approval in (0,1)",
            kParticipantTable,
            static_cast<unsigned int>(root.tournament_id)) ||
        !database->exec(true) ||
        !database->set_query(
            "upDate %s set state=5,room_token=null,room_lease_epoch=null "
            "where tournament_id=%u and state<>4", kMatchTable,
            static_cast<unsigned int>(root.tournament_id)) ||
        !database->exec(true) || !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.command_result = 5u;
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool HandleRemoveMember(MySQL* database,
                        const Message& request,
                        Message& response) {
    if (!Begin(database)) {
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!LockCurrentRoot(database, request.charac_no, request.expires_epoch,
                         root, response)) {
        Rollback(database);
        return false;
    }
    if (response.error_code != 0u) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (root.state != TOURNAMENT_REGISTRATION ||
        request.target_charac_no == root.organizer_charac_no) {
        Rollback(database);
        Fail(response, root.state != TOURNAMENT_REGISTRATION
                           ? ERROR_STATE : ERROR_PERMISSION);
        return true;
    }
    uint32_t actor_team = 0u;
    uint32_t actor_role = 0u;
    uint32_t target_team = 0u;
    uint32_t target_role = 0u;
    if (!LoadActorTeamAndRole(database, root.tournament_id,
                              request.charac_no, actor_team, actor_role)) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (!LoadActorTeamAndRole(database, root.tournament_id,
                              request.target_charac_no,
                              target_team, target_role)) {
        Rollback(database);
        Fail(response, ERROR_PARTICIPANT_MISSING);
        return true;
    }
    if (!IsOrganizer(root, request.charac_no) &&
        (actor_role != PARTICIPANT_CAPTAIN || actor_team != target_team ||
         target_role == PARTICIPANT_CAPTAIN)) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (!database->set_query(
            "upDate %s set approval=3,active_charac_no=null,result=0 "
            "where tournament_id=%u and charac_no=%u and approval=1",
            kParticipantTable, static_cast<unsigned int>(root.tournament_id),
            static_cast<unsigned int>(request.target_charac_no)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u) {
        Rollback(database);
        return false;
    }
    if (target_role == PARTICIPANT_CAPTAIN) {
        uint32_t promoted = 0u;
        if (!database->set_query(
                "seLect charac_no from %s where tournament_id=%u "
                "and team_id=%u and approval=1 order by charac_no limit 1",
                kParticipantTable,
                static_cast<unsigned int>(root.tournament_id),
                static_cast<unsigned int>(target_team)) ||
            !database->exec(true)) {
            Rollback(database);
            return false;
        }
        if (database->get_n_rows() == 1 &&
            (!database->fetch() || !database->get_uint(0, promoted) ||
             !database->set_query(
                 "upDate %s set role=1 where tournament_id=%u "
                 "and charac_no=%u", kParticipantTable,
                 static_cast<unsigned int>(root.tournament_id),
                 static_cast<unsigned int>(promoted)) ||
             !database->exec(true) ||
             database->getAffectedRowCount() != 1u)) {
            Rollback(database);
            return false;
        }
    }
    if (!RefreshCounts(database, root.tournament_id) ||
        !InvalidateBracket(database, root.tournament_id) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.command_result = 4u;
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool LoadTeams(MySQL* database,
               uint32_t tournament_id,
               std::vector<TeamSeed>& teams) {
    teams.clear();
    if (!database ||
        !database->set_query(
            "seLect team_id,max(if(role in (1,2),charac_no,0)),"
            "min(seed) from %s where tournament_id=%u and approval=1 "
            "group by team_id order by team_id limit %u",
            kParticipantTable, static_cast<unsigned int>(tournament_id),
            static_cast<unsigned int>(kMaximumTeamCount + 1)) ||
        !database->exec(true) || database->get_n_rows() > kMaximumTeamCount) {
        return false;
    }
    const int rows = database->get_n_rows();
    for (int index = 0; index < rows; ++index) {
        TeamSeed team;
        if (!database->fetch() || !database->get_uint(0, team.team_id) ||
            !database->get_uint(1, team.captain_charac_no) ||
            !database->get_uint(2, team.seed) || team.team_id == 0u ||
            team.captain_charac_no == 0u || team.seed > 255u) {
            return false;
        }
        teams.push_back(team);
    }
    return true;
}

bool HandleSaveSeeds(MySQL* database,
                     const Message& request,
                     Message& response) {
    if (request.seed_count == 0u ||
        request.seed_count > kMaximumTeamCount || !Begin(database)) {
        if (request.seed_count == 0u ||
            request.seed_count > kMaximumTeamCount) {
            Fail(response, ERROR_BRACKET);
            return true;
        }
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!LockCurrentRoot(database, request.charac_no, request.expires_epoch,
                         root, response)) {
        Rollback(database);
        return false;
    }
    if (response.error_code != 0u) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (!IsOrganizer(root, request.charac_no)) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (root.state != TOURNAMENT_REGISTRATION) {
        Rollback(database);
        Fail(response, ERROR_STATE);
        return true;
    }
    std::vector<TeamSeed> teams;
    if (!LoadTeams(database, root.tournament_id, teams)) {
        Rollback(database);
        return false;
    }
    if (teams.size() != request.seed_count) {
        Rollback(database);
        Fail(response, ERROR_BRACKET);
        return true;
    }
    bool seen_seed[kMaximumTeamCount] = {};
    bool seen_team[kMaximumTeamCount] = {};
    for (uint8_t index = 0u; index < request.seed_count; ++index) {
        const uint32_t captain = request.seed_charac_no[index];
        const uint8_t seed = request.seed_value[index];
        bool found = false;
        if (captain == 0u || seed >= kMaximumTeamCount || seen_seed[seed]) {
            Rollback(database);
            Fail(response, ERROR_BRACKET);
            return true;
        }
        seen_seed[seed] = true;
        for (size_t team_index = 0u; team_index < teams.size(); ++team_index) {
            if (teams[team_index].captain_charac_no == captain) {
                if (seen_team[team_index]) {
                    Rollback(database);
                    Fail(response, ERROR_BRACKET);
                    return true;
                }
                seen_team[team_index] = true;
                if (!database->set_query(
                        "upDate %s set seed=%u where tournament_id=%u "
                        "and team_id=%u and approval=1", kParticipantTable,
                        static_cast<unsigned int>(seed),
                        static_cast<unsigned int>(root.tournament_id),
                        static_cast<unsigned int>(teams[team_index].team_id)) ||
                    !database->exec(true)) {
                    Rollback(database);
                    return false;
                }
                found = true;
                break;
            }
        }
        if (!found) {
            Rollback(database);
            Fail(response, ERROR_BRACKET);
            return true;
        }
    }
    if (!InvalidateBracket(database, root.tournament_id) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.command_result = 4u;
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool HandleSetSeed(MySQL* database,
                   const Message& request,
                   Message& response) {
    if (request.argument != 255u && request.argument >= kMaximumTeamCount) {
        Fail(response, ERROR_BRACKET);
        return true;
    }
    if (!Begin(database)) {
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!LockCurrentRoot(database, request.charac_no, request.expires_epoch,
                         root, response)) {
        Rollback(database);
        return false;
    }
    if (response.error_code != 0u) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (!IsOrganizer(root, request.charac_no)) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (root.state != TOURNAMENT_REGISTRATION) {
        Rollback(database);
        Fail(response, ERROR_STATE);
        return true;
    }
    uint32_t team_id = 0u;
    uint32_t ignored_role = 0u;
    if (!LoadActorTeamAndRole(database, root.tournament_id,
                              request.target_charac_no,
                              team_id, ignored_role)) {
        Rollback(database);
        Fail(response, ERROR_PARTICIPANT_MISSING);
        return true;
    }
    if (request.argument != 255u) {
        uint32_t collision = 0u;
        if (!database->set_query(
                "seLect count(*) from %s where tournament_id=%u "
                "and seed=%u and team_id<>%u and approval=1",
                kParticipantTable,
                static_cast<unsigned int>(root.tournament_id),
                static_cast<unsigned int>(request.argument),
                static_cast<unsigned int>(team_id)) ||
            !database->exec(true) || database->get_n_rows() != 1 ||
            !database->fetch() || !database->get_uint(0, collision)) {
            Rollback(database);
            return false;
        }
        if (collision != 0u) {
            Rollback(database);
            Fail(response, ERROR_BRACKET);
            return true;
        }
    }
    if (!database->set_query(
            "upDate %s set seed=%u where tournament_id=%u and team_id=%u "
            "and approval=1", kParticipantTable,
            static_cast<unsigned int>(request.argument),
            static_cast<unsigned int>(root.tournament_id),
            static_cast<unsigned int>(team_id)) ||
        !database->exec(true) ||
        !InvalidateBracket(database, root.tournament_id) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.command_result = 4u;
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool TeamSeedLess(const TeamSeed& lhs, const TeamSeed& rhs) {
    if (lhs.seed != rhs.seed) {
        return lhs.seed < rhs.seed;
    }
    return lhs.team_id < rhs.team_id;
}

bool LastInsertId(MySQL* database, uint32_t& value) {
    unsigned long long inserted = 0u;
    if (!database || !database->set_query("seLect last_insert_id()") ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() || !database->get_ulonglong(0, inserted) ||
        inserted == 0u || inserted > 0xffffffffULL) {
        return false;
    }
    value = static_cast<uint32_t>(inserted);
    return true;
}

bool InsertBracketMatch(MySQL* database,
                        uint32_t tournament_id,
                        uint8_t match_index,
                        uint8_t round,
                        uint32_t left_child,
                        uint32_t right_child,
                        uint32_t left_team,
                        uint32_t right_team,
                        uint8_t state,
                        uint32_t winner,
                        BracketNode& node) {
    if (!database || match_index >= kMaximumMatchCount ||
        !database->set_query(
            "inSert into %s(tournament_id,match_index,round_no,"
            "parent_match_id,parent_side,left_child_match_id,"
            "right_child_match_id,left_team_id,right_team_id,winner_team_id,"
            "state,room_token,room_lease_epoch,result_nonce) values("
            "%u,%u,%u,0,0,%u,%u,%u,%u,%u,%u,null,null,0)", kMatchTable,
            static_cast<unsigned int>(tournament_id),
            static_cast<unsigned int>(match_index),
            static_cast<unsigned int>(round),
            static_cast<unsigned int>(left_child),
            static_cast<unsigned int>(right_child),
            static_cast<unsigned int>(left_team),
            static_cast<unsigned int>(right_team),
            static_cast<unsigned int>(winner),
            static_cast<unsigned int>(state)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !LastInsertId(database, node.match_id)) {
        return false;
    }
    node.winner_team_id = winner;
    node.match_index = match_index;
    node.round = round;
    return true;
}

bool BuildBracket(MySQL* database,
                  uint32_t tournament_id,
                  std::vector<TeamSeed>& teams) {
    if (teams.size() < kMinimumTeamCountToStart ||
        teams.size() > kMaximumTeamCount) {
        return false;
    }
    std::sort(teams.begin(), teams.end(), TeamSeedLess);
    bool seen[kMaximumTeamCount] = {};
    for (size_t index = 0u; index < teams.size(); ++index) {
        if (teams[index].seed >= kMaximumTeamCount ||
            seen[teams[index].seed]) {
            return false;
        }
        seen[teams[index].seed] = true;
    }
    if (!database->set_query("deLete from %s where tournament_id=%u",
                             kMatchTable,
                             static_cast<unsigned int>(tournament_id)) ||
        !database->exec(true)) {
        return false;
    }

    size_t bracket_size = 1u;
    while (bracket_size < teams.size()) {
        bracket_size <<= 1u;
    }
    const size_t leaf_count = bracket_size / 2u;
    std::vector<BracketNode> level;
    level.reserve(leaf_count);
    uint8_t match_index = 0u;
    for (size_t index = 0u; index < leaf_count; ++index) {
        const uint32_t left = teams[index].team_id;
        const size_t right_index = index + leaf_count;
        const uint32_t right = right_index < teams.size()
                                   ? teams[right_index].team_id : 0u;
        const uint8_t state = right == 0u ? MATCH_DONE : MATCH_READY;
        const uint32_t winner = right == 0u ? left : 0u;
        BracketNode node;
        if (!InsertBracketMatch(database, tournament_id, match_index++, 0u,
                                0u, 0u, left, right, state, winner, node)) {
            return false;
        }
        level.push_back(node);
    }

    uint8_t round = 1u;
    while (level.size() > 1u) {
        std::vector<BracketNode> next;
        next.reserve(level.size() / 2u);
        for (size_t index = 0u; index < level.size(); index += 2u) {
            const BracketNode& left_child = level[index];
            const BracketNode& right_child = level[index + 1u];
            const uint32_t left = left_child.winner_team_id;
            const uint32_t right = right_child.winner_team_id;
            const uint8_t state = left != 0u && right != 0u
                                      ? MATCH_READY : MATCH_WAITING;
            BracketNode parent;
            if (!InsertBracketMatch(database, tournament_id, match_index++,
                                    round, left_child.match_id,
                                    right_child.match_id, left, right,
                                    state, 0u, parent) ||
                !database->set_query(
                    "upDate %s set parent_match_id=%u,parent_side=1 "
                    "where match_id=%u", kMatchTable,
                    static_cast<unsigned int>(parent.match_id),
                    static_cast<unsigned int>(left_child.match_id)) ||
                !database->exec(true) ||
                database->getAffectedRowCount() != 1u ||
                !database->set_query(
                    "upDate %s set parent_match_id=%u,parent_side=2 "
                    "where match_id=%u", kMatchTable,
                    static_cast<unsigned int>(parent.match_id),
                    static_cast<unsigned int>(right_child.match_id)) ||
                !database->exec(true) ||
                database->getAffectedRowCount() != 1u) {
                return false;
            }
            next.push_back(parent);
        }
        level.swap(next);
        ++round;
    }
    return match_index == bracket_size - 1u &&
           database->set_query(
               "upDate %s set bracket_ready=1,version=version+1 "
               "where tournament_id=%u", kRootTable,
               static_cast<unsigned int>(tournament_id)) &&
           database->exec(true) && database->getAffectedRowCount() == 1u;
}

bool HandleFinalizeBracket(MySQL* database,
                           const Message& request,
                           Message& response) {
    if (!Begin(database)) {
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!LockCurrentRoot(database, request.charac_no, request.expires_epoch,
                         root, response)) {
        Rollback(database);
        return false;
    }
    if (response.error_code != 0u) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (!IsOrganizer(root, request.charac_no)) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (root.state != TOURNAMENT_REGISTRATION) {
        Rollback(database);
        Fail(response, ERROR_STATE);
        return true;
    }
    std::vector<TeamSeed> teams;
    if (!LoadTeams(database, root.tournament_id, teams)) {
        Rollback(database);
        return false;
    }
    if (teams.size() < kMinimumTeamCountToStart) {
        Rollback(database);
        Fail(response, ERROR_NEED_THREE_TEAMS);
        return true;
    }
    bool seen_seed[kMaximumTeamCount] = {};
    for (size_t index = 0u; index < teams.size(); ++index) {
        if (teams[index].seed >= kMaximumTeamCount ||
            seen_seed[teams[index].seed]) {
            Rollback(database);
            Fail(response, ERROR_BRACKET);
            return true;
        }
        seen_seed[teams[index].seed] = true;
    }
    if (!BuildBracket(database, root.tournament_id, teams)) {
        Rollback(database);
        return false;
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    response.command_result = 6u;
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool HandleStart(MySQL* database,
                 const Message& request,
                 Message& response) {
    if (!Begin(database)) {
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!LockCurrentRoot(database, request.charac_no, request.expires_epoch,
                         root, response)) {
        Rollback(database);
        return false;
    }
    if (response.error_code != 0u) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (!IsOrganizer(root, request.charac_no)) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if (root.state != TOURNAMENT_REGISTRATION) {
        Rollback(database);
        Fail(response, ERROR_STATE);
        return true;
    }
    if (root.team_count < kMinimumTeamCountToStart) {
        Rollback(database);
        Fail(response, ERROR_NEED_THREE_TEAMS);
        return true;
    }
    if (!root.bracket_ready) {
        Rollback(database);
        Fail(response, ERROR_BRACKET);
        return true;
    }
    if (request.expires_epoch < root.scheduled_epoch) {
        Rollback(database);
        Fail(response, ERROR_SCHEDULE);
        return true;
    }
    if (!database->set_query(
            "upDate %s set state=4,version=version+1 "
            "where tournament_id=%u and state=3 and bracket_ready=1",
            kRootTable, static_cast<unsigned int>(root.tournament_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.command_result = 4u;
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool LoadMatchForUpdate(MySQL* database,
                        uint32_t tournament_id,
                        uint32_t match_id,
                        int match_index,
                        MatchRecord& match,
                        uint32_t& lease_epoch) {
    std::memset(&match, 0, sizeof(match));
    lease_epoch = 0u;
    const bool query = match_id != 0u
        ? database->set_query(
              "seLect tournament_id,match_id,parent_match_id,"
              "left_child_match_id,right_child_match_id,left_team_id,"
              "right_team_id,winner_team_id,ifnull(room_token,0),"
              "match_index,round_no,state,parent_side,"
              "ifnull(room_lease_epoch,0) from %s "
              "where tournament_id=%u and match_id=%u for update",
              kMatchTable, static_cast<unsigned int>(tournament_id),
              static_cast<unsigned int>(match_id))
        : database->set_query(
              "seLect tournament_id,match_id,parent_match_id,"
              "left_child_match_id,right_child_match_id,left_team_id,"
              "right_team_id,winner_team_id,ifnull(room_token,0),"
              "match_index,round_no,state,parent_side,"
              "ifnull(room_lease_epoch,0) from %s "
              "where tournament_id=%u and match_index=%u for update",
              kMatchTable, static_cast<unsigned int>(tournament_id),
              static_cast<unsigned int>(match_index));
    if (!query || !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    uint32_t index = 0u;
    uint32_t round = 0u;
    uint32_t state = 0u;
    uint32_t parent_side = 0u;
    if (!database->get_uint(0, match.tournament_id) ||
        !database->get_uint(1, match.match_id) ||
        !database->get_uint(2, match.parent_match_id) ||
        !database->get_uint(3, match.left_child_match_id) ||
        !database->get_uint(4, match.right_child_match_id) ||
        !database->get_uint(5, match.left_team_id) ||
        !database->get_uint(6, match.right_team_id) ||
        !database->get_uint(7, match.winner_team_id) ||
        !database->get_uint(8, match.room_token) ||
        !database->get_uint(9, index) ||
        !database->get_uint(10, round) ||
        !database->get_uint(11, state) ||
        !database->get_uint(12, parent_side) ||
        !database->get_uint(13, lease_epoch) || index > 62u ||
        round > 6u || state > MATCH_CANCELLED || parent_side > 2u) {
        return false;
    }
    match.match_index = static_cast<uint8_t>(index);
    match.round = static_cast<uint8_t>(round);
    match.state = static_cast<uint8_t>(state);
    match.parent_side = static_cast<uint8_t>(parent_side);
    return true;
}

bool ActorTeam(MySQL* database,
               uint32_t tournament_id,
               uint32_t charac_no,
               uint32_t& team_id) {
    uint32_t role = 0u;
    return LoadActorTeamAndRole(database, tournament_id, charac_no,
                                team_id, role);
}

bool HandleReserveRoom(MySQL* database,
                       const Message& request,
                       Message& response) {
    if (request.argument >= kMaximumMatchCount || !Begin(database)) {
        if (request.argument >= kMaximumMatchCount) {
            Fail(response, ERROR_BRACKET);
            return true;
        }
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!LockCurrentRoot(database, request.charac_no, request.expires_epoch,
                         root, response)) {
        Rollback(database);
        return false;
    }
    if (response.error_code != 0u) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (root.state != TOURNAMENT_RUNNING) {
        Rollback(database);
        Fail(response, ERROR_STATE);
        return true;
    }
    uint32_t actor_team = 0u;
    if (!ActorTeam(database, root.tournament_id, request.charac_no,
                   actor_team)) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    MatchRecord match;
    uint32_t lease_epoch = 0u;
    if (!LoadMatchForUpdate(database, root.tournament_id, 0u,
                            request.argument, match, lease_epoch)) {
        Rollback(database);
        Fail(response, ERROR_ROOM_MISSING);
        return true;
    }
    if (match.winner_team_id != 0u || match.state == MATCH_DONE) {
        Rollback(database);
        Fail(response, ERROR_STATE);
        return true;
    }
    if (actor_team != match.left_team_id && actor_team != match.right_team_id) {
        Rollback(database);
        Fail(response, ERROR_PERMISSION);
        return true;
    }
    if ((match.state == MATCH_RESERVED || match.state == MATCH_RUNNING) &&
        lease_epoch > request.expires_epoch && match.room_token != 0u) {
        response.match_id = match.match_id;
        response.room_token = match.room_token;
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        response.command_result = 4u;
        return FillTournamentSnapshot(database, root.tournament_id, response);
    }
    if ((match.state == MATCH_RESERVED || match.state == MATCH_RUNNING) &&
        lease_epoch <= request.expires_epoch) {
        if (!database->set_query(
                "upDate %s set state=1,room_token=null,"
                "room_lease_epoch=null where match_id=%u",
                kMatchTable, static_cast<unsigned int>(match.match_id)) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u) {
            Rollback(database);
            return false;
        }
        match.state = MATCH_READY;
    }
    if (match.state != MATCH_READY) {
        Rollback(database);
        Fail(response, ERROR_BRACKET);
        return true;
    }
    // match_id is an InnoDB AUTO_INCREMENT identity and is therefore a
    // durable, restart-safe room nonce.  Process-local counters can collide
    // after a restart or across tournament-channel workers.
    const uint32_t room_token = match.match_id;
    const uint32_t lease = request.expires_epoch + kRoomLeaseSeconds;
    if (!database->set_query(
            "upDate %s set state=2,room_token=%u,room_lease_epoch=%u "
            "where match_id=%u and state=1", kMatchTable,
            static_cast<unsigned int>(room_token),
            static_cast<unsigned int>(lease),
            static_cast<unsigned int>(match.match_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate %s set version=version+1 where tournament_id=%u",
            kRootTable, static_cast<unsigned int>(root.tournament_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.match_id = match.match_id;
    response.room_token = room_token;
    response.command_result = 4u;
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool HandleCommitOrReleaseRoom(MySQL* database,
                               const Message& request,
                               Message& response,
                               bool commit_room) {
    if (!Begin(database)) {
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!SelectRoot(database, request.tournament_id, true, root)) {
        Rollback(database);
        Fail(response, ERROR_TOURNAMENT_MISSING);
        return true;
    }
    bool expired = false;
    if (!ExpireRootLocked(database, root, request.expires_epoch, expired)) {
        Rollback(database);
        return false;
    }
    if (expired) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        Fail(response, ERROR_ENDED);
        // Background room owners have no CUser reply path.  Preserve the
        // expired root snapshot so NOTI604/subtype2 is still broadcast
        // immediately instead of waiting for the next minute sweep.
        return FillTournamentSnapshot(database, root.tournament_id,
                                      response);
    }
    MatchRecord match;
    uint32_t lease_epoch = 0u;
    if (!LoadMatchForUpdate(database, request.tournament_id,
                            request.match_id, -1, match, lease_epoch) ||
        match.room_token != request.room_token) {
        Rollback(database);
        Fail(response, ERROR_ROOM_MISSING);
        return true;
    }
    if (commit_room) {
        if (match.state != MATCH_RESERVED ||
            !database->set_query(
                "upDate %s set state=3,room_lease_epoch=%u "
                "where match_id=%u and state=2 and room_token=%u",
                kMatchTable, static_cast<unsigned int>(root.expires_epoch),
                static_cast<unsigned int>(match.match_id),
                static_cast<unsigned int>(request.room_token)) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u) {
            Rollback(database);
            Fail(response, ERROR_STATE);
            return true;
        }
    } else {
        if (match.state != MATCH_RESERVED && match.state != MATCH_RUNNING) {
            Rollback(database);
            Fail(response, ERROR_STATE);
            return true;
        }
        if (!database->set_query(
                "upDate %s set state=1,room_token=null,"
                "room_lease_epoch=null where match_id=%u and room_token=%u",
                kMatchTable, static_cast<unsigned int>(match.match_id),
                static_cast<unsigned int>(request.room_token)) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u) {
            Rollback(database);
            return false;
        }
    }
    if (!database->set_query(
            "upDate %s set version=version+1 where tournament_id=%u",
            kRootTable, static_cast<unsigned int>(root.tournament_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool AdvanceWinner(MySQL* database,
                   const RootRecord& root,
                   const MatchRecord& match,
                   uint32_t winner_team,
                   uint32_t now_epoch) {
    if (match.parent_match_id == 0u) {
        return database->set_query(
                   "upDate %s set state=5,finished_epoch=%u,"
                   "winner_team_id=%u,version=version+1 "
                   "where tournament_id=%u and state=4", kRootTable,
                   static_cast<unsigned int>(now_epoch),
                   static_cast<unsigned int>(winner_team),
                   static_cast<unsigned int>(root.tournament_id)) &&
               database->exec(true) &&
               database->getAffectedRowCount() == 1u &&
               database->set_query(
                   "upDate %s set approval=2,active_charac_no=null,"
                   "result=if(team_id=%u,1,2) where tournament_id=%u "
                   "and approval=1", kParticipantTable,
                   static_cast<unsigned int>(winner_team),
                   static_cast<unsigned int>(root.tournament_id)) &&
               database->exec(true);
    }
    const char* field = match.parent_side == 1u
                            ? "left_team_id" : "right_team_id";
    if (match.parent_side < 1u || match.parent_side > 2u ||
        !database->set_query(
            "upDate %s set %s=%u where match_id=%u and state=0",
            kMatchTable, field, static_cast<unsigned int>(winner_team),
            static_cast<unsigned int>(match.parent_match_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate %s set state=1 where match_id=%u and state=0 "
            "and left_team_id<>0 and right_team_id<>0", kMatchTable,
            static_cast<unsigned int>(match.parent_match_id)) ||
        !database->exec(true)) {
        return false;
    }
    return database->set_query(
               "upDate %s set version=version+1 where tournament_id=%u",
               kRootTable, static_cast<unsigned int>(root.tournament_id)) &&
           database->exec(true) && database->getAffectedRowCount() == 1u;
}

bool HandleMatchResult(MySQL* database,
                       const Message& request,
                       Message& response,
                       bool draw) {
    if (!Begin(database)) {
        return false;
    }
    RootRecord root;
    std::memset(&root, 0, sizeof(root));
    if (!SelectRoot(database, request.tournament_id, true, root)) {
        Rollback(database);
        Fail(response, ERROR_TOURNAMENT_MISSING);
        return true;
    }
    bool expired = false;
    if (!ExpireRootLocked(database, root, request.expires_epoch, expired)) {
        Rollback(database);
        return false;
    }
    if (expired) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        Fail(response, ERROR_ENDED);
        return FillTournamentSnapshot(database, root.tournament_id,
                                      response);
    }
    MatchRecord match;
    uint32_t lease_epoch = 0u;
    if (!LoadMatchForUpdate(database, request.tournament_id,
                            request.match_id, -1, match, lease_epoch)) {
        Rollback(database);
        Fail(response, ERROR_ROOM_MISSING);
        return true;
    }
    uint32_t result_nonce = 0u;
    if (!database->set_query(
            "seLect result_nonce from %s where match_id=%u",
            kMatchTable, static_cast<unsigned int>(match.match_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() || !database->get_uint(0, result_nonce)) {
        Rollback(database);
        return false;
    }
    if (match.state == MATCH_DONE && result_nonce == request.room_token) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return FillTournamentSnapshot(database, root.tournament_id, response);
    }
    if (root.state != TOURNAMENT_RUNNING ||
        (match.state != MATCH_RESERVED && match.state != MATCH_RUNNING) ||
        match.room_token != request.room_token) {
        Rollback(database);
        Fail(response, ERROR_STATE);
        return true;
    }
    if (draw) {
        if (!database->set_query(
                "upDate %s set state=1,room_token=null,"
                "room_lease_epoch=null where match_id=%u and room_token=%u",
                kMatchTable, static_cast<unsigned int>(match.match_id),
                static_cast<unsigned int>(request.room_token)) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u ||
            !database->set_query(
                "upDate %s set version=version+1 where tournament_id=%u",
                kRootTable, static_cast<unsigned int>(root.tournament_id)) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u || !Commit(database)) {
            Rollback(database);
            return false;
        }
        return FillTournamentSnapshot(database, root.tournament_id, response);
    }
    if (request.argument != 1u && request.argument != 2u) {
        Rollback(database);
        Fail(response, ERROR_BRACKET);
        return true;
    }
    const uint32_t winner = request.argument == 1u
                                ? match.left_team_id : match.right_team_id;
    const uint32_t loser = request.argument == 1u
                               ? match.right_team_id : match.left_team_id;
    if (winner == 0u || loser == 0u ||
        !database->set_query(
            "upDate %s set state=4,winner_team_id=%u,result_nonce=%u,"
            "room_token=null,room_lease_epoch=null where match_id=%u "
            "and room_token=%u and state in (2,3)", kMatchTable,
            static_cast<unsigned int>(winner),
            static_cast<unsigned int>(request.room_token),
            static_cast<unsigned int>(match.match_id),
            static_cast<unsigned int>(request.room_token)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate %s set result=2 where tournament_id=%u and team_id=%u "
            "and approval=1", kParticipantTable,
            static_cast<unsigned int>(root.tournament_id),
            static_cast<unsigned int>(loser)) ||
        !database->exec(true) ||
        !AdvanceWinner(database, root, match, winner,
                       request.expires_epoch) || !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.match_id = match.match_id;
    return FillTournamentSnapshot(database, root.tournament_id, response);
}

bool HandleExpirySweep(MySQL* database,
                       const Message& request,
                       Message& response) {
    if (!Begin(database) ||
        !database->set_query(
            "seLect tournament_id from %s where state in (3,4,5) "
            "and expires_epoch<=%u order by expires_epoch limit %u "
            "for update", kRootTable,
            static_cast<unsigned int>(request.expires_epoch),
            static_cast<unsigned int>(kMaximumTournamentCount)) ||
        !database->exec(true)) {
        Rollback(database);
        return false;
    }
    std::vector<uint32_t> ids;
    const int rows = database->get_n_rows();
    ids.reserve(static_cast<size_t>(rows));
    for (int index = 0; index < rows; ++index) {
        uint32_t id = 0u;
        if (!database->fetch() || !database->get_uint(0, id) || id == 0u) {
            Rollback(database);
            return false;
        }
        ids.push_back(id);
    }
    for (size_t index = 0u; index < ids.size(); ++index) {
        if (!database->set_query(
                "upDate %s set state=6,finished_epoch=%u,bracket_ready=0,"
                "winner_team_id=0,version=version+1 where tournament_id=%u "
                "and state in (3,4,5)", kRootTable,
                static_cast<unsigned int>(request.expires_epoch),
                static_cast<unsigned int>(ids[index])) ||
            !database->exec(true) ||
            database->getAffectedRowCount() != 1u ||
            !database->set_query(
                "upDate %s set approval=3,active_charac_no=null "
                "where tournament_id=%u and approval in (0,1)",
                kParticipantTable, static_cast<unsigned int>(ids[index])) ||
            !database->exec(true) ||
            !database->set_query(
                "upDate %s set state=5,room_token=null,"
                "room_lease_epoch=null where tournament_id=%u and state<>4",
                kMatchTable, static_cast<unsigned int>(ids[index])) ||
            !database->exec(true)) {
            Rollback(database);
            return false;
        }
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    ClearSnapshots(response);
    for (size_t index = 0u; index < ids.size(); ++index) {
        RootRecord root;
        std::memset(&root, 0, sizeof(root));
        if (!SelectRoot(database, ids[index], false, root)) {
            return false;
        }
        response.roots[index] = root;
    }
    response.root_count = static_cast<uint8_t>(ids.size());
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

DBPvpTournament::DBPvpTournament()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBPvpTournament::~DBPvpTournament() {}

bool DBPvpTournament::QueueRequest(int user_idx,
                                       const Message& request) {
    if (!IsValidRequest(request, user_idx)) {
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

int DBPvpTournament::dispatch(int signal,
                                  int user_idx,
                                  Stream* stream) {
    (void)signal;
    if (!stream || stream->length() != static_cast<int>(sizeof(Message))) {
        return 0;
    }
    Message request;
    if (stream->get_binary(&request, static_cast<int>(sizeof(request))) != 1 ||
        !IsValidRequest(request, user_idx)) {
        return 0;
    }
    Message response = request;
    ClearSnapshots(response);
    response.error_code = 0u;
    response.command_result = 0u;

    MySQL* const database = GetGameDatabase(request.server_group);
    bool handled = false;
    if (database) {
        switch (request.operation) {
            case OPERATION_LOAD_REGISTRY:
                handled = HandleLoad(database, request, response);
                break;
            case OPERATION_OPEN_DETAIL:
                handled = HandleOpen(database, request, response);
                break;
            case OPERATION_CREATE:
                handled = HandleCreate(database, request, response);
                break;
            case OPERATION_JOIN:
                handled = HandleJoin(database, request, response);
                break;
            case OPERATION_LEAVE:
                handled = HandleLeave(database, request, response);
                break;
            case OPERATION_INVITE_MEMBER:
                handled = HandleInvite(database, request, response);
                break;
            case OPERATION_CANCEL:
                handled = HandleCancel(database, request, response);
                break;
            case OPERATION_SAVE_SEEDS:
                handled = HandleSaveSeeds(database, request, response);
                break;
            case OPERATION_START:
                handled = HandleStart(database, request, response);
                break;
            case OPERATION_RESERVE_ROOM:
                handled = HandleReserveRoom(database, request, response);
                break;
            case OPERATION_REMOVE_MEMBER:
                handled = HandleRemoveMember(database, request, response);
                break;
            case OPERATION_SET_SEED:
                handled = HandleSetSeed(database, request, response);
                break;
            case OPERATION_FINALIZE_BRACKET:
                handled = HandleFinalizeBracket(database, request, response);
                break;
            case OPERATION_COMMIT_ROOM:
                handled = HandleCommitOrReleaseRoom(database, request,
                                                    response, true);
                break;
            case OPERATION_RELEASE_ROOM:
                handled = HandleCommitOrReleaseRoom(database, request,
                                                    response, false);
                break;
            case OPERATION_MATCH_RESULT:
                handled = HandleMatchResult(database, request, response,
                                            false);
                break;
            case OPERATION_MATCH_DRAW:
                handled = HandleMatchResult(database, request, response,
                                            true);
                break;
            case OPERATION_EXPIRE_SWEEP:
                handled = HandleExpirySweep(database, request, response);
                break;
            default:
                break;
        }
    }
    if (!handled) {
        Fail(response, ERROR_DATABASE);
    } else if (response.error_code == 0u &&
               IsClientOperation(response.operation) &&
               response.command_result == 0u) {
        response.command_result = ResultForOperation(response.operation);
    }
    return SendResult(request.reply_user_idx, response) ? 1 : 0;
}

}  // namespace pvp_tournament_0828
