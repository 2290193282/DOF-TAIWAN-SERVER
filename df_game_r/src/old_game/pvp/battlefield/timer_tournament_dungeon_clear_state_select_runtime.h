#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_TOURNAMENT_DUNGEON_CLEAR_STATE_SELECT_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_TOURNAMENT_DUNGEON_CLEAR_STATE_SELECT_RUNTIME_H_

#include "infra/concurrent/timer/timer_sig.h"

// IDA unit: 09_dungeon_pvp_party/Timer_TournametDungeonClearStateSelect.
// The original spelling contains "Tournamet" and is part of the exported ABI.
class Timer_TournametDungeonClearStateSelect : public TimerSig {
public:
    // IDA_EA: 0x863D0DA
    // IDA_NAME: _ZN38Timer_TournametDungeonClearStateSelectC2Ev
    // DEMANGLED: Timer_TournametDungeonClearStateSelect::Timer_TournametDungeonClearStateSelect(void)
    Timer_TournametDungeonClearStateSelect();

    // IDA_EA: 0x863E574
    // IDA_NAME: _ZN38Timer_TournametDungeonClearStateSelectD2Ev
    // DEMANGLED: Timer_TournametDungeonClearStateSelect::~Timer_TournametDungeonClearStateSelect()
    // IDA_EA: 0x863E5A4
    // IDA_NAME: _ZN38Timer_TournametDungeonClearStateSelectD0Ev
    // DEMANGLED: Timer_TournametDungeonClearStateSelect::~Timer_TournametDungeonClearStateSelect()
    virtual ~Timer_TournametDungeonClearStateSelect();

    // IDA_EA: 0x863B4EA
    // IDA_NAME: _ZN38Timer_TournametDungeonClearStateSelect12dispatch_sigEiij
    // DEMANGLED: Timer_TournametDungeonClearStateSelect::dispatch_sig(int,int,unsigned int)
    virtual int dispatch_sig(int party_index,
                             int timer_key,
                             unsigned int payload);
};

#if defined(__i386__)
static_assert(sizeof(Timer_TournametDungeonClearStateSelect) == 4u,
              "Timer_TournametDungeonClearStateSelect must remain TimerSig-only");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TIMER_TOURNAMENT_DUNGEON_CLEAR_STATE_SELECT_RUNTIME_H_
