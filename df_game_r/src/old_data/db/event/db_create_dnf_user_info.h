#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_CREATE_DNF_USER_INFO_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_CREATE_DNF_USER_INFO_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"

class Stream;

struct DBSelectCreateDnfUserInfo : public TaiwanInterfaceDBDispatcher {
    struct MSG { unsigned int account_id; };

    // IDA_EA: 0x815E316
    // IDA_NAME: _ZN25DBSelectCreateDnfUserInfoC2Ev
    DBSelectCreateDnfUserInfo();

    // IDA_EA: 0x815E33A (D2), 0x815E36A (D0)
    // IDA_NAME: _ZN25DBSelectCreateDnfUserInfoD2Ev / D0Ev
    virtual ~DBSelectCreateDnfUserInfo();

    static void makeRequest(int uid, unsigned int account_id);
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};

struct DBInsertCreateDnfUserInfo : public TaiwanInterfaceDBDispatcher {
    struct MSG { unsigned int account_id; };

    // IDA_EA: 0x815E388
    // IDA_NAME: _ZN25DBInsertCreateDnfUserInfoC2Ev
    DBInsertCreateDnfUserInfo();

    // IDA_EA: 0x815E3AC (D2), 0x815E3DC (D0)
    // IDA_NAME: _ZN25DBInsertCreateDnfUserInfoD2Ev / D0Ev
    virtual ~DBInsertCreateDnfUserInfo();

    static void makeRequest(int uid, unsigned int account_id);
    virtual int dispatch(int signal, int user_idx, Stream* stream);
};

#if defined(__i386__)
static_assert(sizeof(DBSelectCreateDnfUserInfo) == 0x08u,
              "DBSelectCreateDnfUserInfo must not extend the Taiwan DB ABI");
static_assert(sizeof(DBInsertCreateDnfUserInfo) == 0x08u,
              "DBInsertCreateDnfUserInfo must not extend the Taiwan DB ABI");
#endif

#endif
