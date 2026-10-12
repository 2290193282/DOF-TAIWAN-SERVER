#include "old_game/pvp/battlefield/war_room.h"

#include "common/config.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/pvp/battlefield/war_area_counter.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/dungeon/instance/dungeon_entrance_log.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

// This helper intentionally has no CUser access.  The peer-endpoint bytes
// are emitted by a lambda inside WarRoom::Join below, where the original room
// owner has direct access to CUser's embedded CNetwork child.
void PutPeerConnectionInfo(PacketGuard& packet,
                           CUser* user,
                           CNetwork4096_450000& network) {
    packet.put_short(user->get_unique_id());
    packet.put_int(static_cast<int>(network.get_inner_ip()));
    packet.put_int(static_cast<int>(network.get_ip()));
    packet.put_short(network.get_port());
    packet.put_int(user->get_acc_id());
    packet.put_byte(network.get_nat_type());
    packet.put_int(static_cast<int>(network.get_mtu()));
    packet.put_byte(user->getCurCharacR() ? user->getCurCharacStamina() : 0);
}

}  // namespace

// IDA_EA: 0x86BACAE
// IDA_NAME: _ZN7WarRoom10IsJoinableEP5CUser
// DEMANGLED: WarRoom::IsJoinable(CUser *)
int WarRoom::IsJoinable(CUser* user) {
    const int state = GetState();
    if (state == 4 || state == 3 || state == -1 || state == 2) return 19;
    if (user->isCompetitionMercenary()) return 21;

    const WarAreaCounter* const counter =
        G_CDataManager()->get_war_area_counter();
    if (user->getCurCharacMoney() < counter->door_fee[room_level_]) return 10;
    if (!user->CheckFatigue()) return 67;

    const int level = user->get_charac_level();
    switch (room_mode_) {
    case 2:
        if (level <= 19 || level > 29) return 14;
        break;
    case 3:
        if (level <= 29 || level > 39) return 14;
        break;
    case 4:
        if (level <= 39 || level > 49) return 14;
        break;
    case 5:
        if (level <= 49 || level > 59) return 14;
        break;
    case 6:
        if (level <= 59 ||
            level > oldsrv::config::kCharacterLevelMaximum) {
            return 14;
        }
        break;
    default:
        return 14;
    }

    UpdateBanList();
    if (IsBanUser(user)) return 30;
    for (int slot = 0; slot <= 5; ++slot) {
        if (!war_field_.room_users_[slot] &&
            war_field_.seat_states_[slot] == 255) {
            return 0;
        }
    }
    return 4;
}

// IDA_EA: 0x86BAE9A
// IDA_NAME: _ZN7WarRoom4JoinEP5CUserRi
// DEMANGLED: WarRoom::Join(CUser *,int &)
int WarRoom::Join(CUser* user, int& slot_out) {
    const int waiter_count = GetWaiterCount();
    PacketGuard basic_info_packet;
    PacketGuard peer_info_packet;
    const auto put_peer_connection_info = [](PacketGuard& packet,
                                             CUser* peer) {
        CNetwork4096_450000& network = peer->network_;
        PutPeerConnectionInfo(packet, peer, network);
    };
    basic_info_packet.put_header(0, 2);
    peer_info_packet.put_header(0, 11);
    basic_info_packet.put_byte(1);
    basic_info_packet.put_short(waiter_count);
    peer_info_packet.put_byte(waiter_count);

    for (int slot = 0; slot <= 5; ++slot) {
        CUser* const occupant = war_field_.room_users_[slot];
        if (!occupant) continue;
        occupant->make_basic_info(basic_info_packet, 1);
        put_peer_connection_info(peer_info_packet, occupant);
    }
    basic_info_packet.finalize(true);
    peer_info_packet.finalize(true);

    int joined_slot = -1;
    for (int slot = 0; slot <= 5; ++slot) {
        if (!war_field_.room_users_[slot] &&
            war_field_.seat_states_[slot] == 255) {
            joined_slot = slot;
            war_field_.room_users_[slot] = user;
            user->SetWarRoomIndex(static_cast<short>(room_index_));
            slot_out = slot;
            break;
        }
    }
    if (joined_slot == -1) return 4;

    user->Send(basic_info_packet);
    user->Send(peer_info_packet);
    if (war_field_.dungeon_) {
        user->history_log().EnterDungeon(
            war_field_.dungeon_->GetDungeonName(), room_mode_, "", 0);
    }

    war_field_.seat_states_[joined_slot] = 0;
    war_field_.live_flags_[joined_slot] = 1;

    basic_info_packet.clear();
    basic_info_packet.put_header(0, 2);
    basic_info_packet.put_byte(1);
    basic_info_packet.put_short(1);
    user->make_basic_info(basic_info_packet, 1);
    basic_info_packet.finalize(true);
    SendToRoom(basic_info_packet);

    peer_info_packet.clear();
    peer_info_packet.put_header(0, 11);
    peer_info_packet.put_byte(1);
    put_peer_connection_info(peer_info_packet, user);
    peer_info_packet.finalize(true);
    SendToRoom(peer_info_packet);

    basic_info_packet.clear();
    basic_info_packet.put_header(1, ENUM_CMDPACKET_ENTER_WARROOM);
    basic_info_packet.put_byte(1);
    basic_info_packet.put_int(user->getCurCharacMoney());
    const uint16_t current_second = static_cast<uint16_t>(
        GlobalData::s_systemTime_.getCurSec());
    basic_info_packet.put_short(
        current_second - static_cast<uint16_t>(war_field_.battle_started_at_sec_));
    basic_info_packet.finalize(true);
    user->Send(basic_info_packet);
    return 0;
}

// IDA_EA: 0x86BD6D4
// IDA_NAME: _ZN7WarRoom5StartEv
// DEMANGLED: WarRoom::Start(void)
void WarRoom::Start() {
    PacketGuard packet;
    for (int slot = 0; slot <= 5; ++slot) {
        CUser* const user = war_field_.room_users_[slot];
        if (!user) continue;

        packet.clear();
        packet.put_header(0, 90);
        packet.finalize(true);
        user->Send(packet);
        if (war_field_.dungeon_) {
            GetInstanceDungeonEntranceLog()->IncrementDungeonEntrance(
                war_field_.dungeon_->get_index(), false);
        }
    }
    ChangeState(static_cast<WARROOM_STATE>(1));
    for (int slot = 0; slot <= 5; ++slot) {
        CUser* const user = war_field_.room_users_[slot];
        if (user) user->getHackAnalyzer()->beginCollectHackInfo(user);
    }
}
