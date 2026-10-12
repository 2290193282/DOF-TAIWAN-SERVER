#include "old_game/dungeon/instance/monster_move_system.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"

#include <map>
#include <stdint.h>

#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_game/player/core/cuser.h"
#include "old_game/dungeon/escort/golden_commerce.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"

namespace monster_move_system {
namespace {

struct MovementRequest {
    MovementRequest()
        : entity_unique_id(0),
          grid_x(0),
          grid_y(0),
          value_0(0),
          value_1(0),
          value_2(0),
          action(0) {}

    uint16_t entity_unique_id;
    uint8_t grid_x;
    uint8_t grid_y;
    uint32_t value_0;
    uint32_t value_1;
    uint32_t value_2;
    uint8_t action;
};

struct EntityState {
    EntityState()
        : grid_x(0),
          grid_y(0),
          value_0(0),
          value_1(0),
          value_2(0),
          action(0),
          initialized(false) {}

    uint8_t grid_x;
    uint8_t grid_y;
    uint32_t value_0;
    uint32_t value_1;
    uint32_t value_2;
    uint8_t action;
    bool initialized;
};

struct PartyState {
    PartyState()
        : party(0),
          map_info(0),
          party_index(-1),
          dungeon_index(0),
          map_index(0),
          manager_unique_id(0),
          entities() {}

    CParty* party;
    MapInfo* map_info;
    int party_index;
    int dungeon_index;
    int map_index;
    int manager_unique_id;
    std::map<uint16_t, EntityState> entities;
};

Mutex& StateMutex() {
    static Mutex mutex;
    return mutex;
}

std::map<CParty*, PartyState>& States() {
    static std::map<CParty*, PartyState> states;
    return states;
}

bool ReadExact(PacketBuf& packet, MovementRequest& request) {
    return packet.get_short(request.entity_unique_id) == 1 &&
           packet.get_byte(request.grid_x) == 1 &&
           packet.get_byte(request.grid_y) == 1 &&
           packet.get_int(request.value_0) == 1 &&
           packet.get_int(request.value_1) == 1 &&
           packet.get_int(request.value_2) == 1 &&
           packet.get_byte(request.action) == 1;
}

bool IsMovementAction(uint8_t action) {
    return action <= 3u;
}

bool IsExpectedStep(const EntityState& previous,
                    const MovementRequest& request) {
    int expected_x = previous.grid_x;
    int expected_y = previous.grid_y;
    switch (request.action) {
        case 0:
            ++expected_x;
            break;
        case 1:
            --expected_x;
            break;
        case 2:
            ++expected_y;
            break;
        case 3:
            --expected_y;
            break;
        default:
            return false;
    }
    return expected_x >= 0 && expected_x <= 0xff &&
           expected_y >= 0 && expected_y <= 0xff &&
           request.grid_x == static_cast<uint8_t>(expected_x) &&
           request.grid_y == static_cast<uint8_t>(expected_y);
}

bool IsReplay(const EntityState& previous,
              const MovementRequest& request) {
    return previous.initialized && previous.grid_x == request.grid_x &&
           previous.grid_y == request.grid_y &&
           previous.value_0 == request.value_0 &&
           previous.value_1 == request.value_1 &&
           previous.value_2 == request.value_2 &&
           previous.action == request.action;
}

void CopyRequest(const MovementRequest& request, EntityState& state) {
    state.grid_x = request.grid_x;
    state.grid_y = request.grid_y;
    state.value_0 = request.value_0;
    state.value_1 = request.value_1;
    state.value_2 = request.value_2;
    state.action = request.action;
    state.initialized = true;
}

bool BuildContext(CUser* user,
                  const MovementRequest& request,
                  CParty*& party,
                  MapInfo*& map_info,
                  int& dungeon_index,
                  int& map_index) {
    party = 0;
    map_info = 0;
    dungeon_index = 0;
    map_index = 0;
    if (!user || user->get_state() != 5 || user->is_fighting() == 0) {
        return false;
    }

    party = user->GetParty();
    if (!party) {
        return false;
    }
    const zombie_map_runtime::HookResult zombie_host =
        zombie_map_runtime::AuthorizeCellMonsterHost(
            user, party, request.entity_unique_id);
    if (zombie_host == zombie_map_runtime::kHookRejected ||
        (zombie_host == zombie_map_runtime::kHookNotManaged &&
         party->getManager() != user)) {
        return false;
    }
    const int seat = party->get_party_seatno(user);
    if (seat < 0 || seat > 3 || party->_checkValidUser(seat) != 1 ||
        party->get_user(seat) != user) {
        return false;
    }

    CBattle_Field& field = party->battle_field();
    map_info = field.GetCurrentMapInfo();
    dungeon_index = field.get_dungeon_index();
    map_index = field.GetCurrentMapIndex();
    if (!field.get_dungeon() || !map_info || dungeon_index <= 0 ||
        map_index <= 0 ||
        map_info->monsters_.find(request.entity_unique_id) ==
            map_info->monsters_.end()) {
        return false;
    }
    return true;
}

bool AcceptMovement(CUser* user,
                    const MovementRequest& request,
                    CParty*& party) {
    MapInfo* map_info = 0;
    int dungeon_index = 0;
    int map_index = 0;
    if (!BuildContext(user,
                      request,
                      party,
                      map_info,
                      dungeon_index,
                      map_index)) {
        return false;
    }

    // Golden-Commerce owns its session under a pthread mutex and calls the
    // bootstrap seeder while holding that mutex.  Authorize before taking the
    // movement-state mutex so all paths use Golden -> movement lock ordering.
    if (!golden_commerce::AuthorizeCartMovement(
            party, request.entity_unique_id)) {
        return false;
    }

    {
        MutexGuard guard(&StateMutex());
        PartyState& party_state = States()[party];
        const bool same_run =
            party_state.party == party && party_state.map_info == map_info &&
            party_state.party_index == party->GetPartyIndex() &&
            party_state.dungeon_index == dungeon_index &&
            party_state.map_index == map_index &&
            party_state.manager_unique_id == user->get_unique_id();
        if (!same_run) {
            party_state = PartyState();
            party_state.party = party;
            party_state.map_info = map_info;
            party_state.party_index = party->GetPartyIndex();
            party_state.dungeon_index = dungeon_index;
            party_state.map_index = map_index;
            party_state.manager_unique_id = user->get_unique_id();
        }

        EntityState& entity = party_state.entities[request.entity_unique_id];
        if (IsReplay(entity, request)) {
            return false;
        }

        if (request.action == 5u) {
            // The native action-5 sender copies the object's current grid
            // before transmitting three script values.  It may be repeated,
            // but it may not teleport an already initialized object.
            if (entity.initialized &&
                (entity.grid_x != request.grid_x ||
                 entity.grid_y != request.grid_y)) {
                return false;
            }
        } else {
            // The directional sender always emits zero for all three script
            // values.  Its action mapping is 0:+x, 1:-x, 2:+y, 3:-y.
            if (!IsMovementAction(request.action) || !entity.initialized ||
                request.value_0 != 0u || request.value_1 != 0u ||
                request.value_2 != 0u || !IsExpectedStep(entity, request)) {
                return false;
            }
        }

        CopyRequest(request, entity);
    }
    golden_commerce::OnCartMovementCommitted(
        party, request.entity_unique_id, request.grid_x, request.grid_y,
        request.action);
    return true;
}

map_monster* FindCurrentEntity(CParty* party, uint16_t entity_unique_id) {
    if (!party || entity_unique_id == 0u) {
        return 0;
    }
    MapInfo* const map_info = party->battle_field().GetCurrentMapInfo();
    if (!map_info) {
        return 0;
    }
    map_monster* found = 0;
    for (std::map<int, map_monster>::iterator it = map_info->monsters_.begin();
         it != map_info->monsters_.end(); ++it) {
        if (it->second.map_monster_unique_id_ != entity_unique_id) {
            continue;
        }
        if (found) {
            return 0;
        }
        found = &it->second;
    }
    return found;
}

void PutMovementNotification(PacketGuard& packet,
                             uint16_t entity_unique_id,
                             uint32_t monster_id,
                             uint16_t monster_level,
                             uint32_t position_x,
                             uint32_t position_y,
                             uint32_t position_z,
                             uint8_t team,
                             uint8_t action) {
    packet.put_byte(1);
    packet.put_short(entity_unique_id);
    packet.put_int(monster_id);
    packet.put_short(monster_level);
    packet.put_int(position_x);
    packet.put_int(position_y);
    packet.put_int(position_z);
    packet.put_byte(team);
    packet.put_byte(action);
}

bool SendToCurrentPartyMembers(CParty* party, PacketGuard& packet) {
    if (!party) return false;
    const zombie_map_runtime::HookResult zombie_route =
        zombie_map_runtime::RouteAdapterPacket(party, packet);
    if (zombie_route != zombie_map_runtime::kHookNotManaged)
        return zombie_route == zombie_map_runtime::kHookHandled;
    int attempted = 0;
    for (int seat = 0; seat < 4; ++seat) {
        CUser* const member = party->get_user(seat);
        if (!member || member->GetParty() != party) continue;
        // CParty::send_to_party calls _checkValidUser(), whose legacy mismatch
        // branch actively disconnects the user.  Map-load notifications must
        // be best-effort and must not turn a transient seat/UID race into
        // DisConnSig(23).
        member->Send(packet);
        ++attempted;
    }
    return attempted > 0;
}

void BroadcastMovement(CParty* party, const MovementRequest& request) {
    if (!party) {
        return;
    }
    map_monster* const entity =
        FindCurrentEntity(party, request.entity_unique_id);
    if (!entity || entity->monster_id_ <= 0 || entity->monster_level_ == 0u) {
        return;
    }

    // CLIENT_IDA_EVIDENCE: record+0x20 is the object team, not HP.  Keep the
    // map-move cart allied with the players on every echo; feeding the reward
    // percentage here made enemy team-100 monsters treat the cart as friendly.
    const uint8_t cart_team = 0u;

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_MONSTER_MOVE_SYSTEM);
    PutMovementNotification(packet,
                            request.entity_unique_id,
                            static_cast<uint32_t>(entity->monster_id_),
                            entity->monster_level_,
                            request.value_0,
                            request.value_1,
                            request.value_2,
                            cart_team,
                            request.action);
    packet.finalize(true);
    (void)SendToCurrentPartyMembers(party, packet);
}

}  // namespace

void ResetParty(CParty* party) {
    if (!party) {
        return;
    }
    MutexGuard guard(&StateMutex());
    States().erase(party);
}

bool SendBootstrap(CParty* party,
                   uint16_t entity_unique_id,
                   uint32_t monster_id,
                   uint16_t monster_level,
                   uint32_t position_x,
                   uint32_t position_y,
                   uint32_t position_z,
                   uint8_t team) {
    if (!party || entity_unique_id == 0u || monster_id == 0u ||
        monster_level == 0u) {
        return false;
    }
    CUser* const manager = party->getManager();
    CBattle_Field& field = party->battle_field();
    MapInfo* const map_info = field.GetCurrentMapInfo();
    if (!manager || !map_info || !field.get_dungeon()) return false;
    std::map<int, map_monster>::iterator entity =
        map_info->monsters_.find(static_cast<int>(entity_unique_id));
    if (entity == map_info->monsters_.end() ||
        entity->second.map_monster_unique_id_ != entity_unique_id ||
        entity->second.monster_id_ != static_cast<int32_t>(monster_id)) {
        return false;
    }
    int grid_x = -1;
    int grid_y = -1;
    field.getCurPosXY(grid_x, grid_y);
    if (grid_x < 0 || grid_x > 0xff || grid_y < 0 || grid_y > 0xff) {
        return false;
    }

    // NOTI476 action 5 is the authoritative origin for the following CMD541
    // directional steps.  The old sender emitted the packet but never seeded
    // this state, so every first action 0..3 failed the initialized/adjacency
    // gate even though the client had moved the cart successfully.
    {
        MutexGuard guard(&StateMutex());
        PartyState& party_state = States()[party];
        party_state = PartyState();
        party_state.party = party;
        party_state.map_info = map_info;
        party_state.party_index = party->GetPartyIndex();
        party_state.dungeon_index = field.get_dungeon_index();
        party_state.map_index = field.GetCurrentMapIndex();
        party_state.manager_unique_id = manager->get_unique_id();
        EntityState& bootstrap = party_state.entities[entity_unique_id];
        bootstrap.grid_x = static_cast<uint8_t>(grid_x);
        bootstrap.grid_y = static_cast<uint8_t>(grid_y);
        bootstrap.value_0 = position_x;
        bootstrap.value_1 = position_y;
        bootstrap.value_2 = position_z;
        bootstrap.action = 5u;
        bootstrap.initialized = true;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_MONSTER_MOVE_SYSTEM);
    PutMovementNotification(packet, entity_unique_id, monster_id, monster_level,
                            position_x, position_y, position_z, team, 5u);
    packet.finalize(true);
    if (!SendToCurrentPartyMembers(party, packet)) {
        ResetParty(party);
        return false;
    }
    cMyTrace trace("MonsterMoveSystem0828", __LINE__, 5);
    trace("[0828 MonsterMove] bootstrap_seeded party_index(%d) "
          "dungeon(%d) map(%d) uid(%u) grid(%d,%d)",
          party->GetPartyIndex(), field.get_dungeon_index(),
          field.GetCurrentMapIndex(),
          static_cast<unsigned int>(entity_unique_id), grid_x, grid_y);
    return true;
}

int Dispatch(CUser* user, PacketBuf& packet) {
    MovementRequest request;
    if (!ReadExact(packet, request)) {
        return 1;
    }
    CParty* party = 0;
    const bool accepted = AcceptMovement(user, request, party);
    cMyTrace trace("MonsterMoveSystem0828", __LINE__, 5);
    trace("[0828 MonsterMove] accepted(%d) account(%d) uid(%u) "
          "grid(%u,%u) values(%u,%u,%u) action(%u)",
          accepted ? 1 : 0,
          user ? user->get_acc_id() : 0,
          static_cast<unsigned int>(request.entity_unique_id),
          static_cast<unsigned int>(request.grid_x),
          static_cast<unsigned int>(request.grid_y),
          static_cast<unsigned int>(request.value_0),
          static_cast<unsigned int>(request.value_1),
          static_cast<unsigned int>(request.value_2),
          static_cast<unsigned int>(request.action));
    if (accepted) {
        BroadcastMovement(party, request);
    }
    // Native CMD541 has no same-command response handler.  Structural input
    // errors fail dispatch; well-formed authorization failures stay silent.
    return 0;
}

}  // namespace monster_move_system

Dispatcher_MonsterMoveSystem::Dispatcher_MonsterMoveSystem() {}

Dispatcher_MonsterMoveSystem::~Dispatcher_MonsterMoveSystem() {}

int Dispatcher_MonsterMoveSystem::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    return monster_move_system::Dispatch(user, packet);
}

int Dispatcher_MonsterMoveSystem::read(PacketBuf& packet,
                                       MSG_BASE& message) {
    static_cast<void>(packet);
    static_cast<void>(message);
    return 0;
}

int Dispatcher_MonsterMoveSystem::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    static_cast<void>(user);
    static_cast<void>(message);
    static_cast<void>(param);
    return 0;
}
