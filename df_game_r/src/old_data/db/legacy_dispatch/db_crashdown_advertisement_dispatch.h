#ifndef OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_CRASHDOWN_ADVERTISEMENT_DISPATCH_H_
#define OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_CRASHDOWN_ADVERTISEMENT_DISPATCH_H_

#include "old_data/db/core/db_dispatcher.h"


// These are real DB-domain providers restored by executor A from the paused-B
// slice.  Their canonical source destination remains the DB domain.  They
// intentionally have no DBTask registration here: the original dispatcher
// table has unresolved neighboring slots and must be restored as one route.
class DB_CrashDownReport : public DBDispatcher {
public:
    DB_CrashDownReport();
    virtual ~DB_CrashDownReport();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_UpdateClientCrashDownLog : public DBDispatcher {
public:
    DB_UpdateClientCrashDownLog();
    virtual ~DB_UpdateClientCrashDownLog();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_NoticeIngameAdvertisement : public DBDispatcher {
public:
    DB_NoticeIngameAdvertisement();
    virtual ~DB_NoticeIngameAdvertisement();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};


#endif  // OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_CRASHDOWN_ADVERTISEMENT_DISPATCH_H_
