#include "old_game/dungeon/instance/zombie_mode_result.h"

#include "infra/concurrent/thread/mutex.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/system_time.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_zombie_mode_result.h"
#include "old_game/player/core/cuser.h"
#include "old_game/progression/skill/combo_skill.h"
#include "old_game/pvp/arena/pvp_room.h"

#include <algorithm>
#include <climits>
#include <cstring>
#include <ctime>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace zombie_mode {
namespace {

const char kZombieModeConfig[] = "Etc/GameMode/ZombieMode.etc";
const uint32_t kResultTelemetryTimeoutMs = 10000u;
const uint32_t kTelemetrySurvivalToleranceMs = 2000u;

struct Policy {
    Policy()
        : ready_time_ms(0u), play_time_ms(0u), damage_divisor(0u),
          attack_points(0u), serious_attack_points(0u),
          kill_zombie_points(0u), kill_human_points(0u),
          survival_points_per_second(0u), last_survivor_points(0u),
          reward_item_id(0u), daily_limit(0u), immunity_item_id(0u),
          champion_level(0u), item_box_passive_index(0u),
          item_box_world_x(0u), item_box_world_y(0u),
          item_box_first_delay_ms(0u), item_box_repeat_delay_ms(0u),
          valid(false) {
        std::memset(play_modes, 0, sizeof(play_modes));
        std::memset(full_counts, 0, sizeof(full_counts));
        std::memset(limited_counts, 0, sizeof(limited_counts));
        std::memset(reward_exp, 0, sizeof(reward_exp));
    }

    uint32_t ready_time_ms;
    uint32_t play_time_ms;
    uint32_t play_modes[10];
    uint32_t damage_divisor;
    uint32_t attack_points;
    uint32_t serious_attack_points;
    uint32_t kill_zombie_points;
    uint32_t kill_human_points;
    uint32_t survival_points_per_second;
    uint32_t last_survivor_points;
    uint32_t full_counts[kMaximumSeats];
    uint32_t limited_counts[kMaximumSeats];
    uint32_t reward_exp[kMaximumSeats];
    uint32_t reward_item_id;
    uint32_t daily_limit;
    uint32_t immunity_item_id;
    uint32_t champion_level;
    uint32_t item_box_passive_index;
    uint16_t item_box_world_x;
    uint16_t item_box_world_y;
    uint32_t item_box_first_delay_ms;
    uint32_t item_box_repeat_delay_ms;
    bool valid;
};

enum RoundPhase {
    ROUND_READY = 0,
    ROUND_RUNNING = 1,
    ROUND_RESULT_PENDING = 2,
    ROUND_FINISHED = 3,
};

enum EntryState {
    ENTRY_IDLE = 0,
    ENTRY_QUEUED = 1,
    ENTRY_DURABLE = 2,
};

enum MemberLifeState {
    MEMBER_LIFE_INACTIVE = 0,
    MEMBER_LIFE_ACTIVE = 1,
    MEMBER_LIFE_ELIMINATED = 2,
};

struct RoundKey {
    RoundKey() : hi(0u), lo(0u) {}
    RoundKey(uint32_t high, uint32_t low) : hi(high), lo(low) {}
    bool operator<(const RoundKey& rhs) const {
        return hi < rhs.hi || (hi == rhs.hi && lo < rhs.lo);
    }
    uint32_t hi;
    uint32_t lo;
};

struct MemberState {
    MemberState()
        : user(0), account_id(0u), charac_no(0u), unique_id(0u), seat(0u),
          role(ROLE_HUMAN), life_state(MEMBER_LIFE_INACTIVE),
          immunity(false), transition_ack_pending(false),
          client_actor_key(0u), damage(0u), attacks(0u),
          serious_attacks(0u), killed_humans(0u), killed_zombies(0u),
          survival_seconds(0u), human_started_at(0u), attack_score(0u),
          survival_ms(0u), total_score(0u), rank(0u),
          last_survivor(false), telemetry_consistent(false),
          entry_state(ENTRY_IDLE), daily_ordinal(0u),
          settle_queued(false), committed(false),
          result_delivery_inflight(false), result_delivered(false),
          result_terminal_resolved(false),
          commit_response_valid(false), next_request_token(1u),
          entry_request(), settle_request(), commit_response() {
        std::memset(&entry_request, 0, sizeof(entry_request));
        std::memset(&settle_request, 0, sizeof(settle_request));
        std::memset(&commit_response, 0, sizeof(commit_response));
    }

    CUser* user;
    uint32_t account_id;
    uint32_t charac_no;
    uint16_t unique_id;
    uint16_t seat;
    uint8_t role;
    uint8_t life_state;
    bool immunity;
    bool transition_ack_pending;
    uint32_t client_actor_key;
    uint32_t damage;
    uint32_t attacks;
    uint32_t serious_attacks;
    uint32_t killed_humans;
    uint32_t killed_zombies;
    uint32_t survival_seconds;
    uint32_t human_started_at;
    uint32_t attack_score;
    uint32_t survival_ms;
    uint32_t total_score;
    uint8_t rank;
    bool last_survivor;
    bool telemetry_consistent;
    uint8_t entry_state;
    uint16_t daily_ordinal;
    bool settle_queued;
    bool committed;
    bool result_delivery_inflight;
    bool result_delivered;
    bool result_terminal_resolved;
    bool commit_response_valid;
    uint32_t next_request_token;
    Message entry_request;
    Message settle_request;
    Message commit_response;
};

struct RoundState {
    RoundState()
        : room(0), room_index(-1), key(), play_mode(0u), member_count(0u),
          initial_zombie_count(0u), phase(ROUND_READY),
          outcome(OUTCOME_DRAW), roles_selected(false),
          terminal_pending(false), pending_outcome(OUTCOME_DRAW),
          pending_last_survivor(0),
          ready_deadline_ms(0u), active_mask(0u),
          telemetry_received_mask(0u), telemetry_deadline_ms(0u),
          telemetry_frozen(false), telemetry_quorum(false),
          telemetry_continuation_queued(false),
          delivery_continuation_queued(false), members() {
        std::memset(telemetry_attack_score, 0,
                    sizeof(telemetry_attack_score));
        std::memset(telemetry_total_score, 0,
                    sizeof(telemetry_total_score));
    }
    PvP_Room* room;
    int room_index;
    RoundKey key;
    uint8_t play_mode;
    uint8_t member_count;
    uint8_t initial_zombie_count;
    uint8_t phase;
    uint8_t outcome;
    bool roles_selected;
    bool terminal_pending;
    uint8_t pending_outcome;
    CUser* pending_last_survivor;
    uint32_t ready_deadline_ms;
    uint8_t active_mask;
    uint8_t telemetry_received_mask;
    uint32_t telemetry_deadline_ms;
    bool telemetry_frozen;
    bool telemetry_quorum;
    bool telemetry_continuation_queued;
    bool delivery_continuation_queued;
    uint32_t telemetry_attack_score[kMaximumSeats][kMaximumSeats];
    uint32_t telemetry_total_score[kMaximumSeats][kMaximumSeats];
    MemberState members[kMaximumSeats];
};

struct PendingClaim {
    PendingClaim()
        : user(0), account_id(0u), charac_no(0u), server_group(-1),
          request(), applying(false), recovery(false) {
        std::memset(&request, 0, sizeof(request));
    }
    CUser* user;
    uint32_t account_id;
    uint32_t charac_no;
    int server_group;
    Message request;
    bool applying;
    bool recovery;
};

struct ClientTelemetry {
    ClientTelemetry()
        : seat(0u), killed_zombies(0u), killed_humans(0u),
          final_status(0u), attack_score(0u), survival_ms(0u),
          total_score(0u) {}
    uint16_t seat;
    uint8_t killed_zombies;
    uint8_t killed_humans;
    uint8_t final_status;
    uint32_t attack_score;
    uint32_t survival_ms;
    uint32_t total_score;
};

struct ResultWireRow {
    ResultWireRow()
        : seat(0u), rank_1based(0u), killed_zombies(0u),
          killed_humans(0u), final_status(0u), attack_score(0u),
          survival_ms(0u), total_score(0u), limited_reward(0u),
          tail0(0u), tail1(0u), reward_item_id(0u),
          reward_item_count(0u), reward_exp(0u) {}
    uint16_t seat;
    uint8_t rank_1based;
    uint8_t killed_zombies;
    uint8_t killed_humans;
    uint8_t final_status;
    uint32_t attack_score;
    uint32_t survival_ms;
    uint32_t total_score;
    uint8_t limited_reward;
    uint32_t tail0;
    uint16_t tail1;
    uint32_t reward_item_id;
    uint16_t reward_item_count;
    uint32_t reward_exp;
};

struct ResultDeliverySnapshot {
    ResultDeliverySnapshot() : count(0u) {}
    uint8_t count;
    ResultWireRow rows[kMaximumSeats];
};

struct ResultDeliveryWork {
    ResultDeliveryWork()
        : key(), room(0), account_id(0u), charac_no(0u),
          unique_id(0u), snapshot() {}
    RoundKey key;
    PvP_Room* room;
    uint32_t account_id;
    uint32_t charac_no;
    uint16_t unique_id;
    ResultDeliverySnapshot snapshot;
};

Mutex g_zombie_mutex;
Policy g_policy;
std::map<RoundKey, RoundState> g_rounds;
std::map<PvP_Room*, RoundKey> g_rooms;
std::map<PvP_Room*, bool> g_finished_rooms;
std::map<PvP_Room*, bool> g_result_delivery_complete_rooms;
std::map<CUser*, RoundKey> g_current_round;
std::map<CUser*, PendingClaim> g_pending_claims;
std::map<CUser*, Message> g_recovery_requests;
uint32_t g_round_counter = 1u;
uint32_t g_request_counter = 1u;

uint32_t NextNonZero(uint32_t& value) {
    uint32_t result = value++;
    if (result == 0u) result = value++;
    if (value == 0u) ++value;
    return result;
}

uint32_t SaturatingAdd(uint32_t current, uint32_t increment) {
    return increment > std::numeric_limits<uint32_t>::max() - current
               ? std::numeric_limits<uint32_t>::max()
               : current + increment;
}

uint8_t BoundedByte(uint32_t value) {
    return static_cast<uint8_t>(value > 255u ? 255u : value);
}

uint8_t SeatBit(size_t seat) {
    return seat < kMaximumSeats
               ? static_cast<uint8_t>(1u << seat)
               : 0u;
}

uint32_t NowSeconds() {
    const std::time_t now = std::time(0);
    return now > 0 ? static_cast<uint32_t>(now) : 1u;
}

uint32_t NowMilliseconds() {
    return static_cast<uint32_t>(
        GlobalData::s_systemTime_.getCurTickCount());
}

bool MillisecondDeadlineReached(uint32_t now, uint32_t deadline) {
    return deadline != 0u &&
           static_cast<int32_t>(now - deadline) >= 0;
}

uint32_t AddMillisecondDelay(uint32_t now, uint32_t delay) {
    const uint32_t deadline = now + delay;
    return deadline ? deadline : 1u;
}

void AccumulateHumanSurvival(MemberState& member, uint32_t now) {
    if (member.role != ROLE_HUMAN || member.human_started_at == 0u ||
        now <= member.human_started_at) return;
    const uint32_t maximum = g_policy.play_time_ms / 1000u;
    const uint32_t elapsed = now - member.human_started_at;
    member.survival_seconds = std::min(
        maximum, SaturatingAdd(member.survival_seconds, elapsed));
    member.human_started_at = now;
}

bool ReadInteger(int& output) {
    bool success = false;
    output = ScanInt(&success);
    return success;
}

bool ReadUnsigned(uint32_t& output) {
    int value = 0;
    if (!ReadInteger(value) || value < 0) return false;
    output = static_cast<uint32_t>(value);
    return true;
}

bool ReadFixedArray(uint32_t* output, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        if (!ReadUnsigned(output[index])) return false;
    }
    return true;
}

bool EqualArray(const uint32_t* lhs,
                const uint32_t* rhs,
                size_t count) {
    for (size_t index = 0; index < count; ++index) {
        if (lhs[index] != rhs[index]) return false;
    }
    return true;
}

bool ResolvePlayModeLocked(uint8_t player_count,
                           ZombiePlayModePolicy& output) {
    if (!g_policy.valid) return false;
    for (size_t offset = 0u; offset < 10u; offset += 5u) {
        if (g_policy.play_modes[offset] != player_count) continue;
        if (g_policy.play_modes[offset + 2u] == 0u ||
            g_policy.play_modes[offset + 2u] > player_count ||
            g_policy.play_modes[offset + 2u] > 0xffu) return false;
        std::memset(&output, 0, sizeof(output));
        output.player_count = player_count;
        output.initial_zombie_count = static_cast<uint8_t>(
            g_policy.play_modes[offset + 2u]);
        output.dungeon_index = g_policy.play_modes[offset + 1u];
        // ZombieMode.etc points at two monster.lst archetypes used as player
        // forms: the four-skill mother form, then the three-skill infected
        // form.  They are never passed to a server monster-spawn owner.
        output.mother_zombie_form_id = g_policy.play_modes[offset + 3u];
        output.infected_zombie_form_id = g_policy.play_modes[offset + 4u];
        return true;
    }
    return false;
}

bool ParsePointInfo(Policy& parsed) {
    std::string tag;
    unsigned int mask = 0u;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/point info]") return mask == 0x7fu;
        uint32_t* destination = 0;
        unsigned int bit = 0u;
        if (tag == "[damage point]") {
            destination = &parsed.damage_divisor; bit = 0x01u;
        } else if (tag == "[attack point]") {
            destination = &parsed.attack_points; bit = 0x02u;
        } else if (tag == "[serious attack point]") {
            destination = &parsed.serious_attack_points; bit = 0x04u;
        } else if (tag == "[kill zombie point]") {
            destination = &parsed.kill_zombie_points; bit = 0x08u;
        } else if (tag == "[kill human point]") {
            destination = &parsed.kill_human_points; bit = 0x10u;
        } else if (tag == "[survival point per sec]") {
            destination = &parsed.survival_points_per_second; bit = 0x20u;
        } else if (tag == "[last survival point]") {
            destination = &parsed.last_survivor_points; bit = 0x40u;
        } else {
            return false;
        }
        if ((mask & bit) != 0u || !ReadUnsigned(*destination)) return false;
        mask |= bit;
    }
    return false;
}

bool ParseAndValidatePolicy(Policy& parsed) {
    bool ready = false;
    bool play_time = false;
    bool modes = false;
    bool points = false;
    bool full = false;
    bool limited = false;
    bool experience = false;
    bool daily = false;
    bool immunity = false;
    bool champion = false;
    bool human_blind = false;
    bool item_box = false;

    if (loadRDARScriptFile("", kZombieModeConfig) != 1) return false;
    std::string tag;
    bool malformed = false;
    while (!malformed && ScanType(tag, true) == 1) {
        if (tag == "[ready time]") {
            malformed = ready || !ReadUnsigned(parsed.ready_time_ms);
            ready = !malformed;
        } else if (tag == "[play time]") {
            malformed = play_time || !ReadUnsigned(parsed.play_time_ms);
            play_time = !malformed;
        } else if (tag == "[play mode]") {
            malformed = modes || !ReadFixedArray(parsed.play_modes, 10u) ||
                        ScanType(tag, true) != 1 || tag != "[/play mode]";
            modes = !malformed;
        } else if (tag == "[point info]") {
            malformed = points || !ParsePointInfo(parsed);
            points = !malformed;
        } else if (tag == "[reward item]") {
            uint32_t values[16] = {};
            malformed = full || !ReadFixedArray(values, 16u) ||
                        ScanType(tag, true) != 1 || tag != "[/reward item]";
            if (!malformed) {
                malformed = values[0] == 0u;
                parsed.reward_item_id = values[0];
            }
            for (size_t index = 0; !malformed && index < kMaximumSeats;
                 ++index) {
                malformed = values[index * 2u] != parsed.reward_item_id;
                parsed.full_counts[index] = values[index * 2u + 1u];
            }
            full = !malformed;
        } else if (tag == "[reward item limit]") {
            uint32_t values[16] = {};
            malformed = limited || !ReadFixedArray(values, 16u) ||
                        ScanType(tag, true) != 1 ||
                        tag != "[/reward item limit]";
            for (size_t index = 0; !malformed && index < kMaximumSeats;
                 ++index) {
                malformed = parsed.reward_item_id == 0u ||
                            values[index * 2u] != parsed.reward_item_id;
                parsed.limited_counts[index] = values[index * 2u + 1u];
            }
            limited = !malformed;
        } else if (tag == "[reward exp]") {
            malformed = experience ||
                        !ReadFixedArray(parsed.reward_exp, kMaximumSeats) ||
                        ScanType(tag, true) != 1 || tag != "[/reward exp]";
            experience = !malformed;
        } else if (tag == "[limit reward]") {
            malformed = daily || !ReadUnsigned(parsed.daily_limit);
            daily = !malformed;
        } else if (tag == "[exp ticket item]") {
            malformed = immunity || !ReadUnsigned(parsed.immunity_item_id);
            immunity = !malformed;
        } else if (tag == "[zombie champion level]") {
            malformed = champion || !ReadUnsigned(parsed.champion_level);
            champion = !malformed;
        } else if (tag == "[human blind]") {
            uint32_t values[3] = {};
            unsigned int mask = 0u;
            while (!malformed && ScanType(tag, true) == 1 &&
                   tag != "[/human blind]") {
                unsigned int slot = 3u;
                if (tag == "[base vision]") slot = 0u;
                else if (tag == "[sub vision]") slot = 1u;
                else if (tag == "[added stuck]") slot = 2u;
                malformed = slot >= 3u || (mask & (1u << slot)) != 0u ||
                            !ReadUnsigned(values[slot]);
                if (!malformed) mask |= 1u << slot;
            }
            human_blind = !malformed && mask == 7u &&
                          values[0] == 0u && values[1] == 80u &&
                          values[2] == 20u;
            malformed = !human_blind;
        } else if (tag == "[random generation itembox]") {
            uint32_t passive = 0u, x = 0u, y = 0u, first = 0u, term = 0u;
            unsigned int mask = 0u;
            while (!malformed && ScanType(tag, true) == 1 &&
                   tag != "[/random generation itembox]") {
                if (tag == "[index]") {
                    malformed = (mask & 1u) != 0u || !ReadUnsigned(passive);
                    mask |= 1u;
                } else if (tag == "[pos]") {
                    malformed = (mask & 2u) != 0u || !ReadUnsigned(x) ||
                                !ReadUnsigned(y);
                    mask |= 2u;
                } else if (tag == "[time after]") {
                    malformed = (mask & 4u) != 0u || !ReadUnsigned(first);
                    mask |= 4u;
                } else if (tag == "[make term]") {
                    malformed = (mask & 8u) != 0u || !ReadUnsigned(term);
                    mask |= 8u;
                } else {
                    malformed = true;
                }
            }
            item_box = !malformed && mask == 15u && passive == 18303u &&
                       x == 300u && y == 300u && first == 120000u &&
                       term == 25000u;
            if (item_box) {
                parsed.item_box_passive_index = passive;
                parsed.item_box_world_x = static_cast<uint16_t>(x);
                parsed.item_box_world_y = static_cast<uint16_t>(y);
                parsed.item_box_first_delay_ms = first;
                parsed.item_box_repeat_delay_ms = term;
            }
            malformed = !item_box;
        }
    }
    unloadRDARScriptFile();

    static const uint32_t expected_modes[10] = {
        5u, 3101u, 2u, 61659u, 61660u,
        6u, 3101u, 2u, 61657u, 61658u,
    };
    static const uint32_t expected_full[kMaximumSeats] = {
        13u, 10u, 7u, 5u, 2u, 1u, 1u, 1u,
    };
    static const uint32_t expected_limited[kMaximumSeats] = {
        4u, 3u, 2u, 1u, 1u, 1u, 1u, 1u,
    };
    static const uint32_t expected_exp[kMaximumSeats] = {
        200u, 150u, 120u, 100u, 50u, 25u, 25u, 25u,
    };
    parsed.valid = !malformed && ready && play_time && modes && points &&
                   full && limited && experience && daily && immunity &&
                   champion && human_blind && item_box &&
                   parsed.ready_time_ms == 5000u &&
                   parsed.play_time_ms == 300000u &&
                   EqualArray(parsed.play_modes, expected_modes, 10u) &&
                   parsed.damage_divisor == 100u &&
                   parsed.attack_points == 5u &&
                   parsed.serious_attack_points == 100u &&
                   parsed.kill_zombie_points == 50u &&
                   parsed.kill_human_points == 50u &&
                   parsed.survival_points_per_second == 2u &&
                   parsed.last_survivor_points == 500u &&
                   EqualArray(parsed.full_counts, expected_full,
                              kMaximumSeats) &&
                   EqualArray(parsed.limited_counts, expected_limited,
                              kMaximumSeats) &&
                   EqualArray(parsed.reward_exp, expected_exp,
                              kMaximumSeats) &&
                   parsed.reward_item_id == kRewardItemId &&
                   parsed.daily_limit == kDailyFullRewardLimit &&
                   parsed.immunity_item_id == kImmunityItemId &&
                   parsed.champion_level == 16u;
    return parsed.valid;
}

void TraceFailure(const char* reason, CUser* user) {
    cMyTrace trace("ZombieMode0828", __LINE__, 5);
    trace("[0828 zombie] %s account(%u) charac(%u)",
          reason ? reason : "unknown",
          static_cast<unsigned int>(
              user && user->get_acc_id() > 0 ? user->get_acc_id() : 0),
          static_cast<unsigned int>(
              user && user->getCurCharacNo() > 0
                  ? user->getCurCharacNo() : 0));
}

bool IsSelectedCharacter(CUser* user) {
    return user && user->getCurCharacR() && user->get_acc_id() > 0 &&
           user->getCurCharacNo() > 0 && user->GetServerGroup() >= 0 &&
           user->GetServerGroup() < kMaximumServerGroupCount;
}

bool HasImmunityItem(CUser* user) {
    if (!IsSelectedCharacter(user) || !g_policy.valid ||
        g_policy.immunity_item_id == 0u) return false;
    CInventory* const inventory = user->getCurCharacInvenR();
    Inven_Item item;
    return inventory &&
           inventory->GetInvenData(
               static_cast<int>(g_policy.immunity_item_id), item) != -1 &&
           item.item_id_ ==
               static_cast<unsigned long>(g_policy.immunity_item_id) &&
           item.add_info_ > 0;
}

RoundKey MakeRoundKey(PvP_Room* room, int server_group) {
    const uint64_t values[] = {
        static_cast<uint64_t>(static_cast<uint32_t>(server_group)),
        static_cast<uint64_t>(static_cast<uint32_t>(room->get_index())),
        static_cast<uint64_t>(static_cast<uint32_t>(std::time(0))),
        static_cast<uint64_t>(NextNonZero(g_round_counter)),
    };
    uint64_t hash = 14695981039346656037ULL;
    for (size_t index = 0; index < 4u; ++index) {
        uint64_t value = values[index];
        for (unsigned int byte = 0u; byte < 8u; ++byte) {
            hash ^= value & 0xffu;
            hash *= 1099511628211ULL;
            value >>= 8u;
        }
    }
    uint32_t hi = static_cast<uint32_t>(hash >> 32u);
    uint32_t lo = static_cast<uint32_t>(hash & 0xffffffffULL);
    if (hi == 0u) hi = 1u;
    if (lo == 0u) lo = 1u;
    return RoundKey(hi, lo);
}

bool IdentityMatches(CUser* user, const MemberState& member) {
    return IsSelectedCharacter(user) && member.user == user &&
           member.account_id == static_cast<uint32_t>(user->get_acc_id()) &&
           member.charac_no ==
               static_cast<uint32_t>(user->getCurCharacNo()) &&
           member.unique_id == static_cast<uint16_t>(user->get_unique_id());
}

MemberState* FindMember(RoundState& round, CUser* user) {
    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        if (round.members[seat].user == user) return &round.members[seat];
    }
    return 0;
}

MemberState* FindMemberByCharacter(RoundState& round,
                                   uint32_t account_id,
                                   uint32_t charac_no) {
    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        MemberState& member = round.members[seat];
        if (member.account_id == account_id && member.charac_no == charac_no)
            return &member;
    }
    return 0;
}

std::map<RoundKey, RoundState>::iterator FindCurrentRoundLocked(CUser* user) {
    std::map<CUser*, RoundKey>::const_iterator key =
        g_current_round.find(user);
    if (key == g_current_round.end()) return g_rounds.end();
    return g_rounds.find(key->second);
}

void FillIdentity(MemberState& member,
                  const RoundState& round,
                  uint8_t operation,
                  Message& message) {
    std::memset(&message, 0, sizeof(message));
    message.account_id = member.account_id;
    message.charac_no = member.charac_no;
    message.round_token_hi = round.key.hi;
    message.round_token_lo = round.key.lo;
    message.request_token = NextNonZero(member.next_request_token);
    message.server_group = member.user->GetServerGroup();
    message.reply_user_idx = member.user->GetUID();
    message.operation = operation;
    message.seat = member.seat;
    message.role = member.role;
    message.immunity_at_entry = member.immunity ? 1u : 0u;
}

bool QueueMessage(CUser* user, const Message& message) {
    return user && DBZombieModeResult::QueueRequest(
                       user->GetUID(), message);
}

void BuildEntryRequest(MemberState& member,
                       const RoundState& round,
                       Message& request) {
    FillIdentity(member, round, OPERATION_BEGIN_ENTRY, request);
}

void BuildSettleRequest(MemberState& member,
                        const RoundState& round,
                        Message& request) {
    FillIdentity(member, round, OPERATION_SETTLE, request);
    request.daily_ordinal = member.daily_ordinal;
    request.outcome = round.outcome;
    request.rank = member.rank;
    request.last_survivor = member.last_survivor ? 1u : 0u;
    request.telemetry_consistent = member.telemetry_consistent ? 1u : 0u;
    // SQL026 keeps its deployed 140-byte ABI/schema.  Phase4C stores the
    // frozen attack-score scalar in the legacy damage column; the two raw
    // client-hit counters have no trusted producer and remain zero.
    request.damage = member.attack_score;
    request.attacks = 0u;
    request.serious_attacks = 0u;
    request.killed_humans = member.killed_humans;
    request.killed_zombies = member.killed_zombies;
    request.survival_seconds = member.survival_ms / 1000u;
    request.total_score = member.total_score;
    const bool limited =
        member.daily_ordinal > g_policy.daily_limit && !member.immunity;
    request.limited_reward = limited ? 1u : 0u;
    request.reward_item_id = g_policy.reward_item_id;
    request.reward_item_count = static_cast<uint16_t>(
        limited ? g_policy.limited_counts[member.rank]
                : g_policy.full_counts[member.rank]);
    request.reward_exp = g_policy.reward_exp[member.rank];
}

bool CaptureSnapshot(CUser* user, CharacterSnapshot& snapshot) {
    if (!IsSelectedCharacter(user)) return false;
    SkillSlot* const skills = user->getCurCharacSkillR();
    const int level = user->get_charac_level();
    const int experience = user->getCurCharacExp();
    if (!skills || level <= 0 || experience < 0) return false;
    const int sp0 = skills->get_remain_sp_at_index(ENUM_SKILL_TREE_0);
    const int sp1 = skills->get_remain_sp_at_index(ENUM_SKILL_TREE_1);
    const int sfp0 = skills->get_remain_sfp_at_index(ENUM_SKILL_TREE_SFP_0);
    const int sfp1 = skills->get_remain_sfp_at_index(ENUM_SKILL_TREE_SFP_1);
    if (sp0 < 0 || sp1 < 0 || sfp0 < 0 || sfp1 < 0) return false;
    snapshot.level = static_cast<uint32_t>(level);
    snapshot.experience = static_cast<uint32_t>(experience);
    snapshot.remain_sp_first = static_cast<uint32_t>(sp0);
    snapshot.remain_sp_second = static_cast<uint32_t>(sp1);
    snapshot.remain_sfp_first = static_cast<uint32_t>(sfp0);
    snapshot.remain_sfp_second = static_cast<uint32_t>(sfp1);
    return true;
}

bool EqualSnapshot(const CharacterSnapshot& lhs,
                   const CharacterSnapshot& rhs) {
    return lhs.level == rhs.level && lhs.experience == rhs.experience &&
           lhs.remain_sp_first == rhs.remain_sp_first &&
           lhs.remain_sp_second == rhs.remain_sp_second &&
           lhs.remain_sfp_first == rhs.remain_sfp_first &&
           lhs.remain_sfp_second == rhs.remain_sfp_second;
}

bool ComputeTarget(const CharacterSnapshot& baseline,
                   uint32_t amount,
                   CharacterSnapshot& target) {
    if (amount == 0u || amount > 1000u || baseline.level == 0u ||
        baseline.level > kMaximumCharacterLevel ||
        baseline.experience > static_cast<uint32_t>(INT_MAX)) return false;
    CDataManager* const manager = G_CDataManager();
    if (!manager) return false;
    uint32_t level = baseline.level;
    int experience = static_cast<int>(baseline.experience);
    int remaining = static_cast<int>(amount);
    int gained_sp = 0;
    int gained_sfp = 0;
    while (remaining > 0 && level < kMaximumCharacterLevel) {
        const int bottom = manager->get_level_exp(static_cast<int>(level));
        const int top = manager->get_level_exp(static_cast<int>(level + 1u));
        if (bottom < 0 || top <= bottom || experience < bottom ||
            experience > top) return false;
        const int needed = top - experience;
        if (remaining < needed) {
            experience += remaining;
            remaining = 0;
            break;
        }
        remaining -= needed;
        experience = top;
        ++level;
        const int level_sp = manager->GetSpAtLevelUp(static_cast<int>(level));
        if (level_sp < 0 || gained_sp > INT_MAX - level_sp) return false;
        gained_sp += level_sp;
        if (level > 49u) ++gained_sfp;
    }
    target = baseline;
    target.level = level;
    target.experience = static_cast<uint32_t>(experience);
    target.remain_sp_first = baseline.remain_sp_first + gained_sp;
    target.remain_sp_second = baseline.remain_sp_second + gained_sp;
    target.remain_sfp_first = baseline.remain_sfp_first + gained_sfp;
    target.remain_sfp_second = baseline.remain_sfp_second + gained_sfp;
    return true;
}

bool ApplyReservedReward(CUser* user,
                         const Message& reserve,
                         CharacterSnapshot& target) {
    CharacterSnapshot expected;
    CharacterSnapshot current;
    if (!ComputeTarget(reserve.baseline, reserve.reward_exp, expected) ||
        !CaptureSnapshot(user, current)) return false;
    if (reserve.claim_state == CLAIM_STATE_TARGET_STORED &&
        EqualSnapshot(current, reserve.target)) {
        target = current;
        return EqualSnapshot(expected, target);
    }
    if (!EqualSnapshot(current, reserve.baseline)) return false;
    int gained_sp = 0, gained_sfp = 0, old_sp = 0, new_sp = 0;
    user->rewardExp(static_cast<int>(reserve.reward_exp), gained_sp,
                    gained_sfp, old_sp, new_sp,
                    static_cast<eExpAddReason>(2), true);
    return CaptureSnapshot(user, target) &&
           EqualSnapshot(target, expected) &&
           (reserve.claim_state != CLAIM_STATE_TARGET_STORED ||
            EqualSnapshot(target, reserve.target));
}

bool SendResultNotice(CUser* user,
                      const ResultDeliverySnapshot& result) {
    if (!user || result.count < kMinimumPlayers ||
        result.count > kMaximumPlayers) return false;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_ZOMBIE_MODE_RESULT_SCORE);
    // CLIENT_IDA_EVIDENCE: ClientZombieResult_HandleNoti475 @ 0x534350
    // consumes u8 count followed by exact 35-byte rows.  Five-player rows
    // therefore produce 176 bytes and six-player rows 211 bytes.  Physical
    // seats are retained; a degraded missing seat is omitted, never compacted.
    packet.put_byte(result.count);
    for (size_t index = 0u; index < result.count; ++index) {
        const ResultWireRow& row = result.rows[index];
        packet.put_short(row.seat);
        packet.put_byte(row.rank_1based);
        packet.put_byte(row.killed_zombies);
        packet.put_byte(row.killed_humans);
        packet.put_byte(row.final_status);
        packet.put_int(static_cast<int>(row.attack_score));
        packet.put_int(static_cast<int>(row.survival_ms));
        packet.put_int(static_cast<int>(row.total_score));
        packet.put_byte(row.limited_reward);
        packet.put_int(static_cast<int>(row.tail0));
        packet.put_short(row.tail1);
        packet.put_int(static_cast<int>(row.reward_item_id));
        packet.put_short(row.reward_item_count);
        packet.put_int(static_cast<int>(row.reward_exp));
    }
    packet.finalize(true);
    return user->Send(packet) != 0;
}

uint32_t ServerSurvivalMilliseconds(const MemberState& member) {
    const uint64_t milliseconds =
        static_cast<uint64_t>(member.survival_seconds) * 1000u;
    return static_cast<uint32_t>(std::min<uint64_t>(
        milliseconds, static_cast<uint64_t>(g_policy.play_time_ms)));
}

uint32_t CalculateScoreFromAttack(const MemberState& member,
                                  uint32_t attack_score) {
    uint64_t score = attack_score;
    score += static_cast<uint64_t>(member.killed_zombies) *
             g_policy.kill_zombie_points;
    score += static_cast<uint64_t>(member.killed_humans) *
             g_policy.kill_human_points;
    score += static_cast<uint64_t>(member.survival_seconds) *
             g_policy.survival_points_per_second;
    if (member.last_survivor) score += g_policy.last_survivor_points;
    return score > std::numeric_limits<uint32_t>::max()
               ? std::numeric_limits<uint32_t>::max()
               : static_cast<uint32_t>(score);
}

bool BuildResultDeliverySnapshotLocked(
        const RoundState& round,
        const MemberState& recipient,
        const Message* committed_reward,
        ResultDeliverySnapshot& output) {
    output = ResultDeliverySnapshot();
    bool recipient_found = false;
    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        const MemberState& member = round.members[seat];
        if ((round.active_mask & SeatBit(seat)) == 0u) continue;
        if (output.count >= kMaximumPlayers) return false;
        ResultWireRow& row = output.rows[output.count++];
        row.seat = member.seat;
        row.rank_1based = static_cast<uint8_t>(member.rank + 1u);
        row.killed_zombies = BoundedByte(member.killed_zombies);
        row.killed_humans = BoundedByte(member.killed_humans);
        row.final_status = member.role == ROLE_ZOMBIE ? 1u : 0u;
        row.attack_score = member.attack_score;
        row.survival_ms = member.survival_ms;
        row.total_score = member.total_score;
        if (member.seat == recipient.seat) {
            recipient_found = true;
            if (committed_reward) {
                row.limited_reward = committed_reward->limited_reward;
                row.tail0 = committed_reward->total_score;
                row.tail1 = committed_reward->daily_ordinal;
                row.reward_item_id = committed_reward->reward_item_id;
                row.reward_item_count = committed_reward->reward_item_count;
                row.reward_exp = committed_reward->reward_exp;
            }
        }
    }
    return recipient_found && output.count == round.member_count;
}

bool RankLess(const std::pair<uint32_t, uint16_t>& lhs,
              const std::pair<uint32_t, uint16_t>& rhs) {
    return lhs.first > rhs.first ||
           (lhs.first == rhs.first && lhs.second < rhs.second);
}

bool AllCommitted(const RoundState& round) {
    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        if (round.members[seat].account_id != 0u &&
            !round.members[seat].committed) return false;
    }
    return true;
}

bool AllResultsResolved(const RoundState& round) {
    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        if (round.members[seat].account_id != 0u &&
            !round.members[seat].result_terminal_resolved) return false;
    }
    return true;
}

size_t CountSeatMask(uint8_t mask) {
    size_t count = 0u;
    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        if ((mask & SeatBit(seat)) != 0u) ++count;
    }
    return count;
}

bool SurvivalWithinTolerance(uint32_t client_ms, uint32_t server_ms) {
    const uint32_t difference = client_ms > server_ms
        ? client_ms - server_ms
        : server_ms - client_ms;
    return difference <= kTelemetrySurvivalToleranceMs;
}

bool ValidateTelemetrySnapshotLocked(
        RoundState& round,
        CUser* sender,
        uint8_t outcome,
        const std::vector<ClientTelemetry>& telemetry,
        uint16_t& sender_seat) {
    sender_seat = 0xffffu;
    if (!sender || round.phase != ROUND_RESULT_PENDING ||
        round.telemetry_frozen || round.outcome != outcome ||
        telemetry.size() != round.member_count ||
        CountSeatMask(round.active_mask) != round.member_count) {
        return false;
    }
    MemberState* const source = FindMember(round, sender);
    if (!source || !IdentityMatches(sender, *source) ||
        source->seat >= kMaximumSeats ||
        (round.active_mask & SeatBit(source->seat)) == 0u ||
        (round.telemetry_received_mask & SeatBit(source->seat)) != 0u) {
        return false;
    }

    uint8_t row_mask = 0u;
    for (size_t index = 0u; index < telemetry.size(); ++index) {
        const ClientTelemetry& row = telemetry[index];
        if (row.seat >= kMaximumSeats) return false;
        const uint8_t bit = SeatBit(row.seat);
        if ((round.active_mask & bit) == 0u || (row_mask & bit) != 0u)
            return false;
        const MemberState& authoritative = round.members[row.seat];
        const uint32_t server_survival =
            ServerSurvivalMilliseconds(authoritative);
        const uint8_t server_status =
            authoritative.role == ROLE_ZOMBIE ? 1u : 0u;
        if (authoritative.account_id == 0u ||
            row.killed_zombies !=
                BoundedByte(authoritative.killed_zombies) ||
            row.killed_humans !=
                BoundedByte(authoritative.killed_humans) ||
            row.final_status != server_status ||
            !SurvivalWithinTolerance(row.survival_ms,
                                     server_survival) ||
            row.total_score !=
                CalculateScoreFromAttack(authoritative,
                                         row.attack_score)) {
            return false;
        }
        round.telemetry_attack_score[source->seat][row.seat] =
            row.attack_score;
        round.telemetry_total_score[source->seat][row.seat] =
            row.total_score;
        row_mask = static_cast<uint8_t>(row_mask | bit);
    }
    if (row_mask != round.active_mask) return false;
    round.telemetry_received_mask = static_cast<uint8_t>(
        round.telemetry_received_mask | SeatBit(source->seat));
    sender_seat = source->seat;
    return true;
}

bool TryFreezeTelemetryQuorumLocked(RoundState& round) {
    if (round.phase != ROUND_RESULT_PENDING || round.telemetry_frozen)
        return false;
    const size_t quorum = round.member_count == kMaximumPlayers ? 4u : 3u;
    if (CountSeatMask(round.telemetry_received_mask) < quorum) return false;

    uint32_t accepted_attack[kMaximumSeats] = {0};
    uint32_t accepted_total[kMaximumSeats] = {0};
    for (size_t target = 0u; target < kMaximumSeats; ++target) {
        if ((round.active_mask & SeatBit(target)) == 0u) continue;
        bool found = false;
        for (size_t candidate = 0u;
             candidate < kMaximumSeats && !found;
             ++candidate) {
            if ((round.telemetry_received_mask & SeatBit(candidate)) == 0u)
                continue;
            const uint32_t attack =
                round.telemetry_attack_score[candidate][target];
            const uint32_t total =
                round.telemetry_total_score[candidate][target];
            size_t votes = 0u;
            for (size_t voter = 0u; voter < kMaximumSeats; ++voter) {
                if ((round.telemetry_received_mask & SeatBit(voter)) == 0u)
                    continue;
                if (round.telemetry_attack_score[voter][target] == attack &&
                    round.telemetry_total_score[voter][target] == total) {
                    ++votes;
                }
            }
            if (votes >= quorum) {
                accepted_attack[target] = attack;
                accepted_total[target] = total;
                found = true;
            }
        }
        if (!found) return false;
    }

    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        MemberState& member = round.members[seat];
        if ((round.active_mask & SeatBit(seat)) == 0u) continue;
        member.attack_score = accepted_attack[seat];
        member.survival_ms = ServerSurvivalMilliseconds(member);
        member.total_score = accepted_total[seat];
        member.telemetry_consistent = true;
    }
    round.telemetry_frozen = true;
    round.telemetry_quorum = true;
    round.telemetry_deadline_ms = 0u;
    return true;
}

bool FreezeTelemetryTimeoutLocked(RoundState& round) {
    if (round.phase != ROUND_RESULT_PENDING || round.telemetry_frozen)
        return false;
    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        MemberState& member = round.members[seat];
        if ((round.active_mask & SeatBit(seat)) == 0u) continue;
        member.attack_score = 0u;
        member.survival_ms = ServerSurvivalMilliseconds(member);
        member.total_score = CalculateScoreFromAttack(member, 0u);
        member.telemetry_consistent = false;
    }
    round.telemetry_frozen = true;
    round.telemetry_quorum = false;
    round.telemetry_deadline_ms = 0u;
    return true;
}

bool PrepareFrozenResultSettlementLocked(
        RoundState& round,
        std::vector<std::pair<CUser*, Message> >& retry_entries,
        std::vector<CUser*>& settle_users) {
    if (round.phase != ROUND_RESULT_PENDING || !round.telemetry_frozen)
        return false;
    std::vector<std::pair<uint32_t, uint16_t> > ranking;
    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        MemberState& member = round.members[seat];
        if ((round.active_mask & SeatBit(seat)) == 0u) continue;
        ranking.push_back(std::make_pair(member.total_score, member.seat));
    }
    if (ranking.size() != round.member_count) return false;
    std::sort(ranking.begin(), ranking.end(), RankLess);
    for (size_t rank = 0u; rank < ranking.size(); ++rank)
        round.members[ranking[rank].second].rank =
            static_cast<uint8_t>(rank);

    round.phase = ROUND_FINISHED;
    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        MemberState& member = round.members[seat];
        if ((round.active_mask & SeatBit(seat)) == 0u || !member.user)
            continue;
        if (member.entry_state == ENTRY_IDLE) {
            BuildEntryRequest(member, round, member.entry_request);
            member.entry_state = ENTRY_QUEUED;
            retry_entries.push_back(
                std::make_pair(member.user, member.entry_request));
        } else if (member.entry_state == ENTRY_DURABLE) {
            settle_users.push_back(member.user);
        }
    }
    return true;
}

void EraseRoundLocked(std::map<RoundKey, RoundState>::iterator round) {
    if (round == g_rounds.end()) return;
    if (round->second.room) {
        std::map<PvP_Room*, RoundKey>::iterator room =
            g_rooms.find(round->second.room);
        if (room != g_rooms.end() &&
            !(room->second < round->first) && !(round->first < room->second))
            g_rooms.erase(room);
    }
    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        CUser* const user = round->second.members[seat].user;
        std::map<CUser*, RoundKey>::iterator current =
            g_current_round.find(user);
        if (current != g_current_round.end() &&
            !(current->second < round->first) &&
            !(round->first < current->second))
            g_current_round.erase(current);
    }
    g_rounds.erase(round);
}

bool BuildNextSettlementLocked(CUser* user,
                               Message& request,
                               RoundKey& round_key) {
    if (!user || g_pending_claims.find(user) != g_pending_claims.end())
        return false;
    for (std::map<RoundKey, RoundState>::iterator round = g_rounds.begin();
         round != g_rounds.end(); ++round) {
        MemberState* const member = FindMember(round->second, user);
        if (!member || round->second.phase != ROUND_FINISHED ||
            member->committed || member->settle_queued ||
            member->entry_state != ENTRY_DURABLE ||
            !IdentityMatches(user, *member)) continue;
        BuildSettleRequest(*member, round->second, request);
        member->settle_request = request;
        member->settle_queued = true;
        round_key = round->first;
        return true;
    }
    return false;
}

void QueueNextSettlement(CUser* user) {
    Message request;
    RoundKey key;
    bool have = false;
    {
        MutexGuard guard(&g_zombie_mutex);
        have = BuildNextSettlementLocked(user, request, key);
    }
    if (!have) return;
    if (!QueueMessage(user, request)) {
        MutexGuard guard(&g_zombie_mutex);
        std::map<RoundKey, RoundState>::iterator round = g_rounds.find(key);
        if (round != g_rounds.end()) {
            MemberState* const member = FindMemberByCharacter(
                round->second, request.account_id, request.charac_no);
            if (member && member->settle_request.request_token ==
                              request.request_token)
                member->settle_queued = false;
        }
        TraceFailure("SQL026 settle queue failed", user);
    }
}

void DispatchFrozenResultSettlement(
        const std::vector<std::pair<CUser*, Message> >& retry_entries,
        const std::vector<CUser*>& settle_users) {
    for (size_t index = 0u; index < retry_entries.size(); ++index) {
        if (!QueueMessage(retry_entries[index].first,
                          retry_entries[index].second)) {
            const Message& request = retry_entries[index].second;
            MutexGuard guard(&g_zombie_mutex);
            const RoundKey key(request.round_token_hi,
                               request.round_token_lo);
            std::map<RoundKey, RoundState>::iterator round =
                g_rounds.find(key);
            if (round != g_rounds.end()) {
                MemberState* const member = FindMemberByCharacter(
                    round->second, request.account_id, request.charac_no);
                if (member && member->entry_state == ENTRY_QUEUED &&
                    member->entry_request.request_token ==
                        request.request_token) {
                    member->entry_state = ENTRY_IDLE;
                }
            }
            TraceFailure("SQL026 entry retry failed",
                         retry_entries[index].first);
        }
    }
    for (size_t index = 0u; index < settle_users.size(); ++index)
        QueueNextSettlement(settle_users[index]);
}

bool InstallPendingClaim(CUser* user,
                         const Message& response,
                         bool recovery) {
    MutexGuard guard(&g_zombie_mutex);
    if (!IsSelectedCharacter(user) ||
        response.account_id != static_cast<uint32_t>(user->get_acc_id()) ||
        response.charac_no !=
            static_cast<uint32_t>(user->getCurCharacNo())) return false;
    PendingClaim& pending = g_pending_claims[user];
    if (pending.request.round_token_hi != 0u &&
        (pending.request.round_token_hi != response.round_token_hi ||
         pending.request.round_token_lo != response.round_token_lo))
        return false;
    pending.user = user;
    pending.account_id = response.account_id;
    pending.charac_no = response.charac_no;
    pending.server_group = response.server_group;
    pending.request = response;
    pending.applying = true;
    pending.recovery = recovery;
    return true;
}

void PrepareAndQueueTargetStore(CUser* user,
                                const Message& response,
                                bool recovery) {
    if (!InstallPendingClaim(user, response, recovery)) return;
    CharacterSnapshot current;
    CharacterSnapshot target;
    if (!ComputeTarget(response.baseline, response.reward_exp, target) ||
        !CaptureSnapshot(user, current) ||
        (!EqualSnapshot(current, response.baseline) &&
         !EqualSnapshot(current, target))) {
        MutexGuard guard(&g_zombie_mutex);
        g_pending_claims.erase(user);
        TraceFailure("canonical EXP pre-store mismatch", user);
        return;
    }
    Message commit = response;
    commit.operation = OPERATION_COMMIT;
    commit.request_token = NextNonZero(g_request_counter);
    commit.target = target;
    commit.result = 0u;
    commit.replayed = 0u;
    commit.delivery_key_hi = response.delivery_key_hi;
    commit.delivery_key_lo = response.delivery_key_lo;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<CUser*, PendingClaim>::iterator pending =
            g_pending_claims.find(user);
        if (pending == g_pending_claims.end() ||
            pending->second.request.round_token_hi !=
                response.round_token_hi ||
            pending->second.request.round_token_lo !=
                response.round_token_lo) return;
        pending->second.request = commit;
        pending->second.applying = true;
    }
    if (!QueueMessage(user, commit)) {
        MutexGuard guard(&g_zombie_mutex);
        std::map<CUser*, PendingClaim>::iterator pending =
            g_pending_claims.find(user);
        if (pending != g_pending_claims.end() &&
            pending->second.request.request_token == commit.request_token) {
            pending->second.applying = false;
        }
        TraceFailure("SQL026 target store queue failed", user);
    }
}

void ApplyStoredTargetAndQueueFinal(CUser* user,
                                    const Message& response,
                                    bool recovery) {
    if (!InstallPendingClaim(user, response, recovery)) return;
    CharacterSnapshot target;
    if (response.claim_state != CLAIM_STATE_TARGET_STORED ||
        !ApplyReservedReward(user, response, target) ||
        !EqualSnapshot(target, response.target)) {
        MutexGuard guard(&g_zombie_mutex);
        g_pending_claims.erase(user);
        TraceFailure("canonical EXP apply mismatch", user);
        return;
    }
    Message commit = response;
    commit.operation = OPERATION_COMMIT;
    commit.claim_state = CLAIM_STATE_TARGET_STORED;
    commit.request_token = NextNonZero(g_request_counter);
    commit.result = 0u;
    commit.replayed = 0u;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<CUser*, PendingClaim>::iterator pending =
            g_pending_claims.find(user);
        if (pending == g_pending_claims.end() ||
            pending->second.request.round_token_hi !=
                response.round_token_hi ||
            pending->second.request.round_token_lo !=
                response.round_token_lo) return;
        pending->second.request = commit;
        pending->second.applying = true;
    }
    if (!QueueMessage(user, commit)) {
        MutexGuard guard(&g_zombie_mutex);
        std::map<CUser*, PendingClaim>::iterator pending =
            g_pending_claims.find(user);
        if (pending != g_pending_claims.end() &&
            pending->second.request.request_token == commit.request_token) {
            pending->second.applying = false;
        }
        TraceFailure("SQL026 phase-B queue failed", user);
    }
}

bool MessageIdentityMatches(const Message& lhs, const Message& rhs) {
    return lhs.account_id == rhs.account_id &&
           lhs.charac_no == rhs.charac_no &&
           lhs.round_token_hi == rhs.round_token_hi &&
           lhs.round_token_lo == rhs.round_token_lo &&
           lhs.request_token == rhs.request_token &&
           lhs.server_group == rhs.server_group && lhs.seat == rhs.seat &&
           lhs.role == rhs.role && lhs.outcome == rhs.outcome &&
           lhs.rank == rhs.rank &&
           lhs.daily_ordinal == rhs.daily_ordinal &&
           lhs.limited_reward == rhs.limited_reward &&
           lhs.immunity_at_entry == rhs.immunity_at_entry &&
           lhs.last_survivor == rhs.last_survivor &&
           lhs.telemetry_consistent == rhs.telemetry_consistent &&
           lhs.damage == rhs.damage && lhs.attacks == rhs.attacks &&
           lhs.serious_attacks == rhs.serious_attacks &&
           lhs.killed_humans == rhs.killed_humans &&
           lhs.killed_zombies == rhs.killed_zombies &&
           lhs.survival_seconds == rhs.survival_seconds &&
           lhs.total_score == rhs.total_score &&
           lhs.reward_item_id == rhs.reward_item_id &&
           lhs.reward_item_count == rhs.reward_item_count &&
           lhs.reward_exp == rhs.reward_exp &&
           lhs.delivery_key_hi == rhs.delivery_key_hi &&
           lhs.delivery_key_lo == rhs.delivery_key_lo &&
           EqualSnapshot(lhs.baseline, rhs.baseline) &&
           EqualSnapshot(lhs.target, rhs.target);
}

void HandleCommitResponse(CUser* user, const Message& response) {
    PendingClaim pending;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<CUser*, PendingClaim>::const_iterator found =
            g_pending_claims.find(user);
        if (found == g_pending_claims.end() ||
            !MessageIdentityMatches(found->second.request, response)) return;
        pending = found->second;
    }
    if (response.result != RESULT_SUCCESS) {
        {
            MutexGuard guard(&g_zombie_mutex);
            std::map<CUser*, PendingClaim>::iterator found =
                g_pending_claims.find(user);
            if (found != g_pending_claims.end() &&
                found->second.request.request_token ==
                    response.request_token) {
                found->second.applying = false;
            }
        }
        TraceFailure("SQL026 commit failed", user);
        return;
    }
    if (response.claim_state == CLAIM_STATE_TARGET_STORED) {
        ApplyStoredTargetAndQueueFinal(user, response, pending.recovery);
        return;
    }
    if (response.claim_state != CLAIM_STATE_COMMITTED) return;

    ResultDeliverySnapshot delivery;
    bool send_result = false;
    bool send_success = false;
    RoundKey delivery_key(response.round_token_hi, response.round_token_lo);
    PvP_Room* delivery_room = 0;
    uint16_t delivery_unique_id = 0u;
    uint16_t delivery_seat = 0xffffu;
    {
        MutexGuard guard(&g_zombie_mutex);
        g_pending_claims.erase(user);
        const RoundKey key(response.round_token_hi, response.round_token_lo);
        std::map<RoundKey, RoundState>::iterator round = g_rounds.find(key);
        if (round != g_rounds.end()) {
            MemberState* const member = FindMemberByCharacter(
                round->second, response.account_id, response.charac_no);
            if (member && round->second.phase == ROUND_FINISHED) {
                member->committed = true;
                member->commit_response = response;
                member->commit_response_valid = true;
                if (!member->result_terminal_resolved &&
                    !member->result_delivered &&
                    !member->result_delivery_inflight &&
                    round->second.room && IdentityMatches(user, *member) &&
                    user->GetPVPRoom() == round->second.room) {
                    send_result = BuildResultDeliverySnapshotLocked(
                        round->second, *member, &response, delivery);
                    member->result_delivery_inflight = send_result;
                    if (send_result) {
                        delivery_key = round->first;
                        delivery_room = round->second.room;
                        delivery_unique_id = member->unique_id;
                        delivery_seat = member->seat;
                    }
                }
            }
        }
    }
    if (send_result) send_success = SendResultNotice(user, delivery);
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(delivery_key);
        if (round != g_rounds.end()) {
            MemberState* const member = FindMemberByCharacter(
                round->second, response.account_id, response.charac_no);
            if (member && send_result &&
                member->user == user &&
                member->unique_id == delivery_unique_id &&
                member->seat == delivery_seat) {
                member->result_delivery_inflight = false;
                if (send_success && round->second.room == delivery_room) {
                    member->result_delivered = true;
                    member->result_terminal_resolved = true;
                }
            }
            if (AllResultsResolved(round->second) && round->second.room) {
                g_result_delivery_complete_rooms[round->second.room] = true;
            }
            if (AllCommitted(round->second) &&
                AllResultsResolved(round->second)) {
                EraseRoundLocked(round);
            }
        }
    }
    if (send_result) {
        cMyTrace trace("zombie_mode::HandleCommitResponse", __LINE__, 5);
        trace("[ZOMBIE_RESULT][DELIVER] seat=%u rows=%u reward=committed success=%u",
              static_cast<unsigned int>(delivery_seat),
              static_cast<unsigned int>(delivery.count),
              send_success ? 1u : 0u);
    }
    QueueNextSettlement(user);
    BeginUserSession(user);
}

bool SubmitTelemetrySnapshot(CUser* user,
                             uint8_t outcome,
                             const std::vector<ClientTelemetry>& telemetry,
                             bool& quorum_frozen,
                             uint16_t& sender_seat) {
    quorum_frozen = false;
    sender_seat = 0xffffu;
    std::vector<std::pair<CUser*, Message> > retry_entries;
    std::vector<CUser*> settle_users;
    bool accepted = false;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<RoundKey, RoundState>::iterator round =
            FindCurrentRoundLocked(user);
        if (round == g_rounds.end()) return false;
        accepted = ValidateTelemetrySnapshotLocked(
            round->second, user, outcome, telemetry, sender_seat);
        if (accepted && TryFreezeTelemetryQuorumLocked(round->second)) {
            quorum_frozen = PrepareFrozenResultSettlementLocked(
                round->second, retry_entries, settle_users);
            if (!quorum_frozen) accepted = false;
        }
    }
    if (quorum_frozen) {
        DispatchFrozenResultSettlement(retry_entries, settle_users);
        cMyTrace trace("zombie_mode::SubmitTelemetrySnapshot", __LINE__, 5);
        trace("[ZOMBIE_RESULT][QUORUM] sender_seat=%u rows=%u",
              static_cast<unsigned int>(sender_seat),
              static_cast<unsigned int>(telemetry.size()));
    }
    return accepted;
}

uint32_t ResultDeliveryGenerationToken(uint32_t hi,
                                       uint32_t lo,
                                       int room_index,
                                       uint32_t action) {
    return hi ^ lo ^ static_cast<uint32_t>(room_index) ^ action ^
           0x52434c31u;
}

bool QueueResultDeliveryContinuation(
        const ResultDeliveryContinuation& continuation) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, -1);
    ResultDeliveryContinuation* const output =
        reinterpret_cast<ResultDeliveryContinuation*>(
            guard.GetInBufferUntyped(
                static_cast<int>(sizeof(ResultDeliveryContinuation))));
    if (!output) return false;
    *output = continuation;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

bool ConfirmZombie(CUser* user, uint32_t actor_key) {
    if (!user || actor_key == 0u) return false;
    MutexGuard guard(&g_zombie_mutex);
    std::map<RoundKey, RoundState>::iterator round =
        FindCurrentRoundLocked(user);
    if (round == g_rounds.end() ||
        (round->second.phase != ROUND_RUNNING &&
         round->second.phase != ROUND_RESULT_PENDING &&
         round->second.phase != ROUND_FINISHED)) return false;
    MemberState* const member = FindMember(round->second, user);
    if (!member || !IdentityMatches(user, *member)) return false;
    if (member->role != ROLE_ZOMBIE ||
        member->life_state != MEMBER_LIFE_ACTIVE) return false;
    if (!member->transition_ack_pending)
        return member->client_actor_key != 0u &&
               member->client_actor_key == actor_key;
    if (member->client_actor_key != 0u &&
        member->client_actor_key != actor_key) return false;
    member->client_actor_key = actor_key;
    member->transition_ack_pending = false;
    return true;
}

}  // namespace

bool LoadConfig() {
    Policy parsed;
    const bool valid = ParseAndValidatePolicy(parsed);
    MutexGuard guard(&g_zombie_mutex);
    g_policy = parsed;
    if (!valid) {
        g_rounds.clear();
        g_rooms.clear();
        g_finished_rooms.clear();
        g_current_round.clear();
    }
    return valid;
}

bool IsConfigLoaded() {
    MutexGuard guard(&g_zombie_mutex);
    return g_policy.valid;
}

bool GetPlayModePolicy(uint8_t player_count, ZombiePlayModePolicy& output) {
    MutexGuard guard(&g_zombie_mutex);
    return ResolvePlayModeLocked(player_count, output);
}

uint32_t GetReadyTimeMs() {
    MutexGuard guard(&g_zombie_mutex);
    return g_policy.valid ? g_policy.ready_time_ms : 0u;
}

uint32_t GetPlayTimeMs() {
    MutexGuard guard(&g_zombie_mutex);
    return g_policy.valid ? g_policy.play_time_ms : 0u;
}

bool GetItemBoxPolicy(ItemBoxPolicy& output) {
    MutexGuard guard(&g_zombie_mutex);
    ZombiePlayModePolicy mode;
    if (!g_policy.valid || g_policy.item_box_passive_index == 0u ||
        !ResolvePlayModeLocked(
            static_cast<uint8_t>(g_policy.play_modes[0]), mode)) return false;
    std::memset(&output, 0, sizeof(output));
    output.passive_index = g_policy.item_box_passive_index;
    output.world_x = g_policy.item_box_world_x;
    output.world_y = g_policy.item_box_world_y;
    output.first_delay_ms = g_policy.item_box_first_delay_ms;
    output.repeat_delay_ms = g_policy.item_box_repeat_delay_ms;
    output.initial_zombie_count = mode.initial_zombie_count;
    return true;
}

bool GetResultPolicy(ResultPolicy& output) {
    MutexGuard guard(&g_zombie_mutex);
    if (!g_policy.valid) return false;
    std::memset(&output, 0, sizeof(output));
    output.reward_item_id = g_policy.reward_item_id;
    output.daily_limit = g_policy.daily_limit;
    output.immunity_item_id = g_policy.immunity_item_id;
    output.champion_level = g_policy.champion_level;
    std::memcpy(output.full_counts, g_policy.full_counts,
                sizeof(output.full_counts));
    std::memcpy(output.limited_counts, g_policy.limited_counts,
                sizeof(output.limited_counts));
    std::memcpy(output.reward_exp, g_policy.reward_exp,
                sizeof(output.reward_exp));
    return true;
}

bool IsActiveRoom(PvP_Room* room) {
    if (!room) return false;
    MutexGuard guard(&g_zombie_mutex);
    std::map<PvP_Room*, RoundKey>::const_iterator key = g_rooms.find(room);
    if (key == g_rooms.end()) return false;
    std::map<RoundKey, RoundState>::const_iterator round =
        g_rounds.find(key->second);
    return round != g_rounds.end() &&
           round->second.phase != ROUND_FINISHED;
}

bool IsRevivalBlocked(PvP_Room* room) {
    if (!room) return false;
    MutexGuard guard(&g_zombie_mutex);
    if (g_finished_rooms.find(room) != g_finished_rooms.end()) return true;
    const std::map<PvP_Room*, RoundKey>::const_iterator key =
        g_rooms.find(room);
    return key != g_rooms.end() && g_rounds.find(key->second) != g_rounds.end();
}

bool IsFinishedRoom(PvP_Room* room) {
    if (!room) return false;
    MutexGuard guard(&g_zombie_mutex);
    if (g_finished_rooms.find(room) != g_finished_rooms.end()) return true;
    std::map<PvP_Room*, RoundKey>::const_iterator key = g_rooms.find(room);
    if (key == g_rooms.end()) return false;
    std::map<RoundKey, RoundState>::const_iterator round =
        g_rounds.find(key->second);
    return round != g_rounds.end() &&
           round->second.phase == ROUND_FINISHED;
}

bool IsResultDeliveryComplete(PvP_Room* room) {
    if (!room) return false;
    MutexGuard guard(&g_zombie_mutex);
    return g_result_delivery_complete_rooms.find(room) !=
           g_result_delivery_complete_rooms.end();
}

bool BeginRoom(PvP_Room* room, uint8_t play_mode) {
    if (!room) return false;
    std::vector<std::pair<CUser*, Message> > requests;
    {
        MutexGuard guard(&g_zombie_mutex);
        ZombiePlayModePolicy mode_policy;
        if (!g_policy.valid || room->get_player_count() != play_mode ||
            !ResolvePlayModeLocked(play_mode, mode_policy) ||
            g_finished_rooms.find(room) != g_finished_rooms.end() ||
            g_rooms.find(room) != g_rooms.end()) return false;
        g_result_delivery_complete_rooms.erase(room);
        RoundState round;
        round.room = room;
        round.room_index = room->get_index();
        round.play_mode = play_mode;
        round.member_count = play_mode;
        round.initial_zombie_count = mode_policy.initial_zombie_count;
        round.ready_deadline_ms = AddMillisecondDelay(
            NowMilliseconds(), g_policy.ready_time_ms);
        int server_group = -1;
        size_t count = 0u;
        for (int seat = 0; seat < kMaximumSeats; ++seat) {
            CUser* const user = room->get_user_seat(seat);
            if (!user) continue;
            if (room->IsPvpObserver(seat) || !IsSelectedCharacter(user) ||
                user->get_unique_id() <= 0 ||
                user->get_unique_id() >= 0xffff ||
                (server_group >= 0 &&
                 server_group != user->GetServerGroup())) return false;
            server_group = user->GetServerGroup();
            MemberState& member = round.members[seat];
            member.user = user;
            member.account_id = static_cast<uint32_t>(user->get_acc_id());
            member.charac_no =
                static_cast<uint32_t>(user->getCurCharacNo());
            member.unique_id =
                static_cast<uint16_t>(user->get_unique_id());
            member.seat = static_cast<uint16_t>(seat);
            member.life_state = MEMBER_LIFE_ACTIVE;
            member.immunity = HasImmunityItem(user);
            round.active_mask = static_cast<uint8_t>(
                round.active_mask | static_cast<uint8_t>(1u << seat));
            ++count;
        }
        if (count != play_mode || server_group < 0) return false;
        round.key = MakeRoundKey(room, server_group);
        while (g_rounds.find(round.key) != g_rounds.end())
            round.key = MakeRoundKey(room, server_group);
        const RoundKey key = round.key;
        std::pair<std::map<RoundKey, RoundState>::iterator, bool> inserted =
            g_rounds.insert(std::make_pair(key, round));
        if (!inserted.second) return false;
        g_rooms[room] = key;
        for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
            MemberState& member = inserted.first->second.members[seat];
            if (!member.user) continue;
            g_current_round[member.user] = key;
            BuildEntryRequest(member, inserted.first->second,
                              member.entry_request);
            member.entry_state = ENTRY_QUEUED;
            requests.push_back(std::make_pair(member.user,
                                               member.entry_request));
        }
    }
    for (size_t index = 0u; index < requests.size(); ++index) {
        if (!QueueMessage(requests[index].first, requests[index].second)) {
            MutexGuard guard(&g_zombie_mutex);
            const RoundKey key(requests[index].second.round_token_hi,
                               requests[index].second.round_token_lo);
            std::map<RoundKey, RoundState>::iterator round =
                g_rounds.find(key);
            if (round != g_rounds.end()) {
                MemberState* const member = FindMemberByCharacter(
                    round->second, requests[index].second.account_id,
                    requests[index].second.charac_no);
                if (member) member->entry_state = ENTRY_IDLE;
            }
            TraceFailure("SQL026 entry queue failed", requests[index].first);
        }
    }
    return true;
}

void ResetRoom(PvP_Room* room) {
    if (!room) return;
    MutexGuard guard(&g_zombie_mutex);
    g_finished_rooms.erase(room);
    g_result_delivery_complete_rooms.erase(room);
    std::map<PvP_Room*, RoundKey>::iterator key = g_rooms.find(room);
    if (key == g_rooms.end()) return;
    std::map<RoundKey, RoundState>::iterator round =
        g_rounds.find(key->second);
    if (round == g_rounds.end()) {
        g_rooms.erase(key);
        return;
    }
    if (round->second.phase == ROUND_FINISHED) {
        for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
            MemberState& member = round->second.members[seat];
            if (!member.user) continue;
            member.result_delivery_inflight = false;
            member.result_terminal_resolved = true;
        }
        if (AllCommitted(round->second) &&
            AllResultsResolved(round->second)) {
            EraseRoundLocked(round);
            return;
        }
        round->second.room = 0;
        g_rooms.erase(key);
    } else {
        EraseRoundLocked(round);
    }
}

int SelectInitialZombies(PvP_Room* room,
                         std::vector<uint32_t>& selected_seats,
                         bool& newly_selected) {
    selected_seats.clear();
    newly_selected = false;
    if (!room) return 19;
    size_t expected_count = 0u;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<PvP_Room*, RoundKey>::const_iterator key =
            g_rooms.find(room);
        if (key == g_rooms.end()) return 19;
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(key->second);
        if (round == g_rounds.end() ||
            (round->second.phase != ROUND_READY &&
             round->second.phase != ROUND_RUNNING) ||
            (round->second.phase == ROUND_READY &&
             !MillisecondDeadlineReached(
                 NowMilliseconds(), round->second.ready_deadline_ms))) {
            return 19;
        }
        expected_count = round->second.initial_zombie_count;
        if (!round->second.roles_selected) {
            std::vector<uint32_t> seats;
            for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
                if (round->second.members[seat].user)
                    seats.push_back(static_cast<uint32_t>(seat));
            }
            if (seats.size() != round->second.member_count ||
                round->second.initial_zombie_count == 0u ||
                seats.size() < round->second.initial_zombie_count) return 19;
            std::random_shuffle(seats.begin(), seats.end());
            selected_seats.assign(seats.begin(),
                                  seats.begin() +
                                      round->second.initial_zombie_count);
            std::sort(selected_seats.begin(), selected_seats.end());
            for (size_t index = 0u; index < selected_seats.size(); ++index) {
                MemberState& selected =
                    round->second.members[selected_seats[index]];
                selected.role = ROLE_ZOMBIE;
                selected.life_state = MEMBER_LIFE_ACTIVE;
                selected.transition_ack_pending = false;
                selected.client_actor_key = 0u;
            }
            const uint32_t now = NowSeconds();
            for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
                MemberState& member = round->second.members[seat];
                if (member.user && member.role == ROLE_HUMAN)
                    member.human_started_at = now;
            }
            round->second.roles_selected = true;
            round->second.phase = ROUND_RUNNING;
            round->second.ready_deadline_ms = 0u;
            newly_selected = true;
        } else {
            for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
                if (round->second.members[seat].user &&
                    round->second.members[seat].role == ROLE_ZOMBIE)
                    selected_seats.push_back(static_cast<uint32_t>(seat));
            }
        }
    }
    if (newly_selected) {
        PacketGuard packet;
        packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                          ENUM_NOTIPACKET_SELECT_ZOMBIE);
        packet.put_byte(static_cast<int>(selected_seats.size()));
        for (size_t index = 0u; index < selected_seats.size(); ++index)
            packet.put_int(static_cast<int>(selected_seats[index]));
        packet.finalize(true);
        room->send_to_pvp(packet);
    }
    return expected_count != 0u && selected_seats.size() == expected_count
               ? 0
               : 19;
}

int ResolveCharacterDeath(PvP_Room* room,
                          CUser* dead_user,
                          CUser* killer_user,
                          ZombieDeathResolution& resolution) {
    resolution = ZombieDeathResolution();
    if (!room || !dead_user || !killer_user || dead_user == killer_user)
        return 19;
    MutexGuard guard(&g_zombie_mutex);
    std::map<PvP_Room*, RoundKey>::const_iterator key = g_rooms.find(room);
    if (key == g_rooms.end()) return 19;
    std::map<RoundKey, RoundState>::iterator round =
        g_rounds.find(key->second);
    if (round == g_rounds.end() || round->second.room != room ||
        round->second.phase != ROUND_RUNNING ||
        !round->second.roles_selected || round->second.terminal_pending)
        return 19;

    MemberState* const dead = FindMember(round->second, dead_user);
    MemberState* const killer = FindMember(round->second, killer_user);
    if (!dead || !killer || !IdentityMatches(dead_user, *dead) ||
        !IdentityMatches(killer_user, *killer) ||
        dead_user->GetPVPRoom() != room ||
        killer_user->GetPVPRoom() != room ||
        dead_user->GetPvpIndex() != round->second.room_index ||
        killer_user->GetPvpIndex() != round->second.room_index ||
        dead->life_state != MEMBER_LIFE_ACTIVE ||
        killer->life_state != MEMBER_LIFE_ACTIVE ||
        dead->role == killer->role) {
        return 19;
    }

    resolution.dead_seat = dead->seat;
    resolution.killer_seat = killer->seat;
    if (dead->role == ROLE_HUMAN && killer->role == ROLE_ZOMBIE) {
        AccumulateHumanSurvival(*dead, NowSeconds());
        dead->human_started_at = 0u;
        dead->role = ROLE_ZOMBIE;
        dead->life_state = MEMBER_LIFE_ACTIVE;
        dead->transition_ack_pending = true;
        dead->client_actor_key = 0u;
        killer->killed_humans =
            SaturatingAdd(killer->killed_humans, 1u);
        resolution.action = ZOMBIE_DEATH_HUMAN_INFECTED;
    } else if (dead->role == ROLE_ZOMBIE &&
               killer->role == ROLE_HUMAN) {
        dead->life_state = MEMBER_LIFE_ELIMINATED;
        dead->transition_ack_pending = false;
        killer->killed_zombies =
            SaturatingAdd(killer->killed_zombies, 1u);
        resolution.action = ZOMBIE_DEATH_ZOMBIE_ELIMINATED;
    } else {
        return 19;
    }

    size_t active_humans = 0u;
    size_t active_zombies = 0u;
    CUser* only_human = 0;
    for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
        const MemberState& member = round->second.members[seat];
        if (!member.user || member.life_state != MEMBER_LIFE_ACTIVE)
            continue;
        if (member.role == ROLE_HUMAN) {
            ++active_humans;
            only_human = member.user;
        } else if (member.role == ROLE_ZOMBIE) {
            ++active_zombies;
        }
    }

    if (active_humans == 0u) {
        resolution.terminal = true;
        resolution.outcome = OUTCOME_ZOMBIE_WIN;
    } else if (active_zombies == 0u) {
        resolution.terminal = true;
        resolution.outcome = OUTCOME_HUMAN_WIN;
        resolution.last_survivor = active_humans == 1u ? only_human : 0;
    }
    if (resolution.terminal) {
        round->second.terminal_pending = true;
        round->second.pending_outcome = resolution.outcome;
        round->second.pending_last_survivor = resolution.last_survivor;
    }
    return 0;
}

void RecordDamage(PvP_Room* room, CUser* attacker, uint32_t damage) {
    if (!room || !attacker || damage == 0u) return;
    MutexGuard guard(&g_zombie_mutex);
    std::map<PvP_Room*, RoundKey>::const_iterator key = g_rooms.find(room);
    if (key == g_rooms.end()) return;
    std::map<RoundKey, RoundState>::iterator round = g_rounds.find(key->second);
    if (round == g_rounds.end() || round->second.phase != ROUND_RUNNING) return;
    MemberState* const member = FindMember(round->second, attacker);
    if (member && member->life_state == MEMBER_LIFE_ACTIVE)
        member->damage = SaturatingAdd(member->damage, damage);
}

void RecordAttack(PvP_Room* room, CUser* attacker, bool serious) {
    if (!room || !attacker) return;
    MutexGuard guard(&g_zombie_mutex);
    std::map<PvP_Room*, RoundKey>::const_iterator key = g_rooms.find(room);
    if (key == g_rooms.end()) return;
    std::map<RoundKey, RoundState>::iterator round = g_rounds.find(key->second);
    if (round == g_rounds.end() || round->second.phase != ROUND_RUNNING) return;
    MemberState* const member = FindMember(round->second, attacker);
    if (!member || member->life_state != MEMBER_LIFE_ACTIVE) return;
    member->attacks = SaturatingAdd(member->attacks, 1u);
    if (serious)
        member->serious_attacks =
            SaturatingAdd(member->serious_attacks, 1u);
}

void RecordSurvival(PvP_Room* room, CUser* user) {
    if (!room || !user) return;
    MutexGuard guard(&g_zombie_mutex);
    std::map<PvP_Room*, RoundKey>::const_iterator key = g_rooms.find(room);
    if (key == g_rooms.end()) return;
    std::map<RoundKey, RoundState>::iterator round = g_rounds.find(key->second);
    if (round == g_rounds.end() || round->second.phase != ROUND_RUNNING) return;
    MemberState* const member = FindMember(round->second, user);
    if (!member || member->role != ROLE_HUMAN ||
        member->life_state != MEMBER_LIFE_ACTIVE) return;
    AccumulateHumanSurvival(*member, NowSeconds());
}

bool SendFactionChat(PvP_Room* room,
                     CUser* sender,
                     PacketGuard& packet) {
    if (!room || !sender) return false;
    std::vector<CUser*> recipients;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<PvP_Room*, RoundKey>::const_iterator key =
            g_rooms.find(room);
        if (key == g_rooms.end()) return false;
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(key->second);
        if (round == g_rounds.end() || !round->second.roles_selected)
            return false;
        MemberState* const source = FindMember(round->second, sender);
        if (!source || !IdentityMatches(sender, *source)) return true;
        for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
            MemberState& member = round->second.members[seat];
            if (member.user && member.role == source->role &&
                IdentityMatches(member.user, member))
                recipients.push_back(member.user);
        }
    }
    for (std::vector<CUser*>::iterator user = recipients.begin();
         user != recipients.end(); ++user)
        (*user)->Send(packet);
    return true;
}

bool FinishRound(PvP_Room* room,
                 uint8_t outcome,
                 CUser* last_survivor) {
    if (!room || outcome > OUTCOME_DRAW) return false;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<PvP_Room*, RoundKey>::const_iterator key = g_rooms.find(room);
        if (key == g_rooms.end()) return false;
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(key->second);
        if (round == g_rounds.end() ||
            round->second.phase != ROUND_RUNNING ||
            !round->second.roles_selected) return false;
        if (round->second.terminal_pending &&
            (round->second.pending_outcome != outcome ||
             round->second.pending_last_survivor != last_survivor)) {
            return false;
        }
        MemberState* const survivor =
            last_survivor ? FindMember(round->second, last_survivor) : 0;
        if (last_survivor &&
            (!survivor || survivor->role != ROLE_HUMAN ||
             survivor->life_state != MEMBER_LIFE_ACTIVE ||
             !IdentityMatches(last_survivor, *survivor))) {
            return false;
        }
        // Terminal combat and result settlement are separate phases.  CMD537
        // peer snapshots get a bounded ten-second quorum window before rank,
        // reward, or SQL026 settlement is allowed to observe a final score.
        round->second.phase = ROUND_RESULT_PENDING;
        g_finished_rooms[room] = true;
        round->second.outcome = outcome;
        round->second.terminal_pending = false;
        round->second.pending_outcome = outcome;
        round->second.pending_last_survivor = last_survivor;
        if (survivor) survivor->last_survivor = true;
        const uint32_t finished_at = NowSeconds();
        for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
            MemberState& member = round->second.members[seat];
            if ((round->second.active_mask & SeatBit(seat)) == 0u)
                continue;
            AccumulateHumanSurvival(member, finished_at);
            member.human_started_at = 0u;
            member.attack_score = 0u;
            member.survival_ms = ServerSurvivalMilliseconds(member);
            member.total_score = 0u;
            member.telemetry_consistent = false;
        }
        round->second.telemetry_received_mask = 0u;
        round->second.telemetry_frozen = false;
        round->second.telemetry_quorum = false;
        round->second.telemetry_deadline_ms = AddMillisecondDelay(
            NowMilliseconds(), kResultTelemetryTimeoutMs);
    }
    cMyTrace trace("zombie_mode::FinishRound", __LINE__, 5);
    trace("[ZOMBIE_RESULT][PENDING] room=%d outcome=%u timeout_ms=%u",
          room->get_index(), static_cast<unsigned int>(outcome),
          static_cast<unsigned int>(kResultTelemetryTimeoutMs));
    return true;
}

bool FinishByTimeout(PvP_Room* room) {
    if (!room) return false;
    CUser* only_human = 0;
    uint8_t outcome = OUTCOME_ZOMBIE_WIN;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<PvP_Room*, RoundKey>::const_iterator key =
            g_rooms.find(room);
        if (key == g_rooms.end()) return false;
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(key->second);
        if (round == g_rounds.end() ||
            round->second.phase != ROUND_RUNNING ||
            !round->second.roles_selected ||
            round->second.terminal_pending) return false;
        size_t human_count = 0u;
        for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
            const MemberState& member = round->second.members[seat];
            if (member.user && member.role == ROLE_HUMAN &&
                member.life_state == MEMBER_LIFE_ACTIVE) {
                ++human_count;
                only_human = member.user;
            }
        }
        if (human_count != 1u) only_human = 0;
        outcome = human_count > 0u ? OUTCOME_HUMAN_WIN
                                   : OUTCOME_ZOMBIE_WIN;
        round->second.terminal_pending = true;
        round->second.pending_outcome = outcome;
        round->second.pending_last_survivor = only_human;
    }
    return FinishRound(room, outcome, only_human);
}

void TickResultTelemetryOnDispatch(PvP_Room* room, uint32_t now_ms) {
    if (!room) return;
    std::vector<std::pair<CUser*, Message> > retry_entries;
    std::vector<CUser*> settle_users;
    std::vector<std::pair<CUser*, Message> > retry_claims;
    bool timeout_frozen = false;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<PvP_Room*, RoundKey>::const_iterator key =
            g_rooms.find(room);
        if (key == g_rooms.end()) return;
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(key->second);
        if (round == g_rounds.end()) return;
        if (round->second.phase == ROUND_RESULT_PENDING) {
            if (!MillisecondDeadlineReached(
                    now_ms, round->second.telemetry_deadline_ms)) return;
            if (FreezeTelemetryTimeoutLocked(round->second)) {
                timeout_frozen = PrepareFrozenResultSettlementLocked(
                    round->second, retry_entries, settle_users);
            }
        } else if (round->second.phase == ROUND_FINISHED) {
            for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
                MemberState& member = round->second.members[seat];
                if (!member.user || member.committed) continue;
                std::map<CUser*, PendingClaim>::iterator pending =
                    g_pending_claims.find(member.user);
                if (pending != g_pending_claims.end()) {
                    if (!pending->second.applying) {
                        pending->second.applying = true;
                        retry_claims.push_back(std::make_pair(
                            member.user, pending->second.request));
                    }
                    continue;
                }
                if (member.entry_state == ENTRY_IDLE) {
                    BuildEntryRequest(member, round->second,
                                      member.entry_request);
                    member.entry_state = ENTRY_QUEUED;
                    retry_entries.push_back(std::make_pair(
                        member.user, member.entry_request));
                } else if (member.entry_state == ENTRY_DURABLE &&
                           !member.settle_queued) {
                    settle_users.push_back(member.user);
                }
            }
        } else {
            return;
        }
    }
    if (!retry_entries.empty() || !settle_users.empty())
        DispatchFrozenResultSettlement(retry_entries, settle_users);
    for (size_t index = 0u; index < retry_claims.size(); ++index) {
        if (QueueMessage(retry_claims[index].first,
                         retry_claims[index].second)) continue;
        MutexGuard guard(&g_zombie_mutex);
        std::map<CUser*, PendingClaim>::iterator pending =
            g_pending_claims.find(retry_claims[index].first);
        if (pending != g_pending_claims.end() &&
            pending->second.request.request_token ==
                retry_claims[index].second.request_token) {
            pending->second.applying = false;
        }
        TraceFailure("SQL026 pending claim retry failed",
                     retry_claims[index].first);
    }
    if (timeout_frozen) {
        cMyTrace trace("zombie_mode::TickResultTelemetry", __LINE__, 5);
        trace("[ZOMBIE_RESULT][TELEMETRY_TIMEOUT] room=%d attack=0 fallback=server",
              room->get_index());
    }
}

void ExecuteResultTelemetryContinuation(
        const ResultDeliveryContinuation& continuation) {
    if (continuation.magic != kResultDeliveryContinuationMagic ||
        continuation.action != kResultContinuationTelemetryTick ||
        continuation.generation_token != ResultDeliveryGenerationToken(
            continuation.round_token_hi, continuation.round_token_lo,
            continuation.room_index, continuation.action)) return;
    const RoundKey key(continuation.round_token_hi,
                       continuation.round_token_lo);
    PvP_Room* room = 0;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<RoundKey, RoundState>::iterator round = g_rounds.find(key);
        if (round == g_rounds.end() || !round->second.room ||
            round->second.room_index != continuation.room_index ||
            round->second.room->get_index() != continuation.room_index ||
            !round->second.telemetry_continuation_queued) return;
        round->second.telemetry_continuation_queued = false;
        room = round->second.room;
    }
    TickResultTelemetryOnDispatch(room, continuation.now_ms);
}

void ExecuteResultDeliveryTimeoutContinuation(
        const ResultDeliveryContinuation& continuation) {
    if (continuation.magic != kResultDeliveryContinuationMagic ||
        continuation.action != kResultContinuationDeliveryTimeout ||
        continuation.generation_token != ResultDeliveryGenerationToken(
            continuation.round_token_hi, continuation.round_token_lo,
            continuation.room_index, continuation.action)) return;
    const RoundKey continuation_key(continuation.round_token_hi,
                                    continuation.round_token_lo);
    PvP_Room* room = 0;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(continuation_key);
        if (round == g_rounds.end() || !round->second.room ||
            round->second.room_index != continuation.room_index ||
            round->second.room->get_index() != continuation.room_index ||
            !round->second.delivery_continuation_queued) return;
        round->second.delivery_continuation_queued = false;
        room = round->second.room;
    }
    // A missed telemetry tick must not let the delivery watchdog bypass score
    // freezing.  This call is idempotent and normally does nothing here.
    TickResultTelemetryOnDispatch(room, continuation.now_ms);

    std::vector<ResultDeliveryWork> deliveries;
    std::vector<std::pair<CUser*, Message> > retry_entries;
    std::vector<CUser*> settle_users;
    std::vector<CUser*> retry_settlement_users;
    std::vector<std::pair<CUser*, Message> > hard_settle_retries;
    std::vector<std::pair<CUser*, Message> > hard_claim_retries;
    size_t zero_reward_count = 0u;
    {
        MutexGuard guard(&g_zombie_mutex);
        std::map<PvP_Room*, RoundKey>::const_iterator key =
            g_rooms.find(room);
        if (key == g_rooms.end()) return;
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(key->second);
        if (round == g_rounds.end()) return;
        if (round->second.phase == ROUND_RESULT_PENDING) {
            if (round->second.telemetry_frozen) {
                // A prior invariant failure after voting must not defeat the
                // terminal delivery watchdog.  Re-freeze from server data.
                round->second.telemetry_frozen = false;
                round->second.telemetry_quorum = false;
            }
            if (!FreezeTelemetryTimeoutLocked(round->second) ||
                !PrepareFrozenResultSettlementLocked(
                    round->second, retry_entries, settle_users)) {
                return;
            }
        }
        if (round->second.phase != ROUND_FINISHED) return;

        for (size_t seat = 0u; seat < kMaximumSeats; ++seat) {
            MemberState& member = round->second.members[seat];
            if ((round->second.active_mask & SeatBit(seat)) == 0u ||
                member.result_terminal_resolved ||
                member.result_delivered ||
                member.result_delivery_inflight) continue;
            ResultDeliverySnapshot snapshot;
            const Message* reward = member.commit_response_valid
                ? &member.commit_response : 0;
            if (!BuildResultDeliverySnapshotLocked(
                    round->second, member, reward, snapshot)) {
                member.result_terminal_resolved = true;
                ++zero_reward_count;
                continue;
            }
            if (member.user && IdentityMatches(member.user, member) &&
                member.user->GetPVPRoom() == room) {
                ResultDeliveryWork work;
                work.key = round->first;
                work.room = room;
                work.account_id = member.account_id;
                work.charac_no = member.charac_no;
                work.unique_id = member.unique_id;
                work.snapshot = snapshot;
                deliveries.push_back(work);
                member.result_delivery_inflight = true;
            } else {
                // Offline/unbound members cannot receive a stale panel.  The
                // hard deadline resolves only UI ownership; DB claims remain
                // durable/recoverable and are not marked committed here.
                member.result_terminal_resolved = true;
            }
            if (!reward) ++zero_reward_count;
            if (!member.committed && member.user) {
                std::map<CUser*, PendingClaim>::const_iterator pending =
                    g_pending_claims.find(member.user);
                if (pending != g_pending_claims.end()) {
                    hard_claim_retries.push_back(std::make_pair(
                        member.user, pending->second.request));
                } else if (member.entry_state == ENTRY_IDLE) {
                    BuildEntryRequest(member, round->second,
                                      member.entry_request);
                    member.entry_state = ENTRY_QUEUED;
                    retry_entries.push_back(
                        std::make_pair(member.user,
                                       member.entry_request));
                } else if (member.entry_state == ENTRY_QUEUED) {
                    // A lost BEGIN_ENTRY response must be replayed with the
                    // same round/request identity before the room is released.
                    retry_entries.push_back(std::make_pair(
                        member.user, member.entry_request));
                } else if (member.entry_state == ENTRY_DURABLE &&
                           member.settle_queued) {
                    hard_settle_retries.push_back(std::make_pair(
                        member.user, member.settle_request));
                } else {
                    retry_settlement_users.push_back(member.user);
                }
            }
        }
    }

    if (!retry_entries.empty() || !settle_users.empty())
        DispatchFrozenResultSettlement(retry_entries, settle_users);
    for (size_t index = 0u; index < retry_settlement_users.size(); ++index)
        QueueNextSettlement(retry_settlement_users[index]);
    for (size_t index = 0u; index < hard_settle_retries.size(); ++index) {
        if (!QueueMessage(hard_settle_retries[index].first,
                          hard_settle_retries[index].second)) {
            TraceFailure("SQL026 hard settle retry failed",
                         hard_settle_retries[index].first);
        }
    }
    for (size_t index = 0u; index < hard_claim_retries.size(); ++index) {
        if (!QueueMessage(hard_claim_retries[index].first,
                          hard_claim_retries[index].second)) {
            TraceFailure("SQL026 hard claim retry failed",
                         hard_claim_retries[index].first);
        }
    }
    std::vector<uint8_t> send_results(deliveries.size(), 0u);
    size_t sent_count = 0u;
    for (size_t index = 0u; index < deliveries.size(); ++index) {
        // This function is reached only through DISPATCH_Q.  Resolve the
        // session from the round again on the unique DispatchThread; no raw
        // CUser pointer crosses the MainLoop continuation boundary.
        CUser* current = 0;
        {
            MutexGuard guard(&g_zombie_mutex);
            std::map<RoundKey, RoundState>::iterator round =
                g_rounds.find(deliveries[index].key);
            if (round != g_rounds.end() &&
                round->second.room == deliveries[index].room) {
                MemberState* const member = FindMemberByCharacter(
                    round->second, deliveries[index].account_id,
                    deliveries[index].charac_no);
                if (member && member->user &&
                    member->unique_id == deliveries[index].unique_id &&
                    IdentityMatches(member->user, *member) &&
                    member->user->GetPVPRoom() == deliveries[index].room) {
                    current = member->user;
                }
            }
        }
        if (!current ||
            !SendResultNotice(current,
                              deliveries[index].snapshot)) {
            // Send failure may synchronously disconnect/reset the whole room;
            // do not dereference any later raw CUser snapshot in this batch.
            send_results[index] = 2u;
            break;
        }
        send_results[index] = 1u;
        ++sent_count;
    }

    {
        MutexGuard guard(&g_zombie_mutex);
        for (size_t index = 0u; index < deliveries.size(); ++index) {
            std::map<RoundKey, RoundState>::iterator round =
                g_rounds.find(deliveries[index].key);
            if (round == g_rounds.end()) continue;
            MemberState* const member = FindMemberByCharacter(
                round->second, deliveries[index].account_id,
                deliveries[index].charac_no);
            if (!member || member->unique_id != deliveries[index].unique_id) {
                continue;
            }
            member->result_delivery_inflight = false;
            if (send_results[index] == 1u &&
                round->second.room == deliveries[index].room) {
                member->result_delivered = true;
            }
            // Hard-timeout send failure/offline resolution is terminal for UI
            // ownership, but never fabricates DB COMMITTED/reward state.
            member->result_terminal_resolved = true;
        }
        std::map<PvP_Room*, RoundKey>::const_iterator key =
            g_rooms.find(room);
        if (key != g_rooms.end()) {
            std::map<RoundKey, RoundState>::iterator round =
                g_rounds.find(key->second);
            if (round != g_rounds.end() &&
                AllResultsResolved(round->second)) {
                g_result_delivery_complete_rooms[room] = true;
                if (AllCommitted(round->second)) EraseRoundLocked(round);
            }
        }
    }
    cMyTrace trace("zombie_mode::ForceResultDeliveryTimeout", __LINE__, 5);
    trace("[ZOMBIE_RESULT][RESULT_DELIVERY_TIMEOUT] room=%d sent=%u zero_reward=%u",
          room->get_index(),
          static_cast<unsigned int>(sent_count),
          static_cast<unsigned int>(zero_reward_count));
}

void TickResultTelemetry(PvP_Room* room, uint32_t now_ms) {
    if (!room) return;
    ResultDeliveryContinuation continuation;
    std::memset(&continuation, 0, sizeof(continuation));
    {
        MutexGuard guard(&g_zombie_mutex);
        if (g_result_delivery_complete_rooms.find(room) !=
            g_result_delivery_complete_rooms.end()) return;
        std::map<PvP_Room*, RoundKey>::const_iterator key =
            g_rooms.find(room);
        if (key == g_rooms.end()) return;
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(key->second);
        if (round == g_rounds.end() ||
            (round->second.phase != ROUND_RESULT_PENDING &&
             round->second.phase != ROUND_FINISHED) ||
            round->second.telemetry_continuation_queued) return;
        if (round->second.phase == ROUND_RESULT_PENDING &&
            !MillisecondDeadlineReached(
                now_ms, round->second.telemetry_deadline_ms)) return;
        continuation.magic = kResultDeliveryContinuationMagic;
        continuation.round_token_hi = round->first.hi;
        continuation.round_token_lo = round->first.lo;
        continuation.now_ms = now_ms;
        continuation.room_index = round->second.room_index;
        continuation.action = kResultContinuationTelemetryTick;
        continuation.generation_token = ResultDeliveryGenerationToken(
            continuation.round_token_hi, continuation.round_token_lo,
            continuation.room_index, continuation.action);
        round->second.telemetry_continuation_queued = true;
    }
    if (QueueResultDeliveryContinuation(continuation)) return;
    {
        MutexGuard guard(&g_zombie_mutex);
        const RoundKey key(continuation.round_token_hi,
                           continuation.round_token_lo);
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(key);
        if (round != g_rounds.end() && round->second.room == room &&
            round->second.room_index == continuation.room_index) {
            round->second.telemetry_continuation_queued = false;
        }
    }
}

void ForceResultDeliveryTimeout(PvP_Room* room, uint32_t now_ms) {
    if (!room) return;
    ResultDeliveryContinuation continuation;
    std::memset(&continuation, 0, sizeof(continuation));
    {
        MutexGuard guard(&g_zombie_mutex);
        if (g_result_delivery_complete_rooms.find(room) !=
            g_result_delivery_complete_rooms.end()) return;
        std::map<PvP_Room*, RoundKey>::const_iterator key =
            g_rooms.find(room);
        if (key == g_rooms.end()) return;
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(key->second);
        if (round == g_rounds.end() ||
            (round->second.phase != ROUND_RESULT_PENDING &&
             round->second.phase != ROUND_FINISHED) ||
            round->second.delivery_continuation_queued) return;
        continuation.magic = kResultDeliveryContinuationMagic;
        continuation.round_token_hi = round->first.hi;
        continuation.round_token_lo = round->first.lo;
        continuation.now_ms = now_ms;
        continuation.room_index = round->second.room_index;
        continuation.action = kResultContinuationDeliveryTimeout;
        continuation.generation_token = ResultDeliveryGenerationToken(
            continuation.round_token_hi, continuation.round_token_lo,
            continuation.room_index, continuation.action);
        round->second.delivery_continuation_queued = true;
    }
    if (QueueResultDeliveryContinuation(continuation)) return;

    {
        MutexGuard guard(&g_zombie_mutex);
        const RoundKey key(continuation.round_token_hi,
                           continuation.round_token_lo);
        std::map<RoundKey, RoundState>::iterator round =
            g_rounds.find(key);
        if (round != g_rounds.end() &&
            round->second.room == room &&
            round->second.room_index == continuation.room_index) {
            round->second.delivery_continuation_queued = false;
        }
    }
    cMyTrace trace("zombie_mode::ForceResultDeliveryTimeout", __LINE__, 5);
    trace("[ZOMBIE_RESULT][DELIVERY_CONTINUATION_QUEUE_FAIL] room=%d",
          continuation.room_index);
}

void BeginUserSession(CUser* user) {
    if (!IsSelectedCharacter(user)) return;
    Message request;
    {
        MutexGuard guard(&g_zombie_mutex);
        if (g_pending_claims.find(user) != g_pending_claims.end() ||
            g_recovery_requests.find(user) != g_recovery_requests.end())
            return;
        std::memset(&request, 0, sizeof(request));
        request.account_id = static_cast<uint32_t>(user->get_acc_id());
        request.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
        request.request_token = NextNonZero(g_request_counter);
        request.server_group = user->GetServerGroup();
        request.reply_user_idx = user->GetUID();
        request.operation = OPERATION_RECOVER;
        g_recovery_requests[user] = request;
    }
    if (!QueueMessage(user, request)) {
        MutexGuard guard(&g_zombie_mutex);
        g_recovery_requests.erase(user);
        TraceFailure("SQL026 recovery queue failed", user);
    }
}

void ResetUserSession(CUser* user) {
    if (!user) return;
    MutexGuard guard(&g_zombie_mutex);
    std::map<RoundKey, RoundState>::iterator round =
        FindCurrentRoundLocked(user);
    if (round != g_rounds.end() &&
        round->second.phase == ROUND_FINISHED) {
        MemberState* const member = FindMember(round->second, user);
        if (member) {
            member->result_delivery_inflight = false;
            member->result_terminal_resolved = true;
            member->user = 0;
        }
        if (AllResultsResolved(round->second) && round->second.room) {
            g_result_delivery_complete_rooms[round->second.room] = true;
        }
        if (AllCommitted(round->second) &&
            AllResultsResolved(round->second)) {
            EraseRoundLocked(round);
        }
    }
    g_recovery_requests.erase(user);
    g_pending_claims.erase(user);
    g_current_round.erase(user);
}

}  // namespace zombie_mode_0828

Dispatcher_ToBeZombie::Dispatcher_ToBeZombie()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_ToBeZombie::~Dispatcher_ToBeZombie() {}

int Dispatcher_ToBeZombie::dispatch_sig(CUser* user, PacketBuf& packet) {
    uint16_t reserved_zero = 0xffffu;
    uint32_t client_actor_key = 0u;
    // CLIENT_IDA_EVIDENCE: sub_533E20@0x5340B1 writes fixed u16(0), then
    // the character object's vslot+0x928 u32, and has no response handler.
    if (packet.get_short(reserved_zero) != 1 ||
        packet.get_int(client_actor_key) != 1 || reserved_zero != 0u ||
        !zombie_mode::ConfirmZombie(user, client_actor_key)) {
        zombie_mode::TraceFailure("rejected CMD536", user);
    }
    return 0;
}

int Dispatcher_ToBeZombie::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet; (void)message; return 0;
}
int Dispatcher_ToBeZombie::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user; (void)message; (void)param; return 0;
}

Dispatcher_ZombieModeResultScore::
    Dispatcher_ZombieModeResultScore()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_ZombieModeResultScore::~
    Dispatcher_ZombieModeResultScore() {}

int Dispatcher_ZombieModeResultScore::dispatch_sig(CUser* user,
                                                        PacketBuf& packet) {
    uint16_t reserved_zero = 0xffffu;
    uint8_t outcome = 0xffu;
    uint8_t count = 0xffu;
    std::vector<zombie_mode::ClientTelemetry> telemetry;
    std::set<uint16_t> seats;
    if (packet.get_short(reserved_zero) != 1 ||
        packet.get_byte(outcome) != 1 || packet.get_byte(count) != 1 ||
        reserved_zero != 0u || outcome > zombie_mode::OUTCOME_DRAW ||
        count < zombie_mode::kMinimumPlayers ||
        count > zombie_mode::kMaximumPlayers) {
        zombie_mode::TraceFailure("malformed CMD537 header", user);
        return 0;
    }
    for (uint8_t index = 0u; index < count; ++index) {
        zombie_mode::ClientTelemetry value;
        if (packet.get_short(value.seat) != 1 ||
            packet.get_byte(value.killed_zombies) != 1 ||
            packet.get_byte(value.killed_humans) != 1 ||
            packet.get_byte(value.final_status) != 1 ||
            packet.get_int(value.attack_score) != 1 ||
            packet.get_int(value.survival_ms) != 1 ||
            packet.get_int(value.total_score) != 1 ||
            value.seat >= zombie_mode::kMaximumSeats ||
            !seats.insert(value.seat).second) {
            zombie_mode::TraceFailure("malformed CMD537 record", user);
            return 0;
        }
        telemetry.push_back(value);
    }

    bool quorum_frozen = false;
    uint16_t sender_seat = 0xffffu;
    if (!zombie_mode::SubmitTelemetrySnapshot(
            user, outcome, telemetry, quorum_frozen, sender_seat)) {
        zombie_mode::TraceFailure("CMD537 telemetry rejected", user);
        return 0;
    }
    cMyTrace trace("Dispatcher_ZombieModeResultScore::dispatch_sig",
                   __LINE__, 5);
    trace("[ZOMBIE_RESULT][SNAPSHOT] seat=%u count=%u quorum_frozen=%u",
          static_cast<unsigned int>(sender_seat),
          static_cast<unsigned int>(count),
          quorum_frozen ? 1u : 0u);
    // Sender-only command: never ACK.  Only a strict peer quorum can freeze
    // attack/total; kills, role and survival remain server authoritative.
    return 0;
}

int Dispatcher_ZombieModeResultScore::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    (void)packet; (void)message; return 0;
}
int Dispatcher_ZombieModeResultScore::process(CUser* user,
                                                   MSG_BASE& message,
                                                   ParamBase& param) {
    (void)user; (void)message; (void)param; return 0;
}

InterZombieModeResult::InterZombieModeResult()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              zombie_mode::kTaiwanInnerCode)) {}
InterZombieModeResult::~InterZombieModeResult() {}

int InterZombieModeResult::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    if (payload && payload_size == static_cast<int>(
            sizeof(zombie_mode::ResultDeliveryContinuation))) {
        const zombie_mode::ResultDeliveryContinuation continuation =
            *reinterpret_cast<
                const zombie_mode::ResultDeliveryContinuation*>(payload);
        if (continuation.action ==
                zombie_mode::kResultContinuationTelemetryTick) {
            zombie_mode::ExecuteResultTelemetryContinuation(continuation);
        } else if (continuation.action ==
                       zombie_mode::kResultContinuationDeliveryTimeout) {
            zombie_mode::ExecuteResultDeliveryTimeoutContinuation(
                continuation);
        }
        return 0;
    }
    if (!user || !payload || payload_size != static_cast<int>(
            sizeof(zombie_mode::Message))) return 0;
    const zombie_mode::Message response =
        *reinterpret_cast<const zombie_mode::Message*>(payload);
    if (response.account_id !=
            static_cast<uint32_t>(user->get_acc_id()) ||
        response.charac_no !=
            static_cast<uint32_t>(user->getCurCharacNo()) ||
        response.server_group != user->GetServerGroup() ||
        response.operation < zombie_mode::OPERATION_BEGIN_ENTRY ||
        response.operation > zombie_mode::OPERATION_RECOVER ||
        response.claim_state > zombie_mode::CLAIM_STATE_COMMITTED)
        return 0;

    if (response.operation == zombie_mode::OPERATION_BEGIN_ENTRY) {
        bool queue_settle = false;
        {
            MutexGuard guard(&zombie_mode::g_zombie_mutex);
            const zombie_mode::RoundKey key(response.round_token_hi,
                                                  response.round_token_lo);
            std::map<zombie_mode::RoundKey,
                     zombie_mode::RoundState>::iterator round =
                zombie_mode::g_rounds.find(key);
            if (round == zombie_mode::g_rounds.end()) return 0;
            zombie_mode::MemberState* const member =
                zombie_mode::FindMemberByCharacter(
                    round->second, response.account_id, response.charac_no);
            if (!member || member->entry_request.request_token !=
                               response.request_token) return 0;
            if (response.result == zombie_mode::RESULT_SUCCESS &&
                response.daily_ordinal != 0u) {
                member->daily_ordinal = response.daily_ordinal;
                member->entry_state = zombie_mode::ENTRY_DURABLE;
                queue_settle =
                    round->second.phase == zombie_mode::ROUND_FINISHED;
            } else {
                member->entry_state = zombie_mode::ENTRY_IDLE;
            }
        }
        if (queue_settle) zombie_mode::QueueNextSettlement(user);
        // Taiwan InterDispatcher treats every non-zero handler return as a
        // fatal dispatch error and disconnects the user.  This reply has
        // already been consumed, irrespective of the feature result.
        return 0;
    }

    if (response.operation == zombie_mode::OPERATION_SETTLE) {
        {
            MutexGuard guard(&zombie_mode::g_zombie_mutex);
            const zombie_mode::RoundKey key(response.round_token_hi,
                                                  response.round_token_lo);
            std::map<zombie_mode::RoundKey,
                     zombie_mode::RoundState>::iterator round =
                zombie_mode::g_rounds.find(key);
            if (round == zombie_mode::g_rounds.end()) return 0;
            zombie_mode::MemberState* const member =
                zombie_mode::FindMemberByCharacter(
                    round->second, response.account_id, response.charac_no);
            if (!member || !member->settle_queued ||
                member->settle_request.request_token !=
                    response.request_token)
                return 0;
        }
        if (response.result != zombie_mode::RESULT_SUCCESS ||
            response.claim_state !=
                zombie_mode::CLAIM_STATE_RESERVED) {
            {
                MutexGuard guard(&zombie_mode::g_zombie_mutex);
                const zombie_mode::RoundKey key(
                    response.round_token_hi, response.round_token_lo);
                std::map<zombie_mode::RoundKey,
                         zombie_mode::RoundState>::iterator round =
                    zombie_mode::g_rounds.find(key);
                if (round != zombie_mode::g_rounds.end()) {
                    zombie_mode::MemberState* const member =
                        zombie_mode::FindMemberByCharacter(
                            round->second, response.account_id,
                            response.charac_no);
                    if (member && member->settle_request.request_token ==
                                      response.request_token) {
                        member->settle_queued = false;
                    }
                }
            }
            zombie_mode::TraceFailure("SQL026 reserve failed", user);
            return 0;
        }
        zombie_mode::PrepareAndQueueTargetStore(user, response, false);
        return 0;
    }

    if (response.operation == zombie_mode::OPERATION_RECOVER) {
        {
            MutexGuard guard(&zombie_mode::g_zombie_mutex);
            std::map<CUser*, zombie_mode::Message>::iterator request =
                zombie_mode::g_recovery_requests.find(user);
            if (request == zombie_mode::g_recovery_requests.end() ||
                request->second.request_token != response.request_token)
                return 0;
            zombie_mode::g_recovery_requests.erase(request);
        }
        if (response.result == zombie_mode::RESULT_NOT_FOUND) return 0;
        if (response.result != zombie_mode::RESULT_SUCCESS ||
            (response.claim_state !=
                 zombie_mode::CLAIM_STATE_RESERVED &&
             response.claim_state !=
                 zombie_mode::CLAIM_STATE_TARGET_STORED)) {
            zombie_mode::TraceFailure("SQL026 recovery failed", user);
            return 0;
        }
        if (response.claim_state ==
                zombie_mode::CLAIM_STATE_RESERVED) {
            zombie_mode::PrepareAndQueueTargetStore(
                user, response, true);
        } else {
            zombie_mode::ApplyStoredTargetAndQueueFinal(
                user, response, true);
        }
        return 0;
    }

    if (response.operation == zombie_mode::OPERATION_COMMIT) {
        zombie_mode::HandleCommitResponse(user, response);
        return 0;
    }
    return 0;
}
