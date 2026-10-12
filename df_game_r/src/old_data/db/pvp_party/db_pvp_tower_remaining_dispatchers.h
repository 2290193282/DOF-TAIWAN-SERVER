#ifndef OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_PVP_TOWER_REMAINING_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_PVP_TOWER_REMAINING_DISPATCHERS_H_

#include "old_data/db/core/db_dispatcher.h"

struct stDeathTowerRecordMemberInfo_t;
struct SIG_PARTY_STATUS;

// IDA units from 09_dungeon_pvp_party and 13_database_dispatch.  These are
// deliberately grouped by the DB domain they update, not by their former
// generated-ID folder.
class DB_StatDungeonFail : public DBDispatcher {
public:
    // IDA_EA: 0x844E16E
    // IDA_NAME: _ZN18DB_StatDungeonFailC2Ev
    // DEMANGLED: DB_StatDungeonFail::DB_StatDungeonFail(void)
    DB_StatDungeonFail();

    // IDA_EA: 0x8450070
    // IDA_NAME: _ZN18DB_StatDungeonFailD2Ev
    // DEMANGLED: DB_StatDungeonFail::~DB_StatDungeonFail()
    // IDA_EA: 0x84500A0
    // IDA_NAME: _ZN18DB_StatDungeonFailD0Ev
    // DEMANGLED: DB_StatDungeonFail::~DB_StatDungeonFail()
    virtual ~DB_StatDungeonFail();

    // IDA_EA: 0x841B6E0
    // IDA_NAME: _ZN18DB_StatDungeonFail8dispatchEiiP6Stream
    // DEMANGLED: DB_StatDungeonFail::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_StatPVPPlay2 : public DBDispatcher {
public:
    // IDA_EA: 0x844E18A
    // IDA_NAME: _ZN15DB_StatPVPPlay2C2Ev
    // DEMANGLED: DB_StatPVPPlay2::DB_StatPVPPlay2(void)
    DB_StatPVPPlay2();

    // IDA_EA: 0x84500BE
    // IDA_NAME: _ZN15DB_StatPVPPlay2D2Ev
    // DEMANGLED: DB_StatPVPPlay2::~DB_StatPVPPlay2()
    // IDA_EA: 0x84500EE
    // IDA_NAME: _ZN15DB_StatPVPPlay2D0Ev
    // DEMANGLED: DB_StatPVPPlay2::~DB_StatPVPPlay2()
    virtual ~DB_StatPVPPlay2();

    // IDA_EA: 0x841BA96
    // IDA_NAME: _ZN15DB_StatPVPPlay28dispatchEiiP6Stream
    // DEMANGLED: DB_StatPVPPlay2::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_GatheringParyStatus : public DBDispatcher {
public:
    // IDA_EA: 0x8420E1A.
    bool save_dnf_pvp_result_history(SIG_PARTY_STATUS* status, int index);
    // IDA_EA: 0x8421066.
    bool save_dnf_pvp_ping_history(SIG_PARTY_STATUS* status);
    // IDA_EA: 0x842156A.
    bool save_dnf_party_result_history(SIG_PARTY_STATUS* status, int index);
    // IDA_EA: 0x84217B6.
    bool save_dnf_party_ping_history(SIG_PARTY_STATUS* status);

    // IDA_EA: 0x844E3BA.
    DB_GatheringParyStatus();
    // IDA_EA: 0x845F2FE / 0x845F32E.
    virtual ~DB_GatheringParyStatus();
    // IDA_EA: 0x8421AEA.
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_UpdateDeathTowerBestRecord : public DBDispatcher {
public:
    DB_UpdateDeathTowerBestRecord();
    virtual ~DB_UpdateDeathTowerBestRecord();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    // IDA_EA: 0x842A35E
    // IDA_NAME: _ZN29DB_UpdateDeathTowerBestRecord11makeRequestEjjjPK30stDeathTowerRecordMemberInfo_tjj
    // DEMANGLED: DB_UpdateDeathTowerBestRecord::makeRequest(
    //     unsigned int,unsigned int,unsigned int,
    //     stDeathTowerRecordMemberInfo_t const*,unsigned int,unsigned int)
    // The first two words deliberately differ from their serialized order:
    // callers pass tower_index first, then charac_no; the request stores
    // charac_no at +0 and tower_index at +4.
    static void makeRequest(unsigned int tower_index,
                            unsigned int charac_no,
                            unsigned int member_count,
                            const stDeathTowerRecordMemberInfo_t* members,
                            unsigned int stage,
                            unsigned int play_time);
};

class DB_DeathTowerLoadTopRanker : public DBDispatcher {
public:
    DB_DeathTowerLoadTopRanker();
    virtual ~DB_DeathTowerLoadTopRanker();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(bool top_five_only);
};

class DB_UpdatePvPResult : public DBDispatcher {
public:
    DB_UpdatePvPResult();
    virtual ~DB_UpdatePvPResult();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_UpdatePvPLastPlayTime : public DBDispatcher {
public:
    DB_UpdatePvPLastPlayTime();
    virtual ~DB_UpdatePvPLastPlayTime();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class CLocalGlobal_DB_STATICS_BOSSTOWER : public DBDispatcher {
public:
    CLocalGlobal_DB_STATICS_BOSSTOWER();
    virtual ~CLocalGlobal_DB_STATICS_BOSSTOWER();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_PVP_TOWER_REMAINING_DISPATCHERS_H_
