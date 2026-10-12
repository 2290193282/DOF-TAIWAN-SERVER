#include "old_game/social/party/party.h"

#include <cstring>
#include <map>
#include <vector>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

// B-owned CDungeon ABI imports.  The timer chain only reads these two script
// flags, so it must not duplicate the data-manager layout in the group owner.
// IDA_EA: 0x85AA5D4
// IDA_NAME: _ZN6CParty16OnTimeoutBossDieEij
// DEMANGLED: CParty::OnTimeoutBossDie(int,unsigned int)
int CParty::OnTimeoutBossDie(int timer_key, unsigned int charac_no) {
    if (battle_field_.boss_die_check_started_at_sec() != timer_key) {
        return 0;
    }

    // RECONSTRUCTED_0828: CMD117 arms this cleanup one second after the
    // client exhausts its boss-die map order.  An ordinary clear can finish
    // during that delay and NOTI31 then destroys the client's monster
    // objects.  Do not let the stale timer award EXP/history or broadcast a
    // late NOTI38 for an assigned-team object after that clear boundary.
    if (tournament_victory_ != 0) {
        LogManager::logFormat(
            1,
            "dungeon42_diag.cpp",
            "bool CParty::OnTimeoutBossDie(int, memberIdentificationNumber_t)",
            0,
            "[DUNGEON42][BOSS_DIE_TIMEOUT_POST_CLEAR_SKIP] party_index=%d charac_no=%u timer_key=%d",
            GetPartyIndex(),
            charac_no,
            timer_key);
        return 1;
    }

    CUser* const user = GetMemberSlotNo(charac_no);
    if (!user) {
        return 0;
    }

    CDungeon* const dungeon = battle_field_.GetDungeon();
    if (dungeon && reinterpret_cast<const CDungeon*>(dungeon)->IsEnterEachMap() != 0) {
        battle_field_.SetCurSeatNo(get_party_seatno(user));
    }

    // kill_monster can erase the map entry.  The original first snapshots
    // every script-backed target ID, then re-finds each entry to obtain HP.
    std::vector<int> monster_unique_ids;
    MapInfo* const initial_map = battle_field_.GetCurrentMapInfo();
    for (std::map<int, map_monster>::const_iterator it =
             initial_map->monsters_.begin();
         it != initial_map->monsters_.end();
         ++it) {
        if (it->second.assigned_team_ != 0) {
            monster_unique_ids.push_back(it->second.map_monster_unique_id_);
        }
    }

    for (std::vector<int>::const_iterator it = monster_unique_ids.begin();
         it != monster_unique_ids.end();
         ++it) {
        MSG_MONSTER_DIE message;
        std::memset(&message, 0, sizeof(message));

        MapInfo* const current_map = battle_field_.GetCurrentMapInfo();
        // getHpMax() is the original non-const ActiveStaticInfo accessor;
        // IDA's loop therefore uses the mutable map iterator returned by the
        // non-const current MapInfo.
        std::map<int, map_monster>::iterator monster =
            current_map->monsters_.find(*it);
        if (monster != current_map->monsters_.end()) {
            message.field_a1a_ = static_cast<uint32_t>(
                monster->second.active_static_info_.getHpMax());
        }

        if (kill_monster(user, *it, 0xffffu, message, 0) == 1) {
            continue;
        }

        cMyTrace trace(
            "bool CParty::OnTimeoutBossDie(int, memberIdentificationNumber_t)",
            8910,
            0);
        trace("KILLBOSS2 server - monster id(%d) failed", *it);

        if (battle_field_.instance_dungeon_mode() == 1) {
            SetEPLPState(2);
            const unsigned int eplp_timer_key = static_cast<unsigned int>(
                gen_timer_key(TIMER_MESSAGE_EPLP_COMMAND_WAIT));
            G_TimerQueue()->InsertTimer(1,
                                        GetPartyIndex(),
                                        TIMER_MESSAGE_EPLP_COMMAND_WAIT,
                                        60,
                                        eplp_timer_key,
                                        0);
            set_state(1);

            PacketGuard instance_packet;
            instance_packet.put_header(0, 254);
            instance_packet.finalize(true);
            send_to_party(instance_packet);
            return 1;
        }

        if (static_cast<signed char>(
                reinterpret_cast<const CDungeon*>(dungeon)->get_blood_dungeon_type()) <= 0) {
            PacketGuard clear_packet;
            clear_packet.put_header(0, 31);
            clear_packet.finalize(true);
            send_to_party(clear_packet);
            return 1;
        }

        battle_field_.blood_clear_reward().onFinishBloodRound(
            true,
            static_cast<short>(
                battle_field_.blood_round().getBloodRoundOrder()),
            this,
            dungeon);
        const unsigned int blood_timer_key = static_cast<unsigned int>(
            gen_timer_key(TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_0));
        G_TimerQueue()->InsertTimer(
            1,
            GetPartyIndex(),
            TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_0,
            5,
            blood_timer_key,
            TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_0);
        return 1;
    }

    return 1;
}
