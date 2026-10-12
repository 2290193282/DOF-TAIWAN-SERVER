#include "old_game/pvp/arena/pvp_room.h"

#include "infra/log/format/trace.h"
#include "old_game/progression/quest/character_mission_list.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"

// IDA_EA: 0x85D9386
// IDA_NAME: _ZN8PvP_Room10die_characEP5CUserS1_
// DEMANGLED: PvP_Room::die_charac(CUser *,CUser *)
int PvP_Room::die_charac(CUser* dead_user, CUser* killer_user) {
    const int battle_mode = get_pvp_battle_mode();
    const int room_index = get_index();
    CSwitchLog trace("int PvP_Room::die_charac(CUser*, CUser*)", 2465, 0, 0);
    trace("pvp@log %s,%s,%d,%d",
          dead_user->get_acc_name(),
          dead_user->getCurCharacName(),
          room_index,
          battle_mode);

    if (battle_state_ != 2) {
        return 19;
    }

    // Zombie Mode is a state-5 dungeon/map owner with an eight-seat PvP_Room
    // combat sidecar.  Resolve its authoritative role/life transaction before
    // NOTI46 so an invalid client-reported killer can never trigger a visual
    // infection that the server later rejects.
    if (zombie_mode::IsActiveRoom(this)) {
        if (!dead_user || !killer_user || dead_user == killer_user)
            return 19;

        int dead_seat = -1;
        int killer_seat = -1;
        lock();
        dead_seat = get_user_seat(dead_user);
        killer_seat = get_user_seat(killer_user);
        if (dead_seat < 0 || killer_seat < 0) {
            unlock();
            return 19;
        }
        if (!battle_live_flags_[dead_seat]) {
            unlock();
            return 18;
        }
        if (!battle_live_flags_[killer_seat]) {
            unlock();
            return 19;
        }
        unlock();

        zombie_mode::ZombieDeathResolution resolution;
        const int resolved = zombie_mode::ResolveCharacterDeath(
            this, dead_user, killer_user, resolution);
        if (resolved != 0 ||
            resolution.dead_seat != static_cast<uint16_t>(dead_seat) ||
            resolution.killer_seat != static_cast<uint16_t>(killer_seat)) {
            return resolved != 0 ? resolved : 19;
        }

        lock();
        battle_live_flags_[dead_seat] =
            resolution.action ==
                    zombie_mode::ZOMBIE_DEATH_HUMAN_INFECTED
                ? 1u
                : 0u;
        unlock();

        PacketGuard zombie_packet;
        zombie_packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                                 ENUM_NOTIPACKET_DIE_PVP_CHARACTER);
        zombie_packet.put_byte(dead_seat);
        zombie_packet.put_byte(killer_seat);
        zombie_packet.finalize(true);
        send_to_pvp(zombie_packet);

        if (!resolution.terminal) return 0;
        if (!zombie_mode::FinishRound(this,
                                      resolution.outcome,
                                      resolution.last_survivor)) {
            return 19;
        }
        pvp_request_pvp_rank();
        return -1;
    }

    lock();
    const int dead_seat = get_user_seat(dead_user);
    if (dead_seat < 0) {
        unlock();
        return -1;
    }
    if (!battle_live_flags_[dead_seat]) {
        unlock();
        return 18;
    }
    battle_live_flags_[dead_seat] = 0;
    unlock();

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_DIE_PVP_CHARACTER);
    packet.put_byte(dead_seat);
    packet.put_byte(killer_user ? get_user_seat(killer_user) : -1);
    packet.finalize(true);
    send_to_pvp(packet);

    if (battle_mode_ == static_cast<PVP_BATTLE_MODE>(4) ||
        battle_mode_ == static_cast<PVP_BATTLE_MODE>(5)) {
        const int killer_seat = get_user_seat(killer_user);
        death_match_battle_mgr_.IncreaseKillCount(killer_seat);
        death_match_battle_mgr_.IncreaseDeathCount(dead_seat);
        death_match_battle_mgr_.InsertTimerKilledUserForRevive(
            get_index(), dead_seat);
    }

    if (killer_user) {
        CMissionList_Charac* const missions =
            static_cast<CMissionList_Charac*>(
                killer_user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
        MissionClearCondition_Parameter condition(battle_mode_);
        missions->Update_Kill_event(*killer_user, condition);
    }

    const int killer_seat = get_user_seat(killer_user);
    if (check_winner()) {
        relay_battle_mgr_.OnDiePlayer(dead_seat, killer_seat, false);
        pvp_request_pvp_rank();
        return -1;
    }

    relay_battle_mgr_.OnDiePlayer(
        dead_seat, killer_seat, battle_mode_ == PVP_BATTLE_MODE_3);
    return 0;
}
