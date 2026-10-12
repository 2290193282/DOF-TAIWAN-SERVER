#include "old_game/social/party/party.h"

#include <stdint.h>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/dungeon/instance/dungeon_entrance_log.h"
#include "old_game/activity/schedule/blue_marble_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x85B3670
// IDA_NAME: _ZN6CParty18party_dungeon_failEv
// DEMANGLED: CParty::party_dungeon_fail(void)
void CParty::party_dungeon_fail() {
    CDungeon* const dungeon = battle_field_.get_dungeon();
    if (tournament_victory_ ||
        static_cast<unsigned char>(check_allmember_die()) != 1u ||
        !dungeon) {
        return;
    }

    bool no_giveup_penalty = dungeon->get_blood_dungeon_type() > 0;
    if (dungeon->isTowerOfDespairDungeon()) {
        no_giveup_penalty = true;
    }
    if (battle_field_.village_monster_timer_token()) {
        no_giveup_penalty = true;
    }
    if (static_cast<signed char>(dungeon->get_dimension_possible()) > 0 ||
        dungeon->ancient_dungeon_) {
        no_giveup_penalty = true;
    }

    if (dungeon->isTournamentDungeon()) {
        battle_field_.tournament_reward().ResetReward();
        MakeTournamentReward(battle_field_.tournament_reward());
        return;
    }

    battle_data_.map_playing_time() =
        static_cast<uint32_t>(GlobalData::system_time()->getCurTickCount()) -
        battle_data_.map_start_time() - battle_data_.map_pause_time();
    SaveCoin(static_cast<eCoinSubReason>(0));
    CheckHackAverageHitCount();
    secu_.dungeonFail();

    PacketGuard packet;
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
            continue;
        }

        CUser* const user = members_[seat].user;
        OnFailedDungeonClear(user);
        G_GameWorld()->out_from_dungeon(user);

        if (user->getMoveSpace() != 1) {
            GetInstanceDungeonEntranceLog()->DecrementDungeonEntrance(
                dungeon->get_index(), false);
        }
        if (user->getMoveSpace() == 1) {
            BlueMarble* const blue_marble = user->getBlueMarble();
            if (blue_marble) {
                blue_marble->giveupDungeon(user);
            }
        }

        if (battle_field_.instance_dungeon_mode() != 1) {
            const bool event_dungeon = dungeon->event_dungeon_ != 0;
            const bool no_penalty_script =
                dungeon->village_attack_revenge_dungeon_ != 0;
            if (!no_giveup_penalty && !event_dungeon && !no_penalty_script) {
                user->giveup_panalty();
            }
        }
    }

    packet.clear();
    packet.put_header(0, 2);
    packet.put_byte(0);
    packet.put_short(get_member_count());

    if (auto_created_) {
        cMyTrace trace("void CParty::party_dungeon_fail()", 13633, 0);
        trace("<IN_DUNGEON_NULL_PARTY> m_id(%s), user_state(%d), PartyIndex(%d)",
              NumberToString(static_cast<unsigned int>(manager_->get_acc_id()), 0),
              manager_->get_state(),
              manager_->GetPartyIndex());
        manager_->SetPartyIndex(-1);
        if (getManager()->getMoveSpace() != 1) {
            manager_->set_state(3);
        }
        manager_->make_basic_info(packet, 0);
        destroy();
        packet.finalize(true);
        if (getManager()->getMoveSpace() != 1) {
            G_GameWorld()->send_all(packet, manager_);
            return;
        }
    } else {
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(_checkValidUser(seat)) == 1u) {
                members_[seat].user->make_basic_info(packet, 0);
            }
        }
        set_state(1);
        battle_data_reset();
        packet.finalize(true);
        if (!IsExistInvisible()) {
            G_GameWorld()->send_all(packet);
            return;
        }
    }

    send_to_party(packet);
}
