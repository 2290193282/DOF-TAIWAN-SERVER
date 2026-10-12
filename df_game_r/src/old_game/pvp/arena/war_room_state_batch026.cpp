#include "old_game/pvp/battlefield/war_room.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "core/config/env/environment.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/user_history_log_abi.h"
#include "old_game/pvp/battlefield/war_area_counter.h"
#include "old_data/channel/channel_script_t.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <algorithm>

namespace {

const int kFirstWarRoomTimerMessage = 63;

}  // namespace

// IDA_EA: 0x86BA834
// IDA_NAME: _ZN7WarRoom8SetIndexEi
// DEMANGLED: WarRoom::SetIndex(int)
int WarRoom::SetIndex(int index) {
    room_index_ = index;
    return Reset();
}

// IDA_EA: 0x86BAA14
// IDA_NAME: _ZN7WarRoom5ResetEv
// DEMANGLED: WarRoom::Reset(void)
bool WarRoom::Reset() {
    BattleReset();

    room_flag_008_ = 0;
    room_mode_ = 2;
    room_level_ = 0;
    shutdown_pending_ = 0;
    for (int slot = 0; slot <= 5; ++slot) {
        war_field_.room_users_[slot] = 0;
        war_field_.seat_states_[slot] = 255;
        war_field_.walkout_types_[slot] =
            static_cast<ENUM_WALKOUT_TYPE>(0);
    }

    // The original loop returns false after testing slot 6.
    return false;
}

// IDA_EA: 0x86BAA8E
// IDA_NAME: _ZN7WarRoom11BattleResetEv
// DEMANGLED: WarRoom::BattleReset(void)
int WarRoom::BattleReset() {
    for (int slot = 0; slot <= 5; ++slot) {
        CUser* const user = war_field_.room_users_[slot];
        if (user) {
            WongWork::CHackAnalyzer* const analyzer = user->getHackAnalyzer();
            analyzer->reportHackInfo();
            user->getHackAnalyzer()->resetHackInfo();
            user->ResetCoinCount();
        }

        war_field_.live_flags_[slot] = 0;
        war_field_.walkout_types_[slot] =
            static_cast<ENUM_WALKOUT_TYPE>(0);
        war_field_.guild_finish_points_[slot] = 0;
        host_change_values_[slot] = UINT32_MAX;
    }

    war_field_.Reset();
    for (int index = 0; index < 4; ++index)
        war_field_.clear_reward_multipliers_[index] = 0.0f;
    host_change_request_count_ = 0;
    result_phase_active_ = 0;
    result_phase_sent_ = 0;
    battle_finished_ = 0;
    CurSpawnStepReset();
    return static_cast<int>(reinterpret_cast<uintptr_t>(this));
}

// IDA_EA: 0x86BE152
// IDA_NAME: _ZN7WarRoom11SendWalkOutEP5CUser17ENUM_WALKOUT_TYPE
// DEMANGLED: WarRoom::SendWalkOut(CUser *,ENUM_WALKOUT_TYPE)
int WarRoom::SendWalkOut(CUser* user, ENUM_WALKOUT_TYPE type) {
    PacketGuard packet;
    packet.put_header(0, 10);
    packet.put_byte(0);
    packet.put_byte(static_cast<int>(type));
    packet.finalize(true);
    user->Send(packet);
    return 1;
}

// IDA_EA: 0x86BABAE
// IDA_NAME: _ZNK7WarRoom13GetRemainSeatEv
// DEMANGLED: WarRoom::GetRemainSeat(void)const
int WarRoom::GetRemainSeat() const {
    return 6 - GetWaiterCount();
}

// IDA_EA: 0x86BAC0C
// IDA_NAME: _ZNK7WarRoom12GetLiveCountEv
// DEMANGLED: WarRoom::GetLiveCount(void)const
int WarRoom::GetLiveCount() const {
    int count = 0;
    for (int slot = 0; slot < 6; ++slot)
        if (war_field_.room_users_[slot] && war_field_.live_flags_[slot])
            ++count;
    return count;
}

// IDA_EA: 0x86BAC5E
// IDA_NAME: _ZN7WarRoom6CreateEP5CUserPcs
// DEMANGLED: WarRoom::Create(CUser *,char *,short)
int WarRoom::Create(CUser* manager, char*, short) {
    war_field_.room_state_ = 0;
    war_field_.room_users_[0] = manager;
    war_field_.seat_states_[0] = 1;
    manager->SetWarRoomIndex(static_cast<short>(room_index_));
    return 0;
}

// IDA_EA: 0x86BB53C
// IDA_NAME: _ZN7WarRoom11ChangeStateE13WARROOM_STATE
// DEMANGLED: WarRoom::ChangeState(WARROOM_STATE)
int WarRoom::ChangeState(WARROOM_STATE state) {
    int32_t& current_state = war_field_.room_state_;
    if (current_state == static_cast<int>(state))
        return static_cast<int>(state);

    OnLeaveState(state);
    current_state = static_cast<int>(state);
    OnEnterState();

    // IDA 0x86BB578 calls the void OnEnterState and returns without writing
    // EAX again.  Preserve that original changed-state machine path instead
    // of inventing a return value from the void callee.
}

// IDA_EA: 0x86BB71C
// IDA_NAME: _ZN7WarRoom12OnEnterStateEv
// DEMANGLED: WarRoom::OnEnterState(void)
void WarRoom::OnEnterState() {
    CancelAllTimer();
    InvalidUserCheck();
    SendStatePacket();

    switch (war_field_.room_state_) {
    case 0: {
        UpdateBanList();
        if (!shutdown_pending_) {
            break;
        }

        for (int slot = 0; slot <= 5; ++slot) {
            if (war_field_.room_users_[slot]) {
                WalkOutUserBySlot(slot);
            }
        }
        Close();
        shutdown_pending_ = 0;

        PacketGuard packet;
        packet.put_header(0, 88);
        packet.put_short(1);
        MakeStateInfo(reinterpret_cast<char*>(&packet));
        packet.finalize(true);
        G_GameWorld()->send_all(packet);
        break;
    }

    case 1: {
        war_field_.battle_started_at_sec_ =
            GlobalData::s_systemTime_.getCurSec();
        BattleReset();
        for (int slot = 0; slot <= 5; ++slot) {
            war_field_.live_flags_[slot] =
                war_field_.room_users_[slot] ? 1u : 0u;
        }

        bool can_start = GetWaiterCount() > 2;
        if (!can_start) {
            for (int slot = 0; slot <= 5; ++slot) {
                CUser* const user = war_field_.room_users_[slot];
                if (user && static_cast<unsigned char>(user->isGMUser())) {
                    can_start = true;
                    break;
                }
            }
        }

        if (can_start) {
            const int key = GenTimerKey(static_cast<TIMER_MESSAGE>(65));
            G_TimerQueue()->InsertTimerInMilisecond(
                3,
                room_index_,
                65,
                G_CDataManager()->get_war_area_counter()->ready_count,
                static_cast<unsigned int>(key),
                0);
        } else {
            ChangeState(static_cast<WARROOM_STATE>(0));
        }
        break;
    }

    case 2: {
        const int key = GenTimerKey(static_cast<TIMER_MESSAGE>(66));
        G_TimerQueue()->InsertTimerInMilisecond(
            3,
            room_index_,
            66,
            11000,
            static_cast<unsigned int>(key),
            0);
        current_spawn_step_ = 1;
        break;
    }

    case 3: {
        result_phase_active_ = 0;
        result_phase_sent_ = 0;

        PacketGuard packet;
        packet.clear();
        const int waiter_count = GetWaiterCount();
        war_field_.ConsistMap(
            static_cast<InterfacePacketBuf*>(&packet),
            war_field_.dungeon_,
            war_field_.map_,
            static_cast<unsigned char>(10 * (room_mode_ + 1)),
            waiter_count);
        SendToRoom(packet);

        for (int slot = 0; slot <= 5; ++slot) {
            CUser* const user = war_field_.room_users_[slot];
            if (user) {
                // CDungeon+0x7e8: STWarRoomData+0xf4, proven by
                // OnEnterState @0x86BBB59..0x86BBB84.
                user->FatigueUp(
                    war_field_.dungeon_->war_room_data_.tail_values_[2]);
            }
        }

        const int key = GenTimerKey(static_cast<TIMER_MESSAGE>(69));
        // CDungeon+0x7f0 is the millisecond battle duration.
        const int delay_seconds =
            war_field_.dungeon_->war_room_data_.tail_values_[4] / 1000;
        G_TimerQueue()->InsertTimer(
            3,
            room_index_,
            69,
            delay_seconds,
            static_cast<unsigned int>(key),
            0);
        break;
    }

    case 4: {
        ReviveAll();
        const int key = GenTimerKey(static_cast<TIMER_MESSAGE>(67));
        G_TimerQueue()->InsertTimer(
            3,
            room_index_,
            67,
            10,
            static_cast<unsigned int>(key),
            0);
        if (current_spawn_step_ !=
            war_field_.dungeon_->war_room_data_.timing_values_[0]) {
            ++current_spawn_step_;
        }
        break;
    }

    case 5: {
        if (result_phase_active_ &&
            war_field_.current_spawn_group_index_ ==
                war_field_.dungeon_->war_room_data_.timing_values_[0]) {
            PacketGuard packet;
            packet.put_header(0, 342);
            packet.put_byte(GetWaiterCount());

            const int penalty =
                G_CDataManager()->get_war_area_counter()->wp_penalty;
            for (int slot = 0; slot <= 5; ++slot) {
                CUser* const user = war_field_.room_users_[slot];
                if (!user) {
                    continue;
                }
                if (user->GetFinishPointTotal() > penalty) {
                    user->DecreaseWarPoint(penalty);
                } else {
                    user->DecreaseWarPoint(0);
                }
                packet.put_byte(slot);
                packet.put_int(user->GetFinishPointTotal());
            }
            packet.finalize(true);
            SendToRoom(packet);
        }

        for (int slot = 0; slot <= 5; ++slot) {
            CUser* const user = war_field_.room_users_[slot];
            if (user && !user->GetFinishPointTotal()) {
                war_field_.seat_states_[slot] = 6;
            }
        }

        ClearReward();
        SendGuildFP();
        RemoveKC();
        const int key = GenTimerKey(static_cast<TIMER_MESSAGE>(68));
        G_TimerQueue()->InsertTimer(
            3,
            room_index_,
            68,
            24,
            static_cast<unsigned int>(key),
            0);
        break;
    }

    default:
        break;
    }
}

// IDA_EA: 0x86BC460
// IDA_NAME: _ZN7WarRoom9ReviveAllEv
// DEMANGLED: WarRoom::ReviveAll(void)
bool WarRoom::ReviveAll() {
    int slot = 0;
    for (; slot < 6; ++slot) {
        CUser* user = war_field_.room_users_[slot];
        if (user && !war_field_.live_flags_[slot])
            SetCharacterLive(slot, user, true, false, 0, 255);
    }
    return slot <= 5;
}

// IDA_EA: 0x86BD3EC
// IDA_NAME: _ZN7WarRoom10CheckStateEv
// DEMANGLED: WarRoom::CheckState(void)
int WarRoom::CheckState() {
    if (!GetWaiterCount())
        return ChangeState(static_cast<WARROOM_STATE>(0));
    const int should_open = GetWaiterCount() <= 2 && GetState() == 1;
    if (should_open)
        return ChangeState(static_cast<WARROOM_STATE>(0));
    return should_open;
}

// IDA_EA: 0x86BDEA0
// IDA_NAME: _ZN7WarRoom12MakeSlotInfoEPci
// DEMANGLED: WarRoom::MakeSlotInfo(char *,int)
int WarRoom::MakeSlotInfo(char* packet_storage, int slot) {
    // The exported ABI deliberately spells this parameter as char*.  The
    // runtime caller supplies the address of the real packet base object.
    InterfacePacketBuf& packet =
        *reinterpret_cast<InterfacePacketBuf*>(packet_storage);
    packet.put_header(0, 89);
    packet.put_short(room_index_);
    if (slot == 0xffff) {
        const int waiter_count = GetWaiterCount();
        packet.put_byte(waiter_count);
        if (!waiter_count) {
            LogManager::logFormat(
                1,
                "WarRoom.cpp",
                "void WarRoom::MakeSlotInfo(char*, int)",
                0x793,
                "Send Member 0 PvpRoom %d State(%d)",
                room_index_,
                war_field_.room_state_);
        }
        for (int seat = 0; seat <= 5; ++seat) {
            CUser* const user = war_field_.room_users_[seat];
            if (!user) {
                continue;
            }
            packet.put_byte(seat);
            packet.put_byte(war_field_.seat_states_[seat]);
            packet.put_short(user->get_unique_id());
        }
    } else {
        packet.put_byte(1);
        packet.put_byte(slot);
        packet.put_byte(war_field_.seat_states_[slot]);
        CUser* const user = war_field_.room_users_[slot];
        packet.put_short(user ? user->get_unique_id() : 0xffff);
    }
    return packet.finalize(true);
}

// IDA_EA: 0x86BD256
// IDA_NAME: _ZN7WarRoom17WalkOutUserBySlotEi
// DEMANGLED: WarRoom::WalkOutUserBySlot(int)
int WarRoom::WalkOutUserBySlot(int slot) {
    CUser* const user = war_field_.room_users_[slot];
    if (!user) {
        LogManager::logFormat(1,
                              "WarRoom.cpp",
                              "int WarRoom::WalkOutUserBySlot(int)",
                              0x5cc,
                              "WarRoom::WalkOutUserBySlot no slot(%d)",
                              slot);
        return 4;
    }

    user->getHackAnalyzer()->reportHackInfo();
    user->getHackAnalyzer()->resetHackInfo();
    war_field_.seat_states_[slot] = 255;

    G_GameWorld()->out_from_warroom(user);
    if (war_field_.dungeon_) {
        CUser_history_log(user)->LeaveDungeon(
            reinterpret_cast<const CDungeon*>((war_field_.dungeon_))->GetDungeonName(),
            static_cast<int>(room_mode_));
    }

    war_field_.room_users_[slot] = 0;
    war_field_.live_flags_[slot] = 0;

    PacketGuard packet;
    MakeSlotInfo(reinterpret_cast<char*>(
                     static_cast<InterfacePacketBuf*>(&packet)),
                 slot);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x86BD460
// IDA_NAME: _ZN7WarRoom11WalkOutUserEP5CUser
// DEMANGLED: WarRoom::WalkOutUser(CUser *)
int WarRoom::WalkOutUser(CUser* user) {
    const int slot = GetUserSlot(user);
    return slot >= 0 ? WalkOutUserBySlot(slot) : 4;
}

// IDA_EA: 0x86BD49C
// IDA_NAME: _ZN7WarRoom7DestroyEi
// DEMANGLED: WarRoom::Destroy(int)
CUser* WarRoom::Destroy(int preferred_slot) {
    CUser* user = war_field_.room_users_[preferred_slot];
    if (!user) {
        for (int slot = 0; slot < 6; ++slot) {
            CUser* const candidate = war_field_.room_users_[slot];
            if (candidate) {
                user = candidate;
                break;
            }
        }
    }
    Reset();
    return user;
}

// IDA_EA: 0x86BD504
// IDA_NAME: _ZN7WarRoom4OpenEv
// DEMANGLED: WarRoom::Open(void)
WarRoom* WarRoom::Open() {
    return SetState(static_cast<WARROOM_STATE>(0));
}

// IDA_EA: 0x86BD520
// IDA_NAME: _ZN7WarRoom5CloseEv
// DEMANGLED: WarRoom::Close(void)
WarRoom* WarRoom::Close() {
    return SetState(static_cast<WARROOM_STATE>(-1));
}

// IDA_EA: 0x86BD53C
// IDA_NAME: _ZN7WarRoom8ShutDownEv
// DEMANGLED: WarRoom::ShutDown(void)
int WarRoom::ShutDown() {
    if (war_field_.room_state_ > 0) {
        shutdown_pending_ = 1;
        return 0;
    }
    for (int slot = 0; slot < 6; ++slot) {
        CUser* user = war_field_.room_users_[slot];
        if (user) {
            WalkOutUserBySlot(slot);
            SendWalkOut(user, static_cast<ENUM_WALKOUT_TYPE>(7));
        }
    }
    Close();
    return 1;
}

// IDA_EA: 0x86BD842
// IDA_NAME: _ZN7WarRoom11GetUserSlotEP5CUser
// DEMANGLED: WarRoom::GetUserSlot(CUser *)
int WarRoom::GetUserSlot(CUser* user) {
    for (int slot = 0; slot < 6; ++slot)
        if (war_field_.room_users_[slot] == user)
            return slot;
    return -1;
}

// IDA_EA: 0x86BE120
// IDA_NAME: _ZN7WarRoom14CancelAllTimerEv
// DEMANGLED: WarRoom::CancelAllTimer(void)
bool WarRoom::CancelAllTimer() {
    int message = 64;
    for (; message <= 71; ++message)
        GenTimerKey(static_cast<TIMER_MESSAGE>(message));
    return message <= 71;
}

// IDA_EA: 0x86C077E
// IDA_NAME: _ZN7WarRoom17CurSpawnStepResetEv
// DEMANGLED: WarRoom::CurSpawnStepReset(void)
WarRoom* WarRoom::CurSpawnStepReset() {
    current_spawn_step_ = 0;
    return this;
}

// IDA_EA: 0x86C0B24
// IDA_NAME: _ZN7WarRoom11GenTimerKeyE13TIMER_MESSAGE
// DEMANGLED: WarRoom::GenTimerKey(TIMER_MESSAGE)
int WarRoom::GenTimerKey(TIMER_MESSAGE message) {
    return ++war_field_.timer_keys_[
        static_cast<int>(message) - kFirstWarRoomTimerMessage];
}

// IDA_EA: 0x86BE8B4
// IDA_NAME: _ZN7WarRoom16SetCharacterLiveEiP5CUserbbsi
// DEMANGLED: WarRoom::SetCharacterLive(int,CUser *,bool,bool,short,int)
void WarRoom::SetCharacterLive(int slot,
                               CUser* user,
                               bool live,
                               bool by_coin,
                               short position,
                               int direction) {
    war_field_.live_flags_[slot] = live ? 1u : 0u;

    PacketGuard packet;
    packet.put_header(0, 95);
    packet.put_byte(GetUserSlot(user));
    packet.put_byte(live ? 1 : 0);
    packet.put_byte(by_coin ? 1 : 0);

    // The old body sends zero when the target seat is absent or the battle has
    // finished; otherwise it takes this dungeon's exact revive-HP field.
    const unsigned int target_slot = static_cast<unsigned int>(direction);
    CUser* const target = target_slot <= 5u
                              ? war_field_.room_users_[target_slot]
                              : 0;
    if (!target || battle_finished_) {
        packet.put_short(0);
    } else {
        packet.put_short(war_field_.dungeon_->war_room_data_.condition_mode_);
    }
    packet.put_short(position);
    packet.finalize(true);
    SendToRoom(packet);
}

// IDA_EA: 0x86BCF14
// IDA_NAME: _ZN7WarRoom12OnLeaveStateE13WARROOM_STATE
// DEMANGLED: WarRoom::OnLeaveState(WARROOM_STATE)
char WarRoom::OnLeaveState(WARROOM_STATE state) {
    if (static_cast<int>(state) == 0) {
        CurSpawnStepReset();
    }

    const int old_state = war_field_.room_state_;
    if (old_state == 2 && static_cast<int>(state) == 3) {
        const unsigned int room_mode = room_mode_;
        const int entry_cost =
            G_CDataManager()->get_war_area_counter()->door_fee[room_mode];

        for (int slot = 0; slot <= 5; ++slot) {
            CUser* const user = war_field_.room_users_[slot];
            if (!user) {
                continue;
            }

            const int paid_money = std::min(entry_cost,
                                             user->getCurCharacMoney());
            user->getCurCharacInvenW()->use_money(
                paid_money, static_cast<eMoneySubReason>(15), true);

            if (user->GetFinishPointTotal() > 0) {
                user->InitFinishPoint();
            }
        }
    }
    return static_cast<char>(old_state);
}

// IDA_EA: 0x86BC5FE
// IDA_NAME: _ZN7WarRoom11ClearRewardEv
// DEMANGLED: WarRoom::ClearReward(void)
bool WarRoom::ClearReward() {
    int reward_unit = 0;
    for (int index = 0; index <= 3; ++index) {
        // IDA truncates each product before adding it to the reward unit.
        reward_unit += static_cast<int>(
            static_cast<long double>(
                war_field_.clear_reward_multipliers_[index]) *
            war_field_.dungeon_->war_room_data_
                .extra_group_values_[index]);
    }

    for (int slot = 0; slot <= 5; ++slot) {
        CUser* const user = war_field_.room_users_[slot];
        if (!user) {
            continue;
        }

        PacketGuard reward_packet;
        PacketGuard level_packet;
        int exp_result = 0;
        int sp_result = 0;

        const float finish_point_factor =
            war_field_.dungeon_->war_room_data_
                .finish_point_reward_factor_;
        const float finish_point_scaled =
            static_cast<float>(user->GetFinishPointTotal()) *
            finish_point_factor;
        const unsigned int base_exp = static_cast<unsigned int>(
            static_cast<long double>(finish_point_scaled + 1.0f) *
            reward_unit);

        const float avatar_factor = user->IsEquipAvatar() ? 0.02f : 0.0f;
        const float creature_factor =
            user->getCurCharacInvenR()
                    ->GetCreatureMgrR()
                    ->IsEquippedCreature()
                ? 0.05f
                : 0.0f;

        unsigned int avatar_bonus = static_cast<unsigned int>(
            static_cast<long double>(base_exp) * avatar_factor);
        if (avatar_factor != 0.0f) {
            avatar_bonus = std::max(avatar_bonus, 1u);
        }

        unsigned int creature_bonus = static_cast<unsigned int>(
            static_cast<long double>(base_exp) * creature_factor);
        if (creature_factor != 0.0f) {
            creature_bonus = std::max(creature_bonus, 1u);
        }

        float repeat_event_factor = 0.0f;
        CEventBase* const repeat_event =
            GlobalData::s_event_manager->GetRepeatEvent(3);
        if (repeat_event->IsEventing(0)) {
            repeat_event_factor =
                static_cast<float>(
                    static_cast<CExpDoubleEvent*>(repeat_event)
                        ->GetExpFactor()) /
                100.0f;
        }
        // The original 0x86BCA0D..0x86BCA48 calculation is deliberately
        // retained even though its result is not part of the final sum.
        const unsigned int repeat_event_bonus =
            static_cast<unsigned int>(
                static_cast<long double>(base_exp) *
                repeat_event_factor);
        (void)repeat_event_bonus;

        channel_script_t* const channel_script =
            G_CDataManager()->GetChannelScript();
        const float channel_factor = static_cast<float>(
            channel_script->getBonusRate(
                static_cast<uint8_t>(
                    G_CEnvironment()->get_server_group()),
                static_cast<uint32_t>(
                    G_CEnvironment()->get_channel_no()),
                static_cast<uint32_t>(
                    war_field_.dungeon_->get_index())));
        const unsigned int channel_bonus =
            static_cast<unsigned int>(
                static_cast<long double>(base_exp) * channel_factor);

        const float server_event_factor =
            static_cast<float>(CServerEvent::GetExpRate()) / 100.0f;
        const unsigned int server_event_bonus =
            static_cast<unsigned int>(
                static_cast<long double>(base_exp) *
                server_event_factor);

        unsigned int premium_bonus = 0;
        if (user->IsHavePremiumAdvantage()) {
            const float premium_factor =
                static_cast<float>(
                    user->GetPremiumInfo()->GetAdvantageExpRate()) /
                100.0f;
            premium_bonus = static_cast<unsigned int>(
                static_cast<long double>(base_exp) * premium_factor);
        }

        unsigned int burning_bonus = 0;
        CBurningFatigueEvent* const burning_event =
            static_cast<CBurningFatigueEvent*>(
                GlobalData::s_event_manager->GetRepeatEvent(15));
        if (burning_event->IsEventing(0)) {
            const int bonus_rate = burning_event->getBonusExpRate(
                user->getCurCharacUsedFatigue());
            if (bonus_rate) {
                burning_bonus =
                    base_exp * static_cast<unsigned int>(bonus_rate) /
                    100u;
            }
        }

        unsigned int total_exp =
            burning_bonus + channel_bonus + server_event_bonus +
            premium_bonus + base_exp + avatar_bonus + creature_bonus;
        const int upper_member_bonus =
            user->gainExpAsUpperMember(static_cast<int>(base_exp));
        total_exp += static_cast<unsigned int>(upper_member_bonus);
        total_exp += static_cast<unsigned int>(
            user->gainGuildSkillExp(static_cast<int>(base_exp)));

        if (user->gain_exp_sp(
                static_cast<int>(total_exp),
                exp_result,
                sp_result,
                static_cast<eExpAddReason>(6),
                0,
                false)) {
            level_packet.clear();
            level_packet.put_header(0, 2);
            level_packet.put_byte(1);
            level_packet.put_short(1);
            user->make_basic_info(level_packet, 1);
            level_packet.finalize(true);
            user->Send(level_packet);
        }

        reward_packet.clear();
        reward_packet.put_header(0, 92);
        reward_packet.put_byte(exp_result);
        reward_packet.put_byte(sp_result);
        reward_packet.put_int(user->GetFinishPointTotal());
        reward_packet.put_int(
            static_cast<int>(
                server_event_bonus + base_exp + burning_bonus));
        reward_packet.put_int(static_cast<int>(avatar_bonus));
        reward_packet.put_int(static_cast<int>(premium_bonus));
        reward_packet.put_int(static_cast<int>(channel_bonus));
        reward_packet.put_int(upper_member_bonus);
        reward_packet.put_int(static_cast<int>(creature_bonus));
        reward_packet.finalize(true);
        user->Send(reward_packet);

        user->getHackAnalyzer()->setLastMonsterDeadTime(
            GlobalData::s_systemTime_.getCurSec());
    }
    return false;
}

// IDA_EA: 0x86BD094
// IDA_NAME: _ZN7WarRoom8RemoveKCEv
// DEMANGLED: WarRoom::RemoveKC(void)
bool WarRoom::RemoveKC() {
    for (int slot = 0; slot <= 5; ++slot) {
        CUser* const user = war_field_.room_users_[slot];
        if (!user) {
            continue;
        }
        user->InitFinishPoint();
        user->getCurCharacInvenW()->RemoveKCItem();
    }
    return false;
}
