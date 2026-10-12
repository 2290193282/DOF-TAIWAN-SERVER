#ifndef OLDSRV_OLD_DATA_DB_DUNGEON_DB_UPDATE_FAIR_PVP_SCORE_H_
#define OLDSRV_OLD_DATA_DB_DUNGEON_DB_UPDATE_FAIR_PVP_SCORE_H_

#include "old_data/db/core/db_dispatcher.h"

// Old DB signal 609 consumes SIG_FAIR_PVP_SCORE written by CDataMgr slot 7.
class DB_UpdateFairPvPScore : public DBDispatcher {
public:
    // IDA_EA: 0x844F6FA
    // IDA_NAME: _ZN21DB_UpdateFairPvPScoreC2Ev
    // DEMANGLED: DB_UpdateFairPvPScore::DB_UpdateFairPvPScore(void)
    DB_UpdateFairPvPScore();

    // IDA_EA: 0x845BDAC
    // IDA_NAME: _ZN21DB_UpdateFairPvPScoreD2Ev
    // DEMANGLED: DB_UpdateFairPvPScore::~DB_UpdateFairPvPScore()
    // IDA_EA: 0x845BDDC
    // IDA_NAME: _ZN21DB_UpdateFairPvPScoreD0Ev
    // DEMANGLED: DB_UpdateFairPvPScore::~DB_UpdateFairPvPScore()
    virtual ~DB_UpdateFairPvPScore();

    // IDA_EA: 0x843FA78
    // IDA_NAME: _ZN21DB_UpdateFairPvPScore8dispatchEiiP6Stream
    // DEMANGLED: DB_UpdateFairPvPScore::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_DUNGEON_DB_UPDATE_FAIR_PVP_SCORE_H_
