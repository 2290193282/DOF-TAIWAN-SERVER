#ifndef OLDSRV_DB_DB_LEGACY_STAT_DISPATCHERS_H_
#define OLDSRV_DB_DB_LEGACY_STAT_DISPATCHERS_H_

#include "old_data/db/core/db_dispatcher.h"


struct MSG_STAT_BREAKJAR;
struct MSG_STAT_COMPOSITON;
struct MSG_STAT_RANKRES;
struct MSG_STAT_USECOIN;

// IDA unit: 13_database_dispatch/DB_StatBreakJar.
class DB_StatBreakJar : public DBDispatcher {
public:
    DB_StatBreakJar();
    virtual ~DB_StatBreakJar();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    int LogWrite(MSG_STAT_BREAKJAR* message);
};

// IDA unit: 13_database_dispatch/DB_StatComposition.
class DB_StatComposition : public DBDispatcher {
public:
    DB_StatComposition();
    virtual ~DB_StatComposition();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    int LogWrite(MSG_STAT_COMPOSITON* message);
};

// IDA unit: 13_database_dispatch/DB_StatCoinUse.
class DB_StatCoinUse : public DBDispatcher {
public:
    DB_StatCoinUse();
    virtual ~DB_StatCoinUse();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    int LogWrite(MSG_STAT_USECOIN& message);
};

// IDA unit: 13_database_dispatch/DB_StatGameRank.
class DB_StatGameRank : public DBDispatcher {
public:
    DB_StatGameRank();
    virtual ~DB_StatGameRank();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    int LogWrite(MSG_STAT_RANKRES* message);
};

// IDA unit: 13_database_dispatch/DB_StatUseShusiaService.
class DB_StatUseShusiaService : public DBDispatcher {
public:
    DB_StatUseShusiaService();
    virtual ~DB_StatUseShusiaService();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_LEGACY_STAT_DISPATCHERS_H_
