#include "old_game/pvp/battlefield/relay_battle_mgr_runtime.h"

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <algorithm>
#include <cstdlib>

// IDA_EA: 0x8568CEE
// IDA_NAME: _ZN15CRelayBattleMgr12GetKillCountEi
// DEMANGLED: CRelayBattleMgr::GetKillCount(int)
int CRelayBattleMgr::GetKillCount(int seat) {
    return battles[seat].kill_count;
}

// IDA_EA: 0x85DE046
// IDA_NAME: _ZN15CRelayBattleMgr5ResetEv
// DEMANGLED: CRelayBattleMgr::Reset(void)
CRelayBattleMgr* CRelayBattleMgr::Reset() {
    for (int seat = 0; seat <= 7; ++seat) {
        battles[seat].Reset();
    }
    ResetChangeSequenceCount();
    timer_key = 0;
    state = 0;
    return this;
}

// IDA_EA: 0x85DE09C
// IDA_NAME: _ZN15CRelayBattleMgr24ResetChangeSequenceCountEv
// DEMANGLED: CRelayBattleMgr::ResetChangeSequenceCount(void)
void CRelayBattleMgr::ResetChangeSequenceCount() {
    for (int team = 1; team <= 2; ++team) {
        change_sequence_count[team] = 0;
    }
}

// IDA_EA: 0x85DE0CC
// IDA_NAME: _ZN15CRelayBattleMgr12OnCreateRoomEi
// DEMANGLED: CRelayBattleMgr::OnCreateRoom(int)
void CRelayBattleMgr::OnCreateRoom(int) {
    // Exact old body is a no-op return.
}

// IDA_EA: 0x85DE0DE
// IDA_NAME: _ZN15CRelayBattleMgr10OnJoinRoomEi
// DEMANGLED: CRelayBattleMgr::OnJoinRoom(int)
void CRelayBattleMgr::OnJoinRoom(int) {
    // Exact old body is a no-op return.
}

// IDA_EA: 0x85DE18C
// IDA_NAME: _ZN15CRelayBattleMgr14OnRequestFightEi
// DEMANGLED: CRelayBattleMgr::OnRequestFight(int)
int CRelayBattleMgr::OnRequestFight(int seat) {
    if (static_cast<unsigned int>(seat) < 8u) {
        if (battles[seat].request_fight) {
            battles[seat].request_fight = false;
            DecreaseChangeSequenceCount(battles[seat].team);
        } else {
            battles[seat].request_fight = true;
            IncreaseChangeSequenceCount(battles[seat].team);
        }
        return SendRequestFight(seat);
    }
    return 0;
}

// IDA_EA: 0x85DE252
// IDA_NAME: _ZN15CRelayBattleMgr7OnReadyEib
// DEMANGLED: CRelayBattleMgr::OnReady(int,bool)
void CRelayBattleMgr::OnReady(int seat, bool ready) {
    if (static_cast<unsigned int>(seat) < 8u) {
        battles[seat].state = ready ? 1 : 0;
    }
}

// IDA_EA: 0x85DE29C
// IDA_NAME: _ZN15CRelayBattleMgr7OnStartEv
// DEMANGLED: CRelayBattleMgr::OnStart(void)
void CRelayBattleMgr::OnStart() {
    timer_key = GlobalData::system_time()->getCurSec();
    active_player_count = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        if (battles[seat].state == 1) {
            ++active_player_count;
        }
    }
    G_TimerQueue()->InsertTimerInMilisecond(
        1, parent->get_index(), 99, 7000, 0, timer_key);
}

// IDA_EA: 0x85DE358
// IDA_NAME: _ZN15CRelayBattleMgr7OnFightEv
// DEMANGLED: CRelayBattleMgr::OnFight(void)
void CRelayBattleMgr::OnFight() {
    timer_key = 0;
    ArrangeMatchSequence();
    TurnPlayer();
    state = 1;
}

// IDA_EA: 0x85DE474
// IDA_NAME: _ZN15CRelayBattleMgr22GetChangeSequenceCountEi
// DEMANGLED: CRelayBattleMgr::GetChangeSequenceCount(int)
int CRelayBattleMgr::GetChangeSequenceCount(int team) {
    if (team > 0 && team <= 2) {
        return change_sequence_count[team];
    }
    return 0;
}

// IDA_EA: 0x85DE498
// IDA_NAME: _ZN15CRelayBattleMgr27IncreaseChangeSequenceCountEi
// DEMANGLED: CRelayBattleMgr::IncreaseChangeSequenceCount(int)
void CRelayBattleMgr::IncreaseChangeSequenceCount(int team) {
    if (team > 0 && team <= 2) {
        int& count = change_sequence_count[team];
        ++count;
        if (count > 3) {
            cMyTrace trace(
                "void CRelayBattleMgr::IncreaseChangeSequenceCount(int)",
                5086,
                0);
            trace("ERROR ChangeSequenceCount overflow (%d)", count);
            count = 4;
        }
    }
}

// IDA_EA: 0x85DE53E
// IDA_NAME: _ZN15CRelayBattleMgr27DecreaseChangeSequenceCountEi
// DEMANGLED: CRelayBattleMgr::DecreaseChangeSequenceCount(int)
void CRelayBattleMgr::DecreaseChangeSequenceCount(int team) {
    if (team > 0 && team <= 2) {
        int& count = change_sequence_count[team];
        --count;
        if (count < 0) {
            cMyTrace trace(
                "void CRelayBattleMgr::DecreaseChangeSequenceCount(int)",
                5098,
                0);
            trace("ERROR ChangeSequenceCount underflow (%d)", count);
            count = 0;
        }
    }
}

// IDA_EA: 0x85DF7FE
// IDA_NAME: _ZN12CRandomValueC2ERSt6vectorIiSaIiEEiii
// DEMANGLED: CRandomValue::CRandomValue(std::vector<int,std::allocator<int>> &,int,int,int)
CRandomValue::CRandomValue(std::vector<int>& values,
                           int minimum,
                           int maximum,
                           int retry_count)
    : values_(&values),
      maximum_(maximum),
      minimum_(minimum),
      retry_count_(retry_count) {}

// IDA_EA: 0x85DF826
// IDA_NAME: _ZN12CRandomValueclEv
// DEMANGLED: CRandomValue::operator()(void)
int CRandomValue::operator()() {
    int value = minimum_;
    for (int attempt = 0; attempt < retry_count_; ++attempt) {
        value = std::rand() % (maximum_ - minimum_ + 1) + minimum_;
        if (std::find(values_->begin(), values_->end(), value) ==
            values_->end()) {
            break;
        }
    }
    return value;
}

// IDA_EA: 0x85DF910
// IDA_NAME: _ZN11CDescendingclEii
// DEMANGLED: CDescending::operator()(int,int)
bool CDescending::operator()(int lhs, int rhs) {
    return lhs > rhs;
}

// IDA_EA: 0x85DE5E4
// IDA_NAME: _ZN15CRelayBattleMgr26CreateRandomSequenceVectorERSt6vectorIiSaIiEEi
// DEMANGLED: CRelayBattleMgr::CreateRandomSequenceVector(std::vector<int,std::allocator<int>> &,int)
void CRelayBattleMgr::CreateRandomSequenceVector(std::vector<int>& sequence,
                                                 int requested_count) {
    if (requested_count > static_cast<int>(sequence.size())) {
        requested_count = static_cast<int>(sequence.size());
    }
    if (requested_count <= 0) {
        int value = static_cast<int>(sequence.size());
        for (size_t index = 0; index < sequence.size(); ++index) {
            sequence[index] = value--;
        }
        return;
    }

    CRandomValue random_value(sequence, 1, 100, 10);
    std::generate(sequence.begin(), sequence.end(), random_value);
    std::sort(sequence.begin(), sequence.end(), CDescending());
    std::random_shuffle(sequence.begin(), sequence.begin() + requested_count);
}

// IDA_EA: 0x85DE790
// IDA_NAME: _ZN15CRelayBattleMgr21ArrangeChangeSequenceERSt6vectorIiSaIiEEiRi
// DEMANGLED: CRelayBattleMgr::ArrangeChangeSequence(std::vector<int,std::allocator<int>> &,int,int &)
void CRelayBattleMgr::ArrangeChangeSequence(std::vector<int>& sequence,
                                            int team,
                                            int& next_index) {
    for (int seat = 0; seat <= 7; ++seat) {
        CRelayBattle& battle = battles[seat];
        if (battle.team == team && battle.state == 1 &&
            battle.request_fight) {
            battle.change_sequence =
                static_cast<uint16_t>(sequence[next_index++]);
        }
    }
}

// IDA_EA: 0x85DE83E
// IDA_NAME: _ZN15CRelayBattleMgr23ArrangeNoChangeSequenceERSt6vectorIiSaIiEEiRi
// DEMANGLED: CRelayBattleMgr::ArrangeNoChangeSequence(std::vector<int,std::allocator<int>> &,int,int &)
void CRelayBattleMgr::ArrangeNoChangeSequence(std::vector<int>& sequence,
                                              int team,
                                              int& next_index) {
    for (int seat = 0; seat <= 7; ++seat) {
        CRelayBattle& battle = battles[seat];
        if (battle.team == team && battle.state == 1 &&
            !battle.request_fight) {
            battle.change_sequence =
                static_cast<uint16_t>(sequence[next_index++]);
        }
    }
}

// IDA_EA: 0x85DE8EE
// IDA_NAME: _ZN15CRelayBattleMgr20ArrangeMatchSequenceEv
// DEMANGLED: CRelayBattleMgr::ArrangeMatchSequence(void)
void CRelayBattleMgr::ArrangeMatchSequence() {
    for (int team = 1; team <= 2; ++team) {
        int next_index = 0;
        std::vector<int> sequence(4, 0);
        CreateRandomSequenceVector(sequence, GetChangeSequenceCount(team));
        ArrangeChangeSequence(sequence, team, next_index);
        ArrangeNoChangeSequence(sequence, team, next_index);
    }
}

// IDA_EA: 0x85DEA14
// IDA_NAME: _ZN15CRelayBattleMgr13GetNextPlayerEi
// DEMANGLED: CRelayBattleMgr::GetNextPlayer(int)
int CRelayBattleMgr::GetNextPlayer(int team) {
    int next_seat = -1;
    int highest_sequence = 0;
    for (int seat = 0; seat <= 7; ++seat) {
        CRelayBattle& battle = battles[seat];
        if (battle.team == team) {
            if (battle.state == 1) {
                if (static_cast<int16_t>(battle.change_sequence) >
                    highest_sequence) {
                    highest_sequence = battle.change_sequence;
                    next_seat = seat;
                }
            } else if (battle.state == 2) {
                return -1;
            }
        }
    }
    // The old code indexes -1 if the caller asks for a team with no ready
    // player and no active fighter.  Retain that evidenced precondition;
    // do not invent a safe fallback branch.
    battles[next_seat].state = 2;
    return next_seat;
}

// IDA_EA: 0x85DEAD6
// IDA_NAME: _ZN15CRelayBattleMgr10TurnPlayerEv
// DEMANGLED: CRelayBattleMgr::TurnPlayer(void)
void CRelayBattleMgr::TurnPlayer() {
    if (!parent->IsEndPvpBattle()) {
        PacketGuard packet;
        packet.put_header(0, 112);
        packet.put_byte(2);
        for (int team = 1; team <= 2; ++team) {
            const int next_player = GetNextPlayer(team);
            packet.put_byte(next_player);
            int count_index = packet.get_index();
            int change_count = 0;
            packet.put_byte(0);
            if (change_sequence_count[team] > 0) {
                for (int seat = 0; seat <= 7; ++seat) {
                    CRelayBattle& battle = battles[seat];
                    if (battle.request_fight && battle.team == team) {
                        packet.put_byte(seat);
                        packet.put_short(battle.change_sequence);
                        ++change_count;
                        battle.request_fight = false;
                    }
                }
            }
            packet.put_byte(count_index, change_count);
        }
        packet.finalize(true);
        parent->send_to_pvp(packet);
        ResetChangeSequenceCount();
    }
}

// IDA_EA: 0x85DECD8
// IDA_NAME: _ZN15CRelayBattleMgr16SendRequestFightEi
// DEMANGLED: CRelayBattleMgr::SendRequestFight(int)
int CRelayBattleMgr::SendRequestFight(int seat) {
    PacketGuard packet;
    packet.put_header(0, 113);
    packet.put_byte(seat);
    packet.finalize(true);
    parent->send_to_pvp(packet);
    return 1;
}

// IDA_EA: 0x85DED74
// IDA_NAME: _ZN15CRelayBattleMgr19GetAllKillUserIndexEPi
// DEMANGLED: CRelayBattleMgr::GetAllKillUserIndex(int *)
int CRelayBattleMgr::GetAllKillUserIndex(int* team_out) {
    for (int seat = 0; seat <= 7; ++seat) {
        const CRelayBattle& battle = battles[seat];
        if (battle.kill_count > 1 &&
            battle.kill_count >= active_player_count / 2 &&
            battle.state == 2) {
            if (team_out) {
                *team_out = battle.team;
            }
            return seat;
        }
    }
    return -1;
}

// IDA_EA: 0x85DF91E
// IDA_NAME: _ZN15CRelayBattleMgr9SetParentEP8PvP_Room
// DEMANGLED: CRelayBattleMgr::SetParent(PvP_Room *)
CRelayBattleMgr* CRelayBattleMgr::SetParent(PvP_Room* parent_value) {
    parent = parent_value;
    return this;
}

// IDA_EA: 0x863BCA8
// IDA_NAME: _ZN15CRelayBattleMgr11GetTimerKeyEv
// DEMANGLED: CRelayBattleMgr::GetTimerKey(void)
int CRelayBattleMgr::GetTimerKey() {
    return timer_key;
}
