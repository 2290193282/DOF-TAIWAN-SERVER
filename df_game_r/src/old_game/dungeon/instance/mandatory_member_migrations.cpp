#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/pvp/arena/pvp_room.h"

#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x82FE6BC
// IDA_NAME: _ZN8CRidable5ClearEv
// DEMANGLED: CRidable::Clear(void)
void CRidable::Clear() {
    std::map<int, DungeonRidableObject>::clear();
}

// IDA_EA: 0x85DE01A
// IDA_NAME: _ZN12CRelayBattle5ResetEv
// DEMANGLED: CRelayBattle::Reset(void)
CRelayBattle* CRelayBattle::Reset() {
    state = 0;
    change_sequence = 0;
    team = 0;
    request_fight = false;
    kill_count = 0;
    return this;
}

// IDA_EA: 0x85DE38A
// IDA_NAME: _ZN15CRelayBattleMgr11OnDiePlayerEiib
// DEMANGLED: CRelayBattleMgr::OnDiePlayer(int,int,bool)
void CRelayBattleMgr::OnDiePlayer(int dead_seat,
                                  int killer_seat,
                                  bool force_timer) {
    if (static_cast<unsigned int>(dead_seat) < 8u) {
        battles[dead_seat].state = 3;
    }
    if (static_cast<unsigned int>(killer_seat) < 8u) {
        ++battles[killer_seat].kill_count;
    }
    if (force_timer && state == 1) {
        state = 2;
        timer_key = GlobalData::system_time()->getCurSec();
        G_TimerQueue()->InsertTimerInMilisecond(
            1, parent->get_index(), 99, 3000, 0, timer_key);
    }
}

// IDA_EA: 0x85DE0F0
// IDA_NAME: _ZN15CRelayBattleMgr11OnLeaveRoomEi
// DEMANGLED: CRelayBattleMgr::OnLeaveRoom(int)
void CRelayBattleMgr::OnLeaveRoom(int seat) {
    if (static_cast<unsigned int>(seat) >= 8u) {
        return;
    }
    if (battles[seat].state == 2) {
        OnDiePlayer(seat, -1, true);
    }
    battles[seat].Reset();
}

// IDA_EA: 0x82EFF42
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr15NotifyGhostTimeEP5CUserii
// DEMANGLED: pvp_assault::CAssaultMgr::NotifyGhostTime(CUser *,int,int)
void pvp_assault::CAssaultMgr::NotifyGhostTime(CUser* user,
                                               int total_ghost_time,
                                               int remaining_ghost_time) {
    if (!user) {
        return;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 389);
    packet.put_int(total_ghost_time);
    packet.put_int(remaining_ghost_time);
    packet.finalize(1);
    user->Send(packet);
    Timer_AssaultGhostTimer::RegisterNextTimer(
        user, remaining_ghost_time,
        static_cast<unsigned int>(user->getCurCharacNo()));
}

// IDA_EA: 0x863B242
// IDA_NAME: _ZN23Timer_AssaultGhostTimer17RegisterNextTimerEP5CUserij
// DEMANGLED: Timer_AssaultGhostTimer::RegisterNextTimer(CUser *,int,unsigned int)
void Timer_AssaultGhostTimer::RegisterNextTimer(CUser* user,
                                                int remaining_ghost_time,
                                                unsigned int charac_no) {
    if (user) {
        G_TimerQueue()->InsertTimer(0, user->GetUID(), 160,
                                    remaining_ghost_time, 0, charac_no);
    }
}

// IDA_EA: 0x82345F0
// IDA_NAME: _ZNK8WongWork11CDeathTower11CDungeonMgr10getDungeonEv
// DEMANGLED: WongWork::CDeathTower::CDungeonMgr::getDungeon(void)const
CDungeon* WongWork::CDeathTower::CDungeonMgr::getDungeon() const {
    return dungeon_;
}

// IDA_EA: 0x8234612
// IDA_NAME: _ZNK8WongWork11CDeathTower14getCDungeonMGrEv
// DEMANGLED: WongWork::CDeathTower::getCDungeonMGr(void)const
WongWork::CDeathTower::CDungeonMgr*
WongWork::CDeathTower::getCDungeonMGr() const {
    return const_cast<CDungeonMgr*>(&dungeon_manager_);
}
