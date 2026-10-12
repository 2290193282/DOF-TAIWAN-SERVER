#ifndef OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_MODE_RESULT_H_
#define OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_MODE_RESULT_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_game/dungeon/instance/zombie_mode_result_types.h"

#include <stdint.h>
#include <vector>

class CUser;
class PacketBuf;
class PacketGuard;
class PvP_Room;

namespace zombie_mode {

struct ZombiePlayModePolicy {
    uint8_t player_count;
    uint8_t initial_zombie_count;
    uint16_t reserved;
    uint32_t dungeon_index;
    // These are player-form archetypes from monster.lst, not spawned mobs
    // and not skill ids.  Their MOB ridableskill blocks own the form skills;
    // NOTI474 only selects seats and the 140828 client applies the forms.
    uint32_t mother_zombie_form_id;
    uint32_t infected_zombie_form_id;
};

struct ItemBoxPolicy {
    uint32_t passive_index;
    uint16_t world_x;
    uint16_t world_y;
    uint32_t first_delay_ms;
    uint32_t repeat_delay_ms;
    uint8_t initial_zombie_count;
    uint8_t reserved[3];
};

struct ResultPolicy {
    uint32_t reward_item_id;
    uint32_t daily_limit;
    uint32_t immunity_item_id;
    uint32_t champion_level;
    uint32_t full_counts[kMaximumSeats];
    uint32_t limited_counts[kMaximumSeats];
    uint32_t reward_exp[kMaximumSeats];
};

enum ZombieDeathAction {
    ZOMBIE_DEATH_REJECTED = 0,
    ZOMBIE_DEATH_HUMAN_INFECTED = 1,
    ZOMBIE_DEATH_ZOMBIE_ELIMINATED = 2,
};

// Pointer-bearing channel-thread result; it never crosses the DB/Taiwan ABI.
// Physical seats remain the original PvP_Room seats so a degraded five-user
// round may keep an arbitrary sixth-seat hole without compaction.
struct ZombieDeathResolution {
    ZombieDeathResolution()
        : action(ZOMBIE_DEATH_REJECTED), dead_seat(0xffffu),
          killer_seat(0xffffu), terminal(false), outcome(OUTCOME_DRAW),
          last_survivor(0) {}

    uint8_t action;
    uint16_t dead_seat;
    uint16_t killer_seat;
    bool terminal;
    uint8_t outcome;
    CUser* last_survivor;
};

// Strictly imports Etc/GameMode/ZombieMode.etc, including the complete score,
// reward, daily-limit and immunity-item policy.  A malformed/mismatched 0828
// file disables this feature closed.
bool LoadConfig();
bool IsConfigLoaded();

// Read-only snapshots of the single imported ZombieMode.etc policy.  These
// getters keep item-box, room and settlement consumers on the same PVF
// authority instead of repeating instance values in each subsystem.
bool GetPlayModePolicy(uint8_t player_count, ZombiePlayModePolicy& output);
uint32_t GetReadyTimeMs();
uint32_t GetPlayTimeMs();
bool GetItemBoxPolicy(ItemBoxPolicy& output);
bool GetResultPolicy(ResultPolicy& output);

// Authoritative 8-seat lifecycle.  The matching producer calls BeginRoom
// only after it has materialized the official 5- or 6-user PvP_Room.
bool BeginRoom(PvP_Room* room, uint8_t play_mode);
void ResetRoom(PvP_Room* room);
bool IsActiveRoom(PvP_Room* room);
// Broader than IsActiveRoom: remains true through result settlement so every
// revival path stays closed after a Zombie has been permanently eliminated.
bool IsRevivalBlocked(PvP_Room* room);
// Read-only terminal gate used by the project-owned matching handoff before
// PvP_Room::reset erases the round.  Only ROUND_FINISHED may emit cohort
// COMPLETE; an absent/in-progress round remains fail-closed.
bool IsFinishedRoom(PvP_Room* room);
// True only after every active physical seat has received (or the 115-second
// watchdog has terminally resolved) its Zombie-specific NOTI475 panel.
bool IsResultDeliveryComplete(PvP_Room* room);

// CMD535/NOTI474 room path.  The server chooses exactly the PVF-owned zombie
// count; the client never supplies role seats.
int SelectInitialZombies(PvP_Room* room,
                         std::vector<uint32_t>& selected_seats,
                         bool& newly_selected);

// Validated Zombie combat death transaction.  Both participants must belong
// to the same running room, be live, and have opposite roles.  A Human victim
// becomes a live Zombie immediately; a Zombie victim becomes permanently
// eliminated.  The caller projects the returned action to PvP_Room live flags
// and broadcasts NOTI46 only after this function succeeds.
int ResolveCharacterDeath(PvP_Room* room,
                          CUser* dead_user,
                          CUser* killer_user,
                          ZombieDeathResolution& resolution);

// Score producers are called only from validated server battle events.
void RecordDamage(PvP_Room* room, CUser* attacker, uint32_t damage);
void RecordAttack(PvP_Room* room, CUser* attacker, bool serious);
// Samples server time at a real PvP heartbeat.  Repeated heartbeats in the
// same second are idempotent and FinishRound accounts for the final tail.
void RecordSurvival(PvP_Room* room, CUser* user);

// Reuses the existing PvP chat packets but restricts their recipient set to
// the sender's current Zombie/Human role.  Returns false for non-Zombie rooms
// so callers can retain the original PvP broadcast path.
bool SendFactionChat(PvP_Room* room, CUser* sender, PacketGuard& packet);

// The authoritative timeout/winner owner closes combat and enters a bounded
// CMD537 result-pending phase.  Rank/reward/SQL026 begin only after telemetry
// quorum freezes, or after the ten-second fail-closed telemetry timeout.
bool FinishRound(PvP_Room* room,
                 uint8_t outcome,
                 CUser* last_survivor);
bool FinishByTimeout(PvP_Room* room);
void TickResultTelemetry(PvP_Room* room, uint32_t now_ms);
// At the authority-owned 115-second settlement watchdog, MainLoop enqueues a
// pointer-free DISPATCH_Q continuation.  DispatchThread then re-resolves the
// round/session and sends online unresolved users full statistics with a zero
// local reward tail.  Durable DB claims remain independently recoverable.
void ForceResultDeliveryTimeout(PvP_Room* room, uint32_t now_ms);

// Character-session recovery drains SQL026 claims left RESERVED or
// TARGET_STORED by disconnect/process failure.
void BeginUserSession(CUser* user);
void ResetUserSession(CUser* user);

}  // namespace zombie_mode_0828

class Dispatcher_ToBeZombie
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ToBeZombie();
    virtual ~Dispatcher_ToBeZombie();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_ZombieModeResultScore
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ZombieModeResultScore();
    virtual ~Dispatcher_ZombieModeResultScore();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class InterZombieModeResult : public TaiwanInterfaceInterDispatcher {
public:
    InterZombieModeResult();
    virtual ~InterZombieModeResult();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ToBeZombie) == 0x04u,
              "CMD536 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_ZombieModeResultScore) == 0x04u,
              "CMD537 dispatcher must remain vptr-only on i386");
static_assert(sizeof(InterZombieModeResult) == 0x08u,
              "Zombie result Inter owner must preserve the base ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_MODE_RESULT_0828_H_
