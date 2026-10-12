#include "old_game/pvp/match/integrated_matching_runtime.h"

#include <algorithm>
#include <deque>
#include <map>
#include <stdint.h>
#include <utility>
#include <vector>

#include "common/manager/data_manager.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/match/channel_zombie_matching_producer.h"
#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/world/region/game_world.h"


namespace integrated_matching {
namespace {

const unsigned char kIntegratedBound = 0;
const unsigned short kModeOneOnOne = 0;
const unsigned short kModeRelay = 1;
const unsigned short kModeIndividual = 2;
const unsigned short kModeTeam = 3;

const int kModeCount = 4;

const unsigned char kMatchingStateStarted = 1;
const unsigned char kMatchingStateCanceled = 2;

// CLIENT_IDA_EVIDENCE: sub_60EA80 proves state 2 closes the matching view,
// while 1 and 3 both open it with different text.  The surviving client does
// not prove the server-side distinction between 1 and 3.  This owner uses 1
// only for the initial accepted request and deliberately never fabricates 3.

struct MemberRef {
    MemberRef()
        : account_id(0),
          world_uid(0xffffu),
          server_group(0),
          charac_no(0) {}

    uint32_t account_id;
    uint16_t world_uid;
    int server_group;
    uint32_t charac_no;
};

struct QueueGroup {
    QueueGroup()
        : bound(kIntegratedBound),
          mode(kModeOneOnOne),
          party_index(-1) {}

    unsigned char bound;
    unsigned short mode;
    int party_index;
    std::vector<MemberRef> members;
};

struct SessionMember {
    SessionMember() : room_team(0) {}

    MemberRef member;
    unsigned char room_team;
};

struct MatchSession {
    MatchSession()
        : token(0),
          bound(kIntegratedBound),
          mode(kModeOneOnOne),
          finalizing(false) {}

    uint32_t token;
    unsigned char bound;
    unsigned short mode;
    bool finalizing;
    std::vector<QueueGroup> groups;
    std::vector<SessionMember> members;
    std::vector<MemberRef> acknowledgements;
};

Mutex g_matching_mutex;
// RECONSTRUCTED_BUSINESS: matching queues, direct-connect acknowledgements
// and session tokens are process-lifetime coordination state.  The inspected
// 0828 schemas contain PvP result/score tables but no persistence owner for a
// live queue, so none of this state is written to DB.
std::deque<QueueGroup> g_queues[kModeCount];
std::map<uint32_t, MatchSession> g_sessions;
uint32_t g_next_session_token = 1;

bool IsModeValid(unsigned short mode) {
    return mode <= kModeTeam;
}

int RequiredPlayerCount(unsigned short mode) {
    // PVF_EVIDENCE: etc/matchingmode.etc caps the four integrated modes at
    // 2/4/4/4.  Individual mode also records min=3, but the client supplies
    // no timeout threshold for starting at that minimum; use the deterministic
    // maximum and leave the 3-player timeout path evidence-blocked.
    switch (mode) {
    case kModeOneOnOne:
        return 2;
    case kModeRelay:
    case kModeIndividual:
    case kModeTeam:
        return 4;
    default:
        return 0;
    }
}

int MaximumPreformedCount(unsigned short mode) {
    switch (mode) {
    case kModeOneOnOne:
    case kModeIndividual:
        return 1;
    case kModeRelay:
    case kModeTeam:
        return 2;
    default:
        return 0;
    }
}

PVP_BATTLE_MODE ResolveRoomBattleMode(unsigned short mode) {
    switch (mode) {
    case kModeOneOnOne:
        return static_cast<PVP_BATTLE_MODE>(2);
    case kModeRelay:
        return static_cast<PVP_BATTLE_MODE>(3);
    case kModeIndividual:
        return static_cast<PVP_BATTLE_MODE>(1);
    case kModeTeam:
        return static_cast<PVP_BATTLE_MODE>(2);
    default:
        return static_cast<PVP_BATTLE_MODE>(0);
    }
}

bool SupportsLocalRoomHandoff() {
    GameWorld* const world = G_GameWorld();
    if (!world || !world->IsIntegratedPvPBaseChannel()) {
        return false;
    }

    // EVIDENCE_BLOCKED: channel 9/10/14/15 installs a specialised IMatch
    // owner (league/tournament/preliminary/fair).  The 0828 client proves the
    // integrated wire state machine, but no surviving server binary proves
    // that its global matcher may bypass those four owners.  Keep this
    // process-local reconstruction on the normal PvP_Room owner only.
    const int channel_type = world->GetChannelType();
    return channel_type != 9 && channel_type != 10 &&
           channel_type != 14 && channel_type != 15;
}

MemberRef MakeMemberRef(CUser* user) {
    MemberRef ref;
    if (!user) {
        return ref;
    }
    ref.account_id = static_cast<uint32_t>(user->get_acc_id());
    ref.world_uid = static_cast<uint16_t>(user->get_unique_id());
    ref.server_group = user->GetServerGroup();
    ref.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    return ref;
}

bool SameMember(const MemberRef& lhs, const MemberRef& rhs) {
    return lhs.account_id == rhs.account_id &&
           lhs.world_uid == rhs.world_uid &&
           lhs.server_group == rhs.server_group &&
           lhs.charac_no == rhs.charac_no;
}

bool ContainsMember(const std::vector<MemberRef>& members,
                    const MemberRef& target) {
    for (std::vector<MemberRef>::const_iterator it = members.begin();
         it != members.end(); ++it) {
        if (SameMember(*it, target)) {
            return true;
        }
    }
    return false;
}

CUser* ResolveMember(const MemberRef& ref) {
    GameWorld* const world = G_GameWorld();
    CUser* const user = world ? world->find_from_world(ref.world_uid) : 0;
    if (!user || static_cast<uint32_t>(user->get_acc_id()) != ref.account_id ||
        user->GetServerGroup() != ref.server_group ||
        static_cast<uint32_t>(user->getCurCharacNo()) != ref.charac_no) {
        return 0;
    }
    return user;
}

int QueueEligibilityError(CUser* user) {
    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        user->IsCurCharacGhost() || user->CheckInPvp() != 0 ||
        !SupportsLocalRoomHandoff()) {
        return 19;
    }

    // EXISTING_OWNER_EVIDENCE: MakePVPRoom and PvP_Room::join_room both
    // apply these restrictions on fair-PvP channel types 15/16.  Type 15 is
    // intentionally excluded above because it owns CFairMatch; type 16 uses
    // the normal room owner, so validate every queued member here (including
    // the future room creator, whom join_room never checks).
    if (G_GameWorld()->GetChannelType() == 16) {
        if (G_CDataManager()->GetMaxGradePvPChannel() <
            user->get_pvp_grade()) {
            return 13;
        }

        fair_pvp::CFairPvPScore* const score =
            static_cast<fair_pvp::CFairPvPScore*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(7)));
        ServerParameterScript* const server_parameter =
            G_CDataManager()->get_server_parameter_script();
        if (!score || !server_parameter) {
            return 19;
        }
        if (score->GetGiveItem() <=
            server_parameter->get_pvp_item_give(user->get_pvp_grade())) {
            return 7;
        }
    }
    return 0;
}

bool IsQueueEligible(CUser* user) {
    return QueueEligibilityError(user) == 0;
}

bool SameGroup(const QueueGroup& lhs, const QueueGroup& rhs) {
    if (lhs.bound != rhs.bound || lhs.mode != rhs.mode ||
        lhs.party_index != rhs.party_index ||
        lhs.members.size() != rhs.members.size()) {
        return false;
    }
    for (size_t index = 0; index < lhs.members.size(); ++index) {
        if (!SameMember(lhs.members[index], rhs.members[index])) {
            return false;
        }
    }
    return true;
}

bool GroupsOverlap(const QueueGroup& lhs, const QueueGroup& rhs) {
    for (std::vector<MemberRef>::const_iterator left = lhs.members.begin();
         left != lhs.members.end(); ++left) {
        if (ContainsMember(rhs.members, *left)) {
            return true;
        }
    }
    return false;
}

bool CaptureQueueGroup(CUser* requester,
                       unsigned short mode,
                       QueueGroup& group,
                       int& error) {
    error = 19;
    if (!IsModeValid(mode)) {
        return false;
    }
    const int requester_error = QueueEligibilityError(requester);
    if (requester_error) {
        error = requester_error;
        return false;
    }
    error = 19;

    group = QueueGroup();
    group.mode = mode;

    if (!requester->CheckInParty()) {
        group.members.push_back(MakeMemberRef(requester));
        return true;
    }

    CParty* const party = requester->GetParty();
    if (!party || party->getManager() != requester) {
        return false;
    }

    const int member_count = party->get_member_count();
    if (member_count <= 0 ||
        member_count > MaximumPreformedCount(mode)) {
        error = 22;
        return false;
    }

    group.party_index = party->GetPartyIndex();
    group.members.push_back(MakeMemberRef(requester));
    for (int seat = 0; seat < 4; ++seat) {
        CUser* const member = party->get_user(seat);
        if (!member || member == requester) {
            continue;
        }
        if (!IsQueueEligible(member) || member->GetPartyIndex() !=
                                            group.party_index) {
            return false;
        }
        group.members.push_back(MakeMemberRef(member));
    }

    return static_cast<int>(group.members.size()) == member_count;
}

bool ValidateQueueGroup(const QueueGroup& group,
                        std::vector<CUser*>* resolved_users) {
    if (group.members.empty() || !IsModeValid(group.mode) ||
        static_cast<int>(group.members.size()) >
            MaximumPreformedCount(group.mode)) {
        return false;
    }

    std::vector<CUser*> users;
    for (std::vector<MemberRef>::const_iterator it = group.members.begin();
         it != group.members.end(); ++it) {
        CUser* const user = ResolveMember(*it);
        if (!IsQueueEligible(user)) {
            return false;
        }
        users.push_back(user);
    }

    if (group.party_index < 0) {
        if (users.size() != 1u || users[0]->CheckInParty()) {
            return false;
        }
    } else {
        CParty* const party = G_CGameManager()->GetParty(group.party_index);
        if (!party || party->get_member_count() !=
                          static_cast<int>(group.members.size()) ||
            !SameMember(MakeMemberRef(party->getManager()),
                        group.members.front())) {
            return false;
        }

        int observed_count = 0;
        for (int seat = 0; seat < 4; ++seat) {
            CUser* const member = party->get_user(seat);
            if (!member) {
                continue;
            }
            ++observed_count;
            if (member->GetPartyIndex() != group.party_index ||
                !ContainsMember(group.members, MakeMemberRef(member))) {
                return false;
            }
        }
        if (observed_count != static_cast<int>(group.members.size())) {
            return false;
        }
    }

    if (resolved_users) {
        *resolved_users = users;
    }
    return true;
}

bool SessionContains(const MatchSession& session, const MemberRef& member) {
    for (std::vector<SessionMember>::const_iterator it =
             session.members.begin();
         it != session.members.end(); ++it) {
        if (SameMember(it->member, member)) {
            return true;
        }
    }
    return false;
}

bool AnySessionContains(const QueueGroup& group) {
    for (std::map<uint32_t, MatchSession>::const_iterator session =
             g_sessions.begin();
         session != g_sessions.end(); ++session) {
        for (std::vector<MemberRef>::const_iterator member =
                 group.members.begin();
             member != group.members.end(); ++member) {
            if (SessionContains(session->second, *member)) {
                return true;
            }
        }
    }
    return false;
}

bool FindQueuedGroup(const MemberRef& member,
                     int& mode_index,
                     size_t& group_index) {
    for (int mode = 0; mode < kModeCount; ++mode) {
        for (size_t index = 0; index < g_queues[mode].size(); ++index) {
            if (ContainsMember(g_queues[mode][index].members, member)) {
                mode_index = mode;
                group_index = index;
                return true;
            }
        }
    }
    return false;
}

void PurgeInvalidQueueGroups() {
    for (int mode = 0; mode < kModeCount; ++mode) {
        size_t index = 0;
        while (index < g_queues[mode].size()) {
            if (!ValidateQueueGroup(g_queues[mode][index], 0)) {
                g_queues[mode].erase(g_queues[mode].begin() + index);
            } else {
                ++index;
            }
        }
    }
}

bool SelectQueueGroups(const std::deque<QueueGroup>& queue,
                       size_t begin,
                       int remaining_players,
                       std::vector<size_t>& selected) {
    if (remaining_players == 0) {
        return true;
    }
    for (size_t index = begin; index < queue.size(); ++index) {
        const int group_size = static_cast<int>(queue[index].members.size());
        if (group_size <= 0 || group_size > remaining_players) {
            continue;
        }
        selected.push_back(index);
        if (SelectQueueGroups(queue, index + 1,
                              remaining_players - group_size, selected)) {
            return true;
        }
        selected.pop_back();
    }
    return false;
}

uint32_t NextSessionToken() {
    for (;;) {
        uint32_t token = g_next_session_token++;
        if (!token) {
            token = g_next_session_token++;
        }
        if (token && g_sessions.find(token) == g_sessions.end()) {
            return token;
        }
    }
}

bool BuildSession(unsigned short mode, MatchSession& result) {
    PurgeInvalidQueueGroups();
    const int required_players = RequiredPlayerCount(mode);
    if (!required_players) {
        return false;
    }

    std::deque<QueueGroup>& queue = g_queues[mode];
    std::vector<size_t> selected;
    if (!SelectQueueGroups(queue, 0, required_players, selected)) {
        return false;
    }

    result = MatchSession();
    result.token = NextSessionToken();
    result.mode = mode;

    int team_one_remaining = mode == kModeOneOnOne ? 1 : 2;
    for (std::vector<size_t>::const_iterator index = selected.begin();
         index != selected.end(); ++index) {
        const QueueGroup& group = queue[*index];
        result.groups.push_back(group);

        unsigned char team = 0;
        if (mode != kModeIndividual) {
            const int group_size = static_cast<int>(group.members.size());
            if (group_size <= team_one_remaining) {
                team = 1;
                team_one_remaining -= group_size;
            } else {
                team = 2;
            }
        }

        for (std::vector<MemberRef>::const_iterator member =
                 group.members.begin();
             member != group.members.end(); ++member) {
            SessionMember session_member;
            session_member.member = *member;
            session_member.room_team = team;
            result.members.push_back(session_member);
        }
    }

    for (std::vector<size_t>::reverse_iterator index = selected.rbegin();
         index != selected.rend(); ++index) {
        queue.erase(queue.begin() + *index);
    }
    g_sessions.insert(std::make_pair(result.token, result));
    return true;
}

std::vector<MemberRef> GetGroupMembers(const QueueGroup& group) {
    return group.members;
}

std::vector<MemberRef> GetSessionMembers(const MatchSession& session) {
    std::vector<MemberRef> members;
    for (std::vector<SessionMember>::const_iterator it =
             session.members.begin();
         it != session.members.end(); ++it) {
        members.push_back(it->member);
    }
    return members;
}

void SendToMembers(PacketGuard& packet,
                   const std::vector<MemberRef>& members) {
    for (std::vector<MemberRef>::const_iterator it = members.begin();
         it != members.end(); ++it) {
        CUser* const user = ResolveMember(*it);
        if (user) {
            user->Send(packet);
        }
    }
}

void SendMatchingState(const std::vector<MemberRef>& members,
                       unsigned char state) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_INTEGRATE_MATCHING_STATE);
    packet.put_byte(state);
    packet.finalize(true);
    SendToMembers(packet, members);
}

void SendAuthoritativeMode(const std::vector<MemberRef>& members,
                           unsigned char bound,
                           unsigned short mode) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_INTEGRATED_MATCHING_MODE_CHANGE);
    packet.put_byte(bound);
    packet.put_short(mode);
    packet.finalize(true);
    SendToMembers(packet, members);
}

void SendMatchingRoster(const MatchSession& session) {
    PacketGuard packet;
    packet.put_header(
        ENUM_PACKETCLASS_NOTIPACKET,
        ENUM_NOTIPACKET_INTEGRATE_MATCHING_PLAYER_UID_LIST);
    packet.put_int(static_cast<int>(session.token));
    packet.put_byte(static_cast<int>(session.members.size()));
    for (std::vector<SessionMember>::const_iterator it =
             session.members.begin();
         it != session.members.end(); ++it) {
        packet.put_short(it->member.world_uid);

        // CLIENT_IDA_EVIDENCE: NOTI501 handler sub_60F9D0 maps wire side
        // 1 -> client side 2 and wire side 2 -> client side 1.
        const unsigned char wire_side =
            it->room_team == 1 ? 2 : (it->room_team == 2 ? 1 : 0);
        packet.put_byte(wire_side);
    }
    packet.finalize(true);
    SendToMembers(packet, GetSessionMembers(session));
}

void SendMatchingComplete(const MatchSession& session) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_INTEGRATE_MATCHING_COMPLETE);
    packet.finalize(true);
    SendToMembers(packet, GetSessionMembers(session));
}

bool SendRequestSuccess(CUser* user,
                        unsigned char bound,
                        unsigned short mode) {
    if (!user) return false;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_REQUEST_INTEGRATED_MATCHING);
    packet.put_byte(1);
    packet.put_byte(bound);
    packet.put_short(mode);
    if (bound == 0 && mode == 0) {
        // CLIENT_IDA_EVIDENCE: sub_4EF8F0 conditionally consumes this u16
        // only for pair (0,0).  Its presentation meaning is not recoverable
        // from the client reader; zero is the explicit compatibility value.
        packet.put_short(0);
    }
    packet.finalize(true);
    return user->Send(packet) != 0;
}

void SendCancelSuccess(CUser* user) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_CANCEL_INTEGRATED_MATCHING);
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
}

bool ValidateSession(const MatchSession& session,
                     std::vector<CUser*>* resolved_users) {
    if (!IsModeValid(session.mode) || session.members.size() !=
                                          static_cast<size_t>(
                                              RequiredPlayerCount(
                                                  session.mode))) {
        return false;
    }

    for (std::vector<QueueGroup>::const_iterator group =
             session.groups.begin();
         group != session.groups.end(); ++group) {
        if (!ValidateQueueGroup(*group, 0)) {
            return false;
        }
    }

    int team_counts[3] = {0, 0, 0};
    std::vector<CUser*> users;
    for (std::vector<SessionMember>::const_iterator member =
             session.members.begin();
         member != session.members.end(); ++member) {
        CUser* const user = ResolveMember(member->member);
        if (!IsQueueEligible(user) || member->room_team > 2) {
            return false;
        }
        ++team_counts[member->room_team];
        users.push_back(user);
    }

    if (session.mode == kModeIndividual) {
        if (team_counts[0] != 4) {
            return false;
        }
    } else {
        const int expected_side_count =
            session.mode == kModeOneOnOne ? 1 : 2;
        if (team_counts[1] != expected_side_count ||
            team_counts[2] != expected_side_count) {
            return false;
        }
    }

    if (resolved_users) {
        *resolved_users = users;
    }
    return true;
}

void RollBackRoom(PvP_Room* room, const std::vector<CUser*>& users) {
    if (!room) {
        return;
    }
    const int room_index = room->get_index();
    for (std::vector<CUser*>::const_iterator it = users.begin();
         it != users.end(); ++it) {
        if (*it && (*it)->GetPvpIndex() == room_index) {
            (*it)->SetPvpIndex(-1);
        }
    }
    room->reset();
    G_CGameManager()->PutPvp(room);
}

void SendCreatorRoomPackets(CUser* creator, PvP_Room* room) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_USERINFO);
    packet.put_byte(1);
    packet.put_short(1);
    creator->make_basic_info(packet, 1);
    packet.finalize(true);
    creator->Send(packet);

    packet.clear();
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_ROOM_INFO);
    packet.put_short(1);
    room->make_room_info(reinterpret_cast<char*>(&packet));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

void CheckOutPreformedGroups(const MatchSession& session) {
    for (std::vector<QueueGroup>::const_iterator group =
             session.groups.begin();
         group != session.groups.end(); ++group) {
        if (group->party_index < 0) {
            continue;
        }
        // Leave non-leaders before the stored leader.  This preserves the
        // ordinary CParty manager invariant while dissolving the preformed
        // group at the PvP_Room ownership boundary.
        for (size_t index = group->members.size(); index > 0; --index) {
            CUser* const user = ResolveMember(group->members[index - 1]);
            if (user && user->CheckInParty()) {
                G_CGameManager()->CheckOutParty(user, false);
            }
        }
    }
}

bool FinalizeSession(const MatchSession& session) {
    std::vector<CUser*> users;
    if (!ValidateSession(session, &users) || users.empty()) {
        return false;
    }

    PvP_Room* const room = G_CGameManager()->GetPvp();
    if (!room) {
        return false;
    }

    char empty_text[1] = {0};
    const int create_result = room->create_room(
        users[0], 1, empty_text, 0, false, empty_text, 0,
        ResolveRoomBattleMode(session.mode));
    if (create_result > 0) {
        RollBackRoom(room, users);
        return false;
    }

    room->change_team(0, session.members[0].room_team);
    SendCreatorRoomPackets(users[0], room);

    for (size_t index = 1; index < users.size(); ++index) {
        int seat = -1;
        const int join_result = room->join_room(users[index], seat, false);
        if (join_result > 0 || seat < 0 || seat > 7) {
            RollBackRoom(room, users);
            return false;
        }
        room->change_team(seat, session.members[index].room_team);

        PacketGuard seat_packet;
        room->make_seat_info(reinterpret_cast<char*>(&seat_packet), seat);
        G_GameWorld()->send_all(seat_packet);
    }

    for (size_t index = 1; index < users.size(); ++index) {
        room->set_ready_state(users[index], true);
    }
    if (room->set_ready_state(users[0], true) != -1) {
        RollBackRoom(room, users);
        return false;
    }

    CheckOutPreformedGroups(session);
    SendMatchingComplete(session);
    for (std::vector<CUser*>::iterator user = users.begin();
         user != users.end(); ++user) {
        G_GameWorld()->goto_pvp(*user);
    }

    room->start_pvp();
    PacketGuard state_packet;
    room->make_state_info(reinterpret_cast<char*>(&state_packet));
    G_GameWorld()->send_all(state_packet);
    return true;
}

void QueueCounts(uint32_t counts[kModeCount]) {
    for (int mode = 0; mode < kModeCount; ++mode) {
        counts[mode] = 0;
    }

    MutexGuard guard(&g_matching_mutex);
    PurgeInvalidQueueGroups();
    for (int mode = 0; mode < kModeCount; ++mode) {
        for (std::deque<QueueGroup>::const_iterator group =
                 g_queues[mode].begin();
             group != g_queues[mode].end(); ++group) {
            counts[mode] += static_cast<uint32_t>(group->members.size());
        }
    }
}

void SendUserCounts(CUser* user) {
    uint32_t counts[kModeCount];
    QueueCounts(counts);

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_INTEGRATE_MATCHING_USER_COUNT);

    // CLIENT_IDA_EVIDENCE: sub_657BA0 consumes u32,u8 then exactly count
    // pairs of (u32 metric_a,u32 metric_b).  metric_b is the displayed count.
    // EVIDENCE_BLOCKED: the first u32 and metric_a have no semantic consumer
    // in this client.  Zero makes its animation take the proven 0.1 s clamp.
    packet.put_int(0);
    packet.put_byte(kModeCount);
    for (int mode = 0; mode < kModeCount; ++mode) {
        packet.put_int(0);
        packet.put_int(static_cast<int>(counts[mode]));
    }
    packet.finalize(true);
    user->Send(packet);
}

int MalformedPacket(const char* function_name, int command) {
    return LineFunc(command, function_name, 0, 0);
}

}  // namespace

void CancelForDisconnect(CUser* user) {
    if (!user) {
        return;
    }

    const MemberRef disconnected = MakeMemberRef(user);
    std::vector<MemberRef> recipients;
    {
        MutexGuard guard(&g_matching_mutex);
        int queued_mode = -1;
        size_t group_index = 0;
        if (FindQueuedGroup(disconnected, queued_mode, group_index)) {
            recipients = g_queues[queued_mode][group_index].members;
            g_queues[queued_mode].erase(
                g_queues[queued_mode].begin() + group_index);
        } else {
            for (std::map<uint32_t, MatchSession>::iterator session =
                     g_sessions.begin();
                 session != g_sessions.end(); ++session) {
                if (!SessionContains(session->second, disconnected) ||
                    session->second.finalizing) {
                    continue;
                }
                recipients = GetSessionMembers(session->second);
                g_sessions.erase(session);
                break;
            }
        }
    }

    if (!recipients.empty()) {
        SendMatchingState(recipients, kMatchingStateCanceled);
    }
}

}  // namespace integrated_matching_0828


using namespace integrated_matching;

Dispatcher_RequestIntegratedMatching::
    Dispatcher_RequestIntegratedMatching()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_RequestIntegratedMatching::
    ~Dispatcher_RequestIntegratedMatching() {}

int Dispatcher_RequestIntegratedMatching::dispatch_sig(CUser* user,
                                                         PacketBuf& packet) {
    unsigned char bound = 0xffu;
    unsigned short mode = 0xffffu;
    if (packet.get_byte(bound) != 1 || packet.get_short(mode) != 1) {
        return MalformedPacket(
            "Dispatcher_RequestIntegratedMatching::dispatch_sig", 518);
    }
    cMyTrace field_trace(
        "Dispatcher_RequestIntegratedMatching::dispatch_sig", 518, 0);
    field_trace("[ZOMBIE_MATCH][CMD518_FIELDS] bound=%u mode=%u channel_type=%d state=%d",
                static_cast<unsigned int>(bound),
                static_cast<unsigned int>(mode),
                G_GameWorld() ? G_GameWorld()->GetChannelType() : -1,
                user ? user->get_state() : -1);
    unsigned char authority_bound = bound;
    unsigned short authority_mode = mode;
    (void)channel_zombie_matching_producer::NormalizeClientPair(
        user, bound, mode, authority_bound, authority_mode);
    const channel_zombie_matching_producer::ProducerResult zombie_result =
        channel_zombie_matching_producer::Request(
            user, authority_bound, authority_mode);
    if (zombie_result !=
            channel_zombie_matching_producer::PRODUCER_NOT_OWNED) {
        if (zombie_result ==
                channel_zombie_matching_producer::PRODUCER_ACCEPTED) {
            const bool ack_queued = SendRequestSuccess(user, bound, mode);
            if (!ack_queued) {
                const bool aborted =
                    channel_zombie_matching_producer::AbortRequest(
                        user, authority_bound, authority_mode);
                cMyTrace trace(
                    "Dispatcher_RequestIntegratedMatching::dispatch_sig",
                    __LINE__, 5);
                trace("[ZOMBIE_MATCH][REQUEST_FINALIZE_FAIL] stage=ack-send state=%d aborted=%d",
                      user ? user->get_state() : -1, aborted ? 1 : 0);
                if (!aborted)
                    channel_zombie_matching_producer::ResetUser(user);
                return 0;
            }

            // The 0828 CMD518 success handler clears its ready-user list.
            // Publish the staged canonical full-lobby NOTI595 snapshot, and
            // only then allow exact-six room launch after that destructive
            // ACK.
            const bool committed =
                channel_zombie_matching_producer::CommitRequest(
                    user, authority_bound, authority_mode);
            if (!committed) {
                const bool aborted =
                    channel_zombie_matching_producer::AbortRequest(
                        user, authority_bound, authority_mode);
                cMyTrace trace(
                    "Dispatcher_RequestIntegratedMatching::dispatch_sig",
                    __LINE__, 5);
                trace("[ZOMBIE_MATCH][REQUEST_FINALIZE_FAIL] stage=commit state=%d aborted=%d",
                      user ? user->get_state() : -1, aborted ? 1 : 0);
                // ACK cannot be retracted.  Consume any surviving tentative
                // membership and reset per-user handoff/result sidecars rather
                // than silently leaving a client-success/server-pending split.
                channel_zombie_matching_producer::ResetUser(user);
            }
        } else {
            // No success ACK is synthesized for unavailable or rejected
            // project-owned authority.  Error 19 is the existing generic
            // matching-state rejection path.
            user->SendCmdErrorPacket(
                ENUM_CMDPACKET_REQUEST_INTEGRATED_MATCHING, 19);
        }
        return 0;
    }
    if (bound != kIntegratedBound || !IsModeValid(mode)) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REQUEST_INTEGRATED_MATCHING, 22);
        return 0;
    }

    QueueGroup requested_group;
    int error = 19;
    if (!CaptureQueueGroup(user, mode, requested_group, error)) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REQUEST_INTEGRATED_MATCHING, error);
        return 0;
    }

    bool accepted = false;
    bool matched = false;
    MatchSession session;
    {
        MutexGuard guard(&g_matching_mutex);
        PurgeInvalidQueueGroups();

        if (!AnySessionContains(requested_group)) {
            bool overlap = false;
            bool exact_existing = false;
            for (int queue_mode = 0; queue_mode < kModeCount;
                 ++queue_mode) {
                for (std::deque<QueueGroup>::const_iterator queued =
                         g_queues[queue_mode].begin();
                     queued != g_queues[queue_mode].end(); ++queued) {
                    if (!GroupsOverlap(*queued, requested_group)) {
                        continue;
                    }
                    overlap = true;
                    exact_existing = SameGroup(*queued, requested_group);
                    break;
                }
                if (overlap) {
                    break;
                }
            }

            if (!overlap || exact_existing) {
                if (!exact_existing) {
                    g_queues[mode].push_back(requested_group);
                }
                accepted = true;
                matched = BuildSession(mode, session);
            }
        }
    }

    if (!accepted) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REQUEST_INTEGRATED_MATCHING, 19);
        return 0;
    }

    SendRequestSuccess(user, bound, mode);
    SendMatchingState(GetGroupMembers(requested_group),
                      kMatchingStateStarted);
    if (matched) {
        SendMatchingRoster(session);
    }
    return 0;
}

int Dispatcher_RequestIntegratedMatching::read(PacketBuf& packet,
                                                 MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_RequestIntegratedMatching::process(CUser* user,
                                                    MSG_BASE& message,
                                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_CancelIntegratedMatching::
    Dispatcher_CancelIntegratedMatching()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_CancelIntegratedMatching::~Dispatcher_CancelIntegratedMatching() {}

int Dispatcher_CancelIntegratedMatching::dispatch_sig(CUser* user,
                                                        PacketBuf& packet) {
    unsigned char bound = 0xffu;
    unsigned short mode = 0xffffu;
    if (packet.get_byte(bound) != 1 || packet.get_short(mode) != 1) {
        return MalformedPacket(
            "Dispatcher_CancelIntegratedMatching::dispatch_sig", 519);
    }
    unsigned char authority_bound = bound;
    unsigned short authority_mode = mode;
    (void)channel_zombie_matching_producer::NormalizeClientPair(
        user, bound, mode, authority_bound, authority_mode);
    const channel_zombie_matching_producer::ProducerResult zombie_result =
        channel_zombie_matching_producer::Cancel(
            user, authority_bound, authority_mode);
    if (zombie_result !=
            channel_zombie_matching_producer::PRODUCER_NOT_OWNED) {
        if (zombie_result ==
                channel_zombie_matching_producer::PRODUCER_ACCEPTED) {
            SendCancelSuccess(user);
        } else {
            user->SendCmdErrorPacket(
                ENUM_CMDPACKET_CANCEL_INTEGRATED_MATCHING, 19);
        }
        return 0;
    }
    if (bound != kIntegratedBound || !IsModeValid(mode)) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CANCEL_INTEGRATED_MATCHING, 22);
        return 0;
    }

    const MemberRef requester = MakeMemberRef(user);
    bool canceled = false;
    bool canceled_session = false;
    bool pair_mismatch = false;
    QueueGroup canceled_group;
    MatchSession session_copy;
    {
        MutexGuard guard(&g_matching_mutex);
        PurgeInvalidQueueGroups();
        int queued_mode = -1;
        size_t group_index = 0;
        if (FindQueuedGroup(requester, queued_mode, group_index)) {
            const QueueGroup& group = g_queues[queued_mode][group_index];
            if (group.bound != bound || group.mode != mode) {
                pair_mismatch = true;
            } else {
                canceled_group = group;
                g_queues[queued_mode].erase(
                    g_queues[queued_mode].begin() + group_index);
                canceled = true;
            }
        } else {
            for (std::map<uint32_t, MatchSession>::iterator session =
                     g_sessions.begin();
                 session != g_sessions.end(); ++session) {
                if (SessionContains(session->second, requester)) {
                    if (session->second.bound != bound ||
                        session->second.mode != mode ||
                        session->second.finalizing) {
                        pair_mismatch = true;
                    } else {
                        session_copy = session->second;
                        g_sessions.erase(session);
                        canceled_session = true;
                    }
                    break;
                }
            }
        }
    }

    if (pair_mismatch) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CANCEL_INTEGRATED_MATCHING, 19);
        return 0;
    }
    SendCancelSuccess(user);
    if (canceled) {
        SendMatchingState(GetGroupMembers(canceled_group),
                          kMatchingStateCanceled);
    } else if (canceled_session) {
        SendMatchingState(GetSessionMembers(session_copy),
                          kMatchingStateCanceled);
    } else {
        std::vector<MemberRef> requester_only;
        requester_only.push_back(requester);
        SendMatchingState(requester_only, kMatchingStateCanceled);
    }
    return 0;
}

int Dispatcher_CancelIntegratedMatching::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_CancelIntegratedMatching::process(CUser* user,
                                                   MSG_BASE& message,
                                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_IntegrateMatchingDirectComplete::
    Dispatcher_IntegrateMatchingDirectComplete()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_IntegrateMatchingDirectComplete::
    ~Dispatcher_IntegrateMatchingDirectComplete() {}

int Dispatcher_IntegrateMatchingDirectComplete::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    unsigned int token = 0;
    if (packet.get_int(token) != 1) {
        return MalformedPacket(
            "Dispatcher_IntegrateMatchingDirectComplete::dispatch_sig", 564);
    }

    const MemberRef requester = MakeMemberRef(user);
    bool cancel_session = false;
    bool finalize_session = false;
    MatchSession session_copy;
    {
        MutexGuard guard(&g_matching_mutex);
        std::map<uint32_t, MatchSession>::iterator session =
            g_sessions.find(static_cast<uint32_t>(token));
        if (session == g_sessions.end() || session->second.finalizing ||
            !SessionContains(session->second, requester)) {
            return 0;
        }

        if (!ContainsMember(session->second.acknowledgements, requester)) {
            session->second.acknowledgements.push_back(requester);
        }
        if (session->second.acknowledgements.size() ==
            session->second.members.size()) {
            if (!ValidateSession(session->second, 0)) {
                session_copy = session->second;
                g_sessions.erase(session);
                cancel_session = true;
            } else {
                session->second.finalizing = true;
                session_copy = session->second;
                finalize_session = true;
            }
        }
    }

    if (cancel_session) {
        SendMatchingState(GetSessionMembers(session_copy),
                          kMatchingStateCanceled);
        return 0;
    }
    if (!finalize_session) {
        return 0;
    }

    const bool completed = FinalizeSession(session_copy);
    {
        MutexGuard guard(&g_matching_mutex);
        g_sessions.erase(session_copy.token);
    }
    if (!completed) {
        SendMatchingState(GetSessionMembers(session_copy),
                          kMatchingStateCanceled);
    }
    return 0;
}

int Dispatcher_IntegrateMatchingDirectComplete::read(PacketBuf& packet,
                                                       MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_IntegrateMatchingDirectComplete::process(
    CUser* user,
    MSG_BASE& message,
    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_IntegratedMatchingModeChange::
    Dispatcher_IntegratedMatchingModeChange()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_IntegratedMatchingModeChange::
    ~Dispatcher_IntegratedMatchingModeChange() {}

int Dispatcher_IntegratedMatchingModeChange::dispatch_sig(CUser* user,
                                                            PacketBuf& packet) {
    unsigned char requested_bound = 0xffu;
    unsigned short requested_mode = 0xffffu;
    if (packet.get_byte(requested_bound) != 1 ||
        packet.get_short(requested_mode) != 1) {
        return MalformedPacket(
            "Dispatcher_IntegratedMatchingModeChange::dispatch_sig", 565);
    }
    if (requested_bound != kIntegratedBound ||
        !IsModeValid(requested_mode)) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_INTEGRATED_MATCHING_MODE_CHANGE, 22);
        return 0;
    }

    const MemberRef requester = MakeMemberRef(user);
    unsigned char authoritative_bound = requested_bound;
    unsigned short authoritative_mode = requested_mode;
    std::vector<MemberRef> recipients;
    recipients.push_back(requester);
    bool matched = false;
    bool rejected_size = false;
    MatchSession matched_session;
    {
        MutexGuard guard(&g_matching_mutex);
        PurgeInvalidQueueGroups();

        bool in_session = false;
        for (std::map<uint32_t, MatchSession>::const_iterator session =
                 g_sessions.begin();
             session != g_sessions.end(); ++session) {
            if (SessionContains(session->second, requester)) {
                authoritative_bound = session->second.bound;
                authoritative_mode = session->second.mode;
                in_session = true;
                break;
            }
        }

        if (!in_session) {
            int old_mode = -1;
            size_t group_index = 0;
            if (FindQueuedGroup(requester, old_mode, group_index)) {
                QueueGroup group = g_queues[old_mode][group_index];
                recipients = group.members;
                if (!SameMember(group.members.front(), requester) ||
                    static_cast<int>(group.members.size()) >
                        MaximumPreformedCount(requested_mode)) {
                    authoritative_bound = group.bound;
                    authoritative_mode = group.mode;
                    rejected_size = true;
                } else {
                    g_queues[old_mode].erase(
                        g_queues[old_mode].begin() + group_index);
                    group.bound = requested_bound;
                    group.mode = requested_mode;
                    g_queues[requested_mode].push_back(group);
                    matched = BuildSession(requested_mode, matched_session);
                }
            }
        }
    }

    if (rejected_size) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_INTEGRATED_MATCHING_MODE_CHANGE, 22);
    }
    SendAuthoritativeMode(recipients, authoritative_bound,
                          authoritative_mode);
    if (matched) {
        SendMatchingRoster(matched_session);
    }
    return 0;
}

int Dispatcher_IntegratedMatchingModeChange::read(PacketBuf& packet,
                                                    MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_IntegratedMatchingModeChange::process(CUser* user,
                                                       MSG_BASE& message,
                                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_IntegrateMatchingUserCount::
    Dispatcher_IntegrateMatchingUserCount()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_IntegrateMatchingUserCount::
    ~Dispatcher_IntegrateMatchingUserCount() {}

int Dispatcher_IntegrateMatchingUserCount::dispatch_sig(CUser* user,
                                                          PacketBuf& packet) {
    (void)packet;
    if (user && user->get_state() >= 3) {
        SendUserCounts(user);
    }
    return 0;
}

int Dispatcher_IntegrateMatchingUserCount::read(PacketBuf& packet,
                                                 MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_IntegrateMatchingUserCount::process(CUser* user,
                                                    MSG_BASE& message,
                                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
