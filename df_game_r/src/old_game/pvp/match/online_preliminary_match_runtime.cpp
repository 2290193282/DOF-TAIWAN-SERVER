#include "old_game/pvp/match/online_preliminary_match_runtime.h"

#include "common/config.h"
#include <cmath>
#include <cstring>
#include <new>

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/season/pvp_rating_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/pvp/match/fair_pvp_match_runtime.h"
#include "old_game/pvp/match/league_match_room_list_runtime.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/session/online_preliminary_match_list_runtime.h"
#include "old_game/player/session/online_preliminary_member_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "old_game/pvp/match/timer_start_matching.h"

namespace online_preliminary {

CLeagueMatchRoomList* COnlinePreliminary::pWaitingRoomList_ = 0;
int COnlinePreliminary::nWaitingRoomListRefCount_ = 0;
int COnlinePreliminary::nextAuthValue_ = 0;
bool COnlinePreliminary::bMatchable_ = false;

// IDA_EA: 0x855FEE8
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary22GetPlayCountPerOneTeamEv
// DEMANGLED: online_preliminary::COnlinePreliminary::GetPlayCountPerOneTeam(void)
int COnlinePreliminary::GetPlayCountPerOneTeam() {
    return G_CDataManager()->GetPvPTeamCount();
}

// IDA_EA: 0x855FEFE
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminaryC2Ev
// DEMANGLED: online_preliminary::COnlinePreliminary::COnlinePreliminary(void)
COnlinePreliminary::COnlinePreliminary()
    : IMatch(),
      room_info_() {
    if (!pWaitingRoomList_) {
        pWaitingRoomList_ = new CLeagueMatchRoomList();
    }
    ++nWaitingRoomListRefCount_;
}

// IDA_EA: 0x855FFD6 (D2), 0x8560064 (D0)
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminaryD2Ev
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminaryD0Ev
// DEMANGLED: online_preliminary::COnlinePreliminary::~COnlinePreliminary()
COnlinePreliminary::~COnlinePreliminary() {
    --nWaitingRoomListRefCount_;
    if (pWaitingRoomList_ && !nWaitingRoomListRefCount_) {
        // The old D2 path releases the allocation directly and deliberately
        // leaves the static address unchanged.
        ::operator delete(pWaitingRoomList_);
    }
}

// IDA_EA: 0x8560082
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary10_GetRatingEP5CUser
float COnlinePreliminary::_GetRating(CUser* user) {
    if (static_cast<uint8_t>(GetPlayCountPerOneTeam()) == 1u) {
        COnlinePreliminaryMember* const member =
            static_cast<COnlinePreliminaryMember*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(4)));
        return member->GetRating();
    }

    COnlinePreliminaryTeam* const team =
        user->GetCurCharacOnlinePreliminaryTeam();
    return team ? team->GetRating() : 0.0f;
}

// IDA_EA: 0x85600F6
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary10_SetRatingEP5CUserf
bool COnlinePreliminary::_SetRating(CUser* user, float rating) {
    if (static_cast<uint8_t>(GetPlayCountPerOneTeam()) == 1u) {
        COnlinePreliminaryMember* const member =
            static_cast<COnlinePreliminaryMember*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(4)));
        return member->SetRating(rating) != 0;
    }

    COnlinePreliminaryTeam* const team =
        user->GetCurCharacOnlinePreliminaryTeam();
    if (!team) {
        return false;
    }
    return team->SetRating(rating) != 0;
}

// IDA_EA: 0x856015E
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary17_IncreaseWinCountEP5CUser
bool COnlinePreliminary::_IncreaseWinCount(CUser* user) {
    if (static_cast<uint8_t>(GetPlayCountPerOneTeam()) == 1u) {
        COnlinePreliminaryMember* const member =
            static_cast<COnlinePreliminaryMember*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(4)));
        return member->IncreaseWinCount() != 0;
    }

    COnlinePreliminaryTeam* const team =
        user->GetCurCharacOnlinePreliminaryTeam();
    if (!team) {
        return false;
    }
    return team->IncreaseWinCount() != 0;
}

// IDA_EA: 0x85601B8
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary18_IncreaseLoseCountEP5CUser
bool COnlinePreliminary::_IncreaseLoseCount(CUser* user) {
    if (static_cast<uint8_t>(GetPlayCountPerOneTeam()) == 1u) {
        COnlinePreliminaryMember* const member =
            static_cast<COnlinePreliminaryMember*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(4)));
        return member->IncreaseLoseCount() != 0;
    }

    COnlinePreliminaryTeam* const team =
        user->GetCurCharacOnlinePreliminaryTeam();
    if (!team) {
        return false;
    }
    return team->IncreaseLoseCount() != 0;
}

// IDA_EA: 0x8560212
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary22_GetEachOtherPlayCountEP5CUserS2_
int COnlinePreliminary::_GetEachOtherPlayCount(CUser* user,
                                               CUser* opponent) {
    if (static_cast<uint8_t>(GetPlayCountPerOneTeam()) == 1u) {
        COnlinePreliminaryMemberMatchList* const match_list =
            static_cast<COnlinePreliminaryMemberMatchList*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(5)));
        return match_list->GetPeerPlayCount(opponent->GetServerGroup(),
                                            opponent->getCurCharacNo());
    }

    COnlinePreliminaryTeam* const team =
        user->GetCurCharacOnlinePreliminaryTeam();
    return team ? team->GetEachOtherTeamPlayCount(
                      opponent->GetCurCharacOnlinePreliminaryTeamId())
                : 0;
}

// IDA_EA: 0x85602AE
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary27_IncreaseEachOtherPlayCountEP5CUserS2_
bool COnlinePreliminary::_IncreaseEachOtherPlayCount(CUser* user,
                                                      CUser* opponent) {
    if (static_cast<uint8_t>(GetPlayCountPerOneTeam()) == 1u) {
        COnlinePreliminaryMemberMatchList* const match_list =
            static_cast<COnlinePreliminaryMemberMatchList*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(5)));
        return match_list->IncreasePeerPlayCount(
                   opponent->GetServerGroup(), opponent->getCurCharacNo()) !=
               0;
    }

    COnlinePreliminaryTeam* const team =
        user->GetCurCharacOnlinePreliminaryTeam();
    if (!team) {
        return false;
    }
    return team->IncreaseEachOtherTeamPlayCount(
               opponent->GetCurCharacOnlinePreliminaryTeamId()) !=
           0;
}

// IDA_EA: 0x8560342
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary10_GetRatingE22ENUM_SCHOOL_MATCH_TEAM
float COnlinePreliminary::_GetRating(ENUM_SCHOOL_MATCH_TEAM team) {
    return teams_[static_cast<int>(team)].rating;
}

// IDA_EA: 0x8560368
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary10_SetRatingE22ENUM_SCHOOL_MATCH_TEAMf
bool COnlinePreliminary::_SetRating(ENUM_SCHOOL_MATCH_TEAM team,
                                    float rating) {
    TeamInfoSecond& team_info = teams_[static_cast<int>(team)];
    for (int index = 0; index <= 3; ++index) {
        PlayerInfoSecond& player = team_info.players[index];
        if (player.occupied && player.user) {
            return _SetRating(player.user, rating);
        }
    }
    return false;
}

// IDA_EA: 0x85603FA
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary17_IncreaseWinCountE22ENUM_SCHOOL_MATCH_TEAM
bool COnlinePreliminary::_IncreaseWinCount(ENUM_SCHOOL_MATCH_TEAM team) {
    TeamInfoSecond& team_info = teams_[static_cast<int>(team)];
    for (int index = 0; index <= 3; ++index) {
        PlayerInfoSecond& player = team_info.players[index];
        if (player.occupied && player.user) {
            return _IncreaseWinCount(player.user);
        }
    }
    return false;
}

// IDA_EA: 0x8560484
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary18_IncreaseLoseCountE22ENUM_SCHOOL_MATCH_TEAM
bool COnlinePreliminary::_IncreaseLoseCount(ENUM_SCHOOL_MATCH_TEAM team) {
    TeamInfoSecond& team_info = teams_[static_cast<int>(team)];
    for (int index = 0; index <= 3; ++index) {
        PlayerInfoSecond& player = team_info.players[index];
        if (player.occupied && player.user) {
            return _IncreaseLoseCount(player.user);
        }
    }
    return false;
}

// IDA_EA: 0x856050E
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary22_GetEachOtherPlayCountEv
int COnlinePreliminary::_GetEachOtherPlayCount() {
    CUser* first_team_user = 0;
    CUser* second_team_user = 0;
    for (int index = 0; index <= 3; ++index) {
        if (teams_[0].players[index].occupied && teams_[0].players[index].user) {
            first_team_user = teams_[0].players[index].user;
        }
    }
    for (int index = 0; index <= 3; ++index) {
        if (teams_[1].players[index].occupied && teams_[1].players[index].user) {
            second_team_user = teams_[1].players[index].user;
        }
    }
    return first_team_user && second_team_user
               ? _GetEachOtherPlayCount(first_team_user, second_team_user)
               : 0;
}

// IDA_EA: 0x85605EE
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary27_IncreaseEachOtherPlayCountEv
bool COnlinePreliminary::_IncreaseEachOtherPlayCount() {
    CUser* first_team_user = 0;
    CUser* second_team_user = 0;
    for (int index = 0; index <= 3; ++index) {
        if (teams_[0].players[index].occupied && teams_[0].players[index].user) {
            first_team_user = teams_[0].players[index].user;
        }
    }
    for (int index = 0; index <= 3; ++index) {
        if (teams_[1].players[index].occupied && teams_[1].players[index].user) {
            second_team_user = teams_[1].players[index].user;
        }
    }
    if (!first_team_user || !second_team_user) {
        return false;
    }
    _IncreaseEachOtherPlayCount(first_team_user, second_team_user);
    return _IncreaseEachOtherPlayCount(second_team_user, first_team_user);
}

// IDA_EA: 0x85606E0
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary14_CheckSameTeamEP8PvP_RoomP5CUser
bool COnlinePreliminary::_CheckSameTeam(PvP_Room*, CUser* user) {
    const int team_id = user->GetCurCharacOnlinePreliminaryTeamId();
    return team_id != -1 && team_id == GetTeamId();
}

// IDA_EA: 0x8560736
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary14_CheckObserverEP8PvP_RoomP5CUser
bool COnlinePreliminary::_CheckObserver(PvP_Room* room, CUser*) {
    int observer_count = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (room->get_team(seat) == 3) {
            ++observer_count;
        }
    }
    return teams_[0].size() - observer_count <
           static_cast<uint8_t>(GetPlayCountPerOneTeam());
}

// IDA_EA: 0x85607B2
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary13CheckJoinableEP8PvP_RoomP5CUser
int COnlinePreliminary::CheckJoinable(PvP_Room* room, CUser* user) {
    if (CheckUnusableEquipmentInUser(user) != 1) {
        return 1;
    }

    const uint8_t player_count_per_team =
        static_cast<uint8_t>(GetPlayCountPerOneTeam());
    if (room->get_player_count() >= 2 * player_count_per_team) {
        return 4;
    }
    if (teams_[0].size() + teams_[1].size() >
        2 * player_count_per_team) {
        return 4;
    }

    switch (state_) {
    case 0:
        return _CheckSameTeam(room, user) && _CheckObserver(room, user) ? 0 : 4;
    case 3:
        return 0;
    case 1:
    case 4:
    case 5:
    default:
        return 4;
    }
}

// IDA_EA: 0x85608BC
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary9CheckSeatEi
int COnlinePreliminary::CheckSeat(int seat) {
    return seat != 2;
}

// IDA_EA: 0x85608D4
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary22SetPlayCountPerOneTeamEj
void COnlinePreliminary::SetPlayCountPerOneTeam(uint32_t) {
    // Exact old body: ret.
}

// IDA_EA: 0x85608DA
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary12SetMatchAbleEb
int COnlinePreliminary::SetMatchAble(bool matchable) {
    bMatchable_ = matchable;
    return static_cast<int>(matchable);
}

// IDA_EA: 0x85608F2
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary10RequestMapERsR15PVP_BATTLE_MODE
void COnlinePreliminary::RequestMap(int16_t& map_no,
                                    PVP_BATTLE_MODE& battle_mode) {
    map_no = 0;
    if (static_cast<int>(battle_mode) != 6) {
        battle_mode = static_cast<PVP_BATTLE_MODE>(G_CDataManager()->GetPvPMode());
    }
}

// IDA_EA: 0x8560920
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary22CheckUnusableEquipmentEPK10Inven_Item
bool COnlinePreliminary::CheckUnusableEquipment(const Inven_Item* item) {
    if (!item) {
        return false;
    }
    const int item_id = item->getKey();
    if (!item_id) {
        return true;
    }

    CItem* const definition = G_CDataManager()->find_item(item_id);
    if (!definition || definition->is_stackable()) {
        return false;
    }

    const CEquipItem* const equipment =
        static_cast<const CEquipItem*>(definition);
    const int usable_equipment_type = equipment->GetUsableEquipmentType();
    if (!usable_equipment_type) {
        return true;
    }

    ServerParameterScript* const parameters =
        G_CDataManager()->get_server_parameter_script();
    if (parameters->CheckUnusableEquipmentInfo(
            usable_equipment_type, definition->getUsableLevel()) != 1) {
        return false;
    }

    // CItem's recovered vtable names the old +0x0c virtual slot.  This is
    // the original item-type query, not an offset/layout substitute.
    const int item_type = definition->vtable_->get_item_type(definition);
    return parameters->CheckUnusableItem(
               item_type,
               static_cast<unsigned long>(definition->get_index())) == 1;
}

// IDA_EA: 0x8560A42
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary28CheckUnusableEquipmentInUserEP5CUser
int COnlinePreliminary::CheckUnusableEquipmentInUser(CUser* user) {
    CInventory* const inventory = user->getCurCharacInvenR();
    for (int slot = oldsrv::config::kAvatarEquipmentTypeFirst;
         slot <= oldsrv::config::kEquipmentTypeCreature;
         ++slot) {
        if (!CheckUnusableEquipment(inventory->GetInvenRef(0, slot))) {
            user->SendNotiPacketMessage(
                g_scriptStringManager_.findString(
                    4, "game_server_msg_145", 0),
                static_cast<ENUM_MESSAGE_TYPE>(0));
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8560AE4
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary12OnCreateRoomEP5CUser
int COnlinePreliminary::OnCreateRoom(CUser* user) {
    if (CheckUnusableEquipmentInUser(user) != 1) {
        return 19;
    }
    auth_value_ = ++nextAuthValue_;
    state_ = 0;
    teams_[1].reset();
    teams_[0].reset();
    teams_[0].add(user);
    teams_[0].online_preliminary_team_id =
        user->GetCurCharacOnlinePreliminaryTeamId();
    teams_[0].rating = _GetRating(user);
    return 0;
}

// IDA_EA: 0x8560B8A
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary13OnDestroyRoomEP5CUser
void COnlinePreliminary::OnDestroyRoom(CUser*) {
    state_ = 0;
    pWaitingRoomList_->DelRoom(this);
}

// IDA_EA: 0x8560BBA
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary10OnJoinRoomEP5CUser
void COnlinePreliminary::OnJoinRoom(CUser* user) {
    if (state_ != 3) {
        teams_[0].add(user);
    }
}

// IDA_EA: 0x8560BE2
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary13_UpdateRatingEi22ENUM_ELO_RATING_SYSTEM
bool COnlinePreliminary::_UpdateRating(
    int team, ENUM_ELO_RATING_SYSTEM rating_system) {
    if (team == 1) {
        float rating = _GetRating(static_cast<ENUM_SCHOOL_MATCH_TEAM>(0));
        float opposing_rating =
            _GetRating(static_cast<ENUM_SCHOOL_MATCH_TEAM>(1));
        CEloRating::calc(rating, opposing_rating, rating_system,
                         PvPGlobalEnvironmentVariable::school_match_elo_k_);
        return _SetRating(static_cast<ENUM_SCHOOL_MATCH_TEAM>(0), rating);
    }

    float rating = _GetRating(static_cast<ENUM_SCHOOL_MATCH_TEAM>(1));
    float opposing_rating =
        _GetRating(static_cast<ENUM_SCHOOL_MATCH_TEAM>(0));
    CEloRating::calc(rating, opposing_rating, rating_system,
                     PvPGlobalEnvironmentVariable::school_match_elo_k_);
    return _SetRating(static_cast<ENUM_SCHOOL_MATCH_TEAM>(1), rating);
}

// IDA_EA: 0x8560CD2
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary11OnLeaveRoomEP5CUserP8PvP_Roomii
void COnlinePreliminary::OnLeaveRoom(CUser* user,
                                     PvP_Room* room,
                                     int team,
                                     int seat) {
    if (state_ == 4) {
        _SendMatchStatePacket(4, 2);
        state_ = 0;
        _ReturnTeam(room, room->get_manager_team(), user);
    } else if (state_ == 5) {
        if (room->GetTeamCount(team) == 1) {
            _UpdateRating(team, static_cast<ENUM_ELO_RATING_SYSTEM>(0));
            _IncreaseLoseCount(
                static_cast<ENUM_SCHOOL_MATCH_TEAM>(
                    _GetTeam(static_cast<ENUM_SEAT_STATE>(team))));
        }
    } else {
        state_ = 0;
    }

    if (team == 1) {
        teams_[0].del(user);
    } else {
        teams_[1].del(user);
    }
    pWaitingRoomList_->DelRoom(this);
    (void)seat;
}

// IDA_EA: 0x8560DF2
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary12_PullOutRoomEP5CUserP8PvP_Roomi
void COnlinePreliminary::_PullOutRoom(CUser* user,
                                      PvP_Room* room,
                                      int seat) {
    if (room->OnSetPVPSeatState(
            user, seat, static_cast<ENUM_SEAT_STATE>(254),
            static_cast<ENUM_WALKOUT_TYPE>(2)) != 1) {
        room->reset();
        G_CGameManager()->PutPvp(room);
    }
}

// IDA_EA: 0x8560E4A
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary10_PutInRoomEP5CUserP8PvP_Roomb
int COnlinePreliminary::_PutInRoom(CUser* user,
                                   PvP_Room* room,
                                   bool move_room) {
    PacketGuard packet;
    int seat = 0;
    const int join_result = room->join_room(user, seat, move_room);
    if (join_result <= 0) {
        room->make_seat_info(reinterpret_cast<char*>(&packet), seat);
        G_GameWorld()->send_all(packet);
        G_GameWorld()->goto_pvp(user);
    } else {
        packet.put_header(1, ENUM_CMDPACKET_ENTER_PVP_ROOM);
        packet.put_byte(0);
        packet.put_byte(join_result);
        packet.finalize(true);
        user->Send(packet);
    }
    return seat;
}

// IDA_EA: 0x8560F74
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary15_BackupRoomInfoEP8PvP_Room
void COnlinePreliminary::_BackupRoomInfo(PvP_Room* room) {
    room->get_room_info(&room_info_);
}

// IDA_EA: 0x8560F92
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary9_MakeRoomEP5CUser
PvP_Room* COnlinePreliminary::_MakeRoom(CUser* user) {
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_MAKE_PVP_ROOM);

    PvP_Room* const room = G_CGameManager()->GetPvp();
    if (!room) {
        packet.put_byte(0);
        packet.put_byte(4);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    const int create_result = room->create_room(
        user, static_cast<char>(room_info_.room_title_mode),
        const_cast<char*>(room_info_.room_title.c_str()),
        static_cast<int16_t>(room_info_.requested_map_no),
        room_info_.password_enabled != 0, room_info_.password.c_str(),
        static_cast<int>(room_info_.password.length()),
        static_cast<PVP_BATTLE_MODE>(2));
    if (create_result <= 0) {
        packet.clear();
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
        return room;
    }

    packet.put_byte(0);
    packet.put_byte(create_result);
    packet.finalize(true);
    user->Send(packet);
    G_CGameManager()->PutPvp(room);
    return 0;
}

// IDA_EA: 0x85612A4
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary20_RegisterWaitingRoomEP8PvP_Room
int COnlinePreliminary::_RegisterWaitingRoom(PvP_Room*) {
    return pWaitingRoomList_->AddRoom(this);
}

// IDA_EA: 0x85612C6
// IDA_NAME: _ZNK18online_preliminary18COnlinePreliminary15_CheckMatchableEP8PvP_Room
int COnlinePreliminary::_CheckMatchable(PvP_Room* room) const {
    int observer_count = 0;
    int team_id = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (room->get_team(seat) == 3) {
            ++observer_count;
            continue;
        }

        CUser* const user = room->get_user_seat(seat);
        if (!user) {
            continue;
        }
        const int user_team_id = user->GetCurCharacOnlinePreliminaryTeamId();
        if (team_id) {
            if (user_team_id != team_id) {
                return 22;
            }
        } else {
            team_id = user_team_id;
        }
    }
    return teams_[0].size() - observer_count ==
                   static_cast<uint8_t>(GetPlayCountPerOneTeam())
               ? 0
               : 22;
}

// IDA_EA: 0x85613DE
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary15OnRegisterMatchEP5CUserP8PvP_Room
int COnlinePreliminary::OnRegisterMatch(CUser* user, PvP_Room* room) {
    if (state_ == 1) {
        return 1;
    }
    if (!bMatchable_) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_MATCH, 19);
        return 0;
    }

    const int error = _CheckMatchable(room);
    if (error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_MATCH,
                                 static_cast<unsigned char>(error));
        return 0;
    }

    if (state_ != 2) {
        _RegisterWaitingRoom(room);
    }
    state_ = 1;
    TimerStartMatching::registNextTimer(10, room->get_index(), auth_value_);
    _SendMatchStatePacket(0, 1);
    return 1;
}

// IDA_EA: 0x85614EA
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary11OnMatchRoomEP8PvP_Roomi
int COnlinePreliminary::OnMatchRoom(PvP_Room* room, int auth_value) {
    if (auth_value_ != auth_value || state_ != 1) {
        return 0;
    }

    IMatch* matched = 0;
    if (!pWaitingRoomList_->Match(this, &matched) || !matched) {
        state_ = 2;
        _SendMatchStatePacket(1, 1);
        return 0;
    }

    CUser* const matched_user = matched->GetUser(0);
    if (!matched_user) {
        state_ = 2;
        _SendMatchStatePacket(1, 1);
        return 0;
    }

    PvP_Room* const matched_room = matched_user->GetPVPRoom();
    if (!matched_room) {
        state_ = 2;
        _SendMatchStatePacket(1, 1);
        return 0;
    }

    const uint8_t player_count_per_team =
        static_cast<uint8_t>(GetPlayCountPerOneTeam());
    if (room->get_player_count() > player_count_per_team ||
        teams_[0].size() > player_count_per_team ||
        matched_room->get_player_count() > player_count_per_team) {
        state_ = 2;
        _SendMatchStatePacket(1, 1);
        return 0;
    }

    _BackupRoomInfo(matched_room);
    teams_[1].reset();
    matched_room->get_user_list(&teams_[1]);
    _SendMatchStatePacket(3, 2);
    state_ = 3;

    for (int seat = 0; seat <= 7; ++seat) {
        CUser* const user = matched_room->get_user_seat(seat);
        if (!user) {
            continue;
        }
        const int previous_team = matched_room->get_team(seat);
        _PullOutRoom(user, matched_room, seat);
        const int destination_seat = _PutInRoom(user, room, false);
        if (previous_team == 3) {
            room->OnSetPVPSeatState(
                user, destination_seat, static_cast<ENUM_SEAT_STATE>(3),
                static_cast<ENUM_WALKOUT_TYPE>(2));
        }
    }

    pWaitingRoomList_->DelRoom(this);
    state_ = 4;
    _SendMatchStatePacket(2, 2);
    p2p_ready_count_ = 0;
    const int timer_key =
        room->gen_timer_key(static_cast<TIMER_MESSAGE>(59));
    G_TimerQueue()->InsertTimer(
        5, room->get_index(), 59,
        GlobalData::s_ref_pvp_grade->getPvPTimeout(), timer_key, 0);
    return 1;
}

// IDA_EA: 0x8561A0E
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary6GetGapEP6IMatch
float COnlinePreliminary::GetGap(IMatch* match) {
    COnlinePreliminary* const other =
        static_cast<COnlinePreliminary*>(match);
    return static_cast<float>(std::fabs(
        _GetRating(static_cast<ENUM_SCHOOL_MATCH_TEAM>(0)) -
        other->_GetRating(static_cast<ENUM_SCHOOL_MATCH_TEAM>(0))));
}

// IDA_EA: 0x8561A52
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary12OnStartFightEv
void COnlinePreliminary::OnStartFight() {
    state_ = 5;
    _IncreaseEachOtherPlayCount();
}

// IDA_EA: 0x8561A70
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary14OnCompleteLoadEP5CUser
void COnlinePreliminary::OnCompleteLoad(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, 278);
    packet.put_short(user->get_unique_id());
    packet.finalize(true);
    for (int team = 0; team <= 1; ++team) {
        for (int index = 0; index <= 3; ++index) {
            PlayerInfoSecond& player = teams_[team].players[index];
            if (player.occupied && player.user) {
                player.user->Send(packet);
            }
        }
    }
}

// IDA_EA: 0x8561B80
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary12OnConnectP2PEP5CUserP8PvP_RoomPci
void COnlinePreliminary::OnConnectP2P(CUser*,
                                      PvP_Room* room,
                                      char* result,
                                      int count) {
    if (state_ != 4) {
        return;
    }

    for (int seat = 0; seat <= 7 && seat < count; ++seat) {
        if (!result[seat]) {
            _SendMatchStatePacket(4, 2);
            OnEndFight(room);
            return;
        }
    }

    ++p2p_ready_count_;
    if (p2p_ready_count_ == 2 *
                                static_cast<uint8_t>(
                                    GetPlayCountPerOneTeam())) {
        PacketGuard ready_packet;
        ready_packet.put_header(0, 279);
        ready_packet.finalize(true);
        for (int team = 0; team <= 1; ++team) {
            for (int index = 0; index <= 3; ++index) {
                PlayerInfoSecond& player = teams_[team].players[index];
                if (player.occupied && player.user) {
                    player.user->Send(ready_packet);
                }
            }
        }
    }

    PacketGuard state_packet;
    state_packet.clear();
    room->battle_reset();
    for (int seat = 0; seat <= 7; ++seat) {
        if (room->get_user_seat(seat)) {
            room->relay_battle_mgr_.OnReady(seat, true);
        }
    }
    room->start_pvp();
    room->make_state_info(reinterpret_cast<char*>(&state_packet));
    G_GameWorld()->send_all(state_packet);
}

// IDA_EA: 0x8561DEE
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary19OnConnectP2PTimeoutEP8PvP_Room
void COnlinePreliminary::OnConnectP2PTimeout(PvP_Room* room) {
    if (state_ == 4) {
        _SendMatchStatePacket(4, 2);
        OnEndFight(room);
    }
}

// IDA_EA: 0x8561E36
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary8OnGiveupEP5CUser
int COnlinePreliminary::OnGiveup(CUser*) {
    return 1;
}

// IDA_EA: 0x8561E40
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary11_ReturnTeamEP8PvP_RoomiP5CUser
COnlinePreliminary* COnlinePreliminary::_ReturnTeam(PvP_Room* room,
                                                     int retained_team,
                                                     CUser* retained_user) {
    PvP_Room* new_room = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        CUser* const user = room->get_user_seat(seat);
        if (!user) {
            continue;
        }

        if (retained_team == room->get_team(seat)) {
            if (retained_team == 2) {
                room->OnSetPVPSeatState(
                    user, seat, static_cast<ENUM_SEAT_STATE>(1),
                    static_cast<ENUM_WALKOUT_TYPE>(2));
            }
        } else if (!retained_user || user != retained_user) {
            _PullOutRoom(user, room, seat);
            if (new_room) {
                _PutInRoom(user, new_room, true);
            } else {
                new_room = _MakeRoom(user);
            }
        }
    }
    state_ = 0;
    return this;
}

// IDA_EA: 0x8561F66
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary16_GetOppositeTeamEi
int COnlinePreliminary::_GetOppositeTeam(int team) {
    if (team == 1) {
        return 2;
    }
    if (team == 2) {
        return 1;
    }
    return team;
}

// IDA_EA: 0x8561F8C
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary9OnWalkOutEP8PvP_Roomi
int COnlinePreliminary::OnWalkOut(PvP_Room* room, int seat) {
    OnLeaveRoom(room->get_user_seat(seat), room, room->get_team(seat), seat);
    return 1;
}

// IDA_EA: 0x8561FF2
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary7OnReadyEP5CUserb
bool COnlinePreliminary::OnReady(CUser*, bool ready) {
    return ready || state_ != 4;
}

// IDA_EA: 0x8562022
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary10OnEndFightEP8PvP_Room
void COnlinePreliminary::OnEndFight(PvP_Room* room) {
    const int manager_team = room->get_manager_team();
    state_ = 0;
    _ReturnTeam(room, manager_team, 0);
    if (manager_team != 1) {
        teams_[0] = teams_[1];
    }
    teams_[0].refresh();
    teams_[1].reset();
}

// IDA_EA: 0x85620C8
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary7GetUserEj
CUser* COnlinePreliminary::GetUser(uint32_t index) {
    if (index > 3 || teams_[0].players[index].occupied != 1u) {
        return 0;
    }
    return teams_[0].players[index].user;
}

// IDA_EA: 0x8562104
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary11GetUserListEPv
void COnlinePreliminary::GetUserList(void* destination) {
    std::memcpy(destination, &teams_[0], sizeof(TeamInfoSecond));
}

// IDA_EA: 0x856214A
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary21_SendMatchStatePacketEii
void COnlinePreliminary::_SendMatchStatePacket(int state, int team_count) {
    PacketGuard packet;
    packet.put_header(0, 175);
    packet.put_byte(state);
    packet.finalize(true);
    for (int team = 0; team < team_count; ++team) {
        for (int index = 0; index <= 3; ++index) {
            PlayerInfoSecond& player = teams_[team].players[index];
            if (player.occupied && player.user) {
                player.user->Send(packet);
            }
        }
    }
}

// IDA_EA: 0x8562250
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary8OnRewardEP8PvP_Roomib
void COnlinePreliminary::OnReward(PvP_Room*, int team, bool draw) {
    if (draw) {
        _UpdateRating(1, static_cast<ENUM_ELO_RATING_SYSTEM>(1));
        _UpdateRating(2, static_cast<ENUM_ELO_RATING_SYSTEM>(1));
        _IncreaseWinCount(static_cast<ENUM_SCHOOL_MATCH_TEAM>(
            _GetTeam(static_cast<ENUM_SEAT_STATE>(1))));
        _IncreaseWinCount(static_cast<ENUM_SCHOOL_MATCH_TEAM>(
            _GetTeam(static_cast<ENUM_SEAT_STATE>(2))));
    } else if (team == 1) {
        _UpdateRating(1, static_cast<ENUM_ELO_RATING_SYSTEM>(2));
        _UpdateRating(2, static_cast<ENUM_ELO_RATING_SYSTEM>(0));
        _IncreaseWinCount(static_cast<ENUM_SCHOOL_MATCH_TEAM>(
            _GetTeam(static_cast<ENUM_SEAT_STATE>(1))));
        _IncreaseLoseCount(static_cast<ENUM_SCHOOL_MATCH_TEAM>(
            _GetTeam(static_cast<ENUM_SEAT_STATE>(2))));
    } else {
        _UpdateRating(2, static_cast<ENUM_ELO_RATING_SYSTEM>(2));
        _UpdateRating(1, static_cast<ENUM_ELO_RATING_SYSTEM>(0));
        _IncreaseLoseCount(static_cast<ENUM_SCHOOL_MATCH_TEAM>(
            _GetTeam(static_cast<ENUM_SEAT_STATE>(1))));
        _IncreaseWinCount(static_cast<ENUM_SCHOOL_MATCH_TEAM>(
            _GetTeam(static_cast<ENUM_SEAT_STATE>(2))));
    }
}

// IDA_EA: 0x85623E0
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary9GetTeamIdEv
int COnlinePreliminary::GetTeamId() {
    return teams_[0].online_preliminary_team_id;
}

// IDA_EA: 0x85623EC
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary14CheckMatchableEP6IMatch
bool COnlinePreliminary::CheckMatchable(IMatch* match) {
    CUser* matching_user = 0;
    for (int index = 0; index <= 3; ++index) {
        matching_user = match->GetUser(index);
        if (matching_user) {
            break;
        }
    }
    if (!matching_user) {
        return false;
    }

    CUser* own_user = 0;
    for (int index = 0; index <= 3; ++index) {
        own_user = GetUser(index);
        if (own_user) {
            break;
        }
    }
    if (!own_user) {
        return false;
    }

    return _GetEachOtherPlayCount(own_user, matching_user) <
           G_CDataManager()->get_server_parameter_script()
               ->GetDailyMatchCount();
}

// IDA_EA: 0x8568866
// IDA_NAME: _ZN18online_preliminary18COnlinePreliminary8_GetTeamE15ENUM_SEAT_STATE
int COnlinePreliminary::_GetTeam(ENUM_SEAT_STATE state) {
    return static_cast<int>(state) - 1;
}

}  // namespace online_preliminary
