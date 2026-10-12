#ifndef OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_MAP_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_MAP_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

class CBattle_Field;
class CParty;
class CUser;
class MapInfo;
class PacketGuard;
class PvP_Room;
struct MSG_MOVE_MAP;

namespace zombie_map_runtime {

// All hooks are fail-closed once an adapter is recognized.  Callers may use
// the original path only for kHookNotManaged.
enum HookResult {
    kHookRejected = -1,
    kHookNotManaged = 0,
    kHookHandled = 1,
};

enum { kZombieDungeonIndex = 3101, kMaximumSeats = 8 };

struct SeatBinding {
    SeatBinding()
        : global_slot(0xffu), user(0), party(0), field(0), party_index(-1) {}

    uint8_t global_slot;
    CUser* user;
    CParty* party;
    CBattle_Field* field;
    int party_index;
};

struct ItemBoxConfig {
    ItemBoxConfig()
        : passive_index(0u), world_x(0u), world_y(0u),
          first_delay_ms(0u), repeat_delay_ms(0u) {}

    uint32_t passive_index;
    uint16_t world_x;
    uint16_t world_y;
    uint32_t first_delay_ms;
    uint32_t repeat_delay_ms;
};

typedef void (*AbortRoomCallback)(PvP_Room* room,
                                  uint32_t generation,
                                  const char* reason);

// INTEGRATION TODO (matching authority): register after all hidden adapters
// are attached but before their dungeon_start(3101), arm after the final
// initial 5/6 load mask is frozen, and activate only after BeginRoom succeeds.
bool RegisterBootstrap(PvP_Room* room,
                       int room_index,
                       uint32_t generation,
                       uint8_t expected_mask,
                       const SeatBinding* seats,
                       size_t seat_count);
bool ArmRoom(PvP_Room* room,
             uint32_t generation,
             uint32_t cohort_load_epoch,
             uint8_t active_mask);
bool ActivateRoom(PvP_Room* room, uint32_t generation);
bool DropSeat(PvP_Room* room, uint32_t generation, uint8_t global_slot);

// Two-phase teardown keeps canonical MapInfo alive while adapter destructors
// run, but removes every reverse lookup before their pool memory is reusable.
bool BeginRoomTermination(PvP_Room* room, uint32_t generation);
bool DetachRoomAdapters(PvP_Room* room, uint32_t generation);
void FinishRoomTermination(PvP_Room* room, uint32_t generation);

void InstallAbortRoomCallback(AbortRoomCallback callback);

bool IsRegisteredParty(CParty* party);

// Canonical mutable map storage.  The returned MapInfo is non-owning and is
// never inserted into any CBattle_Field::map_infos_; RoomState is its sole
// owner.  kHookRejected must never fall back to a local map.
HookResult ResolveCanonicalMapInfo(CBattle_Field* field,
                                   int pos_x,
                                   int pos_y,
                                   bool layered,
                                   MapInfo** output);
// The 0828 zombi2 maze binds every playable coordinate through
// [map specification].  Resolve that binding before the legacy
// CDungeon::GetStartMap(grid_type) random selector is allowed to run.
HookResult ResolveCanonicalStartMapIndex(CBattle_Field* field,
                                         int pos_x,
                                         int pos_y,
                                         int* output);
HookResult InitializeCanonicalStartCell(CBattle_Field* field,
                                        int pos_x,
                                        int pos_y,
                                        int map_index,
                                        uint8_t map_layer);

int ResolveGlobalFocusSeat(CBattle_Field* field);
HookResult RouteDungeonInfoPacket(CParty* party, PacketGuard& packet);
HookResult RouteStartMapPacket(CParty* party, PacketGuard& packet);
// The 0828 initial bootstrap is a strict ordered transaction:
// central NOTI28 -> optional owner fatigue buff frames + NOTI487 -> one
// owner-owned NOTI29 per socket -> optional owner NOTI276 -> central NOTI414.
// The initial NOTI29 keeps the official focus byte 0xff; immutable global
// focus starts only on later owner CMD45 map moves.  This retained scene
// bootstrap is deliberately published before CMD295; later gameplay/start
// transitions remain sealed behind the CMD296/NOTI279 P2P barrier.
HookResult RouteBootstrapFatiguePacket(CUser* user,
                                       PacketGuard& packet,
                                       bool terminal_state_packet);
HookResult RouteBootstrapSecretShopPacket(CUser* user,
                                          PacketGuard& packet);
HookResult RouteBootstrapTagCharacterPacket(CParty* party,
                                            PacketGuard& packet);
// All required initial packets are retained while the six hidden map
// adapters are assembled.  The matching authority releases the ordered
// transaction after the six-seat presentation and before CMD295: START_MAP is
// what activates the 0828 client load/P2P controller.
bool SealBootstrapPackets(PvP_Room* room, uint32_t generation);
bool PublishBootstrapPackets(PvP_Room* room, uint32_t generation);
HookResult RouteAdapterPacket(CParty* party, PacketGuard& packet);

// CMD45 owner-first bridge.  The adapter still executes the original
// singleton seat-0 CParty/CBattle_Field path inside a room move lease; its
// MapInfo lookups and packet focus are supplied by this runtime.
HookResult ConsumeMoveMap(CUser* user,
                          MSG_MOVE_MAP& message,
                          int* move_result);
HookResult CommitMoveMap(CParty* party, int pos_x, int pos_y);

// Per-seat post-initial map-load generation.  These APIs intentionally do
// not own the initial cohort barrier; the matching authority calls them first
// only while the room is ACTIVE.
HookResult ConsumeSeatFinishLoading(CParty* party,
                                    CUser* user,
                                    int32_t inventory_checksum,
                                    int32_t skill_checksum,
                                    uint32_t* completed_map_epoch);
bool ConsumeSeatLoadTimeout(CParty* party);

// One central room timer and claim-once table.  Until StartRoomItemBox is
// called by the matching authority, a registered Zombie room stays closed.
bool StartRoomItemBox(PvP_Room* room,
                      uint32_t generation,
                      const ItemBoxConfig& config);
HookResult OnRoomItemBoxTimer(int room_index,
                              uint32_t timer_key,
                              uint32_t generation);
HookResult ClaimRoomItemBox(CUser* user,
                            uint32_t runtime_uid,
                            int* error,
                            uint32_t* provenance_token);
bool ConsumeItemBoxProvenance(CUser* user, uint32_t* provenance_token);

// CMD541 and similar client-driven map-object movement has one stable host
// per occupied canonical cell: the lowest live global physical seat.
HookResult AuthorizeCellMonsterHost(CUser* user,
                                    CParty* party,
                                    uint16_t monster_unique_id);

}  // namespace zombie_map_runtime

#endif  // OLDSRV_OLD_GAME_DUNGEON_INSTANCE_ZOMBIE_MAP_RUNTIME_H_
