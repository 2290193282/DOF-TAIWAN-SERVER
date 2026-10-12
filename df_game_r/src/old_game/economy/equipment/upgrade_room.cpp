#include "old_game/economy/equipment/upgrade_room.h"

#include <algorithm>
#include <limits.h>
#include <stddef.h>
#include <string.h>

#include <map>
#include <set>
#include <utility>
#include <vector>

#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_data/PVF/event/upgrade_room.h"
#include "old_data/PVF/town/town_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

namespace {

const uint32_t kGoldKey = 0u;
const uint32_t kClearCubeKey = 3037u;
const uint32_t kNoItem = 0xffffffffu;
const uint8_t kRoomIdle = 0u;
const uint8_t kRoomUpgrading = 1u;
const uint8_t kRoomResult = 2u;
const uint8_t kResultSuccess = 0u;
const uint8_t kResultDowngrade = 1u;
const uint8_t kResultBroken = 2u;
const uint8_t kResultUnchanged = 3u;
const int kMainInventory = 1;
const size_t kMaximumMasterMessageUnits = 259u;

// Upgrade Room response errors are tied to the target client's recovered
// text branches.  They are deliberately kept local to this owner rather than
// manufacturing a global enum whose unrelated commands reuse the numbers.
const int kErrorNotInRoom = 3;
const int kErrorInvalidState = 4;
const int kErrorNotOwner = 7;
const int kErrorEventUnavailable = 8;
const int kErrorOtherRoom = 18;
const int kErrorGeneral = 19;
const int kErrorInvalidRoomState = 21;
const int kErrorInsufficientItem = 22;
const int kErrorInsufficientMoney = 70;
const int kErrorNearNpc = 82;
const int kErrorDonationLimit = 95;
const int kErrorInvalidLocation = 190;
// Ordinary CItemUpgrade returns 216 after it has atomically consumed an
// automatic protection ticket and replaced destruction with upgrade +0.
// This is an outcome, not a CMD583 request failure.
const int kOrdinaryUpgradeProtectedDestruction = 216;

struct Room {
    Room()
        : room_id(0u), owner_uid(0u), owner(0), village(0u), area(0u),
          x(0u), y(0u), spectators(), master_message(), escrow(),
          staged(false), target_slot(0xffffu), wire_item_id(kNoItem),
          target_level(0u), guid_server(0), guid_datetime(0),
          guid_sequence(0u), display_state(kRoomIdle),
          result_state(kResultUnchanged), in_upgrade(false) {
        escrow.insert(std::make_pair(kGoldKey, 0u));
        escrow.insert(std::make_pair(kClearCubeKey, 0u));
    }

    uint16_t room_id;
    uint16_t owner_uid;
    CUser* owner;
    uint8_t village;
    uint8_t area;
    uint16_t x;
    uint16_t y;
    std::map<uint16_t, CUser*> spectators;
    std::vector<uint16_t> master_message;
    std::map<uint32_t, uint32_t> escrow;
    bool staged;
    uint16_t target_slot;
    uint32_t wire_item_id;
    uint8_t target_level;
    int32_t guid_server;
    int32_t guid_datetime;
    uint16_t guid_sequence;
    uint8_t display_state;
    uint8_t result_state;
    bool in_upgrade;
};

typedef std::map<uint16_t, Room> RoomMap;
typedef std::map<uint16_t, uint16_t> MembershipMap;

RoomMap& Rooms() {
    static RoomMap rooms;
    return rooms;
}

MembershipMap& OwnerRooms() {
    static MembershipMap rooms;
    return rooms;
}

MembershipMap& SpectatorRooms() {
    static MembershipMap rooms;
    return rooms;
}

uint16_t& NextRoomId() {
    static uint16_t next = 1u;
    return next;
}

Room*& ActiveRoom() {
    static Room* room = 0;
    return room;
}

CUser*& ActiveOwner() {
    static CUser* owner = 0;
    return owner;
}

void TraceFailure(const char* reason, CUser* user) {
    cMyTrace trace("UpgradeRoom0828", __LINE__, 5);
    trace("[0828 UpgradeRoom] %s account(%d) charac(%d) uid(%d)",
          reason ? reason : "unknown",
          user ? user->get_acc_id() : 0,
          user ? user->get_charac_no(-1) : 0,
          user ? user->get_unique_id() : 0);
}

bool GetWireUid(CUser* user, uint16_t& uid) {
    if (!user) return false;
    const int value = user->get_unique_id();
    if (value <= 0 || value > 0xffff) return false;
    uid = static_cast<uint16_t>(value);
    return true;
}

bool IsLive(CUser* user, uint16_t uid) {
    GameWorld* const world = G_GameWorld();
    return user && world && world->find_from_world(uid) == user;
}

bool IsTownSessionReady(CUser* user) {
    GameWorld* const world = G_GameWorld();
    private_store::CPrivateStoreMgr* const private_store_manager =
        private_store::GetInstancePrivateStoreMgr();
    return user && world && user->get_state() == 3 &&
           user->getCurCharacR() && user->getCurCharacInvenR() &&
           user->getMoveSpace() != 1 && !user->CheckInTrade() &&
           !user->IsBusyDisjointStore() &&
           (!private_store_manager ||
            !private_store_manager->IsBusyPrivateStore(user)) &&
           !world->IsPVPChannel() &&
           !world->IsIntegratedPvPBaseChannel();
}

bool IsSameScope(CUser* user, const Room& room) {
    if (!IsTownSessionReady(user)) return false;
    const int village = user->getCurCharacVill();
    const int area = user->get_area(false);
    return village == static_cast<int>(room.village) &&
           area == static_cast<int>(room.area);
}

bool ValidateTownUser(CUser* user,
                      int& village,
                      int& area,
                      Area*& area_object) {
    GameWorld* const world = G_GameWorld();
    if (!IsTownSessionReady(user) || !world) {
        return false;
    }

    village = user->getCurCharacVill();
    area = user->get_area(false);
    if (village < 0 || village > 0xff || area < 0 || area > 0xff ||
        !world->check_valid_area(village, area)) {
        return false;
    }
    Village* const village_object = world->getUserVillage(user);
    area_object = village_object ? village_object->getArea(area) : 0;
    if (!area_object || area_object->private_store_enabled != 1u ||
        area_object->town_area_type == TOWN_AREA_GATE ||
        area_object->town_area_type == TOWN_AREA_FISHING_PLACE ||
        area_object->town_area_type == TOWN_AREA_DUNGEON_GATE) {
        return false;
    }
    return true;
}

Room* FindRoom(uint16_t room_id) {
    RoomMap::iterator found = Rooms().find(room_id);
    return found == Rooms().end() ? 0 : &found->second;
}

Room* FindOwnedRoom(CUser* user) {
    uint16_t uid = 0u;
    if (!GetWireUid(user, uid)) return 0;
    MembershipMap::iterator member = OwnerRooms().find(uid);
    if (member == OwnerRooms().end()) return 0;
    Room* const room = FindRoom(member->second);
    if (!room || room->owner != user || room->owner_uid != uid) return 0;
    return room;
}

Room* FindSpectatorRoom(CUser* user) {
    uint16_t uid = 0u;
    if (!GetWireUid(user, uid)) return 0;
    MembershipMap::iterator member = SpectatorRooms().find(uid);
    if (member == SpectatorRooms().end()) return 0;
    Room* const room = FindRoom(member->second);
    if (!room) return 0;
    std::map<uint16_t, CUser*>::iterator spectator =
        room->spectators.find(uid);
    if (spectator == room->spectators.end() || spectator->second != user) {
        return 0;
    }
    return room;
}

uint16_t AllocateRoomId() {
    for (unsigned int attempt = 0u; attempt < 0xffffu; ++attempt) {
        uint16_t candidate = NextRoomId();
        ++NextRoomId();
        if (NextRoomId() == 0u) NextRoomId() = 1u;
        if (candidate != 0u && Rooms().find(candidate) == Rooms().end()) {
            return candidate;
        }
    }
    return 0u;
}

void PutWideString(PacketGuard& packet,
                   const std::vector<uint16_t>& value) {
    const int bytes = static_cast<int>(value.size() * sizeof(uint16_t));
    packet.put_int(bytes);
    if (bytes > 0) {
        packet.put_binary(
            reinterpret_cast<const char*>(&value[0]), bytes);
    }
}

std::vector<uint16_t> LiveSpectatorUids(const Room& room) {
    std::vector<uint16_t> result;
    result.reserve(room.spectators.size());
    for (std::map<uint16_t, CUser*>::const_iterator it =
             room.spectators.begin();
         it != room.spectators.end(); ++it) {
        if (IsLive(it->second, it->first) &&
            IsSameScope(it->second, room)) {
            result.push_back(it->first);
        }
    }
    return result;
}

void SendToRoom(const Room& room,
                PacketGuard& packet,
                uint16_t excluded_uid) {
    if (room.owner_uid != excluded_uid &&
        IsLive(room.owner, room.owner_uid) &&
        IsSameScope(room.owner, room)) {
        room.owner->Send(packet);
    }
    for (std::map<uint16_t, CUser*>::const_iterator it =
             room.spectators.begin();
         it != room.spectators.end(); ++it) {
        if (it->first != excluded_uid && IsLive(it->second, it->first) &&
            IsSameScope(it->second, room)) {
            it->second->Send(packet);
        }
    }
}

void SendRoomInfo(const Room& room, CUser* recipient) {
    if (!recipient) return;
    const std::vector<uint16_t> spectators = LiveSpectatorUids(room);
    if (spectators.size() > 0xffffu ||
        room.master_message.size() > kMaximumMasterMessageUnits) {
        TraceFailure("NOTI508 count overflow", recipient);
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_UPGRADE_ROOM_INFO);
    packet.put_short(room.room_id);
    // 0828 sub_6241D0 reads this byte at 0x624213 and never branches on,
    // stores, compares or forwards it.  The complete consumer proves it is a
    // reserved/ignored wire byte; zero is the canonical reserved encoding.
    packet.put_byte(0);
    packet.put_short(room.owner_uid);
    PutWideString(packet, room.master_message);
    packet.put_short(static_cast<int>(spectators.size()));
    for (std::vector<uint16_t>::const_iterator it = spectators.begin();
         it != spectators.end(); ++it) {
        packet.put_short(*it);
    }
    packet.finalize(true);
    recipient->Send(packet);
}

void BroadcastCreate(const Room& room) {
    GameWorld* const world = G_GameWorld();
    if (!world) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_CREATE_UPGRADE_ROOM_INFO);
    packet.put_short(room.room_id);
    packet.put_short(room.owner_uid);
    packet.put_byte(room.village);
    packet.put_byte(room.area);
    packet.put_short(room.x);
    packet.put_short(room.y);
    packet.finalize(true);
    world->send_to_area(room.village, room.area, packet);
}

void BroadcastDestroy(const Room& room) {
    GameWorld* const world = G_GameWorld();
    if (!world) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_DESTROY_UPGRADE_ROOM_INFO);
    packet.put_short(room.room_id);
    packet.put_short(room.owner_uid);
    packet.finalize(true);
    world->send_to_area(room.village, room.area, packet);
}

void BroadcastSpectatorJoin(const Room& room,
                            uint16_t spectator_uid) {
    PacketGuard packet;
    packet.put_header(
        ENUM_PACKETCLASS_NOTIPACKET,
        ENUM_NOTIPACKET_UPGRADE_JOIN_SPECTATOR_USER_INFO);
    packet.put_short(spectator_uid);
    packet.finalize(true);
    SendToRoom(room, packet, spectator_uid);
}

void BroadcastSpectatorLeave(const Room& room,
                             uint16_t spectator_uid) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_LEAVE_SPECTATOR_USER_INFO);
    packet.put_short(spectator_uid);
    packet.finalize(true);
    SendToRoom(room, packet, spectator_uid);
}

void BroadcastDonationPool(const Room& room) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_UPGRADE_ROOM_GIVE_ITEM_INFO);
    packet.put_short(room.room_id);
    packet.put_byte(2);
    const std::map<uint32_t, uint32_t>::const_iterator gold =
        room.escrow.find(kGoldKey);
    const std::map<uint32_t, uint32_t>::const_iterator cube =
        room.escrow.find(kClearCubeKey);
    packet.put_int(static_cast<int>(kGoldKey));
    packet.put_int(static_cast<int>(
        gold == room.escrow.end() ? 0u : gold->second));
    packet.put_int(static_cast<int>(kClearCubeKey));
    packet.put_int(static_cast<int>(
        cube == room.escrow.end() ? 0u : cube->second));
    packet.finalize(true);
    SendToRoom(room, packet, 0u);
}

void BroadcastPutItem(const Room& room) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_UPGRADE_ROOM_PUT_UP_ITEM);
    packet.put_short(room.room_id);
    packet.put_short(room.target_slot);
    packet.put_int(static_cast<int>(room.wire_item_id));
    packet.put_byte(room.target_level);
    packet.finalize(true);
    SendToRoom(room, packet, 0u);
}

void BroadcastUpgradeStart(const Room& room) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_UPGRADE_ROOM_UPGRADE_START);
    packet.finalize(true);
    SendToRoom(room, packet, 0u);
}

void BroadcastUpgradeCancel(const Room& room) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_UPGRADE_ROOM_UPGRADE_CANCEL);
    packet.finalize(true);
    SendToRoom(room, packet, 0u);
}

void BroadcastUpgradeResult(const Room& room,
                            uint8_t state,
                            uint32_t item_id,
                            uint8_t level) {
    PacketGuard packet;
    packet.put_header(
        ENUM_PACKETCLASS_NOTIPACKET,
        ENUM_NOTIPACKET_UPGRADE_ROOM_UPGRADE_ITEM_STATE);
    packet.put_short(room.room_id);
    packet.put_byte(state);
    packet.put_int(static_cast<int>(item_id));
    packet.put_byte(level);
    packet.finalize(true);
    SendToRoom(room, packet, 0u);
}

void BroadcastMasterMessage(const Room& room) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_UPGRADE_ROOM_MASTER_MESSAGE);
    PutWideString(packet, room.master_message);
    packet.finalize(true);
    SendToRoom(room, packet, 0u);
}

void PruneStaleSpectators(Room& room) {
    std::vector<uint16_t> stale;
    for (std::map<uint16_t, CUser*>::const_iterator it =
             room.spectators.begin();
         it != room.spectators.end(); ++it) {
        if (!IsLive(it->second, it->first) ||
            !IsSameScope(it->second, room)) {
            stale.push_back(it->first);
        }
    }
    for (std::vector<uint16_t>::const_iterator it = stale.begin();
         it != stale.end(); ++it) {
        room.spectators.erase(*it);
        MembershipMap::iterator member = SpectatorRooms().find(*it);
        if (member != SpectatorRooms().end() &&
            member->second == room.room_id) {
            SpectatorRooms().erase(member);
        }
    }
}

void DestroyRoomInternal(uint16_t room_id, bool notify) {
    RoomMap::iterator found = Rooms().find(room_id);
    if (found == Rooms().end()) return;
    Room& room = found->second;
    if (notify) BroadcastDestroy(room);

    MembershipMap::iterator owner = OwnerRooms().find(room.owner_uid);
    if (owner != OwnerRooms().end() && owner->second == room_id) {
        OwnerRooms().erase(owner);
    }
    for (std::map<uint16_t, CUser*>::const_iterator it =
             room.spectators.begin();
         it != room.spectators.end(); ++it) {
        MembershipMap::iterator spectator = SpectatorRooms().find(it->first);
        if (spectator != SpectatorRooms().end() &&
            spectator->second == room_id) {
            SpectatorRooms().erase(spectator);
        }
    }
    if (ActiveRoom() == &room) {
        ActiveRoom() = 0;
        ActiveOwner() = 0;
    }
    // Official client text 22343/22363 proves unused donation escrow expires
    // when public reinforcement ends.  Erasing this session room is the
    // authoritative destruction point; no refund or DB persistence occurs.
    Rooms().erase(found);
}

bool SameTargetIdentity(const Room& room, const Inven_Item& item) {
    return !item.isEmpty() &&
           static_cast<uint32_t>(item.item_id_) == room.wire_item_id &&
           item.globally_unique_id_.server_id_ == room.guid_server &&
           item.globally_unique_id_.datetime_ == room.guid_datetime &&
           item.globally_unique_id_.sequence_no_ == room.guid_sequence &&
           static_cast<uint8_t>(item.GetUpgrade()) == room.target_level;
}

void SnapshotTarget(Room& room,
                    uint16_t slot,
                    const Inven_Item& item) {
    room.staged = true;
    room.target_slot = slot;
    room.wire_item_id = static_cast<uint32_t>(item.item_id_);
    room.target_level = static_cast<uint8_t>(item.GetUpgrade());
    room.guid_server = item.globally_unique_id_.server_id_;
    room.guid_datetime = item.globally_unique_id_.datetime_;
    room.guid_sequence = item.globally_unique_id_.sequence_no_;
    room.result_state = kResultUnchanged;
}

uint32_t Escrow(const Room& room, uint32_t item_id) {
    const std::map<uint32_t, uint32_t>::const_iterator found =
        room.escrow.find(item_id);
    return found == room.escrow.end() ? 0u : found->second;
}

int SaturatingAdd(int owner_value, uint32_t room_value) {
    if (owner_value < 0) owner_value = 0;
    const uint64_t total = static_cast<uint64_t>(owner_value) + room_value;
    return total > static_cast<uint64_t>(INT_MAX)
               ? INT_MAX : static_cast<int>(total);
}

}  // namespace

namespace upgrade_room {

int CreateRoom(CUser* user, uint16_t client_x, uint16_t client_y) {
    if (!GetUpgradeRoomConfig()) return kErrorEventUnavailable;

    uint16_t uid = 0u;
    int village = 0;
    int area = 0;
    Area* area_object = 0;
    if (!GetWireUid(user, uid) ||
        !ValidateTownUser(user, village, area, area_object)) {
        return kErrorInvalidState;
    }
    if (OwnerRooms().find(uid) != OwnerRooms().end()) return kErrorNotOwner;
    if (SpectatorRooms().find(uid) != SpectatorRooms().end()) {
        return kErrorOtherRoom;
    }

    const int server_x = user->get_posX();
    const int server_y = user->get_posY();
    GameWorld* const world = G_GameWorld();
    if (server_x < 0 || server_x > 0xffff || server_y < 0 ||
        server_y > 0xffff || client_x != static_cast<uint16_t>(server_x) ||
        client_y != static_cast<uint16_t>(server_y) || !world ||
        !world->is_available_point(village, area, server_x, server_y)) {
        return kErrorInvalidLocation;
    }
    const int zone_error = world->check_restrictive_commercial_transaction_zone(
        village, area, server_x, server_y);
    if (zone_error == kErrorNearNpc) return kErrorNearNpc;
    if (zone_error != 0) return kErrorInvalidLocation;

    const uint16_t room_id = AllocateRoomId();
    if (room_id == 0u) return kErrorGeneral;

    Room room;
    room.room_id = room_id;
    room.owner_uid = uid;
    room.owner = user;
    room.village = static_cast<uint8_t>(village);
    room.area = static_cast<uint8_t>(area);
    room.x = static_cast<uint16_t>(server_x);
    room.y = static_cast<uint16_t>(server_y);
    std::pair<RoomMap::iterator, bool> inserted =
        Rooms().insert(std::make_pair(room_id, room));
    if (!inserted.second ||
        !OwnerRooms().insert(std::make_pair(uid, room_id)).second) {
        if (inserted.second) Rooms().erase(inserted.first);
        return kErrorGeneral;
    }

    SendRoomInfo(inserted.first->second, user);
    BroadcastCreate(inserted.first->second);
    return 0;
}

int DestroyRoom(CUser* user) {
    Room* const room = FindOwnedRoom(user);
    if (!room) return kErrorNotInRoom;
    const uint16_t room_id = room->room_id;
    DestroyRoomInternal(room_id, true);
    return 0;
}

int JoinRoom(CUser* user, uint16_t room_id, uint16_t owner_uid) {
    const UpgradeRoomConfig* const config = GetUpgradeRoomConfig();
    if (!config) return kErrorInvalidState;

    uint16_t uid = 0u;
    int village = 0;
    int area = 0;
    Area* area_object = 0;
    if (!GetWireUid(user, uid) ||
        !ValidateTownUser(user, village, area, area_object)) {
        return kErrorInvalidRoomState;
    }
    if (OwnerRooms().find(uid) != OwnerRooms().end()) return kErrorOtherRoom;
    MembershipMap::iterator prior = SpectatorRooms().find(uid);
    if (prior != SpectatorRooms().end()) {
        return prior->second == room_id ? kErrorNotOwner : kErrorOtherRoom;
    }

    Room* room = FindRoom(room_id);
    if (!room || room->owner_uid != owner_uid ||
        !IsLive(room->owner, room->owner_uid) ||
        !IsSameScope(room->owner, *room)) {
        if (room && (!IsLive(room->owner, room->owner_uid) ||
                     !IsSameScope(room->owner, *room))) {
            DestroyRoomInternal(room_id, true);
        }
        return kErrorInvalidRoomState;
    }
    if (uid == owner_uid) return kErrorNotOwner;
    if (village != room->village || area != room->area) {
        return kErrorInvalidLocation;
    }
    PruneStaleSpectators(*room);
    if (room->spectators.size() >= config->max_spectators) {
        return kErrorInvalidState;
    }
    if (!room->spectators.insert(std::make_pair(uid, user)).second ||
        !SpectatorRooms().insert(std::make_pair(uid, room_id)).second) {
        room->spectators.erase(uid);
        TraceFailure("spectator membership commit failed", user);
        return -1;
    }

    SendRoomInfo(*room, user);
    BroadcastSpectatorJoin(*room, uid);
    BroadcastDonationPool(*room);
    return 0;
}

int LeaveRoom(CUser* user) {
    uint16_t uid = 0u;
    if (!GetWireUid(user, uid)) return kErrorNotInRoom;
    Room* const room = FindSpectatorRoom(user);
    if (!room) return kErrorNotInRoom;
    const uint16_t room_id = room->room_id;
    room->spectators.erase(uid);
    MembershipMap::iterator member = SpectatorRooms().find(uid);
    if (member != SpectatorRooms().end() && member->second == room_id) {
        SpectatorRooms().erase(member);
    }
    BroadcastSpectatorLeave(*room, uid);
    return 0;
}

int Donate(CUser* user, const std::vector<Donation>& donations) {
    const UpgradeRoomConfig* const config = GetUpgradeRoomConfig();
    if (!config) return kErrorInvalidRoomState;

    Room* const room = FindSpectatorRoom(user);
    if (!room) {
        return FindOwnedRoom(user) ? kErrorNotOwner : kErrorNotInRoom;
    }
    if (!IsLive(room->owner, room->owner_uid) ||
        !IsSameScope(room->owner, *room)) {
        const uint16_t room_id = room->room_id;
        DestroyRoomInternal(room_id, true);
        return kErrorInvalidRoomState;
    }
    if (!IsSameScope(user, *room) || donations.empty() ||
        donations.size() > config->donation_limits.size()) {
        return kErrorInvalidRoomState;
    }

    std::map<uint32_t, uint32_t> requested;
    for (std::vector<Donation>::const_iterator it = donations.begin();
         it != donations.end(); ++it) {
        const std::map<uint32_t, uint32_t>::const_iterator limit =
            config->donation_limits.find(it->item_id);
        if (limit == config->donation_limits.end() || it->amount == 0u ||
            it->amount > limit->second ||
            !requested.insert(std::make_pair(it->item_id,
                                             it->amount)).second) {
            return kErrorDonationLimit;
        }
        if (static_cast<uint64_t>(Escrow(*room, it->item_id)) +
                static_cast<uint64_t>(it->amount) >
            static_cast<uint64_t>(limit->second)) {
            return kErrorDonationLimit;
        }
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) return kErrorInvalidRoomState;
    std::map<uint32_t, uint32_t>::const_iterator gold =
        requested.find(kGoldKey);
    if (gold != requested.end() &&
        inventory->get_money() < static_cast<int>(gold->second)) {
        return kErrorInsufficientMoney;
    }
    std::map<uint32_t, uint32_t>::const_iterator cube =
        requested.find(kClearCubeKey);
    if (cube != requested.end() &&
        inventory->get_item_count(kMainInventory, kClearCubeKey) <
            static_cast<int>(cube->second)) {
        return kErrorInsufficientItem;
    }

    std::vector<std::pair<int, int> > required;
    for (std::map<uint32_t, uint32_t>::const_iterator it = requested.begin();
         it != requested.end(); ++it) {
        required.push_back(std::make_pair(static_cast<int>(it->first),
                                          static_cast<int>(it->second)));
    }
    if (inventory->CheckNeedItemList(required) != 1) {
        return cube != requested.end() ? kErrorInsufficientItem
                                       : kErrorInsufficientMoney;
    }
    std::vector<int> deleted_slots;
    bool equipment_changed = false;
    const int deleted = inventory->delete_event_items(
        required, deleted_slots, equipment_changed,
        static_cast<eMoneySubReason>(13),
        static_cast<eItemDelReason>(12), false);
    if (deleted == 0) {
        TraceFailure("donation deduction failed after preflight", user);
        return -1;
    }

    for (std::map<uint32_t, uint32_t>::const_iterator it = requested.begin();
         it != requested.end(); ++it) {
        room->escrow[it->first] += it->second;
    }
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0), 0);
    BroadcastDonationPool(*room);
    return 0;
}

int PutUpItem(CUser* user, uint32_t item_id, uint16_t inventory_slot) {
    Room* const room = FindOwnedRoom(user);
    if (!room) {
        return FindSpectatorRoom(user) ? kErrorNotOwner : kErrorNotInRoom;
    }
    if (!IsSameScope(user, *room) || room->in_upgrade ||
        room->display_state != kRoomIdle) {
        return kErrorInvalidRoomState;
    }
    CInventory* const inventory = user->getCurCharacInvenR();
    if (!inventory ||
        inventory->CheckValidSlot(kMainInventory, inventory_slot) != 1) {
        return kErrorInvalidRoomState;
    }
    if (user->CheckItemLock(kMainInventory, inventory_slot)) {
        return kErrorInvalidRoomState;
    }
    const Inven_Item target = inventory->GetInvenSlot(
        static_cast<INVEN_TYPE>(kMainInventory), inventory_slot);
    if (target.isEmpty() || static_cast<uint32_t>(target.item_id_) != item_id ||
        target.item_category_ != 1u || target.amplify_option_.hasAbility()) {
        return kErrorInvalidRoomState;
    }
    CItem* const prototype =
        G_CDataManager() ? G_CDataManager()->find_item(target.item_id_) : 0;
    if (!prototype || !prototype->vtable_ ||
        !prototype->vtable_->is_impossible_upgrade ||
        prototype->vtable_->is_impossible_upgrade(prototype) != 0) {
        return kErrorInvalidRoomState;
    }

    SnapshotTarget(*room, inventory_slot, target);
    room->display_state = kRoomIdle;
    BroadcastPutItem(*room);
    return 0;
}

int StartUpgrade(CUser* user) {
    Room* const room = FindOwnedRoom(user);
    if (!room) {
        return FindSpectatorRoom(user) ? kErrorNotOwner : kErrorNotInRoom;
    }
    if (!GetUpgradeRoomConfig() || !IsSameScope(user, *room) ||
        !room->staged || room->in_upgrade ||
        room->display_state != kRoomIdle) {
        return kErrorInvalidRoomState;
    }
    CInventory* const inventory = user->getCurCharacInvenR();
    if (!inventory ||
        inventory->CheckValidSlot(kMainInventory, room->target_slot) != 1 ||
        user->CheckItemLock(kMainInventory, room->target_slot)) {
        return kErrorInvalidRoomState;
    }
    const Inven_Item before = inventory->GetInvenSlot(
        static_cast<INVEN_TYPE>(kMainInventory), room->target_slot);
    if (!SameTargetIdentity(*room, before)) return kErrorInvalidRoomState;

    room->display_state = kRoomUpgrading;
    room->in_upgrade = true;
    BroadcastUpgradeStart(*room);

    MSG_ITEM_UPGRADE message;
    message.body_.mode = 0u;
    message.body_.target_slot = room->target_slot;
    message.body_.target_item_id = room->wire_item_id;
    message.body_.ticket_slot = 0xffffu;
    // CMD583 has no material slot on wire.  The scoped adapter resolves a
    // virtual 3037 stack from owner inventory + room escrow and consumes the
    // escrow first; 0xffff cannot alias a physical owner slot.
    message.body_.material_slot = 0xffffu;

    PacketGuard sink;
    sink.clear();
    int result = -1;
    {
        ScopedOwnerUpgradeContext context(user);
        if (!context.ready()) {
            room->in_upgrade = false;
            room->display_state = kRoomIdle;
            BroadcastUpgradeCancel(*room);
            TraceFailure("could not enter scoped upgrade context", user);
            return -1;
        }
        WongWork::CItemUpgrade* const upgrade =
            G_CDataManager() ? G_CDataManager()->get_item_upgrade() : 0;
        result = upgrade ? upgrade->ProcUpgrade(user, &message, &sink) : -1;
    }

    room->in_upgrade = false;
    const bool protected_destruction =
        result == kOrdinaryUpgradeProtectedDestruction;
    if (result != 0 && !protected_destruction) {
        room->display_state = kRoomIdle;
        BroadcastUpgradeCancel(*room);
        if (result < 0) {
            TraceFailure("ordinary upgrade returned unframed error", user);
            return -1;
        }
        // CMD583's target handler only closes 3/7/21.  Ordinary reinforcement
        // has a broader error vocabulary, so no raw ProcUpgrade code may leak
        // onto this wire.  All owner-side revalidation failures collapse to
        // the Upgrade Room's proven invalid-state branch 21.
        TraceFailure("ordinary upgrade rejected Upgrade Room request", user);
        return kErrorInvalidRoomState;
    }

    const Inven_Item after = inventory->GetInvenSlot(
        static_cast<INVEN_TYPE>(kMainInventory), room->target_slot);
    uint8_t state = kResultUnchanged;
    uint8_t level = room->target_level;
    if (after.isEmpty() ||
        static_cast<uint32_t>(after.item_id_) != room->wire_item_id) {
        state = kResultBroken;
        level = 0u;
    } else {
        level = static_cast<uint8_t>(after.GetUpgrade());
        if (level > room->target_level) {
            state = kResultSuccess;
        } else if (level < room->target_level) {
            state = kResultDowngrade;
        }
    }

    const uint32_t original_item_id = room->wire_item_id;
    room->result_state = state;
    if (state == kResultBroken) {
        room->staged = false;
        room->wire_item_id = kNoItem;
        room->target_level = 0u;
        room->display_state = kRoomResult;
    } else {
        SnapshotTarget(*room, room->target_slot, after);
        room->result_state = state;
        // The 0828 NOTI509 handler intentionally leaves the room record in
        // state 1 for result=3; the owner's later CMD584/NOTI517 closes that
        // no-change animation.  States 0/1/2 transition the record to 2.
        room->display_state = state == kResultUnchanged
                                  ? kRoomUpgrading : kRoomResult;
    }
    BroadcastUpgradeResult(*room, state, original_item_id, level);
    return 0;
}

int CancelUpgrade(CUser* user) {
    Room* const room = FindOwnedRoom(user);
    if (!room) {
        return FindSpectatorRoom(user) ? kErrorNotOwner : kErrorNotInRoom;
    }
    if (room->in_upgrade) return kErrorInvalidRoomState;
    room->display_state = kRoomIdle;
    BroadcastUpgradeCancel(*room);
    return 0;
}

int SetMasterMessage(CUser* user,
                     const std::vector<uint16_t>& utf16_message) {
    Room* const room = FindOwnedRoom(user);
    if (!room) {
        return FindSpectatorRoom(user) ? kErrorNotOwner : kErrorNotInRoom;
    }
    if (!IsSameScope(user, *room) ||
        utf16_message.size() > kMaximumMasterMessageUnits) {
        return kErrorInvalidRoomState;
    }
    room->master_message = utf16_message;
    BroadcastMasterMessage(*room);
    return 0;
}

void OnUserDisconnect(CUser* user) {
    uint16_t uid = 0u;
    if (!GetWireUid(user, uid)) return;
    MembershipMap::iterator owner = OwnerRooms().find(uid);
    if (owner != OwnerRooms().end()) {
        Room* const room = FindRoom(owner->second);
        if (room && room->owner == user) {
            DestroyRoomInternal(room->room_id, true);
            return;
        }
    }
    MembershipMap::iterator spectator = SpectatorRooms().find(uid);
    if (spectator == SpectatorRooms().end()) return;
    Room* const room = FindRoom(spectator->second);
    if (!room) {
        SpectatorRooms().erase(spectator);
        return;
    }
    std::map<uint16_t, CUser*>::iterator found = room->spectators.find(uid);
    if (found != room->spectators.end() && found->second == user) {
        room->spectators.erase(found);
        SpectatorRooms().erase(spectator);
        BroadcastSpectatorLeave(*room, uid);
    }
}

void OnUserAreaChanged(CUser* user) {
    Room* room = FindOwnedRoom(user);
    if (room && !IsSameScope(user, *room)) {
        DestroyRoomInternal(room->room_id, true);
        return;
    }
    room = FindSpectatorRoom(user);
    if (room && !IsSameScope(user, *room)) {
        OnUserDisconnect(user);
    }
}

void OnEnterWorld(CUser* user) {
    if (!user || user->get_state() != 3) return;
    const int village = user->getCurCharacVill();
    const int area = user->get_area(false);
    if (village < 0 || village > 0xff || area < 0 || area > 0xff) return;

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_UPGRADE_ROOM_LIST_INFO);
    int count_index = packet.get_index();
    packet.put_short(0);
    int count = 0;
    for (RoomMap::const_iterator it = Rooms().begin(); it != Rooms().end();
         ++it) {
        const Room& room = it->second;
        if (room.village != village || room.area != area ||
            !IsLive(room.owner, room.owner_uid) ||
            !IsSameScope(room.owner, room) || count >= 0xffff) {
            continue;
        }
        packet.put_short(room.room_id);
        packet.put_short(room.owner_uid);
        packet.put_byte(room.village);
        packet.put_byte(room.area);
        packet.put_short(room.x);
        packet.put_short(room.y);
        packet.put_byte(room.display_state);
        packet.put_int(static_cast<int>(room.wire_item_id));
        packet.put_byte(room.target_level);
        ++count;
    }
    packet.put_short(count_index, count);
    packet.finalize(true);
    user->Send(packet);
}

ScopedOwnerUpgradeContext::ScopedOwnerUpgradeContext(CUser* user)
    : user_(user), room_(0), ready_(false) {
    if (!user || ActiveRoom() || ActiveOwner()) return;
    Room* const room = FindOwnedRoom(user);
    if (!room || !room->in_upgrade ||
        room->display_state != kRoomUpgrading) {
        return;
    }
    ActiveRoom() = room;
    ActiveOwner() = user;
    room_ = room;
    ready_ = true;
}

ScopedOwnerUpgradeContext::~ScopedOwnerUpgradeContext() {
    if (ready_ && ActiveRoom() == static_cast<Room*>(room_) &&
        ActiveOwner() == user_) {
        ActiveRoom() = 0;
        ActiveOwner() = 0;
    }
}

bool IsOwnerUpgradeContextActive(const CUser* user) {
    return user && ActiveOwner() == user && ActiveRoom() != 0;
}

bool ResolveUpgradeMaterial(CUser* user,
                            uint16_t requested_slot,
                            Inven_Item& material) {
    (void)requested_slot;
    if (!IsOwnerUpgradeContextActive(user)) return false;
    CInventory* const inventory = user->getCurCharacInvenR();
    Room* const room = ActiveRoom();
    if (!inventory || !room) return true;

    Inven_Item physical;
    const int physical_slot = inventory->GetInvenData(
        static_cast<int>(kClearCubeKey), physical);
    if (physical_slot >= 0 && !physical.isEmpty()) {
        material = physical;
    } else {
        CItem* const prototype = G_CDataManager()
            ? G_CDataManager()->find_item(static_cast<int>(kClearCubeKey)) : 0;
        if (!prototype || !prototype->vtable_ ||
            !prototype->vtable_->make_item) {
            material.reset();
            return true;
        }
        prototype->vtable_->make_item(prototype, material);
    }
    // Ordinary ProcUpgrade consumes one concrete stack.  Use the same stack
    // count selected by GetInvenData rather than summing several stacks that
    // its eventual delete path cannot consume atomically.
    const int owner_count = physical_slot >= 0 && !physical.isEmpty()
                                ? physical.get_add_info() : 0;
    material.set_add_info(SaturatingAdd(owner_count,
                                        Escrow(*room, kClearCubeKey)));
    return true;
}

int EffectiveMoney(const CUser* user, int owner_money) {
    if (!IsOwnerUpgradeContextActive(user)) return owner_money;
    return SaturatingAdd(owner_money, Escrow(*ActiveRoom(), kGoldKey));
}

int ConsumeUpgradeMoney(CUser* user, int total, int money_reason) {
    if (!IsOwnerUpgradeContextActive(user) || total < 0) return 0;
    if (total == 0) return 1;
    Room* const room = ActiveRoom();
    CInventory* const inventory = user->getCurCharacInvenW();
    if (!room || !inventory) return 0;

    uint32_t& pool = room->escrow[kGoldKey];
    const uint32_t used = std::min<uint32_t>(
        pool, static_cast<uint32_t>(total));
    pool -= used;
    const int remainder = total - static_cast<int>(used);
    if (remainder > 0 &&
        inventory->use_money(remainder,
                             static_cast<eMoneySubReason>(money_reason),
                             true) != 1) {
        pool += used;
        return 0;
    }
    if (used != 0u) BroadcastDonationPool(*room);
    return 1;
}

int ConsumeUpgradeMaterial(CUser* user,
                           uint16_t requested_slot,
                           int item_id,
                           int total,
                           int item_reason) {
    (void)requested_slot;
    if (!IsOwnerUpgradeContextActive(user) || total < 0 ||
        item_id != static_cast<int>(kClearCubeKey)) {
        return 0;
    }
    if (total == 0) return 1;
    Room* const room = ActiveRoom();
    CInventory* const inventory = user->getCurCharacInvenW();
    if (!room || !inventory) return 0;

    uint32_t& pool = room->escrow[kClearCubeKey];
    const uint32_t used = std::min<uint32_t>(
        pool, static_cast<uint32_t>(total));
    pool -= used;
    const int remainder = total - static_cast<int>(used);
    if (remainder > 0) {
        std::vector<std::pair<int, int> > required;
        required.push_back(std::make_pair(item_id, remainder));
        if (inventory->CheckNeedItemList(required) != 1) {
            pool += used;
            return 0;
        }
        std::vector<int> deleted_slots;
        bool equipment_changed = false;
        if (inventory->delete_event_items(
                required, deleted_slots, equipment_changed,
                static_cast<eMoneySubReason>(13),
                static_cast<eItemDelReason>(item_reason), false) == 0) {
            pool += used;
            return 0;
        }
    }
    if (used != 0u) BroadcastDonationPool(*room);
    return 1;
}

}  // namespace upgrade_room_0828
