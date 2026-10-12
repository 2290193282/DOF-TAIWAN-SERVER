#include "old_data/db/pvp_party/db_pvp_stat_dispatchers.h"

#include "infra/network/packet/stream.h"


// IDA_EA: 0x8417C74
// IDA_NAME: _ZN19DB_StatDungeonClearC2Ev
// DEMANGLED: DB_StatDungeonClear::DB_StatDungeonClear(void)
DB_StatDungeonClear::DB_StatDungeonClear() {}

// IDA_EA: 0x8417C90
// IDA_NAME: _ZN19DB_StatDungeonClearD2Ev
// DEMANGLED: DB_StatDungeonClear::~DB_StatDungeonClear()
// IDA_EA: 0x8417CC0
// IDA_NAME: _ZN19DB_StatDungeonClearD0Ev
// DEMANGLED: DB_StatDungeonClear::~DB_StatDungeonClear() deleting destructor
DB_StatDungeonClear::~DB_StatDungeonClear() {}

// IDA_EA: 0x8417CDE
// IDA_NAME: _ZN19DB_StatDungeonClear8dispatchEiiP6Stream
// DEMANGLED: DB_StatDungeonClear::dispatch(int,int,Stream *)
bool DB_StatDungeonClear::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    MSG_STAT_DUNGEONCLEAR* message =
        stream->GetOutBuffer<MSG_STAT_DUNGEONCLEAR>();
    return message != 0 && LogWrite(message) == 1;
}

// IDA_EA: 0x8417D5C
// IDA_NAME: _ZN19DB_StatDungeonClear8LogWriteEP21MSG_STAT_DUNGEONCLEAR
// DEMANGLED: DB_StatDungeonClear::LogWrite(MSG_STAT_DUNGEONCLEAR *)
int DB_StatDungeonClear::LogWrite(MSG_STAT_DUNGEONCLEAR* message) {
    (void)message;
    return 1;
}

// IDA_EA: 0x8417E58
// IDA_NAME: _ZN14DB_StatPvpPlayC2Ev
// DEMANGLED: DB_StatPvpPlay::DB_StatPvpPlay(void)
DB_StatPvpPlay::DB_StatPvpPlay() {}

// IDA_EA: 0x8417E74
// IDA_NAME: _ZN14DB_StatPvpPlayD2Ev
// DEMANGLED: DB_StatPvpPlay::~DB_StatPvpPlay()
// IDA_EA: 0x8417EA4
// IDA_NAME: _ZN14DB_StatPvpPlayD0Ev
// DEMANGLED: DB_StatPvpPlay::~DB_StatPvpPlay() deleting destructor
DB_StatPvpPlay::~DB_StatPvpPlay() {}

// IDA_EA: 0x8417EC2
// IDA_NAME: _ZN14DB_StatPvpPlay8dispatchEiiP6Stream
// DEMANGLED: DB_StatPvpPlay::dispatch(int,int,Stream *)
bool DB_StatPvpPlay::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    MSG_STAT_PVPPLAY* message = stream->GetOutBuffer<MSG_STAT_PVPPLAY>();
    return message != 0 && LogWrite(message) == 1;
}

// IDA_EA: 0x8417F40
// IDA_NAME: _ZN14DB_StatPvpPlay8LogWriteEP16MSG_STAT_PVPPLAY
// DEMANGLED: DB_StatPvpPlay::LogWrite(MSG_STAT_PVPPLAY *)
int DB_StatPvpPlay::LogWrite(MSG_STAT_PVPPLAY* message) {
    (void)message;
    return 1;
}
