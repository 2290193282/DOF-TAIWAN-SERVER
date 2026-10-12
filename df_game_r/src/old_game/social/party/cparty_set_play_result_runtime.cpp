#include "old_game/social/party/party.h"

#include "common/manager/global_data.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/dungeon/progress/best_clear_time_runtime.h"
#include "old_game/dungeon/progress/dragon_timeattack_rank_runtime.h"
#include "old_game/dungeon/raid/dual_raid.h"
#include "old_game/dungeon/escort/golden_commerce.h"
#include "old_game/social/party/packet_dispatcher_set_play_result.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

bool IsZombieResultOwner(CParty* party, CUser* user) {
    if (!party) return false;
    CDungeon* const dungeon = party->battle_field().get_dungeon();
    if (dungeon && dungeon->isZombieDungeon()) return true;
    PvP_Room* const room = user ? user->GetPVPRoom() : 0;
    return room && (zombie_mode::IsActiveRoom(room) ||
                    zombie_mode::IsFinishedRoom(room));
}

}  // namespace

// IDA_EA: 0x85B2068
// IDA_NAME: _ZN6CParty15set_play_resultEP5CUserR13GameResultSet
// DEMANGLED: CParty::set_play_result(CUser *,GameResultSet &)
int CParty::set_play_result(CUser* manager, GameResultSet& result) {
    (void)result;
    if (IsZombieResultOwner(this, manager)) {
        return 0;
    }
    if (host_ != manager || recv_result_flag_ != 0u) {
        return 0;
    }
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(_checkValidUser(seat)) == 1u) {
            get_user(seat)->setPlayingDungeonIndex(-1);
        }
    }
    state_ = 3;
    recv_result_flag_ = 1;
    return 0;
}

// IDA_EA: 0x85B21C8 is adjacent to the other card-state timer insertors;
// CMD049 alone owns message 41 rather than reusing message 13.
int CParty::insert_time_set_play_result_msg(int delay_sec) {
    const unsigned int timer_key = static_cast<unsigned int>(
        gen_timer_key(TIMER_MESSAGE_SET_PLAY_RESULT));
    return G_TimerQueue()->InsertTimer(1,
                                       party_index_,
                                       TIMER_MESSAGE_SET_PLAY_RESULT,
                                       delay_sec,
                                       timer_key,
                                       0);
}

// IDA_EA: 0x85BE178
// IDA_NAME: _ZN6CParty16setBestClearTimeEiii
bool CParty::setBestClearTime(int dungeon_index,
                              int dungeon_difficulty,
                              int clear_time) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(checkValidUser(seat)) != 1u) {
            continue;
        }
        BestClearTime* const best_time = static_cast<BestClearTime*>(
            get_user(seat)->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(18)));
        if (best_time) {
            best_time->setBestClearTime(dungeon_index,
                                        dungeon_difficulty,
                                        clear_time);
        }
    }
    return true;
}

// IDA_EA: 0x85BE1F8
// IDA_NAME: _ZN6CParty23makeBestClearTimePacketER11PacketGuardi
bool CParty::makeBestClearTimePacket(PacketGuard& packet, int clear_time) {
    packet.put_byte(battle_field_.isQuestMaze() != 0 ? 1 : 0);
    packet.put_byte(get_member_count());
    const int dungeon_index = battle_field_.get_dungeon_index();
    const int dungeon_difficulty = battle_field_.get_dungeon_diff();

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(checkValidUser(seat)) != 1u) {
            continue;
        }
        CUser* const user = get_user(seat);
        BestClearTime* const best_time = static_cast<BestClearTime*>(
            user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(18)));
        int record_time = best_time
                              ? best_time->getBestClearTime(dungeon_index,
                                                            dungeon_difficulty)
                              : 0;
        const bool is_new_record = best_time && best_time->checkNewRecord();
        if (record_time == 0) {
            record_time = clear_time;
        }
        packet.put_short(user->get_unique_id());
        packet.put_int(record_time);
        packet.put_byte(is_new_record ? 1 : 0);
    }
    return true;
}

// IDA_EA: 0x85B2412
// IDA_NAME: _ZN6CParty13SetPlayResultEP5CUser
// DEMANGLED: CParty::SetPlayResult(CUser *)
int CParty::SetPlayResult(CUser* manager) {
    if (IsZombieResultOwner(this, manager)) {
        return 1;
    }
    if (get_dungeon_clear_state() != 1) {
        return 1;
    }

    const int dungeon_index = battle_field_.get_dungeon_index();
    GameResultSet& result = game_result_set_;
    set_play_result(manager, result);

    if (golden_commerce::OnPartyDungeonResult(
            this, dungeon_index, true)) {
        return 1;
    }
    if (dual_raid::OnPartyDungeonResult(
            this, dungeon_index, true)) {
        return 1;
    }

    // ClearDungeon has already frozen the whole-dungeon timer in
    // BattleData+0.  GetMapPlayingTime() owns only the current room and is
    // reset on every map transition.
    const int clear_time =
        static_cast<int>(battle_data_.dungeon_playing_time());
    const int dungeon_difficulty = battle_field_.get_dungeon_diff();
    const int average_clear_time = G_CDataManager()->getDungeonClearAverageTime(
        dungeon_index, getStartGamePartyCount(), dungeon_difficulty);
    const int clear_time_bonus = G_CDataManager()->getClearTimeBonusPoint(
        average_clear_time, clear_time);
    const int rank_point = G_CDataManager()->calRankPoint(
        clear_time_bonus, result.raw_09);

    result.raw_48 = static_cast<uint16_t>(
        G_CDataManager()->getRankIndex(rank_point));
    result.raw_0c = clear_time;
    result.raw_10 = static_cast<uint8_t>(clear_time_bonus);
    if (battle_field_.isQuestMaze() != 1) {
        setBestClearTime(dungeon_index, dungeon_difficulty, clear_time);
        dragon_timeattack_rank::RecordPartyClear(
            this, dungeon_index, dungeon_difficulty, clear_time);
    }
    if (DisPatcher_SetPlayResult::SendResult(this, result) != 1) {
        return 0;
    }

    // This is intentionally the real reward-preparation handoff.  It must
    // precede the state-2/card timers because dungeon_clear() consumes these
    // generated pairs after card selection.
    clear_reward(result.raw_48);
    set_dungeon_clear_state(2);
    insert_time_clear_dungeon_reward_msg(30);
    return 1;
}

// IDA_EA: 0x85B26E8
// IDA_NAME: _ZN6CParty22forced_set_play_resultEv
// DEMANGLED: CParty::forced_set_play_result(void)
void CParty::forced_set_play_result() {
    if (IsZombieResultOwner(this, getManager())) {
        return;
    }
    if (state_ == 3) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_PLAY_RESULT);
    packet.put_byte(0);
    packet.put_int(0);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.finalize(true);
    send_to_party(packet);
}
