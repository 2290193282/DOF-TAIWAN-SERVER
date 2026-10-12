#include "old_game/pvp/tournament/pvp_tournament.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/system_time.h"
#include "core/config/env/environment.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/db/pvp_party/db_pvp_tournament.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/social/party/party.h"
#include "old_game/world/region/game_world.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace {

const char kTournamentPvf[] = "tw_pvp_tournament_0828.evt";

struct TournamentPolicy {
    TournamentPolicy()
        : enabled(0), maximum_tournaments(0), maximum_teams(0),
          maximum_team_members(0), minimum_teams(0), lifetime_seconds(0),
          maximum_schedule_lead(0), room_lease_seconds(0), minimum_level(0),
          maximum_qualification(0), title_maximum_bytes(0),
          password_maximum_bytes(0), default_map(0), battle_mode(0) {}

    int enabled;
    int maximum_tournaments;
    int maximum_teams;
    int maximum_team_members;
    int minimum_teams;
    int lifetime_seconds;
    int maximum_schedule_lead;
    int room_lease_seconds;
    int minimum_level;
    int maximum_qualification;
    int title_maximum_bytes;
    int password_maximum_bytes;
    int default_map;
    int battle_mode;
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }
private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

class TournamentPolicyCache {
public:
    TournamentPolicyCache() : attempted_(false), valid_(false), policy_() {}

    const TournamentPolicy* get() {
        if (!attempted_) {
            attempted_ = true;
            valid_ = load();
        }
        return valid_ ? &policy_ : 0;
    }

private:
    bool load() {
        if (!loadRDARScriptFile("Event/", kTournamentPvf)) {
            return false;
        }
        RdarUnloadGuard unload;
        TournamentPolicy parsed;
        unsigned int mask = 0u;
        bool malformed = false;
        std::string tag;
        while (ScanType(tag, true) == 1) {
            int* destination = 0;
            unsigned int bit = 0u;
            if (tag == "[enabled]") {
                destination = &parsed.enabled; bit = 1u << 0;
            } else if (tag == "[maximum tournament count]") {
                destination = &parsed.maximum_tournaments; bit = 1u << 1;
            } else if (tag == "[maximum team count]") {
                destination = &parsed.maximum_teams; bit = 1u << 2;
            } else if (tag == "[maximum team member count]") {
                destination = &parsed.maximum_team_members; bit = 1u << 3;
            } else if (tag == "[minimum team count to start]") {
                destination = &parsed.minimum_teams; bit = 1u << 4;
            } else if (tag == "[tournament lifetime seconds]") {
                destination = &parsed.lifetime_seconds; bit = 1u << 5;
            } else if (tag == "[maximum schedule lead seconds]") {
                destination = &parsed.maximum_schedule_lead; bit = 1u << 6;
            } else if (tag == "[room lease seconds]") {
                destination = &parsed.room_lease_seconds; bit = 1u << 7;
            } else if (tag == "[minimum character level]") {
                destination = &parsed.minimum_level; bit = 1u << 8;
            } else if (tag == "[maximum qualification]") {
                destination = &parsed.maximum_qualification; bit = 1u << 9;
            } else if (tag == "[title maximum bytes]") {
                destination = &parsed.title_maximum_bytes; bit = 1u << 10;
            } else if (tag == "[password maximum bytes]") {
                destination = &parsed.password_maximum_bytes; bit = 1u << 11;
            } else if (tag == "[default pvp map]") {
                destination = &parsed.default_map; bit = 1u << 12;
            } else if (tag == "[pvp battle mode]") {
                destination = &parsed.battle_mode; bit = 1u << 13;
            } else {
                malformed = true;
                continue;
            }
            bool success = false;
            const int value = ScanInt(&success);
            if (!success || (mask & bit) != 0u) {
                malformed = true;
            } else {
                *destination = value;
                mask |= bit;
            }
        }
        const unsigned int complete = (1u << 14) - 1u;
        if (malformed || mask != complete || parsed.enabled != 1 ||
            parsed.maximum_tournaments !=
                pvp_tournament::kMaximumTournamentCount ||
            parsed.maximum_teams !=
                pvp_tournament::kMaximumTeamCount ||
            parsed.maximum_team_members !=
                pvp_tournament::kMaximumTeamMemberCount ||
            parsed.minimum_teams !=
                pvp_tournament::kMinimumTeamCountToStart ||
            parsed.lifetime_seconds !=
                pvp_tournament::kTournamentLifetimeSeconds ||
            parsed.maximum_schedule_lead !=
                pvp_tournament::kTournamentLifetimeSeconds ||
            parsed.room_lease_seconds !=
                pvp_tournament::kRoomLeaseSeconds ||
            parsed.minimum_level != 20 ||
            parsed.maximum_qualification != 30 ||
            parsed.title_maximum_bytes != 32 ||
            parsed.password_maximum_bytes != 8 ||
            parsed.default_map < 0 || parsed.default_map > 32767 ||
            parsed.battle_mode != 2) {
            return false;
        }
        policy_ = parsed;
        return true;
    }

    bool attempted_;
    bool valid_;
    TournamentPolicy policy_;
};

struct SessionState {
    SessionState()
        : account_id(0u), charac_no(0u), token(0u), next_request_token(1u),
          current_tournament_id(0u), viewed_tournament_id(0u),
          server_group(-1), pending_operation(0u), pending_subtype(0u),
          pending_command(0u), loaded(false) {}

    uint32_t account_id;
    uint32_t charac_no;
    uint32_t token;
    uint32_t next_request_token;
    uint32_t current_tournament_id;
    uint32_t viewed_tournament_id;
    int server_group;
    uint8_t pending_operation;
    uint8_t pending_subtype;
    uint8_t pending_command;
    bool loaded;
};

class TournamentRoomMatch;

struct RoomBinding {
    RoomBinding()
        : room(0), adapter(0), tournament_id(0u), match_id(0u),
          room_token(0u), server_group(-1), expires_epoch(0u) {}
    PvP_Room* room;
    TournamentRoomMatch* adapter;
    uint32_t tournament_id;
    uint32_t match_id;
    uint32_t room_token;
    int server_group;
    uint32_t expires_epoch;
};

struct RetiredAdapter {
    RetiredAdapter() : adapter(0), release_epoch(0u) {}
    TournamentRoomMatch* adapter;
    uint32_t release_epoch;
};

Mutex g_tournament_mutex;
std::map<CUser*, SessionState> g_sessions;
std::map<uint32_t, CUser*> g_online_characters;
std::map<uint32_t, RoomBinding> g_rooms_by_token;
std::map<PvP_Room*, uint32_t> g_tokens_by_room;
std::vector<RetiredAdapter> g_retired_adapters;
uint32_t g_next_session_token = 1u;
uint32_t g_last_expiry_sweep = 0u;

TournamentPolicyCache& PolicyCache() {
    static TournamentPolicyCache cache;
    return cache;
}

uint32_t NowEpoch() {
    const int value = GlobalData::system_time()
                          ? GlobalData::system_time()->getCurSec() : 0;
    return value > 0 ? static_cast<uint32_t>(value) : 0u;
}

uint32_t NextNonZero(uint32_t& source) {
    uint32_t value = source++;
    if (value == 0u) {
        value = source++;
    }
    if (source == 0u) {
        ++source;
    }
    return value;
}

bool IsCharacterReady(CUser* user) {
    return user && user->get_state() > 2 && user->getCurCharacR() &&
           user->getCurCharacNo() > 0 && user->get_acc_id() > 0 &&
           user->GetServerGroup() >= 0 &&
           user->GetServerGroup() <
               pvp_tournament::kMaximumServerGroupCount;
}

bool IsTournamentChannel() {
    return G_GameWorld() && G_GameWorld()->GetChannelType() == 10;
}

bool IsTownReady(CUser* user) {
    return IsCharacterReady(user) && user->get_state() == 3 &&
           IsTournamentChannel();
}

void TraceFailure(const char* reason,
                  uint32_t tournament_id,
                  uint32_t match_id) {
    cMyTrace trace("PvpTournament0828", __LINE__, 5);
    trace("[0828 tournament] %s tournament(%u) match(%u)",
          reason ? reason : "unknown",
          static_cast<unsigned int>(tournament_id),
          static_cast<unsigned int>(match_id));
}

void Send544Result(CUser* user, bool success, uint8_t error,
                   uint32_t tournament_id) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_PVP_TOURNAMENT_MATCH_LIST);
    packet.put_byte(success ? 1 : 0);
    if (success) {
        packet.put_int(static_cast<int>(tournament_id));
    } else {
        packet.put_byte(error ? error : pvp_tournament::ERROR_GENERIC);
    }
    packet.finalize(true);
    user->Send(packet);
}

void Send545Result(CUser* user, bool success, uint8_t value,
                   uint8_t request_subtype) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_PVP_TOURNAMENT_REQUEST);
    packet.put_byte(success ? 1 : 0);
    if (success) {
        packet.put_byte(value);
    } else {
        // CLIENT_IDA_EVIDENCE: sub_510F60 consumes the request subtype tail
        // only on failure.  Generic {0,error} helpers corrupt its cursor.
        packet.put_byte(value ? value : pvp_tournament::ERROR_GENERIC);
        packet.put_byte(request_subtype);
    }
    packet.finalize(true);
    user->Send(packet);
}

bool PutWireString(PacketGuard& packet, const char* text, size_t capacity) {
    size_t length = 0u;
    while (length < capacity && text[length] != 0) ++length;
    if (length >= capacity) return false;
    if (packet.put_int(static_cast<int>(length)) != 1) return false;
    return length == 0u ||
           packet.put_binary(text, static_cast<int>(length)) == 1;
}

void SendEmptyRootList(CUser* user) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_TOURNAMENT_INFO);
    packet.put_byte(0);
    packet.put_short(0);
    packet.finalize(true);
    user->Send(packet);
}

void SendRootList(CUser* user,
                  const pvp_tournament::Message& state,
                  bool reset_registry) {
    if (!user || state.root_count >
                     pvp_tournament::kMaximumTournamentCount) return;
    // CLIENT_IDA_EVIDENCE: NOTI604/subtype0 clears the local map only when
    // count != 1.  A full load containing exactly one record therefore needs
    // an explicit empty batch first; count==1 remains the incremental-add
    // form used for a newly created public tournament.
    if (reset_registry && state.root_count == 1u) {
        SendEmptyRootList(user);
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_TOURNAMENT_INFO);
    packet.put_byte(0);
    packet.put_short(state.root_count);
    for (uint8_t index = 0u; index < state.root_count; ++index) {
        const pvp_tournament::RootRecord& root = state.roots[index];
        packet.put_int(static_cast<int>(root.tournament_id));
        packet.put_int(static_cast<int>(root.scheduled_epoch));
        if (!PutWireString(packet, root.title, sizeof(root.title)) ||
            !PutWireString(packet, root.mode, sizeof(root.mode)) ||
            !PutWireString(packet, root.organizer_name,
                           sizeof(root.organizer_name))) {
            return;
        }
        // CLIENT_IDA_EVIDENCE: subtype0 maps the remaining fields to the
        // 0x60-byte client record at +16,+91..+95.  +92/+93 are rendered as
        // "participants (current/maximum)" by sub_66C730; +94/+95 round-trip
        // the two create-dialog flags in sub_66E200/sub_66E490.  The +16 u32
        // is not a count (that would duplicate +93); the feature aggregate
        // owns it as the stable organizer character id.
        packet.put_int(static_cast<int>(root.organizer_charac_no));
        packet.put_byte(root.state);
        packet.put_byte(pvp_tournament::kMaximumParticipantCount);
        packet.put_byte(root.participant_count > 255u
                            ? 255
                            : static_cast<uint8_t>(root.participant_count));
        packet.put_byte(root.private_flag);
        packet.put_byte(root.invite_only);
    }
    packet.finalize(true);
    user->Send(packet);
}

void SendRootState(CUser* user,
                   const pvp_tournament::RootRecord& root) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_TOURNAMENT_INFO);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(root.tournament_id));
    packet.put_int(static_cast<int>(root.organizer_charac_no));
    packet.put_byte(root.state);
    // subtype1 overwrites exactly +91,+92,+93: state, maximum participants,
    // current participants.  The create flags live at +94/+95 and therefore
    // must not be shifted into this delta packet.
    packet.put_byte(pvp_tournament::kMaximumParticipantCount);
    packet.put_byte(root.participant_count > 255u
                        ? 255
                        : static_cast<uint8_t>(root.participant_count));
    packet.finalize(true);
    user->Send(packet);
}

void SendRootDelete(CUser* user, uint32_t tournament_id) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_TOURNAMENT_INFO);
    packet.put_byte(2);
    packet.put_int(static_cast<int>(tournament_id));
    packet.finalize(true);
    user->Send(packet);
}

uint16_t OnlineUniqueId(uint32_t charac_no) {
    CUser* online = 0;
    {
        MutexGuard guard(&g_tournament_mutex);
        std::map<uint32_t, CUser*>::const_iterator found =
            g_online_characters.find(charac_no);
        if (found != g_online_characters.end()) online = found->second;
    }
    if (!online || !IsCharacterReady(online) ||
        static_cast<uint32_t>(online->getCurCharacNo()) != charac_no) {
        return 0u;
    }
    const int unique_id = online->get_unique_id();
    return unique_id > 0 && unique_id <= 0xffff
               ? static_cast<uint16_t>(unique_id) : 0u;
}

void SendParticipants(CUser* user,
                      const pvp_tournament::Message& state) {
    if (!user || state.participant_count >
                     pvp_tournament::kMaximumParticipantCount) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_TOURNAMENT_INFO);
    packet.put_byte(3);
    packet.put_byte(state.participant_count);
    for (uint8_t index = 0u; index < state.participant_count; ++index) {
        const pvp_tournament::ParticipantRecord& member =
            state.participants[index];
        packet.put_int(static_cast<int>(member.team_id));
        packet.put_int(static_cast<int>(member.charac_no));
        packet.put_int(static_cast<int>(member.account_id));
        if (!PutWireString(packet, member.name, sizeof(member.name))) return;
        packet.put_short(OnlineUniqueId(member.charac_no));
        // Local record offsets +40..+46: four identity bytes followed by
        // role, bracket seed (the subtype-10 sender reads +45), and result.
        packet.put_byte(member.job);
        packet.put_byte(member.grow_type);
        packet.put_byte(member.level);
        packet.put_byte(member.pvp_grade);
        packet.put_byte(member.role);
        packet.put_byte(member.seed);
        packet.put_byte(member.result);
    }
    packet.finalize(true);
    user->Send(packet);
}

void SendMatchState(CUser* user, uint32_t match_id, uint8_t status) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_TOURNAMENT_INFO);
    packet.put_byte(4);
    packet.put_int(static_cast<int>(match_id));
    packet.put_byte(status);
    packet.finalize(true);
    user->Send(packet);
}

void SendRootValue(CUser* user, uint32_t tournament_id, uint32_t value) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_TOURNAMENT_INFO);
    packet.put_byte(5);
    packet.put_int(static_cast<int>(tournament_id));
    packet.put_int(static_cast<int>(value));
    packet.finalize(true);
    user->Send(packet);
}

void SendManagerGate(CUser* user,
                     const pvp_tournament::RootRecord& root,
                     uint32_t charac_no) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_TOURNAMENT_INFO);
    packet.put_byte(6);
    packet.put_int(static_cast<int>(root.tournament_id));
    packet.put_byte(root.organizer_charac_no == charac_no ? 1 : 0);
    packet.put_byte(root.bracket_ready ? 1 : 0);
    packet.finalize(true);
    user->Send(packet);
}

void SendBracket(CUser* user,
                 const pvp_tournament::Message& state) {
    if (!user || state.match_count >
                     pvp_tournament::kMaximumMatchCount) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_TOURNAMENT_INFO);
    packet.put_byte(7);
    packet.put_int(static_cast<int>(state.tournament_id));
    packet.put_byte(state.match_count);
    for (uint8_t index = 0u; index < state.match_count; ++index) {
        packet.put_byte(state.matches[index].match_index);
        packet.put_byte(state.matches[index].state);
    }
    packet.finalize(true);
    user->Send(packet);
}

void SendBracketDelta(CUser* user, uint32_t tournament_id,
                      uint8_t index, uint8_t value) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_TOURNAMENT_INFO);
    packet.put_byte(8);
    packet.put_int(static_cast<int>(tournament_id));
    packet.put_byte(index);
    packet.put_byte(value);
    packet.finalize(true);
    user->Send(packet);
}

void SendAnnouncement(CUser* user) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PVP_TOURNAMENT_INFO);
    packet.put_byte(10);
    packet.finalize(true);
    user->Send(packet);
}

const pvp_tournament::RootRecord* CurrentRoot(
    const pvp_tournament::Message& state) {
    if (state.tournament_id == 0u) return 0;
    for (uint8_t index = 0u; index < state.root_count; ++index) {
        if (state.roots[index].tournament_id == state.tournament_id) {
            return &state.roots[index];
        }
    }
    return 0;
}

void SendAggregate(CUser* user,
                   const pvp_tournament::Message& state,
                   bool full_registry) {
    if (!user) return;
    if (full_registry) SendRootList(user, state, true);
    if (!full_registry &&
        (state.operation == pvp_tournament::OPERATION_CREATE ||
         state.operation == pvp_tournament::OPERATION_JOIN)) {
        // A creator and a password-based private joiner do not necessarily
        // have this root in the client's map yet.  subtype0/count=1 is the
        // proven incremental-add form and must precede subtype1 deltas.
        SendRootList(user, state, false);
    }
    if (state.operation ==
        pvp_tournament::OPERATION_EXPIRE_SWEEP) {
        for (uint8_t index = 0u; index < state.root_count; ++index) {
            if (state.roots[index].state ==
                pvp_tournament::TOURNAMENT_CANCELLED_OR_EXPIRED) {
                SendRootDelete(user, state.roots[index].tournament_id);
            }
        }
        return;
    }
    const pvp_tournament::RootRecord* const root = CurrentRoot(state);
    if (!root) return;
    if (root->state == pvp_tournament::TOURNAMENT_CANCELLED_OR_EXPIRED) {
        SendRootDelete(user, root->tournament_id);
        return;
    }
    SendRootState(user, *root);
    SendRootValue(user, root->tournament_id, root->participant_count);
    if (state.participant_count > 0u || state.match_count > 0u ||
        state.tournament_id == root->tournament_id) {
        SendParticipants(user, state);
        SendManagerGate(user, *root,
                        static_cast<uint32_t>(user->getCurCharacNo()));
        SendBracket(user, state);
    }
    if (state.match_id != 0u) {
        for (uint8_t index = 0u; index < state.match_count; ++index) {
            if (state.matches[index].match_id == state.match_id) {
                SendMatchState(user, state.match_id,
                               state.matches[index].state);
                SendBracketDelta(user, root->tournament_id,
                                 state.matches[index].match_index,
                                 state.matches[index].state);
                break;
            }
        }
    }
    if ((state.operation == pvp_tournament::OPERATION_START &&
         root->state == pvp_tournament::TOURNAMENT_RUNNING) ||
        (state.operation == pvp_tournament::OPERATION_MATCH_RESULT &&
         root->state == pvp_tournament::TOURNAMENT_FINISHED)) {
        SendAnnouncement(user);
    }
}

bool QueueBackground(uint8_t operation,
                     uint32_t tournament_id,
                     uint32_t match_id,
                     uint32_t room_token,
                     int server_group,
                     uint8_t argument) {
    pvp_tournament::Message request;
    std::memset(&request, 0, sizeof(request));
    request.operation = operation;
    request.tournament_id = tournament_id;
    request.match_id = match_id;
    request.room_token = room_token;
    request.server_group = server_group;
    request.reply_user_idx = -1;
    request.expires_epoch = NowEpoch();
    request.argument = argument;
    return pvp_tournament::DBPvpTournament::QueueRequest(-1,
                                                                  request);
}

class TournamentRoomMatch : public IMatch {
public:
    TournamentRoomMatch(const pvp_tournament::Message& state,
                        const pvp_tournament::MatchRecord& match,
                        int server_group,
                        int16_t requested_map,
                        PVP_BATTLE_MODE battle_mode)
        : IMatch(), tournament_id_(state.tournament_id),
          match_id_(match.match_id), room_token_(match.room_token),
          left_team_id_(match.left_team_id),
          right_team_id_(match.right_team_id), server_group_(server_group),
          requested_map_(requested_map), battle_mode_(battle_mode),
          left_count_(0u), right_count_(0u), result_queued_(false) {
        std::memset(left_characters_, 0, sizeof(left_characters_));
        std::memset(right_characters_, 0, sizeof(right_characters_));
        for (uint8_t index = 0u; index < state.participant_count; ++index) {
            const pvp_tournament::ParticipantRecord& member =
                state.participants[index];
            if (member.approval !=
                    pvp_tournament::PARTICIPANT_ACTIVE) continue;
            if (member.team_id == left_team_id_ && left_count_ < 4u) {
                left_characters_[left_count_++] = member.charac_no;
            } else if (member.team_id == right_team_id_ && right_count_ < 4u) {
                right_characters_[right_count_++] = member.charac_no;
            }
        }
    }

    virtual ~TournamentRoomMatch() {}

    virtual int GetIndentification() {
        return static_cast<int>(match_id_);
    }

    virtual void RequestMap(int16_t& map_no, PVP_BATTLE_MODE& battle_mode) {
        map_no = requested_map_;
        battle_mode = battle_mode_;
    }

    virtual int OnCreateRoom(CUser* user) {
        return side(user) == 0 ? 19 : 0;
    }

    virtual void OnLeaveRoom(CUser*, PvP_Room* room, int team, int) {
        if (!room || result_queued_ || room->get_state() != 2 ||
            (team != 1 && team != 2) || room->GetTeamCount(team) > 0) {
            return;
        }
        result_queued_ = QueueBackground(
            pvp_tournament::OPERATION_MATCH_RESULT,
            tournament_id_, match_id_, room_token_, server_group_,
            static_cast<uint8_t>(team == 1 ? 2 : 1));
    }

    virtual void OnReward(PvP_Room*, int team, bool draw) {
        if (result_queued_) return;
        if (draw) {
            result_queued_ = QueueBackground(
                pvp_tournament::OPERATION_MATCH_DRAW,
                tournament_id_, match_id_, room_token_, server_group_, 0u);
        } else if (team == 1 || team == 2) {
            result_queued_ = QueueBackground(
                pvp_tournament::OPERATION_MATCH_RESULT,
                tournament_id_, match_id_, room_token_, server_group_,
                static_cast<uint8_t>(team));
        }
    }

    virtual void OnEndPvP(PvP_Room* room) {
        if (!result_queued_) {
            // Aborted/end-without-reward returns the lease to READY instead
            // of manufacturing a winner.
            result_queued_ = QueueBackground(
                pvp_tournament::OPERATION_MATCH_DRAW,
                tournament_id_, match_id_, room_token_, server_group_, 0u);
        }
        restore(room);
    }

    virtual int CheckJoinable(PvP_Room* room, CUser* user) {
        if (!room || !user || side(user) == 0) return 19;
        for (int seat_index = 0; seat_index < 8; ++seat_index) {
            if (room->get_user_seat(seat_index) == user) return 19;
        }
        return 0;
    }

    virtual int CheckSeat(int seat_index) {
        return seat_index >= 0 && seat_index < 8;
    }

    virtual bool OnReady(CUser* user, bool) {
        return side(user) != 0;
    }

    virtual int GetPlayerCount() {
        return static_cast<int>(left_count_ + right_count_);
    }

    int side(CUser* user) const {
        if (!user || !user->getCurCharacR() || user->getCurCharacNo() <= 0) {
            return 0;
        }
        const uint32_t charac_no =
            static_cast<uint32_t>(user->getCurCharacNo());
        for (uint8_t index = 0u; index < left_count_; ++index) {
            if (left_characters_[index] == charac_no) return 1;
        }
        for (uint8_t index = 0u; index < right_count_; ++index) {
            if (right_characters_[index] == charac_no) return 2;
        }
        return 0;
    }

    void restore(PvP_Room* room) {
        if (room && room->current_match_ == this && room->matching_system_) {
            room->current_match_ = room->matching_system_->GetMatchingSystem(
                ENUM_PVP_MATCHING_TOURNAMENT);
        }
    }

    uint32_t tournament_id() const { return tournament_id_; }
    uint32_t match_id() const { return match_id_; }
    uint32_t room_token() const { return room_token_; }
    bool result_queued() const { return result_queued_; }

private:
    uint32_t tournament_id_;
    uint32_t match_id_;
    uint32_t room_token_;
    uint32_t left_team_id_;
    uint32_t right_team_id_;
    int server_group_;
    int16_t requested_map_;
    PVP_BATTLE_MODE battle_mode_;
    uint32_t left_characters_[4];
    uint32_t right_characters_[4];
    uint8_t left_count_;
    uint8_t right_count_;
    bool result_queued_;
};

const pvp_tournament::MatchRecord* FindMatch(
    const pvp_tournament::Message& state) {
    for (uint8_t index = 0u; index < state.match_count; ++index) {
        if ((state.match_id != 0u &&
             state.matches[index].match_id == state.match_id) ||
            (state.room_token != 0u &&
             state.matches[index].room_token == state.room_token)) {
            return &state.matches[index];
        }
    }
    return 0;
}

void SetRoomTeam(PvP_Room* room, int seat, int team) {
    if (!room || seat < 0 || seat >= 8 || (team != 1 && team != 2)) return;
    room->teams_[seat] = static_cast<uint8_t>(team);
    room->relay_battle_mgr_.OnChangeTeam(seat, team);
}

bool SendCreatedRoomPackets(CUser* user, PvP_Room* room) {
    if (!user || !room) return false;
    PacketGuard packet;
    packet.put_header(0, 2);
    packet.put_byte(1);
    packet.put_short(1);
    user->make_basic_info(packet, 1);
    packet.finalize(true);
    user->Send(packet);

    packet.clear();
    packet.put_header(0, 41);
    packet.put_short(1);
    room->make_room_info(reinterpret_cast<char*>(&packet));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    G_GameWorld()->goto_pvp(user);
    return true;
}

bool EnterTournamentRoom(CUser* user,
                         const pvp_tournament::Message& state) {
    const TournamentPolicy* const policy = PolicyCache().get();
    const pvp_tournament::MatchRecord* const match = FindMatch(state);
    if (!policy || !match || match->room_token == 0u ||
        state.room_token != match->room_token || !IsTownReady(user) ||
        !G_GameWorld() || G_GameWorld()->GetChannelType() != 10) {
        return false;
    }

    RoomBinding binding;
    bool existing = false;
    {
        MutexGuard guard(&g_tournament_mutex);
        std::map<uint32_t, RoomBinding>::const_iterator found =
            g_rooms_by_token.find(match->room_token);
        if (found != g_rooms_by_token.end()) {
            binding = found->second;
            existing = true;
        }
    }
    if (existing) {
        if (!binding.room || !binding.adapter ||
            binding.adapter->side(user) == 0 ||
            binding.room->current_match_ != binding.adapter) {
            return false;
        }
        if (user->CheckInParty()) {
            G_CGameManager()->CheckOutParty(user, false);
        }
        int seat = -1;
        const int join_result = binding.room->join_room(user, seat, false);
        if (join_result > 0 || seat < 0 || seat >= 8) return false;
        SetRoomTeam(binding.room, seat, binding.adapter->side(user));
        PacketGuard packet;
        binding.room->make_seat_info(reinterpret_cast<char*>(&packet), seat);
        G_GameWorld()->send_all(packet);
        G_GameWorld()->goto_pvp(user);
        return true;
    }

    PvP_Room* const room = G_CGameManager()->GetPvp();
    if (!room) return false;
    TournamentRoomMatch* const adapter = new TournamentRoomMatch(
        state, *match, user->GetServerGroup(),
        static_cast<int16_t>(policy->default_map),
        static_cast<PVP_BATTLE_MODE>(policy->battle_mode));
    if (!adapter || adapter->side(user) == 0) {
        delete adapter;
        G_CGameManager()->PutPvp(room);
        return false;
    }
    if (user->CheckInParty()) {
        G_CGameManager()->CheckOutParty(user, false);
    }
    char title[30] = {};
    const pvp_tournament::RootRecord* const root = CurrentRoot(state);
    if (root) {
        std::strncpy(title, root->title, sizeof(title) - 1u);
    } else {
        std::strncpy(title, "Tournament", sizeof(title) - 1u);
    }
    const int create_result = room->create_room(
        user, 0, title, static_cast<int16_t>(policy->default_map), false,
        "", 0, static_cast<PVP_BATTLE_MODE>(policy->battle_mode));
    if (create_result > 0) {
        delete adapter;
        G_CGameManager()->PutPvp(room);
        return false;
    }
    room->current_match_ = adapter;
    SetRoomTeam(room, 0, adapter->side(user));
    room->setReamtchable(false);

    binding.room = room;
    binding.adapter = adapter;
    binding.tournament_id = state.tournament_id;
    binding.match_id = match->match_id;
    binding.room_token = match->room_token;
    binding.server_group = user->GetServerGroup();
    binding.expires_epoch = root ? root->expires_epoch :
        NowEpoch() + pvp_tournament::kTournamentLifetimeSeconds;
    {
        MutexGuard guard(&g_tournament_mutex);
        g_rooms_by_token[binding.room_token] = binding;
        g_tokens_by_room[room] = binding.room_token;
    }
    if (!QueueBackground(pvp_tournament::OPERATION_COMMIT_ROOM,
                         binding.tournament_id, binding.match_id,
                         binding.room_token, binding.server_group, 0u)) {
        // The command must not succeed with a purely local room and a DB
        // lease that was never committed.  No room packet has been emitted
        // yet, so unwind locally; the caller queues the durable release.
        {
            MutexGuard guard(&g_tournament_mutex);
            g_rooms_by_token.erase(binding.room_token);
            g_tokens_by_room.erase(room);
        }
        adapter->restore(room);
        user->SetPvpIndex(static_cast<int16_t>(-1));
        room->reset();
        G_CGameManager()->PutPvp(room);
        delete adapter;
        return false;
    }
    return SendCreatedRoomPackets(user, room);
}

bool FillClientIdentityLocked(CUser* user,
                              SessionState& session,
                              pvp_tournament::Message& request) {
    if (!IsCharacterReady(user) ||
        session.charac_no != static_cast<uint32_t>(user->getCurCharacNo()) ||
        session.account_id != static_cast<uint32_t>(user->get_acc_id()) ||
        session.server_group != user->GetServerGroup()) {
        return false;
    }
    request.account_id = session.account_id;
    request.charac_no = session.charac_no;
    request.session_token = session.token;
    request.request_token = NextNonZero(session.next_request_token);
    request.server_group = session.server_group;
    request.reply_user_idx = user->GetUID();
    request.expires_epoch = NowEpoch();
    const char* const name = user->getCurCharacName();
    if (name) {
        std::strncpy(request.actor_name, name,
                     sizeof(request.actor_name) - 1u);
    }
    return request.expires_epoch != 0u;
}

bool QueueClient(CUser* user,
                 pvp_tournament::Message& request,
                 uint8_t operation,
                 uint8_t subtype,
                 uint8_t command_kind,
                 bool require_loaded) {
    {
        MutexGuard guard(&g_tournament_mutex);
        std::map<CUser*, SessionState>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() || found->second.pending_operation != 0u ||
            (require_loaded && !found->second.loaded) ||
            !FillClientIdentityLocked(user, found->second, request)) {
            return false;
        }
        request.operation = operation;
        request.request_subtype = subtype;
        request.command_kind = command_kind;
        found->second.pending_operation = operation;
        found->second.pending_subtype = subtype;
        found->second.pending_command = command_kind;
    }
    if (pvp_tournament::DBPvpTournament::QueueRequest(
            user->GetUID(), request)) {
        return true;
    }
    MutexGuard guard(&g_tournament_mutex);
    std::map<CUser*, SessionState>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end() &&
        found->second.token == request.session_token &&
        found->second.pending_operation == operation) {
        found->second.pending_operation = 0u;
        found->second.pending_subtype = 0u;
        found->second.pending_command = 0u;
    }
    return false;
}

bool ReadWireString(PacketBuf& packet, char* output, size_t capacity,
                    size_t maximum_length) {
    unsigned int length = 0u;
    if (!output || capacity == 0u || maximum_length >= capacity ||
        packet.get_int(length) != 1 || length > maximum_length) {
        return false;
    }
    std::memset(output, 0, capacity);
    if (length == 0u) return true;
    if (packet.get_binary(output, static_cast<int>(length)) != 1 ||
        std::memchr(output, 0, length) != 0) {
        return false;
    }
    output[length] = 0;
    return true;
}

int NoRead(PacketBuf&, MSG_BASE&) { return 0; }
int NoProcess(CUser*, MSG_BASE&, ParamBase&) { return 0; }

uint8_t OperationForSubtype(uint8_t subtype) {
    static const uint8_t operations[12] = {
        pvp_tournament::OPERATION_OPEN_DETAIL,
        pvp_tournament::OPERATION_CREATE,
        pvp_tournament::OPERATION_JOIN,
        pvp_tournament::OPERATION_LEAVE,
        pvp_tournament::OPERATION_INVITE_MEMBER,
        pvp_tournament::OPERATION_CANCEL,
        pvp_tournament::OPERATION_SAVE_SEEDS,
        pvp_tournament::OPERATION_START,
        pvp_tournament::OPERATION_RESERVE_ROOM,
        pvp_tournament::OPERATION_REMOVE_MEMBER,
        pvp_tournament::OPERATION_SET_SEED,
        pvp_tournament::OPERATION_FINALIZE_BRACKET,
    };
    return subtype < 12u ? operations[subtype] : 0u;
}

bool Parse545(PacketBuf& packet,
              pvp_tournament::Message& request,
              uint8_t& subtype,
              uint8_t& parse_error) {
    subtype = 0u;
    parse_error = pvp_tournament::ERROR_PACKET;
    if (packet.get_byte(subtype) != 1) return false;
    if (subtype > 11u) {
        parse_error = pvp_tournament::ERROR_COMMAND;
        return false;
    }
    switch (subtype) {
        case 0:
            if (packet.get_int(request.tournament_id) != 1 ||
                request.tournament_id == 0u) return false;
            break;
        case 1: {
            unsigned short qualification = 0u;
            unsigned char literal = 0u;
            if (packet.get_short(qualification) != 1 ||
                packet.get_int(request.scheduled_epoch) != 1 ||
                !ReadWireString(packet, request.title,
                                sizeof(request.title), 32u) ||
                !ReadWireString(packet, request.password,
                                sizeof(request.password), 8u) ||
                packet.get_byte(literal) != 1 || literal != 64u ||
                packet.get_byte(request.flag_a) != 1 ||
                packet.get_byte(request.flag_b) != 1 ||
                request.flag_a > 1u || request.flag_b > 1u) {
                return false;
            }
            request.qualification = qualification;
            break;
        }
        case 2:
            if (packet.get_int(request.tournament_id) != 1 ||
                request.tournament_id == 0u ||
                !ReadWireString(packet, request.password,
                                sizeof(request.password), 8u)) return false;
            break;
        case 3:
        case 5:
        case 7:
        case 11:
            break;
        case 4:
        case 9:
            if (packet.get_int(request.target_charac_no) != 1 ||
                request.target_charac_no == 0u) return false;
            break;
        case 6:
            if (packet.get_byte(request.seed_count) != 1 ||
                request.seed_count == 0u ||
                request.seed_count >
                    pvp_tournament::kMaximumTeamCount) return false;
            for (uint8_t index = 0u; index < request.seed_count; ++index) {
                if (packet.get_int(request.seed_charac_no[index]) != 1 ||
                    request.seed_charac_no[index] == 0u ||
                    packet.get_byte(request.seed_value[index]) != 1 ||
                    request.seed_value[index] >=
                        pvp_tournament::kMaximumTeamCount) return false;
            }
            break;
        case 8:
            if (packet.get_byte(request.argument) != 1 ||
                request.argument >=
                    pvp_tournament::kMaximumMatchCount) return false;
            break;
        case 10:
            if (packet.get_int(request.target_charac_no) != 1 ||
                request.target_charac_no == 0u ||
                packet.get_byte(request.argument) != 1 ||
                (request.argument != 255u &&
                 request.argument >=
                    pvp_tournament::kMaximumTeamCount)) return false;
            break;
    }
    return true;
}

bool CorrelateLiveResult(CUser* user,
                         const pvp_tournament::Message& response,
                         SessionState& snapshot) {
    if (!user) return false;
    MutexGuard guard(&g_tournament_mutex);
    std::map<CUser*, SessionState>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end() ||
        found->second.account_id != response.account_id ||
        found->second.charac_no != response.charac_no ||
        found->second.token != response.session_token ||
        found->second.server_group != response.server_group ||
        found->second.pending_operation != response.operation ||
        found->second.pending_subtype != response.request_subtype ||
        found->second.pending_command != response.command_kind ||
        !IsCharacterReady(user) ||
        static_cast<uint32_t>(user->getCurCharacNo()) != response.charac_no ||
        static_cast<uint32_t>(user->get_acc_id()) != response.account_id ||
        user->GetServerGroup() != response.server_group) {
        return false;
    }
    // request_token is immutable per operation; equality with the session's
    // post-increment token closes delayed replies after another request.
    if (response.request_token + 1u != found->second.next_request_token &&
        !(response.request_token == 0xffffffffu &&
          found->second.next_request_token == 1u)) {
        return false;
    }
    found->second.pending_operation = 0u;
    found->second.pending_subtype = 0u;
    found->second.pending_command = 0u;
    if (response.error_code == 0u) {
        found->second.loaded = true;
        if (response.operation ==
                pvp_tournament::OPERATION_LOAD_REGISTRY) {
            found->second.current_tournament_id = response.tournament_id;
            found->second.viewed_tournament_id = response.tournament_id;
        } else if (response.operation ==
                       pvp_tournament::OPERATION_OPEN_DETAIL) {
            found->second.viewed_tournament_id = response.tournament_id;
        } else if (response.operation ==
                       pvp_tournament::OPERATION_CREATE ||
                   response.operation ==
                       pvp_tournament::OPERATION_JOIN) {
            found->second.current_tournament_id = response.tournament_id;
            found->second.viewed_tournament_id = response.tournament_id;
        } else if (response.operation ==
                       pvp_tournament::OPERATION_LEAVE ||
                   response.operation ==
                       pvp_tournament::OPERATION_CANCEL) {
            found->second.current_tournament_id = 0u;
            found->second.viewed_tournament_id = 0u;
        } else if (response.operation ==
                       pvp_tournament::OPERATION_FINALIZE_BRACKET) {
            // CMD545 success result 6 closes/resets the organizer UI.  Keep
            // durable participation but stop treating it as an open detail
            // subscription until CMD544/CMD545 subtype0 opens it again.
            found->second.viewed_tournament_id = 0u;
        }
    }
    snapshot = found->second;
    return true;
}

bool HasActiveParticipant(const pvp_tournament::Message& response,
                          uint32_t charac_no) {
    for (uint8_t index = 0u; index < response.participant_count; ++index) {
        if (response.participants[index].charac_no == charac_no &&
            response.participants[index].approval ==
                pvp_tournament::PARTICIPANT_ACTIVE) {
            return true;
        }
    }
    return false;
}

void BroadcastBackground(const pvp_tournament::Message& response,
                         CUser* excluded_user = 0) {
    struct Delivery {
        Delivery()
            : user(0), detail(false), add_root(false),
              public_summary(false), deletes() {}
        CUser* user;
        bool detail;
        bool add_root;
        bool public_summary;
        std::vector<uint32_t> deletes;
    };
    std::vector<Delivery> deliveries;
    const pvp_tournament::RootRecord* const primary =
        CurrentRoot(response);
    {
        MutexGuard guard(&g_tournament_mutex);
        for (std::map<CUser*, SessionState>::iterator it = g_sessions.begin();
             it != g_sessions.end(); ++it) {
            if (it->first == excluded_user || !it->second.loaded) continue;
            Delivery delivery;
            delivery.user = it->first;

            if (response.operation ==
                    pvp_tournament::OPERATION_EXPIRE_SWEEP) {
                for (uint8_t index = 0u; index < response.root_count;
                     ++index) {
                    const pvp_tournament::RootRecord& root =
                        response.roots[index];
                    if (root.private_flag == 0u ||
                        it->second.current_tournament_id ==
                            root.tournament_id ||
                        it->second.viewed_tournament_id ==
                            root.tournament_id) {
                        delivery.deletes.push_back(root.tournament_id);
                    }
                    if (it->second.current_tournament_id ==
                            root.tournament_id) {
                        it->second.current_tournament_id = 0u;
                    }
                    if (it->second.viewed_tournament_id ==
                            root.tournament_id) {
                        it->second.viewed_tournament_id = 0u;
                    }
                }
                if (!delivery.deletes.empty()) {
                    deliveries.push_back(delivery);
                }
                continue;
            }

            if (!primary) continue;
            const bool had_current = it->second.current_tournament_id ==
                                     response.tournament_id;
            const bool had_view = it->second.viewed_tournament_id ==
                                  response.tournament_id;
            const bool active_in_snapshot =
                response.tournament_id != 0u &&
                HasActiveParticipant(response, it->second.charac_no);
            if (active_in_snapshot) {
                it->second.current_tournament_id = response.tournament_id;
            } else if (had_current && response.participant_count > 0u &&
                       response.operation >=
                           pvp_tournament::OPERATION_CREATE &&
                       response.operation <=
                           pvp_tournament::OPERATION_MATCH_DRAW) {
                // State-changing responses carry a complete participant
                // snapshot.  A formerly active character absent from it was
                // removed/left and must not retain stale session ownership.
                it->second.current_tournament_id = 0u;
            }
            delivery.detail = had_current || had_view || active_in_snapshot;
            delivery.add_root = active_in_snapshot && !had_current &&
                                !had_view;
            delivery.public_summary = !delivery.detail &&
                                      primary->private_flag == 0u;
            if (primary->state ==
                    pvp_tournament::TOURNAMENT_FINISHED && had_current) {
                it->second.current_tournament_id = 0u;
            }
            if (primary->state ==
                    pvp_tournament::TOURNAMENT_CANCELLED_OR_EXPIRED) {
                if (had_current) it->second.current_tournament_id = 0u;
                if (had_view) it->second.viewed_tournament_id = 0u;
            }
            if (delivery.detail || delivery.public_summary) {
                deliveries.push_back(delivery);
            }
        }
    }
    for (size_t index = 0u; index < deliveries.size(); ++index) {
        Delivery& delivery = deliveries[index];
        if (!IsCharacterReady(delivery.user)) continue;
        for (size_t delete_index = 0u;
             delete_index < delivery.deletes.size(); ++delete_index) {
            SendRootDelete(delivery.user, delivery.deletes[delete_index]);
        }
        if (delivery.detail) {
            if (delivery.add_root) {
                SendRootList(delivery.user, response, false);
            }
            SendAggregate(delivery.user, response, false);
        } else if (delivery.public_summary && primary) {
            if (primary->state ==
                    pvp_tournament::TOURNAMENT_CANCELLED_OR_EXPIRED) {
                SendRootDelete(delivery.user, primary->tournament_id);
                continue;
            }
            if (response.operation ==
                    pvp_tournament::OPERATION_CREATE) {
                SendRootList(delivery.user, response, false);
            }
            SendRootState(delivery.user, *primary);
            SendRootValue(delivery.user, primary->tournament_id,
                          primary->participant_count);
            if ((response.operation ==
                     pvp_tournament::OPERATION_START &&
                 primary->state ==
                     pvp_tournament::TOURNAMENT_RUNNING) ||
                (response.operation ==
                     pvp_tournament::OPERATION_MATCH_RESULT &&
                 primary->state ==
                     pvp_tournament::TOURNAMENT_FINISHED)) {
                SendAnnouncement(delivery.user);
            }
        }
    }
}

void QueueReleaseAfterRoomFailure(
    const pvp_tournament::Message& response) {
    if (response.tournament_id != 0u && response.match_id != 0u &&
        response.room_token != 0u) {
        (void)QueueBackground(pvp_tournament::OPERATION_RELEASE_ROOM,
                              response.tournament_id, response.match_id,
                              response.room_token, response.server_group, 0u);
    }
}

}  // namespace

namespace pvp_tournament {

void BeginSession(CUser* user) {
    if (!IsTownReady(user) || !PolicyCache().get()) return;
    SessionState session;
    session.account_id = static_cast<uint32_t>(user->get_acc_id());
    session.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    session.server_group = user->GetServerGroup();
    {
        MutexGuard guard(&g_tournament_mutex);
        std::map<CUser*, SessionState>::iterator prior = g_sessions.find(user);
        if (prior != g_sessions.end()) {
            if (prior->second.charac_no == session.charac_no &&
                prior->second.account_id == session.account_id &&
                prior->second.server_group == session.server_group) {
                return;
            }
            g_online_characters.erase(prior->second.charac_no);
            g_sessions.erase(prior);
        }
        session.token = NextNonZero(g_next_session_token);
        g_sessions[user] = session;
        g_online_characters[session.charac_no] = user;
    }
    Message request;
    std::memset(&request, 0, sizeof(request));
    if (!QueueClient(user, request, OPERATION_LOAD_REGISTRY, 0u,
                     COMMAND_NONE, false)) {
        ResetSession(user);
    }
}

void ResetSession(CUser* user) {
    if (!user) return;
    MutexGuard guard(&g_tournament_mutex);
    std::map<CUser*, SessionState>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end()) {
        std::map<uint32_t, CUser*>::iterator online =
            g_online_characters.find(found->second.charac_no);
        if (online != g_online_characters.end() && online->second == user) {
            g_online_characters.erase(online);
        }
        g_sessions.erase(found);
    }
}

void OnPeriodicTick(uint32_t now_epoch) {
    if (!IsTournamentChannel() || !PolicyCache().get() || now_epoch == 0u ||
        now_epoch - g_last_expiry_sweep < 60u) return;
    g_last_expiry_sweep = now_epoch;
    CEnvironment* const environment = G_CEnvironment();
    const int group = environment ? environment->get_server_group() : -1;
    if (group >= 0 && group < kMaximumServerGroupCount) {
        Message request;
        std::memset(&request, 0, sizeof(request));
        request.operation = OPERATION_EXPIRE_SWEEP;
        request.server_group = group;
        request.reply_user_idx = -1;
        request.expires_epoch = now_epoch;
        (void)DBPvpTournament::QueueRequest(-1, request);
    }

    std::vector<PvP_Room*> stale;
    std::vector<TournamentRoomMatch*> reclaim;
    {
        MutexGuard guard(&g_tournament_mutex);
        for (std::map<PvP_Room*, uint32_t>::const_iterator it =
                 g_tokens_by_room.begin(); it != g_tokens_by_room.end(); ++it) {
            std::map<uint32_t, RoomBinding>::const_iterator binding =
                g_rooms_by_token.find(it->second);
            if (binding == g_rooms_by_token.end() || !it->first ||
                it->first->current_match_ != binding->second.adapter ||
                (binding->second.expires_epoch <= now_epoch &&
                 it->first->get_state() == 0)) {
                stale.push_back(it->first);
            }
        }
        for (std::vector<RetiredAdapter>::iterator it =
                 g_retired_adapters.begin();
             it != g_retired_adapters.end();) {
            if (it->release_epoch <= now_epoch) {
                reclaim.push_back(it->adapter);
                it = g_retired_adapters.erase(it);
            } else {
                ++it;
            }
        }
    }
    for (size_t index = 0u; index < stale.size(); ++index) {
        OnRoomReturned(stale[index]);
    }
    for (size_t index = 0u; index < reclaim.size(); ++index) {
        delete reclaim[index];
    }
}

void OnRoomReturned(PvP_Room* room) {
    if (!room) return;
    RoomBinding removed;
    bool have_binding = false;
    bool release_lease = false;
    {
        MutexGuard guard(&g_tournament_mutex);
        std::map<PvP_Room*, uint32_t>::iterator token =
            g_tokens_by_room.find(room);
        if (token == g_tokens_by_room.end()) return;
        std::map<uint32_t, RoomBinding>::iterator binding =
            g_rooms_by_token.find(token->second);
        if (binding != g_rooms_by_token.end()) {
            removed = binding->second;
            have_binding = true;
            if (removed.adapter) {
                release_lease = !removed.adapter->result_queued();
                removed.adapter->restore(room);
                RetiredAdapter retired;
                retired.adapter = removed.adapter;
                const uint32_t now_epoch = NowEpoch();
                retired.release_epoch = now_epoch == 0u
                    ? 0xffffffffu : now_epoch + 120u;
                g_retired_adapters.push_back(retired);
            }
            g_rooms_by_token.erase(binding);
        }
        g_tokens_by_room.erase(token);
    }
    if (have_binding && release_lease) {
        (void)QueueBackground(pvp_tournament::OPERATION_RELEASE_ROOM,
                              removed.tournament_id, removed.match_id,
                              removed.room_token, removed.server_group, 0u);
    }
}

bool IsPolicyLoaded() { return PolicyCache().get() != 0; }

}  // namespace pvp_tournament_0828

Dispatcher_PvpTournamentMatchList::
    Dispatcher_PvpTournamentMatchList()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_PvpTournamentMatchList::
    ~Dispatcher_PvpTournamentMatchList() {}

int Dispatcher_PvpTournamentMatchList::dispatch_sig(
    CUser* user, PacketBuf& packet) {
    unsigned int tournament_id = 0u;
    if (!PolicyCache().get()) {
        Send544Result(user, false, pvp_tournament::ERROR_STATE, 0u);
        return 0;
    }
    if (!IsTownReady(user)) {
        Send544Result(user, false, pvp_tournament::ERROR_PERMISSION, 0u);
        return 0;
    }
    if (packet.get_int(tournament_id) != 1 || tournament_id == 0u) {
        Send544Result(user, false, pvp_tournament::ERROR_PACKET, 0u);
        return 0;
    }

    pvp_tournament::Message request;
    std::memset(&request, 0, sizeof(request));
    request.tournament_id = static_cast<uint32_t>(tournament_id);
    if (!QueueClient(user, request,
                     pvp_tournament::OPERATION_OPEN_DETAIL, 0u,
                     pvp_tournament::COMMAND_544, true)) {
        Send544Result(user, false, pvp_tournament::ERROR_STATE, 0u);
    }
    return 0;
}

int Dispatcher_PvpTournamentMatchList::read(PacketBuf& packet,
                                                  MSG_BASE& message) {
    return NoRead(packet, message);
}

int Dispatcher_PvpTournamentMatchList::process(CUser* user,
                                                     MSG_BASE& message,
                                                     ParamBase& param) {
    return NoProcess(user, message, param);
}

Dispatcher_PvpTournamentRequest::Dispatcher_PvpTournamentRequest()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_PvpTournamentRequest::~Dispatcher_PvpTournamentRequest() {}

int Dispatcher_PvpTournamentRequest::dispatch_sig(CUser* user,
                                                        PacketBuf& packet) {
    pvp_tournament::Message request;
    std::memset(&request, 0, sizeof(request));
    uint8_t subtype = 0u;
    uint8_t parse_error = pvp_tournament::ERROR_PACKET;
    if (!Parse545(packet, request, subtype, parse_error)) {
        Send545Result(user, false, parse_error, subtype);
        return 0;
    }
    // The failure wire always carries the original request subtype.  Parse
    // the bounded body before policy/state gates so those gates never emit a
    // misleading subtype 0 for requests 1..11.
    if (!PolicyCache().get()) {
        Send545Result(user, false, pvp_tournament::ERROR_STATE,
                      subtype);
        return 0;
    }
    if (!IsTownReady(user)) {
        Send545Result(user, false, pvp_tournament::ERROR_PERMISSION,
                      subtype);
        return 0;
    }

    const TournamentPolicy* const policy = PolicyCache().get();
    if (subtype == 1u) {
        const uint32_t now_epoch = NowEpoch();
        if (!policy || request.title[0] == 0 ||
            request.qualification >
                static_cast<uint16_t>(policy->maximum_qualification)) {
            Send545Result(user, false, pvp_tournament::ERROR_PACKET,
                          subtype);
            return 0;
        }
        if (now_epoch == 0u || request.scheduled_epoch < now_epoch ||
            request.scheduled_epoch >
                now_epoch +
                    static_cast<uint32_t>(policy->maximum_schedule_lead)) {
            Send545Result(user, false, pvp_tournament::ERROR_SCHEDULE,
                          subtype);
            return 0;
        }
    }
    const uint8_t operation = OperationForSubtype(subtype);
    if (operation == 0u ||
        !QueueClient(user, request, operation, subtype,
                     pvp_tournament::COMMAND_545, true)) {
        Send545Result(user, false, pvp_tournament::ERROR_STATE,
                      subtype);
    }
    return 0;
}

int Dispatcher_PvpTournamentRequest::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    return NoRead(packet, message);
}

int Dispatcher_PvpTournamentRequest::process(CUser* user,
                                                   MSG_BASE& message,
                                                   ParamBase& param) {
    return NoProcess(user, message, param);
}

InterPvpTournament::InterPvpTournament()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              pvp_tournament::kTaiwanInnerCode)) {}

InterPvpTournament::~InterPvpTournament() {}

int InterPvpTournament::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    if (!payload ||
        payload_size != static_cast<int>(
            sizeof(pvp_tournament::Message))) {
        return 0;
    }
    const pvp_tournament::Message response =
        *reinterpret_cast<const pvp_tournament::Message*>(payload);
    if (response.operation <
            pvp_tournament::OPERATION_LOAD_REGISTRY ||
        response.operation >
            pvp_tournament::OPERATION_EXPIRE_SWEEP ||
        response.server_group < 0 ||
        response.server_group >=
            pvp_tournament::kMaximumServerGroupCount ||
        response.root_count >
            pvp_tournament::kMaximumTournamentCount ||
        response.participant_count >
            pvp_tournament::kMaximumParticipantCount ||
        response.match_count >
            pvp_tournament::kMaximumMatchCount ||
        response.error_code > pvp_tournament::ERROR_BRACKET ||
        response.command_kind > pvp_tournament::COMMAND_545 ||
        response.request_subtype > 11u) {
        return 0;
    }

    if (!user) {
        BroadcastBackground(response);
        return 0;
    }

    SessionState session;
    if (!CorrelateLiveResult(user, response, session)) return 0;

    if (response.error_code != 0u) {
        if (response.error_code == pvp_tournament::ERROR_ENDED &&
            response.tournament_id != 0u) {
            SendRootDelete(user, response.tournament_id);
        }
        if (response.command_kind == pvp_tournament::COMMAND_544) {
            Send544Result(user, false, response.error_code, 0u);
        } else if (response.command_kind ==
                       pvp_tournament::COMMAND_545) {
            Send545Result(user, false, response.error_code,
                          response.request_subtype);
        } else {
            TraceFailure("registry load failed", response.tournament_id,
                         response.match_id);
        }
        return 0;
    }

    if (response.operation ==
        pvp_tournament::OPERATION_LOAD_REGISTRY) {
        SendAggregate(user, response, true);
        return 0;
    }

    // Entering a match is a two-phase lease.  The room and exact roster are
    // materialized before the command succeeds; local failure immediately
    // releases the DB lease and returns the 0828 room-missing error body.
    if (response.operation ==
        pvp_tournament::OPERATION_RESERVE_ROOM) {
        if (!EnterTournamentRoom(user, response)) {
            QueueReleaseAfterRoomFailure(response);
            Send545Result(user, false,
                          pvp_tournament::ERROR_ROOM_MISSING,
                          response.request_subtype);
            return 0;
        }
        SendAggregate(user, response, false);
        BroadcastBackground(response, user);
        Send545Result(user, true, response.command_result,
                      response.request_subtype);
        return 0;
    }

    if (response.command_kind == pvp_tournament::COMMAND_544) {
        // CLIENT_IDA_EVIDENCE: NOTI604 owns the detail model and must be
        // applied before CMD544's small success body {1,tournament_id}.
        SendAggregate(user, response, false);
        Send544Result(user, true, 0u, response.tournament_id);
    } else if (response.command_kind ==
                   pvp_tournament::COMMAND_545) {
        SendAggregate(user, response, false);
        BroadcastBackground(response, user);
        Send545Result(user, true, response.command_result,
                      response.request_subtype);
    }
    return 0;
}
