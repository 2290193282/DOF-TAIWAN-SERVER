#ifndef OLDSRV_OLD_GAME_PVP_TOURNAMENT_PVP_TOURNAMENT_TYPES_H_
#define OLDSRV_OLD_GAME_PVP_TOURNAMENT_PVP_TOURNAMENT_TYPES_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"

#include <stdint.h>

namespace pvp_tournament {

enum {
    // Taiwan outer signal 846, feature-owned inner route.  Shared enum/table
    // registration is intentionally left to the root integration change.
    kTaiwanInnerCode = TaiwanInternalPack::PVP_TOURNAMENT,
    kMaximumServerGroupCount = 18,
    kMaximumTournamentCount = 64,
    kMaximumTeamCount = 64,
    kMaximumParticipantCount = 64,
    kMaximumTeamMemberCount = 4,
    kMaximumMatchCount = 63,
    kMinimumTeamCountToStart = 3,
    kTournamentLifetimeSeconds = 72 * 60 * 60,
    kRoomLeaseSeconds = 90,
    kTitleCapacity = 33,
    kPasswordCapacity = 9,
    kCharacterNameCapacity = 30,
    kModeCapacity = 5,
};

enum Operation {
    OPERATION_LOAD_REGISTRY = 1,
    OPERATION_OPEN_DETAIL = 2,
    OPERATION_CREATE = 3,
    OPERATION_JOIN = 4,
    OPERATION_LEAVE = 5,
    OPERATION_INVITE_MEMBER = 6,
    OPERATION_CANCEL = 7,
    OPERATION_SAVE_SEEDS = 8,
    OPERATION_START = 9,
    OPERATION_RESERVE_ROOM = 10,
    OPERATION_REMOVE_MEMBER = 11,
    OPERATION_SET_SEED = 12,
    OPERATION_FINALIZE_BRACKET = 13,
    OPERATION_COMMIT_ROOM = 14,
    OPERATION_RELEASE_ROOM = 15,
    OPERATION_MATCH_RESULT = 16,
    OPERATION_MATCH_DRAW = 17,
    OPERATION_EXPIRE_SWEEP = 18,
};

enum CommandKind {
    COMMAND_NONE = 0,
    COMMAND_544 = 1,
    COMMAND_545 = 2,
};

enum TournamentState {
    // The 0828 client has explicit UI modes for 3, then 4/5.
    TOURNAMENT_REGISTRATION = 3,
    TOURNAMENT_RUNNING = 4,
    TOURNAMENT_FINISHED = 5,
    TOURNAMENT_CANCELLED_OR_EXPIRED = 6,
};

enum ParticipantApproval {
    PARTICIPANT_PENDING = 0,
    PARTICIPANT_ACTIVE = 1,
    PARTICIPANT_ELIMINATED = 2,
    PARTICIPANT_REMOVED = 3,
};

enum ParticipantRole {
    PARTICIPANT_MEMBER = 0,
    PARTICIPANT_CAPTAIN = 1,
    PARTICIPANT_ORGANIZER = 2,
};

enum MatchState {
    MATCH_WAITING = 0,
    MATCH_READY = 1,
    MATCH_RESERVED = 2,
    MATCH_RUNNING = 3,
    MATCH_DONE = 4,
    MATCH_CANCELLED = 5,
};

enum ErrorCode {
    ERROR_GENERIC = 1,
    ERROR_PLAYER_MISSING = 2,
    ERROR_PERMISSION = 3,
    ERROR_PARTICIPANT_MISSING = 4,
    ERROR_TOURNAMENT_MISSING = 5,
    ERROR_ALREADY_TEAMMATE = 6,
    ERROR_NEED_THREE_TEAMS = 7,
    ERROR_PASSWORD = 8,
    ERROR_NO_SLOT = 9,
    ERROR_ITEM = 10,
    ERROR_PVP_GRADE = 11,
    ERROR_COMMAND = 12,
    ERROR_PACKET = 13,
    ERROR_SCHEDULE = 14,
    ERROR_ROOM_MISSING = 15,
    ERROR_ENDED = 16,
    ERROR_STATE = 17,
    ERROR_DATABASE = 18,
    ERROR_LEVEL = 19,
    ERROR_BRACKET = 20,
};

// Fixed DB_Q/DISPATCH_Q records deliberately avoid pointers and native
// uint64_t alignment.  They are snapshots, not a public ABI extension.
struct RootRecord {
    uint32_t tournament_id;
    uint32_t scheduled_epoch;
    uint32_t expires_epoch;
    uint32_t organizer_charac_no;
    uint32_t organizer_account_id;
    uint32_t participant_count;
    uint32_t team_count;
    uint32_t version;
    uint32_t winner_team_id;
    uint16_t qualification;
    uint8_t state;
    uint8_t private_flag;
    uint8_t invite_only;
    uint8_t bracket_ready;
    uint8_t maximum_teams;
    uint8_t reserved;
    char title[kTitleCapacity];
    char organizer_name[kCharacterNameCapacity];
    char mode[kModeCapacity];
};

struct ParticipantRecord {
    uint32_t tournament_id;
    uint32_t team_id;
    uint32_t charac_no;
    uint32_t account_id;
    uint16_t online_unique_id;
    uint8_t job;
    uint8_t grow_type;
    uint8_t level;
    uint8_t pvp_grade;
    uint8_t role;
    uint8_t approval;
    uint8_t seed;
    uint8_t result;
    char name[kCharacterNameCapacity];
};

struct MatchRecord {
    uint32_t tournament_id;
    uint32_t match_id;
    uint32_t parent_match_id;
    uint32_t left_child_match_id;
    uint32_t right_child_match_id;
    uint32_t left_team_id;
    uint32_t right_team_id;
    uint32_t winner_team_id;
    uint32_t room_token;
    uint8_t match_index;
    uint8_t round;
    uint8_t state;
    uint8_t parent_side;
};

struct Message {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t request_token;
    uint32_t tournament_id;
    uint32_t target_charac_no;
    uint32_t scheduled_epoch;
    uint32_t expires_epoch;
    uint32_t room_token;
    uint32_t match_id;
    int32_t server_group;
    int32_t reply_user_idx;
    uint16_t qualification;
    uint8_t operation;
    uint8_t request_subtype;
    uint8_t command_kind;
    uint8_t command_result;
    uint8_t error_code;
    uint8_t argument;
    uint8_t flag_a;
    uint8_t flag_b;
    uint8_t seed_count;
    uint8_t root_count;
    uint8_t participant_count;
    uint8_t match_count;
    uint8_t reserved[2];
    char title[kTitleCapacity];
    char password[kPasswordCapacity];
    char actor_name[kCharacterNameCapacity];
    uint32_t seed_charac_no[kMaximumTeamCount];
    uint8_t seed_value[kMaximumTeamCount];
    RootRecord roots[kMaximumTournamentCount];
    ParticipantRecord participants[kMaximumParticipantCount];
    MatchRecord matches[kMaximumMatchCount];
};

static_assert(sizeof(RootRecord) == 112u,
              "0828 tournament root snapshot must remain 112 bytes");
static_assert(sizeof(ParticipantRecord) == 56u,
              "0828 tournament participant snapshot must remain 56 bytes");
static_assert(sizeof(MatchRecord) == 40u,
              "0828 tournament match snapshot must remain 40 bytes");
static_assert(sizeof(Message) == 13728u,
              "0828 tournament Taiwan payload must remain 13728 bytes");
static_assert(sizeof(Message) < 16384u,
              "0828 tournament payload must fit the default Stream pool");

}  // namespace pvp_tournament_0828

#endif  // OLDSRV_OLD_GAME_PVP_TOURNAMENT_PVP_TOURNAMENT_0828_TYPES_H_
