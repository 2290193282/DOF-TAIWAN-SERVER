#include "old_game/social/party/party.h"

#include <cstring>
#include <stdint.h>

#include "old_game/player/core/cuser.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/game_manager.h"
#include "common/manager/history_log_abi.h"
#include "common/manager/statistics_abi.h"
#include "common/manager/user.h"
#include "common/manager/user_history_log_abi.h"
#include "old_game/economy/inventory/special_item_routing_manager_abi.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/match/channel_zombie_matching_authority.h"
#include "old_game/dungeon/instance/dungeon_entrance_log.h"
#include "old_game/dungeon/raid/dual_raid.h"
#include "old_game/dungeon/instance/time_spiral_runtime.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/dungeon/escort/golden_commerce.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

namespace {
bool ShouldApplyGiveupPenalty(bool dimension_dungeon,
                              bool special_dungeon,
                              int eplp_state,
                              bool requested) {
    return !dimension_dungeon && !special_dungeon && eplp_state == 1 &&
           requested;
}

}  // namespace

// IDA_EA: 0x85B2BAA
// IDA_NAME: _ZN6CParty11giveup_gameEP5CUserbbb
// DEMANGLED: CParty::giveup_game(CUser *,bool,bool,bool)
void CParty::giveup_game(CUser* user,
                         bool apply_giveup_penalty,
                         bool clear_dungeon,
                         bool force_return) {
    if (channel_zombie_matching_authority::
            AbortMapOwnerBeforeLifecycle(this, user)) {
        return;
    }
    champion_break::AbortDungeonRun(this);
    time_spiral::ResetParty(this);
    const CDungeon* const dungeon = battle_field_.dungeon_;
    bool special_dungeon = false;
    if (dungeon) {
        special_dungeon = dungeon->blood_dungeon_type_ > 0;
        if (static_cast<unsigned char>(dungeon->isTowerOfDespairDungeon()) !=
            0u) {
            special_dungeon = true;
        }
        if (static_cast<signed char>(
                dungeon->get_dimension_possible()) > 0 ||
            dungeon->ancient_dungeon_ ||
            static_cast<unsigned char>(dungeon->isTournamentDungeon()) !=
                0u ||
            dungeon->event_dungeon_ ||
            dungeon->village_attack_revenge_dungeon_) {
            apply_giveup_penalty = false;
        }
    }

    const bool dimension_dungeon =
        battle_field_.instance_dungeon_mode_ == 1;
    if (!clear_dungeon) {
        golden_commerce::OnPartyDungeonResult(
            this, battle_field_.get_dungeon_index(), false);
        dual_raid::OnPartyDungeonResult(
            this, battle_field_.get_dungeon_index(), false);
    }
    user->reset_level_before_dungeon();
    battle_field_.dungeon_clear_tracer_.Trace(
        "void CParty::giveup_game(CUser*, bool, bool, bool)");
    Reset_party_overlapped_drop_ratio();

    if (user && user->getCurCharacR() && !dimension_dungeon) {
        const _Charac_info* const character = user->getCurCharacR();
        Statistics::DungeonFail(character->fields_.level_);
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) == 1u) {
            HistoryLog::WriteGiveUp(members_[seat].user->history_stream(),
                                    user->getCurCharacName());
        }
    }

    if (battle_field_.village_monster_uid_ ||
        (dungeon && dungeon->get_index() == 10000)) {
        apply_giveup_penalty = false;
    }

    if (clear_dungeon) {
        OnSuccessedDungeonClear(user);
    } else {
        if (this->get_member_count() == 1) {
            CheckHackAverageHitCount();
            secu_.dungeonFail();
        }
        OnFailedDungeonClear(user);
    }

    if (user && static_cast<unsigned char>(
                    battle_field_.check_start_point()) != 0u) {
        reinterpret_cast<WongWork::CHackAnalyzer*>((user->getHackAnalyzer()))->addServerHackCnt(
                                                user,
                                                static_cast<WongWork::ENUM_HACKTYPE>(0x325),
                                                1,
                                                0,
                                                0);
    }

    SaveCoin(static_cast<eCoinSubReason>(0));
    const int eplp_state = GetEPLPState();
    if (eplp_state == 1) {
        CUser* const manager = this->getManager();
        if (manager->getMoveSpace() != 1) {
            GetInstanceDungeonEntranceLog()->DecrementDungeonEntrance(
                dungeon->get_index(), false);
        }
    }

    PacketGuard packet;
    if (auto_created_) {
        leave_user(user, static_cast<ENUM_PARTY_INFO_TYPE>(2));
        G_GameWorld()->out_from_dungeon(user);
        if (ShouldApplyGiveupPenalty(
                dimension_dungeon, special_dungeon, eplp_state, apply_giveup_penalty)) {
            user->giveup_panalty();
        }
        packet.clear();
        packet.put_header(0, 2);
        packet.put_byte(0);
        packet.put_short(1);
        user->make_basic_info(packet, 0);
        packet.finalize(true);
        if (this->getManager()->getMoveSpace() == 1) {
            this->send_to_party(packet);
        } else {
            G_GameWorld()->send_all(packet, user);
        }
        return;
    }

    if (!clear_dungeon) {
        set_charac_live(user, static_cast<ENUM_USER_DUNGEON_STATE>(0));
    }
    char member_names[256];
    std::memset(member_names, 0, sizeof(member_names));
    _getMemberNames(member_names);

    if ((static_cast<unsigned char>(GetSelectedEPLPCmd()) != 3u &&
         static_cast<unsigned char>(check_allmember_die()) != 0u) ||
        clear_dungeon || force_return) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(this->_checkValidUser(seat)) != 1u) {
                continue;
            }
            CUser* const member = members_[seat].user;
            if (dungeon) {
                CUser_history_log(member)->LeaveDungeon(
                    dungeon->GetDungeonName(),
                    battle_field_.get_dungeon_diff(),
                    _getMemberNames(member_names),
                    battle_field_.instance_dungeon_mode_);
            }
            if (battle_field_.village_monster_uid_ &&
                member != user) {
                if (clear_dungeon) {
                    OnSuccessedDungeonClear(member);
                } else {
                    OnFailedDungeonClear(member);
                }
            }
            G_GameWorld()->out_from_dungeon(member);
            if (ShouldApplyGiveupPenalty(dimension_dungeon,
                                         special_dungeon,
                                         eplp_state,
                                         apply_giveup_penalty)) {
                member->giveup_panalty();
            }
        }

        packet.clear();
        packet.put_header(0, 2);
        packet.put_byte(0);
        packet.put_short(static_cast<short>(this->get_member_count()));
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(this->_checkValidUser(seat)) == 1u) {
                members_[seat].user->make_basic_info(packet, 0);
            }
        }
        packet.finalize(true);
        if (static_cast<unsigned char>(this->IsExistInvisible()) != 0u) {
            this->send_to_party(packet);
        } else {
            G_GameWorld()->send_all(packet);
        }
        this->set_state(1);
        battle_data_reset();
    } else {
        const int leave_result =
            leave_user(user, static_cast<ENUM_PARTY_INFO_TYPE>(2));
        if (leave_result != 1) {
            G_GameWorld()->out_from_dungeon(user);
            if (ShouldApplyGiveupPenalty(dimension_dungeon,
                                         special_dungeon,
                                         eplp_state,
                                         apply_giveup_penalty)) {
                user->giveup_panalty();
            }
            packet.clear();
            packet.put_header(0, 2);
            packet.put_byte(0);
            packet.put_short(1);
            user->make_basic_info(packet, 0);
            packet.finalize(true);
            G_GameWorld()->send_all(packet, user);
        }
        sendInoutConditionDungeon(static_cast<RetryFailReason::T>(
            checkInoutConditionDungeon()));
    }

    CSpecialItemRoutingManager* const special_item_routing =
        G_CGameManager()->GetSpecialItemRoutingManager();
    if (special_item_routing) {
        special_item_routing->ProcessGiveUp(this);
    }
}
