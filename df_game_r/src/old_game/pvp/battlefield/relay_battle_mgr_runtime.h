#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_RELAY_BATTLE_MGR_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_RELAY_BATTLE_MGR_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

class PvP_Room;

// IDA unit: 99_misc_global/CRelayBattle.  This is a named, embedded
// 12-byte relay-player state, not an anonymous slice of PvP_Room.
class CRelayBattle {
public:
    // IDA_EA: 0x85DE01A
    // IDA_NAME: _ZN12CRelayBattle5ResetEv
    // DEMANGLED: CRelayBattle::Reset(void)
    CRelayBattle* Reset();

    int state;                 // +0x00
    uint16_t change_sequence;  // +0x04
    int8_t team;               // +0x06
    bool request_fight;        // +0x07
    uint8_t kill_count;        // +0x08
};

static_assert(sizeof(CRelayBattle) == 0x0cu,
              "CRelayBattle IDA ABI size must stay 0x0c");
static_assert(offsetof(CRelayBattle, team) == 0x06u,
              "CRelayBattle team must stay at +0x06");
static_assert(offsetof(CRelayBattle, kill_count) == 0x08u,
              "CRelayBattle kill count must stay at +0x08");

// IDA unit: 99_misc_global/CRelayBattleMgr.  PvP_Room embeds this manager at
// +0x620.  The active old IDB accesses active_player_count at +0x78, so the
// complete typed manager is 0x7c bytes (the prior 0x78 partial declaration
// could not represent GetAllKillUserIndex or OnStart faithfully).
class CRelayBattleMgr {
public:
    // IDA_EA: 0x8568CEE
    // IDA_NAME: _ZN15CRelayBattleMgr12GetKillCountEi
    // DEMANGLED: CRelayBattleMgr::GetKillCount(int)
    int GetKillCount(int seat);

    // IDA_EA: 0x85DE046
    // IDA_NAME: _ZN15CRelayBattleMgr5ResetEv
    // DEMANGLED: CRelayBattleMgr::Reset(void)
    CRelayBattleMgr* Reset();

    // IDA_EA: 0x85DE09C
    // IDA_NAME: _ZN15CRelayBattleMgr24ResetChangeSequenceCountEv
    // DEMANGLED: CRelayBattleMgr::ResetChangeSequenceCount(void)
    void ResetChangeSequenceCount();

    // IDA_EA: 0x85DE0CC
    // IDA_NAME: _ZN15CRelayBattleMgr12OnCreateRoomEi
    // DEMANGLED: CRelayBattleMgr::OnCreateRoom(int)
    void OnCreateRoom(int seat);

    // IDA_EA: 0x85DE0DE
    // IDA_NAME: _ZN15CRelayBattleMgr10OnJoinRoomEi
    // DEMANGLED: CRelayBattleMgr::OnJoinRoom(int)
    void OnJoinRoom(int seat);

    // IDA_EA: 0x85DE0F0
    // IDA_NAME: _ZN15CRelayBattleMgr11OnLeaveRoomEi
    // DEMANGLED: CRelayBattleMgr::OnLeaveRoom(int)
    void OnLeaveRoom(int seat);

    // IDA_EA: 0x85DE15A
    // IDA_NAME: _ZN15CRelayBattleMgr12OnChangeTeamEii
    // DEMANGLED: CRelayBattleMgr::OnChangeTeam(int,int)
    void OnChangeTeam(int seat, int team);

    // IDA_EA: 0x85DE18C
    // IDA_NAME: _ZN15CRelayBattleMgr14OnRequestFightEi
    // DEMANGLED: CRelayBattleMgr::OnRequestFight(int)
    int OnRequestFight(int seat);

    // IDA_EA: 0x85DE252
    // IDA_NAME: _ZN15CRelayBattleMgr7OnReadyEib
    // DEMANGLED: CRelayBattleMgr::OnReady(int,bool)
    void OnReady(int seat, bool ready);

    // IDA_EA: 0x85DE29C
    // IDA_NAME: _ZN15CRelayBattleMgr7OnStartEv
    // DEMANGLED: CRelayBattleMgr::OnStart(void)
    void OnStart();

    // IDA_EA: 0x85DE358
    // IDA_NAME: _ZN15CRelayBattleMgr7OnFightEv
    // DEMANGLED: CRelayBattleMgr::OnFight(void)
    void OnFight();

    // IDA_EA: 0x85DE38A
    // IDA_NAME: _ZN15CRelayBattleMgr11OnDiePlayerEiib
    // DEMANGLED: CRelayBattleMgr::OnDiePlayer(int,int,bool)
    void OnDiePlayer(int dead_seat, int killer_seat, bool force_timer);

    // IDA_EA: 0x85DE474
    // IDA_NAME: _ZN15CRelayBattleMgr22GetChangeSequenceCountEi
    // DEMANGLED: CRelayBattleMgr::GetChangeSequenceCount(int)
    int GetChangeSequenceCount(int team);

    // IDA_EA: 0x85DE498
    // IDA_NAME: _ZN15CRelayBattleMgr27IncreaseChangeSequenceCountEi
    // DEMANGLED: CRelayBattleMgr::IncreaseChangeSequenceCount(int)
    void IncreaseChangeSequenceCount(int team);

    // IDA_EA: 0x85DE53E
    // IDA_NAME: _ZN15CRelayBattleMgr27DecreaseChangeSequenceCountEi
    // DEMANGLED: CRelayBattleMgr::DecreaseChangeSequenceCount(int)
    void DecreaseChangeSequenceCount(int team);

    // IDA_EA: 0x85DE5E4
    // IDA_NAME: _ZN15CRelayBattleMgr26CreateRandomSequenceVectorERSt6vectorIiSaIiEEi
    // DEMANGLED: CRelayBattleMgr::CreateRandomSequenceVector(std::vector<int,std::allocator<int>> &,int)
    void CreateRandomSequenceVector(std::vector<int>& sequence,
                                    int requested_count);

    // IDA_EA: 0x85DE790
    // IDA_NAME: _ZN15CRelayBattleMgr21ArrangeChangeSequenceERSt6vectorIiSaIiEEiRi
    // DEMANGLED: CRelayBattleMgr::ArrangeChangeSequence(std::vector<int,std::allocator<int>> &,int,int &)
    void ArrangeChangeSequence(std::vector<int>& sequence,
                               int team,
                               int& next_index);

    // IDA_EA: 0x85DE83E
    // IDA_NAME: _ZN15CRelayBattleMgr23ArrangeNoChangeSequenceERSt6vectorIiSaIiEEiRi
    // DEMANGLED: CRelayBattleMgr::ArrangeNoChangeSequence(std::vector<int,std::allocator<int>> &,int,int &)
    void ArrangeNoChangeSequence(std::vector<int>& sequence,
                                 int team,
                                 int& next_index);

    // IDA_EA: 0x85DE8EE
    // IDA_NAME: _ZN15CRelayBattleMgr20ArrangeMatchSequenceEv
    // DEMANGLED: CRelayBattleMgr::ArrangeMatchSequence(void)
    void ArrangeMatchSequence();

    // IDA_EA: 0x85DEA14
    // IDA_NAME: _ZN15CRelayBattleMgr13GetNextPlayerEi
    // DEMANGLED: CRelayBattleMgr::GetNextPlayer(int)
    int GetNextPlayer(int team);

    // IDA_EA: 0x85DEAD6
    // IDA_NAME: _ZN15CRelayBattleMgr10TurnPlayerEv
    // DEMANGLED: CRelayBattleMgr::TurnPlayer(void)
    void TurnPlayer();

    // IDA_EA: 0x85DECD8
    // IDA_NAME: _ZN15CRelayBattleMgr16SendRequestFightEi
    // DEMANGLED: CRelayBattleMgr::SendRequestFight(int)
    int SendRequestFight(int seat);

    // IDA_EA: 0x85DED74
    // IDA_NAME: _ZN15CRelayBattleMgr19GetAllKillUserIndexEPi
    // DEMANGLED: CRelayBattleMgr::GetAllKillUserIndex(int *)
    int GetAllKillUserIndex(int* team_out);

    // IDA_EA: 0x85DF91E
    // IDA_NAME: _ZN15CRelayBattleMgr9SetParentEP8PvP_Room
    // DEMANGLED: CRelayBattleMgr::SetParent(PvP_Room *)
    CRelayBattleMgr* SetParent(PvP_Room* parent_value);

    // IDA_EA: 0x863BCA8
    // IDA_NAME: _ZN15CRelayBattleMgr11GetTimerKeyEv
    // DEMANGLED: CRelayBattleMgr::GetTimerKey(void)
    int GetTimerKey();

    CRelayBattle battles[8];            // +0x00
    // Old code addresses this as this[24 + team], with team in [1, 2].
    // Index 0 is retained because it is part of that 1-based array's ABI.
    int change_sequence_count[3];       // +0x60, entries 1/2 at +0x64/+0x68
    PvP_Room* parent;                   // +0x6c
    int timer_key;                      // +0x70
    int state;                          // +0x74
    int active_player_count;            // +0x78
};

static_assert(offsetof(CRelayBattleMgr, change_sequence_count) == 0x60u,
              "CRelayBattleMgr one-based change counts must stay at +0x60");
static_assert(offsetof(CRelayBattleMgr, parent) == 0x6cu,
              "CRelayBattleMgr parent must stay at +0x6c");
static_assert(offsetof(CRelayBattleMgr, active_player_count) == 0x78u,
              "CRelayBattleMgr active player count must stay at +0x78");
static_assert(sizeof(CRelayBattleMgr) == 0x7cu,
              "CRelayBattleMgr IDA ABI size must stay 0x7c");

// These helpers are real old-server class families directly used by
// CreateRandomSequenceVector.  They remain typed so the original retry,
// descending-sort, and random_shuffle control flow is not substituted.
class CRandomValue {
public:
    // IDA_EA: 0x85DF7FE
    // IDA_NAME: _ZN12CRandomValueC2ERSt6vectorIiSaIiEEiii
    // DEMANGLED: CRandomValue::CRandomValue(std::vector<int,std::allocator<int>> &,int,int,int)
    CRandomValue(std::vector<int>& values,
                 int minimum,
                 int maximum,
                 int retry_count);

    // IDA_EA: 0x85DF826
    // IDA_NAME: _ZN12CRandomValueclEv
    // DEMANGLED: CRandomValue::operator()(void)
    int operator()();

private:
    std::vector<int>* values_;
    int maximum_;
    int minimum_;
    int retry_count_;
};

static_assert(sizeof(CRandomValue) == 0x10u,
              "CRandomValue IDA ABI size must stay 0x10");

class CDescending {
public:
    // IDA_EA: 0x85DF910
    // IDA_NAME: _ZN11CDescendingclEii
    // DEMANGLED: CDescending::operator()(int,int)
    bool operator()(int lhs, int rhs);
};

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_RELAY_BATTLE_MGR_RUNTIME_H_
