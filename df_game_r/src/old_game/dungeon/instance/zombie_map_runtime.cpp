#include "old_game/dungeon/instance/zombie_map_runtime.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <deque>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "common/manager/global_lifecycle.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/player/core/cuser.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/world/broadcast/world_messages.h"

namespace zombie_map_runtime {
namespace {

enum RoomPhase {
    ROOM_BOOTSTRAP = 1,
    ROOM_ARMED = 2,
    ROOM_ACTIVE = 3,
    ROOM_FAILED = 4,
    ROOM_TERMINATING = 5,
};

struct CellKey {
    CellKey() : x(0), y(0) {}
    CellKey(int input_x, int input_y) : x(input_x), y(input_y) {}

    bool operator<(const CellKey& rhs) const {
        return x < rhs.x || (x == rhs.x && y < rhs.y);
    }
    bool operator==(const CellKey& rhs) const {
        return x == rhs.x && y == rhs.y;
    }

    int x;
    int y;
};

struct CellState {
    CellState() : info(new MapInfo()), initialized(false) {}

    boost::shared_ptr<MapInfo> info;
    bool initialized;
};

struct SeatState {
    SeatState()
        : global_slot(0xffu), user(0), party(0), field(0), party_index(-1),
          positioned(false), dropped(false), x(-1), y(-1), last_x(-1),
          last_y(-1), map_load_epoch(0u), map_load_pending(false),
          provenance_tokens() {}

    uint8_t global_slot;
    CUser* user;
    CParty* party;
    CBattle_Field* field;
    int party_index;
    bool positioned;
    bool dropped;
    int x;
    int y;
    int last_x;
    int last_y;
    uint32_t map_load_epoch;
    bool map_load_pending;
    std::deque<uint32_t> provenance_tokens;
};

struct BoxSpawn {
    BoxSpawn() : cell(), claimed(false) {}
    explicit BoxSpawn(const CellKey& input) : cell(input), claimed(false) {}

    CellKey cell;
    bool claimed;
};

struct RoomState {
    RoomState()
        : room(0), room_index(-1), generation(0u), cohort_load_epoch(0u),
          expected_mask(0u), active_mask(0u), phase(ROOM_BOOTSTRAP),
          adapters_detached(false), dungeon_info_sent(false),
          bootstrap_packets_sealed(false),
          bootstrap_packets_published(false), pending_dungeon_info(),
          move_in_progress(false),
          move_slot(0xffu), move_target_x(-1), move_target_y(-1),
          item_box_started(false), item_box_timer_key(0u),
          next_provenance_token(1u), item_box_config(),
          failure_info(new MapInfo()), cells(), box_spawns() {
        for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
            fatigue_state_ready[slot] = false;
            secret_shop_ready[slot] = false;
            tag_character_ready[slot] = false;
        }
    }

    Mutex mutex;
    PvP_Room* room;
    int room_index;
    uint32_t generation;
    uint32_t cohort_load_epoch;
    uint8_t expected_mask;
    uint8_t active_mask;
    RoomPhase phase;
    bool adapters_detached;
    bool dungeon_info_sent;
    bool bootstrap_packets_sealed;
    bool bootstrap_packets_published;
    boost::shared_ptr<PacketBuf> pending_dungeon_info;
    std::vector<boost::shared_ptr<PacketBuf> >
        pending_fatigue_packets[kMaximumSeats];
    bool fatigue_state_ready[kMaximumSeats];
    boost::shared_ptr<PacketBuf> pending_start_maps[kMaximumSeats];
    boost::shared_ptr<PacketBuf> pending_secret_shop[kMaximumSeats];
    bool secret_shop_ready[kMaximumSeats];
    bool tag_character_ready[kMaximumSeats];
    bool move_in_progress;
    uint8_t move_slot;
    int move_target_x;
    int move_target_y;
    SeatState seats[kMaximumSeats];
    bool item_box_started;
    uint32_t item_box_timer_key;
    uint32_t next_provenance_token;
    ItemBoxConfig item_box_config;
    boost::shared_ptr<MapInfo> failure_info;
    std::map<CellKey, CellState> cells;
    std::map<uint32_t, BoxSpawn> box_spawns;
};

struct AdapterRef {
    AdapterRef()
        : room(), room_ptr(0), generation(0u), global_slot(0xffu),
          party_index(-1) {}

    boost::weak_ptr<RoomState> room;
    PvP_Room* room_ptr;
    uint32_t generation;
    uint8_t global_slot;
    int party_index;
};

Mutex g_registry_mutex;
std::map<PvP_Room*, boost::shared_ptr<RoomState> > g_rooms;
std::map<int, boost::weak_ptr<RoomState> > g_room_indices;
std::map<CParty*, AdapterRef> g_party_adapters;
std::map<CBattle_Field*, AdapterRef> g_field_adapters;
uint32_t g_next_item_box_timer_key = 1u;
AbortRoomCallback g_abort_room_callback = 0;

uint8_t SeatBit(size_t slot) {
    return slot < kMaximumSeats
               ? static_cast<uint8_t>(1u << slot)
               : static_cast<uint8_t>(0u);
}

size_t CountSeats(uint8_t mask) {
    size_t count = 0u;
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
        if ((mask & SeatBit(slot)) != 0u) ++count;
    }
    return count;
}

uint32_t NextNonZero(uint32_t& value) {
    uint32_t result = value++;
    if (result == 0u) result = value++;
    if (value == 0u) ++value;
    return result;
}

uint32_t AdvanceNonZero(uint32_t& value) {
    ++value;
    if (value == 0u) ++value;
    return value;
}

int ExpectedMapIndex(int x, int y) {
    if (x == 0 && y == 1) return 11009;
    if (x == 0 && y == 2) return 11010;
    if (x == 1 && y == 0) return 11011;
    if (x == 1 && y == 1) return 11012;
    if (x == 1 && y == 2) return 11013;
    if (x == 2 && y == 0) return 11014;
    if (x == 2 && y == 1) return 11015;
    return 0;
}

boost::shared_ptr<PacketBuf> CloneFinalizedPacket(PacketGuard& packet) {
    PacketBuf* const source = packet.get();
    if (!source || !packet.is_finallized() ||
        !source->isEncryptRequired() || packet.get_len() <= 0 ||
        packet.get_len() > PacketBuf::PACKET_MAX_SIZE) {
        return boost::shared_ptr<PacketBuf>();
    }
    boost::shared_ptr<PacketBuf> copy(new PacketBuf());
    copy->copy(*source);
    return copy;
}

bool ReadFinalizedStartMapFocus(PacketBuf* packet,
                                unsigned int& wire_focus) {
    wire_focus = 0u;
    if (!packet || !packet->is_finallized()) return false;

    const int frame_len = packet->get_len();
    // PacketBuf::finalize calls _NS_PI_MakeHash_NOTI before fixing length;
    // that helper appends one native 32-bit integrity word.  NOTI29 is not in
    // PacketBuf::isCompressRequired's {2, 9, 87} set, so the last semantic
    // byte remains exactly five bytes before the finalized frame end:
    // [focus:u8][integrity:u32].  Reading frame_len - 1 observes the hash,
    // not the focus.  The 22:06 focus_non_ff=0x3f trace was the six-seat
    // rejection mask; it proves all six frames took the wrong branch but does
    // not expose any individual hash byte.
    const int integrity_bytes = static_cast<int>(sizeof(int32_t));
    if (frame_len <= PacketBuf::PACKET_HEADER_SIZE + integrity_bytes)
        return false;

    const SC_Header* const header = reinterpret_cast<const SC_Header*>(
        packet->get_packet(0));
    if (!header || header->cmd != ENUM_PACKETCLASS_NOTIPACKET ||
        header->type != ENUM_NOTIPACKET_START_MAP ||
        header->length != static_cast<uint32_t>(frame_len)) {
        return false;
    }

    const int focus_offset = frame_len - integrity_bytes - 1;
    wire_focus = static_cast<unsigned int>(static_cast<unsigned char>(
        packet->get_packet(focus_offset)[0]));
    return true;
}

boost::shared_ptr<RoomState> FindRoom(PvP_Room* room,
                                      uint32_t generation) {
    MutexGuard guard(&g_registry_mutex);
    std::map<PvP_Room*, boost::shared_ptr<RoomState> >::const_iterator found =
        g_rooms.find(room);
    if (found == g_rooms.end() || !found->second ||
        found->second->generation != generation) {
        return boost::shared_ptr<RoomState>();
    }
    return found->second;
}

bool FindPartyAdapter(CParty* party,
                      boost::shared_ptr<RoomState>& room,
                      AdapterRef& adapter) {
    room.reset();
    if (!party) return false;
    MutexGuard guard(&g_registry_mutex);
    std::map<CParty*, AdapterRef>::const_iterator found =
        g_party_adapters.find(party);
    if (found == g_party_adapters.end()) return false;
    adapter = found->second;
    room = adapter.room.lock();
    return room && room->room == adapter.room_ptr &&
           room->generation == adapter.generation;
}

bool FindFieldAdapter(CBattle_Field* field,
                      boost::shared_ptr<RoomState>& room,
                      AdapterRef& adapter) {
    room.reset();
    if (!field) return false;
    MutexGuard guard(&g_registry_mutex);
    std::map<CBattle_Field*, AdapterRef>::const_iterator found =
        g_field_adapters.find(field);
    if (found == g_field_adapters.end()) return false;
    adapter = found->second;
    room = adapter.room.lock();
    return room && room->room == adapter.room_ptr &&
           room->generation == adapter.generation;
}

bool ValidateAdapterLocked(const RoomState& room,
                           const AdapterRef& adapter,
                           CParty* party,
                           CBattle_Field* field,
                           CUser* user) {
    if (adapter.global_slot >= kMaximumSeats ||
        room.generation != adapter.generation ||
        room.room != adapter.room_ptr) {
        return false;
    }
    const SeatState& seat = room.seats[adapter.global_slot];
    if (seat.dropped || seat.global_slot != adapter.global_slot ||
        seat.party != party || seat.field != field ||
        seat.party_index != adapter.party_index ||
        !seat.user || (user && seat.user != user)) {
        return false;
    }
    return party && field && party->GetPartyIndex() == seat.party_index &&
           &party->battle_field() == field &&
           seat.user->GetParty() == party &&
           seat.user->GetPartyIndex() == seat.party_index;
}

void SnapshotRecipientsLocked(RoomState& room,
                              bool bootstrap,
                              std::vector<CUser*>& recipients) {
    recipients.clear();
    const uint8_t mask = bootstrap ? room.expected_mask : room.active_mask;
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
        if ((mask & SeatBit(slot)) == 0u) continue;
        SeatState& seat = room.seats[slot];
        if (!seat.dropped && seat.user &&
            room.room->get_user_seat(static_cast<int>(slot)) == seat.user) {
            recipients.push_back(seat.user);
        }
    }
}

HookResult SendRecipients(const std::vector<CUser*>& recipients,
                          PacketGuard& packet) {
    if (recipients.empty()) return kHookRejected;
    bool sent = true;
    for (std::vector<CUser*>::const_iterator it = recipients.begin();
         it != recipients.end(); ++it) {
        if (!*it || (*it)->Send(packet) == 0) sent = false;
    }
    return sent ? kHookHandled : kHookRejected;
}

void MarkRoomFailed(const boost::shared_ptr<RoomState>& room,
                    const char* reason) {
    if (!room) return;
    AbortRoomCallback callback = 0;
    PvP_Room* target_room = 0;
    uint32_t generation = 0u;
    {
        MutexGuard guard(&room->mutex);
        if (room->phase == ROOM_TERMINATING) return;
        room->phase = ROOM_FAILED;
        room->move_in_progress = false;
        room->item_box_started = false;
        target_room = room->room;
        generation = room->generation;
    }
    {
        MutexGuard guard(&g_registry_mutex);
        callback = g_abort_room_callback;
    }
    cMyTrace trace("zombie_map_runtime::MarkRoomFailed", __LINE__, 5);
    trace("[ZOMBIE_MAP][FAIL] room=%d generation=%u reason=%s",
          room->room_index,
          static_cast<unsigned int>(generation),
          reason ? reason : "unknown");
    if (callback) callback(target_room, generation, reason);
}

HookResult LookupCellForParty(CParty* party,
                              boost::shared_ptr<RoomState>& room,
                              AdapterRef& adapter,
                              CellKey& cell) {
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    MutexGuard guard(&room->mutex);
    if (!ValidateAdapterLocked(*room, adapter, party,
                               &party->battle_field(), 0) ||
        room->phase == ROOM_FAILED || room->phase == ROOM_TERMINATING) {
        return kHookRejected;
    }
    SeatState& seat = room->seats[adapter.global_slot];
    if (!seat.positioned) return kHookRejected;
    cell = CellKey(seat.x, seat.y);
    return kHookHandled;
}

void BuildItemBoxGeneratedPacket(PacketGuard& packet,
                                 uint32_t runtime_uid,
                                 const ItemBoxConfig& config,
                                 const CellKey& cell) {
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_GENERATE_ITEMBOX);
    packet.put_int(static_cast<int>(runtime_uid));
    packet.put_int(static_cast<int>(config.passive_index));
    packet.put_short(cell.x);
    packet.put_short(cell.y);
    packet.put_short(config.world_x);
    packet.put_short(config.world_y);
    packet.put_short(0);
    packet.finalize(true);
}

void BuildItemBoxClaimedPacket(PacketGuard& packet, uint32_t runtime_uid) {
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_GET_ITEMBOX);
    packet.put_int(static_cast<int>(runtime_uid));
    packet.finalize(true);
}

}  // namespace

bool RegisterBootstrap(PvP_Room* room,
                       int room_index,
                       uint32_t generation,
                       uint8_t expected_mask,
                       const SeatBinding* seats,
                       size_t seat_count) {
    if (!room || room_index < 0 || room->get_index() != room_index ||
        generation == 0u || !seats || seat_count != 6u ||
        expected_mask != static_cast<uint8_t>(0x3fu) ||
        CountSeats(expected_mask) != seat_count) {
        return false;
    }

    boost::shared_ptr<RoomState> state(new RoomState());
    state->room = room;
    state->room_index = room_index;
    state->generation = generation;
    state->expected_mask = expected_mask;

    uint8_t seen = 0u;
    for (size_t index = 0u; index < seat_count; ++index) {
        const SeatBinding& input = seats[index];
        const uint8_t bit = SeatBit(input.global_slot);
        if (input.global_slot >= 6u || bit == 0u || (seen & bit) != 0u ||
            !input.user || !input.party || !input.field ||
            input.party_index < 0 ||
            input.party->GetPartyIndex() != input.party_index ||
            &input.party->battle_field() != input.field ||
            input.party->get_member_count() != 1 ||
            input.party->get_user(0) != input.user ||
            input.user->GetParty() != input.party ||
            input.user->GetPartyIndex() != input.party_index ||
            room->get_user_seat(input.global_slot) != input.user) {
            return false;
        }
        seen = static_cast<uint8_t>(seen | bit);
        SeatState& output = state->seats[input.global_slot];
        output.global_slot = input.global_slot;
        output.user = input.user;
        output.party = input.party;
        output.field = input.field;
        output.party_index = input.party_index;
    }
    if (seen != expected_mask) return false;

    MutexGuard guard(&g_registry_mutex);
    if (g_rooms.find(room) != g_rooms.end() ||
        g_room_indices.find(room_index) != g_room_indices.end()) {
        return false;
    }
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
        if ((seen & SeatBit(slot)) == 0u) continue;
        SeatState& seat = state->seats[slot];
        if (g_party_adapters.find(seat.party) != g_party_adapters.end() ||
            g_field_adapters.find(seat.field) != g_field_adapters.end()) {
            return false;
        }
    }
    g_rooms[room] = state;
    g_room_indices[room_index] = state;
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
        if ((seen & SeatBit(slot)) == 0u) continue;
        SeatState& seat = state->seats[slot];
        AdapterRef adapter;
        adapter.room = state;
        adapter.room_ptr = room;
        adapter.generation = generation;
        adapter.global_slot = static_cast<uint8_t>(slot);
        adapter.party_index = seat.party_index;
        g_party_adapters[seat.party] = adapter;
        g_field_adapters[seat.field] = adapter;
    }
    return true;
}

bool ArmRoom(PvP_Room* room,
             uint32_t generation,
             uint32_t cohort_load_epoch,
             uint8_t active_mask) {
    boost::shared_ptr<RoomState> state = FindRoom(room, generation);
    if (!state || cohort_load_epoch == 0u ||
        (active_mask & static_cast<uint8_t>(~state->expected_mask)) != 0u ||
        (CountSeats(active_mask) != 5u && CountSeats(active_mask) != 6u)) {
        return false;
    }
    MutexGuard guard(&state->mutex);
    if (state->phase != ROOM_BOOTSTRAP) return false;
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
        if ((active_mask & SeatBit(slot)) == 0u) continue;
        if (state->seats[slot].dropped || !state->seats[slot].party ||
            !state->seats[slot].positioned) {
            return false;
        }
    }
    state->cohort_load_epoch = cohort_load_epoch;
    state->active_mask = active_mask;
    state->phase = ROOM_ARMED;
    return true;
}

bool ActivateRoom(PvP_Room* room, uint32_t generation) {
    boost::shared_ptr<RoomState> state = FindRoom(room, generation);
    if (!state) return false;
    MutexGuard guard(&state->mutex);
    if (state->phase != ROOM_ARMED || !state->cohort_load_epoch ||
        (CountSeats(state->active_mask) != 5u &&
         CountSeats(state->active_mask) != 6u)) {
        return false;
    }
    state->phase = ROOM_ACTIVE;
    return true;
}

bool DropSeat(PvP_Room* room, uint32_t generation, uint8_t global_slot) {
    boost::shared_ptr<RoomState> state = FindRoom(room, generation);
    if (!state || global_slot >= kMaximumSeats) return false;
    CParty* party = 0;
    CBattle_Field* field = 0;
    {
        MutexGuard guard(&state->mutex);
        if (state->phase != ROOM_BOOTSTRAP && state->phase != ROOM_ARMED)
            return false;
        SeatState& seat = state->seats[global_slot];
        if (seat.dropped || !seat.party || !seat.field) return false;
        party = seat.party;
        field = seat.field;
        seat.dropped = true;
        seat.map_load_pending = false;
        state->active_mask = static_cast<uint8_t>(
            state->active_mask & static_cast<uint8_t>(~SeatBit(global_slot)));
    }
    MutexGuard guard(&g_registry_mutex);
    g_party_adapters.erase(party);
    g_field_adapters.erase(field);
    return true;
}

bool BeginRoomTermination(PvP_Room* room, uint32_t generation) {
    boost::shared_ptr<RoomState> state = FindRoom(room, generation);
    if (!state) return false;
    MutexGuard guard(&state->mutex);
    if (state->phase == ROOM_TERMINATING) return true;
    state->phase = ROOM_TERMINATING;
    state->move_in_progress = false;
    state->item_box_started = false;
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot)
        state->seats[slot].map_load_pending = false;
    return true;
}

bool DetachRoomAdapters(PvP_Room* room, uint32_t generation) {
    boost::shared_ptr<RoomState> state = FindRoom(room, generation);
    if (!state) return false;
    MutexGuard registry_guard(&g_registry_mutex);
    MutexGuard room_guard(&state->mutex);
    if (state->phase != ROOM_TERMINATING) return false;
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
        SeatState& seat = state->seats[slot];
        if (seat.party) g_party_adapters.erase(seat.party);
        if (seat.field) g_field_adapters.erase(seat.field);
    }
    state->adapters_detached = true;
    return true;
}

void FinishRoomTermination(PvP_Room* room, uint32_t generation) {
    boost::shared_ptr<RoomState> state;
    {
        MutexGuard guard(&g_registry_mutex);
        std::map<PvP_Room*, boost::shared_ptr<RoomState> >::iterator found =
            g_rooms.find(room);
        if (found == g_rooms.end() || !found->second ||
            found->second->generation != generation) {
            return;
        }
        state = found->second;
        {
            MutexGuard room_guard(&state->mutex);
            if (state->phase != ROOM_TERMINATING ||
                !state->adapters_detached) {
                return;
            }
        }
        std::map<int, boost::weak_ptr<RoomState> >::iterator indexed =
            g_room_indices.find(state->room_index);
        if (indexed != g_room_indices.end()) g_room_indices.erase(indexed);
        g_rooms.erase(found);
    }
    // state is the final local owner here.  Its CellState shared_ptr values
    // destroy each canonical MapInfo exactly once outside registry locks.
}

void InstallAbortRoomCallback(AbortRoomCallback callback) {
    MutexGuard guard(&g_registry_mutex);
    g_abort_room_callback = callback;
}

bool IsRegisteredParty(CParty* party) {
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    return FindPartyAdapter(party, room, adapter);
}

HookResult ResolveCanonicalMapInfo(CBattle_Field* field,
                                   int pos_x,
                                   int pos_y,
                                   bool layered,
                                   MapInfo** output) {
    if (output) *output = 0;
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindFieldAdapter(field, room, adapter)) return kHookNotManaged;
    MutexGuard guard(&room->mutex);
    if (output && room->failure_info) *output = room->failure_info.get();
    if (!ValidateAdapterLocked(*room, adapter,
                               room->seats[adapter.global_slot].party,
                               field, 0) ||
        room->phase == ROOM_FAILED || room->phase == ROOM_TERMINATING ||
        field->get_dungeon_index() != kZombieDungeonIndex ||
        pos_x < 0 || pos_x > 2 || pos_y < 0 || pos_y > 2) {
        return kHookRejected;
    }
    // Zombi2 has no layered cells.  Normalize legacy layered probes to the
    // same canonical descriptor instead of creating a second mutable owner.
    static_cast<void>(layered);
    CellState& cell = room->cells[CellKey(pos_x, pos_y)];
    if (!cell.info) return kHookRejected;
    if (output) *output = cell.info.get();
    return kHookHandled;
}

HookResult ResolveCanonicalStartMapIndex(CBattle_Field* field,
                                         int pos_x,
                                         int pos_y,
                                         int* output) {
    if (output) *output = 0;
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindFieldAdapter(field, room, adapter)) return kHookNotManaged;
    MutexGuard guard(&room->mutex);
    const int expected = ExpectedMapIndex(pos_x, pos_y);
    if (!output || expected <= 0 || room->phase != ROOM_BOOTSTRAP ||
        !ValidateAdapterLocked(*room, adapter,
                               room->seats[adapter.global_slot].party,
                               field, 0) ||
        field->get_dungeon_index() != kZombieDungeonIndex) {
        return kHookRejected;
    }
    *output = expected;
    return kHookHandled;
}

HookResult InitializeCanonicalStartCell(CBattle_Field* field,
                                        int pos_x,
                                        int pos_y,
                                        int map_index,
                                        uint8_t map_layer) {
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindFieldAdapter(field, room, adapter)) return kHookNotManaged;
    MutexGuard guard(&room->mutex);
    if (room->phase != ROOM_BOOTSTRAP ||
        !ValidateAdapterLocked(*room, adapter,
                               room->seats[adapter.global_slot].party,
                               field, 0) ||
        field->get_dungeon_index() != kZombieDungeonIndex ||
        ExpectedMapIndex(pos_x, pos_y) != map_index) {
        return kHookRejected;
    }
    CellState& cell = room->cells[CellKey(pos_x, pos_y)];
    if (!cell.info || cell.initialized || cell.info->map_index_ != 0) {
        return kHookRejected;
    }
    cell.info->map_index_ = map_index;
    cell.info->map_layer_ = map_layer;
    cell.initialized = true;
    SeatState& seat = room->seats[adapter.global_slot];
    seat.positioned = true;
    seat.x = pos_x;
    seat.y = pos_y;
    seat.last_x = pos_x;
    seat.last_y = pos_y;
    return kHookHandled;
}

int ResolveGlobalFocusSeat(CBattle_Field* field) {
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindFieldAdapter(field, room, adapter)) return -1;
    MutexGuard guard(&room->mutex);
    if (!ValidateAdapterLocked(*room, adapter,
                               room->seats[adapter.global_slot].party,
                               field, 0) ||
        room->phase == ROOM_FAILED || room->phase == ROOM_TERMINATING) {
        return -2;
    }

    // OFFICIAL_INITIAL_MAP_CONTRACT: CBattle_Field::reset/SelectDungeon
    // leaves current_seat_no_ at 0xff.  CParty::move_map is the first native
    // path which installs a seat for an IsEnterEachMap dungeon.  The retained
    // initial NOTI29 is produced before any CMD45, so exporting the hidden
    // adapter's global slot here makes the 0828 reader enter its focused
    // B57BB0 branch against a scene whose PvP actors are not committed yet.
    // Preserve the official 0xff sentinel for ROOM_BOOTSTRAP; after the room
    // is active, subsequent owner map moves still use their immutable global
    // slot instead of singleton party seat zero.
    const bool initial_bootstrap = room->phase == ROOM_BOOTSTRAP;
    cMyTrace trace("zombie_map_runtime::ResolveGlobalFocusSeat", __LINE__, 5);
    trace("[ZOMBIE_MAP][START_MAP_FOCUS] room=%d generation=%u phase=%s owner_slot=%u wire_focus=%u",
          room->room_index,
          static_cast<unsigned int>(room->generation),
          initial_bootstrap ? "bootstrap" : "active",
          static_cast<unsigned int>(adapter.global_slot),
          initial_bootstrap ? 255u :
              static_cast<unsigned int>(adapter.global_slot));
    if (initial_bootstrap) return -2;
    return adapter.global_slot;
}

HookResult RouteDungeonInfoPacket(CParty* party, PacketGuard& packet) {
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    {
        MutexGuard guard(&room->mutex);
        if (!ValidateAdapterLocked(*room, adapter, party,
                                   &party->battle_field(), 0) ||
            room->phase != ROOM_BOOTSTRAP ||
            room->bootstrap_packets_sealed ||
            room->bootstrap_packets_published) {
            return kHookRejected;
        }
        if (room->dungeon_info_sent) return kHookHandled;
        room->pending_dungeon_info = CloneFinalizedPacket(packet);
        if (!room->pending_dungeon_info) return kHookRejected;
        room->dungeon_info_sent = true;
    }
    return kHookHandled;
}

HookResult RouteStartMapPacket(CParty* party, PacketGuard& packet) {
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    std::vector<CUser*> recipients;
    {
        MutexGuard guard(&room->mutex);
        if (!ValidateAdapterLocked(*room, adapter, party,
                                   &party->battle_field(), 0) ||
            room->phase == ROOM_FAILED || room->phase == ROOM_TERMINATING) {
            return kHookRejected;
        }
        int x = -1;
        int y = -1;
        party->battle_field().getCurPosXY(x, y);
        SeatState& seat = room->seats[adapter.global_slot];
        if (!seat.positioned || seat.x != x || seat.y != y) {
            return kHookRejected;
        }
        const std::map<CellKey, CellState>::const_iterator cell =
            room->cells.find(CellKey(x, y));
        if (cell == room->cells.end() || !cell->second.initialized ||
            !cell->second.info ||
            cell->second.info->map_index_ != ExpectedMapIndex(x, y)) {
            return kHookRejected;
        }
        if (room->phase == ROOM_BOOTSTRAP) {
            if (room->bootstrap_packets_sealed ||
                room->bootstrap_packets_published ||
                room->pending_start_maps[adapter.global_slot]) {
                return kHookRejected;
            }
            room->pending_start_maps[adapter.global_slot] =
                CloneFinalizedPacket(packet);
            return room->pending_start_maps[adapter.global_slot]
                       ? kHookHandled
                       : kHookRejected;
        }
        SnapshotRecipientsLocked(*room, false, recipients);
    }
    return SendRecipients(recipients, packet);
}

HookResult RouteBootstrapFatiguePacket(CUser* user,
                                       PacketGuard& packet,
                                       bool terminal_state_packet) {
    CParty* const party = user ? user->GetParty() : 0;
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    MutexGuard guard(&room->mutex);
    if (!ValidateAdapterLocked(*room, adapter, party,
                               &party->battle_field(), user) ||
        room->phase == ROOM_FAILED || room->phase == ROOM_TERMINATING) {
        return kHookRejected;
    }
    if (room->phase != ROOM_BOOTSTRAP) return kHookNotManaged;
    if (room->bootstrap_packets_sealed ||
        room->bootstrap_packets_published ||
        room->fatigue_state_ready[adapter.global_slot]) {
        return kHookRejected;
    }
    boost::shared_ptr<PacketBuf> retained = CloneFinalizedPacket(packet);
    if (!retained) return kHookRejected;
    room->pending_fatigue_packets[adapter.global_slot].push_back(retained);
    if (terminal_state_packet)
        room->fatigue_state_ready[adapter.global_slot] = true;
    return kHookHandled;
}

HookResult RouteBootstrapSecretShopPacket(CUser* user,
                                          PacketGuard& packet) {
    CParty* const party = user ? user->GetParty() : 0;
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    MutexGuard guard(&room->mutex);
    if (!ValidateAdapterLocked(*room, adapter, party,
                               &party->battle_field(), user) ||
        room->phase == ROOM_FAILED || room->phase == ROOM_TERMINATING) {
        return kHookRejected;
    }
    if (room->phase != ROOM_BOOTSTRAP) return kHookNotManaged;
    if (room->bootstrap_packets_sealed ||
        room->bootstrap_packets_published ||
        room->secret_shop_ready[adapter.global_slot]) {
        return kHookRejected;
    }
    room->pending_secret_shop[adapter.global_slot] =
        CloneFinalizedPacket(packet);
    if (!room->pending_secret_shop[adapter.global_slot])
        return kHookRejected;
    room->secret_shop_ready[adapter.global_slot] = true;
    return kHookHandled;
}

HookResult RouteBootstrapTagCharacterPacket(CParty* party,
                                            PacketGuard& packet) {
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    MutexGuard guard(&room->mutex);
    if (!ValidateAdapterLocked(*room, adapter, party,
                               &party->battle_field(), 0) ||
        room->phase == ROOM_FAILED || room->phase == ROOM_TERMINATING) {
        return kHookRejected;
    }
    if (room->phase != ROOM_BOOTSTRAP) return kHookNotManaged;
    if (room->bootstrap_packets_sealed ||
        room->bootstrap_packets_published ||
        room->tag_character_ready[adapter.global_slot] ||
        !packet.is_finallized()) {
        return kHookRejected;
    }
    // The singleton packet itself is intentionally not retained: publication
    // synthesizes one official central roster packet containing all six users.
    room->tag_character_ready[adapter.global_slot] = true;
    return kHookHandled;
}

bool SealBootstrapPackets(PvP_Room* target_room,
                          uint32_t generation) {
    boost::shared_ptr<RoomState> room = FindRoom(target_room, generation);
    if (!room) return false;

    std::vector<CUser*> recipients;
    MutexGuard guard(&room->mutex);
    if (room->phase != ROOM_BOOTSTRAP ||
        room->bootstrap_packets_sealed ||
        room->bootstrap_packets_published ||
        !room->dungeon_info_sent || !room->pending_dungeon_info ||
        room->expected_mask == 0u) {
        return false;
    }
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
        if ((room->expected_mask & SeatBit(slot)) == 0u) continue;
        if (!room->fatigue_state_ready[slot] ||
            room->pending_fatigue_packets[slot].empty() ||
            !room->pending_start_maps[slot] ||
            !room->tag_character_ready[slot]) return false;
    }
    SnapshotRecipientsLocked(*room, true, recipients);
    if (recipients.size() != CountSeats(room->expected_mask)) return false;
    room->bootstrap_packets_sealed = true;
    return true;
}

bool PublishBootstrapPackets(PvP_Room* target_room,
                             uint32_t generation) {
    boost::shared_ptr<RoomState> room = FindRoom(target_room, generation);
    if (!room) return false;

    std::vector<CUser*> recipients;
    CUser* owner_recipients[kMaximumSeats] = {0};
    boost::shared_ptr<PacketBuf> dungeon_info;
    std::vector<boost::shared_ptr<PacketBuf> >
        fatigue_packets[kMaximumSeats];
    boost::shared_ptr<PacketBuf> start_maps[kMaximumSeats];
    boost::shared_ptr<PacketBuf> secret_shop[kMaximumSeats];
    uint8_t expected_mask = 0u;
    unsigned int noti29_focus_ff = 0u;
    uint8_t noti29_focus_non_ff_mask = 0u;
    uint8_t noti29_focus_decode_fail_mask = 0u;
    bool focus_validation_passed = false;
    {
        MutexGuard guard(&room->mutex);
        if (room->phase != ROOM_BOOTSTRAP ||
            !room->bootstrap_packets_sealed ||
            room->bootstrap_packets_published ||
            !room->dungeon_info_sent || !room->pending_dungeon_info) {
            return false;
        }
        expected_mask = room->expected_mask;
        for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
            if ((expected_mask & SeatBit(slot)) == 0u) continue;
            if (!room->fatigue_state_ready[slot] ||
                room->pending_fatigue_packets[slot].empty() ||
                !room->pending_start_maps[slot] ||
                !room->tag_character_ready[slot]) return false;
            fatigue_packets[slot] = room->pending_fatigue_packets[slot];
            start_maps[slot] = room->pending_start_maps[slot];
            secret_shop[slot] = room->pending_secret_shop[slot];
            unsigned int wire_focus = 0u;
            if (!ReadFinalizedStartMapFocus(start_maps[slot].get(),
                                            wire_focus)) {
                noti29_focus_decode_fail_mask = static_cast<uint8_t>(
                    noti29_focus_decode_fail_mask | SeatBit(slot));
            } else if (wire_focus != 0xffu) {
                noti29_focus_non_ff_mask = static_cast<uint8_t>(
                    noti29_focus_non_ff_mask | SeatBit(slot));
            } else {
                ++noti29_focus_ff;
            }
            SeatState& seat = room->seats[slot];
            if (seat.dropped || !seat.user ||
                room->room->get_user_seat(static_cast<int>(slot)) !=
                    seat.user) {
                return false;
            }
            owner_recipients[slot] = seat.user;
        }
        SnapshotRecipientsLocked(*room, true, recipients);
        if (recipients.size() != CountSeats(expected_mask)) return false;
        dungeon_info = room->pending_dungeon_info;
        focus_validation_passed =
            noti29_focus_decode_fail_mask == 0u &&
            noti29_focus_non_ff_mask == 0u &&
            noti29_focus_ff == CountSeats(expected_mask);
        // A partial socket write is not retryable.  The authority has already
        // committed the six-seat CONNECTING epoch, so a missing recipient is
        // resolved by the later load barrier instead of replaying a partial
        // START_MAP transaction.  Local shared_ptr copies keep these packets
        // alive while the room-owned cache is released immediately.
        // Validate the actual serialized focus before consuming the retained
        // transaction.  On rejection the authority owns the whole-cohort
        // rollback; no cache is marked published and no socket has been
        // touched.
        if (focus_validation_passed) {
            room->bootstrap_packets_published = true;
            room->pending_dungeon_info.reset();
            for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
                room->pending_fatigue_packets[slot].clear();
                room->pending_start_maps[slot].reset();
                room->pending_secret_shop[slot].reset();
            }
        }
    }

    // Validate every retained owner frame before the first socket write.  A
    // focus regression must not leave a partially visible bootstrap which
    // the authority can only repair by timing out the clients.
    if (!focus_validation_passed) {
        cMyTrace trace(
            "zombie_map_runtime::PublishBootstrapPackets", __LINE__, 5);
        trace("[ZOMBIE_MAP][BOOTSTRAP_FOCUS_REJECT] room=%d generation=%u expected=%u focus_ff=%u focus_non_ff=0x%02x focus_decode_fail=0x%02x sent_before_reject=0",
              target_room ? target_room->get_index() : -1,
              static_cast<unsigned int>(generation),
              static_cast<unsigned int>(CountSeats(expected_mask)),
              noti29_focus_ff,
              static_cast<unsigned int>(noti29_focus_non_ff_mask),
              static_cast<unsigned int>(noti29_focus_decode_fail_mask));
        return false;
    }

    bool sent = true;
    PacketGuard dungeon_packet(dungeon_info.get());
    if (SendRecipients(recipients, dungeon_packet) != kHookHandled) {
        sent = false;
    }
    // Preserve the original per-stage order across the central cohort.  Each
    // personal fatigue/buff frame remains owner-only.
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
        if ((expected_mask & SeatBit(slot)) == 0u) continue;
        std::vector<CUser*> owner(1u, owner_recipients[slot]);
        for (std::vector<boost::shared_ptr<PacketBuf> >::const_iterator it =
                 fatigue_packets[slot].begin();
             it != fatigue_packets[slot].end(); ++it) {
            PacketGuard fatigue_packet(it->get());
            if (SendRecipients(owner, fatigue_packet) != kHookHandled)
                sent = false;
        }
    }
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
        if ((expected_mask & SeatBit(slot)) == 0u) continue;
        PacketBuf* const start_packet_buf = start_maps[slot].get();
        PacketGuard start_packet(start_packet_buf);
        // START_MAP is the personal map-owner context produced by one hidden
        // singleton CParty.  The central actor/seat/peer snapshots above own
        // the shared six-player roster; replaying every owner's START_MAP to
        // every socket makes each client consume five foreign map contexts
        // after its own and prevents the 0828 loader from emitting CMD295.
        // Keep the original one-owner/one-recipient delivery contract.
        std::vector<CUser*> start_owner(1u, owner_recipients[slot]);
        if (SendRecipients(start_owner, start_packet) != kHookHandled) {
            sent = false;
        }
    }
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
        if ((expected_mask & SeatBit(slot)) == 0u || !secret_shop[slot])
            continue;
        PacketGuard secret_packet(secret_shop[slot].get());
        std::vector<CUser*> owner(1u, owner_recipients[slot]);
        if (SendRecipients(owner, secret_packet) != kHookHandled)
            sent = false;
    }

    PacketGuard tag_packet;
    tag_packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                          ENUM_NOTIPACKET_TAG_CHARACTER_INFO);
    int count_index = tag_packet.get_index();
    tag_packet.put_short(0);
    int tag_count = 0;
    for (std::vector<CUser*>::const_iterator user = recipients.begin();
         user != recipients.end(); ++user) {
        if (*user && (*user)->MakeTagCharacInfo(tag_packet) == 1) ++tag_count;
    }
    tag_packet.put_short(count_index, tag_count);
    tag_packet.finalize(true);
    // count==0 is an official, successful payload when no member has tag
    // character data; the client still posts its one bootstrap event.
    if (SendRecipients(recipients, tag_packet) != kHookHandled) {
        sent = false;
    }
    cMyTrace trace("zombie_map_runtime::PublishBootstrapPackets", __LINE__, 5);
    trace("[ZOMBIE_MAP][BOOTSTRAP_PUBLISH] room=%d generation=%u recipients=%u noti28_deliveries=%u noti29_frames=%u noti29_deliveries=%u owner_unicast=1 noti29_focus_ff=%u noti29_focus_non_ff=0x%02x noti29_focus_decode_fail=0x%02x sent=%d",
          target_room ? target_room->get_index() : -1,
          static_cast<unsigned int>(generation),
          static_cast<unsigned int>(recipients.size()),
          static_cast<unsigned int>(recipients.size()),
          static_cast<unsigned int>(CountSeats(expected_mask)),
          static_cast<unsigned int>(CountSeats(expected_mask)),
          noti29_focus_ff,
          static_cast<unsigned int>(noti29_focus_non_ff_mask),
          static_cast<unsigned int>(noti29_focus_decode_fail_mask),
          sent ? 1 : 0);
    return sent;
}

HookResult RouteAdapterPacket(CParty* party, PacketGuard& packet) {
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    std::vector<CUser*> recipients;
    {
        MutexGuard guard(&room->mutex);
        if (!ValidateAdapterLocked(*room, adapter, party,
                                   &party->battle_field(), 0) ||
            room->phase == ROOM_FAILED || room->phase == ROOM_TERMINATING) {
            return kHookRejected;
        }
        // Initial Phase3 completion in ROOM_ARMED remains per-adapter so
        // every client receives exactly one NOTI30.  Room-wide map/event
        // broadcast starts only after the Zombie round becomes ACTIVE.
        if (room->phase != ROOM_ACTIVE) return kHookNotManaged;
        SnapshotRecipientsLocked(*room, false, recipients);
    }
    return SendRecipients(recipients, packet);
}

HookResult ConsumeMoveMap(CUser* user,
                          MSG_MOVE_MAP& message,
                          int* move_result) {
    if (move_result) *move_result = -1;
    CParty* const party = user ? user->GetParty() : 0;
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    const int target_x = static_cast<signed char>(message.map_x);
    const int target_y = static_cast<signed char>(message.map_y);
    {
        MutexGuard guard(&room->mutex);
        if (room->phase != ROOM_ACTIVE ||
            !ValidateAdapterLocked(*room, adapter, party,
                                   &party->battle_field(), user) ||
            (room->active_mask & SeatBit(adapter.global_slot)) == 0u ||
            room->seats[adapter.global_slot].map_load_pending ||
            room->move_in_progress || ExpectedMapIndex(target_x, target_y) == 0) {
            return kHookRejected;
        }
        room->move_in_progress = true;
        room->move_slot = adapter.global_slot;
        room->move_target_x = target_x;
        room->move_target_y = target_y;
    }

    message.party_seat = 0u;
    const int result = party->move_map(message);
    if (move_result) *move_result = result;
    {
        MutexGuard guard(&room->mutex);
        if (room->move_in_progress && room->move_slot == adapter.global_slot) {
            room->move_in_progress = false;
            room->move_slot = 0xffu;
            room->move_target_x = -1;
            room->move_target_y = -1;
        }
    }
    return result == 1 ? kHookHandled : kHookRejected;
}

HookResult CommitMoveMap(CParty* party, int pos_x, int pos_y) {
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    MutexGuard guard(&room->mutex);
    if (room->phase != ROOM_ACTIVE || !room->move_in_progress ||
        room->move_slot != adapter.global_slot ||
        room->move_target_x != pos_x || room->move_target_y != pos_y ||
        !ValidateAdapterLocked(*room, adapter, party,
                               &party->battle_field(), 0)) {
        return kHookRejected;
    }
    const std::map<CellKey, CellState>::iterator found =
        room->cells.find(CellKey(pos_x, pos_y));
    if (found == room->cells.end() || !found->second.info ||
        found->second.info->map_index_ != ExpectedMapIndex(pos_x, pos_y)) {
        return kHookRejected;
    }
    found->second.initialized = true;
    SeatState& seat = room->seats[adapter.global_slot];
    seat.last_x = seat.x;
    seat.last_y = seat.y;
    seat.x = pos_x;
    seat.y = pos_y;
    seat.positioned = true;
    (void)AdvanceNonZero(seat.map_load_epoch);
    seat.map_load_pending = true;
    room->move_in_progress = false;
    room->move_slot = 0xffu;
    room->move_target_x = -1;
    room->move_target_y = -1;
    cMyTrace trace("zombie_map_runtime::CommitMoveMap", __LINE__, 5);
    trace("[ZOMBIE_MAP][MOVE] room=%d generation=%u slot=%u cell=(%d,%d) map=%d map_epoch=%u",
          room->room_index,
          static_cast<unsigned int>(room->generation),
          static_cast<unsigned int>(adapter.global_slot),
          pos_x,
          pos_y,
          found->second.info->map_index_,
          static_cast<unsigned int>(seat.map_load_epoch));
    return kHookHandled;
}

HookResult ConsumeSeatFinishLoading(CParty* party,
                                    CUser* user,
                                    int32_t inventory_checksum,
                                    int32_t skill_checksum,
                                    uint32_t* completed_map_epoch) {
    if (completed_map_epoch) *completed_map_epoch = 0u;
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    uint32_t epoch = 0u;
    bool invalid_owner = false;
    {
        MutexGuard guard(&room->mutex);
        if (room->phase != ROOM_ACTIVE) return kHookNotManaged;
        if (!ValidateAdapterLocked(*room, adapter, party,
                                   &party->battle_field(), user) ||
            (room->active_mask & SeatBit(adapter.global_slot)) == 0u) {
            invalid_owner = true;
        } else {
            SeatState& seat = room->seats[adapter.global_slot];
            if (!seat.map_load_pending) return kHookHandled;
            epoch = seat.map_load_epoch;
        }
    }
    if (invalid_owner) {
        MarkRoomFailed(room, "map_finish_owner_invalid");
        return kHookRejected;
    }

    if (user->getCurCharacInvenCheckSum() != inventory_checksum) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x25b),
            1u, 0u, 0u);
    }
    if (user->getCurCharacSkillCheckSum() != skill_checksum) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x25c),
            1u, 0u, 0u);
    }
    if (party->battle_data().member_finish_loading(0) != 0u) {
        MarkRoomFailed(room, "map_finish_already_set");
        return kHookRejected;
    }
    if (party->battle_data().member_start_map_finish_loading(0) != 1u) {
        if (!party->OnStartMapFinishLoading(user)) {
            MarkRoomFailed(room, "map_on_start_failed");
            return kHookRejected;
        }
        party->battle_data().member_start_map_finish_loading(0) = 1u;
    }
    (void)party->finish_loading(user);
    if (party->battle_data().member_finish_loading(0) != 1u) {
        MarkRoomFailed(room, "map_finish_failed");
        return kHookRejected;
    }

    bool commit_invalid = false;
    {
        MutexGuard guard(&room->mutex);
        SeatState& seat = room->seats[adapter.global_slot];
        if (room->phase != ROOM_ACTIVE || !seat.map_load_pending ||
            seat.map_load_epoch != epoch) {
            commit_invalid = true;
        } else {
            seat.map_load_pending = false;
        }
    }
    if (commit_invalid) {
        MarkRoomFailed(room, "map_finish_epoch_changed");
        return kHookRejected;
    }
    if (completed_map_epoch) *completed_map_epoch = epoch;
    return kHookHandled;
}

bool ConsumeSeatLoadTimeout(CParty* party) {
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return false;
    bool fail = false;
    {
        MutexGuard guard(&room->mutex);
        if (room->phase != ROOM_ACTIVE) return false;
        if (!ValidateAdapterLocked(*room, adapter, party,
                                   &party->battle_field(), 0)) {
            fail = true;
        } else if (!room->seats[adapter.global_slot].map_load_pending) {
            return true;
        } else {
            fail = true;
        }
    }
    if (fail) MarkRoomFailed(room, "map_load_timeout");
    return true;
}

bool StartRoomItemBox(PvP_Room* target_room,
                      uint32_t generation,
                      const ItemBoxConfig& config) {
    boost::shared_ptr<RoomState> room = FindRoom(target_room, generation);
    if (!room || config.passive_index == 0u ||
        config.repeat_delay_ms == 0u) {
        return false;
    }
    uint32_t timer_key = 0u;
    {
        MutexGuard guard(&g_registry_mutex);
        timer_key = NextNonZero(g_next_item_box_timer_key);
    }
    {
        MutexGuard guard(&room->mutex);
        if (room->phase != ROOM_ACTIVE || room->item_box_started)
            return false;
        room->item_box_config = config;
        room->item_box_timer_key = timer_key;
        room->item_box_started = true;
        room->next_provenance_token = room->generation ^ 0x70a00001u;
        if (room->next_provenance_token == 0u)
            room->next_provenance_token = 1u;
    }
    const bool scheduled = G_TimerQueue()->InsertTimerInMilisecond(
                               1,
                               room->room_index,
                               48,
                               static_cast<int64_t>(config.first_delay_ms),
                               timer_key,
                               generation) != 0;
    if (!scheduled) {
        MutexGuard guard(&room->mutex);
        if (room->item_box_timer_key == timer_key)
            room->item_box_started = false;
    }
    return scheduled;
}

HookResult OnRoomItemBoxTimer(int room_index,
                              uint32_t timer_key,
                              uint32_t generation) {
    boost::shared_ptr<RoomState> room;
    {
        MutexGuard guard(&g_registry_mutex);
        std::map<int, boost::weak_ptr<RoomState> >::const_iterator found =
            g_room_indices.find(room_index);
        if (found == g_room_indices.end()) return kHookNotManaged;
        room = found->second.lock();
    }
    if (!room || room->generation != generation) return kHookRejected;
    if (!zombie_mode::IsActiveRoom(room->room)) {
        MutexGuard guard(&room->mutex);
        room->item_box_started = false;
        return kHookHandled;
    }

    std::vector<std::pair<uint32_t, CellKey> > generated;
    std::vector<CUser*> recipients;
    ItemBoxConfig config;
    uint32_t repeat_delay = 0u;
    {
        MutexGuard guard(&room->mutex);
        if (room->phase != ROOM_ACTIVE || !room->item_box_started ||
            room->item_box_timer_key != timer_key) {
            return kHookRejected;
        }
        std::set<CellKey> occupied;
        for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
            if ((room->active_mask & SeatBit(slot)) == 0u) continue;
            const SeatState& seat = room->seats[slot];
            if (!seat.dropped && seat.positioned)
                occupied.insert(CellKey(seat.x, seat.y));
        }
        for (std::set<CellKey>::const_iterator cell = occupied.begin();
             cell != occupied.end(); ++cell) {
            bool already_spawned = false;
            for (std::map<uint32_t, BoxSpawn>::const_iterator existing =
                     room->box_spawns.begin();
                 existing != room->box_spawns.end(); ++existing) {
                if (!existing->second.claimed &&
                    existing->second.cell == *cell) {
                    already_spawned = true;
                    break;
                }
            }
            if (already_spawned) continue;
            std::map<CellKey, CellState>::iterator canonical =
                room->cells.find(*cell);
            if (canonical == room->cells.end() ||
                !canonical->second.initialized ||
                !canonical->second.info) {
                continue;
            }
            uint32_t uid = 0u;
            do {
                uid = static_cast<uint32_t>(
                    canonical->second.info->next_drop_item_unique_id_++);
            } while (uid == 0u ||
                     room->box_spawns.find(uid) != room->box_spawns.end());
            room->box_spawns[uid] = BoxSpawn(*cell);
            generated.push_back(std::make_pair(uid, *cell));
        }
        config = room->item_box_config;
        repeat_delay = config.repeat_delay_ms;
        SnapshotRecipientsLocked(*room, false, recipients);
    }

    for (std::vector<std::pair<uint32_t, CellKey> >::const_iterator it =
             generated.begin(); it != generated.end(); ++it) {
        PacketGuard packet;
        BuildItemBoxGeneratedPacket(packet, it->first, config, it->second);
        if (SendRecipients(recipients, packet) != kHookHandled) {
            MarkRoomFailed(room, "itembox_broadcast_failed");
            return kHookRejected;
        }
    }
    if (repeat_delay != 0u) {
        if (!G_TimerQueue()->InsertTimerInMilisecond(
                1,
                room->room_index,
                48,
                static_cast<int64_t>(repeat_delay),
                timer_key,
                generation)) {
            MarkRoomFailed(room, "itembox_timer_requeue_failed");
            return kHookRejected;
        }
    }
    return kHookHandled;
}

HookResult ClaimRoomItemBox(CUser* user,
                            uint32_t runtime_uid,
                            int* error,
                            uint32_t* provenance_token) {
    if (error) *error = 19;
    if (provenance_token) *provenance_token = 0u;
    CParty* const party = user ? user->GetParty() : 0;
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    if (!zombie_mode::IsActiveRoom(room->room)) return kHookRejected;
    std::vector<CUser*> recipients;
    uint32_t token = 0u;
    {
        MutexGuard guard(&room->mutex);
        if (room->phase != ROOM_ACTIVE || !room->item_box_started ||
            !ValidateAdapterLocked(*room, adapter, party,
                                   &party->battle_field(), user) ||
            (room->active_mask & SeatBit(adapter.global_slot)) == 0u) {
            return kHookRejected;
        }
        SeatState& seat = room->seats[adapter.global_slot];
        std::map<uint32_t, BoxSpawn>::iterator spawned =
            room->box_spawns.find(runtime_uid);
        if (spawned == room->box_spawns.end() || spawned->second.claimed ||
            !seat.positioned ||
            !(spawned->second.cell == CellKey(seat.x, seat.y))) {
            return kHookRejected;
        }
        spawned->second.claimed = true;
        room->box_spawns.erase(spawned);
        token = NextNonZero(room->next_provenance_token);
        seat.provenance_tokens.push_back(token);
        SnapshotRecipientsLocked(*room, false, recipients);
    }
    PacketGuard packet;
    BuildItemBoxClaimedPacket(packet, runtime_uid);
    if (SendRecipients(recipients, packet) != kHookHandled) {
        MarkRoomFailed(room, "itembox_claim_broadcast_failed");
        return kHookRejected;
    }
    if (error) *error = 0;
    if (provenance_token) *provenance_token = token;
    return kHookHandled;
}

bool ConsumeItemBoxProvenance(CUser* user, uint32_t* provenance_token) {
    if (provenance_token) *provenance_token = 0u;
    CParty* const party = user ? user->GetParty() : 0;
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return false;
    MutexGuard guard(&room->mutex);
    if (room->phase != ROOM_ACTIVE ||
        !ValidateAdapterLocked(*room, adapter, party,
                               &party->battle_field(), user)) {
        return false;
    }
    SeatState& seat = room->seats[adapter.global_slot];
    if (seat.provenance_tokens.empty())
        return false;
    if (provenance_token) *provenance_token =
        seat.provenance_tokens.front();
    seat.provenance_tokens.pop_front();
    return true;
}

HookResult AuthorizeCellMonsterHost(CUser* user,
                                    CParty* party,
                                    uint16_t monster_unique_id) {
    boost::shared_ptr<RoomState> room;
    AdapterRef adapter;
    if (!FindPartyAdapter(party, room, adapter)) return kHookNotManaged;
    MutexGuard guard(&room->mutex);
    if (room->phase != ROOM_ACTIVE ||
        !ValidateAdapterLocked(*room, adapter, party,
                               &party->battle_field(), user) ||
        (room->active_mask & SeatBit(adapter.global_slot)) == 0u) {
        return kHookRejected;
    }
    const SeatState& actor = room->seats[adapter.global_slot];
    if (!actor.positioned) return kHookRejected;
    uint8_t host = 0xffu;
    for (size_t slot = 0u; slot < kMaximumSeats; ++slot) {
        if ((room->active_mask & SeatBit(slot)) == 0u) continue;
        const SeatState& candidate = room->seats[slot];
        if (!candidate.dropped && candidate.positioned &&
            candidate.x == actor.x && candidate.y == actor.y) {
            host = static_cast<uint8_t>(slot);
            break;
        }
    }
    if (host != adapter.global_slot) return kHookRejected;
    const std::map<CellKey, CellState>::const_iterator cell =
        room->cells.find(CellKey(actor.x, actor.y));
    if (cell == room->cells.end() || !cell->second.info ||
        cell->second.info->monsters_.find(monster_unique_id) ==
            cell->second.info->monsters_.end()) {
        return kHookRejected;
    }
    return kHookHandled;
}

}  // namespace zombie_map_runtime
