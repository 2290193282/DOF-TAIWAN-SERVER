#ifndef OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_BLOOD_DUNGEON_H_
#define OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_BLOOD_DUNGEON_H_

#include "old_data/db/core/db_dispatcher.h"

struct SIG_BLOOD_DUNGEON;

// Old DB signal 0x197 writes the current Blood Dungeon week state and marks
// a claimed reward in the preceding week record.
class DB_SaveBloodDungeon : public DBDispatcher {
public:
    // IDA_EA: 0x844F06A
    // IDA_NAME: _ZN19DB_SaveBloodDungeonC2Ev
    // DEMANGLED: DB_SaveBloodDungeon::DB_SaveBloodDungeon(void)
    DB_SaveBloodDungeon();

    // IDA_EA: 0x845D17A
    // IDA_NAME: _ZN19DB_SaveBloodDungeonD2Ev
    // DEMANGLED: DB_SaveBloodDungeon::~DB_SaveBloodDungeon()
    // IDA_EA: 0x845D1AA
    // IDA_NAME: _ZN19DB_SaveBloodDungeonD0Ev
    // DEMANGLED: DB_SaveBloodDungeon::~DB_SaveBloodDungeon()
    virtual ~DB_SaveBloodDungeon();

    // IDA_EA: 0x8437728
    // IDA_NAME: _ZN19DB_SaveBloodDungeon8dispatchEiiP6Stream
    // DEMANGLED: DB_SaveBloodDungeon::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x84374F8
    // IDA_NAME: _ZN19DB_SaveBloodDungeon22SaveBloodDungeonRewardEP17SIG_BLOOD_DUNGEON
    // DEMANGLED: DB_SaveBloodDungeon::SaveBloodDungeonReward(SIG_BLOOD_DUNGEON *)
    bool SaveBloodDungeonReward(SIG_BLOOD_DUNGEON* payload);
};

static_assert(sizeof(DB_SaveBloodDungeon) == 0x04u,
              "DB_SaveBloodDungeon IDA ABI size must stay 0x04");

#endif  // OLDSRV_OLD_DATA_DB_DUNGEON_DB_SAVE_BLOOD_DUNGEON_H_
