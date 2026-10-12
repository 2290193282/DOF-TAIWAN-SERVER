#include "old_game/dungeon/instance/time_spiral_runtime.h"

#include <map>

#include "common/manager/mt_rand.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/log_manager.h"
#include "old_data/PVF/etc/time_spiral_script.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"

namespace time_spiral {
namespace {

enum RunPhase {
    kDormant = 0,
    kArmed = 1,
    kSelected = 2,
};

struct FieldContext {
    FieldContext()
        : party(0), party_index(-1), dungeon_index(0), map_index(0),
          pos_x(-1), pos_y(-1) {}

    CParty* party;
    int party_index;
    int dungeon_index;
    int map_index;
    int pos_x;
    int pos_y;
};

struct UserContext : public FieldContext {
    UserContext()
        : FieldContext(), user(0), seat(-1), charac_no(0), unique_id(0) {}

    CUser* user;
    int seat;
    int charac_no;
    int unique_id;
};

struct RunState {
    RunState()
        : party(0), party_index(-1), dungeon_index(0), generation(0),
          owner_charac_no(0), owner_unique_id(0),
          phase(kDormant), source_map(0), source_x(-1), source_y(-1),
          normal_x(-1), normal_y(-1), selection(),
          ticket_consumed(false) {}

    CParty* party;
    int party_index;
    int dungeon_index;
    uint64_t generation;
    int owner_charac_no;
    int owner_unique_id;
    RunPhase phase;
    int source_map;
    int source_x;
    int source_y;
    int normal_x;
    int normal_y;
    RouteSelection selection;
    bool ticket_consumed;
};

typedef std::map<CParty*, RunState> RunTable;

Mutex& StateMutex() {
    static Mutex mutex;
    return mutex;
}

RunTable& Runs() {
    static RunTable runs;
    return runs;
}

CMTRand& RouteRandom() {
    static CMTRand random;
    return random;
}

uint64_t& NextGeneration() {
    static uint64_t generation = 0;
    return generation;
}

uint64_t AllocateGeneration() {
    uint64_t& generation = NextGeneration();
    ++generation;
    if (generation == 0) ++generation;
    return generation;
}

bool BuildFieldContext(CParty* party, FieldContext* output) {
    if (!party || !output || party->get_member_count() != 1) return false;

    CBattle_Field& field = party->battle_field();
    if (!field.get_dungeon() ||
        !IsDungeon(field.get_dungeon_index())) {
        return false;
    }

    FieldContext context;
    context.party = party;
    context.party_index = party->GetPartyIndex();
    context.dungeon_index = field.get_dungeon_index();
    context.map_index = field.GetCurrentMapIndex();
    context.pos_x = field.GetCurPosX();
    context.pos_y = field.GetCurPosY();
    if (context.party_index < 0 || context.map_index <= 0 ||
        context.pos_x < 0 || context.pos_y < 0) {
        return false;
    }
    *output = context;
    return true;
}

bool BuildUserContext(CUser* user,
                      bool require_active,
                      bool require_live,
                      UserContext* output) {
    if (!user || !output ||
        (require_active &&
         (user->get_state() != 5 || user->is_fighting() == 0))) {
        return false;
    }

    CParty* const party = user->GetParty();
    FieldContext field;
    if (!BuildFieldContext(party, &field)) return false;

    const int seat = party->get_party_seatno(user);
    if (seat < 0 || seat > 3 || party->_checkValidUser(seat) != 1 ||
        party->get_user(seat) != user ||
        (require_live && party->battle_data().member_live(seat) != 1u)) {
        return false;
    }

    UserContext context;
    static_cast<FieldContext&>(context) = field;
    context.user = user;
    context.seat = seat;
    context.charac_no = user->getCurCharacNo();
    context.unique_id = user->get_unique_id();
    if (context.charac_no <= 0 || context.unique_id <= 0) return false;
    *output = context;
    return true;
}

bool GetOnlyPartyUserIdentity(CParty* party,
                              int* charac_no,
                              int* unique_id) {
    if (!party || !charac_no || !unique_id ||
        party->get_member_count() != 1) {
        return false;
    }

    CUser* only_user = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        if (party->_checkValidUser(seat) != 1) continue;
        CUser* const member = party->get_user(seat);
        if (!member || only_user) return false;
        only_user = member;
    }
    if (!only_user) return false;
    *charac_no = only_user->getCurCharacNo();
    *unique_id = only_user->get_unique_id();
    return *charac_no > 0 && *unique_id > 0;
}

bool SameRun(const RunState& state, const FieldContext& context) {
    return state.party == context.party &&
           state.party_index == context.party_index &&
           state.dungeon_index == context.dungeon_index &&
           state.generation != 0;
}

bool SameSource(const RunState& state, const FieldContext& context) {
    return SameRun(state, context) && state.source_map == context.map_index &&
           state.source_x == context.pos_x && state.source_y == context.pos_y;
}

bool SameUser(const RunState& state, const UserContext& context) {
    return SameRun(state, context) &&
           state.owner_charac_no == context.charac_no &&
           state.owner_unique_id == context.unique_id;
}

bool SamePartyOwner(const RunState& state, CParty* party) {
    int charac_no = 0;
    int unique_id = 0;
    return GetOnlyPartyUserIdentity(party, &charac_no, &unique_id) &&
           state.owner_charac_no == charac_no &&
           state.owner_unique_id == unique_id;
}

void ClearSelection(RunState* state) {
    if (!state) return;
    state->phase = kDormant;
    state->source_map = 0;
    state->source_x = -1;
    state->source_y = -1;
    state->normal_x = -1;
    state->normal_y = -1;
    state->selection = RouteSelection();
}

}  // namespace

BreakTrapSelection::BreakTrapSelection()
    : target_x(0), target_y(0), result(0) {}

bool BeginRun(CParty* party, int dungeon_index) {
    if (!party) {
        if (dungeon_index == kDungeonIndex) {
            LogManager::logFormat(
                1,
                "time_spiral_diag.cpp",
                "bool time_spiral::BeginRun(CParty*, int)",
                0,
                "[TIME_SPIRAL][BEGIN_FAIL] reason=null_party ready=%d dungeon=%d",
                IsReady() ? 1 : 0,
                dungeon_index);
        }
        return false;
    }
    MutexGuard guard(&StateMutex());
    Runs().erase(party);
    int owner_charac_no = 0;
    int owner_unique_id = 0;
    if (!IsDungeon(dungeon_index) || party->get_member_count() != 1 ||
        party->battle_field().get_dungeon_index() != dungeon_index ||
        !party->battle_field().get_dungeon() ||
        !GetOnlyPartyUserIdentity(
            party, &owner_charac_no, &owner_unique_id)) {
        if (dungeon_index == kDungeonIndex) {
            LogManager::logFormat(
                1,
                "time_spiral_diag.cpp",
                "bool time_spiral::BeginRun(CParty*, int)",
                0,
                "[TIME_SPIRAL][BEGIN_FAIL] reason=context ready=%d party=%d requested_dungeon=%d field_dungeon=%d members=%d has_dungeon=%d owner_charac=%d owner_uid=%d",
                IsReady() ? 1 : 0,
                party->GetPartyIndex(),
                dungeon_index,
                party->battle_field().get_dungeon_index(),
                party->get_member_count(),
                party->battle_field().get_dungeon() ? 1 : 0,
                owner_charac_no,
                owner_unique_id);
        }
        return false;
    }

    RunState state;
    state.party = party;
    state.party_index = party->GetPartyIndex();
    state.dungeon_index = dungeon_index;
    state.generation = AllocateGeneration();
    state.owner_charac_no = owner_charac_no;
    state.owner_unique_id = owner_unique_id;
    if (state.party_index < 0) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::BeginRun(CParty*, int)",
            0,
            "[TIME_SPIRAL][BEGIN_FAIL] reason=party_index party=%d dungeon=%d owner_charac=%d owner_uid=%d",
            state.party_index,
            dungeon_index,
            owner_charac_no,
            owner_unique_id);
        return false;
    }
    Runs().insert(std::make_pair(party, state));
    LogManager::logFormat(
        1,
        "time_spiral_diag.cpp",
        "bool time_spiral::BeginRun(CParty*, int)",
        0,
        "[TIME_SPIRAL][BEGIN_OK] party=%d dungeon=%d generation=%llu owner_charac=%d owner_uid=%d",
        state.party_index,
        dungeon_index,
        static_cast<unsigned long long>(state.generation),
        owner_charac_no,
        owner_unique_id);
    return true;
}

void ResetParty(CParty* party) {
    if (!party) return;
    MutexGuard guard(&StateMutex());
    Runs().erase(party);
}

bool ArmAfterTrapDestroyed(CParty* party, int object_id) {
    if (object_id != GetTrapPassiveObjectId()) return false;
    FieldContext context;
    if (!BuildFieldContext(party, &context)) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::ArmAfterTrapDestroyed(CParty*, int)",
            0,
            "[TIME_SPIRAL][ARM_FAIL] reason=field_context ready=%d party=%d dungeon=%d map=%d pos=%d,%d members=%d object=%d",
            IsReady() ? 1 : 0,
            party ? party->GetPartyIndex() : -1,
            party ? party->battle_field().get_dungeon_index() : -1,
            party ? party->battle_field().GetCurrentMapIndex() : -1,
            party ? party->battle_field().GetCurPosX() : -1,
            party ? party->battle_field().GetCurPosY() : -1,
            party ? party->get_member_count() : -1,
            object_id);
        return false;
    }
    if (!IsTrapSource(context.map_index, context.pos_x, context.pos_y)) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::ArmAfterTrapDestroyed(CParty*, int)",
            0,
            "[TIME_SPIRAL][ARM_FAIL] reason=not_trap_source party=%d dungeon=%d map=%d pos=%d,%d object=%d",
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            object_id);
        return false;
    }

    int normal_x = -1;
    int normal_y = -1;
    if (!GetNormalExit(context.map_index, &normal_x, &normal_y)) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::ArmAfterTrapDestroyed(CParty*, int)",
            0,
            "[TIME_SPIRAL][ARM_FAIL] reason=normal_exit party=%d dungeon=%d map=%d pos=%d,%d object=%d",
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            object_id);
        return false;
    }

    MutexGuard guard(&StateMutex());
    RunTable::iterator found = Runs().find(party);
    if (found == Runs().end()) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::ArmAfterTrapDestroyed(CParty*, int)",
            0,
            "[TIME_SPIRAL][ARM_FAIL] reason=missing_run party=%d dungeon=%d map=%d pos=%d,%d object=%d",
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            object_id);
        return false;
    }
    const bool same_run = SameRun(found->second, context);
    const bool same_owner = same_run && SamePartyOwner(found->second, party);
    if (!same_run || !same_owner) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::ArmAfterTrapDestroyed(CParty*, int)",
            0,
            "[TIME_SPIRAL][ARM_FAIL] reason=%s party=%d dungeon=%d map=%d pos=%d,%d object=%d generation=%llu",
            same_run ? "owner" : "run_identity",
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            object_id,
            static_cast<unsigned long long>(found->second.generation));
        Runs().erase(found);
        return false;
    }

    RunState& state = found->second;
    if (state.phase == kArmed || state.phase == kSelected) {
        // Repeated DIE_MONSTER reports cannot re-arm or redraw a selection.
        const bool same_source = SameSource(state, context);
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::ArmAfterTrapDestroyed(CParty*, int)",
            0,
            "[TIME_SPIRAL][ARM_REPLAY] accepted=%d party=%d dungeon=%d map=%d pos=%d,%d object=%d phase=%d generation=%llu",
            same_source ? 1 : 0,
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            object_id,
            static_cast<int>(state.phase),
            static_cast<unsigned long long>(state.generation));
        return same_source;
    }

    state.phase = kArmed;
    state.source_map = context.map_index;
    state.source_x = context.pos_x;
    state.source_y = context.pos_y;
    state.normal_x = normal_x;
    state.normal_y = normal_y;
    state.selection = RouteSelection();
    LogManager::logFormat(
        1,
        "time_spiral_diag.cpp",
        "bool time_spiral::ArmAfterTrapDestroyed(CParty*, int)",
        0,
        "[TIME_SPIRAL][ARM_OK] party=%d dungeon=%d map=%d source=%d,%d normal=%d,%d object=%d generation=%llu",
        context.party_index,
        context.dungeon_index,
        context.map_index,
        context.pos_x,
        context.pos_y,
        normal_x,
        normal_y,
        object_id,
        static_cast<unsigned long long>(state.generation));
    return true;
}

bool SelectOrReplay(CUser* user, BreakTrapSelection* selection) {
    if (!selection) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::SelectOrReplay(CUser*, BreakTrapSelection*)",
            0,
            "[TIME_SPIRAL][SELECT_FAIL] reason=null_output uid=%d",
            user ? user->GetUID() : -1);
        return false;
    }
    UserContext context;
    if (!BuildUserContext(user, true, true, &context)) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::SelectOrReplay(CUser*, BreakTrapSelection*)",
            0,
            "[TIME_SPIRAL][SELECT_FAIL] reason=user_context uid=%d state=%d fighting=%d party=%d charac=%d unique=%d",
            user ? user->GetUID() : -1,
            user ? user->get_state() : -1,
            user ? user->is_fighting() : -1,
            user ? user->GetPartyIndex() : -1,
            user ? user->getCurCharacNo() : -1,
            user ? user->get_unique_id() : -1);
        return false;
    }

    MutexGuard guard(&StateMutex());
    RunTable::iterator found = Runs().find(context.party);
    if (found == Runs().end()) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::SelectOrReplay(CUser*, BreakTrapSelection*)",
            0,
            "[TIME_SPIRAL][SELECT_FAIL] reason=missing_run uid=%d party=%d dungeon=%d map=%d pos=%d,%d",
            user->GetUID(),
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y);
        return false;
    }
    const bool same_source = SameSource(found->second, context);
    const bool same_user = same_source && SameUser(found->second, context);
    if (!same_source || !same_user) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::SelectOrReplay(CUser*, BreakTrapSelection*)",
            0,
            "[TIME_SPIRAL][SELECT_FAIL] reason=%s uid=%d party=%d dungeon=%d map=%d pos=%d,%d phase=%d source_map=%d source=%d,%d generation=%llu",
            same_source ? "user_identity" : "source",
            user->GetUID(),
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            static_cast<int>(found->second.phase),
            found->second.source_map,
            found->second.source_x,
            found->second.source_y,
            static_cast<unsigned long long>(found->second.generation));
        return false;
    }

    RunState& state = found->second;
    int roll = -1;
    bool replay = false;
    if (state.phase == kArmed) {
        const unsigned long maximum_roll = 999999UL;
        roll = static_cast<int>(
            RouteRandom().randInt(maximum_roll));
        RouteSelection selected;
        if (!SelectRouteByRoll(state.source_map, roll, &selected)) {
            LogManager::logFormat(
                1,
                "time_spiral_diag.cpp",
                "bool time_spiral::SelectOrReplay(CUser*, BreakTrapSelection*)",
                0,
                "[TIME_SPIRAL][SELECT_FAIL] reason=route_table uid=%d party=%d map=%d source=%d,%d roll=%d generation=%llu",
                user->GetUID(),
                context.party_index,
                state.source_map,
                state.source_x,
                state.source_y,
                roll,
                static_cast<unsigned long long>(state.generation));
            return false;
        }
        state.selection = selected;
        state.phase = kSelected;
    } else if (state.phase == kSelected) {
        replay = true;
    } else {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "bool time_spiral::SelectOrReplay(CUser*, BreakTrapSelection*)",
            0,
            "[TIME_SPIRAL][SELECT_FAIL] reason=phase uid=%d party=%d map=%d phase=%d generation=%llu",
            user->GetUID(),
            context.party_index,
            context.map_index,
            static_cast<int>(state.phase),
            static_cast<unsigned long long>(state.generation));
        return false;
    }

    selection->target_x = static_cast<uint8_t>(state.selection.target_x);
    selection->target_y = static_cast<uint8_t>(state.selection.target_y);
    selection->result = static_cast<uint8_t>(state.selection.result);
    LogManager::logFormat(
        1,
        "time_spiral_diag.cpp",
        "bool time_spiral::SelectOrReplay(CUser*, BreakTrapSelection*)",
        0,
        "[TIME_SPIRAL][SELECT_OK] uid=%d party=%d dungeon=%d map=%d source=%d,%d roll=%d replay=%d target=%d,%d result=%d weight=%d normal=%d,%d generation=%llu",
        user->GetUID(),
        context.party_index,
        context.dungeon_index,
        context.map_index,
        state.source_x,
        state.source_y,
        roll,
        replay ? 1 : 0,
        state.selection.target_x,
        state.selection.target_y,
        state.selection.result,
        state.selection.weight,
        state.normal_x,
        state.normal_y,
        static_cast<unsigned long long>(state.generation));
    return true;
}

MoveRequestResolution ResolveMoveRequest(CParty* party,
                                         int request_x,
                                         int request_y,
                                         int* target_x,
                                         int* target_y) {
    if (!target_x || !target_y) return kMoveRequestRejected;
    if (!party) return kMoveRequestOrdinary;

    // A published TimeSpiral dungeon must fail closed when its field
    // identity is internally inconsistent.  Falling back to ordinary
    // MoveMap here would turn a damaged member count/current map/coordinate
    // into authority for an unrestricted request.
    const bool active_time_spiral =
        IsDungeon(party->battle_field().get_dungeon_index());
    FieldContext context;
    if (!BuildFieldContext(party, &context)) {
        if (active_time_spiral) {
            LogManager::logFormat(
                1,
                "time_spiral_diag.cpp",
                "time_spiral::MoveRequestResolution time_spiral::ResolveMoveRequest(CParty*, int, int, int*, int*)",
                0,
                "[TIME_SPIRAL][MOVE_REJECT] reason=field_context party=%d dungeon=%d map=%d pos=%d,%d members=%d request=%d,%d",
                party->GetPartyIndex(),
                party->battle_field().get_dungeon_index(),
                party->battle_field().GetCurrentMapIndex(),
                party->battle_field().GetCurPosX(),
                party->battle_field().GetCurPosY(),
                party->get_member_count(),
                request_x,
                request_y);
        }
        return active_time_spiral ? kMoveRequestRejected
                                  : kMoveRequestOrdinary;
    }

    MutexGuard guard(&StateMutex());
    RunTable::iterator found = Runs().find(party);
    if (found == Runs().end()) {
        // BuildFieldContext proved this is an active, published TimeSpiral
        // dungeon.  A missing generation is corruption/lifecycle mismatch,
        // not authority to fall back to unrestricted ordinary movement.
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "time_spiral::MoveRequestResolution time_spiral::ResolveMoveRequest(CParty*, int, int, int*, int*)",
            0,
            "[TIME_SPIRAL][MOVE_REJECT] reason=missing_run party=%d dungeon=%d map=%d pos=%d,%d request=%d,%d",
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            request_x,
            request_y);
        return kMoveRequestRejected;
    }
    if (!SameRun(found->second, context) ||
        !SamePartyOwner(found->second, party)) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "time_spiral::MoveRequestResolution time_spiral::ResolveMoveRequest(CParty*, int, int, int*, int*)",
            0,
            "[TIME_SPIRAL][MOVE_REJECT] reason=run_identity party=%d dungeon=%d map=%d pos=%d,%d request=%d,%d generation=%llu",
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            request_x,
            request_y,
            static_cast<unsigned long long>(found->second.generation));
        Runs().erase(found);
        return kMoveRequestRejected;
    }

    RunState& state = found->second;
    if (state.phase == kDormant) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "time_spiral::MoveRequestResolution time_spiral::ResolveMoveRequest(CParty*, int, int, int*, int*)",
            0,
            "[TIME_SPIRAL][MOVE_ORDINARY] party=%d dungeon=%d map=%d pos=%d,%d request=%d,%d generation=%llu",
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            request_x,
            request_y,
            static_cast<unsigned long long>(state.generation));
        return kMoveRequestOrdinary;
    }
    if (!SameSource(state, context)) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "time_spiral::MoveRequestResolution time_spiral::ResolveMoveRequest(CParty*, int, int, int*, int*)",
            0,
            "[TIME_SPIRAL][MOVE_REJECT] reason=source_changed party=%d dungeon=%d map=%d pos=%d,%d request=%d,%d armed_map=%d armed_pos=%d,%d phase=%d generation=%llu",
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            request_x,
            request_y,
            state.source_map,
            state.source_x,
            state.source_y,
            static_cast<int>(state.phase),
            static_cast<unsigned long long>(state.generation));
        ClearSelection(&state);
        return kMoveRequestRejected;
    }
    if (state.phase != kSelected) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "time_spiral::MoveRequestResolution time_spiral::ResolveMoveRequest(CParty*, int, int, int*, int*)",
            0,
            "[TIME_SPIRAL][MOVE_REJECT] reason=not_selected party=%d dungeon=%d map=%d pos=%d,%d request=%d,%d phase=%d generation=%llu",
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            request_x,
            request_y,
            static_cast<int>(state.phase),
            static_cast<unsigned long long>(state.generation));
        return kMoveRequestRejected;
    }

    const bool selected = request_x == state.selection.target_x &&
                          request_y == state.selection.target_y;
    const bool ordinary_exit = request_x == state.normal_x &&
                               request_y == state.normal_y;
    if (!selected && !ordinary_exit) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "time_spiral::MoveRequestResolution time_spiral::ResolveMoveRequest(CParty*, int, int, int*, int*)",
            0,
            "[TIME_SPIRAL][MOVE_REJECT] reason=unexpected_exit party=%d dungeon=%d map=%d pos=%d,%d request=%d,%d selected=%d,%d normal=%d,%d generation=%llu",
            context.party_index,
            context.dungeon_index,
            context.map_index,
            context.pos_x,
            context.pos_y,
            request_x,
            request_y,
            state.selection.target_x,
            state.selection.target_y,
            state.normal_x,
            state.normal_y,
            static_cast<unsigned long long>(state.generation));
        return kMoveRequestRejected;
    }

    *target_x = state.selection.target_x;
    *target_y = state.selection.target_y;
    LogManager::logFormat(
        1,
        "time_spiral_diag.cpp",
        "time_spiral::MoveRequestResolution time_spiral::ResolveMoveRequest(CParty*, int, int, int*, int*)",
        0,
        "[TIME_SPIRAL][MOVE_AUTHORIZE] party=%d dungeon=%d map=%d pos=%d,%d request=%d,%d request_kind=%s target=%d,%d result=%d generation=%llu",
        context.party_index,
        context.dungeon_index,
        context.map_index,
        context.pos_x,
        context.pos_y,
        request_x,
        request_y,
        selected ? "selected" : "normal_rewrite",
        state.selection.target_x,
        state.selection.target_y,
        state.selection.result,
        static_cast<unsigned long long>(state.generation));
    return kMoveRequestAuthorized;
}

bool GetMoveOverride(CParty* party,
                     int target_x,
                     int target_y,
                     int* logical_exit_x,
                     int* logical_exit_y) {
    if (!logical_exit_x || !logical_exit_y) return false;
    FieldContext context;
    if (!BuildFieldContext(party, &context)) return false;

    MutexGuard guard(&StateMutex());
    RunTable::const_iterator found = Runs().find(party);
    if (found == Runs().end() || found->second.phase != kSelected ||
        !SameSource(found->second, context) ||
        !SamePartyOwner(found->second, party) ||
        target_x != found->second.selection.target_x ||
        target_y != found->second.selection.target_y) {
        return false;
    }

    *logical_exit_x = found->second.normal_x;
    *logical_exit_y = found->second.normal_y;
    return true;
}

void ConsumeMoveSuccess(CParty* party, int target_x, int target_y) {
    if (!party) return;
    CBattle_Field& field = party->battle_field();
    const int current_x = field.GetCurPosX();
    const int current_y = field.GetCurPosY();
    int map_x = -1;
    int map_y = -1;
    if (current_x != target_x || current_y != target_y ||
        !FindCoordinateForMap(field.GetCurrentMapIndex(), &map_x, &map_y) ||
        map_x != target_x || map_y != target_y) {
        return;
    }

    MutexGuard guard(&StateMutex());
    RunTable::iterator found = Runs().find(party);
    if (found == Runs().end() || found->second.phase != kSelected ||
        found->second.party_index != party->GetPartyIndex() ||
        found->second.dungeon_index != field.get_dungeon_index() ||
        !SamePartyOwner(found->second, party) ||
        found->second.selection.target_x != target_x ||
        found->second.selection.target_y != target_y) {
        return;
    }
    LogManager::logFormat(
        1,
        "time_spiral_diag.cpp",
        "void time_spiral::ConsumeMoveSuccess(CParty*, int, int)",
        0,
        "[TIME_SPIRAL][MOVE_COMMIT] party=%d dungeon=%d source_map=%d source=%d,%d target=%d,%d result=%d generation=%llu",
        party->GetPartyIndex(),
        field.get_dungeon_index(),
        found->second.source_map,
        found->second.source_x,
        found->second.source_y,
        target_x,
        target_y,
        found->second.selection.result,
        static_cast<unsigned long long>(found->second.generation));
    ClearSelection(&found->second);
}

bool ShouldConsumeTicket(CParty* party, CUser* user, int dungeon_index) {
    if (!IsDungeon(dungeon_index)) return false;
    UserContext context;
    if (!BuildUserContext(user, false, false, &context) ||
        context.party != party ||
        context.dungeon_index != dungeon_index) {
        return false;
    }

    MutexGuard guard(&StateMutex());
    RunTable::const_iterator found = Runs().find(party);
    return found != Runs().end() && SameRun(found->second, context) &&
           SameUser(found->second, context) &&
           !found->second.ticket_consumed;
}

bool IsTicketConsumed(CParty* party, CUser* user, int dungeon_index) {
    if (!IsDungeon(dungeon_index)) return false;
    UserContext context;
    if (!BuildUserContext(user, false, false, &context) ||
        context.party != party ||
        context.dungeon_index != dungeon_index) {
        return false;
    }

    MutexGuard guard(&StateMutex());
    RunTable::const_iterator found = Runs().find(party);
    return found != Runs().end() && SameRun(found->second, context) &&
           SameUser(found->second, context) &&
           found->second.ticket_consumed;
}

void MarkTicketConsumed(CParty* party, CUser* user, int dungeon_index) {
    if (!party || !user || !IsDungeon(dungeon_index)) return;
    UserContext context;
    if (!BuildUserContext(user, false, false, &context) ||
        context.party != party ||
        context.dungeon_index != dungeon_index) {
        return;
    }

    MutexGuard guard(&StateMutex());
    RunTable::iterator found = Runs().find(party);
    if (found == Runs().end() || !SameRun(found->second, context) ||
        !SameUser(found->second, context)) {
        return;
    }
    found->second.ticket_consumed = true;
}

}  // namespace time_spiral
