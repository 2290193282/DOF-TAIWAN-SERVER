#include "old_game/social/party/party.h"

#include "common/manager/global_data.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/dungeon/racing/racing_dungeon.h"
#include "old_game/dungeon/escort/golden_commerce.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/match/channel_zombie_matching_authority.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x85B15E0
// IDA_NAME: _ZN6CParty14finish_loadingEP5CUser
// DEMANGLED: CParty::finish_loading(CUser *)
int CParty::finish_loading(CUser* user) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (members_[seat].user != user) {
            continue;
        }
        if (battle_data_.member_finish_loading(seat)) {
            return battle_data_.member_finish_loading(seat);
        }
        battle_data_.member_finish_loading(seat) = 1;
        if (battle_data_.member_start_map_finish_loading(seat) != 1u &&
            OnStartMapFinishLoading(user)) {
            battle_data_.member_start_map_finish_loading(seat) = 1;
        }
    }

    CDungeon* const dungeon = battle_field_.get_dungeon();
    if (!dungeon || reinterpret_cast<const CDungeon*>(dungeon)->IsEnterEachMap() != 1 ||
        battle_field_.IsFirstEnterDungeon()) {
        check_allmember_loading(false);
    } else {
        check_allmember_loading(true);
    }
    return 0;
}

// IDA_EA: 0x85B170A
// IDA_NAME: _ZN6CParty23OnStartMapFinishLoadingEP5CUser
// DEMANGLED: CParty::OnStartMapFinishLoading(CUser *)
int CParty::OnStartMapFinishLoading(CUser* user) {
    CDungeon* const dungeon = battle_field_.get_dungeon();
    if (dungeon && do_after_dungeon_start(dungeon, user)) {
        giveup_game(user, false, false, false);
        return 0;
    }
    if (dungeon &&
        (dungeon->get_dimension_possible() > 0 ||
         dungeon->get_blood_dungeon_type() != 0 ||
         dungeon_runtime::IsLimitedInCountDungeon(dungeon->get_index()) ||
         dungeon->isTowerOfDespairDungeon() ||
         dungeon->isRevengeDungeon())) {
        // The successful start transaction owns the count decrement.  The
        // 0828 client receives the updated values through USERINFO subtype 1.
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             ENUM_NOTIPACKET_USERINFO,
                             1);
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_FINISH_LOADING);
    return 1;
}

// IDA_EA: 0x85B1E66
// IDA_NAME: _ZN6CParty22map_load_forced_finishEv
// DEMANGLED: CParty::map_load_forced_finish(void)
void CParty::map_load_forced_finish() {
    if (zombie_map_runtime::ConsumeSeatLoadTimeout(this)) {
        return;
    }
    if (channel_zombie_matching_authority::
            ConsumeMapOwnerLoadTimeout(this)) {
        return;
    }
    if (channel_zombie_matching_authority::
            AbortMapOwnerBeforeLifecycle(this, getManager())) {
        return;
    }
    // The timeout only owns a not-yet-started load transition.  Once the
    // loading check was emitted, the normal completion path owns the party.
    if (loading_check_pending_) {
        return;
    }

    if (get_member_count() <= 0) {
        // The original emits a trace before this destruction.  The state
        // transition itself is the important behavior: return an empty party
        // to its pool instead of leaving a timer-owned orphan behind.
        destroy();
        return;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) == 1 &&
            battle_data_.member_finish_loading(seat) != 1) {
            giveup_game(members_[seat].user, false, false, false);
        }
    }

    if (get_member_count() > 0) {
        check_allmember_loading(true);
    }
}

// IDA_EA: 0x85B17A4
// IDA_NAME: _ZN6CParty23check_allmember_loadingEb
// DEMANGLED: CParty::check_allmember_loading(bool)
void CParty::check_allmember_loading(bool enter_each_map) {
    const int member_count = get_member_count();
    if (member_count <= 0) {
        destroy();
        return;
    }

    int loaded_member_count = 0;
    for (int seat = 0; seat <= 3; ++seat) {
        if (checkValidUser(seat) == 1 &&
            battle_data_.member_finish_loading(seat)) {
            ++loaded_member_count;
        }
    }
    if (loaded_member_count != member_count &&
        (!enter_each_map || loaded_member_count <= 0)) {
        return;
    }

    battle_field_.ResetFirstEnterDungeon();
    // Client NOTI476 only appends a deferred map-move record.  Its sole
    // materialization pass runs from NOTI30, so the cart must be queued on
    // the same ordered connection immediately before FINISH_LOADING.
    (void)golden_commerce::SendCartBeforeFinishLoading(this);
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_FINISH_LOADING);
    packet.put_int(battle_field_.village_monster_route());
    packet.put_byte(0);
    packet.finalize(true);
    send_to_party(packet);

    const uint32_t current_tick = static_cast<uint32_t>(
        GlobalData::s_systemTime_.getCurTickCount());
    const uint64_t prior_play_time =
        static_cast<uint64_t>(battle_data_.map_tick_214()) |
        (static_cast<uint64_t>(battle_data_.map_tick_215()) << 32u);
    const uint64_t start_tick =
        static_cast<uint64_t>(battle_data_.map_tick_216()) |
        (static_cast<uint64_t>(battle_data_.map_tick_217()) << 32u);
    const uint64_t revised_play_time = prior_play_time + current_tick - start_tick;
    battle_data_.map_tick_214() = static_cast<uint32_t>(revised_play_time);
    battle_data_.map_tick_215() =
        static_cast<uint32_t>(revised_play_time >> 32u);
    battle_data_.map_pause_time() =
        current_tick - battle_data_.map_start_time();

    const bool special_dungeon = IsAutoCreated() != 1 && member_count != 1;
    trace_mob_die_hack_.traceBegin(GlobalData::s_systemTime_.getCurSec(),
                                   special_dungeon);
    for (int seat = 0; seat <= 3; ++seat) {
        if (checkValidUser(seat) != 1) {
            continue;
        }
        CUser* const member = members_[seat].user;
        member->GetPCRoomNo();
        member->getHackAnalyzer()->beginCollectHackInfo(member);
    }

    loading_check_pending_ = 1;
    racing_dungeon::OnAllMembersLoaded(this);
    battle_field_.onStartMap();
    (void)golden_commerce::OnMapLoaded(this);

    // IDA 0x85B1C0D..0x85B1C81: only dungeon 53 consumes the reset-field
    // one-shot flag, resets the elevator sequence, and queues message 40.
    // Every follow-up callback validates this exact generated timer key.
    if (battle_field_.dungeon_index_ == 53 &&
        battle_field_.elevator_timer_armed_) {
        battle_field_.elevator_timer_count_ = 0;
        battle_field_.elevator_timer_state_ = 2;
        battle_field_.elevator_timer_armed_ = 0;
        const int timer_key = gen_timer_key(static_cast<TIMER_MESSAGE>(40));
        G_TimerQueue()->InsertTimer(
            1,
            party_index_,
            static_cast<TIMER_MESSAGE>(40),
            15,
            static_cast<unsigned int>(timer_key),
            0);
    }
}
