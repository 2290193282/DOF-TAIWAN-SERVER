#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_CHARACTER_BLOOD_DUNGEON_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_CHARACTER_BLOOD_DUNGEON_RUNTIME_H_

#include <stdint.h>

#include "old_game/player/core/charac_expand_cdata.h"

class CUser;

// Canonical C-owned persisted character Blood Dungeon state.  The DB request
// carried by _saveData stays a real dispatcher/stream request; no synthetic
// success path is introduced here.
class CCharacBloodDungeon : public charac_expand::CData {
public:
    // IDA_EA: 0x832AF6E
    // IDA_NAME: _ZN19CCharacBloodDungeonC2Ev
    // DEMANGLED: CCharacBloodDungeon::CCharacBloodDungeon(void)
    CCharacBloodDungeon();

    // IDA_EA: 0x832A716
    // IDA_NAME: _ZN19CCharacBloodDungeonD2Ev
    // DEMANGLED: CCharacBloodDungeon::~CCharacBloodDungeon()
    // The virtual C++ destructor also emits the IDA D0 deleting-destructor
    // symbol (_ZN19CCharacBloodDungeonD0Ev); no free-function shim is used.
    virtual ~CCharacBloodDungeon();

    // IDA_EA: 0x832A53A
    // IDA_NAME: _ZNK19CCharacBloodDungeon7getDataEPc
    // DEMANGLED: CCharacBloodDungeon::getData(char *)const
    virtual void getData(char* data) const;

    // IDA_EA: 0x832A4C6
    // IDA_NAME: _ZN19CCharacBloodDungeon8loadDataEP5CUserPc
    // DEMANGLED: CCharacBloodDungeon::loadData(CUser *,char *)
    virtual bool loadData(CUser* user, char* data);

    // IDA_EA: 0x832A5B6
    // IDA_NAME: _ZN19CCharacBloodDungeon10ResetDailyEv
    // DEMANGLED: CCharacBloodDungeon::ResetDaily(void)
    virtual void ResetDaily();

    // IDA_EA: 0x85BFE88
    // IDA_NAME: _ZNK19CCharacBloodDungeon13GetEnterCountEv
    // DEMANGLED: CCharacBloodDungeon::GetEnterCount(void)const
    int GetEnterCount() const;

    // IDA_EA: 0x85BFE94
    // IDA_NAME: _ZNK19CCharacBloodDungeon19GetWeeklyEnterCountEv
    // DEMANGLED: CCharacBloodDungeon::GetWeeklyEnterCount(void)const
    int GetWeeklyEnterCount() const;

    // IDA_EA: 0x85BFEA0
    // IDA_NAME: _ZN19CCharacBloodDungeon13IncEnterCountEv
    // DEMANGLED: CCharacBloodDungeon::IncEnterCount(void)
    CCharacBloodDungeon* IncEnterCount();

    // IDA_EA: 0x85BFED2
    // IDA_NAME: _ZN19CCharacBloodDungeon8IncPointEi
    // DEMANGLED: CCharacBloodDungeon::IncPoint(int)
    CCharacBloodDungeon* IncPoint(int value);

    // IDA_EA: 0x85BFEF6
    // IDA_NAME: _ZN19CCharacBloodDungeon10IncUseGoldEi
    // DEMANGLED: CCharacBloodDungeon::IncUseGold(int)
    CCharacBloodDungeon* IncUseGold(int value);

protected:
    // IDA_EA: 0x832A330
    // IDA_NAME: _ZN19CCharacBloodDungeon9_saveDataEP5CUser
    // DEMANGLED: CCharacBloodDungeon::_saveData(CUser *)
    virtual int _saveData(CUser* user);

    // IDA_EA: 0x832A2D4
    // IDA_NAME: _ZN19CCharacBloodDungeon6_resetEv
    // DEMANGLED: CCharacBloodDungeon::_reset(void)
    virtual int _reset();

private:
    int week_point_;                  // IDA +0x08
    int week_enter_count_;            // IDA +0x0c
    int week_use_gold_;               // IDA +0x10
    int last_play_date_;              // IDA +0x14
    int enter_count_;                 // IDA +0x18
    int previous_rank_;               // IDA +0x1c
    uint8_t previous_reward_claimed_; // IDA +0x20
    uint8_t raw_21_[0x03];            // IDA +0x21
    int previous_reward_item_id_;     // IDA +0x24
    int previous_reward_gold_;        // IDA +0x28
};

static_assert(sizeof(CCharacBloodDungeon) == 0x2cu,
              "CCharacBloodDungeon IDA ABI size must stay 0x2c");

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_CHARACTER_BLOOD_DUNGEON_RUNTIME_H_
