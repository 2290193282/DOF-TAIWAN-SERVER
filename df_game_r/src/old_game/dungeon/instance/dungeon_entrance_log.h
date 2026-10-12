#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_DUNGEON_ENTRANCE_LOG_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_DUNGEON_ENTRANCE_LOG_H_

#include <map>

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

// IDA unit: 09_dungeon_pvp_party/CDungeonEntranceLog.  The constructor at
// 0x86103FE constructs std::map<int, STDungeonInoutCnt> at `this`, so the
// gameplay type has that map as its base object.
class CDungeonEntranceLog : public std::map<int, STDungeonInoutCnt> {
public:
    typedef std::map<int, STDungeonInoutCnt> Base;
    typedef Base::value_type Entry;

    // IDA_EA: 0x86103FE
    // IDA_NAME: _ZN19CDungeonEntranceLogC2Ev
    // DEMANGLED: CDungeonEntranceLog::CDungeonEntranceLog(void)
    CDungeonEntranceLog();

    // IDA_EA: 0x861089A
    // IDA_NAME: _ZN19CDungeonEntranceLogD2Ev
    // DEMANGLED: CDungeonEntranceLog::~CDungeonEntranceLog()
    ~CDungeonEntranceLog();

    // IDA_EA: 0x860E1EE
    // IDA_NAME: _ZN19CDungeonEntranceLog24IncrementDungeonEntranceEib
    // DEMANGLED: CDungeonEntranceLog::IncrementDungeonEntrance(int,bool)
    Entry* IncrementDungeonEntrance(int dungeon_index, bool tower);

    // IDA_EA: 0x860E324
    // IDA_NAME: _ZN19CDungeonEntranceLog24DecrementDungeonEntranceEib
    // DEMANGLED: CDungeonEntranceLog::DecrementDungeonEntrance(int,bool)
    int DecrementDungeonEntrance(int dungeon_index, bool tower);

    // IDA_EA: 0x860E38E
    // IDA_NAME: _ZN19CDungeonEntranceLog26InsertDungeonEntrance_HourEv
    // DEMANGLED: CDungeonEntranceLog::InsertDungeonEntrance_Hour(void)
    // Flushes normal_entrance_hour through LOGDB signal 225,
    // then clears only that counter. The cumulative counters stay resident.
    int InsertDungeonEntrance_Hour();

    // IDA_EA: 0x860E59C
    // IDA_NAME: _ZN19CDungeonEntranceLog21InsertDungeonEntranceEv
    // DEMANGLED: CDungeonEntranceLog::InsertDungeonEntrance(void)
    // Flushes the cumulative entrance/tower counters through LOGDB signal 224
    // and clears the complete map exactly as the original does.
    int InsertDungeonEntrance();
};

// The singleton is a global-runtime ABI entry point, but its returned object
// is the real C-owned CDungeonEntranceLog.  Publish the typed declaration with
// the owner class so all C gameplay callers invoke the member directly.
CDungeonEntranceLog* GetInstanceDungeonEntranceLog()
    __asm__("_Z29GetInstanceDungeonEntranceLogv");

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_DUNGEON_ENTRANCE_LOG_H_
